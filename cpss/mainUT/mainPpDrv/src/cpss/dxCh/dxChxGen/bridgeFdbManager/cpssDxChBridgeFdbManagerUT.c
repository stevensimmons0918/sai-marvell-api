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
* @file cpssDxChBridgeFdbManagerUT.c
*
* @brief Unit tests for cpssDxChBridgeFdbManager.c/h, that provides
* 'FDB manager' CPSS DxCh facility implementation.
*
* @version   1
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/bridgeFdbManager/cpssDxChBrgFdbManager.h>
/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_utils.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_db.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_debug.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_hw.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrityTables.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfBridgeGen.h>
#include <extUtils/tgf/tgfBridgeGen.h>

/* fdbSize is initialized at the beginning of the test suite (create), only once
 * In case of shared memory, the size is = actual/2 */
static GT_U32    fdbSize;

#define FDB_SIZE_FULL_INIT()                                                        \
    if(!fdbSize)                                                                    \
    {                                                                               \
        fdbSize = (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.fdb);   \
    }

/* use FDB size that match the fineTunning (128K in Falcon/AC5P , 32K with Phoenix)
 * In case of shared memory build, due to memory constraints used 32K size */
#if defined(SHARED_MEMORY) || defined(CPSS_USE_MUTEX_PROFILER)
#define FDB_SIZE_FULL                   ((fdbSize > _32K) ? _32K : fdbSize)
#else
#define FDB_SIZE_FULL                   fdbSize
#endif

#define HASH_AS_PER_DEVICE(num)         ((PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))?num:(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?16:1)
#define HASH_MODE_AS_PER_DEVICE(mode)   ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E:mode)
#define SHADOW_TYPE_PER_DEVICE()        ((userDefinedShadowType)?(globalShadowType):((PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))?CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E:(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E:CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E))
#define BANK_AS_PER_DEVICE(bank)        ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?bank:bank%3)
#define FID_MASK_AS_PER_DEVICE()        ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?(0x1FFF):(0xFFF))
#define DEV_ID_MASK_AS_PER_DEVICE()     ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?(0x3FF ):(0X1F ))
#define PORT_MASK_AS_PER_DEVICE()       ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?(0x1FFF):(0x3F ))
#define TRUNK_MASK_AS_PER_DEVICE()      ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?(0xFFF ):(0x7F ))
#define VIDX_MASK_AS_PER_DEVICE()       ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?(0x3FFF):(0xFFF))
#define IS_SIP5_OR_HYBRID               (((SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E) || (SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E)))
#define TUNNEL_POINTER_MASK_AS_PER_DEVICE()     ((IS_SIP5_OR_HYBRID)?(0x7FFF):(0xFFFF))
#define ARP_POINTER_MASK_AS_PER_DEVICE()        ((IS_SIP5_OR_HYBRID)?(0x1FFFF):(0xFFFF))

#define CLEAR_VAR_MAC(var)  \
    cpssOsMemSet(&var,0,sizeof(var))

extern void *malloc(size_t size);
extern void free(void *ptr);
void ipv6UcAddressGet(GT_U32 iteration, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC  *entryPtr, GT_BOOL isRand, GT_BOOL updateDipLowOctets);
typedef struct {
    GT_U32      expErrArrSize;
    char       *errString;
    GT_U32      expErrArr[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
} FDB_MANAGER_DEBUG_ERROR_INFO;

/* Global dbCheck(All test selected) */
static CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC                 dbCheckParam;

/* Used List 1st pointer error, skips other checks in used list */
#ifdef PRV_CPSS_DXCH_BRG_FDB_MANAGER_ALLOW_HW_BANK_COUNTER_UPDATE_CNS
#define FDB_SELF_TEST_ALL_ERR  {                                                           \
/*0 */    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E                    \
/*1 */   ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E                      \
/*2 */   ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E                      \
/*3 */   ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_SW_BANK_COUNTERS_MISMATCH_E               \
/*4 */   ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E                  \
/*5 */   ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E             \
/*6 */   ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E                  \
/*7    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E            */     \
/*8    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E         */     \
/*9    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_TRANS_SCAN_PTR_INVALID_E      */     \
/*10    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E    */     \
/*11    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E     */     \
/*12    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E       */     \
/*13*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E                   \
/*14*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E                         \
/*15*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E              \
/*16*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E                    \
/*17*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E               \
/*18*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E                      \
/*19*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E               \
/*20*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E               \
/*21*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E                            \
/*22*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E                   \
/*23*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E                      \
/*24*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_GLOBAL_INVALID_E                  \
/*25    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_FID_INVALID_E              */     \
/*26    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_GLOBAL_EPORT_INVALID_E     */     \
/*27    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_TRUNK_INVALID_E            */     \
/*28    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_PORT_INVALID_E             */     \
/*29    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_SRC_INTERFACES_CURRENT_INVALID_E*/\
    }
#else
#define FDB_SELF_TEST_ALL_ERR  {                                                           \
/* 0*/   CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E                     \
/* 1*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E                       \
/* 2*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E                       \
/* 3*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E                   \
/* 4*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E              \
/* 5*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E                   \
/* 6*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E                   \
/* 7*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E                         \
/* 8*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E              \
/* 9*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E                    \
/*10*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E               \
/*11*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E                      \
/*12*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E               \
/*13*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E               \
/*14*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E                            \
/*15*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E                   \
/*16*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E                      \
/*17*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_GLOBAL_INVALID_E                  \
    }
#endif

static const FDB_MANAGER_DEBUG_ERROR_INFO debugErrorArr[] =
{
    /*Error Count*/ /*Error String*/                                                          /*Exp Errors*/    /*Enum Num*/
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E"          , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E}            }, /* 0*/

    {2              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E"            , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_GLOBAL_INVALID_E}            }, /* 1*/
#ifdef PRV_CPSS_DXCH_BRG_FDB_MANAGER_ALLOW_HW_BANK_COUNTER_UPDATE_CNS
    {2              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E"            , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_SW_BANK_COUNTERS_MISMATCH_E}          }, /* 2*/
    {2              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_SW_BANK_COUNTERS_MISMATCH_E"     , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_SW_BANK_COUNTERS_MISMATCH_E}          }, /* 3*/
#else
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E"            , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E}          }, /* 2*/
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_SW_BANK_COUNTERS_MISMATCH_E"     , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E}          }, /* 3*/
#endif
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E"        , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E}            }, /* 4*/
    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E"   , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E}      }, /* 5*/

    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E"        , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E}       }, /* 6*/
    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E"       , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E}       }, /* 7*/
    {5              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E"    , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E}}, /* 8*/
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_TRANS_SCAN_PTR_INVALID_E" , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_TRANS_SCAN_PTR_INVALID_E}            }, /* 9*/
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E", {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E}           }, /*10*/
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E" , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E}           }, /*11*/
    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E"   , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E}      }, /*12*/

    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E"        , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E}       }, /*13*/
    {2              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E"              , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E}         }, /*14*/

    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E"   , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E}     }, /*15*/
    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E"         , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E}      }, /*16*/
    {4              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E"    , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E}  }, /*17*/
    {5              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E"           , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E}}, /*18*/
    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E"    , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E}     }, /*19*/
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E"    , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E}           }, /*20*/

    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E"                 , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E}           }, /*21*/
    {5              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E"        , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E}}, /*22*/
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E"           , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E}           }, /*23*/
#ifdef PRV_CPSS_DXCH_BRG_FDB_MANAGER_ALLOW_HW_BANK_COUNTER_UPDATE_CNS
    {19             , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ALL_E"                              , FDB_SELF_TEST_ALL_ERR},
#else
    {18             , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ALL_E"                              , FDB_SELF_TEST_ALL_ERR},
#endif
    {GT_NA,"must be last",{0}}
};

/* Used List 1st pointer error, skips other checks in used list */
#define AC5_FDB_SELF_TEST_ALL_ERR  {                                                       \
/*0 */    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E                    \
/*1 */   ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E                      \
/*2 */   ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E                      \
/*3      ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_SW_BANK_COUNTERS_MISMATCH_E        */     \
/*4 */   ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E                  \
/*5 */   ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E             \
/*6 */   ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E                  \
/*7    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E            */     \
/*8    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E         */     \
/*9    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_TRANS_SCAN_PTR_INVALID_E      */     \
/*10    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E    */     \
/*11    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E     */     \
/*12    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E       */     \
/*13*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E                   \
/*14*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E                         \
/*15*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E              \
/*16*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E                    \
/*17*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E               \
/*18*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E                      \
/*19*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E               \
/*20*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E               \
/*21*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E                            \
/*22*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E                   \
/*23*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E                      \
/*24*/  ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_GLOBAL_INVALID_E                  \
/*25    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_FID_INVALID_E              */     \
/*26    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_GLOBAL_EPORT_INVALID_E     */     \
/*27    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_TRUNK_INVALID_E            */     \
/*28    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_PORT_INVALID_E             */     \
/*29    ,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_SRC_INTERFACES_CURRENT_INVALID_E*/\
    }


static const FDB_MANAGER_DEBUG_ERROR_INFO debugErrorArr_AC5[] =
{
    /*Error Count*/ /*Error String*/                                                          /*Exp Errors*/    /*Enum Num*/
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E"          , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E}            }, /* 0*/

    {2              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E"            , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_GLOBAL_INVALID_E}            }, /* 1*/
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E"            , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E}            }, /* 2*/
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_SW_BANK_COUNTERS_MISMATCH_E"     , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E}            }, /* 3*/

    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E"        , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E}            }, /* 4*/
    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E"   , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E}      }, /* 5*/

    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E"        , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E}       }, /* 6*/
    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E"       , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E}       }, /* 7*/
    {5              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E"    , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E}}, /* 8*/
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_TRANS_SCAN_PTR_INVALID_E" , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_TRANS_SCAN_PTR_INVALID_E}            }, /* 9*/
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E", {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E}           }, /*10*/
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E" , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E}           }, /*11*/
    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E"   , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E}      }, /*12*/

    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E"        , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E}       }, /*13*/
    {2              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E"              , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E}         }, /*14*/

    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E"   , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E}     }, /*15*/
    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E"         , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E}      }, /*16*/
    {4              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E"    , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E}  }, /*17*/
    {5              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E"           , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E}}, /*18*/
    {3              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E"    , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E}     }, /*19*/
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E"    , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E}           }, /*20*/

    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E"                 , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E}           }, /*21*/
    {5              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E"        , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E,CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E}}, /*22*/
    {1              , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E"           , {CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E}           }, /*23*/
    {18             , "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ALL_E"                              , AC5_FDB_SELF_TEST_ALL_ERR},

    {GT_NA,"must be last",{0}}
};


/**
* @enum UTF_PRV_CPSS_API_STATISTICS_ENT
 *
 * @brief This enum defines API statistics types
*/
typedef enum {
    /** @brief add API statistics */
     UTF_PRV_CPSS_API_STATISTICS_ADD_E
    /** @brief delet API statistics */
    ,UTF_PRV_CPSS_API_STATISTICS_DELETE_E
    /** @brief update API statistics */
    ,UTF_PRV_CPSS_API_STATISTICS_UPDATE_E
    /** @brief scan learning API statistics */
    ,UTF_PRV_CPSS_API_STATISTICS_SCAN_LEARNING_E
} UTF_PRV_CPSS_BRG_FDB_API_STATISTICS_ENT;

static CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC        statistics_global;
static void fdbManagerEntry_updateDbStatistics(
    IN  UTF_PRV_CPSS_BRG_FDB_API_STATISTICS_ENT         statisticType,
    IN  GT_STATUS                                       status,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC        *statisticsPtr
);

static GT_BOOL         globalRehashEnable = GT_FALSE;

#define FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, entryPtr, paramsPtr, st)        \
    st = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId, entryPtr, paramsPtr);                  \
    fdbManagerEntry_updateDbStatistics(UTF_PRV_CPSS_API_STATISTICS_ADD_E, st, &statistics_global)

#define FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId, entryPtr, st)                \
    st = cpssDxChBrgFdbManagerEntryDelete(fdbManagerId, entryPtr);                          \
    fdbManagerEntry_updateDbStatistics(UTF_PRV_CPSS_API_STATISTICS_DELETE_E, st, &statistics_global)

#define FDB_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(fdbManagerId, entryPtr, paramsPtr, st)     \
    st = cpssDxChBrgFdbManagerEntryUpdate(fdbManagerId, entryPtr, paramsPtr);               \
    fdbManagerEntry_updateDbStatistics(UTF_PRV_CPSS_API_STATISTICS_UPDATE_E, st, &statistics_global)

#define START_TIME                                          \
        rc = cpssOsTimeRT(&secondsStart,&nanoSecondsStart); \
        if(rc != GT_OK)                                     \
        {                                                   \
            return rc;                                      \
        }

#define HOW_MUCH_TIME(apiName,numIter)                                              \
    rc = cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);                                 \
    if(rc != GT_OK)                                                                 \
    {                                                                               \
        return rc;                                                                  \
    }                                                                               \
                                                                                    \
    seconds = secondsEnd-secondsStart;                                              \
    if(nanoSecondsEnd >= nanoSecondsStart)                                          \
    {                                                                               \
        nanoSec = nanoSecondsEnd-nanoSecondsStart;                                  \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        nanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsEnd;                 \
        seconds--;                                                                  \
    }                                                                               \
    cpssOsPrintf("[%s] [%d] entries , total time takes : %d sec., %d nanosec.\n",   \
        #apiName, numIter , seconds, nanoSec)

extern GT_STATUS cpssDxChFdbDump(IN GT_U8 devNum);

/* wrapper for standard malloc to avoid warnings about size_t type */
GT_VOID * utfMalloc
(
    IN GT_U32 size
)
{
   return malloc(size);
}

#define PRV_TGF_AU_MSG_RATE_STAGE_A                 _128K
#define PRV_TGF_AU_MSG_RATE_STAGE_B                 _5K
/**
* @enum PRV_TGF_BRG_FDB_MANAGER_THRESHOLD_TYPE_ENT
 *
 * @brief This enum defines the type of threshold
*/
typedef enum {

    /** @brief The FDB utilization is under Threshold_A */
    PRV_TGF_BRG_FDB_MANAGER_THRESHOLD_TYPE_A_E,

    /** @brief The FDB utilization in under Threshold_B */
    PRV_TGF_BRG_FDB_MANAGER_THRESHOLD_TYPE_B_E,

    /** @brief The FDB utilization is under Threshold_C */
    PRV_TGF_BRG_FDB_MANAGER_THRESHOLD_TYPE_C_E

} PRV_TGF_BRG_FDB_MANAGER_THRESHOLD_TYPE_ENT;

/**
* @enum UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC
 *
 * @brief Structure returns information as a result of FDB API call
*/
typedef struct {
    /** @brief (pointer to) FDB manager entry */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC     *entryPtr;
    /** @brief (pointer to) result of API add function
     *  GT_TRUE     - entry added to FDB
     *  GT_FALSE    - entry not added to FDB */
    GT_BOOL                                 *entryAddedPtr;
} UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC;

/* defines */
/* we force application to give 'num of entries' in steps of 256 */
#define NUM_ENTRIES_STEPS_CNS 256

/* static GT_BOOL      isHwBankCounterSupported = GT_TRUE;*/
/* perform test of cleaning after FDB Manager tests */
void prvTgf_testsFDB(void)
{
    GT_STATUS st;
    GT_U8 devNum = prvTgfDevNum;
    GT_U32 ii, value;

    if(!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        return;
    }
    /* check bank counters. They must be 0. */
    for(ii = 0; ii< 16; ii++)
    {
        st = cpssDxChBrgFdbBankCounterValueGet(devNum, ii, &value);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ii);
        UTF_VERIFY_EQUAL2_PARAM_MAC(0, value, devNum, ii);
    }
}

static void restoreGlobalEport(void);

/* cleanup on specific manager */
#define CLEANUP_MANAGER(fdbManagerId) \
    (void)cpssDxChBrgFdbManagerDelete(fdbManagerId);

/* cleanup on ALL managers */
#define CLEANUP_ALL_MANAGERS \
    {                        \
        GT_U32  ii;          \
        for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++) \
        {                                                                       \
            (void)cpssDxChBrgFdbManagerDelete(ii);                              \
        }                                                                       \
        prvTgf_testsFDB(); \
        restoreGlobalEport(); \
    }

static GT_U32   checkNonSupportedValues[32] = {
    /*0*/ 0xFFFFFFFF,
    /*1*/ 0xA5A5A5A5,
    /*2*/ 0x7FFFFFFF
};

static GT_U16   checkNonSupportedValues_U16[32] = {
    /*0*/ 0xFFFF,
    /*1*/ 0xA5A5,
    /*2*/ 0x7FFF
};

/* check bounders of numeric field , and high values (31,32 bits value)  :
expected to fail */
#define CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(                   \
    fieldName,                                                                      \
    /*IN GT_U32                                          */ testedField_minValue,   \
    /*IN GT_U32                                          */ testedField_maxValue,   \
    /*IN GT_U32                                          */ step                    \
)                                                                                   \
{                                                                                   \
    GT_U32  ii,iiMax = 3,iiMin;                                                     \
    GT_U32  origValue = fieldName;                                                  \
    iiMax = 3;                                                                      \
    if((testedField_minValue) != 0) /* we check 0xFFFFFFFF anyway */                \
    {                                                                               \
        checkNonSupportedValues[iiMax++] = (GT_U32)((testedField_minValue) - 1);    \
    }                                                                               \
    checkNonSupportedValues[iiMax++] = (GT_U32)((testedField_maxValue) + 1);        \
                                                                                    \
    for(ii = 0; ii < iiMax; ii++)                                                   \
    {                                                                               \
        fieldName = checkNonSupportedValues[ii];                                    \
        st = cpssDxChBrgFdbManagerCreate(fdbManagerId,                              \
            &capacityInfo,&entryAttrInfo,&learningInfo,&lookupInfo,&agingInfo);     \
        /* restore orig value */                                                    \
        fieldName = origValue;                                                      \
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,                              \
            "cpssDxChBrgFdbManagerCreate: %s expected to GT_BAD_PARAM on value [%d]",\
            #fieldName,                                                             \
            checkNonSupportedValues[ii]);                                           \
    }                                                                               \
                                                                                    \
    iiMax = testedField_maxValue;                                                   \
    iiMin = testedField_minValue;                                                   \
    for(ii = iiMin ; ii <= iiMax; ii+=step)                                         \
    {                                                                               \
        fieldName = ii;                                                             \
        st = cpssDxChBrgFdbManagerCreate(fdbManagerId,                              \
            &capacityInfo,&entryAttrInfo,&learningInfo,&lookupInfo,&agingInfo);     \
        /* kill the manager ... for next time */                                    \
        CLEANUP_MANAGER(fdbManagerId);                                              \
        /* restore orig value */                                                    \
        fieldName = origValue;                                                      \
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,                                     \
            "cpssDxChBrgFdbManagerCreate: %s expected to GT_OK on value [%d]",      \
            #fieldName,                                                             \
            ii);                                                                    \
        if(ii < step || ii == 0) break;                                             \
    }                                                                               \
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(         \
    fieldName,                                                                  \
    /*IN GT_U32                                          */ testedField_Value   \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChBrgFdbManagerCreate(fdbManagerId,                              \
        &capacityInfo,&entryAttrInfo,&learningInfo,&lookupInfo,&agingInfo);     \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,                              \
        "cpssDxChBrgFdbManagerCreate: %s expected to GT_BAD_PARAM on value [%d]",\
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/* check NULL pointer param , expected to fail */
#define FAIL_NULL_PTR_cpssDxChBrgFdbManagerCreate(                              \
    paramPtr /*one of : ptr1,ptr2,ptr3,ptr4,ptr5*/,                             \
    ptr1,ptr2,ptr3,ptr4,ptr5                                                    \
)                                                                               \
{                                                                               \
    GT_VOID*  origPtr = (GT_VOID*)paramPtr;                                     \
                                                                                \
    paramPtr = NULL;                                                            \
                                                                                \
    st = cpssDxChBrgFdbManagerCreate(fdbManagerId,                              \
        ptr1,ptr2,ptr3,ptr4,ptr5);                                              \
    /* restore orig value */                                                    \
    paramPtr = origPtr;                                                         \
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,                                \
        "cpssDxChBrgFdbManagerCreate: %s is NULL pointer expected GT_BAD_PTR",  \
        #paramPtr);                                                             \
}

/* check NULL pointer param , expected to fail */
#define FAIL_NULL_PTR_cpssDxChBrgFdbManagerConfigGet(                           \
    paramPtr /*one of : ptr1,ptr2,ptr3,ptr4,ptr5*/,                             \
    ptr1,ptr2,ptr3,ptr4,ptr5                                                    \
)                                                                               \
{                                                                               \
    GT_VOID*  origPtr = (GT_VOID*)paramPtr;                                     \
                                                                                \
    paramPtr = NULL;                                                            \
                                                                                \
    st = cpssDxChBrgFdbManagerConfigGet(fdbManagerId,                              \
        ptr1,ptr2,ptr3,ptr4,ptr5);                                              \
    /* restore orig value */                                                    \
    paramPtr = origPtr;                                                         \
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,                                \
        "cpssDxChBrgFdbManagerConfigGet: %s is NULL pointer expected GT_BAD_PTR",  \
        #paramPtr);                                                             \
}


#define UTF_CPSS_PP_ALL_SIP5_CNS             (UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E | UTF_BOBCAT3_E)
#define FDB_MANAGER_NON_SUPPORTED_DEV_LIST  ~(UTF_CPSS_PP_ALL_SIP6_CNS | UTF_AC5_E | UTF_CPSS_PP_ALL_SIP5_CNS)

/* macro to skip devices that not applicable for the FDB manager 'device-less APIs'*/
#define DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE  \
    {                                               \
        GT_U8   dev;                                \
        /* prepare device iterator */               \
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, FDB_MANAGER_NON_SUPPORTED_DEV_LIST);\
        if (GT_OK != prvUtfNextDeviceGet(&dev, GT_TRUE))                       \
        {                                                                      \
            /* the current running device is NOT in the supporting list */     \
            SKIP_TEST_MAC;                                                     \
        }                                                                      \
    }                                                                          \
    FDB_SIZE_FULL_INIT()

/*UTF_SIP6_GM_NOT_READY_SKIP_MAC(_log_string)*/

#define TESTED_FDB_MANAGER_ID_CNS 17

static GT_BOOL  userDefinedShadowType             = GT_FALSE;
static CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT    globalShadowType  = CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E;

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbManagerCreate
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           *capacityPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             *lookupPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              *agingPtr
);
*/
static void internal_cpssDxChBrgFdbManagerCreateUT(void)
{
    GT_STATUS   st;
    IN GT_U32                                           fdbManagerId;
    IN CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           capacityInfo,*capacityInfoPtr;
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfo,*entryAttrInfoPtr;
    IN CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           learningInfo,*learningInfoPtr;
    IN CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             lookupInfo,*lookupInfoPtr;
    IN CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              agingInfo,*agingInfoPtr;

    CLEAR_VAR_MAC(capacityInfo );
    CLEAR_VAR_MAC(entryAttrInfo);
    CLEAR_VAR_MAC(learningInfo );
    CLEAR_VAR_MAC(lookupInfo   );
    CLEAR_VAR_MAC(agingInfo    );

    capacityInfoPtr  = &capacityInfo  ;
    entryAttrInfoPtr = &entryAttrInfo ;
    learningInfoPtr  = &learningInfo  ;
    lookupInfoPtr    = &lookupInfo    ;
    agingInfoPtr     = &agingInfo     ;

    /************************/
    /* create valid manager */
    /* proof of good parameters */
    /************************/
    fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;

    capacityInfo.hwCapacity.numOfHwIndexes = FDB_SIZE_FULL;
    capacityInfo.hwCapacity.numOfHashes    = HASH_AS_PER_DEVICE(16);
    capacityInfo.hwCapacity.mode           = HASH_MODE_AS_PER_DEVICE(0);
    capacityInfo.maxTotalEntries           = 1024;
    capacityInfo.maxEntriesPerAgingScan    = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerDeleteScan   = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerTransplantScan = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerLearningScan = NUM_ENTRIES_STEPS_CNS; /*256*/

    entryAttrInfo.macEntryMuxingMode  = CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E;
    entryAttrInfo.ipmcEntryMuxingMode = CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E;
    entryAttrInfo.saDropCommand       = CPSS_PACKET_CMD_DROP_SOFT_E;
    entryAttrInfo.daDropCommand       = CPSS_PACKET_CMD_DROP_HARD_E;
    entryAttrInfo.ipNhPacketcommand   = CPSS_PACKET_CMD_ROUTE_E;
    entryAttrInfo.shadowType          = SHADOW_TYPE_PER_DEVICE();

    learningInfo.macNoSpaceUpdatesEnable = GT_FALSE;
    learningInfo.macRoutedLearningEnable = GT_FALSE;
    learningInfo.macVlanLookupMode       = CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E;

    lookupInfo.crcHashUpperBitsMode     = CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;
    lookupInfo.ipv4PrefixLength         = 32;
    lookupInfo.ipv6PrefixLength         = 128;

    agingInfo.destinationUcRefreshEnable = GT_FALSE;
    agingInfo.destinationMcRefreshEnable = GT_FALSE;
    agingInfo.ipUcRefreshEnable          = GT_FALSE;

    st = cpssDxChBrgFdbManagerCreate(fdbManagerId,
        &capacityInfo,&entryAttrInfo,&learningInfo,&lookupInfo,&agingInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerCreate ");

    CLEANUP_MANAGER(fdbManagerId);

    /***** we have good parameters , lets check changes in them ***/

    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
        capacityInfo.hwCapacity.numOfHwIndexes,
        (FDB_SIZE_FULL-1));

    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
        capacityInfo.hwCapacity.numOfHwIndexes,
        (_256K+1));

    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
        capacityInfo.hwCapacity.numOfHashes,
        15);

    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
        capacityInfo.hwCapacity.numOfHashes,
        17);

    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
        capacityInfo.maxTotalEntries,
        0);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
        capacityInfo.maxTotalEntries,
        NUM_ENTRIES_STEPS_CNS,/* must not be ZERO (and must be in steps of 256)*/
        capacityInfo.hwCapacity.numOfHwIndexes,
        NUM_ENTRIES_STEPS_CNS/*step of NUM_ENTRIES_STEPS_CNS*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
        capacityInfo.maxEntriesPerAgingScan,
        NUM_ENTRIES_STEPS_CNS,
        capacityInfo.maxTotalEntries,
        NUM_ENTRIES_STEPS_CNS/*step*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
        capacityInfo.maxEntriesPerDeleteScan,
        NUM_ENTRIES_STEPS_CNS,
        capacityInfo.maxTotalEntries,
        NUM_ENTRIES_STEPS_CNS/*step*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
        capacityInfo.maxEntriesPerTransplantScan,
        NUM_ENTRIES_STEPS_CNS,
        capacityInfo.maxTotalEntries,
        NUM_ENTRIES_STEPS_CNS/*step*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
        capacityInfo.maxEntriesPerLearningScan,
        NUM_ENTRIES_STEPS_CNS,
        capacityInfo.maxTotalEntries,
        NUM_ENTRIES_STEPS_CNS/*step*/);

    /* Valid for SIP-5 */
    if(IS_SIP5_OR_HYBRID)
    {
        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
                entryAttrInfo.ipNhPacketcommand,
                CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
                1/*step*/);

        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
                lookupInfo.ipv4PrefixLength,
                0,
                32,
                1/*step*/);

        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
                lookupInfo.ipv6PrefixLength,
                0,
                128,
                1/*step*/);

        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
                lookupInfo.crcHashUpperBitsMode,
                CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E,
                CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_USE_MAC_E,
                1/*step*/);
    }
    else if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
    {
        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
                entryAttrInfo.macEntryMuxingMode,
                CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E,
                CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_8_SRC_ID_3_DA_ACCESS_LEVEL_E,
                1/*step*/);

        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
                entryAttrInfo.ipmcEntryMuxingMode,
                CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E,
                CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_E,
                1/*step*/);

        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
                entryAttrInfo.saDropCommand,
                CPSS_PACKET_CMD_DROP_HARD_E,
                CPSS_PACKET_CMD_DROP_SOFT_E,
                1/*step*/);

        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
                entryAttrInfo.daDropCommand,
                CPSS_PACKET_CMD_DROP_HARD_E,
                CPSS_PACKET_CMD_DROP_SOFT_E,
                1/*step*/);
    }

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerCreate(
        learningInfo.macVlanLookupMode,
        CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_ONLY_E,
        CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E,
        1/*step*/);

    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChBrgFdbManagerCreate(capacityInfoPtr/*checked for NULL*/,
        capacityInfoPtr,entryAttrInfoPtr,learningInfoPtr,lookupInfoPtr,agingInfoPtr);
    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChBrgFdbManagerCreate(entryAttrInfoPtr/*checked for NULL*/,
        capacityInfoPtr,entryAttrInfoPtr,learningInfoPtr,lookupInfoPtr,agingInfoPtr);
    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChBrgFdbManagerCreate(learningInfoPtr/*checked for NULL*/,
        capacityInfoPtr,entryAttrInfoPtr,learningInfoPtr,lookupInfoPtr,agingInfoPtr);
    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChBrgFdbManagerCreate(lookupInfoPtr/*checked for NULL*/,
        capacityInfoPtr,entryAttrInfoPtr,learningInfoPtr,lookupInfoPtr,agingInfoPtr);
    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChBrgFdbManagerCreate(agingInfoPtr/*checked for NULL*/,
        capacityInfoPtr,entryAttrInfoPtr,learningInfoPtr,lookupInfoPtr,agingInfoPtr);

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbManagerCreate
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           *capacityPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             *lookupPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              *agingPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerCreate)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerCreateUT();

    CLEANUP_ALL_MANAGERS;
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbManagerConfigGet
(
    IN  GT_U32                                          fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC          *capacityPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC          *learningPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC            *lookupPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC             *agingPtr
);
*/
static void internal_cpssDxChBrgFdbManagerConfigGetUT(void)
{
    GT_STATUS   st;
    IN  GT_U32                                           fdbManagerId;
    IN  CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           capacityInfo,*capacityInfoPtr;
    IN  CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfo,*entryAttrInfoPtr;
    IN  CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           learningInfo,*learningInfoPtr;
    IN  CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             lookupInfo,*lookupInfoPtr;
    IN  CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              agingInfo,*agingInfoPtr;

    OUT CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           capacityInfoGet;
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfoGet;
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           learningInfoGet;
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             lookupInfoGet;
    OUT CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              agingInfoGet;


    CLEAR_VAR_MAC(capacityInfo );
    CLEAR_VAR_MAC(entryAttrInfo);
    CLEAR_VAR_MAC(learningInfo );
    CLEAR_VAR_MAC(lookupInfo   );
    CLEAR_VAR_MAC(agingInfo    );

    CLEAR_VAR_MAC(capacityInfoGet );
    CLEAR_VAR_MAC(entryAttrInfoGet);
    CLEAR_VAR_MAC(learningInfoGet );
    CLEAR_VAR_MAC(lookupInfoGet   );
    CLEAR_VAR_MAC(agingInfoGet    );


    capacityInfoPtr  = &capacityInfoGet  ;
    entryAttrInfoPtr = &entryAttrInfoGet ;
    learningInfoPtr  = &learningInfoGet  ;
    lookupInfoPtr    = &lookupInfoGet    ;
    agingInfoPtr     = &agingInfoGet     ;

    /************************/
    /* create valid manager */
    /* proof of good parameters */
    /************************/
    fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;

    capacityInfo.hwCapacity.numOfHwIndexes = FDB_SIZE_FULL;
    capacityInfo.hwCapacity.numOfHashes    = HASH_AS_PER_DEVICE(16);
    capacityInfo.hwCapacity.mode           = HASH_MODE_AS_PER_DEVICE(0);
    capacityInfo.maxTotalEntries           = 1024;
    capacityInfo.maxEntriesPerAgingScan    = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerDeleteScan   = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerTransplantScan = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerLearningScan = NUM_ENTRIES_STEPS_CNS; /*256*/

    entryAttrInfo.macEntryMuxingMode  = CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E;
    entryAttrInfo.ipmcEntryMuxingMode = CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E;
    entryAttrInfo.saDropCommand       = CPSS_PACKET_CMD_DROP_SOFT_E;
    entryAttrInfo.daDropCommand       = CPSS_PACKET_CMD_DROP_HARD_E;
    entryAttrInfo.ipNhPacketcommand   = CPSS_PACKET_CMD_ROUTE_E;
    entryAttrInfo.shadowType          = SHADOW_TYPE_PER_DEVICE();

    learningInfo.macNoSpaceUpdatesEnable = GT_FALSE;
    learningInfo.macRoutedLearningEnable = GT_FALSE;
    learningInfo.macVlanLookupMode       = CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E;

    lookupInfo.crcHashUpperBitsMode     = CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;
    lookupInfo.ipv4PrefixLength         = 32;
    lookupInfo.ipv6PrefixLength         = 128;

    agingInfo.destinationUcRefreshEnable = GT_FALSE;
    agingInfo.destinationMcRefreshEnable = GT_FALSE;
    agingInfo.ipUcRefreshEnable          = GT_FALSE;

    /* Create FDB manager */
    st = cpssDxChBrgFdbManagerCreate(fdbManagerId,
        &capacityInfo,&entryAttrInfo,&learningInfo,&lookupInfo,&agingInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerCreate ");

    /* Retrieve data */
    st = cpssDxChBrgFdbManagerConfigGet(fdbManagerId,
        &capacityInfoGet,&entryAttrInfoGet,&learningInfoGet,&lookupInfoGet,&agingInfoGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerGet ");

    UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(
                                        &capacityInfo, &capacityInfoGet,
                                        sizeof(CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC)),
        "cpssDxChBrgFdbManagerGet : expected 'get' entry to match 'set' entry in manager [%d]",
        fdbManagerId);

    UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(
                                        &entryAttrInfo, &entryAttrInfoGet,
                                        sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC)),
        "cpssDxChBrgFdbManagerGet : expected 'get' entry to match 'set' entry in manager [%d]",
        fdbManagerId);

    UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(
                                        &learningInfo, &learningInfoGet,
                                        sizeof(CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC)),
        "cpssDxChBrgFdbManagerGet : expected 'get' entry to match 'set' entry in manager [%d]",
        fdbManagerId);

    UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(
                                        &lookupInfo, &lookupInfoGet,
                                        sizeof(CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC)),
        "cpssDxChBrgFdbManagerGet : expected 'get' entry to match 'set' entry in manager [%d]",
        fdbManagerId);

    UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(
                                        &agingInfo, &agingInfoGet,
                                        sizeof(CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC)),
        "cpssDxChBrgFdbManagerGet : expected 'get' entry to match 'set' entry in manager [%d]",
        fdbManagerId);

    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChBrgFdbManagerConfigGet(capacityInfoPtr/*checked for NULL*/,
        capacityInfoPtr,entryAttrInfoPtr,learningInfoPtr,lookupInfoPtr,agingInfoPtr);
    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChBrgFdbManagerConfigGet(entryAttrInfoPtr/*checked for NULL*/,
        capacityInfoPtr,entryAttrInfoPtr,learningInfoPtr,lookupInfoPtr,agingInfoPtr);
    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChBrgFdbManagerConfigGet(learningInfoPtr/*checked for NULL*/,
        capacityInfoPtr,entryAttrInfoPtr,learningInfoPtr,lookupInfoPtr,agingInfoPtr);
    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChBrgFdbManagerConfigGet(lookupInfoPtr/*checked for NULL*/,
        capacityInfoPtr,entryAttrInfoPtr,learningInfoPtr,lookupInfoPtr,agingInfoPtr);
    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChBrgFdbManagerConfigGet(agingInfoPtr/*checked for NULL*/,
        capacityInfoPtr,entryAttrInfoPtr,learningInfoPtr,lookupInfoPtr,agingInfoPtr);

    CLEANUP_MANAGER(fdbManagerId);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgFdbManagerConfigGet
(
    IN  GT_U32                                          fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC          *capacityPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC          *learningPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC            *lookupPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC             *agingPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerConfigGet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerConfigGetUT();

    CLEANUP_ALL_MANAGERS;
}

static CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_ENT  global_macVlanLookupMode  = CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E;
static void set_global_macVlanLookupMode(IN CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_ENT macVlanLookupMode)
{
    PRV_UTF_LOG1_MAC("use manager with macVlanLookupMode [%s] \n",
        macVlanLookupMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_ONLY_E      ? "MAC_ONLY" :
        macVlanLookupMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E   ? "MAC_AND_FID" :
        macVlanLookupMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E   ? "MAC_AND_FID_AND_VID1" :
        "unknown");
    global_macVlanLookupMode = macVlanLookupMode;
}
static void restore_global_macVlanLookupMode(void)
{
    set_global_macVlanLookupMode(CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E);
}
static CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_ENT  global_macEntryMuxingMode = CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E;
static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_ENT      global_entryMuxingMode    = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E;
static void set_global_macEntryMuxingMode(IN CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_ENT macEntryMuxingMode)
{
    PRV_UTF_LOG1_MAC("use manager with macEntryMuxingMode [%s] \n",
        macEntryMuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E ? "TAG1_VID" :
        macEntryMuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E   ? "SRC_ID" :
        macEntryMuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_10_SRC_ID_1_DA_ACCESS_LEVEL_E   ? "UDB_10_SRC_ID_1_DA_ACCESS_LEVEL" :
        macEntryMuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_8_SRC_ID_3_DA_ACCESS_LEVEL_E    ? "UDB_8_SRC_ID_3_DA_ACCESS_LEVEL" :
        "unknown");
    global_macEntryMuxingMode = macEntryMuxingMode;
}
static void restore_global_macEntryMuxingMode(void)
{
    set_global_macEntryMuxingMode(CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E);
}
static void set_global_entryMuxingMode(IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_ENT entryMuxingMode)
{
    PRV_UTF_LOG1_MAC("use manager with entryMuxingMode [%s] \n",
        entryMuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E ? "9_SRC_ID_8_UDB_TAG1_VID_DISABLE" :
        entryMuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E? "12_SRC_ID_5_UDB_TAG1_VID_DISABLE" :
        entryMuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E  ? "6_SRC_ID_8_UDB_TAG1_VID_ENABLE" :
        entryMuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E  ? "6_SRC_ID_5_UDB_TAG1_VID_ENABLE" :
        "unknown");
    global_entryMuxingMode = entryMuxingMode;
}
static void restore_global_entryMuxingMode(void)
{
    set_global_entryMuxingMode(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E);
}

static CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_ENT global_ipMcEntryMuxingMode = CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E;
#if 0
static void set_global_ipMcEntryMuxingMode(IN CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_ENT ipMcEntryMuxingMode)
{
    PRV_UTF_LOG1_MAC("use manager with ipMcEntryMuxingMode [%s] \n",
                     ipMcEntryMuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E   ? "SRC_ID" :
                                                                                                          ipMcEntryMuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_4_SRC_ID_3_E   ? "UDB_4_SRC_ID_3" :
                                                                                                                                                                                                       ipMcEntryMuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_E    ? "UDB" :
                                                                                                                                                                                                                                                                                          "unknown");
    global_ipMcEntryMuxingMode = ipMcEntryMuxingMode;
}
static void restore_global_ipMcEntryMuxingMode(void)
{
    set_global_ipMcEntryMuxingMode(CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E);
}
#endif

static CPSS_DXCH_CFG_GLOBAL_EPORT_STC  orig_globalEportInfo;
static GT_BOOL                         need_orig_globalEportInfo = GT_FALSE;

static void restoreGlobalEport(void)
{
    GT_STATUS   st;
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  globalEportInfo,l2EcmpInfo,l2DlbInfo;

    if(GT_FALSE == need_orig_globalEportInfo)
    {
        return;
    }

    st = cpssDxChCfgGlobalEportGet(prvTgfDevNum,&globalEportInfo,&l2EcmpInfo,&l2DlbInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
        "cpssDxChCfgGlobalEportGet ");

    st = cpssDxChCfgGlobalEportSet(prvTgfDevNum,&orig_globalEportInfo,&l2EcmpInfo,&l2DlbInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
        "cpssDxChCfgGlobalEportSet ");

    need_orig_globalEportInfo = GT_FALSE;
    return;
}

/* indication that the test should define global eports range to work with            */
/* the global eport info is on full range                     */
static GT_BOOL  run_with_global_eports_full_range = GT_FALSE;
/* indication that the test should run with limitations on the number of dynamic MAC UC entries*/
/* the global eport info is on full range                     */
static GT_BOOL  run_with_dynamic_uc_limits        = GT_FALSE;

/* create fdb manager - minimal parameters for tests */
static void createFdbManager(
    IN GT_U32                                       fdbManagerId,
    IN GT_U32                                       maxTotalEntries,/* up to 128K (limited by numOfHwIndexes) */
    IN CPSS_DXCH_BRG_FDB_MANAGER_HW_CAPACITY_STC   *hwCapacity,
    IN GT_BOOL                                      macNoSpaceUpdatesEnable
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           capacityInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           learningInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             lookupInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              agingInfo;

    CLEAR_VAR_MAC(capacityInfo );
    CLEAR_VAR_MAC(entryAttrInfo);
    CLEAR_VAR_MAC(learningInfo );
    CLEAR_VAR_MAC(lookupInfo   );
    CLEAR_VAR_MAC(agingInfo    );

    if(hwCapacity != NULL)
    {
        capacityInfo.hwCapacity.numOfHwIndexes = hwCapacity->numOfHwIndexes;
        capacityInfo.hwCapacity.numOfHashes    = hwCapacity->numOfHashes;
    }
    else
    {
        capacityInfo.hwCapacity.numOfHwIndexes = FDB_SIZE_FULL;
        capacityInfo.hwCapacity.numOfHashes    = HASH_AS_PER_DEVICE(16);
    }
    capacityInfo.hwCapacity.mode           = HASH_MODE_AS_PER_DEVICE(0);
    capacityInfo.maxTotalEntries           = maxTotalEntries;
    capacityInfo.maxEntriesPerAgingScan    = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerDeleteScan   = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerTransplantScan = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerLearningScan = NUM_ENTRIES_STEPS_CNS; /*256*/

    entryAttrInfo.macEntryMuxingMode  = global_macEntryMuxingMode;
    entryAttrInfo.ipmcEntryMuxingMode = global_ipMcEntryMuxingMode;
    entryAttrInfo.entryMuxingMode     = global_entryMuxingMode;
    entryAttrInfo.saDropCommand       = CPSS_PACKET_CMD_DROP_SOFT_E;
    entryAttrInfo.daDropCommand       = CPSS_PACKET_CMD_DROP_HARD_E;
    entryAttrInfo.ipNhPacketcommand   = CPSS_PACKET_CMD_ROUTE_E;
    entryAttrInfo.shadowType          = SHADOW_TYPE_PER_DEVICE();
    if(entryAttrInfo.shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)
    {
        CPSS_DXCH_CFG_GLOBAL_EPORT_STC  globalEportInfo,l2EcmpInfo,l2DlbInfo;

        st = cpssDxChCfgGlobalEportGet(prvTgfDevNum,&globalEportInfo,&l2EcmpInfo,&l2DlbInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
            "cpssDxChCfgGlobalEportGet ");

        orig_globalEportInfo = globalEportInfo;
        if(run_with_global_eports_full_range)
        {
            globalEportInfo.enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E;
            globalEportInfo.minValue = 0x00000000;
            globalEportInfo.maxValue = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(prvTgfDevNum);

            need_orig_globalEportInfo = GT_TRUE;

            st = cpssDxChCfgGlobalEportSet(prvTgfDevNum,&globalEportInfo,&l2EcmpInfo,&l2DlbInfo);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                "cpssDxChCfgGlobalEportSet ");
        }

        /* use global eports , as defined by the test/environment prior to calling the 'Create Manager' */
        entryAttrInfo.globalEportInfo     = globalEportInfo;
    }

    if(run_with_dynamic_uc_limits)
    {
        capacityInfo.enableLearningLimits = GT_TRUE;
        capacityInfo.maxDynamicUcMacGlobalLimit = maxTotalEntries / 2;
        capacityInfo.maxDynamicUcMacFidLimit    = capacityInfo.maxDynamicUcMacGlobalLimit / 4;
        capacityInfo.maxDynamicUcMacGlobalEportLimit = capacityInfo.maxDynamicUcMacGlobalLimit / 2;
        capacityInfo.maxDynamicUcMacTrunkLimit  = capacityInfo.maxDynamicUcMacGlobalLimit / 6;
        capacityInfo.maxDynamicUcMacPortLimit   = capacityInfo.maxDynamicUcMacGlobalLimit / 8;
    }


    learningInfo.macNoSpaceUpdatesEnable = macNoSpaceUpdatesEnable;
    learningInfo.macRoutedLearningEnable = GT_FALSE;
    learningInfo.macVlanLookupMode       = global_macVlanLookupMode;

    lookupInfo.crcHashUpperBitsMode     = CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;
    lookupInfo.ipv4PrefixLength         = 32;
    lookupInfo.ipv6PrefixLength         = 128;

    agingInfo.destinationUcRefreshEnable = GT_FALSE;
    agingInfo.destinationMcRefreshEnable = GT_FALSE;
    agingInfo.ipUcRefreshEnable          = GT_FALSE;

    st = cpssDxChBrgFdbManagerCreate(fdbManagerId,
        &capacityInfo,&entryAttrInfo,&learningInfo,&lookupInfo,&agingInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerCreate ");
}
/**
GT_STATUS cpssDxChBrgFdbManagerDelete
(
    IN GT_U32 fdbManagerId
)
*/
static void internal_cpssDxChBrgFdbManagerDeleteUT(void)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_U32      fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChBrgFdbManagerDelete(ii);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChBrgFdbManagerDelete: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_BAD_PARAM' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChBrgFdbManagerDelete(ii);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerDelete: expected to GT_BAD_PARAM on manager [%d]",
            ii);
    }

    /* create single valid manager */
    createFdbManager(fdbManagerId, FDB_SIZE_FULL, NULL, GT_FALSE);
    /* delete it */
    st = cpssDxChBrgFdbManagerDelete(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerDelete: expected to GT_OK on manager [%d]",
        fdbManagerId);

}

/**
GT_STATUS cpssDxChBrgFdbManagerDelete
(
    IN GT_U32 fdbManagerId
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerDelete)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerDeleteUT();

    CLEANUP_ALL_MANAGERS;
}

/**
GT_STATUS cpssDxChBrgFdbManagerDevListAdd
(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[],
    IN GT_U32 numOfDevs
);
*/
static void internal_cpssDxChBrgFdbManagerDevListAddUT(void)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_U32      fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    GT_U8  devListArr[2];
    GT_U32 numOfDevs = 1;

    devListArr[0] = prvTgfDevNum;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChBrgFdbManagerDevListAdd(ii,devListArr,numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChBrgFdbManagerDevListAdd: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_BAD_PARAM' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChBrgFdbManagerDevListAdd(ii,devListArr,numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerDevListAdd: expected to GT_BAD_PARAM on manager [%d]",
            ii);
    }

    /* create single valid manager */
    createFdbManager(fdbManagerId, FDB_SIZE_FULL, NULL, GT_FALSE);
    /* add device to it */
    st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
        fdbManagerId);

    /* re-add device to it */
    st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_ALREADY_EXIST, st,
        "cpssDxChBrgFdbManagerDevListAdd: expected to GT_ALREADY_EXIST on manager [%d]",
        fdbManagerId);

    /* remove the device from it */
    st = cpssDxChBrgFdbManagerDevListRemove(fdbManagerId,devListArr,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerDevListRemove: expected to GT_OK on manager [%d]",
        fdbManagerId);


    /* add 2 of the same device */
    devListArr[1] = devListArr[0];
    numOfDevs     = 2;

    st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
        "cpssDxChBrgFdbManagerDevListAdd: expected to GT_BAD_PARAM on manager [%d]",
        fdbManagerId);

    /* add valid device and non exist device */
    devListArr[1] = 225;/*8 bits value*/
    numOfDevs     = 2;

    st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
        "cpssDxChBrgFdbManagerDevListAdd: expected to GT_BAD_PARAM on manager [%d]",
        fdbManagerId);

    /**************************************************/
    /* check that we can still add valid device to it */
    /**************************************************/
    numOfDevs     = 1;
    /* add device to it */
    st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
        fdbManagerId);
    /* remove the device from it */
    st = cpssDxChBrgFdbManagerDevListRemove(fdbManagerId,devListArr,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerDevListRemove: expected to GT_OK on manager [%d]",
        fdbManagerId);


    /* NULL pointer */
    st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,NULL,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerDevListAdd: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);
    /* 0 devices */
    st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
        "cpssDxChBrgFdbManagerDevListAdd: expected to GT_BAD_PARAM on manager [%d]",
        fdbManagerId);

}
/**
GT_STATUS cpssDxChBrgFdbManagerDevListAdd
(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[],
    IN GT_U32 numOfDevs
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerDevListAdd)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerDevListAddUT();

    CLEANUP_ALL_MANAGERS;
}

/**
GT_STATUS internal_cpssDxChBrgFdbManagerDevListRemoveUT
(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[],
    IN GT_U32 numOfDevs
);
*/
static void internal_cpssDxChBrgFdbManagerDevListRemoveUT(void)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_U32      fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    GT_U8  devListArr[2];
    GT_U32 numOfDevs = 1;

    devListArr[0] = prvTgfDevNum;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChBrgFdbManagerDevListRemove(ii,devListArr,numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChBrgFdbManagerDevListRemove: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_BAD_PARAM' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChBrgFdbManagerDevListRemove(ii,devListArr,numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerDevListRemove: expected to GT_BAD_PARAM on manager [%d]",
            ii);
    }

    /* create single valid manager */
    createFdbManager(fdbManagerId, FDB_SIZE_FULL, NULL, GT_FALSE);

    /* remove a device from it (device not in DB) */
    st = cpssDxChBrgFdbManagerDevListRemove(fdbManagerId,devListArr,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_SUCH, st,
        "cpssDxChBrgFdbManagerDevListRemove: expected to GT_NO_SUCH on manager [%d]",
        fdbManagerId);


    /* add a device */
    st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
        fdbManagerId);

    /* remove valid device and non exist device */
    devListArr[1] = 225;/*8 bits value*/
    numOfDevs     = 2;

    st = cpssDxChBrgFdbManagerDevListRemove(fdbManagerId,devListArr,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_SUCH, st,
        "cpssDxChBrgFdbManagerDevListAdd: expected to GT_NO_SUCH on manager [%d]",
        fdbManagerId);

    /*****************************************************/
    /* check that we can still remove valid device to it */
    /*****************************************************/
    numOfDevs = 1;
    st = cpssDxChBrgFdbManagerDevListRemove(fdbManagerId,devListArr,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerDevListRemove: expected to GT_OK on manager [%d]",
        fdbManagerId);

    /* NULL pointer */
    st = cpssDxChBrgFdbManagerDevListRemove(fdbManagerId,NULL,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerDevListRemove: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);
    /* 0 devices */
    st = cpssDxChBrgFdbManagerDevListRemove(fdbManagerId,devListArr,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
        "cpssDxChBrgFdbManagerDevListRemove: expected to GT_BAD_PARAM on manager [%d]",
        fdbManagerId);

}
/**
GT_STATUS cpssDxChBrgFdbManagerDevListRemove
(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[],
    IN GT_U32 numOfDevs
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerDevListRemove)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerDevListRemoveUT();

    CLEANUP_ALL_MANAGERS;
}


/**
GT_STATUS internal_cpssDxChBrgFdbManagerDevListGetUT
(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[],
    IN GT_U32 numOfDevs
);
*/
static void internal_cpssDxChBrgFdbManagerDevListGetUT(void)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_U32      fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    GT_U8  devListArr[2], devListArrGet[2] = {0xAB, 0xCD};
    GT_U32 numOfDevs;

    devListArr[0] = prvTgfDevNum;
    /* init to avoid trash values */
    devListArr[1] = 0;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChBrgFdbManagerDevListGet(ii, &numOfDevs, devListArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChBrgFdbManagerDevListGet: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_BAD_PARAM' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChBrgFdbManagerDevListGet(ii, &numOfDevs, devListArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerDevListGet: expected to GT_BAD_PARAM on manager [%d]",
            ii);
    }

    /* create single valid manager */
    createFdbManager(fdbManagerId,FDB_SIZE_FULL,NULL, GT_FALSE);

    numOfDevs = 1;
    /* add one device */
    st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId, devListArr, numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
        fdbManagerId);

    /*****************************************************/
    /* check that we can retrieve one valid device       */
    /*****************************************************/
    numOfDevs = 2;
    st = cpssDxChBrgFdbManagerDevListGet(fdbManagerId, &numOfDevs, devListArrGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerDevListGet: expected to GT_NO_SUCH on manager [%d]",
        fdbManagerId);

    if (st == GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(1, numOfDevs,
            "cpssDxChBrgFdbManagerDevListGet: expected to one added device [%d]",
            fdbManagerId);

        UTF_VERIFY_EQUAL1_STRING_MAC(devListArr[0], devListArrGet[0],
            "cpssDxChBrgFdbManagerDevListGet: expected to device ID[%d]",
            devListArr[0]);

        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(devListArr[1], devListArrGet[1],
            "cpssDxChBrgFdbManagerDevListGet: not expected to more than one device[%d]",
            fdbManagerId);
    }
    /* NULL pointer */
    st = cpssDxChBrgFdbManagerDevListGet(fdbManagerId, &numOfDevs, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerDevListGet: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);

    /* 0 devices */
    st = cpssDxChBrgFdbManagerDevListGet(fdbManagerId, NULL, devListArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerDevListGet: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);
}

/**
GT_STATUS cpssDxChBrgFdbManagerDevListGet
(
    IN GT_U32           fdbManagerId,
    INOUT GT_U32        *numOfDevicesPtr,
    OUT GT_U8           deviceListArray[]
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerDevListGet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /* skip mixed multidevice boards */
    if(prvUtfIsMultiPpDeviceSystem())
    {
        SKIP_TEST_MAC;
    }

    internal_cpssDxChBrgFdbManagerDevListGetUT();

    CLEANUP_ALL_MANAGERS;
}

/* indication that the test should run without device bound to the manager            */
/* the default is run with device from the start (all entries inserted to the device) */
static GT_BOOL  run_device_less = GT_FALSE;
/* indication that the test should run add device in runtime to the manager           */
/* the default is run with device from the start (all entries inserted to the device) */
static GT_BOOL  add_device_in_runtime = GT_FALSE;
/* indication that the test should check for entries to exist in the HW               */
/* the default is not to check it , because it slow done the test                     */
static GT_BOOL  check_hw_hold_entry = GT_FALSE;
/* if ZERO - we not force the 'add entry' for specific back , and it is randomized */
/* if non-ZERO - this is the bankId to try to use to the 'add entry'               */
static GT_U32   force_single_bank_to_add = 0;

static GT_BOOL  check_hw_hold_entry_all_tiles = GT_TRUE;

/* allow to debug limited number of entries inserted to the manager */
static GT_U32   debug_limited_entries = 0;
GT_STATUS  debug_limited_entries_set(IN GT_U32  numEntries)
{
    debug_limited_entries = numEntries ?
            /* rounded UP to nearest 256 ... otherwise fail to 'create' manager */
            (((NUM_ENTRIES_STEPS_CNS-1)+numEntries)/NUM_ENTRIES_STEPS_CNS)*NUM_ENTRIES_STEPS_CNS :
            /* set to ZERO --> disable the 'debug' */
            0;
    return GT_OK;
}
extern GT_STATUS cpssDxChFdbDump(IN GT_U8 devNum);
static void fdbManagerHwEntryCheck(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr
);
static void fdbManagerHwEntryCheckAllTiles(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC  *entryPtr,
    IN GT_U32                               iteration
);


/* create fdb manager - minimal parameters for tests */
static void createFdbManagerWithDevice(
    IN GT_U32                                       fdbManagerId,
    IN GT_U32                                       maxTotalEntries,/* up to 128K (limited by numOfHwIndexes) */
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_HW_CAPACITY_STC   *hwCapacity,
    IN GT_BOOL                                      macNoSpaceUpdatesEnable
)
{
    GT_STATUS   st;
    GT_U8  devListArr[1];
    GT_U32 numOfDevs = 1;

    devListArr[0] = devNum;

    /* create single valid manager */
    createFdbManager(fdbManagerId, maxTotalEntries, hwCapacity, macNoSpaceUpdatesEnable);

    if(run_device_less == GT_TRUE)
    {
        /* we no add the device to the manager */
        return;
    }

    /* add device to it */
    st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
        fdbManagerId);
}

#define UT_FDB_ENTRY_TYPE_VALID_NUM ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?(5):(3))
static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT fdbEntryType_valid_arr[6] =
{
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E
    , CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E
    , CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E
    , CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E
    , CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E
    , GT_NA
};

/* add fdb entry to the manager : type : mac addr */
static void fdbManagerEntryBuild_macAddr(
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr
)
{
    CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT  shadowType;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC *fdbEntryMacAddrFormatPtr;

    cpssOsMemSet(entryPtr, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));

    fdbEntryMacAddrFormatPtr = &entryPtr->format.fdbEntryMacAddrFormat;

    entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;

    fdbEntryMacAddrFormatPtr->macAddr.arEther[0] = 0x00;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[1] = 0x11;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[2] = 0x22;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[3] = 0x33;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[4] = 0x44;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[5] = 0x55;
    fdbEntryMacAddrFormatPtr->fid                = 1;
    fdbEntryMacAddrFormatPtr->vid1               = (global_macEntryMuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E) ? 0/* muxing mode must be 0 */ : 1;
    fdbEntryMacAddrFormatPtr->daSecurityLevel = 0;      /* muxing mode must be 0 */
    shadowType = SHADOW_TYPE_PER_DEVICE();
    if(shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)
    {
        switch(global_entryMuxingMode)
        {
            default:
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E:
                fdbEntryMacAddrFormatPtr->saSecurityLevel = 1;
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E:
                fdbEntryMacAddrFormatPtr->saSecurityLevel = 0;
                break;
        }
        fdbEntryMacAddrFormatPtr->sourceID = (global_macEntryMuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E) ? 0 /* muxing mode must be 0 */: 1;
    }
    else if(shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E)
    {
        fdbEntryMacAddrFormatPtr->daSecurityLevel = 0;
        fdbEntryMacAddrFormatPtr->daSecurityLevel = 0;
        fdbEntryMacAddrFormatPtr->sourceID        = 1;
    }
    else
    {
        fdbEntryMacAddrFormatPtr->saSecurityLevel = 1;
    }
    fdbEntryMacAddrFormatPtr->userDefined     = 0;      /* muxing mode must be 0 */
    fdbEntryMacAddrFormatPtr->daCommand       = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    fdbEntryMacAddrFormatPtr->saCommand       = CPSS_PACKET_CMD_DROP_SOFT_E  ;
    fdbEntryMacAddrFormatPtr->isStatic        = GT_FALSE;
    fdbEntryMacAddrFormatPtr->age             = GT_TRUE;
    fdbEntryMacAddrFormatPtr->daRoute         = GT_FALSE;
    fdbEntryMacAddrFormatPtr->appSpecificCpuCode = GT_TRUE;

    return;
}

/* add fdb entry to the manager : type : IPvx UC common info */
static void fdbManagerEntryBuild_DstInterfaceInfo(
    IN  GT_U32                                                      rand32Bits,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC          *dstInterfacePtr
)
{
    cpssOsMemSet(dstInterfacePtr, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC));

    dstInterfacePtr->type         = rand32Bits % 3;

    switch(dstInterfacePtr->type)
    {
        case CPSS_INTERFACE_PORT_E:
            dstInterfacePtr->interfaceInfo.devPort.hwDevNum   = (rand32Bits >> 1) & DEV_ID_MASK_AS_PER_DEVICE();
            dstInterfacePtr->interfaceInfo.devPort.portNum    = (rand32Bits >> 2) & PORT_MASK_AS_PER_DEVICE();
            break;
        case CPSS_INTERFACE_TRUNK_E:
            dstInterfacePtr->interfaceInfo.trunkId = /* (rand32Bits >> 3) */ 12 & TRUNK_MASK_AS_PER_DEVICE();
            break;
        case CPSS_INTERFACE_VIDX_E:
            dstInterfacePtr->interfaceInfo.vidx    = (rand32Bits >> 4) & VIDX_MASK_AS_PER_DEVICE();
            /* exclude special value 0xFFF from interface because such entry
               is recognized as VID interface type and HA related test may fail. */
            if(dstInterfacePtr->interfaceInfo.vidx == 0xFFF)
            {
                dstInterfacePtr->interfaceInfo.vidx -= 1;
            }

            break;
        default:
            break;
    }
}

/* add fdb entry to the manager : type : IP MC addr */
static void fdbManagerEntryBuild_ipMcAddr(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN GT_BOOL                                          isIpv4Mc,
    IN GT_U32                                           iterationNum
)
{
    static GT_U32 ipAddr    = 0;
    static GT_U32 fId       = 0;

    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC *fdbEntryIpMcAddrFormatPtr;
    GT_U32 rand32Bits = cpssOsRand() | iterationNum << 16;

    cpssOsMemSet(entryPtr, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));

    if (isIpv4Mc)
    {
        fdbEntryIpMcAddrFormatPtr = &entryPtr->format.fdbEntryIpv4McFormat;
        entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E;
    }
    else
    {
        fdbEntryIpMcAddrFormatPtr = &entryPtr->format.fdbEntryIpv6McFormat;
        entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E;
    }

    /* Build random part */
    ipAddr++;
    fdbEntryIpMcAddrFormatPtr->sipAddr[0] = (ipAddr >> 24) & 0xff;
    fdbEntryIpMcAddrFormatPtr->sipAddr[1] = (ipAddr >> 16) & 0xff;
    fdbEntryIpMcAddrFormatPtr->sipAddr[2] = (ipAddr >>  8) & 0xff;
    fdbEntryIpMcAddrFormatPtr->sipAddr[3] = (ipAddr >>  0) & 0xff;

    ipAddr++;
    fdbEntryIpMcAddrFormatPtr->dipAddr[0] = (ipAddr >> 24) & 0xff;
    fdbEntryIpMcAddrFormatPtr->dipAddr[1] = (ipAddr >> 16) & 0xff;
    fdbEntryIpMcAddrFormatPtr->dipAddr[2] = (ipAddr >>  8) & 0xff;
    fdbEntryIpMcAddrFormatPtr->dipAddr[3] = (ipAddr >>  0) & 0xff;

    fId++;
    fdbEntryIpMcAddrFormatPtr->fid        = fId & FID_MASK_AS_PER_DEVICE();

    fdbEntryIpMcAddrFormatPtr->dstInterface.type                = CPSS_INTERFACE_VIDX_E;
    fdbEntryIpMcAddrFormatPtr->dstInterface.interfaceInfo.vidx  = (rand32Bits >> 4) & VIDX_MASK_AS_PER_DEVICE();
    fdbEntryIpMcAddrFormatPtr->daSecurityLevel = 0;      /* muxing mode must be 0 */
    fdbEntryIpMcAddrFormatPtr->userDefined     = 0;      /* muxing mode must be 0 */
    fdbEntryIpMcAddrFormatPtr->sourceID        = (global_ipMcEntryMuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E) ? 0 /* muxing mode must be 0 */: 1;
    fdbEntryIpMcAddrFormatPtr->daCommand       = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    fdbEntryIpMcAddrFormatPtr->isStatic        = GT_FALSE;
    fdbEntryIpMcAddrFormatPtr->age             = GT_TRUE;
    fdbEntryIpMcAddrFormatPtr->daRoute         = GT_FALSE;
    fdbEntryIpMcAddrFormatPtr->appSpecificCpuCode = GT_TRUE;

    return;
}

/* add fdb entry to the manager : type : IPvx UC common info */
static void fdbManagerEntryBuild_ipvxUcCommonInfo(
    IN  CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT               ucRouteType,
    IN  GT_U32                                                      rand32Bits,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT * ucRouteInfoPtr
)
{
    switch (ucRouteType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E:
            ucRouteInfoPtr->fullFdbInfo.pointer.tunnelStartPointer  = (rand32Bits >> 3) & TUNNEL_POINTER_MASK_AS_PER_DEVICE();
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E:
            ucRouteInfoPtr->fullFdbInfo.pointer.natPointer  = (rand32Bits >> 4) & 0xffff;/* 16 bit */
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E:
            ucRouteInfoPtr->fullFdbInfo.pointer.arpPointer  = (rand32Bits >> 5) & ARP_POINTER_MASK_AS_PER_DEVICE();
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E:
            ucRouteInfoPtr->nextHopPointerToRouter = (rand32Bits >> 6) & 0x7fff;/* 15 bit */
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E:
            ucRouteInfoPtr->ecmpPointerToRouter = (rand32Bits >> 7) & 0x7fff; /* 15 bit */
            break;
        default:
            break;
    }

    if((ucRouteType == CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E) ||
            (ucRouteType == CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E) ||
            (ucRouteType == CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E))
    {
        fdbManagerEntryBuild_DstInterfaceInfo(rand32Bits, &ucRouteInfoPtr->fullFdbInfo.dstInterface);
    }
}

/* add fdb entry to the manager : type : IPv4 UC addr */
static void fdbManagerEntryBuild_ipv4UcAddr(
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC     *entryPtr,
    IN GT_U32                                   iterationNum
)
{
    static GT_U32 ipAddr    = 0;
    static GT_U32 vrfId     = 0;

    CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC  *fdbEntryFormatPtr = &entryPtr->format.fdbEntryIpv4UcFormat;
    GT_U32 rand32Bits = cpssOsRand() | iterationNum << 16;

    cpssOsMemSet(entryPtr, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));

    entryPtr->fdbEntryType      = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E;

    fdbEntryFormatPtr->age      = GT_TRUE;

    /* Build random part */
    ipAddr++;
    fdbEntryFormatPtr->ipv4Addr.arIP[0] = (ipAddr >> 24) & 0xff;
    fdbEntryFormatPtr->ipv4Addr.arIP[1] = (ipAddr >> 16) & 0xff;
    fdbEntryFormatPtr->ipv4Addr.arIP[2] = (ipAddr >>  8) & 0xff;
    fdbEntryFormatPtr->ipv4Addr.arIP[3] = (ipAddr >>  0) & 0xff;

    vrfId++;
    fdbEntryFormatPtr->vrfId                        = vrfId & 0xFFF;

    fdbEntryFormatPtr->ucRouteType                  = (rand32Bits >> 2) % 5;
    if(IS_SIP5_OR_HYBRID)
    {
        /* In case of SIP5 only TUNNEL & ARP is supported */
        fdbEntryFormatPtr->ucRouteType = (fdbEntryFormatPtr->ucRouteType)?CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E:CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
    }

    fdbManagerEntryBuild_ipvxUcCommonInfo(fdbEntryFormatPtr->ucRouteType, rand32Bits, &fdbEntryFormatPtr->ucRouteInfo);
}

/* add fdb entry to the manager : type : IPv6 UC addr */
static void fdbManagerEntryBuild_ipv6UcAddr(
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC         *entryPtr,
    IN GT_U32                                       iterationNum
)
{
    static GT_U32 ipAddr[4]     = {0, 0, 0, 0};
    static GT_U32 vrfId         = 0;
    GT_U32 maxVal               = 0xFFFFFFFF;

    CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC  *fdbEntryFormatPtr = &entryPtr->format.fdbEntryIpv6UcFormat;
    GT_U32 rand32Bits = cpssOsRand() | iterationNum << 16;

    cpssOsMemSet(entryPtr, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));

    entryPtr->fdbEntryType      = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E;

    fdbEntryFormatPtr->age      = GT_TRUE;

    /* Build random part */
    if (ipAddr[3] < maxVal)
    {
        ipAddr[3]++;
    }
    else if (ipAddr[2] < maxVal)
    {
        ipAddr[3] = 0;
        ipAddr[2]++;
    }
    else if (ipAddr[1] < maxVal)
    {
        ipAddr[3] = ipAddr[2] = 0;
        ipAddr[1]++;
    }
    else
    {
        ipAddr[3] = ipAddr[2] = ipAddr[1] = 0;
        ipAddr[0]++;
    }

    fdbEntryFormatPtr->ipv6Addr.arIP[0]  = (ipAddr[0] >> 24) & 0xff;
    fdbEntryFormatPtr->ipv6Addr.arIP[1]  = (ipAddr[0] >> 16) & 0xff;
    fdbEntryFormatPtr->ipv6Addr.arIP[2]  = (ipAddr[0] >>  8) & 0xff;
    fdbEntryFormatPtr->ipv6Addr.arIP[3]  = (ipAddr[0] >>  0) & 0xff;

    fdbEntryFormatPtr->ipv6Addr.arIP[4]  = (ipAddr[1] >> 24) & 0xff;
    fdbEntryFormatPtr->ipv6Addr.arIP[5]  = (ipAddr[1] >> 16) & 0xff;
    fdbEntryFormatPtr->ipv6Addr.arIP[6]  = (ipAddr[1] >>  8) & 0xff;
    fdbEntryFormatPtr->ipv6Addr.arIP[7]  = (ipAddr[1] >>  0) & 0xff;

    fdbEntryFormatPtr->ipv6Addr.arIP[8]  = (ipAddr[2] >> 24) & 0xff;
    fdbEntryFormatPtr->ipv6Addr.arIP[9]  = (ipAddr[2] >> 16) & 0xff;
    fdbEntryFormatPtr->ipv6Addr.arIP[10] = (ipAddr[2] >>  8) & 0xff;
    fdbEntryFormatPtr->ipv6Addr.arIP[11] = (ipAddr[2] >>  0) & 0xff;

    fdbEntryFormatPtr->ipv6Addr.arIP[12] = (ipAddr[3] >> 24) & 0xff;
    fdbEntryFormatPtr->ipv6Addr.arIP[13] = (ipAddr[3] >> 16) & 0xff;
    fdbEntryFormatPtr->ipv6Addr.arIP[14] = (ipAddr[3] >>  8) & 0xff;
    fdbEntryFormatPtr->ipv6Addr.arIP[15] = (ipAddr[3] >>  0) & 0xff;

    vrfId++;
    fdbEntryFormatPtr->vrfId                        = vrfId & 0xFFF;

    fdbEntryFormatPtr->ucRouteType                  = (rand32Bits >> 2) % 5;
    if((SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E) ||
        (SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E))
    {
        /* In case of SIP5 only TUNNEL & ARP is supported */
        fdbEntryFormatPtr->ucRouteType = (fdbEntryFormatPtr->ucRouteType)?CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E:CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
    }

    fdbEntryFormatPtr->ucRouteInfo.fullFdbInfo.ipv6ExtInfo.scopeCheckingEnable = GT_TRUE;
    fdbEntryFormatPtr->ucRouteInfo.fullFdbInfo.ipv6ExtInfo.siteId = CPSS_IP_SITE_ID_INTERNAL_E;

    fdbManagerEntryBuild_ipvxUcCommonInfo(fdbEntryFormatPtr->ucRouteType, rand32Bits, &fdbEntryFormatPtr->ucRouteInfo);
}

/* add fdb entry to the manager : type : mac addr */
static void fdbManagerEntryAdd_macAddr_extParam(
    IN  GT_U32                                      fdbManagerId,
    IN  GT_U32                                      iterationNum,
    UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC *outputResultsPtr,
    IN  GT_BOOL                                     isStatic,
    IN  GT_BOOL                                     isMulticast
)
{
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     entryGet;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    static GT_BOOL isFirstTime = GT_TRUE;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC *fdbEntryMacAddrFormatPtr;
    GT_U32  rand32Bits; /* random value in 32 bits (random 30 bits + 2 bits from iterationNum) */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC             *entryPtr;
    GT_BOOL                                         *entryAddedPtr;
    GT_STATUS                                       st;

    entryPtr        = outputResultsPtr->entryPtr;
    entryAddedPtr   = outputResultsPtr->entryAddedPtr;

    fdbEntryMacAddrFormatPtr = &entryPtr->format.fdbEntryMacAddrFormat;

    if(isFirstTime)
    {
        cpssOsMemSet(&fdbEntryAddInfo,0,sizeof(fdbEntryAddInfo));
        isFirstTime = GT_FALSE;


        fdbEntryAddInfo.rehashEnable    = globalRehashEnable;
    }

    fdbManagerEntryBuild_macAddr(entryPtr);

    rand32Bits = (cpssOsRand() & 0x7FFF) << 15 |
                 (cpssOsRand() & 0x7FFF)       |
                 iterationNum << 30;

    if(isMulticast == GT_FALSE)
    {
        fdbManagerEntryBuild_DstInterfaceInfo(rand32Bits, &fdbEntryMacAddrFormatPtr->dstInterface);
    }

    if(force_single_bank_to_add)
    {
        fdbEntryAddInfo.tempEntryExist  = GT_TRUE;
        fdbEntryAddInfo.tempEntryOffset = force_single_bank_to_add;
    }
    else
    {
        fdbEntryAddInfo.tempEntryExist  = (rand32Bits & BIT_7) ? GT_TRUE : GT_FALSE;
        fdbEntryAddInfo.tempEntryOffset = BANK_AS_PER_DEVICE((rand32Bits >> 15) & 0xF);/*bankId*/
    }

    fdbEntryMacAddrFormatPtr->macAddr.arEther[0] = 0x00;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[1] = 0x00;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[2] = (GT_U8)(rand32Bits >> 24);
    fdbEntryMacAddrFormatPtr->macAddr.arEther[3] = (GT_U8)(rand32Bits >> 16);
    fdbEntryMacAddrFormatPtr->macAddr.arEther[4] = (GT_U8)(rand32Bits >>  8);
    fdbEntryMacAddrFormatPtr->macAddr.arEther[5] = (GT_U8)(rand32Bits >>  0);
    fdbEntryMacAddrFormatPtr->fid                = cpssOsRand() & FID_MASK_AS_PER_DEVICE();
    fdbEntryMacAddrFormatPtr->isStatic           = isStatic;
    if(isMulticast == GT_TRUE)
    {
        fdbEntryMacAddrFormatPtr->macAddr.arEther[0]             |= 0x1;
        fdbEntryMacAddrFormatPtr->dstInterface.type               = CPSS_INTERFACE_VIDX_E;
        fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.vidx = (rand32Bits >> 4) & VIDX_MASK_AS_PER_DEVICE();
    }

    if(global_macVlanLookupMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E)
    {
        /* test configures FID = VID in VLAN table, and enables VID1 mode only for FIDs in VLAN range.
           need to limit FID to VLAN range.  */
        fdbEntryMacAddrFormatPtr->fid %= PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(prvTgfDevNum);
    }

    /* muxed fields */
    fdbEntryMacAddrFormatPtr->sourceID = 0;
    fdbEntryMacAddrFormatPtr->vid1     = 0;

    if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
    {
        switch(global_macEntryMuxingMode)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E:
                fdbEntryMacAddrFormatPtr->sourceID           = cpssOsRand() & 0xFFF; /* 12 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E:
                fdbEntryMacAddrFormatPtr->vid1               = cpssOsRand() & 0xFFF; /* 12 bits */
                break;
            default:
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,GT_NOT_IMPLEMENTED,
                        "fdbManagerEntryAdd_macAddr: not implemented case in manager [%d]",
                        fdbManagerId);
        }
    }
    else if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)
    {
        switch(global_entryMuxingMode)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E:
                fdbEntryMacAddrFormatPtr->sourceID           = cpssOsRand() & 0x1FF; /* 9 bits */
                fdbEntryMacAddrFormatPtr->vid1               = 0;                    /* 0 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E:
                fdbEntryMacAddrFormatPtr->sourceID           = cpssOsRand() & 0xFFF; /* 12 bits */
                fdbEntryMacAddrFormatPtr->vid1               = 0;                    /* 0 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E:
                fdbEntryMacAddrFormatPtr->sourceID           = cpssOsRand() & 0x3F;  /* 6 bits */
                fdbEntryMacAddrFormatPtr->vid1               = cpssOsRand() & 0xFFF; /* 12 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E:
                fdbEntryMacAddrFormatPtr->sourceID           = cpssOsRand() & 0x3F;  /* 6 bits */
                fdbEntryMacAddrFormatPtr->vid1               = cpssOsRand() & 0xFFF; /* 12 bits */
                break;
            default:
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,GT_NOT_IMPLEMENTED,
                        "fdbManagerEntryAdd_macAddr: not implemented case in manager [%d]",
                        fdbManagerId);
        }
    }
    else if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E)
    {
        fdbEntryMacAddrFormatPtr->sourceID               = cpssOsRand() & 0x1F; /* 5 bits */
        fdbEntryMacAddrFormatPtr->daQoSParameterSetIndex = 0;
        fdbEntryMacAddrFormatPtr->saQoSParameterSetIndex = 0;
        fdbEntryMacAddrFormatPtr->mirrorToAnalyzerPort   = GT_FALSE;
    }
    else
    {
        fdbEntryMacAddrFormatPtr->sourceID               = cpssOsRand() & 0x1F; /* 5 bits */
        fdbEntryMacAddrFormatPtr->daQoSParameterSetIndex = cpssOsRand() & 0x7;  /* 3 bits */
        fdbEntryMacAddrFormatPtr->saQoSParameterSetIndex = cpssOsRand() & 0x7;  /* 3 bits */
        fdbEntryMacAddrFormatPtr->mirrorToAnalyzerPort   = GT_FALSE;
    }

    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, entryPtr, &fdbEntryAddInfo, st);
    if(st == GT_OK)
    {
        *entryAddedPtr = GT_TRUE;
    }
    else if ((st != GT_FULL                              && run_with_dynamic_uc_limits == GT_FALSE) ||
             (run_with_dynamic_uc_limits == GT_TRUE      &&
              st != GT_FULL                              &&
              st != GT_LEARN_LIMIT_PORT_ERROR            &&
              st != GT_LEARN_LIMIT_TRUNK_ERROR           &&
              st != GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR    &&
              st != GT_LEARN_LIMIT_FID_ERROR             &&
              st != GT_LEARN_LIMIT_GLOBAL_ERROR))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "fdbManagerEntryAdd_macAddr: unexpected error in manager [%d]",
            fdbManagerId);
    }

    if((*entryAddedPtr) == GT_TRUE)
    {
        /* the entry was added ... check if the entry retrieved as was set */

        /* start -- the parts of the key */
        cpssOsMemCpy(&entryGet.format.fdbEntryMacAddrFormat.macAddr,
                     &fdbEntryMacAddrFormatPtr->macAddr,
                     sizeof(fdbEntryMacAddrFormatPtr->macAddr));
        entryGet.format.fdbEntryMacAddrFormat.fid  = fdbEntryMacAddrFormatPtr->fid;
        entryGet.format.fdbEntryMacAddrFormat.vid1 = fdbEntryMacAddrFormatPtr->vid1;
        /* end -- the parts of the key */
        entryGet.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;

        st = cpssDxChBrgFdbManagerEntryGet(fdbManagerId,&entryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryGet: expected GT_OK in manager [%d]",
            fdbManagerId);

        /***************************************************************/
        /* compare the full entry : entry type , fdbEntryMacAddrFormat */
        /***************************************************************/
        UTF_VERIFY_EQUAL1_STRING_MAC(entryPtr->fdbEntryType,entryGet.fdbEntryType,
            "cpssDxChBrgFdbManagerEntryGet: expected fdbEntryType in 'get' entry to match 'set' in manager [%d]",
            fdbManagerId);

        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(
                                            &entryGet.format.fdbEntryMacAddrFormat,
                                            &entryPtr->format.fdbEntryMacAddrFormat,
                                            sizeof(entryGet.format.fdbEntryMacAddrFormat)),
            "cpssDxChBrgFdbManagerEntryGet: expected 'get' entry to match 'set' entry in manager [%d]",
            fdbManagerId);
    }

}

static void fdbManagerEntryAdd_macAddr(
    IN  GT_U32                                      fdbManagerId,
    IN  GT_U32                                      iterationNum,
    UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC *outputResultsPtr
)
{
    fdbManagerEntryAdd_macAddr_extParam(fdbManagerId,iterationNum,outputResultsPtr,GT_FALSE,GT_FALSE);
}



/* add fdb entry to the manager : type : IPMC ipv4/ipv6 */
static void fdbManagerEntryAdd_ipvMc(
    IN GT_U32                                               fdbManagerId,
    IN GT_BOOL                                              isIpv4,
    IN GT_U32                                               iterationNum,
    OUT UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC    *outputResultsPtr
)
{
    GT_STATUS   st;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entryGet;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC *fdbEntryIpMcAddrFormatPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC *fdbEntryIpMcAddrFormatGetPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     *entryPtr;
    GT_BOOL                                                 *entryAddedPtr;

    /* FDB Lookup key mode == CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_VID1_E is not supported
       for IPv4/6 MC entry types in SIP6 devices */
    if (global_macVlanLookupMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E)
    {
        return;
    }


    entryPtr        = outputResultsPtr->entryPtr;
    entryAddedPtr   = outputResultsPtr->entryAddedPtr;


    cpssOsMemSet(&fdbEntryAddInfo,0,sizeof(fdbEntryAddInfo));

    fdbManagerEntryBuild_ipMcAddr(entryPtr, isIpv4, iterationNum);

    entryGet.fdbEntryType = entryPtr->fdbEntryType;
    if (isIpv4)
    {
        fdbEntryIpMcAddrFormatPtr = &entryPtr->format.fdbEntryIpv4McFormat;
        fdbEntryIpMcAddrFormatGetPtr = &entryGet.format.fdbEntryIpv4McFormat;
    }
    else
    {
        fdbEntryIpMcAddrFormatPtr = &entryPtr->format.fdbEntryIpv6McFormat;
        fdbEntryIpMcAddrFormatGetPtr = &entryGet.format.fdbEntryIpv6McFormat;
    }

    fdbEntryAddInfo.rehashEnable    = globalRehashEnable;
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, entryPtr, &fdbEntryAddInfo, st);
    if(st == GT_OK)
    {
        *entryAddedPtr = GT_TRUE;
    }
    else if (st != GT_FULL)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "fdbManagerEntryAdd_ipvMc: unexpected error in manager [%d]",
            fdbManagerId);
    }

    if((*entryAddedPtr) == GT_TRUE)
    {
        /* the entry was added ... check if the entry retrieved as was set */

        /* start -- the parts of the key */
        cpssOsMemCpy(&fdbEntryIpMcAddrFormatGetPtr->sipAddr,
                     &fdbEntryIpMcAddrFormatPtr->sipAddr,
                     sizeof(fdbEntryIpMcAddrFormatPtr->sipAddr));

        cpssOsMemCpy(&fdbEntryIpMcAddrFormatGetPtr->dipAddr,
                     &fdbEntryIpMcAddrFormatPtr->dipAddr,
                     sizeof(fdbEntryIpMcAddrFormatPtr->dipAddr));

        fdbEntryIpMcAddrFormatGetPtr->fid = fdbEntryIpMcAddrFormatPtr->fid;
        /* end -- the parts of the key */

        st = cpssDxChBrgFdbManagerEntryGet(fdbManagerId,&entryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryGet: expected GT_OK in manager [%d]",
            fdbManagerId);

        /***************************************************************/
        /* compare the full entry : entry type, fdbEntryIpvMcFormat    */
        /***************************************************************/
        UTF_VERIFY_EQUAL1_STRING_MAC(entryPtr->fdbEntryType,entryGet.fdbEntryType,
            "cpssDxChBrgFdbManagerEntryGet: expected fdbEntryType in 'get' entry to match 'set' in manager [%d]",
            fdbManagerId);

        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(
                                            &entryGet.format.fdbEntryIpv4McFormat,
                                            &entryPtr->format.fdbEntryIpv4McFormat,
                                            sizeof(entryGet.format.fdbEntryIpv4McFormat)),
            "cpssDxChBrgFdbManagerEntryGet: expected 'get' entry to match 'set' entry in manager [%d]",
            fdbManagerId);
    }
}


/* add fdb entry to the manager : type : IPMC ipv4 */
static void fdbManagerEntryAdd_ipv4Mc(
    IN  GT_U32                                              fdbManagerId,
    IN  GT_U32                                              iterationNum,
    OUT UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC    *outputResultsPtr
)
{
    fdbManagerEntryAdd_ipvMc(fdbManagerId, GT_TRUE, iterationNum, outputResultsPtr);
}

/* add fdb entry to the manager : type : IPMC ipv6 */
static void fdbManagerEntryAdd_ipv6Mc(
    IN GT_U32                                               fdbManagerId,
    IN GT_U32                                               iterationNum,
    OUT UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC    *outputResultsPtr
)
{
    fdbManagerEntryAdd_ipvMc(fdbManagerId, GT_FALSE, iterationNum, outputResultsPtr);
}

/* add fdb entry to the manager : type : IPMC ipv4 */
static void fdbManagerEntryAdd_ipv4Uc(
    IN GT_U32                                               fdbManagerId,
    IN GT_U32                                               iterationNum,
    OUT UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC    *outputResultsPtr
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entryGet;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC      *fdbEntryFormatPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC      *fdbEntryFormatGetPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     *entryPtr;
    GT_BOOL                                                 *entryAddedPtr;

    entryPtr        = outputResultsPtr->entryPtr;
    entryAddedPtr   = outputResultsPtr->entryAddedPtr;

    fdbEntryFormatPtr = &entryPtr->format.fdbEntryIpv4UcFormat;
    fdbEntryFormatGetPtr = &entryGet.format.fdbEntryIpv4UcFormat;

    cpssOsMemSet(&entryGet,0,sizeof(entryGet));

    fdbManagerEntryBuild_ipv4UcAddr(entryPtr, iterationNum);

    fdbEntryAddInfo.rehashEnable    = globalRehashEnable;
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, entryPtr, &fdbEntryAddInfo, st);
    if(st == GT_OK)
    {
        *entryAddedPtr = GT_TRUE;
    }
    else if (st != GT_FULL)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "fdbManagerEntryAdd_ipv4Uc: unexpected error in manager [%d]",
            fdbManagerId);
    }

    if((*entryAddedPtr) == GT_TRUE)
    {
        /* the entry was added ... check if the entry retrieved as was set */

        /* start -- the parts of the key */
        cpssOsMemCpy(&fdbEntryFormatGetPtr->ipv4Addr,
                     &fdbEntryFormatPtr->ipv4Addr,
                     sizeof(fdbEntryFormatPtr->ipv4Addr));

        fdbEntryFormatGetPtr->vrfId = fdbEntryFormatPtr->vrfId;
        /* end -- the parts of the key */
        entryGet.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E;

        st = cpssDxChBrgFdbManagerEntryGet(fdbManagerId,&entryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryGet: expected GT_OK in manager [%d]",
            fdbManagerId);

        /***************************************************************/
        /* compare the full entry : entry type , fdbEntryIpv4UcFormat */
        /***************************************************************/
        UTF_VERIFY_EQUAL1_STRING_MAC(entryPtr->fdbEntryType, entryGet.fdbEntryType,
            "cpssDxChBrgFdbManagerEntryGet: expected fdbEntryType in 'get' entry to match 'set' in manager [%d]",
            fdbManagerId);

        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(
                                            fdbEntryFormatGetPtr, fdbEntryFormatPtr,
                                            sizeof(CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC)),
            "cpssDxChBrgFdbManagerEntryGet: expected 'get' entry to match 'set' entry in manager [%d]",
            fdbManagerId);
    }
}

/* add fdb entry to the manager : type : IPMC ipv6 */
static void fdbManagerEntryAdd_ipv6Uc(
    IN GT_U32                                               fdbManagerId,
    IN GT_U32                                               iterationNum,
    OUT UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC    *outputResultsPtr

)
{
    GT_STATUS   st, st1;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entryGet;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC      *fdbEntryFormatPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC      *fdbEntryFormatGetPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     *entryPtr;
    GT_BOOL                                                 *entryAddedPtr;
    GT_BOOL                                                 updateDipLowOctets = GT_FALSE;

    entryPtr        = outputResultsPtr->entryPtr;
    entryAddedPtr   = outputResultsPtr->entryAddedPtr;

    fdbEntryFormatPtr = &entryPtr->format.fdbEntryIpv6UcFormat;
    fdbEntryFormatGetPtr = &entryGet.format.fdbEntryIpv6UcFormat;

    cpssOsMemSet(&entryGet,0,sizeof(entryGet));

    if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E ||
       SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
    {
        updateDipLowOctets = GT_TRUE;
    }

    ipv6UcAddressGet(iterationNum, entryPtr, 1, updateDipLowOctets);

    fdbEntryAddInfo.rehashEnable    = globalRehashEnable;
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, entryPtr, &fdbEntryAddInfo, st);
    st1 = prvCpssDxChBrgFdbManagerBanksPopulationCheck(fdbManagerId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st1,
        "prvCpssDxChBrgFdbManagerBanksPopulationCheck: expected GT_OK in manager [%d], iteration [%d]",
        fdbManagerId, iterationNum);

    if(st == GT_OK)
    {
        *entryAddedPtr = GT_TRUE;
    }
    else if (st != GT_FULL)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "fdbManagerEntryAdd_ipv4Uc: unexpected error in manager [%d]",
            fdbManagerId);
    }

    if((*entryAddedPtr) == GT_TRUE)
    {
        /* the entry was added ... check if the entry retrieved as was set */

        /* start -- the parts of the key */
        cpssOsMemCpy(&fdbEntryFormatGetPtr->ipv6Addr,
                     &fdbEntryFormatPtr->ipv6Addr,
                     sizeof(fdbEntryFormatPtr->ipv6Addr));

        fdbEntryFormatGetPtr->vrfId = fdbEntryFormatPtr->vrfId;
        /* end -- the parts of the key */
        entryGet.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E;

        st = cpssDxChBrgFdbManagerEntryGet(fdbManagerId,&entryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryGet: expected GT_OK in manager [%d]",
            fdbManagerId);

        /***************************************************************/
        /* compare the full entry : entry type , fdbEntryIpv4UcFormat */
        /***************************************************************/
        UTF_VERIFY_EQUAL1_STRING_MAC(entryPtr->fdbEntryType, entryGet.fdbEntryType,
            "cpssDxChBrgFdbManagerEntryGet: expected fdbEntryType in 'get' entry to match 'set' in manager [%d]",
            fdbManagerId);

        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(
                                            fdbEntryFormatGetPtr, fdbEntryFormatPtr,
                                            sizeof(CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC)),
            "cpssDxChBrgFdbManagerEntryGet: expected 'get' entry to match 'set' entry in manager [%d]",
            fdbManagerId);
    }
}

/* Test API to add entries
 * fdbEntryType -> if any specific type only
 *                 any value higher than 4 represents random entries
 *
 */
GT_STATUS prvTgfBrgFdbManagerAddEntries
(
    IN GT_U32                                       fdbManagerId,
    IN GT_U32                                       entriesToAdd,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT     inputFdbEntryType,
    IN GT_BOOL                                      cuckooEnable,
    INOUT GT_U32                                    *numEntriesAddedPtr
)
{
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                 entry;
    GT_U32                                              numEntriesAdded = 0;
    GT_U32                                              ii;
    GT_BOOL                                             entryAdded;
    UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC    outputResults;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT            fdbEntryType;
    GT_BOOL                                             restoreRehashEnable;

    outputResults.entryPtr                          = &entry;
    outputResults.entryAddedPtr                     = &entryAdded;

    restoreRehashEnable = globalRehashEnable;
    globalRehashEnable = cuckooEnable;
    PRV_UTF_LOG1_MAC("start - add [%d] entries \n", entriesToAdd);

    for(ii = 0 ; ii < entriesToAdd; ii++)
    {
        entryAdded = GT_FALSE;
        fdbEntryType = (inputFdbEntryType < CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE__LAST__E)?
            inputFdbEntryType : fdbEntryType_valid_arr[ii % UT_FDB_ENTRY_TYPE_VALID_NUM];
        switch(fdbEntryType)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
                fdbManagerEntryAdd_macAddr(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
                fdbManagerEntryAdd_ipv4Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
                fdbManagerEntryAdd_ipv6Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
                fdbManagerEntryAdd_ipv4Uc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
                fdbManagerEntryAdd_ipv6Uc(fdbManagerId, ii, &outputResults);
                break;
            default:
                break;
        }
        if(entryAdded)
        {
            numEntriesAdded++;
        }
    }

    if(entriesToAdd)
    {
        PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
            entriesToAdd, numEntriesAdded, (100*numEntriesAdded)/entriesToAdd);

        *numEntriesAddedPtr += numEntriesAdded;
    }
    globalRehashEnable = restoreRehashEnable;

    return GT_OK;
}


/* check bounders of numeric field , and high values (31,32 bits value)  :
expected to fail */
static void CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd
(
    IN GT_U32                       fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr,
    IN GT_U32                       *fieldPtr,
    IN GT_U32                       testedField_minValue,
    IN GT_U32                       testedField_maxValue,
    IN GT_U32                       step
)
{

    GT_STATUS   st, st1;
    GT_U32  ii,iiMax = 3,iiMin;
    GT_U32  origValue = *fieldPtr;

    if(testedField_minValue != 0) /* we check 0xFFFFFFFF anyway */
    {
        checkNonSupportedValues[iiMax++] = (GT_U32)((testedField_minValue) - 1);
    }
    if(testedField_maxValue!= 0)
    {
        checkNonSupportedValues[iiMax++] = (GT_U32)((testedField_maxValue)+1);
    }

    PRV_UTF_LOG0_MAC("check field with invalid field values\n");

    for(ii = 0; ii < iiMax; ii++)
    {
        *fieldPtr = checkNonSupportedValues[ii];
        FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, entryPtr, paramsPtr, st);
        /* restore orig value */
        *fieldPtr = origValue;
        if (st != GT_OUT_OF_RANGE)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                "cpssDxChBrgFdbManagerEntryAdd: [%d] expected to GT_BAD_PARAM on value [%d]",
                *fieldPtr,
                checkNonSupportedValues[ii]);
        }
    }

    iiMax = testedField_maxValue;
    iiMin = testedField_minValue;

    if (iiMax == iiMin)
    {
        return;
    }

    PRV_UTF_LOG4_MAC("check [%d] with valid values , range[%d..%d] step[%d] \n", *fieldPtr, iiMin, iiMax, step); /**/

    for(ii = iiMin ; ii <= iiMax; ii+=step)
    {
        *fieldPtr = ii;
        FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, entryPtr, paramsPtr, st);
        /* remove the entry */
        FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId, entryPtr, st1);
        /* restore orig value */
        *fieldPtr = origValue;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryAdd: %d expected to GT_OK on value [%d]",
            fieldPtr,
            ii);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st1,
            "cpssDxChBrgFdbManagerEntryDelete: %d expected to GT_OK on value [%d]",
            *fieldPtr,
            ii);
    }
}

/* check bounders of numeric U16 field , and high values (15,16 bits value)  :
expected to fail */
static void CHECK_BOUNDERS_NUMERIC_FIELD_U16_cpssDxChBrgFdbManagerEntryAdd
(
    IN GT_U32                       fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr,
    IN GT_U16                       *fieldPtr,
    IN GT_U16                       testedField_minValue,
    IN GT_U16                       testedField_maxValue,
    IN GT_U16                       step
)
{

    GT_STATUS   st, st1;
    GT_U16  ii,iiMax = 3,iiMin;
    GT_U16  origValue = *fieldPtr;

    if(testedField_minValue != 0) /* we check 0xFFFFFFFF anyway */
    {
        checkNonSupportedValues_U16[iiMax++] = (GT_U16)((testedField_minValue) - 1);
    }
    if((testedField_maxValue!= 0) && (testedField_maxValue != 0xFFFF))
    {
        checkNonSupportedValues_U16[iiMax++] = (GT_U16)((testedField_maxValue)+1);
    }

    PRV_UTF_LOG0_MAC("check U16 field with invalid field values\n");

    for(ii = 0; ii < iiMax; ii++)
    {
        *fieldPtr = checkNonSupportedValues_U16[ii];
        FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, entryPtr, paramsPtr, st);
        /* restore orig value */
        *fieldPtr = origValue;
        if (st != GT_OUT_OF_RANGE)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                "cpssDxChBrgFdbManagerEntryAdd: [%d] expected to GT_BAD_PARAM on value [%d]",
                *fieldPtr,
                checkNonSupportedValues_U16[ii]);
        }
    }

    iiMax = testedField_maxValue;
    iiMin = testedField_minValue;

    if (iiMax == iiMin)
    {
        return;
    }

    PRV_UTF_LOG4_MAC("check [%d] with valid values , range[%d..%d] step[%d] \n", *fieldPtr, iiMin, iiMax, step); /**/

    for(ii = iiMin ; ii <= iiMax; ii+=step)
    {
        *fieldPtr = ii;
        FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, entryPtr, paramsPtr, st);
        /* remove the entry */
        FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId, entryPtr, st1);
        /* restore orig value */
        *fieldPtr = origValue;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryAdd: %d expected to GT_OK on value [%d]",
            fieldPtr,
            ii);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st1,
            "cpssDxChBrgFdbManagerEntryDelete: %d expected to GT_OK on value [%d]",
            *fieldPtr,
            ii);
    }
}

/* check bounders of numeric field , and high values (31,32 bits value)  :
expected to fail */
static void CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryDelete
(
    IN GT_U32                       fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC  *entryPtr,
    IN GT_U32                       *fieldPtr,
    IN GT_U32                       testedField_minValue,
    IN GT_U32                       testedField_maxValue,
    IN GT_U32                       step
)
{

    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   params;
    GT_STATUS   st, st1;
    GT_U32  ii,iiMax = 3,iiMin;
    GT_U32  origValue = *fieldPtr;
    iiMax = 3;
    if((testedField_minValue) != 0) /* we check 0xFFFFFFFF anyway */
    {
        checkNonSupportedValues[iiMax++] = (GT_U32)((testedField_minValue) - 1);
    }
    checkNonSupportedValues[iiMax++] = (GT_U32)((testedField_maxValue) + 1);

    PRV_UTF_LOG0_MAC("check field with invalid field values\n");

    for(ii = 0; ii < iiMax; ii++)
    {
        *fieldPtr = checkNonSupportedValues[ii];
        FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId, entryPtr,st);
        /* restore orig value */
        *fieldPtr = origValue;
        if (st != GT_OUT_OF_RANGE)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                "cpssDxChBrgFdbManagerEntryDelete: [%d] expected to GT_BAD_PARAM on value [%d]",
                *fieldPtr,
                checkNonSupportedValues[ii]);
        }
    }

    iiMax = testedField_maxValue;
    iiMin = testedField_minValue;

    PRV_UTF_LOG4_MAC("check [%d] with valid values , range[%d..%d] step[%d] \n",*fieldPtr,iiMin,iiMax,step); /**/

    for(ii = iiMin ; ii <= iiMax; ii+=step)
    {
        *fieldPtr = ii;
        FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, entryPtr, &params, st);
        /* remove the entry */
        FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId, entryPtr, st1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryAdd: %d expected to GT_OK on value [%d]",
            fieldPtr,
            ii);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st1,
            "cpssDxChBrgFdbManagerEntryDelete: %d expected to GT_OK on value [%d]",
            *fieldPtr,
            ii);
    }
    /* restore orig value */
    *fieldPtr = origValue;
}

/* check bounds of numeric field , and high values (31,32 bits value)  :
expected to fail */
static void CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate
(
    IN GT_U32                       fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC  *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC * paramsPtr,
    IN GT_U32                       *fieldPtr,
    IN GT_U32                       testedField_minValue,
    IN GT_U32                       testedField_maxValue,
    IN GT_U32                       step
)
{

    GT_STATUS   st;
    GT_U32  ii,iiMax = 3, iiMin;
    GT_U32  origValue = *fieldPtr;

    if (testedField_minValue != 0) /* we check 0xFFFFFFFF anyway */
    {
        checkNonSupportedValues[iiMax++] = (GT_U32)((testedField_minValue) - 1);
    }

    if (testedField_maxValue != 0)
    {
        checkNonSupportedValues[iiMax++] = (GT_U32)((testedField_maxValue)+1);
    }

    PRV_UTF_LOG0_MAC("check field with invalid field values\n");

    for(ii = 0; ii < iiMax; ii++)
    {
        *fieldPtr = checkNonSupportedValues[ii];
        FDB_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(fdbManagerId, entryPtr, paramsPtr,st);
        /* restore orig value */
        *fieldPtr = origValue;
        if (st != GT_OUT_OF_RANGE)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                "cpssDxChBrgFdbManagerEntryUpdate: [%d] expected to GT_BAD_PARAM on value [%d]",
                *fieldPtr,
                checkNonSupportedValues[ii]);
        }
    }

    iiMax = testedField_maxValue;
    iiMin = testedField_minValue;

    if (iiMax == iiMin)
    {
        return;
    }

    PRV_UTF_LOG4_MAC("check [%d] with valid values , range[%d..%d] step[%d] \n", *fieldPtr, iiMin, iiMax, step);

    for(ii = iiMin ; ii <= iiMax; ii+=step)
    {
        *fieldPtr = ii;
        FDB_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(fdbManagerId, entryPtr, paramsPtr,st);
        /* restore orig value */
        *fieldPtr = origValue;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryUpdate: %d expected to GT_OK on value [%d]",
            fieldPtr,
            ii);
    }
}

/* check bounds of numeric U16 field , and high values (15,16 bits value)  :
expected to fail */
static void CHECK_BOUNDERS_NUMERIC_FIELD_U16_cpssDxChBrgFdbManagerEntryUpdate
(
    IN GT_U32                       fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC  *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC * paramsPtr,
    IN GT_U16                       *fieldPtr,
    IN GT_U16                       testedField_minValue,
    IN GT_U16                       testedField_maxValue,
    IN GT_U16                       step
)
{

    GT_STATUS   st;
    GT_U16  ii,iiMax = 3, iiMin;
    GT_U16  origValue = *fieldPtr;

    if (testedField_minValue != 0) /* we check 0xFFFF anyway */
    {
        checkNonSupportedValues_U16[iiMax++] = (GT_U16)((testedField_minValue) - 1);
    }

    if (testedField_maxValue != 0)
    {
        checkNonSupportedValues_U16[iiMax++] = (GT_U16)((testedField_maxValue)+1);
    }

    PRV_UTF_LOG0_MAC("check field with invalid field values\n");

    for(ii = 0; ii < iiMax; ii++)
    {
        *fieldPtr = checkNonSupportedValues_U16[ii];
        FDB_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(fdbManagerId, entryPtr, paramsPtr,st);
        /* restore orig value */
        *fieldPtr = origValue;
        if (st != GT_OUT_OF_RANGE)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                "cpssDxChBrgFdbManagerEntryUpdate: [%d] expected to GT_BAD_PARAM on value [%d]",
                *fieldPtr,
                checkNonSupportedValues[ii]);
        }
    }

    iiMax = testedField_maxValue;
    iiMin = testedField_minValue;

    if (iiMax == iiMin)
    {
        return;
    }

    PRV_UTF_LOG4_MAC("check [%d] with valid values , range[%d..%d] step[%d] \n", *fieldPtr, iiMin, iiMax, step);

    for(ii = iiMin ; ii <= iiMax; ii+=step)
    {
        *fieldPtr = ii;
        FDB_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(fdbManagerId, entryPtr, paramsPtr,st);
        /* restore orig value */
        *fieldPtr = origValue;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryUpdate: %d expected to GT_OK on value [%d]",
            fieldPtr,
            ii);
    }
}

/* check 4 valid values of numeric field , and values 0..28 and high values (31,32 bits value)  :
expected to fail */
static void CHECK_BOUNDERS_5_VALUES_FIELD_cpssDxChBrgFdbManagerEntryAdd
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr,
    IN CPSS_PACKET_CMD_ENT                              *fieldPtr,
    IN CPSS_PACKET_CMD_ENT                              v1,
    IN CPSS_PACKET_CMD_ENT                              v2,
    IN CPSS_PACKET_CMD_ENT                              v3,
    IN CPSS_PACKET_CMD_ENT                              v4,
    IN CPSS_PACKET_CMD_ENT                              v5
)
{
    GT_STATUS   st, st1;
    GT_U32  ii, iiMax;
    GT_U32  origValue = *fieldPtr;
    GT_U32  checkSupportedValues[5];
    iiMax = 32;
    for(ii=4; ii<iiMax;ii++)
    {
        checkNonSupportedValues[ii] = ii-4;
    }

    PRV_UTF_LOG0_MAC("check enum with invalid values \n");

    for(ii = 0; ii < iiMax; ii++)
    {
        *fieldPtr = checkNonSupportedValues[ii];
        if(*fieldPtr == v1 || *fieldPtr == v2 || *fieldPtr == v3 || *fieldPtr == v4 || *fieldPtr == v5 )
        {   /* valid value */
            continue;
        }
        FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, entryPtr, paramsPtr, st);
        /* restore orig value */
        *fieldPtr = origValue;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerEntryAdd: %d expected to GT_BAD_PARAM on value [%d]",
            *fieldPtr,
            checkNonSupportedValues[ii]);
    }

    iiMax = 0;
    checkSupportedValues[iiMax++] = v1;
    checkSupportedValues[iiMax++] = v2;
    checkSupportedValues[iiMax++] = v3;
    checkSupportedValues[iiMax++] = v4;
    checkSupportedValues[iiMax++] = v5;
    PRV_UTF_LOG6_MAC("check [%d] with valid values , [%d,%d,%d,%d,%d] \n",*fieldPtr,v1,v2,v3,v4,v5); /**/
    /* check valid values */
    for(ii = 0; ii < iiMax; ii++)
    {
        *fieldPtr = checkSupportedValues[ii];
        FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, entryPtr, paramsPtr, st);
        /* remove the entry */
        FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId, entryPtr, st1);
        /* restore orig value */
        *fieldPtr = origValue;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryAdd: %d expected to GT_OK on value [%d]",
            *fieldPtr,
            checkSupportedValues[ii]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st1,
            "cpssDxChBrgFdbManagerEntryDelete: %d expected to GT_OK on value [%d]",
            *fieldPtr,
            ii);
    }
}

/* check 5 valid values of numeric field , and values 0..28 and high values (31,32 bits value)  :
expected to fail */
static void CHECK_BOUNDERS_5_VALUES_FIELD_cpssDxChBrgFdbManagerEntryUpdate
(
    IN GT_U32                                   fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC      *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC * paramsPtr,
    IN CPSS_PACKET_CMD_ENT                      *fieldPtr,
    IN CPSS_PACKET_CMD_ENT                      v1,
    IN CPSS_PACKET_CMD_ENT                      v2,
    IN CPSS_PACKET_CMD_ENT                      v3,
    IN CPSS_PACKET_CMD_ENT                      v4,
    IN CPSS_PACKET_CMD_ENT                      v5
)
{
    GT_STATUS   st;
    GT_U32  ii, iiMax;
    GT_U32  checkSupportedValues[5];
    iiMax = 32;

    for(ii=4; ii<iiMax;ii++)
    {
        checkNonSupportedValues[ii] = ii-4;
    }

    PRV_UTF_LOG0_MAC("check enum with invalid values \n");

    for(ii = 0; ii < iiMax; ii++)
    {
        *fieldPtr = checkNonSupportedValues[ii];
        if(*fieldPtr == v1 || *fieldPtr == v2 || *fieldPtr == v3 || *fieldPtr == v4 || *fieldPtr == v5)
        {   /* valid value */
            continue;
        }
        FDB_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(fdbManagerId, entryPtr, paramsPtr,st);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerEntryAdd: %d expected to GT_BAD_PARAM on value [%d]",
            *fieldPtr,
            checkNonSupportedValues[ii]);
    }

    iiMax = 0;
    checkSupportedValues[iiMax++] = v1;
    checkSupportedValues[iiMax++] = v2;
    checkSupportedValues[iiMax++] = v3;
    checkSupportedValues[iiMax++] = v4;
    checkSupportedValues[iiMax++] = v5;
    PRV_UTF_LOG6_MAC("check [%d] with valid values , [%d,%d,%d,%d,%d] \n",*fieldPtr,v1,v2,v3,v4,v5); /**/
    /* check valid values */
    for(ii = 0; ii < iiMax; ii++)
    {
        *fieldPtr = checkSupportedValues[ii];
        FDB_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(fdbManagerId, entryPtr, paramsPtr,st);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryUpdate: %d expected to GT_OK on value [%d]",
            *fieldPtr,
            checkSupportedValues[ii]);
    }
}

static void     bad_param_check_fdbManagerEntryAdd_macAddr(
    IN GT_U32                                               fdbManagerId
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    GT_U32                                           max_sourceID = 0;
    GT_U32                                           max_vid1     = 0;
    GT_U32                                           max_security_level = 0;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC *macAddrFormatPtr = &entry.format.fdbEntryMacAddrFormat;

    /* build 'valid' entry : mac addr */
    fdbManagerEntryBuild_macAddr(&entry);

    fdbEntryAddInfo.tempEntryExist  = GT_TRUE;
    fdbEntryAddInfo.tempEntryOffset = BANK_AS_PER_DEVICE(9);/*bankId*/
    fdbEntryAddInfo.rehashEnable    = GT_FALSE;

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId,&entry,&fdbEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "fdbManagerEntryAdd_macAddr: unexpected GT_OK [%d]",
        fdbManagerId);

    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId,&entry,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerEntryDelete: unexpected GT_OK [%d]",
        fdbManagerId);

    if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
    {
        switch(global_macEntryMuxingMode)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E:
                max_sourceID       = BIT_12-1; /* 12 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E:
                max_vid1           = BIT_12-1; /* 12 bits */
                break;
            default:
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,GT_NOT_IMPLEMENTED,
                        "bad_param_check_fdbManagerEntryAdd_macAddr: not implemented case in manager [%d]",
                        fdbManagerId);
        }
        max_security_level  = 0;
    }
    else if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E)
    {
        max_sourceID        = 0xFFF;    /* 12 bits */
        max_vid1            = 0;        /* Not supported */
        max_security_level  = 0x0;      /* Like SIP6 */

        CHECK_BOUNDERS_5_VALUES_FIELD_cpssDxChBrgFdbManagerEntryAdd(
                fdbManagerId, &entry, &fdbEntryAddInfo,
                &macAddrFormatPtr->daCommand,
                CPSS_PACKET_CMD_FORWARD_E,
                CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
                CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                CPSS_PACKET_CMD_DROP_HARD_E,
                CPSS_PACKET_CMD_DROP_SOFT_E);

        CHECK_BOUNDERS_5_VALUES_FIELD_cpssDxChBrgFdbManagerEntryAdd(
                fdbManagerId, &entry, &fdbEntryAddInfo,
                &macAddrFormatPtr->saCommand,
                CPSS_PACKET_CMD_FORWARD_E        ,
                CPSS_PACKET_CMD_FORWARD_E        ,/* not support CPSS_PACKET_CMD_MIRROR_TO_CPU_E */
                CPSS_PACKET_CMD_FORWARD_E        ,/* not support CPSS_PACKET_CMD_TRAP_TO_CPU_E   */
                CPSS_PACKET_CMD_DROP_HARD_E,
                CPSS_PACKET_CMD_DROP_SOFT_E);
    }
    else
    {
        if (SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)
        {
            switch(global_entryMuxingMode)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E:
                    max_sourceID = BIT_9-1;   /* 9 Bits */
                    max_vid1     = 0;
                    max_security_level  = 0x7;   /* 3 Bits */
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E:
                    max_sourceID = BIT_12-1;  /* 12 Bits */
                    max_vid1     = 0;
                    max_security_level  = 0x7;   /* 3 Bits */
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E:
                    max_sourceID = BIT_6-1;   /* 6 Bits */
                    max_vid1     = BIT_12-1;  /* 12 bits */
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E:
                    max_sourceID = BIT_6-1;   /* 6 Bits */
                    max_vid1     = BIT_12-1;  /* 12 bits */
                    break;
                default:
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,GT_NOT_IMPLEMENTED,
                            "bad_param_check_fdbManagerEntryAdd_macAddr: not implemented case in manager [%d]",
                            fdbManagerId);
            }
            CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
                    fdbManagerId, &entry, &fdbEntryAddInfo,
                    &macAddrFormatPtr->daQoSParameterSetIndex,
                    0/*min*/,
                    0x7,
                    1/*step*/);
            CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
                    fdbManagerId, &entry, &fdbEntryAddInfo,
                    &macAddrFormatPtr->saQoSParameterSetIndex,
                    0/*min*/,
                    0x7,
                    1/*step*/);
            CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
                    fdbManagerId, &entry, &fdbEntryAddInfo,
                    &macAddrFormatPtr->daCommand,
                    0/*min*/,
                    0x4/*max*/,
                    1/*step*/);
            CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
                    fdbManagerId, &entry, &fdbEntryAddInfo,
                    &macAddrFormatPtr->saCommand,
                    0/*min*/,
                    0x4/*max*/,
                    1/*step*/);
        }
        else
        {
            max_sourceID        = 0x1F;  /* 5 bits */
            max_vid1            = 0;     /* Not supported */
            max_security_level  = 0x7;   /* 3 Bits */
        }
   }

    /**************************/
    /* start to modify fields */
    /**************************/
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &macAddrFormatPtr->fid,
        0/*min*/,
        FID_MASK_AS_PER_DEVICE()/*max*/,
        1/*step*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(/* muxing mode must be 0 */
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &macAddrFormatPtr->vid1,
        0/*min*/,
        max_vid1/*max*/,
        1/*step*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &macAddrFormatPtr->saSecurityLevel,
        0/*min*/,
        max_security_level/*max*/,
        1/*step*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(/* muxing mode must be 0 */
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &macAddrFormatPtr->daSecurityLevel,
        0/*min*/,
        max_security_level/*max*/,
        1/*step*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(/* muxing mode must be 0 */
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &macAddrFormatPtr->userDefined,
        0/*min*/,
        0/*max*/,
        1/*step*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(/* muxing mode must be 0 */
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &macAddrFormatPtr->sourceID,
        0/*min*/,
        max_sourceID/*max*/,
        1/*step*/);

    /* we not check GT_BOOL ! */
    /*
        fdbEntryMacAddrFormatPtr->isStatic        = GT_FALSE;
        fdbEntryMacAddrFormatPtr->age             = GT_TRUE;
        fdbEntryMacAddrFormatPtr->daRoute         = GT_FALSE;
        fdbEntryMacAddrFormatPtr->appSpecificCpuCode = GT_TRUE;
    */

}

static void bad_param_check_fdbManagerEntryUpdate_ipvMc(
    IN GT_U32   fdbManagerId,
    IN GT_BOOL  isIpv4
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC params;

    GT_U32  max_sourceID = 0;
    GT_U32  max_udb      = 0;
    GT_U32  max_da_access= 0;

    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC *ipMcAddrFormatPtr;
    /* build 'valid' entry : IP MC addr */
    fdbManagerEntryBuild_ipMcAddr(&entry, isIpv4, 0);

    if (isIpv4)
    {
        ipMcAddrFormatPtr = &entry.format.fdbEntryIpv4McFormat;
    }
    else
    {
        ipMcAddrFormatPtr = &entry.format.fdbEntryIpv6McFormat;
    }

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId,&entry,&fdbEntryAddInfo, st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryAdd: unexpected GT_OK [%d]",
        fdbManagerId);

    if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
    {
        switch(global_ipMcEntryMuxingMode)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E:
                max_sourceID           = BIT_7-1; /* 7 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_4_SRC_ID_3_E:
                max_sourceID           = BIT_3-1; /* 3 bits */
                max_udb                = BIT_4-1; /* 4 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_E:
                max_udb                = BIT_7-1; /* 7 bits */
                GT_ATTR_FALLTHROUGH;
            default:
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_BAD_PARAM,
                        "bad_param_check_fdbManagerEntryAdd_ipv4Mc: bad IP MC muxing mode [%d]",
                        global_ipMcEntryMuxingMode);
        }
        CHECK_BOUNDERS_5_VALUES_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
                fdbManagerId, &entry, &params,
                &ipMcAddrFormatPtr->daCommand,
                CPSS_PACKET_CMD_FORWARD_E              ,
                CPSS_PACKET_CMD_MIRROR_TO_CPU_E        ,
                CPSS_PACKET_CMD_TRAP_TO_CPU_E          ,
                CPSS_PACKET_CMD_DROP_HARD_E,
                CPSS_PACKET_CMD_DROP_SOFT_E);
        max_da_access = 1;
    }
    else if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E)
    {
        max_sourceID        = 0;     /* 0 in case of AC5 multicast entry */
        max_udb             = 0;     /* 0 in case of AC5 multicast entry */
        max_da_access       = 0;
    }
    else
    {
        if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
        {
            max_sourceID = 0;
            switch(global_entryMuxingMode)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E:
                    max_udb      = BIT_8  - 1;  /* 8 Bits */
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E:
                    max_udb      = BIT_5  - 1;  /* 5  Bits */
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E:
                    max_udb      = BIT_8  - 1;  /* 8 Bits */
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E:
                    max_udb      = BIT_5  - 1;  /* 5 Bits */
                    break;
                default:
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_BAD_PARAM,
                            "bad_param_check_fdbManagerEntryAdd_ipv4Mc: bad IP MC muxing mode [%d]",
                            global_ipMcEntryMuxingMode);
            }
        }
        else
        {
            max_sourceID        = 0;     /* 0 in case of AC5 multicast entry */
            max_udb             = 0;     /* 0 in case of AC5 multicast entry */
        }
        max_da_access           = 0x7;   /* 3 Bits */
        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
                fdbManagerId, &entry, &params,
                &ipMcAddrFormatPtr->daCommand,
                0/*min*/,
                0x4/*max*/,
                1/*step*/);
    }

    /**************************/
    /* start to check fields */
    /**************************/
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
            fdbManagerId, &entry, &params,
            &ipMcAddrFormatPtr->daSecurityLevel,
            0/*min*/,
            max_da_access/*max*/,
            1/*step*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
            fdbManagerId, &entry, &params,
        &ipMcAddrFormatPtr->userDefined,
        0/*min*/,
        max_udb/*max*/,
        1/*step*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
        fdbManagerId, &entry, &params,
        &ipMcAddrFormatPtr->sourceID,
        0/*min*/,
        max_sourceID/*max*/,
        1/*step*/);

    /* we not check GT_BOOL ! */
    /*
        fdbEntryMacAddrFormatPtr->isStatic        = GT_FALSE;
        fdbEntryMacAddrFormatPtr->age             = GT_TRUE;
        fdbEntryMacAddrFormatPtr->daRoute         = GT_FALSE;
        fdbEntryMacAddrFormatPtr->appSpecificCpuCode = GT_TRUE;
    */

    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId, &entry, st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryDelete: unexpected GT_OK [%d]",
        fdbManagerId);

}

static void bad_param_check_fdbManagerEntryUpdate_ipv4Mc(
    IN GT_U32   fdbManagerId
)
{
    bad_param_check_fdbManagerEntryUpdate_ipvMc(fdbManagerId, GT_TRUE);
}

static void bad_param_check_fdbManagerEntryUpdate_macAddr(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                 entry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC      fdbEntryAddInfo;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC params;

    GT_U32  max_sourceID              = 0;
    GT_U32  max_vid1                  = 0;
    GT_U32  max_sa_security_level     = 0;
    GT_U32  max_da_security_level     = 0;

    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC *macAddrFormatPtr = &entry.format.fdbEntryMacAddrFormat;

    /* build 'valid' entry : mac addr */
    fdbManagerEntryBuild_macAddr(&entry);

    fdbEntryAddInfo.tempEntryExist  = GT_TRUE;
    fdbEntryAddInfo.tempEntryOffset = BANK_AS_PER_DEVICE(9);/*bankId*/
    fdbEntryAddInfo.rehashEnable    = GT_FALSE;

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, &entry, &fdbEntryAddInfo, st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "fdbManagerEntryAdd_macAddr: unexpected GT_OK [%d]",
        fdbManagerId);

    if (SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
    {
        switch(global_macEntryMuxingMode)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E:
                max_sourceID           = BIT_12-1; /* 12 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E:
                max_vid1           = BIT_12-1; /* 12 bits */
                break;
            default:
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,GT_NOT_IMPLEMENTED,
                        "bad_param_check_fdbManagerEntryAdd_macAddr: not implemented case in manager [%d]",
                        fdbManagerId);
        }
        CHECK_BOUNDERS_5_VALUES_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
                fdbManagerId, &entry, &params,
                &macAddrFormatPtr->daCommand,
                CPSS_PACKET_CMD_FORWARD_E,
                CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
                CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                CPSS_PACKET_CMD_DROP_HARD_E,
                CPSS_PACKET_CMD_DROP_SOFT_E);

        CHECK_BOUNDERS_5_VALUES_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
                fdbManagerId, &entry, &params,
                &macAddrFormatPtr->saCommand,
                CPSS_PACKET_CMD_FORWARD_E        ,
                CPSS_PACKET_CMD_FORWARD_E        ,/* not support CPSS_PACKET_CMD_MIRROR_TO_CPU_E */
                CPSS_PACKET_CMD_FORWARD_E        ,/* not support CPSS_PACKET_CMD_TRAP_TO_CPU_E   */
                CPSS_PACKET_CMD_DROP_HARD_E,
                CPSS_PACKET_CMD_DROP_SOFT_E);

        max_da_security_level      = 0;
        max_sa_security_level      = 1;
    }
    else if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E)
    {
        CHECK_BOUNDERS_5_VALUES_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
                fdbManagerId, &entry, &params,
                &macAddrFormatPtr->daCommand,
                CPSS_PACKET_CMD_FORWARD_E,
                CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
                CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                CPSS_PACKET_CMD_DROP_HARD_E,
                CPSS_PACKET_CMD_DROP_SOFT_E);

        CHECK_BOUNDERS_5_VALUES_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
                fdbManagerId, &entry, &params,
                &macAddrFormatPtr->saCommand,
                CPSS_PACKET_CMD_FORWARD_E        ,
                CPSS_PACKET_CMD_FORWARD_E        ,/* not support CPSS_PACKET_CMD_MIRROR_TO_CPU_E */
                CPSS_PACKET_CMD_FORWARD_E        ,/* not support CPSS_PACKET_CMD_TRAP_TO_CPU_E   */
                CPSS_PACKET_CMD_DROP_HARD_E,
                CPSS_PACKET_CMD_DROP_SOFT_E);
    }
    else
    {
        if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)
        {
            switch(global_entryMuxingMode)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E:
                    max_sourceID = BIT_9-1;   /* 9 Bits */
                    max_vid1     = 0;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E:
                    max_sourceID = BIT_12-1;  /* 12 Bits */
                    max_vid1     = 0;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E:
                    max_sourceID = BIT_6-1;   /* 6 Bits */
                    max_vid1     = BIT_12-1;  /* 12 bits */
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E:
                    max_sourceID = BIT_6-1;   /* 6 Bits */
                    max_vid1     = BIT_12-1;  /* 12 bits */
                    break;
                default:
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,GT_NOT_IMPLEMENTED,
                            "bad_param_check_fdbManagerEntryAdd_macAddr: not implemented case in manager [%d]",
                            fdbManagerId);
            }
        }
        else /* SIP4 */
        {
            max_sourceID           = 0x1F;  /* 5 bits */
            max_vid1               = 0;     /* Not supported */
        }
        max_da_security_level      = 0x7;   /* 3 Bits */
        max_sa_security_level      = 0x7;   /* 3 Bits */

        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
                fdbManagerId, &entry, &params,
                &macAddrFormatPtr->daCommand,
                0/*min*/,
                0x4/*max*/,
                1/*step*/);
        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
                fdbManagerId, &entry, &params,
                &macAddrFormatPtr->saCommand,
                0/*min*/,
                0x4/*max*/,
                1/*step*/);
    }

    /**************************/
    /* start to modify fields */
    /**************************/
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(/* muxing mode must be 0 */
        fdbManagerId, &entry, &params,
        &macAddrFormatPtr->vid1,
        0/*min*/,
        max_vid1/*max*/,
        1/*step*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
        fdbManagerId, &entry, &params,
        &macAddrFormatPtr->saSecurityLevel,
        0/*min*/,
        max_sa_security_level/*max*/,
        1/*step*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(/* muxing mode must be 0 */
        fdbManagerId, &entry, &params,
        &macAddrFormatPtr->daSecurityLevel,
        0/*min*/,
        max_da_security_level/*max*/,
        1/*step*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(/* muxing mode must be 0 */
        fdbManagerId, &entry, &params,
        &macAddrFormatPtr->userDefined,
        0/*min*/,
        0/*max*/,
        1/*step*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(/* muxing mode must be 0 */
        fdbManagerId, &entry, &params,
        &macAddrFormatPtr->sourceID,
        0/*min*/,
        max_sourceID/*max*/,
        1/*step*/);

    /* we not check GT_BOOL ! */
    /*
        fdbEntryMacAddrFormatPtr->isStatic        = GT_FALSE;
        fdbEntryMacAddrFormatPtr->age             = GT_TRUE;
        fdbEntryMacAddrFormatPtr->daRoute         = GT_FALSE;
        fdbEntryMacAddrFormatPtr->appSpecificCpuCode = GT_TRUE;
    */
    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId, &entry, st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryDelete: unexpected GT_OK [%d]",
        fdbManagerId);

}

static void bad_param_check_fdbManagerEntryAdd_ipvMc(
    IN GT_U32                                               fdbManagerId,
    IN GT_BOOL                                              isIpv4
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     entry;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    GT_U32  max_sourceID = 0;
    GT_U32  max_udb      = 0;
    GT_U32  max_da_security_level = 0;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC *ipMcAddrFormatPtr = &entry.format.fdbEntryIpv4McFormat;

    /* build 'valid' entry : IP MC addr */
    fdbManagerEntryBuild_ipMcAddr(&entry, isIpv4, 1);

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId,&entry,&fdbEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerEntryAdd: unexpected GT_OK [%d]",
        fdbManagerId);

    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId,&entry,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerEntryDelete: unexpected GT_OK [%d]",
        fdbManagerId);

    if (SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
    {
        switch(global_ipMcEntryMuxingMode)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E:
                max_sourceID           = BIT_7-1; /* 7 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_4_SRC_ID_3_E:
                max_sourceID           = BIT_3-1; /* 3 bits */
                max_udb                = BIT_4-1; /* 4 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_E:
                max_udb                = BIT_7-1; /* 7 bits */
                GT_ATTR_FALLTHROUGH;
            default:
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_BAD_PARAM,
                        "bad_param_check_fdbManagerEntryAdd_ipv4Mc: bad IP MC muxing mode [%d]",
                        global_ipMcEntryMuxingMode);
        }
        CHECK_BOUNDERS_5_VALUES_FIELD_cpssDxChBrgFdbManagerEntryAdd(
                fdbManagerId, &entry, &fdbEntryAddInfo,
                &ipMcAddrFormatPtr->daCommand,
                CPSS_PACKET_CMD_FORWARD_E              ,
                CPSS_PACKET_CMD_MIRROR_TO_CPU_E        ,
                CPSS_PACKET_CMD_TRAP_TO_CPU_E          ,
                CPSS_PACKET_CMD_DROP_HARD_E,
                CPSS_PACKET_CMD_DROP_SOFT_E);
        max_da_security_level = 1;
    }
    else
    {
        if (SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)
        {
            max_sourceID = 0;           /* IPMC - source ID is not valid */
            switch(global_entryMuxingMode)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E:
                    max_udb      = BIT_8-1;
                    max_da_security_level  = 0x7;   /* 3 Bits */
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E:
                    max_udb      = BIT_5-1;
                    max_da_security_level  = 0x7;   /* 3 Bits */
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E:
                    max_udb      = BIT_8-1;  /* 12 bits */
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E:
                    max_udb      = BIT_5-1;  /* 12 bits */
                    break;
                default:
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_BAD_PARAM,
                            "bad_param_check_fdbManagerEntryAdd_ipv4Mc: bad IP MC muxing mode [%d]",
                            global_ipMcEntryMuxingMode);
            }
        }
        else if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E)
        {
            max_sourceID            = 0;        /* 12 bits */
            max_da_security_level   = 0;        /* Like SIP6 */
        }
        else
        {
            max_sourceID           = 0;     /* 0 in case of AC5 multicast entry */
            max_udb                = 0;     /* 0 in case of AC5 multicast entry */
            max_da_security_level  = 0x7;   /* 3 Bits */
        }
        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
                fdbManagerId, &entry, &fdbEntryAddInfo,
                &ipMcAddrFormatPtr->daCommand,
                0/*min*/,
                0x4/*max*/,
                1/*step*/);
    }

    /**************************/
    /* start to check fields */
    /**************************/
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &ipMcAddrFormatPtr->fid,
        0/*min*/,
        FID_MASK_AS_PER_DEVICE()/*max*/,
        100/*step*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &ipMcAddrFormatPtr->daSecurityLevel,
        0/*min*/,
        max_da_security_level/*max*/,
        1/*step*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &ipMcAddrFormatPtr->userDefined,
        0/*min*/,
        max_udb/*max*/,
        1/*step*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &ipMcAddrFormatPtr->sourceID,
        0/*min*/,
        max_sourceID/*max*/,
        1/*step*/);

    /* we not check GT_BOOL ! */
    /*
        fdbEntryMacAddrFormatPtr->isStatic        = GT_FALSE;
        fdbEntryMacAddrFormatPtr->age             = GT_TRUE;
        fdbEntryMacAddrFormatPtr->daRoute         = GT_FALSE;
        fdbEntryMacAddrFormatPtr->appSpecificCpuCode = GT_TRUE;
    */
}

static void bad_param_check_fdbManagerEntryAdd_ipv4Mc(
    IN GT_U32                                               fdbManagerId
)
{
    bad_param_check_fdbManagerEntryAdd_ipvMc(fdbManagerId, GT_TRUE);
}

static void bad_param_check_fdbManagerEntryAdd_ipv6Mc(
    IN GT_U32                                               fdbManagerId
)
{
    bad_param_check_fdbManagerEntryAdd_ipvMc(fdbManagerId, GT_FALSE);
}

static void bad_param_check_common_fdbManagerEntryAdd_ipvxUc(
    IN GT_U32   fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                          * entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC               * fdbEntryAddInfoPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT                * ucRouteTypePtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT  * ucRouteInfoPtr
)
{
    GT_U32  maxValue;
    CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT shadowType = SHADOW_TYPE_PER_DEVICE();

    cpssOsMemSet(ucRouteInfoPtr, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT));
    /**************************/
    /* start to check fields */
    /**************************/
    /* check union value */
    *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
    ucRouteInfoPtr->fullFdbInfo.dstInterface.type = CPSS_INTERFACE_PORT_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        &ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.hwDevNum,
        0/*min*/,
        (BIT_10-1)/*max*/,
        BIT_10/5/*step*/);

    if((shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E) ||
        (shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E))
    {
        maxValue = BIT_13;
    }
    else
    {
        maxValue = BIT_14;
    }
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        &ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum,
        0/*min*/,
        (maxValue-1)/*max*/,
        maxValue/5/*step*/);

    /* check union value */
    ucRouteInfoPtr->fullFdbInfo.dstInterface.type = CPSS_INTERFACE_TRUNK_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_U16_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        &ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.trunkId,
        0/*min*/,
        (BIT_12-1)/*max*/,
        BIT_12/5/*step*/);

    /* check union value */
    /* In case SIP5 no invalid value, complete 16 bits are valid */
    if(shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)
    {
        ucRouteInfoPtr->fullFdbInfo.dstInterface.type = CPSS_INTERFACE_VIDX_E;
        CHECK_BOUNDERS_NUMERIC_FIELD_U16_cpssDxChBrgFdbManagerEntryAdd(
                fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
                &ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.vidx,
                0/*min*/,
                (BIT_14-1)/*max*/,
                BIT_14/5/*step*/);
    }

    /* check union value */
    *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        &ucRouteInfoPtr->fullFdbInfo.nextHopVlanId,
        0/*min*/,
        (BIT_13-1)/*max*/,
        BIT_13/5/*step*/);

    maxValue = (shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)?BIT_3:BIT_1;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        &ucRouteInfoPtr->fullFdbInfo.mtuProfileIndex,
        0/*min*/,
        (maxValue-1)/*max*/,
        1/*step*/);

    maxValue = ((shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E) || (shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E))?BIT_17:BIT_18;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        &ucRouteInfoPtr->fullFdbInfo.pointer.arpPointer,
        0/*min*/,
        (maxValue-1)/*max*/,
        maxValue/5/*step*/);

    if((shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E) &&
        (shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E))
    {
        /* check union value */
        *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E;
        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
                fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
                &ucRouteInfoPtr->fullFdbInfo.pointer.natPointer,
                0/*min*/,
                (BIT_16-1)/*max*/,
                BIT_16/5/*step*/);

        /* check union value */
        *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E;
        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
                fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
                &ucRouteInfoPtr->nextHopPointerToRouter,
                0/*min*/,
                (BIT_15-1)/*max*/,
                BIT_15/5/*step*/);

        *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E;
        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
                fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
                &ucRouteInfoPtr->ecmpPointerToRouter,
                0/*min*/,
                (BIT_15-1)/*max*/,
                BIT_15/5/*step*/);
    }

    /* check union value */
    maxValue = ((shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E) ||
            (shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E))?BIT_15:BIT_16;
    *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        &ucRouteInfoPtr->fullFdbInfo.pointer.tunnelStartPointer,
        0/*min*/,
        (maxValue-1)/*max*/,
        maxValue/5/*step*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        (GT_U32 *)ucRouteTypePtr,
        0/*min - don't care*/,
        0/*max - don't care*/,
        0/*step - don't care*/);

    *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        (GT_U32 *)&ucRouteInfoPtr->fullFdbInfo.dstInterface.type,
        0/*min - don't care*/,
        0/*max - don't care*/,
        0/*step - don't care*/);

    *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        (GT_U32 *)&ucRouteInfoPtr->fullFdbInfo.countSet,
        0/*min - don't care*/,
        0/*max - don't care*/,
        0/*step - don't care*/);
}


static void bad_param_check_common_fdbManagerEntryUpdate_ipvxUc(
    IN GT_U32   fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                          * entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC            * paramsPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT                * ucRouteTypePtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT  * ucRouteInfoPtr
)
{
    GT_U32  maxValue;
    cpssOsMemSet(ucRouteInfoPtr, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT));

    /**************************/
    /* start to check fields */
    /**************************/
    /* check union value */
    *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E;
    ucRouteInfoPtr->fullFdbInfo.dstInterface.type = CPSS_INTERFACE_PORT_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
        fdbManagerId, entryPtr, paramsPtr,
        &ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.hwDevNum,
        0/*min*/,
        (BIT_10-1)/*max*/,
        BIT_10/5/*step*/);

    maxValue = (IS_SIP5_OR_HYBRID)?BIT_13:BIT_14;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
        fdbManagerId, entryPtr, paramsPtr,
        &ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum,
        0/*min*/,
        (maxValue-1)/*max*/,
        maxValue/5/*step*/);

    /* check union value */
    ucRouteInfoPtr->fullFdbInfo.dstInterface.type = CPSS_INTERFACE_TRUNK_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_U16_cpssDxChBrgFdbManagerEntryUpdate(
        fdbManagerId, entryPtr, paramsPtr,
        &ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.trunkId,
        0/*min*/,
        (BIT_12-1)/*max*/,
        BIT_12/5/*step*/);

    /* check union value */
    /* In case SIP5 no invalid value, complete 16 bits are valid */
    if(SHADOW_TYPE_PER_DEVICE() != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)
    {
        ucRouteInfoPtr->fullFdbInfo.dstInterface.type = CPSS_INTERFACE_VIDX_E;
        CHECK_BOUNDERS_NUMERIC_FIELD_U16_cpssDxChBrgFdbManagerEntryUpdate(
                fdbManagerId, entryPtr, paramsPtr,
                &ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.vidx,
                0/*min*/,
                (BIT_14-1)/*max*/,
                BIT_14/5/*step*/);
    }

    /* check union value */
    *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
        fdbManagerId, entryPtr, paramsPtr,
        &ucRouteInfoPtr->fullFdbInfo.nextHopVlanId,
        0/*min*/,
        (BIT_13-1)/*max*/,
        BIT_13/5/*step*/);

    maxValue = (SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)?BIT_3:BIT_1;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
        fdbManagerId, entryPtr, paramsPtr,
        &ucRouteInfoPtr->fullFdbInfo.mtuProfileIndex,
        0/*min*/,
        (maxValue-1)/*max*/,
        1/*step*/);

    maxValue = (IS_SIP5_OR_HYBRID)?BIT_17:BIT_18;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
        fdbManagerId, entryPtr, paramsPtr,
        &ucRouteInfoPtr->fullFdbInfo.pointer.arpPointer,
        0/*min*/,
        (maxValue-1)/*max*/,
        maxValue/5/*step*/);


    /* check union value */
    maxValue = (IS_SIP5_OR_HYBRID)?BIT_15:BIT_16;
    *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
        fdbManagerId, entryPtr, paramsPtr,
        &ucRouteInfoPtr->fullFdbInfo.pointer.tunnelStartPointer,
        0/*min*/,
        (maxValue-1)/*max*/,
        maxValue/5/*step*/);

    if(!(IS_SIP5_OR_HYBRID))
    {
        /* check union value */
        *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E;
        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
                fdbManagerId, entryPtr, paramsPtr,
                &ucRouteInfoPtr->fullFdbInfo.pointer.natPointer,
                0/*min*/,
                (BIT_16-1)/*max*/,
                BIT_16/5/*step*/);

        /* check union value */
        *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E;
        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
                fdbManagerId, entryPtr, paramsPtr,
                &ucRouteInfoPtr->nextHopPointerToRouter,
                0/*min*/,
                (BIT_15-1)/*max*/,
                BIT_15/5/*step*/);

        *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E;
        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
                fdbManagerId, entryPtr, paramsPtr,
                &ucRouteInfoPtr->ecmpPointerToRouter,
                0/*min*/,
                (BIT_15-1)/*max*/,
                BIT_15/5/*step*/);
    }

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
        fdbManagerId, entryPtr, paramsPtr,
        (GT_U32 *)ucRouteTypePtr,
        0/*min - don't care*/,
        0/*max - don't care*/,
        0/*step - don't care*/);

    *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
        fdbManagerId, entryPtr, paramsPtr,
        (GT_U32 *)&ucRouteInfoPtr->fullFdbInfo.dstInterface.type,
        0/*min - don't care*/,
        0/*max - don't care*/,
        0/*step - don't care*/);

    *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryUpdate(
        fdbManagerId, entryPtr, paramsPtr,
        (GT_U32 *)&ucRouteInfoPtr->fullFdbInfo.countSet,
        0/*min - don't care*/,
        0/*max - don't care*/,
        0/*step - don't care*/);
}

static void bad_param_check_fdbManagerEntryAdd_ipv4Uc(
    IN GT_U32                                               fdbManagerId
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC *ipv4UcAddrFormatPtr = &entry.format.fdbEntryIpv4UcFormat;

    /* build 'valid' entry : IP UC addr */
    fdbManagerEntryBuild_ipv4UcAddr(&entry, 0);

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, &entry, &fdbEntryAddInfo, st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerEntryAdd: unexpected GT_OK [%d]",
        fdbManagerId);

    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId, &entry, st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerEntryDelete: unexpected GT_OK [%d]",
        fdbManagerId);

    /**************************/
    /* start to check fields */
    /**************************/
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &ipv4UcAddrFormatPtr->vrfId,
        0/*min*/,
        (BIT_12-1)/*max*/,
        BIT_12/5/*step*/);

    bad_param_check_common_fdbManagerEntryAdd_ipvxUc(fdbManagerId,
                                                     &entry, &fdbEntryAddInfo,
                                                     &ipv4UcAddrFormatPtr->ucRouteType,
                                                     &ipv4UcAddrFormatPtr->ucRouteInfo);
}

static void bad_param_check_fdbManagerEntryAdd_ipv6Uc(
    IN GT_U32                                               fdbManagerId
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC *ipv6UcAddrFormatPtr = &entry.format.fdbEntryIpv6UcFormat;

    /* build 'valid' entry : IP UC addr */
    fdbManagerEntryBuild_ipv6UcAddr(&entry, 0);

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, &entry, &fdbEntryAddInfo, st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerEntryAdd: unexpected GT_OK [%d]",
        fdbManagerId);

    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId, &entry, st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerEntryDelete: unexpected GT_OK [%d]",
        fdbManagerId);

    /**************************/
    /* start to check fields */
    /**************************/
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryAdd(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &ipv6UcAddrFormatPtr->vrfId,
        0/*min*/,
        (BIT_12-1)/*max*/,
        BIT_12/5/*step*/);

    bad_param_check_common_fdbManagerEntryAdd_ipvxUc(fdbManagerId,
                                                     &entry, &fdbEntryAddInfo,
                                                     &ipv6UcAddrFormatPtr->ucRouteType,
                                                     &ipv6UcAddrFormatPtr->ucRouteInfo);
}

static void bad_param_check_fdbManagerEntryUpdate_ipv6Mc(
    IN GT_U32   fdbManagerId
)
{
    bad_param_check_fdbManagerEntryUpdate_ipvMc(fdbManagerId, GT_FALSE);
}

static void bad_param_check_fdbManagerEntryUpdate_ipv4Uc(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC params;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC *ipv4UcAddrFormatPtr = &entry.format.fdbEntryIpv4UcFormat;

    /* build 'valid' entry : IP UC addr */
    fdbManagerEntryBuild_ipv4UcAddr(&entry, 0);

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, &entry, &fdbEntryAddInfo, st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryAdd: unexpected GT_OK [%d]",
        fdbManagerId);

    /**************************/
    /* start to check fields */
    /**************************/
    bad_param_check_common_fdbManagerEntryUpdate_ipvxUc(fdbManagerId, &entry, &params,
                                                        &ipv4UcAddrFormatPtr->ucRouteType,
                                                        &ipv4UcAddrFormatPtr->ucRouteInfo);

    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId, &entry, st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryDelete: unexpected GT_OK [%d]",
        fdbManagerId);
}

static void bad_param_check_fdbManagerEntryUpdate_ipv6Uc(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC params;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC *ipv6UcAddrFormatPtr = &entry.format.fdbEntryIpv6UcFormat;

    /* build 'valid' entry : IP UC addr */
    fdbManagerEntryBuild_ipv6UcAddr(&entry, 0);

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, &entry, &fdbEntryAddInfo, st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryAdd: unexpected GT_OK [%d]",
        fdbManagerId);

    /**************************/
    /* start to check fields */
    /**************************/
    bad_param_check_common_fdbManagerEntryUpdate_ipvxUc(fdbManagerId, &entry, &params,
                                                        &ipv6UcAddrFormatPtr->ucRouteType,
                                                        &ipv6UcAddrFormatPtr->ucRouteInfo);

    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId, &entry, st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryDelete: unexpected GT_OK [%d]",
        fdbManagerId);
}


/**
GT_STATUS cpssDxChBrgFdbManagerEntryAdd
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr
);
*/
static void internal_cpssDxChBrgFdbManagerEntryAddUT(void)
{
    GT_STATUS                                               st;
    GT_U32                                                  ii;
    GT_U32                                                  numEntriesRemoved;/* total number of entries removed from the DB */
    GT_U32                                                  numEntriesAdded;  /* total number of entries added to the DB */
    GT_BOOL                                                 entryAdded;/* indication that the entry was added to the DB */
    GT_BOOL                                                 getFirst;  /* indication to get first entry from the DB */
    GT_U32                                                  fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     entry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC          params;
    GT_U32                                                  maxTotalEntries;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT                fdbEntryType;
    UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC        outputResults;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNum, errorNumItr;

    cpssOsMemSet(&entry ,0,sizeof(entry));
    cpssOsMemSet(&params,0,sizeof(params));

    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;

    PRV_UTF_LOG0_MAC("Check non-initialized managers \n");

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(ii,&entry,&params,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChBrgFdbManagerEntryAdd: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    PRV_UTF_LOG0_MAC("Check check out of range managers \n");
    /* check out of range managers ... expect 'GT_BAD_PARAM' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(ii,&entry,&params,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerEntryAdd: expected to GT_BAD_PARAM on manager [%d]",
            ii);
    }

    numEntriesAdded = 0;
    maxTotalEntries = FDB_SIZE_FULL;

    if(debug_limited_entries)
    {
        maxTotalEntries = debug_limited_entries;
    }

    if(run_device_less == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("create manager with device \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("create manager without device \n");
    }
    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId,maxTotalEntries,prvTgfDevNum, NULL, GT_FALSE);

    PRV_UTF_LOG1_MAC("start - add [%d] entries \n",maxTotalEntries);
    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);

    for(ii = 0 ; ii < maxTotalEntries; ii++)
    {
        entryAdded = GT_FALSE;

        fdbEntryType = fdbEntryType_valid_arr[ii % UT_FDB_ENTRY_TYPE_VALID_NUM];

        switch(fdbEntryType)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
                fdbManagerEntryAdd_macAddr(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
                fdbManagerEntryAdd_ipv4Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
                fdbManagerEntryAdd_ipv6Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
                fdbManagerEntryAdd_ipv4Uc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
                fdbManagerEntryAdd_ipv6Uc(fdbManagerId, ii, &outputResults);
                break;
            default:
                break;
        }

        if(entryAdded)
        {
            numEntriesAdded++;
        }

        if(add_device_in_runtime == GT_TRUE &&
           (ii == (maxTotalEntries/2)))
        {
            GT_U8  devListArr[1];
            GT_U32 numOfDevs = 1;

            /* now in the middle of the insertions we add the device. */
            PRV_UTF_LOG0_MAC("now in the middle of the insertions we add the device \n");

            devListArr[0] = prvTgfDevNum;

            /* add device to it */
            st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
                fdbManagerId);
        }
    }

    PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
        maxTotalEntries,numEntriesAdded,
            (100*numEntriesAdded)/maxTotalEntries);

    if(debug_limited_entries)
    {
        cpssDxChFdbDump(prvTgfDevNum);
    }

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);

    /* Verify DB */
    st = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    numEntriesRemoved = 0;
    getFirst = GT_TRUE;

    PRV_UTF_LOG1_MAC("start - remove the [%d] entries \n",numEntriesAdded);
    if(check_hw_hold_entry == GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("(slow mode) for each entry check that HW hold the entry in proper index according to it's index \n");
    }
    /* remove all entries from the DB (one by one) */
    while(GT_OK == cpssDxChBrgFdbManagerEntryGetNext(fdbManagerId,getFirst,&entry))
    {
        getFirst = GT_FALSE;

        if(check_hw_hold_entry == GT_TRUE)
        {
            if (check_hw_hold_entry_all_tiles)
            {
                /* just before we going to remove the entry ... lets check that it exists in all tiles the HW ! */
                fdbManagerHwEntryCheckAllTiles(&entry, numEntriesRemoved);
            }
            else
            {
                /* just before we going to remove the entry ... lets check that it exists in the HW ! */
                fdbManagerHwEntryCheck(&entry);
            }
        }

        FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId,&entry,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryDelete: expected to GT_OK on manager [%d]",
            fdbManagerId);
        numEntriesRemoved++;

        if((numEntriesRemoved % 10000) == 0)
        {
            PRV_UTF_LOG1_MAC("removed [%d] entries \n",numEntriesRemoved);
        }
    }

    PRV_UTF_LOG2_MAC("ended - removed [%d] entries (out of expected [%d]) \n",numEntriesRemoved,numEntriesAdded);

    UTF_VERIFY_EQUAL2_STRING_MAC(numEntriesAdded, numEntriesRemoved,
        "expected number of removed entries [%d] to be the same as the added entries[%d]",
        numEntriesRemoved,numEntriesAdded);

    /* Verify DB */
    st = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    /* check invalid parameters */
    for(ii = 0 ; ii < UT_FDB_ENTRY_TYPE_VALID_NUM; ii++)
    {
        fdbEntryType = fdbEntryType_valid_arr[ii % UT_FDB_ENTRY_TYPE_VALID_NUM];

        switch(fdbEntryType)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : mac addr \n");
                bad_param_check_fdbManagerEntryAdd_macAddr(fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : ipv4mc \n");
                bad_param_check_fdbManagerEntryAdd_ipv4Mc (fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : ipv6mc \n");
                bad_param_check_fdbManagerEntryAdd_ipv6Mc (fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : ipv4uc \n");
                bad_param_check_fdbManagerEntryAdd_ipv4Uc (fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : ipv6uc \n");
                bad_param_check_fdbManagerEntryAdd_ipv6Uc (fdbManagerId);
                break;
            default:
                break;
        }
    }

    PRV_UTF_LOG0_MAC("check NULL pointers \n");
    /* NULL pointer */
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId,NULL,&params,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerEntryAdd: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);

    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId,&entry,NULL,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerEntryAdd: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);

    /* Verify DB */
    st = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    PRV_UTF_LOG0_MAC("Ended \n");
    return;
}
/**
GT_STATUS cpssDxChBrgFdbManagerEntryAdd
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerEntryAdd)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    check_hw_hold_entry = GT_TRUE;/* very slow mode ! */
    force_single_bank_to_add = BANK_AS_PER_DEVICE(9); /* force using this bank as 'first choice' */

    internal_cpssDxChBrgFdbManagerEntryAddUT();

    check_hw_hold_entry = GT_FALSE;
    force_single_bank_to_add = 0;/* restore */

    CLEANUP_ALL_MANAGERS;
}

/* run the test of 'cpssDxChBrgFdbManagerEntryAdd' , but without device bound to the manager */
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerEntryAdd_run_device_less)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    run_device_less = GT_TRUE;

    internal_cpssDxChBrgFdbManagerEntryAddUT();

    run_device_less = GT_FALSE;

    CLEANUP_ALL_MANAGERS;
}
/* run the test of 'cpssDxChBrgFdbManagerEntryAdd' , but the device added to the
   manager after 1/2 table already full */
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerEntryAdd_add_device_in_runtime)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    run_device_less = GT_TRUE;
    add_device_in_runtime = GT_TRUE;

    internal_cpssDxChBrgFdbManagerEntryAddUT();

    run_device_less = GT_FALSE;
    add_device_in_runtime = GT_FALSE;

    CLEANUP_ALL_MANAGERS;
}

/* run the test of 'cpssDxChBrgFdbManagerEntryAdd_add_device_in_runtime' ,
    but the manager will be created with support of global eports */
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerEntryAdd_add_device_in_runtime_withGlobalEportsFullRange)
{
    GT_BOOL orig_run_with_dynamic_uc_limits = run_with_dynamic_uc_limits;
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)
    {
        /* no need to run the cpssDxChBrgFdbManagerEntryAdd_add_device_in_runtime
           again for manager that not supports global eports */
        SKIP_TEST_MAC;
    }

    run_with_global_eports_full_range = GT_TRUE;
    run_with_dynamic_uc_limits        = GT_TRUE;

    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryAdd_add_device_in_runtime);

    run_with_global_eports_full_range = GT_FALSE;
    run_with_dynamic_uc_limits        = orig_run_with_dynamic_uc_limits;
}

/* update fdb entry in the manager : type : mac addr */
static void fdbManagerEntryUpdate_macAddr(
    IN GT_U32   fdbManagerId,
    IN GT_U32   iterationNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC *entryPtr,
    IN GT_BOOL  *entryUpfdatedPtr,
    IN GT_BOOL  *wasDeletedOnLimitPtr
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                  entryGet;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC    fdbEntryUpdateInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC *fdbEntryMacAddrFormatPtr;
    GT_BOOL                                     checkCounters;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC      counters, counters1;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC    statistics, statistics1;
    GT_U32                                      *statisticValuePtr = (GT_U32 *)&statistics;
    GT_U32                                      *statisticValuePtr1 =(GT_U32 *)&statistics1;
    GT_U32                                      ii, maxStatistics;
    GT_U32                                      rand32Bits = cpssOsRand() | iterationNum << 16;

    maxStatistics = sizeof(CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC) / 4;

    *entryUpfdatedPtr = GT_FALSE;
    *wasDeletedOnLimitPtr = GT_FALSE;
    fdbEntryMacAddrFormatPtr = &entryPtr->format.fdbEntryMacAddrFormat;
    fdbEntryUpdateInfo.updateOnlySrcInterface = (iterationNum % 2) ? GT_TRUE : GT_FALSE;

    checkCounters = GT_FALSE;
    /* Override dynamic entry with static */
    if (fdbEntryMacAddrFormatPtr->isStatic == GT_FALSE &&
        fdbEntryUpdateInfo.updateOnlySrcInterface == GT_FALSE)
    {
        fdbEntryMacAddrFormatPtr->isStatic = GT_TRUE;

        checkCounters = GT_TRUE;
        st = cpssDxChBrgFdbManagerCountersGet(fdbManagerId, &counters);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
            "cpssDxChBrgFdbManagerCountersGet: expected GT_OK in manager [%d]",
            fdbManagerId);

        st = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
            "cpssDxChBrgFdbManagerStatisticsGet: expected GT_OK in manager [%d]",
            fdbManagerId);
    }

    /* muxed fields */
    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        switch(global_macEntryMuxingMode)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E:
                fdbEntryMacAddrFormatPtr->sourceID          = (fdbEntryMacAddrFormatPtr->sourceID + 10) & 0xFFF; /* 12 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E:
                fdbEntryMacAddrFormatPtr->vid1              = (fdbEntryMacAddrFormatPtr->vid1 + 10) & 0xFFF; /* 12 bits */
                break;
            default:
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,GT_NOT_IMPLEMENTED,
                        "fdbManagerEntryAdd_macAddr: not implemented case in manager [%d]",
                        fdbManagerId);
        }
    }
    else
    {
        if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
        {
            switch(global_entryMuxingMode)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E:
                    fdbEntryMacAddrFormatPtr->sourceID           = cpssOsRand() & 0x1FF; /* 9 bits */
                    fdbEntryMacAddrFormatPtr->vid1               = 0;                    /* 0 bits */
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E:
                    fdbEntryMacAddrFormatPtr->sourceID           = cpssOsRand() & 0xFFF; /* 12 bits */
                    fdbEntryMacAddrFormatPtr->vid1               = 0;                    /* 0 bits */
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E:
                    fdbEntryMacAddrFormatPtr->sourceID           = cpssOsRand() & 0x3F;  /* 6 bits */
                    fdbEntryMacAddrFormatPtr->vid1               = cpssOsRand() & 0xFFF; /* 12 bits */
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E:
                    fdbEntryMacAddrFormatPtr->sourceID           = cpssOsRand() & 0x3F;  /* 6 bits */
                    fdbEntryMacAddrFormatPtr->vid1               = cpssOsRand() & 0xFFF; /* 12 bits */
                    break;
                default:
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,GT_NOT_IMPLEMENTED,
                            "fdbManagerEntryAdd_macAddr: not implemented case in manager [%d]",
                            fdbManagerId);
            }
        }
        else
        {

            fdbEntryMacAddrFormatPtr->sourceID  = 0x1F;  /* 5 bits */
            fdbEntryMacAddrFormatPtr->vid1      = 0;     /* Not supported */
        }
    }

    fdbManagerEntryBuild_DstInterfaceInfo(rand32Bits, &fdbEntryMacAddrFormatPtr->dstInterface);

    FDB_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(fdbManagerId, entryPtr, &fdbEntryUpdateInfo, st);
    if(run_with_dynamic_uc_limits == GT_TRUE)
    {
        switch(st)
        {
            case GT_LEARN_LIMIT_PORT_ERROR          :
            case GT_LEARN_LIMIT_TRUNK_ERROR         :
            case GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR  :
            case GT_LEARN_LIMIT_FID_ERROR           :
            case GT_LEARN_LIMIT_GLOBAL_ERROR        :
                /* the entry was deleted ! so we can't do more operations on this entry */
                /* and it is not error */
                *wasDeletedOnLimitPtr = GT_TRUE;
                break;
            default:
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbManagerEntryUpdate: unexpected error in manager [%d]",
                    fdbManagerId);
                break;
        }
    }
    else
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryUpdate: unexpected error in manager [%d]",
            fdbManagerId);
    }

    if(st == GT_OK)
    {
        /* the entry was upated ... check if the entry retrieved as was set */
        if(debug_limited_entries)
        {
            cpssDxChFdbDump(prvTgfDevNum);
        }

        /* start -- the parts of the key */
        cpssOsMemCpy(&entryGet.format.fdbEntryMacAddrFormat.macAddr,
                     &fdbEntryMacAddrFormatPtr->macAddr,
                     sizeof(fdbEntryMacAddrFormatPtr->macAddr));
        entryGet.format.fdbEntryMacAddrFormat.fid = fdbEntryMacAddrFormatPtr->fid;
        entryGet.format.fdbEntryMacAddrFormat.vid1 = fdbEntryMacAddrFormatPtr->vid1;
        /* end -- the parts of the key */
        entryGet.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;

        st = cpssDxChBrgFdbManagerEntryGet(fdbManagerId, &entryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
            "cpssDxChBrgFdbManagerEntryGet: expected GT_OK in manager [%d]",
            fdbManagerId);

        /***************************************************************/
        /* compare the full entry : entry type , fdbEntryMacAddrFormat */
        /***************************************************************/
        UTF_VERIFY_EQUAL1_STRING_MAC(entryPtr->fdbEntryType, entryGet.fdbEntryType,
            "cpssDxChBrgFdbManagerEntryGet: expected fdbEntryType in 'get' entry to match 'set' in manager [%d]",
            fdbManagerId);

                /* Check updated entry according to updating parameter 'update only source interface */
        if (fdbEntryUpdateInfo.updateOnlySrcInterface == GT_FALSE)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(
                                                &entryGet.format.fdbEntryMacAddrFormat,
                                                &entryPtr->format.fdbEntryMacAddrFormat,
                                                sizeof(entryGet.format.fdbEntryMacAddrFormat)),
                "cpssDxChBrgFdbManagerEntryGet: expected 'get' entry to match 'set' entry in manager [%d]",
                fdbManagerId);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(
                                                &entryGet.format.fdbEntryMacAddrFormat,
                                                &entryPtr->format.fdbEntryMacAddrFormat,
                                                sizeof(entryGet.format.fdbEntryMacAddrFormat)),
                "cpssDxChBrgFdbManagerEntryGet: expected 'get' entry to match 'set' entry in manager [%d]",
                fdbManagerId);

            UTF_VERIFY_EQUAL1_STRING_MAC(entryGet.format.fdbEntryMacAddrFormat.sourceID,
                                         entryPtr->format.fdbEntryMacAddrFormat.sourceID,
                "cpssDxChBrgFdbManagerEntryGet: expected 'get' entry to match 'set' entry in manager [%d]",
                fdbManagerId);

        }

        if (checkCounters == GT_TRUE)
        {
            st = cpssDxChBrgFdbManagerCountersGet(fdbManagerId, &counters1);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
                "cpssDxChBrgFdbManagerCountersGet: expected GT_OK in manager [%d]",
                fdbManagerId);

            UTF_VERIFY_EQUAL1_STRING_MAC(1, counters.macUnicastDynamic - counters1.macUnicastDynamic,
                                         "cpssDxChBrgFdbManagerCountersGet: expected 'get' entry not equal 'set' entry in manager [%d]",
                                         fdbManagerId);

            UTF_VERIFY_EQUAL1_STRING_MAC(1, counters1.macUnicastStatic - counters.macUnicastStatic,
                                         "cpssDxChBrgFdbManagerCountersGet: expected 'get' entry not equal 'set' entry in manager [%d]",
                                         fdbManagerId);

            st = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics1);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
                "cpssDxChBrgFdbManagerStatisticsGet: expected GT_OK in manager [%d]",
                fdbManagerId);

            for (ii = 0; ii < maxStatistics; ii++)
            {

                if (&statisticValuePtr[ii] == &statistics.entryUpdateOk)
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(1, statistics1.entryUpdateOk - statistics.entryUpdateOk,
                                                 "cpssDxChBrgFdbManagerStatisticsGet: expected 'get' entry not equal 'set' entry in manager [%d]",
                                                 fdbManagerId);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(0, statisticValuePtr1[ii] - statisticValuePtr[ii],
                                                 "cpssDxChBrgFdbManagerStatisticsGet: expected 'get' entry not equal 'set' entry in manager [%d]",
                                                 fdbManagerId);
                }
            }

        }

        *entryUpfdatedPtr = GT_TRUE;
    }
}

/* update fdb entry in the manager : type : IPMC ipv4/ipv6 */
static void fdbManagerEntryUpdate_ipvMc(
    IN GT_U32   fdbManagerId,
    IN GT_BOOL  isIpv4,
    IN GT_U32   iterationNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC *entryPtr,
    IN GT_BOOL  *entryUpfdatedPtr
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                         entryGet;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC    fdbEntryUpdateInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC            *fdbEntryFormatPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC            *fdbEntryIpMcAddrFormatGetPtr;

    cpssOsMemSet(&entryGet, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));

    *entryUpfdatedPtr = GT_FALSE;

    if (isIpv4)
    {
        entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E;
        entryGet.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E;
        fdbEntryFormatPtr = &entryPtr->format.fdbEntryIpv4McFormat;
        fdbEntryIpMcAddrFormatGetPtr = &entryGet.format.fdbEntryIpv4McFormat;
    }
    else
    {
        entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E;
        entryGet.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E;
        fdbEntryFormatPtr = &entryPtr->format.fdbEntryIpv6McFormat;
        fdbEntryIpMcAddrFormatGetPtr = &entryGet.format.fdbEntryIpv6McFormat;
    }

    fdbEntryUpdateInfo.updateOnlySrcInterface = (iterationNum % 2) ? GT_TRUE : GT_FALSE;

    /* muxed fields */
    if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
    {
        switch(global_ipMcEntryMuxingMode)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E:
                fdbEntryFormatPtr->sourceID         = (fdbEntryFormatPtr->sourceID + 10) & 0x7F; /* 7 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_4_SRC_ID_3_E:
                fdbEntryFormatPtr->userDefined      = (fdbEntryFormatPtr->userDefined + 10) & 0xF;  /* 4 bits */
                fdbEntryFormatPtr->sourceID         = (fdbEntryFormatPtr->sourceID + 10) & 0x7;  /* 3 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_E:
                fdbEntryFormatPtr->userDefined      = (fdbEntryFormatPtr->userDefined + 10) & 0x7F;  /* 7 bits */
                break;
            default:
                return;
        }
        fdbEntryFormatPtr->dstInterface.interfaceInfo.vidx  = (fdbEntryFormatPtr->dstInterface.interfaceInfo.vidx + 10) & 0x3FFF; /* 14 Bits */
    }
    else if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)
    {
        fdbEntryFormatPtr->sourceID = 0; /* source ID 0, for IPMC in SIP5 */
        switch(global_entryMuxingMode)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E:
                fdbEntryFormatPtr->userDefined      = (fdbEntryFormatPtr->userDefined + 10) & 0xFF;  /* 8 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E:
                fdbEntryFormatPtr->userDefined      = (fdbEntryFormatPtr->userDefined + 10) & 0x1F;  /* 5 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E:
                fdbEntryFormatPtr->userDefined      = (fdbEntryFormatPtr->userDefined + 10) & 0xFF;  /* 8 bits */
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E:
                fdbEntryFormatPtr->userDefined      = (fdbEntryFormatPtr->userDefined + 10) & 0x1F;  /* 5 bits */
                break;
            default:
                return;
        }
        fdbEntryFormatPtr->dstInterface.interfaceInfo.vidx  = (fdbEntryFormatPtr->dstInterface.interfaceInfo.vidx + 10) & 0x3FFF; /* 14 Bits */
    }
    else
    {
        fdbEntryFormatPtr->sourceID         = 0;
        fdbEntryFormatPtr->userDefined      = 0;
        fdbEntryFormatPtr->dstInterface.interfaceInfo.vidx  = (fdbEntryFormatPtr->dstInterface.interfaceInfo.vidx + 10) & 0xFFF; /* 12 Bits */
    }
    fdbEntryFormatPtr->daCommand                        = (fdbEntryFormatPtr->daCommand + 10) & 0x3;   /* 2 bits */

    FDB_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(fdbManagerId, entryPtr, &fdbEntryUpdateInfo, st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerEntryUpdate: unexpected error in manager [%d]",
        fdbManagerId);

    if(st == GT_OK)
    {
        /* the entry was upated ... check if the entry retrieved as was set */
        if(debug_limited_entries)
        {
            cpssDxChFdbDump(prvTgfDevNum);
        }

        /* start -- the parts of the key */
        cpssOsMemCpy(&fdbEntryIpMcAddrFormatGetPtr->sipAddr,
                     &fdbEntryFormatPtr->sipAddr,
                     sizeof(fdbEntryFormatPtr->sipAddr));

        cpssOsMemCpy(&fdbEntryIpMcAddrFormatGetPtr->dipAddr,
                     &fdbEntryFormatPtr->dipAddr,
                     sizeof(fdbEntryFormatPtr->dipAddr));

        fdbEntryIpMcAddrFormatGetPtr->fid = fdbEntryFormatPtr->fid;
        /* end -- the parts of the key */

        st = cpssDxChBrgFdbManagerEntryGet(fdbManagerId,&entryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
            "cpssDxChBrgFdbManagerEntryGet: expected GT_OK in manager [%d]",
            fdbManagerId);

        /***************************************************************/
        /* compare the full entry : entry type , fdbEntryMacAddrFormat */
        /***************************************************************/
        UTF_VERIFY_EQUAL1_STRING_MAC(entryPtr->fdbEntryType, entryGet.fdbEntryType,
            "cpssDxChBrgFdbManagerEntryGet: expected fdbEntryType in 'get' entry to match 'set' in manager [%d]",
            fdbManagerId);

        /* Check updated entry according to updating parameter 'update only source interface */
        if (fdbEntryUpdateInfo.updateOnlySrcInterface == GT_FALSE)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(
                                                fdbEntryIpMcAddrFormatGetPtr,
                                                fdbEntryFormatPtr,
                                                sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC)),
                "cpssDxChBrgFdbManagerEntryGet: expected 'get' entry to match 'set' entry in manager [%d]",
                fdbManagerId);
        }
        else
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(0, cpssOsMemCmp(
                                                fdbEntryIpMcAddrFormatGetPtr,
                                                fdbEntryFormatPtr,
                                                sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC)),
                "cpssDxChBrgFdbManagerEntryGet: expected 'get' entry to match 'set' entry in manager [%d]",
                fdbManagerId);

            UTF_VERIFY_EQUAL1_STRING_MAC(fdbEntryIpMcAddrFormatGetPtr->sourceID,
                                         fdbEntryFormatPtr->sourceID,
                "cpssDxChBrgFdbManagerEntryGet: expected 'get' entry to match 'set' entry in manager [%d]",
                fdbManagerId);


        }

        *entryUpfdatedPtr = GT_TRUE;
    }
}

/* update fdb entry in the manager : type : IPMC ipv4 */
static void fdbManagerEntryUpdate_ipv4Mc(
    IN GT_U32   fdbManagerId,
    IN GT_U32   iterationNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC *entryPtr,
    IN GT_BOOL  *entryUpfdatedPtr
)
{
    fdbManagerEntryUpdate_ipvMc(fdbManagerId, GT_TRUE, iterationNum, entryPtr, entryUpfdatedPtr);
}

/* update fdb entry in the manager : type : IPMC ipv6 */
static void fdbManagerEntryUpdate_ipv6Mc(
    IN GT_U32   fdbManagerId,
    IN GT_U32   iterationNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC *entryPtr,
    IN GT_BOOL  *entryUpfdatedPtr
)
{
    fdbManagerEntryUpdate_ipvMc(fdbManagerId, GT_FALSE, iterationNum, entryPtr, entryUpfdatedPtr);
}

/* update fdb entry to the manager : type : IPMC ipv4 */
static void fdbManagerEntryUpdate_ipv4Uc(
    IN GT_U32   fdbManagerId,
    IN GT_U32   iterationNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC *entryPtr,
    IN GT_BOOL  *entryUpfdatedPtr
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                         entryGet;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC          *fdbEntryFormatPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC          *fdbEntryFormatGetPtr;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC     params;

    GT_U32 rand32Bits = cpssOsRand() | iterationNum << 16;

    cpssOsMemSet(&entryGet, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));

    *entryUpfdatedPtr = GT_FALSE;

    fdbEntryFormatPtr = &entryPtr->format.fdbEntryIpv4UcFormat;
    fdbEntryFormatGetPtr = &entryGet.format.fdbEntryIpv4UcFormat;


    if((fdbEntryFormatPtr->ucRouteType == CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E) ||
            (fdbEntryFormatPtr->ucRouteType == CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E) ||
            (fdbEntryFormatPtr->ucRouteType == CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E))
    {
        fdbManagerEntryBuild_DstInterfaceInfo(rand32Bits, &fdbEntryFormatPtr->ucRouteInfo.fullFdbInfo.dstInterface);
    }

    FDB_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(fdbManagerId, entryPtr, &params, st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerEntryUpdate: unexpected error in manager [%d]",
        fdbManagerId);

    if(st == GT_OK)
    {
        /* the entry was upated ... check if the entry retrieved as was set */
        if(debug_limited_entries)
        {
            cpssDxChFdbDump(prvTgfDevNum);
        }

        /* start -- the parts of the key */
        cpssOsMemCpy(&fdbEntryFormatGetPtr->ipv4Addr,
                     &fdbEntryFormatPtr->ipv4Addr,
                     sizeof(fdbEntryFormatPtr->ipv4Addr));

        fdbEntryFormatGetPtr->vrfId = fdbEntryFormatPtr->vrfId;
        /* end -- the parts of the key */
        entryGet.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E;

        st = cpssDxChBrgFdbManagerEntryGet(fdbManagerId,&entryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
            "cpssDxChBrgFdbManagerEntryGet: expected GT_OK in manager [%d]",
            fdbManagerId);

        /***************************************************************/
        /* compare the full entry : entry type , fdbEntryIpv4UcFormat  */
        /***************************************************************/
        UTF_VERIFY_EQUAL1_STRING_MAC(entryPtr->fdbEntryType, entryGet.fdbEntryType,
            "cpssDxChBrgFdbManagerEntryGet: expected fdbEntryType in 'get' entry to match 'set' in manager [%d]",
            fdbManagerId);

        UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(
                                            fdbEntryFormatGetPtr,
                                            fdbEntryFormatPtr,
                                            sizeof(CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC)),
            "cpssDxChBrgFdbManagerEntryGet: expected 'get' entry to match 'set' entry in manager [%d]",
            fdbManagerId);

        *entryUpfdatedPtr = GT_TRUE;
    }
}

/* update fdb entry to the manager : type : IPMC ipv6 */
static void fdbManagerEntryUpdate_ipv6Uc(
    IN GT_U32   fdbManagerId,
    IN GT_U32   iterationNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC *entryPtr,
    IN GT_BOOL  *entryUpfdatedPtr
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                         entryGet;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC          *fdbEntryFormatPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC          *fdbEntryFormatGetPtr;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC    params;

    GT_U32 rand32Bits = cpssOsRand() | iterationNum << 16;

    cpssOsMemSet(&entryGet, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));

    *entryUpfdatedPtr = GT_FALSE;

    fdbEntryFormatPtr = &entryPtr->format.fdbEntryIpv6UcFormat;
    fdbEntryFormatGetPtr = &entryGet.format.fdbEntryIpv6UcFormat;

    if((fdbEntryFormatPtr->ucRouteType == CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E) ||
            (fdbEntryFormatPtr->ucRouteType == CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E) ||
            (fdbEntryFormatPtr->ucRouteType == CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E))
    {
        fdbManagerEntryBuild_DstInterfaceInfo(rand32Bits, &fdbEntryFormatPtr->ucRouteInfo.fullFdbInfo.dstInterface);
    }

    FDB_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(fdbManagerId, entryPtr, &params, st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerEntryUpdate: unexpected error in manager [%d]",
        fdbManagerId);

    if(st == GT_OK)
    {
        /* the entry was upated ... check if the entry retrieved as was set */
        if(debug_limited_entries)
        {
            cpssDxChFdbDump(prvTgfDevNum);
        }

        /* start -- the parts of the key */
        cpssOsMemCpy(&fdbEntryFormatGetPtr->ipv6Addr,
                     &fdbEntryFormatPtr->ipv6Addr,
                     sizeof(fdbEntryFormatPtr->ipv6Addr));

        fdbEntryFormatGetPtr->vrfId = fdbEntryFormatPtr->vrfId;
        /* end -- the parts of the key */
        entryGet.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E;

        st = cpssDxChBrgFdbManagerEntryGet(fdbManagerId, &entryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
            "cpssDxChBrgFdbManagerEntryGet: expected GT_OK in manager [%d]",
            fdbManagerId);

        /***************************************************************/
        /* compare the full entry : entry type , fdbEntryIpv4UcFormat  */
        /***************************************************************/
        UTF_VERIFY_EQUAL1_STRING_MAC(entryPtr->fdbEntryType, entryGet.fdbEntryType,
            "cpssDxChBrgFdbManagerEntryGet: expected fdbEntryType in 'get' entry to match 'set' in manager [%d]",
            fdbManagerId);

        UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(
                                            fdbEntryFormatGetPtr,
                                            fdbEntryFormatPtr,
                                            sizeof(CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC)),
            "cpssDxChBrgFdbManagerEntryGet: expected 'get' entry to match 'set' entry in manager [%d]",
            fdbManagerId);

        *entryUpfdatedPtr = GT_TRUE;
    }
}

/**
GT_STATUS cpssDxChBrgFdbManagerEntryUpdate
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC *paramsPtr
);
*/
static void internal_cpssDxChBrgFdbManagerEntryUpdateUT(void)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_U32      numEntriesRemoved;/* total number of entries removed from the DB */
    GT_U32      numEntriesAdded;  /* total number of entries added to the DB */
    GT_U32      numEntriesUpdated;  /* total number of entries updated in the DB */
    GT_BOOL     entryAdded;/* indication that the entry was added to the DB */
    GT_BOOL     entryUpdated;/* indication that the entry was updated in the DB */
    GT_BOOL     wasDeletedOnLimit;/* indication that the entry was deleted during 'update' of entry as
                                    the entry is over one of the UC limits  in the DB */
    GT_BOOL     getFirst;  /* indication to get first entry from the DB */
    GT_U32      fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                 entry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC   params;
    GT_U32    maxTotalEntries;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT            fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E;
    UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC    outputResults;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                              errorNum, errorNumItr;

    cpssOsMemSet(&entry ,0,sizeof(entry));
    cpssOsMemSet(&params,0,sizeof(params));

    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;

    PRV_UTF_LOG0_MAC("Check non-initialized managers \n");

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        FDB_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(ii,&entry,&params,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChBrgFdbManagerEntryUpdate: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    PRV_UTF_LOG0_MAC("Check check out of range managers \n");
    /* check out of range managers ... expect 'GT_BAD_PARAM' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        FDB_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(ii,&entry,&params,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerEntryUpdate: expected to GT_BAD_PARAM on manager [%d]",
            ii);
    }

    numEntriesAdded = 0;
    maxTotalEntries = FDB_SIZE_FULL;

    if(debug_limited_entries)
    {
        maxTotalEntries = debug_limited_entries;
    }

    if(run_device_less == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("create manager with device \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("create manager without device \n");
    }
    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId,maxTotalEntries,prvTgfDevNum, NULL, GT_FALSE);

    PRV_UTF_LOG1_MAC("start - add [%d] entries \n",maxTotalEntries);
    for(ii = 0 ; ii < maxTotalEntries; ii++)
    {
        entryAdded = GT_FALSE;

        fdbEntryType = fdbEntryType_valid_arr[ii % UT_FDB_ENTRY_TYPE_VALID_NUM];
        switch(fdbEntryType)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
                fdbManagerEntryAdd_macAddr(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
                fdbManagerEntryAdd_ipv4Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
                fdbManagerEntryAdd_ipv6Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
                fdbManagerEntryAdd_ipv4Uc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
                fdbManagerEntryAdd_ipv6Uc(fdbManagerId, ii, &outputResults);
                break;
            default:
                break;
        }

        if(entryAdded)
        {
            numEntriesAdded++;
        }

        if(add_device_in_runtime == GT_TRUE &&
           (ii == (maxTotalEntries/2)))
        {
            GT_U8  devListArr[1];
            GT_U32 numOfDevs = 1;

            /* now in the middle of the insertions we add the device. */
            PRV_UTF_LOG0_MAC("now in the middle of the insertions we add the device \n");

            devListArr[0] = prvTgfDevNum;

            /* add device to it */
            st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
                fdbManagerId);
        }
    }
    PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
        maxTotalEntries,numEntriesAdded,
            (100*numEntriesAdded)/maxTotalEntries);

    if(debug_limited_entries)
    {
        cpssDxChFdbDump(prvTgfDevNum);
    }

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);
    /* Verify DB */
    st = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    numEntriesRemoved = 0;
    numEntriesUpdated = 0;
    getFirst = GT_TRUE;

    PRV_UTF_LOG1_MAC("start - remove the [%d] entries \n",numEntriesAdded);
    if(check_hw_hold_entry == GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("(slow mode) for each entry check that HW hold the entry in proper index according to it's index \n");
    }

    ii = 0;

    /* update all entries from the DB (one by one) */
    while(GT_OK == cpssDxChBrgFdbManagerEntryGetNext(fdbManagerId,getFirst,&entry))
    {
        getFirst = GT_FALSE;

        if(check_hw_hold_entry == GT_TRUE)
        {
            if (check_hw_hold_entry_all_tiles)
            {
                /* just before we going to remove the entry ... lets check that it exists in all tiles the HW ! */
                fdbManagerHwEntryCheckAllTiles(&entry, numEntriesUpdated);
            }
            else
            {
                /* just before we going to remove the entry ... lets check that it exists in the HW ! */
                fdbManagerHwEntryCheck(&entry);
            }
        }

        entryUpdated = GT_TRUE;
        wasDeletedOnLimit = GT_FALSE;

        switch(entry.fdbEntryType)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
                fdbManagerEntryUpdate_macAddr(fdbManagerId, ii, &entry, &entryUpdated,&wasDeletedOnLimit);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
                fdbManagerEntryUpdate_ipv4Mc(fdbManagerId, ii, &entry, &entryUpdated);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
                fdbManagerEntryUpdate_ipv6Mc(fdbManagerId, ii, &entry, &entryUpdated);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
                fdbManagerEntryUpdate_ipv4Uc(fdbManagerId, ii, &entry, &entryUpdated);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
                fdbManagerEntryUpdate_ipv6Uc(fdbManagerId, ii, &entry, &entryUpdated);
                break;
            default:
                break;
        }

        if (entryUpdated == GT_TRUE)
        {
            numEntriesUpdated++;
        }
        if((numEntriesUpdated % 10000) == 0)
        {
            PRV_UTF_LOG1_MAC("updated [%d] entries \n",numEntriesUpdated);
        }

        FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId,&entry,st);
        if(wasDeletedOnLimit == GT_TRUE)
        {
            UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerEntryDelete: expected to NOT GT_OK on manager [%d]",
                fdbManagerId);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerEntryDelete: expected to GT_OK on manager [%d]",
                fdbManagerId);
        }
        numEntriesRemoved++;

        if((numEntriesRemoved % 10000) == 0)
        {
            PRV_UTF_LOG1_MAC("removed [%d] entries \n",numEntriesRemoved);
        }
        ii++;
    }

    PRV_UTF_LOG2_MAC("ended - removed [%d] entries (out of expected [%d]) \n",numEntriesRemoved,numEntriesAdded);

    UTF_VERIFY_EQUAL2_STRING_MAC(numEntriesAdded, numEntriesRemoved,
        "expected number of removed entries [%d] to be the same as the added entries[%d]",
        numEntriesRemoved,numEntriesAdded);

    /* Verify DB */
    st = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    /* check invalid parameters */
    for(ii = 0 ; ii < UT_FDB_ENTRY_TYPE_VALID_NUM; ii++)
    {
        fdbEntryType = fdbEntryType_valid_arr[ii % UT_FDB_ENTRY_TYPE_VALID_NUM];

        switch(fdbEntryType)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : mac addr \n");
                bad_param_check_fdbManagerEntryUpdate_macAddr(fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : ipv4mc \n");
                bad_param_check_fdbManagerEntryUpdate_ipv4Mc(fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : ipv6mc \n");
                bad_param_check_fdbManagerEntryUpdate_ipv6Mc(fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : ipv4uc \n");
                bad_param_check_fdbManagerEntryUpdate_ipv4Uc(fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : ipv6uc \n");
                bad_param_check_fdbManagerEntryUpdate_ipv6Uc(fdbManagerId);
                break;
            default:
                break;
        }
    }

    PRV_UTF_LOG0_MAC("check NULL pointers \n");
    /* NULL pointer */
    FDB_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(fdbManagerId,NULL,&params,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerEntryAdd: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);

    FDB_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(fdbManagerId,&entry,NULL,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerEntryAdd: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);

    /* Verify DB */
    st = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    PRV_UTF_LOG0_MAC("Ended \n");
    return;
}

static GT_STATUS   fdbManagerEntry_CounterPacketTypeGet
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC            *entryPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_ENT   *counterTypePtr
)
{
    GT_U32      isMacMc;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT fdbEntryType;

    fdbEntryType = entryPtr->fdbEntryType;

    switch(fdbEntryType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
            isMacMc  = entryPtr->format.fdbEntryMacAddrFormat.macAddr.arEther[0] & 1; /* check bit 40 of the mac address */
            if(isMacMc)
            {
                *counterTypePtr = entryPtr->format.fdbEntryMacAddrFormat.isStatic == GT_TRUE ?
                    PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_MULTICAST_STATIC_E :
                    PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_MULTICAST_DYNAMIC_E;
            }
            else
            {
                *counterTypePtr = entryPtr->format.fdbEntryMacAddrFormat.isStatic == GT_TRUE ?
                    PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_STATIC_E :
                    PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E;
            }
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
            *counterTypePtr = PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV4_UNICAST_ROUTE_E;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
            *counterTypePtr = PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_UNICAST_ROUTE_KEY_E;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
            *counterTypePtr = PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV4_MULTICAST_E;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
            *counterTypePtr = PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_MULTICAST_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

static void fdbManagerEntry_incrementDbCounter(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC       *entryPtr,
    IN GT_BOOL                                   entryAdded,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC   *countersPtr
)
{
#define UPDATE_COUNTER_VALUE(val, cnt) \
    val = (entryAdded) ? val + cnt : val - cnt

    GT_U8       cnt = 1;

    PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_ENT   counterType =
        PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E;

    fdbManagerEntry_CounterPacketTypeGet(entryPtr, &counterType);

    switch (counterType)
    {
        case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E:
            UPDATE_COUNTER_VALUE(countersPtr->macUnicastDynamic, cnt);
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_STATIC_E:
            UPDATE_COUNTER_VALUE(countersPtr->macUnicastStatic, cnt);
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_MULTICAST_DYNAMIC_E:
            UPDATE_COUNTER_VALUE(countersPtr->macMulticastDynamic, cnt);
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_MULTICAST_STATIC_E:
            UPDATE_COUNTER_VALUE(countersPtr->macMulticastStatic, cnt);
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV4_MULTICAST_E:
            UPDATE_COUNTER_VALUE(countersPtr->ipv4Multicast, cnt);
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_MULTICAST_E:
            UPDATE_COUNTER_VALUE(countersPtr->ipv6Multicast, cnt);
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV4_UNICAST_ROUTE_E:
            UPDATE_COUNTER_VALUE(countersPtr->ipv4UnicastRoute, cnt);
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_UNICAST_ROUTE_KEY_E:
            UPDATE_COUNTER_VALUE(countersPtr->ipv6UnicastRouteKey, cnt);
            UPDATE_COUNTER_VALUE(countersPtr->ipv6UnicastRouteData, cnt);
            break;
        default:
            break;
    }
    cnt = (counterType == PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_UNICAST_ROUTE_KEY_E)? 2 : 1;
    UPDATE_COUNTER_VALUE(countersPtr->usedEntries, cnt);

    if (entryAdded)
    {
        countersPtr->freeEntries -= cnt;
    }
    else
    {
        countersPtr->freeEntries += cnt;
    }
}

/**
GT_STATUS cpssDxChBrgFdbManagerCountersGet
(
    IN GT_U32                                   fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC *countersPtr
);
*/
static void internal_cpssDxChBrgFdbManagerCountersGetUT(void)
{
    GT_STATUS   st;
    GT_U32      ii, bankId;
    GT_U32      numEntriesRemoved;/* total number of entries removed from the DB */
    GT_U32      numEntriesAdded, banksTotalEntries;  /* total number of entries added to the DB */
    GT_BOOL     entryAdded;/* indication that the entry was added to the DB */
    GT_BOOL     getFirst;  /* indication to get first entry from the DB */
    GT_U32      fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                 entry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC   params;
    GT_U32      maxTotalEntries;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC counters;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC countersGet;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT  fdbEntryType;
    UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC outputResults;

    cpssOsMemSet(&entry , 0, sizeof(entry));
    cpssOsMemSet(&params, 0, sizeof(params));
    cpssOsMemSet(&counters, 0, sizeof(counters));
    cpssOsMemSet(&countersGet, 0, sizeof(countersGet));

    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;

    PRV_UTF_LOG0_MAC("Check non-initialized managers \n");

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChBrgFdbManagerCountersGet(ii, &countersGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChBrgFdbManagerCountersGe: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    PRV_UTF_LOG0_MAC("Check out of range managers \n");
    /* check out of range managers ... expect 'GT_BAD_PARAM' */
    for(/*continue*/; ii < 100 ; ii+= 7)
    {
        st = cpssDxChBrgFdbManagerCountersGet(ii, &countersGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerCountersGet: expected to GT_BAD_PARAM on manager [%d]",
            ii);
    }

    numEntriesAdded = 0;
    maxTotalEntries = FDB_SIZE_FULL;

    if(debug_limited_entries)
    {
        maxTotalEntries = debug_limited_entries;
    }

    counters.freeEntries = maxTotalEntries;

    if(run_device_less == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("create manager with device \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("create manager without device \n");
    }

    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId,maxTotalEntries,prvTgfDevNum, NULL, GT_FALSE);

    PRV_UTF_LOG1_MAC("start - add [%d] entries \n",maxTotalEntries);
    for(ii = 0 ; ii < maxTotalEntries; ii++)
    {
        entryAdded = GT_FALSE;

        fdbEntryType = fdbEntryType_valid_arr[ii % UT_FDB_ENTRY_TYPE_VALID_NUM];
        switch(fdbEntryType)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
                fdbManagerEntryAdd_macAddr(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
                fdbManagerEntryAdd_ipv4Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
                fdbManagerEntryAdd_ipv6Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
                fdbManagerEntryAdd_ipv4Uc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
                fdbManagerEntryAdd_ipv6Uc(fdbManagerId, ii, &outputResults);
                break;
            default:
                break;
        }

        if(entryAdded)
        {
            numEntriesAdded++;
            /* increment counters */
            fdbManagerEntry_incrementDbCounter(&entry, GT_TRUE, &counters);
        }

        if(add_device_in_runtime == GT_TRUE &&
           (ii == (maxTotalEntries/2)))
        {
            GT_U8  devListArr[1];
            GT_U32 numOfDevs = 1;

            /* now in the middle of the insertions we add the device. */
            PRV_UTF_LOG0_MAC("now in the middle of the insertions we add the device \n");

            devListArr[0] = prvTgfDevNum;

            /* add device to it */
            st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
                fdbManagerId);
        }
    }

    /* get counters */
    st = cpssDxChBrgFdbManagerCountersGet(fdbManagerId, &countersGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerCountersGet: expected to GT_OK on manager [%d]",
        ii);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(
                                        &counters, &countersGet,
                                        4 * ((GT_U32 *)&countersGet.bankCounters[0] - (GT_U32 *)&countersGet)),
        "cpssDxChBrgFdbManagerCountersGet: expected 'get' entry to match 'set' entry in manager [%d]",
        fdbManagerId);

    banksTotalEntries = 0;
    for (bankId = 0; bankId < CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS; bankId++)
    {
        banksTotalEntries +=countersGet.bankCounters[bankId];
    }

    /* Bank counters count IPv6 DATA entries, where entry number doe snot */
    UTF_VERIFY_EQUAL2_STRING_MAC(numEntriesAdded+countersGet.ipv6UnicastRouteData, banksTotalEntries,
        "cpssDxChBrgFdbManagerCountersGet: expected equal number of added entries[%d] banks entries[%d]",
        numEntriesAdded, banksTotalEntries);

    PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
        maxTotalEntries,numEntriesAdded,
            (100*numEntriesAdded)/maxTotalEntries);

    if(debug_limited_entries)
    {
        cpssDxChFdbDump(prvTgfDevNum);
    }

    numEntriesRemoved = 0;
    getFirst = GT_TRUE;

    PRV_UTF_LOG1_MAC("start - remove the [%d] entries \n",numEntriesAdded);
    if(check_hw_hold_entry == GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("(slow mode) for each entry check that HW hold the entry in proper index according to it's index \n");
    }

    ii = 0;

    /* delete all entries from the DB (one by one) */
    while(GT_OK == cpssDxChBrgFdbManagerEntryGetNext(fdbManagerId,getFirst,&entry))
    {
        getFirst = GT_FALSE;

        if(check_hw_hold_entry == GT_TRUE)
        {
            if (check_hw_hold_entry_all_tiles)
            {
                /* just before we going to remove the entry ... lets check that it exists in all tiles the HW ! */
                fdbManagerHwEntryCheckAllTiles(&entry, ii);
            }
            else
            {
                /* just before we going to remove the entry ... lets check that it exists in the HW ! */
                fdbManagerHwEntryCheck(&entry);
            }
        }

        FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId, &entry, st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryDelete: expected to GT_OK on manager [%d]",
            fdbManagerId);

        numEntriesRemoved++;
        /* decrement counters */
        fdbManagerEntry_incrementDbCounter(&entry, GT_FALSE, &counters);


        if((numEntriesRemoved % 10000) == 0)
        {
            PRV_UTF_LOG1_MAC("removed [%d] entries \n",numEntriesRemoved);
        }
        ii++;
    }

    /* get counters */
    st = cpssDxChBrgFdbManagerCountersGet(fdbManagerId, &countersGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerCountersGet: expected to GT_OK on manager [%d]",
        ii);

    UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(
                                        &counters, &countersGet,
                                        4 * ((GT_U32 *)&countersGet.bankCounters[0] - (GT_U32 *)&countersGet)),
        "cpssDxChBrgFdbManagerCountersGet: expected 'get' entry to match 'set' entry in manager [%d]",
        fdbManagerId);

    banksTotalEntries = 0;
    for (bankId = 0; bankId < CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS; bankId++)
    {
        banksTotalEntries +=countersGet.bankCounters[bankId];
    }

    UTF_VERIFY_EQUAL1_STRING_MAC(0, banksTotalEntries,
        "cpssDxChBrgFdbManagerCountersGet: not expected entries[%d] in banks after delete",
        banksTotalEntries);

    PRV_UTF_LOG2_MAC("ended - removed [%d] entries (out of expected [%d]) \n",numEntriesRemoved, numEntriesAdded);

    UTF_VERIFY_EQUAL2_STRING_MAC(numEntriesAdded, numEntriesRemoved,
        "expected number of removed entries [%d] to be the same as the added entries[%d]",
        numEntriesRemoved,numEntriesAdded);


    PRV_UTF_LOG0_MAC("check NULL pointers \n");
    /* NULL pointer */
    st = cpssDxChBrgFdbManagerCountersGet(fdbManagerId, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerCountersGet: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);


    PRV_UTF_LOG0_MAC("Ended \n");
    return;
}

static void fdbManagerEntry_updateDbAddStatistics(
    IN  GT_STATUS                                       status,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC        *statisticsPtr
)
{
    switch (status)
    {
        case GT_OK:
            statisticsPtr->entryAddOkRehashingStage0++;
            break;
        case GT_BAD_PARAM:
        case GT_BAD_PTR:
        case GT_OUT_OF_RANGE:
            statisticsPtr->entryAddErrorInputInvalid++;
            break;
        case GT_FULL:
            statisticsPtr->entryAddErrorTableFull++;
            break;
        case GT_BAD_STATE:
            statisticsPtr->entryAddErrorBadState++;
            break;
        case GT_NOT_INITIALIZED:
            break;
        case GT_HW_ERROR:
            break;
        case GT_ALREADY_EXIST:
            statisticsPtr->entryAddErrorEntryExist++;
            break;
        case GT_LEARN_LIMIT_PORT_ERROR              :
            statisticsPtr->entryAddErrorPortLimit++ ;
            break;
        case GT_LEARN_LIMIT_TRUNK_ERROR             :
            statisticsPtr->entryAddErrorTrunkLimit++;
            break;
        case GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR      :
            statisticsPtr->entryAddErrorGlobalEPortLimit++;
            break;
        case GT_LEARN_LIMIT_FID_ERROR               :
            statisticsPtr->entryAddErrorFidLimit++  ;
            break;
        case GT_LEARN_LIMIT_GLOBAL_ERROR            :
            statisticsPtr->entryAddErrorGlobalLimit++;
            break;

        default:
            PRV_UTF_LOG1_MAC("fdbManagerEntry_updateDbAddStatistics: bad parameter [%d]) \n", status);
    }
}

static void fdbManagerEntry_updateDbUpdateStatistics(
    IN  GT_STATUS                                       status,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC        *statisticsPtr
)
{
    switch (status)
    {
        case GT_OK:
            statisticsPtr->entryUpdateOk++;
            break;
        case GT_BAD_PARAM:
        case GT_BAD_PTR:
        case GT_OUT_OF_RANGE:
            statisticsPtr->entryUpdateErrorInputInvalid++;
            break;
        case GT_NOT_FOUND:
            statisticsPtr->entryUpdateErrorNotFound++;
            break;
        case GT_NOT_INITIALIZED:
            break;
        case GT_LEARN_LIMIT_PORT_ERROR              :
            statisticsPtr->entryUpdateErrorPortLimit++ ;
            break;
        case GT_LEARN_LIMIT_TRUNK_ERROR             :
            statisticsPtr->entryUpdateErrorTrunkLimit++;
            break;
        case GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR      :
            statisticsPtr->entryUpdateErrorGlobalEPortLimit++;
            break;
        case GT_LEARN_LIMIT_FID_ERROR               :
            statisticsPtr->entryUpdateErrorFidLimit++  ;
            break;
        default:
            PRV_UTF_LOG1_MAC("fdbManagerEntry_updateDbUpdateStatistics: bad parameter [%d]) \n", status);
    }
}

static void fdbManagerEntry_updateDbDeleteStatistics(
    IN  GT_STATUS                                       status,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC        *statisticsPtr
)
{
    switch (status)
    {
        case GT_OK:
            statisticsPtr->entryDeleteOk++;
            break;
        case GT_BAD_PARAM:
        case GT_BAD_PTR:
        case GT_OUT_OF_RANGE:
            statisticsPtr->entryDeleteErrorInputInvalid++;
            break;
        case GT_NOT_FOUND:
            statisticsPtr->entryDeleteErrorNotfound++;
            break;
        case GT_NOT_INITIALIZED:
            break;
        default:
            PRV_UTF_LOG1_MAC("fdbManagerEntry_updateDbDeleteStatistics: bad parameter [%d]) \n", status);
    }
}

static void fdbManagerEntry_updateDbStatistics(
    IN  UTF_PRV_CPSS_BRG_FDB_API_STATISTICS_ENT         statisticType,
    IN  GT_STATUS                                       status,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC        *statisticsPtr
)
{
    switch (statisticType)
    {
        case UTF_PRV_CPSS_API_STATISTICS_ADD_E:
            fdbManagerEntry_updateDbAddStatistics(status, statisticsPtr);
            break;
        case UTF_PRV_CPSS_API_STATISTICS_DELETE_E:
            fdbManagerEntry_updateDbDeleteStatistics(status, statisticsPtr);
            break;
        case UTF_PRV_CPSS_API_STATISTICS_UPDATE_E:
            fdbManagerEntry_updateDbUpdateStatistics(status, statisticsPtr);
            break;
        case UTF_PRV_CPSS_API_STATISTICS_SCAN_LEARNING_E:
            break;
        default:
            break;
    }
}

/**
GT_STATUS cpssDxChBrgFdbManagerStatisticsGet
(
    IN GT_U32                                      fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC  *statisticsPtr
)
*/
static void internal_cpssDxChBrgFdbManagerStatisticsGetUT(void)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_U32      numEntriesRemoved;/* total number of entries removed from the DB */
    GT_U32      numEntriesAdded;  /* total number of entries added to the DB */
    GT_BOOL     entryAdded;/* indication that the entry was added to the DB */
    GT_BOOL     getFirst;  /* indication to get first entry from the DB */
    GT_U32      fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   params;
    GT_U32    maxTotalEntries;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT        fdbEntryType;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC        statisticsGet;
    UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC outputResults;

    cpssOsMemSet(&entry ,0,sizeof(entry));
    cpssOsMemSet(&params,0,sizeof(params));
    cpssOsMemSet(&statistics_global,0,sizeof(statistics_global));
    cpssOsMemSet(&statisticsGet,0,sizeof(statisticsGet));


    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;

    PRV_UTF_LOG0_MAC("Check non-initialized managers \n");

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChBrgFdbManagerStatisticsGet(ii, &statisticsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChBrgFdbManagerStatisticsGet: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    PRV_UTF_LOG0_MAC("Check check out of range managers \n");
    /* check out of range managers ... expect 'GT_BAD_PARAM' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChBrgFdbManagerStatisticsGet(ii, &statisticsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerStatisticsGet: expected to GT_BAD_PARAM on manager [%d]",
            ii);
    }

    numEntriesAdded = 0;
    maxTotalEntries = FDB_SIZE_FULL;

    if(debug_limited_entries)
    {
        maxTotalEntries = debug_limited_entries;
    }

    if(run_device_less == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("create manager with device \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("create manager without device \n");
    }
    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId,maxTotalEntries,prvTgfDevNum, NULL, GT_FALSE);

    st = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
        fdbManagerId);

    PRV_UTF_LOG1_MAC("start - add [%d] entries \n",maxTotalEntries);
    for(ii = 0 ; ii < maxTotalEntries; ii++)
    {
        entryAdded = GT_FALSE;

        fdbEntryType = fdbEntryType_valid_arr[ii % UT_FDB_ENTRY_TYPE_VALID_NUM];

        switch(fdbEntryType)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
                fdbManagerEntryAdd_macAddr(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
                fdbManagerEntryAdd_ipv4Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
                fdbManagerEntryAdd_ipv6Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
                fdbManagerEntryAdd_ipv4Uc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
                fdbManagerEntryAdd_ipv6Uc(fdbManagerId, ii, &outputResults);
                break;
            default:
                break;
        }

        if(entryAdded)
        {
            numEntriesAdded++;
        }

        if(add_device_in_runtime == GT_TRUE &&
           (ii == (maxTotalEntries/2)))
        {
            GT_U8  devListArr[1];
            GT_U32 numOfDevs = 1;

            /* now in the middle of the insertions we add the device. */
            PRV_UTF_LOG0_MAC("now in the middle of the insertions we add the device \n");

            devListArr[0] = prvTgfDevNum;

            /* add device to it */
            st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
                fdbManagerId);
        }
    }

    PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
        maxTotalEntries,numEntriesAdded,
            (100*numEntriesAdded)/maxTotalEntries);

    if(debug_limited_entries)
    {
        cpssDxChFdbDump(prvTgfDevNum);
    }

    numEntriesRemoved = 0;
    getFirst = GT_TRUE;

    PRV_UTF_LOG1_MAC("start - remove the [%d] entries \n",numEntriesAdded);
    if(check_hw_hold_entry == GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("(slow mode) for each entry check that HW hold the entry in proper index according to it's index \n");
    }

    /* remove all entries from the DB (one by one) */
    while(GT_OK == cpssDxChBrgFdbManagerEntryGetNext(fdbManagerId,getFirst,&entry))
    {
        getFirst = GT_FALSE;

        if(check_hw_hold_entry == GT_TRUE)
        {
            if (check_hw_hold_entry_all_tiles)
            {
                /* just before we going to remove the entry ... lets check that it exists in all tiles the HW ! */
                fdbManagerHwEntryCheckAllTiles(&entry, numEntriesRemoved);
            }
            else
            {
                /* just before we going to remove the entry ... lets check that it exists in the HW ! */
                fdbManagerHwEntryCheck(&entry);
            }
        }

        FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId,&entry,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryDelete: expected to GT_OK on manager [%d]",
            fdbManagerId);
        numEntriesRemoved++;

        if((numEntriesRemoved % 10000) == 0)
        {
            PRV_UTF_LOG1_MAC("removed [%d] entries \n",numEntriesRemoved);
        }
    }

    st = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statisticsGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerStatisticsGet: expected to GT_OK on manager [%d]",
        fdbManagerId);

    UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(&statisticsGet, &statistics_global, sizeof(statisticsGet)),
                                 "cpssDxChBrgFdbManagerStatisticsGet: expected 'get' entry to match 'set' entry in manager [%d]",
                                 fdbManagerId);

    PRV_UTF_LOG2_MAC("ended - removed [%d] entries (out of expected [%d]) \n",numEntriesRemoved,numEntriesAdded);

    UTF_VERIFY_EQUAL2_STRING_MAC(numEntriesAdded, numEntriesRemoved,
        "expected number of removed entries [%d] to be the same as the added entries[%d]",
        numEntriesRemoved,numEntriesAdded);

    cpssOsMemSet(&statistics_global,0,sizeof(statistics_global));
    cpssOsMemSet(&statisticsGet,0,sizeof(statisticsGet));

    st = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
        fdbManagerId);

    /* check invalid parameters */
    for(ii = 0 ; ii < UT_FDB_ENTRY_TYPE_VALID_NUM; ii++)
    {
        fdbEntryType = fdbEntryType_valid_arr[ii % UT_FDB_ENTRY_TYPE_VALID_NUM];

        switch(fdbEntryType)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : mac addr \n");
                bad_param_check_fdbManagerEntryAdd_macAddr(fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : ipv4mc \n");
                bad_param_check_fdbManagerEntryAdd_ipv4Mc (fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : ipv6mc \n");
                bad_param_check_fdbManagerEntryAdd_ipv6Mc (fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : ipv4uc \n");
                bad_param_check_fdbManagerEntryAdd_ipv4Uc (fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : ipv6uc \n");
                bad_param_check_fdbManagerEntryAdd_ipv6Uc (fdbManagerId);
                break;
            default:
                break;
        }
    }

    PRV_UTF_LOG0_MAC("check NULL pointers \n");
    /* NULL pointer */
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId,NULL,&params,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerEntryAdd: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);

    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId,&entry,NULL,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerEntryAdd: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);

    st = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statisticsGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerStatisticsGet: expected to GT_OK on manager [%d]",
        fdbManagerId);

    UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(&statisticsGet, &statistics_global, sizeof(statisticsGet)),
        "cpssDxChBrgFdbManagerStatisticsGet: expected 'get' entry to match 'set' entry in manager [%d]",
        fdbManagerId);

    PRV_UTF_LOG0_MAC("Ended \n");
    return;
}

/**
GT_STATUS cpssDxChBrgFdbManagerEntryUpdate
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC *paramsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerEntryUpdate)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    check_hw_hold_entry = GT_TRUE;/* very slow mode ! */
    force_single_bank_to_add = BANK_AS_PER_DEVICE(9); /* force using this bank as 'first choice' */

    internal_cpssDxChBrgFdbManagerEntryUpdateUT();

    check_hw_hold_entry = GT_FALSE;
    force_single_bank_to_add = 0;/* restore */

    CLEANUP_ALL_MANAGERS;
}

/**
GT_STATUS cpssDxChBrgFdbManagerCountersGet
(
    IN GT_U32                                   fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC *countersPtr
e;
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerCountersGet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    check_hw_hold_entry = GT_TRUE;/* very slow mode ! */
    force_single_bank_to_add = BANK_AS_PER_DEVICE(9); /* force using this bank as 'first choice' */

    internal_cpssDxChBrgFdbManagerCountersGetUT();

    check_hw_hold_entry = GT_FALSE;
    force_single_bank_to_add = 0;/* restore */

    CLEANUP_ALL_MANAGERS;
}

/**
GT_STATUS cpssDxChBrgFdbManagerStatisticsGet
(
    IN GT_U32                                   fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC *countersPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerStatisticsGet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    check_hw_hold_entry = GT_TRUE;/* very slow mode ! */
    force_single_bank_to_add = BANK_AS_PER_DEVICE(9); /* force using this bank as 'first choice' */

    internal_cpssDxChBrgFdbManagerStatisticsGetUT();

    check_hw_hold_entry = GT_FALSE;
    force_single_bank_to_add = 0;/* restore */

    CLEANUP_ALL_MANAGERS;
}

/* run the test of 'cpssDxChBrgFdbManagerEntryUp-date' , but without device bound to the manager */
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerEntryUpdate_run_device_less)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    run_device_less = GT_TRUE;

    internal_cpssDxChBrgFdbManagerEntryUpdateUT();

    run_device_less = GT_FALSE;

    CLEANUP_ALL_MANAGERS;
}

/* run the test of 'cpssDxChBrgFdbManagerEntryUpdate' , but the device added to the
   manager after 1/2 table already full */
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerEntryUpdate_add_device_in_runtime)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    run_device_less = GT_TRUE;
    add_device_in_runtime = GT_TRUE;

    internal_cpssDxChBrgFdbManagerEntryUpdateUT();

    run_device_less = GT_FALSE;
    add_device_in_runtime = GT_FALSE;

    CLEANUP_ALL_MANAGERS;
}

static void setAllVlansModeVid1lookup(IN GT_BOOL enable)
{
    GT_STATUS st;
    CPSS_PORTS_BMP_STC                    portsMembers;
    CPSS_PORTS_BMP_STC                    portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC           cpssVlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    GT_U8   devNum = prvTgfDevNum;
    GT_U16  vid;
    GT_BOOL isValid;
    GT_U32  numVlans;

    /* read default VLAN entry */
    st = cpssDxChBrgVlanEntryRead(devNum, 1, &portsMembers, &portsTagging,
                                  &cpssVlanInfo, &isValid, &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgVlanEntryRead: expected to GT_OK on vlan [%d]",
        1);

    cpssVlanInfo.fdbLookupKeyMode = (enable == GT_TRUE) ?
                CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_VID1_E :
                CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_E;

    numVlans = 1 + PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(devNum);

    for(vid = 0 ; vid < numVlans ; vid++)
    {
        cpssVlanInfo.fidValue = vid;/*fid = vid */
        st = cpssDxChBrgVlanEntryWrite(devNum,vid, &portsMembers, &portsTagging,
                                  &cpssVlanInfo, &portsTaggingCmd);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgVlanEntryWrite: expected to GT_OK on vlan [%d]",
            vid);
    }

    return;
}


/* run the test of 'cpssDxChBrgFdbManagerEntryAdd' , but create manager with use of vid1 in the key (and the entry)
   NOTE: muxed with the sourceId */
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerEntryAdd_use_vid1)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /* AC5 - vid1 is not supported */
    if(!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        SKIP_TEST_MAC;
    }

    /* skip mixed multidevice boards */
    if(prvUtfIsMultiPpDeviceSystem())
    {
        SKIP_TEST_MAC;
    }

    set_global_macVlanLookupMode(CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E);
    set_global_macEntryMuxingMode(CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E);
    set_global_entryMuxingMode(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E);
    check_hw_hold_entry = GT_TRUE;/* very slow mode ! */

    /* the 'per vlan' need to be set to use vid1 in the key ! */
    setAllVlansModeVid1lookup(GT_TRUE);

    internal_cpssDxChBrgFdbManagerEntryAddUT();

    setAllVlansModeVid1lookup(GT_FALSE);

    check_hw_hold_entry = GT_FALSE;
    restore_global_macEntryMuxingMode();
    restore_global_macVlanLookupMode();
    restore_global_entryMuxingMode();

    CLEANUP_ALL_MANAGERS;
}


static void CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value
(
    IN GT_U32   fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *fdbEntryAddInfoPtr,
    IN GT_U32                                           *fieldPtr
)
{
    GT_STATUS st;
    GT_U32  ii,iiMax;
    GT_U32  origValue = *fieldPtr;
    GT_U32  step = 1111111;

    iiMax = 0xFFFFFFFF - (step + 1);
    for(ii = 0 ; ii < iiMax; ii+=step)
    {
        FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId, entryPtr, fdbEntryAddInfoPtr,st);
        *fieldPtr = ii;
        /* remove the entry */
        FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId, entryPtr,st);
        /* restore orig value */
        *fieldPtr = origValue;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryDelete: %d expected to GT_OK on value [%d]",
            *fieldPtr,
            ii);
    }
}

static void bad_param_check_fdbManagerEntryDelete_macAddr(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC *macAddrFormatPtr = &entry.format.fdbEntryMacAddrFormat;
    /* build 'valid' entry : mac addr */
    fdbManagerEntryBuild_macAddr(&entry);

    fdbEntryAddInfo.tempEntryExist  = GT_TRUE;
    fdbEntryAddInfo.tempEntryOffset = BANK_AS_PER_DEVICE(9);/*bankId*/
    fdbEntryAddInfo.rehashEnable    = GT_FALSE;

    /* check that info for the entry is valid */
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId,&entry,&fdbEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "fdbManagerEntryDelete_macAddr: unexpected GT_OK [%d]",
        fdbManagerId);

    /* remove the entry */
    FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId,&entry,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryDelete: unexpected GT_OK [%d]",
        fdbManagerId);

    /**************************************************************/
    /* the delete by 'key' fields must be tested with 'add entry' */
    /**************************************************************/

    /* next fields are not part of the KEY ! so can hold any value ! */
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &macAddrFormatPtr->saSecurityLevel);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &macAddrFormatPtr->daSecurityLevel);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &macAddrFormatPtr->userDefined);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &macAddrFormatPtr->sourceID);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        (GT_U32 *)&macAddrFormatPtr->daCommand);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        (GT_U32 *)&macAddrFormatPtr->saCommand);

    /* we not check GT_BOOL ! */
    /*
        fdbEntryMacAddrFormatPtr->isStatic        = GT_FALSE;
        fdbEntryMacAddrFormatPtr->age             = GT_TRUE;
        fdbEntryMacAddrFormatPtr->daRoute         = GT_FALSE;
        fdbEntryMacAddrFormatPtr->appSpecificCpuCode = GT_TRUE;
    */

}

static void bad_param_check_fdbManagerEntryDelete_ipvMc(
    IN GT_U32   fdbManagerId,
    IN GT_BOOL  isIpv4Mc
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC *ipMcAddrFormatPtr = &entry.format.fdbEntryIpv4McFormat;

    /* build 'valid' entry : ip_mc addr */
    fdbManagerEntryBuild_ipMcAddr(&entry, isIpv4Mc, 2);

    /* check that info for the entry is valid */
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId,&entry,&fdbEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryAdd: unexpected GT_OK [%d]",
        fdbManagerId);

    /* remove the entry */
    FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId,&entry,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryDelete: unexpected GT_OK [%d]",
        fdbManagerId);

    /**************************************************************/
    /* the delete by 'key' fields must be tested with 'add entry' */
    /**************************************************************/

    /* next fields are not part of the KEY ! so can hold any value ! */
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &ipMcAddrFormatPtr->daSecurityLevel);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &ipMcAddrFormatPtr->userDefined);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        &ipMcAddrFormatPtr->sourceID);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, &entry, &fdbEntryAddInfo,
        (GT_U32 *)&ipMcAddrFormatPtr->daCommand);

    /* we not check GT_BOOL ! */
    /*
        fdbEntryMacAddrFormatPtr->isStatic        = GT_FALSE;
        fdbEntryMacAddrFormatPtr->age             = GT_TRUE;
        fdbEntryMacAddrFormatPtr->daRoute         = GT_FALSE;
        fdbEntryMacAddrFormatPtr->appSpecificCpuCode = GT_TRUE;
    */
}

static void bad_param_check_fdbManagerEntryDelete_ipv4Mc(
    IN GT_U32   fdbManagerId
)
{
    bad_param_check_fdbManagerEntryDelete_ipvMc(fdbManagerId, GT_TRUE);
}

static void bad_param_check_fdbManagerEntryDelete_ipv6Mc(
    IN GT_U32   fdbManagerId
)
{
    bad_param_check_fdbManagerEntryDelete_ipvMc(fdbManagerId, GT_FALSE);
}

static void bad_param_check_common_fdbManagerEntryDelete_ipvxUc_any_value(
    IN GT_U32   fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                          * entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT                * ucRouteTypePtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT  * ucRouteInfoPtr
)
{
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC  fdbEntryAddInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC  *fdbEntryAddInfoPtr = &fdbEntryAddInfo;

    cpssOsMemSet(ucRouteInfoPtr, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT));
    /**************************/
    /* start to check fields */
    /**************************/
    /* check union value */
    ucRouteInfoPtr->fullFdbInfo.dstInterface.type = CPSS_INTERFACE_PORT_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        &ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.hwDevNum);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        &ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum);

    /* check union value */
    ucRouteInfoPtr->fullFdbInfo.dstInterface.type = CPSS_INTERFACE_TRUNK_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        (GT_U32 *)&ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.trunkId);

    /* check union value */
    ucRouteInfoPtr->fullFdbInfo.dstInterface.type = CPSS_INTERFACE_VIDX_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        (GT_U32 *)&ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.vidx);

    /* check union value */
    *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        &ucRouteInfoPtr->fullFdbInfo.nextHopVlanId);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        &ucRouteInfoPtr->fullFdbInfo.mtuProfileIndex);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        &ucRouteInfoPtr->fullFdbInfo.pointer.arpPointer);

    /* check union value */
    *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
        fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
        &ucRouteInfoPtr->fullFdbInfo.pointer.tunnelStartPointer);

    if(!(IS_SIP5_OR_HYBRID))
    {
        /* check union value */
        *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E;
        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
                fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
                &ucRouteInfoPtr->fullFdbInfo.pointer.natPointer);

        /* check union value */
        *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E;
        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
                fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
                &ucRouteInfoPtr->nextHopPointerToRouter);

        *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E;
        CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgfdbManagerEntryDelete_any_value(
                fdbManagerId, entryPtr, fdbEntryAddInfoPtr,
                &ucRouteInfoPtr->ecmpPointerToRouter);
    }
}

static void bad_param_check_fdbManagerEntryDelete_ipv4Uc(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC *ipv4UcAddrFormatPtr = &entry.format.fdbEntryIpv4UcFormat;

    /* build 'valid' entry : ipv4_uc addr */
    fdbManagerEntryBuild_ipv4UcAddr(&entry, 0);

    /* check that info for the entry is valid */
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId,&entry,&fdbEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryAdd: unexpected GT_OK [%d]",
        fdbManagerId);

    /* remove the entry */
    FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId,&entry,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryDelete: unexpected GT_OK [%d]",
        fdbManagerId);

    /**************************************************************/
    /* the delete by 'key' fields must be tested with 'add entry' */
    /**************************************************************/
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryDelete(
        fdbManagerId, &entry,
        &ipv4UcAddrFormatPtr->vrfId,
        0/*min*/,
        (BIT_12-1)/*max*/,
        BIT_12/5/*step*/);

    bad_param_check_common_fdbManagerEntryDelete_ipvxUc_any_value(fdbManagerId, &entry,
                                                                  &ipv4UcAddrFormatPtr->ucRouteType,
                                                                  &ipv4UcAddrFormatPtr->ucRouteInfo);
}

static void bad_param_check_fdbManagerEntryDelete_ipv6Uc(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC *ipv6UcAddrFormatPtr = &entry.format.fdbEntryIpv6UcFormat;

    /* build 'valid' entry : ipv4_uc addr */
    fdbManagerEntryBuild_ipv6UcAddr(&entry, 0);

    /* check that info for the entry is valid */
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId,&entry,&fdbEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryAdd: unexpected GT_OK [%d]",
        fdbManagerId);

    /* remove the entry */
    FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId,&entry,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryDelete: unexpected GT_OK [%d]",
        fdbManagerId);

    /**************************************************************/
    /* the delete by 'key' fields must be tested with 'add entry' */
    /**************************************************************/
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChBrgFdbManagerEntryDelete(
        fdbManagerId, &entry,
        &ipv6UcAddrFormatPtr->vrfId,
        0/*min*/,
        (BIT_12-1)/*max*/,
        BIT_12/5/*step*/);

    bad_param_check_common_fdbManagerEntryDelete_ipvxUc_any_value(fdbManagerId, &entry,
                                                                  &ipv6UcAddrFormatPtr->ucRouteType,
                                                                  &ipv6UcAddrFormatPtr->ucRouteInfo);
}

/*
GT_STATUS cpssDxChBrgFdbManagerEntryDelete
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr
);
*/
static void internal_cpssDxChBrgFdbManagerEntryDeleteUT(void)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_U32      fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT  fdbEntryType;

    cpssOsMemSet(&entry ,0,sizeof(entry));

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(ii,&entry,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChBrgFdbManagerEntryDelete: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_BAD_PARAM' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(ii,&entry,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerEntryDelete: expected to GT_BAD_PARAM on manager [%d]",
            ii);
    }

    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId, FDB_SIZE_FULL, prvTgfDevNum, NULL, GT_FALSE);

    /* check invalid parameters */
    for(ii = 0 ; ii < UT_FDB_ENTRY_TYPE_VALID_NUM; ii++)
    {
        fdbEntryType = fdbEntryType_valid_arr[ii % UT_FDB_ENTRY_TYPE_VALID_NUM];

        switch(fdbEntryType)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
                bad_param_check_fdbManagerEntryDelete_macAddr(fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
                bad_param_check_fdbManagerEntryDelete_ipv4Mc (fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
                bad_param_check_fdbManagerEntryDelete_ipv6Mc (fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
                bad_param_check_fdbManagerEntryDelete_ipv4Uc (fdbManagerId);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
                bad_param_check_fdbManagerEntryDelete_ipv6Uc (fdbManagerId);
                break;
            default:
                break;
        }
    }

    /* NULL pointer */
    FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId,NULL,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerEntryDelete: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);

}
/*
GT_STATUS cpssDxChBrgFdbManagerEntryDelete
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerEntryDelete)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerEntryDeleteUT();

    CLEANUP_ALL_MANAGERS;
}

/*
GT_STATUS cpssDxChBrgFdbManagerEntryTempDelete
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN GT_U32                                           tempEntryOffset
);
*/
static GT_VOID internal_cpssDxChBrgFdbManagerEntryTempDeleteUT(void)
{
    GT_STATUS                                           st;
    GT_U32                                              ii;
    GT_U32                                              fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                 entry;
    GT_U32                                              tempEntryOffset=0;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC      addParams;
    UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC    outputResults;
    GT_BOOL                                             entryAdded;

    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;

    cpssOsMemSet(&entry ,0,sizeof(entry));

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        tempEntryOffset = ii % 16;
        st = cpssDxChBrgFdbManagerEntryTempDelete(ii, &entry, tempEntryOffset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChBrgFdbManagerEntryDelete: expected to GT_BAD_PARAM on manager [%d], tempEntryOffset [%d]",
            ii, tempEntryOffset);
    }

    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId, FDB_SIZE_FULL, prvTgfDevNum, NULL, GT_FALSE);

    /* check out of range managers ... expect 'GT_BAD_PARAM' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        /* FDB Manager Bad Param */
        tempEntryOffset = ii % 16;
        st = cpssDxChBrgFdbManagerEntryTempDelete(ii, &entry, tempEntryOffset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerEntryDelete: expected to GT_BAD_PARAM on manager [%d], tempEntryOffset [%d]",
            ii, tempEntryOffset);

        /* tempEntryOffset bad param */
        tempEntryOffset = ii;
        st = cpssDxChBrgFdbManagerEntryTempDelete(fdbManagerId, &entry, tempEntryOffset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerEntryDelete: expected to GT_BAD_PARAM on manager [%d], tempEntryOffset [%d]",
            fdbManagerId, tempEntryOffset);
    }

    /* NULL pointer */
    st = cpssDxChBrgFdbManagerEntryTempDelete(fdbManagerId, NULL, tempEntryOffset);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerEntryDelete: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);

    /* Set FDB manager AU Message handler methord as FDB Manager for appDemo */
    st = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E,
            GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    tempEntryOffset             = BANK_AS_PER_DEVICE(5);
    force_single_bank_to_add    = tempEntryOffset; /* force using this bank as 'first choice' */
    /* Add Entry(non-SP) & Remove entry(Entry will not be removed) */
    for(ii=0; ii<5; ii++)
    {
        cpssOsMemSet(&addParams, 0, sizeof(addParams));
        fdbManagerEntryAdd_macAddr(fdbManagerId, ii, &outputResults);
        if(!entryAdded)
        {
            continue;
        }

        st = cpssDxChBrgFdbManagerEntryTempDelete(fdbManagerId, &entry, tempEntryOffset);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChBrgFdbManagerEntryTempDelete: Failed");

        st = cpssDxChBrgFdbManagerEntryGet(fdbManagerId, &entry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChBrgFdbManagerEntryGet: Failed");
    }

    /* Restore the app demo mode */
    st = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E,
            GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);
}

/*
GT_STATUS cpssDxChBrgFdbManagerEntryTempDelete
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN GT_U32                                           tempEntryOffset
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerEntryTempDelete)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerEntryTempDeleteUT();

    CLEANUP_ALL_MANAGERS;
}

/**
GT_STATUS cpssDxChBrgFdbManagerEntryGet
(
    IN GT_U32                                           fdbManagerId,
    INOUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC           *entryPtr
);
*/
static void internal_cpssDxChBrgFdbManagerEntryGetUT(void)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_U32      fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChBrgFdbManagerEntryGet(ii,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChBrgFdbManagerEntryGet: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_BAD_PARAM' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChBrgFdbManagerEntryGet(ii,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerEntryGet: expected to GT_BAD_PARAM on manager [%d]",
            ii);
    }

    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId, FDB_SIZE_FULL, prvTgfDevNum, NULL, GT_FALSE);

    /* NULL pointer */
    st = cpssDxChBrgFdbManagerEntryGet(fdbManagerId,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerEntryGet: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);
}

/**
GT_STATUS cpssDxChBrgFdbManagerEntryGet
(
    IN GT_U32                                           fdbManagerId,
    INOUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC           *entryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerEntryGet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerEntryGetUT();

    CLEANUP_ALL_MANAGERS;
}

/**
GT_STATUS cpssDxChBrgFdbManagerEntryGetNext
(
    IN GT_U32                                           fdbManagerId,
    IN GT_BOOL                                          getFirst,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC             *entryPtr
);
*/
static void internal_cpssDxChBrgFdbManagerEntryGetNextUT(void)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_U32      fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChBrgFdbManagerEntryGetNext(ii,GT_TRUE,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChBrgFdbManagerEntryGetNext: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_BAD_PARAM' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChBrgFdbManagerEntryGetNext(ii,GT_TRUE,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerEntryGetNext: expected to GT_BAD_PARAM on manager [%d]",
            ii);
    }

    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId, FDB_SIZE_FULL, prvTgfDevNum, NULL, GT_FALSE);

    /* build 'valid' entry : mac addr */
    fdbManagerEntryBuild_macAddr(&entry);

    fdbEntryAddInfo.tempEntryExist  = GT_TRUE;
    fdbEntryAddInfo.tempEntryOffset = BANK_AS_PER_DEVICE(9);/*bankId*/
    fdbEntryAddInfo.rehashEnable    = GT_FALSE;

    /* check that info for the entry is valid */
    FDB_MANAGER_ENTRY_ADD_WITH_STATISTICS(fdbManagerId,&entry,&fdbEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "fdbManagerEntryDelete_macAddr: unexpected GT_OK [%d]",
        fdbManagerId);


    /* calling 'next' without calling 'first' (valid use according to SRS document) */
    st = cpssDxChBrgFdbManagerEntryGetNext(fdbManagerId,GT_FALSE,&entry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerEntryGetNext: expected to GT_OK on manager [%d]",
        fdbManagerId);

    /* calling 'next' -- no more */
    st = cpssDxChBrgFdbManagerEntryGetNext(fdbManagerId,GT_FALSE,&entry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, st,
        "cpssDxChBrgFdbManagerEntryGetNext: expected to GT_NO_MORE on manager [%d]",
        fdbManagerId);

    /* calling 'first' */
    st = cpssDxChBrgFdbManagerEntryGetNext(fdbManagerId,GT_TRUE,&entry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerEntryGetNext: expected to GT_OK on manager [%d]",
        fdbManagerId);

    /* calling 'next' -- no more */
    st = cpssDxChBrgFdbManagerEntryGetNext(fdbManagerId,GT_FALSE,&entry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, st,
        "cpssDxChBrgFdbManagerEntryGetNext: expected to GT_NO_MORE on manager [%d]",
        fdbManagerId);

    /* NULL pointer */
    st = cpssDxChBrgFdbManagerEntryGetNext(fdbManagerId,GT_TRUE,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerEntryGetNext: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);
}

/**
GT_STATUS cpssDxChBrgFdbManagerEntryGetNext
(
    IN GT_U32                                           fdbManagerId,
    IN GT_BOOL                                          getFirst,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC             *entryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerEntryGetNext)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerEntryGetNextUT();

    CLEANUP_ALL_MANAGERS;
}

extern GT_STATUS prvTgfBrgFdbMacEntryIndexFind_MultiHash
(
    IN  PRV_TGF_MAC_ENTRY_KEY_STC    *macEntryKeyPtr,
    OUT GT_U32                       *indexPtr
);

/* function to check that the entry exists in the HW */
static void fdbManagerFdbEntryBuildKey(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC  *entryPtr,
    IN PRV_TGF_MAC_ENTRY_KEY_STC   *tgf_macEntryKeyPtr
)
{
    /* convert 'FDB manager key' to 'TGF manager key' */
    switch(entryPtr->fdbEntryType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
            if(global_macVlanLookupMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E)
            {
                tgf_macEntryKeyPtr->entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_FID_VID1_E;
                tgf_macEntryKeyPtr->vid1 = entryPtr->format.fdbEntryMacAddrFormat.vid1;
            }
            else
            {
                tgf_macEntryKeyPtr->entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
                tgf_macEntryKeyPtr->vid1  = 0;
            }
            cpssOsMemCpy(&tgf_macEntryKeyPtr->key.macVlan.macAddr,
                         &entryPtr->format.fdbEntryMacAddrFormat.macAddr,
                         sizeof(entryPtr->format.fdbEntryMacAddrFormat.macAddr));
            tgf_macEntryKeyPtr->key.macVlan.vlanId = (GT_U16)entryPtr->format.fdbEntryMacAddrFormat.fid;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
            tgf_macEntryKeyPtr->entryType    = PRV_TGF_FDB_ENTRY_TYPE_IPV4_MCAST_E;
            cpssOsMemCpy(&tgf_macEntryKeyPtr->key.ipMcast.sip,
                         entryPtr->format.fdbEntryIpv4McFormat.sipAddr, 4);
            cpssOsMemCpy(&tgf_macEntryKeyPtr->key.ipMcast.dip,
                         entryPtr->format.fdbEntryIpv4McFormat.dipAddr, 4);

            tgf_macEntryKeyPtr->key.ipMcast.vlanId = (GT_U16)entryPtr->format.fdbEntryIpv4McFormat.fid;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
            tgf_macEntryKeyPtr->entryType    = PRV_TGF_FDB_ENTRY_TYPE_IPV6_MCAST_E;
            cpssOsMemCpy(&tgf_macEntryKeyPtr->key.ipMcast.sip,
                         entryPtr->format.fdbEntryIpv6McFormat.sipAddr, 4);
            cpssOsMemCpy(&tgf_macEntryKeyPtr->key.ipMcast.dip,
                         entryPtr->format.fdbEntryIpv6McFormat.dipAddr, 4);

            tgf_macEntryKeyPtr->key.ipMcast.vlanId = (GT_U16)entryPtr->format.fdbEntryIpv4McFormat.fid;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
            tgf_macEntryKeyPtr->entryType    = PRV_TGF_FDB_ENTRY_TYPE_IPV4_UC_E;
            cpssOsMemCpy(tgf_macEntryKeyPtr->key.ipv4Unicast.dip,
                         entryPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP, 4);
            tgf_macEntryKeyPtr->key.ipv4Unicast.vrfId = entryPtr->format.fdbEntryIpv4UcFormat.vrfId;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
            tgf_macEntryKeyPtr->entryType      = PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
            cpssOsMemCpy(tgf_macEntryKeyPtr->key.ipv6Unicast.dip,
                         entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP, 16);
            tgf_macEntryKeyPtr->key.ipv6Unicast.vrfId = entryPtr->format.fdbEntryIpv6UcFormat.vrfId;
            break;
        default:
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, GT_OK,
                "fdbManagerHwEntryCheck: invalid entry type [%d]",
                entryPtr->fdbEntryType);
    }
}
/* function to check that the entry exists in the HW */
static void fdbManagerHwEntryCheck(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC  *entryPtr
)
{
    GT_STATUS   st;
    PRV_TGF_MAC_ENTRY_KEY_STC   tgf_macEntryKey;
    GT_U32  hwIndex;

    cpssOsMemSet(&tgf_macEntryKey ,0, sizeof(PRV_TGF_MAC_ENTRY_KEY_STC));

    fdbManagerFdbEntryBuildKey(entryPtr, &tgf_macEntryKey);

    st = prvTgfBrgFdbMacEntryIndexFind_MultiHash(&tgf_macEntryKey,&hwIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "prvTgfBrgFdbMacEntryIndexFind_MultiHash: expected to find entry type [%d]",
        entryPtr->fdbEntryType);
}

/* function to check that the entry exists in all tiles in the HW */
static void fdbManagerHwEntryCheckAllTiles(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC  *entryPtr,
    IN GT_U32                               iteration
)
{
    GT_STATUS   st;
    PRV_TGF_MAC_ENTRY_KEY_STC   tgf_macEntryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC   tmpMacEntry;
    GT_U32  hwIndex;
    GT_U32  prevFdbIndex = 0xFFFFFFFF;
    GT_U32  ii;
    PRV_TGF_BRG_FDB_ACCESS_MODE_ENT oldMode;

    /*  skip entries - reduce test running time */
    if (iteration % 1000)
    {
        return;
    }

    /* state that the FDB accessing is 'by index' */
    oldMode = prvTgfBrgFdbAccessModeSet(PRV_TGF_BRG_FDB_ACCESS_MODE_BY_INDEX_E);

    cpssOsMemSet(&tgf_macEntryKey ,0, sizeof(PRV_TGF_MAC_ENTRY_KEY_STC));

    usePortGroupsBmp =  GT_TRUE;

    fdbManagerFdbEntryBuildKey(entryPtr, &tgf_macEntryKey);

    for(ii = 0; ii < PRV_CPSS_PP_MAC(prvTgfDevNum)->multiPipe.numOfTiles; ii++)
    {
        currPortGroupsBmp = BIT_0 << ii;

        st = prvTgfBrgFdbMacEntryFind(&tgf_macEntryKey, &hwIndex, NULL, NULL, &tmpMacEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if(prevFdbIndex == 0xFFFFFFFF)
        {
            prevFdbIndex = hwIndex;
        }
        else
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(hwIndex, prevFdbIndex);
        }
    }

    usePortGroupsBmp =  GT_FALSE;

    /* restore old mode */
    prvTgfBrgFdbAccessModeSet(oldMode);
}

void test_Jira_CPSS_9420_FDB_MANAGER_Delete_mac_entry_doesnt_work(void)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry,entryZero;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;

    GT_U32      fdbManagerId = 11;

     /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId, FDB_SIZE_FULL, prvTgfDevNum, NULL, GT_FALSE);

    cpssOsMemSet(&entry,0,sizeof(entry));
    cpssOsMemSet(&entryZero,0,sizeof(entryZero));
    cpssOsMemSet(&fdbEntryAddInfo,0,sizeof(fdbEntryAddInfo));

    entry.format.fdbEntryMacAddrFormat.macAddr.arEther[2]=
    entry.format.fdbEntryMacAddrFormat.macAddr.arEther[3]=
    entry.format.fdbEntryMacAddrFormat.macAddr.arEther[4]=
    entry.format.fdbEntryMacAddrFormat.macAddr.arEther[5]=0xff;
    entry.format.fdbEntryMacAddrFormat.fid = 1767;
    entry.format.fdbEntryMacAddrFormat.vid1 = 0;
    entry.format.fdbEntryMacAddrFormat.dstInterface.type = CPSS_INTERFACE_PORT_E;
    entry.format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.hwDevNum = 0;
    entry.format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.portNum = 54;
    entry.format.fdbEntryMacAddrFormat.daSecurityLevel = 0;
    entry.format.fdbEntryMacAddrFormat.saSecurityLevel = 0;
    entry.format.fdbEntryMacAddrFormat.sourceID = 0;
    entry.format.fdbEntryMacAddrFormat.userDefined = 0;
    entry.format.fdbEntryMacAddrFormat.daCommand = CPSS_PACKET_CMD_FORWARD_E;
    entry.format.fdbEntryMacAddrFormat.saCommand = CPSS_PACKET_CMD_FORWARD_E;
    entry.format.fdbEntryMacAddrFormat.isStatic = GT_TRUE;
    entry.format.fdbEntryMacAddrFormat.age = GT_FALSE;
    entry.format.fdbEntryMacAddrFormat.daRoute = GT_FALSE;
    entry.format.fdbEntryMacAddrFormat.appSpecificCpuCode = GT_FALSE;

    /* check that info for the entry is valid */
    st = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId,&entry,&fdbEntryAddInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryAdd: GT_OK [%d]",
        fdbManagerId);

    st = cpssDxChBrgFdbManagerEntryGet(fdbManagerId,&entryZero);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND,st,
        "cpssDxChBrgFdbManagerEntryGet: expected GT_NOT_FOUND [%d]",
        fdbManagerId);

    cpssOsMemSet(&entry,0,sizeof(entry));
    entry.format.fdbEntryMacAddrFormat.macAddr.arEther[2]=
    entry.format.fdbEntryMacAddrFormat.macAddr.arEther[3]=
    entry.format.fdbEntryMacAddrFormat.macAddr.arEther[4]=
    entry.format.fdbEntryMacAddrFormat.macAddr.arEther[5]=0xff;
    entry.format.fdbEntryMacAddrFormat.fid = 1767;

    /* check that info for the entry is valid */
    st = cpssDxChBrgFdbManagerEntryDelete(fdbManagerId,&entry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryDelete: GT_OK [%d]",
        fdbManagerId);
}

UTF_TEST_CASE_MAC(test_Jira_CPSS_9420_FDB_MANAGER_Delete_mac_entry_doesnt_work)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    test_Jira_CPSS_9420_FDB_MANAGER_Delete_mac_entry_doesnt_work();

    CLEANUP_ALL_MANAGERS;
}

static GT_VOID verify_cpssDxChBrgFdbManagerPortLearningSetUT
(
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC *learningSetPtr
)
{
    GT_BOOL                                     dummy;
    GT_STATUS                                   st;
    CPSS_PORT_LOCK_CMD_ENT                      cmdGet;
    CPSS_DXCH_FDB_LEARN_PRIORITY_ENT            learnPriorityGet;
    CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC learningGet;
    GT_U32                                      userGroupGet;
    CPSS_PACKET_CMD_ENT                         movedMacSaCmdGet;
    GT_U8                                       devNum = prvTgfDevNum;

    /* Set the learning attributes */
    st = cpssDxChBrgFdbManagerPortLearningSet(devNum, portNum, learningSetPtr);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerPortLearningSet: %d, %d", devNum, portNum);

    /* Verify Learning set params with FDB Manager API */
    st = cpssDxChBrgFdbManagerPortLearningGet(devNum, portNum, &learningGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerPortLearningGet: %d, %d", devNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(learningSetPtr->naMsgToCpuEnable, learningGet.naMsgToCpuEnable,
            "Get another naMsgToCpuEnable than was set: %d, %d", devNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(learningSetPtr->unkSrcAddrCmd, learningGet.unkSrcAddrCmd,
            "Get another unkSrcAddrCmd than was set: %d, %d", devNum, portNum);
    if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(learningSetPtr->learnPriority, learningGet.learnPriority,
                "Get another learnPriority than was set: %d, %d", devNum, portNum);
        UTF_VERIFY_EQUAL2_STRING_MAC(learningSetPtr->userGroup, learningGet.userGroup,
                "Get another userGroup than was set: %d, %d", devNum, portNum);
        UTF_VERIFY_EQUAL2_STRING_MAC(learningSetPtr->movedMacSaCmd, learningGet.movedMacSaCmd,
                "Get another movedMacSaCmd than was set: %d, %d", devNum, portNum);

        /* Verify Learning set params with low level API - learnPriority */
        st = cpssDxChBrgFdbLearnPriorityGet(devNum, portNum, &learnPriorityGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbLearnPriorityGet: %d, %d", devNum, portNum);
        UTF_VERIFY_EQUAL2_STRING_MAC(learningSetPtr->learnPriority, learnPriorityGet,
                "get another learnPriority than was set: %d, %d", devNum, portNum);

        /* Verify Learning set params with low level API - userGroup */
        st = cpssDxChBrgFdbUserGroupGet(devNum, portNum, &userGroupGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbUserGroupGet: %d, %d", devNum, portNum);
        UTF_VERIFY_EQUAL2_STRING_MAC(learningSetPtr->userGroup, userGroupGet,
                "get another learnPriority than was set: %d, %d", devNum, portNum);

        /* Verify Learning set params with low level API - movedMacSaCmd */
        st = cpssDxChBrgFdbPortMovedMacSaCommandGet(devNum, portNum, &movedMacSaCmdGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(learningSetPtr->movedMacSaCmd, movedMacSaCmdGet,
                "get another cmd than was set: %d, %d", devNum, portNum);
    }

    /* Verify Learning set params with low level API - unkSrcAddrCmd */
    st = cpssDxChBrgFdbPortLearnStatusGet(devNum, portNum, &dummy, &cmdGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbPortLearnStatusGet: %d, %d", devNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(learningSetPtr->unkSrcAddrCmd, cmdGet,
            "get another cmd than was set: %d, %d", devNum, portNum);
}

/**
GT_STATUS cpssDxChBrgFdbManagerPortLearningSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC *learningPtr
);
*/
static GT_VOID internal_cpssDxChBrgFdbManagerPortLearningSetUT(GT_VOID)
{
    GT_STATUS                                   st;
    CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC learningSet;
    GT_U32                                      unkSrcAddrCmdCns;
    GT_U32                                      userGroupCns;
    GT_U32                                      naMsgToCpuEnableCns;
    GT_PORT_NUM                                 portNum = 0;
    GT_U8                                       devNum = prvTgfDevNum;
    GT_U32                                      learnPriorityCns;
    GT_BOOL                                     validNaMsgToCpuEnableArr[]  = { GT_TRUE, GT_FALSE };
    CPSS_DXCH_FDB_LEARN_PRIORITY_ENT            validLearnPriorityArr[] = {
                                                            CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E,
                                                            CPSS_DXCH_FDB_LEARN_PRIORITY_HIGH_E};
    CPSS_PORT_LOCK_CMD_ENT                      validUnkSrcAddrCmdArr[] = {
                                                            CPSS_LOCK_FRWRD_E,
                                                            CPSS_LOCK_DROP_E,
                                                            CPSS_LOCK_TRAP_E,
                                                            CPSS_LOCK_MIRROR_E,
                                                            CPSS_LOCK_SOFT_DROP_E};

    st = prvUtfNextVirtPortReset(&portNum, devNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* For all active devices go over all available physical ports. */
    while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
    {
        for(userGroupCns = 0 ; userGroupCns < 16; userGroupCns ++)
        {
            for(unkSrcAddrCmdCns = 0; unkSrcAddrCmdCns < 5; unkSrcAddrCmdCns++)
            {
                for(naMsgToCpuEnableCns = 0; naMsgToCpuEnableCns < 2; naMsgToCpuEnableCns++)
                {
                    for(learnPriorityCns = 0; learnPriorityCns < 2; learnPriorityCns++)
                    {
                        learningSet.naMsgToCpuEnable = validNaMsgToCpuEnableArr[naMsgToCpuEnableCns];
                        learningSet.learnPriority    = validLearnPriorityArr[learnPriorityCns];
                        learningSet.unkSrcAddrCmd    = validUnkSrcAddrCmdArr[unkSrcAddrCmdCns];
                        learningSet.userGroup        = userGroupCns;
                        learningSet.movedMacSaCmd    = 4 - unkSrcAddrCmdCns;/* also 5 values supported */
                        verify_cpssDxChBrgFdbManagerPortLearningSetUT(portNum, &learningSet);
                    }
                }
            }
        }

        learningSet.naMsgToCpuEnable = validNaMsgToCpuEnableArr[0];
        learningSet.learnPriority    = validLearnPriorityArr[0];
        learningSet.unkSrcAddrCmd    = validUnkSrcAddrCmdArr[0];
        learningSet.userGroup        = 0;
        /* check non-valid values for unkSrcAddrCmd */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbManagerPortLearningSet
                                                    (devNum, portNum, &learningSet),
                                                    learningSet.unkSrcAddrCmd);

        if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
        {
            /* check non-valid values for learnPriority */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbManagerPortLearningSet
                    (devNum, portNum, &learningSet),
                    learningSet.learnPriority);

            /* check non-valid values for movedMacSaCmd */
            UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbManagerPortLearningSet
                    (devNum, portNum, &learningSet),
                    learningSet.movedMacSaCmd);

            /* Verify Out Of Range for userGroup */
            learningSet.userGroup        = 16;
            st = cpssDxChBrgFdbManagerPortLearningSet(devNum, portNum, &learningSet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
            learningSet.userGroup        = 0;
        }

        /* Verify NULL pointer check */
        st = cpssDxChBrgFdbManagerPortLearningSet(devNum, portNum, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
    }

    /* verify Wrong PortNum */
    portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);
    st = cpssDxChBrgFdbManagerPortLearningSet(devNum, portNum, &learningSet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

    st = prvUtfNextVirtPortReset(&portNum, devNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* For all active devices go over all non available virtual ports. */
    while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
    {
        /* Call function for each non-active port */
        st = cpssDxChBrgFdbManagerPortLearningSet(devNum, portNum, &learningSet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, FDB_MANAGER_NON_SUPPORTED_DEV_LIST);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbManagerPortLearningSet(devNum, portNum, &learningSet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbManagerPortLearningSet(devNum, portNum, &learningSet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}
/**
GT_STATUS cpssDxChBrgFdbManagerPortLearningSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC *learningPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerPortLearningSet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerPortLearningSetUT();
}

/**
GT_STATUS cpssDxChBrgFdbManagerPortLearningGet
(
    IN  GT_U8                                        devNum,
    IN  GT_PORT_NUM                                  portNum,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC *learningPtr
);
*/
static GT_VOID internal_cpssDxChBrgFdbManagerPortLearningGetUT(GT_VOID)
{
    GT_STATUS                                   st;
    CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC learningGet;
    GT_PORT_NUM                                 portNum = 0;
    GT_U8                                       devNum = prvTgfDevNum;

    /* For all active devices go over all available physical ports. */
    while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
    {
        st = cpssDxChBrgFdbManagerPortLearningGet(devNum, portNum, &learningGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

        /* Verify NULL pointer check */
        st = cpssDxChBrgFdbManagerPortLearningGet(devNum, portNum, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
    }

    /* verify Wrong PortNum */
    portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);
    st = cpssDxChBrgFdbManagerPortLearningGet(devNum, portNum, &learningGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

    st = prvUtfNextVirtPortReset(&portNum, devNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* For all active devices go over all non available virtual ports. */
    while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
    {
        /* Call function for each non-active port */
        st = cpssDxChBrgFdbManagerPortLearningGet(devNum, portNum, &learningGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, FDB_MANAGER_NON_SUPPORTED_DEV_LIST);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbManagerPortLearningGet(devNum, portNum, &learningGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbManagerPortLearningGet(devNum, portNum, &learningGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/**
GT_STATUS cpssDxChBrgFdbManagerPortLearningGet
(
    IN  GT_U8                                        devNum,
    IN  GT_PORT_NUM                                  portNum,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC *learningPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerPortLearningGet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerPortLearningGetUT();
}

/**
GT_STATUS cpssDxChBrgFdbManagerPortRoutingGet
(
    IN  GT_U8                                        devNum,
    IN  GT_PORT_NUM                                  portNum,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC  *routingPtr
);
*/
static GT_VOID internal_cpssDxChBrgFdbManagerPortRoutingGetUT(GT_VOID)
{
    GT_STATUS                                   st;
    CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC  routingGet;
    GT_PORT_NUM                                 portNum = 0;
    GT_U8                                       devNum = prvTgfDevNum;

    /* For all active devices go over all available physical ports. */
    while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
    {
        st = cpssDxChBrgFdbManagerPortRoutingGet(devNum, portNum, &routingGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, portNum);

        /* Verify NULL pointer check */
        st = cpssDxChBrgFdbManagerPortRoutingGet(devNum, portNum, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
    }

    /* verify Wrong PortNum */
    portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);
    st = cpssDxChBrgFdbManagerPortRoutingGet(devNum, portNum, &routingGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

    st = prvUtfNextVirtPortReset(&portNum, devNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* For all active devices go over all non available virtual ports. */
    while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
    {
        /* Call function for each non-active port */
        st = cpssDxChBrgFdbManagerPortRoutingGet(devNum, portNum, &routingGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, FDB_MANAGER_NON_SUPPORTED_DEV_LIST);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbManagerPortRoutingGet(devNum, portNum, &routingGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbManagerPortRoutingGet(devNum, portNum, &routingGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/**
GT_STATUS cpssDxChBrgFdbManagerPortRoutingGet
(
    IN  GT_U8                                        devNum,
    IN  GT_PORT_NUM                                  portNum,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC  *routingPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerPortRoutingGet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /* AC5 - IP UC not supported */
    if(!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        SKIP_TEST_MAC;
    }

    internal_cpssDxChBrgFdbManagerPortRoutingGetUT();
}

static GT_VOID verify_cpssDxChBrgFdbManagerPortRoutingSetUT
(
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC  *routingSetPtr
)
{
    GT_STATUS                                   st;
    CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC  routingGet;
    GT_U8                                       devNum = prvTgfDevNum;

    /* Set the routing attributes */
    st = cpssDxChBrgFdbManagerPortRoutingSet(devNum, portNum, routingSetPtr);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerPortRoutingSet: %d, %d", devNum, portNum);

    /* Verify Routing set params with FDB Manager API */
    st = cpssDxChBrgFdbManagerPortRoutingGet(devNum, portNum, &routingGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerPortRoutingGet: %d, %d", devNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(routingSetPtr->unicastIpv6RoutingEn,
                                 routingGet.unicastIpv6RoutingEn,
            "Get another unicastIpv6RoutingEn than was set: %d, %d", devNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(routingSetPtr->unicastIpv4RoutingEn,
                                 routingGet.unicastIpv4RoutingEn,
            "Get another unicastIpv4RoutingEn than was set: %d, %d", devNum, portNum);
}

/**
GT_STATUS cpssDxChBrgFdbManagerPorRoutingSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC  *routingPtr
);
*/
static GT_VOID internal_cpssDxChBrgFdbManagerPortRoutingSetUT(GT_VOID)
{
    GT_STATUS                                   st;
    CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC  routingSet;
    GT_PORT_NUM                                 portNum = 0;
    GT_U8                                       devNum = prvTgfDevNum;
    GT_U32                                      iiIpv4, iiIpv6;
    GT_BOOL                                     validunicastIpv6RoutingEnArr[]  = { GT_TRUE, GT_FALSE };
    GT_BOOL                                     validunicastIpv4RoutingEnArr[]  = { GT_TRUE, GT_FALSE };

    st = prvUtfNextVirtPortReset(&portNum, devNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* For all active devices go over all available physical ports. */
    while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_TRUE))
    {
        for(iiIpv4 = 0; iiIpv4 < 2; iiIpv4++)
        {
            for(iiIpv6 = 0; iiIpv6 < 2; iiIpv6++)
            {
                routingSet.unicastIpv6RoutingEn    = validunicastIpv6RoutingEnArr[iiIpv6];
                routingSet.unicastIpv4RoutingEn    = validunicastIpv4RoutingEnArr[iiIpv4];
                verify_cpssDxChBrgFdbManagerPortRoutingSetUT(portNum, &routingSet);
            }
        }

        /* Verify NULL pointer check */
        st = cpssDxChBrgFdbManagerPortRoutingSet(devNum, portNum, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, portNum);
    }

    /* verify Wrong PortNum */
    portNum = UTF_CPSS_PP_MAX_PORT_NUM_CNS(devNum);
    st = cpssDxChBrgFdbManagerPortRoutingSet(devNum, portNum, &routingSet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);

    st = prvUtfNextVirtPortReset(&portNum, devNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* For all active devices go over all non available virtual ports. */
    while (GT_OK == prvUtfNextVirtPortGet(&portNum, GT_FALSE))
    {
        /* Call function for each non-active port */
        st = cpssDxChBrgFdbManagerPortRoutingSet(devNum, portNum, &routingSet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, portNum);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, FDB_MANAGER_NON_SUPPORTED_DEV_LIST);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChBrgFdbManagerPortRoutingSet(devNum, portNum, &routingSet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* Call function with out of bound value for device id */
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgFdbManagerPortRoutingSet(devNum, portNum, &routingSet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/**
GT_STATUS cpssDxChBrgFdbManagerPortRoutingSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC  *routingPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerPortRoutingSet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /* AC5 - IP UC not supported */
    if(!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        SKIP_TEST_MAC;
    }

    internal_cpssDxChBrgFdbManagerPortRoutingSetUT();
}

/**
GT_STATUS cpssDxChBrgFdbManagerLearningThrottlingGet
(
    IN  GT_U32                                              fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC   *dataPtr
)
*/
static GT_VOID internal_cpssDxChBrgFdbManagerLearningThrottlingGetUT(GT_VOID)
{
    GT_STATUS                                           st;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC   throttlingGet;
    GT_U32                                              fdbManagerId;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(fdbManagerId = 0 ; fdbManagerId < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; fdbManagerId++)
    {
        st = cpssDxChBrgFdbManagerLearningThrottlingGet(fdbManagerId, &throttlingGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_INITIALIZED, st);

        createFdbManager(fdbManagerId, _16K, NULL, GT_FALSE);

        st = cpssDxChBrgFdbManagerLearningThrottlingGet(fdbManagerId, &throttlingGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChBrgFdbManagerLearningThrottlingGet(fdbManagerId, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    st = cpssDxChBrgFdbManagerLearningThrottlingGet(CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS+1, &throttlingGet);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
}

/**
GT_STATUS cpssDxChBrgFdbManagerLearningThrottlingGet
(
    IN  GT_U32                                              fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC   *dataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerLearningThrottlingGet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerLearningThrottlingGetUT();

    CLEANUP_ALL_MANAGERS;
}

/**
GT_STATUS cpssDxChBrgFdbManagerLearningScan
(
    IN  GT_U32                                             fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC   entriesLearningArray[],
    OUT GT_U32                                             *entriesLearningNumPtr
)
*/
static GT_VOID internal_cpssDxChBrgFdbManagerLearningScanUT(GT_VOID)
{
    GT_STATUS                                           st;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC  scanParam;
    GT_U32                                              fdbManagerId;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesLearningArray;
    GT_U32                                              entriesLearningNum;
    GT_U8                                               devListArr[1];
    GT_U32                                              numOfDevs = 1;
    GT_U32                                              errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];

    devListArr[0] = prvTgfDevNum;
    if(!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        scanParam.addWithRehashEnable     = GT_FALSE;
        scanParam.updateMovedMacUcEntries = GT_FALSE;
    }

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(fdbManagerId = 0 ; fdbManagerId < 2; fdbManagerId++)
    {
        st = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &scanParam, &entriesLearningArray, &entriesLearningNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_INITIALIZED, st);

        createFdbManager(fdbManagerId, _16K, NULL, GT_FALSE);

        st = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &scanParam, NULL, &entriesLearningNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &scanParam, &entriesLearningArray, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &scanParam, NULL, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, NULL, &entriesLearningArray, &entriesLearningNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /* set to enable all the checkers */
        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);

        /* Verify DB */
        st = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);

        /* add device to it */
        st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId, devListArr ,numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
                fdbManagerId);

        st = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &scanParam, NULL, &entriesLearningNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        st = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &scanParam, &entriesLearningArray, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        st = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &scanParam, NULL, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        st = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, NULL, &entriesLearningArray, &entriesLearningNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        st = cpssDxChBrgFdbManagerDevListRemove(fdbManagerId, devListArr ,numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
                fdbManagerId);

        /* Verify DB */
        st = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    }

    st = cpssDxChBrgFdbManagerLearningScan(CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS+1, &scanParam, &entriesLearningArray, &entriesLearningNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
}

/**
GT_STATUS cpssDxChBrgFdbManagerLearningScan
(
    IN  GT_U32                                             fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC   entriesLearningArray[],
    OUT GT_U32                                             *entriesLearningNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerLearningScan)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerLearningScanUT();

    CLEANUP_ALL_MANAGERS;
}

/**
GT_STATUS cpssDxChBrgFdbManagerLearningThrottlingGet_threshold_Verify
(
    IN  GT_U32                                              fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC   *dataPtr
)
*/
static GT_VOID internal_cpssDxChBrgFdbManagerLearningThrottlingGet_threshold_VerifyUT(GT_VOID)
{
    CPSS_DXCH_BRG_FDB_MANAGER_HW_CAPACITY_STC   hwCapacity;
    GT_STATUS                                   st;
    GT_U32                                      validNumOfHwIndexesArr[]= {_8K, _16K, _32K, _64K, _128K, _256K};
    GT_U32                                      validNumOfHashes[]= {4,  8,  16};
    GT_U32                                      thresholdB[]      = {40, 60, 80};
    GT_U32                                      thresholdC[]      = {90, 95, 95};
    GT_U32                                      hashCns;
    GT_U32                                      hwIndexCns;
    GT_U32                                      fdbManagerId = 0;
    GT_U32                                      errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC   throttlingGet;

    for(hashCns = 0; hashCns < 3; hashCns++)
    {
        /* SIP5 Devices support only 16 hashes */
        if((IS_SIP5_OR_HYBRID) && (validNumOfHashes[hashCns] != 16))
        {
            continue;
        }

        for(hwIndexCns = 0; hwIndexCns < 6; hwIndexCns++)
        {
            if(FDB_SIZE_FULL < validNumOfHwIndexesArr[hwIndexCns])
            {
                break;
            }

            if((!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)) &&
                    ((hwIndexCns >= 2) || (hashCns >= 1)))
            {
                /* AC5 Supports upto 4 hash and 16K entries */
                continue;
            }

            hwCapacity.numOfHwIndexes   = validNumOfHwIndexesArr[hwIndexCns];
            hwCapacity.numOfHashes      = HASH_AS_PER_DEVICE(validNumOfHashes[hashCns]);
            createFdbManager(fdbManagerId, _4K, &hwCapacity, GT_FALSE);

            /* Verify the threshold values */
            st = cpssDxChBrgFdbManagerLearningThrottlingGet(fdbManagerId, &throttlingGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            UTF_VERIFY_EQUAL0_STRING_MAC(throttlingGet.thresholdB, ((hwCapacity.numOfHwIndexes * thresholdB[hashCns])/100),
                    "Threshold B is not as expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(throttlingGet.thresholdC, ((hwCapacity.numOfHwIndexes * thresholdC[hashCns])/100),
                    "Threshold C is not as expected");

            /* set to enable all the checkers */
            CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);

            /* Verify DB */
            st = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
            UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
            fdbManagerId++;
        }
    }

}


/**
GT_STATUS cpssDxChBrgFdbManagerLearningThrottlingGet_threshold_Verify
(
    IN  GT_U32                                              fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC   *dataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerLearningThrottlingGet_threshold_Verify)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerLearningThrottlingGet_threshold_VerifyUT();

    CLEANUP_ALL_MANAGERS;
}

static GT_STATUS prvCpssDxChBrgFdbManagerLearningStage_Verify
(
    IN  GT_U32                                              fdbManagerId,
    IN PRV_TGF_BRG_FDB_MANAGER_THRESHOLD_TYPE_ENT           expectedThresoldType,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC    *throttlingGetPtr
)
{
    GT_STATUS                                           rc;

    #define MAX_AU_RATE_STAGE_B_VALUE                   PRV_TGF_AU_MSG_RATE_STAGE_B + 500
    #define MIN_AU_RATE_STAGE_B_VALUE                   PRV_TGF_AU_MSG_RATE_STAGE_B - 500

    /* SIP_4 devices do not support threshold */
    if(!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        return GT_OK;
    }

    rc = cpssDxChBrgFdbManagerLearningThrottlingGet(fdbManagerId, throttlingGetPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    PRV_UTF_LOG3_MAC("Throttling State %d auMsgRate %d hashTooLongEnabled %d\n", expectedThresoldType, throttlingGetPtr->auMsgRate, throttlingGetPtr->hashTooLongEnabled);

    /* Au message rate verify - test case ignores the last 3 digit in decimal for round up error */
    switch(expectedThresoldType)
    {
        case PRV_TGF_BRG_FDB_MANAGER_THRESHOLD_TYPE_A_E:
            if((throttlingGetPtr->hashTooLongEnabled == GT_TRUE) &&
                ((throttlingGetPtr->auMsgRateLimitEn == GT_FALSE)))
            {
                return GT_OK;
            }
            break;

        case PRV_TGF_BRG_FDB_MANAGER_THRESHOLD_TYPE_B_E:
            if((throttlingGetPtr->hashTooLongEnabled == GT_TRUE) &&
                ((throttlingGetPtr->auMsgRate > MIN_AU_RATE_STAGE_B_VALUE &&
                  throttlingGetPtr->auMsgRate < MAX_AU_RATE_STAGE_B_VALUE)))
            {
                return GT_OK;
            }
            break;

        case PRV_TGF_BRG_FDB_MANAGER_THRESHOLD_TYPE_C_E:
            if(throttlingGetPtr->hashTooLongEnabled == GT_FALSE)
            {
                return GT_OK;
            }
            break;
    }
    PRV_UTF_LOG1_MAC("Throttling is not as expected - Stage %d\n", expectedThresoldType);
    return GT_FAIL;
}


/**
GT_STATUS cpssDxChBrgFdbManagerLearningThrottlingGet_auMsgRate_Verify
(
    IN  GT_U32                                              fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC   *dataPtr
);
*/
static void internal_cpssDxChBrgFdbManagerLearningThrottlingGet_auMsgRate_VerifyUT(void)
{
    GT_U32                                                  ii;
    GT_STATUS                                               rc;
    GT_U32                                                  numEntriesAdded;  /* total number of entries added to the DB */
    GT_U32                                                  numEntriesRemoved;/* total number of entries removed from the DB */
    GT_BOOL                                                 entryAdded;       /* indication that the entry was added to the DB */
    GT_U32                                                  fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    GT_U32                                                  maxTotalEntries;
    CPSS_DXCH_BRG_FDB_MANAGER_HW_CAPACITY_STC               hwCapacity;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC       throttlingGet;
    GT_BOOL                                                 thresholdB_verified = GT_FALSE;
    GT_BOOL                                                 thresholdC_verified = GT_FALSE;
    GT_BOOL                                                 getFirst;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     entry;
    UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC        outputResults;
    GT_U32                                                  errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNumItr;

    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;

    /* create manager with device */
    hwCapacity.numOfHwIndexes   = FDB_SIZE_FULL;
    hwCapacity.numOfHashes      = HASH_AS_PER_DEVICE(16);
    /* maxTotalEntries should be less than numOfHwIndexes
     * To reach the threshold, make it as much max can be supported
     */
    maxTotalEntries = (hwCapacity.numOfHwIndexes - NUM_ENTRIES_STEPS_CNS);
    createFdbManagerWithDevice(fdbManagerId, maxTotalEntries, prvTgfDevNum, &hwCapacity, GT_TRUE);

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    /* Verify throttling parameters - Area 'A' */
    rc = prvCpssDxChBrgFdbManagerLearningStage_Verify(fdbManagerId,
                                                     PRV_TGF_BRG_FDB_MANAGER_THRESHOLD_TYPE_A_E,
                                                     &throttlingGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, fdbManagerId);

    numEntriesAdded = 0;
    PRV_UTF_LOG1_MAC("start - add [%d] entries \n",maxTotalEntries);
    for(ii = 0 ; numEntriesAdded < maxTotalEntries; ii++)
    {
        entryAdded = GT_FALSE;

        if(run_with_dynamic_uc_limits == GT_TRUE)
        {
            fdbManagerEntryAdd_macAddr_extParam(fdbManagerId, ii, &outputResults,
                (ii & 1)    ? GT_TRUE : GT_FALSE,/*isStatic*/
                (ii & 7)==7 ? GT_TRUE : GT_FALSE/*is multicast*/
                );
        }
        else
        {
            fdbManagerEntryAdd_macAddr(fdbManagerId, ii, &outputResults);
        }

        if(entryAdded)
        {
            numEntriesAdded+=1;
        }
        if((numEntriesAdded % 10000) == 0)
        {
            PRV_UTF_LOG1_MAC("Added [%d] entries \n", numEntriesAdded);
        }

        if((numEntriesAdded > throttlingGet.thresholdB) && (!thresholdB_verified))
        {
            rc = prvCpssDxChBrgFdbManagerLearningStage_Verify(fdbManagerId,
                                                     PRV_TGF_BRG_FDB_MANAGER_THRESHOLD_TYPE_B_E,
                                                     &throttlingGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, fdbManagerId);
            thresholdB_verified = GT_TRUE;

            /* Verify DB */
            rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
            for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
            {
                PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                        errorNumItr,
                        testResultArray[errorNumItr],
                        debugErrorArr[testResultArray[errorNumItr]].errString);
            }
        }
        if((numEntriesAdded > throttlingGet.thresholdC) && (!thresholdC_verified))
        {
            rc = prvCpssDxChBrgFdbManagerLearningStage_Verify(fdbManagerId,
                                                     PRV_TGF_BRG_FDB_MANAGER_THRESHOLD_TYPE_C_E,
                                                     &throttlingGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, fdbManagerId);
            thresholdC_verified = GT_TRUE;

            /* Verify DB */
            rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
            for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
            {
                PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                        errorNumItr,
                        testResultArray[errorNumItr],
                        debugErrorArr[testResultArray[errorNumItr]].errString);
            }
        }
    }
    if(!thresholdB_verified || !thresholdC_verified )
    {
        PRV_UTF_LOG3_MAC("Threshold Verification Failed: ThresholdB - [%d] entries , ThresholdC - [%d] entries, Total Learnt - [%d]",
                          throttlingGet.thresholdB, throttlingGet.thresholdC, numEntriesAdded);
    }

    getFirst = GT_TRUE;
    thresholdB_verified = GT_FALSE;
    thresholdC_verified = GT_FALSE;
    numEntriesRemoved = 0;
    /* remove entries from the DB (one by one) to verify threshold values */
    while(GT_OK == cpssDxChBrgFdbManagerEntryGetNext(fdbManagerId, getFirst, &entry))
    {
        getFirst = GT_FALSE;
        rc = cpssDxChBrgFdbManagerEntryDelete(fdbManagerId,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerEntryDelete: expected to GT_OK on manager [%d]",
            fdbManagerId);
        numEntriesRemoved++;

        if((numEntriesRemoved % 100) == 0)
        {
            PRV_UTF_LOG1_MAC("removed [%d] entries \n",numEntriesRemoved);
        }

        if(((numEntriesAdded - numEntriesRemoved) <= throttlingGet.thresholdB) && (!thresholdB_verified))
        {
            rc = prvCpssDxChBrgFdbManagerLearningStage_Verify(fdbManagerId,
                                                     PRV_TGF_BRG_FDB_MANAGER_THRESHOLD_TYPE_A_E,
                                                     &throttlingGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, fdbManagerId);
            thresholdB_verified = GT_TRUE;

            /* Verify DB */
            rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
            for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
            {
                PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                        errorNumItr,
                        testResultArray[errorNumItr],
                        debugErrorArr[testResultArray[errorNumItr]].errString);
            }
        }
        if(((numEntriesAdded - numEntriesRemoved) <= throttlingGet.thresholdC) && (!thresholdC_verified))
        {
            rc = prvCpssDxChBrgFdbManagerLearningStage_Verify(fdbManagerId,
                                                     PRV_TGF_BRG_FDB_MANAGER_THRESHOLD_TYPE_B_E,
                                                     &throttlingGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, fdbManagerId);
            thresholdC_verified = GT_TRUE;
            /* Verify DB */
            rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
            UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
            for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
            {
                PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                        errorNumItr,
                        testResultArray[errorNumItr],
                        debugErrorArr[testResultArray[errorNumItr]].errString);
            }
        }
    }

    PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
                    maxTotalEntries, numEntriesAdded, (100*numEntriesAdded)/maxTotalEntries);
    if(!thresholdB_verified || !thresholdC_verified )
    {
        PRV_UTF_LOG3_MAC("Threshold Verification Failed: ThresholdB - [%d] entries , ThresholdC - [%d] entries, Total Learnt - [%d]",
                          throttlingGet.thresholdB, throttlingGet.thresholdC, numEntriesAdded);
    }
}

/**
GT_STATUS cpssDxChBrgFdbManagerLearningThrottlingGet_auMsgRate_Verify
(
    IN  GT_U32                                              fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC   *dataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerLearningThrottlingGet_auMsgRate_Verify)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerLearningThrottlingGet_auMsgRate_VerifyUT();

    CLEANUP_ALL_MANAGERS;
}

/* clean configurations after FDB Manager suite */
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManager_clean)
{
    GT_STATUS st;
    GT_U8 devNum = prvTgfDevNum;

    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /* Restore the app demo mode */
    st = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E,
            GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            0);

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* restore drop mode */
        st = cpssDxChBrgFdbSaDaCmdDropModeSet(devNum,CPSS_MAC_TABLE_SOFT_DROP_E, CPSS_MAC_TABLE_SOFT_DROP_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    }
}

/**
GT_STATUS cpssDxChBrgFdbManagerAgingScan
(
    IN  GT_U32                                             fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC   *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC   entriesAgedoutArray[],
    OUT GT_U32                                            *entriesAgedoutNumPtr
)
*/
static GT_VOID internal_cpssDxChBrgFdbManagerAgingScanUT(GT_VOID)
{
    GT_STATUS                                           st;
    CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC     scanParam;
    GT_U32                                              fdbManagerId;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesAgedoutArray;
    GT_U32                                              entriesAgedoutNum;
    GT_U8                                               devListArr[1];
    GT_U32                                              numOfDevs = 1;

    devListArr[0] = prvTgfDevNum;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(fdbManagerId = 0 ; fdbManagerId < 2; fdbManagerId++)
    {
        st = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &scanParam, &entriesAgedoutArray, &entriesAgedoutNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_INITIALIZED, st);

        createFdbManager(fdbManagerId, _16K, NULL, GT_FALSE);

        st = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &scanParam, NULL, &entriesAgedoutNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &scanParam, &entriesAgedoutArray, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &scanParam, NULL, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, NULL, &entriesAgedoutArray, &entriesAgedoutNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /* add device to it */
        st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId, devListArr ,numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
                fdbManagerId);

        st = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &scanParam, NULL, &entriesAgedoutNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &scanParam, &entriesAgedoutArray, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &scanParam, NULL, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, NULL, &entriesAgedoutArray, &entriesAgedoutNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        st = cpssDxChBrgFdbManagerDevListRemove(fdbManagerId, devListArr ,numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
                fdbManagerId);
    }

    st = cpssDxChBrgFdbManagerAgingScan(CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS+1, &scanParam, &entriesAgedoutArray, &entriesAgedoutNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
}

/**
GT_STATUS cpssDxChBrgFdbManagerAgingScan
(
    IN  GT_U32                                             fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC   *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC   entriesAgedoutArray[],
    OUT GT_U32                                            *entriesAgedoutNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerAgingScan)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerAgingScanUT();

    CLEANUP_ALL_MANAGERS;
}

/**
GT_STATUS cpssDxChBrgFdbManagerDeleteScan
(
    IN  GT_U32                                            fdbManagerId,
    IN  GT_BOOL                                           scanStart,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC  entriesDeletedArray[],
    OUT GT_U32                                           *entriesDeletedNumPtr
)
*/
static GT_VOID internal_cpssDxChBrgFdbManagerDeleteScanUT(GT_VOID)
{
    GT_STATUS                                           st;
    CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC    scanParam;
    GT_U32                                              fdbManagerId;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesDeleteArray;
    GT_U32                                              entriesDeleteNum;
    GT_U8                                               devListArr[1];
    GT_U32                                              numOfDevs = 1;
    GT_BOOL                                             scanStart = GT_TRUE;
    GT_U32                                              errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                              errorNumItr;

    devListArr[0] = prvTgfDevNum;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(fdbManagerId = 0 ; fdbManagerId < 2; fdbManagerId++)
    {
        st = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, scanStart, &scanParam, &entriesDeleteArray, &entriesDeleteNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_INITIALIZED, st);

        createFdbManager(fdbManagerId, _16K, NULL, GT_FALSE);

        st = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, scanStart, &scanParam, NULL, &entriesDeleteNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        st = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, scanStart, &scanParam, &entriesDeleteArray, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        st = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, scanStart, &scanParam, NULL, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        st = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, scanStart, NULL, &entriesDeleteArray, &entriesDeleteNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /* add device to it */
        st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId, devListArr ,numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
                fdbManagerId);

        st = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, scanStart, &scanParam, NULL, &entriesDeleteNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        st = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, scanStart, &scanParam, &entriesDeleteArray, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        st = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, scanStart, &scanParam, NULL, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        st = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, scanStart, NULL, &entriesDeleteArray, &entriesDeleteNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /* set to enable all the checkers */
        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);

        /* Verify DB */
        st = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        st = cpssDxChBrgFdbManagerDevListRemove(fdbManagerId, devListArr ,numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
                fdbManagerId);
        scanStart = GT_FALSE;
    }

    st = cpssDxChBrgFdbManagerDeleteScan(CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS+1, scanStart, &scanParam, &entriesDeleteArray, &entriesDeleteNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
}

/**
GT_STATUS cpssDxChBrgFdbManagerDeleteScan
(
    IN  GT_U32                                            fdbManagerId,
    IN  GT_BOOL                                           scanStart,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC  entriesDeletedArray[],
    OUT GT_U32                                           *entriesDeletedNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerDeleteScan)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerDeleteScanUT();

    CLEANUP_ALL_MANAGERS;
}

/**
GT_STATUS cpssDxChBrgFdbManagerTransplantScan
(
    IN  GT_U32                                                fdbManagerId,
    IN  GT_BOOL                                               scanStart,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC      entriesTransplantedArray[],
    OUT GT_U32                                               *entriesTransplantedNumPtr
)
*/
static GT_VOID internal_cpssDxChBrgFdbManagerTransplantScanUT(GT_VOID)
{
    GT_STATUS                                            st;
    CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC scanParam;
    GT_U32                                              fdbManagerId;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesTransplantArray;
    GT_U32                                              entriesTransplantNum;
    GT_U8                                               devListArr[1];
    GT_U32                                              numOfDevs = 1;
    GT_BOOL                                             scanStart = GT_TRUE;
    GT_U32                                              errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                              errorNumItr;

    devListArr[0] = prvTgfDevNum;

    CLEAR_VAR_MAC(scanParam);

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(fdbManagerId = 0 ; fdbManagerId < 2; fdbManagerId++)
    {
        st = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart, &scanParam, &entriesTransplantArray, &entriesTransplantNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_INITIALIZED, st);

        createFdbManager(fdbManagerId, _16K, NULL, GT_FALSE);

        st = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart, &scanParam, NULL, &entriesTransplantNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        st = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart, &scanParam, &entriesTransplantArray, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        st = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart, &scanParam, NULL, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        st = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart, NULL, &entriesTransplantArray, &entriesTransplantNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /* only trunk/port supported as 'old' or as 'new' */
        scanParam.oldInterface.type = CPSS_INTERFACE_VIDX_E;
        scanParam.oldInterface.interfaceInfo.vidx = 0x03FF;

        st = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart, &scanParam, &entriesTransplantArray, &entriesTransplantNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        scanParam.oldInterface.type = CPSS_INTERFACE_PORT_E;
        scanParam.oldInterface.interfaceInfo.vidx = 0x0;

        /* only trunk/port supported as 'old' or as 'new' */
        scanParam.newInterface.type = CPSS_INTERFACE_VIDX_E;
        scanParam.newInterface.interfaceInfo.vidx = 0x03FF;

        st = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart, &scanParam, &entriesTransplantArray, &entriesTransplantNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        scanParam.newInterface.type = CPSS_INTERFACE_PORT_E;
        scanParam.newInterface.interfaceInfo.vidx = 0x0;

        /* add device to it */
        st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId, devListArr ,numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
                fdbManagerId);

        st = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart, &scanParam, NULL, &entriesTransplantNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        st = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart, &scanParam, &entriesTransplantArray, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        st = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart, &scanParam, NULL, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NO_MORE, st);

        st = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart, NULL, &entriesTransplantArray, &entriesTransplantNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /* set to enable all the checkers */
        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);

        /* Verify DB */
        st = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }
        st = cpssDxChBrgFdbManagerDevListRemove(fdbManagerId, devListArr ,numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
                fdbManagerId);
        scanStart = GT_FALSE;
    }

    st = cpssDxChBrgFdbManagerTransplantScan(CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS+1, scanStart, &scanParam, &entriesTransplantArray, &entriesTransplantNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
}

/**
GT_STATUS cpssDxChBrgFdbManagerTransplantScan
(
    IN  GT_U32                                                fdbManagerId,
    IN  GT_BOOL                                               scanStart,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC      entriesTransplantedArray[],
    OUT GT_U32                                               *entriesTransplantedNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerTransplantScan)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerTransplantScanUT();

    CLEANUP_ALL_MANAGERS;
}

/**
GT_STATUS cpssDxChBrgFdbManagerDatabaseCheck
(
    IN  GT_U32                                            fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC          *checksPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT   *resultPtr
)
*/
static GT_VOID internal_cpssDxChBrgFdbManagerDatabaseCheckUT(GT_VOID)
{
    GT_STATUS                                               rc;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          expResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNum, errorNumItr, expErrorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT                fdbEntryType;
    GT_U32                                                  ii = 0, i;
    GT_BOOL                                                 entryAdded;
    GT_U32                                                  fdbManagerId    = 1;
    GT_U32                                                  numEntriesAdded = 0;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     entry;
    UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC        outputResults;
    GT_U32                                                  maxTotalEntriesArr[3];
    const FDB_MANAGER_DEBUG_ERROR_INFO                      *errorArr;

    maxTotalEntriesArr[0] = (FDB_SIZE_FULL * 20)/100;
    maxTotalEntriesArr[1] = (FDB_SIZE_FULL * 50)/100;
    maxTotalEntriesArr[2] = (FDB_SIZE_FULL * 80)/100;

    outputResults.entryPtr                          = &entry;
    outputResults.entryAddedPtr                     = &entryAdded;

    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId, FDB_SIZE_FULL, prvTgfDevNum, NULL, GT_FALSE);

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);

    /* Basic API IN/OUT parameter Validation */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, NULL);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, NULL, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, NULL, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

    rc = cpssDxChBrgFdbManagerDatabaseCheck(CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS+1, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, rc);

    for(i=0; i<3; i++)
    {
        /* Utilize FDB by 20% , 50%, 80% - and verify the tests */
        PRV_UTF_LOG1_MAC("Start - add [%d] entries\n", maxTotalEntriesArr[i]);
        for(; ii < FDB_SIZE_FULL; ii++)
        {
            entryAdded = GT_FALSE;
            fdbEntryType = fdbEntryType_valid_arr[ii % UT_FDB_ENTRY_TYPE_VALID_NUM];

            switch(fdbEntryType)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
                    fdbManagerEntryAdd_macAddr(fdbManagerId, ii, &outputResults);
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
                    fdbManagerEntryAdd_ipv4Mc(fdbManagerId, ii, &outputResults);
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
                    fdbManagerEntryAdd_ipv6Mc(fdbManagerId, ii, &outputResults);
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
                    fdbManagerEntryAdd_ipv4Uc(fdbManagerId, ii, &outputResults);
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
                    fdbManagerEntryAdd_ipv6Uc(fdbManagerId, ii, &outputResults);
                    break;
                default:
                    break;
            }

            if(entryAdded)
            {
                numEntriesAdded++;
            }
            if((numEntriesAdded % 10000) == 0)
            {
                PRV_UTF_LOG1_MAC("Added [%d] entries \n", numEntriesAdded);
            }
            if(numEntriesAdded == maxTotalEntriesArr[i])
            {
                break;
            }
        }
        PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
                maxTotalEntriesArr[i], numEntriesAdded, (100*numEntriesAdded)/maxTotalEntriesArr[i]);

        cpssOsMemSet(testResultArray, 0, sizeof(testResultArray));
        rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "cpssDxChBrgFdbManagerDatabaseCheck: expected to GT_OK on manager [%d]",
                fdbManagerId);
        UTF_VERIFY_EQUAL2_STRING_MAC(errorNum, 0,
                "cpssDxChBrgFdbManagerDatabaseCheck: expected testResult SUCCESS, got [%d]errors on manager [%d]",
                errorNum, fdbManagerId);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                             errorNumItr,
                             testResultArray[errorNumItr],
                             debugErrorArr[testResultArray[errorNumItr]].errString);
        }
    }

    /* DEBUG API - prvCpssDxChFdbManagerDebugSelfTest */
    expErrorNum = 1;
    cpssOsMemSet(expResultArray, 0, sizeof(expResultArray));
    errorArr = (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?debugErrorArr:debugErrorArr_AC5;
    for(ii=0; errorArr[ii].expErrArrSize != GT_NA ; ii++)
    {
        if(errorArr[ii+1].expErrArrSize == GT_NA/*CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E*/)
        {
            expErrorNum = ii;
            expResultArray[0] = CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E;
        }
        else
        {
            expResultArray[0]  = ii;
            expResultArray[ii] = ii;    /* Preparing array for full run */
        }

        rc = prvCpssDxChFdbManagerDebugSelfTest(fdbManagerId, expResultArray, expErrorNum, GT_TRUE/* Inject Error */);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvCpssDxChFdbManagerDebugSelfTest: expected to GT_OK on manager [%d]",
                fdbManagerId);
        cpssOsMemSet(testResultArray, 0, sizeof(testResultArray));
        rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "cpssDxChBrgFdbManagerDatabaseCheck: expected to GT_OK on manager [%d]", fdbManagerId);

        if( ii == CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E /*14*/ &&
            errorArr[ii].expErrArrSize != errorNum &&
            errorNum == 5)
        {
            /* NOTE: we get here in Phoenix if running at mainUT :
                shell-execute prvUtfSkipLongTestsFlagSet 1
                shell-execute utfTestsTypeRun 1,0,1,2,4,5,0

                but we not get here if running just after cpssInitSystem.

                we can get here also after cpssInitSystem , if calling 24 times to osRand() !!!
                before calling the test
            */

            /* see in the error LOG : for the first 2 errors :
                ERROR GT_BAD_PTR in function: internal_cpssDxChBrgFdbManagerDatabaseCheck, file cpssdxchbrgfdbmanager.c, line[8230]. errorNumPtr = 0
                ERROR GT_BAD_PTR in function: internal_cpssDxChBrgFdbManagerDatabaseCheck, file cpssdxchbrgfdbmanager.c, line[8229]. resultArray = 0
                ERROR GT_BAD_PTR in function: internal_cpssDxChBrgFdbManagerDatabaseCheck, file cpssdxchbrgfdbmanager.c, line[8231]. checksPtr = 0
                ERROR GT_BAD_PARAM in function: internal_cpssDxChBrgFdbManagerDatabaseCheck, file cpssdxchbrgfdbmanager.c, line[8226]. fdbManagerId [33] >= [32] (out of range)
                ERROR GT_FULL in function: mainLogicEntryAdd, file cpssdxchbrgfdbmanager.c, line[2977]. the entry not exists and not able to add it (before trying 'Cuckoo')
                ERROR 0x0000ffff in function: prvCpssDxChBrgFdbManagerDbCheck_counters, file prvcpssdxchbrgfdbmanager_db.c, line[3515]. Debug Error number [1] of Type[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E]
                ERROR 0x0000ffff in function: prvCpssDxChBrgFdbManagerDbCheck_counters, file prvcpssdxchbrgfdbmanager_db.c, line[3519]. mismatch for bankNum[3] : bankPopulation[2047] != db.bankPopulation[2048]
                ERROR 0x0000ffff in function: prvCpssDxChBrgFdbManagerDbCheck_indexPointer, file prvcpssdxchbrgfdbmanager_db.c, line[3823]. Debug Error number [2] of Type[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E]
                ERROR 0x0000ffff in function: prvCpssDxChBrgFdbManagerDbCheck_indexPointer, file prvcpssdxchbrgfdbmanager_db.c, line[3827]. mismatch for dbIndex[0] : fdbManagerPtr->entryPoolPtr[dbIndex].hwIndex[22772] != hwIndex[22771]
            */

            /* the invalid hwIndex +1 cause 3 more error places ! :
                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E ,
                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E ,
                CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E
            */

            /* see in the error LOG : for the 3 next errors : *
                ERROR GT_BAD_STATE in function: prvCpssDxChFdbManagerDbAgeBinEntryGetNext, file prvcpssdxchbrgfdbmanager_db.c, line[3172].
                ERROR 0x0000ffff in function: prvCpssDxChBrgFdbManagerDbCheck_agingBin, file prvcpssdxchbrgfdbmanager_db.c, line[3907]. Debug Error number [3] of Type[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E]
                ERROR 0x0000ffff in function: prvCpssDxChBrgFdbManagerDbCheck_agingBin, file prvcpssdxchbrgfdbmanager_db.c, line[3913]. Debug Error number [4] of Type[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E]
                ERROR 0x0000ffff in function: prvCpssDxChBrgFdbManagerDbCheck_agingBin, file prvcpssdxchbrgfdbmanager_db.c, line[3916]. mismatch for totalAgeBinCnt[26213] != totalPopulation[26214]
                ERROR 0x0000ffff in function: prvCpssDxChBrgFdbManagerDbCheck_agingBin, file prvcpssdxchbrgfdbmanager_db.c, line[3928]. Debug Error number [5] of Type[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E]
            */

            errorNum = 2;/* make the test happy ! */
        }

        UTF_VERIFY_EQUAL1_STRING_MAC(errorArr[ii].expErrArrSize, errorNum,
            "cpssDxChBrgFdbManagerDatabaseCheck: failed for [%s]",errorArr[ii].errString);

        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            UTF_VERIFY_EQUAL3_STRING_MAC(errorArr[ii].expErrArr[errorNumItr], testResultArray[errorNumItr],
                    "Expected error is not matching for debug Test[%s]: [Err_No-%d]-[%s]",
                    errorArr[ii].errString,
                    testResultArray[errorNumItr],
                    errorArr[testResultArray[errorNumItr]].errString);
        }
        rc = prvCpssDxChFdbManagerDebugSelfTest(fdbManagerId, expResultArray, expErrorNum, GT_FALSE/* Restore Error */);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvCpssDxChFdbManagerDebugSelfTest: expected to GT_OK on manager [%d]",
                fdbManagerId);
    }
}

/**
GT_STATUS cpssDxChBrgFdbManagerDatabaseCheck
(
    IN  GT_U32                                            fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC          *checksPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT   *resultPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerDatabaseCheck)
{
    GT_BOOL orig_run_with_dynamic_uc_limits = run_with_dynamic_uc_limits;
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    run_with_dynamic_uc_limits        = GT_TRUE;

    internal_cpssDxChBrgFdbManagerDatabaseCheckUT();

    run_with_dynamic_uc_limits        = orig_run_with_dynamic_uc_limits;

    CLEANUP_ALL_MANAGERS;
}


/**
GT_STATUS cpssDxChBrgFdbManagerEntryRewrite
(

    IN  GT_U32 fdbManagerId,
    IN  GT_U32 entriesIndexesArray[],
    IN  GT_U32 entriesIndexesNum
)
*/
static GT_VOID internal_cpssDxChBrgFdbManagerEntryRewriteUT()
{
    GT_STATUS                                       st;
    GT_U32                                          ii;
    GT_U32                                          fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    GT_U32                                          entriesIndexesArray[512];
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC        statistics;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChBrgFdbManagerEntryRewrite(ii, entriesIndexesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChBrgFdbManagerEntryRewrite: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_BAD_PARAM' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChBrgFdbManagerEntryRewrite(ii, entriesIndexesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerEntryRewrite: expected to GT_BAD_PARAM on manager [%d]",
            ii);
    }

    /* create single valid manager */
    createFdbManagerWithDevice(fdbManagerId, FDB_SIZE_FULL, prvTgfDevNum, NULL, GT_FALSE);
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /* Check for NULL pointer */
    st = cpssDxChBrgFdbManagerEntryRewrite(fdbManagerId, NULL, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerEntryRewrite: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);

    st = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,st, "cpssDxChBrgFdbManagerStatisticsGet: GT_OK");
    UTF_VERIFY_EQUAL1_STRING_MAC(statistics.entriesRewriteOk, 0,
        "cpssDxChBrgFdbManagerStatisticsGet: rewrite OK - invalid counter", fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(statistics.entriesRewriteErrorInputInvalid, 1,
        "cpssDxChBrgFdbManagerStatisticsGet: rewrite Error input invalid - invalid counter", fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(statistics.entriesRewriteTotalRewrite, 0,
        "cpssDxChBrgFdbManagerStatisticsGet: rewrite total count - invalid counter", fdbManagerId);

    /* Fill with random hwIndex */
    for(ii=0; ii<5; ii++)
    {
        /* hw index range 20 bits */
        entriesIndexesArray[ii] = cpssOsRand() | (1 << 21);
    }

    /* Check for invalid FDB hwIndex */
    st = cpssDxChBrgFdbManagerEntryRewrite(fdbManagerId, entriesIndexesArray, 5);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
        "cpssDxChBrgFdbManagerEntryRewrite: expected to GT_OK on manager [%d]",
        fdbManagerId);

    st = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,st, "cpssDxChBrgFdbManagerStatisticsGet: GT_OK");
    UTF_VERIFY_EQUAL1_STRING_MAC(statistics.entriesRewriteOk, 0,
        "cpssDxChBrgFdbManagerStatisticsGet: rewrite OK - invalid counter", fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(statistics.entriesRewriteErrorInputInvalid, 2,
        "cpssDxChBrgFdbManagerStatisticsGet: rewrite Error input invalid - invalid counter", fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(statistics.entriesRewriteTotalRewrite, 0,
        "cpssDxChBrgFdbManagerStatisticsGet: rewrite total count - invalid counter", fdbManagerId);

    /* Fill with random hwIndex */
    for(ii=0; ii<512; ii++)
    {
        entriesIndexesArray[ii] = cpssOsRand() % fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes;
    }

    st = cpssDxChBrgFdbManagerEntryRewrite(fdbManagerId, entriesIndexesArray, 512);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerEntryRewrite: expected to GT_OK on manager [%d]",
        fdbManagerId);

    st = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,st, "cpssDxChBrgFdbManagerStatisticsGet: GT_OK");
    UTF_VERIFY_EQUAL1_STRING_MAC(statistics.entriesRewriteOk, 1,
        "cpssDxChBrgFdbManagerStatisticsGet: rewrite OK - invalid counter", fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(statistics.entriesRewriteErrorInputInvalid, 2,
        "cpssDxChBrgFdbManagerStatisticsGet: rewrite Error input invalid - invalid counter", fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(statistics.entriesRewriteTotalRewrite, 512,
        "cpssDxChBrgFdbManagerStatisticsGet: rewrite total count - invalid counter", fdbManagerId);
}

/**
GT_STATUS cpssDxChBrgFdbManagerEntryRewrite
(

    IN  GT_U32 fdbManagerId,
    IN  GT_U32 entriesIndexesArray[],
    IN  GT_U32 entriesIndexesNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerEntryRewrite)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerEntryRewriteUT();

    CLEANUP_ALL_MANAGERS;
}

#define convert_CPSS_MAC_TABLE_CMD_ENT__to__CPSS_PACKET_CMD_ENT(IN_cmd,OUT_cmd)                     \
    switch(IN_cmd)                                                                                  \
    {                                                                                               \
        case CPSS_MAC_TABLE_FRWRD_E             : OUT_cmd = CPSS_PACKET_CMD_FORWARD_E; break;       \
        case CPSS_MAC_TABLE_DROP_E              : OUT_cmd = CPSS_PACKET_CMD_DROP_HARD_E; break;     \
        case CPSS_MAC_TABLE_INTERV_E            : OUT_cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E; break;   \
        case CPSS_MAC_TABLE_CNTL_E              : OUT_cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E; break;   \
        case CPSS_MAC_TABLE_MIRROR_TO_CPU_E     : OUT_cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E; break; \
        case CPSS_MAC_TABLE_SOFT_DROP_E         : OUT_cmd = CPSS_PACKET_CMD_DROP_SOFT_E; break;     \
        default:OUT_cmd = (CPSS_PACKET_CMD_ENT)0xacbdabcd; break;                                   \
    }

/**
GT_STATUS cpssDxChBrgFdbManagerConfigUpdate
(
    IN  GT_U32                                                        fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC          *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC             *agingPtr
);

*/
static GT_VOID internal_cpssDxChBrgFdbManagerConfigUpdate(void)
{
    GT_STATUS                                       st;
    GT_U32                                          ii;
    GT_U32                                          fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC  entryAttr;
    CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC          learning;
    CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC             aging;

    CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC  last_ok_entryAttr;
    CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC          last_ok_learning;
    CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC             last_ok_aging;

    CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC  hw_entryAttr;
    CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC          hw_learning;
    CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC             hw_aging;
    CPSS_MAC_TABLE_CMD_ENT                                        hwSaDropCmd;
    CPSS_MAC_TABLE_CMD_ENT                                        hwDaDropCmd;

    OUT CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           capacityInfoGet;
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfoGet;
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           learningInfoGet;
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             lookupInfoGet;
    OUT CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              agingInfoGet;


    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChBrgFdbManagerConfigUpdate(ii, NULL,NULL,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChBrgFdbManagerConfigUpdate: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_BAD_PARAM' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChBrgFdbManagerConfigUpdate(ii,NULL,NULL,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChBrgFdbManagerConfigUpdate: expected to GT_BAD_PARAM on manager [%d]",
            ii);
    }

    /* create single valid manager */
    createFdbManagerWithDevice(fdbManagerId, FDB_SIZE_FULL, prvTgfDevNum, NULL, GT_FALSE);

    cpssOsMemSet(&entryAttr  , 0, sizeof(entryAttr)  );
    cpssOsMemSet(&learning   , 0, sizeof(learning)   );
    cpssOsMemSet(&aging      , 0, sizeof(aging)      );
    /*OK values*/
    entryAttr.daDropCommand = CPSS_PACKET_CMD_DROP_HARD_E;
    entryAttr.saDropCommand = CPSS_PACKET_CMD_DROP_HARD_E;
    entryAttr.ipNhPacketcommand = CPSS_PACKET_CMD_ROUTE_E;
    /* all the rest are Booleans that not checked for specific value */

    /* manage to get GT_OK before checking for fails */
    st = cpssDxChBrgFdbManagerConfigUpdate(fdbManagerId, &entryAttr,&learning,&aging);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerConfigUpdate: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);

    /* Check for single NULL pointer */
    st = cpssDxChBrgFdbManagerConfigUpdate(fdbManagerId, NULL, &learning,&aging);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerConfigUpdate: expected to GT_OK on manager [%d]",
        fdbManagerId);
    /* Check for single NULL pointer */
    st = cpssDxChBrgFdbManagerConfigUpdate(fdbManagerId, &entryAttr, NULL,&aging);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerConfigUpdate: expected to GT_OK on manager [%d]",
        fdbManagerId);
    /* Check for single NULL pointer */
    st = cpssDxChBrgFdbManagerConfigUpdate(fdbManagerId, &entryAttr, &learning,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerConfigUpdate: expected to GT_OK on manager [%d]",
        fdbManagerId);

    /* Check for 2 NULL parameter pointers */
    st = cpssDxChBrgFdbManagerConfigUpdate(fdbManagerId, &entryAttr, NULL,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerConfigUpdate: expected to GT_OK on manager [%d]",
        fdbManagerId);

    /* Check for 2 NULL parameter pointers */
    st = cpssDxChBrgFdbManagerConfigUpdate(fdbManagerId, NULL, &learning,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerConfigUpdate: expected to GT_OK on manager [%d]",
        fdbManagerId);

    /* Check for 2 NULL parameter pointers */
    st = cpssDxChBrgFdbManagerConfigUpdate(fdbManagerId, NULL, NULL,&aging);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerConfigUpdate: expected to GT_OK on manager [%d]",
        fdbManagerId);

    /* Check for 3 NULL parameter pointers --> this is error */
    st = cpssDxChBrgFdbManagerConfigUpdate(fdbManagerId, NULL, NULL,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChBrgFdbManagerConfigUpdate: expected to GT_BAD_PTR on manager [%d]",
        fdbManagerId);

    if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
    {
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbManagerConfigUpdate
                        (fdbManagerId, &entryAttr, &learning ,&aging),
                        entryAttr.daDropCommand);
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbManagerConfigUpdate
                        (fdbManagerId, &entryAttr, &learning ,&aging),
                        entryAttr.saDropCommand);
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbManagerConfigUpdate
                        (fdbManagerId, &entryAttr, &learning ,&aging),
                        entryAttr.ipNhPacketcommand);

        /* with NULL pointers  */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbManagerConfigUpdate
                        (fdbManagerId, &entryAttr, NULL ,&aging),
                        entryAttr.daDropCommand);
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbManagerConfigUpdate
                        (fdbManagerId, &entryAttr, &learning ,NULL),
                        entryAttr.saDropCommand);
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgFdbManagerConfigUpdate
                        (fdbManagerId, &entryAttr, NULL ,NULL),
                        entryAttr.ipNhPacketcommand);

    }

    for(ii = 0 ; ii < 100 ; ii++)
    {
        entryAttr.daDropCommand = (ii % 3) == 0 ? CPSS_PACKET_CMD_DROP_HARD_E :
                                CPSS_PACKET_CMD_DROP_SOFT_E;
        entryAttr.saDropCommand = (ii % 3) == 0 ? CPSS_PACKET_CMD_DROP_SOFT_E :
                                CPSS_PACKET_CMD_DROP_HARD_E;
        entryAttr.ipNhPacketcommand = (ii % 5) == 0 ? CPSS_PACKET_CMD_ROUTE_E :
                                      (ii % 5) == 1 ? CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E:
                                      (ii % 5) == 2 ? CPSS_PACKET_CMD_TRAP_TO_CPU_E:
                                      (ii % 5) == 3 ? CPSS_PACKET_CMD_DROP_HARD_E:
                                                      CPSS_PACKET_CMD_DROP_SOFT_E;

        learning.macRoutedLearningEnable = (ii % 4) == 0 ? GT_TRUE : GT_FALSE;

        aging.destinationUcRefreshEnable = (ii % 4) == 1 ? GT_TRUE : GT_FALSE;
        aging.destinationMcRefreshEnable = (ii % 4) == 2 ? GT_TRUE : GT_FALSE;
        aging.ipUcRefreshEnable          = (ii % 4) == 3 ? GT_TRUE : GT_FALSE;

        st = cpssDxChBrgFdbManagerConfigUpdate(fdbManagerId, &entryAttr,&learning,&aging);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerConfigUpdate: expected to GT_BAD_PTR on manager [%d]",
            fdbManagerId);


        st = cpssDxChBrgFdbManagerConfigGet(fdbManagerId,
            &capacityInfoGet,&entryAttrInfoGet,&learningInfoGet,&lookupInfoGet,&agingInfoGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerGet ");

        UTF_VERIFY_EQUAL0_STRING_MAC(entryAttr.daDropCommand,entryAttrInfoGet.daDropCommand,
            "cpssDxChBrgFdbManagerConfigGet : not get proper daDropCommand");
        UTF_VERIFY_EQUAL0_STRING_MAC(entryAttr.saDropCommand,entryAttrInfoGet.saDropCommand,
            "cpssDxChBrgFdbManagerConfigGet : not get proper saDropCommand");
        UTF_VERIFY_EQUAL0_STRING_MAC(entryAttr.ipNhPacketcommand,entryAttrInfoGet.ipNhPacketcommand,
            "cpssDxChBrgFdbManagerConfigGet : not get proper ipNhPacketcommand");
        UTF_VERIFY_EQUAL0_STRING_MAC(learning.macRoutedLearningEnable,learningInfoGet.macRoutedLearningEnable,
            "cpssDxChBrgFdbManagerConfigGet : not get proper macRoutedLearningEnable");
        UTF_VERIFY_EQUAL0_STRING_MAC(aging.destinationUcRefreshEnable,agingInfoGet.destinationUcRefreshEnable,
            "cpssDxChBrgFdbManagerConfigGet : not get proper destinationUcRefreshEnable");
        UTF_VERIFY_EQUAL0_STRING_MAC(aging.destinationMcRefreshEnable,agingInfoGet.destinationMcRefreshEnable,
            "cpssDxChBrgFdbManagerConfigGet : not get proper destinationMcRefreshEnable");
        UTF_VERIFY_EQUAL0_STRING_MAC(aging.ipUcRefreshEnable,agingInfoGet.ipUcRefreshEnable,
            "cpssDxChBrgFdbManagerConfigGet : not get proper ipUcRefreshEnable");

        if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)
        {
            if(run_device_less == GT_FALSE)
            {
                st = cpssDxChBrgFdbRoutedLearningEnableGet(prvTgfDevNum,
                        &hw_learning.macRoutedLearningEnable);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbRoutedLearningEnableGet: expected to GT_OK on device [%d]",
                    prvTgfDevNum);
                UTF_VERIFY_EQUAL0_STRING_MAC(learning.macRoutedLearningEnable,hw_learning.macRoutedLearningEnable,
                    "cpssDxChBrgFdbManagerConfigGet : not get proper macRoutedLearningEnable");
            }
        }
        else
        {
            if(run_device_less == GT_FALSE)
            {
                if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
                {
                    st = cpssDxChBrgFdbSaDaCmdDropModeGet(prvTgfDevNum,
                            &hwSaDropCmd,
                            &hwDaDropCmd);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                            "cpssDxChBrgFdbSaDaCmdDropModeGet: expected to GT_OK on device [%d]",
                            prvTgfDevNum);
                    convert_CPSS_MAC_TABLE_CMD_ENT__to__CPSS_PACKET_CMD_ENT(hwSaDropCmd,hw_entryAttr.saDropCommand);
                    convert_CPSS_MAC_TABLE_CMD_ENT__to__CPSS_PACKET_CMD_ENT(hwDaDropCmd,hw_entryAttr.daDropCommand);
                    UTF_VERIFY_EQUAL0_STRING_MAC(hw_entryAttr.daDropCommand,entryAttrInfoGet.daDropCommand,
                            "cpssDxChBrgFdbManagerConfigGet : not get proper daDropCommand");
                    UTF_VERIFY_EQUAL0_STRING_MAC(hw_entryAttr.saDropCommand,entryAttrInfoGet.saDropCommand,
                            "cpssDxChBrgFdbManagerConfigGet : not get proper saDropCommand");
                }

                st = cpssDxChBrgFdbRoutingNextHopPacketCmdGet(prvTgfDevNum,
                            &hw_entryAttr.ipNhPacketcommand);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbRoutingNextHopPacketCmdGet: expected to GT_OK on device [%d]",
                    prvTgfDevNum);

                st = prvCpssDxChBrgFdbAgeBitDaRefreshEnableGet(prvTgfDevNum,
                            &hw_aging.destinationUcRefreshEnable,
                            &hw_aging.destinationMcRefreshEnable);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "prvCpssDxChBrgFdbAgeBitDaRefreshEnableGet: expected to GT_OK on device [%d]",
                    prvTgfDevNum);

                st = cpssDxChBrgFdbRoutingUcRefreshEnableGet(prvTgfDevNum,
                            &hw_aging.ipUcRefreshEnable);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbRoutingUcRefreshEnableGet: expected to GT_OK on device [%d]",
                    prvTgfDevNum);

                st = cpssDxChBrgFdbRoutedLearningEnableGet(prvTgfDevNum,
                        &hw_learning.macRoutedLearningEnable);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbRoutedLearningEnableGet: expected to GT_OK on device [%d]",
                    prvTgfDevNum);

                UTF_VERIFY_EQUAL0_STRING_MAC(hw_entryAttr.ipNhPacketcommand,entryAttrInfoGet.ipNhPacketcommand,
                    "cpssDxChBrgFdbManagerConfigGet : not get proper ipNhPacketcommand");
                UTF_VERIFY_EQUAL0_STRING_MAC(hw_learning.macRoutedLearningEnable,learningInfoGet.macRoutedLearningEnable,
                    "cpssDxChBrgFdbManagerConfigGet : not get proper macRoutedLearningEnable");
                UTF_VERIFY_EQUAL0_STRING_MAC(hw_aging.destinationUcRefreshEnable,agingInfoGet.destinationUcRefreshEnable,
                    "cpssDxChBrgFdbManagerConfigGet : not get proper destinationUcRefreshEnable");
                UTF_VERIFY_EQUAL0_STRING_MAC(hw_aging.destinationMcRefreshEnable,agingInfoGet.destinationMcRefreshEnable,
                    "cpssDxChBrgFdbManagerConfigGet : not get proper destinationMcRefreshEnable");
                UTF_VERIFY_EQUAL0_STRING_MAC(hw_aging.ipUcRefreshEnable,agingInfoGet.ipUcRefreshEnable,
                    "cpssDxChBrgFdbManagerConfigGet : not get proper ipUcRefreshEnable");
            }
        }
    }

    /* save info about the 'last ok' */
    last_ok_entryAttr = entryAttr;
    last_ok_learning  = learning;
    last_ok_aging     = aging;

    /* start to check 'bad param' */
    for(ii = 0 ; ii < 100 ; ii++)
    {
        /* good values like previous 100 iterations */
        entryAttr.daDropCommand = (ii % 3) == 0 ? CPSS_PACKET_CMD_DROP_HARD_E :
                                  (ii % 3) == 1 ? CPSS_PACKET_CMD_DROP_SOFT_E :
                                  (CPSS_PACKET_CMD_ENT)cpssOsRand();
        entryAttr.saDropCommand = (ii % 3) == 0 ? CPSS_PACKET_CMD_DROP_HARD_E :
                                  (ii % 3) == 2 ? CPSS_PACKET_CMD_DROP_SOFT_E :
                                  (CPSS_PACKET_CMD_ENT)cpssOsRand();
        entryAttr.ipNhPacketcommand = (ii % 6) == 0 ? CPSS_PACKET_CMD_ROUTE_E :
                                      (ii % 6) == 1 ? CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E:
                                      (ii % 6) == 2 ? CPSS_PACKET_CMD_TRAP_TO_CPU_E:
                                      (ii % 6) == 3 ? CPSS_PACKET_CMD_DROP_HARD_E:
                                      (ii % 6) == 4 ? (CPSS_PACKET_CMD_ENT)cpssOsRand():
                                                      CPSS_PACKET_CMD_DROP_SOFT_E;

        learning.macRoutedLearningEnable = (ii % 4) == 0 ? GT_TRUE : GT_FALSE;

        aging.destinationUcRefreshEnable = (ii % 4) == 1 ? GT_TRUE : GT_FALSE;
        aging.destinationMcRefreshEnable = (ii % 4) == 2 ? GT_TRUE : GT_FALSE;
        aging.ipUcRefreshEnable          = (ii % 4) == 3 ? GT_TRUE : GT_FALSE;

        st = cpssDxChBrgFdbManagerConfigUpdate(fdbManagerId, &entryAttr,&learning,&aging);
        if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)
        {
            /* only relevant parameter is learning.macRoutedLearningEnable , and
               as GT_BOOL it will not fail the function

               so always expect GT_OK
            */

            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerConfigUpdate: expected to GT_OK on manager [%d]",
                fdbManagerId);

            if(run_device_less == GT_FALSE)
            {
                st = cpssDxChBrgFdbRoutedLearningEnableGet(prvTgfDevNum,
                        &hw_learning.macRoutedLearningEnable);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbRoutedLearningEnableGet: expected to GT_OK on device [%d]",
                    prvTgfDevNum);
                UTF_VERIFY_EQUAL0_STRING_MAC(learning.macRoutedLearningEnable,hw_learning.macRoutedLearningEnable,
                    "cpssDxChBrgFdbManagerConfigGet : not get proper macRoutedLearningEnable");
            }
        }
        else if(
             entryAttr.daDropCommand >= CPSS_PACKET_CMD_DROP_HARD_E &&
             entryAttr.daDropCommand <= CPSS_PACKET_CMD_DROP_SOFT_E &&
             entryAttr.saDropCommand >= CPSS_PACKET_CMD_DROP_HARD_E &&
             entryAttr.saDropCommand <= CPSS_PACKET_CMD_DROP_SOFT_E &&
             entryAttr.ipNhPacketcommand >= CPSS_PACKET_CMD_TRAP_TO_CPU_E &&
             entryAttr.ipNhPacketcommand <= CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E)
        {
            /* good values expected to be OK */
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerConfigUpdate: expected to GT_OK on manager [%d]",
                fdbManagerId);

            /* save info about the 'last ok' */
            last_ok_entryAttr = entryAttr;
            last_ok_learning  = learning;
            last_ok_aging     = aging;
        }
        else
        {
            /* one of the values expected to be bad param */
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                "cpssDxChBrgFdbManagerConfigUpdate: expected to GT_BAD_PARAM on manager [%d]",
                fdbManagerId);

            /* make sure that the config not changed on this iteration that caused error */

            st = cpssDxChBrgFdbManagerConfigGet(fdbManagerId,
                &capacityInfoGet,&entryAttrInfoGet,&learningInfoGet,&lookupInfoGet,&agingInfoGet);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerGet ");

            UTF_VERIFY_EQUAL0_STRING_MAC(last_ok_entryAttr.daDropCommand,entryAttrInfoGet.daDropCommand,
                "cpssDxChBrgFdbManagerConfigGet : not get proper daDropCommand");
            UTF_VERIFY_EQUAL0_STRING_MAC(last_ok_entryAttr.saDropCommand,entryAttrInfoGet.saDropCommand,
                "cpssDxChBrgFdbManagerConfigGet : not get proper saDropCommand");
            UTF_VERIFY_EQUAL0_STRING_MAC(last_ok_entryAttr.ipNhPacketcommand,entryAttrInfoGet.ipNhPacketcommand,
                "cpssDxChBrgFdbManagerConfigGet : not get proper ipNhPacketcommand");
            UTF_VERIFY_EQUAL0_STRING_MAC(last_ok_learning.macRoutedLearningEnable,learningInfoGet.macRoutedLearningEnable,
                "cpssDxChBrgFdbManagerConfigGet : not get proper macRoutedLearningEnable");
            UTF_VERIFY_EQUAL0_STRING_MAC(last_ok_aging.destinationUcRefreshEnable,agingInfoGet.destinationUcRefreshEnable,
                "cpssDxChBrgFdbManagerConfigGet : not get proper destinationUcRefreshEnable");
            UTF_VERIFY_EQUAL0_STRING_MAC(last_ok_aging.destinationMcRefreshEnable,agingInfoGet.destinationMcRefreshEnable,
                "cpssDxChBrgFdbManagerConfigGet : not get proper destinationMcRefreshEnable");
            UTF_VERIFY_EQUAL0_STRING_MAC(last_ok_aging.ipUcRefreshEnable,agingInfoGet.ipUcRefreshEnable,
                "cpssDxChBrgFdbManagerConfigGet : not get proper ipUcRefreshEnable");

            if(run_device_less == GT_FALSE)
            {
                if(SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
                {
                    st = cpssDxChBrgFdbSaDaCmdDropModeGet(prvTgfDevNum,
                            &hwSaDropCmd,
                            &hwDaDropCmd);
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                            "cpssDxChBrgFdbSaDaCmdDropModeGet: expected to GT_OK on device [%d]",
                            prvTgfDevNum);

                    convert_CPSS_MAC_TABLE_CMD_ENT__to__CPSS_PACKET_CMD_ENT(hwSaDropCmd,hw_entryAttr.saDropCommand);
                    convert_CPSS_MAC_TABLE_CMD_ENT__to__CPSS_PACKET_CMD_ENT(hwDaDropCmd,hw_entryAttr.daDropCommand);
                    UTF_VERIFY_EQUAL0_STRING_MAC(hw_entryAttr.daDropCommand,entryAttrInfoGet.daDropCommand,
                            "cpssDxChBrgFdbManagerConfigGet : not get proper daDropCommand");
                    UTF_VERIFY_EQUAL0_STRING_MAC(hw_entryAttr.saDropCommand,entryAttrInfoGet.saDropCommand,
                            "cpssDxChBrgFdbManagerConfigGet : not get proper saDropCommand");
                }

                st = cpssDxChBrgFdbRoutingNextHopPacketCmdGet(prvTgfDevNum,
                            &hw_entryAttr.ipNhPacketcommand);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbRoutingNextHopPacketCmdGet: expected to GT_OK on device [%d]",
                    prvTgfDevNum);

                st = prvCpssDxChBrgFdbAgeBitDaRefreshEnableGet(prvTgfDevNum,
                            &hw_aging.destinationUcRefreshEnable,
                            &hw_aging.destinationMcRefreshEnable);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "prvCpssDxChBrgFdbAgeBitDaRefreshEnableGet: expected to GT_OK on device [%d]",
                    prvTgfDevNum);

                st = cpssDxChBrgFdbRoutingUcRefreshEnableGet(prvTgfDevNum,
                            &hw_aging.ipUcRefreshEnable);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbRoutingUcRefreshEnableGet: expected to GT_OK on device [%d]",
                    prvTgfDevNum);

                st = cpssDxChBrgFdbRoutedLearningEnableGet(prvTgfDevNum,
                        &hw_learning.macRoutedLearningEnable);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgFdbRoutedLearningEnableGet: expected to GT_OK on device [%d]",
                    prvTgfDevNum);

                UTF_VERIFY_EQUAL0_STRING_MAC(hw_entryAttr.ipNhPacketcommand,entryAttrInfoGet.ipNhPacketcommand,
                    "cpssDxChBrgFdbManagerConfigGet : not get proper ipNhPacketcommand");
                UTF_VERIFY_EQUAL0_STRING_MAC(hw_learning.macRoutedLearningEnable,learningInfoGet.macRoutedLearningEnable,
                    "cpssDxChBrgFdbManagerConfigGet : not get proper macRoutedLearningEnable");
                UTF_VERIFY_EQUAL0_STRING_MAC(hw_aging.destinationUcRefreshEnable,agingInfoGet.destinationUcRefreshEnable,
                    "cpssDxChBrgFdbManagerConfigGet : not get proper destinationUcRefreshEnable");
                UTF_VERIFY_EQUAL0_STRING_MAC(hw_aging.destinationMcRefreshEnable,agingInfoGet.destinationMcRefreshEnable,
                    "cpssDxChBrgFdbManagerConfigGet : not get proper destinationMcRefreshEnable");
                UTF_VERIFY_EQUAL0_STRING_MAC(hw_aging.ipUcRefreshEnable,agingInfoGet.ipUcRefreshEnable,
                    "cpssDxChBrgFdbManagerConfigGet : not get proper ipUcRefreshEnable");
            }
        }
    }

}

/**
GT_STATUS cpssDxChBrgFdbManagerConfigUpdate
(
    IN  GT_U32                                                        fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC          *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC             *agingPtr
);

*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerConfigUpdate)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerConfigUpdate();

    CLEANUP_ALL_MANAGERS;

    run_device_less = GT_TRUE;

    internal_cpssDxChBrgFdbManagerConfigUpdate();

    run_device_less = GT_FALSE;

    CLEANUP_ALL_MANAGERS;
}

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
static void writeEntry(IN GT_U8 devNum, IN GT_32 hwIndex , IN GT_U32   wordsArr[])
{
    /* write the entry to the HW */
    prvCpssDxChPortGroupWriteTableEntry(devNum,
        0xFFFFFFFF,
        CPSS_DXCH_TABLE_FDB_E,
        hwIndex,
        &wordsArr[0]);
}

static GT_STATUS writeFdbTableEntry
(
    IN GT_U8                    devNum,
    IN GT_U32                   hwIndex,
    IN GT_U32                   wordsArr[],
    IN GT_U32                   writeAac
)
{
    GT_STATUS rc;

    if(writeAac)
    {
        rc = prvCpssDxChFdbManagerAacHwWriteEntry(devNum, hwIndex, wordsArr);
    }
    else
    {
        rc = prvCpssDxChFdbManagerHwWriteEntry(devNum, hwIndex, wordsArr);
    }

    return rc;
}

static GT_BOOL  rehashEnable = GT_TRUE;

void enableCuckoo(GT_U32 enable)
{
    rehashEnable = (enable) ? GT_TRUE : GT_FALSE;
}

void macAddressGet(GT_U32 iteration, GT_U64  *macAddrPtr, GT_BOOL isRand)
{
    GT_U64  temp64;
    GT_U64  macAddr48;
    GT_U32  rand32Bits;

    temp64.l[0] = 1;
    temp64.l[1] = 0;

    if (isRand == GT_FALSE)
    {
        macAddr48 = *macAddrPtr;
        macAddr48 = prvCpssMathAdd64(macAddr48, temp64);
    }
    else
    {
        rand32Bits = cpssOsRand();

        macAddr48.l[1] = (iteration >>   8) & 0xFF;
        macAddr48.l[1] = (iteration >>   0) & 0xFF;

        macAddr48.l[0] = (rand32Bits >> 24) & 0xFF;
        macAddr48.l[0] = (rand32Bits >> 16) & 0xFF;
        macAddr48.l[0] = (rand32Bits >>  8) & 0xFF;
        macAddr48.l[0] = (rand32Bits >>  0) & 0xFF;
    }

    *macAddrPtr = macAddr48;
}

void ipv6UcAddressGet(GT_U32 iteration, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC  *entryPtr, GT_BOOL isRand, GT_BOOL updateDipLowOctets)
{
    GT_U32  rand32Bits;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC  *fdbEntryFormatPtr = &entryPtr->format.fdbEntryIpv6UcFormat;

    fdbManagerEntryBuild_ipv6UcAddr(entryPtr, iteration);
    if (isRand == GT_TRUE)
    {
        rand32Bits = cpssOsRand();
        fdbEntryFormatPtr->ipv6Addr.arIP[15] = (rand32Bits >> 24) & 0xFF;
        fdbEntryFormatPtr->ipv6Addr.arIP[14] = (rand32Bits >> 16) & 0xFF;
        fdbEntryFormatPtr->ipv6Addr.arIP[13] = (rand32Bits >>  8) & 0xFF;
        fdbEntryFormatPtr->ipv6Addr.arIP[12] = (rand32Bits >>  0) & 0xFF;

        rand32Bits = cpssOsRand();
        fdbEntryFormatPtr->ipv6Addr.arIP[11] = (rand32Bits >> 24) & 0xFF;
        fdbEntryFormatPtr->ipv6Addr.arIP[10] = (rand32Bits >> 16) & 0xFF;
        fdbEntryFormatPtr->ipv6Addr.arIP[9]  = (rand32Bits >>  8) & 0xFF;
        fdbEntryFormatPtr->ipv6Addr.arIP[8]  = (rand32Bits >>  0) & 0xFF;

        rand32Bits = cpssOsRand();
        fdbEntryFormatPtr->ipv6Addr.arIP[7] = (rand32Bits >> 24) & 0xFF;
        fdbEntryFormatPtr->ipv6Addr.arIP[6] = (rand32Bits >> 16) & 0xFF;
        fdbEntryFormatPtr->ipv6Addr.arIP[5] = (rand32Bits >>  8) & 0xFF;
        fdbEntryFormatPtr->ipv6Addr.arIP[4] = (rand32Bits >>  0) & 0xFF;

        if(updateDipLowOctets)
        {
            rand32Bits = cpssOsRand();
            fdbEntryFormatPtr->ipv6Addr.arIP[3] = (rand32Bits >> 24) & 0xFF;
            fdbEntryFormatPtr->ipv6Addr.arIP[2] = (rand32Bits >> 16) & 0xFF;
            fdbEntryFormatPtr->ipv6Addr.arIP[1] = (rand32Bits >>  8) & 0xFF;
            fdbEntryFormatPtr->ipv6Addr.arIP[0] = (rand32Bits >>  0) & 0xFF;
        }

    }
}

GT_STATUS   prvCpssDxChFdbManagerTest(IN GT_U32 withHw , IN GT_U32 doPrint, GT_U32 numOfHashes, GT_32 stages, GT_BOOL isRand)
{
    GT_STATUS   rc;
    CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           capacityInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           learningInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             lookupInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              agingInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              fdbEntry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC *fdbEntryMacAddrFormatPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    CPSS_PACKET_CMD_ENT saDropCommand       = CPSS_PACKET_CMD_DROP_SOFT_E;
    GT_U8   devListArr[1] = {0};
    GT_U32  numOfDevs = 1;
    GT_U32  fdbManagerId = 7;
    GT_U32  ii,iiMax;
    GT_U32  numSuccsses;
    GT_U64  macAddr48;
    GT_U32  firstMissStatus = 0;
    GT_BOOL getFirst;  /* indication to get first entry from the DB */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr;

    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */
    GT_U32  maxTotalEntries;

    macAddr48.l[0] = 1;
    macAddr48.l[1] = 0;

    CLEAR_VAR_MAC(capacityInfo );
    CLEAR_VAR_MAC(entryAttrInfo);
    CLEAR_VAR_MAC(learningInfo );
    CLEAR_VAR_MAC(lookupInfo   );
    CLEAR_VAR_MAC(agingInfo    );

    /* lets check 'sizeof' */
    PRINT_SIZEOF_MAC(PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC);
    PRINT_SIZEOF_MAC(PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV4_UC_FORMAT_STC);
    PRINT_SIZEOF_MAC(PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC);

    FDB_SIZE_FULL_INIT();

    switch (numOfHashes)
    {
        case 4:
            maxTotalEntries = _32K;
            break;
        case 8:
            maxTotalEntries = _64K;
            break;
        default:
            maxTotalEntries = FDB_SIZE_FULL;
    }

    capacityInfo.hwCapacity.numOfHwIndexes = maxTotalEntries;
    capacityInfo.hwCapacity.numOfHashes    = numOfHashes;
    capacityInfo.maxTotalEntries           = maxTotalEntries;
    capacityInfo.maxEntriesPerAgingScan    = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerDeleteScan   = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerTransplantScan = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerLearningScan = NUM_ENTRIES_STEPS_CNS; /*256*/

    entryAttrInfo.macEntryMuxingMode  = CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E;
    entryAttrInfo.ipmcEntryMuxingMode = CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E;
    entryAttrInfo.saDropCommand       = saDropCommand;
    entryAttrInfo.daDropCommand       = CPSS_PACKET_CMD_DROP_HARD_E;
    entryAttrInfo.ipNhPacketcommand   = CPSS_PACKET_CMD_ROUTE_E;

    learningInfo.macNoSpaceUpdatesEnable = GT_FALSE;
    learningInfo.macRoutedLearningEnable = GT_FALSE;
    learningInfo.macVlanLookupMode       = CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E;

    lookupInfo.crcHashUpperBitsMode     = CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;
    lookupInfo.ipv4PrefixLength         = 32;
    lookupInfo.ipv6PrefixLength         = 128;

    agingInfo.destinationUcRefreshEnable = GT_FALSE;
    agingInfo.destinationMcRefreshEnable = GT_FALSE;
    agingInfo.ipUcRefreshEnable          = GT_FALSE;

    rc = cpssDxChBrgFdbManagerCreate(fdbManagerId,
        &capacityInfo,&entryAttrInfo,&learningInfo,
        &lookupInfo,&agingInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* remove exiting devices ! (maybe existing maybe not ... from previous run of test) */
    (void)cpssDxChBrgFdbManagerDevListRemove(fdbManagerId,devListArr,numOfDevs);

    if(withHw)
    {
        rc = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    fdbEntry.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
    fdbEntryMacAddrFormatPtr = &fdbEntry.format.fdbEntryMacAddrFormat;
    fdbEntryMacAddrFormatPtr->mirrorToAnalyzerPort   = GT_FALSE;
    fdbEntryMacAddrFormatPtr->daQoSParameterSetIndex = 0;
    fdbEntryMacAddrFormatPtr->saQoSParameterSetIndex = 0;
    fdbEntryMacAddrFormatPtr->fid                = 1;
    fdbEntryMacAddrFormatPtr->vid1               = 0;   /* muxing mode must be 0 */
    fdbEntryMacAddrFormatPtr->daSecurityLevel = 0;      /* muxing mode must be 0 */
    fdbEntryMacAddrFormatPtr->saSecurityLevel = 1;
    fdbEntryMacAddrFormatPtr->sourceID        = 0xABC;
    fdbEntryMacAddrFormatPtr->userDefined     = 0;      /* muxing mode must be 0 */
    fdbEntryMacAddrFormatPtr->daCommand       = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    fdbEntryMacAddrFormatPtr->saCommand       = saDropCommand  ;
    fdbEntryMacAddrFormatPtr->isStatic        = GT_FALSE;
    fdbEntryMacAddrFormatPtr->age             = GT_TRUE;
    fdbEntryMacAddrFormatPtr->daRoute         = GT_FALSE;
    fdbEntryMacAddrFormatPtr->appSpecificCpuCode = GT_TRUE;

    fdbEntryAddInfo.tempEntryExist  = GT_TRUE;
    fdbEntryAddInfo.rehashEnable    = GT_FALSE;


    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);
    fdbManagerPtr->totalStages = stages;
    fdbEntryAddInfo.tempEntryOffset = 2;

    cpssOsPrintf("fdbManagerPtr->totalStages[%d]\n", fdbManagerPtr->totalStages);


    START_TIME;

    numSuccsses = 0;
    iiMax = _32K * (numOfHashes >> 2);
    for(ii = 0 ; ii < iiMax; ii++)
    {
        macAddressGet(ii, &macAddr48, isRand);

        fdbEntryMacAddrFormatPtr->fid                = (1 + ii) & 0x1fff;

        fdbEntryMacAddrFormatPtr->macAddr.arEther[0] = (GT_U8)((macAddr48.l[1] >> 8));
        fdbEntryMacAddrFormatPtr->macAddr.arEther[1] = (GT_U8)((macAddr48.l[1] >> 0));

        fdbEntryMacAddrFormatPtr->macAddr.arEther[2] = (GT_U8)((macAddr48.l[0] >> 24));
        fdbEntryMacAddrFormatPtr->macAddr.arEther[3] = (GT_U8)((macAddr48.l[0] >> 16));
        fdbEntryMacAddrFormatPtr->macAddr.arEther[4] = (GT_U8)((macAddr48.l[0] >>  8));
        fdbEntryMacAddrFormatPtr->macAddr.arEther[5] = (GT_U8)((macAddr48.l[0]));

        fdbEntryMacAddrFormatPtr->dstInterface.type = ii & 0x3;/* port/trunk/vid/vidx */

        switch(fdbEntryMacAddrFormatPtr->dstInterface.type)
        {
            case CPSS_INTERFACE_PORT_E:
                fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.devPort.hwDevNum = (ii + 0x10) & 0x03ff;
                fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.devPort.portNum  = (ii + 0x55) & 0x1fff;
                break;
            case CPSS_INTERFACE_TRUNK_E:
                fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.trunkId = (ii + 0x22) & 0x03ff;
                break;
            default: /* vidx */
                fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.vidx    = (ii + 0x11) & 0x03ff;
                break;
        }

        fdbEntryMacAddrFormatPtr->sourceID        = (0xABC + ii) & 0xFFF;

        rc = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId,&fdbEntry,&fdbEntryAddInfo);
        if(rc == GT_ALREADY_EXIST)
        {
            /*cpssOsPrintf("x");*/
        }
        else
        if(rc != GT_OK)
        {
            if (rc == GT_FULL)
            {
                if (firstMissStatus == 0)
                {
                    if (rehashEnable)
                    {
                        fdbEntryAddInfo.rehashEnable    = GT_TRUE;
                    }
                    firstMissStatus                 = 1;
                    cpssOsPrintf("Native 'ADD' first miss capacity %d%%\n", (ii * 100) / iiMax);
                }
                else
                {
                    if (rehashEnable)
                    {
                        if (firstMissStatus == 1)
                        {
                            firstMissStatus = 2;
                            cpssOsPrintf("Cuckoo 'ADD' first miss capacity %d%%\n", (ii * 100) / iiMax);
                        }
                    }
                }
                continue;
            }
            cpssOsPrintf("rc %d\n", rc);
            return rc;
        }
        else
        {
            numSuccsses++;
        }
    }

    HOW_MUCH_TIME(cpssDxChBrgFdbManagerEntryAdd,numSuccsses);

    if (rehashEnable == GT_FALSE)
    {
        cpssOsPrintf("Native 'ADD' total capacity %d%%\n", (numSuccsses * 100) / iiMax);
    }
    else
    {
        cpssOsPrintf("Cuckoo 'ADD' total capacity %d%%\n", (numSuccsses * 100) / iiMax);
    }

    if (doPrint)
    {
        rc = prvCpssDxChFdbManagerDebugPrintCounters(fdbManagerId);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    START_TIME;

    macAddr48.l[0] = 1;
    macAddr48.l[1] = 0;

    numSuccsses = 0;
    /* delete all entries */

    getFirst = GT_TRUE;

    /* remove all entries from the DB (one by one) */
    while(GT_OK == cpssDxChBrgFdbManagerEntryGetNext(fdbManagerId, getFirst, &fdbEntry))
    {
        getFirst = GT_FALSE;
        rc = cpssDxChBrgFdbManagerEntryDelete(fdbManagerId, &fdbEntry);
        if (rc != GT_OK)
        {
            if (rc == GT_NOT_FOUND)
            {
                continue;
            }
            return rc;
        }
        else
        {
            numSuccsses++;
        }
    }

    HOW_MUCH_TIME(cpssDxChBrgFdbManagerEntryDelete,numSuccsses);

    if(doPrint)
    {
        rc = prvCpssDxChFdbManagerDebugPrintCounters(fdbManagerId);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChFdbManagerDebugPrintStatistics(fdbManagerId);
        if(rc != GT_OK)
        {
            return rc;
        }

    }

    if(withHw)
    {
        rc = cpssDxChBrgFdbManagerDevListRemove(fdbManagerId,devListArr,numOfDevs);
        if(rc != GT_OK)
        {
            return rc;
        }

        START_TIME;

        rc = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
        if(rc != GT_OK)
        {
            return rc;
        }

        HOW_MUCH_TIME(cpssDxChBrgFdbManagerDevListAdd,iiMax);
    }

    CLEANUP_MANAGER(fdbManagerId);

    return GT_OK;
}

GT_STATUS   prvCpssDxChFdbManagerRehashingTest()
{
    GT_U32  rc;
    GT_U32  validNumOfHashes[]= {4,  8,  16};
    GT_U32  validNumOfStages[]= {5,  3,   2};
    GT_U32  ii;

    for (ii = 0; ii < 3; ii++)
    {
        rc = prvCpssDxChFdbManagerTest(1, 0, validNumOfHashes[ii], GT_FALSE, validNumOfStages[ii]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

GT_STATUS   prvCpssDxChFdbManagerTest_HW_accessTime(void)
{
    GT_STATUS   rc;
    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */
    GT_U8   devNum;
    GT_U32  numSuccsses;
    GT_U32  hwIndex;
    GT_U32  wordsArr[4];

    devNum = 0;

    numSuccsses = 0;
    START_TIME;
    for(hwIndex = 0 ; hwIndex < FDB_SIZE_FULL ; hwIndex += 128)
    {
        wordsArr[0] = 0x11223344 + hwIndex;
        wordsArr[1] = 0x55667788 + hwIndex;
        wordsArr[2] = 0x99aabbcc + hwIndex;
        wordsArr[3] = 0xddeeff00 + hwIndex;

        writeEntry(devNum,hwIndex,wordsArr);
        numSuccsses++;
    }
    HOW_MUCH_TIME(prvCpssDxChPortGroupWriteTableEntry,numSuccsses);

    return GT_OK;
}

static GT_STATUS   prvCpssDxChFdbManagerUpdateFdbMacEntryTest(IN GT_U32 fdbManagerId, IN GT_U32 doPrint, IN GT_U32 iiMax)
{
    GT_STATUS   rc;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              fdbEntry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC *fdbEntryMacAddrFormatPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC fdbEntryUpdateInfo;
    GT_U32  ii;
    GT_U32  numSuccsses;

    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */

    fdbEntry.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
    fdbEntryMacAddrFormatPtr = &fdbEntry.format.fdbEntryMacAddrFormat;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[0] = 0x00;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[1] = 0x11;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[2] = 0x22;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[3] = 0x33;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[4] = 0x44;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[5] = 0x55;
    fdbEntryMacAddrFormatPtr->fid                = 1;
    fdbEntryMacAddrFormatPtr->vid1               = 0;   /* muxing mode must be 0 */
    fdbEntryMacAddrFormatPtr->daSecurityLevel = 0;      /* muxing mode must be 0 */
    fdbEntryMacAddrFormatPtr->saSecurityLevel = 1;
    fdbEntryMacAddrFormatPtr->sourceID        = 0xABC;
    fdbEntryMacAddrFormatPtr->userDefined     = 0;      /* muxing mode must be 0 */
    fdbEntryMacAddrFormatPtr->daCommand       = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    fdbEntryMacAddrFormatPtr->saCommand       = CPSS_PACKET_CMD_DROP_SOFT_E;
    fdbEntryMacAddrFormatPtr->isStatic        = GT_FALSE;
    fdbEntryMacAddrFormatPtr->age             = GT_TRUE;
    fdbEntryMacAddrFormatPtr->daRoute         = GT_FALSE;
    fdbEntryMacAddrFormatPtr->appSpecificCpuCode = GT_TRUE;

    fdbEntryAddInfo.tempEntryExist  = GT_TRUE;
    fdbEntryAddInfo.rehashEnable    = GT_FALSE;

    START_TIME;

    numSuccsses = 0;
    for(ii = 0 ; ii < iiMax; ii++)
    {
        fdbEntryAddInfo.tempEntryOffset = BANK_AS_PER_DEVICE((ii + 9) & 0xF);/*bankId*/

        fdbEntryMacAddrFormatPtr->fid                = 1 + ii;

        fdbEntryMacAddrFormatPtr->macAddr.arEther[3] = (GT_U8)((ii >> 16) + 0x33);
        fdbEntryMacAddrFormatPtr->macAddr.arEther[4] = (GT_U8)((ii >>  8) + 0x44);
        fdbEntryMacAddrFormatPtr->macAddr.arEther[5] = (GT_U8)((ii >>  0) + 0x55);

        fdbEntryMacAddrFormatPtr->dstInterface.type = ii & 0x3;/* port/trunk/vid/vidx */

        switch(fdbEntryMacAddrFormatPtr->dstInterface.type)
        {
            case CPSS_INTERFACE_PORT_E:
                fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.devPort.hwDevNum = (ii + 0x10) & 0x03ff;
                fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.devPort.portNum  = (ii + 0x55) & 0x1fff;
                break;
            case CPSS_INTERFACE_TRUNK_E:
                fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.trunkId = (ii + 0x22) & 0x03ff;
                break;
            default: /* vidx */
                fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.vidx    = (ii + 0x11) & 0x03ff;
                break;
        }

        fdbEntryMacAddrFormatPtr->sourceID        = (0xABC + ii) & 0xFFF;


        rc = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId,&fdbEntry,&fdbEntryAddInfo);
        if(rc == GT_ALREADY_EXIST)
        {
            /*cpssOsPrintf("x");*/
        }
        else
        if(rc != GT_OK)
        {
            return rc;
        }
        else
        {
            numSuccsses++;
        }
    }

    HOW_MUCH_TIME(cpssDxChBrgFdbManagerEntryAdd,numSuccsses);

    if(doPrint)
    {
        rc = prvCpssDxChFdbManagerDebugPrintValidEntries(fdbManagerId,1/*withMetadata*/);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    START_TIME;

    numSuccsses = 0;
    cpssOsMemSet(&fdbEntry.format.fdbEntryMacAddrFormat, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC));
    fdbEntryMacAddrFormatPtr->macAddr.arEther[0] = 0x00;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[1] = 0x11;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[2] = 0x22;

    /* update the entries */
    for(ii = 0 ; ii < iiMax; ii++)
    {
        /* start -- the parts of the key */

        fdbEntryMacAddrFormatPtr->fid                = 1 + ii;

        fdbEntryMacAddrFormatPtr->macAddr.arEther[3] = (GT_U8)((ii >> 16) + 0x33);
        fdbEntryMacAddrFormatPtr->macAddr.arEther[4] = (GT_U8)((ii >>  8) + 0x44);
        fdbEntryMacAddrFormatPtr->macAddr.arEther[5] = (GT_U8)((ii >>  0) + 0x55);
        /* end -- the parts of the key */

        rc = cpssDxChBrgFdbManagerEntryGet(fdbManagerId, &fdbEntry);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Update source ID */
        fdbEntryMacAddrFormatPtr->sourceID           = (fdbEntryMacAddrFormatPtr->sourceID + 10) & 0xFFF;

        fdbEntryUpdateInfo.updateOnlySrcInterface = (ii % 2) ? GT_TRUE : GT_FALSE;

        switch(fdbEntryMacAddrFormatPtr->dstInterface.type)
        {
            case CPSS_INTERFACE_PORT_E:
                fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.devPort.hwDevNum = (fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.devPort.hwDevNum + 10) & 0x03ff;
                fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.devPort.portNum  = (fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.devPort.portNum + 10) & 0x1fff;
                break;
            case CPSS_INTERFACE_TRUNK_E:
                fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.trunkId = (fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.trunkId + 10) & 0x03ff;
                break;
            default: /* vidx */
                fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.vidx    = (fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.vidx + 10) & 0x03ff;
                break;
        }

        rc = cpssDxChBrgFdbManagerEntryUpdate(fdbManagerId, &fdbEntry, &fdbEntryUpdateInfo);
        if(rc != GT_OK)
        {
            return rc;
        }
        else
        {
            numSuccsses++;
        }
    }

    HOW_MUCH_TIME(cpssDxChBrgFdbManagerEntryDelete,numSuccsses);

    if(doPrint)
    {
        rc = prvCpssDxChFdbManagerDebugPrintValidEntries(fdbManagerId,1/*withMetadata*/);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

GT_STATUS   prvCpssDxChFdbManagerUpdateFdbEntryTest(IN GT_U32 withHw ,
                                                    IN GT_U32 doPrint,
                                                    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT entryType,
                                                    IN GT_U32 iiMax)
{
    GT_STATUS   rc;
    CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           capacityInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           learningInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             lookupInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              agingInfo;
    GT_U8   devListArr[1] = {0};
    GT_U32  numOfDevs = 1;
    GT_U32  fdbManagerId = 7;

    if(!MANAGER_GET_MAC(fdbManagerId))
    {
        /* lets check 'sizeof' */
        PRINT_SIZEOF_MAC(PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC);
        PRINT_SIZEOF_MAC(PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV4_UC_FORMAT_STC);
        PRINT_SIZEOF_MAC(PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC);
        PRINT_SIZEOF_MAC(PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_DATA_FORMAT_STC);
        PRINT_SIZEOF_MAC(PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_KEY_FORMAT_STC);
        PRINT_SIZEOF_MAC(PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC);

        CLEAR_VAR_MAC(capacityInfo );
        CLEAR_VAR_MAC(entryAttrInfo);
        CLEAR_VAR_MAC(learningInfo );
        CLEAR_VAR_MAC(lookupInfo   );
        CLEAR_VAR_MAC(agingInfo    );

        capacityInfo.hwCapacity.numOfHwIndexes = FDB_SIZE_FULL;
        capacityInfo.hwCapacity.numOfHashes    = HASH_AS_PER_DEVICE(16);
        capacityInfo.maxTotalEntries           = 1024;
        capacityInfo.maxEntriesPerAgingScan    = NUM_ENTRIES_STEPS_CNS; /*256*/
        capacityInfo.maxEntriesPerDeleteScan   = NUM_ENTRIES_STEPS_CNS; /*256*/
        capacityInfo.maxEntriesPerTransplantScan = NUM_ENTRIES_STEPS_CNS; /*256*/
        capacityInfo.maxEntriesPerLearningScan = NUM_ENTRIES_STEPS_CNS; /*256*/

        entryAttrInfo.macEntryMuxingMode  = CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E;
        entryAttrInfo.ipmcEntryMuxingMode = CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E;
        entryAttrInfo.saDropCommand       = CPSS_PACKET_CMD_DROP_SOFT_E;
        entryAttrInfo.daDropCommand       = CPSS_PACKET_CMD_DROP_HARD_E;
        entryAttrInfo.ipNhPacketcommand   = CPSS_PACKET_CMD_ROUTE_E;

        learningInfo.macNoSpaceUpdatesEnable = GT_FALSE;
        learningInfo.macRoutedLearningEnable = GT_FALSE;
        learningInfo.macVlanLookupMode       = CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E;

        lookupInfo.crcHashUpperBitsMode     = CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;
        lookupInfo.ipv4PrefixLength         = 32;
        lookupInfo.ipv6PrefixLength         = 128;

        agingInfo.destinationUcRefreshEnable = GT_FALSE;
        agingInfo.destinationMcRefreshEnable = GT_FALSE;
        agingInfo.ipUcRefreshEnable          = GT_FALSE;

        rc = cpssDxChBrgFdbManagerCreate(fdbManagerId,
            &capacityInfo,&entryAttrInfo,&learningInfo,
            &lookupInfo,&agingInfo);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* remove exiting devices ! (maybe existing maybe not ... from previous run of test) */
    (void)cpssDxChBrgFdbManagerDevListRemove(fdbManagerId,devListArr,numOfDevs);

    if(withHw)
    {
        rc = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    switch (entryType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
            rc = prvCpssDxChFdbManagerUpdateFdbMacEntryTest(fdbManagerId, doPrint, iiMax);
            if(rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(withHw)
    {
        rc = cpssDxChBrgFdbManagerDevListRemove(fdbManagerId,devListArr,numOfDevs);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

GT_STATUS   prvCpssDxChFdbManagerAacTest(GT_U32 writeAac)
{
    GT_STATUS   rc;
    CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           capacityInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           learningInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             lookupInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              agingInfo;
    CPSS_PACKET_CMD_ENT saDropCommand               = CPSS_PACKET_CMD_DROP_SOFT_E;
    GT_U8   devListArr[1] = {0};
    GT_U32  numOfDevs = 1;
    GT_U32  fdbManagerId = 7;
    GT_U32  ii,iiMax,numSuccsses;
    GT_U32  hwIndex = 100;
    GT_U32  wordsArr[4] = {0x55555555, 0xeeeeeeee, 0xdddddddd, 0x33333333};

    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */


    if(!MANAGER_GET_MAC(fdbManagerId))
    {
        CLEAR_VAR_MAC(capacityInfo );
        CLEAR_VAR_MAC(entryAttrInfo);
        CLEAR_VAR_MAC(learningInfo );
        CLEAR_VAR_MAC(lookupInfo   );
        CLEAR_VAR_MAC(agingInfo    );

        capacityInfo.hwCapacity.numOfHwIndexes = FDB_SIZE_FULL;
        capacityInfo.hwCapacity.numOfHashes    = HASH_AS_PER_DEVICE(16);
        capacityInfo.maxTotalEntries           = capacityInfo.hwCapacity.numOfHwIndexes;
        capacityInfo.maxEntriesPerAgingScan    = NUM_ENTRIES_STEPS_CNS; /*256*/
        capacityInfo.maxEntriesPerDeleteScan   = NUM_ENTRIES_STEPS_CNS; /*256*/
        capacityInfo.maxEntriesPerTransplantScan = NUM_ENTRIES_STEPS_CNS; /*256*/
        capacityInfo.maxEntriesPerLearningScan = NUM_ENTRIES_STEPS_CNS; /*256*/

        entryAttrInfo.macEntryMuxingMode  = CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E;
        entryAttrInfo.ipmcEntryMuxingMode = CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E;
        entryAttrInfo.saDropCommand       = saDropCommand;
        entryAttrInfo.daDropCommand       = CPSS_PACKET_CMD_DROP_HARD_E;
        entryAttrInfo.ipNhPacketcommand   = CPSS_PACKET_CMD_ROUTE_E;

        learningInfo.macNoSpaceUpdatesEnable = GT_FALSE;
        learningInfo.macRoutedLearningEnable = GT_FALSE;
        learningInfo.macVlanLookupMode       = CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E;

        lookupInfo.crcHashUpperBitsMode     = CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;
        lookupInfo.ipv4PrefixLength         = 32;
        lookupInfo.ipv6PrefixLength         = 128;

        agingInfo.destinationUcRefreshEnable = GT_FALSE;
        agingInfo.destinationMcRefreshEnable = GT_FALSE;
        agingInfo.ipUcRefreshEnable          = GT_FALSE;

        rc = cpssDxChBrgFdbManagerCreate(fdbManagerId,
            &capacityInfo,&entryAttrInfo,&learningInfo,
            &lookupInfo,&agingInfo);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* remove exiting devices ! (maybe existing maybe not ... from previous run of test) */
    (void)cpssDxChBrgFdbManagerDevListRemove(fdbManagerId,devListArr,numOfDevs);

    rc = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
    if(rc != GT_OK)
    {
        return rc;
    }


    START_TIME;

    iiMax = FDB_SIZE_FULL;
    numSuccsses = 0;

    for(ii = 0 ; ii < iiMax; ii++)
    {
        rc = writeFdbTableEntry(devListArr[0], hwIndex, wordsArr, writeAac);
        if(rc != GT_OK)
        {
            break;
        }

        numSuccsses++;
    }

    if (writeAac)
    {
        HOW_MUCH_TIME(prvCpssDxChFdbManagerAacHwWriteEntry, numSuccsses);
    }
    else
    {
        HOW_MUCH_TIME(prvCpssDxChFdbManagerHwWriteEntry, numSuccsses);
    }

    return rc;
}

GT_VOID  prvCpssDxChFdbManagerAacSpecialTest(GT_U32 numOfHashes, GT_U32 loops)
{
    GT_STATUS st;
    CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           capacityInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           learningInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             lookupInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              agingInfo;
    GT_U8   devListArr[1] = {0};
    GT_U32  numOfDevs = 1;
    GT_U32  fdbManagerId = 7;
    GT_U32  hwIndex;
    GT_U32  wordIndex, iiMax, numSuccsses;
    GT_U32  wordsArr[4], wordsArrGet[4];
    GT_U32 tmpPortGroupId;
    GT_U32 tableType;
    GT_U32 tile, numOfTiles, testLoop;
    GT_U32 addFailed = 0, deleteFailed = 0;

    numOfTiles = PRV_CPSS_PP_MAC(prvTgfDevNum)->multiPipe.numOfTiles;
    numOfTiles = numOfTiles ? numOfTiles : 1;

    FDB_SIZE_FULL_INIT();

    if(!MANAGER_GET_MAC(fdbManagerId))
    {
        CLEAR_VAR_MAC(capacityInfo );
        CLEAR_VAR_MAC(entryAttrInfo);
        CLEAR_VAR_MAC(learningInfo );
        CLEAR_VAR_MAC(lookupInfo   );
        CLEAR_VAR_MAC(agingInfo    );

        switch (numOfHashes)
        {
            case 4:
            case 8:
            case 16:
                break;
            default:
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, GT_BAD_PARAM,
                    "prvCpssDxChFdbManagerAacSpecialTest: wrong number of hashes[%d] on manager [%d]",
                    numOfHashes,
                    fdbManagerId);
        }

        capacityInfo.hwCapacity.numOfHwIndexes = FDB_SIZE_FULL;
        capacityInfo.hwCapacity.numOfHashes    = numOfHashes;
        capacityInfo.maxTotalEntries           = capacityInfo.hwCapacity.numOfHwIndexes;
        capacityInfo.maxEntriesPerAgingScan    = NUM_ENTRIES_STEPS_CNS; /*256*/
        capacityInfo.maxEntriesPerDeleteScan   = NUM_ENTRIES_STEPS_CNS; /*256*/
        capacityInfo.maxEntriesPerTransplantScan = NUM_ENTRIES_STEPS_CNS; /*256*/
        capacityInfo.maxEntriesPerLearningScan = NUM_ENTRIES_STEPS_CNS; /*256*/

        entryAttrInfo.macEntryMuxingMode  = CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E;
        entryAttrInfo.ipmcEntryMuxingMode = CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E;
        entryAttrInfo.saDropCommand       = CPSS_PACKET_CMD_DROP_SOFT_E;
        entryAttrInfo.daDropCommand       = CPSS_PACKET_CMD_DROP_HARD_E;
        entryAttrInfo.ipNhPacketcommand   = CPSS_PACKET_CMD_ROUTE_E;

        learningInfo.macNoSpaceUpdatesEnable = GT_FALSE;
        learningInfo.macRoutedLearningEnable = GT_FALSE;
        learningInfo.macVlanLookupMode       = CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E;

        lookupInfo.crcHashUpperBitsMode     = CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;
        lookupInfo.ipv4PrefixLength         = 32;
        lookupInfo.ipv6PrefixLength         = 128;

        agingInfo.destinationUcRefreshEnable = GT_FALSE;
        agingInfo.destinationMcRefreshEnable = GT_FALSE;
        agingInfo.ipUcRefreshEnable          = GT_FALSE;

        st = cpssDxChBrgFdbManagerCreate(fdbManagerId,
            &capacityInfo,&entryAttrInfo,&learningInfo,
            &lookupInfo,&agingInfo);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerCreate: expected to GT_OK on manager [%d]",
            fdbManagerId);
    }

    /* remove exiting devices ! (maybe existing maybe not ... from previous run of test) */
    (void)cpssDxChBrgFdbManagerDevListRemove(fdbManagerId,devListArr,numOfDevs);

    st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
        fdbManagerId);


    iiMax = FDB_SIZE_FULL;

    for (testLoop = 0; testLoop < loops; testLoop++)
    {
        cpssOsPrintf("\nStart iteration %d from %d\n", testLoop, loops);

        numSuccsses = 0;

        for(hwIndex = 0 ; hwIndex < iiMax; hwIndex++)
        {
            for(wordIndex = 0; wordIndex < 4; wordIndex++)
            {
                wordsArr[wordIndex] = (hwIndex + 1) * 4 + wordIndex;
            }

            if(numOfHashes < 16)
            {
                /* convert if needed the entry index , get specific portGroupId (or all port groups) */
                prvCpssDxChIndexAsPortNumConvert(devListArr[0], CPSS_DXCH_TABLE_FDB_E, hwIndex, &tmpPortGroupId , &hwIndex, &tableType);
            }

            /* Write FDB entry */
            st = prvCpssDxChFdbManagerAacHwWriteEntry(devListArr[0], hwIndex, wordsArr);
            if(st != GT_OK)
            {
                cpssOsPrintf(" prvCpssDxChFdbManagerAacHwWriteEntry Failed %d\n", st);
                break;
            }

            numSuccsses++;
        }

        cpssOsPrintf(" Add [%d] entries - done\n", numSuccsses);

        numSuccsses = 0;
        for(hwIndex = 0; hwIndex < iiMax; hwIndex++)
        {
            /* Build 'get' entry to check */
            for(wordIndex = 0; wordIndex < 4; wordIndex++)
            {
                wordsArr[wordIndex] = (hwIndex + 1) * 4 + wordIndex;
            }

            /* FDB entry has 115 bits - mask not existing bits of last word*/
            wordsArr[3] &= 0x7FFFF;

            for (tile = 0; tile < numOfTiles; tile++)
            {
                /* Read entry from FDB table */
                st = prvCpssDxChPortGroupReadTableEntry(devListArr[0],
                        tile * 2,
                        CPSS_DXCH_TABLE_FDB_E,
                        hwIndex,
                        &wordsArrGet[0]);
                if(st != GT_OK)
                {
                    cpssOsPrintf(" prvCpssDxChPortGroupReadTableEntry Failed %d\n", st);
                }

                if (0 != cpssOsMemCmp(&wordsArr, &wordsArrGet, (4*sizeof(GT_U32))))
                {
                    cpssOsPrintf("prvCpssDxChPortGroupReadTableEntry : Error in tile [%d] entry [%d] data [%d %d %d %d]\n",
                                 tile, hwIndex, wordsArrGet[0], wordsArrGet[1], wordsArrGet[2], wordsArrGet[3]);
                    addFailed++;
                }
            }

            numSuccsses++;
        }

        cpssOsPrintf(" Read and check [%d] entries - done\n", numSuccsses);

        for(hwIndex = 0; hwIndex < iiMax; hwIndex++)
        {
            wordsArr[0] = 0;
            if(numOfHashes < 16)
            {
                /* convert if needed the entry index , get specific portGroupId (or all port groups) */
                prvCpssDxChIndexAsPortNumConvert(devListArr[0], CPSS_DXCH_TABLE_FDB_E, hwIndex, &tmpPortGroupId , &hwIndex, &tableType);
            }
            /* Delete entry from FDB table */
            st = prvCpssDxChFdbManagerAacHwWriteEntry(devListArr[0], hwIndex, wordsArr);
            if (st != GT_OK)
            {
                cpssOsPrintf(" prvCpssDxChFdbManagerAacHwWriteEntry Failed %d\n", st);
                break;
            }
        }

        numSuccsses = 0;
        for(hwIndex = 0; hwIndex < iiMax; hwIndex++)
        {
            for (tile = 0; tile < numOfTiles; tile++)
            {
                st = prvCpssDxChPortGroupReadTableEntry(devListArr[0],
                        tile * 2,
                        CPSS_DXCH_TABLE_FDB_E,
                        hwIndex,
                        &wordsArr[0]);
                if(st != GT_OK)
                {
                    cpssOsPrintf(" prvCpssDxChPortGroupReadTableEntry Failed %d\n", st);
                }

                if (wordsArr[0] != 0)
                {
                    cpssOsPrintf("prvCpssDxChPortGroupReadTableEntry: Delete error for tile %d entry %d received [%d]\n",
                     tile, hwIndex, wordsArr[0]);
                    deleteFailed++;
                }
            }

            numSuccsses++;
        }

        cpssOsPrintf(" Delete and check [%d] entries - done\n", numSuccsses);
    }

    cpssOsPrintf("\n Test done Add Failures %d, Delete Failures %d\n", addFailed, deleteFailed);

    CLEANUP_MANAGER(fdbManagerId);
}

GT_STATUS   prvCpssDxChFdbManagerIpV6CuckooTest(IN GT_U32 withHw , IN GT_U32 doPrint, GT_U32 numOfHashes, GT_32 stages, GT_BOOL isRand)
{
    GT_STATUS   rc;
    CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           capacityInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           learningInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             lookupInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              agingInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              fdbEntry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    CPSS_PACKET_CMD_ENT saDropCommand       = CPSS_PACKET_CMD_DROP_SOFT_E;
    GT_U8   devListArr[1] = {0};
    GT_U32  numOfDevs = 1;
    GT_U32  fdbManagerId = 7;
    GT_U32  ii,iiMax;
    GT_U32  numSuccsses;
    GT_U32  firstMissStatus = 0;
    GT_BOOL getFirst;  /* indication to get first entry from the DB */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr;
    CPSS_OS_FILE    textDump = CPSS_OS_FILE_INVALID;
    CPSS_OS_FILE_TYPE_STC     file;

    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */

    GT_U32  maxTotalEntries;

    CLEAR_VAR_MAC(capacityInfo );
    CLEAR_VAR_MAC(entryAttrInfo);
    CLEAR_VAR_MAC(learningInfo );
    CLEAR_VAR_MAC(lookupInfo   );
    CLEAR_VAR_MAC(agingInfo    );

    if (doPrint)
    {
        textDump = cpssOsFopen("c:/temp/dtemp/vadim.txt","w", &file);
    }

    switch (numOfHashes)
    {
        case 4:
            maxTotalEntries = _32K;
            break;
        case 8:
            maxTotalEntries = _64K;
            break;
        default:
            maxTotalEntries = FDB_SIZE_FULL;
    }

    capacityInfo.hwCapacity.numOfHwIndexes = maxTotalEntries;
    capacityInfo.hwCapacity.numOfHashes    = numOfHashes;
    capacityInfo.maxTotalEntries           = maxTotalEntries;
    capacityInfo.maxEntriesPerAgingScan    = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerDeleteScan   = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerTransplantScan = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerLearningScan = NUM_ENTRIES_STEPS_CNS; /*256*/

    entryAttrInfo.macEntryMuxingMode  = CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E;
    entryAttrInfo.ipmcEntryMuxingMode = CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E;
    entryAttrInfo.saDropCommand       = saDropCommand;
    entryAttrInfo.daDropCommand       = CPSS_PACKET_CMD_DROP_HARD_E;
    entryAttrInfo.ipNhPacketcommand   = CPSS_PACKET_CMD_ROUTE_E;

    learningInfo.macNoSpaceUpdatesEnable = GT_FALSE;
    learningInfo.macRoutedLearningEnable = GT_FALSE;
    learningInfo.macVlanLookupMode       = CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E;

    lookupInfo.crcHashUpperBitsMode     = CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;
    lookupInfo.ipv4PrefixLength         = 32;
    lookupInfo.ipv6PrefixLength         = 128;

    agingInfo.destinationUcRefreshEnable = GT_FALSE;
    agingInfo.destinationMcRefreshEnable = GT_FALSE;
    agingInfo.ipUcRefreshEnable          = GT_FALSE;

    rc = cpssDxChBrgFdbManagerCreate(fdbManagerId,
        &capacityInfo,&entryAttrInfo,&learningInfo,
        &lookupInfo,&agingInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* remove exiting devices ! (maybe existing maybe not ... from previous run of test) */
    (void)cpssDxChBrgFdbManagerDevListRemove(fdbManagerId,devListArr,numOfDevs);

    if(withHw)
    {
        rc = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    fdbEntryAddInfo.rehashEnable    = GT_FALSE;


    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);
    fdbManagerPtr->totalStages = stages;

    cpssOsPrintf("fdbManagerPtr->totalStages[%d]\n", fdbManagerPtr->totalStages);


    START_TIME;

    numSuccsses = 0;
    iiMax = _32K * (numOfHashes >> 2) / 2;

    for(ii = 0; ii < iiMax; ii++)
    {
        ipv6UcAddressGet(ii, &fdbEntry, isRand, GT_FALSE);

        rc = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId, &fdbEntry, &fdbEntryAddInfo);
        if(rc == GT_ALREADY_EXIST)
        {
            /*cpssOsPrintf("x");*/
        }
        else
        if(rc != GT_OK)
        {
            if (rc == GT_FULL)
            {
                if (firstMissStatus == 0)
                {
                    if (rehashEnable)
                    {
                        fdbEntryAddInfo.rehashEnable    = GT_TRUE;
                    }
                    firstMissStatus                 = 1;
                    cpssOsPrintf("Native 'ADD' first miss capacity %d%%\n", (ii * 100) / iiMax);
                }
                else
                {
                    if (rehashEnable)
                    {
                        if (firstMissStatus == 1)
                        {
                            firstMissStatus = 2;
                            cpssOsPrintf("Cuckoo 'ADD' first miss capacity %d%%\n", (ii * 100) / iiMax);
                        }
                    }
                }
                continue;
            }
            cpssOsPrintf("rc %d\n", rc);
            return rc;
        }
        else
        {
            if (textDump != CPSS_OS_FILE_INVALID)
            {
                F_PRINT_IPV6_UC_MAC(textDump, (&fdbEntry));
            }

            numSuccsses++;
        }
    }

    HOW_MUCH_TIME(cpssDxChBrgFdbManagerEntryAdd, numSuccsses);

    if (rehashEnable == GT_FALSE)
    {
        cpssOsPrintf("Native 'ADD' total capacity %d%%\n", (numSuccsses * 100) / iiMax);
    }
    else
    {
        cpssOsPrintf("Cuckoo 'ADD' total capacity %d%%\n", (numSuccsses * 100) / iiMax);
    }

    if (doPrint)
    {
        rc = prvCpssDxChFdbManagerDebugPrintCounters(fdbManagerId);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    START_TIME;

    numSuccsses = 0;

    /* delete all entries */
    getFirst = GT_TRUE;

    /* remove all entries from the DB (one by one) */
    while(GT_OK == cpssDxChBrgFdbManagerEntryGetNext(fdbManagerId, getFirst, &fdbEntry))
    {
        getFirst = GT_FALSE;
        rc = cpssDxChBrgFdbManagerEntryDelete(fdbManagerId, &fdbEntry);
        if (rc != GT_OK)
        {
            if (rc == GT_NOT_FOUND)
            {
                continue;
            }
            return rc;
        }
        else
        {
            if (textDump != CPSS_OS_FILE_INVALID)
            {
                F_PRINT_IPV6_UC_MAC(textDump, (&fdbEntry));
            }

            numSuccsses++;
        }
    }

    HOW_MUCH_TIME(cpssDxChBrgFdbManagerEntryDelete,numSuccsses);

    if(doPrint)
    {
        rc = prvCpssDxChFdbManagerDebugPrintCounters(fdbManagerId);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChFdbManagerDebugPrintStatistics(fdbManagerId);
        if(rc != GT_OK)
        {
            return rc;
        }

    }

    if(withHw)
    {
        rc = cpssDxChBrgFdbManagerDevListRemove(fdbManagerId,devListArr,numOfDevs);
        if(rc != GT_OK)
        {
            return rc;
        }

        START_TIME;

        rc = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
        if(rc != GT_OK)
        {
            return rc;
        }

        HOW_MUCH_TIME(cpssDxChBrgFdbManagerDevListAdd,iiMax);
    }

    CLEANUP_MANAGER(fdbManagerId);

    if (textDump != CPSS_OS_FILE_INVALID)
    {
        cpssOsFclose(&file);
    }

    return GT_OK;
}

void prvCpssDxChFdbManagerIpv6EntryAddTest()
{
    GT_STATUS                                               st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     entry;
    GT_U32                                                  fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    GT_U32                                                  maxTotalEntries;
    UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC        outputResults;
    GT_BOOL                                                 entryAdded;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC          addParams;
    GT_BOOL                                                 isValid;                       /* Indicates Valid non-SP entry */
    GT_U32                                                  hwIndex;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    GT_U32                                                  numEntriesRemoved = 0;

    outputResults.entryPtr                                  = &entry;
    outputResults.entryAddedPtr                             = &entryAdded;


    cpssOsMemSet(&entry ,0,sizeof(entry));

    /* Required as this API is used by lua CLI HA scripts */
    FDB_SIZE_FULL_INIT();

    maxTotalEntries = FDB_SIZE_FULL;

    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId, maxTotalEntries, prvTgfDevNum, NULL, GT_FALSE);
    
    fdbManagerEntryAdd_ipv6Uc(fdbManagerId, 0, &outputResults);

    PRINT_IPV6_UC_MAC(outputResults.entryPtr);

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    for(hwIndex = 0; hwIndex<fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; hwIndex++)
    {
        st = prvCpssDxChFdbManagerHwEntryRead(0, hwIndex, fdbManagerPtr, PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E, &addParams, &isValid);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "prvCpssDxChFdbManagerHwEntryRead: expected to GT_OK on manager [%d]",
                                     fdbManagerId);
        if(isValid)
        {
            st = prvCpssDxChFdbManagerDbBuildDbFormatFromHwEntry(fdbManagerPtr, PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E, &entry);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "prvCpssDxChFdbManagerDbBuildDbFormatFromHwEntry: expected to GT_OK on manager [%d]",
                                         fdbManagerId);

            PRINT_IPV6_UC_MAC(outputResults.entryPtr);

            fdbManagerHwEntryCheckAllTiles(&entry, numEntriesRemoved);

            st = cpssDxChBrgFdbManagerEntryDelete(fdbManagerId, &entry);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChBrgFdbManagerEntryDelete: expected to GT_OK on manager [%d]",
                fdbManagerId);

            numEntriesRemoved++;

        }
    }

    CLEANUP_ALL_MANAGERS;
}

void prvCpssDxChFdbManagerMixedEntriesCuckooTest(GT_BOOL addOrderFlag)
{
    GT_STATUS                                               st;
    GT_U32                                                  numEntriesRemoved;/* total number of entries removed from the DB */
    GT_U32                                                  numEntriesAdded;  /* total number of entries added to the DB */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     entry;
    GT_BOOL                                                 getFirst;  /* indication to get first entry from the DB */
    GT_U32                                                  fdbManagerId = TESTED_FDB_MANAGER_ID_CNS;
    GT_U32                                                  maxTotalEntries;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT                fdbEntryType;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNum, errorNumItr;
    GT_U32                                                  numOfMacIPv4Entries = 43690;
    GT_U32                                                  numOfIPv6Entries = 10922;

    cpssOsMemSet(&entry ,0,sizeof(entry));

    /* Required as this API is used by lua CLI HA scripts */
    FDB_SIZE_FULL_INIT();

    numEntriesAdded = 0;
    maxTotalEntries = FDB_SIZE_FULL;

    if(debug_limited_entries)
    {
        maxTotalEntries = debug_limited_entries;
    }

    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId, maxTotalEntries, prvTgfDevNum, NULL, GT_FALSE);
    
    PRV_UTF_LOG1_MAC("Add [%d] MAC entries \n", numOfMacIPv4Entries);
        
    fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;

    st = prvTgfBrgFdbManagerAddEntries(fdbManagerId, numOfMacIPv4Entries, fdbEntryType, GT_TRUE, &numEntriesAdded);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    if(addOrderFlag == GT_TRUE)
    {
        PRV_UTF_LOG1_MAC("Add [%d] IPv6 UC entries \n", numOfIPv6Entries);

        fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E;

        st = prvTgfBrgFdbManagerAddEntries(fdbManagerId, numOfIPv6Entries, fdbEntryType, GT_TRUE, &numEntriesAdded);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        PRV_UTF_LOG1_MAC("Add [%d] IPv4 UC entries \n", numOfMacIPv4Entries);

        fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E;

        st = prvTgfBrgFdbManagerAddEntries(fdbManagerId, numOfMacIPv4Entries, fdbEntryType, GT_TRUE, &numEntriesAdded);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }
    else
    {
        PRV_UTF_LOG1_MAC("Add [%d] IPv4 UC entries \n", numOfMacIPv4Entries);

        fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E;

        st = prvTgfBrgFdbManagerAddEntries(fdbManagerId, numOfMacIPv4Entries, fdbEntryType, GT_TRUE, &numEntriesAdded);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        PRV_UTF_LOG1_MAC("Add [%d] IPv6 UC entries \n", numOfIPv6Entries);

        fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E;

        st = prvTgfBrgFdbManagerAddEntries(fdbManagerId, numOfIPv6Entries, fdbEntryType, GT_TRUE, &numEntriesAdded);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    if(debug_limited_entries)
    {
        cpssDxChFdbDump(prvTgfDevNum);
    }

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);

    /* Verify DB */
    st = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    numEntriesRemoved = 0;
    getFirst = GT_TRUE;

    PRV_UTF_LOG1_MAC("start - remove the [%d] entries \n",numEntriesAdded);
    if(check_hw_hold_entry == GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("(slow mode) for each entry check that HW hold the entry in proper index according to it's index \n");
    }
    /* remove all entries from the DB (one by one) */
    while(GT_OK == cpssDxChBrgFdbManagerEntryGetNext(fdbManagerId,getFirst,&entry))
    {
        getFirst = GT_FALSE;

        if(check_hw_hold_entry == GT_TRUE)
        {
            if (check_hw_hold_entry_all_tiles)
            {
                /* just before we going to remove the entry ... lets check that it exists in all tiles the HW ! */
                fdbManagerHwEntryCheckAllTiles(&entry, numEntriesRemoved);
            }
            else
            {
                /* just before we going to remove the entry ... lets check that it exists in the HW ! */
                fdbManagerHwEntryCheck(&entry);
            }
        }

        FDB_MANAGER_ENTRY_DELETE_WITH_STATISTICS(fdbManagerId,&entry,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerEntryDelete: expected to GT_OK on manager [%d]",
            fdbManagerId);
        numEntriesRemoved++;

        if((numEntriesRemoved % 10000) == 0)
        {
            PRV_UTF_LOG1_MAC("removed [%d] entries \n",numEntriesRemoved);
        }
    }

    PRV_UTF_LOG2_MAC("ended - removed [%d] entries (out of expected [%d]) \n",numEntriesRemoved,numEntriesAdded);

    UTF_VERIFY_EQUAL2_STRING_MAC(numEntriesAdded, numEntriesRemoved,
        "expected number of removed entries [%d] to be the same as the added entries[%d]",
        numEntriesRemoved,numEntriesAdded);

    /* Verify DB */
    st = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    PRV_UTF_LOG0_MAC("Ended \n");

    CLEANUP_ALL_MANAGERS;

    return;
}

void prvTgfBrgFdbManagerHighAvailability_Init
(
    IN GT_U32               fdbManagerId,
    IN GT_BOOL              isHaSyncUp
)
{
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                 entry;
    GT_STATUS                                           rc;
    GT_U32                                              maxTotalEntries;
    GT_U32                                              entriesToAdd    = _8K;
    GT_U32                                              numEntriesAdded = 0;
    GT_U32                                              ii;
    GT_BOOL                                             entryAdded;
    UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC    outputResults;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT            fdbEntryType;

    /* Required as this API is used by lua CLI HA scripts */
    FDB_SIZE_FULL_INIT();

    maxTotalEntries = FDB_SIZE_FULL;
    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId, maxTotalEntries, prvTgfDevNum, NULL, GT_FALSE);

    /* Only create device (Do not add any entry), in case of HA syncup */
    if(isHaSyncUp)
    {
        return;
    }

    outputResults.entryPtr                          = &entry;
    outputResults.entryAddedPtr                     = &entryAdded;

    PRV_UTF_LOG1_MAC("start - add [%d] entries \n", entriesToAdd);
    for(ii = 0 ; ii < entriesToAdd; ii++)
    {
        entryAdded = GT_FALSE;
        fdbEntryType = fdbEntryType_valid_arr[ii % UT_FDB_ENTRY_TYPE_VALID_NUM];
        switch(fdbEntryType)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
                fdbManagerEntryAdd_macAddr(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
                fdbManagerEntryAdd_ipv4Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
                fdbManagerEntryAdd_ipv6Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
                fdbManagerEntryAdd_ipv4Uc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
                fdbManagerEntryAdd_ipv6Uc(fdbManagerId, ii, &outputResults);
                break;
            default:
                break;
        }
        if(entryAdded)
        {
            numEntriesAdded++;
        }
    }
    PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
        entriesToAdd, numEntriesAdded, (100*numEntriesAdded)/entriesToAdd);

    /* Backup the current manager content */
    rc = prvCpssDxChBrgFdbManagerBackUp(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvCpssDxChBrgFdbManagerBackUp: GT_OK [%d]", fdbManagerId);
}

void internal_cpssDxChBrgFdbManagerHighAvailabilityEntriesSync_basicUT()
{
    GT_U32                                              fdbManagerId = 2;
    GT_STATUS                                           rc;
    GT_U32                                              errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    CPSS_SYSTEM_RECOVERY_INFO_STC                       oldSystemRecoveryInfo,newSystemRecoveryInfo;
    GT_U32                                              errorNumItr;

    /***********************************************/
    /***** Simulated FDB Manager HA Sequence *******/
    /***********************************************/
    prvTgfBrgFdbManagerHighAvailability_Init(fdbManagerId, GT_FALSE);

    /* Clean FDB Manager */
    rc = prvCpssDxChFdbManagerDbDelete(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvCpssDxChFdbManagerDbDelete: GT_OK [%d]", fdbManagerId);

    /* HA recovery state as CATCH_UP */
    rc = cpssSystemRecoveryStateGet(&oldSystemRecoveryInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "cpssSystemRecoveryStateGet: GT_OK [%d]", fdbManagerId);
    newSystemRecoveryInfo = oldSystemRecoveryInfo;

    newSystemRecoveryInfo.systemRecoveryState   = CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E;
    newSystemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
    prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);

    /* create manager with device */
    prvTgfBrgFdbManagerHighAvailability_Init(fdbManagerId, GT_TRUE);

    /* HW sync-up call */
    rc = cpssDxChBrgFdbManagerHighAvailabilityEntriesSync(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "cpssDxChBrgFdbManagerHighAvailabilityEntriesSync: GT_OK [%d]", fdbManagerId);

    /* Restore HA recovery state */
    prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);

    /* Verify FDB manager entries before and after manager restart */
    rc = prvCpssDxChBrgFdbManagerBackUpInstanceVerify(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvCpssDxChBrgFdbManagerBackUpInstanceVerify: GT_OK [%d]", fdbManagerId);

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }
}

/**
GT_STATUS cpssDxChBrgFdbManagerHighAvailabilityEntriesSync
(
    IN  GT_U32 fdbManagerId
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerHighAvailabilityEntriesSync_basic)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerHighAvailabilityEntriesSync_basicUT();

    CLEANUP_ALL_MANAGERS;
}

void internal_cpssDxChBrgFdbManagerDeleteScan_with_filter(void)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                 entry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC      fdbEntryAddInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC    deleteScanParam;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesDeleteArray;
    GT_U32                                              entriesDeleteNum;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC            statistics;
    GT_U32                                              fdbManagerId = 11;
    GT_U32                                              errorNum, errorNumItr;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT           shadowType = SHADOW_TYPE_PER_DEVICE();

    cpssOsMemSet(&deleteScanParam, 0, sizeof(deleteScanParam));
    cpssOsMemSet(&entry,0,sizeof(entry));
    cpssOsMemSet(&fdbEntryAddInfo,0,sizeof(fdbEntryAddInfo));
    if((shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E) &&
        (shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E))
    {
        /* Lookup Mode - MAC_AND_FID_AND_VID1, Muxing Mode - TAG1_VID */
        set_global_macVlanLookupMode(CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E);
        set_global_macEntryMuxingMode(CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E);
        set_global_entryMuxingMode(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E);

        entry.format.fdbEntryMacAddrFormat.vid1     = 3;
        deleteScanParam.vid1                        = 3;
        deleteScanParam.vid1Mask                    = 0xF;
    }
    else
    {
        /* AC5 does not support vid1 */
        set_global_macVlanLookupMode(CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E);
        deleteScanParam.fid                        = 12;
        deleteScanParam.fidMask                    = 0xFFF;
    }

    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId, FDB_SIZE_FULL, prvTgfDevNum, NULL, GT_FALSE);

    /* Add 2 entry with vid1 3 and 5 or FID - 12 & 15*/
    entry.format.fdbEntryMacAddrFormat.macAddr.arEther[0] = 0x00;
    entry.format.fdbEntryMacAddrFormat.macAddr.arEther[1] = 0x11;
    entry.format.fdbEntryMacAddrFormat.macAddr.arEther[2] = 0x22;
    entry.format.fdbEntryMacAddrFormat.macAddr.arEther[3] = 0x33;
    entry.format.fdbEntryMacAddrFormat.macAddr.arEther[4] = 0x44;
    entry.format.fdbEntryMacAddrFormat.macAddr.arEther[5] = 0x55;
    entry.format.fdbEntryMacAddrFormat.fid                = 12;
    entry.format.fdbEntryMacAddrFormat.dstInterface.type  = CPSS_INTERFACE_VID_E;
    entry.format.fdbEntryMacAddrFormat.daSecurityLevel    = 0;
    entry.format.fdbEntryMacAddrFormat.saSecurityLevel    = 0;
    entry.format.fdbEntryMacAddrFormat.sourceID           = 0;
    entry.format.fdbEntryMacAddrFormat.userDefined        = 0;
    entry.format.fdbEntryMacAddrFormat.daCommand          = CPSS_PACKET_CMD_FORWARD_E;
    entry.format.fdbEntryMacAddrFormat.saCommand          = CPSS_PACKET_CMD_FORWARD_E;
    entry.format.fdbEntryMacAddrFormat.isStatic           = GT_TRUE;
    entry.format.fdbEntryMacAddrFormat.age                = GT_FALSE;
    entry.format.fdbEntryMacAddrFormat.daRoute            = GT_FALSE;
    entry.format.fdbEntryMacAddrFormat.appSpecificCpuCode = GT_FALSE;

    st = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId, &entry, &fdbEntryAddInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryAdd: GT_OK [%d]",
        fdbManagerId);

    entry.format.fdbEntryMacAddrFormat.macAddr.arEther[5] = 0x66;
    entry.format.fdbEntryMacAddrFormat.fid                = 15;
    if((shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E) &&
        (shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E))
    {
        entry.format.fdbEntryMacAddrFormat.vid1               = 5;
    }
    st = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId, &entry, &fdbEntryAddInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChBrgFdbManagerEntryAdd: GT_OK [%d]",
        fdbManagerId);

    deleteScanParam.deleteMacMcEntries      = GT_TRUE;
    deleteScanParam.deleteStaticEntries     = GT_TRUE;
    st = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, GT_TRUE, &deleteScanParam, &entriesDeleteArray, &entriesDeleteNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, st, "cpssDxChBrgFdbManagerDeleteScan: GT_OK");

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);

    /* Verify DB */
    st = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    st = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,st, "cpssDxChBrgFdbManagerStatisticsGet: GT_OK");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, statistics.scanDeleteOk,                    "ERROR: cpssDxChBrgFdbManagerStatisticsGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(1, statistics.scanDeleteOkNoMore,              "ERROR: cpssDxChBrgFdbManagerStatisticsGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, statistics.scanDeleteErrorInputInvalid,     "ERROR: cpssDxChBrgFdbManagerStatisticsGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, statistics.scanDeleteErrorInputInvalid,     "ERROR: cpssDxChBrgFdbManagerStatisticsGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(1, statistics.scanDeleteTotalDeletedEntries,   "ERROR: cpssDxChBrgFdbManagerStatisticsGet");
}

UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerDeleteScan_with_filter)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerDeleteScan_with_filter();

    restore_global_macEntryMuxingMode();
    restore_global_macVlanLookupMode();
    restore_global_entryMuxingMode();

    CLEANUP_ALL_MANAGERS;
}

void internal_cpssDxChBrgFdbManagerAgingScan_with_appDemo(void)
{
    GT_STATUS                                           rc;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                 entry;
    GT_BOOL                                             entryAdded;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC            statistics;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT            fdbEntryType;
    GT_U32                                              fdbManagerId = 11;
    GT_U32                                              ii;
    UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC    outputResults;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC              counters;
    GT_U32                                              agingTime = 60;
    APP_DEMO_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC   appDemoStatistics;

    cpssOsMemSet(&entry ,0,sizeof(entry));

    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;

    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId, FDB_SIZE_FULL, prvTgfDevNum, NULL, GT_FALSE);

    /* Set FDB manager ID for appDemo */
    rc = prvWrAppDemoFdbManagerGlobalIDSet(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerGlobalIDSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Set FDB manager AU Message handler methord as FDB_MANAGER for appDemo */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E,
            GT_TRUE,
            agingTime);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Add 10 entries with different types. */
    for(ii = 0 ; ii <= NUM_ENTRIES_STEPS_CNS*2; ii++)
    {
        entryAdded = GT_FALSE;

        fdbEntryType = fdbEntryType_valid_arr[ii % UT_FDB_ENTRY_TYPE_VALID_NUM];

        switch(fdbEntryType)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
                fdbManagerEntryAdd_macAddr(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
                fdbManagerEntryAdd_ipv4Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
                fdbManagerEntryAdd_ipv6Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
                fdbManagerEntryAdd_ipv4Uc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
                fdbManagerEntryAdd_ipv6Uc(fdbManagerId, ii, &outputResults);
                break;
            default:
                break;
        }
    }

    /* Wait till entries get expired */
    cpssOsTimerWkAfter((2*agingTime + 1) * 1000);

    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbManagerStatisticsGet: GT_OK");

    rc = cpssDxChBrgFdbManagerCountersGet(fdbManagerId, &counters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbManagerCountersGet: GT_OK");

    /* All entries should be flushed out */
    UTF_VERIFY_EQUAL0_STRING_MAC(counters.usedEntries, 0, "cpssDxChBrgFdbManagerCountersGet: GT_OK");

    /* Verify no errors encountered in aging task - Aging task statistics */
    rc = prvWrAppDemoFdbManagerAutoAgingStatisticsGet(&appDemoStatistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: prWrAppDemoFdbManagerAutoAgingStatisticsGet", prvTgfDevNum);

    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, appDemoStatistics.autoAgingOk,           "autoAging Statistics: Not as expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, appDemoStatistics.autoAgingErrorInputInvalid,"autoAging Statistics: Not as expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, appDemoStatistics.autoAgingErrorFail,        "autoAging Statistics: Not as expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, appDemoStatistics.autoAgingErrorHwError,     "autoAging Statistics: Not as expected");

    /* Restore Event Handler logic to be regular (default) one. */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E,
            GT_FALSE,
            0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);
}

UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerAgingScan_with_appDemo)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    internal_cpssDxChBrgFdbManagerAgingScan_with_appDemo();

    CLEANUP_ALL_MANAGERS;
}

typedef GT_STATUS DXCH_DATA_INTEGRITY_EVENT_CB_FUNC
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
);
extern DXCH_DATA_INTEGRITY_EVENT_CB_FUNC        *dxChDataIntegrityEventIncrementFunc;
static DXCH_DATA_INTEGRITY_EVENT_CB_FUNC        *savedDataIntegrityHandler;
#define FDB_MANAGER_SER_INDEX_ARRAY_SIZE        10
static GT_U32                                   indexesArray[FDB_MANAGER_SER_INDEX_ARRAY_SIZE];
static GT_U32                                   indexesNum;
static CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC     errorLocationInfo;

GT_VOID prvCpssDxChBrgFdbManagerEntryRewrite_SER_errorInject
(
    IN GT_BOOL    status
)
{
    GT_STATUS                               rc;
    CPSS_EVENT_MASK_SET_ENT                 mask;

    mask = (status == GT_TRUE)?CPSS_EVENT_UNMASK_E:CPSS_EVENT_MASK_E;

    /* AUTODOC: Unmask/Mask interrupts from the DFX */
    rc = cpssDxChDiagDataIntegrityEventMaskSet(prvTgfDevNum, &errorLocationInfo,
            CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E,
            mask);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: enable/disabe error injection to FDB table */
    rc = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(prvTgfDevNum,
        &errorLocationInfo,
        CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E,
        status);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}

/**
* @internal hwInfoEventErrorHandlerFunc function
* @endinternal
*
* @brief   Callback called in case of data integrity error.
*          Store HW related info from event to global structure.
* @param[in] dev                      - the device.
* @param[in] eventPtr                 - filled structure which describes data integrity error event
*/
static GT_STATUS hwInfoEventErrorHandlerFunc
(
    IN GT_U8                                     dev,
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr

)
{
    GT_U32                                              ii;
    CPSS_NULL_PTR_CHECK_MAC(eventPtr);
    if(dev >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        /* check device out of range */
        return GT_BAD_PARAM;
    }
    if(indexesNum > FDB_MANAGER_SER_INDEX_ARRAY_SIZE-1)
    {
        PRV_UTF_LOG0_MAC("indexesArray : index out of range");
        return GT_BAD_PARAM;
    }

    CPSS_ZERO_LEVEL_API_LOCK_MAC
    /* Add the HW indexes to the index array - will be processed by the correction API */
    for(ii=0; ii<indexesNum; ii++)
    {
        if(indexesArray[ii] == eventPtr->location.hwEntryInfo.hwTableEntryIndex)
        {
            CPSS_ZERO_LEVEL_API_UNLOCK_MAC
            return GT_OK;
        }
    }
    indexesArray[indexesNum] = eventPtr->location.hwEntryInfo.hwTableEntryIndex;
    indexesNum+=1;
    CPSS_ZERO_LEVEL_API_UNLOCK_MAC

    PRV_UTF_LOG2_MAC("Interrupt received for HW table index : 0x%x - portGroup 0x%x\n",
            eventPtr->location.hwEntryInfo.hwTableEntryIndex,
            eventPtr->location.portGroupsBmp);
    return GT_OK;
}

void internal_cpssDxChBrgFdbManagerEntryRewrite_SER(void)
{
    GT_STATUS                                           rc;
    GT_U32                                              fdbManagerId = 20;
    GT_U32                                              errorNum, errorNumItr;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                              ii;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                 entry;
    GT_U32                                              numEntriesAdded = 0;
    GT_BOOL                                             entryAdded;
    UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC    outputResults;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT            fdbEntryType;
    GT_U32                                              entriesToAdd = 5;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC            statistics;
    GT_U32                                              nextEntryIndex, scanEntries;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC                locationInfo;
    GT_BOOL                                             wasWrapAround;

    cpssOsMemSet(&errorLocationInfo,0,sizeof(errorLocationInfo));
    indexesNum = 0;
    cpssOsMemSet(indexesArray,0,sizeof(indexesArray));
    errorLocationInfo.portGroupsBmp                                                   = 0;
    errorLocationInfo.type                                                            = CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE;
    errorLocationInfo.info.logicalEntryInfo.numOfLogicalTables                        = 1;
    errorLocationInfo.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType       = CPSS_DXCH_LOGICAL_TABLE_FDB_E;
    errorLocationInfo.info.logicalEntryInfo.logicaTableInfo[0].logicalTableEntryIndex = 0;
    errorLocationInfo.info.logicalEntryInfo.logicaTableInfo[0].numEntries             = 1;
    savedDataIntegrityHandler                                                         = dxChDataIntegrityEventIncrementFunc;
    dxChDataIntegrityEventIncrementFunc                                               = hwInfoEventErrorHandlerFunc;

    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId, FDB_SIZE_FULL, prvTgfDevNum, NULL, GT_FALSE);
    outputResults.entryPtr                          = &entry;
    outputResults.entryAddedPtr                     = &entryAdded;

    PRV_UTF_LOG1_MAC("start - add [%d] entries \n", entriesToAdd);
    for(ii = 0 ; ii < entriesToAdd; ii++)
    {
        entryAdded = GT_FALSE;
        fdbEntryType = fdbEntryType_valid_arr[ii % UT_FDB_ENTRY_TYPE_VALID_NUM];

        /* Inject error for FDB table */
        prvCpssDxChBrgFdbManagerEntryRewrite_SER_errorInject(GT_TRUE);

        switch(fdbEntryType)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
                fdbManagerEntryAdd_macAddr(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
                fdbManagerEntryAdd_ipv4Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
                fdbManagerEntryAdd_ipv6Mc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
                fdbManagerEntryAdd_ipv4Uc(fdbManagerId, ii, &outputResults);
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
                fdbManagerEntryAdd_ipv6Uc(fdbManagerId, ii, &outputResults);
                break;
            default:
                break;
        }

        if(entryAdded)
        {
            /* numEntriesAdded - also indiacte number of events expected
             * In case of IPv6 2 entries written, so 2 events are expected
             */
            if(fdbEntryType == CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E)
            {
                numEntriesAdded+=2;
            }
            else
            {
                numEntriesAdded+=1;
            }
        }

    }
    PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
        entriesToAdd, numEntriesAdded, (100*numEntriesAdded)/entriesToAdd);

    /* Backup the current manager content */
    rc = prvCpssDxChBrgFdbManagerBackUp(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvCpssDxChBrgFdbManagerBackUp: GT_OK [%d]", fdbManagerId);

#ifdef ASIC_SIMULATION
    {
        /* Collect hwIndex(Valid) & corrupt the HW data not DB */
        PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC      *fdbManagerPtr;
        fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);
        for(indexesNum=0; indexesNum<numEntriesAdded; indexesNum++)
        {
            indexesArray[indexesNum] = fdbManagerPtr->entryPoolPtr[indexesNum].hwIndex;
            PRV_UTF_LOG2_MAC("WM DB Index      :  %x - Entry Type (%d)\n",
                    indexesArray[indexesNum],
                    fdbManagerPtr->entryPoolPtr[indexesNum].hwFdbEntryType);
        }

        rc = prvCpssDxChBrgFdbManagerInjectHwCorruption(fdbManagerId, indexesArray, indexesNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvCpssDxChBrgFdbManagerInjectHwCorruption: GT_OK [%d]", fdbManagerId);
    }
#endif

    /* AUTODOC: scan full FDB table */
    locationInfo.portGroupsBmp = 0xFFFFFFFF;
    locationInfo.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
    locationInfo.info.hwEntryInfo.hwTableType = CPSS_DXCH_TABLE_FDB_E;
    locationInfo.info.hwEntryInfo.hwTableEntryIndex = 0;

    /* scan table by small portions */
    scanEntries = 128;
    wasWrapAround = GT_FALSE;
    nextEntryIndex = 0xFFFFFFFF;

    while((wasWrapAround == GT_FALSE) && (nextEntryIndex >= 128))
    {
        /* AUTODOC: scan whole table */
        rc = cpssDxChDiagDataIntegrityTableScan(prvTgfDevNum, &locationInfo, scanEntries,
                                                &nextEntryIndex, &wasWrapAround);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
        locationInfo.info.hwEntryInfo.hwTableEntryIndex = nextEntryIndex;

        /* provide time for interrupt tasks */
        cpssOsTimerWkAfter(1);
    }

    /* Waiting for the events to come and populate the index array */
    cpssOsTimerWkAfter(2000);

    /* AUTODOC: DISABLE : error injection to FDB table */
    /* DISABLE error injection */
    prvCpssDxChBrgFdbManagerEntryRewrite_SER_errorInject(GT_FALSE);
    dxChDataIntegrityEventIncrementFunc = savedDataIntegrityHandler;

    rc = cpssDxChBrgFdbManagerEntryRewrite(fdbManagerId, indexesArray, indexesNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "cpssDxChBrgFdbManagerEntryRewrite: GT_OK [%d]", fdbManagerId);

    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "cpssDxChBrgFdbManagerStatisticsGet: GT_OK [%d]", fdbManagerId);

    UTF_VERIFY_EQUAL1_STRING_MAC(statistics.entriesRewriteOk, 1,
        "cpssDxChBrgFdbManagerStatisticsGet: rewrite OK - invalid counter", fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(statistics.entriesRewriteErrorInputInvalid, 0,
        "cpssDxChBrgFdbManagerStatisticsGet: rewrite Error input invalid - invalid counter", fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(statistics.entriesRewriteTotalRewrite, indexesNum,
        "cpssDxChBrgFdbManagerStatisticsGet: rewrite total count - invalid counter", fdbManagerId);

    /* Verify FDB manager entries before and after rewrite entries */
    rc = prvCpssDxChBrgFdbManagerBackUpInstanceVerify(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvCpssDxChBrgFdbManagerBackUpInstanceVerify: GT_OK [%d]", fdbManagerId);

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, errorNum);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }
}

UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerEntryRewrite_SER)
{
    /* this is device less API , check if we allow it to run on current device */
    /* SER Test case if based on DI, and its not supported on some devices     */
    {
        GT_U8   dev;
        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~(UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E));
        if (GT_OK != prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            /* the current running device is NOT in the supporting list */
            SKIP_TEST_MAC;
        }
    }
    FDB_SIZE_FULL_INIT();

    internal_cpssDxChBrgFdbManagerEntryRewrite_SER();

    CLEANUP_ALL_MANAGERS;
}

static void internal_cpssDxChBrgFdbManagerHighScale_with_IP_entries()
{
    GT_STATUS                                           rc;
    GT_U32                                              entryCnt;
    GT_U32                                              numEntriesAdded = 0;
    GT_U32                                              fdbManagerId = 20;
    GT_BOOL                                             entryAdded;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                 entry;
    UTF_PRV_CPSS_BRG_FDB_API_OUTPUT_RESULTS_INFO_STC    outputResults;
    GT_U32                                              errorNum, errorNumItr;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];

    outputResults.entryPtr                          = &entry;
    outputResults.entryAddedPtr                     = &entryAdded;

    /* create single valid manager */
    createFdbManagerWithDevice(fdbManagerId, FDB_SIZE_FULL, prvTgfDevNum, NULL, GT_FALSE);

    /* Set FDB manager ID for appDemo */
    rc = prvWrAppDemoFdbManagerGlobalIDSet(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerGlobalIDSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Set FDB manager AU Message handler methord as FDB_MANAGER for appDemo */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Add MAC address - 50% of total capacity */
    for(entryCnt = 0; entryCnt < FDB_SIZE_FULL/2; entryCnt++)
    {
        entryAdded = GT_FALSE;
        fdbManagerEntryAdd_macAddr(fdbManagerId, entryCnt, &outputResults);
        if(entryAdded == GT_TRUE)
        {
            numEntriesAdded+=1;
        }
        if((entryCnt % 2000) == 0)
        {
            PRV_UTF_LOG2_MAC("FDB MAC: numEntriesAdded = %d, entryCnt = %d\n", numEntriesAdded, entryCnt);
        }
    }

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, errorNum);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    /* Add IPv4 UC entries - 50% of total capacity */
    numEntriesAdded = 0;
    for(entryCnt = 0; entryCnt < FDB_SIZE_FULL/2; entryCnt++)
    {
        entryAdded = GT_FALSE;
        fdbManagerEntryAdd_ipv4Uc(fdbManagerId, entryCnt, &outputResults);
        if(entryAdded == GT_TRUE)
        {
            numEntriesAdded+=1;
        }
        if((entryCnt % 2000) == 0)
        {
            PRV_UTF_LOG2_MAC("FDB IP: numEntriesAdded = %d, entryCnt = %d\n", numEntriesAdded, entryCnt);
        }
    }

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(0, errorNum);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    /* Restore the app demo mode */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E,
            GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    return;
}

UTF_TEST_CASE_MAC(cpssDxChBrgFdbManagerHighScale_with_IP_entries)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /* test with Rehash enabled */
    if(!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        SKIP_TEST_MAC
    }

    internal_cpssDxChBrgFdbManagerHighScale_with_IP_entries();

    CLEANUP_ALL_MANAGERS;
}


static GT_U32   allow_TRACE_TEST_ADD_TO_FDB = 0;
/* debug function to allow trace of the test of 'add fdb entries' task */
GT_STATUS fdbMgr_allow_TRACE_TEST_ADD_TO_FDB(IN GT_U32   allowTrace)
{
    allow_TRACE_TEST_ADD_TO_FDB = allowTrace;
    return GT_OK;
}
#define TRACE_TEST_ADD_TO_FDB(...)  if(allow_TRACE_TEST_ADD_TO_FDB) cpssOsPrintf(__VA_ARGS__)

static GT_U32   allow_TRACE_TEST_ADD_TO_FDB_level1 = 0;
/* debug function to allow trace of the test of 'add fdb entries' task */
GT_STATUS fdbMgr_allow_TRACE_TEST_ADD_TO_FDB_level1(IN GT_U32   allowTrace)
{
    allow_TRACE_TEST_ADD_TO_FDB_level1 = allowTrace;
    return GT_OK;
}
#define LEVEL1_TRACE_TEST_ADD_TO_FDB(...)  if(allow_TRACE_TEST_ADD_TO_FDB_level1) cpssOsPrintf(__VA_ARGS__)


static GT_U32   pause_testTaskForAddingMacEntriesToFdb = 0;
GT_STATUS fdbMgr_pause_testTaskForAddingMacEntriesToFdb(IN GT_U32   pause)
{
    pause_testTaskForAddingMacEntriesToFdb = pause;
    return GT_OK;
}

static CPSS_TASK tid_testTaskForAddingMacEntriesToFdb = 0;
/* main function for the TESTING of adding FDB entries , so we can see the aging task delete entries */
static unsigned __TASKCONV testTaskForAddingMacEntriesToFdb(GT_VOID * param/* not used*/)
{
    GT_STATUS   rc;
    static CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC fdbCounters;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC fdbManagerEntryAddParam;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC  fdbEntry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC  *fdbEntryMacAddrFormatPtr;
    GT_U32  rand30Bits;
    GT_U32  iterator = 0;
    GT_U32  numEntriesOK = 0;
    static GT_U32 fdbManagerId = 7;/* (single) global FDB MANAGER ID (unique number between 0..31) */

    param = param;/* not used */


    /* create manager with device */
    createFdbManagerWithDevice(fdbManagerId, FDB_SIZE_FULL, prvTgfDevNum, NULL, GT_FALSE);

    cpssOsMemSet(&fdbManagerEntryAddParam,0,sizeof(fdbManagerEntryAddParam));
    fdbManagerEntryAddParam.tempEntryExist = GT_FALSE;/* adding entry , that not triggered from the AUQ of 'new address'/'moved address' */
    fdbManagerEntryAddParam.tempEntryOffset = 0;/* not relevant*/
    fdbManagerEntryAddParam.rehashEnable = GT_TRUE;

    cpssOsMemSet(&fdbEntry,0,sizeof(fdbEntry));
    fdbEntry.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
    fdbEntryMacAddrFormatPtr = &fdbEntry.format.fdbEntryMacAddrFormat;
    /* this bit is actually 'is entry refreshed' , and for '2 cycle' aging. we need this field to be with value GT_TRUE */
    fdbEntryMacAddrFormatPtr->age = GT_TRUE;

    while(1)
    {
        while(pause_testTaskForAddingMacEntriesToFdb)
        {
            cpssOsTimerWkAfter(500);
        }

        rand30Bits = cpssOsRand() | cpssOsRand() << 15;
        fdbEntryMacAddrFormatPtr->macAddr.arEther[0] = ((rand30Bits >> 24) & 0xFF);
        fdbEntryMacAddrFormatPtr->macAddr.arEther[1] = ((rand30Bits >> 16) & 0xFF);
        fdbEntryMacAddrFormatPtr->macAddr.arEther[2] = ((rand30Bits >>  8) & 0xFF);
        fdbEntryMacAddrFormatPtr->macAddr.arEther[3] = ((rand30Bits >>  0) & 0xFF);

        rand30Bits = cpssOsRand() | cpssOsRand() << 15;
        fdbEntryMacAddrFormatPtr->macAddr.arEther[4] = ((rand30Bits >>  8) & 0xFF);
        fdbEntryMacAddrFormatPtr->macAddr.arEther[5] = ((rand30Bits >>  0) & 0xFF);

        fdbEntryMacAddrFormatPtr->fid = iterator & 0x1FFF;

        rc = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId,&fdbEntry,&fdbManagerEntryAddParam);
        if(rc == GT_OK)
        {
            TRACE_TEST_ADD_TO_FDB("++: fid[%d]",fdbEntryMacAddrFormatPtr->fid);
            if((numEntriesOK & 0xff) == 0xff)
            {
                TRACE_TEST_ADD_TO_FDB("\n");
                if(allow_TRACE_TEST_ADD_TO_FDB_level1)
                {
                    static GT_U32 lastTime = 0;
                    cpssDxChBrgFdbManagerCountersGet(fdbManagerId,&fdbCounters);

                    if(lastTime != fdbCounters.usedEntries)
                    {
                        LEVEL1_TRACE_TEST_ADD_TO_FDB("++: usedEntries[%d]",fdbCounters.usedEntries);
                        lastTime = fdbCounters.usedEntries;
                    }
                }

                /* every 256 entries ... get some sleep */
                cpssOsTimerWkAfter(5);
            }
            numEntriesOK++;
        }
        iterator++;
    }

    return 0;/* should never get here */
}

#define TEST_ADD_FDB_ENTRIES_TASK_PRIO  100 /* task for TESTING purposes only */
/* create task for TESTING purposes only */
GT_STATUS xpsFdbMgrTest_createTaskForAddingMacEntriesToFdb(void)
{
    GT_STATUS   status = GT_OK;

    if(tid_testTaskForAddingMacEntriesToFdb == 0)
    {
        status = cpssOsTaskCreate("testAddFdbEntries",
                          TEST_ADD_FDB_ENTRIES_TASK_PRIO,
                          _24KB,
                          testTaskForAddingMacEntriesToFdb,
                          NULL,
                          &tid_testTaskForAddingMacEntriesToFdb);
    }

    return status;
}

UTF_TEST_CASE_MAC(cpssDxChBrgFdbManager_run_all_with_limits_part_1)
{
    /* skip this test when 'prvUtfSkipLongTestsFlagSet' for Mutex Profiler */
    PRV_TGF_SKIP_LONG_TEST_MUTEX_PROFILER_MAC(UTF_ALL_FAMILY_E);

    run_with_dynamic_uc_limits        = GT_TRUE;

    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerCreate)                  ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerDelete)                  ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerDevListAdd)              ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerDevListRemove)           ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryAdd)                ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryDelete)             ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryTempDelete)         ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryGet)                ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryGetNext)            ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryUpdate)             ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerConfigGet)               ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerDevListGet)              ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerPortLearningSet)         ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerPortLearningGet)         ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerPortRoutingSet)          ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerPortRoutingGet)          ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerLearningThrottlingGet)   ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerLearningScan)            ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerCountersGet)             ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerStatisticsGet)           ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerAgingScan)               ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerDeleteScan)              ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerTransplantScan)          ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryRewrite)            ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerConfigUpdate)            ;

    /* state this test was not reset - even though some of the internal tests were skipped */
    prvUtfSkipTestsFlagReset();

    run_with_dynamic_uc_limits        = GT_FALSE;
}

UTF_TEST_CASE_MAC(cpssDxChBrgFdbManager_run_all_with_limits_part_2)
{
    /* skip this test when 'prvUtfSkipLongTestsFlagSet' for Mutex Profiler */
    PRV_TGF_SKIP_LONG_TEST_MUTEX_PROFILER_MAC(UTF_ALL_FAMILY_E);

    run_with_dynamic_uc_limits        = GT_TRUE;

    UTF_TEST_CALL_MAC(test_Jira_CPSS_9420_FDB_MANAGER_Delete_mac_entry_doesnt_work) ;

    /* extra permutations of : cpssDxChBrgFdbManagerEntryAdd test */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryAdd_run_device_less)                ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryAdd_add_device_in_runtime)          ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryAdd_use_vid1)                       ;

    /* extra permutations of cpssDxChBrgFdbManagerEntryUpdate test */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryUpdate_run_device_less)             ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryUpdate_add_device_in_runtime)       ;

    /* extra permutations of cpssDxChBrgFdbManagerLearningThrottlingGet test */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerLearningThrottlingGet_threshold_Verify)  ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerLearningThrottlingGet_auMsgRate_Verify)  ;

    /* extra permutations of cpssDxChBrgFdbManagerDeleteScan test */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerDeleteScan_with_filter)                  ;

#if !defined(SHARED_MEMORY)
    /* FDB Manager HA scenario */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerHighAvailabilityEntriesSync_basic)       ;
#endif

    /* extra permutations of cpssDxChBrgFdbManagerAgingScan test */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerAgingScan_with_appDemo)                  ;

    /* state this test was not reset - even though some of the internal tests were skipped */
    prvUtfSkipTestsFlagReset();

    run_with_dynamic_uc_limits        = GT_FALSE;
}

UTF_TEST_CASE_MAC(cpssDxChBrgFdbManager_run_all_with_limits_part_3)
{
    run_with_dynamic_uc_limits        = GT_TRUE;

    /* Test for SER functionality */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryRewrite_SER)                        ;

    /* Test High scale MAC & IP entries */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerHighScale_with_IP_entries)               ;

    /* must be last to clean after previouse tests */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManager_clean)                                  ;

    /* state this test was not reset - even though some of the internal tests were skipped */
    prvUtfSkipTestsFlagReset();

    run_with_dynamic_uc_limits        = GT_FALSE;
}

UTF_TEST_CASE_MAC(cpssDxChBrgFdbManager_run_all_with_Hybrid_shadow_type_part_1)
{
    /* AC5 - HYBRID is not supported */
    if(!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        SKIP_TEST_MAC;
    }

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' for Mutex Profiler */
    PRV_TGF_SKIP_LONG_TEST_MUTEX_PROFILER_MAC(UTF_ALL_FAMILY_E);

    userDefinedShadowType   = GT_TRUE;
    globalShadowType        = CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E;
    set_global_entryMuxingMode(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E);

    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerCreate)                  ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerDelete)                  ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerDevListAdd)              ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerDevListRemove)           ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryAdd)                ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryDelete)             ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryTempDelete)         ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryGet)                ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryGetNext)            ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryUpdate)             ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerConfigGet)               ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerDevListGet)              ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryRewrite)            ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerConfigUpdate)            ;

    userDefinedShadowType   = GT_FALSE;
    restore_global_entryMuxingMode();
    /* state this test was not reset - even though some of the internal tests were skipped */
    prvUtfSkipTestsFlagReset();
}

UTF_TEST_CASE_MAC(cpssDxChBrgFdbManager_run_all_with_Hybrid_shadow_type_part_2)
{
    /* AC5 - HYBRID is not supported */
    if(!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        SKIP_TEST_MAC;
    }

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' for Mutex Profiler */
    PRV_TGF_SKIP_LONG_TEST_MUTEX_PROFILER_MAC(UTF_ALL_FAMILY_E);

    userDefinedShadowType   = GT_TRUE;
    globalShadowType        = CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E;
    set_global_entryMuxingMode(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E);

    /* HYBRID model can not support vid1
     * UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryAdd_use_vid1)                    ;
     *
     * Not much required for HYBRID model scenario
     * UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerAgingScan_with_appDemo)               ;
     */
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryAdd_run_device_less)                ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryAdd_add_device_in_runtime)          ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerLearningThrottlingGet_threshold_Verify)  ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerLearningThrottlingGet_auMsgRate_Verify)  ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerDeleteScan_with_filter)                  ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerHighAvailabilityEntriesSync_basic)       ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerEntryRewrite_SER)                        ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManagerHighScale_with_IP_entries)               ;
    UTF_TEST_CALL_MAC(cpssDxChBrgFdbManager_clean)                                  ;

    userDefinedShadowType   = GT_FALSE;
    restore_global_entryMuxingMode();
    /* state this test was not reset - even though some of the internal tests were skipped */
    prvUtfSkipTestsFlagReset();
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBridgeFdbManagerUT suit
   !!! the FDB manager tests guaranty to cleanup configurations !!!
*/
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBridgeFdbManager)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerDevListAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerDevListRemove)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerEntryAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerEntryDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerEntryTempDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerEntryGetNext)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerEntryUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerDevListGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerPortLearningSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerPortLearningGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerPortRoutingSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerPortRoutingGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerLearningThrottlingGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerLearningScan)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerCountersGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerStatisticsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerAgingScan)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerDeleteScan)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerTransplantScan)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerDatabaseCheck)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerEntryRewrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerConfigUpdate)

    /* test driven by JIRA from CV team */
    UTF_SUIT_DECLARE_TEST_MAC(test_Jira_CPSS_9420_FDB_MANAGER_Delete_mac_entry_doesnt_work)

    /* extra permutations of : cpssDxChBrgFdbManagerEntryAdd test */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerEntryAdd_run_device_less)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerEntryAdd_add_device_in_runtime)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerEntryAdd_add_device_in_runtime_withGlobalEportsFullRange)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerEntryAdd_use_vid1)

    /* extra permutations of cpssDxChBrgFdbManagerEntryUpdate test */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerEntryUpdate_run_device_less)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerEntryUpdate_add_device_in_runtime)

    /* extra permutations of cpssDxChBrgFdbManagerLearningThrottlingGet test */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerLearningThrottlingGet_threshold_Verify)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerLearningThrottlingGet_auMsgRate_Verify)

    /* extra permutations of cpssDxChBrgFdbManagerDeleteScan test */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerDeleteScan_with_filter)

    /* FDB Manager HA scenario */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerHighAvailabilityEntriesSync_basic)

    /* extra permutations of cpssDxChBrgFdbManagerAgingScan test */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerAgingScan_with_appDemo)

    /* Test for SER functionality */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerEntryRewrite_SER)

    /* Test High scale MAC & IP entries */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManagerHighScale_with_IP_entries)

    /* must be last to clean after previous tests */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManager_clean)

    /* running most of the tests again in mode of : run_with_dynamic_uc_limits = GT_TRUE */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManager_run_all_with_limits_part_1)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManager_run_all_with_limits_part_2)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManager_run_all_with_limits_part_3)

    /* Shadow type - SIP5_SIP6_HYBRID */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManager_run_all_with_Hybrid_shadow_type_part_1)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgFdbManager_run_all_with_Hybrid_shadow_type_part_2)

UTF_SUIT_END_TESTS_MAC(cpssDxChBridgeFdbManager)

