#include <cpssCommon/cpssPresteraDefs.h>
#include <cpssCommon/private/prvCpssMath.h>

#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcamUT.h>
#include <virtualTcam/prvTgfVirtualTcam.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTcamGen.h>
#include <common/tgfCosGen.h>
#include <common/tgfCncGen.h>
#include <common/tgfVntGen.h>
#include <cnc/prvTgfCncGen.h>
#include <cnc/prvTgfCncVlanL2L3.h>
#include <port/prvTgfPortFWS.h>
#include <pcl/prvTgfPclQuadLookup.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define NUM_60_RULES_PER_BLOCK 256

static GT_BOOL ignore_write_ret = GT_FALSE;
static CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC vtcamMngCfgParam = {GT_TRUE, CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_REPLAY_FAILED_RULES_ADD_E};

/* PCL IDs - use different MSBs for port list testing */
#define PRV_TGF_IPCL_PCL_ID_0_0_CNS  0x000
#define PRV_TGF_IPCL_PCL_ID_1_CNS    0x204

static VT_SEQUENCE draggCfgTest_sequence[] =
{
    {VT_CREATE,  100, CG_PCL,  RS_60, 1002, GT_OK, NULL},
    {VT_CREATE,    1, CG_PCL,  RS_40,  300, GT_OK, NULL},
    {VT_CREATE,    2, CG_PCL,  RS_50,   60, GT_OK, NULL},
    {VT_CREATE,    3, CG_PCL,  RS_40,   30, GT_OK, NULL},
    {VT_CREATE,    4, CG_PCL,  RS_50,  122, GT_OK, NULL},
    {VT_CREATE,   10, CG_PCL,  RS_20,  330, GT_OK, NULL},
    {VT_CREATE,   11, CG_PCL,  RS_10,  180, GT_OK, NULL},
    {VT_REMOVE,    1, CG_NONE, RS_NONE,  0, GT_OK, NULL},
    {VT_REMOVE,    2, CG_NONE, RS_NONE,  0, GT_OK, NULL},
    {VT_REMOVE,    3, CG_NONE, RS_NONE,  0, GT_OK, NULL},
    {VT_REMOVE,    4, CG_NONE, RS_NONE,  0, GT_OK, NULL}
};

static VT_SEQUENCE draggRestoreTest_sequence[] =
{
    {VT_REMOVE,  100, CG_NONE,  RS_NONE,  0, GT_OK, NULL},
    {VT_REMOVE,   10, CG_NONE,  RS_NONE,  0, GT_OK, NULL},
    {VT_REMOVE,   11, CG_NONE,  RS_NONE,  0, GT_OK, NULL},
    {VT_REMOVE,   20, CG_NONE,  RS_NONE,  0, GT_OK, NULL}
};

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS          1968

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;
static GT_U32  prvTgfPclFwsAddSize = 0;

/******************************* Test packets **********************************/

/*  L2 part of packet  */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x10, 0x12, 0x14, 0x15, 0x16},                /* daMac */
    {0x00, 0x11, 0x22, 0x44, 0x55, 0x66}                 /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    5, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};


/* DATA of bypass packet */
static GT_U8 prvTgfPayloadDataArr[48] ={0x01, 0x02, 0x03, 0x04, 0x05, 0x06};


/* Bypass PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

/*  PACKET to send info */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_CRC_LEN_CNS,                                          /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/***************************    packet for IPv6    *******************/

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};
/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    19,                 /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x2222, 0, 0, 0, 0, 0, 0x0000, 0x2222}, /* TGF_IPV6_ADDR srcAddr */
    {0xff00, 0, 0, 0, 0, 0, 0x0000, 0x2222}  /* TGF_IPV6_ADDR dstAddr */
};

/* PARTS of IPv6 packet */
static TGF_PACKET_PART_STC prvTgfIpv6PacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of IPv6 packet */
#define PRV_TGF_IPV6_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* IPv6 PACKET1 to send */
static TGF_PACKET_STC prvTgfIpv6PacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfIpv6PacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfIpv6PacketPartArray                                        /* partsArray */
};

/***************************    packet for FWS    *******************/

/* L2 part of packet FWS */
static TGF_PACKET_L2_STC prvTgfPacketFWSL2Part = {
    {0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0x11},               /* dstMac */
    {0x00, 0x88, 0x99, 0x33, 0x44, 0x11}                /* srcMac */
};

/* DATA of FWS packet */
static GT_U8 prvTgfPayloadDataFWSArr[] = {
    0x12, 0x34, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
    0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
    0xde, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadFWSPart = {
    sizeof(prvTgfPayloadDataFWSArr),                       /* dataLength */
    prvTgfPayloadDataFWSArr                                /* dataPtr */
};

/* PARTS of packet FWS */
static TGF_PACKET_PART_STC prvTgfPacketFWSPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketFWSL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadFWSPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_FWS_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataFWSArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_FWS_CRC_LEN_CNS  PRV_TGF_PACKET_FWS_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET FWS to send */
static TGF_PACKET_STC prvTgfPacketFWSInfo = {
    PRV_TGF_PACKET_FWS_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketFWSPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketFWSPartArray                                        /* partsArray */
};

/* maximal size of rule in words - 80 Byte / 4 */
#define UTF_TCAM_MAX_RULE_SIZE_CNS 20

/******************************************************************************\
 *                   Private variables & definitions                          *
\******************************************************************************/

/* traffic generation sending port */
#define ING_PORT_IDX_CNS    0

/* target port */
#define EGR_PORT_A    1

#define EGR_PORT_B    2

/* TCAM floor info saved for restore */
static PRV_TGF_TCAM_BLOCK_INFO_STC saveFloorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_FLOORS_CNS][PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS];

#define QOS_PROFILE_INDEX_CNS   3

/* PCL TCAM group binding saved for restore */
static GT_U32 tcamGroup;
static GT_BOOL tcamGroupEnable;
static PRV_TGF_CNC_COUNTER_FORMAT_ENT prvTgfPclFwsCncFormat;

/* TTI TCAM group binding saved for restore */
static GT_U32 ttiTcamGroup;
static GT_BOOL ttiTcamGroupEnable;
static GT_BOOL ttiTcamLookupEnable;
static PRV_TGF_TTI_MAC_MODE_ENT ttiTcamMacMode;

/* enable debug printing to log */
extern GT_U32 prvTgfPclFwsDebug;
extern GT_U32 prvTgfPclFwsLookupDebug;
extern GT_U32 prvTgfPclFwsCncBlockBmpDebug;

static CPSS_DXCH_VIRTUAL_TCAM_HA_POST_CREATE_MGR_FUNC *prvTgfVirtualTcamPostCreateMgrFuncPtr = NULL;
static GT_U32 prvTgfVirtualTcamPostCreateMgrFunBound = 0;

GT_VOID prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_BOOL toBind)
{
    if (toBind ==  GT_FALSE)
    {
        prvTgfVirtualTcamPostCreateMgrFuncPtr  = prvTgfVirtualTcamHaHwDbAccessUnbind;
        prvTgfVirtualTcamPostCreateMgrFunBound = 0;
    }
    else
    {
        prvTgfVirtualTcamPostCreateMgrFuncPtr  = prvTgfVirtualTcamHaHwDbLpmAccessBind;
        prvTgfVirtualTcamPostCreateMgrFunBound = 1;
    }
}

extern GT_STATUS vtcamHaAppWithCb
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           createMngr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_HA_POST_CREATE_MGR_FUNC *postCreateMgrFuncPtr,
    IN  GT_U8                           createVtcam,
    IN  GT_U32                          vtcamId,
    IN  GT_U32                          ruleSize,
    IN  GT_U32                          vtcamSize,
    IN  GT_U32                          numRules,
    IN  GT_U8                           vtcamIdOffset,
    IN  GT_U32                          ruleIdOffset,
    IN  GT_U8                           vtcamMode,
    IN  GT_U32                          priority
);

extern GT_STATUS vtcamHaAppValidityWithCb
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           createMngr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_HA_POST_CREATE_MGR_FUNC *postCreateMgrFuncPtr,
    IN  GT_U8                           createVtcam,
    IN  GT_U32                          vtcamId,
    IN  GT_U32                          ruleSize,
    IN  GT_U32                          vtcamSize,
    IN  GT_U32                          numRules,
    IN  GT_U32                          ruleIdOffset,
    IN  GT_U8                           vtcamMode,
    IN  GT_U32                          priority,
    IN  GT_BOOL                         valid
);

GT_STATUS vtcamHaAppValidity
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           createMngr,
    IN  GT_U8                           createVtcam,
    IN  GT_U32                          vtcamId,
    IN  GT_U32                          ruleSize,
    IN  GT_U32                          vtcamSize,
    IN  GT_U32                          numRules,
    IN  GT_U32                          ruleIdOffset,
    IN  GT_U8                           vtcamMode,
    IN  GT_U32                          priority,
    IN  GT_U32                          valid
)
{
    return vtcamHaAppValidityWithCb(
        devNum, createMngr,
        prvTgfVirtualTcamPostCreateMgrFuncPtr,
        createVtcam, vtcamId, ruleSize, vtcamSize,
        numRules, ruleIdOffset, vtcamMode, priority, valid);
}

GT_STATUS vtcamHaApp
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           createMngr,
    IN  GT_U8                           createVtcam,
    IN  GT_U32                          vtcamId,
    IN  GT_U32                          ruleSize,
    IN  GT_U32                          vtcamSize,
    IN  GT_U32                          numRules,
    IN  GT_U8                           vtcamIdOffset,
    IN  GT_U32                          ruleIdOffset,
    IN  GT_U8                           vtcamMode,
    IN  GT_U32                          priority
)
{
    return vtcamHaAppWithCb(
        devNum, createMngr,
        prvTgfVirtualTcamPostCreateMgrFuncPtr,
        createVtcam, vtcamId, ruleSize, vtcamSize,
        numRules, vtcamIdOffset, ruleIdOffset, vtcamMode, priority);
}

extern GT_STATUS prvCpssDxChVirtualTcamHaSystemRecoverySet
(
    IN GT_U8                                devNum,
    IN GT_U32                               vTcamMngId,
    IN CPSS_SYSTEM_RECOVERY_STATE_ENT       systemRecoveryState,
    IN CPSS_SYSTEM_RECOVERY_PROCESS_ENT     systemRecoveryProcess,
    IN GT_U32                               tcamReset
);

/* vTCAMs numbers*/
#define V_TCAM_1 0
#define GUARANTEED_NUM_OF_RULES_PER_V_TCAM 100

/* default vTCAM manager */
#define V_TCAM_MANAGER 1

/* second vTCAM manager */
#define V_TCAM_MANAGER2 2

/* PCL TCAM group binding */
#define PCL_TCAM_GROUP_CNS  (HARD_WIRE_TCAM_MAC(prvTgfDevNum) ? 1U : 0U)

/* TTI TCAM group binding */
#define TTI_TCAM_GROUP_CNS  (HARD_WIRE_TCAM_MAC(prvTgfDevNum) ? 0 : \
                             (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.tcam.sip6maxTcamGroupId == 1) ? 0 : \
                            1)

#define PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_MAC(delay) cpssOsTimerWkAfter(delay)
#ifndef  ASIC_SIMULATION
#define PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_ON_SIMULATION_MAC(delay)
#else
#define PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_ON_SIMULATION_MAC(delay) cpssOsTimerWkAfter(delay)
#endif

/* number of test iterations */
#define PRV_TGF_VIRTUAL_TCAM_TEST_NUM_ITERATIONS 512
/* step to reduce number of test iterations on simulation */
#ifndef  ASIC_SIMULATION
#define PRV_TGF_VIRTUAL_TCAM_TEST_ITERATIONS_STEP 1
#else
#define PRV_TGF_VIRTUAL_TCAM_TEST_ITERATIONS_STEP 8
#endif

/* default TC */
#define PRV_TGF_CNC_GEN_DEF_TC_CNS            5

/* default DP */
#define PRV_TGF_CNC_GEN_DEF_DP_CNS            CPSS_DP_YELLOW_E

/* used DxCh Qos profile */
#define PRV_TGF_CNC_GEN_DXCH_TEST_QOS_PROFILE0 0

/* block number */
#define PRV_TGF_CNC_GEN_BLOCK_NUM_CNS  1

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfTcamRuleRangeWrite function
* @endinternal
*
* @brief   Write range of TCAM rules
*
* @param[in] devNum                - device number
* @param[in] ruleSize              - rule size
* @param[in] valid                 - GT_TRUE - valid, GT_FALSE - invalid
* @param[in] zeros                 - GT_TRUE - pattern and mask are zeros, GT_FALSE - mask 0xFF, pattern - stamp
* @param[in] startRuleIndex        - start rule index
* @param[in] ruleIndexIncrement    - rule index increment, for 80B - 12, 60B, 50B, 40B - 6, 30B - 3, 20B - 2, 10B - 1
* @param[in] rulesAmount           - rules amount
*
*/
GT_STATUS prvTgfTcamRuleRangeWrite
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_BOOL                             valid,
    IN  GT_BOOL                             zeros,
    IN  GT_U32                              startRuleIndex,
    IN  GT_U32                              ruleIndexIncrement,
    IN  GT_U32                              rulesAmount
)
{
    GT_STATUS rc;

    GT_U32   ruleIndex;
    GT_U32   ruleIndexBound;
    GT_U32   tcamPattern[24];
    GT_U32   tcamMask[24];

    if (zeros == GT_FALSE)
    {
        cpssOsMemSet(tcamMask,    0xFF, sizeof(tcamMask));
        cpssOsMemSet(tcamPattern, 0x11, sizeof(tcamPattern));
    }
    else
    {
        cpssOsMemSet(tcamMask, 0, sizeof(tcamMask));
        cpssOsMemSet(tcamPattern, 0, sizeof(tcamPattern));
    }

    ruleIndexBound = startRuleIndex + (rulesAmount * ruleIndexIncrement);
    for (ruleIndex = startRuleIndex; (ruleIndex < ruleIndexBound); ruleIndex += ruleIndexIncrement)
    {
        if (zeros == GT_FALSE)
        {
            tcamPattern[0] = ruleIndex;
            tcamPattern[1] = ruleSize;
        }
        rc = cpssDxChTcamPortGroupRuleWrite(
            devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            ruleIndex, valid, ruleSize,
            tcamPattern, tcamMask);
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvTgfTcamRuleRangeWrite - Failed\n");
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvTgfTcamRuleRangeInvalidate function
* @endinternal
*
* @brief   Invalidate range of rules
*
* @param[in] devNum                - device number
* @param[in] startRuleIndex        - start rule index
* @param[in] ruleIndexIncrement    - rule index increment, for 80B - 12, 60B, 50B, 40B - 6, 30B - 3, 20B - 2, 10B - 1
* @param[in] rulesAmount           - rules amount
*
*/
GT_VOID prvTgfTcamRuleRangeInvalidate
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              startRuleIndex,
    IN  GT_U32                              ruleIndexIncrement,
    IN  GT_U32                              rulesAmount
)
{
    GT_U32   ruleIndex;
    GT_U32   ruleIndexBound;

    ruleIndexBound = startRuleIndex + (rulesAmount * ruleIndexIncrement);
    for (ruleIndex = startRuleIndex; (ruleIndex < ruleIndexBound); ruleIndex += ruleIndexIncrement)
    {
        cpssDxChTcamPortGroupRuleValidStatusSet(
            devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, ruleIndex, GT_FALSE);
    }
}

/**
* @internal prvTgfTcamAllTcamClear function
* @endinternal
*
* @brief   Invalidate rall rules in TCAM.
*
* @param[in] devNum                - device number
*
*/
GT_VOID prvTgfTcamAllTcamClear
(
    IN  GT_U8      devNum
)
{
    GT_U32   rulesAmount;

    rulesAmount = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm;

    prvTgfTcamRuleRangeWrite(
        devNum, CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
        GT_FALSE /*valid*/, GT_TRUE /*zeros*/,
        0 /*startRuleIndex*/, 6/*ruleIndexIncrement*/, (rulesAmount / 6));
}

/**
* @internal prvTgfConvertGenericToDxChRuleIngressUdbOnly function
* @endinternal
*
* @brief   Convert generic into device specific ingress UDB Only key
*
* @param[in] genRuleIngrUdbOnlyPtr    - (pointer to) generic ingress UDB Only key
*
* @param[out] dxChRuleIngrUdbOnlyPtr   - (pointer to) DxCh ingress UDB Only key
*                                       None.
*/
static GT_VOID prvTgfConvertGenericToDxChRuleIngressUdbOnly
(
    IN  PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC    *genRuleIngrUdbOnlyPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC  *dxChRuleIngrUdbOnlyPtr
)
{
    GT_U32 i;

    for (i = 0; (i < PRV_TGF_PCL_UDB_MAX_NUMBER_MAC(prvTgfDevNum)); i++)
    {
        dxChRuleIngrUdbOnlyPtr->udb[i] = genRuleIngrUdbOnlyPtr->udb[i];
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.isUdbValid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.srcPort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.srcDevIsOwn);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.macToMe);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.qosProfile);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, replacedFld.flowId);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.isUdbValid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.pclId);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.vid);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.srcPort);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.srcDevIsOwn);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.vid1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.up1);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.macToMe);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.qosProfile);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(
        dxChRuleIngrUdbOnlyPtr, genRuleIngrUdbOnlyPtr, udb60FixedFld.flowId);
}

/**
* @internal prvTgfVirtualTcamTestVlanInit function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
static GT_VOID prvTgfVirtualTcamTestVlanInit
(
    IN GT_U16           vlanId
)
{
    GT_U32                      portIter  = 0;
    GT_U32                      portCount = 0;
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* set vlan entry */
    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IgmpToCpuEn      = GT_FALSE;
    vlanInfo.mirrToRxAnalyzerEn   = GT_FALSE;
    vlanInfo.ipv6IcmpToCpuEn      = GT_FALSE;
    vlanInfo.ipCtrlToCpuEn        = PRV_TGF_BRG_IP_CTRL_NONE_E;
    vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv4IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.portIsolationMode    = PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;
    vlanInfo.fidValue              = vlanId;

    /* set vlan entry */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    portCount = prvTgfPortsNum;
    for (portIter = 0; portIter < portCount; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter]);

        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E;

        /* reset counters and set force link up */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);
}

/**
* @internal prvTgfVirtualTcamPclBuildRule function
* @endinternal
*
* @brief   Build maks and pattern for PCL rule to be tested
*
* @param[in] ruleSize                 - Rule size to be tested (currently only 30 and 80 bytes supported)
*
* @param[out] maskPtr                  - (Pointer to) rule mask.
* @param[out] patternPtr               - (Pointer to) rule pattern.
*                                       None
*/
static GT_VOID prvTgfVirtualTcamPclBuildRule
(
    IN CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT     ruleSize,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT           *maskPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT           *patternPtr
)
{
    GT_U16                                      ii;

    /* AUTODOC: mask for DST MAC address */
    cpssOsMemSet(maskPtr, 0, sizeof(*maskPtr));
    if (ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
    {
        maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.macToMe = 0x1;
        maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.isTagged = 0x1;
        maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.vid = 0xFFF;
        maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.up = 0x7;
        maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.tos = 0xFF;
        maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.ipProtocol = 0xFF;
        maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.isL4Valid = 0x1;
        maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte0 = 0xFF;
        maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte1 = 0xFF;
        maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte2 = 0xFF;
        maskPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte3 = 0xFF;
        maskPtr->ruleUltraIpv6PortVlanQos.isL2Valid = 0x1;
        maskPtr->ruleUltraIpv6PortVlanQos.isNd = 0x1;
        maskPtr->ruleUltraIpv6PortVlanQos.isBc = 0x1;
        maskPtr->ruleUltraIpv6PortVlanQos.isIp = 0x1;
        maskPtr->ruleUltraIpv6PortVlanQos.l2Encap = 0x1;
        maskPtr->ruleUltraIpv6PortVlanQos.etherType = 0xFFFF;
        cpssOsMemSet(maskPtr->ruleUltraIpv6PortVlanQos.macDa.arEther, 0xFF,
                     sizeof(maskPtr->ruleUltraIpv6PortVlanQos.macDa.arEther));
        cpssOsMemSet(maskPtr->ruleUltraIpv6PortVlanQos.macSa.arEther, 0xFF,
                     sizeof(maskPtr->ruleUltraIpv6PortVlanQos.macSa.arEther));
        cpssOsMemSet(maskPtr->ruleUltraIpv6PortVlanQos.sip.arIP, 0xFF,
                     sizeof(maskPtr->ruleUltraIpv6PortVlanQos.sip.arIP));
        cpssOsMemSet(maskPtr->ruleUltraIpv6PortVlanQos.dip.arIP, 0xFF,
                     sizeof(maskPtr->ruleUltraIpv6PortVlanQos.dip.arIP));
        maskPtr->ruleUltraIpv6PortVlanQos.isIpv6ExtHdrExist = 0x1;
        maskPtr->ruleUltraIpv6PortVlanQos.isIpv6HopByHop = 0x1;
        maskPtr->ruleUltraIpv6PortVlanQos.ipHeaderOk = 0x1;
        maskPtr->ruleUltraIpv6PortVlanQos.isUdbValid = 0x1;
    }
    else
    {
        maskPtr->ruleStdNotIp.common.macToMe = 0x1;
        maskPtr->ruleStdNotIp.common.isTagged = 0x1;
        maskPtr->ruleStdNotIp.common.vid = 0xFFF;
        maskPtr->ruleStdNotIp.common.up = 0x7;
        maskPtr->ruleStdNotIp.common.isIp = 0x1;
        maskPtr->ruleStdNotIp.common.isL2Valid = 0x1;
        maskPtr->ruleStdNotIp.common.isUdbValid = 0x1;
        maskPtr->ruleStdNotIp.isIpv4 = 0x1;
        maskPtr->ruleStdNotIp.isArp = 0x1;
        maskPtr->ruleStdNotIp.l2Encap = 0x1;
        cpssOsMemSet(maskPtr->ruleStdNotIp.macDa.arEther, 0xFF,
                     sizeof(maskPtr->ruleStdNotIp.macDa.arEther));
        cpssOsMemSet(maskPtr->ruleStdNotIp.macSa.arEther, 0xFF,
                     sizeof(maskPtr->ruleStdNotIp.macSa.arEther));
    }

    /* AUTODOC: pattern for DST MAC address */
    cpssOsMemSet(patternPtr, 0, sizeof(*patternPtr));
    if (ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
    {
        patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.macToMe = 0;
        patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.isTagged = 1;
        patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.vid = prvTgfPacketVlanTagPart.vid;
        patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.up = prvTgfPacketVlanTagPart.pri;
        patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.tos = prvTgfPacketIpv6Part.trafficClass;
        patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.ipProtocol = prvTgfPacketIpv6Part.nextHeader;
        patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.isL4Valid = 1;
        patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte0 = prvTgfPayloadDataArr[0];
        patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte1 = prvTgfPayloadDataArr[1];
        patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte2 = prvTgfPayloadDataArr[2];
        patternPtr->ruleUltraIpv6PortVlanQos.ingressIpCommon.l4Byte3 = prvTgfPayloadDataArr[3];
        patternPtr->ruleUltraIpv6PortVlanQos.isL2Valid = 0x1;
        patternPtr->ruleUltraIpv6PortVlanQos.isNd = 0;
        patternPtr->ruleUltraIpv6PortVlanQos.isBc = 0;
        patternPtr->ruleUltraIpv6PortVlanQos.isIp = 1;
        patternPtr->ruleUltraIpv6PortVlanQos.l2Encap = 1;   /* Ethernet V2 */
        patternPtr->ruleUltraIpv6PortVlanQos.etherType = prvTgfPacketEtherTypePart.etherType;
        cpssOsMemCpy(patternPtr->ruleUltraIpv6PortVlanQos.macDa.arEther, prvTgfPacketL2Part.daMac,
                     sizeof(prvTgfPacketL2Part.daMac));
        cpssOsMemCpy(patternPtr->ruleUltraIpv6PortVlanQos.macSa.arEther, prvTgfPacketL2Part.saMac,
                     sizeof(prvTgfPacketL2Part.saMac));
        for (ii = 0; ii < sizeof(patternPtr->ruleUltraIpv6PortVlanQos.sip.arIP); ii += 2)
        {
            patternPtr->ruleUltraIpv6PortVlanQos.sip.arIP[ii+0] = (prvTgfPacketIpv6Part.srcAddr[ii/2] >>  8) & 0xFF;
            patternPtr->ruleUltraIpv6PortVlanQos.sip.arIP[ii+1] = (prvTgfPacketIpv6Part.srcAddr[ii/2] >>  0) & 0xFF;
            patternPtr->ruleUltraIpv6PortVlanQos.dip.arIP[ii+0] = (prvTgfPacketIpv6Part.dstAddr[ii/2] >>  8) & 0xFF;
            patternPtr->ruleUltraIpv6PortVlanQos.dip.arIP[ii+1] = (prvTgfPacketIpv6Part.dstAddr[ii/2] >>  0) & 0xFF;
        }
        patternPtr->ruleUltraIpv6PortVlanQos.isIpv6ExtHdrExist = 0;
        patternPtr->ruleUltraIpv6PortVlanQos.isIpv6HopByHop = 0;
        patternPtr->ruleUltraIpv6PortVlanQos.ipHeaderOk = 1;
        patternPtr->ruleUltraIpv6PortVlanQos.isUdbValid = 1;
    }
    else
    {
        patternPtr->ruleStdNotIp.common.macToMe = 0;
        patternPtr->ruleStdNotIp.common.isTagged = 1;
        patternPtr->ruleStdNotIp.common.vid = prvTgfPacketVlanTagPart.vid;
        patternPtr->ruleStdNotIp.common.up = prvTgfPacketVlanTagPart.pri;
        patternPtr->ruleStdNotIp.common.isIp = 0;
        patternPtr->ruleStdNotIp.common.isL2Valid = 1;
        patternPtr->ruleStdNotIp.common.isUdbValid = 1;
        patternPtr->ruleStdNotIp.isIpv4 = 0;
        patternPtr->ruleStdNotIp.isArp = 0;
        patternPtr->ruleStdNotIp.l2Encap = 0;
        cpssOsMemCpy(patternPtr->ruleStdNotIp.macDa.arEther, prvTgfPacketL2Part.daMac,
                     sizeof(prvTgfPacketL2Part.daMac));
        cpssOsMemCpy(patternPtr->ruleStdNotIp.macSa.arEther, prvTgfPacketL2Part.saMac,
                     sizeof(prvTgfPacketL2Part.saMac));
    }

}

/**
* @internal prvTgfVirtualTcamTtiBuildRule function
* @endinternal
*
* @brief   Build maks and pattern for TTI rule to be tested
*
* @param[in] ruleSize                 - Rule size to be tested (currently only 30 bytes supported)
*
* @param[out] maskPtr                  - (Pointer to) rule mask.
* @param[out] patternPtr               - (Pointer to) rule pattern.
*                                       None
*/
static GT_VOID prvTgfVirtualTcamTtiBuildRule
(
    IN CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT     ruleSize,
    OUT CPSS_DXCH_TTI_RULE_UNT                  *maskPtr,
    OUT CPSS_DXCH_TTI_RULE_UNT                  *patternPtr
)
{
    ruleSize = ruleSize;

    /* AUTODOC: mask for DST MAC address */
    cpssOsMemSet(maskPtr, 0, sizeof(*maskPtr));
    cpssOsMemSet(maskPtr->eth.common.mac.arEther, 0xFF,
                 sizeof(maskPtr->eth.common.mac.arEther));
    maskPtr->eth.common.srcIsTrunk = 0x1;
    maskPtr->eth.common.vid = 0xFFF;
    maskPtr->eth.common.isTagged = 0x1;
    maskPtr->eth.up0 = 0x7;
    maskPtr->eth.cfi0 = 0x1;
    maskPtr->eth.macToMe = 0x1;

    /* AUTODOC: pattern for DST MAC address */
    cpssOsMemSet(patternPtr, 0, sizeof(*patternPtr));
    cpssOsMemCpy(patternPtr->eth.common.mac.arEther, prvTgfPacketL2Part.daMac,
                 sizeof(prvTgfPacketL2Part.daMac));
    patternPtr->eth.common.srcIsTrunk = GT_FALSE;
    patternPtr->eth.common.vid = prvTgfPacketVlanTagPart.vid;
    patternPtr->eth.common.isTagged = GT_TRUE;
    patternPtr->eth.up0 = prvTgfPacketVlanTagPart.pri;
    patternPtr->eth.cfi0 = prvTgfPacketVlanTagPart.cfi;
    patternPtr->eth.macToMe = GT_FALSE;

}

/*add and write some STD_NOT_IP_E rule by ruleID and priority */
static void addRuleNoHitByIndexPriority
(
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize,
    GT_U16 ruleId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC *ruleAttributesPtr
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    PRV_TGF_PCL_ACTION_STC                     action;
    CPSS_DXCH_PCL_ACTION_STC                   dxChAction;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;
    TGF_MAC_ADDR                                prvTgfArpMac = {0x00, 0x00, 0x00,
                                                                0x00, 0x00, 0x22};
    /* AUTODOC: mask for DST MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    if (ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
    {
        cpssOsMemSet(mask.ruleUltraIpv6PortVlanQos.macDa.arEther, 0xFF,
                     sizeof(mask.ruleUltraIpv6PortVlanQos.macDa.arEther));
    }
    else
    {
        cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFF,
                     sizeof(mask.ruleStdNotIp.macDa.arEther));
    }

    /* AUTODOC: pattern for DST MAC address */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    if (ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
    {
        cpssOsMemCpy(pattern.ruleUltraIpv6PortVlanQos.macDa.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));
    }
    else
    {
        cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));
    }

    /* AUTODOC: action0 - redirect to port A */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                                           = PRV_TGF_PACKET_CMD_FORWARD_E;
    action.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
    action.redirect.redirectCmd                             = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum =
        prvTgfPortsArray[3];

    action.bypassBridge = GT_TRUE;

    ruleData.valid                   = GT_TRUE;
    prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);
    ruleData.rule.pcl.actionPtr      = &dxChAction;
    ruleData.rule.pcl.maskPtr        = &mask;
    ruleData.rule.pcl.patternPtr     = &pattern;
    tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
    if (ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
    {
        tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
    }
    else
    {
        tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    }

    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, V_TCAM_1, ruleId,
                                          ruleAttributesPtr, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                     V_TCAM_1, ruleId);
}

/*add and write some TTI rule (supposed to be no-hit) by ruleID and priority */
static void addTtiRuleNoHitByIndexPriority
(
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize,
    GT_U16 ruleId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC *ruleAttributesPtr
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_TTI_RULE_UNT                     mask;
    CPSS_DXCH_TTI_RULE_UNT                     pattern;
    CPSS_DXCH_TTI_ACTION_STC                   action;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;
    TGF_MAC_ADDR                                prvTgfArpMac = {0x00, 0x00, 0x00,
                                                                0x00, 0x00, 0x22};

    ruleSize = ruleSize;

    /* AUTODOC: mask for DST MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(mask.eth.common.mac.arEther, 0xFF,
                 sizeof(mask.eth.common.mac.arEther));

    /* AUTODOC: pattern for DST MAC address */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemCpy(pattern.eth.common.mac.arEther, prvTgfArpMac,
                    sizeof(pattern.eth.common.mac.arEther));

    /* AUTODOC: action0 - redirect to port A */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.command                                          = CPSS_PACKET_CMD_FORWARD_E;
    action.userDefinedCpuCode                               = CPSS_NET_FIRST_USER_DEFINED_E;
    action.redirectCommand                                  = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
    action.tag1VlanCmd                                      = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    action.egressInterface.type            = CPSS_INTERFACE_PORT_E;
    prvUtfHwDeviceNumberGet(prvTgfDevNum, &action.egressInterface.devPort.hwDevNum);
    action.egressInterface.devPort.portNum =   prvTgfPortsArray[3];

    action.bridgeBypass = GT_TRUE;

    ruleData.valid                   = GT_TRUE;
    ruleData.rule.tti.actionPtr      = &action;
    ruleData.rule.tti.maskPtr        = &mask;
    ruleData.rule.tti.patternPtr     = &pattern;
    tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
    tcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_ETH_E;

    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, V_TCAM_1, ruleId,
                                          ruleAttributesPtr, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                     V_TCAM_1, ruleId);
}

/**
* @internal prvTgfVirtualTcamPclRulesAndActionsSet function
* @endinternal
*
* @brief   setup PCL rules and action for vTCAM
*
* @param[in] ruleSize                 - Rule size to be tested (currently only 30 and 80 bytes supported)
* @param[in] cncUsed                  - to include CNC for cheking in
*                                      prvTgfVirtualTcamMoveRulePriorityUnderTrafficTest
*                                       None
*/
static GT_VOID prvTgfVirtualTcamPclRulesAndActionsSet
(
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize,
    GT_BOOL cncUsed
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    PRV_TGF_PCL_ACTION_STC                     action;
    CPSS_DXCH_PCL_ACTION_STC                   dxChAction;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType; /*for 8,9,500 rules */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    GT_U16                                      ii;

    /*AUTODOC: 2.  Priority 5 - Make rules #0..3 */
    ruleAttributes.priority      = 5;
    for (ii = 0; ii < 4; ii++)
    {
        addRuleNoHitByIndexPriority(ruleSize, ii, &ruleAttributes);
    }

    /*AUTODOC: 3.  Priority 7 - Make rules #4..7 */
    ruleAttributes.priority      = 7;
    for (ii = 4; ii < 8; ii++)
    {
        addRuleNoHitByIndexPriority(ruleSize, ii, &ruleAttributes);
    }

    prvTgfVirtualTcamPclBuildRule (ruleSize, &mask, &pattern);

    /* AUTODOC: action0 - redirect to port A */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                                            = PRV_TGF_PACKET_CMD_FORWARD_E;
    action.mirror.cpuCode                                    = CPSS_NET_FIRST_USER_DEFINED_E;
    action.redirect.redirectCmd                              = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type             = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum =
        prvTgfPortsArray[EGR_PORT_A];

    action.bypassBridge = GT_TRUE;

    ruleData.valid                   = GT_TRUE;
    prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);
    ruleData.rule.pcl.actionPtr      = &dxChAction;
    ruleData.rule.pcl.maskPtr        = &mask;
    ruleData.rule.pcl.patternPtr     = &pattern;
    tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
    if (ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
    {
        tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
    }
    else
    {
        tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    }

    /*AUTODOC: 4. Priority 5 - Make rule #8 with action to redirect to single egress port A */
    ruleAttributes.priority      = 5;
    action.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_A];
    if (cncUsed == GT_TRUE)
    {
        /* action - CNC counter */
        action.matchCounter.enableMatchCount = GT_TRUE;
        action.matchCounter.matchCounterIndex = 0;
    }

    prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);

    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, V_TCAM_1, ii,
                                          &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                     V_TCAM_1, ii);
    ii++;

    /*AUTODOC: 5. Priority 6 - Make SAME content of rule #9 (like #8)*/
    ruleAttributes.priority      = 6;

    if (cncUsed == GT_TRUE)
    {
        /*AUTODOC: Test #6 with same action but CNC index 1 */
        action.matchCounter.matchCounterIndex = 1;
    }
    else
    {
        /*AUTODOC: Test #4 with action to redirect to egress port B */
        action.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_B];
    }

    prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);

    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, V_TCAM_1, ii,
                                      &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                 V_TCAM_1, ii);

    /*AUTODOC: 6. Priority 5 - Make rule #499 */
    ruleAttributes.priority      = 5;
    ii = 499;
    addRuleNoHitByIndexPriority(ruleSize, ii, &ruleAttributes);

    /*AUTODOC: 7. Priority 7 - Make rule #498 */
    ruleAttributes.priority      = 7;
    ii = 498;
    addRuleNoHitByIndexPriority(ruleSize, ii, &ruleAttributes);

    if (cncUsed == GT_TRUE)
    {
        /*AUTODOC: Test#6.8. Priority 500 - Make SAME content of rule #500 (like #8) with action
         * to bypass bridge and flood */
        cpssOsMemSet(&action, 0, sizeof(action));
        ruleAttributes.priority      = 500;
        ii = 500;
        action.pktCmd                                            = PRV_TGF_PACKET_CMD_FORWARD_E;
        /* redirection does not works to VID             */
        /* it can only flood in the current packets VLAN */
        /* Vlan assignment used                          */
        action.bypassBridge = GT_TRUE;
        action.vlan.vlanId                                      = PRV_TGF_VLANID_CNS;
        action.vlan.modifyVlan                                  = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    }
    else
    {
        /*AUTODOC: Test#4.8. Priority 500 - Make SAME content of rule #500 (like #8) with action (DROP) */
        ruleAttributes.priority      = 500;
        ii = 500;
        cpssOsMemSet(&action, 0, sizeof(action));
        /* AUTODOC: action - hard drop */
        action.pktCmd = PRV_TGF_PACKET_CMD_DROP_HARD_E;
        action.mirror.cpuCode        = CPSS_NET_FIRST_USER_DEFINED_E;
    }

    prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);

    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, V_TCAM_1, ii,
                                      &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                 V_TCAM_1, ii);
}

/**
* @internal prvTgfVirtualTcamTtiRulesAndActionsSet function
* @endinternal
*
* @brief   setup TTI rules and action for vTCAM
*
* @param[in] ruleSize                 - Rule size to be tested (currently only 30 bytes supported)
* @param[in] cncUsed                  - to include CNC for cheking in
*                                      prvTgfVirtualTcamMoveRulePriorityUnderTrafficTest
*                                       None
*/
static GT_VOID prvTgfVirtualTcamTtiRulesAndActionsSet
(
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize,
    GT_BOOL cncUsed
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_TTI_RULE_UNT                     mask;
    CPSS_DXCH_TTI_RULE_UNT                     pattern;
    CPSS_DXCH_TTI_ACTION_STC                   action;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType; /*for 8,9,500 rules */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    GT_U16                                      ii;

    /*AUTODOC: 2.  Priority 5 - Make rules #0..3 */
    ruleAttributes.priority      = 5;
    for (ii = 0; ii < 4; ii++)
    {
        addTtiRuleNoHitByIndexPriority(ruleSize, ii, &ruleAttributes);
    }

    /*AUTODOC: 3.  Priority 7 - Make rules #4..7 */
    ruleAttributes.priority      = 7;
    for (ii = 4; ii < 8; ii++)
    {
        addTtiRuleNoHitByIndexPriority(ruleSize, ii, &ruleAttributes);
    }

    prvTgfVirtualTcamTtiBuildRule (ruleSize, &mask, &pattern);

    /* AUTODOC: action0 - redirect to port A */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.command                                          = CPSS_PACKET_CMD_FORWARD_E;
    action.userDefinedCpuCode                               = CPSS_NET_FIRST_USER_DEFINED_E;
    action.redirectCommand                                  = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
    action.tag1VlanCmd                                      = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    action.egressInterface.type            = CPSS_INTERFACE_PORT_E;
    prvUtfHwDeviceNumberGet(prvTgfDevNum, &action.egressInterface.devPort.hwDevNum);
    action.egressInterface.devPort.portNum =   prvTgfPortsArray[EGR_PORT_A];

    action.bridgeBypass = GT_TRUE;

    ruleData.valid                   = GT_TRUE;
    ruleData.rule.tti.actionPtr      = &action;
    ruleData.rule.tti.maskPtr        = &mask;
    ruleData.rule.tti.patternPtr     = &pattern;
    tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
    tcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_ETH_E;

    /*AUTODOC: 4. Priority 5 - Make rule #8 with action to redirect to single egress port A */
    ruleAttributes.priority      = 5;
    action.egressInterface.devPort.portNum =   prvTgfPortsArray[EGR_PORT_A];
    if (cncUsed == GT_TRUE)
    {
        /* action - CNC counter */
        action.bindToCentralCounter = GT_TRUE;
        action.centralCounterIndex = 0;
    }

    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, V_TCAM_1, ii,
                                          &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                     V_TCAM_1, ii);
    ii++;

    /*AUTODOC: 5. Priority 6 - Make SAME content of rule #9 (like #8)*/
    ruleAttributes.priority      = 6;

    if (cncUsed == GT_TRUE)
    {
        /*AUTODOC: Test #6 with same action but CNC index 1 */
        action.centralCounterIndex = 1;
    }
    else
    {
        /*AUTODOC: Test #4 with action to redirect to egress port B */
        action.egressInterface.devPort.portNum =   prvTgfPortsArray[EGR_PORT_B];
    }

    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, V_TCAM_1, ii,
                                      &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                 V_TCAM_1, ii);

    /*AUTODOC: 6. Priority 5 - Make rule #499 */
    ruleAttributes.priority      = 5;
    ii = 499;
    addTtiRuleNoHitByIndexPriority(ruleSize, ii, &ruleAttributes);

    /*AUTODOC: 7. Priority 7 - Make rule #498 */
    ruleAttributes.priority      = 7;
    ii = 498;
    addTtiRuleNoHitByIndexPriority(ruleSize, ii, &ruleAttributes);

    cpssOsMemSet(&action, 0, sizeof(action));
    ruleAttributes.priority      = 500;
    ii = 500;
    if (cncUsed == GT_TRUE)
    {
        /*AUTODOC: Test#6.8. Priority 500 - Make SAME content of rule #500 (like #8) with action
         * to bypass bridge and flood */
        action.command                            = CPSS_PACKET_CMD_FORWARD_E;
        /* redirection does not works to VID             */
        /* it can only flood in the current packets VLAN */
        /* Vlan assignment used                          */
        action.bridgeBypass = GT_TRUE;
        action.tag0VlanId                         = PRV_TGF_VLANID_CNS;
        action.tag0VlanCmd                        = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
        action.tag1VlanCmd                        = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    }
    else
    {
        /*AUTODOC: Test#4.8. Priority 500 - Make SAME content of rule #500 (like #8) with action (DROP) */
        /* AUTODOC: action - hard drop */
        action.command                            = CPSS_PACKET_CMD_DROP_HARD_E;
        action.userDefinedCpuCode                 = CPSS_NET_FIRST_USER_DEFINED_E;
        action.tag1VlanCmd                        = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    }

    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, V_TCAM_1, ii,
                                      &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                 V_TCAM_1, ii);
}

/**
* @internal prvTgfPclUdb10KeyCustomerUdbOnlyRangeSelectSet function
* @endinternal
*
* @brief   Set UDB Only PCL Key UDB Range selection
*         Replaced UDB positions ommited.
* @param[in] lookupNum                - PCL Lookup number
* @param[in] packetType               - packet Type
* @param[in] ruleFormat               - rule Format
* @param[in] udbReplaceBitmap         - bitmap of replace UDBs (12 bits ingress, 2 bits egress)
* @param[in] udbAmount                - udb Amount
* @param[in] udbIndexBase             - udb Index Base
*                                      udbOffsetIncrement  - udb Offset Increment (signed value, typical 1 or -1)
*                                       None
*/
static GT_STATUS prvTgfPclUdb10KeyCustomerUdbOnlyRangeSelectSet
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                           udbReplaceBitmap,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbIndexBase,
    IN GT_32                            udbIndexIncrement
)
{
    GT_STATUS                           rc;
    GT_U32                              i;
    GT_U32                              udbInKeyMapped;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;

    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));

    udbInKeyMapped = 0;

    for (i = 0; (i < udbAmount); i++)
    {
        if (udbReplaceBitmap & (1 << i))
        {
            if (i == 0) udbSelect.egrUdb01Replace      = GT_TRUE;
            if (i == 1) udbSelect.egrUdbBit15Replace   = GT_TRUE;
            if (i < 12) udbSelect.ingrUdbReplaceArr[i] = GT_TRUE;
        }
        else
        {
            udbSelect.udbSelectArr[i] =
                udbIndexBase + (udbIndexIncrement * udbInKeyMapped);
            udbInKeyMapped ++;
        }
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(
        ruleFormat, packetType, lookupNum,
        &udbSelect);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclUserDefinedBytesSelectSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdbKeyPclLegacyCfgSet function
* @endinternal
*
* @brief   Set Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] pktType                  - packet type: 0- not IP, 1 - IPV4, 2 - IPV6
* @param[in] ruleFormat               - rule Format
* @param[in] maskPtr                  - (pointer to) rule mask
* @param[in] patternPtr               - (pointer to) rule pattern
*                                       None
*/
static GT_VOID prvTgfPclUdbKeyPclLegacyCfgSet
(
    IN CPSS_PCL_DIRECTION_ENT             direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT         lookupNum,
    IN GT_U32                             pktType, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT        *maskPtr,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT        *patternPtr,
    IN GT_U32                             ruleId,
    IN GT_U32                             portNum,
    IN GT_U32                             pclId,
    IN GT_U16                             vTcamId
)
{
    GT_STATUS                                  rc;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT           ruleFormatArr[3];
    PRV_TGF_PCL_ACTION_STC                     action;
    CPSS_DXCH_PCL_ACTION_STC                   dxChAction;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              dxChPattern;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              dxChMask;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;

    ruleFormatArr[0] =
        (lookupNum == CPSS_PCL_LOOKUP_0_E)
            ? PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E;

    ruleFormatArr[1] =
        (lookupNum == CPSS_PCL_LOOKUP_0_E)
            ? PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E;

    ruleFormatArr[2] = ruleFormatArr[1];

    ruleFormatArr[pktType] = ruleFormat;

    /* AUTODOC: Init and configure all needed PCL configuration per port Lookup0 */
    rc = prvTgfPclDefPortInitExt1(
        portNum,
        direction, lookupNum,
        pclId,
        ruleFormatArr[0] /*nonIpKey*/,
        ruleFormatArr[1] /*ipv4Key*/,
        ruleFormatArr[2] /*ipv6Key*/);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInitExt1: Lookup# = %d, portIndex = %d",
                                     lookupNum, portNum);

    /* AUTODOC: action - Match CNC index  */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.egressPolicy                  = GT_FALSE;
    action.pktCmd                        = PRV_TGF_PACKET_CMD_FORWARD_E;
    action.mirror.cpuCode                = CPSS_NET_FIRST_USER_DEFINED_E;
    /*Set CNC match Counter index*/
    action.matchCounter.enableMatchCount = GT_TRUE;
    action.matchCounter.matchCounterIndex =
            (lookupNum == CPSS_PCL_LOOKUP_0_E)
            ? portNum
            : portNum + 100;

    /* AUTODOC: PCL rule: Write UDB Rule per port with CNC match*/
    prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);
    cpssOsMemSet(&dxChPattern, 0, sizeof(dxChPattern));
    cpssOsMemSet(&dxChMask, 0, sizeof(dxChMask));

    ruleAttributes.priority          = 5;
    tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;

    if (ruleFormat == PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E )
    {
        tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
    }
    else
    {
        tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
    }

    /* convert into device specific Pattern */
    prvTgfConvertGenericToDxChRuleIngressUdbOnly(
        &(patternPtr->ruleIngrUdbOnly), &(dxChPattern.ruleIngrUdbOnly));

    /* convert into device specific Mask */
    prvTgfConvertGenericToDxChRuleIngressUdbOnly(
        &(maskPtr->ruleIngrUdbOnly), &(dxChMask.ruleIngrUdbOnly));

    /*Rule data for vTcam*/
    ruleData.valid = GT_TRUE;
    ruleData.rule.pcl.actionPtr      = &dxChAction;
    ruleData.rule.pcl.maskPtr        = &dxChMask;
    ruleData.rule.pcl.patternPtr     = &dxChPattern;

    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, vTcamId, ruleId,
                                          &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                     vTcamId, ruleId);

    /* AUTODOC: PCL rule: Write UDB Rule per port with the DROP action*/
    ruleId++;
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd         = PRV_TGF_PACKET_CMD_DROP_HARD_E;
    action.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
    action.egressPolicy = GT_FALSE;

    prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);

    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, vTcamId, ruleId,
                                          &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                     vTcamId, ruleId);
}

static GT_STATUS createUdbRuleWithReplacedFld
(
    IN GT_U16                             pclId,
    IN GT_U16                             ruleId,
    IN GT_U32                             portNum,
    IN GT_U16                             vTcamId,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT         lookupNum,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT   ruleFormat
)
{
    GT_STATUS                                  rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT                mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT                pattern;
    GT_U16                                     vid;

    /* AUTODOC: PCL configuration */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    mask.ruleIngrUdbOnly.replacedFld.pclId = 0x3FF;
    pattern.ruleIngrUdbOnly.replacedFld.pclId = pclId;

    /* eVID */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 portNum,
                                 &vid);

    mask.ruleIngrUdbOnly.replacedFld.vid    = 0x1FFF;
    pattern.ruleIngrUdbOnly.replacedFld.vid = vid;

    /* Source ePort*/
    mask.ruleIngrUdbOnly.replacedFld.srcPort    = 0x1FFF;
    pattern.ruleIngrUdbOnly.replacedFld.srcPort = portNum;

    rc = prvTgfPclUdb10KeyCustomerUdbOnlyRangeSelectSet(
        lookupNum,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        ruleFormat,
        0xFF /*udbReplaceBitmap*/,
        6 /*udbAmount*/,
        30 /*udbIndexBase*/,
        1  /*udbIndexIncrement*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc,
                      "Failed prvTgfPclUdb10KeyCustomerUdbOnlyRangeSelectSet");

    prvTgfPclUdbKeyPclLegacyCfgSet(
    CPSS_PCL_DIRECTION_INGRESS_E,
    lookupNum,
    0, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
    ruleFormat,
    &mask, &pattern, ruleId, portNum, pclId, vTcamId);

    return rc;
}

/**
* @internal prvTgfVirtualTcamPclRulesAndActionsSet_DraggingTest function
* @endinternal
*
* @brief   setup PCL rules and action for vTCAM
*/
static GT_VOID prvTgfVirtualTcamPclRulesAndActionsSet_DraggingTest
(
    IN  GT_U32  portRxRateArr[CPSS_MAX_PORTS_NUM_CNS]
)
{
    GT_STATUS                                  rc;
    GT_U16                                     vTcamId = 1;
    GT_U32                                     portNum;
    GT_U16                                     ruleId = 0;

    rc = prvUtfNextMacPortReset(&portNum, prvTgfDevNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("prvUtfNextMacPortReset failure 1 \n");
        return;
    }

    /* make 2 rules per port with CNC matching and 2 with Drop in 2 vTcams (#10, #11) */
    while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
    {
        if (portNum >= (CPSS_MAX_PORTS_NUM_CNS-1))
        {
            PRV_UTF_LOG1_MAC("too many ports %d\n", portNum);
            return;
        }

        if (prvUtfIsTrafficManagerUsed() && (portNum >= 128))
        {
            /* skip interlacken channels */
            continue;
        }

        if (portRxRateArr[portNum] == 0)
        {
            /* skip ports without traffic */
            continue;
        }

        prvTgfPortsArray[0] = portNum;

        vTcamId = 10;
        rc = createUdbRuleWithReplacedFld(PRV_TGF_IPCL_PCL_ID_0_0_CNS,
                                          ruleId, portNum, vTcamId,
                                          CPSS_PCL_LOOKUP_0_E,
                                          PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                "createUdbRuleWithReplacedFld: rc = %d, port = %d, vTcamId = %d",
                                     rc, portNum, vTcamId);

        if (!prvTgfPclFwsUseSeparateTcamBlockPerLookupGet())
        {
            vTcamId = 11;
            rc = createUdbRuleWithReplacedFld(PRV_TGF_IPCL_PCL_ID_1_CNS,
                                              ruleId, portNum, vTcamId,
                                              CPSS_PCL_LOOKUP_1_E,
                                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                    "createUdbRuleWithReplacedFld: rc = %d, port = %d, vTcamId = %d",
                                         rc, portNum, vTcamId);
        }

        ruleId+=2;
    }
}

/**
* @internal prvTgfVirtualTcamTestPacketSend function
* @endinternal
*
* @brief   Function sends packet and check results.
*
* @param[in] portInterfacePtr         - (pointer to) port interface
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfVirtualTcamTestPacketSend
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_PACKET_STC           *packetInfoPtr
)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portsCount  = prvTgfPortsNum;
    GT_U32    portIter    = 0;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum, portInterfacePtr->devPort.portNum, GT_TRUE);

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS]);

    /* wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum, portInterfacePtr->devPort.portNum, GT_FALSE);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterfacePtr->devPort.portNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfPclVirtualTcamTestSendAndCheck function
* @endinternal
*
* @brief   Function sends packetand performs trace.
*
* @param[in] expPacketFld             - expected numebr of packets on "other" vlan ports:
*                                      1 on flooding, 0 on forwarding.
* @param[in] expectedRcRxToCpu        - return code expected for Rx to CPU:
*                                      GT_OK on mirroring, GT_NO_MORE if not.
* @param[in] callIdentifier           - function call identifier
*                                       None
*/
static GT_VOID prvTgfPclVirtualTcamTestSendAndCheck
(
    IN TGF_PACKET_STC   *tgfPacketInfoPtr,
    IN GT_U32           expPacketFld,
    IN GT_STATUS        expectedRcRxToCpu,
    IN GT_U32           callIdentifier,
    IN CPSS_INTERFACE_INFO_STC *portInterfacePtr
)
{
    GT_STATUS                       rc;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    GT_U32  actualCapturedNumOfPackets;

    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    GT_U8           devNum;
    GT_U8           queue;
    TGF_NET_DSA_STC rxParam;

    /* AUTODOC: send packet. */
    prvTgfVirtualTcamTestPacketSend(portInterfacePtr, tgfPacketInfoPtr);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter],
                                     callIdentifier);

        if (ING_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d, %d",
                                         prvTgfPortsArray[portIter], callIdentifier);
            continue;
        }

        /* check Tx counters */
        if ( portInterfacePtr->devPort.portNum == prvTgfPortsArray[portIter])
        {
            if (callIdentifier == 4)
            {
                prvTgfBurstCount = 0; /*for the DROP rule */
            }

            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d, %d",
                                         prvTgfPortsArray[portIter], callIdentifier);

            /* print captured packets and check TriggerCounters */
            rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                    portInterfacePtr,
                    tgfPacketInfoPtr,
                    prvTgfBurstCount,/*numOfPackets*/
                    0/*vfdNum*/,
                    NULL /*vfdArray*/,
                    NULL, /* bytesNum's skip list */
                    0,    /* length of skip list */
                    &actualCapturedNumOfPackets,
                    NULL/*onFirstPacketNumTriggersBmpPtr*/);

            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                     "port = %d, rc = 0x%02X, %d\n",
                                     portInterfacePtr->devPort.portNum, rc, callIdentifier);

            if (callIdentifier == 4)
            {
                prvTgfBurstCount = 1; /*restore default burst count */
            }
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(expPacketFld, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expectedon port %d, %d",
                                         prvTgfPortsArray[portIter], callIdentifier);
        }
    }

    /* get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(expectedRcRxToCpu, rc, "tgfTrafficGeneratorRxInCpuGet %d",
                                 callIdentifier);

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet %d",
                                 callIdentifier);

}

/*start index for vTCAM sequences*/
static GT_U32 startSequenceFrom                       = 0;

/**
* @internal prvTgfPclVirtualTcamDraggingTestConfigurationSet function
* @endinternal
*
* @brief   Virtual Tcam Dragging test configuration set
*/
GT_VOID prvTgfPclVirtualTcamDraggingTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                                  st   = GT_OK;
    GT_U32                                     vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    VT_SEQUENCE                                *sequencePtr;
    GT_U32                                     sequenceSize;
    GT_U32                                     idx;
    GT_U32                                     tcamEntriesNum;
    GT_U32                                     ii;
    GT_U32                                     cncBlockCntrs;
    GT_U32                                     cncBlocks;
    GT_U32                                     indexRangesBmp[4];
    PRV_TGF_CNC_CLIENT_ENT                     client;
    GT_U8                                      floorIndex;
    GT_PHYSICAL_PORT_NUM                       portNum;
    GT_U64                                     portDropCntrValue;
    GT_U8                                      devNum = prvTgfDevNum;

    /* AUTODOC: Save TCAM floor info for restore */
    for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet() ; floorIndex++ )
    {
        st = prvTgfTcamIndexRangeHitNumAndGroupGet(floorIndex,
                                              &saveFloorInfoArr[floorIndex][0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "prvTgfTcamIndexRangeHitNumAndGroupGet: %d",
            floorIndex);
    }

    sequencePtr = draggCfgTest_sequence;
    sequenceSize = sizeof(draggCfgTest_sequence) / sizeof(draggCfgTest_sequence[0]);

    /*AUTODOC: 1. Defining amount of the rules for vTCAM id = 100, ruleSize=60-bytes
     * must covering all TCAM beside 2 blocks (device depended size)*/
    st = cpssDxChCfgTableNumEntriesGet(prvTgfDevNum, CPSS_DXCH_CFG_TABLE_TTI_TCAM_E, &tcamEntriesNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet failed\n");

    if (tcamEntriesNum/6 > NUM_60_RULES_PER_BLOCK*2) /*shrink the TCAM to the 2 blocks */
    {
        sequencePtr[0].rulesAmount = tcamEntriesNum/6 - NUM_60_RULES_PER_BLOCK*2;
    }
    else
    {
        startSequenceFrom = 1;
    }

    vTcamMngId = V_TCAM_MANAGER;
    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
    vTcamInfo.hitNumber     = 0;
    vTcamInfo.clientGroup   = PCL_TCAM_GROUP_CNS;
    vTcamInfo.autoResize    = GT_FALSE;
    vTcamInfo.ruleAdditionMethod =
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

    /* Create vTCAM manager */
    st = cpssDxChVirtualTcamManagerCreate(vTcamMngId, &vtcamMngCfgParam);
        if (st == GT_ALREADY_EXIST)
            st = GT_OK;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerCreate failed\n");

    st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId,&prvTgfDevNum,1);
    if (st != GT_OK)
    {
        cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, &prvTgfDevNum, 1);
        st = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId, &prvTgfDevNum, 1);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

    for (idx = startSequenceFrom; idx < sequenceSize; idx++)
    {
        if ((sequencePtr[idx].operation == VT_CREATE) && HARD_WIRE_TCAM_MAC(prvTgfDevNum))
        {
            sequencePtr[idx].clientGroup = 1;
        }

        st = vtcamSequenceEntryProcess(sequencePtr, idx, vTcamMngId, &vTcamInfo);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(
            GT_FAIL, st,
            "vtcamSequenceEntryProcess failed, index: %d\n", idx);
    }

    prvTgfPclFwsCncFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_2_E;
    /* get CNC size info */
    prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

    /* use first range */
    indexRangesBmp[0] = 0x1;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    for (ii = 0; ii < cncBlocks; ii++)
    {
        if (prvTgfPclFwsCncBlockBmpDebug != 0xFFFFFFFF)
        {
            if (!(prvTgfPclFwsCncBlockBmpDebug & (1 << ii)))
            {
                continue;
            }
        }

        /* Get CNC client for a block */
        client = prvTgfPclCncClientGet(cncBlocks, ii);

        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            /* limit Falcon only by two PCL lookups */
            if (client != PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E &&
                client != PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E)
            {
                continue;
            }
        }

        /* configure CNC */
        st = prvTgfCncTestCncBlockConfigure(
            ii,
            client,
            GT_TRUE /*enable*/,
            indexRangesBmp,
            prvTgfPclFwsCncFormat);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, st, "prvTgfCncTestCncBlockConfigure: block %d, client %d",
                ii,
                client);

     }

    st = prvUtfNextMacPortReset(&portNum, prvTgfDevNum);
    if (st != GT_OK)
    {
        PRV_UTF_LOG0_MAC("prvUtfNextMacPortReset failure 1 \n");
        return;
    }

    /* make 2 rules per port with CNC matching and 2 with Drop in 2 vTcams (#10, #11) */
    while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
    {
        st= prvTgfPortMacCounterGet(prvTgfDevNum, portNum,
                                     CPSS_DROP_EVENTS_E,
                                     &portDropCntrValue);
           if (st != GT_OK)
           {
               PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet: dev %d port %d\n", devNum, portNum);
           }
    }

    /* AUTODOC: enable PCL clients for CNC */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS ))
    {
        st = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfCncCountingEnableSet");
    }
}

/**
* @internal prvTgfVirtualTcamTestConfigurationSet function
* @endinternal
*
* @brief   Virtual Tcam test configuration set
*
* @param[in] clientGroup              - Client group to be tested (TTI/PCL supported)
* @param[in] ruleSize                 - Rule size to be tested (currently only 30 and 80 bytes supported)
* @param[in] ruleAdditionMethod       - Either priority or logical index methods
*                                       None
*/
GT_VOID prvTgfVirtualTcamTestConfigurationSet
(
    GT_U32                                          clientGroup,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_ENT ruleAdditionMethod
)
{
    GT_STATUS                           rc;
    GT_U32                              floorIndex;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC     vTcamInfo;
    GT_U32                              vTcamId;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* AUTODOC: Create 30 Bytes logical index based vTCAMs */
    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
    vTcamInfo.ruleSize             = ruleSize;
    vTcamInfo.autoResize           = GT_FALSE;
    vTcamInfo.guaranteedNumOfRules = GUARANTEED_NUM_OF_RULES_PER_V_TCAM;
    vTcamInfo.ruleAdditionMethod   = ruleAdditionMethod;

    /* Create vTCAM manager */
    rc = cpssDxChVirtualTcamManagerCreate(V_TCAM_MANAGER, &vtcamMngCfgParam);
    if (rc == GT_ALREADY_EXIST)
        rc = GT_OK;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerCreate failed\n");

    rc = cpssDxChVirtualTcamManagerDevListAdd(V_TCAM_MANAGER, &prvTgfDevNum, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDevListAdd failed\n");


    /* AUTODOC: create VLAN 1968 with port 0,1,2,3. */
    prvTgfVirtualTcamTestVlanInit(PRV_TGF_VLANID_CNS);

    if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Save TCAM floor info for restore */
        for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet() ; floorIndex++ )
        {
            rc = prvTgfTcamIndexRangeHitNumAndGroupGet(floorIndex,
                                                  &saveFloorInfoArr[floorIndex][0]);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupGet: %d",
                floorIndex);
        }
    }

    /* AUTODOC: create vTCAM */
    vTcamInfo.hitNumber   = 0;
    vTcamInfo.clientGroup = clientGroup;
    vTcamId               = V_TCAM_1;

    rc = cpssDxChVirtualTcamCreate(V_TCAM_MANAGER, vTcamId, &vTcamInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamCreate FAILED for vTCAM = %d",
        vTcamId);

    if(HARD_WIRE_TCAM_MAC(prvTgfDevNum))
    {
        /* BC3 uses ipcl0 as 'group 1' */
    }
    else
    if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Save PCL TCAM group binding for restore */
        rc = prvTgfTcamClientGroupGet(PRV_TGF_TCAM_IPCL_0_E, &tcamGroup, &tcamGroupEnable);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "prvTgfTcamClientGroupGet: %d, %d, %d",
                PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);

        /* AUTODOC: Save TTI TCAM group binding for restore */
        rc = prvTgfTcamClientGroupGet(PRV_TGF_TCAM_TTI_E, &ttiTcamGroup, &ttiTcamGroupEnable);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "prvTgfTcamClientGroupGet: %d, %d, %d",
                PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);

        /* AUTODOC: PCL TCAM group binding */
        rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_IPCL_0_E, PCL_TCAM_GROUP_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
                PRV_TGF_TCAM_IPCL_0_E, PCL_TCAM_GROUP_CNS, GT_TRUE);

        /* AUTODOC: TTI TCAM group binding */
        rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_TTI_E, TTI_TCAM_GROUP_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
                PRV_TGF_TCAM_TTI_E, TTI_TCAM_GROUP_CNS, GT_TRUE);
    }

    /* AUTODOC: Init IPCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[ING_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* AUTODOC: save TTI lookup key for port 0 */
    rc = prvTgfTtiPortLookupEnableGet(prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, &ttiTcamLookupEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableGet: %d", prvTgfDevNum);

    /* AUTODOC: save MAC_MODE_DA for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_ETH_E, &ttiTcamMacMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: set MAC_MODE_DA for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfVirtualTcamPriorityMoveRuleConfigurationRestore function
* @endinternal
*
* @brief   Virtual Tcam Move Rules with priority configurations restore
*
* @param[in] cncUsed                  - to include CNC for cheking in
*                                      prvTgfVirtualTcamMoveRulePriorityUnderTrafficTest
*                                       None
*/
GT_VOID prvTgfVirtualTcamPriorityMoveRuleConfigurationRestore
(
   GT_BOOL cncUsed
)
{
    GT_STATUS                        rc;
    GT_U32                           floorIndex;
    GT_U16                           jj;
    GT_U32                           indexRangesBmp[4];
    GT_U32                           cncBlockCntrs;
    GT_U32                           cncBlocks;
    PRV_TGF_CNC_CLIENT_ENT           client;
    GT_U32                           cncBlockNum = 1;
    GT_U32                           counterIdx;
    PRV_TGF_CNC_COUNTER_STC          counter;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: Remove vTCAMs with PCL rules  */
    for(jj = 0; jj < 8; jj++ )
    {
        rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, jj);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                     V_TCAM_1, jj);
    }
    if(cncUsed)
    {
        rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, jj);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                     V_TCAM_1, jj);
        jj++;
        rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, jj);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                     V_TCAM_1, jj);
        jj = 498;
        rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, jj);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                     V_TCAM_1, jj);
        jj = 500;
        rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, jj);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                     V_TCAM_1, jj);

        cpssOsMemSet(&counter, 0, sizeof(counter));

        /* get CNC size info */
        prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

        /* use first range */
        indexRangesBmp[0] = 0x0;
        indexRangesBmp[1] = 0x0;
        indexRangesBmp[2] = 0x0;
        indexRangesBmp[3] = 0x0;

        {
            /* Get CNC client for a block */
            client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;

            /* disable client */
            rc = prvTgfCncBlockClientEnableSet(
                cncBlockNum, client, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", cncBlockNum, client);

            /* reset ranges */
            rc = prvTgfCncBlockClientRangesSet(
                cncBlockNum, client, indexRangesBmp);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", cncBlockNum, client);
        }
        {
            /* Get CNC client for a block */
            client = PRV_TGF_CNC_CLIENT_TTI_E;

            /* disable client */
            rc = prvTgfCncBlockClientEnableSet(
                cncBlockNum, client, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", cncBlockNum, client);

            /* reset ranges */
            rc = prvTgfCncBlockClientRangesSet(
                cncBlockNum, client, indexRangesBmp);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", cncBlockNum, client);
        }
        /* clean counters */
        for (counterIdx = 0; counterIdx < cncBlockCntrs; counterIdx++)
        {
            rc = prvTgfCncCounterSet(
                cncBlockNum, counterIdx,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncCounterSet: block %d idx %d", cncBlockNum, counterIdx);
        }

        /* disable PCL clients for CNC */
        if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA3_E | UTF_CPSS_PP_E_ARCH_CNS ))
        {
            rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_FALSE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
        }
    }

    jj = 499;
    rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, jj);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                 V_TCAM_1, jj);

    rc = cpssDxChVirtualTcamRemove(V_TCAM_MANAGER, V_TCAM_1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRemove(vTCAM:%d) failed\n", V_TCAM_1);

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy for port 1 (the ingress port) */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[ING_PORT_IDX_CNS],
                                 GT_FALSE);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: restore TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, ttiTcamLookupEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore MAC mode for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, ttiTcamMacMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Restore TCAM floor info */
        for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet() ; floorIndex++ )
        {
            rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex,
                                                  &saveFloorInfoArr[floorIndex][0]);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
                floorIndex);
        }

        if(HARD_WIRE_TCAM_MAC(prvTgfDevNum))
        {
            /* BC3 uses ipcl0 as 'group 1' */
        }
        else
        {
            /* AUTODOC: Restore PCL TCAM group binding */
            rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);
            UTF_VERIFY_EQUAL3_STRING_MAC(
                    GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
                    PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);

            /* AUTODOC: Restore TTI TCAM group binding */
            rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);
            UTF_VERIFY_EQUAL3_STRING_MAC(
                    GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
                    PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);
        }
    }

    rc = cpssDxChVirtualTcamManagerDevListRemove(V_TCAM_MANAGER, &prvTgfDevNum, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                 V_TCAM_MANAGER);

    rc = cpssDxChVirtualTcamManagerDelete(V_TCAM_MANAGER);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                 V_TCAM_MANAGER);

    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* Restore TCAM Active number of floors */
        rc = cpssDxChTcamActiveFloorsSet(prvTgfDevNum,
                fineTuningPtr->tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);

        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "cpssDxChTcamActiveFloorsSet failed for device: %d", prvTgfDevNum);
    }
}

/**
* @internal prvTgfVirtualTcamPriorityMoveRuleTest function
* @endinternal
*
* @brief   Test basic moved rule between priorities (use traffic)-
*         priority mode
*         PURPOSE:
*         1. Check that the moved rule between priorities changed actual priority
*         LOGIC:
*         1. Creat vTcam
*         2. Set rules with diffirent priorites
*         3. Send packet to hit the rules
*         4. Move rules
*         5. Send and check packets to garantee that moving are correct
* @param[in] useTti                   - GT_TRUE: test TTI TCAM, GT_FALSE: test PCL TCAM
* @param[in] ruleSize                 - Rule size to be tested (currently only 30 and 80 bytes supported)
*                                       None
*/

GT_VOID prvTgfVirtualTcamPriorityMoveRuleTest
(
    GT_BOOL                                         useTti,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize
)
{
    GT_U32                                     sendCallInst = 0;
    CPSS_INTERFACE_INFO_STC                    portInterface;
    GT_U16                                     ruleIndex;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    TGF_PACKET_STC                            *tgfPacketInfoPtr;
    GT_U32                                     clientGroup = PCL_TCAM_GROUP_CNS;
    GT_STATUS                                  rc;

    if (useTti)
    {
        clientGroup = TTI_TCAM_GROUP_CNS;
    }

    /* AUTODOC: Virtual Tcam Priority Move Test configuration set*/
    prvTgfVirtualTcamTestConfigurationSet(clientGroup,
                                        ruleSize,
                                        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E);

    if (clientGroup == PCL_TCAM_GROUP_CNS)
    {
        /* AUTODOC: Configure PCL rules */
        prvTgfVirtualTcamPclRulesAndActionsSet(ruleSize, GT_FALSE);
    }
    else
    {
        /* AUTODOC: Configure TTI rules */
        prvTgfVirtualTcamTtiRulesAndActionsSet(ruleSize, GT_FALSE);
    }

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_A];

    /* AUTODOC: 9. Send single packet that can hit those rules (8,9,500)
     *         10. Check that packet egress port A , without flooding.
     *                                     (associated with priority 5) */
    if (ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
    {
        tgfPacketInfoPtr = &prvTgfIpv6PacketInfo;
    }
    else
    {
        tgfPacketInfoPtr = &prvTgfPacketInfo;
    }
    prvTgfPclVirtualTcamTestSendAndCheck(tgfPacketInfoPtr,
                                            0, GT_NO_MORE, sendCallInst++/*0*/,
                                                   &portInterface);

    /* AUTODOC: 11. Move index 8 to index 498 (priority 5 to priority 7) */
    rc = cpssDxChVirtualTcamRuleMove(V_TCAM_MANAGER, V_TCAM_1, 8, 498);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "cpssDxChVirtualTcamRuleMove failed: rc %d src %d dst %d\n",
                rc, 8, 498);

    /* AUTODOC: 12. Send single packet that can hit those rules (498,9,500)
                13. Check that packet egress port B , without flooding.
                                             (associated with priority 6)*/
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_B];
    prvTgfPclVirtualTcamTestSendAndCheck(tgfPacketInfoPtr,
                                            0, GT_NO_MORE, sendCallInst++/*1*/,
                                                   &portInterface);

    /* AUTODOC: 14. Add rule with �no hit� to index 8 priority 4*/
    ruleAttributes.priority     = 4;
    ruleIndex = 8;
    if (clientGroup == PCL_TCAM_GROUP_CNS)
    {
        addRuleNoHitByIndexPriority(ruleSize, ruleIndex, &ruleAttributes);
    }
    else
    {
        addTtiRuleNoHitByIndexPriority(ruleSize, ruleIndex, &ruleAttributes);
    }

    /* AUTODOC: 15. Move index 498 to index 8 (priority 7 to priority 4) */
    rc = cpssDxChVirtualTcamRuleMove(V_TCAM_MANAGER, V_TCAM_1, 498, 8);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "cpssDxChVirtualTcamRuleMove failed: rc %d src %d dst %d\n",
                rc, 498, 8);

    /* AUTODOC: 16. Send single packet that can hit those rules (8,9,500)
                17. Check that packet egress port A , without flooding.
                          (associated with priority 4)*/

    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_A];
    prvTgfPclVirtualTcamTestSendAndCheck(tgfPacketInfoPtr,
                                            0, GT_NO_MORE, sendCallInst++/*2*/,
                                                   &portInterface);

    /*AUTODOC:  18. Delete rule #8 */
    ruleIndex = 8;
    rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, ruleIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                     V_TCAM_1, ruleIndex);

    /*AUTODOC:  19. Send single packet that can hit those rules (9,500)
                20. Check that packet egress port B , without flooding.
                                           (associated with priority 6)*/

    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_B];
    prvTgfPclVirtualTcamTestSendAndCheck(tgfPacketInfoPtr,
                                            0, GT_NO_MORE, sendCallInst++/*3*/,
                                                   &portInterface);

    /*AUTODOC:  21. Delete rule #9 */
    ruleIndex = 9;
    rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, ruleIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                     V_TCAM_1, ruleIndex);

    /*AUTODOC:  22. Send single packet that can hit last relevant rule (500)
                23. Check that no packet egress ! (DROPED on rule 500)    */

    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_B];
    prvTgfPclVirtualTcamTestSendAndCheck(tgfPacketInfoPtr,
                                            0, GT_NO_MORE, sendCallInst++/*4*/,
                                                        &portInterface);

    /*AUTODOC:  24. Delete rule #500 */
    ruleIndex = 500;
    rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, ruleIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                     V_TCAM_1, ruleIndex);

    /*AUTODOC:  25. Check that packet flooding to all ports (since no tcam hits)*/
    prvTgfPclVirtualTcamTestSendAndCheck(tgfPacketInfoPtr,
                                            1, GT_NO_MORE, sendCallInst++/*5*/,
                                                   &portInterface);
}

/**
* @internal prvTgfVirtualTcamUpdateActionTestPclRulesAndActionsSet function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfVirtualTcamUpdateActionTestPclRulesAndActionsSet
(
        GT_VOID
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    PRV_TGF_PCL_ACTION_STC                     action;
    CPSS_DXCH_PCL_ACTION_STC                   dxChAction;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType; /*for 8,9,500 rules */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    GT_U16                                      ii;

    /* AUTODOC: mask for DST MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFF,
                 sizeof(mask.ruleStdNotIp.macDa.arEther));

    /* AUTODOC: pattern for DST MAC address */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther, prvTgfPacketL2Part.daMac,
                 sizeof(prvTgfPacketL2Part.daMac));

    /* AUTODOC: action0 - count to CNC index 0, redirect to port A */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                                            = PRV_TGF_PACKET_CMD_FORWARD_E;
    action.mirror.cpuCode                                    = CPSS_NET_FIRST_USER_DEFINED_E;
    action.redirect.redirectCmd                              = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type             = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum =
        prvTgfPortsArray[EGR_PORT_A];
    action.matchCounter.enableMatchCount                    = GT_TRUE;
    action.matchCounter.matchCounterIndex                   = 0;

    action.bypassBridge = GT_TRUE;

    ruleData.valid                   = GT_TRUE;
    prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);
    ruleData.rule.pcl.actionPtr      = &dxChAction;
    ruleData.rule.pcl.maskPtr        = &mask;
    ruleData.rule.pcl.patternPtr     = &pattern;
    tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
    tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    /*4. Priority 5 - Make rule #8 with action to count to CNC index 0, redirect to single egress port A */
    ruleAttributes.priority      = 5;
    ii = 8;

    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, V_TCAM_1, ii,
                                          &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                     V_TCAM_1, ii);
    ii++;

    /* 5. Priority 6 - Make SAME content of rule #9 (like #8)
     * with action to bypass bridge and flood */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                                            = PRV_TGF_PACKET_CMD_FORWARD_E;
    /* redirection does not works to VID             */
    /* it can only flood in the current packets VLAN */
    /* Vlan assignment used                          */
    action.bypassBridge = GT_TRUE;
    action.vlan.vlanId                                      = PRV_TGF_VLANID_CNS;
    action.vlan.modifyVlan                                  = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);

    ruleAttributes.priority      = 6;
    ii = 9;
    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, V_TCAM_1, ii,
                                      &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                 V_TCAM_1, ii);

}

/**
* @internal prvTgfVirtualTcamUpdateActionTestTtiRulesAndActionsSet function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfVirtualTcamUpdateActionTestTtiRulesAndActionsSet
(
        GT_VOID
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_TTI_RULE_UNT                     mask;
    CPSS_DXCH_TTI_RULE_UNT                     pattern;
    CPSS_DXCH_TTI_ACTION_STC                   action;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType; /*for 8,9,500 rules */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    GT_U16                                      ii;

    prvTgfVirtualTcamTtiBuildRule(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E,
                        &mask,
                        &pattern);
    /* AUTODOC: action0 - count to CNC index 0, redirect to port A */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.command                                           = CPSS_PACKET_CMD_FORWARD_E;
    action.userDefinedCpuCode                                = CPSS_NET_FIRST_USER_DEFINED_E;
    action.redirectCommand                                   = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
    action.egressInterface.type                              = CPSS_INTERFACE_PORT_E;
    prvUtfHwDeviceNumberGet(prvTgfDevNum, &action.egressInterface.devPort.hwDevNum);
    action.egressInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_A];
    action.bindToCentralCounter                              = GT_TRUE;
    action.centralCounterIndex                               = 0;
    action.tag1VlanCmd                                       = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;

    action.bridgeBypass = GT_TRUE;

    ruleData.valid                   = GT_TRUE;
    ruleData.rule.tti.actionPtr      = &action;
    ruleData.rule.tti.maskPtr        = &mask;
    ruleData.rule.tti.patternPtr     = &pattern;
    tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
    tcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_ETH_E;

    /*4. Priority 5 - Make rule #8 with action to count to CNC index 0, redirect to single egress port A */
    ruleAttributes.priority      = 5;
    ii = 8;

    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, V_TCAM_1, ii,
                                          &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                     V_TCAM_1, ii);
    ii++;

    /* 5. Priority 6 - Make SAME content of rule #9 (like #8)
     * with action to bypass bridge and flood */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.command                                           = CPSS_PACKET_CMD_FORWARD_E;
    /* redirection does not works to VID             */
    /* it can only flood in the current packets VLAN */
    /* Vlan assignment used                          */
    action.bridgeBypass = GT_TRUE;
    action.tag0VlanCmd                                       = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
    action.tag0VlanId                                        = PRV_TGF_VLANID_CNS;
    action.tag1VlanCmd                                       = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;

    ruleAttributes.priority      = 6;
    ii = 9;
    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, V_TCAM_1, ii,
                                      &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                 V_TCAM_1, ii);

}

/**
* @internal prvTgfVirtualTcamUpdateActionTrafficGenerate function
* @endinternal
*
* @brief   Configure, start traffic from port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] packetInfoPtr            - (Pointer to) packet data
*                                       None
*/
static GT_STATUS prvTgfVirtualTcamUpdateActionTrafficGenerate
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  TGF_PACKET_STC                 *packetInfoPtr
)
{
    GT_STATUS   st;

    /* reset counters */
    st = prvTgfResetCountersEth(devNum, portNum);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("prvTgfResetCountersEth FAILED:st-%d,devNum-%d,portNum-%d\n",
                                                    st, devNum, portNum);
        return st;
    }

    /* setup Packet */
    st = prvTgfSetTxSetupEth(devNum, packetInfoPtr, 1, 0, NULL);
    if (GT_OK != st)
    {
        PRV_UTF_LOG2_MAC("prvTgfSetTxSetupEth FAILED:st-%d,devNum-%d\n",
                                                    st, devNum);
        return st;
    }

    /* configure WS mode */
    st = prvTgfTxModeSetupEth(devNum, portNum,
                              PRV_TGF_TX_MODE_CONTINUOUS_E,
                              PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                              0);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("prvTgfSetTxSetupEth FAILED:st-%d,devNum-%d,portNum-%d\n",
                                                    st, devNum, portNum);
        return st;
    }

    /* send Packet from port portNum */
    st = prvTgfStartTransmitingEth(devNum, portNum);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("prvTgfStartTransmitingEth FAILED:st-%d,devNum-%d,portNum-%d\n",
                                                    st, devNum, portNum);
        return st;
    }

    /* wait to get stable traffic */
    PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_MAC(10);

    return GT_OK;
}

/**
* @internal prvTgfVirtualTcamUpdateActionTrafficStop function
* @endinternal
*
* @brief   Stop traffic from port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                       None
*/
static GT_STATUS prvTgfVirtualTcamUpdateActionTrafficStop
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
)
{
    GT_STATUS   st;

    /* ----- stop send Packet from port portNum -------- */
    st = prvTgfStopTransmitingEth(devNum, portNum);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("prvTgfStopTransmitingEth FAILED:st-%d,devNum-%d,portNum-%d\n",
                                                    st, devNum, portNum);
        return st;
    }

    /* wait some time to guarantee that traffic stopped */
    PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_MAC(10);

    /* configure default mode */
    st = prvTgfTxModeSetupEth(devNum, portNum,
                                PRV_TGF_TX_MODE_SINGLE_BURST_E,
                                PRV_TGF_TX_CONT_MODE_UNIT_WS_E,
                                0);
    if (GT_OK != st)
    {
        PRV_UTF_LOG3_MAC("prvTgfTxModeSetupEth FAILED:st-%d,devNum-%d,portNum-%d\n",
                            st, devNum, portNum);
        return st;
    }

    return GT_OK;
}

/**
* @internal prvTgfVirtualTcamUpdateActionPortAndCncCntrsCheckOrReset function
* @endinternal
*
* @brief   Check that CNC counter is same as Rx one.
*         Or reset both CNC and Rx counters
* @param[in] portNum                  - Rx port
* @param[in] check                    - if to check/reset Rx counters
* @param[in] counterIdxNum            - number of counterIdx for CNC checking
* @param[in] cncCntrStep              - counters step to check CNC counters
*
* @retval GT_OK                    - on OK
* @retval GT_FAIL                  - on failure
*/
static GT_STATUS  prvTgfVirtualTcamUpdateActionPortAndCncCntrsCheckOrReset
(
  IN   GT_PHYSICAL_PORT_NUM    portNum,
  IN   GT_BOOL                 check,
  IN   GT_U32                  counterIdxNum,
  IN   GT_U32                  cncCntrStep
)
{
    GT_STATUS               rc = GT_OK; /* return code */
    GT_STATUS               rc1 = GT_OK; /* return code */
    GT_U8                   devNum = prvTgfDevNum; /* device number */
    GT_U32                  cncBlockNum = 1; /* CNC block number */
    GT_U32                  counterIdx;      /* counter index */
    PRV_TGF_CNC_COUNTER_STC cncCounter;         /* counter */
    PRV_TGF_CNC_COUNTER_STC cncTotalCounter;         /* counter */
    GT_PORT_GROUPS_BMP      portGroupBmp;    /* port group bitmap */
    GT_U64                  portCntrValue;   /* value of port counters */
    GT_U64                  portDropCntrValue;   /* value of port drop event counters */

    cpssOsMemSet(&cncTotalCounter, 0x0, sizeof(cncTotalCounter));

    /* use PortGroupsBmp API in the test */
    /* set <currPortGroupsBmp> */
    TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(portNum);
    portGroupBmp = currPortGroupsBmp;

    /* restore values after using currPortGroupsBmp*/
    usePortGroupsBmp = GT_FALSE;
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    for (counterIdx = 0; counterIdx < counterIdxNum; counterIdx += cncCntrStep)
    {

        rc = prvTgfCncPortGroupCounterGet(portGroupBmp,
            cncBlockNum, counterIdx,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &cncCounter);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfCncPortGroupCounterGet: block %d idx %d\n", cncBlockNum, counterIdx);
            return rc;
        }

        if ((counterIdxNum < 10) && check)
        {
            PRV_UTF_LOG4_MAC("CNC block %d idx %d packetCount %d byteCount %d\n", cncBlockNum, counterIdx, cncCounter.packetCount.l[0], cncCounter.byteCount.l[0]);
        }

        /* sum over all counters */
        cncTotalCounter.packetCount =
            prvCpssMathAdd64(
                cncTotalCounter.packetCount, cncCounter.packetCount);
        cncTotalCounter.byteCount =
            prvCpssMathAdd64(
                cncTotalCounter.byteCount, cncCounter.byteCount);
    }

    rc = prvTgfPortMacCounterGet(devNum, portNum,
                                  CPSS_GOOD_UC_PKTS_RCV_E,
                                  &portCntrValue);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet: dev %d port %d\n", devNum, portNum);
        return rc;
    }

    /* non MTI MAC counts Drop Events in Good Rx also. MTI MACs do not count them  */
    if (GT_FALSE == prvUtfIsPortMacTypeMti(devNum, portNum))
    {
        rc = prvTgfPortMacCounterGet(devNum, portNum,
                                      CPSS_DROP_EVENTS_E,
                                      &portDropCntrValue);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet: dev %d port %d\n", devNum, portNum);
            return rc;
        }
    }
    else
    {
        portDropCntrValue.l[0] = portDropCntrValue.l[1] = 0;
    }

    if (check)
    {
        if ((portCntrValue.l[0] - portDropCntrValue.l[0]) != cncTotalCounter.packetCount.l[0])
        {
            PRV_UTF_LOG4_MAC("prvTgfVirtualTcamUpdateActionPortAndCncCntrsCheckOrReset failure block %d port %d cnc %d port %d\n", cncBlockNum, portNum, cncTotalCounter.packetCount.l[0], portCntrValue.l[0]);
            rc1 = GT_FAIL;
        }
    }

    return rc1;
}

/**
* @internal prvTgfVirtualTcamPriorityMoveRuleUnderTrafficTest function
* @endinternal
*
* @brief   Test basic moved rule between priorities (use traffic)-
*         priority mode
*         PURPOSE:
*         1. Check that the moved rule between priorities changed actual priority
*         LOGIC:
*         1. Create vTcam
*         2. Set rules with different priorities
*         3. Generate traffic to hit the rules
*         4. Move rules under traffic
*         5. Stop traffic
*         6. Check the CNC counters
*/
GT_VOID prvTgfVirtualTcamPriorityMoveRuleUnderTrafficTest
(
    GT_VOID
)
{
    GT_U16                                     ruleIndex;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    GT_STATUS                                  rc;
    GT_U32                                     ii;
    GT_U32                                     indexRangesBmp[4];
    PRV_TGF_CNC_CLIENT_ENT                     client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;
    GT_U32                                     cncBlockNum = 1;
    GT_U32                                     portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC              portCntrs;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT       ruleSize;
    GT_U32                                     clientGroup = PCL_TCAM_GROUP_CNS;

    /* AUTODOC: Vitual Tcam Priority Move Test configuration set*/
    ruleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    prvTgfVirtualTcamTestConfigurationSet(clientGroup,
                    ruleSize,
                    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E);

    /* AUTODOC: Configure PCL rules */
    prvTgfVirtualTcamPclRulesAndActionsSet(ruleSize, GT_TRUE);

    /* use first range */
    indexRangesBmp[0] = 0x1;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    /* configure CNC */
    rc = prvTgfCncTestCncBlockConfigure(
        cncBlockNum,
        client,
        GT_TRUE /*enable*/,
        indexRangesBmp,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfCncTestCncBlockConfigure: block %d, client %d",
            cncBlockNum,
            client);

    /* AUTODOC: enable PCL clients for CNC */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS ))
    {
        rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    }

    /* reset counters */
    rc = prvTgfVirtualTcamUpdateActionPortAndCncCntrsCheckOrReset(
        prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamUpdateActionPortAndCncCntrsReset");

    /* AUTODOC: configure and generate FWS
        5.        Send continous WS traffic that hit this rule */
    rc = prvTgfVirtualTcamUpdateActionTrafficGenerate(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], &prvTgfPacketInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamUpdateActionTrafficGenerate: port %d", prvTgfPortsArray[ING_PORT_IDX_CNS]);

    /*14. Goto to #9 for 1K times.*/
    for (ii = PRV_TGF_VIRTUAL_TCAM_TEST_ITERATIONS_STEP;
            ii < PRV_TGF_VIRTUAL_TCAM_TEST_NUM_ITERATIONS;
            ii += PRV_TGF_VIRTUAL_TCAM_TEST_ITERATIONS_STEP)
    {
        /* AUTODOC: 10. Move index 8 to index 498 (priority 5 to priority 7) */
        rc = cpssDxChVirtualTcamRuleMove(V_TCAM_MANAGER, V_TCAM_1, 8, 498);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "cpssDxChVirtualTcamRuleMove failed: rc %d src %d dst %d\n",
                    rc, 8, 498);
        /* AUTODOC:10a. For simulation do sleep 50 millisec (to allow traffic hit the entry) */
        PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_ON_SIMULATION_MAC(50);

        /* AUTODOC:10b. Add rule with "no hit" to index 8 priority 7*/
        ruleAttributes.priority = 7;
        ruleIndex               = 8;
        addRuleNoHitByIndexPriority(ruleSize, ruleIndex, &ruleAttributes);

        /* AUTODOC: 11. Move index 9 to index 499 (priority 6 to priority 5) */
        rc = cpssDxChVirtualTcamRuleMove(V_TCAM_MANAGER, V_TCAM_1, 9, 499);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "cpssDxChVirtualTcamRuleMove failed: rc %d src %d dst %d\n",
                    rc, 9, 499);

        /* AUTODOC: 11a. Add rule with "no hit" to index 9 priority 6*/
        ruleAttributes.priority = 6;
        ruleIndex               = 9;
        addRuleNoHitByIndexPriority(ruleSize, ruleIndex, &ruleAttributes);

        /* AUTODOC: 12. Move index 499 to index 9 (priority 5 to priority 6)*/
        rc = cpssDxChVirtualTcamRuleMove(V_TCAM_MANAGER, V_TCAM_1, 499, 9);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "cpssDxChVirtualTcamRuleMove failed: rc %d src %d dst %d\n",
                    rc, 499, 9);

        /* AUTODOC: 12a. Add rule with "no hit" to index 499 priority 5*/
        ruleAttributes.priority = 5;
        ruleIndex               = 499;
        addRuleNoHitByIndexPriority(ruleSize, ruleIndex, &ruleAttributes);

        /* AUTODOC: 13. Move index 498 to index 8 (priority 7 to priority 5) */
        rc = cpssDxChVirtualTcamRuleMove(V_TCAM_MANAGER, V_TCAM_1, 498, 8);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "cpssDxChVirtualTcamRuleMove failed: rc %d src %d dst %d\n",
                    rc, 498, 8);

        /* AUTODOC: 12a. Add rule with "no hit" to index 498 priority 7 */
        ruleAttributes.priority = 7;
        ruleIndex               = 498;
        addRuleNoHitByIndexPriority(ruleSize, ruleIndex, &ruleAttributes);
    }

    /* AUTODOC: 15. stop traffic */
    rc = prvTgfVirtualTcamUpdateActionTrafficStop(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop");

    PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_MAC(1000);

    /* AUTODOC: 16. Check that the CNC summary of indexes 0,1 is equal to number of packets that inress the device.
     *              Compare Port and CNC counters */
    rc = prvTgfVirtualTcamUpdateActionPortAndCncCntrsCheckOrReset(prvTgfPortsArray[ING_PORT_IDX_CNS], GT_TRUE, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamUpdateActionPortAndCncCntrsReset");

    /* AUTODOC: 17. Check that there was no flooding to other ports. */
    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* skip ingress port */
        if (portIter == ING_PORT_IDX_CNS)
        {
            continue;
        }
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (portIter == EGR_PORT_A)
        {
            /* check WS traffic by compare with some value */
            if (portCntrs.goodPktsSent.l[0] < 10)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(10, portCntrs.goodPktsSent.l[0],
                                             "get another goodPktsSent counter than expected on port %d",
                                             prvTgfPortsArray[portIter]);
            }
        }
        else
        {
            /* check Tx counters. Should be 0 - no flooding. */
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }
}




/**
* @internal prvTgfVirtualTcamUpdateActionTest function
* @endinternal
*
* @brief   Test action update under traffic (use traffic)-
*         priority mode
*         PURPOSE:
*         1. Check that update action of rules under traffic not losing packets.
*         LOGIC:
*         1.    Create tcam manager with vtcam with 10 rules
*         2.    Make rule #8 with action to count to CNC index 0 (redirect to single egress port)
*         3.    Make rule #9 Make SAME content of rule like #8 with action (bypass bridge + flood)
*         4.
*         5.    Send continues WS traffic that hit this rule
*         6.    Updated the action of rule 8 to count to CNC index 1 (cpssDxChVirtualTcamRuleActionUpdate)
*         a.    For simulation do sleep 50 millisec (to allow traffic hit the entry)
*         7.    Updated the action of rule 8 to count to CNC index 2
*         8.    �
*         9.    Updated the action of rule 8 to count to CNC index 511
*         10.    Stop the traffic
*         11.    Check that the CNC summary of indexes 0..511 is equal to number of packets that egress the device.
*         12.    Check that there was no flooding to other ports.
* @param[in] useTti                   - GT_TRUE: test TTI TCAM, GT_FALSE: test PCL TCAM
*                                       None
*/

GT_VOID prvTgfVirtualTcamUpdateActionTest
(
    GT_BOOL                                         useTti
)
{
    PRV_TGF_PCL_ACTION_STC                      action;
    CPSS_DXCH_PCL_ACTION_STC                    dxChAction;
    CPSS_DXCH_TTI_ACTION_STC                    ttiAction;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC actionType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC actionData;
    GT_U32                                      indexRangesBmp[4];
    GT_U32                                      cncBlockCntrs;
    GT_U32                                      cncBlocks;
    GT_U32                                      ii;
    PRV_TGF_CNC_CLIENT_ENT                      client;
    GT_U32                                      cncBlockNum = 1;
    GT_STATUS                                   rc;
    GT_U32                                      portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC               portCntrs;
    GT_U32                                      clientGroup = PCL_TCAM_GROUP_CNS;

    if (useTti)
    {
        clientGroup = TTI_TCAM_GROUP_CNS;
    }

    /* AUTODOC: Virtual Tcam Update Action Test configuration set
        1.        Create tcam manager with vtcam with 10 rules */
    prvTgfVirtualTcamTestConfigurationSet(clientGroup,
                    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E,
                    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E);

    /* AUTODOC: Configure PCL rules
        2.  Make rule #8 with action to count to CNC index 0 (redirect to single egress port)
        3.  Make rule #9 Make SAME content of rule like #8 with action (bypass bridge + flood)
    */
    if (clientGroup == PCL_TCAM_GROUP_CNS)
    {
        prvTgfVirtualTcamUpdateActionTestPclRulesAndActionsSet();
    }
    else
    {
        prvTgfVirtualTcamUpdateActionTestTtiRulesAndActionsSet();
    }

    /* get CNC size info */
    prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

    /* use first range */
    indexRangesBmp[0] = 0x1;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    {
        /* Get CNC client for a block */
        client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;

        /* configure CNC */
        rc = prvTgfCncTestCncBlockConfigure(
            cncBlockNum,
            client,
            GT_TRUE /*enable*/,
            indexRangesBmp,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfCncTestCncBlockConfigure: block %d, client %d",
                cncBlockNum,
                client);
    }
    {
        /* Get CNC client for a block */
        client = PRV_TGF_CNC_CLIENT_TTI_E;

        /* configure CNC */
        rc = prvTgfCncTestCncBlockConfigure(
            cncBlockNum,
            client,
            GT_TRUE /*enable*/,
            indexRangesBmp,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfCncTestCncBlockConfigure: block %d, client %d",
                cncBlockNum,
                client);
    }

    /* AUTODOC: enable PCL clients for CNC */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS ))
    {
        rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
        rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    }

    /* reset counters */
    rc = prvTgfVirtualTcamUpdateActionPortAndCncCntrsCheckOrReset(
        prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE,
        PRV_TGF_VIRTUAL_TCAM_TEST_NUM_ITERATIONS,
        PRV_TGF_VIRTUAL_TCAM_TEST_ITERATIONS_STEP);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamUpdateActionPortAndCncCntrsReset");

    /* AUTODOC: configure and generate FWS
        5.        Send continous WS traffic that hit this rule */
    rc = prvTgfVirtualTcamUpdateActionTrafficGenerate(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], &prvTgfPacketInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamUpdateActionTrafficGenerate: port %d", prvTgfPortsArray[ING_PORT_IDX_CNS]);

    for (ii = PRV_TGF_VIRTUAL_TCAM_TEST_ITERATIONS_STEP;
            ii < PRV_TGF_VIRTUAL_TCAM_TEST_NUM_ITERATIONS;
            ii += PRV_TGF_VIRTUAL_TCAM_TEST_ITERATIONS_STEP)
    {
        /* AUTODOC: 6a.        For simulation do sleep 50 millisec (to allow traffic hit the entry) */
        PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_ON_SIMULATION_MAC(50);

        /* AUTODOC: 6.        Updated the action of rule 8  to count to CNC index 1
            7.        Updated the action of rule 8 to count to CNC index 2
            8.        ..
            9.        Updated the action of rule 8 to count to CNC index 511 */
        /* AUTODOC: action0 - count to CNC index 1, redirect to port A */
        if (clientGroup == PCL_TCAM_GROUP_CNS)
        {
            cpssOsMemSet(&action, 0, sizeof(action));
            action.pktCmd                                            = PRV_TGF_PACKET_CMD_FORWARD_E;
            action.mirror.cpuCode                                    = CPSS_NET_FIRST_USER_DEFINED_E;
            action.redirect.redirectCmd                              = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            action.redirect.data.outIf.outInterface.type             = CPSS_INTERFACE_PORT_E;
            action.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
            action.redirect.data.outIf.outInterface.devPort.portNum =
                prvTgfPortsArray[EGR_PORT_A];
            action.matchCounter.enableMatchCount                    = GT_TRUE;
            action.matchCounter.matchCounterIndex                   = ii;

            action.bypassBridge = GT_TRUE;

            prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);
            actionType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
            actionType.action.pcl.direction = CPSS_PCL_DIRECTION_INGRESS_E;
            actionData.action.pcl.actionPtr = &dxChAction;
        }
        else
        {
            cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
            ttiAction.command                                           = CPSS_PACKET_CMD_FORWARD_E;
            ttiAction.userDefinedCpuCode                                = CPSS_NET_FIRST_USER_DEFINED_E;
            ttiAction.redirectCommand                                   = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
            ttiAction.egressInterface.type                              = CPSS_INTERFACE_PORT_E;
            prvUtfHwDeviceNumberGet(prvTgfDevNum, &ttiAction.egressInterface.devPort.hwDevNum);
            ttiAction.egressInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_A];
            ttiAction.bindToCentralCounter                              = GT_TRUE;
            ttiAction.centralCounterIndex                               = ii;
            ttiAction.tag1VlanCmd                                       = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;

            ttiAction.bridgeBypass = GT_TRUE;
            actionType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
            actionData.action.tti.actionPtr = &ttiAction;
        }

        rc = cpssDxChVirtualTcamRuleActionUpdate(V_TCAM_MANAGER, V_TCAM_1, 8, &actionType, &actionData);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "cpssDxChVirtualTcamRuleMove failed: rc %d src %d dst %d\n",
                    rc, 8, 498);

    }


    /* AUTODOC: stop traffic
        10.        Stop the traffic */
    rc = prvTgfVirtualTcamUpdateActionTrafficStop(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop");

    /* AUTODOC: Compare Port and CNC counters
        11.        Check that the CNC summary of indexes 0..511 is equal to number of packets that egress the device. */
    rc = prvTgfVirtualTcamUpdateActionPortAndCncCntrsCheckOrReset(prvTgfPortsArray[ING_PORT_IDX_CNS], GT_TRUE,
                                                                  PRV_TGF_VIRTUAL_TCAM_TEST_NUM_ITERATIONS,
                                                                  PRV_TGF_VIRTUAL_TCAM_TEST_ITERATIONS_STEP);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamUpdateActionPortAndCncCntrsReset");

    /* AUTODOC: 12.        Check that there was no flooding to other ports. */
    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* skip ingress port */
        if (portIter == ING_PORT_IDX_CNS)
        {
            continue;
        }
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (portIter == EGR_PORT_A)
        {
            /* check WS traffic by compare with some value */
            if (portCntrs.goodPktsSent.l[0] < 10)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(10, portCntrs.goodPktsSent.l[0],
                                             "get another goodPktsSent counter than expected on port %d",
                                             prvTgfPortsArray[portIter]);
            }
        }
        else
        {
            /* check Tx counters. Should be 0 - no flooding. */
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }
}

/**
* @internal prvTgfVirtualTcamUpdateActionConfigurationRestore function
* @endinternal
*
* @brief   Virtual Tcam Update Action configurations restore
*/
GT_VOID prvTgfVirtualTcamUpdateActionConfigurationRestore
(
   GT_VOID
)
{
    GT_STATUS                           rc;
    GT_U32                              floorIndex;
    GT_U16                               jj;
    GT_U32                           indexRangesBmp[4];
    GT_U32                           cncBlockCntrs;
    GT_U32                           cncBlocks;
    GT_U32                           cncBlockNum = 1;
    PRV_TGF_CNC_CLIENT_ENT           client;
    GT_U32                           counterIdx;
    PRV_TGF_CNC_COUNTER_STC          counter;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning;

    /* AUTODOC: RESTORE CONFIGURATION: */

    cpssOsMemSet(&counter, 0, sizeof(counter));

    /* get CNC size info */
    prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

    /* use first range */
    indexRangesBmp[0] = 0x0;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    {
        /* Get CNC client for a block */
        client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;

        /* disable client */
        rc = prvTgfCncBlockClientEnableSet(
            cncBlockNum, client, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", cncBlockNum, client);

        /* reset ranges */
        rc = prvTgfCncBlockClientRangesSet(
            cncBlockNum, client, indexRangesBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", cncBlockNum, client);
    }
    {
        /* Get CNC client for a block */
        client = PRV_TGF_CNC_CLIENT_TTI_E;

        /* disable client */
        rc = prvTgfCncBlockClientEnableSet(
            cncBlockNum, client, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", cncBlockNum, client);

        /* reset ranges */
        rc = prvTgfCncBlockClientRangesSet(
            cncBlockNum, client, indexRangesBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", cncBlockNum, client);
    }
    /* clean counters */
    for (counterIdx = 0; counterIdx < cncBlockCntrs; counterIdx++)
    {
        rc = prvTgfCncCounterSet(
            cncBlockNum, counterIdx,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncCounterSet: block %d idx %d", cncBlockNum, counterIdx);
    }

    /* disable PCL clients for CNC */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA3_E | UTF_CPSS_PP_E_ARCH_CNS ))
    {
        rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
        rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E,GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    }

    /* AUTODOC: Remove vTCAMs with PCL rules  */
    jj = 8;
    rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, jj);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                 V_TCAM_1, jj);

    jj = 9;
    rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, jj);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                 V_TCAM_1, jj);

    rc = cpssDxChVirtualTcamRemove(V_TCAM_MANAGER, V_TCAM_1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRemove(vTCAM:%d) failed\n", V_TCAM_1);

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy for port 1 (the ingress port) */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[ING_PORT_IDX_CNS],
                                 GT_FALSE);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: restore TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, ttiTcamLookupEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore MAC mode for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, ttiTcamMacMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Restore TCAM floor info */
        for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet(); floorIndex++ )
        {
            rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex,
                                                  &saveFloorInfoArr[floorIndex][0]);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
                floorIndex);
        }

        if(HARD_WIRE_TCAM_MAC(prvTgfDevNum))
        {
            /* BC3 uses ipcl0 as 'group 1' */
        }
        else
        {
            /* AUTODOC: Restore PCL TCAM group binding */
            rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);
            UTF_VERIFY_EQUAL3_STRING_MAC(
                    GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
                    PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);

            /* AUTODOC: Restore TTI TCAM group binding */
            rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);
            UTF_VERIFY_EQUAL3_STRING_MAC(
                    GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
                    PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);
        }
    }

    rc = cpssDxChVirtualTcamManagerDevListRemove(V_TCAM_MANAGER, &prvTgfDevNum, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                 V_TCAM_MANAGER);

    rc = cpssDxChVirtualTcamManagerDelete(V_TCAM_MANAGER);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                 V_TCAM_MANAGER);

    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* Restore TCAM Active number of floors */
        rc = cpssDxChTcamActiveFloorsSet(prvTgfDevNum,
                fineTuningPtr->tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);

        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "cpssDxChTcamActiveFloorsSet failed for device: %d", prvTgfDevNum);
    }
}

/**
* @internal prvTgfVirtualTcamResizeTestRulesAndActionsSet function
* @endinternal
*
* @brief   None
*
* @param[in] ruleSize                 - Rule size to be tested (currently only 30 and 80 bytes supported)
*                                       None
*/
static GT_VOID prvTgfVirtualTcamResizeTestRulesAndActionsSet
(
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    PRV_TGF_PCL_ACTION_STC                     action;
    CPSS_DXCH_PCL_ACTION_STC                   dxChAction;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType; /*for 8,9 rules */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             ruleId;

    prvTgfVirtualTcamPclBuildRule (ruleSize, &mask, &pattern);

    /* AUTODOC: action0 - redirect to port A */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                                            = PRV_TGF_PACKET_CMD_FORWARD_E;
    action.mirror.cpuCode                                    = CPSS_NET_FIRST_USER_DEFINED_E;
    action.redirect.redirectCmd                              = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type             = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum  = prvTgfPortsArray[EGR_PORT_A];

    action.bypassBridge = GT_TRUE;

    ruleData.valid                   = GT_TRUE;
    prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);
    ruleData.rule.pcl.actionPtr      = &dxChAction;
    ruleData.rule.pcl.maskPtr        = &mask;
    ruleData.rule.pcl.patternPtr     = &pattern;
    tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
    if (ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
    {
        tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
    }
    else
    {
        tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    }

    /*4. Priority 5 - Make rule #8 with action to redirect to single egress port A */
    ruleAttributes.priority      = 5;
    ruleId = 8;

    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, V_TCAM_1, ruleId,
                                          &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                     V_TCAM_1, ruleId);

    /* 5. Priority 6 - Make SAME content of rule #9 (like #8)
     * with action to redirect to single egress port B */
    action.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[EGR_PORT_B];
    prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);

    ruleAttributes.priority      = 6;
    ruleId = 9;
    rc = cpssDxChVirtualTcamRuleWrite(V_TCAM_MANAGER, V_TCAM_1, ruleId,
                                      &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                                 V_TCAM_1, ruleId);

}

/**
* @internal prvTgfVirtualTcamResizeTestCncConfig function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfVirtualTcamResizeTestCncConfig
(
        GT_VOID
)
{
    GT_STATUS                       rc;
    GT_BOOL                         droppedPacket = GT_TRUE;
    GT_BOOL                         cnMode = GT_FALSE;
    PRV_TGF_CNC_CONFIGURATION_STC   cncCfg;
    GT_U32                          txqIndex;
    GT_U32                          blockSize;
    CPSS_QOS_ENTRY_STC              qosEntry;
    PRV_TGF_COS_PROFILE_STC         qosProfile;

    blockSize = prvTgfCncFineTuningBlockSizeGet();

    txqIndex = prvTgfCncTxqClientIndexGet(
        prvTgfDevNum, cnMode, droppedPacket,
        prvTgfPortsArray[EGR_PORT_A] /*portNum*/,
        PRV_TGF_CNC_GEN_DEF_TC_CNS,
        PRV_TGF_CNC_GEN_DEF_DP_CNS);

    /* Qos Configuration */
        /* DxCh version */

    cpssOsMemSet(&qosProfile, 0, sizeof(qosProfile));
    qosProfile.dropPrecedence = PRV_TGF_CNC_GEN_DEF_DP_CNS;
    qosProfile.trafficClass   = PRV_TGF_CNC_GEN_DEF_TC_CNS;

    cpssOsMemSet(&qosEntry, 0, sizeof(qosEntry));
    qosEntry.assignPrecedence =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    qosEntry.enableModifyDscp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    qosEntry.enableModifyUp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    qosEntry.qosProfileId = PRV_TGF_CNC_GEN_DXCH_TEST_QOS_PROFILE0;

    rc = prvTgfCosProfileEntrySet(
        PRV_TGF_CNC_GEN_DXCH_TEST_QOS_PROFILE0, &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");

    rc = prvTgfCosPortQosConfigSet(
        prvTgfPortsArray[EGR_PORT_A], &qosEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet");

    rc = prvTgfCosPortQosTrustModeSet(
        prvTgfPortsArray[EGR_PORT_A],
        CPSS_QOS_PORT_NO_TRUST_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCosPortQosTrustModeSet");

    /* CNC Configuration */

    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    /* common parameters */
    cncCfg.clientType    = PRV_TGF_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E;
    cncCfg.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCfg.counterNum    = (txqIndex % blockSize);
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncBlockClientRangesBmpArrSet */
    cncCfg.configureClientEnable     = GT_TRUE;
    cncCfg.clientEnable              = GT_TRUE;
    /* prvTgfCncCounterFormatSet */
    cncCfg.configureIndexRangeBitMap = GT_TRUE;
    cncCfg.indexRangeBitMap[0]       =
        (1 << (txqIndex / blockSize)); /* [1,2,3] - zeros */
    /* prvTgfCncCounterSet */
    cncCfg.configureCounterValue = GT_TRUE; /*counterValue == 0*/

    /* prvTgfCncClientByteCountModeSet */
    cncCfg.configureByteCountMode = GT_TRUE;
    cncCfg.byteCountMode = PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E;

    /* prvTgfCncEgressQueueClientModeSet */
    if (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(prvTgfDevNum))
    {
        cncCfg.configureEgressQueueClientMode = GT_TRUE;
        cncCfg.egressQueueClientMode =
            (cnMode == GT_FALSE)
               ? PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E
               : PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E;
    }

    PRV_UTF_LOG3_MAC(
        "blockSize %d, txqIndex %d, cncCfg.indexRangeBitMap[0] 0x%X\n",
        blockSize, txqIndex, cncCfg.indexRangeBitMap[0]);

    rc = prvTgfCncGenConfigure(
        &cncCfg, GT_TRUE /*stopOnErrors*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncGenConfigure");

}

/**
* @internal prvTgfVirtualTcamResizeTestCheck function
* @endinternal
*
* @brief   Function performs ingress vs. egress port counters check.
*
* @param[in] expPacketFld             - expected number of packets on "other" vlan ports:
*                                      1 on flooding, 0 on forwarding.
*                                       None
*/
static GT_VOID prvTgfVirtualTcamResizeTestCheck
(
    IN GT_U32           expPacketFld
)
{
    GT_STATUS                       rc;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          ingPortGoodPktsRcv = (GT_U32) -1;
    GT_U32                          ingPortDropEvents = 0;
    GT_U32                          egrPortGoodPktsSent = (GT_U32) -2;
    GT_BOOL                         droppedPacket = GT_TRUE;
    GT_BOOL                         cnMode = GT_FALSE;
    GT_U32                          txqIndex;
    GT_U32                          blockSize;
    PRV_TGF_CNC_CHECK_STC           cncCheck;

    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    GT_U8           devNum;
    GT_U8           queue;
    TGF_NET_DSA_STC rxParam;

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (ING_PORT_IDX_CNS == portIter)
        {
            /* Save Rx counter */
            ingPortGoodPktsRcv = portCntrs.goodPktsRcv.l[0];

            /* non MTI MAC counts Drop Events in Good Rx also. MTI MACs do not count them  */
            if (GT_FALSE == prvUtfIsPortMacTypeMti(prvTgfDevNum, prvTgfPortsArray[portIter]))
            {
                ingPortDropEvents = portCntrs.dropEvents.l[0];
            }
            else
            {
                ingPortDropEvents = 0;
            }
        }
        else
        if ( EGR_PORT_A == portIter)
        {
            /* Save Tx counter */
            egrPortGoodPktsSent = portCntrs.goodPktsSent.l[0];
        }
        else
        {
            /* check non-flooding counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(expPacketFld, portCntrs.goodPktsSent.l[0],
                                         "get different goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }

    /* get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet first");

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet next");

    /* check Rx vs. Tx (good packets on MAC + egress drops on CNC) counters */
    blockSize = prvTgfCncFineTuningBlockSizeGet();

    txqIndex = prvTgfCncTxqClientIndexGet(
        prvTgfDevNum, cnMode, droppedPacket,
        prvTgfPortsArray[EGR_PORT_A] /*portNum*/,
        PRV_TGF_CNC_GEN_DEF_TC_CNS,
        PRV_TGF_CNC_GEN_DEF_DP_CNS);

    cpssOsMemSet(&cncCheck, 0, sizeof(cncCheck));
    /* common parameters */
    cncCheck.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCheck.counterNum    = (txqIndex % blockSize);
    cncCheck.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncCounterGet */
    cncCheck.compareCounterValue = GT_TRUE;
    /* All packets unaccounted for must be egress dropped packets */
    cncCheck.counterValue.packetCount.l[0] = (ingPortGoodPktsRcv - ingPortDropEvents) - egrPortGoodPktsSent;

    if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(prvTgfDevNum) && (GT_TRUE == prvUtfIsTrafficManagerUsed()))
    {
        /* Cetus with TM use TM for all ports. There is no tail drop events in TXQ for the system. */
        cncCheck.counterValue.packetCount.l[0] = 0;
    }

    /* don't compare stamp */
    cncCheck.counterValue.byteCount.l[0] = 0xFFFFFFFF;
    cncCheck.counterValue.byteCount.l[1] = 0xFFFFFFFF;

    rc = prvTgfCncGenCheck(&cncCheck);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfCncGenCheck port: %d, txqIndex: 0x%04x",
                        prvTgfPortsArray[EGR_PORT_A], txqIndex);

}

/**
* @internal prvTgfVirtualTcamResizeTest function
* @endinternal
*
* @brief   Check that resize under traffic not losing packets.-
*         logical index mode
*         PURPOSE:
*         1. Check that resize under traffic not losing packets.
*         LOGIC:
*         1.    Create vtcam , guaranteed 10 entries (30B rules)
*         2.    Write rule#9 as redirect to egresss port A
*         3.    Send WS traffic to hit this entry
*         4.    Resize vtcam adding 1000 entries from index 0
*         5.    Sleep 100 millisec
*         6.    Resize vtcam adding 1000 entries from index 0
*         a.    Sleep 100 millisec
*         7.    Resize vtcam removing 500 entries from index 0
*         a.    Sleep 100 millisec
*         8.    Repeat 7. three more times (total 2000 removed)
*         9.    Check no flooding in the vlan (all egress port A)
* @param[in] ruleSize                 - Rule size to be tested (currently only 30 and 80 bytes supported)
* @param[in] ruleAdditionMethod       - Either priority or logical index method
*                                       None
*/

GT_VOID prvTgfVirtualTcamResizeTest
(
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_ENT ruleAdditionMethod
)
{
    TGF_PACKET_STC                 *packetInfoPtr;
    GT_U32                          totalRulesAllowed = 0;
    GT_U32                          sizeInRules;
    GT_U32                          clientGroup = PCL_TCAM_GROUP_CNS;
    GT_STATUS                                   rc;

    internal_ut_cpssDxChVirtualTcamTotalRulesAllowedPerDevGet(
                prvTgfDevNum,
                ruleSize,
                &totalRulesAllowed);
    sizeInRules =  totalRulesAllowed / 12;
    sizeInRules -= (sizeInRules % 12);

    /* AUTODOC: Virtual Tcam Resize Test configuration set
        1.        Create vtcam , guaranteed 10 entries (30B or 80B rules) */
    prvTgfVirtualTcamTestConfigurationSet(clientGroup, ruleSize, ruleAdditionMethod);

    /* AUTODOC: Configure PCL rules
        2.  Write rule#9 as redirect to egresss port A
    */
    prvTgfVirtualTcamResizeTestRulesAndActionsSet(ruleSize);
    prvTgfVirtualTcamResizeTestCncConfig();

    /* AUTODOC: configure and generate FWS
        3.  Send WS traffic to hit this entry */
    if (ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E)
    {
        packetInfoPtr = &prvTgfIpv6PacketInfo;
    }
    else
    {
        packetInfoPtr = &prvTgfPacketInfo;
    }
    rc = prvTgfVirtualTcamUpdateActionTrafficGenerate(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], packetInfoPtr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamUpdateActionTrafficGenerate: port %d", prvTgfPortsArray[ING_PORT_IDX_CNS]);


    /* AUTODOC: resize (increase)
        4.        Resize vtcam adding 1000 entries from index 0 */
    rc = cpssDxChVirtualTcamResize(V_TCAM_MANAGER, V_TCAM_1, 0, GT_TRUE, sizeInRules);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamResize (increase) failed\n");

    /* AUTODOC: allow traffic
        5.        Sleep 100 millisec */
    PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_MAC(100);

    /* AUTODOC: resize (increase)
        6.        Resize vtcam adding 1000 entries from index 0 */
    rc = cpssDxChVirtualTcamResize(V_TCAM_MANAGER, V_TCAM_1, 0, GT_TRUE, sizeInRules);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamResize (increase) failed\n");

    /* AUTODOC: allow traffic
        6.a.        Sleep 100 millisec */
    PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_MAC(100);

    /* AUTODOC: resize (decrease)
        7.        Resize vtcam removing 500 entries from index 0 */
    rc = cpssDxChVirtualTcamResize(V_TCAM_MANAGER, V_TCAM_1, 0, GT_FALSE, sizeInRules / 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamResize (decrease) failed\n");

    /* AUTODOC: allow traffic
        7.a.        Sleep 100 millisec */
    PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_MAC(100);

    /* AUTODOC: resize (decrease)
        8.        Repeat 7. three more times (total 2000 removed) */
    rc = cpssDxChVirtualTcamResize(V_TCAM_MANAGER, V_TCAM_1, 0, GT_FALSE, sizeInRules / 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamResize (decrease) failed\n");
    PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_MAC(100);

    rc = cpssDxChVirtualTcamResize(V_TCAM_MANAGER, V_TCAM_1, 0, GT_FALSE, sizeInRules / 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamResize (decrease) failed\n");
    PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_MAC(100);

    rc = cpssDxChVirtualTcamResize(V_TCAM_MANAGER, V_TCAM_1, 0, GT_FALSE, sizeInRules / 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamResize (decrease) failed\n");
    PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_MAC(100);

    /* AUTODOC: stop traffic */
    rc = prvTgfVirtualTcamUpdateActionTrafficStop(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop");

    /* AUTODOC: Compare Port counters
        9.  Check no flooding in the vlan (all egress port A). */
    prvTgfVirtualTcamResizeTestCheck(0);
}

/**
* @internal prvTgfVirtualTcamResizeConfigurationRestore function
* @endinternal
*
* @brief   Virtual Tcam Resize configurations restore
*/
GT_VOID prvTgfVirtualTcamResizeConfigurationRestore
(
   GT_VOID
)
{
    GT_STATUS                           rc;
    GT_BOOL                             droppedPacket = GT_TRUE;
    GT_BOOL                             cnMode = GT_FALSE;
    GT_U32                              txqIndex;
    GT_U32                              blockSize;
    GT_U32                              floorIndex;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID      ruleId;
    PRV_TGF_CNC_CONFIGURATION_STC       cncCfg;
    CPSS_QOS_ENTRY_STC                  qosEntry;
    PRV_TGF_COS_PROFILE_STC             qosProfile;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: Restore CNC and QOS configuration */
    blockSize = prvTgfCncFineTuningBlockSizeGet();

    txqIndex = prvTgfCncTxqClientIndexGet(
        prvTgfDevNum, cnMode, droppedPacket,
        prvTgfPortsArray[EGR_PORT_A] /*portNum*/,
        PRV_TGF_CNC_GEN_DEF_TC_CNS,
        PRV_TGF_CNC_GEN_DEF_DP_CNS);

    /* Qos Configuration */
    /* DxCh version */

    cpssOsMemSet(&qosProfile, 0, sizeof(qosProfile));

    cpssOsMemSet(&qosEntry, 0, sizeof(qosEntry));
    qosEntry.assignPrecedence =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    qosEntry.enableModifyDscp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    qosEntry.enableModifyUp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    qosEntry.qosProfileId = 0;

    rc = prvTgfCosProfileEntrySet(
        PRV_TGF_CNC_GEN_DXCH_TEST_QOS_PROFILE0, &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");

    rc = prvTgfCosPortQosConfigSet(
        prvTgfPortsArray[EGR_PORT_A], &qosEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet");

    rc = prvTgfCosPortQosTrustModeSet(
        prvTgfPortsArray[EGR_PORT_A],
        CPSS_QOS_PORT_NO_TRUST_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCosPortQosTrustModeSet");

    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));
    /* common parameters */
    cncCfg.clientType    = PRV_TGF_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E;
    cncCfg.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCfg.counterNum    = (txqIndex % blockSize);
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    /* prvTgfCncBlockClientRangesBmpArrSet */
    cncCfg.configureClientEnable     = GT_TRUE;
    cncCfg.clientEnable              = GT_FALSE;
    /* prvTgfCncCounterFormatSet */
    cncCfg.configureIndexRangeBitMap = GT_TRUE; /* indexRangeBitMap[0,1,2,3] - zeros */
    /* prvTgfCncCounterSet */
    cncCfg.configureCounterValue = GT_TRUE; /*counterValue == 0*/

    /* prvTgfCncClientByteCountModeSet */
    cncCfg.configureByteCountMode = GT_TRUE;
    cncCfg.byteCountMode = PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E;

    /* prvTgfCncEgressQueueClientModeSet */
    if (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(prvTgfDevNum))
    {
        cncCfg.configureEgressQueueClientMode = GT_TRUE;
        cncCfg.egressQueueClientMode =
            PRV_TGF_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E;
    }

    prvTgfCncGenConfigure(
        &cncCfg, GT_FALSE /*stopOnErrors*/);

    /* AUTODOC: Remove vTCAMs with PCL rules  */
    ruleId = 8;
    rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, ruleId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                 V_TCAM_1, ruleId);

    ruleId = 9;
    rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, ruleId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
                                 V_TCAM_1, ruleId);

    rc = cpssDxChVirtualTcamRemove(V_TCAM_MANAGER, V_TCAM_1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRemove(vTCAM:%d) failed\n", V_TCAM_1);

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy for port 1 (the ingress port) */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[ING_PORT_IDX_CNS],
                                 GT_FALSE);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Restore TCAM floor info */
        for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet() ; floorIndex++ )
        {
            rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex,
                                                  &saveFloorInfoArr[floorIndex][0]);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
                floorIndex);
        }

        if(HARD_WIRE_TCAM_MAC(prvTgfDevNum))
        {
            /* BC3 uses ipcl0 as 'group 1' */
        }
        else
        {
            /* AUTODOC: Restore PCL TCAM group binding */
            rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);
            UTF_VERIFY_EQUAL3_STRING_MAC(
                    GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
                    PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);
        }
    }

    rc = cpssDxChVirtualTcamManagerDevListRemove(V_TCAM_MANAGER, &prvTgfDevNum, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                 V_TCAM_MANAGER);

    rc = cpssDxChVirtualTcamManagerDelete(V_TCAM_MANAGER);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                 V_TCAM_MANAGER);
    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* Restore TCAM Active number of floors */
        rc = cpssDxChTcamActiveFloorsSet(prvTgfDevNum,
                fineTuningPtr->tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);

        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "cpssDxChTcamActiveFloorsSet failed for device: %d", prvTgfDevNum);
    }
}

/**
* @internal prvTgfVirtualTcamDraggingConfigurationRestore function
* @endinternal
*
* @brief   Virtual Tcam Dragging configurations restore
*/
GT_VOID prvTgfVirtualTcamDraggingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                        rc;
    GT_U32                           floorIndex;
    GT_U32                           idx;
    GT_U32                           indexRangesBmp[4];
    GT_U32                           cncBlockCntrs;
    GT_U32                           cncBlocks;
    GT_U32                           ii;
    PRV_TGF_CNC_CLIENT_ENT           client;
    GT_U32                           counterIdx;
    PRV_TGF_CNC_COUNTER_STC          counter;
    VT_SEQUENCE                      *sequencePtr;
    GT_U32                           sequenceSize;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC  vTcamInfo;
    PRV_TGF_PCL_UDB_SELECT_STC       udbSelect;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    GT_U8                            devNum = prvTgfDevNum;
    GT_PHYSICAL_PORT_NUM             portNum;              /* port number */
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning;

    /* stop traffic and restore port/vlan config */
    prvTgfFWSRestore();

    /* provide time for event task to manage port link change procedures */
    cpssOsTimerWkAfter(500);

    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));

     /* AUTODOC: RESTORE CONFIGURATION: */
    rc = prvTgfPclUserDefinedBytesSelectSet(
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                CPSS_PCL_LOOKUP_0_0_E, &udbSelect);
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        GT_FAIL, rc,
        "prvTgfPclUserDefinedBytesSelectSet failed for  CPSS_PCL_LOOKUP_0_0_E\n");

    rc = prvTgfPclUserDefinedBytesSelectSet(
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                CPSS_PCL_LOOKUP_1_E, &udbSelect);
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        GT_FAIL, rc,
        "prvTgfPclUserDefinedBytesSelectSet failed for  CPSS_PCL_LOOKUP_1_E\n");

    /* AUTODOC: Remove vTCAMs with PCL rules  */
    sequencePtr = draggRestoreTest_sequence;
    sequenceSize = sizeof(draggRestoreTest_sequence) / sizeof(draggRestoreTest_sequence[0]);

    for (idx = startSequenceFrom; (idx < sequenceSize); idx++)
    {
        rc = cpssDxChVirtualTcamInfoGet(V_TCAM_MANAGER,sequencePtr[idx].vTcamId, &vTcamInfo);

        /*  Delete all rules before removing vTCAM */
        vTcamDeleteConsecutiveRules(V_TCAM_MANAGER,
                                sequencePtr[idx].vTcamId,
                                CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS,
                                DELETE_ALL_CNS);

        rc = vtcamSequenceEntryProcess(sequencePtr, idx, V_TCAM_MANAGER, &vTcamInfo);
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(
            GT_FAIL, rc,
            "vtcamSequenceEntryProcess failed, index: %d\n", idx);
    }

    cpssOsMemSet(&counter, 0, sizeof(counter));

    /* get CNC size info */
    prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

    /* use first range */
    indexRangesBmp[0] = 0x0;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    for (ii = 0; ii < cncBlocks; ii++)
    {
        /* Get CNC client for a block */
        client = prvTgfPclCncClientGet(cncBlocks, ii);

        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* limit SIP_6 only by two PCL lookups */
            if (client != PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E &&
                client != PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E)
            {
                continue;
            }
        }

        /* disable client */
        rc = prvTgfCncBlockClientEnableSet(
            ii, client, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", ii, client);

        /* reset ranges */
        rc = prvTgfCncBlockClientRangesSet(
            ii, client, indexRangesBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", ii, client);

        /* clean counters */
        for (counterIdx = 0; counterIdx < cncBlockCntrs; counterIdx++)
        {
            rc = prvTgfCncCounterSet(
                ii, counterIdx,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_2_E, &counter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncCounterSet: block %d idx %d", ii, counterIdx);
        }
    }

    /* disable PCL clients for CNC */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA3_E | UTF_CPSS_PP_E_ARCH_CNS ))
    {
        rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    }

    rc = cpssDxChVirtualTcamManagerDevListRemove(V_TCAM_MANAGER, &prvTgfDevNum, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                 V_TCAM_MANAGER);

    rc = cpssDxChVirtualTcamManagerDelete(V_TCAM_MANAGER);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                 V_TCAM_MANAGER);

    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* Restore TCAM Active number of floors */
        rc = cpssDxChTcamActiveFloorsSet(prvTgfDevNum,
                fineTuningPtr->tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);

        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "cpssDxChTcamActiveFloorsSet failed for device: %d", prvTgfDevNum);
    }

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Restore TCAM floor info */
    for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet() ; floorIndex++ )
    {
        rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex,
                                              &saveFloorInfoArr[floorIndex][0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
            floorIndex);
    }

    /* AUTODOC: Disable PCL on all ports */
    rc = prvUtfNextMacPortReset(&portNum, devNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG0_MAC("prvUtfNextMacPortReset failure 1 \n");
        return;
    }

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup = GT_FALSE;
    lookupCfg.dualLookup = GT_FALSE;

    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type = CPSS_INTERFACE_PORT_E;

    /* For device go over all available MAC physical ports. */
    while(GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
    {
        if (portNum >= (CPSS_MAX_PORTS_NUM_CNS-1))
        {
            PRV_UTF_LOG1_MAC("port too big  %d\n", portNum);
            return;
        }

        prvTgfPortsArray[0] = portNum;

        if (prvUtfIsTrafficManagerUsed() && (portNum >= 128))
        {
            /* skip interlacken channels */
            continue;
        }

        interfaceInfo.devPort.hwDevNum = prvTgfDevNum;
        interfaceInfo.devPort.portNum = portNum;

        lookupCfg.groupKeyTypes.nonIpKey =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        lookupCfg.groupKeyTypes.ipv4Key =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
        lookupCfg.groupKeyTypes.ipv6Key =
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

        /* disable IPCL 0_0 and IPCL 0_1 lookups */
        rc = prvTgfPclCfgTblSet(
            &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E, &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

        rc = prvTgfPclPortIngressPolicyEnable(portNum, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
            prvTgfDevNum, portNum);

        if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* disable IPCL 1 lookup */
            rc = prvTgfPclCfgTblSet(
                &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                CPSS_PCL_LOOKUP_1_E, &lookupCfg);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");
        }
    }

    /* restore port configuration according to device family */
    prvTgfPortsArrayByDevTypeSet();
}

/**
* @internal prvTgfPclFWSPortAndCncCntrsCheckOrReset function
* @endinternal
*
* @brief   Check that CNC counter is same as Rx one.
*         Or reset both CNC and Rx counters
* @param[in] portRxRateArr[CPSS_MAX_PORTS_NUM_CNS] - Rx rate of port in packets per second, may be NULL
* @param[in] check                    - if to check/reset Rx counters
*
* @retval GT_OK                    - on OK
* @retval GT_FAIL                  - on failure
*/
static GT_STATUS  prvTgfPclFWSPortAndCncCntrsCheckOrReset
(
  IN   GT_U32   portRxRateArr[CPSS_MAX_PORTS_NUM_CNS],
  IN   GT_BOOL  check
)
{
    GT_PHYSICAL_PORT_NUM    portNum;   /* ingress port number */
    GT_STATUS               rc = GT_OK; /* return code */
    GT_STATUS               rc1 = GT_OK; /* return code */
    GT_U8                   devNum = prvTgfDevNum; /* device number */
    GT_U32                  cncBlockCntrs;   /* number of CNC counters per block */
    GT_U32                  cncBlocks;       /* CNC blocks number */
    GT_U32                  ii;              /* iterator */
    PRV_TGF_CNC_CLIENT_ENT  client;          /* CNC client */
    GT_U32                  counterIdx;      /* counter index */
    PRV_TGF_CNC_COUNTER_STC cncCounter;         /* counter */
    GT_PORT_GROUPS_BMP      portGroupBmp;    /* port group bitmap */
    GT_U64                  portCntrValue;   /* value of port counters */
    GT_U64                  portDropCntrValue;   /* value of port drop event counters */
    GT_U32                  portCntrArr[CPSS_MAX_PORTS_NUM_CNS]; /* value of counter per port */

    cpssOsMemSet(portCntrArr, 0xFE, sizeof(portCntrArr));

    /* get CNC size info */
    prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

    for (ii = 0; ii < cncBlocks; ii++)
    {
        /* Get CNC client for a block */
        client = prvTgfPclCncClientGet(cncBlocks, ii);

        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            /* limit Falcon only by two PCL lookups */
            if (client != PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E &&
                client != PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E)
            {
                continue;
            }
        }
        else
        {
            if (client != PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E &&
                client != PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E)
            {
                continue;
            }
        }

        if (prvTgfPclFwsUseSeparateTcamBlockPerLookupGet() && (client != PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E))
        {
            continue;
        }

        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if (portRxRateArr[portNum] == 0)
            {
                /* skip not used port */
                continue;
            }

            /* use PortGroupsBmp API in the test */
            /* set <currPortGroupsBmp> */
            TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(portNum);
            portGroupBmp = currPortGroupsBmp;

            /* restore values after using currPortGroupsBmp*/
            usePortGroupsBmp = GT_FALSE;
            currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

            counterIdx = portNum;
            if (client == PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E ||
                client == PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E)
            {
                counterIdx += 100;
            }

            rc = prvTgfCncPortGroupCounterGet(portGroupBmp,
                ii, counterIdx,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_2_E, &cncCounter);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG3_MAC("prvTgfCncCounterGet: block %d idx %d portGroupBmp %d\n", ii, counterIdx,portGroupBmp);
                return rc;
            }

            rc = prvTgfPortMacCounterGet(devNum, portNum,
                                          CPSS_GOOD_UC_PKTS_RCV_E,
                                          &portCntrValue);
            if (rc != GT_OK)
            {
                PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet: dev %d port %d\n", devNum, portNum);
                return rc;
            }

            /* non MTI MAC counts Drop Events in Good Rx also. MTI MACs do not count them  */
            if (GT_FALSE == prvUtfIsPortMacTypeMti(devNum, portNum))
            {
                rc = prvTgfPortMacCounterGet(devNum, portNum,
                                              CPSS_DROP_EVENTS_E,
                                              &portDropCntrValue);
                if (rc != GT_OK)
                {
                    PRV_UTF_LOG2_MAC("prvTgfPortMacCounterGet: dev %d port %d\n", devNum, portNum);
                    return rc;
                }
            }
            else
            {
                portDropCntrValue.l[0] = portDropCntrValue.l[1] = 0;
            }

            if (check)
            {
                if (client == PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E)
                {
                    /* port counter value is valid only for first read
                       because it get reset on read.
                       store value of port counter after first read. */
                    portCntrArr[portNum] = portCntrValue.l[0] - portDropCntrValue.l[0];
                }

                if (prvTgfPclFwsDebug)
                {
                    PRV_UTF_LOG4_MAC("DBG block %d port %d cnc %d port %d\n", ii, portNum, cncCounter.packetCount.l[0], portCntrArr[portNum]);
                }

                if (portCntrArr[portNum] != cncCounter.packetCount.l[0])
                {
                    PRV_UTF_LOG4_MAC("prvTgfFWSPclCncCheck failure block %d port %d cnc %d port %d\n", ii, portNum, cncCounter.packetCount.l[0], portCntrArr[portNum]);
                    rc1 = GT_FAIL;
                }
            }
        }
    }

    return rc1;
}

/**
* @internal prvTgfVirtualTcamDraggingTest function
* @endinternal
*
* @brief   Test basic moved rule between priorities (use traffic)-
*         priority mode
*         PURPOSE:
*         1. Check that the moved rule between priorities changed actual priority
*         LOGIC:
*         1. Create vTcam #100 to fill all Tcam except 2 Blocks
*         2. Create vTcams #1 #2 #3 #4 #10 #11
*         3. Delete vTcams #1 #2 #3 #4.
*         2. Set rules with different CNC index`s in vTcams #10 #11
*         that would be dragged
*         3. Generate traffic to hit this rules
*         4. Create vTcam #20 after dragging vTcam #10 #11
*         5. Stop traffic
*         6. Check the CNC counters
*/
GT_VOID prvTgfVirtualTcamDraggingTest
(
    GT_VOID
)
{
    GT_STATUS                       rc   = GT_OK;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC vTcamInfo;
    GT_U16                          vTcamId;
    GT_U16                          startVid = 5;/*first VLAN id for the test*/
    GT_U32                          packetSize = PRV_TGF_PACKET_FWS_CRC_LEN_CNS  + prvTgfPclFwsAddSize; /* packet size */
    GT_U32                          portRxRateArr[CPSS_MAX_PORTS_NUM_CNS];       /* rate of packets per port */

    cpssOsMemSet(portRxRateArr, 0, sizeof(portRxRateArr));

    prvTgfPacketFWSInfo.totalLen += prvTgfPclFwsAddSize;

    /* AUTODOC: Vitual Tcam Dragging Test configuration set*/
    prvTgfPclVirtualTcamDraggingTestConfigurationSet();

    /* configure and generate FWS */
    rc = prvTgfFWSLoopConfigTrafficGenerate(startVid, &prvTgfPacketFWSInfo, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate: vid %d", startVid);

    /* check FWS traffic */
    rc = prvTgfFWSTrafficCheck(packetSize, GT_TRUE, portRxRateArr);
    prvTgfFWSTrafficRatePrint(packetSize,portRxRateArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficCheck before PCL Rules: size %d", packetSize);

    /* AUTODOC: Configure PCL rules */
    prvTgfVirtualTcamPclRulesAndActionsSet_DraggingTest(portRxRateArr);

    /* the function above load the TCAM rules and decreases 100G ports rate in device like Bobcat3 and above.
       use generate FWS again to return the expected rate*/
    if (prvTgfPclFwsUseSeparateTcamBlockPerLookupGet())
    {
        /* and generate FWS */
        rc = prvTgfFWSLoopConfigTrafficGenerate(startVid, &prvTgfPacketFWSInfo, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate: vid %d", startVid);
    }

    utfPrintKeepAlive();

    /* check FWS traffic */
    rc = prvTgfFWSTrafficCheck(packetSize, GT_TRUE, portRxRateArr);
    prvTgfFWSTrafficRatePrint(packetSize,portRxRateArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficCheck after PCL: size %d", packetSize);

    utfPrintKeepAlive();

    /* stop traffic */
    rc = prvTgfFWSTrafficStop();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop");

    /* reset Port and CNC counters */
    rc = prvTgfPclFWSPortAndCncCntrsCheckOrReset(portRxRateArr, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclFWSPortAndCncCntrsReset");

    /* generate FWS */
    rc = prvTgfFWSLoopConfigTrafficGenerate(startVid, &prvTgfPacketFWSInfo, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfFWSLoopConfigTrafficGenerate: vid %d", startVid);

    /* Create vTCAM #20 to trigering Dragging */
    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
    vTcamInfo.hitNumber            = 0;
    vTcamInfo.autoResize           = GT_FALSE;
    vTcamInfo.ruleAdditionMethod   =
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;
    vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E;
    vTcamInfo.guaranteedNumOfRules = 342;
    vTcamId = 20;
    if (1 == HARD_WIRE_TCAM_MAC(prvTgfDevNum))
    {
        vTcamInfo.clientGroup = 1;
    }
    rc = cpssDxChVirtualTcamCreate(V_TCAM_MANAGER, vTcamId, &vTcamInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamCreate FAILED for vTCAM = %d",
        vTcamId);

    /* stop traffic */
    rc = prvTgfFWSTrafficStop();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFWSTrafficStop");

    /* Compare Port and CNC counters */
    rc = prvTgfPclFWSPortAndCncCntrsCheckOrReset(portRxRateArr, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclFWSPortAndCncCntrsReset");
}

/*========================================================================*/
/* PriorityUpdateTest */

/*add and write some STD_NOT_IP_E rule for PriorityUpdateTest */
static void priorityUpdateTestAddMacDaRuleCountingCnc
(
    GT_U32 ruleId,
    GT_U32 priority,
    GT_U32 macDaPatternLow,
    GT_U16 macDaPatternHigh,
    GT_U32 cncIndex
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    PRV_TGF_PCL_ACTION_STC                     action;
    CPSS_DXCH_PCL_ACTION_STC                   dxChAction;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;

    ruleAttributes.priority = priority;
    /* AUTODOC: mask for DST MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    mask.ruleStdNotIp.macDa.arEther[0] = (((macDaPatternHigh <<  8) & 0xFF) ? 0xFF : 0);
    mask.ruleStdNotIp.macDa.arEther[1] = ((macDaPatternHigh & 0xFF)         ? 0xFF : 0);
    mask.ruleStdNotIp.macDa.arEther[2] = (((macDaPatternLow  << 24) & 0xFF) ? 0xFF : 0);
    mask.ruleStdNotIp.macDa.arEther[3] = (((macDaPatternLow  << 16) & 0xFF) ? 0xFF : 0);
    mask.ruleStdNotIp.macDa.arEther[4] = (((macDaPatternLow  <<  8) & 0xFF) ? 0xFF : 0);
    mask.ruleStdNotIp.macDa.arEther[5] = ((macDaPatternLow  & 0xFF)         ? 0xFF : 0);
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    pattern.ruleStdNotIp.macDa.arEther[0] = ((macDaPatternHigh <<  8) & 0xFF);
    pattern.ruleStdNotIp.macDa.arEther[1] = (macDaPatternHigh & 0xFF)        ;
    pattern.ruleStdNotIp.macDa.arEther[2] = ((macDaPatternLow  << 24) & 0xFF);
    pattern.ruleStdNotIp.macDa.arEther[3] = ((macDaPatternLow  << 16) & 0xFF);
    pattern.ruleStdNotIp.macDa.arEther[4] = ((macDaPatternLow  <<  8) & 0xFF);
    pattern.ruleStdNotIp.macDa.arEther[5] = (macDaPatternLow  & 0xFF)        ;

    /* AUTODOC: action0 - redirect to port A */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                         = PRV_TGF_PACKET_CMD_FORWARD_E;
    action.mirror.cpuCode                 = CPSS_NET_FIRST_USER_DEFINED_E;
    action.redirect.redirectCmd           = PRV_TGF_PCL_ACTION_REDIRECT_CMD_NONE_E;
    action.matchCounter.enableMatchCount  = GT_TRUE;
    action.matchCounter.matchCounterIndex = cncIndex;
    prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);

    ruleData.valid                   = GT_TRUE;
    ruleData.rule.pcl.actionPtr      = &dxChAction;
    ruleData.rule.pcl.maskPtr        = &mask;
    ruleData.rule.pcl.patternPtr     = &pattern;
    tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
    tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    rc = cpssDxChVirtualTcamRuleWrite(
        V_TCAM_MANAGER, V_TCAM_1, ruleId,
        &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
        V_TCAM_1, ruleId);
}

static void priorityUpdateTestAddMacDaRuleGroupCountingCnc(GT_U32 priorityBase)
{
    GT_U32 i;

    for (i = 1; (i < 6); i++)
    {
        priorityUpdateTestAddMacDaRuleCountingCnc(
            i /*ruleId*/, (priorityBase + i) /*priority*/,
            (0x01020300 | i) /*macDaPatternLow*/, (GT_U16)0x0C06 /*macDaPatternHigh*/,
            i /*cncIndex*/);
    }
}

static void priorityUpdateTestUpdateRuleGroupPriority(GT_U32 priority, GT_U32 positionFirst)
{
    GT_STATUS rc;
    GT_U32 i;

    for (i = 1; (i < 6); i++)
    {
        rc = cpssDxChVirtualTcamRulePriorityUpdate(
            V_TCAM_MANAGER, V_TCAM_1, i /*ruleId*/, priority,
            (positionFirst
                ? CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E
                : CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E));
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "cpssDxChVirtualTcamRulePriorityUpdate: vTCAM = %d, RuleID = %d",
            V_TCAM_1, i);
    }
}

static void  priorityUpdateTestCncCountersCheckOrReset
(
    IN   GT_U32   portNum,
    IN   GT_U32   cncBlock,
    IN   GT_U32   expectedPackets,
    IN   GT_BOOL  check
)
{
    GT_STATUS               rc = GT_OK; /* return code */
    PRV_TGF_CNC_COUNTER_STC cncCounter;         /* counter */
    GT_PORT_GROUPS_BMP      portGroupBmp;    /* port group bitmap */
    GT_U32                  i;

    /* use PortGroupsBmp API in the test */
    /* set <currPortGroupsBmp> */
    TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(portNum);
    portGroupBmp = currPortGroupsBmp;

    for (i = 1; (i < 6); i++)
    {
        rc = prvTgfCncPortGroupCounterGet(portGroupBmp,
            cncBlock, i,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &cncCounter);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfCncPortGroupCounterGet: cncBlock = %d, index = %d",
            cncBlock, i);
        if (check != GT_FALSE)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(
                expectedPackets, cncCounter.packetCount.l[0], "expected and found");
            PRV_UTF_LOG3_MAC(
                "CNC block %d index %d packets %d\n",
                cncBlock, i, cncCounter.packetCount.l[0]);
        }
    }

    /* restore values after using currPortGroupsBmp*/
    usePortGroupsBmp = GT_FALSE;
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
}


/**
* @internal priorityUpdateTestTrafficGenerate function
* @endinternal
*
* @brief   Configure, start traffic from port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*/
static void priorityUpdateTestTrafficGenerate
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
)
{
    GT_STATUS   st;
    GT_U32      i;

    /* DATA of bypass packet */
    static GT_U8 payloadDataArr[48] ={0x33, 0x33, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    /* Bypass PAYLOAD part */
    static TGF_PACKET_PAYLOAD_STC payloadPart = {
        sizeof(payloadDataArr),                       /* dataLength */
        payloadDataArr                                /* dataPtr */
    };
    /*  L2 part of packet  */
    static TGF_PACKET_L2_STC packetL2Part = {
        {0x0C, 0x06, 0x01, 0x02, 0x03, 0x00},                /* daMac */
        {0x00, 0x11, 0x22, 0x44, 0x55, 0x66}                 /* saMac */
    };
    /* PARTS of packet */
    static TGF_PACKET_PART_STC packetPartArray[] = {
        {TGF_PACKET_PART_L2_E,        &packetL2Part},  /* type, partPtr */
        {TGF_PACKET_PART_PAYLOAD_E,   &payloadPart}
    };
    /*  PACKET to send info */
    static TGF_PACKET_STC packetInfo = {
    /* LENGTH of packet with CRC */
        (TGF_L2_HEADER_SIZE_CNS + sizeof(payloadDataArr) + TGF_CRC_LEN_CNS),                                          /* totalLen */
        sizeof(packetPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
        packetPartArray                                        /* partsArray */
    };

    for (i = 1; (i < 6); i++)
    {
        packetL2Part.daMac[5] = (GT_U8)i;
        /* setup Packet */
        st = prvTgfSetTxSetupEth(devNum, &packetInfo, 1, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "prvTgfSetTxSetupEth error");

        /* send Packet from port portNum */
        st = prvTgfStartTransmitingEth(devNum, portNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "prvTgfStartTransmitingEth error");
    }

    /* wait to get stable traffic */
    PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_MAC(10);
}

/**
* @internal prvTgfVirtualTcamRulePriorityUpdateTrafficTest function
* @endinternal
*
* @brief   Test checks changing priorities of existing rules (use traffic)-
*         1. Create vTcam, Configure PCL and CNC
*         2. Set 5 rules with priorities 101-105 counting macthes in different CNC counters
*         3. Clean CNC counters
*         4. Generate traffic to hit the rules
*         5. Check and clean CNC counters
*         6. Change priority of each rule to 150 aligning to first.
*         7. Generate traffic to hit the rules
*         8. Check and clean CNC counters
*         9. Change priority of each rule to 50 aligning to last.
*        10. Generate traffic to hit the rules
*        11. Check and clean CNC counters
*/
GT_VOID prvTgfVirtualTcamRulePriorityUpdateTrafficTest
(
    GT_VOID
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT       ruleSize;
    GT_U32                                     indexRangesBmp[4];
    GT_U32                                     cncBlockNum = 1;
    GT_U32                                     clientGroup = PCL_TCAM_GROUP_CNS;
    PRV_TGF_CNC_CLIENT_ENT                     client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;

    /* AUTODOC: Vitual Tcam Priority Move Test configuration set*/
    ruleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    prvTgfVirtualTcamTestConfigurationSet(clientGroup,
                    ruleSize,
                    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E);

    /* AUTODOC: Configure PCL rules */
    priorityUpdateTestAddMacDaRuleGroupCountingCnc(100 /*priorityBase*/);

    /* use first range */
    indexRangesBmp[0] = 0x1;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    /* configure CNC */
    rc = prvTgfCncTestCncBlockConfigure(
        cncBlockNum,
        client,
        GT_TRUE /*enable*/,
        indexRangesBmp,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfCncTestCncBlockConfigure: block %d, client %d",
            cncBlockNum,
            client);

    /* AUTODOC: enable PCL clients for CNC */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS ))
    {
        rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    }

    /* AUTODOC: Clean CNC counters */
    priorityUpdateTestCncCountersCheckOrReset(
        prvTgfPortsArray[ING_PORT_IDX_CNS],
        cncBlockNum,
        1 /*expectedPackets*/,
        GT_FALSE  /*check*/);

    /* AUTODOC: send 5 packets each to be matched by one rule and counted by one counter */
    priorityUpdateTestTrafficGenerate(
        prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS]);

    /* AUTODOC: check and reset CNC conters */
    priorityUpdateTestCncCountersCheckOrReset(
        prvTgfPortsArray[ING_PORT_IDX_CNS],
        cncBlockNum,
        1 /*expectedPackets*/,
        GT_TRUE  /*check*/);

    /* AUTODOC: set priority 150 to all rules - move will be caused */
    priorityUpdateTestUpdateRuleGroupPriority(
        150 /*priority*/, 1 /*positionFirst*/);

    /* AUTODOC: send 5 packets each to be matched by one rule and counted by one counter */
    priorityUpdateTestTrafficGenerate(
        prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS]);

    /* AUTODOC: check and reset CNC conters */
    priorityUpdateTestCncCountersCheckOrReset(
        prvTgfPortsArray[ING_PORT_IDX_CNS],
        cncBlockNum,
        1 /*expectedPackets*/,
        GT_TRUE  /*check*/);

    /* AUTODOC: set priority 50 to all rules - move will be caused */
    priorityUpdateTestUpdateRuleGroupPriority(
        50 /*priority*/, 0 /*positionFirst*/);

    /* AUTODOC: send 5 packets each to be matched by one rule and counted by one counter */
    priorityUpdateTestTrafficGenerate(
        prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS]);

    /* AUTODOC: check and reset CNC conters */
    priorityUpdateTestCncCountersCheckOrReset(
        prvTgfPortsArray[ING_PORT_IDX_CNS],
        cncBlockNum,
        1 /*expectedPackets*/,
        GT_TRUE  /*check*/);
}

/**
* @internal prvTgfVirtualTcamRulePriorityUpdateConfigurationRestore function
* @endinternal
*
* @brief   Virtual Tcam Update Rules Priority Test configurations restore
*
*/
GT_VOID prvTgfVirtualTcamRulePriorityUpdateConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                        rc;
    GT_U32                           floorIndex;
    GT_U16                           jj;
    GT_U32                           indexRangesBmp[4];
    GT_U32                           cncBlockCntrs;
    GT_U32                           cncBlocks;
    PRV_TGF_CNC_CLIENT_ENT           client;
    GT_U32                           cncBlockNum = 1;
    GT_U32                           counterIdx;
    PRV_TGF_CNC_COUNTER_STC          counter;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: Remove vTCAMs with PCL rules  */
    for (jj = 1; jj < 6; jj++ )
    {
        rc = cpssDxChVirtualTcamRuleDelete(V_TCAM_MANAGER, V_TCAM_1, jj);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "cpssDxChVirtualTcamRuleDelete(vTCAM:%d) for ruleId[%d] failed\n",
            V_TCAM_1, jj);
    }
    cpssOsMemSet(&counter, 0, sizeof(counter));

    /* get CNC size info */
    prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

    /* use first range */
    indexRangesBmp[0] = 0x0;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    {
        /* Get CNC client for a block */
        client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;

        /* disable client */
        rc = prvTgfCncBlockClientEnableSet(
            cncBlockNum, client, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", cncBlockNum, client);

        /* reset ranges */
        rc = prvTgfCncBlockClientRangesSet(
            cncBlockNum, client, indexRangesBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", cncBlockNum, client);
    }
    {
        /* Get CNC client for a block */
        client = PRV_TGF_CNC_CLIENT_TTI_E;

        /* disable client */
        rc = prvTgfCncBlockClientEnableSet(
            cncBlockNum, client, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", cncBlockNum, client);

        /* reset ranges */
        rc = prvTgfCncBlockClientRangesSet(
            cncBlockNum, client, indexRangesBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", cncBlockNum, client);
    }
    /* clean counters */
    for (counterIdx = 0; counterIdx < cncBlockCntrs; counterIdx++)
    {
        rc = prvTgfCncCounterSet(
            cncBlockNum, counterIdx,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncCounterSet: block %d idx %d", cncBlockNum, counterIdx);
    }

    /* disable PCL clients for CNC */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA3_E | UTF_CPSS_PP_E_ARCH_CNS ))
    {
        rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    }

    rc = cpssDxChVirtualTcamRemove(V_TCAM_MANAGER, V_TCAM_1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRemove(vTCAM:%d) failed\n", V_TCAM_1);

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy for port 1 (the ingress port) */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[ING_PORT_IDX_CNS],
                                 GT_FALSE);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: restore TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, ttiTcamLookupEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore MAC mode for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, ttiTcamMacMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Restore TCAM floor info */
        for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet() ; floorIndex++ )
        {
            rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex,
                                                  &saveFloorInfoArr[floorIndex][0]);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
                floorIndex);
        }

        if(HARD_WIRE_TCAM_MAC(prvTgfDevNum))
        {
            /* BC3 uses ipcl0 as 'group 1' */
        }
        else
        {
            /* AUTODOC: Restore PCL TCAM group binding */
            rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);
            UTF_VERIFY_EQUAL3_STRING_MAC(
                    GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
                    PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);

            /* AUTODOC: Restore TTI TCAM group binding */
            rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);
            UTF_VERIFY_EQUAL3_STRING_MAC(
                    GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
                    PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);
        }
    }

    rc = cpssDxChVirtualTcamManagerDevListRemove(V_TCAM_MANAGER, &prvTgfDevNum, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
                                 V_TCAM_MANAGER);

    rc = cpssDxChVirtualTcamManagerDelete(V_TCAM_MANAGER);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
                                 V_TCAM_MANAGER);

    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* Restore TCAM Active number of floors */
        rc = cpssDxChTcamActiveFloorsSet(prvTgfDevNum,
                fineTuningPtr->tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);

        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "cpssDxChTcamActiveFloorsSet failed for device: %d", prvTgfDevNum);
    }
}

/* data of TcamMngPerPortGroupTrafficTest passed to restore */
static GT_BOOL prvTgfVirtualTcamMngPerPortGroupDataValid;
static struct
{
    GT_PORT_NUM               portNum1;
    GT_PORT_NUM               portNum2;
    GT_U32                    portGroup1;
    GT_U32                    portGroup2;
    GT_U32                    pclTcamClientGroup1;
    GT_U32                    ttiTcamClientGroup1;
    GT_U32                    pclTcamClientGroup2;
    GT_U32                    ttiTcamClientGroup2;
    PRV_TGF_TTI_MAC_MODE_ENT  ttiMacMode;
    GT_BOOL                   cncPclUnitEnable;
    GT_BOOL                   cncTtiUnitEnable;
    struct
    {
      CPSS_DXCH_TCAM_BLOCK_INFO_STC  portGroup1FiArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];
      CPSS_DXCH_TCAM_BLOCK_INFO_STC  portGroup2FiArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];
    } floorInfo[CPSS_DXCH_TCAM_MAX_NUM_FLOORS_CNS];

} prvTgfVirtualTcamMngPerPortGroupData;

/**
* @internal prvTgfVirtualTcamMngPerPortGroupGetPorts function
* @endinternal
*
* @brief   Get ports for Virtual Tcam manager per port group Test.
*
*/
static GT_VOID prvTgfVirtualTcamMngPerPortGroupGetPorts
(
    GT_VOID
)
{
    GT_STATUS                  rc;
    GT_PORT_NUM                portNum1 = 0xFFFFFFFF;
    GT_PORT_NUM                portNum2 = 0xFFFFFFFF;
    GT_U32                     portGroup1 = 0xFFFFFFFF;
    GT_U32                     portGroup2 = 0xFFFFFFFF;
    GT_PORT_GROUPS_BMP         portGroupsBmp;
    GT_U32                     portGroupId;
    GT_PORT_NUM                portNum;
    GT_U32                     i;
    GT_U32                     portMacNum;
    GT_U32                     numOfPortGroups;
    GT_BOOL                    enable;
    GT_U32                     numOfFloors;
    GT_U32                     floorIndex;

    prvTgfVirtualTcamMngPerPortGroupDataValid = GT_FALSE;

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.isMultiPortGroupDevice == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("Not isMultiPortGroupDevice - test skipped\n");
        SKIP_TEST_MAC;
    }
    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* Falcon */
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        rc = prvCpssMultiPortGroupsBmpCheckSpecificUnit(
            prvTgfDevNum, &portGroupsBmp, PRV_CPSS_DXCH_UNIT_TCAM_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvCpssMultiPortGroupsBmpCheckSpecificUnit for TCAM failed");
    }
    else
    {
        /* Lion2 */
        portGroupsBmp = PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.activePortGroupsBmp;
    }

    if (CPSS_PORT_GROUP_UNAWARE_MODE_CNS == portGroupsBmp)
    {
        PRV_UTF_LOG0_MAC("single TCAM unit - test skipped\n");
        SKIP_TEST_MAC;
    }
    numOfPortGroups = 0;
    for (i = 0; (i <= PRV_CPSS_PP_MAC(prvTgfDevNum)->portGroupsInfo.lastActivePortGroup); i++)
    {
        if (portGroupsBmp & (1 << i))
        {
            numOfPortGroups++;
        }
    }
    if (1 >= numOfPortGroups)
    {
        PRV_UTF_LOG0_MAC("single TCAM unit - test skipped\n");
        SKIP_TEST_MAC;
    }

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        portNum = prvTgfPortsArray[i];
        if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
        {
            rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(
                prvTgfDevNum, portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &portMacNum);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                    GT_OK, rc, "prvCpssDxChPortPhysicalPortMapCheckAndConvert failed");

            portGroupId = portMacNum / PRV_CPSS_PP_MAC(prvTgfDevNum)->multiPipe.numOfPortsPerPipe;
        }
        else
        {
            portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(prvTgfDevNum, portNum);
        }
        /* find TCAM port group decreasig up to nearest from bitmap */
        for (/*from already calculated portGroupId*/;
              (((1 << portGroupId) & portGroupsBmp) == 0);
              portGroupId--)
        {
            UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
                    0, portGroupId, "TCAM port group for port not found");
        }
        if (i == 0)
        {
            portNum1 = portNum;
            portGroup1 = portGroupId;
        }
        else if (portGroup1 != portGroupId)
        {
            portNum2 = portNum;
            portGroup2 = portGroupId;
            break;
        }
    }
    if (0xFFFFFFFF == portGroup2)
    {
        PRV_UTF_LOG0_MAC("all tested ports use the same TCAM");
        SKIP_TEST_MAC;
    }

    prvTgfVirtualTcamMngPerPortGroupDataValid       = GT_TRUE;
    prvTgfVirtualTcamMngPerPortGroupData.portNum1   = portNum1;
    prvTgfVirtualTcamMngPerPortGroupData.portNum2   = portNum2;
    prvTgfVirtualTcamMngPerPortGroupData.portGroup1 = portGroup1;
    prvTgfVirtualTcamMngPerPortGroupData.portGroup2 = portGroup2;

    /* defaults for not SIP5 devices */
    prvTgfVirtualTcamMngPerPortGroupData.pclTcamClientGroup1 = 0;
    prvTgfVirtualTcamMngPerPortGroupData.ttiTcamClientGroup1 = 1;
    prvTgfVirtualTcamMngPerPortGroupData.pclTcamClientGroup2 = 0;
    prvTgfVirtualTcamMngPerPortGroupData.ttiTcamClientGroup2 = 1;

    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        rc = cpssDxChTcamPortGroupClientGroupGet(
            prvTgfDevNum, (1 << portGroup1), CPSS_DXCH_TCAM_IPCL_0_E,
            &(prvTgfVirtualTcamMngPerPortGroupData.pclTcamClientGroup1), &enable);
        UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "cpssDxChTcamPortGroupClientGroupGet failed");
        UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_TRUE, enable, "pclTcamClientGroup1 disabled");

        rc = cpssDxChTcamPortGroupClientGroupGet(
            prvTgfDevNum, (1 << portGroup1), CPSS_DXCH_TCAM_TTI_E,
            &(prvTgfVirtualTcamMngPerPortGroupData.ttiTcamClientGroup1), &enable);
        UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "cpssDxChTcamPortGroupClientGroupGet failed");
        UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_TRUE, enable, "ttiTcamClientGroup1 disabled");

        rc = cpssDxChTcamPortGroupClientGroupGet(
            prvTgfDevNum, (1 << portGroup2), CPSS_DXCH_TCAM_IPCL_0_E,
            &(prvTgfVirtualTcamMngPerPortGroupData.pclTcamClientGroup2), &enable);
        UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "cpssDxChTcamPortGroupClientGroupGet failed");
        UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_TRUE, enable, "pclTcamClientGroup2 disabled");

        rc = cpssDxChTcamPortGroupClientGroupGet(
            prvTgfDevNum, (1 << portGroup2), CPSS_DXCH_TCAM_TTI_E,
            &(prvTgfVirtualTcamMngPerPortGroupData.ttiTcamClientGroup2), &enable);
        UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "cpssDxChTcamPortGroupClientGroupGet failed");
        UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_TRUE, enable, "ttiTcamClientGroup2 disabled");
    }
    rc = prvTgfTtiMacModeGet(
        prvTgfDevNum, PRV_TGF_TTI_KEY_ETH_E,
        &(prvTgfVirtualTcamMngPerPortGroupData.ttiMacMode));
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    rc = cpssDxChCncCountingEnableGet(
        prvTgfDevNum, CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_TTI_E,
        &(prvTgfVirtualTcamMngPerPortGroupData.cncTtiUnitEnable));
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "cpssDxChCncCountingEnableGet: %d", prvTgfDevNum);

    rc = cpssDxChCncCountingEnableGet(
        prvTgfDevNum, CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PCL_E,
        &(prvTgfVirtualTcamMngPerPortGroupData.cncPclUnitEnable));
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "cpssDxChCncCountingEnableGet: %d", prvTgfDevNum);

    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        numOfFloors = prvTgfTcamFloorsNumGet();
        for (floorIndex = 0 ; (floorIndex < numOfFloors); floorIndex++ )
        {
            rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet(
                prvTgfDevNum, (1 << portGroup1), floorIndex,
                prvTgfVirtualTcamMngPerPortGroupData.floorInfo[floorIndex].portGroup1FiArr);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet: %d",
                floorIndex);
            rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet(
                prvTgfDevNum, (1 << portGroup2), floorIndex,
                prvTgfVirtualTcamMngPerPortGroupData.floorInfo[floorIndex].portGroup2FiArr);
            UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet: %d",
                floorIndex);
        }
    }

    prvTgfVirtualTcamMngPerPortGroupDataValid = GT_TRUE;
}

/**
* @internal prvTgfVirtualTcamMngPerPortGroupPclTrafficTest function
* @endinternal
*
* @brief   Test Virtual Tcam manager per port group PCL Test by traffic.
*
*/
GT_VOID prvTgfVirtualTcamMngPerPortGroupPclTrafficTest
(
    GT_VOID
)
{
    GT_STATUS          rc;
    GT_U32             vTcamMngId1 = V_TCAM_MANAGER;
    GT_U32             vTcamMngId2 = V_TCAM_MANAGER2;
    GT_PORT_NUM        portNum1;
    GT_PORT_NUM        portNum2;
    GT_U32             portGroup1;
    GT_U32             portGroup2;
    CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC    portGroupBmpList;
    GT_U32                           indexRangesBmp[4];
    PRV_TGF_CNC_CLIENT_ENT           client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;
    GT_U32                           cncBlockNum = 0;
    GT_U32                           counterIdx1 = V_TCAM_MANAGER;
    GT_U32                           counterIdx2 = V_TCAM_MANAGER2;
    PRV_TGF_CNC_COUNTER_STC          counter;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC  vTcamInfo;
    GT_U32                           vTcamId;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    CPSS_DXCH_PCL_ACTION_STC                   action;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;

    /* DATA of bypass packet */
    static GT_U8 payloadDataArr[48] ={0x33, 0x33, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    /* Bypass PAYLOAD part */
    static TGF_PACKET_PAYLOAD_STC payloadPart = {
        sizeof(payloadDataArr),                       /* dataLength */
        payloadDataArr                                /* dataPtr */
    };
    /*  L2 part of packet  */
    static TGF_PACKET_L2_STC packetL2Part = {
        {0x0C, 0x06, 0x01, 0x02, 0x03, 0x00},                /* daMac */
        {0x00, 0x11, 0x22, 0x44, 0x55, 0x66}                 /* saMac */
    };
    /* PARTS of packet */
    static TGF_PACKET_PART_STC packetPartArray[] = {
        {TGF_PACKET_PART_L2_E,        &packetL2Part},  /* type, partPtr */
        {TGF_PACKET_PART_PAYLOAD_E,   &payloadPart}
    };
    /*  PACKET to send info */
    static TGF_PACKET_STC packetInfo = {
    /* LENGTH of packet with CRC */
        (TGF_L2_HEADER_SIZE_CNS + sizeof(payloadDataArr) + TGF_CRC_LEN_CNS),                                          /* totalLen */
        sizeof(packetPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
        packetPartArray                                        /* partsArray */
    };

    prvTgfVirtualTcamMngPerPortGroupGetPorts();
    if (prvTgfVirtualTcamMngPerPortGroupDataValid == GT_FALSE)
    {
        return;
    }

    portNum1   = prvTgfVirtualTcamMngPerPortGroupData.portNum1;
    portNum2   = prvTgfVirtualTcamMngPerPortGroupData.portNum2;
    portGroup1 = prvTgfVirtualTcamMngPerPortGroupData.portGroup1;
    portGroup2 = prvTgfVirtualTcamMngPerPortGroupData.portGroup2;

    /* prvTgfVirtualTcamRulePriorityUpdateTrafficTest */

    /* Create vTCAM manager */
    rc = cpssDxChVirtualTcamManagerCreate(vTcamMngId1, &vtcamMngCfgParam);
    if (rc == GT_ALREADY_EXIST)
        rc = GT_OK;
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerCreate failed\n");

    rc = cpssDxChVirtualTcamManagerCreate(vTcamMngId2, &vtcamMngCfgParam);
    if (rc == GT_ALREADY_EXIST)
        rc = GT_OK;
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerCreate failed\n");

    portGroupBmpList.devNum = prvTgfDevNum;
    portGroupBmpList.portGroupsBmp = (1 << portGroup1);
    rc = cpssDxChVirtualTcamManagerPortGroupListAdd(
        vTcamMngId1, &portGroupBmpList, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerPortGroupListAdd failed\n");

    portGroupBmpList.devNum = prvTgfDevNum;
    portGroupBmpList.portGroupsBmp = (1 << portGroup2);
    rc = cpssDxChVirtualTcamManagerPortGroupListAdd(
        vTcamMngId2, &portGroupBmpList, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerPortGroupListAdd failed\n");

    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
    vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_STD_E;
    vTcamInfo.autoResize           = GT_FALSE;
    vTcamInfo.guaranteedNumOfRules = 6;
    vTcamInfo.ruleAdditionMethod   =
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;
    vTcamInfo.hitNumber   = 0;
    vTcamId               = V_TCAM_1;

    vTcamInfo.clientGroup = prvTgfVirtualTcamMngPerPortGroupData.pclTcamClientGroup1;
    rc = cpssDxChVirtualTcamCreate(vTcamMngId1, vTcamId, &vTcamInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamCreate FAILED for vTCAM = %d",
        vTcamId);

    vTcamInfo.clientGroup = prvTgfVirtualTcamMngPerPortGroupData.pclTcamClientGroup2;
    rc = cpssDxChVirtualTcamCreate(vTcamMngId2, vTcamId, &vTcamInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamCreate FAILED for vTCAM = %d",
        vTcamId);

    /* rule matching any packet */
    /* action - CNC counter     */
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    action.pktCmd                         = CPSS_PACKET_CMD_FORWARD_E;
    action.mirror.cpuCode                 = CPSS_NET_FIRST_USER_DEFINED_E;
    action.matchCounter.enableMatchCount  = GT_TRUE;

    ruleData.valid                   = GT_TRUE;
    ruleData.rule.pcl.actionPtr      = &action;
    ruleData.rule.pcl.maskPtr        = &mask;
    ruleData.rule.pcl.patternPtr     = &pattern;
    tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
    tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ruleAttributes.priority      = 0;

    action.matchCounter.matchCounterIndex = counterIdx1;
    rc = cpssDxChVirtualTcamRuleWrite(
        vTcamMngId1, vTcamId, 0, &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: failed");

    action.matchCounter.matchCounterIndex = counterIdx2;
    rc = cpssDxChVirtualTcamRuleWrite(
        vTcamMngId2, vTcamId, 0, &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: failed");

    /* AUTODOC: Init IPCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        portNum1,
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: port %d", portNum1);

    rc = prvTgfPclDefPortInit(
        portNum2,
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: port %d", portNum2);

    cpssOsMemSet(indexRangesBmp, 0, sizeof(indexRangesBmp));
    indexRangesBmp[0] = 0x1;

    /* enable client and set range for CNC block */
    rc = prvTgfCncTestCncBlockConfigure(
        cncBlockNum,
        client,
        GT_TRUE /*enable*/,
        indexRangesBmp,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfCncTestCncBlockConfigure: block %d, client %d",
            cncBlockNum,
            client);

    /* AUTODOC: enable PCL clients for CNC */
    rc = prvTgfCncCountingEnableSet(
        PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncCountingEnableSet failed");

    /* clean counters */
    cpssOsMemSet(&counter, 0, sizeof(counter));
    rc = prvTgfCncCounterSet(
        cncBlockNum, counterIdx1,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncCounterSet failed");

    rc = prvTgfCncCounterSet(
        cncBlockNum, counterIdx2,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncCounterSet failed");

    /*traffic*/

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth error");

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum1);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth error");

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum2);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth error");

    /* wait to get stable traffic */
    PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_MAC(0);

    /* check CNC counters */
    rc = prvTgfCncCounterGet(
        cncBlockNum, counterIdx1,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncCounterGet failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(
        1, counter.packetCount.l[0], "Wrong CNC counter value");

    rc = prvTgfCncCounterGet(
        cncBlockNum, counterIdx2,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncCounterGet failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(
        1, counter.packetCount.l[0], "Wrong CNC counter value");
}

/**
* @internal prvTgfVirtualTcamMngPerPortGroupPclTrafficConfigurationRestore function
* @endinternal
*
* @brief   Virtual TCAM manager per port group PCL Test configurations restore
*
*/
GT_VOID prvTgfVirtualTcamMngPerPortGroupPclTrafficConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS          rc;
    GT_U32             vTcamMngId1 = V_TCAM_MANAGER;
    GT_U32             vTcamMngId2 = V_TCAM_MANAGER2;
    GT_PORT_NUM        portNum1;
    GT_PORT_NUM        portNum2;
    GT_U32             portGroup1;
    GT_U32             portGroup2;
    CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC    portGroupBmpList;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    GT_U32                           indexRangesBmp[4];
    PRV_TGF_CNC_CLIENT_ENT           client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;
    GT_U32                           cncBlockNum = 0;
    GT_U32                           counterIdx1 = V_TCAM_MANAGER;
    GT_U32                           counterIdx2 = V_TCAM_MANAGER2;
    PRV_TGF_CNC_COUNTER_STC          counter;
    GT_U32                           numOfFloors;
    GT_U32                           floorIndex;

    if (prvTgfVirtualTcamMngPerPortGroupDataValid == GT_FALSE)
    {
        return;
    }

    portNum1   = prvTgfVirtualTcamMngPerPortGroupData.portNum1;
    portNum2   = prvTgfVirtualTcamMngPerPortGroupData.portNum2;
    portGroup1 = prvTgfVirtualTcamMngPerPortGroupData.portGroup1;
    portGroup2 = prvTgfVirtualTcamMngPerPortGroupData.portGroup2;

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning;

    cpssOsMemSet(indexRangesBmp, 0, sizeof(indexRangesBmp));
    cpssOsMemSet(&counter, 0, sizeof(counter));

    /* disable client */
    rc = prvTgfCncBlockClientEnableSet(
        cncBlockNum, client, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncBlockClientEnableSet failed");

    /* reset ranges */
    rc = prvTgfCncBlockClientRangesSet(
        cncBlockNum, client, indexRangesBmp);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncBlockClientRangesSet failed");

    /* clean counters */
    rc = prvTgfCncCounterSet(
        cncBlockNum, counterIdx1,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncCounterSet failed");

    rc = prvTgfCncCounterSet(
        cncBlockNum, counterIdx2,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncCounterSet failed");

    rc = prvTgfCncCountingEnableSet(
        PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,
        prvTgfVirtualTcamMngPerPortGroupData.cncPclUnitEnable);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncCountingEnableSet failed");

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy for port 1 (the ingress port) */
    rc = prvTgfPclPortIngressPolicyEnable(portNum1, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfPclPortIngressPolicyEnable failed");

    rc = prvTgfPclPortIngressPolicyEnable(portNum2, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfPclPortIngressPolicyEnable failed");

    /*vTcamMngId1*/
    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId1, V_TCAM_1, 0);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed");

    rc = cpssDxChVirtualTcamRemove(vTcamMngId1, V_TCAM_1);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamRemove failed");

    portGroupBmpList.devNum = prvTgfDevNum;
    portGroupBmpList.portGroupsBmp = (1 << portGroup1);
    rc = cpssDxChVirtualTcamManagerPortGroupListRemove(
        vTcamMngId1, &portGroupBmpList, 1, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerPortGroupListRemove failed");

    rc = cpssDxChVirtualTcamManagerDelete(vTcamMngId1);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerDelete failed");

    /*vTcamMngId2*/
    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId2, V_TCAM_1, 0);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed");

    rc = cpssDxChVirtualTcamRemove(vTcamMngId2, V_TCAM_1);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamRemove failed");

    portGroupBmpList.devNum = prvTgfDevNum;
    portGroupBmpList.portGroupsBmp = (1 << portGroup2);
    rc = cpssDxChVirtualTcamManagerPortGroupListRemove(
        vTcamMngId2, &portGroupBmpList, 1, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerPortGroupListRemove failed");

    rc = cpssDxChVirtualTcamManagerDelete(vTcamMngId2);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerDelete failed");

    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* Restore TCAM Active number of floors */
        rc = cpssDxChTcamActiveFloorsSet(prvTgfDevNum,
                fineTuningPtr->tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
            GT_OK, rc, "cpssDxChTcamActiveFloorsSet failed");
    }

    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        numOfFloors = prvTgfTcamFloorsNumGet();
        for (floorIndex = 0 ; (floorIndex < numOfFloors); floorIndex++ )
        {
            rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet(
                prvTgfDevNum, (1 << portGroup1), floorIndex,
                prvTgfVirtualTcamMngPerPortGroupData.floorInfo[floorIndex].portGroup1FiArr);
            UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
                GT_OK, rc, "prvTgfVirtualTcamMngPerPortGroupData failed");
            rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet(
                prvTgfDevNum, (1 << portGroup2), floorIndex,
                prvTgfVirtualTcamMngPerPortGroupData.floorInfo[floorIndex].portGroup2FiArr);
            UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
                GT_OK, rc, "prvTgfVirtualTcamMngPerPortGroupData failed");
        }
    }

    prvTgfVirtualTcamMngPerPortGroupDataValid = GT_FALSE;
}

/**
* @internal prvTgfVirtualTcamMngPerPortGroupTtiTrafficTest function
* @endinternal
*
* @brief   Test Virtual Tcam manager per port group TTI Test by traffic.
*
*/
GT_VOID prvTgfVirtualTcamMngPerPortGroupTtiTrafficTest
(
    GT_VOID
)
{
    GT_STATUS          rc;
    GT_U32             vTcamMngId1 = V_TCAM_MANAGER;
    GT_U32             vTcamMngId2 = V_TCAM_MANAGER2;
    GT_PORT_NUM        portNum1;
    GT_PORT_NUM        portNum2;
    GT_U32             portGroup1;
    GT_U32             portGroup2;
    CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC    portGroupBmpList;
    GT_U32                           indexRangesBmp[4];
    PRV_TGF_CNC_CLIENT_ENT           client = PRV_TGF_CNC_CLIENT_TTI_E;
    GT_U32                           cncBlockNum = 0;
    GT_U32                           counterIdx1 = V_TCAM_MANAGER;
    GT_U32                           counterIdx2 = V_TCAM_MANAGER2;
    PRV_TGF_CNC_COUNTER_STC          counter;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC  vTcamInfo;
    GT_U32                           vTcamId;
    CPSS_DXCH_TTI_RULE_UNT                     mask;
    CPSS_DXCH_TTI_RULE_UNT                     pattern;
    CPSS_DXCH_TTI_ACTION_STC                   action;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;

    /* DATA of bypass packet */
    static GT_U8 payloadDataArr[48] ={0x33, 0x33, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    /* Bypass PAYLOAD part */
    static TGF_PACKET_PAYLOAD_STC payloadPart = {
        sizeof(payloadDataArr),                       /* dataLength */
        payloadDataArr                                /* dataPtr */
    };
    /*  L2 part of packet  */
    static TGF_PACKET_L2_STC packetL2Part = {
        {0x0C, 0x06, 0x01, 0x02, 0x03, 0x00},                /* daMac */
        {0x00, 0x11, 0x22, 0x44, 0x55, 0x66}                 /* saMac */
    };
    /* PARTS of packet */
    static TGF_PACKET_PART_STC packetPartArray[] = {
        {TGF_PACKET_PART_L2_E,        &packetL2Part},  /* type, partPtr */
        {TGF_PACKET_PART_PAYLOAD_E,   &payloadPart}
    };
    /*  PACKET to send info */
    static TGF_PACKET_STC packetInfo = {
    /* LENGTH of packet with CRC */
        (TGF_L2_HEADER_SIZE_CNS + sizeof(payloadDataArr) + TGF_CRC_LEN_CNS),                                          /* totalLen */
        sizeof(packetPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
        packetPartArray                                        /* partsArray */
    };

    prvTgfVirtualTcamMngPerPortGroupGetPorts();
    if (prvTgfVirtualTcamMngPerPortGroupDataValid == GT_FALSE)
    {
        return;
    }

    portNum1   = prvTgfVirtualTcamMngPerPortGroupData.portNum1;
    portNum2   = prvTgfVirtualTcamMngPerPortGroupData.portNum2;
    portGroup1 = prvTgfVirtualTcamMngPerPortGroupData.portGroup1;
    portGroup2 = prvTgfVirtualTcamMngPerPortGroupData.portGroup2;

    /* prvTgfVirtualTcamRulePriorityUpdateTrafficTest */

    /* Create vTCAM manager */
    rc = cpssDxChVirtualTcamManagerCreate(vTcamMngId1, &vtcamMngCfgParam);
    if (rc == GT_ALREADY_EXIST)
        rc = GT_OK;
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerCreate failed\n");

    rc = cpssDxChVirtualTcamManagerCreate(vTcamMngId2, &vtcamMngCfgParam);
    if (rc == GT_ALREADY_EXIST)
        rc = GT_OK;
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerCreate failed\n");

    portGroupBmpList.devNum = prvTgfDevNum;
    portGroupBmpList.portGroupsBmp = (1 << portGroup1);
    rc = cpssDxChVirtualTcamManagerPortGroupListAdd(
        vTcamMngId1, &portGroupBmpList, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerPortGroupListAdd failed\n");

    portGroupBmpList.devNum = prvTgfDevNum;
    portGroupBmpList.portGroupsBmp = (1 << portGroup2);
    rc = cpssDxChVirtualTcamManagerPortGroupListAdd(
        vTcamMngId2, &portGroupBmpList, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerPortGroupListAdd failed\n");

    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
    vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_STD_E;
    vTcamInfo.autoResize           = GT_FALSE;
    vTcamInfo.guaranteedNumOfRules = 6;
    vTcamInfo.ruleAdditionMethod   =
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;
    vTcamInfo.hitNumber   = 0;
    vTcamId               = V_TCAM_1;

    vTcamInfo.clientGroup = prvTgfVirtualTcamMngPerPortGroupData.ttiTcamClientGroup1;
    rc = cpssDxChVirtualTcamCreate(vTcamMngId1, vTcamId, &vTcamInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamCreate FAILED for vTCAM = %d",
        vTcamId);

    vTcamInfo.clientGroup = prvTgfVirtualTcamMngPerPortGroupData.ttiTcamClientGroup2;
    rc = cpssDxChVirtualTcamCreate(vTcamMngId2, vTcamId, &vTcamInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamCreate FAILED for vTCAM = %d",
        vTcamId);

    /* rule matching any packet */
    /* action - CNC counter     */
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    action.command              = CPSS_PACKET_CMD_FORWARD_E;
    action.userDefinedCpuCode   = CPSS_NET_FIRST_USER_DEFINED_E;
    action.bindToCentralCounter = GT_TRUE;
    action.tag1VlanCmd          = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;

    ruleData.valid                   = GT_TRUE;
    ruleData.rule.tti.actionPtr      = &action;
    ruleData.rule.tti.maskPtr        = &mask;
    ruleData.rule.tti.patternPtr     = &pattern;
    tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
    tcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_ETH_E;
    ruleAttributes.priority      = 0;

    action.centralCounterIndex  = counterIdx1;
    rc = cpssDxChVirtualTcamRuleWrite(
        vTcamMngId1, vTcamId, 0, &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: failed");

    action.centralCounterIndex  = counterIdx2;
    rc = cpssDxChVirtualTcamRuleWrite(
        vTcamMngId2, vTcamId, 0, &ruleAttributes, &tcamRuleType, &ruleData);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: failed");

    rc = prvTgfTtiPortLookupEnableSet(portNum1, PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    rc = prvTgfTtiPortLookupEnableSet(portNum2, PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    cpssOsMemSet(indexRangesBmp, 0, sizeof(indexRangesBmp));
    indexRangesBmp[0] = 0x1;

    /* enable client and set range for CNC block */
    rc = prvTgfCncTestCncBlockConfigure(
        cncBlockNum,
        client,
        GT_TRUE /*enable*/,
        indexRangesBmp,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfCncTestCncBlockConfigure: block %d, client %d",
            cncBlockNum,
            client);

    /* AUTODOC: enable PCL clients for CNC */
    rc = prvTgfCncCountingEnableSet(
        PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncCountingEnableSet failed");

    /* clean counters */
    cpssOsMemSet(&counter, 0, sizeof(counter));
    rc = prvTgfCncCounterSet(
        cncBlockNum, counterIdx1,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncCounterSet failed");

    rc = prvTgfCncCounterSet(
        cncBlockNum, counterIdx2,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncCounterSet failed");

    /*traffic*/

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth error");

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum1);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth error");

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum2);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth error");

    /* wait to get stable traffic */
    PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_MAC(0);

    /* check CNC counters */
    rc = prvTgfCncCounterGet(
        cncBlockNum, counterIdx1,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncCounterGet failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(
        1, counter.packetCount.l[0], "Wrong CNC counter value");

    rc = prvTgfCncCounterGet(
        cncBlockNum, counterIdx2,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncCounterGet failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(
        1, counter.packetCount.l[0], "Wrong CNC counter value");
}

/**
* @internal prvTgfVirtualTcamMngPerPortGroupTtiTrafficConfigurationRestore function
* @endinternal
*
* @brief   Virtual TCAM manager per port group TTI Test configurations restore
*
*/
GT_VOID prvTgfVirtualTcamMngPerPortGroupTtiTrafficConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS          rc;
    GT_U32             vTcamMngId1 = V_TCAM_MANAGER;
    GT_U32             vTcamMngId2 = V_TCAM_MANAGER2;
    GT_PORT_NUM        portNum1;
    GT_PORT_NUM        portNum2;
    GT_U32             portGroup1;
    GT_U32             portGroup2;
    CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC    portGroupBmpList;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    GT_U32                           indexRangesBmp[4];
    PRV_TGF_CNC_CLIENT_ENT           client = PRV_TGF_CNC_CLIENT_TTI_E;
    GT_U32                           cncBlockNum = 0;
    GT_U32                           counterIdx1 = V_TCAM_MANAGER;
    GT_U32                           counterIdx2 = V_TCAM_MANAGER2;
    PRV_TGF_CNC_COUNTER_STC          counter;
    GT_U32                           numOfFloors;
    GT_U32                           floorIndex;

    if (prvTgfVirtualTcamMngPerPortGroupDataValid == GT_FALSE)
    {
        return;
    }

    portNum1   = prvTgfVirtualTcamMngPerPortGroupData.portNum1;
    portNum2   = prvTgfVirtualTcamMngPerPortGroupData.portNum2;
    portGroup1 = prvTgfVirtualTcamMngPerPortGroupData.portGroup1;
    portGroup2 = prvTgfVirtualTcamMngPerPortGroupData.portGroup2;

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning;

    cpssOsMemSet(indexRangesBmp, 0, sizeof(indexRangesBmp));
    cpssOsMemSet(&counter, 0, sizeof(counter));

    /* disable client */
    rc = prvTgfCncBlockClientEnableSet(
        cncBlockNum, client, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncBlockClientEnableSet failed");

    /* reset ranges */
    rc = prvTgfCncBlockClientRangesSet(
        cncBlockNum, client, indexRangesBmp);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncBlockClientRangesSet failed");

    /* clean counters */
    rc = prvTgfCncCounterSet(
        cncBlockNum, counterIdx1,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncCounterSet failed");

    rc = prvTgfCncCounterSet(
        cncBlockNum, counterIdx2,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncCounterSet failed");

    rc = prvTgfCncCountingEnableSet(
        PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E,
        prvTgfVirtualTcamMngPerPortGroupData.cncTtiUnitEnable);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncCountingEnableSet failed");

    rc = prvTgfTtiPortLookupEnableSet(portNum1, PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncCountingEnableSet failed");

    rc = prvTgfTtiPortLookupEnableSet(portNum2, PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncCountingEnableSet failed");

    rc = prvTgfTtiMacModeSet(
        PRV_TGF_TTI_KEY_ETH_E, prvTgfVirtualTcamMngPerPortGroupData.ttiMacMode);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfTtiMacModeSet failed");

    /*vTcamMngId1*/
    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId1, V_TCAM_1, 0);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed");

    rc = cpssDxChVirtualTcamRemove(vTcamMngId1, V_TCAM_1);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamRemove failed");

    portGroupBmpList.devNum = prvTgfDevNum;
    portGroupBmpList.portGroupsBmp = (1 << portGroup1);
    rc = cpssDxChVirtualTcamManagerPortGroupListRemove(
        vTcamMngId1, &portGroupBmpList, 1, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerPortGroupListRemove failed");

    rc = cpssDxChVirtualTcamManagerDelete(vTcamMngId1);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerDelete failed");

    /*vTcamMngId2*/
    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId2, V_TCAM_1, 0);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed");

    rc = cpssDxChVirtualTcamRemove(vTcamMngId2, V_TCAM_1);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamRemove failed");

    portGroupBmpList.devNum = prvTgfDevNum;
    portGroupBmpList.portGroupsBmp = (1 << portGroup2);
    rc = cpssDxChVirtualTcamManagerPortGroupListRemove(
        vTcamMngId2, &portGroupBmpList, 1, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerPortGroupListRemove failed");

    rc = cpssDxChVirtualTcamManagerDelete(vTcamMngId2);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerDelete failed");

    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* Restore TCAM Active number of floors */
        rc = cpssDxChTcamActiveFloorsSet(prvTgfDevNum,
                fineTuningPtr->tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);
        UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
            GT_OK, rc, "cpssDxChTcamActiveFloorsSet failed");
    }

    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        numOfFloors = prvTgfTcamFloorsNumGet();
        for (floorIndex = 0 ; (floorIndex < numOfFloors); floorIndex++ )
        {
            rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet(
                prvTgfDevNum, (1 << portGroup1), floorIndex,
                prvTgfVirtualTcamMngPerPortGroupData.floorInfo[floorIndex].portGroup1FiArr);
            UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
                GT_OK, rc, "prvTgfVirtualTcamMngPerPortGroupData failed");
            rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet(
                prvTgfDevNum, (1 << portGroup2), floorIndex,
                prvTgfVirtualTcamMngPerPortGroupData.floorInfo[floorIndex].portGroup2FiArr);
            UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
                GT_OK, rc, "prvTgfVirtualTcamMngPerPortGroupData failed");
        }
    }

    prvTgfVirtualTcamMngPerPortGroupDataValid = GT_FALSE;
}

/************* HA Tests for vTCAM *************/
/** HA Tests use vTCAM Mng Id = devNum - since the test function assigns vTcamMngId = devNum */
GT_STATUS prvTgfVirtualTcamRuleSizesHa
(
    GT_U32 ruleSize,
    GT_U32 GuranteedRules,
    GT_U32 rulesCount,
    GT_U32 tcamReset
)
{
    GT_STATUS rc;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, ruleSize, GuranteedRules, rulesCount, 0, 0, 0, 0);
    if(rc != GT_OK)
    {
        cpssOsPrintf("vtcamHaApp failed before HA\n");
        return rc;
    }

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    if(rc != GT_OK)
    {
        cpssOsPrintf("prvCpssDxChVirtualTcamHaSystemRecoverySet INIT failed\n");
        return rc;
    }

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, ruleSize, GuranteedRules, rulesCount, 0, 0, 0, 0);
    if(rc != GT_OK)
    {
        cpssOsPrintf("vtcamHaApp failed after HA\n");
        return rc;
    }

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    if(rc != GT_OK)
    {
        cpssOsPrintf("prvCpssDxChVirtualTcamHaSystemRecoverySet HW CATCHUP failed\n");
        return rc;
    }

    if(tcamReset)
    {
        rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
        if(rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDxChVirtualTcamHaSystemRecoverySet COMPLETION failed\n");
            return rc;
        }
    }
    else
    {
        rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 0);
        if(rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDxChVirtualTcamHaSystemRecoverySet COMPLETION failed\n");
            return rc;
        }
    }
    return GT_OK;
}

/*
 * @internal prvTgfVirtualTcamRuleSizeCheckHa function
 * @endinternal
 *
 * @brief -  Tests to check vtcam creation/rules addition of different sizes - 10, 20, 30, 40, 50, 60, 80B
 * Vary the #rules as 1, 100, 1000
 */

GT_VOID prvTgfVirtualTcamRuleSizeCheckHa
(
    GT_VOID
)
{
    GT_STATUS rc;
    /*RuleSize = 10B, RuleWrites = 1*/
    rc = prvTgfVirtualTcamRuleSizesHa(0,1,1,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 10B with 1 rule write\n");

    /*RuleSize = 20B, RuleWrites = 1*/
    rc = prvTgfVirtualTcamRuleSizesHa(1,1,1,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 20B with 1 rule write\n");

    /*RuleSize = 30B, RuleWrites = 1*/
    rc = prvTgfVirtualTcamRuleSizesHa(2,1,1,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 30B with 1 rule write\n");

    /*RuleSize = 40B, RuleWrites = 1*/
    rc = prvTgfVirtualTcamRuleSizesHa(3,1,1,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 40B with 1 rule write\n");

    /*RuleSize = 50B, RuleWrites = 1*/
    rc = prvTgfVirtualTcamRuleSizesHa(4,1,1,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 50B with 1 rule write\n");

    /*RuleSize = 60B, RuleWrites = 1*/
    rc = prvTgfVirtualTcamRuleSizesHa(5,1,1,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 60B with 1 rule write\n");

    /*RuleSize = 80B, RuleWrites = 1*/
    rc = prvTgfVirtualTcamRuleSizesHa(7,1,1,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 80B with 1 rule write\n");

    /*RuleSize = 10B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHa(0,100,100,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 10B with 100 rule write\n");

    /*RuleSize = 20B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHa(1,100,100,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 20B with 100 rule write\n");

    /*RuleSize = 30B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHa(2,100,100,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 30B with 100 rule write\n");

    /*RuleSize = 40B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHa(3,100,100,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 40B with 100 rule write\n");

    /*RuleSize = 50B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHa(4,100,100,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 50B with 100 rule write\n");

    /*RuleSize = 60B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHa(5,100,100,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 60B with 100 rule write\n");

    /*RuleSize = 80B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHa(7,100,100,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 80B with 100 rule write\n");

    /*RuleSize = 10B, RuleWrites = 1000*/
    rc = prvTgfVirtualTcamRuleSizesHa(0,1000,1000,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 10B with 1000 rule write\n");

    /*RuleSize = 20B, RuleWrites = 1000*/
    rc = prvTgfVirtualTcamRuleSizesHa(1,1000,1000,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 20B with 1000 rule write\n");

    /*RuleSize = 30B, RuleWrites = 1000*/
    rc = prvTgfVirtualTcamRuleSizesHa(2,1000,1000,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 30B with 1000 rule write\n");

    /*RuleSize = 40B, RuleWrites = 1000*/
    rc = prvTgfVirtualTcamRuleSizesHa(3,1000,1000,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 40B with 1000 rule write\n");

    /*RuleSize = 50B, RuleWrites = 1000*/
    rc = prvTgfVirtualTcamRuleSizesHa(4,1000,1000,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 50B with 1000 rule write\n");

    /*RuleSize = 60B, RuleWrites = 1000*/
    rc = prvTgfVirtualTcamRuleSizesHa(5,1000,1000,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 60B with 1000 rule write\n");

    /*RuleSize = 80B, RuleWrites = 1000*/
    rc = prvTgfVirtualTcamRuleSizesHa(7,1000,1000,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 80B with 1000 rule write\n");
}

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_5010B function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) 50B rules & then 256 10B rules. Also, do for 1000 rules each
* */

GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_5010B
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 4, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 0, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 256 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 50B10B with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 4, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B with 256 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 0, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 256 rule write after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 50B10B with 256 rule write\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 50B10B with 256 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 4, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 0, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 1000 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 50B10B with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 4, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B with 1000 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 0, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 1000 rule write after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 50B10B with 1000 rule write\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 50B10B with 1000 rule write\n");
}

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_4020B function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) 40B rules & then 256 20B rules. Also, do for 1000 rules each
* */

GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_4020B
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 3, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 1, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B with 256 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 40B20B with 256 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 3, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B with 256 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 1, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B with 256 rule write after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 40B20B with 256 rule write\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 40B20B with 256 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 3, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 1, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B with 1000 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 40B20B with 1000 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 3, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B with 1000 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 1, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B with 1000 rule write after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 40B20B with 1000 rule write\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 40B20B with 1000 rule write\n");
}

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_4010B function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) 40B rules & then 256 10B rules. Also, do for 1000 rules each
* */

GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_4010B
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 3, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 0, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 256 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 40B10B with 256 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 3, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B with 256 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 0, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 256 rule write after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 40B10B with 256 rule write\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 40B10B with 256 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 3, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 0, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 1000 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 40B10B with 1000 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 3, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B with 1000 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 0, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 1000 rule write after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 40B10B with 1000 rule write\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 40B10B with 1000 rule write\n");
}

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_2010B function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) 20B rules & then 256 10B rules. Also, do for 1000 rules each
* */

GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_2010B
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 1, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 0, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 256 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 20B10B with 256 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 1, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B with 256 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 0, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 256 rule write after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 20B10B with 256 rule write\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 20B10B with 256 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 1, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 0, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 1000 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 20B10B with 1000 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 1, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B with 1000 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 0, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 1000 rule write after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 20B10B with 1000 rule write\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 20B10B with 1000 rule write\n");
}

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_802010B function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) 80B rules & then 256 20B rules & then 256 10B rules. Also, do for 1000 rules each
* */

GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_802010B
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 7, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 1, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 3, 0, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 256 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 80B20B10B with 256 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 7, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B with 256 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 1, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B with 256 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 3, 0, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 256 rule write after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 80B20B10B with 256 rule write\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 80B20B10B with 256 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 7, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 1, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 3, 0, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 1000 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 80B20B10B with 1000 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 7, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B with 1000 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 1, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B with 1000 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 3, 0, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B with 1000 rule write after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 80B20B10B with 1000 rule write\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 80B20B10B with 1000 rule write\n");
}

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_8040B function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) 80B rules & then 256 40B rules. Also, do for 1000 rules each
* */

GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_8040B
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 7, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 3, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B with 256 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 80B40B with 256 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 7, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B with 256 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 3, 256, 256, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B with 256 rule write after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 80B40B with 256 rule write\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 80B40B with 256 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 7, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 3, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B with 1000 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 80B40B with 1000 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 7, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B with 1000 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 3, 1000, 1000, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B with 1000 rule write after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 80B40B with 1000 rule write\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 80B40B with 1000 rule write\n");
}

/**
* @internal prvTgfVirtualTcamNormalRuleAddCheckHa function
* @endinternal
*
* @brief -  Test to install rules to vTcam after HA is complete (normal operation) - should fail for full vTcams & success for vTcams with free space
*/

GT_VOID prvTgfVirtualTcamNormalRuleAddCheckHa
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    vTcamMngId;

    vTcamMngId =  prvTgfDevNum % CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS;

    /*RuleSize = 10B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHa(0,100,99,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 10B with 100 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 0, 0, 1, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failed for 10B with 1 rule write rule ID = 99\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 0, 0, 10, 0, 100, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, rc, "cpssDxChVirtualTcamRuleWrite didnt fail for 10B with 10 rule write rule ID = 100\n");

    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,1,99);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed for 10B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 10B\n");

    /*RuleSize = 20B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHa(1,100,99,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 20B with 100 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 1, 0, 1, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failed for 20B with 1 rule write rule ID = 99\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 1, 0, 10, 0, 100, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, rc, "cpssDxChVirtualTcamRuleWrite didnt fail for 20B with 10 rule write rule ID = 100\n");

    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,1,99);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed for 20B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 20B\n");

    /*RuleSize = 30B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHa(2,100,99,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 30B with 100 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 2, 0, 1, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failed for 30B with 1 rule write rule ID = 99\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 2, 0, 10, 0, 100, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, rc, "cpssDxChVirtualTcamRuleWrite didnt fail for 30B with 10 rule write rule ID = 100\n");

    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,1,99);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed for 30B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 30B\n");

    /*RuleSize = 40B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHa(3,100,99,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 40B with 100 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 3, 0, 1, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failed for 40B with 1 rule write rule ID = 99\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 3, 0, 10, 0, 100, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, rc, "cpssDxChVirtualTcamRuleWrite didnt fail for 40B with 10 rule write rule ID = 100\n");

    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,1,99);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed for 40B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 40B\n");

    /*RuleSize = 50B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHa(4,100,99,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 50B with 100 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 4, 0, 1, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failed for 50B with 1 rule write rule ID = 99\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 4, 0, 10, 0, 100, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, rc, "cpssDxChVirtualTcamRuleWrite didnt fail for 50B with 10 rule write rule ID = 100\n");

    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,1,99);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed for 50B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 50B\n");

    /*RuleSize = 60B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHa(5,100,99,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 20B\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 5, 0, 1, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failed for 60B with 1 rule write rule ID = 99\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 5, 0, 10, 0, 100, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, rc, "cpssDxChVirtualTcamRuleWrite didnt fail for 60B with 10 rule write rule ID = 100\n");

    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,1,99);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed for 60B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 60B\n");

    /*RuleSize = 80B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHa(7,100,99,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHa failed for 80B with 100 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 7, 0, 1, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failed for 80B with 1 rule write rule ID = 99\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 7, 0, 10, 0, 100, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, rc, "cpssDxChVirtualTcamRuleWrite didnt fail for 80B with 10 rule write rule ID = 100\n");

    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,1,99);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed for 80B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 80B\n");
}

/**
* @internal prvTgfVirtualTcamSingleHoleStartHa function
* @endinternal
*
* @brief -  Test to install 100 rules for a vTcam with #guranteedrules = 200 & initial 100 ruleIDs not having ruleWrites
**/

GT_VOID prvTgfVirtualTcamSingleHoleStartHa
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 0, 200, 100, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 10B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 0, 200, 100, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 10B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 10B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 1, 200, 100, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 20B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 1, 200, 100, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 20B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 20B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 2, 200, 100, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 30B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 2, 200, 100, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 30B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 30B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 3, 200, 100, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 40B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 3, 200, 100, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 40B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 40B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 4, 200, 100, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 50B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 4, 200, 100, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 50B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 50B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 5, 200, 100, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 60B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 5, 200, 100, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 60B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 60B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 7, 200, 100, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 80B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 7, 200, 100, 0, 99, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 80B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 80B\n");
}

/**
* @internal prvTgfVirtualTcamMultipleHolesHa function
* @endinternal
*
* @brief -  Test to Install 100 rules for a vTcam with #guranteedrules = 200 & ruleWrites for ruleIds(30-59, 90-129, 150-179); Holes - (0-30, 60-89, 130-149, 180-199)
**/

GT_VOID prvTgfVirtualTcamMultipleHolesHa
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 0, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 10B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 0, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 10B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 10B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 1, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 1, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 1, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 20B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 1, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 1, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 1, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 20B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 20B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 2, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 2, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 2, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 30B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 2, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 2, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 2, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 30B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 30B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 3, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 3, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 3, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 40B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 3, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 3, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 3, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 40B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 40B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 4, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 4, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 4, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 50B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 4, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 4, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 4, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 50B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 50B \n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 5, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 5, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 5, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 60B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 5, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 5, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 5, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 60B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 60B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 7, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 7, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 7, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 80B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 7, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 7, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 7, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 80B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 80B\n");
}

/**
* @internal prvTgfVirtualTcamMultipleVtcamsMulipleHolesHa function
* @endinternal
*
* @brief -  Test to create 2 vTcams with MultipleHoles in each(MultipleHolesHa for each vTcam)
**/

GT_VOID prvTgfVirtualTcamMultipleVtcamsMulipleHolesHa
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 0, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 1, 1, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 1, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 1, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 0, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 1, 1, 200, 30, 0, 30, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 1, 200, 40, 0, 90, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 1, 200, 30, 0, 150, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed\n");
}

/**
* @internal prvTgfVirtualTcamMultipleVtcams100Ha_Single function
* @endinternal
*
* @brief -  Test to create 100 vtcams of varying size <10 for same rulesize
**/

GT_VOID prvTgfVirtualTcamMultipleVtcams100Ha_Single
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32 i;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 5, 10, 10, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B before HA\n");

    for(i=1;i<100;i++)
    {
        rc = vtcamHaApp(prvTgfDevNum, 0, 1, i, 5, 10, 10, 0, 0, 0, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B before HA\n");
    }

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 60B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 5, 10, 10, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B after HA\n");

    for(i=1;i<100;i++)
    {
        rc = vtcamHaApp(prvTgfDevNum, 0, 1, i, 5, 10, 10, 0, 0, 0, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B after HA\n");
    }

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 60B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 60B\n");
}

/**
* @internal prvTgfVirtualTcamMultipleVtcams100Ha_Multiple function
* @endinternal
*
* @brief -  Test to create 100 vtcams of varying size <10 for varying rulesize
**/

GT_VOID prvTgfVirtualTcamMultipleVtcams100Ha_Multiple
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32 i;

    rc = vtcamHaApp(prvTgfDevNum, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp Manager create failed for 30B before HA\n");

    for(i=0;i<100;i=i+2)
    {
        rc = vtcamHaApp(prvTgfDevNum, 0, 1, i, 2, 10, 10, 0, 0, 0, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B before HA\n");

        rc = vtcamHaApp(prvTgfDevNum, 0, 1, i+1, 5, 10, 10, 0, 0, 0, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B before HA\n");
    }

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 30B60B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp Manager create failed for 30B after HA\n");

    for(i=0;i<100;i=i+2)
    {
        rc = vtcamHaApp(prvTgfDevNum, 0, 1, i, 2, 10, 10, 0, 0, 0, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B after HA\n");

        rc = vtcamHaApp(prvTgfDevNum, 0, 1, i+1, 5, 10, 10, 0, 0, 0, 0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B after HA\n");
    }

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 30B60B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 30B60B\n");
}

/*Test Cases for Priority Driven Mode*/
GT_STATUS prvTgfVirtualTcamRuleSizesHaPriority
(
    GT_U32 ruleSize,
    GT_U32 GuranteedRules,
    GT_U32 rulesCount,
    GT_U32 tcamReset
)
{
    GT_STATUS rc;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, ruleSize, GuranteedRules, (rulesCount/2), 0, 0, 1, 40);
    if(rc != GT_OK)
    {
        cpssOsPrintf("vtcamHaApp failed before HA\n");
        return rc;
    }

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, ruleSize, GuranteedRules, (rulesCount - (rulesCount/2)), 0, (rulesCount/2), 1, 30);
    if(rc != GT_OK)
    {
        cpssOsPrintf("vtcamHaApp failed before HA\n");
        return rc;
    }

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    if(rc != GT_OK)
    {
        cpssOsPrintf("prvCpssDxChVirtualTcamHaSystemRecoverySet INIT failed\n");
        return rc;
    }

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, ruleSize, GuranteedRules, (rulesCount/2), 0, 0, 1, 40);
    if(rc != GT_OK)
    {
        cpssOsPrintf("vtcamHaApp failed in HA\n");
        return rc;
    }

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, ruleSize, GuranteedRules, (rulesCount - (rulesCount/2)), 0, (rulesCount/2), 1, 30);
    if(rc != GT_OK)
    {
        cpssOsPrintf("vtcamHaApp failed in HA\n");
        return rc;
    }

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    if(rc != GT_OK)
    {
        cpssOsPrintf("prvCpssDxChVirtualTcamHaSystemRecoverySet HW CATCHUP failed\n");
        return rc;
    }

    if(tcamReset)
    {
        rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
        if(rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDxChVirtualTcamHaSystemRecoverySet COMPLETION failed\n");
            return rc;
        }
    }
    else
    {
        rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 0);
        if(rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDxChVirtualTcamHaSystemRecoverySet COMPLETION failed\n");
            return rc;
        }
    }
    return GT_OK;
}

/*
 * @internal prvTgfVirtualTcamRuleSizeCheckHaPriority function
 * @endinternal
 *
 * @brief -  Tests to check vtcam creation/rules addition of different sizes - 10, 20, 30, 40, 50, 60, 80B
 * Vary the #rules as 1, 100, 1000
 */

GT_VOID prvTgfVirtualTcamRuleSizeCheckHaPriority
(
    GT_VOID
)
{
    GT_STATUS rc;
    /*RuleSize = 10B, RuleWrites = 1*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(0,1,1,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 10B with 1 rule write\n");

    /*RuleSize = 20B, RuleWrites = 1*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(1,1,1,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 20B with 1 rule write\n");

    /*RuleSize = 30B, RuleWrites = 1*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(2,1,1,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 30B with 1 rule write\n");

    /*RuleSize = 40B, RuleWrites = 1*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(3,1,1,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 40B with 1 rule write\n");

    /*RuleSize = 50B, RuleWrites = 1*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(4,1,1,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 50B with 1 rule write\n");

    /*RuleSize = 60B, RuleWrites = 1*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(5,1,1,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 60B with 1 rule write\n");

    /*RuleSize = 80B, RuleWrites = 1*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(7,1,1,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 80B with 1 rule write\n");

    /*RuleSize = 10B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(0,100,100,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 10B with 100 rule write\n");

    /*RuleSize = 20B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(1,100,100,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 20B with 100 rule write\n");

    /*RuleSize = 30B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(2,100,100,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 30B with 100 rule write\n");

    /*RuleSize = 40B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(3,100,100,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 40B with 100 rule write\n");

    /*RuleSize = 50B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(4,100,100,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 50B with 100 rule write\n");

    /*RuleSize = 60B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(5,100,100,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 60B with 100 rule write\n");

    /*RuleSize = 80B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(7,100,100,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 80B with 100 rule write\n");

    /*RuleSize = 10B, RuleWrites = 1000*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(0,1000,1000,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 10B with 1000 rule write\n");

    /*RuleSize = 20B, RuleWrites = 1000*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(1,1000,1000,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 20B with 1000 rule write\n");

    /*RuleSize = 30B, RuleWrites = 1000*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(2,1000,1000,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 30B with 1000 rule write\n");

    /*RuleSize = 40B, RuleWrites = 1000*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(3,1000,1000,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 40B with 1000 rule write\n");

    /*RuleSize = 50B, RuleWrites = 1000*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(4,1000,1000,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 50B with 1000 rule write\n");

    /*RuleSize = 60B, RuleWrites = 1000*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(5,1000,1000,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 60B with 1000 rule write\n");

    /*RuleSize = 80B, RuleWrites = 1000*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(7,1000,1000,1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 80B with 1000 rule write\n");
}

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_2RulesPriority function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) 50B rules & then 256 10B rules. Also, do for 1000 rules each
**/
GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_2RulesPriority
(
    GT_U32 ruleSize1,
    GT_U32 ruleSize2
)
{
    GT_STATUS rc;
    GT_U32 rulesCount = 0, rules1Half = 0, rules2Half = 0;

    rulesCount = 256;
    rules1Half = rulesCount/2;
    rules2Half = rulesCount - rules1Half;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, ruleSize1, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, ruleSize1, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, ruleSize2, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 2, ruleSize2, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 256 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed with 256 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, ruleSize1, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 256 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, ruleSize1, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 256 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, ruleSize2, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 256 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 2, ruleSize2, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 256 rule write after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed with 256 rule write in HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed with 256 rule write in HA\n");

    rulesCount = 1000;
    rules1Half = rulesCount/2;
    rules2Half = rulesCount - rules1Half;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, ruleSize1, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, ruleSize1, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, ruleSize2, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 2, ruleSize2, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 1000 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed with 1000 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, ruleSize1, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 1000 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, ruleSize1, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 1000 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, ruleSize2, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 1000 rule write after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 2, ruleSize2, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 1000 rule write after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed with 1000 rule write in HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed with 1000 rule write in HA\n");
}

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_3RulesPriority function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) 50B rules & then 256 10B rules. Also, do for 1000 rules each
**/

GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_3RulesPriority
(
    GT_U32 ruleSize1,
    GT_U32 ruleSize2,
    GT_U32 ruleSize3
)
{
    GT_STATUS rc;
    GT_U32 rulesCount = 0, rules1Half = 0, rules2Half = 0;

    rulesCount = 256;
    rules1Half = rulesCount/2;
    rules2Half = rulesCount - rules1Half;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, ruleSize1, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, ruleSize1, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, ruleSize2, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 2, ruleSize2, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 3, ruleSize3, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 256 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 3, ruleSize3, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 256 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed with 256 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, ruleSize1, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 256 rule write in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, ruleSize1, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 256 rule write in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, ruleSize2, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 256 rule write in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 2, ruleSize2, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 256 rule write in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 3, ruleSize3, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 256 rule write in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 3, ruleSize3, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 256 rule write in HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed with 256 rule write in HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed with 256 rule write in HA\n");

    rulesCount = 1000;
    rules1Half = rulesCount/2;
    rules2Half = rulesCount - rules1Half;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, ruleSize1, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, ruleSize1, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, ruleSize2, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 2, ruleSize2, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 3, ruleSize3, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 1000 rule write before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 3, ruleSize3, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 1000 rule write before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed with 1000 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, ruleSize1, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 1000 rule write in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, ruleSize1, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize1 with 1000 rule write in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, ruleSize2, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 1000 rule write in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 2, ruleSize2, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 1000 rule write in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 3, ruleSize3, rulesCount, rules1Half, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 1000 rule write in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 3, ruleSize3, rulesCount, rules2Half, 0, rules1Half, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for ruleSize2 with 1000 rule write in HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed with 1000 rule write in HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed with 1000 rule write in HA\n");
}

/**
* @internal prvTgfVirtualTcamNormalRuleAddCheckHaPriority function
* @endinternal
*
* @brief -  Test to install rules to vTcam after HA is complete (normal operation) - should fail for full vTcams & success for vTcams with free space
*/

GT_VOID prvTgfVirtualTcamNormalRuleAddCheckHaPriority
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    vTcamMngId;

    vTcamMngId =  prvTgfDevNum % CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS;

    /*RuleSize = 10B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(0,100,99,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 10B with 100 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 0, 0, 1, 0, 99, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failed for 10B with 1 rule write rule ID = 99\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 0, 0, 10, 0, 100, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, rc, "cpssDxChVirtualTcamRuleWrite didnt fail for 10B with 10 rule write rule ID = 100\n");

    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,1,99);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed for 10B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 10B\n");

    /*RuleSize = 20B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(1,100,99,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 20B with 100 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 1, 0, 1, 0, 99, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failed for 20B with 1 rule write rule ID = 99\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 1, 0, 10, 0, 100, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, rc, "cpssDxChVirtualTcamRuleWrite didnt fail for 20B with 1 rule write rule ID = 100\n");

    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,1,99);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed for 20B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 20B\n");

    /*RuleSize = 30B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(2,100,99,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 30B with 100 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 2, 0, 1, 0, 99, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failed for 30B with 1 rule write rule ID = 99\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 2, 0, 10, 0, 100, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, rc, "cpssDxChVirtualTcamRuleWrite didnt fail for 30B with 1 rule write rule ID = 100\n");

    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,1,99);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed for 30B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 30B\n");

    /*RuleSize = 40B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(3,100,99,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 40B with 100 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 3, 0, 1, 0, 99, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failed for 40B with 1 rule write rule ID = 99\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 3, 0, 10, 0, 100, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, rc, "cpssDxChVirtualTcamRuleWrite didnt fail for 40B with 1 rule write rule ID = 100\n");

    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,1,99);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed for 40B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 40B\n");

    /*RuleSize = 50B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(4,100,99,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 50B with 100 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 4, 0, 1, 0, 99, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failed for 50B with 1 rule write rule ID = 99\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 4, 0, 10, 0, 100, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, rc, "cpssDxChVirtualTcamRuleWrite didnt fail for 50B with 1 rule write rule ID = 100\n");

    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,1,99);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed for 50B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 50B\n");

    /*RuleSize = 60B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(5,100,99,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 60B with 100 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 5, 0, 1, 0, 99, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failed for 60B with 1 rule write rule ID = 99\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 5, 0, 10, 0, 100, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, rc, "cpssDxChVirtualTcamRuleWrite didnt fail for 60B with 1 rule write rule ID = 100\n");

    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,1,99);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed for 60B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 60B\n");

    /*RuleSize = 80B, RuleWrites = 100*/
    rc = prvTgfVirtualTcamRuleSizesHaPriority(7,100,99,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfVirtualTcamRuleSizesHaPriority failed for 80B with 100 rule write\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 7, 0, 1, 0, 99, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleWrite failed for 80B with 1 rule write rule ID = 99\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 7, 0, 10, 0, 100, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, rc, "cpssDxChVirtualTcamRuleWrite didnt fail for 80B with 1 rule write rule ID = 100\n");

    rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,1,99);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed for 80B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 80B\n");
}

/**
* @internal prvTgfVirtualTcamSingleHoleStartHaPriority function
* @endinternal
*
* @brief -  Test to install 100 rules for a vTcam with #guranteedrules = 200 & initial 100 ruleIDs not having ruleWrites
**/

GT_VOID prvTgfVirtualTcamSingleHoleStartHaPriority
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 0, 200, 50, 0, 99, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 50, 0, 149, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 10B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 0, 200, 50, 0, 99, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 50, 0, 149, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B in HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 10B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 10B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 1, 200, 50, 0, 99, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 1, 200, 50, 0, 149, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 20B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 1, 200, 50, 0, 99, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 1, 200, 50, 0, 149, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B in HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 20B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 20B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 2, 200, 50, 0, 99, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 2, 200, 50, 0, 149, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 30B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 2, 200, 50, 0, 99, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 2, 200, 50, 0, 149, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B in HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 30B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 30B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 3, 200, 50, 0, 99, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 3, 200, 50, 0, 149, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 40B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 3, 200, 50, 0, 99, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 3, 200, 50, 0, 149, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B in HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 40B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 40B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 4, 200, 50, 0, 99, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 4, 200, 50, 0, 149, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 50B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 4, 200, 50, 0, 99, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 4, 200, 50, 0, 149, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B in HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 50B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 50B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 5, 200, 50, 0, 99, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 5, 200, 50, 0, 149, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 60B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 5, 200, 50, 0, 99, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 5, 200, 50, 0, 149, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B in HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 60B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 60B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 7, 200, 50, 0, 99, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 7, 200, 50, 0, 149, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 80B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 7, 200, 50, 0, 99, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B in HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 7, 200, 50, 0, 149, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B in HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 80B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 80B\n");
}

/**
* @internal prvTgfVirtualTcamMultipleHolesHaPriority function
* @endinternal
*
* @brief -  Test to Install 100 rules for a vTcam with #guranteedrules = 200 & ruleWrites for ruleIds(30-59, 90-129, 150-179); Holes - (0-30, 60-89, 130-149, 180-199)
**/

GT_VOID prvTgfVirtualTcamMultipleHolesHaPriority
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 0, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 10B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 0, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 10B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 10B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 1, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 1, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 1, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 20B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 1, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 1, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 1, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 20B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 20B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 2, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 2, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 2, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 30B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 2, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 2, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 2, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 30B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 30B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 3, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 3, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 3, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 40B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 3, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 3, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 3, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 40B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 40B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 40B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 4, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 4, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 4, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 50B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 4, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 4, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 4, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 50B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 50B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 50B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 5, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 5, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 5, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 60B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 5, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 5, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 5, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 60B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 60B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 7, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 7, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 7, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 80B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 7, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 7, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 7, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 80B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 80B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 80B\n");
}

/**
* @internal prvTgfVirtualTcamMultipleVtcamsMulipleHolesHaPriority function
* @endinternal
*
* @brief -  Test to create 2 vTcams with MultipleHoles in each(MultipleHolesHa for each vTcam)
**/

GT_VOID prvTgfVirtualTcamMultipleVtcamsMulipleHolesHaPriority
(
    GT_VOID
)
{
    GT_STATUS rc;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 0, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 1, 1, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 1, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B before HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 1, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B before HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 0, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 0, 0, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 10B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 1, 1, 200, 30, 0, 30, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 1, 200, 40, 0, 90, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B after HA\n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 1, 200, 30, 0, 150, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 20B after HA\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed\n");
}

/**
* @internal prvTgfVirtualTcamMultipleVtcams100Ha_SinglePriority function
* @endinternal
*
* @brief -  Test to create 100 vtcams of varying size <10 for same rulesize
**/

GT_VOID prvTgfVirtualTcamMultipleVtcams100Ha_SinglePriority
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32 i;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 5, 10, 10, 0, 0, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B before HA\n");

    for(i=1;i<100;i++)
    {
        rc = vtcamHaApp(prvTgfDevNum, 0, 1, i, 5, 10, 10, 0, 0, 1, i);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B before HA\n");
    }

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 60B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 0, 5, 10, 10, 0, 0, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B after HA\n");

    for(i=1;i<100;i++)
    {
        rc = vtcamHaApp(prvTgfDevNum, 0, 1, i, 5, 10, 10, 0, 0, 1, i);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B after HA\n");
    }

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed for 60B\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 60B\n");
}

/**
* @internal prvTgfVirtualTcamMultipleVtcams100Ha_MultiplePriority function
* @endinternal
*
* @brief -  Test to create 100 vtcams of varying size <10 for varying rulesize
**/

GT_VOID prvTgfVirtualTcamMultipleVtcams100Ha_MultiplePriority
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32 i;

    rc = vtcamHaApp(prvTgfDevNum, 1, 0, 0, 0, 0, 0, 0, 0, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B before HA\n");

    for(i=0;i<100;i=i+2)
    {
        rc = vtcamHaApp(prvTgfDevNum, 0, 1, i, 2, 10, 10, 0, 0, 1, i);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B before HA\n");

        rc = vtcamHaApp(prvTgfDevNum, 0, 1, i+1, 5, 10, 10, 0, 0, 1, i);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B before HA\n");
    }

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 30B60B\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 0, 0, 0, 0, 0, 0, 0, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B after HA\n");

    for(i=0;i<100;i=i+2)
    {
        rc = vtcamHaApp(prvTgfDevNum, 0, 1, i, 2, 10, 10, 0, 0, 1, i);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 30B after HA\n");

        rc = vtcamHaApp(prvTgfDevNum, 0, 1, i+1, 5, 10, 10, 0, 0, 1, i);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed for 60B after HA\n");
    }

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed for 30B60B\n");
}

/*
 * @internal prvTgfVirtualTcamMultipleVtcamsPriorityLogicalMode function
 * @endinternal
 *
 * @brief -  Test to Create 4 vtams - 2 each of Priority & Logical Mode for rulesizes 30 & 60B
 */

GT_VOID prvTgfVirtualTcamMultipleVtcamsPriorityLogicalMode
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 2, 100, 100, 200, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 2, 100, 100, 200, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 3, 5, 100, 100, 200, 0, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 4, 5, 100, 100, 200, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 2, 100, 100, 200, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 2, 100, 100, 200, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 3, 5, 100, 100, 200, 0, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 4, 5, 100, 100, 200, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed \n");
}

/*
 * @internal prvTgfVirtualTcamEmptyVtcamsHa function
 * @endinternal
 *
 * @brief -  Test to Create 4 vtams - 2 empty and 2 full with ruleSizes 30 & 60B
 */
GT_VOID prvTgfVirtualTcamEmptyVtcamsHa
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 2, 100, 0, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 5, 100, 0, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 3, 5, 100, 100, 0, 0, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 4, 2, 100, 100, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 2, 100, 0, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 5, 100, 0, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 3, 5, 100, 100, 0, 0, 1, 30);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 4, 2, 100, 100, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed \n");
}

/*
 * @internal prvTgfVirtualTcamListsGetHa function
 * @endinternal
 *
 * @brief -  Test to check functionality of the two lists of rules corres. to unmapped and failed rules during config replay
 */
GT_VOID prvTgfVirtualTcamListsGetHa
(
    GT_VOID
)
{
    GT_STATUS                                            rc                   = GT_FAIL;
    GT_U32                                               bmpBackup;
    GT_U32                                               nokRulesCount = 0, unmappedRulesCount = 0;
    CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC  *gtNokRulesListPtr    = NULL;
    CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC   *unmappedRulesListPtr = NULL;
    CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC *haAppVtcamMngCfgParamPtr;

    haAppVtcamMngCfgParamPtr = &(VTCAM_GLOVAR(haAppVtcamMngCfgParam));

    bmpBackup = haAppVtcamMngCfgParamPtr->haFeaturesEnabledBmp;
    haAppVtcamMngCfgParamPtr->haFeaturesEnabledBmp = CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_SAVE_ALL_RULES_E;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 2, 3, 1000, 10, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    /* Preventing below config to simulate NOK during config-replay
       rc = vtcamHaApp(prvTgfDevNum, 0, 0, 2, 3, 1000, 11, 0, 800, 0, 0);
       UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");
     */

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 1, 7, 1000, 10, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 7, 1000, 10, 0, 800, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 2, 3, 1000, 10, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    VTCAM_GLOVAR(haapp_ignore_write_ret) = GT_TRUE;
    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 2, 3, 1000, 11, 0, 800, 0, 0);
    VTCAM_GLOVAR(haapp_ignore_write_ret) = GT_FALSE;

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 1, 7, 1000, 10, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    /* Preventing below config-replay to simulate un-mapped rules
       rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 7, 1000, 10, 0, 800, 1, 40);
       UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");
     */

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed \n");

    rc = cpssDxChVirtualTcamManagerHaConfigReplayRuleListsGet( &gtNokRulesListPtr, &unmappedRulesListPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA rule lists get failed \n");

    while(gtNokRulesListPtr)
    {
        nokRulesCount++;
        gtNokRulesListPtr = gtNokRulesListPtr->next;
    }

    while (unmappedRulesListPtr)
    {
        unmappedRulesCount++;
        unmappedRulesListPtr = unmappedRulesListPtr->next;
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(10, unmappedRulesCount, "HA unmapped rule list get failed\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(11, nokRulesCount, "HA NOK rule list get failed\n");

    rc = cpssDxChVirtualTcamManagerHaConfigReplayRuleListsFree();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA NOK rule list free failed \n");

    haAppVtcamMngCfgParamPtr->haFeaturesEnabledBmp = bmpBackup;
}
/*
 * @internal prvTgfVirtualTcamMultipleBlockHolesHa function
 * @endinternal
 *
 * @brief -  Test to Create 2 vtams  - 1 logical and then 1 priority vTcam with holes spanning multiple blocks- 40B & 80B
 */
GT_VOID prvTgfVirtualTcamMultipleBlockHolesHa
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 2, 3, 1000, 100, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 2, 3, 1000, 100, 0, 800, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 1, 7, 1000, 100, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 7, 1000, 100, 0, 800, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 2, 3, 1000, 100, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 2, 3, 1000, 100, 0, 800, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 1, 7, 1000, 100, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 7, 1000, 100, 0, 800, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed \n");
}

/*
 * @internal prvTgfVirtualTcamMultipleBlockHolesTest2Ha function
 * @endinternal
 *
 * @brief -  Test to Create 2 vtams  - 1 priority, 1 logical with holes spanning multiple blocks- 80B & 40B
 */
GT_VOID prvTgfVirtualTcamMultipleBlockHolesTest2Ha
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 7, 1000, 100, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 7, 1000, 100, 0, 800, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 3, 1000, 100, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 2, 3, 1000, 100, 0, 800, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 7, 1000, 100, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 7, 1000, 100, 0, 800, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 1, 2, 3, 1000, 100, 0, 0, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 2, 3, 1000, 100, 0, 800, 0, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed \n");
}

/*
 * @internal prvTgfVirtualTcamSingleRulePriorityHa function
 * @endinternal
 *
 * @brief -  Test to Create 1 priority vtam with a single ruleWrite & then add a rule after HA complete
 */
GT_VOID prvTgfVirtualTcamSingleRulePriorityHa
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 5, 10, 1, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 5, 10, 1, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed \n");

    rc = vtcamHaApp(prvTgfDevNum, 0, 0, 1, 5, 10, 1, 0, 1, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed \n");
}

/*
 * @internal prvTgfVirtualTcamAutoResizeHa function
 * @endinternal
 *
 * @brief -  Test to Create 1 priority  autoresized vtams with guaranteedSize 10 & 20 rule adds each
 */
GT_VOID prvTgfVirtualTcamAutoResizeHa
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 5, 10, 20, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed\n");

    rc = vtcamHaApp(prvTgfDevNum, 1, 1, 1, 5, 10, 20, 0, 0, 1, 40);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed \n");
}

/*
 * @internal prvTgfVirtualTcamZeroRuleHa function
 * @endinternal
 *
 * @brief -  Test to check valid/invalid rules with pattern and mask 0's - valid should be supported, invalid should fail
 */
GT_VOID prvTgfVirtualTcamZeroRuleHa
(
    GT_VOID
)
{
    GT_STATUS rc = GT_FAIL;

    /*check for valid rule*/
    rc = vtcamHaAppValidity(prvTgfDevNum, 1, 1, 1, 5, 1, 1, 0, 0, 0, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed\n");

    /*rule found in db*/
    rc = vtcamHaAppValidity(prvTgfDevNum, 1, 1, 1, 5, 1, 1, 0, 0, 0, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed \n");

    /*check for invalid rule*/
    rc = vtcamHaAppValidity(prvTgfDevNum, 1, 1, 1, 5, 1, 1, 0, 0, 0, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed\n");

    /*rule not part of db - fails*/
    rc = vtcamHaAppValidity(prvTgfDevNum, 1, 1, 1, 5, 1, 1, 0, 0, 0, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_FOUND, rc, "vtcamHaApp failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA CATCHUP failed \n");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed \n");
}

static GT_VOID prvTgfVirtualTcamTestConfigurationSetHa
(
    GT_U32                                          clientGroup,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize,
    GT_U32                                          haActive
)
{
    GT_STATUS                           rc;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC     vTcamInfo;
    GT_U32                              vTcamMngId;

    vTcamMngId =  prvTgfDevNum % CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS;

    /* Create vTCAM manager */
    rc = cpssDxChVirtualTcamManagerCreate(vTcamMngId, &vtcamMngCfgParam);
    if (rc == GT_ALREADY_EXIST)
    {
        cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        rc = cpssDxChVirtualTcamManagerCreate(vTcamMngId, &vtcamMngCfgParam);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerCreate failed\n");

    if (prvTgfVirtualTcamPostCreateMgrFuncPtr)
    {
        prvTgfVirtualTcamPostCreateMgrFuncPtr(vTcamMngId);
    }

    rc = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId, &prvTgfDevNum, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

    /* AUTODOC: SETUP CONFIGURATION: */
    /* AUTODOC: Create 30 Bytes logical index based vTCAMs */
    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
    vTcamInfo.ruleSize             = ruleSize;
    vTcamInfo.autoResize           = GT_FALSE;
    vTcamInfo.guaranteedNumOfRules = 10;
    vTcamInfo.ruleAdditionMethod   = CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;

    /* AUTODOC: create vTCAM */
    vTcamInfo.hitNumber   = 0;
    vTcamInfo.clientGroup = clientGroup;

    rc = cpssDxChVirtualTcamCreate(vTcamMngId, 1, &vTcamInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "cpssDxChVirtualTcamCreate FAILED for vTCAM = 1");

    vTcamInfo.ruleAdditionMethod   = CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

    rc = cpssDxChVirtualTcamCreate(vTcamMngId, 2, &vTcamInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "cpssDxChVirtualTcamCreate FAILED for vTCAM = 1");

    if(!haActive)
    {
        /* AUTODOC: Save PCL TCAM group binding for restore */
        rc = prvTgfTcamClientGroupGet(PRV_TGF_TCAM_IPCL_0_E, &tcamGroup, &tcamGroupEnable);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "prvTgfTcamClientGroupGet: %d, %d, %d",
                PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);

        /* AUTODOC: Save TTI TCAM group binding for restore */
        rc = prvTgfTcamClientGroupGet(PRV_TGF_TCAM_TTI_E, &ttiTcamGroup, &ttiTcamGroupEnable);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "prvTgfTcamClientGroupGet: %d, %d, %d",
                PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);

        /* AUTODOC: PCL TCAM group binding */
        rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_IPCL_0_E, PCL_TCAM_GROUP_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
                PRV_TGF_TCAM_IPCL_0_E, PCL_TCAM_GROUP_CNS, GT_TRUE);

        /* AUTODOC: TTI TCAM group binding */
        rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_TTI_E, TTI_TCAM_GROUP_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
                PRV_TGF_TCAM_TTI_E, TTI_TCAM_GROUP_CNS, GT_TRUE);

        /* AUTODOC: Init IPCL Engine for send port */
        rc = prvTgfPclDefPortInit(
                prvTgfPortsArray[ING_PORT_IDX_CNS],
                CPSS_PCL_DIRECTION_INGRESS_E,
                CPSS_PCL_LOOKUP_0_E,
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E /*ipv6Key*/);
        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

        /* AUTODOC: save TTI lookup key for port 0 */
        rc = prvTgfTtiPortLookupEnableGet(prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, &ttiTcamLookupEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableGet: %d", prvTgfDevNum);

        /* AUTODOC: save MAC_MODE_DA for TTI_KEY_ETH */
        rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_ETH_E, &ttiTcamMacMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

        /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_ETH */
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

        /* AUTODOC: set MAC_MODE_DA for TTI_KEY_ETH */
        rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, PRV_TGF_TTI_MAC_MODE_DA_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);
    }
}

static void haTestTrafficGenerate
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U8                           ruleStart,
    IN  GT_U8                           ruleEnd
)
{
    GT_STATUS   st;
    GT_U32      i;

    /* DATA of bypass packet */
    static GT_U8 payloadDataArr[48] ={0x33, 0x33, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    /* Bypass PAYLOAD part */
    static TGF_PACKET_PAYLOAD_STC payloadPart = {
        sizeof(payloadDataArr),                       /* dataLength */
        payloadDataArr                                /* dataPtr */
    };
    /*  L2 part of packet  */
    static TGF_PACKET_L2_STC packetL2Part = {
        {0x0C, 0x06, 0x01, 0x02, 0x03, 0x00},                /* daMac */
        {0x00, 0x11, 0x22, 0x44, 0x55, 0x66}                 /* saMac */
    };
    /* PARTS of packet */
    static TGF_PACKET_PART_STC packetPartArray[] = {
        {TGF_PACKET_PART_L2_E,        &packetL2Part},  /* type, partPtr */
        {TGF_PACKET_PART_PAYLOAD_E,   &payloadPart}
    };
    /*  PACKET to send info */
    static TGF_PACKET_STC packetInfo = {
    /* LENGTH of packet with CRC */
        (TGF_L2_HEADER_SIZE_CNS + sizeof(payloadDataArr) + TGF_CRC_LEN_CNS),                                          /* totalLen */
        sizeof(packetPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
        packetPartArray                                        /* partsArray */
    };

    for (i = ruleStart; (i < ruleEnd); i++)
    {
        packetL2Part.daMac[5] = (GT_U8)i;
        /* setup Packet */
        st = prvTgfSetTxSetupEth(devNum, &packetInfo, 1, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "prvTgfSetTxSetupEth error");

        /* send Packet from port portNum */
        st = prvTgfStartTransmitingEth(devNum, portNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, st, "prvTgfStartTransmitingEth error");
    }

    /* wait to get stable traffic */
    PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_MAC(10);
}

static void ruleWriteHa
(
    GT_U32 mode,
    GT_U32 vTcamId,
    GT_U32 ruleId,
    GT_U32 priority,
    GT_U32 macDaPatternLow,
    GT_U16 macDaPatternHigh,
    GT_U32 cncIndex
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    PRV_TGF_PCL_ACTION_STC                     action;
    CPSS_DXCH_PCL_ACTION_STC                   dxChAction;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;
    GT_U32                                     vTcamMngId;

    vTcamMngId =  prvTgfDevNum % CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS;

    if(mode)
        ruleAttributes.priority = priority;

    /* AUTODOC: mask for DST MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    mask.ruleStdNotIp.macDa.arEther[0] = (((macDaPatternHigh <<  8) & 0xFF) ? 0xFF : 0);
    mask.ruleStdNotIp.macDa.arEther[1] = ((macDaPatternHigh & 0xFF)         ? 0xFF : 0);
    mask.ruleStdNotIp.macDa.arEther[2] = (((macDaPatternLow  << 24) & 0xFF) ? 0xFF : 0);
    mask.ruleStdNotIp.macDa.arEther[3] = (((macDaPatternLow  << 16) & 0xFF) ? 0xFF : 0);
    mask.ruleStdNotIp.macDa.arEther[4] = (((macDaPatternLow  <<  8) & 0xFF) ? 0xFF : 0);
    mask.ruleStdNotIp.macDa.arEther[5] = ((macDaPatternLow  & 0xFF)         ? 0xFF : 0);
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    pattern.ruleStdNotIp.macDa.arEther[0] = ((macDaPatternHigh <<  8) & 0xFF);
    pattern.ruleStdNotIp.macDa.arEther[1] = (macDaPatternHigh & 0xFF)        ;
    pattern.ruleStdNotIp.macDa.arEther[2] = ((macDaPatternLow  << 24) & 0xFF);
    pattern.ruleStdNotIp.macDa.arEther[3] = ((macDaPatternLow  << 16) & 0xFF);
    pattern.ruleStdNotIp.macDa.arEther[4] = ((macDaPatternLow  <<  8) & 0xFF);
    pattern.ruleStdNotIp.macDa.arEther[5] = (macDaPatternLow  & 0xFF)        ;

    /* AUTODOC: action0 - redirect to port A */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                         = PRV_TGF_PACKET_CMD_FORWARD_E;
    action.mirror.cpuCode                 = CPSS_NET_FIRST_USER_DEFINED_E;
    action.redirect.redirectCmd           = PRV_TGF_PCL_ACTION_REDIRECT_CMD_NONE_E;
    action.matchCounter.enableMatchCount  = GT_TRUE;
    action.matchCounter.matchCounterIndex = cncIndex;
    prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);

    ruleData.valid                   = GT_TRUE;
    ruleData.rule.pcl.actionPtr      = &dxChAction;
    ruleData.rule.pcl.maskPtr        = &mask;
    ruleData.rule.pcl.patternPtr     = &pattern;
    tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
    tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    rc = cpssDxChVirtualTcamRuleWrite(
        vTcamMngId, vTcamId, ruleId,
        &ruleAttributes, &tcamRuleType, &ruleData);

    if (ignore_write_ret == GT_FALSE)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
                vTcamId, ruleId);
    }
}

static GT_STATUS ruleWriteHa2
(
    GT_U32 mode,
    GT_U32 vTcamId,
    GT_U32 ruleId,
    GT_U32 priority,
    GT_U32 macDaPatternLow,
    GT_U16 macDaPatternHigh,
    GT_U32 cncIndex
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    PRV_TGF_PCL_ACTION_STC                     action;
    CPSS_DXCH_PCL_ACTION_STC                   dxChAction;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;
    GT_U32                                     vTcamMngId;

    vTcamMngId =  prvTgfDevNum % CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS;

    if(mode)
        ruleAttributes.priority = priority;

    /* AUTODOC: mask for DST MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    mask.ruleStdNotIp.macDa.arEther[0] = (((macDaPatternHigh <<  8) & 0xFF) ? 0xFF : 0);
    mask.ruleStdNotIp.macDa.arEther[1] = ((macDaPatternHigh & 0xFF)         ? 0xFF : 0);
    mask.ruleStdNotIp.macDa.arEther[2] = (((macDaPatternLow  << 24) & 0xFF) ? 0xFF : 0);
    mask.ruleStdNotIp.macDa.arEther[3] = (((macDaPatternLow  << 16) & 0xFF) ? 0xFF : 0);
    mask.ruleStdNotIp.macDa.arEther[4] = (((macDaPatternLow  <<  8) & 0xFF) ? 0xFF : 0);
    mask.ruleStdNotIp.macDa.arEther[5] = ((macDaPatternLow  & 0xFF)         ? 0xFF : 0);
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    pattern.ruleStdNotIp.macDa.arEther[0] = ((macDaPatternHigh <<  8) & 0xFF);
    pattern.ruleStdNotIp.macDa.arEther[1] = (macDaPatternHigh & 0xFF)        ;
    pattern.ruleStdNotIp.macDa.arEther[2] = ((macDaPatternLow  << 24) & 0xFF);
    pattern.ruleStdNotIp.macDa.arEther[3] = ((macDaPatternLow  << 16) & 0xFF);
    pattern.ruleStdNotIp.macDa.arEther[4] = ((macDaPatternLow  <<  8) & 0xFF);
    pattern.ruleStdNotIp.macDa.arEther[5] = (macDaPatternLow  & 0xFF)        ;

    /* AUTODOC: action0 - redirect to port A */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                         = PRV_TGF_PACKET_CMD_FORWARD_E;
    action.mirror.cpuCode                 = CPSS_NET_FIRST_USER_DEFINED_E;
    action.redirect.redirectCmd           = PRV_TGF_PCL_ACTION_REDIRECT_CMD_NONE_E;
    action.matchCounter.enableMatchCount  = GT_TRUE;
    action.matchCounter.matchCounterIndex = cncIndex;
    prvTgfConvertGenericToDxChRuleAction(&action, &dxChAction);

    ruleData.valid                   = GT_TRUE;
    ruleData.rule.pcl.actionPtr      = &dxChAction;
    ruleData.rule.pcl.maskPtr        = &mask;
    ruleData.rule.pcl.patternPtr     = &pattern;
    tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
    tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    rc = cpssDxChVirtualTcamRuleWrite(
        vTcamMngId, vTcamId, ruleId,
        &ruleAttributes, &tcamRuleType, &ruleData);

   return rc;
}

static void  haTestCncCountersCheckOrReset
(
    IN   GT_U32   portNum,
    IN   GT_U32   cncBlock,
    IN   GT_U32   expectedPackets,
    IN   GT_BOOL  check,
    IN   GT_U8    ruleStart,
    IN   GT_U8    ruleEnd
)
{
    GT_STATUS               rc = GT_OK; /* return code */
    PRV_TGF_CNC_COUNTER_STC cncCounter;         /* counter */
    GT_PORT_GROUPS_BMP      portGroupBmp;    /* port group bitmap */
    GT_U32                  i;

    /* use PortGroupsBmp API in the test */
    /* set <currPortGroupsBmp> */
    TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(portNum);
    portGroupBmp = currPortGroupsBmp;

    for (i = ruleStart; (i < ruleEnd); i++)
    {
        rc = prvTgfCncPortGroupCounterGet(portGroupBmp,
            cncBlock, i,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &cncCounter);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfCncPortGroupCounterGet: cncBlock = %d, index = %d",
            cncBlock, i);
        if (check != GT_FALSE)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(
                expectedPackets, cncCounter.packetCount.l[0], "expected and found");
            PRV_UTF_LOG3_MAC(
                "CNC block %d index %d packets %d\n",
                cncBlock, i, cncCounter.packetCount.l[0]);
        }
    }

    /* restore values after using currPortGroupsBmp*/
    usePortGroupsBmp = GT_FALSE;
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
}

GT_VOID prvTgfVirtualTcamTrafficTestReplayHa
(
    GT_VOID
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT       ruleSize;
    GT_U32                                     indexRangesBmp[4];
    GT_U32                                     cncBlockNum = 1;
    GT_U32                                     clientGroup = PCL_TCAM_GROUP_CNS;
    PRV_TGF_CNC_CLIENT_ENT                     client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;
    GT_U32                                     counterIdx;
    PRV_TGF_CNC_COUNTER_STC                    counter;
    GT_U32                                     floorIndex;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC    *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning;
    GT_U32                                     cncBlockCntrs;
    GT_U32                                     cncBlocks;

    ruleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    /*  Create vTcamManager 0 with vTcam 1(priority) & vTcam 2(logical) */
    prvTgfVirtualTcamTestConfigurationSetHa(clientGroup, ruleSize, 0);

    /*  Preventing Config of second rule, to simulate failed rule write, which will be attempted during config replay */
    /*  Configure 2 PCL rules - 1 each in vtcam1 & vtcam2 */
    ruleWriteHa(1, 1, 0, 5, (0x01020300 | 1), (GT_U16)0x0C06, 1);

    /*
    ruleWriteHa(0, 2, 0, 5, (0x01020300 | 2), (GT_U16)0x0C06, 2);
     */

    /* use first range */
    indexRangesBmp[0] = 0x1;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    /* configure CNC */
    rc = prvTgfCncTestCncBlockConfigure(
            cncBlockNum,
            client,
            GT_TRUE /*enable*/,
            indexRangesBmp,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfCncTestCncBlockConfigure: block %d, client %d",
            cncBlockNum,
            client);

    /* AUTODOC: enable PCL clients for CNC */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS ))
    {
        rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    }

    /* AUTODOC: Clean CNC counters */
    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_FALSE  /*check*/, 1, 5);

    /* Test traffic to hit 2 configured rules */
    haTestTrafficGenerate(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], 1, 3);

    /* AUTODOC: Clean CNC counters */
    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_TRUE  /*check*/, 1, 2);

    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            0 /*expectedPackets*/,
            GT_TRUE  /*check*/, 2, 3);

    /* HA started - INIT State: DB(no h/w) cleared */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 30B60B\n");

    /* HA config replay started */
    prvTgfVirtualTcamTestConfigurationSetHa(clientGroup, ruleSize, 1);

    ruleWriteHa(1, 1, 0, 5, (0x01020300 | 1), (GT_U16)0x0C06, 1);

    ignore_write_ret = GT_TRUE;
    ruleWriteHa(0, 2, 0, 5, (0x01020300 | 2), (GT_U16)0x0C06, 2);
    ignore_write_ret = GT_FALSE;

    /* HA Catch up started - DB recreated */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    /* HA Complete - DB intact */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed\n");

    /* Test traffic to hit 2 configured rules */
    haTestTrafficGenerate(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], 1, 3);

    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_TRUE  /*check*/, 1, 3);

    /* After HA(normal oper.)  - configure 2 PCL rules - 1 each in vtcam1 & vtcam2 */
    ruleWriteHa(1, 1, 5, 6, (0x01020300 | 3), (GT_U16)0x0C06, 3);

    ruleWriteHa(0, 2, 2, 0, (0x01020300 | 4), (GT_U16)0x0C06, 4);

    /* Test traffic to hit 4 configured rules */
    haTestTrafficGenerate(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], 1, 5);

    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_TRUE  /*check*/, 1, 5);


    /* AUTODOC: RESTORE CONFIGURATION: */

    cpssOsMemSet(&counter, 0, sizeof(counter));

    /* get CNC size info */
    prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

    /* use first range */
    indexRangesBmp[0] = 0x0;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    {
        /* Get CNC client for a block */
        client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;

        /* disable client */
        rc = prvTgfCncBlockClientEnableSet(
                cncBlockNum, client, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", cncBlockNum, client);

        /* reset ranges */
        rc = prvTgfCncBlockClientRangesSet(
                cncBlockNum, client, indexRangesBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", cncBlockNum, client);
    }
    {
        /* Get CNC client for a block */
        client = PRV_TGF_CNC_CLIENT_TTI_E;

        /* disable client */
        rc = prvTgfCncBlockClientEnableSet(
                cncBlockNum, client, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", cncBlockNum, client);

        /* reset ranges */
        rc = prvTgfCncBlockClientRangesSet(
                cncBlockNum, client, indexRangesBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", cncBlockNum, client);
    }
    /* clean counters */
    for (counterIdx = 0; counterIdx < cncBlockCntrs; counterIdx++)
    {
        rc = prvTgfCncCounterSet(
                cncBlockNum, counterIdx,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncCounterSet: block %d idx %d", cncBlockNum, counterIdx);
    }

    /* disable PCL clients for CNC */
    rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E,GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed\n");

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy for port 1 (the ingress port) */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            GT_FALSE);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: restore TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, ttiTcamLookupEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore MAC mode for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, ttiTcamMacMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
            GT_TRUE);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
            prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Restore TCAM floor info */
    for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet(); floorIndex++ )
    {
        rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex,
                &saveFloorInfoArr[floorIndex][0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
                floorIndex);
    }

    /* AUTODOC: Restore PCL TCAM group binding */
    rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
            PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);

    /* AUTODOC: Restore TTI TCAM group binding */
    rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
            PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);

    /* Restore TCAM Active number of floors */
    rc = cpssDxChTcamActiveFloorsSet(prvTgfDevNum,
            fineTuningPtr->tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);

    UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "cpssDxChTcamActiveFloorsSet failed for device: %d", prvTgfDevNum);
}

GT_VOID prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaPositive
(
    GT_VOID
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT       ruleSize;
    GT_U32                                     clientGroup = PCL_TCAM_GROUP_CNS;
    GT_U32                                     floorIndex;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC   *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning;
    GT_U32                                     bmpBackup;

    bmpBackup = vtcamMngCfgParam.haFeaturesEnabledBmp;
    vtcamMngCfgParam.haFeaturesEnabledBmp = CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_REPLAY_MATCH_FIRST_OCCURRENCE_ONLY_E;

    ruleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    /*  Create vTcamManager 0 with vTcam 1(priority) & vTcam 2(logical) */
    prvTgfVirtualTcamTestConfigurationSetHa(clientGroup, ruleSize, 0);

    /*  Preventing Config of second rule, to simulate failed rule write, which will be attempted during config replay */
    /*  Configure 2 PCL rules - 1 each in vtcam1 & vtcam2 */
    rc = ruleWriteHa2 (1, 1, 0, 5, (0x01020300 | 1), (GT_U16)0x0C06, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=1, prio=5) (config) failed.\n");

    rc = ruleWriteHa2 (1, 1, 5, 6, (0x01020300 | 1), (GT_U16)0x0C06, 3);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=1, prio=6) (config) failed.\n");

    rc = ruleWriteHa2 (0, 2, 0, 5, (0x01020300 | 2), (GT_U16)0x0C06, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=2, rule-id=0) (config) failed.\n");

    rc = ruleWriteHa2 (0, 2, 1, 5, (0x01020300 | 2), (GT_U16)0x0C06, 4);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=2, rule-id=0) (config) failed.\n");

    /* HA started - INIT State: DB(no h/w) cleared */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 30B60B\n");

    /* HA config replay started */
    prvTgfVirtualTcamTestConfigurationSetHa (clientGroup, ruleSize, 1);

    rc = ruleWriteHa2 (1, 1, 0, 5, (0x01020300 | 1), (GT_U16)0x0C06, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=1, prio=5) (config) failed.\n");

    if (prvTgfVirtualTcamPostCreateMgrFunBound == 0)
    {
        /* if HW DB used the rule will be found - let it to be cleared */
        rc = ruleWriteHa2 (1, 1, 5, 6, (0x01020300 | 1), (GT_U16)0x0C06, 3);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_FOUND, rc, "Rule write (vtcam=1, prio=6) (config) failed.\n");
    }

    rc = ruleWriteHa2 (0, 2, 0, 5, (0x01020300 | 2), (GT_U16)0x0C06, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=2, rule-id=0) (config) failed.\n");

    if (prvTgfVirtualTcamPostCreateMgrFunBound == 0)
    {
        /* if HW DB used the rule will be found - let it to be cleared */
        rc = ruleWriteHa2 (0, 2, 1, 5, (0x01020300 | 2), (GT_U16)0x0C06, 4);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_FOUND, rc, "Rule write (vtcam=2, rule-id=0) (config) failed.\n");
    }


    /* HA Catch up started - DB recreated */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    /* HA Complete - DB intact */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed\n");

    /* restore haFeaturesEnabledBmp */
    vtcamMngCfgParam.haFeaturesEnabledBmp = bmpBackup;

    /* AUTODOC: RESTORE CONFIGURATION: */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed\n");

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy for port 1 (the ingress port) */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            GT_FALSE);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: restore TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, ttiTcamLookupEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore MAC mode for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, ttiTcamMacMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
            GT_TRUE);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
            prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Restore TCAM floor info */
    for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet(); floorIndex++ )
    {
        rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex,
                &saveFloorInfoArr[floorIndex][0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
                floorIndex);
    }

    /* AUTODOC: Restore PCL TCAM group binding */
    rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
            PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);

    /* AUTODOC: Restore TTI TCAM group binding */
    rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
            PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);

    /* Restore TCAM Active number of floors */
    rc = cpssDxChTcamActiveFloorsSet(prvTgfDevNum,
            fineTuningPtr->tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);

    UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "cpssDxChTcamActiveFloorsSet failed for device: %d", prvTgfDevNum);
}


GT_VOID prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaNegative
(
    GT_VOID
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT       ruleSize;
    GT_U32                                     clientGroup = PCL_TCAM_GROUP_CNS;
    GT_U32                                     floorIndex;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC   *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning;
    GT_U32                                     bmpBackup;

    bmpBackup = vtcamMngCfgParam.haFeaturesEnabledBmp;
    vtcamMngCfgParam.haFeaturesEnabledBmp = 0;
    /* Disable the below mode:
       CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_REPLAY_MATCH_FIRST_OCCURRENCE_ONLY_E;
     */

    ruleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    /*  Create vTcamManager 0 with vTcam 1(priority) & vTcam 2(logical) */
    prvTgfVirtualTcamTestConfigurationSetHa(clientGroup, ruleSize, 0);

    /*  Preventing Config of second rule, to simulate failed rule write, which will be attempted during config replay */
    /*  Configure 2 PCL rules - 1 each in vtcam1 & vtcam2 */
    rc = ruleWriteHa2 (1, 1, 0, 5, (0x01020300 | 1), (GT_U16)0x0C06, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=1, prio=5) (config) failed.\n");

    rc = ruleWriteHa2 (1, 1, 5, 6, (0x01020300 | 1), (GT_U16)0x0C06, 3);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=1, prio=6) (config) failed.\n");

    rc = ruleWriteHa2 (0, 2, 0, 5, (0x01020300 | 2), (GT_U16)0x0C06, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=2, rule-id=0) (config) failed.\n");

    rc = ruleWriteHa2 (0, 2, 1, 5, (0x01020300 | 2), (GT_U16)0x0C06, 4);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=2, rule-id=0) (config) failed.\n");

    /* HA started - INIT State: DB(no h/w) cleared */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 30B60B\n");

    /* HA config replay started */
    prvTgfVirtualTcamTestConfigurationSetHa (clientGroup, ruleSize, 1);

    rc = ruleWriteHa2 (1, 1, 0, 5, (0x01020300 | 1), (GT_U16)0x0C06, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=1, prio=5) (config) failed.\n");

    rc = ruleWriteHa2 (1, 1, 5, 6, (0x01020300 | 1), (GT_U16)0x0C06, 3);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=1, prio=6) (config) failed.\n");

    rc = ruleWriteHa2 (0, 2, 0, 5, (0x01020300 | 2), (GT_U16)0x0C06, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=2, rule-id=0) (config) failed.\n");

    rc = ruleWriteHa2 (0, 2, 1, 5, (0x01020300 | 2), (GT_U16)0x0C06, 4);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=2, rule-id=0) (config) failed.\n");


    /* HA Catch up started - DB recreated */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    /* HA Complete - DB intact */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed\n");

    /* restore haFeaturesEnabledBmp */
    vtcamMngCfgParam.haFeaturesEnabledBmp = bmpBackup;

    /* AUTODOC: RESTORE CONFIGURATION: */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed\n");

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy for port 1 (the ingress port) */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            GT_FALSE);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: restore TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, ttiTcamLookupEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore MAC mode for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, ttiTcamMacMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
            GT_TRUE);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
            prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Restore TCAM floor info */
    for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet(); floorIndex++ )
    {
        rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex,
                &saveFloorInfoArr[floorIndex][0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
                floorIndex);
    }

    /* AUTODOC: Restore PCL TCAM group binding */
    rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
            PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);

    /* AUTODOC: Restore TTI TCAM group binding */
    rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
            PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);

    /* Restore TCAM Active number of floors */
    rc = cpssDxChTcamActiveFloorsSet(prvTgfDevNum,
            fineTuningPtr->tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);

    UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "cpssDxChTcamActiveFloorsSet failed for device: %d", prvTgfDevNum);
}


GT_VOID prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModeNegative
(
    GT_VOID
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT       ruleSize;
    GT_U32                                     indexRangesBmp[4];
    GT_U32                                     cncBlockNum = 1;
    GT_U32                                     clientGroup = PCL_TCAM_GROUP_CNS;
    PRV_TGF_CNC_CLIENT_ENT                     client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;
    GT_U32                                     counterIdx;
    PRV_TGF_CNC_COUNTER_STC                    counter;
    GT_U32                                     floorIndex;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC    *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning;
    GT_U32                                     cncBlockCntrs;
    GT_U32                                     cncBlocks;
    GT_U32                                     bmpBackup;


    bmpBackup = vtcamMngCfgParam.haFeaturesEnabledBmp;
    vtcamMngCfgParam.haFeaturesEnabledBmp = CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_REPLAY_HIGHER_PRIO_FIRST_E;

    ruleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    /*  Create vTcamManager 0 with vTcam 1(priority) & vTcam 2(logical) */
    prvTgfVirtualTcamTestConfigurationSetHa(clientGroup, ruleSize, 0);

    /*  Preventing Config of second rule, to simulate failed rule write, which will be attempted during config replay */
    /*  Configure 2 PCL rules - 1 each in vtcam1 & vtcam2 */
    ruleWriteHa(1, 1, 0, 5, (0x01020300 | 1), (GT_U16)0x0C06, 1);
    ruleWriteHa(0, 2, 0, 5, (0x01020300 | 2), (GT_U16)0x0C06, 2);
    ruleWriteHa(1, 1, 5, 6, (0x01020300 | 3), (GT_U16)0x0C06, 3);

    /* use first range */
    indexRangesBmp[0] = 0x1;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    /* configure CNC */
    rc = prvTgfCncTestCncBlockConfigure(
            cncBlockNum,
            client,
            GT_TRUE /*enable*/,
            indexRangesBmp,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfCncTestCncBlockConfigure: block %d, client %d",
            cncBlockNum,
            client);

    /* AUTODOC: enable PCL clients for CNC */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS ))
    {
        rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    }

    /* AUTODOC: Clean CNC counters */
    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_FALSE  /*check*/, 1, 5);

    /* Test traffic to hit 2 configured rules */
    haTestTrafficGenerate(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], 1, 4);

    /* AUTODOC: Clean CNC counters */
    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_TRUE  /*check*/, 1, 4);

    /* HA started - INIT State: DB(no h/w) cleared */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 30B60B\n");

    /* HA config replay started */
    prvTgfVirtualTcamTestConfigurationSetHa(clientGroup, ruleSize, 1);

    rc = ruleWriteHa2(1, 1, 5, 6, (0x01020300 | 3), (GT_U16)0x0C06, 3);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=1, prio=6) (config-replay) failed.\n");

    if (prvTgfVirtualTcamPostCreateMgrFunBound == 0)
    {
        /* if used HW DB rule will be found - let it be cleared insted */
        rc = ruleWriteHa2(1, 1, 0, 5, (0x01020300 | 1), (GT_U16)0x0C06, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_NOT_FOUND, rc, "Rule write (vtcam=1, prio=5) (config-replay) failed.\n");
    }

    rc = ruleWriteHa2(0, 2, 0, 5, (0x01020300 | 2), (GT_U16)0x0C06, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=2, ruleId=0) (config-replay) failed.\n");

    /* HA Catch up started - DB recreated */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    /* HA Complete - DB intact */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed\n");

    /* Test traffic to hit 2 configured rules */
    haTestTrafficGenerate(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], 1, 4);

    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_TRUE  /*check*/, 2, 4);

    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            0 /*expectedPackets*/,
            GT_TRUE  /*check*/, 1, 2);
    /* After HA(normal oper.)  - configure 1 PCL rules - 1 in vtcam2 */

    ruleWriteHa(0, 2, 2, 0, (0x01020300 | 4), (GT_U16)0x0C06, 4);

    /* restore haFeaturesEnabledBmp */
    vtcamMngCfgParam.haFeaturesEnabledBmp = bmpBackup;

    /* Test traffic to hit 4 configured rules */
    haTestTrafficGenerate(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], 1, 5);

    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            0 /*expectedPackets*/,
            GT_TRUE  /*check*/, 1, 2);

    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_TRUE  /*check*/, 2, 5);

    /* AUTODOC: RESTORE CONFIGURATION: */

    cpssOsMemSet(&counter, 0, sizeof(counter));

    /* get CNC size info */
    prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

    /* use first range */
    indexRangesBmp[0] = 0x0;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    {
        /* Get CNC client for a block */
        client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;

        /* disable client */
        rc = prvTgfCncBlockClientEnableSet(
                cncBlockNum, client, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", cncBlockNum, client);

        /* reset ranges */
        rc = prvTgfCncBlockClientRangesSet(
                cncBlockNum, client, indexRangesBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", cncBlockNum, client);
    }
    {
        /* Get CNC client for a block */
        client = PRV_TGF_CNC_CLIENT_TTI_E;

        /* disable client */
        rc = prvTgfCncBlockClientEnableSet(
                cncBlockNum, client, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", cncBlockNum, client);

        /* reset ranges */
        rc = prvTgfCncBlockClientRangesSet(
                cncBlockNum, client, indexRangesBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", cncBlockNum, client);
    }
    /* clean counters */
    for (counterIdx = 0; counterIdx < cncBlockCntrs; counterIdx++)
    {
        rc = prvTgfCncCounterSet(
                cncBlockNum, counterIdx,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncCounterSet: block %d idx %d", cncBlockNum, counterIdx);
    }

    /* disable PCL clients for CNC */
    rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E,GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed\n");

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy for port 1 (the ingress port) */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            GT_FALSE);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: restore TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, ttiTcamLookupEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore MAC mode for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, ttiTcamMacMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
            GT_TRUE);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
            prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Restore TCAM floor info */
    for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet(); floorIndex++ )
    {
        rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex,
                &saveFloorInfoArr[floorIndex][0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
                floorIndex);
    }

    /* AUTODOC: Restore PCL TCAM group binding */
    rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
            PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);

    /* AUTODOC: Restore TTI TCAM group binding */
    rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
            PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);

    /* Restore TCAM Active number of floors */
    rc = cpssDxChTcamActiveFloorsSet(prvTgfDevNum,
            fineTuningPtr->tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);

    UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "cpssDxChTcamActiveFloorsSet failed for device: %d", prvTgfDevNum);
}

GT_VOID prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModePositive
(
    GT_VOID
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT       ruleSize;
    GT_U32                                     indexRangesBmp[4];
    GT_U32                                     cncBlockNum = 1;
    GT_U32                                     clientGroup = PCL_TCAM_GROUP_CNS;
    PRV_TGF_CNC_CLIENT_ENT                     client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;
    GT_U32                                     counterIdx;
    PRV_TGF_CNC_COUNTER_STC                    counter;
    GT_U32                                     floorIndex;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC    *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning;
    GT_U32                                     cncBlockCntrs;
    GT_U32                                     cncBlocks;
    GT_U32                                     bmpBackup;

    bmpBackup = vtcamMngCfgParam.haFeaturesEnabledBmp;
    vtcamMngCfgParam.haFeaturesEnabledBmp = CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_REPLAY_HIGHER_PRIO_FIRST_E;

    ruleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    /*  Create vTcamManager 0 with vTcam 1(priority) & vTcam 2(logical) */
    prvTgfVirtualTcamTestConfigurationSetHa(clientGroup, ruleSize, 0);

    /*  Preventing Config of second rule, to simulate failed rule write, which will be attempted during config replay */
    /*  Configure 2 PCL rules - 1 each in vtcam1 & vtcam2 */
    ruleWriteHa(1, 1, 0, 5, (0x01020300 | 1), (GT_U16)0x0C06, 1);
    ruleWriteHa(0, 2, 0, 5, (0x01020300 | 2), (GT_U16)0x0C06, 2);
    ruleWriteHa(1, 1, 5, 6, (0x01020300 | 3), (GT_U16)0x0C06, 3);

    /* use first range */
    indexRangesBmp[0] = 0x1;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    /* configure CNC */
    rc = prvTgfCncTestCncBlockConfigure(
            cncBlockNum,
            client,
            GT_TRUE /*enable*/,
            indexRangesBmp,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfCncTestCncBlockConfigure: block %d, client %d",
            cncBlockNum,
            client);

    /* AUTODOC: enable PCL clients for CNC */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS ))
    {
        rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    }

    /* AUTODOC: Clean CNC counters */
    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_FALSE  /*check*/, 1, 5);

    /* Test traffic to hit 2 configured rules */
    haTestTrafficGenerate(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], 1, 4);

    /* AUTODOC: Clean CNC counters */
    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_TRUE  /*check*/, 1, 4);

    /* HA started - INIT State: DB(no h/w) cleared */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 30B60B\n");

    /* HA config replay started */
    prvTgfVirtualTcamTestConfigurationSetHa(clientGroup, ruleSize, 1);

    rc = ruleWriteHa2(1, 1, 0, 5, (0x01020300 | 1), (GT_U16)0x0C06, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=1, prio=5) (config-replay) failed.\n");

    rc = ruleWriteHa2(1, 1, 5, 6, (0x01020300 | 3), (GT_U16)0x0C06, 3);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=1, prio=6) (config-replay) failed.\n");

    rc = ruleWriteHa2(0, 2, 0, 5, (0x01020300 | 2), (GT_U16)0x0C06, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Rule write (vtcam=2, ruleId=0) (config-replay) failed.\n");

    /* HA Catch up started - DB recreated */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    /* HA Complete - DB intact */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed\n");

    /* Test traffic to hit 2 configured rules */
    haTestTrafficGenerate(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], 1, 4);

    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_TRUE  /*check*/, 1, 4);

    /* restore haFeaturesEnabledBmp */
    vtcamMngCfgParam.haFeaturesEnabledBmp = bmpBackup;

    /* After HA(normal oper.)  - configure 1 PCL rules - 1 in vtcam2 */
    ruleWriteHa(0, 2, 2, 0, (0x01020300 | 4), (GT_U16)0x0C06, 4);

    /* Test traffic to hit 4 configured rules */
    haTestTrafficGenerate(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], 1, 5);

    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_TRUE  /*check*/, 1, 5);


    /* AUTODOC: RESTORE CONFIGURATION: */

    cpssOsMemSet(&counter, 0, sizeof(counter));

    /* get CNC size info */
    prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

    /* use first range */
    indexRangesBmp[0] = 0x0;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    {
        /* Get CNC client for a block */
        client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;

        /* disable client */
        rc = prvTgfCncBlockClientEnableSet(
                cncBlockNum, client, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", cncBlockNum, client);

        /* reset ranges */
        rc = prvTgfCncBlockClientRangesSet(
                cncBlockNum, client, indexRangesBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", cncBlockNum, client);
    }
    {
        /* Get CNC client for a block */
        client = PRV_TGF_CNC_CLIENT_TTI_E;

        /* disable client */
        rc = prvTgfCncBlockClientEnableSet(
                cncBlockNum, client, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", cncBlockNum, client);

        /* reset ranges */
        rc = prvTgfCncBlockClientRangesSet(
                cncBlockNum, client, indexRangesBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", cncBlockNum, client);
    }
    /* clean counters */
    for (counterIdx = 0; counterIdx < cncBlockCntrs; counterIdx++)
    {
        rc = prvTgfCncCounterSet(
                cncBlockNum, counterIdx,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncCounterSet: block %d idx %d", cncBlockNum, counterIdx);
    }

    /* disable PCL clients for CNC */
    rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E,GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed\n");

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy for port 1 (the ingress port) */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            GT_FALSE);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: restore TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, ttiTcamLookupEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore MAC mode for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, ttiTcamMacMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
            GT_TRUE);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
            prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Restore TCAM floor info */
    for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet(); floorIndex++ )
    {
        rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex,
                &saveFloorInfoArr[floorIndex][0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
                floorIndex);
    }

    /* AUTODOC: Restore PCL TCAM group binding */
    rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
            PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);

    /* AUTODOC: Restore TTI TCAM group binding */
    rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
            PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);

    /* Restore TCAM Active number of floors */
    rc = cpssDxChTcamActiveFloorsSet(prvTgfDevNum,
            fineTuningPtr->tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);

    UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "cpssDxChTcamActiveFloorsSet failed for device: %d", prvTgfDevNum);
}

GT_VOID prvTgfVirtualTcamTrafficTestHa
(
    GT_VOID
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT       ruleSize;
    GT_U32                                     indexRangesBmp[4];
    GT_U32                                     cncBlockNum = 1;
    GT_U32                                     clientGroup = PCL_TCAM_GROUP_CNS;
    PRV_TGF_CNC_CLIENT_ENT                     client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;
    GT_U32                                     counterIdx;
    PRV_TGF_CNC_COUNTER_STC                    counter;
    GT_U32                                     floorIndex;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC    *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning;
    GT_U32                                     cncBlockCntrs;
    GT_U32                                     cncBlocks;

    ruleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    /*  Create vTcamManager 0 with vTcam 1(priority) & vTcam 2(logical) */
    prvTgfVirtualTcamTestConfigurationSetHa(clientGroup, ruleSize, 0);

    /*  Configure 2 PCL rules - 1 each in vtcam1 & vtcam2 */
    ruleWriteHa(1, 1, 0, 5, (0x01020300 | 1), (GT_U16)0x0C06, 1);

    ruleWriteHa(0, 2, 0, 5, (0x01020300 | 2), (GT_U16)0x0C06, 2);

    /* use first range */
    indexRangesBmp[0] = 0x1;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    /* configure CNC */
    rc = prvTgfCncTestCncBlockConfigure(
            cncBlockNum,
            client,
            GT_TRUE /*enable*/,
            indexRangesBmp,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfCncTestCncBlockConfigure: block %d, client %d",
            cncBlockNum,
            client);

    /* AUTODOC: enable PCL clients for CNC */
    if(GT_FALSE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS ))
    {
        rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    }

    /* AUTODOC: Clean CNC counters */
    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_FALSE  /*check*/, 1, 5);

    /* Test traffic to hit 2 configured rules */
    haTestTrafficGenerate(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], 1, 3);

    /* AUTODOC: Clean CNC counters */
    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_TRUE  /*check*/, 1, 3);

    /* HA started - INIT State: DB(no h/w) cleared */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 1, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA INIT failed for 30B60B\n");

    /* HA config replay started */
    prvTgfVirtualTcamTestConfigurationSetHa(clientGroup, ruleSize, 1);

    ruleWriteHa(1, 1, 0, 5, (0x01020300 | 1), (GT_U16)0x0C06, 1);

    ruleWriteHa(0, 2, 0, 5, (0x01020300 | 2), (GT_U16)0x0C06, 2);

    /* HA Catch up started - DB recreated */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 3, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA H/w CATCHUP failed\n");

    /* HA Complete - DB intact */
    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed\n");

    /* Test traffic to hit 2 configured rules */
    haTestTrafficGenerate(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], 1, 3);

    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_TRUE  /*check*/, 1, 3);

    /* After HA(normal oper.)  - configure 2 PCL rules - 1 each in vtcam1 & vtcam2 */
    ruleWriteHa(1, 1, 5, 6, (0x01020300 | 3), (GT_U16)0x0C06, 3);

    ruleWriteHa(0, 2, 2, 0, (0x01020300 | 4), (GT_U16)0x0C06, 4);

    /* Test traffic to hit 4 configured rules */
    haTestTrafficGenerate(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], 1, 5);

    haTestCncCountersCheckOrReset(
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            cncBlockNum,
            1 /*expectedPackets*/,
            GT_TRUE  /*check*/, 1, 5);


    /* AUTODOC: RESTORE CONFIGURATION: */

    cpssOsMemSet(&counter, 0, sizeof(counter));

    /* get CNC size info */
    prvTgfPclCncSizeGet(&cncBlockCntrs,&cncBlocks);

    /* use first range */
    indexRangesBmp[0] = 0x0;
    indexRangesBmp[1] = 0x0;
    indexRangesBmp[2] = 0x0;
    indexRangesBmp[3] = 0x0;

    {
        /* Get CNC client for a block */
        client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;

        /* disable client */
        rc = prvTgfCncBlockClientEnableSet(
                cncBlockNum, client, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", cncBlockNum, client);

        /* reset ranges */
        rc = prvTgfCncBlockClientRangesSet(
                cncBlockNum, client, indexRangesBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", cncBlockNum, client);
    }
    {
        /* Get CNC client for a block */
        client = PRV_TGF_CNC_CLIENT_TTI_E;

        /* disable client */
        rc = prvTgfCncBlockClientEnableSet(
                cncBlockNum, client, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientEnableSet: block %d client %d", cncBlockNum, client);

        /* reset ranges */
        rc = prvTgfCncBlockClientRangesSet(
                cncBlockNum, client, indexRangesBmp);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncBlockClientRangesSet: block %d client %d", cncBlockNum, client);
    }
    /* clean counters */
    for (counterIdx = 0; counterIdx < cncBlockCntrs; counterIdx++)
    {
        rc = prvTgfCncCounterSet(
                cncBlockNum, counterIdx,
                PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCncCounterSet: block %d idx %d", cncBlockNum, counterIdx);
    }

    /* disable PCL clients for CNC */
    rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");
    rc = prvTgfCncCountingEnableSet(PRV_TGF_CNC_COUNTING_ENABLE_UNIT_TTI_E,GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCountingEnableSet");

    rc = prvCpssDxChVirtualTcamHaSystemRecoverySet(prvTgfDevNum, prvTgfDevNum, 2, 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "HA COMPLETE failed\n");

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy for port 1 (the ingress port) */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
            prvTgfPortsArray[ING_PORT_IDX_CNS],
            GT_FALSE);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: restore TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, ttiTcamLookupEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore MAC mode for TTI_KEY_ETH */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, ttiTcamMacMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet: %d", prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
            GT_TRUE);

    /* AUTODOC: invalidate vlans entry (and reset vlans entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
            prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Restore TCAM floor info */
    for( floorIndex = 0 ; floorIndex < prvTgfTcamFloorsNumGet(); floorIndex++ )
    {
        rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex,
                &saveFloorInfoArr[floorIndex][0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(
                GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
                floorIndex);
    }

    /* AUTODOC: Restore PCL TCAM group binding */
    rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
            PRV_TGF_TCAM_IPCL_0_E, tcamGroup, tcamGroupEnable);

    /* AUTODOC: Restore TTI TCAM group binding */
    rc = prvTgfTcamClientGroupSet(PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfTcamClientGroupSet: %d, %d, %d",
            PRV_TGF_TCAM_TTI_E, ttiTcamGroup, ttiTcamGroupEnable);

    /* Restore TCAM Active number of floors */
    rc = cpssDxChTcamActiveFloorsSet(prvTgfDevNum,
            fineTuningPtr->tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);

    UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "cpssDxChTcamActiveFloorsSet failed for device: %d", prvTgfDevNum);
}

/*========================================AC5 TTI and two PCL TCAMs =======================================*/

static const struct
{
    GT_U32                         vTcamId;
    GT_U32                         clientGroup;
    GT_U32                         cncIndex;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID ruleId;
} prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[3] =
{
    {1, 1/*TTI*/,       1,  0},
    {2, 0/*PCL*/,       2,  0},
    {3, 2/*PCL_TACM1*/, 3,  0}
};

static GT_BOOL                         prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestTtiLookupEnable = GT_FALSE;
static CPSS_DXCH_TTI_MAC_MODE_ENT      prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestTtiMacMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;
CPSS_DXCH_PCL_LOOKUP_TCAM_INDEXES_STC  prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestPclLookupsTcamIndexes = {0, 0, 0, 0};
/**
* @internal prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestConfigurationSet function
* @endinternal
*
* @brief   Virtual Tcam AC5 TTI IPCL0_0 and IPCL1 test configuration set
*
*/
GT_VOID prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                                  rc;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC            vTcamInfo;
    GT_U64                                     indexRangesBmp[2];
    CPSS_DXCH_CNC_COUNTER_STC                  counter;
    GT_U32                                     i;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pclMask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pclPattern;
    CPSS_DXCH_PCL_ACTION_STC                   pclAction;
    CPSS_DXCH_TTI_RULE_UNT                     ttiMask;
    CPSS_DXCH_TTI_RULE_UNT                     ttiPattern;
    CPSS_DXCH_TTI_ACTION_STC                   ttiAction;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;
    CPSS_DXCH_PCL_LOOKUP_TCAM_INDEXES_STC      pclLookupsTcamIndexes;


    /* defaults for early fail */
    prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestTtiLookupEnable = GT_FALSE;
    prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestTtiMacMode = CPSS_DXCH_TTI_MAC_MODE_DA_E;
    prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestPclLookupsTcamIndexes.ipcl0TcamIndex = 0;
    prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestPclLookupsTcamIndexes.ipcl1TcamIndex = 0;
    prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestPclLookupsTcamIndexes.ipcl2TcamIndex = 0;
    prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestPclLookupsTcamIndexes.epclTcamIndex  = 0;

    /* AUTODOC: save TTI lookup key for port 0 */
    rc = cpssDxChTtiPortLookupEnableGet(
        prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], CPSS_DXCH_TTI_KEY_ETH_E,
        &prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestTtiLookupEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, ":cpssDxChTtiPortLookupEnableGet %d", prvTgfDevNum);

    /* AUTODOC: save MAC_MODE_DA for TTI_KEY_ETH */
    rc = cpssDxChTtiMacModeGet(
        prvTgfDevNum, CPSS_DXCH_TTI_KEY_ETH_E, &prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestTtiMacMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChTtiMacModeGet: %d", prvTgfDevNum);

    rc = cpssDxChPclMapLookupsToTcamIndexesGet(prvTgfDevNum, &prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestPclLookupsTcamIndexes);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, ":cpssDxChPclMapLookupsToTcamIndexesGet %d", prvTgfDevNum);

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = cpssDxChTtiPortLookupEnableSet(
        prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], CPSS_DXCH_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: set MAC_MODE_DA for TTI_KEY_ETH */
    rc = cpssDxChTtiMacModeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_ETH_E, CPSS_DXCH_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChTtiMacModeSet: %d", prvTgfDevNum);

    pclLookupsTcamIndexes.ipcl0TcamIndex = 0;
    pclLookupsTcamIndexes.ipcl1TcamIndex = 0;
    pclLookupsTcamIndexes.ipcl2TcamIndex = 1;
    pclLookupsTcamIndexes.epclTcamIndex  = 0;
    rc = cpssDxChPclMapLookupsToTcamIndexesSet(prvTgfDevNum, &pclLookupsTcamIndexes);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, ":cpssDxChPclMapLookupsToTcamIndexesSet %d", prvTgfDevNum);

    /* AUTODOC: Init IPCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[ING_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[ING_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_1_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    rc = cpssDxChCncBlockClientEnableSet(
        prvTgfDevNum, 0 /*blockNum*/, CPSS_DXCH_CNC_CLIENT_TTI_E, GT_TRUE/*enable*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncBlockClientEnableSet");

    rc = cpssDxChCncBlockClientEnableSet(
        prvTgfDevNum, 0 /*blockNum*/, CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E, GT_TRUE/*enable*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncBlockClientEnableSet");

    rc = cpssDxChCncBlockClientEnableSet(
        prvTgfDevNum, 0 /*blockNum*/, CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E, GT_TRUE/*enable*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncBlockClientEnableSet");

    cpssOsMemSet(indexRangesBmp, 0, sizeof(indexRangesBmp));
    indexRangesBmp[0].l[0] = 1;
    rc = cpssDxChCncBlockClientRangesSet(
        prvTgfDevNum, 0 /*blockNum*/, CPSS_DXCH_CNC_CLIENT_TTI_E, indexRangesBmp);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncBlockClientRangesSet");

    rc = cpssDxChCncBlockClientRangesSet(
        prvTgfDevNum, 0 /*blockNum*/, CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E, indexRangesBmp);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncBlockClientRangesSet");

    rc = cpssDxChCncBlockClientRangesSet(
        prvTgfDevNum, 0 /*blockNum*/, CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E, indexRangesBmp);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncBlockClientRangesSet");

    /* clean counters */
    cpssOsMemSet(&counter, 0, sizeof(counter));
    for (i = 0; (i < 3); i++)
    {
        rc = cpssDxChCncCounterSet(
            prvTgfDevNum, 0 /*blockNum*/, prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].cncIndex,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfCncCounterSet: block %d idx %d",
            i, prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].cncIndex);
    }


    /* Create vTCAM manager */
    rc = cpssDxChVirtualTcamManagerCreate(V_TCAM_MANAGER, &vtcamMngCfgParam);
    if (rc == GT_ALREADY_EXIST)
        rc = GT_OK;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerCreate failed\n");

    rc = cpssDxChVirtualTcamManagerDevListAdd(V_TCAM_MANAGER, &prvTgfDevNum, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamManagerDevListAdd failed\n");

    cpssOsMemSet(&ruleAttributes, 0, sizeof(ruleAttributes));
    cpssOsMemSet(&ruleData, 0, sizeof(ruleData));

    ruleData.valid                   = GT_TRUE;

    /* PCL and TTI rules match any packet and increment CNC counter */

    cpssOsMemSet(&pclMask, 0, sizeof(pclMask));
    cpssOsMemSet(&pclPattern, 0, sizeof(pclPattern));
    cpssOsMemSet(&pclAction, 0, sizeof(pclAction));
    pclAction.matchCounter.enableMatchCount   = GT_TRUE;
    pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;

    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    ttiAction.bindToCentralCounter  = GT_TRUE;
    ttiAction.tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* AUTODOC: Create 30 Bytes logical index based vTCAMs */
    cpssOsMemSet(&vTcamInfo, 0, sizeof(vTcamInfo));
    vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    vTcamInfo.autoResize           = GT_FALSE;
    vTcamInfo.guaranteedNumOfRules = 1;
    vTcamInfo.ruleAdditionMethod   = CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;
    vTcamInfo.hitNumber            = 0;

    for (i = 0; (i < 3); i++)
    {
        /* AUTODOC: create vTCAM */
        vTcamInfo.clientGroup = prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].clientGroup;

        rc = cpssDxChVirtualTcamCreate(
            V_TCAM_MANAGER, prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].vTcamId, &vTcamInfo);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "cpssDxChVirtualTcamCreate FAILED for vTCAM = %d",
            prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].vTcamId);

        switch (prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].clientGroup)
        {
            default:
            case 0:
            case 2:
                pclAction.matchCounter.matchCounterIndex = prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].cncIndex;
                ruleData.rule.pcl.actionPtr      = &pclAction;
                ruleData.rule.pcl.maskPtr        = &pclMask;
                ruleData.rule.pcl.patternPtr     = &pclPattern;
                tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
                tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
                break;
            case 1:
                ttiAction.centralCounterIndex = prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].cncIndex;
                ruleData.rule.tti.actionPtr      = &ttiAction;
                ruleData.rule.tti.maskPtr        = &ttiMask;
                ruleData.rule.tti.patternPtr     = &ttiPattern;
                tcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
                tcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_ETH_E;
                break;
        }
        rc = cpssDxChVirtualTcamRuleWrite(
            V_TCAM_MANAGER,
            prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].vTcamId,
            prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].ruleId,
            &ruleAttributes, &tcamRuleType, &ruleData);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "cpssDxChVirtualTcamRuleWrite: vTCAM = %d, RuleID = %d",
            prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].vTcamId,
            prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].ruleId);
    }
}

/**
* @internal prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestConfigurationRestore function
* @endinternal
*
* * @brief   Virtual Tcam AC5 TTI IPCL0_0 and IPCL1 test configuration restore
*
*/
GT_VOID prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestConfigurationRestore
(
   GT_VOID
)
{
    GT_STATUS                                  rc;
    GT_U64                                     indexRangesBmp[2];
    CPSS_DXCH_CNC_COUNTER_STC                  counter;
    GT_U32                                     i;

    /* AUTODOC: save TTI lookup key for port 0 */
    rc = cpssDxChTtiPortLookupEnableSet(
        prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], CPSS_DXCH_TTI_KEY_ETH_E,
        prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestTtiLookupEnable);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "ssDxChTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: save MAC_MODE_DA for TTI_KEY_ETH */
    rc = cpssDxChTtiMacModeSet(
        prvTgfDevNum, CPSS_DXCH_TTI_KEY_ETH_E, prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestTtiMacMode);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChTtiMacModeSet: %d", prvTgfDevNum);

    rc = cpssDxChPclMapLookupsToTcamIndexesSet(prvTgfDevNum, &prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestPclLookupsTcamIndexes);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, ":cpssDxChPclMapLookupsToTcamIndexesSet %d", prvTgfDevNum);

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy for port 1 (the ingress port) */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
        prvTgfPortsArray[ING_PORT_IDX_CNS], GT_FALSE);

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    rc = cpssDxChCncBlockClientEnableSet(
        prvTgfDevNum, 0 /*blockNum*/, CPSS_DXCH_CNC_CLIENT_TTI_E, GT_FALSE/*enable*/);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChCncBlockClientEnableSet");

    rc = cpssDxChCncBlockClientEnableSet(
        prvTgfDevNum, 0 /*blockNum*/, CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E, GT_FALSE/*enable*/);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChCncBlockClientEnableSet");

    rc = cpssDxChCncBlockClientEnableSet(
        prvTgfDevNum, 0 /*blockNum*/, CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E, GT_FALSE/*enable*/);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChCncBlockClientEnableSet");

    cpssOsMemSet(indexRangesBmp, 0, sizeof(indexRangesBmp));
    rc = cpssDxChCncBlockClientRangesSet(
        prvTgfDevNum, 0 /*blockNum*/, CPSS_DXCH_CNC_CLIENT_TTI_E, indexRangesBmp);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChCncBlockClientRangesSet");

    rc = cpssDxChCncBlockClientRangesSet(
        prvTgfDevNum, 0 /*blockNum*/, CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E, indexRangesBmp);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChCncBlockClientRangesSet");

    rc = cpssDxChCncBlockClientRangesSet(
        prvTgfDevNum, 0 /*blockNum*/, CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E, indexRangesBmp);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChCncBlockClientRangesSet");

    /* clean counters */
    cpssOsMemSet(&counter, 0, sizeof(counter));
    for (i = 0; (i < 3); i++)
    {
        rc = cpssDxChCncCounterSet(
            prvTgfDevNum, 0 /*blockNum*/, prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].cncIndex,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(
            GT_OK, rc, "prvTgfCncCounterSet: block %d idx %d",
            i, prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].cncIndex);
    }

    for (i = 0; (i < 3); i++)
    {
        rc = cpssDxChVirtualTcamRuleDelete(
            V_TCAM_MANAGER,
            prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].vTcamId,
            prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].ruleId);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(
            GT_OK, rc, "cpssDxChVirtualTcamRuleDelete: vTCAM = %d, RuleID = %d",
            prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].vTcamId,
            prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].ruleId);

        rc = cpssDxChVirtualTcamRemove(V_TCAM_MANAGER, prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].vTcamId);
        UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(
            GT_OK, rc, "cpssDxChVirtualTcamRemove FAILED for vTCAM = %d",
            prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].vTcamId);
    }

    rc = cpssDxChVirtualTcamManagerDevListRemove(V_TCAM_MANAGER, &prvTgfDevNum, 1);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerDevListRemove failed for vTcamMngId = %d\n",
        V_TCAM_MANAGER);

    rc = cpssDxChVirtualTcamManagerDelete(V_TCAM_MANAGER);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(
        GT_OK, rc, "cpssDxChVirtualTcamManagerDelete failed for vTcamMngId = %d\n",
        V_TCAM_MANAGER);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);
}

/**
* @internal prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestTraffic function
* @endinternal
*
* @brief    Virtual Tcam AC5 TTI IPCL0_0 and IPCL1 test traffic.
*
*/
GT_VOID prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestTraffic
(
    GT_VOID
)
{
    GT_STATUS                        rc;
    GT_U32                           i;
    CPSS_DXCH_CNC_COUNTER_STC        counter;

    /* DATA of bypass packet */
    static GT_U8 payloadDataArr[48] ={0x33, 0x33, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    /* Bypass PAYLOAD part */
    static TGF_PACKET_PAYLOAD_STC payloadPart = {
        sizeof(payloadDataArr),                       /* dataLength */
        payloadDataArr                                /* dataPtr */
    };
    /*  L2 part of packet  */
    static TGF_PACKET_L2_STC packetL2Part = {
        {0x0C, 0x06, 0x01, 0x02, 0x03, 0x00},                /* daMac */
        {0x00, 0x11, 0x22, 0x44, 0x55, 0x66}                 /* saMac */
    };
    /* PARTS of packet */
    static TGF_PACKET_PART_STC packetPartArray[] = {
        {TGF_PACKET_PART_L2_E,        &packetL2Part},  /* type, partPtr */
        {TGF_PACKET_PART_PAYLOAD_E,   &payloadPart}
    };
    /*  PACKET to send info */
    static TGF_PACKET_STC packetInfo = {
    /* LENGTH of packet with CRC */
        (TGF_L2_HEADER_SIZE_CNS + sizeof(payloadDataArr) + TGF_CRC_LEN_CNS),                                          /* totalLen */
        sizeof(packetPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
        packetPartArray                                        /* partsArray */
    };

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth error");

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth error");

    PRV_TGF_VIRTUAL_TCAM_TEST_SLEEP_MAC(0);

    /* check CNC counters */
    for (i = 0; (i < 3); i++)
    {
        cpssOsMemSet(&counter, 0, sizeof(counter));
        rc = cpssDxChCncCounterGet(
            prvTgfDevNum, 0 /*blockNum*/, prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].cncIndex,
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(
            GT_OK, rc, "prvTgfCncCounterGet: block %d idx %d",
            i, prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestvTcamsArr[i].cncIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            1, counter.packetCount.l[0], "Wrong CNC counter value");
    }
}

/***********************************************************************************************/
/* Using LPM memory as HW DB for VTCAM manager */

/**
* @internal prvTgfVirtualTcamHaHwDbLpmAccessBind function
* @endinternal
*
* @brief   Bind LPM memort HW DB access functions to VTCAM Manager.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] vTcamMngId         - VTCAM Manager Id.
*
* @retval GT_OK                    - on success, others on fail.
*/
GT_STATUS prvTgfVirtualTcamHaHwDbLpmAccessBind
(
    IN GT_U32    vTcamMngId
)
{
    return cpssDxChVirtualTcamHaHwDbAccessBind(
        vTcamMngId,
        &prvCpssDxChVirtualTcamHaHwDbLpmWrite,
        &prvCpssDxChVirtualTcamHaHwDbLpmRead);
}

/**
* @internal prvTgfVirtualTcamHaHwDbAccessUnbind function
* @endinternal
*
* @brief   Unbind any HW DB access functions from VTCAM Manager.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] vTcamMngId         - VTCAM Manager Id.
*
* @retval GT_OK                    - on success, others on fail.
*/
GT_STATUS prvTgfVirtualTcamHaHwDbAccessUnbind
(
    IN GT_U32    vTcamMngId
)
{
    return cpssDxChVirtualTcamHaHwDbAccessBind(
        vTcamMngId, NULL, NULL);
}


