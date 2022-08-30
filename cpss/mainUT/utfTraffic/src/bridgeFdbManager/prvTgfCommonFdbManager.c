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
* @file prvTgfCommonFdbManager.c
*
* @brief FDB Manager test cases
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfIpGen.h>
#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>
#include <bridgeFdbManager/prvTgfCommonFdbManager.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_utils.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_db.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_hw.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_debug.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/cpssDxChBrgFdbManager.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSecurityBreach.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* we force application to give 'num of entries' in steps of 256 */
#define NUM_ENTRIES_STEPS_CNS                       256

#define PRV_TGF_VLANID_2_CNS                        2
#define PRV_TGF_VLANID_5_CNS                        5
#define PRV_TGF_AGE_OUT_IN_SEC                      60

/* default trunk index */
#define PRV_TGF_TRUNK_ID_CNS                        2

/* trunk enabled members num */
#define PRV_TGF_EMEMBERS_NUM_CNS                    2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_1_CNS                 1
#define PRV_TGF_SEND_PORT_IDX_2_CNS                 2
#define PRV_TGF_SEND_PORT_IDX_3_CNS                 3

/* use global ePorts in upper 2/4 range of eports
 *   Note - To make the ePort the range below 13 Bits */
#define PRV_TGF_GLOBAL_EPORT_MIN_CNS    (UTF_CPSS_PP_MAX_PORT_NUM_CNS(prvTgfDevNum)/4)
#define PRV_TGF_GLOBAL_EPORT_MAX_CNS    ((UTF_CPSS_PP_MAX_PORT_NUM_CNS(prvTgfDevNum)/4)*2)

#define LOG_DELIMITER_MAC   \
    PRV_UTF_LOG0_MAC("-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ \n")

extern GT_STATUS cpssDxChFdbDump(IN GT_U8 devNum);

#define PRINT_FDB(fdbManagerId)     \
    /*cpssDxChFdbDump(prvTgfDevNum);*/  \
    /*prvCpssDxChFdbManagerDebugPrintValidEntries_mac   (fdbManagerId)*/

/* trunk enabled members index array */
static GT_U32 prvTgfTrunkMembersIdx[PRV_TGF_EMEMBERS_NUM_CNS] = {0, 3};

/* fdbSize is initialized at the beginning of the test suite (create), only once
 * In case of shared memory, the size is = actual/2 */
static GT_U32    fdbSize;

/* use FDB size that match the fineTunning (128K in Falcon/AC5P , 32K with AC5X) */
#define FDB_SIZE_FULL_INIT()                                                        \
    if(!fdbSize)                                                                    \
    {                                                                               \
        fdbSize = (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.fdb);   \
    }

/* use FDB size that match the fineTunning (128K in Falcon/AC5P , 32K with Phoenix)
 * In case of shared memory build, due to memory constraints used half of the supported size */
#if defined(CPSS_USE_MUTEX_PROFILER)
#define FDB_SIZE_FULL                   ((fdbSize > _32K) ? _32K : fdbSize)
#else
#if defined(SHARED_MEMORY)
#define FDB_SIZE_FULL                   (fdbSize/2)
#else
#define FDB_SIZE_FULL                   fdbSize
#endif /* SHARED_MEMORY */
#endif /* CPSS_USE_MUTEX_PROFILER */

#define PRV_TGF_TOTAL_HW_CAPACITY                   FDB_SIZE_FULL
#define PRV_TGF_MAX_ENTRIES_PER_LEARNING_SCAN       (NUM_ENTRIES_STEPS_CNS)
#define PRV_TGF_MAX_TOTAL_ENTRIES                   (PRV_TGF_TOTAL_HW_CAPACITY - NUM_ENTRIES_STEPS_CNS)
#define PRV_CPSS_OUI_LSB_MASK                       0xFC
#define PRV_TGF_FDB_MANAGER_SHARED_FID_CNS          2;

#define NEW_OR_UPDATE_EVENT_PER_DEVICE()    ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_E:CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_E)
#define HASH_AS_PER_DEVICE(num)         ((PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))?num:(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?16:1)
#define HASH_MODE_AS_PER_DEVICE(mode)   ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E:mode)
#define SHADOW_TYPE_PER_DEVICE()        ((userDefinedShadowType)?(globalShadowType):((PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))?CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E:(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E:CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E))
#define IP_UNICAST_SUPPORT_AS_PER_DEVICE()  ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?GT_OK:GT_BAD_PARAM)
#define BANK_AS_PER_DEVICE(bank)            ((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?bank:bank%3);

/* Global dbCheck(All test selected) */
static CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC      dbCheckParam;
static GT_BOOL  userDefinedShadowType               = GT_FALSE;
static CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT    globalShadowType  = CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E;
void prvTgf_testsFDB(void);

typedef struct
{
    GT_BOOL enableLearningLimits;
    GT_U32  maxDynamicUcMacGlobalLimit;
    GT_U32  maxDynamicUcMacFidLimit;
    GT_U32  maxDynamicUcMacGlobalEportLimit;
    GT_U32  maxDynamicUcMacTrunkLimit;
    GT_U32  maxDynamicUcMacPortLimit;
}LEARN_LIMITS_STC;

#define DEFAULT_LEARN_LIMITS_CNS                        \
     GT_FALSE /*enableLearningLimits               */   \
    ,0x1000   /*maxDynamicUcMacGlobalLimit         */   \
    ,0x100    /*maxDynamicUcMacFidLimit            */   \
    ,0x80     /*maxDynamicUcMacGlobalEportLimit    */   \
    ,0x40     /*maxDynamicUcMacTrunkLimit          */   \
    ,0x10     /*maxDynamicUcMacPortLimit           */

static LEARN_LIMITS_STC learnLimits      = {DEFAULT_LEARN_LIMITS_CNS};

static void setLearnLimits(IN LEARN_LIMITS_STC *newLearnLimitsPtr)
{
    learnLimits = *newLearnLimitsPtr;
    return;
}

static void restoreLearnLimits(void)
{
    LEARN_LIMITS_STC defaultLearnLimits = {DEFAULT_LEARN_LIMITS_CNS};

    learnLimits = defaultLearnLimits;

    return;
}


/* cleanup on ALL managers */
#define CLEANUP_ALL_MANAGERS                                                \
{                                                                           \
    GT_U32  ii, _rc;                                                        \
    for(ii = 0 ; ii < CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS; ii++) \
    {                                                                       \
        _rc = cpssDxChBrgFdbManagerDelete(ii);                               \
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, _rc, "cpssDxChBrgFdbManagerDelete: expected to GT_OK on manager [%d]", ii);\
    }                                                                       \
    prvTgf_testsFDB();                                                      \
}

/* cleanup on specific manager */
#define CLEANUP_MANAGER(fdbManagerId)                                       \
{                                                                           \
    GT_U32 _rc;                                                             \
    _rc = cpssDxChBrgFdbManagerDelete(fdbManagerId);                        \
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, _rc, "cpssDxChBrgFdbManagerDelete: expected to GT_OK on manager [%d]", fdbManagerId);\
    prvTgf_testsFDB();                                                      \
}

#define CLEAR_VAR_MAC(var)  \
    cpssOsMemSet(&var,0,sizeof(var))


static GT_U32   prvSpecificFidFilterValue = 1;
static GT_BOOL  prvIsStatic               = GT_FALSE;
static CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_ENT  vlanLookUpMode       = CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E;
static CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_ENT macEntryMuxingMode   = CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E;
static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_ENT entryMuxingMode          = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E;

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* Unicast packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */ /*0x5EA0*/
    {10, 10, 10, 10},   /* srcAddr */
    {10, 10,  0,  1}    /* dstAddr */
};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x11, 0xAA},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_2_CNS                          /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/************************ Single tagged packet ********************************/
/* PARTS of single tagged packet */
static TGF_PACKET_PART_STC prvTgfPacketSingleTagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of single tagged packet */
#define PRV_TGF_PACKET_SINGLE_TAG_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of single tagged packet with CRC */
#define PRV_TGF_PACKET_SINGLE_TAG_CRC_LEN_CNS  PRV_TGF_PACKET_SINGLE_TAG_LEN_CNS + TGF_CRC_LEN_CNS

/* Single tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoSingleTag =
{
    PRV_TGF_PACKET_SINGLE_TAG_LEN_CNS,                                    /* totalLen */
    sizeof(prvTgfPacketSingleTagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketSingleTagPartArray                                        /* partsArray */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
{
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* PARTS of unicast packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET IPv4 to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/**
* @internal prvTgfBrgFdbManagerHWFlushAll function
* @endinternal
*
* @brief   Flush All HW FDB table entries
*          Note: It does not clean DB, caller need to take care
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
*/
static GT_STATUS prvTgfBrgFdbManagerHWFlushAll
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr
)
{
    GT_U32      hwIndex;
    GT_STATUS   rc;

    if(fdbManagerPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    for(hwIndex = 0; hwIndex < fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; hwIndex++)
    {
        rc = prvCpssDxChFdbManagerHwFlushByHwIndex(fdbManagerPtr,
                hwIndex/*hwIndex*/,
                hwIndex % fdbManagerPtr->numOfBanks,/* bankId */
                GT_TRUE);/* we update the bank counters in HW */
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvTgfBrgFdbManager_restoreFdbSize function
* @endinternal
*
* @brief  restore FDB size
*/
GT_VOID prvTgfBrgFdbManager_restoreFdbSize()
{
    #if defined(SHARED_MEMORY)
    GT_STATUS               rc;
    /* FDB Size was reduced due to memory constraints of FDB manager,
     * Restore back the FDB size */
    rc = prvCpssDxChBrgFdbSizeSet(prvTgfDevNum, fdbSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDxChBrgFdbSizeSet: expected to GT_OK on manager [%d]");
    #endif
}

/**
* @internal prvTgfBrgFdb_cleanup function
* @endinternal
*
* @brief  Cleans up FDB and explicitly reset the HW bank counters.
*/
GT_VOID prvTgfBrgFdb_cleanup
(
    GT_BOOL initialClean
)
{
    GT_STATUS   rc;
    GT_U32      bankIndex;
    GT_U32      value;
    GT_U8       devNum = prvTgfDevNum;
    GT_U32      hwIndex;

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) || (initialClean == GT_FALSE))
    {
        /* flush FDB include static entries */
        rc = prvTgfBrgFdbFlush(GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
    }
    else
    {
        /* use entry by entry invalidation to guarantee empty FDB.
           "flushFdb" may leave some SKIPep valid entries when non multi-hash mode is used. */
        for(hwIndex = 0; hwIndex < FDB_SIZE_FULL; hwIndex++)
        {
            rc = cpssDxChBrgFdbMacEntryInvalidate(devNum,hwIndex);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbMacEntryInvalidate: %d, %d", prvTgfDevNum, hwIndex);
        }
    }

    if(!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        return;
    }

    /* Get Bank counter, make it 0 in HW */
    for(bankIndex = 0; bankIndex < 16; bankIndex++)
    {
        rc = cpssDxChBrgFdbBankCounterValueGet(devNum, bankIndex, &value);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssDxChBrgFdbBankCounterValueGet: unexpected error");
        while(value>0)
        {
            rc = cpssDxChBrgFdbBankCounterUpdate(devNum, bankIndex, GT_FALSE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                    "cpssDxChBrgFdbBankCounterValueGet: unexpected error");
            value-=1;
        }
    }
}


static GT_VOID setAllVlansModeVid1lookup
(
    IN GT_BOOL  enable,
    IN GT_U16   vlanId
)
{
    GT_STATUS st;
    CPSS_PORTS_BMP_STC                    portsMembers;
    CPSS_PORTS_BMP_STC                    portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC           cpssVlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    GT_U8   devNum = prvTgfDevNum;
    GT_BOOL isValid;

    /* read default VLAN entry */
    st = cpssDxChBrgVlanEntryRead(devNum, vlanId, &portsMembers, &portsTagging,
                                  &cpssVlanInfo, &isValid, &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChBrgVlanEntryRead: expected to GT_OK on vlan [%d]",
        1);

    cpssVlanInfo.fdbLookupKeyMode = (enable == GT_TRUE) ?
                CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_VID1_E :
                CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_E;

    cpssVlanInfo.fidValue = vlanId;/*fid = vid */
    st = cpssDxChBrgVlanEntryWrite(devNum, vlanId, &portsMembers, &portsTagging,
            &cpssVlanInfo, &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgVlanEntryWrite: expected to GT_OK on vlan [%d]",
            vlanId);

    return;
}

static GT_STATUS fdbManagerDbValidate
(
    IN GT_U32   fdbManagerId
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    GT_U32                                                   entryCnt, ageBinCnt;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC           *dbEntryPtr;
    GT_U32                                                   dbIndex;

    /* Age-bin Verify - counters against list */
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);
    if(fdbManagerPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    for(ageBinCnt=0; ageBinCnt < fdbManagerPtr->agingBinInfo.totalAgeBinAllocated; ageBinCnt++)
    {
        /* Empty age-bin */
        if(fdbManagerPtr->agingBinInfo.ageBinListPtr[ageBinCnt].totalUsedEntriesCnt == 0)
        {
            continue;
        }

        dbIndex    = fdbManagerPtr->agingBinInfo.ageBinListPtr[ageBinCnt].headOfAgeBinEntryList.entryPointer;
        dbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbIndex];

        /* Single Node */
        if(fdbManagerPtr->agingBinInfo.ageBinListPtr[ageBinCnt].totalUsedEntriesCnt == 1)
        {
            if((fdbManagerPtr->agingBinInfo.ageBinListPtr[ageBinCnt].headOfAgeBinEntryList.isValid_entryPointer != 1) ||
                    (dbEntryPtr->isValid_age_prevEntryPointer == 1) ||
                    (dbEntryPtr->isValid_age_nextEntryPointer == 1))
            {
                return GT_FAIL;
            }
            continue;
        }

        /* Head Node - Prev should be invalid */
        if((fdbManagerPtr->agingBinInfo.ageBinListPtr[ageBinCnt].headOfAgeBinEntryList.isValid_entryPointer != 1) ||
                (dbEntryPtr->isValid_age_prevEntryPointer == 1) ||
                (dbEntryPtr->isValid_age_nextEntryPointer != 1))
        {
            return GT_FAIL;
        }

        for(entryCnt=2; entryCnt < fdbManagerPtr->agingBinInfo.ageBinListPtr[ageBinCnt].totalUsedEntriesCnt; entryCnt++)
        {
            /* Middle Node - prev/next both valid */
            dbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbEntryPtr->age_nextEntryPointer];
            if((fdbManagerPtr->indexArr[dbEntryPtr->hwIndex].isValid_entryPointer != 1) ||
                    (dbEntryPtr->isValid_age_prevEntryPointer != 1) ||
                    (dbEntryPtr->isValid_age_nextEntryPointer != 1))
            {
                return GT_FAIL;
            }
        }

        /* End Node - next should be invalid */
        dbEntryPtr = &fdbManagerPtr->entryPoolPtr[dbEntryPtr->age_nextEntryPointer];
        if((fdbManagerPtr->indexArr[dbEntryPtr->hwIndex].isValid_entryPointer != 1) ||
                 (dbEntryPtr->isValid_age_prevEntryPointer != 1) ||
                 (dbEntryPtr->isValid_age_nextEntryPointer == 1))
        {
            return GT_FAIL;
        }
    }
    return GT_OK;
}


/* add fdb entry to the manager : type : mac addr */
static void fdbManagerEntryBuild_macAddr(
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr
)
{
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC *fdbEntryMacAddrFormatPtr;
    cpssOsMemSet(entryPtr ,0,sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));

    fdbEntryMacAddrFormatPtr = &entryPtr->format.fdbEntryMacAddrFormat;

    entryPtr->fdbEntryType                       = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[0] = 0x00;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[1] = 0x11;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[2] = 0x22;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[3] = 0x33;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[4] = 0x44;
    fdbEntryMacAddrFormatPtr->macAddr.arEther[5] = 0x55;
    fdbEntryMacAddrFormatPtr->fid                = 1;
    fdbEntryMacAddrFormatPtr->vid1               = 0;
    fdbEntryMacAddrFormatPtr->daSecurityLevel    = 0;
    fdbEntryMacAddrFormatPtr->userDefined        = 0;
    fdbEntryMacAddrFormatPtr->sourceID           = 0;
    fdbEntryMacAddrFormatPtr->daCommand          = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    fdbEntryMacAddrFormatPtr->saCommand          = CPSS_PACKET_CMD_DROP_SOFT_E;
    fdbEntryMacAddrFormatPtr->isStatic           = GT_FALSE;
    fdbEntryMacAddrFormatPtr->age                = GT_TRUE;
    fdbEntryMacAddrFormatPtr->daRoute            = GT_FALSE;
    fdbEntryMacAddrFormatPtr->appSpecificCpuCode = GT_TRUE;

    return;
}

/* MAC update mode while adding entries by fdbManagerEntryAdd_macAddr
 *  PRV_TGF_FDB_MANAGER_MAC_RANDOM_MODE_E       - MAC bits will be generated by osRand
 *  PRV_TGF_FDB_MANAGER_MAC_INCREMENTAL_MODE_E  - MAC bits will be incremented starting from startAddr
 */
typedef enum {
    PRV_TGF_FDB_MANAGER_MAC_RANDOM_MODE_E,
    PRV_TGF_FDB_MANAGER_MAC_INCREMENTAL_MODE_E
} PRV_TGF_FDB_MANAGER_MAC_BUILD_MODE_ENT;

static PRV_TGF_FDB_MANAGER_MAC_BUILD_MODE_ENT prvTgfFdbManagerMacBuildMode = PRV_TGF_FDB_MANAGER_MAC_RANDOM_MODE_E;
static GT_U8        startAddr[6];
static GT_BOOL      prvTgfFdbManagerRehashEnable = GT_FALSE;

/* add fdb entry to the manager : type : mac addr */
static void fdbManagerEntryAdd_macAddr(
    IN GT_U32   fdbManagerId,
    IN GT_U32   iterationNum,
    OUT GT_BOOL *entryAddedPtr
)
{
    GT_STATUS   st;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     entryGet;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   fdbEntryAddInfo;
    static GT_BOOL                                          isFirstTime = GT_TRUE;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC     *fdbEntryMacAddrFormatPtr;
    GT_U32                                                  rand32Bits; /* random value in 32 bits (random 30 bits + 2 bits from iterationNum) */

    fdbEntryMacAddrFormatPtr = &entry.format.fdbEntryMacAddrFormat;

    if(isFirstTime)
    {
        cpssOsMemSet(&fdbEntryAddInfo,0,sizeof(fdbEntryAddInfo));
        isFirstTime = GT_FALSE;

        fdbManagerEntryBuild_macAddr(&entry);
    }
    fdbEntryMacAddrFormatPtr->saSecurityLevel = (SHADOW_TYPE_PER_DEVICE() == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E)?0:1;
    fdbEntryAddInfo.rehashEnable = prvTgfFdbManagerRehashEnable;

    rand32Bits = (cpssOsRand() & 0x7FFF) << 15 |
                 (cpssOsRand() & 0x7FFF)       |
                 iterationNum << 30;

    fdbEntryAddInfo.tempEntryExist  = (rand32Bits & BIT_7) ? GT_TRUE : GT_FALSE;
    fdbEntryAddInfo.tempEntryOffset = BANK_AS_PER_DEVICE((rand32Bits >> 15) & 0xF);/*bankId*/
    if(prvTgfFdbManagerMacBuildMode == PRV_TGF_FDB_MANAGER_MAC_INCREMENTAL_MODE_E)
    {
        /* In case of incremental mode start, XX: XX: 0X00: 0X00: 0X00: 0x00 */
        if(iterationNum == 0)
        {
            fdbEntryMacAddrFormatPtr->macAddr.arEther[0] = startAddr[0];
            fdbEntryMacAddrFormatPtr->macAddr.arEther[1] = startAddr[1];
            fdbEntryMacAddrFormatPtr->macAddr.arEther[2] = startAddr[2];
            fdbEntryMacAddrFormatPtr->macAddr.arEther[3] = startAddr[3];
            fdbEntryMacAddrFormatPtr->macAddr.arEther[4] = startAddr[4];
            fdbEntryMacAddrFormatPtr->macAddr.arEther[5] = startAddr[5];
        }

        fdbEntryMacAddrFormatPtr->macAddr.arEther[5] += 1;
        if(fdbEntryMacAddrFormatPtr->macAddr.arEther[5] == 0)
        {
            fdbEntryMacAddrFormatPtr->macAddr.arEther[4] +=1;
            if(fdbEntryMacAddrFormatPtr->macAddr.arEther[4] == 0)
            {
                fdbEntryMacAddrFormatPtr->macAddr.arEther[3] += 1;
                if(fdbEntryMacAddrFormatPtr->macAddr.arEther[3] == 0)
                {
                    fdbEntryMacAddrFormatPtr->macAddr.arEther[2] += 1;
                }
            }
        }
    }
    else if(prvTgfFdbManagerMacBuildMode == PRV_TGF_FDB_MANAGER_MAC_RANDOM_MODE_E)
    {
        fdbEntryMacAddrFormatPtr->macAddr.arEther[2] = (GT_U8)(rand32Bits >> 24);
        fdbEntryMacAddrFormatPtr->macAddr.arEther[3] = (GT_U8)(rand32Bits >> 16);
        fdbEntryMacAddrFormatPtr->macAddr.arEther[4] = (GT_U8)(rand32Bits >>  8);
        fdbEntryMacAddrFormatPtr->macAddr.arEther[5] = (GT_U8)(rand32Bits >>  0);
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        fdbEntryMacAddrFormatPtr->fid                = (prvSpecificFidFilterValue != 1)?prvSpecificFidFilterValue:cpssOsRand() & 0x1FFF; /* 13 bits */
        fdbEntryMacAddrFormatPtr->sourceID           = cpssOsRand() & 0xFFF; /* 12 bits */
        fdbEntryMacAddrFormatPtr->isStatic           = prvIsStatic;
    }
    else
    {
        fdbEntryMacAddrFormatPtr->fid                = (prvSpecificFidFilterValue != 1)?prvSpecificFidFilterValue:cpssOsRand() & 0xFFF; /* 12 bits */
        fdbEntryMacAddrFormatPtr->sourceID           = cpssOsRand() & 0x1F; /* 5 bits */
        fdbEntryMacAddrFormatPtr->isStatic           = prvIsStatic;
        fdbEntryMacAddrFormatPtr->daQoSParameterSetIndex = cpssOsRand() & 0x7;  /* 3 bits */
        fdbEntryMacAddrFormatPtr->saQoSParameterSetIndex = cpssOsRand() & 0x7;  /* 3 bits */
        fdbEntryMacAddrFormatPtr->mirrorToAnalyzerPort   = GT_FALSE;
    }

    st = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId, &entry, &fdbEntryAddInfo);
    if(st == GT_OK)
    {
        *entryAddedPtr = GT_TRUE;
    }
    else if ((st != GT_FULL                              && learnLimits.enableLearningLimits == GT_FALSE) ||
             (learnLimits.enableLearningLimits == GT_TRUE&&
              st != GT_FULL                              &&
              st != GT_LEARN_LIMIT_PORT_ERROR            &&
              st != GT_LEARN_LIMIT_TRUNK_ERROR           &&
              st != GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR    &&
              st != GT_LEARN_LIMIT_FID_ERROR             &&
              st != GT_LEARN_LIMIT_GLOBAL_ERROR))
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
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
        entryGet.format.fdbEntryMacAddrFormat.fid = fdbEntryMacAddrFormatPtr->fid;
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
        UTF_VERIFY_EQUAL1_STRING_MAC(entry.fdbEntryType, entryGet.fdbEntryType,
                "cpssDxChBrgFdbManagerEntryGet: expected fdbEntryType in 'get' entry to match 'set' in manager [%d]",
                fdbManagerId);

        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(
                    &entryGet.format.fdbEntryMacAddrFormat,
                    &entry.format.fdbEntryMacAddrFormat,
                    sizeof(entryGet.format.fdbEntryMacAddrFormat)),
                "cpssDxChBrgFdbManagerEntryGet: expected 'get' entry to match 'set' entry in manager [%d]",
                fdbManagerId);
    }

    /* Validate DB corruption */
    st = fdbManagerDbValidate(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
            "fdbManagerDbValidate: unexpected error in manager [%d]",
            fdbManagerId);
}

/* create fdb manager - minimal parameters for tests */
static GT_STATUS createFdbManagerWithDevice(
    IN GT_U32                                       fdbManagerId,
    IN GT_U32                                       maxTotalEntries,/* up to 128K (limited by numOfHwIndexes) */
    IN CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC      *inputCapacityInfo,
    IN GT_BOOL                                      macNoSpaceUpdatesEnable
)
{
    GT_STATUS   st;
    GT_U8  devListArr[1];
    GT_U32 numOfDevs = 1;

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

    if(inputCapacityInfo != NULL)
    {
        capacityInfo.hwCapacity.numOfHwIndexes = inputCapacityInfo->hwCapacity.numOfHwIndexes;
        capacityInfo.hwCapacity.numOfHashes    = HASH_AS_PER_DEVICE(inputCapacityInfo->hwCapacity.numOfHashes);
        capacityInfo.maxEntriesPerLearningScan = inputCapacityInfo->maxEntriesPerLearningScan;
        capacityInfo.maxEntriesPerAgingScan    = inputCapacityInfo->maxEntriesPerAgingScan;
    }
    else
    {
        capacityInfo.hwCapacity.numOfHwIndexes = FDB_SIZE_FULL;
        capacityInfo.hwCapacity.numOfHashes    = HASH_AS_PER_DEVICE(16);
        capacityInfo.maxEntriesPerLearningScan = NUM_ENTRIES_STEPS_CNS; /*256*/
        capacityInfo.maxEntriesPerAgingScan    = NUM_ENTRIES_STEPS_CNS; /*256*/
    }
    capacityInfo.hwCapacity.mode             = HASH_MODE_AS_PER_DEVICE(0);
    capacityInfo.maxTotalEntries             = maxTotalEntries;
    capacityInfo.maxEntriesPerDeleteScan     = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerTransplantScan = NUM_ENTRIES_STEPS_CNS; /*256*/

    entryAttrInfo.macEntryMuxingMode         = macEntryMuxingMode;
    entryAttrInfo.ipmcEntryMuxingMode        = CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E;
    entryAttrInfo.entryMuxingMode            = entryMuxingMode;
    entryAttrInfo.saDropCommand              = CPSS_PACKET_CMD_DROP_SOFT_E;
    entryAttrInfo.daDropCommand              = CPSS_PACKET_CMD_DROP_HARD_E;
    entryAttrInfo.ipNhPacketcommand          = CPSS_PACKET_CMD_ROUTE_E;
    entryAttrInfo.shadowType                 = SHADOW_TYPE_PER_DEVICE();
    if(entryAttrInfo.shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)
    {
        CPSS_DXCH_CFG_GLOBAL_EPORT_STC  globalEportInfo,l2EcmpInfo,l2DlbInfo;

        st = cpssDxChCfgGlobalEportGet(prvTgfDevNum,&globalEportInfo,&l2EcmpInfo,&l2DlbInfo);
        if (st == GT_OK)
        {
            if(entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)
            {
                globalEportInfo.enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E;
                globalEportInfo.minValue = PRV_TGF_GLOBAL_EPORT_MIN_CNS;
                globalEportInfo.maxValue = PRV_TGF_GLOBAL_EPORT_MAX_CNS;

                st = cpssDxChCfgGlobalEportSet(prvTgfDevNum,&globalEportInfo,&l2EcmpInfo,&l2DlbInfo);
                UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, st, "cpssDxChCfgGlobalEportSet: min %d, max %d",
                        globalEportInfo.minValue,globalEportInfo.maxValue);
            }
            /* use global eports , as defined by the test/environment prior to calling the 'Create Manager' */
            entryAttrInfo.globalEportInfo     = globalEportInfo;
        }
    }

    capacityInfo.enableLearningLimits               = learnLimits.enableLearningLimits              ;
    capacityInfo.maxDynamicUcMacGlobalLimit         = learnLimits.maxDynamicUcMacGlobalLimit        ;
    capacityInfo.maxDynamicUcMacFidLimit            = learnLimits.maxDynamicUcMacFidLimit           ;
    capacityInfo.maxDynamicUcMacGlobalEportLimit    = learnLimits.maxDynamicUcMacGlobalEportLimit   ;
    capacityInfo.maxDynamicUcMacTrunkLimit          = learnLimits.maxDynamicUcMacTrunkLimit         ;
    capacityInfo.maxDynamicUcMacPortLimit           = learnLimits.maxDynamicUcMacPortLimit          ;

    learningInfo.macNoSpaceUpdatesEnable     = macNoSpaceUpdatesEnable;
    learningInfo.macRoutedLearningEnable     = GT_FALSE;
    learningInfo.macVlanLookupMode           = vlanLookUpMode;

    lookupInfo.crcHashUpperBitsMode          = CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;
    lookupInfo.ipv4PrefixLength              = 32;
    lookupInfo.ipv6PrefixLength              = 128;

    agingInfo.destinationUcRefreshEnable     = GT_FALSE;
    agingInfo.destinationMcRefreshEnable     = GT_FALSE;
    agingInfo.ipUcRefreshEnable              = GT_FALSE;

    st = cpssDxChBrgFdbManagerCreate(fdbManagerId,
        &capacityInfo,&entryAttrInfo,&learningInfo,&lookupInfo,&agingInfo);
    UTF_VERIFY_EQUAL0_NO_RETURN_MAC(GT_OK, st);

    /* add device to it */
    devListArr[0] = prvTgfDevNum;
    st = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId,devListArr,numOfDevs);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, st,
            "cpssDxChBrgFdbManagerDevListAdd: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* disable informative events to improve performance of tests */
    (GT_VOID)cpssEventDeviceMaskSet(prvTgfDevNum,
                                    CPSS_PP_MAC_NA_LEARNED_E,
                                    CPSS_EVENT_MASK_E);
    (GT_VOID)cpssEventDeviceMaskSet(prvTgfDevNum,
                                    CPSS_PP_TX_ERR_QUEUE_E,
                                    CPSS_EVENT_MASK_E);
    (GT_VOID)cpssEventDeviceMaskSet(prvTgfDevNum,
                                    CPSS_PP_TX_END_E,
                                    CPSS_EVENT_MASK_E);

    return st;
}

/**
* @internal createFdbManagerWithAppDemoSupport function
* @endinternal
*
* @brief  create fdb manager - with created device and with appDemo support.
*
*
*/
GT_VOID createFdbManagerWithAppDemoSupport
(
    IN GT_U32       fdbManagerId,
    IN GT_U32       agingPeriod
)
{
    CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC      capacity;
    GT_STATUS                                   rc;

    /* Customize HW capacity */
    cpssOsMemSet(&capacity, 0, sizeof(capacity));
    capacity.hwCapacity.numOfHwIndexes   = PRV_TGF_TOTAL_HW_CAPACITY;
    capacity.hwCapacity.numOfHashes      = 16;
    capacity.maxEntriesPerLearningScan   = PRV_TGF_MAX_ENTRIES_PER_LEARNING_SCAN;
    capacity.maxEntriesPerAgingScan      = NUM_ENTRIES_STEPS_CNS; /*256*/

    /* create single valid manager */
    rc = createFdbManagerWithDevice(fdbManagerId, PRV_TGF_MAX_TOTAL_ENTRIES, &capacity, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "createFdbManager: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Set FDB manager ID for appDemo */
    rc = prvWrAppDemoFdbManagerGlobalIDSet(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerGlobalIDSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Set FDB manager AU Message handler methord as FDB_MANAGER for appDemo */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E, agingPeriod ? GT_TRUE : GT_FALSE, agingPeriod);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);
}


static GT_VOID prvCpssDxChBrgFdbManagerCounterVerify
(
    IN GT_U32                                     fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC    *expCountersPtr
)
{
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC      getCounters;
    GT_STATUS                                   rc;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC      expCounters = *expCountersPtr;


    rc = cpssDxChBrgFdbManagerCountersGet(fdbManagerId, &getCounters);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbManagerCountersGet: %d", prvTgfDevNum);

    if(learnLimits.enableLearningLimits == GT_TRUE &&
       expCounters.macUnicastDynamic > learnLimits.maxDynamicUcMacGlobalLimit)
    {
        /* Verify Counter : can't learn more than the limit */
        expCounters.usedEntries       -= expCounters.macUnicastDynamic;
        expCounters.macUnicastDynamic = learnLimits.maxDynamicUcMacGlobalLimit;
        expCounters.usedEntries       += expCounters.macUnicastDynamic;
    }

    /* Verify Counter */
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters.macUnicastDynamic,   getCounters.macUnicastDynamic,    "macUnicastDynamic: unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters.macUnicastStatic,    getCounters.macUnicastStatic,     "macUnicastStatic: unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters.macMulticastDynamic, getCounters.macMulticastDynamic,  "macMulticastDynamic: unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters.macMulticastStatic,  getCounters.macMulticastStatic,   "macMulticastStatic: unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters.ipv4Multicast,       getCounters.ipv4Multicast,        "ipv4Multicast: unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters.ipv6Multicast,       getCounters.ipv6Multicast,        "ipv6Multicast: unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters.ipv4UnicastRoute,    getCounters.ipv4UnicastRoute,     "ipv4UnicastRoute: unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters.ipv6UnicastRouteKey, getCounters.ipv6UnicastRouteKey,  "ipv6UnicastRouteKey: unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters.ipv6UnicastRouteData,getCounters.ipv6UnicastRouteData, "ipv6UnicastRouteData: unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters.usedEntries,         getCounters.usedEntries,          "usedEntries: unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters.freeEntries,         getCounters.freeEntries,          "freeEntries: unexpected");
}

static GT_U32  prvSaveFdbSize;
/**
* @internal prvTgfBrgFdbManagerLearningScan_statistics_config function
* @endinternal
*
* @brief  configures the learning scan related parameters to verify
*         learning scan statistics
*/
GT_VOID prvTgfBrgFdbManagerLearningScan_statistics_config
(
    IN GT_U32       fdbManagerId
)
{
    CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC      capacity;
    GT_STATUS                                   rc;

    FDB_SIZE_FULL_INIT();

    prvSaveFdbSize = fdbSize;
    if(!PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* limit test to use upto 128K FDB size to reduce duration of test.*/
        fdbSize = (fdbSize > _128K) ? _128K : fdbSize;
    }

    /* add ports to vlan 2 */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
            prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* Customize HW capacity */
    cpssOsMemSet(&capacity, 0, sizeof(capacity));
    capacity.hwCapacity.numOfHwIndexes   = PRV_TGF_TOTAL_HW_CAPACITY;
    capacity.hwCapacity.numOfHashes      = 16;
    capacity.maxEntriesPerLearningScan   = PRV_TGF_MAX_ENTRIES_PER_LEARNING_SCAN;
    capacity.maxEntriesPerAgingScan      = NUM_ENTRIES_STEPS_CNS; /*256*/

    /* create single valid manager */
    rc = createFdbManagerWithDevice(fdbManagerId, PRV_TGF_MAX_TOTAL_ENTRIES, &capacity, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "createFdbManager: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Set FDB manager ID for appDemo */
    rc = prvWrAppDemoFdbManagerGlobalIDSet(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerGlobalIDSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Set FDB manager AU Message handler method as FDB_MANAGER for appDemo */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_DISABLED_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);
}

static GT_VOID prvTgfBrgFdbManagerLearningScan_statistics_verify
(
    IN GT_U32           fdbManagerId,
    IN GT_U32           learningScanStatisticsArr[],
    IN GT_U32           entryAddStatisticsArr[]
)
{
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC                statistics;
    GT_STATUS                                               rc;
    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    if(learningScanStatisticsArr)
    {
        if(learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E] == GT_NA)
        {
            /* we can't really predict the exact number of 'chunks of messages from AUQ' will be */
            /* but we can make sure it is not MORE than the number of total messages */
            if(learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E] != (statistics.scanLearningOk + statistics.scanLearningOkNoMore))
            {
                UTF_VERIFY_EQUAL4_STRING_MAC(1,
                                         (((statistics.scanLearningOk + statistics.scanLearningOkNoMore) < learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]) ? 1 : 0),
                                         "ERROR: cpssDxChBrgFdbManagerStatisticsGet (scanLearningOk[%d]+scanLearningOkNoMore[%d]) = [%d] but should be less than [%d]",
                                         statistics.scanLearningOk ,
                                         statistics.scanLearningOkNoMore ,
                                         statistics.scanLearningOk + statistics.scanLearningOkNoMore,
                                         learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]);
            }
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E],
                                         statistics.scanLearningOk,
                                         "ERROR: cpssDxChBrgFdbManagerStatisticsGet : API_STATISTICS_OK", prvTgfDevNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E],
                                         statistics.scanLearningOkNoMore,
                                         "ERROR: cpssDxChBrgFdbManagerStatisticsGet : NO_MORE", prvTgfDevNum);
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_ERROR_INPUT_INVALID_E],
                                     statistics.scanLearningErrorInputInvalid,
                                     "ERROR: cpssDxChBrgFdbManagerStatisticsGet : ERROR_INPUT_INVALID", prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_ERROR_FAILED_TABLE_UPDATE_E],
                                     statistics.scanLearningErrorFailedTableUpdate,
                                     "ERROR: cpssDxChBrgFdbManagerStatisticsGet : ERROR_FAILED_TABLE_UPDATE", prvTgfDevNum);
        if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E],
                    statistics.scanLearningTotalHwEntryNewMessages,
                    "ERROR: cpssDxChBrgFdbManagerStatisticsGet : TOTAL_NA_MSG", prvTgfDevNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E],
                    statistics.scanLearningTotalHwEntryMovedMessages,
                    "ERROR: cpssDxChBrgFdbManagerStatisticsGet : MOVED_MSG", prvTgfDevNum);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E] +
                    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E],
                    statistics.scanLearningTotalHwEntryNewMessages + statistics.scanLearningTotalHwEntryMovedMessages,
                    "ERROR: cpssDxChBrgFdbManagerStatisticsGet : TOTAL_NA_MSG + TOTAL_MOVED_MSG", prvTgfDevNum);

            UTF_VERIFY_EQUAL1_STRING_MAC(statistics.scanLearningTotalHwEntryMovedMessages, 0,
                    "ERROR: cpssDxChBrgFdbManagerStatisticsGet : EntryMovedMessages", prvTgfDevNum);
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NO_SPACE_MSG_E],
                                     statistics.scanLearningTotalHwEntryNoSpaceMessages,
                                     "ERROR: cpssDxChBrgFdbManagerStatisticsGet : NO_SPACE_MSG", prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NO_SPACE_FILTERED_MSG_E],
                                     statistics.scanLearningTotalHwEntryNoSpaceFilteredMessages,
                                     "ERROR: cpssDxChBrgFdbManagerStatisticsGet : NO_SPACE_FILTERED_MSG", prvTgfDevNum);
    }

    if(entryAddStatisticsArr)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(entryAddStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_PORT_LIMIT_E],
                                     statistics.entryAddErrorPortLimit,
                                     "ERROR: cpssDxChBrgFdbManagerStatisticsGet : ERROR_PORT_LIMIT", prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(entryAddStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TRUNK_LIMIT_E],
                                     statistics.entryAddErrorTrunkLimit,
                                     "ERROR: cpssDxChBrgFdbManagerStatisticsGet : ERROR_TRUNK_LIMIT", prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(entryAddStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E],
                                     statistics.entryAddErrorGlobalEPortLimit,
                                     "ERROR: cpssDxChBrgFdbManagerStatisticsGet : ERROR_GLOBAL_EPORT_LIMIT", prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(entryAddStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_FID_LIMIT_E],
                                     statistics.entryAddErrorFidLimit,
                                     "ERROR: cpssDxChBrgFdbManagerStatisticsGet : ERROR_FID_LIMIT", prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(entryAddStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_LIMIT_E],
                                     statistics.entryAddErrorGlobalLimit,
                                     "ERROR: cpssDxChBrgFdbManagerStatisticsGet : ERROR_GLOBAL_LIMIT", prvTgfDevNum);
    }
}

/**
* @internal prvTgfBrgFdbManagerLearningScan_statistics_traffic_and_verify function
* @endinternal
*
* @brief  send traffic and verify the statistics
*/

GT_VOID prvTgfBrgFdbManagerLearningScan_statistics_traffic_and_verify
(
    IN GT_U32   fdbManagerId,
    IN GT_BOOL  reducedRun
)
{
    GT_STATUS                                               rc;
    GT_U8                                                   portIter;
    GT_U32                                                  packetCnt;
    GT_U32                                                  learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E];
    GT_U32                                                  numEntriesAdded = 0;
    GT_BOOL                                                 entryAdded;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC       throttlingData;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    GT_U32                                                  noSpaceCounter;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     entry;
    GT_U32                                                  errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNumItr;
    GT_U32                                                  maxTotalEntries;
    GT_BOOL                                                 prevState;

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    rc = cpssDxChBrgFdbManagerLearningThrottlingGet(fdbManagerId, &throttlingData);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);

    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
        "prvWrAppDemoFdbManagerControlSet: expected to GT_OK");

    prvTgfPacketL2Part.saMac[4]=1;
    prvTgfPacketL2Part.saMac[5]=1;
    prvTgfPacketVlanTag0Part.vid = 2;
    for(packetCnt = 0; packetCnt < 4; packetCnt++)
    {
        /* setup Packet with single tag */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoSingleTag, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                prvTgfDevNum, prvTgfBurstCount, 0, NULL);

        /* send Packet From port 1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

        /* Change the src MAC */
        prvTgfPacketL2Part.saMac[5]+=1;

        /* sleep to receive the interrupt and process learning scan */
        cpssOsTimerWkAfter(500);
    }

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);
    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    /* Verify Statistics counter */
    cpssOsMemSet(learningScanStatisticsArr, 0, sizeof(learningScanStatisticsArr));
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]           = 0;
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E]   = 4;
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E] = 4;
    prvTgfBrgFdbManagerLearningScan_statistics_verify(fdbManagerId, learningScanStatisticsArr,NULL);


    prvTgfPacketL2Part.saMac[5] = 1;
    for(packetCnt = 0; packetCnt < 4; packetCnt++)
    {
        /* setup Packet with single tag */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoSingleTag, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                prvTgfDevNum, prvTgfBurstCount, 0, NULL);

        /* send Packet From port 2 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

        /* Change the src MAC */
        prvTgfPacketL2Part.saMac[5]+=1;

        /* sleep to receive the interrupt and process learning scan */
        cpssOsTimerWkAfter(500);
    }

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    /* Verify Statistics counter */
    /* Verify Statistics counter */
    cpssOsMemSet(learningScanStatisticsArr, 0, sizeof(learningScanStatisticsArr));
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]              = 0;
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E]      = 8;
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E]    = 4;
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E] = 4;
    prvTgfBrgFdbManagerLearningScan_statistics_verify(fdbManagerId, learningScanStatisticsArr,NULL);
    numEntriesAdded = 4;

    maxTotalEntries = (reducedRun)?100:PRV_TGF_MAX_TOTAL_ENTRIES;
    /* Fill the table till maxTotalEntries entries */
    for(packetCnt = 0; packetCnt < maxTotalEntries; packetCnt++)
    {
        entryAdded = GT_FALSE;
        fdbManagerEntryAdd_macAddr(fdbManagerId, packetCnt, &entryAdded);
        if(entryAdded == GT_TRUE)
        {
            numEntriesAdded+=1;
        }
        if((packetCnt % 10000) == 0)
        {
            PRV_UTF_LOG2_MAC("numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
        }
        if(numEntriesAdded == fdbManagerPtr->thresholdB)
        {
            PRV_UTF_LOG2_MAC("ThresholdB reached, numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
            break;
        }
    }

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    /* Skip no space cache check, In case of reduced run */
    if(reducedRun)
    {
        return;
    }

    prevState = prvTgfTrafficPrintPacketTxEnableSet(GT_FALSE);

    /* we need verify NO_SPACE before reaching threshold C, as at thresholdC - NO_SPACE update will be disabled
     * in case of hash 16 and 128K hw entries - (thresholdC-thresholdB) = 19661 */
    for(packetCnt = 0; packetCnt < (throttlingData.thresholdC - throttlingData.thresholdB); packetCnt++)
    {
        /* setup Packet with single tag */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoSingleTag, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                prvTgfDevNum, prvTgfBurstCount, 0, NULL);

        /* disable printing of packets */
        tgfTrafficTracePacketByteSet(GT_FALSE);

        /* send Packet From port 2 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

        if((packetCnt % 10) == 0)
        {
            /* If No space cache filled completely, break to test the no space filter cache */
            if(fdbManagerPtr->noSpaceCacheEntry.usedEntriesCnt == 4)
            {
                PRV_UTF_LOG1_MAC("NO space cache filled, packetCnt = %d\n", packetCnt);
                break;
            }
        }

        /* Change the src MAC */
        prvTgfPacketL2Part.saMac[5]+=1;
        if(prvTgfPacketL2Part.saMac[5] == 1)    /* 1 Byte overflowed */
        {
            prvTgfPacketL2Part.saMac[4]+=1;
        }
    }

    tgfTrafficTracePacketByteSet(GT_TRUE);
    (void)prvTgfTrafficPrintPacketTxEnableSet(prevState);

    /* sleep for 1 sec */
    cpssOsTimerWkAfter(1000);

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    /* Verify Statistics counter */
    /* Verify no space counter - 4 (Can be >4 due to some packets are processed by learning scan later)
     * No space filter counter - 0 (Must be 0 as no repeat MAC sent)
     **/
     noSpaceCounter = fdbManagerPtr->apiLearningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NO_SPACE_MSG_E];
     UTF_VERIFY_EQUAL0_STRING_MAC(0,
                  fdbManagerPtr->apiLearningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NO_SPACE_FILTERED_MSG_E],
                  "No space counter is not as expected");

    /* read the no space cache from throttlingGet
     * and send the same packets to verify no space filtered counter */
    rc = cpssDxChBrgFdbManagerLearningThrottlingGet(fdbManagerId, &throttlingData);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLearningThrottlingGet: expected to GT_OK");

    for(packetCnt = 0; packetCnt < 4; packetCnt++)
    {
        if(throttlingData.noSpaceCache.noSpaceEntryCache[packetCnt].fdbEntryType ==
            CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE__LAST__E)
        {
            /* no more valid entries in the cache */
            break;
        }

        prvTgfPacketL2Part.saMac[4] = throttlingData.noSpaceCache.noSpaceEntryCache[packetCnt].format.fdbEntryMacAddrFormat.macAddr.arEther[4];
        prvTgfPacketL2Part.saMac[5] = throttlingData.noSpaceCache.noSpaceEntryCache[packetCnt].format.fdbEntryMacAddrFormat.macAddr.arEther[5];

        /* setup Packet with single tag */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoSingleTag, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                prvTgfDevNum, prvTgfBurstCount, 0, NULL);

        /* send Packet From port 2 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);
    }

    if (learnLimits.enableLearningLimits == GT_FALSE)
    {
        if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(4,packetCnt,"Not enough valid cache entries");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(0,packetCnt,"Valid cache entries expected 0");
        }
    }
    else
    {
        /* the limits may cause not to get to threshold problems */
    }

    /* sleep for 1 sec */
    cpssOsTimerWkAfter(1000);

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    /* Verify Statistics counter */
    /* Verify no space counter - (should be same as previous value)
     * No space filter counter - 4 (Must be 4 as the same cache MAC are sent)
     **/
    UTF_VERIFY_EQUAL0_STRING_MAC(noSpaceCounter,
                 fdbManagerPtr->apiLearningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NO_SPACE_MSG_E],
                 "No space counter is not as expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(packetCnt,
                 fdbManagerPtr->apiLearningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NO_SPACE_FILTERED_MSG_E],
                 "No space counter is not as expected");

    /* Verify Delete entry is flushing the no space cache */
    rc = cpssDxChBrgFdbManagerEntryGetNext(fdbManagerId, GT_TRUE, &entry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
           "cpssDxChBrgFdbManagerEntryGetNext: expected to GT_OK");

    rc = cpssDxChBrgFdbManagerEntryDelete(fdbManagerId, &entry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
           "cpssDxChBrgFdbManagerEntryDelete: expected to GT_OK");

    /* read the no space cache, verify entry count should be 0 */
    UTF_VERIFY_EQUAL0_STRING_MAC(0,
                                fdbManagerPtr->noSpaceCacheEntry.usedEntriesCnt,
                                "No space cache used entries is not as expected");
}

/**
* @internal prvTgfBrgFdbManagerLearningScan_statistics_restore function
* @endinternal
*
* @brief  restore default for learning scan
*/

GT_VOID prvTgfBrgFdbManagerLearningScan_statistics_restore
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS           rc;

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* FDB Manager Specific Configuration restore */

    /* Set FDB manager AU Message handler methord as FDB_MANAGER for appDemo */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    CLEANUP_MANAGER(fdbManagerId);

    /* restore FDB size */
    if(!PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        fdbSize =
        PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.fdb = prvSaveFdbSize;
    }
}

/**
* @internal prvTgfBrgFdbManagerLearningScan_config function
* @endinternal
*
* @brief  configures the learning scan related parameters to verify
*         learning scan logic
*/
GT_VOID prvTgfBrgFdbManagerLearningScan_config
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS                                   rc;
    GT_U32                                      portIter = 0;
    GT_TRUNK_ID                                 trunkId;
    CPSS_TRUNK_MEMBER_STC                       enabledMembersArray[PRV_TGF_EMEMBERS_NUM_CNS];


    /* calculate trunk id */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    FDB_SIZE_FULL_INIT();
    /* fill enabled members parameters array */
    for (portIter = 0; portIter < PRV_TGF_EMEMBERS_NUM_CNS; portIter++)
    {
        enabledMembersArray[portIter].hwDevice = prvTgfDevNum;
        enabledMembersArray[portIter].port = prvTgfPortsArray[prvTgfTrunkMembersIdx[portIter]];
    }

    /* AUTODOC: create trunk 2 with ports [0,3] */
    rc = prvTgfTrunkMembersSet(trunkId,
            PRV_TGF_EMEMBERS_NUM_CNS,
            enabledMembersArray, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfTrunkMembersSet: %d", prvTgfDevNum);

    /* add ports to vlan 2 */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
            prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
            prvTgfDevNum, PRV_TGF_VLANID_5_CNS);

    /* create single valid manager */
    rc = createFdbManagerWithDevice(fdbManagerId, PRV_TGF_MAX_TOTAL_ENTRIES, NULL, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "createFdbManager: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Set FDB manager AU Message handler methord as FDB_MANAGER for appDemo */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_DISABLED_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);
}

static CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC             learningParam;
static CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC             restoreLearningParam;

/**
* @internal prvTgfBrgFdbManagerLearningScan_traffic_and_verify function
* @endinternal
*
* @brief  send traffic and verify the statistics
*/

GT_VOID prvTgfBrgFdbManagerLearningScan_traffic_and_verify
(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS                                               rc;
    GT_U8                                                   portIter;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC      paramsPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC        entriesLearningArray[NUM_ENTRIES_STEPS_CNS]; /* TODO - Can we make it 1, as we are sending only 1 packet */
    GT_U32                                                  entriesLearningNum;
    PRV_TGF_MAC_ENTRY_KEY_STC                               macEntryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC                               macEntry;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNumItr;
    GT_U32                                                  errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_ENT      vlanLookUpModeRestore;
    CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_ENT     macEntryMuxingModeRestore;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_ENT         entryMuxingModeRestore;
    GT_U16                                                  prvTgfvidRestore = prvTgfPacketVlanTag0Part.vid;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          learningScanExpectedError[2] = {
        /*CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_SW_BANK_COUNTERS_MISMATCH_E, bug was fixed in CPSS ! */
        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E,
        CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E
    };
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     updatedEntry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC       updatedParams;
    GT_U32                                                  ii;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC                statistics;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /*** CASE_1: Transmit Packet - Generate NA in OUT Array ***/
    paramsPtr.addNewMacUcEntries        = GT_TRUE;
    paramsPtr.addWithRehashEnable       = GT_FALSE;
    paramsPtr.updateMovedMacUcEntries   = GT_FALSE;

    /* setup Packet with single tag */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoSingleTag, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
            prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* send Packet From port 1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

    /* Call Learning Scan(With customized param) to make processing of events */
    rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, entriesLearningArray, &entriesLearningNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "cpssDxChBrgFdbManagerLearningScan: failed");

    /* Verify - NA event in OUT array & Entry(Without SP) in FDB */
    UTF_VERIFY_EQUAL0_STRING_MAC(1, entriesLearningNum, "entriesLearningNum is not as expected:");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_INTERFACE_PORT_E,
            entriesLearningArray[0].entry.format.fdbEntryMacAddrFormat.dstInterface.type,
            "MAC address did not match in port number: ");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
            entriesLearningArray[0].entry.format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.portNum,
            "MAC address did not match in port number: ");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketL2Part.saMac[5],
            entriesLearningArray[0].entry.format.fdbEntryMacAddrFormat.macAddr.arEther[5],
            "MAC address did not match in OUT event: ");
    UTF_VERIFY_EQUAL0_STRING_MAC(entriesLearningArray[0].updateType,
            CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_E,
            "Update Event Type did not match in OUT event: ");

    /* get each FDB entry */
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_2_CNS;
    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, &prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(macEntry.spUnknown, GT_FALSE,
                        "MAC Entry spUnknown is not as expected");

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);
    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    /*** CASE_2: Transmit Packet - Generate MOVED in OUT Array ***/
    /* In case of AC5 this scenario is valid - updated entry will come as a NEW message
     * When key params matches add entry will trigger update entry */
    paramsPtr.updateMovedMacUcEntries = (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?GT_TRUE:GT_FALSE;

    /* send the same Packet From port 2 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_2_CNS);

    cpssOsTimerWkAfter(1000);

    /* Call Learning Scan(With customized param) to make processing of events */
    rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, entriesLearningArray, &entriesLearningNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "cpssDxChBrgFdbManagerLearningScan: failed");

    /* Verify - MOVED event in OUT array & Entry(Without SP) in FDB */
    UTF_VERIFY_EQUAL0_STRING_MAC(1, entriesLearningNum, "entriesLearningNum is not as expected:");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_INTERFACE_PORT_E,
            entriesLearningArray[0].entry.format.fdbEntryMacAddrFormat.dstInterface.type,
            "MAC address did not match in port number: ");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
            entriesLearningArray[0].entry.format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.portNum,
            "MAC address did not match in port number: ");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketL2Part.saMac[5],
            entriesLearningArray[0].entry.format.fdbEntryMacAddrFormat.macAddr.arEther[5],
            "MAC address did not match in OUT event: ");
    UTF_VERIFY_EQUAL0_STRING_MAC(entriesLearningArray[0].updateType,
            NEW_OR_UPDATE_EVENT_PER_DEVICE(),
            "Update Event Type did not match in OUT event: ");

    /* get each FDB entry */
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_2_CNS;
    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, &prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(macEntry.spUnknown, GT_FALSE,
                        "MAC Entry spUnknown is not as expected");

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    /*** CASE_2.a: update manually an entry with fields that are not in a 'moved entry'  ***/
    /***           then send the packet from other port to make it moved ,
     ***           and check that the learning scan give proper info for this entry
     ***           AC5 does not support entry move msg, so this test is not valid        ***/
    updatedEntry = entriesLearningArray[0].entry;
    updatedParams.updateOnlySrcInterface = GT_FALSE;

    /* lets start with getting the current info in the FDB manager about the entry */
    rc = cpssDxChBrgFdbManagerEntryGet(fdbManagerId,&updatedEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbManagerEntryGet: failed");

    updatedEntry.format.fdbEntryMacAddrFormat.daRoute = GT_TRUE;
    updatedEntry.format.fdbEntryMacAddrFormat.daCommand = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    updatedEntry.format.fdbEntryMacAddrFormat.saCommand = CPSS_PACKET_CMD_FORWARD_E;/*cant use CPSS_PACKET_CMD_DROP_SOFT_E because no NA to cpu on security breach */
    updatedEntry.format.fdbEntryMacAddrFormat.daSecurityLevel = 0;/* muxing mode force it to 0 */
    updatedEntry.format.fdbEntryMacAddrFormat.saSecurityLevel = 1;

    rc = cpssDxChBrgFdbManagerEntryUpdate(fdbManagerId,&updatedEntry,&updatedParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbManagerEntryUpdate: failed");

    if((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        paramsPtr.updateMovedMacUcEntries   = GT_TRUE;
    }
    /* send the same Packet From port 1 - to get moved message */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

    cpssOsTimerWkAfter(1000);

    /* Call Learning Scan(With customized param) to make processing of events */
    rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, &entriesLearningArray[0], &entriesLearningNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "cpssDxChBrgFdbManagerLearningScan: failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(1, entriesLearningNum, "entriesLearningNum is not as expected:");

    /* send the same Packet From port 2 - to get moved message */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_2_CNS);

    cpssOsTimerWkAfter(1000);


    /* Call Learning Scan(With customized param) to make processing of events */
    rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, &entriesLearningArray[1], &entriesLearningNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "cpssDxChBrgFdbManagerLearningScan: failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(1, entriesLearningNum, "entriesLearningNum is not as expected:");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
            entriesLearningArray[0].entry.format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.portNum,
            "MAC address did not match in port number: ");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS],
            entriesLearningArray[1].entry.format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.portNum,
            "MAC address did not match in port number: ");
    for(ii = 0 ; ii < 2 ; ii++)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketL2Part.saMac[5],
                entriesLearningArray[ii].entry.format.fdbEntryMacAddrFormat.macAddr.arEther[5],
                "MAC address did not match in OUT event: ");
        UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_INTERFACE_PORT_E,
                entriesLearningArray[ii].entry.format.fdbEntryMacAddrFormat.dstInterface.type,
                "MAC address did not match in port number: ");
        UTF_VERIFY_EQUAL0_STRING_MAC(PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum),
                entriesLearningArray[ii].entry.format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.hwDevNum,
                "MAC address did not match in hwDevNum number: ");
        UTF_VERIFY_EQUAL0_STRING_MAC(entriesLearningArray[ii].updateType,
                NEW_OR_UPDATE_EVENT_PER_DEVICE(),
                "Update Event Type did not match in OUT event: ");
        /* AC5 does not support MAC Move message, so Out event array will not have the updated fields */
        if((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(updatedEntry.format.fdbEntryMacAddrFormat.daRoute,
                    entriesLearningArray[ii].entry.format.fdbEntryMacAddrFormat.daRoute,
                    "MAC address did not match in OUT daRoute: ");
            UTF_VERIFY_EQUAL0_STRING_MAC(updatedEntry.format.fdbEntryMacAddrFormat.daCommand,
                    entriesLearningArray[ii].entry.format.fdbEntryMacAddrFormat.daCommand,
                    "MAC address did not match in OUT daCommand: ");
            UTF_VERIFY_EQUAL0_STRING_MAC(updatedEntry.format.fdbEntryMacAddrFormat.saCommand,
                    entriesLearningArray[ii].entry.format.fdbEntryMacAddrFormat.saCommand,
                    "MAC address did not match in OUT saCommand: ");
            UTF_VERIFY_EQUAL0_STRING_MAC(updatedEntry.format.fdbEntryMacAddrFormat.daSecurityLevel,
                    entriesLearningArray[ii].entry.format.fdbEntryMacAddrFormat.daSecurityLevel,
                    "MAC address did not match in OUT daSecurityLevel: ");
            UTF_VERIFY_EQUAL0_STRING_MAC(updatedEntry.format.fdbEntryMacAddrFormat.saSecurityLevel,
                    entriesLearningArray[ii].entry.format.fdbEntryMacAddrFormat.saSecurityLevel,
                    "MAC address did not match in OUT saSecurityLevel: ");
        }
    }

    /* get each FDB entry */
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_2_CNS;
    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, &prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(macEntry.spUnknown, GT_FALSE,
            "MAC Entry spUnknown is not as expected");

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    if((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        /*** CASE_3: Transmit Packet - Generate MOVED in OUT Array (Without updated in hardware)***/
        paramsPtr.updateMovedMacUcEntries   = GT_FALSE;
        /* send the Packet From port 1 again to generate MOVE */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

        /* Call Learning Scan(With customized param) to make processing of events */
        rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, entriesLearningArray, &entriesLearningNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "cpssDxChBrgFdbManagerLearningScan: failed");

        /* Verify - MOVED event in OUT array & Entry(With SP) in FDB */
        UTF_VERIFY_EQUAL0_STRING_MAC(1, entriesLearningNum, "entriesLearningNum is not as expected:");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketL2Part.saMac[5],
                entriesLearningArray[0].entry.format.fdbEntryMacAddrFormat.macAddr.arEther[5],
                "MAC address did not match in OUT event: ");
        UTF_VERIFY_EQUAL0_STRING_MAC(entriesLearningArray[0].updateType,
                NEW_OR_UPDATE_EVENT_PER_DEVICE(),
                "Update Event Type did not match in OUT event: ");

        /* get each FDB entry */
        macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_2_CNS;
        cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, &prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

        rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(macEntry.spUnknown, GT_TRUE,
                "MAC Entry spUnknown is not as expected");

        /* Verify DB */
        rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 1);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(learningScanExpectedError[errorNumItr], testResultArray[errorNumItr],
                    "Expected error is not matching");
        }
    }

    /*** CASE_4: Transmit Packet - Generate NA in OUT Array (Without Learnt in hardware)***/
    prvTgfPacketL2Part.saMac[5]+=1; /* New Packet SA */
    paramsPtr.addNewMacUcEntries        = GT_FALSE;
    /* send the Packet From port 1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

    /* Call Learning Scan(With customized param) to make processing of events */
    rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, entriesLearningArray, &entriesLearningNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "cpssDxChBrgFdbManagerLearningScan: failed");

    /* Verify - NA event in OUT array & Entry(With SP) in FDB */
    UTF_VERIFY_EQUAL0_STRING_MAC(1, entriesLearningNum, "entriesLearningNum is not as expected:");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketL2Part.saMac[5],
            entriesLearningArray[0].entry.format.fdbEntryMacAddrFormat.macAddr.arEther[5],
            "MAC address did not match in OUT event: ");
    UTF_VERIFY_EQUAL0_STRING_MAC(entriesLearningArray[0].updateType,
            CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_E,
            "Update Event Type did not match in OUT event: ");

    /* get each FDB entry */
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_2_CNS;
    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, &prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(macEntry.spUnknown, GT_TRUE,
                        "MAC Entry spUnknown is not as expected");

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    if((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 2);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(learningScanExpectedError[errorNumItr], testResultArray[errorNumItr],
                    "Expected error is not matching");
        }
    }
    else
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 1);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E,
                    testResultArray[errorNumItr],
                    "Expected error is not matching");
        }
    }

    /*** CASE_5: Transmit Packet(From Trunk) - Generate NA in OUT Array ***/
    paramsPtr.addNewMacUcEntries        = GT_TRUE;
    paramsPtr.addWithRehashEnable       = GT_FALSE;
    paramsPtr.updateMovedMacUcEntries   = GT_FALSE;

    prvTgfPacketL2Part.saMac[5]+=1; /* New Packet SA */
    /* send Packet From port 0 (Trunk port) */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfTrunkMembersIdx[0]]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[prvTgfTrunkMembersIdx[0]]);

    /* Call Learning Scan(With customized param) to make processing of events */
    rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, entriesLearningArray, &entriesLearningNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "cpssDxChBrgFdbManagerLearningScan: failed");

    /* Verify - NA event in OUT array & Entry(Without SP) in FDB */
    UTF_VERIFY_EQUAL0_STRING_MAC(1, entriesLearningNum, "entriesLearningNum is not as expected:");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketL2Part.saMac[5],
            entriesLearningArray[0].entry.format.fdbEntryMacAddrFormat.macAddr.arEther[5],
            "MAC address did not match in OUT event: ");
    UTF_VERIFY_EQUAL0_STRING_MAC(entriesLearningArray[0].updateType,
            CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_E,
            "Update Event Type did not match in OUT event: ");

    /* get each FDB entry */
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_2_CNS;
    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, &prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(macEntry.dstInterface.trunkId, 2,
                        "MAC Entry trunk ID is not as expected");
    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    if((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 2);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(learningScanExpectedError[errorNumItr], testResultArray[errorNumItr],
                    "Expected error is not matching");
        }
    }
    else
    {
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 1);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E,
                    testResultArray[errorNumItr],
                    "Expected error is not matching");
        }
    }

    /*** CASE_6: create new FDB manager with VLAN lookup mode MAC only
     * Transmit Packet(With Vlan ID) - FDB manager should remove the entry ***/
    /* Remove FDB Manager */
    rc = prvTgfBrgFdbManagerHWFlushAll(MANAGER_GET_MAC(fdbManagerId));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbManagerHWFlushAll: %d", GT_TRUE);
    CLEANUP_MANAGER(fdbManagerId);

    vlanLookUpMode = CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_ONLY_E;
    /* create single valid manager */
    rc = createFdbManagerWithDevice(fdbManagerId, PRV_TGF_MAX_TOTAL_ENTRIES, NULL, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "createFdbManager: expected to GT_OK on manager [%d]",
            fdbManagerId);

    prvTgfPacketL2Part.saMac[5]  +=1; /* New Packet SA */
    prvTgfPacketVlanTag0Part.vid = 2;
    paramsPtr.addNewMacUcEntries = GT_TRUE;
    /* send the Packet From port 1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

    /* get each FDB entry - Verify SP - enable (Vlan as 5)*/
    macEntryKey.entryType           = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId  = 2;
    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, &prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, macEntry.spUnknown, "MAC Entry spUnknown is not as expected");

    /* Call Learning Scan(With customized param) to make processing of events */
    rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, entriesLearningArray, &entriesLearningNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "cpssDxChBrgFdbManagerLearningScan: failed");

    /* get each FDB entry - Verify Entry should found (SP should be disabled) */
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, macEntry.spUnknown, "MAC Entry spUnknown is not as expected");

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    /*** CASE_7: send the same Packet From port 1 - different vlan*/
    prvTgfPacketVlanTag0Part.vid = PRV_TGF_VLANID_5_CNS;
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

    /* Call Learning Scan(With customized param) to make processing of events */
    rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, entriesLearningArray, &entriesLearningNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "cpssDxChBrgFdbManagerLearningScan: failed");

    /* get FDB entry - Verify Entry should found with old vlan ID */
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, macEntry.spUnknown, "MAC Entry spUnknown is not as expected");

    /* New VLAN with the MAC should not be there */
    macEntryKey.key.macVlan.vlanId  = PRV_TGF_VLANID_5_CNS;
    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, &prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    /*** CASE_8: FDB Manager with few ports learning disable status
     * SP Entry should not be created for traffic from these ports */
    prvTgfPacketL2Part.saMac[5]  +=1; /* New Packet SA */
    prvTgfPacketVlanTag0Part.vid = 2;
    paramsPtr.addNewMacUcEntries = GT_TRUE;

    /* Get the port learning attributes */
    rc = cpssDxChBrgFdbManagerPortLearningGet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
            &restoreLearningParam);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerPortLearningGet: %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);

    /* Disable port learning status */
    learningParam = restoreLearningParam;
    learningParam.naMsgToCpuEnable  = GT_FALSE;
    rc = cpssDxChBrgFdbManagerPortLearningSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
            &learningParam);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerPortLearningSet: %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);

    /* send the Packet From port 1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

    /* get each FDB entry - SP entry should not be created )*/
    macEntryKey.entryType           = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId  = 2;
    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, &prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

    /* Call Learning Scan(With customized param) to make processing of events */
    rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, entriesLearningArray, &entriesLearningNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "cpssDxChBrgFdbManagerLearningScan: failed");

    /* get each FDB entry - No entry should be present */
    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    /* Restore - Enable port learning status */
    rc = cpssDxChBrgFdbManagerPortLearningSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
            &restoreLearningParam);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerPortLearningSet: %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);


    /*** CASE_9: Create SP entry and delete with EntryTempDelete API */
    vlanLookUpModeRestore       = vlanLookUpMode;
    macEntryMuxingModeRestore   = macEntryMuxingMode;
    entryMuxingModeRestore      = entryMuxingMode;
    vlanLookUpMode              = CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_ONLY_E;

    for(;vlanLookUpMode<=CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E; vlanLookUpMode++)
    {
        /* cleanup & create single valid manager - as VLAN mode changed */
        if((!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)) &&
                (vlanLookUpMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E))
        {
            continue;
        }
        CLEANUP_MANAGER(fdbManagerId);
        if(vlanLookUpMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E)
        {
            macEntryMuxingMode  = CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E;
            entryMuxingMode     = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E;
            setAllVlansModeVid1lookup(GT_TRUE, PRV_TGF_VLANID_5_CNS);
            macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_FID_VID1_E;
        }
        else
        {
            macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        }
        rc = createFdbManagerWithDevice(fdbManagerId, PRV_TGF_MAX_TOTAL_ENTRIES, NULL, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "createFdbManager: expected to GT_OK on manager [%d]",
                fdbManagerId);

        prvTgfPacketL2Part.saMac[5]  +=1; /* New Packet SA */
        prvTgfPacketVlanTag0Part.vid = PRV_TGF_VLANID_5_CNS;
        /* send the Packet From port 1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

        /* get FDB Entry - SP entry should be created */
        macEntryKey.vid1                = 0;
        macEntryKey.key.macVlan.vlanId  = PRV_TGF_VLANID_5_CNS;
        cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, &prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, macEntry.spUnknown, "MAC Entry spUnknown is not as expected");

        /* Verify DB - Sp entry exist error expected */
        rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 1);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E,
                    testResultArray[errorNumItr],
                    "Expected error is not matching");
        }

        /* Call Learning Scan(With customized param) to make processing of events */
        paramsPtr.addNewMacUcEntries        = GT_FALSE;
        rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, entriesLearningArray, &entriesLearningNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "cpssDxChBrgFdbManagerLearningScan: failed");

        /* Expecting a temp entry */
        UTF_VERIFY_EQUAL0_STRING_MAC(entriesLearningNum, 1, "Expected AU msg not matched");

        /* Delete temp Entry */
        rc = cpssDxChBrgFdbManagerEntryTempDelete(fdbManagerId,
                &entriesLearningArray[0].entry,
                entriesLearningArray[0].updateInfo.newEntryFormat.tempEntryOffset);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbManagerEntryTempDelete: failed");

        paramsPtr.addNewMacUcEntries        = GT_TRUE;
        rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, entriesLearningArray, &entriesLearningNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "cpssDxChBrgFdbManagerLearningScan: failed");

        /* get FDB entry - No entry should be present */
        rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

        /* Verify DB */
        rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                    errorNumItr,
                    testResultArray[errorNumItr]);
        }

        /* verify the statistics */
        rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
        /* Verify temp delete statistics */
        UTF_VERIFY_EQUAL1_STRING_MAC(1, statistics.entryTempDeleteOk, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.entryTempDeleteErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    }

    /* Restore vlan mode */
    setAllVlansModeVid1lookup(GT_FALSE, PRV_TGF_VLANID_5_CNS);
    vlanLookUpMode               = vlanLookUpModeRestore;
    prvTgfPacketVlanTag0Part.vid = prvTgfvidRestore;
    macEntryMuxingMode           = macEntryMuxingModeRestore;
    entryMuxingMode              = entryMuxingModeRestore;
}

/**
* @internal prvTgfBrgFdbManagerLearningScan_restore function
* @endinternal
*
* @brief  restore default for learning scan
*/

GT_VOID prvTgfBrgFdbManagerLearningScan_restore
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS           rc;
    /* General Configuration restore */

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_5_CNS);

    /* FDB Manager Specific Configuration restore */

    /* restore trunk 2 */
    rc = prvTgfTrunkMembersSet(
            PRV_TGF_TRUNK_ID_CNS /*trunkId*/,
            0 /*numOfEnabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL /*enabledMembersArray*/,
            0 /*numOfDisabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* Set FDB manager AU Message handler methord as FDB_MANAGER for appDemo */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    vlanLookUpMode = CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E;
    prvTgfPacketVlanTag0Part.vid = PRV_TGF_VLANID_2_CNS;

    CLEANUP_MANAGER(fdbManagerId);

    /* Learning scan test is dealing with manual call to learning scan with different combination
     * which creates some SP entry and counter issue, clean it up for next test case */
    prvTgfBrgFdb_cleanup(GT_FALSE);
}

/**
* @internal prvTgfBrgFdbManagerAgingScan_config function
* @endinternal
*
* @brief  configures the aging scan related parameters to verify
*         aging scan logic
*/
GT_VOID prvTgfBrgFdbManagerAgingScan_config
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS                                   rc;

    FDB_SIZE_FULL_INIT();
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
            prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* create single valid manager */
    rc = createFdbManagerWithDevice(fdbManagerId, PRV_TGF_MAX_TOTAL_ENTRIES, NULL, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "createFdbManager: expected to GT_OK on manager [%d]",
            fdbManagerId);

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
}

static GT_VOID prvTgfBrgFdbManagerAgingScan_statistics_verify
(
    IN GT_U32           fdbManagerId,
    IN GT_U32           agingScanStatisticsArr[]
)
{
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC                statistics;
    GT_STATUS                                               rc;
    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    UTF_VERIFY_EQUAL1_STRING_MAC(agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_OK_E],
                                 statistics.scanAgingOk,
                                 "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_ERROR_INPUT_INVALID_E],
                                 statistics.scanAgingErrorInputInvalid,
                                 "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_E],
                                 statistics.scanAgingTotalAgedOutEntries,
                                 "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_DELETED_E],
                                 statistics.scanAgingTotalAgedOutDeleteEntries,
                                 "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
}

static GT_VOID prvCpssDxChBrgFdbManagerAgeBinUsage_Verify
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC      *fdbManagerPtr,
    IN  GT_U32                                              numEntriesAdded
)
{
    GT_U32                                              i;
    /* /256 Number of ageBin bits should be used */
    GT_U32                                              ageBinIndex = (numEntriesAdded >> 8);

    for(i = 0; i < ageBinIndex; i++)
    {
        if( ((fdbManagerPtr->agingBinInfo.level1UsageBitmapPtr[i >> 5] & (1 << (i & 0x1F))) == 0) ||
             (fdbManagerPtr->agingBinInfo.ageBinListPtr[i].totalUsedEntriesCnt != NUM_ENTRIES_STEPS_CNS) )
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "ageBin usage matrix check failed");
            break;
        }
        if((i != 0) && ((i % 32) == 0))
        {
            if((fdbManagerPtr->agingBinInfo.level2UsageBitmap & (1 << ((i >>  5) -1))) == 0)
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "ageBin usage matrix check failed");
                break;
            }
        }
    }

    /* Last bin will have the less entries, others should full full capacity*/
    UTF_VERIFY_EQUAL0_STRING_MAC(fdbManagerPtr->agingBinInfo.ageBinListPtr[ageBinIndex].totalUsedEntriesCnt,
                                    (numEntriesAdded & 0xFF),
                                    "ageBin usage matrix check failed");
}

/**
* @internal prvTgfBrgFdbManager_manual_config function
* @endinternal
*
* @brief  Create FDB manager and add some random entries
*/
GT_VOID prvTgfBrgFdbManager_manual_config
(
    IN GT_U32       fdbManagerId,
    IN GT_U32       addNum
)
{
    GT_STATUS            rc;
    GT_U32               totalSize = 512, failed, ii;
    GT_BOOL              entryAdded;

    /* create single valid manager */
    rc = createFdbManagerWithDevice(fdbManagerId, totalSize, NULL, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "createFdbManager: expected to GT_OK on manager [%d]",
            fdbManagerId);

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

    failed = 0;
    for(ii = 0; ii < addNum; ii++)
    {
        entryAdded = GT_FALSE;
        fdbManagerEntryAdd_macAddr(fdbManagerId, ii, &entryAdded);
        if(!entryAdded)
        {
            failed++;
        }
    }

    cpssOsPrintf("\nFDB Manager [%d] created with size [%d], added [%d] entries\n", fdbManagerId, totalSize, (addNum-failed));

}

/**
* @internal prvTgfBrgFdbManagerAgingScan_traffic_and_verify function
* @endinternal
*
* @brief  send traffic and verify the statistics
*/

GT_VOID prvTgfBrgFdbManagerAgingScan_traffic_and_verify
(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS                                               rc;
    GT_U8                                                   portIter;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC        entriesAgingArray[NUM_ENTRIES_STEPS_CNS];
    GT_U32                                                  entriesAgingNum;
    CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC         ageParams;
    GT_U32                                                  packetCnt;
    GT_BOOL                                                 entryAdded;
    GT_U32                                                  numEntriesAdded = 0;
    GT_U32                                                  agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS___LAST___E];
    GT_U32                                                  i;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    GT_U32                                                  errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNumItr;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC                statistics;
    GT_U32                                                  numOfStaticEntries = 0;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);
    if(fdbManagerPtr == NULL)
    {
        return;
    }

    /* Verify that the static entries are not aged out even if aged-out */
    prvIsStatic = GT_TRUE;

    /*** CASE_1: Age scan verify - with delete ***/
    for(packetCnt = 0; packetCnt < PRV_TGF_MAX_TOTAL_ENTRIES; packetCnt++)
    {
        entryAdded = GT_FALSE;
        /* Add first 100 static entries to make sure there is no aging on them */
        if (numOfStaticEntries < 100)
        {
            numOfStaticEntries += 1;
        }
        else
        {
            prvIsStatic = GT_FALSE;
        }

        fdbManagerEntryAdd_macAddr(fdbManagerId, packetCnt, &entryAdded);
        if(entryAdded == GT_TRUE)
        {
            numEntriesAdded+=1;
        }
        if((packetCnt % 100) == 0)
        {
            prvCpssDxChBrgFdbManagerAgeBinUsage_Verify(fdbManagerPtr, numEntriesAdded);
            PRV_UTF_LOG2_MAC("numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
        }

        /* To verify the age-bin usage matrix level1/level2
         *      - each 256 entries      - 1 level_1 bit
         *      - each 32 level1 bit    - 1 level_2 bit
         */
        if(numEntriesAdded == ((256 * 32) * 3))
        {
            PRV_UTF_LOG2_MAC("Required entries reached, numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
            break;
        }
    }

    cpssOsMemSet(&ageParams, 0, sizeof(ageParams));
    ageParams.checkAgeMacUcEntries          = GT_TRUE;
    ageParams.deleteAgeoutMacUcEportEntries = GT_TRUE;

    cpssOsMemSet(&agingScanStatisticsArr, 0, sizeof(agingScanStatisticsArr));
    /* Call aging scan - First iteration - for complete DB */
    for(i = 0; i < fdbManagerPtr->agingBinInfo.totalAgeBinAllocated; i++)
    {
        rc = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &ageParams, entriesAgingArray, &entriesAgingNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerAgingScan", prvTgfDevNum);

        agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_OK_E] += 1;
        prvTgfBrgFdbManagerAgingScan_statistics_verify(fdbManagerId, agingScanStatisticsArr);
    }

    /* Call aging scan - Second time iteration */
    for(i = 0; i < fdbManagerPtr->agingBinInfo.totalAgeBinAllocated; i++)
    {
        rc = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &ageParams, entriesAgingArray, &entriesAgingNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerAgingScan", prvTgfDevNum);

        agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_OK_E] += 1;
        agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_DELETED_E] += 256;

        /* Iteration may go beyond added entries, but processed entries will be the maximum entries added */
        if(agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_DELETED_E] > numEntriesAdded)
        {
            agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_DELETED_E] = numEntriesAdded;
        }

        /*clear check ageBinIndex*/
        if((fdbManagerPtr->agingBinInfo.level1UsageBitmapPtr[i >> 5] & (1 << (i & 0x1F))) != 0)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FALSE, "ERROR: level1 usage matrix check failed", prvTgfDevNum);
        }
        if( (i != 0) && ((i % 32) == 0) &&
            ((fdbManagerPtr->agingBinInfo.level2UsageBitmap & (1 << (i >>  5))) != 0) )
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FALSE, "ERROR: level2 usage matrix check failed", prvTgfDevNum);
        }
    }

    agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_DELETED_E] -= numOfStaticEntries;

    /* Check deleted aged-out deleted entries */
    prvTgfBrgFdbManagerAgingScan_statistics_verify(fdbManagerId, agingScanStatisticsArr);

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);
    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    /* CASE_2: Age scan verify - With filter
     * first age scan iteration - for both FID 3 and FID 7 (Using mask as0x3)
     * Second iteration for FID - 3 (Mask F)
     * Second iteration for FID - 7 (Mask F)
     */
    prvSpecificFidFilterValue = 3;
    numEntriesAdded = 0;
    for(packetCnt = 0; packetCnt < PRV_TGF_MAX_TOTAL_ENTRIES; packetCnt++)
    {
        entryAdded = GT_FALSE;
        fdbManagerEntryAdd_macAddr(fdbManagerId, packetCnt, &entryAdded);
        if(entryAdded == GT_TRUE)
        {
            numEntriesAdded+=1;
        }
        if((packetCnt % 30) == 0)
        {
            PRV_UTF_LOG2_MAC("numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
        }

        if(numEntriesAdded == 128)
        {
            PRV_UTF_LOG1_MAC("numEntriesAdded = %d, with FID - 3\n", numEntriesAdded);
            prvSpecificFidFilterValue = 7; /* change fid */
        }

        if(numEntriesAdded == 256)
        {
            PRV_UTF_LOG1_MAC("numEntriesAdded = %d, with FID - 7\n", numEntriesAdded);
            break;
        }
    }

    cpssOsMemSet(&ageParams, 0, sizeof(ageParams));
    ageParams.checkAgeMacUcEntries          = GT_TRUE;
    ageParams.deleteAgeoutMacUcEportEntries = GT_TRUE;
    ageParams.fid                           = 3;
    ageParams.fidMask                       = 0x3;  /* First age pass for FID (3 & 7) */

    /* Call aging scan - First iteration - for complete DB */
    for(i = 0; i < fdbManagerPtr->agingBinInfo.totalAgeBinAllocated; i++)
    {
        rc = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &ageParams, entriesAgingArray, &entriesAgingNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerAgingScan", prvTgfDevNum);

        agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_OK_E] += 1;
        prvTgfBrgFdbManagerAgingScan_statistics_verify(fdbManagerId, agingScanStatisticsArr);
    }

    ageParams.fid                           = 7;
    ageParams.fidMask                       = 0xFF;  /* second age pass for FID 7 Only */
    /* Call aging scan - Second time iteration - Complete the age scan(To make the last age-bin to scan as 0) */
    for(i = 0; i < fdbManagerPtr->agingBinInfo.totalAgeBinAllocated; i++)
    {
        rc = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &ageParams, entriesAgingArray, &entriesAgingNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerAgingScan", prvTgfDevNum);

        agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_OK_E] += 1;
    }

    /* Total entries should have deleted is 128 */
    agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_DELETED_E] += 128;
    prvTgfBrgFdbManagerAgingScan_statistics_verify(fdbManagerId, agingScanStatisticsArr);

    ageParams.fid                           = 3;
    ageParams.fidMask                       = 0xFF;  /* second age pass for FID 3 Only */
    /* Call aging scan - Second time iteration */
    for(i = 0; i < fdbManagerPtr->agingBinInfo.totalAgeBinAllocated; i++)
    {
        rc = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &ageParams, entriesAgingArray, &entriesAgingNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerAgingScan", prvTgfDevNum);
        agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_OK_E] += 1;
    }
    /* Complete the age scan(To make the last age-bin to scan as 0), total entries should have deleted is 128 */
    agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_DELETED_E] += 128;
    prvTgfBrgFdbManagerAgingScan_statistics_verify(fdbManagerId, agingScanStatisticsArr);
    /* Restore back the filter */
    prvSpecificFidFilterValue = 1;

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    /*** CASE_3: Age scan verify - with out delete ***/
    numEntriesAdded = 0;

    for(packetCnt = 0; packetCnt < PRV_TGF_MAX_TOTAL_ENTRIES; packetCnt++)
    {
        entryAdded = GT_FALSE;
        fdbManagerEntryAdd_macAddr(fdbManagerId, packetCnt, &entryAdded);
        if(entryAdded == GT_TRUE)
        {
            numEntriesAdded+=1;
        }
        if((packetCnt % 1000) == 0)
        {
            PRV_UTF_LOG2_MAC("numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
        }

        /* To verify the age-bin usage matrix level1/level2
         *      - each 256 entries      - 1 level_1 bit
         *      - each 32 level1 bit    - 1 level_2 bit
         */
        if(numEntriesAdded == ((256 * 32) * 3))
        {
            PRV_UTF_LOG2_MAC("Required entries reached, numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
            break;
        }
    }

    cpssOsMemSet(&ageParams, 0, sizeof(ageParams));
    ageParams.checkAgeMacUcEntries          = GT_TRUE;
    ageParams.deleteAgeoutMacUcEportEntries = GT_FALSE;

    /* Call aging scan - First iteration - for complete DB */
    for(i = 0; i < fdbManagerPtr->agingBinInfo.totalAgeBinAllocated; i++)
    {
        rc = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &ageParams, entriesAgingArray, &entriesAgingNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerAgingScan", prvTgfDevNum);

        agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_OK_E] += 1;
        prvTgfBrgFdbManagerAgingScan_statistics_verify(fdbManagerId, agingScanStatisticsArr);
    }

    /* Call aging scan - Second time iteration */
    for(i = 0; i < fdbManagerPtr->agingBinInfo.totalAgeBinAllocated; i++)
    {
        rc = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &ageParams, entriesAgingArray, &entriesAgingNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerAgingScan", prvTgfDevNum);

        agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_OK_E] += 1;
        agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_E] += 256;

        /* Iteration may go beyond added entries, but processed entries will be the maximum entries added */
        if(agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_E] > numEntriesAdded)
        {
            agingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_E] = numEntriesAdded;
        }

        /*clear check ageBinIndex*/
        if((fdbManagerPtr->agingBinInfo.level1UsageBitmapPtr[i >> 5] & (1 << (i & 0x1F))) != 0)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FALSE, "ERROR: level1 usage matrix check failed", prvTgfDevNum);
        }
        if( (i != 0) && ((i % 32) == 0) &&
            ((fdbManagerPtr->agingBinInfo.level2UsageBitmap & (1 << (i >>  5))) != 0) )
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FALSE, "ERROR: level2 usage matrix check failed", prvTgfDevNum);
        }
    }

    /* Check aged-out entries */
    prvTgfBrgFdbManagerAgingScan_statistics_verify(fdbManagerId, agingScanStatisticsArr);

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    /*** CASE_4: Update entry update the age-bit ***/
    CLEANUP_MANAGER(fdbManagerId);

    /* create single valid manager */
    rc = createFdbManagerWithDevice(fdbManagerId, PRV_TGF_MAX_TOTAL_ENTRIES, NULL, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "createFdbManager: expected to GT_OK on manager [%d]",
            fdbManagerId);

    prvTgfPacketL2Part.saMac[4]=1;
    prvTgfPacketL2Part.saMac[5]=1;
    for(packetCnt = 0; packetCnt < 4; packetCnt++)
    {
        /* setup Packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoSingleTag, 1, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                prvTgfDevNum, prvTgfBurstCount, 0, NULL);

        /* send Packet From port 1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

        /* Change the src MAC */
        prvTgfPacketL2Part.saMac[5]+=1;
    }

    /* sleep for 2 sec - to receive the interrupt and process learning scan */
    cpssOsTimerWkAfter(2000);

    /* Verify 4 entries learnt */
    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsGet: expected to GT_OK on manager [%d]",
            fdbManagerId);
    UTF_VERIFY_EQUAL0_STRING_MAC(statistics.entryAddOkRehashingStage0, 4,
            "cpssDxChBrgFdbManagerStatisticsGet: Added entry is not as expected [%d]");
    UTF_VERIFY_EQUAL0_STRING_MAC(statistics.scanLearningTotalHwEntryNewMessages, 4,
            "cpssDxChBrgFdbManagerStatisticsGet: Added entry is not as expected [%d]");

    /* Call 2 times iteration for aging scan */
    for(i = 0; i < fdbManagerPtr->agingBinInfo.totalAgeBinAllocated*2; i++)
    {
        rc = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &ageParams, entriesAgingArray, &entriesAgingNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerAgingScan", prvTgfDevNum);
    }

    /* verify no entries should have aged-out */
    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsGet: expected to GT_OK on manager [%d]",
            fdbManagerId);
    UTF_VERIFY_EQUAL0_STRING_MAC(statistics.scanAgingTotalAgedOutEntries, 4,
            "cpssDxChBrgFdbManagerStatisticsGet: Aged out is not as expected [%d]");
    UTF_VERIFY_EQUAL0_STRING_MAC(statistics.scanAgingTotalAgedOutDeleteEntries, 0,
            "cpssDxChBrgFdbManagerStatisticsGet: Aged out deleted is not as expected [%d]");

    /* Update Entry - Send same packets from port2 */
    prvTgfPacketL2Part.saMac[4]=1;
    prvTgfPacketL2Part.saMac[5]=1;
    for(packetCnt = 0; packetCnt < 4; packetCnt++)
    {
        /* setup Packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoSingleTag, 1, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                prvTgfDevNum, prvTgfBurstCount, 0, NULL);

        /* send Packet From port 1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

        /* Change the src MAC */
        prvTgfPacketL2Part.saMac[5]+=1;
    }

    /* sleep for 2 sec - to receive the interrupt and process learning scan */
    cpssOsTimerWkAfter(2000);

    /* Verify update entry counter should be 4 */
    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsGet: expected to GT_OK on manager [%d]",
            fdbManagerId);
    UTF_VERIFY_EQUAL0_STRING_MAC(statistics.entryUpdateOk, 4,
            "cpssDxChBrgFdbManagerStatisticsGet: Aged out deleted is not as expected [%d]");

    /* Call first iteration for aging scan - for updated entry
     * Entry should not be aged-out, as after update the age bit should have been updated */
    for(i = 0; i < fdbManagerPtr->agingBinInfo.totalAgeBinAllocated; i++)
    {
        rc = cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &ageParams, entriesAgingArray, &entriesAgingNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerAgingScan", prvTgfDevNum);
    }

    /* Verify no entries should have aged-out */
    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsGet: expected to GT_OK on manager [%d]",
            fdbManagerId);
    UTF_VERIFY_EQUAL0_STRING_MAC(statistics.scanAgingTotalAgedOutEntries, 4,
            "cpssDxChBrgFdbManagerStatisticsGet: Aged out is not as expected [%d]");
    UTF_VERIFY_EQUAL0_STRING_MAC(statistics.scanAgingTotalAgedOutDeleteEntries, 0,
            "cpssDxChBrgFdbManagerStatisticsGet: Aged out deleted is not as expected");
}

/**
* @internal prvTgfBrgFdbManagerAgingScan_restore function
* @endinternal
*
* @brief  restore default for aging scan
*/

GT_VOID prvTgfBrgFdbManagerAgingScan_restore
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS           rc;

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* General Configuration restore */
    /* FDB Manager Specific Configuration restore */

    /* Set FDB manager AU Message handler methord as FDB_MANAGER for appDemo */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);

    CLEANUP_MANAGER(fdbManagerId);
}

/**
* @internal prvTgfBrgFdbManagerAgingScan_performance_config function
* @endinternal
*
* @brief  configures the aging scan performance
*/
GT_VOID prvTgfBrgFdbManagerAgingScan_performance_config
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS                                   rc;
    CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC      capacity;


    FDB_SIZE_FULL_INIT();

    /* Customize HW capacity */
    cpssOsMemSet(&capacity, 0, sizeof(capacity));
    capacity.hwCapacity.numOfHwIndexes   = PRV_TGF_TOTAL_HW_CAPACITY;
    capacity.hwCapacity.numOfHashes      = 16;
    capacity.maxEntriesPerLearningScan   = PRV_TGF_MAX_ENTRIES_PER_LEARNING_SCAN;
    capacity.maxEntriesPerAgingScan      = (1 * NUM_ENTRIES_STEPS_CNS);

    /* create single valid manager */
    rc = createFdbManagerWithDevice(fdbManagerId, PRV_TGF_MAX_TOTAL_ENTRIES, &capacity, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "createFdbManager: expected to GT_OK on manager [%d]",
            fdbManagerId);

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
}

/**
* @internal prvTgfBrgFdbManagerAgingScan_performance_traffic_and_verify function
* @endinternal
*
* @brief  send traffic and verify the performance
*/

GT_VOID prvTgfBrgFdbManagerAgingScan_performance_traffic_and_verify
(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS                                               rc;
    GT_U32                                                  packetCnt;
    GT_BOOL                                                 entryAdded;
    GT_U32                                                  numEntriesAdded = 0;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    GT_U32                                                  i;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC                statistics;
    APP_DEMO_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC       appDemoStatistics;
    GT_U32                                                  requiredTimeToAgedOut;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC                  counters;
    GT_U32                                                  errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNumItr;

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);
    if(fdbManagerPtr == NULL)
    {
        return;
    }

    /* Add the first entry as static and next onwards dynamic entries
     * Verify that the static entries are not deleted even if aged-out */
    prvIsStatic = GT_TRUE;

    /* Fill the table till PRV_TGF_MAX_TOTAL_ENTRIES entries */
    for(packetCnt = 0; packetCnt < PRV_TGF_MAX_TOTAL_ENTRIES; packetCnt++)
    {
        entryAdded = GT_FALSE;
        fdbManagerEntryAdd_macAddr(fdbManagerId, packetCnt, &entryAdded);
        if(entryAdded == GT_TRUE)
        {
            numEntriesAdded+=1;
        }
        if((packetCnt % 100) == 0)
        {
            prvCpssDxChBrgFdbManagerAgeBinUsage_Verify(fdbManagerPtr, numEntriesAdded);
            PRV_UTF_LOG2_MAC("numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
        }
        if(numEntriesAdded == 512)
        {
            PRV_UTF_LOG2_MAC("Required entries reached, numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
            break;
        }
        prvIsStatic = GT_FALSE;
    }

    /* wait for age-out */
    requiredTimeToAgedOut = (PRV_TGF_AGE_OUT_IN_SEC * 2);

    /* Enable the aging task with timeout */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E, GT_TRUE, PRV_TGF_AGE_OUT_IN_SEC);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    for(i = 1; i <= requiredTimeToAgedOut; i++)
    {
        cpssOsTimerWkAfter(1000);
        PRV_UTF_LOG1_MAC("waiting for age-out = %d sec\n", (PRV_TGF_AGE_OUT_IN_SEC * 2) - i);

        /* Entry should exist at least 1x time of configured age-out */
        if(i == PRV_TGF_AGE_OUT_IN_SEC-1)
        {
            /* Counter should be valid */
            cpssOsMemSet(&counters, 0, sizeof(counters));
            counters.macUnicastDynamic      = numEntriesAdded - 1;
            counters.macUnicastStatic       = 1;
            counters.usedEntries            = numEntriesAdded;
            counters.freeEntries            = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntries;
            prvCpssDxChBrgFdbManagerCounterVerify(fdbManagerId, &counters);
        }
    }

    /* Buffer */
    cpssOsTimerWkAfter(2000);

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);
    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    /* verify the statistics */
    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    /* Dynamic entry - aged-out and deleted */
    UTF_VERIFY_EQUAL1_STRING_MAC(numEntriesAdded-1, statistics.scanAgingTotalAgedOutDeleteEntries, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    /* Verify no errors encountered in aging task - Aging task statistics */
    rc = prvWrAppDemoFdbManagerAutoAgingStatisticsGet(&appDemoStatistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: prWrAppDemoFdbManagerAutoAgingStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, appDemoStatistics.autoAgingOk,             "autoAging Statistics: Not as expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, appDemoStatistics.autoAgingErrorInputInvalid, "autoAging Statistics: Not as expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, appDemoStatistics.autoAgingErrorFail,         "autoAging Statistics: Not as expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, appDemoStatistics.autoAgingErrorHwError,      "autoAging Statistics: Not as expected");

    /* Disable aging task */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Buffer if any ongoing call to CPSS aging api */
    cpssOsTimerWkAfter(200);
    rc = prvWrAppDemoFdbManagerAutoAgingStatisticsClear();

    /* Verify auto aging timer task should not call CPSS aging API */
    cpssOsTimerWkAfter(200);
    rc = prvWrAppDemoFdbManagerAutoAgingStatisticsGet(&appDemoStatistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: prWrAppDemoFdbManagerAutoAgingStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(0, appDemoStatistics.autoAgingOk,                "autoAging Statistics: Not as expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, appDemoStatistics.autoAgingErrorInputInvalid, "autoAging Statistics: Not as expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, appDemoStatistics.autoAgingErrorFail,         "autoAging Statistics: Not as expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, appDemoStatistics.autoAgingErrorHwError,      "autoAging Statistics: Not as expected");
}

/**
* @internal prvTgfBrgFdbManagerAgingScan_performance_restore function
* @endinternal
*
* @brief  restore default for aging performance test
*/
GT_VOID prvTgfBrgFdbManagerAgingScan_performance_restore
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS           rc;

    /* FDB Manager Specific Configuration restore */

    /* Restore appDemo to default: use low level learning, disable aging */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);

    CLEANUP_MANAGER(fdbManagerId);
}

/**
* @internal prvTgfBrgFdbManagerDeleteScan_config function
* @endinternal
*
* @brief  configures the Delete scan related parameters to verify
*         delete scan logic
*/
GT_VOID prvTgfBrgFdbManagerDeleteScan_config
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS                                   rc;
    GT_U32                                      portIter = 0;
    GT_TRUNK_ID                                 trunkId;
    CPSS_TRUNK_MEMBER_STC                       enabledMembersArray[PRV_TGF_EMEMBERS_NUM_CNS];

    /* calculate trunk id */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    FDB_SIZE_FULL_INIT();
    /* fill enabled members parameters array */
    for (portIter = 0; portIter < PRV_TGF_EMEMBERS_NUM_CNS; portIter++)
    {
        enabledMembersArray[portIter].hwDevice = prvTgfDevNum;
        enabledMembersArray[portIter].port = prvTgfPortsArray[prvTgfTrunkMembersIdx[portIter]];
    }

    /* AUTODOC: create trunk 2 with ports [0,3] */
    rc = prvTgfTrunkMembersSet(trunkId,
            PRV_TGF_EMEMBERS_NUM_CNS,
            enabledMembersArray, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfTrunkMembersSet: %d", prvTgfDevNum);

    /* add ports to vlan 2 */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
            prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* create single valid manager */
    rc = createFdbManagerWithDevice(fdbManagerId, PRV_TGF_MAX_TOTAL_ENTRIES, NULL, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "createFdbManager: expected to GT_OK on manager [%d]",
            fdbManagerId);

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
}

/**
* @internal prvTgfBrgFdbManagerDeleteScan_traffic_and_verify function
* @endinternal
*
* @brief  send traffic and verify the statistics
*/

GT_VOID prvTgfBrgFdbManagerDeleteScan_traffic_and_verify
(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS                                               rc;
    GT_U8                                                   portIter;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC        entriesDeleteArray[NUM_ENTRIES_STEPS_CNS];
    GT_U32                                                  entriesDeleteNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC        scanParams;
    GT_U32                                                  packetCnt;
    GT_BOOL                                                 entryAdded;
    GT_U32                                                  numEntriesAdded = 0;
    GT_U32                                                  i;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC                statistics;
    GT_U32                                                  errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNumItr;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /*** CASE_1: Delete scan verify - without filter ***/
    for(packetCnt = 0; packetCnt < PRV_TGF_MAX_TOTAL_ENTRIES; packetCnt++)
    {
        entryAdded = GT_FALSE;
        fdbManagerEntryAdd_macAddr(fdbManagerId, packetCnt, &entryAdded);
        if(entryAdded == GT_TRUE)
        {
            numEntriesAdded+=1;
        }
        if((packetCnt % 100) == 0)
        {
            PRV_UTF_LOG2_MAC("numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
        }

        if(numEntriesAdded == 4 * NUM_ENTRIES_STEPS_CNS)
        {
            PRV_UTF_LOG2_MAC("Required entries reached, numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
            break;
        }
    }

    cpssOsMemSet(&statistics, 0, sizeof(statistics));
    cpssOsMemSet(&scanParams, 0, sizeof(scanParams));
    scanParams.deleteMacUcEntries            = GT_TRUE;

    /* Call delete scan - First iteration - for complete DB */
    for(i = 1; i <= 4; i++)
    {
        rc = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, GT_FALSE, &scanParams, entriesDeleteArray, &entriesDeleteNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerDeleteScan", fdbManagerId);

        rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(i, statistics.scanDeleteOk, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanDeleteOkNoMore, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanDeleteErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanDeleteErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(statistics.scanDeleteTotalDeletedEntries, numEntriesAdded, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);
    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    numEntriesAdded = 0;
    prvSpecificFidFilterValue = 3;
    for(packetCnt = 0; packetCnt < PRV_TGF_MAX_TOTAL_ENTRIES; packetCnt++)
    {
        entryAdded = GT_FALSE;
        fdbManagerEntryAdd_macAddr(fdbManagerId, packetCnt, &entryAdded);
        if(entryAdded == GT_TRUE)
        {
            numEntriesAdded+=1;
        }
        if((packetCnt % 100) == 0)
        {
            PRV_UTF_LOG2_MAC("numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
        }

        if(numEntriesAdded == 128)
        {
            prvSpecificFidFilterValue = 7; /* change fid */
        }

        if(numEntriesAdded == 256)
        {
            PRV_UTF_LOG1_MAC("Required entries reached, numEntriesAdded = %d\n", numEntriesAdded);
            break;
        }
    }

    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsClear", prvTgfDevNum);

    scanParams.fid                           = 7;
    scanParams.fidMask                       = 0xFF;
    rc = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, GT_TRUE, &scanParams, entriesDeleteArray, &entriesDeleteNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerDeleteScan", fdbManagerId);

    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(128, statistics.scanDeleteTotalDeletedEntries, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(1, statistics.scanDeleteOk, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanDeleteOkNoMore, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanDeleteErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanDeleteErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    scanParams.fid                           = 3;
    scanParams.fidMask                       = 0xFF;
    rc = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, GT_TRUE, &scanParams, entriesDeleteArray, &entriesDeleteNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "ERROR: cpssDxChBrgFdbManagerDeleteScan", fdbManagerId);

    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(256, statistics.scanDeleteTotalDeletedEntries, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(1, statistics.scanDeleteOk, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(1, statistics.scanDeleteOkNoMore, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanDeleteErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanDeleteErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    /*** CASE_2: Delete scan verify - for trunk interface ***/
    prvTgfPacketL2Part.saMac[3]=1;
    prvTgfPacketL2Part.saMac[4]=1;
    prvTgfPacketL2Part.saMac[5]=1;
    /* setup Packet with single tag */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoSingleTag, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
            prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* send Packet From port 0 (Trunk port) */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfTrunkMembersIdx[0]]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[prvTgfTrunkMembersIdx[0]]);

    /* sleep for 1 sec */
    cpssOsTimerWkAfter(1000);

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    cpssOsMemSet(&scanParams, 0, sizeof(scanParams));
    scanParams.deleteMacUcEntries       = GT_TRUE;
    scanParams.ePortTrunkNum            = PRV_TGF_TRUNK_ID_CNS;
    scanParams.ePortTrunkNumMask        = 0xFF;
    scanParams.isTrunkMask              = GT_TRUE;
    scanParams.isTrunk                  = GT_TRUE;

    rc = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, GT_TRUE, &scanParams, entriesDeleteArray, &entriesDeleteNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "ERROR: cpssDxChBrgFdbManagerDeleteScan", fdbManagerId);

    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(1, statistics.scanDeleteOk, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(257, statistics.scanDeleteTotalDeletedEntries, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(2, statistics.scanDeleteOkNoMore, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanDeleteErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanDeleteErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    /* Restore back the filter */
    prvSpecificFidFilterValue = 1;

    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }
}

/**
* @internal prvTgfBrgFdbManagerDeleteScan_restore function
* @endinternal
*
* @brief  restore default for aging scan
*/

GT_VOID prvTgfBrgFdbManagerDeleteScan_restore
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS           rc;
    /* General Configuration restore */

    /* restore trunk 2 */
    rc = prvTgfTrunkMembersSet(
            PRV_TGF_TRUNK_ID_CNS /*trunkId*/,
            0 /*numOfEnabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL /*enabledMembersArray*/,
            0 /*numOfDisabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* Restore appDemo to default: use low level learning, disable aging */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);

    CLEANUP_MANAGER(fdbManagerId);
}

/**
* @internal prvTgfBrgFdbManagerTransplantScan_config function
* @endinternal
*
* @brief  configures the Transplant scan related parameters to verify
*         transplant scan logic
*/
GT_VOID prvTgfBrgFdbManagerTransplantScan_config
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS                                   rc;
    GT_U32                                      portIter = 0;
    GT_TRUNK_ID                                 trunkId;
    CPSS_TRUNK_MEMBER_STC                       enabledMembersArray[PRV_TGF_EMEMBERS_NUM_CNS];

    /* calculate trunk id */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    FDB_SIZE_FULL_INIT();
    /* fill enabled members parameters array */
    for (portIter = 0; portIter < PRV_TGF_EMEMBERS_NUM_CNS; portIter++)
    {
        enabledMembersArray[portIter].hwDevice = prvTgfDevNum;
        enabledMembersArray[portIter].port = prvTgfPortsArray[prvTgfTrunkMembersIdx[portIter]];
    }

    /* AUTODOC: create trunk 2 with ports [0,3] */
    rc = prvTgfTrunkMembersSet(trunkId,
            PRV_TGF_EMEMBERS_NUM_CNS,
            enabledMembersArray, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfTrunkMembersSet: %d", prvTgfDevNum);

    /* add ports to vlan 2 */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
            prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* create single valid manager */
    rc = createFdbManagerWithDevice(fdbManagerId, PRV_TGF_MAX_TOTAL_ENTRIES, NULL, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "createFdbManager: expected to GT_OK on manager [%d]",
            fdbManagerId);

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
}

/**
* @internal prvTgfBrgFdbManagerTransplantScan_traffic_and_verify function
* @endinternal
*
* @brief  send traffic and verify the statistics
*/

GT_VOID prvTgfBrgFdbManagerTransplantScan_traffic_and_verify
(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS                                               rc, expRc;
    GT_U8                                                   portIter;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC        entriesTransplantArray[NUM_ENTRIES_STEPS_CNS];
    GT_U32                                                  entriesTransplantNum;
    CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC    scanParams;
    GT_U32                                                  packetCnt;
    GT_BOOL                                                 entryAdded;
    GT_U32                                                  numEntriesAdded = 0;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    GT_U32                                                  i;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC                statistics;
    GT_BOOL                                                 scanStart;
    GT_U32                                                  errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNumItr;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);
    if(fdbManagerPtr == NULL)
    {
        return;
    }

    /*** CASE_1: Transplant scan verify - for trunk interface ***/
    prvTgfPacketL2Part.saMac[3]=1;
    prvTgfPacketL2Part.saMac[4]=1;
    prvTgfPacketL2Part.saMac[5]=1;
    /* setup Packet with single tag */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoSingleTag, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
            prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* send Packet From port 0 (Trunk port) */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfTrunkMembersIdx[0]]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[prvTgfTrunkMembersIdx[0]]);

    /* provide time to learn FDB entries */
    cpssOsTimerWkAfter(10);

    cpssOsMemSet(&scanParams, 0, sizeof(scanParams));
    scanParams.transplantMacUcEntries                       = GT_TRUE;
    scanParams.oldInterface.type                            = CPSS_INTERFACE_TRUNK_E;
    scanParams.oldInterface.interfaceInfo.trunkId           = PRV_TGF_TRUNK_ID_CNS;
    scanParams.newInterface.type                            = CPSS_INTERFACE_PORT_E;
    scanParams.newInterface.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    scanParams.newInterface.interfaceInfo.devPort.portNum   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];

    rc = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, GT_TRUE, &scanParams, entriesTransplantArray, &entriesTransplantNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "ERROR: cpssDxChBrgFdbManagerTransplantScan", prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(1, statistics.scanTransplantTotalTransplantedEntries, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanTransplantOk, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(1, statistics.scanTransplantOkNoMore, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanTransplantErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanTransplantErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    /*** CASE_2: Transplant scan verify - without filter ***/
    for(packetCnt = 0; packetCnt < PRV_TGF_MAX_TOTAL_ENTRIES; packetCnt++)
    {
        entryAdded = GT_FALSE;
        fdbManagerEntryAdd_macAddr(fdbManagerId, packetCnt, &entryAdded);
        if(entryAdded == GT_TRUE)
        {
            numEntriesAdded+=1;
        }
        if((packetCnt % 100) == 0)
        {
            PRV_UTF_LOG2_MAC("numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
        }

        if(numEntriesAdded == _1K)
        {
            PRV_UTF_LOG2_MAC("Required entries reached, numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
            break;
        }
    }

    cpssOsMemSet(&scanParams, 0, sizeof(scanParams));
    scanParams.transplantMacUcEntries                       = GT_TRUE;
    scanParams.newInterface.type                            = CPSS_INTERFACE_PORT_E;
    scanParams.newInterface.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    scanParams.newInterface.interfaceInfo.devPort.portNum   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];

    /* Call Transplant scan - First iteration - for complete DB */
    for(i = 1; i <= (_1K/NUM_ENTRIES_STEPS_CNS)+1; i++)
    {
        expRc = (i == (_1K/NUM_ENTRIES_STEPS_CNS)+1)?GT_NO_MORE:GT_OK;
        rc = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, GT_FALSE, &scanParams, entriesTransplantArray, &entriesTransplantNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(expRc, rc, "ERROR: cpssDxChBrgFdbManagerTransplantScan", prvTgfDevNum);

        rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(4, statistics.scanTransplantOk, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(2, statistics.scanTransplantOkNoMore, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(numEntriesAdded + 1, statistics.scanTransplantTotalTransplantedEntries, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanTransplantErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanTransplantErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    numEntriesAdded = 0;
    prvSpecificFidFilterValue = 3;
    for(; packetCnt < PRV_TGF_MAX_TOTAL_ENTRIES; packetCnt++)
    {
        entryAdded = GT_FALSE;
        fdbManagerEntryAdd_macAddr(fdbManagerId, packetCnt, &entryAdded);
        if(entryAdded == GT_TRUE)
        {
            numEntriesAdded+=1;
        }
        if((packetCnt % 100) == 0)
        {
            PRV_UTF_LOG2_MAC("numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
        }

        if(numEntriesAdded == 128)
        {
            prvSpecificFidFilterValue = 7; /* change fid */
        }

        if(numEntriesAdded == 256)
        {
            PRV_UTF_LOG1_MAC("Required entries reached, numEntriesAdded = %d\n", numEntriesAdded);
            break;
        }
    }

    scanStart = GT_TRUE;
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsClear", prvTgfDevNum);

    for(i = 1; i <= (_1K/NUM_ENTRIES_STEPS_CNS)+2; i++)
    {
        expRc = (i == ((_1K/NUM_ENTRIES_STEPS_CNS)+2))?GT_NO_MORE:GT_OK;
        scanParams.fid                           = 3;
        scanParams.fidMask                       = 0xFF;
        rc = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart, &scanParams, entriesTransplantArray, &entriesTransplantNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(expRc, rc, "ERROR: cpssDxChBrgFdbManagerTransplantScan", prvTgfDevNum);
        scanStart = GT_FALSE;
    }

    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(128, statistics.scanTransplantTotalTransplantedEntries, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(5, statistics.scanTransplantOk, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(1, statistics.scanTransplantOkNoMore, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanTransplantErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanTransplantErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    scanStart = GT_TRUE;
    for(i = 1; i <= (_1K/NUM_ENTRIES_STEPS_CNS)+2; i++)
    {
        expRc = (i == ((_1K/NUM_ENTRIES_STEPS_CNS)+2))?GT_NO_MORE:GT_OK;
        scanParams.fid                           = 7;
        scanParams.fidMask                       = 0xFF;
        rc = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart, &scanParams, entriesTransplantArray, &entriesTransplantNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(expRc, rc, "ERROR: cpssDxChBrgFdbManagerTransplantScan", prvTgfDevNum);
        scanStart = GT_FALSE;
    }

    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(256, statistics.scanTransplantTotalTransplantedEntries, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(10, statistics.scanTransplantOk, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(2, statistics.scanTransplantOkNoMore, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanTransplantErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.scanTransplantErrorInputInvalid, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    /* Restore back the filter */
    prvSpecificFidFilterValue = 1;

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);
    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }
}

/**
* @internal prvTgfBrgFdbManagerTransplantScan_restore function
* @endinternal
*
* @brief  restore default for aging scan
*/

GT_VOID prvTgfBrgFdbManagerTransplantScan_restore
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS           rc;
    /* General Configuration restore */

    /* restore trunk 2 */
    rc = prvTgfTrunkMembersSet(
            PRV_TGF_TRUNK_ID_CNS /*trunkId*/,
            0 /*numOfEnabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL /*enabledMembersArray*/,
            0 /*numOfDisabledMembers*/,
            (CPSS_TRUNK_MEMBER_STC*)NULL  /*disabledMembersArray*/);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* Restore appDemo to default: use low level learning, disable aging */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);

    CLEANUP_MANAGER(fdbManagerId);
}
/**
* @internal prvTgfBrgFdbManagerAddEntry_config function
* @endinternal
*
*/
GT_VOID prvTgfBrgFdbManagerAddEntry_config
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS                                               rc;
    GT_U8                                                   portIter;

    FDB_SIZE_FULL_INIT();
    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: restore PVID to 1 for untagged packets   */
        /* somewhere there is a test that didn't restored it */
        rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[portIter], 1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                     prvTgfPortsArray[portIter], 1);
    }

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    rc = createFdbManagerWithDevice(fdbManagerId, PRV_TGF_MAX_TOTAL_ENTRIES, NULL, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "createFdbManager: expected to GT_OK on manager [%d]",
            fdbManagerId);

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
}

/**
* @internal prvTgfBrgFdbManagerAddEntry_traffic_and_verify function
* @endinternal
*
*/
GT_VOID prvTgfBrgFdbManagerAddEntry_traffic_and_verify
(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS                                               rc;
    GT_U32                                                  errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNumItr;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     entry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC          fdbEntryAddInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC     *fdbEntryMacAddrFormatPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC      *fdbEntryIpv4UcFormatPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC      *fdbEntryIpv6UcFormatPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC                  counters;
    PRV_TGF_MAC_ENTRY_KEY_STC                               macEntryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC                               macEntry;

    /*** Add MAC entry and test entry counters ***/
    cpssOsMemSet(&entry ,0,sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));
    entry.fdbEntryType                           = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
    fdbEntryMacAddrFormatPtr = &entry.format.fdbEntryMacAddrFormat;
    cpssOsMemCpy(fdbEntryMacAddrFormatPtr->macAddr.arEther, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    fdbEntryMacAddrFormatPtr->fid                = 1;
    fdbEntryMacAddrFormatPtr->vid1               = 0;
    fdbEntryMacAddrFormatPtr->daSecurityLevel    = 0;
    fdbEntryMacAddrFormatPtr->saSecurityLevel    = (SHADOW_TYPE_PER_DEVICE() ==CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E)?0:1;
    fdbEntryMacAddrFormatPtr->userDefined        = 0;
    fdbEntryMacAddrFormatPtr->sourceID           = 0;
    fdbEntryMacAddrFormatPtr->daCommand          = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    fdbEntryMacAddrFormatPtr->saCommand          = CPSS_PACKET_CMD_DROP_SOFT_E;
    fdbEntryMacAddrFormatPtr->isStatic           = GT_FALSE;
    fdbEntryMacAddrFormatPtr->age                = GT_TRUE;
    fdbEntryMacAddrFormatPtr->daRoute            = GT_TRUE;
    fdbEntryMacAddrFormatPtr->appSpecificCpuCode = GT_TRUE;
    fdbEntryMacAddrFormatPtr->dstInterface.type  = CPSS_INTERFACE_PORT_E;
    fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.devPort.portNum   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];

    cpssOsMemSet(&fdbEntryAddInfo,0,sizeof(fdbEntryAddInfo));
    fdbEntryAddInfo.rehashEnable    = GT_FALSE;
    fdbEntryAddInfo.tempEntryExist  = GT_FALSE;

    rc = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId, &entry, &fdbEntryAddInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbManagerEntryAdd: unexpected error in manager [%d]", fdbManagerId);

    /* Verify the FDB entry Added successfully */
    cpssOsMemSet(&macEntryKey,0,sizeof(macEntryKey));
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = 1;
    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, &prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(macEntry.spUnknown, GT_FALSE,
                        "MAC Entry spUnknown is not as expected");

    /* Verify Counter - After MAC entry addition */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    counters.macUnicastDynamic      += 1;
    counters.usedEntries            += 1;
    counters.freeEntries            = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntries;
    prvCpssDxChBrgFdbManagerCounterVerify(fdbManagerId, &counters);

    /* Add IPV4 Entry */
    cpssOsMemSet(&entry ,0,sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));
    entry.fdbEntryType                                          = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E;
    fdbEntryIpv4UcFormatPtr                                     = &entry.format.fdbEntryIpv4UcFormat;
    fdbEntryIpv4UcFormatPtr->age                                = GT_TRUE;
    fdbEntryIpv4UcFormatPtr->vrfId                              = 1;
    fdbEntryIpv4UcFormatPtr->ucRouteType                        = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
    fdbEntryIpv4UcFormatPtr->ucRouteInfo.nextHopPointerToRouter = 1;
    cpssOsMemCpy(fdbEntryIpv4UcFormatPtr->ipv4Addr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(fdbEntryIpv4UcFormatPtr->ipv4Addr.arIP));

    rc = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId, &entry, &fdbEntryAddInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(IP_UNICAST_SUPPORT_AS_PER_DEVICE(), rc,
            "cpssDxChBrgFdbManagerEntryAdd: unexpected error in manager [%d]", fdbManagerId);

    /* Verify Counter */
    if((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        counters.ipv4UnicastRoute       += 1;
        counters.usedEntries            += 1;
        counters.freeEntries            = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntries;
    }
    prvCpssDxChBrgFdbManagerCounterVerify(fdbManagerId, &counters);

    /* Add IPV6 Entry */
    cpssOsMemSet(&entry ,0,sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));
    entry.fdbEntryType                                          = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E;
    fdbEntryIpv6UcFormatPtr                                     = &entry.format.fdbEntryIpv6UcFormat;
    fdbEntryIpv6UcFormatPtr->age                                = GT_TRUE;
    fdbEntryIpv6UcFormatPtr->ipv6Addr.arIP[0]                   = 0x22;
    fdbEntryIpv6UcFormatPtr->ipv6Addr.arIP[1]                   = 0x22;
    fdbEntryIpv6UcFormatPtr->ipv6Addr.arIP[14]                  = 0x44;
    fdbEntryIpv6UcFormatPtr->ipv6Addr.arIP[15]                  = 0x44;
    fdbEntryIpv6UcFormatPtr->vrfId                              = 1;

    fdbEntryIpv6UcFormatPtr->ucRouteType                        = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
    fdbEntryIpv6UcFormatPtr->ucRouteInfo.nextHopPointerToRouter = 1;

    rc = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId, &entry, &fdbEntryAddInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(IP_UNICAST_SUPPORT_AS_PER_DEVICE(), rc,
            "cpssDxChBrgFdbManagerEntryAdd: unexpected error in manager [%d]", fdbManagerId);

    /* Verify Counter */
    if((PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)))
    {
        counters.ipv6UnicastRouteKey    += 1;
        counters.ipv6UnicastRouteData   += 1;
        counters.usedEntries            += 2; /* IPv6 Entry */
        counters.freeEntries = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntries;
    }
    prvCpssDxChBrgFdbManagerCounterVerify(fdbManagerId, &counters);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
            prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* send Packet From port 1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

    /* sleep for 1 sec */
    cpssOsTimerWkAfter(1000);

    /* Verify counters - Known traffic should not be impacted */
    prvCpssDxChBrgFdbManagerCounterVerify(fdbManagerId, &counters);

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);
    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }
}

/**
* @internal prvTgfBrgFdbManagerAddEntry_restore function
* @endinternal
*
* @brief  restore default
*/

GT_VOID prvTgfBrgFdbManagerAddEntry_restore
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS           rc;

    /* General Configuration restore */

    /* Restore appDemo to default: use low level learning, disable aging */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);

    CLEANUP_MANAGER(fdbManagerId);
}

/**
* @internal prvTgfBrgFdbManagerAddEntry_cuckoo_config function
* @endinternal
*
* @brief  configures the learning scan related parameters to verify
*         learning scan with cuckoo algorithm
*/
GT_VOID prvTgfBrgFdbManagerAddEntry_cuckoo_config
(
    IN GT_U32       fdbManagerId
)
{
    CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC      capacity;
    GT_STATUS                                   rc;

    FDB_SIZE_FULL_INIT();

    prvSaveFdbSize = fdbSize;
    if(!PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* limit test to use upto 128K FDB size to reduce duration of test.*/
        fdbSize = (fdbSize > _128K) ? _128K : fdbSize;
    }

    /* add ports to vlan 2 */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
            prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* Customize HW capacity */
    cpssOsMemSet(&capacity, 0, sizeof(capacity));
    capacity.hwCapacity.numOfHwIndexes   = PRV_TGF_TOTAL_HW_CAPACITY;
    capacity.hwCapacity.numOfHashes      = 16;
    capacity.maxEntriesPerLearningScan   = PRV_TGF_MAX_ENTRIES_PER_LEARNING_SCAN;
    capacity.maxEntriesPerAgingScan      = NUM_ENTRIES_STEPS_CNS; /*256*/

    /* create single valid manager */
    rc = createFdbManagerWithDevice(fdbManagerId, PRV_TGF_MAX_TOTAL_ENTRIES, &capacity, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "createFdbManager: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Set FDB manager ID for appDemo */
    rc = prvWrAppDemoFdbManagerGlobalIDSet(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerGlobalIDSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Set FDB manager AU Message handler methord as FDB_MANAGER for appDemo */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_DISABLED_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);
}

GT_VOID prvTgfBrgFdbManagerAddEntry_cuckoo_restore
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS           rc;

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* FDB Manager Specific Configuration restore */

    /* Set FDB manager AU Message handler methord as FDB_MANAGER for appDemo */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    CLEANUP_MANAGER(fdbManagerId);

    /* restore FDB size */
    if(!PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        fdbSize =
        PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.fdb = prvSaveFdbSize;
    }
}

/**
* @internal prvTgfBrgFdbManagerAddEntry_cuckoo_traffic function
* @endinternal
*
* @brief  send traffic and verify the statistics
*/

GT_VOID prvTgfBrgFdbManagerAddEntry_cuckoo_traffic
(
    IN GT_U32   fdbManagerId
)
{
    GT_STATUS                                               rc;
    GT_U8                                                   portIter;
    GT_U32                                                  packetCnt;
    GT_U32                                                  numEntriesAdded = 0;
    GT_BOOL                                                 entryAdded;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC       throttlingData;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    GT_U32                                                  noSpaceCounter;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC                statistics;
    GT_U32                                                  firstMissStatus = 0;
    GT_BOOL                                                 prevState;

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    rc = cpssDxChBrgFdbManagerLearningThrottlingGet(fdbManagerId, &throttlingData);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);

    prvTgfPacketL2Part.saMac[4]=1;
    prvTgfPacketL2Part.saMac[5]=1;
    prvTgfPacketVlanTag0Part.vid = 2;

    /* Fill the table till PRV_TGF_MAX_TOTAL_ENTRIES entries */
    for(packetCnt = 0; packetCnt < PRV_TGF_MAX_TOTAL_ENTRIES; packetCnt++)
    {
        entryAdded = GT_FALSE;
        fdbManagerEntryAdd_macAddr(fdbManagerId, packetCnt, &entryAdded);
        if(entryAdded == GT_TRUE)
        {
            numEntriesAdded+=1;
        }
        if((packetCnt % 10000) == 0)
        {
            PRV_UTF_LOG2_MAC("numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
        }
        if(numEntriesAdded == fdbManagerPtr->thresholdB)
        {
            PRV_UTF_LOG2_MAC("ThresholdB reached, numEntriesAdded = %d, packetCnt = %d\n", numEntriesAdded, packetCnt);
            break;
        }
    }

    /* sleep for 1 sec */
    cpssOsTimerWkAfter(1000);

    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    /* Verify that Stage0 is equal to numEntriesAdded */
    UTF_VERIFY_EQUAL1_STRING_MAC(numEntriesAdded, statistics.entryAddOkRehashingStage0,
                                 "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    /* Verify that Stage1 is not incremented before using cuckoo */
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.entryAddOkRehashingStage1,
                                 "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    /* Enable the handler before sending packets */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK");

    prevState = prvTgfTrafficPrintPacketTxEnableSet(GT_FALSE);

    /* we need verify NO_SPACE before reaching threshold C, as at thresholdC - NO_SPACE update will be disabled
     * in case of hash 16 and 128K hw entries - (thresholdC-thresholdB) = 19661 */
    for(packetCnt = 0; packetCnt < (throttlingData.thresholdC - throttlingData.thresholdB); packetCnt++)
    {
        /* setup Packet with single tag */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoSingleTag, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                prvTgfDevNum, prvTgfBurstCount, 0, NULL);

        /* disable printing of packets */
        tgfTrafficTracePacketByteSet(GT_FALSE);

        /* send Packet From port 2 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

        /* If No space cache filled completely, switch to mode with cuckoo */
        if(fdbManagerPtr->noSpaceCacheEntry.usedEntriesCnt == 4)
        {
            if((packetCnt % 10) == 0)
            {
                if (firstMissStatus == 0)
                {
                    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_WITH_CUCKOO_E, GT_FALSE, 0);
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK");
                }
                if (++firstMissStatus > 100)
                {
                    break;
                }
            }
        }

        /* Change the src MAC */
        prvTgfPacketL2Part.saMac[5]+=1;
        if(prvTgfPacketL2Part.saMac[5] == 1)    /* 1 Byte overflowed */
        {
            prvTgfPacketL2Part.saMac[4]+=1;
        }
    }

    PRV_UTF_LOG2_MAC("firstMissStatus reached %d, packetCnt = %d\n", firstMissStatus, packetCnt);

    tgfTrafficTracePacketByteSet(GT_TRUE);
    (void)prvTgfTrafficPrintPacketTxEnableSet(prevState);

    /* sleep for 1 sec */
    cpssOsTimerWkAfter(1000);

    if (learnLimits.enableLearningLimits == GT_FALSE)
    {
        /* Verify no space counter > 0 */
        noSpaceCounter = fdbManagerPtr->apiLearningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NO_SPACE_MSG_E];
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, noSpaceCounter, "No space counter is not as expected");
    }

    rc = cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    if(learnLimits.enableLearningLimits == GT_FALSE)
    {
        /* Verify that Stage1 is not zero after using cuckoo */
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(0, statistics.entryAddOkRehashingStage1,
                                     "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    }
    /* Verify that all entries where added */
    UTF_VERIFY_EQUAL1_STRING_MAC(0, statistics.entryAddErrorInputInvalid,
                                  "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
}

/* Global cusomized params for Hashing test case 1 */
static GT_U32  hash_test1_max_capacity          = _16K;
static GT_U32  hash_test1_stream_entries        = 0x800 /* 0x0 - 0x7FF */;
static GT_U32  hash_test1_Number_of_hash        = 16;
static GT_U32  hash_test1_fdbManagerId          = 10;
static CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC counters;

static GT_VOID prvTgfBrgFdbManagerHashTest1_restore(GT_VOID)
{
    GT_STATUS               rc;
    GT_U16                  vlanId;

    /* Restore appDemo to default: use low level learning, disable aging */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            hash_test1_fdbManagerId);

    /* Invalidate VLANs 100-107 - */
    for(vlanId = 100; vlanId<=107; vlanId++)
    {
        /* invalidate vlan entry (and reset vlan entry) */
        rc = prvTgfBrgDefVlanEntryInvalidate(vlanId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                prvTgfDevNum, vlanId);
    }

    /* Restore */
    prvSpecificFidFilterValue    = 1;

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(hash_test1_fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            hash_test1_fdbManagerId);

    CLEANUP_MANAGER(hash_test1_fdbManagerId);
}

static GT_VOID prvTgfBrgFdbManagerHashTest1_stream_verify(GT_VOID)
{
    GT_U32                                              entiesCnt, numEntriesAdded;
    GT_STATUS                                           rc;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC            statistics;
    GT_U32                                              errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                              errorNumItr;
    GT_BOOL                                             entryAdded;
    GT_U16                                              vlanId;

    /* Learn address with MAC - 0x000000 - 0x0007FF */
    prvTgfFdbManagerMacBuildMode = PRV_TGF_FDB_MANAGER_MAC_INCREMENTAL_MODE_E;
    /***** Stream set 1 : 2048 consecutive static entries (vid 100)                *****/
    /***** Stream set 2 : 2048 consecutive static entries(Same MAC as 3) (vid 101) *****/
    /***** Stream set 3 : 2048 consecutive static entries(Same MAC as 3) (vid 102) *****/
    /***** Stream set 4 : 2048 consecutive static entries(Same MAC as 3) (vid 103) *****/
    for(vlanId = 100; vlanId<=103; vlanId++)
    {
        /* vlan ID - individual Learning */
        prvSpecificFidFilterValue = vlanId;
        for(numEntriesAdded = 0, entiesCnt = 0; entiesCnt < hash_test1_stream_entries; entiesCnt++)
        {
            entryAdded = GT_FALSE;
            fdbManagerEntryAdd_macAddr(hash_test1_fdbManagerId, entiesCnt, &entryAdded);
            if(entryAdded == GT_TRUE)
            {
                numEntriesAdded+=1;
            }
        }
        PRV_UTF_LOG4_MAC("Stream Case Vlan %d: ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
                vlanId, hash_test1_stream_entries, numEntriesAdded, (100*numEntriesAdded)/hash_test1_stream_entries);

        /* In case of SIP6 all the entries should have learnt
         * In case of AC5, counters should match with the numbers of entries learnt */
        numEntriesAdded = (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))?hash_test1_stream_entries:numEntriesAdded;

        /* Verify Counter - After MAC entry addition */
        counters.macUnicastDynamic      += numEntriesAdded;
        counters.usedEntries            += numEntriesAdded;
        counters.freeEntries            -= numEntriesAdded;
        prvCpssDxChBrgFdbManagerCounterVerify(hash_test1_fdbManagerId, &counters);
    }

    prvTgfFdbManagerMacBuildMode = PRV_TGF_FDB_MANAGER_MAC_RANDOM_MODE_E;

    /* Verify Statistics counter */
    rc = cpssDxChBrgFdbManagerStatisticsGet(hash_test1_fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
            statistics.scanLearningErrorInputInvalid,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
            statistics.scanLearningErrorFailedTableUpdate,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
            statistics.scanLearningTotalHwEntryMovedMessages,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
            statistics.scanLearningTotalHwEntryNoSpaceMessages,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
            statistics.scanLearningTotalHwEntryNoSpaceFilteredMessages,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);
    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(hash_test1_fdbManagerId,
            &dbCheckParam,
            testResultArray,
            &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }
}

/**
* @internal prvTgfBrgFdbManagerHashTest1_non_stream_verify function
* @endinternal
*
* @brief  Test non-stream scenario for Hashing test 1
*/
static GT_VOID prvTgfBrgFdbManagerHashTest1_non_stream_verify(GT_VOID)
{
    GT_STATUS                                               rc;
    GT_U32                                                  entiesCnt;
    GT_U32                                                  numEntriesAdded = 0;
    GT_BOOL                                                 entryAdded;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC                statistics;
    GT_U32                                                  entriesToAdd;
    GT_U32                                                  errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNumItr;

    /***** SET 1 : 128  Random static  entries(vid 104-107)                 *****/
    /***** SET 2 : 1024 Random dynamic entries(vid 104-107)                 *****/
    entriesToAdd = 128;
    prvIsStatic  = GT_TRUE;
    prvSpecificFidFilterValue = PRV_TGF_FDB_MANAGER_SHARED_FID_CNS;
    for(entiesCnt = 0; entiesCnt < entriesToAdd; entiesCnt++)
    {
        entryAdded = GT_FALSE;
        fdbManagerEntryAdd_macAddr(hash_test1_fdbManagerId, entiesCnt, &entryAdded);
        if(entryAdded == GT_TRUE)
        {
            numEntriesAdded+=1;
        }
    }
    prvIsStatic = GT_FALSE;
    PRV_UTF_LOG3_MAC("SET 1 :ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
            entriesToAdd, numEntriesAdded, (100*numEntriesAdded)/entriesToAdd);

    /* Verify Counter - After MAC entry addition */
    counters.macUnicastStatic       += entriesToAdd;
    counters.usedEntries            += entriesToAdd;
    counters.freeEntries            -= entriesToAdd;
    prvCpssDxChBrgFdbManagerCounterVerify(hash_test1_fdbManagerId, &counters);

    /***** SET 2 : 1024 Random dynamic entries(vid 104-107) *****/
    entriesToAdd = 1024;
    prvSpecificFidFilterValue = PRV_TGF_FDB_MANAGER_SHARED_FID_CNS;
    for(numEntriesAdded = 0, entiesCnt = 0; entiesCnt < entriesToAdd; entiesCnt++)
    {
        entryAdded = GT_FALSE;
        fdbManagerEntryAdd_macAddr(hash_test1_fdbManagerId, entiesCnt, &entryAdded);
        if(entryAdded == GT_TRUE)
        {
            numEntriesAdded+=1;
        }
    }
    PRV_UTF_LOG3_MAC("SET 2 :ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
            entriesToAdd, numEntriesAdded, (100*numEntriesAdded)/entriesToAdd);

    /* Verify Counter - After MAC entry addition */
    counters.macUnicastDynamic      += entriesToAdd;
    counters.usedEntries            += entriesToAdd;
    counters.freeEntries            -= entriesToAdd;
    prvCpssDxChBrgFdbManagerCounterVerify(hash_test1_fdbManagerId, &counters);

    /* Verify Statistics counter */
    rc = cpssDxChBrgFdbManagerStatisticsGet(hash_test1_fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
            statistics.scanLearningErrorInputInvalid,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
            statistics.scanLearningErrorFailedTableUpdate,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
            statistics.scanLearningTotalHwEntryMovedMessages,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
            statistics.scanLearningTotalHwEntryNoSpaceMessages,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
            statistics.scanLearningTotalHwEntryNoSpaceFilteredMessages,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);
    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(hash_test1_fdbManagerId,
            &dbCheckParam,
            testResultArray,
            &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }
}

/**
* @internal prvTgfBrgFdbManagerHashTest1_first_miss_verify function
* @endinternal
*
* @brief  Test first miss scenario for Hashing test 1
*/
static GT_VOID prvTgfBrgFdbManagerHashTest1_first_miss_verify(GT_VOID)
{
    GT_U32                                              currentAddedEntries = 0, numEntriesAdded;
    GT_STATUS                                           rc;
    GT_U32                                              errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC            statistics;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                              errorNumItr;
    GT_BOOL                                             entryAdded;

    /* Random Start */
    prvTgfFdbManagerMacBuildMode = PRV_TGF_FDB_MANAGER_MAC_INCREMENTAL_MODE_E;
    startAddr[4] = 0x44;
    startAddr[5] = 0x44;
    for(numEntriesAdded = counters.usedEntries; numEntriesAdded < hash_test1_max_capacity; numEntriesAdded++)
    {
        /* Random vlan ID - individual Learning */
        prvSpecificFidFilterValue = 100 + (cpssOsRand() & 3);

        entryAdded = GT_FALSE;
        fdbManagerEntryAdd_macAddr(hash_test1_fdbManagerId, currentAddedEntries, &entryAdded);
        if(entryAdded == GT_FALSE)
        {
            if(prvTgfFdbManagerRehashEnable == GT_TRUE)
            {
                /* First Miss with Cuckoo */
                PRV_UTF_LOG3_MAC("First miss detected(With Cuckoo): Total Entry [%d], Entries beyond given set (%d), Filling %d%% \n",
                        numEntriesAdded, currentAddedEntries, (100*numEntriesAdded)/hash_test1_max_capacity);
                break;
            }
            else
            {
                /* First Miss */
                PRV_UTF_LOG3_MAC("First miss detected(Without Cuckoo): Total Entry [%d], Entries beyond given set (%d), Filling %d%% \n",
                        numEntriesAdded, currentAddedEntries, (100*numEntriesAdded)/hash_test1_max_capacity);

                /* Set FDB manager AU Message handler methord as FDB_MANAGER for appDemo */
                rc = prvWrAppDemoFdbManagerControlSet(
                        APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_WITH_CUCKOO_E,
                        GT_FALSE, 0);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                        "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
                        hash_test1_fdbManagerId);

                if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
                {
                    prvTgfFdbManagerRehashEnable = GT_TRUE;
                }
                else/* AC5 Does not support cuckkoo */
                {
                    break;
                }
            }
        }
        /* Entry added successfully without cuckoo */
        else
        {
            currentAddedEntries += 1;
        }
    }
    prvTgfFdbManagerRehashEnable = GT_FALSE;
    prvTgfFdbManagerMacBuildMode = PRV_TGF_FDB_MANAGER_MAC_RANDOM_MODE_E;

    /* Verify Counter - After MAC entry addition */
    counters.macUnicastDynamic += currentAddedEntries;
    counters.usedEntries       += currentAddedEntries;
    counters.freeEntries       -= currentAddedEntries;
    prvCpssDxChBrgFdbManagerCounterVerify(hash_test1_fdbManagerId, &counters);

    /* Verify Statistics counter */
    rc = cpssDxChBrgFdbManagerStatisticsGet(hash_test1_fdbManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
            statistics.scanLearningErrorInputInvalid,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
            statistics.scanLearningErrorFailedTableUpdate,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
            statistics.scanLearningTotalHwEntryMovedMessages,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
            statistics.scanLearningTotalHwEntryNoSpaceMessages,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
            statistics.scanLearningTotalHwEntryNoSpaceFilteredMessages,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* Verify that Stage1 is not zero after using cuckoo */
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(0,
                statistics.entryAddOkRehashingStage1,
                "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);
    }

    /* First table full for without cuckoo.
     * second table full for with cuckoo */
    UTF_VERIFY_NOT_EQUAL1_STRING_MAC(0,
            statistics.entryAddErrorTableFull,
            "ERROR: cpssDxChBrgFdbManagerStatisticsGet", prvTgfDevNum);

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);
    /* Verify DB */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(hash_test1_fdbManagerId,
            &dbCheckParam,
            testResultArray,
            &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }
}

/**
* @internal prvTgfBrgFdbManagerHashTest1_test function
* @endinternal
*
* @brief  Test Hashing scenario.
*/
GT_VOID prvTgfBrgFdbManagerHashTest1_test(GT_VOID)
{
    GT_STATUS                                   rc;
    GT_U16                                      vlanId;
    CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC      capacity;
    CPSS_PORTS_BMP_STC                          portsMembers;
    CPSS_PORTS_BMP_STC                          portsTagging;
    GT_BOOL                                     isValid;
    PRV_TGF_BRG_VLAN_INFO_STC                   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC          portsTaggingCmd;

    FDB_SIZE_FULL_INIT();
    /* create VLANs 100-107 - */
    for(vlanId = 100; vlanId<=107; vlanId++)
    {
        rc = prvTgfBrgDefVlanEntryWithPortsSet(vlanId,
                prvTgfPortsArray, NULL, NULL, 0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgDefVlanEntryWithPortsSet: %d", vlanId);

        /* Update fidValue for VID[104-107] = fid = 2, - shared learning */
        if(vlanId>103)
        {
            prvTgfBrgVlanEntryRead(prvTgfDevNum, vlanId, &portsMembers,
                    &portsTagging, &isValid, &vlanInfo, &portsTaggingCmd);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryRead");

            vlanInfo.fidValue = PRV_TGF_FDB_MANAGER_SHARED_FID_CNS;
            prvTgfBrgVlanEntryWrite(prvTgfDevNum, vlanId, &portsMembers,
                    &portsTagging, &vlanInfo, &portsTaggingCmd);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryRead");
        }
    }

    /* Customize HW capacity */
    cpssOsMemSet(&capacity, 0, sizeof(capacity));
    capacity.hwCapacity.numOfHwIndexes   = hash_test1_max_capacity;
    capacity.hwCapacity.numOfHashes      = HASH_AS_PER_DEVICE(hash_test1_Number_of_hash);
    capacity.maxEntriesPerLearningScan   = PRV_TGF_MAX_ENTRIES_PER_LEARNING_SCAN;
    capacity.maxEntriesPerAgingScan      = NUM_ENTRIES_STEPS_CNS;

    /* create single valid manager */
    rc = createFdbManagerWithDevice(hash_test1_fdbManagerId,
            hash_test1_max_capacity/* SW Limit */,
            &capacity,
            GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "createFdbManager: expected to GT_OK on manager [%d]",
            hash_test1_fdbManagerId);

    /* Set FDB manager ID for appDemo */
    rc = prvWrAppDemoFdbManagerGlobalIDSet(hash_test1_fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerGlobalIDSet: expected to GT_OK on manager [%d]",
            hash_test1_fdbManagerId);

    /* Set FDB manager AU Message handler methord as FDB_MANAGER for appDemo */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            hash_test1_fdbManagerId);

    /* Global OUI */
    cpssOsMemSet(&startAddr, 0, sizeof(startAddr));
    startAddr[0]  = 0x01 & PRV_CPSS_OUI_LSB_MASK;
    startAddr[1]  = 0x0E;
    startAddr[2]  = 0xCF;

    /* Reset Global counters */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    counters.freeEntries = hash_test1_max_capacity;

    /* Cases with non-stream(Learning Via API call) -
     * Total Entries = (2048*4 + 128 + 1024) = 9344 */
    prvTgfBrgFdbManagerHashTest1_non_stream_verify();

    /* Cases with stream - 2048*4=8192 Entries */
    prvTgfBrgFdbManagerHashTest1_stream_verify();

    /* case to verify - first miss */
    prvTgfBrgFdbManagerHashTest1_first_miss_verify();

    prvTgfBrgFdbManagerHashTest1_restore();
}

/*
    function to check if the mac entry is on the proper portIndex
*/
static void checkEntyOnPort(
    IN PRV_TGF_MAC_ENTRY_KEY_STC        *macEntryKeyPtr,
    IN GT_U32                           portIndex
)
{
    GT_STATUS                   rc;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;

    /* make sure the entry on needed port index */
    rc = prvTgfBrgFdbMacEntryGet(macEntryKeyPtr, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

    UTF_VERIFY_EQUAL1_STRING_MAC(macEntry.dstInterface.type, CPSS_INTERFACE_PORT_E,
                        "MAC Entry dstInterface.type is not as expected",
                        macEntry.dstInterface.type);
    UTF_VERIFY_EQUAL2_STRING_MAC(macEntry.dstInterface.devPort.hwDevNum, prvTgfDevNum,
                        "MAC Entry devPort.hwDevNum is not as expected",
                        macEntry.dstInterface.devPort.hwDevNum,prvTgfDevNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(macEntry.dstInterface.devPort.portNum, prvTgfPortsArray[portIndex],
                        "MAC Entry devPort.portNum is not as expected",
                        macEntry.dstInterface.devPort.portNum,prvTgfPortsArray[portIndex]);
}

static GT_VOID prvTgfBrgFdbManagerPortLearning_movedMacSaCmd_traffic_and_verify
(
    IN GT_U32       fdbManagerId,
    IN GT_BOOL      checkDynamicEntry,
    IN GT_U32       iteration
)
{
    typedef struct{
        CPSS_PACKET_CMD_ENT     packetCmd;
        GT_U32                  movedPortIndex;
        GT_U32                  portsGetTraffic[4];
        GT_BOOL                 cpuGetTraffic;
    }MOVED_MAC_SA_CMD_INFO_STC;
    CPSS_PACKET_CMD_ENT     restorePacketCmd = 0;
    GT_U32  ii;
    GT_U32  portIter;
    GT_U32  portsCount = prvTgfPortsNum;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32  actualCapturedNumOfPackets;
    static MOVED_MAC_SA_CMD_INFO_STC   testInfoArr[] = {
         {CPSS_PACKET_CMD_FORWARD_E       , PRV_TGF_SEND_PORT_IDX_1_CNS, {1,1,1,1} , GT_FALSE}
        ,{CPSS_PACKET_CMD_MIRROR_TO_CPU_E , PRV_TGF_SEND_PORT_IDX_1_CNS, {1,1,1,1} , GT_TRUE }
        ,{CPSS_PACKET_CMD_TRAP_TO_CPU_E   , PRV_TGF_SEND_PORT_IDX_2_CNS, {0,1,0,0} , GT_TRUE }
        ,{CPSS_PACKET_CMD_DROP_HARD_E     , PRV_TGF_SEND_PORT_IDX_2_CNS, {0,1,0,0} , GT_FALSE}
        ,{CPSS_PACKET_CMD_MIRROR_TO_CPU_E , PRV_TGF_SEND_PORT_IDX_1_CNS, {1,1,1,1} , GT_TRUE }
        ,{CPSS_PACKET_CMD_DROP_SOFT_E     , PRV_TGF_SEND_PORT_IDX_2_CNS, {0,1,0,0} , GT_FALSE}
        ,{CPSS_PACKET_CMD_TRAP_TO_CPU_E   , PRV_TGF_SEND_PORT_IDX_2_CNS, {0,1,0,0} , GT_TRUE }


        /* must be last */
        ,{CPSS_PACKET_CMD_FORWARD_E        , GT_NA ,{0,0,0,0} , GT_FALSE}
    };
    GT_STATUS                                               rc;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     entry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC          fdbEntryAddInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC     *fdbEntryMacAddrFormatPtr;
    PRV_TGF_MAC_ENTRY_KEY_STC                               macEntryKey;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC       updateParams;

    /* add manually dynamic FDB entry on port '2' , then send this mac from port '1' */
    /* 1. check that the 'movedMacSaCmd' apply to the 'moved' entry */
    /* 2. check that static entry not moved according to 'movedMacSaCmd' but rather according to
        cpssDxChBrgSecurBreachMovedStaticAddrSet - per device enable/disable.
        cpssDxChBrgSecurBreachEventPacketCommandSet , CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E - per device command.
    */
    cpssOsMemSet(&fdbEntryAddInfo,0,sizeof(fdbEntryAddInfo));
    cpssOsMemSet(&entry,0,sizeof(entry));
    entry.fdbEntryType                           = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
    fdbEntryMacAddrFormatPtr = &entry.format.fdbEntryMacAddrFormat;
    cpssOsMemCpy(fdbEntryMacAddrFormatPtr->macAddr.arEther, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    fdbEntryMacAddrFormatPtr->fid                = 1;
    fdbEntryMacAddrFormatPtr->daCommand          = CPSS_PACKET_CMD_FORWARD_E;
    fdbEntryMacAddrFormatPtr->saCommand          = CPSS_PACKET_CMD_FORWARD_E;
    fdbEntryMacAddrFormatPtr->isStatic           = checkDynamicEntry == GT_TRUE ? GT_FALSE : GT_TRUE;
    fdbEntryMacAddrFormatPtr->age                = GT_TRUE;
    fdbEntryMacAddrFormatPtr->dstInterface.type  = CPSS_INTERFACE_PORT_E;
    fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.devPort.portNum   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];

    rc = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId, &entry, &fdbEntryAddInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbManagerEntryAdd: unexpected error in manager [%d]", fdbManagerId);

    /* make sure the entry on port '2' */
    /* Verify the FDB entry Added successfully */
    cpssOsMemSet(&macEntryKey,0,sizeof(macEntryKey));
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = (GT_U16)fdbEntryMacAddrFormatPtr->fid;
    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, &prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

    /* make sure the entry on port '2' */
    checkEntyOnPort(&macEntryKey,PRV_TGF_SEND_PORT_IDX_2_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
            prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* send Packet From port 1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

    /* sleep */
    cpssOsTimerWkAfter(1000);

    /* make sure the entry on port '1' */
    checkEntyOnPort(&macEntryKey,
        checkDynamicEntry == GT_TRUE ?
            PRV_TGF_SEND_PORT_IDX_1_CNS :
            PRV_TGF_SEND_PORT_IDX_2_CNS);

    /* reset counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");


    /* Get the port learning attributes */
    rc = cpssDxChBrgFdbManagerPortLearningGet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
            &restoreLearningParam);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerPortLearningGet: %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);

    /* set port movedMacSaCmd value */
    learningParam = restoreLearningParam;

    cpssOsMemSet(&updateParams,0,sizeof(updateParams));
    updateParams.updateOnlySrcInterface = GT_TRUE;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];


    if(checkDynamicEntry == GT_FALSE && iteration == 1)
    {
        /* AUTODOC: set port 1 enable Security Breach for Moved Static addresses */
        rc = prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
                                                                 GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet");

        rc = cpssDxChBrgSecurBreachEventPacketCommandGet(prvTgfDevNum,
                CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E,
                &restorePacketCmd);
        /* AUTODOC: set 'MOVED_STATIC' packet command to soft drop , so it will be counted */
        rc = prvTgfBrgSecurBreachEventPacketCommandSet(CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E,
            CPSS_PACKET_CMD_DROP_SOFT_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSecurBreachEventPacketCommandSet");
    }

    for(ii = 0 ; testInfoArr[ii].movedPortIndex != GT_NA ; ii++)
    {
        if((!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum)) &&
                testInfoArr[ii].packetCmd != CPSS_PACKET_CMD_FORWARD_E)
        {
            continue;
        }
        /* restore it to port '2' */
        fdbEntryMacAddrFormatPtr->dstInterface.interfaceInfo.devPort.portNum   =
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
        /* updated the entry */
        rc = cpssDxChBrgFdbManagerEntryUpdate(fdbManagerId, &entry , &updateParams);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
               "cpssDxChBrgFdbManagerEntryUpdate: expected to GT_OK");

        learningParam.movedMacSaCmd     = testInfoArr[ii].packetCmd;
        rc = cpssDxChBrgFdbManagerPortLearningSet(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
                &learningParam);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "cpssDxChBrgFdbManagerPortLearningSet: %d",
                prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);

        /* send Packet From port 1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

        /* sleep */
        cpssOsTimerWkAfter(1000);

        /* make sure the entry on port '1' or port '2' */
        checkEntyOnPort(&macEntryKey,
            checkDynamicEntry == GT_TRUE ?
                testInfoArr[ii].movedPortIndex :
                PRV_TGF_SEND_PORT_IDX_2_CNS);

        /* AUTODOC: read and verify counters on all ports */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);

            if(checkDynamicEntry == GT_TRUE)
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount* testInfoArr[ii].portsGetTraffic[portIter],
                    portCntrs.goodPktsSent.l[0],
                    "get another goodPktsSent counter than expected");
            }
            else
            if(checkDynamicEntry == GT_FALSE && iteration == 1 &&
               portIter != PRV_TGF_SEND_PORT_IDX_1_CNS)
            {
                /* the static entry , cause security breach , so we have drops */
                UTF_VERIFY_EQUAL0_STRING_MAC(0/* expect no bridging */,
                    portCntrs.goodPktsSent.l[0],
                    "get another goodPktsSent counter than expected");
            }
            else
            {
                /* the static entry , not cause security breach , so we have flood */
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount,
                    portCntrs.goodPktsSent.l[0],
                    "get another goodPktsSent counter than expected");
            }
        }/*portIter*/

        if(testInfoArr[ii].cpuGetTraffic == GT_TRUE &&
           checkDynamicEntry == GT_TRUE)
        {
            /* check if packets came to the CPU */
            /* print captured packets and check TriggerCounters */
            rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                    &portInterface,
                    &prvTgfPacketInfo,
                    prvTgfBurstCount,/*numOfPackets*/
                    0/*vfdNum*/,
                    NULL /*vfdArray*/,
                    NULL, /* bytesNum's skip list */
                    0,    /* length of skip list */
                    &actualCapturedNumOfPackets,
                    NULL/*onFirstPacketNumTriggersBmpPtr*/);
            rc = rc == GT_NO_MORE ? GT_OK : rc;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                         "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                         " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

            /* clear table */
            rc = tgfTrafficTableRxPcktTblClear();
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
        }

    }/*ii*/

    /* reset counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    if(checkDynamicEntry == GT_FALSE)
    {
        /* check NO packets came to the CPU */
        /* print captured packets and check TriggerCounters */
        rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                &portInterface,
                &prvTgfPacketInfo,
                0,/*numOfPackets*/
                0/*vfdNum*/,
                NULL /*vfdArray*/,
                NULL, /* bytesNum's skip list */
                0,    /* length of skip list */
                &actualCapturedNumOfPackets,
                NULL/*onFirstPacketNumTriggersBmpPtr*/);
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                     " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);
    }

    /* delete the entry*/
    rc = cpssDxChBrgFdbManagerEntryDelete(fdbManagerId, &entry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbManagerEntryDelete: unexpected error in manager [%d]", fdbManagerId);

    /* stop capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* restore movedMacSaCmd */
    rc = cpssDxChBrgFdbManagerPortLearningSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
            &restoreLearningParam);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerPortLearningSet: %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);

    if(checkDynamicEntry == GT_FALSE && iteration == 1)
    {
        /* AUTODOC: restore port 1 disable Security Breach for Moved Static addresses */
        rc = prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS],
                                                                 GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSecurityBreachPortMovedStaticAddrEnableSet");

        /* AUTODOC: restore 'MOVED_STATIC' packet command to forward */
        rc = prvTgfBrgSecurBreachEventPacketCommandSet(CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E,
            restorePacketCmd);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSecurBreachEventPacketCommandSet");
    }
}


GT_VOID prvTgfBrgFdbManagerPortLearning_movedMacSaCmd
(
    IN GT_U32       fdbManagerId
)
{
    prvTgfBrgFdbManagerAddEntry_config(fdbManagerId);
    prvTgfBrgFdbManagerPortLearning_movedMacSaCmd_traffic_and_verify(fdbManagerId,GT_TRUE/*dynamic*/,0);
    prvTgfBrgFdbManagerPortLearning_movedMacSaCmd_traffic_and_verify(fdbManagerId,GT_FALSE/*static*/,0/* no security breach*/);
    prvTgfBrgFdbManagerPortLearning_movedMacSaCmd_traffic_and_verify(fdbManagerId,GT_FALSE/*static*/,1/*    security breach*/);
    prvTgfBrgFdbManagerAddEntry_restore(fdbManagerId);
}

GT_VOID prvTgfBrgFdbManager_clean()
{

    FDB_SIZE_FULL_INIT();
    prvTgfBrgFdbManager_restoreFdbSize();
    prvTgfBrgFdb_cleanup(GT_TRUE);
}

/* Test sequence:
 *      Create FDB manager & Disable learning scan call from appDemo. (Need to call manually)
 *      Allow multicast SA packet to enter
 *      send mixed traffic to AUQ:
 *          2 Valid packet
 *          2 invalid packet
 *          2 valid packet
 *          clean all entries and then send fresh 2 packets.
 *
 *      1st learning scan:
 *          should Get 2 valid entries & 1 invalid entries in OUT array & return GT_BAD_PARAM.
 *      2nd learning scan:
 *          should Get 1 invalid entries in OUT array & return GT_BAD_PARAM.
 *      3rd Learning scan:
 *          should Get 2 valid entries in OUT array & return GT_NO_MORE.
 *      4th Learning scan:
 *          should Get 2 invalid entries in OUT array & return GT_NO_MORE.
 *
 */
GT_VOID prvTgfBrgFdbManagerInvalidAuq_verify_test()
{
    GT_STATUS                                               rc;
    GT_U32                                                  fdbManagerId = 20;
    GT_U8                                                   portIter;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC        entriesLearningArray[NUM_ENTRIES_STEPS_CNS];
    GT_U32                                                  entriesLearningNum;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC      paramsPtr;
    GT_U32                                                  packetCnt;
    GT_U32                                                  errorNum;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNumItr;

    FDB_SIZE_FULL_INIT();
    rc = createFdbManagerWithDevice(fdbManagerId, PRV_TGF_MAX_TOTAL_ENTRIES, NULL, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "createFdbManager: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Set FDB manager ID for appDemo */
    rc = prvWrAppDemoFdbManagerGlobalIDSet(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerGlobalIDSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* Set FDB manager AU Message handler methord as FDB_MANAGER for appDemo */
    rc = prvWrAppDemoFdbManagerControlSet(APP_DEMO_AU_MESSAGE_HANDLER_DISABLED_E, GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvWrAppDemoFdbManagerControlSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    rc = prvTgfBrgSecurBreachEventPacketCommandSet(CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E, CPSS_PACKET_CMD_TRAP_TO_CPU_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgSecurBreachEventPacketCommandSet %d", prvTgfDevNum);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    prvTgfPacketVlanTag0Part.vid = 1;

    /* Send 2 valid packets -> 2 InValid packet -> 2 Valid packet */
    for(packetCnt = 0; packetCnt < 6; packetCnt++)
    {
        /* setup Packet with single tag */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoSingleTag, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                prvTgfDevNum, prvTgfBurstCount, 0, NULL);

        /* Enable the handler just before sending the last packet, to trigger the processing */
        if(packetCnt == 2)
        {
            prvTgfPacketL2Part.saMac[0] = 1;
        }
        else if(packetCnt == 4)
        {
            prvTgfPacketL2Part.saMac[0] = 0;
        }

        /* send Packet From port 1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

        /* Change the src MAC */
        prvTgfPacketL2Part.saMac[5]+=1;
    }

    paramsPtr.addNewMacUcEntries        = GT_TRUE;
    paramsPtr.addWithRehashEnable       = GT_FALSE;
    paramsPtr.updateMovedMacUcEntries   = GT_FALSE;

    /* 1st Learning Scan */
    rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, entriesLearningArray, &entriesLearningNum);

    /* Verify - Out event array & return code */
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, rc, "cpssDxChBrgFdbManagerLearningScan");
    UTF_VERIFY_EQUAL0_STRING_MAC(3, entriesLearningNum, "entriesLearningNum is not as expected:");

    rc = cpssDxChBrgFdbManagerEntryTempDelete(fdbManagerId,
            &entriesLearningArray[entriesLearningNum-1].entry,
            entriesLearningArray[entriesLearningNum-1].updateInfo.newEntryFormat.tempEntryOffset);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbManagerLearningScan");

    /* 2nd Learning Scan */
    rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, entriesLearningArray, &entriesLearningNum);

    /* Verify - Out event array & return code */
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, rc, "cpssDxChBrgFdbManagerLearningScan");
    UTF_VERIFY_EQUAL0_STRING_MAC(1, entriesLearningNum, "entriesLearningNum is not as expected:");

    rc = cpssDxChBrgFdbManagerEntryTempDelete(fdbManagerId,
            &entriesLearningArray[entriesLearningNum-1].entry,
            entriesLearningArray[entriesLearningNum-1].updateInfo.newEntryFormat.tempEntryOffset);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbManagerLearningScan");

    /* 3rd Learning Scan */
    rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, entriesLearningArray, &entriesLearningNum);

    /* Verify - Out event array & return code */
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "cpssDxChBrgFdbManagerLearningScan");
    UTF_VERIFY_EQUAL0_STRING_MAC(2, entriesLearningNum, "entriesLearningNum is not as expected:");

    /* Fresh entries - Verify that lastscan store index should have been reset */
    for(packetCnt = 0; packetCnt < 2; packetCnt++)
    {
        /* setup Packet with single tag */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoSingleTag, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                prvTgfDevNum, prvTgfBurstCount, 0, NULL);

        /* send Packet From port 1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_1_CNS);

        /* Change the src MAC */
        prvTgfPacketL2Part.saMac[5]+=1;
    }

    /* 4th Learning Scan */
    rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &paramsPtr, entriesLearningArray, &entriesLearningNum);

    /* Verify - Out event array & return code */
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "cpssDxChBrgFdbManagerLearningScan");
    UTF_VERIFY_EQUAL0_STRING_MAC(2, entriesLearningNum, "entriesLearningNum is not as expected:");

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);
    /* Verify DB - NO SP entry should found */
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG2_MAC("testResult Error[Itr - %d] -[Err_No - %d]\n",
                errorNumItr,
                testResultArray[errorNumItr]);
    }

    /* Restore */
    rc = prvTgfBrgSecurBreachEventPacketCommandSet(CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E, CPSS_PACKET_CMD_DROP_HARD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgSecurBreachEventPacketCommandSet %d", prvTgfDevNum);

    CLEANUP_MANAGER(fdbManagerId);
}


#define LOG_PARAM(x)    \
    PRV_UTF_LOG2_MAC("[%s]=(%d) \n",#x,x)

#define LOG_LIMIT_PARAM(var,limit)    \
    PRV_UTF_LOG3_MAC("info [%s][%d] hold limit[%d] \n",#var,var,limit)

static GT_U32   totalSent = 0;

/**
* @internal basicLimitDynamicUcLearn_sendTraffic function
* @endinternal
*
* @brief   send common traffic
*/
static GT_VOID basicLimitDynamicUcLearn_sendTraffic(
    IN GT_U32  portNum,
    IN GT_U16  vlanId,
    IN GT_U32  numOfPackets,
    IN GT_U32  startIndex
)
{
    GT_STATUS               rc;
    GT_U8                   portIter;
    TGF_PACKET_L2_STC       orig_prvTgfPacketL2Part;
    TGF_VFD_INFO_STC        vfdArray[1];

    PRV_UTF_LOG4_MAC("send [%d] incremental from port [%d] vlan[%d] startIndex[%d] \n",
        numOfPackets,portNum,vlanId,startIndex);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    cpssOsMemCpy(&orig_prvTgfPacketL2Part,&prvTgfPacketL2Part,sizeof(prvTgfPacketL2Part));
    /* set the start SA */
    prvTgfPacketL2Part.saMac[1] = (GT_U8)(portNum >> 8);
    prvTgfPacketL2Part.saMac[2] = (GT_U8)(portNum >> 0);
    prvTgfPacketVlanTag0Part.vid = vlanId;

    /* set the vlanId */
    prvTgfPacketVlanTag0Part.vid = vlanId;

    cpssOsMemSet(vfdArray,0,sizeof(vfdArray));
    vfdArray[0].mode = TGF_VFD_MODE_INCREMENT_E;
    vfdArray[0].incValue       = 1;
    vfdArray[0].offset         = 10;/* SaMac offset to last 2 bytes     */
    vfdArray[0].cycleCount     = 2;/* only 2 bytes to hold 'startIndex' */
    vfdArray[0].patternPtr[0] = (GT_U8)(startIndex >> 8);
    vfdArray[0].patternPtr[1] = (GT_U8)(startIndex >> 0);

    /* setup Packet with single tag */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoSingleTag, numOfPackets, 1, vfdArray);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
            prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* send Packet From the port */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, portNum);

    /* restore info */
    cpssOsMemCpy(&prvTgfPacketL2Part,&orig_prvTgfPacketL2Part,sizeof(prvTgfPacketL2Part));
    prvTgfPacketVlanTag0Part.vid = PRV_TGF_VLANID_2_CNS;

    totalSent += numOfPackets;
}

/**
* @internal basicLimitDynamicUcLearn_common_traffic function
* @endinternal
*
* @brief   send common traffic of the test
*/
static GT_VOID basicLimitDynamicUcLearn_common_traffic
(
    IN GT_U32   startIndex
)
{
    GT_U32  _p[3];
    GT_U16  _v[2]={PRV_TGF_VLANID_2_CNS,PRV_TGF_VLANID_5_CNS};

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    totalSent = 0;

    /* send 3 sets of 30 from port 0,1 (vlan 2)*/
    basicLimitDynamicUcLearn_sendTraffic(_p[0]/*port*/,_v[0]/*vlanId*/,30/*numSend*/,startIndex + 0/*startIndex*/);
    basicLimitDynamicUcLearn_sendTraffic(_p[1]/*port*/,_v[0]/*vlanId*/,30/*numSend*/,startIndex + 0/*startIndex*/);
    basicLimitDynamicUcLearn_sendTraffic(_p[0]/*port*/,_v[0]/*vlanId*/,30/*numSend*/,startIndex +30/*startIndex*/);
    basicLimitDynamicUcLearn_sendTraffic(_p[1]/*port*/,_v[0]/*vlanId*/,30/*numSend*/,startIndex +30/*startIndex*/);
    basicLimitDynamicUcLearn_sendTraffic(_p[0]/*port*/,_v[0]/*vlanId*/,30/*numSend*/,startIndex +60/*startIndex*/);
    basicLimitDynamicUcLearn_sendTraffic(_p[1]/*port*/,_v[0]/*vlanId*/,30/*numSend*/,startIndex +60/*startIndex*/);

    /* send single set of 100 from port 2 (vlan 5)*/
    basicLimitDynamicUcLearn_sendTraffic(_p[2]/*port*/,_v[1]/*vlanId*/,100/*numSend*/,startIndex +0/*startIndex*/);

    /* sleep for 1.5 sec - to receive the interrupt and process learning scan */
    cpssOsTimerWkAfter(1500);
}

/**
* @internal basicLimitDynamicUcLearn_part1 function
* @endinternal
*
* @brief   part 1 of the test : test with default of 'learnLimitsInfo'
*/
static GT_VOID basicLimitDynamicUcLearn_part1
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS               rc;
    GT_U32  learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E];
    GT_U32  addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E];
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC    counters;
    GT_U32  _p[3];
    GT_U16  _v[2]={PRV_TGF_VLANID_2_CNS,PRV_TGF_VLANID_5_CNS};

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    LOG_PARAM(learnLimits.maxDynamicUcMacGlobalLimit);

    LOG_LIMIT_PARAM(_p[0],learnLimits.maxDynamicUcMacPortLimit);
    LOG_LIMIT_PARAM(_p[1],learnLimits.maxDynamicUcMacPortLimit);
    LOG_LIMIT_PARAM(_p[2],learnLimits.maxDynamicUcMacPortLimit);

    LOG_LIMIT_PARAM(_v[0],learnLimits.maxDynamicUcMacFidLimit);
    LOG_LIMIT_PARAM(_v[1],learnLimits.maxDynamicUcMacFidLimit);

    /* send the traffic */
    basicLimitDynamicUcLearn_common_traffic(0/*start index*/);

    PRV_UTF_LOG0_MAC("Check counters and statistics \n");
    /* Counter should be valid */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    counters.macUnicastDynamic      = learnLimits.maxDynamicUcMacGlobalLimit;
    counters.macUnicastStatic       = 0;
    counters.usedEntries            = counters.macUnicastDynamic;
    counters.freeEntries            = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntries;
    prvCpssDxChBrgFdbManagerCounterVerify(fdbManagerId, &counters);

    LOG_PARAM(counters.usedEntries);

    /* Verify Statistics counter */
    cpssOsMemSet(learningScanStatisticsArr, 0, sizeof(learningScanStatisticsArr));
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]           = totalSent;
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E]   = GT_NA;/* indication that 'OK'+'OK_NO_MORE' checked together */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E] = totalSent;

    cpssOsMemSet(addEntryStatisticsArr, 0, sizeof(addEntryStatisticsArr));
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_PORT_LIMIT_E        ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TRUNK_LIMIT_E       ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_FID_LIMIT_E         ] = 30;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_LIMIT_E      ] = 50;

    prvTgfBrgFdbManagerLearningScan_statistics_verify(fdbManagerId, learningScanStatisticsArr,addEntryStatisticsArr);

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);
}
/**
* @internal basicLimitDynamicUcLearn_part2_config function
* @endinternal
*
* @brief  part 2 of the test : test with global,fid,port different limits of 'learnLimitsInfo'
*/
static GT_VOID basicLimitDynamicUcLearn_part2_config
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS                                   rc;
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   limitInfo;
    GT_U32  limitValue,currentValue;
    GT_U32  _p[3];
    GT_U16  _v[2]={PRV_TGF_VLANID_2_CNS,PRV_TGF_VLANID_5_CNS};
    GT_U32  fidCounter;

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    PRV_UTF_LOG0_MAC("Reduce Global limit from 200 to 180 \n");
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_E;
    limitInfo.limitIndex.fid = 0;/* dummy : limitInfo.limitIndex not used for 'global' */
    limitValue = 180;/* was 200 , and we already hold 200 in DB ! */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG1_MAC("actual value [%d] on global \n",currentValue);
    UTF_VERIFY_EQUAL0_STRING_MAC(learnLimits.maxDynamicUcMacGlobalLimit,currentValue,"actual num in global limit");

    PRV_UTF_LOG1_MAC("Reduce fid[%d] limit from 150 to 99 \n",_v[0]);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[0];
    limitValue = 99;/* was 150 */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG2_MAC("actual value [%d] on fid[%d] \n",currentValue,limitInfo.limitIndex.fid);
    UTF_VERIFY_EQUAL1_STRING_MAC(learnLimits.maxDynamicUcMacFidLimit,currentValue,"actual num in fid[%d] limit",
        limitInfo.limitIndex.fid);

    fidCounter = currentValue;/* the number of entries on first vlan */

    PRV_UTF_LOG1_MAC("Enlarge fid[%d] limit from 150 to 170 \n",_v[1]);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[1];
    limitValue = 170;/* was 150 */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG2_MAC("actual value [%d] on fid[%d] \n",currentValue,limitInfo.limitIndex.fid);
    UTF_VERIFY_EQUAL1_STRING_MAC(learnLimits.maxDynamicUcMacGlobalLimit - fidCounter,currentValue,"actual num in fid[%d] limit",
        limitInfo.limitIndex.fid);


    PRV_UTF_LOG1_MAC("Reduce port[%d] limit from 100 to 80 \n",_p[0]);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[0];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    limitValue = 80;/* was 100 */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG2_MAC("actual value [%d] on port[%d] \n",currentValue,limitInfo.limitIndex.devPort.portNum);
    /* we expect only 90 , the number of sent packets (less than the 100 that was the limit) */
    UTF_VERIFY_EQUAL1_STRING_MAC(90,currentValue,"actual num in port[%d] limit",
        limitInfo.limitIndex.devPort.portNum);

    PRV_UTF_LOG1_MAC("Reduce port[%d] limit from 100 to 55 \n",_p[1]);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[1];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    limitValue = 55;/* was 100 */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG2_MAC("actual value [%d] on port[%d] \n",currentValue,limitInfo.limitIndex.devPort.portNum);
    /* we expect only 60 , as _p[0] got 90 and together where limited to 150 in _v[0] */
    UTF_VERIFY_EQUAL1_STRING_MAC(60,currentValue,"actual num in port[%d] limit",
        limitInfo.limitIndex.devPort.portNum);

    PRV_UTF_LOG1_MAC("Reduce port[%d] limit from 100 to 59 \n",_p[2]);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[2];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    limitValue = 59;/* was 100 */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG2_MAC("actual value [%d] on port[%d] \n",currentValue,limitInfo.limitIndex.devPort.portNum);
    /* we expect only 50 , as _v[1] hold till limit of 200 with the 150 in _v[0] */
    UTF_VERIFY_EQUAL1_STRING_MAC(50,currentValue,"actual num in port[%d] limit",
        limitInfo.limitIndex.devPort.portNum);
}

/**
* @internal basicLimitDynamicUcLearn_part2 function
* @endinternal
*
* @brief  part 2 of the test : test with global,fid,port different limits of 'learnLimitsInfo'
*/
static GT_VOID basicLimitDynamicUcLearn_part2
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS               rc;
    GT_U32  learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E];
    GT_U32  addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E];
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC    counters;

    basicLimitDynamicUcLearn_part2_config(fdbManagerId);

    /* send the traffic */
    basicLimitDynamicUcLearn_common_traffic(100/*start index*/);

    PRV_UTF_LOG0_MAC("Check counters and statistics \n");

    /* Counter should be valid */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    /* the old value '200' not the 150 set by basicLimitDynamicUcLearn_part2_config() */
    counters.macUnicastDynamic      = learnLimits.maxDynamicUcMacGlobalLimit;
    counters.macUnicastStatic       = 0;
    counters.usedEntries            = counters.macUnicastDynamic;
    counters.freeEntries            = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntries;
    prvCpssDxChBrgFdbManagerCounterVerify(fdbManagerId, &counters);

    LOG_PARAM(counters.usedEntries);

    /* Verify Statistics counter */
    cpssOsMemSet(learningScanStatisticsArr, 0, sizeof(learningScanStatisticsArr));
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]           = totalSent;
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E]   = GT_NA;/* indication that 'OK'+'OK_NO_MORE' checked together */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E] = totalSent;

    cpssOsMemSet(addEntryStatisticsArr, 0, sizeof(addEntryStatisticsArr));
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_PORT_LIMIT_E        ] = 180;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TRUNK_LIMIT_E       ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_FID_LIMIT_E         ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_LIMIT_E      ] = 100;

    prvTgfBrgFdbManagerLearningScan_statistics_verify(fdbManagerId, learningScanStatisticsArr,addEntryStatisticsArr);

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);

}

/**
* @internal basicLimitDynamicUcLearn_part3_checkLimits function
* @endinternal
*
* @brief  part 3 of the test : check limit values
*/
static GT_VOID basicLimitDynamicUcLearn_part3_checkLimits
(
    IN GT_U32  fdbManagerId,
    IN GT_U32  total_entriesDeletedNum
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   limitInfo;
    GT_U32  limitValue,currentValue;
    GT_U32  _p[3];
    GT_U16  _v[2]={PRV_TGF_VLANID_2_CNS,PRV_TGF_VLANID_5_CNS};

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_E;
    limitInfo.limitIndex.fid = 0;/* dummy : limitInfo.limitIndex not used for 'global' */
    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG2_MAC("actual value [%d] on global out of limit [%d] \n",currentValue,limitValue);
    UTF_VERIFY_EQUAL0_STRING_MAC(learnLimits.maxDynamicUcMacGlobalLimit/*the old limit*/ - currentValue,
        total_entriesDeletedNum,
        "number of 'over the top'");

    UTF_VERIFY_EQUAL0_STRING_MAC(180,
        limitValue,
        "global limit");

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[0];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on fid[%d] \n",currentValue,limitValue,limitInfo.limitIndex.fid);

    UTF_VERIFY_EQUAL1_STRING_MAC(99,
        currentValue,
        "actual number on fid[%d]",limitInfo.limitIndex.fid);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[1];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on fid[%d] \n",currentValue,limitValue,limitInfo.limitIndex.fid);
    UTF_VERIFY_EQUAL1_STRING_MAC(50,
        currentValue,
        "actual number on fid[%d]",limitInfo.limitIndex.fid);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[0];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(60,
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[1];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(39,
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[2];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(50,
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);
}

/**
* @internal basicLimitDynamicUcLearn_part3 function
* @endinternal
*
* @brief  part 3 of the test : delete of the top dynamic entries
*/
static GT_VOID basicLimitDynamicUcLearn_part3
(
    IN GT_U32       fdbManagerId
)
{
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC  entriesDeletedArray[NUM_ENTRIES_STEPS_CNS];
    CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC  scanParams;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC    counters;
    GT_STATUS rc;
    GT_U32  learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E];
    GT_U32  addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E];
    GT_U32  entriesDeletedNum;
    GT_BOOL scanStart;
    GT_U32  total_entriesDeletedNum = 0;

    cpssOsMemSet(&scanParams, 0, sizeof(scanParams));
    scanParams.deleteMacUcEntries       = GT_TRUE;
    scanParams.onlyAboveLearnLimit      = GT_TRUE;

    scanStart = GT_TRUE;

    PRV_UTF_LOG0_MAC("Delete scan to remove the 'over the top' entries \n");

    rc = GT_OK;
    while(rc == GT_OK)
    {
        rc = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, scanStart,
            &scanParams, entriesDeletedArray, &entriesDeletedNum);
        if(rc != GT_OK)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "ERROR: cpssDxChBrgFdbManagerDeleteScan", fdbManagerId);
        }
        scanStart  = GT_FALSE;

        total_entriesDeletedNum += entriesDeletedNum;
    }

    PRV_UTF_LOG0_MAC("check that the 'over the top' entries removed \n");

    basicLimitDynamicUcLearn_part3_checkLimits(fdbManagerId,total_entriesDeletedNum);

    PRV_UTF_LOG0_MAC("Check counters and statistics \n");
    /* Counter should be valid */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    /* the old value '200' not the 150 set by basicLimitDynamicUcLearn_part2_config() */
    counters.macUnicastDynamic      = learnLimits.maxDynamicUcMacGlobalLimit - total_entriesDeletedNum;
    counters.macUnicastStatic       = 0;
    counters.usedEntries            = counters.macUnicastDynamic;
    counters.freeEntries            = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntries;
    prvCpssDxChBrgFdbManagerCounterVerify(fdbManagerId, &counters);

    LOG_PARAM(counters.usedEntries);

    /* Verify Statistics counter */
    cpssOsMemSet(learningScanStatisticsArr, 0, sizeof(learningScanStatisticsArr));
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]           = 0;
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E]   = 0;
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E] = 0;

    cpssOsMemSet(addEntryStatisticsArr, 0, sizeof(addEntryStatisticsArr));
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_PORT_LIMIT_E        ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TRUNK_LIMIT_E       ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_FID_LIMIT_E         ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_LIMIT_E      ] = 0;

    prvTgfBrgFdbManagerLearningScan_statistics_verify(fdbManagerId, learningScanStatisticsArr,addEntryStatisticsArr);

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);
}

/**
* @internal basicLimitDynamicUcLearn_part4_config function
* @endinternal
*
* @brief  part 4 of the test : test with trunk limit defer from limits of 'learnLimitsInfo'
*/
static GT_VOID basicLimitDynamicUcLearn_part4_config
(
    IN GT_U32       fdbManagerId,
    IN GT_TRUNK_ID  trunkId
)
{
    GT_STATUS                                   rc;
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   limitInfo;
    GT_U32  limitValue,currentValue;
    CPSS_TRUNK_MEMBER_STC    enabledMembersArray[2];
    GT_U32  _p[3];

    PRV_UTF_LOG1_MAC("set trunk[%d] limit from 120 to 27 \n",trunkId);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_TRUNK_E;
    limitInfo.limitIndex.trunkId = trunkId;
    limitValue = 27;/* was 120 in the default value */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    UTF_VERIFY_EQUAL1_STRING_MAC(0,
        currentValue,
        "actual number on trunk[%d] must be ZERO",trunkId);

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    enabledMembersArray[0].hwDevice = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    enabledMembersArray[0].port     = _p[1];

    enabledMembersArray[1].hwDevice = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    enabledMembersArray[1].port     = _p[0];

    PRV_UTF_LOG3_MAC("define trunk [%d] with 2 members : port[%d],port[%d] \n",trunkId,_p[1],_p[0]);
    /* define trunk with 2 members */
    rc = cpssDxChTrunkMembersSet(prvTgfDevNum,trunkId,
        2,enabledMembersArray,
        0,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChTrunkMembersSet: expected to GT_OK on trunkId [%d]",
            trunkId);

}

/**
* @internal basicLimitDynamicUcLearn_part4_checkLimits function
* @endinternal
*
* @brief  part 4 of the test : check limit values
*/
static GT_VOID basicLimitDynamicUcLearn_part4_checkLimits
(
    IN GT_U32  fdbManagerId
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   limitInfo;
    GT_U32  limitValue,currentValue;
    GT_U32  _p[3];
    GT_U16  _v[2]={PRV_TGF_VLANID_2_CNS,PRV_TGF_VLANID_5_CNS};
    GT_TRUNK_ID  trunkId;

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    /* calculate trunk id */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_E;
    limitInfo.limitIndex.fid = 0;/* dummy : limitInfo.limitIndex not used for 'global' */
    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);
    /* the number of entries reduced , because we allow only 27 entries on the trunk
       and it steals entries from the ports _p[0],_p[1] in _v[0]
       and many entries deleted due to 'over the limit'
    */
    PRV_UTF_LOG2_MAC("actual value [%d] on global out of limit [%d] \n",currentValue,limitValue);
    UTF_VERIFY_EQUAL0_STRING_MAC((27+59)/*86*/,
        currentValue,
        "number of entries");

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[0];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on fid[%d] \n",currentValue,limitValue,limitInfo.limitIndex.fid);

    UTF_VERIFY_EQUAL1_STRING_MAC(27,
        currentValue,
        "actual number on fid[%d]",limitInfo.limitIndex.fid);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[1];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on fid[%d] \n",currentValue,limitValue,limitInfo.limitIndex.fid);
    UTF_VERIFY_EQUAL1_STRING_MAC(59,
        currentValue,
        "actual number on fid[%d]",limitInfo.limitIndex.fid);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[0];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[1];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[2];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(59,
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);


    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_TRUNK_E;
    limitInfo.limitIndex.trunkId = trunkId;

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on trunk[%d] \n",currentValue,limitValue,limitInfo.limitIndex.trunkId);
    UTF_VERIFY_EQUAL1_STRING_MAC(27,
        currentValue,
        "actual number on trunk[%d]",limitInfo.limitIndex.trunkId);


}
/**
* @internal basicLimitDynamicUcLearn_part4 function
* @endinternal
*
* @brief  part 4 of the test : define trunk and check for it limits
*/
static GT_VOID basicLimitDynamicUcLearn_part4
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC    counters;
    GT_U32  learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E];
    GT_U32  addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E];
    GT_TRUNK_ID trunkId;

    /* calculate trunk id */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* set trunk config and limits */
    basicLimitDynamicUcLearn_part4_config(fdbManagerId,trunkId);

    /* send the traffic */
    basicLimitDynamicUcLearn_common_traffic(0/*start index*/);

    /* check limit values */
    basicLimitDynamicUcLearn_part4_checkLimits(fdbManagerId);

    PRV_UTF_LOG0_MAC("Check counters and statistics \n");
    /* Counter should be valid */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    counters.macUnicastDynamic      = 27+59/*86*/;
    counters.macUnicastStatic       = 0;
    counters.usedEntries            = counters.macUnicastDynamic;
    counters.freeEntries            = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntries;
    prvCpssDxChBrgFdbManagerCounterVerify(fdbManagerId, &counters);

    LOG_PARAM(counters.usedEntries);

    /* Verify Statistics counter */
    cpssOsMemSet(learningScanStatisticsArr, 0, sizeof(learningScanStatisticsArr));
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]           = totalSent - 50;/*_p[2] traffic already known*/
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E]   = GT_NA;/* indication that 'OK'+'OK_NO_MORE' checked together */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E] = 99;/* all entries from _p[0],_p[1] moved to the trunk (99-27) of them dropped on 'over the limit' */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E] =
        learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]-
        learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E];

    cpssOsMemSet(addEntryStatisticsArr, 0, sizeof(addEntryStatisticsArr));
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_PORT_LIMIT_E        ] = 41;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TRUNK_LIMIT_E       ] = 30;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_FID_LIMIT_E         ] = 51;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_LIMIT_E      ] = 0;

    prvTgfBrgFdbManagerLearningScan_statistics_verify(fdbManagerId, learningScanStatisticsArr,addEntryStatisticsArr);

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);

}

/**
* @internal basicLimitDynamicUcLearn_part5_checkLimits function
* @endinternal
*
* @brief  part 5 of the test : check limit values
*/
static GT_VOID basicLimitDynamicUcLearn_part5_checkLimits
(
    IN GT_U32  fdbManagerId
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   limitInfo;
    GT_U32  limitValue,currentValue;
    GT_U32  _p[3];
    GT_U16  _v[2]={PRV_TGF_VLANID_2_CNS,PRV_TGF_VLANID_5_CNS};
    GT_TRUNK_ID  trunkId;

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    /* calculate trunk id */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_E;
    limitInfo.limitIndex.fid = 0;/* dummy : limitInfo.limitIndex not used for 'global' */
    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);
    /* the number of entries reduced , because we allow only 27 entries on the trunk
       we allow 55 on _p[1] , we allow 59 on _p[2]
    */
    PRV_UTF_LOG2_MAC("actual value [%d] on global out of limit [%d] \n",currentValue,limitValue);
    UTF_VERIFY_EQUAL0_STRING_MAC((27+55+59)/*132*/,
        currentValue,
        "number of entries");

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[0];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on fid[%d] \n",currentValue,limitValue,limitInfo.limitIndex.fid);

    UTF_VERIFY_EQUAL1_STRING_MAC((27+55),/*trunk-->[27],_p[1]-->55*/
        currentValue,
        "actual number on fid[%d]",limitInfo.limitIndex.fid);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[1];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on fid[%d] \n",currentValue,limitValue,limitInfo.limitIndex.fid);
    UTF_VERIFY_EQUAL1_STRING_MAC(59,
        currentValue,
        "actual number on fid[%d]",limitInfo.limitIndex.fid);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[0];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[1];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(55,
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[2];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(59,
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);


    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_TRUNK_E;
    limitInfo.limitIndex.trunkId = trunkId;

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on trunk[%d] \n",currentValue,limitValue,limitInfo.limitIndex.trunkId);
    UTF_VERIFY_EQUAL1_STRING_MAC(27,
        currentValue,
        "actual number on trunk[%d]",limitInfo.limitIndex.trunkId);
}
/**
* @internal basicLimitDynamicUcLearn_part5 function
* @endinternal
*
* @brief  part 5 of the test : remove port from trunk and check limits
*/
static GT_VOID basicLimitDynamicUcLearn_part5
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC    counters;
    GT_U32  learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E];
    GT_U32  addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E];
    GT_TRUNK_ID trunkId;
    CPSS_TRUNK_MEMBER_STC    member;
    GT_U32  _p[3];

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    /* calculate trunk id */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    member.port = _p[1];
    member.hwDevice = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    PRV_UTF_LOG2_MAC("Remove port [%d] from trunkId[%d] \n",
        member.port,trunkId);
    rc = cpssDxChTrunkMemberRemove(prvTgfDevNum,trunkId,&member);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChTrunkMemberRemove: expected to GT_OK on trunkId [%d]",
            trunkId);

    /* send the traffic */
    basicLimitDynamicUcLearn_common_traffic(0/*start index*/);

    /* check limit values */
    basicLimitDynamicUcLearn_part5_checkLimits(fdbManagerId);

    PRV_UTF_LOG0_MAC("Check counters and statistics \n");
    /* Counter should be valid */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    counters.macUnicastDynamic      = 27+55+59/*141*/;/*sum of trunk,_p[1],_p[2]*/
    counters.macUnicastStatic       = 0;
    counters.usedEntries            = counters.macUnicastDynamic;
    counters.freeEntries            = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntries;
    prvCpssDxChBrgFdbManagerCounterVerify(fdbManagerId, &counters);

    LOG_PARAM(counters.usedEntries);

    /* Verify Statistics counter */
    cpssOsMemSet(learningScanStatisticsArr, 0, sizeof(learningScanStatisticsArr));
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]           = totalSent - 50 - 27;/*_p[2] traffic already known , trunk traffic already known */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E]   = GT_NA;/* indication that 'OK'+'OK_NO_MORE' checked together */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E] = 9;/* some entries from trunk moved back to _p[1] */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E] =
        learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]-
        learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E];

    cpssOsMemSet(addEntryStatisticsArr, 0, sizeof(addEntryStatisticsArr));
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_PORT_LIMIT_E        ] = 76;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TRUNK_LIMIT_E       ] = 63;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_FID_LIMIT_E         ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_LIMIT_E      ] = 0;

    prvTgfBrgFdbManagerLearningScan_statistics_verify(fdbManagerId, learningScanStatisticsArr,addEntryStatisticsArr);

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);

}

/**
* @internal basicLimitDynamicUcLearn_part6_config function
* @endinternal
*
* @brief  part 6 of the test : set new higher limits on ports (not modify trunk/fid/global)
*/
static GT_VOID basicLimitDynamicUcLearn_part6_config
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS                                   rc;
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   limitInfo;
    GT_U32  limitValue,currentValue;
    GT_U32  _p[3];

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    PRV_UTF_LOG1_MAC("Enlarge port[%d] limit from 80 to 150 \n",_p[0]);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[0];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    limitValue = 150;/* was 80 */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    /* we expect 0 , as it's entries moved to the trunk */
    UTF_VERIFY_EQUAL1_STRING_MAC(0,currentValue,"actual num in port[%d] limit",
        limitInfo.limitIndex.devPort.portNum);


    PRV_UTF_LOG1_MAC("Enlarge port[%d] limit from 55 to 150 \n",_p[1]);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[1];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    limitValue = 150;/* was 55 */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    /* we expect 55 , as it reached the limit */
    UTF_VERIFY_EQUAL1_STRING_MAC(55,currentValue,"actual num in port[%d] limit",
        limitInfo.limitIndex.devPort.portNum);

    PRV_UTF_LOG1_MAC("Enlarge port[%d] limit from 59 to 150 \n",_p[2]);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[2];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    limitValue = 150;/* was 59 */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    /* we expect 59 , as it reached the limit */
    UTF_VERIFY_EQUAL1_STRING_MAC(59,currentValue,"actual num in port[%d] limit",
        limitInfo.limitIndex.devPort.portNum);
}

/**
* @internal basicLimitDynamicUcLearn_part6_checkLimits function
* @endinternal
*
* @brief  part 6 of the test : check limit values
*/
static GT_VOID basicLimitDynamicUcLearn_part6_checkLimits
(
    IN GT_U32  fdbManagerId
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   limitInfo;
    GT_U32  limitValue,currentValue;
    GT_U32  _p[3];
    GT_U16  _v[2]={PRV_TGF_VLANID_2_CNS,PRV_TGF_VLANID_5_CNS};
    GT_TRUNK_ID  trunkId;

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    /* calculate trunk id */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_E;
    limitInfo.limitIndex.fid = 0;/* dummy : limitInfo.limitIndex not used for 'global' */
    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);
    /* we reach the global limit */
    PRV_UTF_LOG2_MAC("actual value [%d] on global out of limit [%d] \n",currentValue,limitValue);
    UTF_VERIFY_EQUAL0_STRING_MAC(180,
        currentValue,
        "number of entries");

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[0];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on fid[%d] \n",currentValue,limitValue,limitInfo.limitIndex.fid);
    /* we reach fid limit */
    UTF_VERIFY_EQUAL1_STRING_MAC(99,
        currentValue,
        "actual number on fid[%d]",limitInfo.limitIndex.fid);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[1];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on fid[%d] \n",currentValue,limitValue,limitInfo.limitIndex.fid);
    UTF_VERIFY_EQUAL1_STRING_MAC(180-99,/* global-fid2 limit */
        currentValue,
        "actual number on fid[%d]",limitInfo.limitIndex.fid);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[0];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,/* entries of the port are in the trunk */
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[1];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(99-27,/*vlan limit minus 27 on the trunk */
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[2];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(180-99,/* all those on _v[1]*/
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);


    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_TRUNK_E;
    limitInfo.limitIndex.trunkId = trunkId;

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on trunk[%d] \n",currentValue,limitValue,limitInfo.limitIndex.trunkId);
    UTF_VERIFY_EQUAL1_STRING_MAC(27,
        currentValue,
        "actual number on trunk[%d]",limitInfo.limitIndex.trunkId);
}

/**
* @internal basicLimitDynamicUcLearn_part6 function
* @endinternal
*
* @brief  part 6 of the test : enlage port limits and hit vlan limits and global
*/
static GT_VOID basicLimitDynamicUcLearn_part6
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC    counters;
    GT_U32  learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E];
    GT_U32  addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E];

    /* set new higher limits on ports (not modify trunk/fid/global) */
    basicLimitDynamicUcLearn_part6_config(fdbManagerId);

    /* send the traffic */
    basicLimitDynamicUcLearn_common_traffic(0/*start index*/);

    /* check limit values */
    basicLimitDynamicUcLearn_part6_checkLimits(fdbManagerId);

    PRV_UTF_LOG0_MAC("Check counters and statistics \n");
    /* Counter should be valid */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    counters.macUnicastDynamic      = 180;/*the global limit*/
    counters.macUnicastStatic       = 0;
    counters.usedEntries            = counters.macUnicastDynamic;
    counters.freeEntries            = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntries;
    prvCpssDxChBrgFdbManagerCounterVerify(fdbManagerId, &counters);

    LOG_PARAM(counters.usedEntries);

    /* Verify Statistics counter */
    cpssOsMemSet(learningScanStatisticsArr, 0, sizeof(learningScanStatisticsArr));
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]           = totalSent - 59/*_p[2]*/ - 55/*_p[1]*/ - 27/*trunk*/;
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E]   = GT_NA;/* indication that 'OK'+'OK_NO_MORE' checked together */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E] = 0;/* no movements */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E] =
        learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]-
        learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E];

    cpssOsMemSet(addEntryStatisticsArr, 0, sizeof(addEntryStatisticsArr));
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_PORT_LIMIT_E        ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TRUNK_LIMIT_E       ] = 63;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_FID_LIMIT_E         ] = 18;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_LIMIT_E      ] = 19;

    prvTgfBrgFdbManagerLearningScan_statistics_verify(fdbManagerId, learningScanStatisticsArr,addEntryStatisticsArr);

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);

}

/**
* @internal basicLimitDynamicUcLearn_part7_config function
* @endinternal
*
* @brief  part 7 of the test : set new lower limits on ports on trunk
*/
static GT_VOID basicLimitDynamicUcLearn_part7_config
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS                                   rc;
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   limitInfo;
    GT_U32  limitValue,currentValue;
    GT_U32  _p[3];
    GT_TRUNK_ID  trunkId;

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    /* calculate trunk id */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    PRV_UTF_LOG1_MAC("Reduce port[%d] limit from 150 to 23 \n",_p[0]);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[0];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    limitValue = 23;/* was 150 -- we need less than 27 as we transplant from trunk to it later */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    /* we expect 0 , as it's entries moved to the trunk */
    UTF_VERIFY_EQUAL1_STRING_MAC(0,currentValue,"actual num in port[%d] limit",
        limitInfo.limitIndex.devPort.portNum);


    PRV_UTF_LOG1_MAC("Reduce port[%d] limit from 150 to 49 \n",_p[1]);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[1];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    limitValue = 49;/* was 150 */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    /* we expect 72 , as it reached the limit */
    UTF_VERIFY_EQUAL1_STRING_MAC(99/*vlan limit*/-27/*trunk limit*/,currentValue,"actual num in port[%d] limit",
        limitInfo.limitIndex.devPort.portNum);

    PRV_UTF_LOG1_MAC("Reduce port[%d] limit from 150 to 39 \n",_p[2]);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[2];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    limitValue = 39;/* was 150 */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    /* we expect 81 , as it reached the limit */
    UTF_VERIFY_EQUAL1_STRING_MAC(180/*global*/-99/*_v[0]*/,currentValue,"actual num in port[%d] limit",
        limitInfo.limitIndex.devPort.portNum);

    PRV_UTF_LOG1_MAC("Reduce trunk[%d] limit from 27 to 15 \n",trunkId);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_TRUNK_E;
    limitInfo.limitIndex.trunkId = trunkId;
    limitValue = 15;/* was 27 */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on trunk[%d] \n",currentValue,limitValue,limitInfo.limitIndex.trunkId);
    /* we expect 27 , as it reached the limit */
    UTF_VERIFY_EQUAL1_STRING_MAC(27,currentValue,"actual num in port[%d] limit",
        limitInfo.limitIndex.devPort.portNum);
}
/**
* @internal basicLimitDynamicUcLearn_part7_aging function
* @endinternal
*
* @brief  part 7 of the test : run single aging scan on the fdb
*/
static GT_VOID basicLimitDynamicUcLearn_part7_aging
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC entriesAgedoutArray[NUM_ENTRIES_STEPS_CNS];
    GT_U32                                           entriesAgedoutNum;
    CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC  ageParams;
    GT_U32                                           total_entriesAgedoutNum = 0;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    GT_U32      ii;

    PRV_UTF_LOG0_MAC("SINGLE Aging scan - make sure no entry removed (over the port/trunk limit) \n");

    cpssOsMemSet(&ageParams, 0, sizeof(ageParams));
    ageParams.checkAgeMacUcEntries          = GT_TRUE;
    ageParams.deleteAgeoutMacUcEportEntries = GT_TRUE;

    /* Age-bin Verify - counters against list */
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    rc = GT_OK;
    for(ii = 0; ii < fdbManagerPtr->agingBinInfo.totalAgeBinAllocated; ii++)
    {
        rc = cpssDxChBrgFdbManagerAgingScan(fdbManagerId,
            &ageParams, entriesAgedoutArray, &entriesAgedoutNum);
        if(rc != GT_OK)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "ERROR: cpssDxChBrgFdbManagerAgingScan", fdbManagerId);
        }

        total_entriesAgedoutNum += entriesAgedoutNum;

    }

    UTF_VERIFY_EQUAL0_STRING_MAC(0, total_entriesAgedoutNum,
        "single aging scan must not remove any entry");


}
/**
* @internal basicLimitDynamicUcLearn_part7 function
* @endinternal
*
* @brief  part 7 of the test : lower port , trunk limits and check SINGLE aging scan not delete over the top
*/
static GT_VOID basicLimitDynamicUcLearn_part7
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC    counters;
    GT_U32  learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E];
    GT_U32  addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E];

    /* set new lower limits on ports on trunk */
    basicLimitDynamicUcLearn_part7_config(fdbManagerId);

    /* run single aging scan on the fdb */
    basicLimitDynamicUcLearn_part7_aging(fdbManagerId);

    /* check limit values : the same check as was in part 6 (precious part) */
    basicLimitDynamicUcLearn_part6_checkLimits(fdbManagerId);

    PRV_UTF_LOG0_MAC("Check counters and statistics \n");
    /* Counter should be valid */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    counters.macUnicastDynamic      = 180;/*the global limit*/
    counters.macUnicastStatic       = 0;
    counters.usedEntries            = counters.macUnicastDynamic;
    counters.freeEntries            = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntries;
    prvCpssDxChBrgFdbManagerCounterVerify(fdbManagerId, &counters);

    LOG_PARAM(counters.usedEntries);

    /* Verify Statistics counter */
    cpssOsMemSet(learningScanStatisticsArr, 0, sizeof(learningScanStatisticsArr));
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]           = 0;/* no learning during this part */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E]   = GT_NA;/* indication that 'OK'+'OK_NO_MORE' checked together */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E] = 0;/* no movements */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E] =
        learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]-
        learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E];

    cpssOsMemSet(addEntryStatisticsArr, 0, sizeof(addEntryStatisticsArr));
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_PORT_LIMIT_E        ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TRUNK_LIMIT_E       ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_FID_LIMIT_E         ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_LIMIT_E      ] = 0;

    prvTgfBrgFdbManagerLearningScan_statistics_verify(fdbManagerId, learningScanStatisticsArr,addEntryStatisticsArr);

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);

}

/**
* @internal basicLimitDynamicUcLearn_part8_checkLimits function
* @endinternal
*
* @brief  part 8 of the test : check limit values
*/
static GT_VOID basicLimitDynamicUcLearn_part8_checkLimits
(
    IN GT_U32  fdbManagerId
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   limitInfo;
    GT_U32  limitValue,currentValue;
    GT_U32  _p[3];
    GT_U16  _v[2]={PRV_TGF_VLANID_2_CNS,PRV_TGF_VLANID_5_CNS};
    GT_TRUNK_ID  trunkId;

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    /* calculate trunk id */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_E;
    limitInfo.limitIndex.fid = 0;/* dummy : limitInfo.limitIndex not used for 'global' */
    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);
    /* we reach the global limit */
    PRV_UTF_LOG2_MAC("actual value [%d] on global out of limit [%d] \n",currentValue,limitValue);
    UTF_VERIFY_EQUAL0_STRING_MAC(23+49+39/*122*/, /* limit on _p[0] + _p[1] + _p[2] */
        currentValue,
        "number of entries");

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[0];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on fid[%d] \n",currentValue,limitValue,limitInfo.limitIndex.fid);
    /* we reach fid limit */
    UTF_VERIFY_EQUAL1_STRING_MAC((23+49),/* limit on _p[0] + _p[1] */
        currentValue,
        "actual number on fid[%d]",limitInfo.limitIndex.fid);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[1];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on fid[%d] \n",currentValue,limitValue,limitInfo.limitIndex.fid);
    UTF_VERIFY_EQUAL1_STRING_MAC(39,/* limit on _p[2] */
        currentValue,
        "actual number on fid[%d]",limitInfo.limitIndex.fid);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[0];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(23,/* limit on _p[0]*/
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[1];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(49,/* limit on _p[1] */
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[2];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(39,/* limit on _p[2] */
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);


    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_TRUNK_E;
    limitInfo.limitIndex.trunkId = trunkId;

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on trunk[%d] \n",currentValue,limitValue,limitInfo.limitIndex.trunkId);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,/*no entries on the trunk*/
        currentValue,
        "actual number on trunk[%d]",limitInfo.limitIndex.trunkId);
}

/**
* @internal basicLimitDynamicUcLearn_part8_transplant function
* @endinternal
*
* @brief  part 8 of the test : run transplant scan on the fdb
*/
static GT_VOID basicLimitDynamicUcLearn_part8_transplant_trunk_to_port
(
    IN GT_U32       fdbManagerId,
    IN GT_TRUNK_ID  oldTrunkId,
    IN GT_U32       newPort,
    IN GT_U32       numEntriesToTransplant
)
{
    CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC  transplantParams;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC      entriesTransplantedArray[NUM_ENTRIES_STEPS_CNS];
    GT_U32                                                entriesTransplantedNum;
    GT_STATUS   rc;
    GT_U32  total_entriesTransplantedNum = 0;
    GT_BOOL scanStart;

    cpssOsMemSet(&transplantParams, 0, sizeof(transplantParams));
    transplantParams.transplantMacUcEntries                       = GT_TRUE;
    transplantParams.oldInterface.type                            = CPSS_INTERFACE_TRUNK_E;
    transplantParams.oldInterface.interfaceInfo.trunkId           = oldTrunkId;
    transplantParams.newInterface.type                            = CPSS_INTERFACE_PORT_E;
    transplantParams.newInterface.interfaceInfo.devPort.hwDevNum  = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    transplantParams.newInterface.interfaceInfo.devPort.portNum   = newPort;

    scanStart = GT_TRUE;
    rc = GT_OK;

    PRV_UTF_LOG2_MAC("Transplant scan to move from trunk[%d] to port [%d] also remove the 'over the top' entries \n",
        oldTrunkId,newPort);

    while(rc == GT_OK)
    {
        rc = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart,
            &transplantParams, entriesTransplantedArray, &entriesTransplantedNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "ERROR: cpssDxChBrgFdbManagerTransplantScan", prvTgfDevNum);
        if(rc != GT_OK)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "ERROR: cpssDxChBrgFdbManagerDeleteScan", fdbManagerId);
        }

        scanStart  = GT_FALSE;

        total_entriesTransplantedNum += entriesTransplantedNum;
    }

    PRV_UTF_LOG0_MAC("check that the 'over the top' entries removed \n");

    UTF_VERIFY_EQUAL0_STRING_MAC(numEntriesToTransplant, total_entriesTransplantedNum,
        "The number of entries from the trunk");
}

/**
* @internal basicLimitDynamicUcLearn_part8_transplant function
* @endinternal
*
* @brief  part 8 of the test : run transplant scan on the fdb
*/
static GT_VOID basicLimitDynamicUcLearn_part8_transplant_port_to_port
(
    IN GT_U32       fdbManagerId,
    IN GT_U32       oldPort,
    IN GT_U32       newPort,
    IN GT_U32       numEntriesToTransplant
)
{
    CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC  transplantParams;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC      entriesTransplantedArray[NUM_ENTRIES_STEPS_CNS];
    GT_U32                                                entriesTransplantedNum;
    GT_STATUS   rc;
    GT_BOOL scanStart;
    GT_U32  total_entriesTransplantedNum = 0;

    cpssOsMemSet(&transplantParams, 0, sizeof(transplantParams));
    transplantParams.transplantMacUcEntries                       = GT_TRUE;
    transplantParams.oldInterface.type                            = CPSS_INTERFACE_PORT_E;
    transplantParams.oldInterface.interfaceInfo.devPort.hwDevNum  = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    transplantParams.oldInterface.interfaceInfo.devPort.portNum   = oldPort;
    transplantParams.newInterface.type                            = CPSS_INTERFACE_PORT_E;
    transplantParams.newInterface.interfaceInfo.devPort.hwDevNum  = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    transplantParams.newInterface.interfaceInfo.devPort.portNum   = newPort;

    scanStart = GT_TRUE;
    rc = GT_OK;

    PRV_UTF_LOG2_MAC("Transplant scan to move from port[%d] to port [%d] also remove the 'over the top' entries \n",
        oldPort , newPort);

    while(rc == GT_OK)
    {
        rc = cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart,
            &transplantParams, entriesTransplantedArray, &entriesTransplantedNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "ERROR: cpssDxChBrgFdbManagerTransplantScan", prvTgfDevNum);
        if(rc != GT_OK)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "ERROR: cpssDxChBrgFdbManagerDeleteScan", fdbManagerId);
        }

        scanStart  = GT_FALSE;

        total_entriesTransplantedNum += entriesTransplantedNum;
    }

    PRV_UTF_LOG0_MAC("check that the 'over the top' entries removed \n");

    UTF_VERIFY_EQUAL0_STRING_MAC(numEntriesToTransplant, total_entriesTransplantedNum,
        "The number of entries from the trunk");
}

/**
* @internal basicLimitDynamicUcLearn_part8_transplant function
* @endinternal
*
* @brief  part 8 of the test : run transplant scan on the fdb
*/
static GT_VOID basicLimitDynamicUcLearn_part8_transplant_port_to_port_and_back
(
    IN GT_U32       fdbManagerId,
    IN GT_U32       oldPort,
    IN GT_U32       newPort,
    IN GT_U32       numEntriesToTransplant
)
{
    /* transplant old to new */
    basicLimitDynamicUcLearn_part8_transplant_port_to_port(fdbManagerId,oldPort,newPort,numEntriesToTransplant);
    /* transplant new to old (transplant back) */
    basicLimitDynamicUcLearn_part8_transplant_port_to_port(fdbManagerId,newPort,oldPort,numEntriesToTransplant);
}

/**
* @internal basicLimitDynamicUcLearn_part8 function
* @endinternal
*
* @brief  part 8 of the test : check transplant scan to remove over the top entries
*/
static GT_VOID basicLimitDynamicUcLearn_part8
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC    counters;
    GT_U32  learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E];
    GT_U32  addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E];
    GT_U32  _p[3];
    GT_TRUNK_ID  trunkId;
    GT_U32  dummyPort;

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];
    dummyPort = prvTgfPortsArray[4];

    /* calculate trunk id */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* run transplant scan on the fdb */
    basicLimitDynamicUcLearn_part8_transplant_trunk_to_port(fdbManagerId,
        trunkId,_p[0],27/* num entries on the trunk*/);

    /* run transplant scan on the fdb */
    basicLimitDynamicUcLearn_part8_transplant_port_to_port_and_back(fdbManagerId,
        _p[1],dummyPort,72/* num entries on the port*/);

    /* run transplant scan on the fdb */
    basicLimitDynamicUcLearn_part8_transplant_port_to_port_and_back(fdbManagerId,
        _p[2],dummyPort,81/* num entries on the port*/);

    /* check limit values */
    basicLimitDynamicUcLearn_part8_checkLimits(fdbManagerId);

    PRV_UTF_LOG0_MAC("Check counters and statistics \n");
    /* Counter should be valid */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    counters.macUnicastDynamic      = 23+49+39/*122*/; /* limit on _p[0] + _p[1] + _p[2] */
    counters.macUnicastStatic       = 0;
    counters.usedEntries            = counters.macUnicastDynamic;
    counters.freeEntries            = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntries;
    prvCpssDxChBrgFdbManagerCounterVerify(fdbManagerId, &counters);

    LOG_PARAM(counters.usedEntries);

    /* Verify Statistics counter */
    cpssOsMemSet(learningScanStatisticsArr, 0, sizeof(learningScanStatisticsArr));
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]           = 0;/* no learning during this part */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E]   = GT_NA;/* indication that 'OK'+'OK_NO_MORE' checked together */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E] = 0;/* no movements */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E] =
        learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]-
        learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E];

    cpssOsMemSet(addEntryStatisticsArr, 0, sizeof(addEntryStatisticsArr));
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_PORT_LIMIT_E        ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TRUNK_LIMIT_E       ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_FID_LIMIT_E         ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_LIMIT_E      ] = 0;

    prvTgfBrgFdbManagerLearningScan_statistics_verify(fdbManagerId, learningScanStatisticsArr,addEntryStatisticsArr);

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG1_MAC("delete the trunk [%d] \n",trunkId);
    /* delete trunk */
    rc = cpssDxChTrunkMembersSet(prvTgfDevNum,trunkId,0,NULL,0,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChTrunkMembersSet: expected to GT_OK on trunkId [%d] - delete the trunk",
            trunkId);
}


/**
* @internal basicLimitDynamicUcLearn_part9 function
* @endinternal
*
* @brief  part 9 of the test : bind the global eport to the port and vise versa
*/
static GT_VOID basicLimitDynamicUcLearn_part9_Bind_Eport_to_Physical
(
    IN GT_U32       eportNum,
    IN GT_U32       portNum
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC physicalPortInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC learningInfo;

    physicalPortInfo.type = CPSS_INTERFACE_PORT_E;
    physicalPortInfo.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    physicalPortInfo.devPort.portNum  = portNum;

    /* bind the trg eport to be mapped to trg physical port */
    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
        eportNum,&physicalPortInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet: expected to GT_OK on device [%d]",
            prvTgfDevNum);

    /* bind the src physical port to be mapped to src eport */
    rc = cpssDxChCfgPortDefaultSourceEportNumberSet(prvTgfDevNum,portNum,eportNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChCfgPortDefaultSourceEportNumberSet: expected to GT_OK on device [%d]",
            prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerPortLearningGet(prvTgfDevNum,eportNum,&learningInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerPortLearningGet: expected to GT_OK on device [%d] eport[%d]",
            prvTgfDevNum,eportNum);

    /* enable the NA msg to CPU */
     learningInfo.naMsgToCpuEnable = GT_TRUE;

     cpssDxChBrgFdbManagerPortLearningSet(prvTgfDevNum,eportNum,&learningInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerPortLearningSet: expected to GT_OK on device [%d] eport[%d]",
            prvTgfDevNum,eportNum);

}
/**
* @internal basicLimitDynamicUcLearn_part9_restore_Eport_and_Physical function
* @endinternal
*
* @brief  part 9 of the test : bind the global eport to the port and vise versa
*/
static GT_VOID basicLimitDynamicUcLearn_part9_restore_Eport_and_Physical
(
    IN GT_U32       eportNum,
    IN GT_U32       portNum
)
{
    GT_STATUS   rc;

    eportNum = eportNum;/* not used */

    /* restore the src physical port to be mapped to src eport */
    rc = cpssDxChCfgPortDefaultSourceEportNumberSet(prvTgfDevNum,portNum,portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChCfgPortDefaultSourceEportNumberSet: expected to GT_OK on device [%d]",
            prvTgfDevNum);
}

/**
* @internal basicLimitDynamicUcLearn_part9_checkLimits function
* @endinternal
*
* @brief  part 9 of the test : check limit values
*/
static GT_VOID basicLimitDynamicUcLearn_part9_checkLimits
(
    IN GT_U32  fdbManagerId
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   limitInfo;
    GT_U32  limitValue,currentValue;
    GT_U16  _v[2]={PRV_TGF_VLANID_2_CNS,PRV_TGF_VLANID_5_CNS};
    GT_U32  _p[3]; /* physical ports */
    GT_U32  _ep[3];/* global  eports */

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    /* global eport from the start  of range */
    _ep[0] = PRV_TGF_GLOBAL_EPORT_MIN_CNS + 1;
    /* global eport from the middle of range */
    _ep[1] = (PRV_TGF_GLOBAL_EPORT_MAX_CNS + PRV_TGF_GLOBAL_EPORT_MIN_CNS)/2;
    /* global eport from the end    of range */
    _ep[2] = PRV_TGF_GLOBAL_EPORT_MAX_CNS - 1;

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_E;
    limitInfo.limitIndex.fid = 0;/* dummy : limitInfo.limitIndex not used for 'global' */
    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);
    /* not reach global limit */
    PRV_UTF_LOG2_MAC("actual value [%d] on global out of limit [%d] \n",currentValue,limitValue);
    UTF_VERIFY_EQUAL0_STRING_MAC(180,/*all the packets we sent*/
        currentValue,
        "number of entries");

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[0];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on fid[%d] \n",currentValue,limitValue,limitInfo.limitIndex.fid);
    /* we reach fid limit */
    UTF_VERIFY_EQUAL1_STRING_MAC(99,
        currentValue,
        "actual number on fid[%d]",limitInfo.limitIndex.fid);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[1];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on fid[%d] \n",currentValue,limitValue,limitInfo.limitIndex.fid);
    UTF_VERIFY_EQUAL1_STRING_MAC(180-99,/* global-fid2 limit */
        currentValue,
        "actual number on fid[%d]",limitInfo.limitIndex.fid);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[0];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,/* entries of the port are in the eport */
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[1];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,/* entries of the port are in the eport */
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[2];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,/* entries of the port are in the eport */
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E;
    limitInfo.limitIndex.globalEport = _ep[0];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on global eport[%d] \n",currentValue,limitValue,limitInfo.limitIndex.globalEport);
    UTF_VERIFY_EQUAL1_STRING_MAC(50,
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.globalEport);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E;
    limitInfo.limitIndex.globalEport = _ep[1];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on global eport[%d] \n",currentValue,limitValue,limitInfo.limitIndex.globalEport);
    UTF_VERIFY_EQUAL1_STRING_MAC(99-50,/*v[0]-ep[0]*/
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.globalEport);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E;
    limitInfo.limitIndex.globalEport = _ep[2];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on global eport[%d] \n",currentValue,limitValue,limitInfo.limitIndex.globalEport);
    UTF_VERIFY_EQUAL1_STRING_MAC(180-99,/* all those on _v[1]*/
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.globalEport);

}

/**
* @internal basicLimitDynamicUcLearn_part9_noGlobalEportLimit function
* @endinternal
*
* @brief  part 9 of the test : check global eports learning - no global eport limits
*/
static GT_VOID basicLimitDynamicUcLearn_part9_noGlobalEportLimit
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC    counters;
    GT_U32  learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E];
    GT_U32  addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E];
    GT_U32  _p[3]; /* physical ports */
    GT_U32  _ep[3];/* global  eports */

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    /* global eport from the start  of range */
    _ep[0] = PRV_TGF_GLOBAL_EPORT_MIN_CNS + 1;
    /* global eport from the middle of range */
    _ep[1] = (PRV_TGF_GLOBAL_EPORT_MAX_CNS + PRV_TGF_GLOBAL_EPORT_MIN_CNS)/2;
    /* global eport from the end    of range */
    _ep[2] = PRV_TGF_GLOBAL_EPORT_MAX_CNS - 1;

    /* bind the global eport to the port and vise versa */
    basicLimitDynamicUcLearn_part9_Bind_Eport_to_Physical(_ep[0],_p[0]);
    basicLimitDynamicUcLearn_part9_Bind_Eport_to_Physical(_ep[1],_p[1]);
    basicLimitDynamicUcLearn_part9_Bind_Eport_to_Physical(_ep[2],_p[2]);

    /* send the traffic */
    basicLimitDynamicUcLearn_common_traffic(0/*start index*/);

    /* check limit values */
    basicLimitDynamicUcLearn_part9_checkLimits(fdbManagerId);

    PRV_UTF_LOG0_MAC("Check counters and statistics \n");
    /* Counter should be valid */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    counters.macUnicastDynamic      = 180;
    counters.macUnicastStatic       = 0;
    counters.usedEntries            = counters.macUnicastDynamic;
    counters.freeEntries            = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntries;
    prvCpssDxChBrgFdbManagerCounterVerify(fdbManagerId, &counters);

    LOG_PARAM(counters.usedEntries);

    /* Verify Statistics counter */
    cpssOsMemSet(learningScanStatisticsArr, 0, sizeof(learningScanStatisticsArr));
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]           = 280;
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E]   = GT_NA;/* indication that 'OK'+'OK_NO_MORE' checked together */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E] = 111;
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E] =
        learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]-
        learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E];

    cpssOsMemSet(addEntryStatisticsArr, 0, sizeof(addEntryStatisticsArr));
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_PORT_LIMIT_E        ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TRUNK_LIMIT_E       ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_FID_LIMIT_E         ] = 81;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_LIMIT_E      ] = 19;

    prvTgfBrgFdbManagerLearningScan_statistics_verify(fdbManagerId, learningScanStatisticsArr,addEntryStatisticsArr);

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);

    /* restore */
    basicLimitDynamicUcLearn_part9_restore_Eport_and_Physical(_ep[0],_p[0]);
    basicLimitDynamicUcLearn_part9_restore_Eport_and_Physical(_ep[1],_p[1]);
    basicLimitDynamicUcLearn_part9_restore_Eport_and_Physical(_ep[2],_p[2]);
}

/**
* @internal basicLimitDynamicUcLearn_part9_setGlobalEportsLimits function
* @endinternal
*
* @brief  part 9 of the test : set global eport limits
*/
static GT_VOID basicLimitDynamicUcLearn_part9_setGlobalEportsLimits
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS                                   rc;
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   limitInfo;
    GT_U32  limitValue,currentValue;
    GT_U32  _ep[3];/* global  eports */

    /* global eport from the start  of range */
    _ep[0] = PRV_TGF_GLOBAL_EPORT_MIN_CNS + 1;
    /* global eport from the middle of range */
    _ep[1] = (PRV_TGF_GLOBAL_EPORT_MAX_CNS + PRV_TGF_GLOBAL_EPORT_MIN_CNS)/2;
    /* global eport from the end    of range */
    _ep[2] = PRV_TGF_GLOBAL_EPORT_MAX_CNS - 1;


    PRV_UTF_LOG1_MAC("Set Global Eport limit to 55 on eport [%d] \n",_ep[0]);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E;
    limitInfo.limitIndex.globalEport = _ep[0];/* dummy : limitInfo.limitIndex not used for 'global' */
    limitValue = 55;/* was unlimited */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG1_MAC("Set Global Eport limit to 34 on eport [%d] \n",_ep[1]);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E;
    limitInfo.limitIndex.globalEport = _ep[1];/* dummy : limitInfo.limitIndex not used for 'global' */
    limitValue = 34;/* was unlimited */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG1_MAC("Set Global Eport limit to 36 on eport [%d] \n",_ep[2]);
    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E;
    limitInfo.limitIndex.globalEport = _ep[2];/* dummy : limitInfo.limitIndex not used for 'global' */
    limitValue = 36;/* was unlimited */

    rc = cpssDxChBrgFdbManagerLimitSet(fdbManagerId,&limitInfo,limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitSet: expected to GT_OK on manager [%d]",
            fdbManagerId);
}
/**
* @internal basicLimitDynamicUcLearn_part9_checkLimits_with_GlobalEportLimit function
* @endinternal
*
* @brief  part 9 of the test : check limit values with Global Eport Limit
*/
static GT_VOID basicLimitDynamicUcLearn_part9_checkLimits_with_GlobalEportLimit
(
    IN GT_U32  fdbManagerId,
    IN GT_BOOL      firstTime
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   limitInfo;
    GT_U32  limitValue,currentValue;
    GT_U16  _v[2]={PRV_TGF_VLANID_2_CNS,PRV_TGF_VLANID_5_CNS};
    GT_U32  _p[3]; /* physical ports */
    GT_U32  _ep[3];/* global  eports */

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    /* global eport from the start  of range */
    _ep[0] = PRV_TGF_GLOBAL_EPORT_MIN_CNS + 1;
    /* global eport from the middle of range */
    _ep[1] = (PRV_TGF_GLOBAL_EPORT_MAX_CNS + PRV_TGF_GLOBAL_EPORT_MIN_CNS)/2;
    /* global eport from the end    of range */
    _ep[2] = PRV_TGF_GLOBAL_EPORT_MAX_CNS - 1;

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_E;
    limitInfo.limitIndex.fid = 0;/* dummy : limitInfo.limitIndex not used for 'global' */
    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);
    /* not reach global limit */
    PRV_UTF_LOG2_MAC("actual value [%d] on global out of limit [%d] \n",currentValue,limitValue);
    UTF_VERIFY_EQUAL0_STRING_MAC(firstTime ? 89 : 125,
        currentValue,
        "number of entries");

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[0];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on fid[%d] \n",currentValue,limitValue,limitInfo.limitIndex.fid);
    /* not reach fid limit */
    UTF_VERIFY_EQUAL1_STRING_MAC(firstTime ? 55 : 91,
        currentValue,
        "actual number on fid[%d]",limitInfo.limitIndex.fid);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
    limitInfo.limitIndex.fid = _v[1];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on fid[%d] \n",currentValue,limitValue,limitInfo.limitIndex.fid);
    UTF_VERIFY_EQUAL1_STRING_MAC(firstTime ? 89-55 : 125-91,/* global-fid2 limit */
        currentValue,
        "actual number on fid[%d]",limitInfo.limitIndex.fid);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[0];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,/* entries of the port are in the eport */
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[1];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,/* entries of the port are in the eport */
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
    limitInfo.limitIndex.devPort.portNum = _p[2];
    limitInfo.limitIndex.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on port[%d] \n",currentValue,limitValue,limitInfo.limitIndex.devPort.portNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(0,/* entries of the port are in the eport */
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.devPort.portNum);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E;
    limitInfo.limitIndex.globalEport = _ep[0];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on global eport[%d] \n",currentValue,limitValue,limitInfo.limitIndex.globalEport);
    /*hit the eport limit */
    UTF_VERIFY_EQUAL1_STRING_MAC(55,
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.globalEport);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E;
    limitInfo.limitIndex.globalEport = _ep[1];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on global eport[%d] \n",currentValue,limitValue,limitInfo.limitIndex.globalEport);
    /*hit the eport limit */
    UTF_VERIFY_EQUAL1_STRING_MAC(89-55,/*v[0]-ep[0]*/
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.globalEport);

    limitInfo.limitType     = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E;
    limitInfo.limitIndex.globalEport = _ep[2];

    rc = cpssDxChBrgFdbManagerLimitGet(fdbManagerId,&limitInfo,&limitValue,&currentValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerLimitGet: expected to GT_OK on manager [%d]",
            fdbManagerId);

    PRV_UTF_LOG3_MAC("actual value [%d] out of limit [%d] on global eport[%d] \n",currentValue,limitValue,limitInfo.limitIndex.globalEport);
    UTF_VERIFY_EQUAL1_STRING_MAC(firstTime ? 0 : 36,/* all those on _v[1]*/
        currentValue,
        "actual number on port[%d]",limitInfo.limitIndex.globalEport);

}

/**
* @internal basicLimitDynamicUcLearn_part9_noGlobalEportLimit function
* @endinternal
*
* @brief  part 9 of the test : check global eports learning - with global eport limits
*/
static GT_VOID basicLimitDynamicUcLearn_part9_with_GlobalEportLimit
(
    IN GT_U32       fdbManagerId,
    IN GT_BOOL      firstTime
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC    counters;
    GT_U32  learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E];
    GT_U32  addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E];
    GT_U32  _p[3]; /* physical ports */
    GT_U32  _ep[3];/* global  eports */

    _p[0] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_1_CNS];
    _p[1] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_2_CNS];
    _p[2] = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_3_CNS];

    /* global eport from the start  of range */
    _ep[0] = PRV_TGF_GLOBAL_EPORT_MIN_CNS + 1;
    /* global eport from the middle of range */
    _ep[1] = (PRV_TGF_GLOBAL_EPORT_MAX_CNS + PRV_TGF_GLOBAL_EPORT_MIN_CNS)/2;
    /* global eport from the end    of range */
    _ep[2] = PRV_TGF_GLOBAL_EPORT_MAX_CNS - 1;

    if(firstTime ==GT_TRUE)
    {
        /* bind the global eport to the port and vise versa */
        basicLimitDynamicUcLearn_part9_Bind_Eport_to_Physical(_ep[0],_p[1] /* SWAP the ports !!! */);
        basicLimitDynamicUcLearn_part9_Bind_Eport_to_Physical(_ep[1],_p[2] /* SWAP the ports !!! */);
        basicLimitDynamicUcLearn_part9_Bind_Eport_to_Physical(_ep[2],_p[0] /* SWAP the ports !!! */);

        /* set global eports limits */
        basicLimitDynamicUcLearn_part9_setGlobalEportsLimits(fdbManagerId);
    }

    /* send the traffic */
    basicLimitDynamicUcLearn_common_traffic(0/*start index*/);

    /* check limit values */
    basicLimitDynamicUcLearn_part9_checkLimits_with_GlobalEportLimit(fdbManagerId,firstTime);

    PRV_UTF_LOG0_MAC("Check counters and statistics \n");
    /* Counter should be valid */
    cpssOsMemSet(&counters, 0, sizeof(counters));
    counters.macUnicastDynamic      = firstTime ? 89 : 125;
    counters.macUnicastStatic       = 0;
    counters.usedEntries            = counters.macUnicastDynamic;
    counters.freeEntries            = PRV_TGF_MAX_TOTAL_ENTRIES - counters.usedEntries;
    prvCpssDxChBrgFdbManagerCounterVerify(fdbManagerId, &counters);

    LOG_PARAM(counters.usedEntries);

    /* Verify Statistics counter */
    cpssOsMemSet(learningScanStatisticsArr, 0, sizeof(learningScanStatisticsArr));
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]           = firstTime ? 280 : 191;
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E]   = GT_NA;/* indication that 'OK'+'OK_NO_MORE' checked together */
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E] = firstTime ? 180 : 0;
    learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E] =
        learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E]-
        learningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E];

    cpssOsMemSet(addEntryStatisticsArr, 0, sizeof(addEntryStatisticsArr));
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_PORT_LIMIT_E        ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TRUNK_LIMIT_E       ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E] = firstTime ? 94 : 155;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_FID_LIMIT_E         ] = 0;
    addEntryStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_LIMIT_E      ] = 0;

    prvTgfBrgFdbManagerLearningScan_statistics_verify(fdbManagerId, learningScanStatisticsArr,addEntryStatisticsArr);

    /* Clear Statistics */
    rc = cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgFdbManagerStatisticsClear: expected to GT_OK on manager [%d]",
            fdbManagerId);

    if(firstTime ==GT_FALSE)
    {
        /* restore */
        basicLimitDynamicUcLearn_part9_restore_Eport_and_Physical(_ep[0],_p[0]);
        basicLimitDynamicUcLearn_part9_restore_Eport_and_Physical(_ep[1],_p[1]);
        basicLimitDynamicUcLearn_part9_restore_Eport_and_Physical(_ep[2],_p[2]);
    }
}

/**
* @internal basicLimitDynamicUcLearn_part9 function
* @endinternal
*
* @brief  part 9 of the test : check global eports learning and limits
*/
static GT_VOID basicLimitDynamicUcLearn_part9
(
    IN GT_U32       fdbManagerId
)
{
    PRV_UTF_LOG0_MAC("part 9 of the test : (sub-part 1) check global eports learning \n");
    basicLimitDynamicUcLearn_part9_noGlobalEportLimit(fdbManagerId);
    /* print the fdb manager content */
    PRINT_FDB   (fdbManagerId);

    PRV_UTF_LOG0_MAC("part 9 of the test : (sub-part 2) check global eports learning with global eports limits \n");
    basicLimitDynamicUcLearn_part9_with_GlobalEportLimit(fdbManagerId,GT_TRUE);
    /* print the fdb manager content */
    PRINT_FDB   (fdbManagerId);

    PRV_UTF_LOG0_MAC("part 9 of the test : (sub-part 3) resend same traffic \n");
    basicLimitDynamicUcLearn_part9_with_GlobalEportLimit(fdbManagerId,GT_FALSE);
}


/**
* @internal prvTgfBrgFdbManagerLearningScan_statistics_config function
* @endinternal
*
* @brief  test the limits on learning of dynamic UC MAC entries.
*       according to port/trunk/vlan/global limits
*/
GT_VOID prvTgfBrgFdbManagerBasicLimitDynamicUcLearn
(
    IN GT_U32       fdbManagerId
)
{
    GT_STATUS                                   rc;
    LEARN_LIMITS_STC        learnLimitsInfo;

    LOG_DELIMITER_MAC;
    PRV_UTF_LOG0_MAC("Create FDB manager with limits on dynamic uc learning \n");
    PRV_UTF_LOG0_MAC("test will use 2 vlans (fids) \n");
    PRV_UTF_LOG0_MAC("test will use 3 ports in those 2 vlans \n");
    learnLimitsInfo.enableLearningLimits        = GT_TRUE;
    learnLimitsInfo.maxDynamicUcMacGlobalLimit  = 200;
    learnLimitsInfo.maxDynamicUcMacFidLimit     = 150;
    learnLimitsInfo.maxDynamicUcMacGlobalEportLimit = GT_NA;
    learnLimitsInfo.maxDynamicUcMacTrunkLimit   = 120;
    learnLimitsInfo.maxDynamicUcMacPortLimit    = 100;

    FDB_SIZE_FULL_INIT();
    /* set limit values to be used by createFdbManagerWithDevice that called from
       createFdbManagerWithAppDemoSupport*/
    setLearnLimits(&learnLimitsInfo);

    if(SHADOW_TYPE_PER_DEVICE() != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)
    {
        CPSS_DXCH_CFG_GLOBAL_EPORT_STC  globalEportInfo,l2EcmpInfo,l2DlbInfo;

        rc = cpssDxChCfgGlobalEportGet(prvTgfDevNum,&globalEportInfo,&l2EcmpInfo,&l2DlbInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgGlobalEportGet: ");

        globalEportInfo.enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E;

        globalEportInfo.minValue = PRV_TGF_GLOBAL_EPORT_MIN_CNS;
        globalEportInfo.maxValue = PRV_TGF_GLOBAL_EPORT_MAX_CNS;

        rc = cpssDxChCfgGlobalEportSet(prvTgfDevNum,&globalEportInfo,&l2EcmpInfo,&l2DlbInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChCfgGlobalEportSet: min %d, max %d",
                globalEportInfo.minValue,globalEportInfo.maxValue);
    }

    /* create manager , add device to it and add appDemo support for learning/aging */
    createFdbManagerWithAppDemoSupport(fdbManagerId,0/*no aging*/);

    /* add ports to vlan 2 */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
            prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* add ports to vlan 5 */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_5_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
            prvTgfDevNum, PRV_TGF_VLANID_5_CNS);

    LOG_DELIMITER_MAC;
    PRV_UTF_LOG0_MAC("part 1 of the test : test with default of 'learnLimitsInfo' \n");
    /* part 1 of the test : test with default of 'learnLimitsInfo' */
    basicLimitDynamicUcLearn_part1(fdbManagerId);
    /* print the fdb manager content */
    PRINT_FDB   (fdbManagerId);

    LOG_DELIMITER_MAC;
    PRV_UTF_LOG0_MAC("part 2 of the test : test with global,fid,port different limits of 'learnLimitsInfo' \n");
    /* part 2 of the test : test with global,fid,port different limits of 'learnLimitsInfo' */
    basicLimitDynamicUcLearn_part2(fdbManagerId);
    /* print the fdb manager content */
    PRINT_FDB   (fdbManagerId);

    LOG_DELIMITER_MAC;
    PRV_UTF_LOG0_MAC("part 3 of the test : delete over the top entries \n");
    /* part 3 of the test : delete over the top entries */
    basicLimitDynamicUcLearn_part3(fdbManagerId);
    /* print the fdb manager content */
    PRINT_FDB   (fdbManagerId);

    LOG_DELIMITER_MAC;
    PRV_UTF_LOG0_MAC("part 4 of the test : define trunk and check for it limits \n");
    /* part 4 of the test : define trunk and check for it limits */
    basicLimitDynamicUcLearn_part4(fdbManagerId);
    /* print the fdb manager content */
    PRINT_FDB   (fdbManagerId);

    LOG_DELIMITER_MAC;
    PRV_UTF_LOG0_MAC("part 5 of the test : remove port from trunk and check limits \n");
    basicLimitDynamicUcLearn_part5(fdbManagerId);
    /* print the fdb manager content */
    PRINT_FDB   (fdbManagerId);

    LOG_DELIMITER_MAC;
    PRV_UTF_LOG0_MAC("part 6 of the test : enlage port limits and hit vlan limits and global \n");
    basicLimitDynamicUcLearn_part6(fdbManagerId);
    /* print the fdb manager content */
    PRINT_FDB   (fdbManagerId);

    LOG_DELIMITER_MAC;
    PRV_UTF_LOG0_MAC("part 7 of the test : lower port , trunk limits and check SINGLE aging scan not delete over the top \n");
    basicLimitDynamicUcLearn_part7(fdbManagerId);
    /* print the fdb manager content */
    PRINT_FDB   (fdbManagerId);

    LOG_DELIMITER_MAC;
    PRV_UTF_LOG0_MAC("part 8 of the test : check transplant scan to remove over the top entries \n");
    basicLimitDynamicUcLearn_part8(fdbManagerId);
    /* print the fdb manager content */
    PRINT_FDB   (fdbManagerId);

    if(SHADOW_TYPE_PER_DEVICE() != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)
    {
        LOG_DELIMITER_MAC;
        PRV_UTF_LOG0_MAC("part 9 of the test : check global eports learning and limits \n");
        basicLimitDynamicUcLearn_part9(fdbManagerId);
        /* print the fdb manager content */
        PRINT_FDB   (fdbManagerId);
    }

    LOG_DELIMITER_MAC;
    PRV_UTF_LOG0_MAC("test ended and destroy the fdb manager \n");

    if(SHADOW_TYPE_PER_DEVICE() != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)
    {
        CPSS_DXCH_CFG_GLOBAL_EPORT_STC  globalEportInfo,l2EcmpInfo,l2DlbInfo;

        rc = cpssDxChCfgGlobalEportGet(prvTgfDevNum,&globalEportInfo,&l2EcmpInfo,&l2DlbInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgGlobalEportGet: ");

        globalEportInfo.enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;

        rc = cpssDxChCfgGlobalEportSet(prvTgfDevNum,&globalEportInfo,&l2EcmpInfo,&l2DlbInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChCfgGlobalEportSet: min %d, max %d",
                globalEportInfo.minValue,globalEportInfo.maxValue);
    }

    restoreLearnLimits();

    CLEANUP_MANAGER(fdbManagerId);
}

/**
* @internal prvTgfBrgFdbManager_hybridShadowTest function
* @endinternal
*
* @brief  test Hybrid shadow type.
*/
GT_VOID prvTgfBrgFdbManager_hybridShadowTest()
{
    GT_U32                  fdbManagerId = 10;
    GT_STATUS               rc;
    userDefinedShadowType   = GT_TRUE;
    globalShadowType        = CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E;

    rc = prvCpssDxChBrgFdbSizeSet(prvTgfDevNum, FDB_SIZE_FULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDxChBrgFdbSizeSet: expected to GT_OK on manager [%d]");

    /* Test prvTgfBrgFdbManagerAddEntry */
    prvTgfBrgFdbManagerAddEntry_config(fdbManagerId);
    prvTgfBrgFdbManagerAddEntry_traffic_and_verify(fdbManagerId);
    prvTgfBrgFdbManagerAddEntry_restore(fdbManagerId);

    /* Test prvTgfBrgFdbManagerLearningScan */
    prvTgfBrgFdbManagerLearningScan_statistics_config(fdbManagerId);
    prvTgfBrgFdbManagerLearningScan_statistics_traffic_and_verify(fdbManagerId,
            GT_TRUE/* Reduced Run */);
    prvTgfBrgFdbManagerLearningScan_statistics_restore(fdbManagerId);

    prvTgfBrgFdbManager_restoreFdbSize();

    userDefinedShadowType   = GT_FALSE;
    globalShadowType        = CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E;
    prvUtfSkipTestsFlagReset();
}
