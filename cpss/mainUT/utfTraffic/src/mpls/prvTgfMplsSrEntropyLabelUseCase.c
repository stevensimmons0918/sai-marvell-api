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
* @file prvTgfMplsSrEntropyLabelUseCase
*
* @brief MPLS Functionality with Entropy Label
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfMplsGen.h>
#include <mpls/prvTgfMplsSrEntropyLabelUseCase.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <common/tgfTrunkGen.h>

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
        CPSS_INTERFACE_INFO_STC             physicalInfo[4];
        PRV_TGF_BRG_EGRESS_PORT_INFO_STC    egressInfo[4];
        PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT   globalHashModeGet;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* Phy Port & Eport Numbers */
#define PRV_TGF_EPORT1_CNS                      1111
#define PRV_TGF_EPORT2_CNS                      1112
#define PRV_TGF_EPORT3_CNS                      1113
#define PRV_TGF_EPORT4_CNS                      1114

#define PRV_TGF_RCV_PORT0_IDX_CNS               0
#define PRV_TGF_RCV_PORT1_IDX_CNS               1
#define PRV_TGF_RCV_PORT2_IDX_CNS               2
#define PRV_TGF_RCV_PORT3_IDX_CNS               3
#define PRV_TGF_SEND_PORT_IDX_CNS               4

#define PRV_TGF_PHA_THREAD_ID_MPLS_SR_NO_EL     15
#define PRV_TGF_PHA_THREAD_ID_MPLS_SR_ONE_EL    16
#define PRV_TGF_PHA_THREAD_ID_MPLS_SR_TWO_EL    17
#define PRV_TGF_PHA_THREAD_ID_MPLS_SR_THREE_EL  18

#define PRV_TGF_MAX_PORT_INDEX_CNS              4
#define PRV_TGF_BURST_COUNT_CNS                 4
#define PRV_TGF_VLANID_1_CNS                    1
#define PRV_TGF_MPLS_ETHER_TYPE_CNS             0X8847

#define PRV_TGF_EL_SIZE                         8
#define PRV_TGF_LABEL_SIZE                      4
#define PRV_TGF_EXP_DATA_SIZE                   52

/* Array of ePorts */
static GT_U32 prvTgfePortsArray[PRV_TGF_MAX_PORT_INDEX_CNS] = {
    PRV_TGF_EPORT1_CNS,
    PRV_TGF_EPORT2_CNS,
    PRV_TGF_EPORT3_CNS,
    PRV_TGF_EPORT4_CNS
};

/* Array of PHA Thread Numbers */
static GT_U32 prvTgfPhaArray[PRV_TGF_MAX_PORT_INDEX_CNS] = {
    PRV_TGF_PHA_THREAD_ID_MPLS_SR_NO_EL,
    PRV_TGF_PHA_THREAD_ID_MPLS_SR_ONE_EL,
    PRV_TGF_PHA_THREAD_ID_MPLS_SR_TWO_EL,
    PRV_TGF_PHA_THREAD_ID_MPLS_SR_THREE_EL
};

/* Array of PHA Thread types */
static CPSS_DXCH_PHA_THREAD_TYPE_ENT prvTgfPhaThreadTypeArr[] = {
    CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_NO_EL_E,
    CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_ONE_EL_E,
    CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_TWO_EL_E,
    CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_THREE_EL_E
};

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

#define IS_SUPPORT_CRC_RETAIN                                                \
    /* sip6 not support 'retain inner CRC' */                                \
    ((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(prvTgfDevNum,                  \
     PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E) ||   \
        PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)) ? 0 : 1)

static GT_U32   calculatedHash[4];

/* Tunnel start Entry : Data & MPLS SR Labels */
static GT_U8 prvTgfTunnelStartArr[4][36] = {
    {
        0x0,  0x0,  0x0,  0x0,   /*EL3 Offset, EL2 Offset, EL1 Offset, new_L3 offset  - Will be filled dynamically*/
        0x10, 0x0,  0x0,  0x1,
        0x10, 0x0,  0x0,  0x2,
        0x10, 0x0,  0x0,  0x3,
        0x10, 0x0,  0x0,  0x4,
        0x10, 0x0,  0x0,  0x5,
        0x10, 0x0,  0x0,  0x6,
        0x10, 0x0,  0x0,  0x7,
        0x10, 0x0,  0x0,  0x8
    },
    {
        0x0,  0x0,  0x0,  0x0,
        0x20, 0x0,  0x0,  0x1,
        0x20, 0x0,  0x0,  0x2,
        0x20, 0x0,  0x0,  0x3,
        0x20, 0x0,  0x0,  0x4,
        0x20, 0x0,  0x0,  0x5,
        0x20, 0x0,  0x0,  0x6,
        0x20, 0x0,  0x0,  0x7,
        0x20, 0x0,  0x0,  0x8
    },
    {
        0x0,  0x0,  0x0,  0x0,
        0x30, 0x0,  0x0,  0x1,
        0x30, 0x0,  0x0,  0x2,
        0x30, 0x0,  0x0,  0x3,
        0x30, 0x0,  0x0,  0x4,
        0x30, 0x0,  0x0,  0x5,
        0x30, 0x0,  0x0,  0x6,
        0x30, 0x0,  0x0,  0x7,
        0x30, 0x0,  0x0,  0x8
    },
    {
        0x0,  0x0,  0x0,  0x0,
        0x40, 0x0,  0x0,  0x1,
        0x40, 0x0,  0x0,  0x2,
        0x40, 0x0,  0x0,  0x3,
        0x40, 0x0,  0x0,  0x4,
        0x40, 0x0,  0x0,  0x5,
        0x40, 0x0,  0x0,  0x6,
        0x40, 0x0,  0x0,  0x7,
        0x40, 0x0,  0x0,  0x8
    }
};

/* Expected MPLS SR Labels in the egress packet(PHA processed)
 * Each entry Last Label is set with BoS(Bottom of stack) set
 *     Note: In 4th Entry BoS is not set,
 *           as EL is at the last not the SR label, so EL will have BoS set
 */
static GT_U32 prvTgfTunnelStartLabelGetArr[4][8] = {
    {
        0x10000001, 0x10000002, 0x10000003, 0x10000004,
        0x10000005, 0x10000006, 0x10000007, 0x10000108
    },

    {
        0x20000001, 0x20000002, 0x20000003, 0x20000004,
        0x20000005, 0x20000006, 0x20000007, 0x20000108
    },
    {
        0x30000001, 0x30000002, 0x30000003, 0x30000004,
        0x30000005, 0x30000006, 0x30000007, 0x30000108
    },
    {
        0x40000001, 0x40000002, 0x40000003, 0x40000004,
        0x40000005, 0x40000006, 0x40000007, 0x40000008
    },
};

/* Array of Macs used for different packet */
static TGF_MAC_ADDR prvTgfDaMacArr[] = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x01},
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x03},
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x04}
};

/************************ common parts of packets **************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x01},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x11}                 /* saMac */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29
};

/* packet's IPv4 over IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2b,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,      /* csum */ /*0x5EA0*/
    {1,  1,  1,  1},   /* srcAddr */
    {2,  2,  2,  2}    /* dstAddr */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    3, 0, PRV_TGF_VLANID_1_CNS                          /* pri, cfi, VlanId */
};

/* Packet ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = { TGF_ETHERTYPE_0800_IPV4_TAG_CNS };

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),  /* dataLength */
    prvTgfPayloadDataArr           /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of packet */
#define PRV_TGF_PACKET_LEN_CNS    \
    TGF_L2_HEADER_SIZE_CNS      + \
    TGF_VLAN_TAG_SIZE_CNS       + \
    TGF_ETHERTYPE_SIZE_CNS      + \
    TGF_IPV4_HEADER_SIZE_CNS    + \
    sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                  /* totalLen */
    (sizeof(prvTgfPacketPartArray)
     / sizeof(prvTgfPacketPartArray[0])),    /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

/**
* @enum PRV_TGF_EL_OFFSET_INFO_STC
*
* @brief This enum defines EL label offset in number of labels(Not Bytes)
*/
typedef struct {
    /** @brief This indicates after how many MPLS labels EL1 need to be inserted
     *  (APPLICABLE RANGES: 0-8, 0 Means ELI1 will be at the beginning of the MPLS Labels)
     **/
    GT_U8    EL1_Offset;

    /** @brief This indicates after how many MPLS labels EL2 need to be inserted
     *  (APPLICABLE RANGES: 0-8, 0 Means ELI2 will be at the beginning of the MPLS Labels)
     **/
    GT_U8    EL2_Offset;

    /** @brief This indicates after how many MPLS labels EL3 need to be inserted
     *  (APPLICABLE RANGES: 0-8, 0 Means ELI3 will be at the beginning of the MPLS Labels)
     **/
    GT_U8    EL3_Offset;

    /** @brief This indicates, how many base MPLS labels need to be removed from beginning of the MPLS stack
     *  (APPLICABLE RANGES: 0-8, 0 Means no MPLS labels will be removed)
     **/
    GT_U8    new_L3_Offset;
}PRV_TGF_EL_OFFSET_INFO_STC;


static PRV_TGF_EL_OFFSET_INFO_STC ELOffset[4] = { /* EL1 , EL2, EL3, L3_offset */
                                            /* NO EL Labels, Remove 3 Base MPLS Labels from Beginning */
                                            {0, 0, 0, 3},

                                            /* EL1 Label after 5 MPLS base Label,
                                             * Remove 2 Base MPLS Labels from Beginning
                                             */
                                            {0, 0, 5, 2},

                                            /*
                                             * EL1 Label after 7 MPLS base Label,
                                             * EL2 Label after 4 MPLS base Label,
                                             * Remove 1 Base MPLS Labels */
                                            {0, 4, 7, 1},

                                            /*
                                             * EL1 Label after 8 MPLS base Label,
                                             * EL2 Label after 6 MPLS base Label,
                                             * EL3 Label after 3 MPLS base Label,
                                             * No MPLS Labels will be removed
                                             */
                                            {3, 6, 8, 0}};
/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* internal prvTgfElOffsetFrame function
* @endinternal
*
* @brief   Construct the actual offset values for the TS, from the input EL positions
*/
GT_VOID prvTgfElOffsetFrame
(
    GT_U32  index,
    GT_U8   offsetData[]
)
{
    if(ELOffset[index].EL1_Offset)
    {
        offsetData[0] = (PRV_TGF_EXP_DATA_SIZE - (3 * PRV_TGF_EL_SIZE)) + (ELOffset[index].EL1_Offset * PRV_TGF_LABEL_SIZE);
    }
    if(ELOffset[index].EL2_Offset)
    {
        offsetData[1] = (PRV_TGF_EXP_DATA_SIZE - (2 * PRV_TGF_EL_SIZE)) + (ELOffset[index].EL2_Offset * PRV_TGF_LABEL_SIZE);
    }
    if(ELOffset[index].EL3_Offset)
    {
        offsetData[2] = (PRV_TGF_EXP_DATA_SIZE - (1 * PRV_TGF_EL_SIZE)) + (ELOffset[index].EL3_Offset * PRV_TGF_LABEL_SIZE);
    }
    offsetData[3] = (PRV_TGF_EXP_DATA_SIZE - (index * PRV_TGF_EL_SIZE)) + (ELOffset[index].new_L3_Offset * PRV_TGF_LABEL_SIZE);
}
/**
* internal prvTgfTunnelConfigSet function
* @endinternal
*
* @brief   Tunnel configurations for MPLS Entropy label use case
*/
static GT_VOID prvTgfTunnelConfigSet
(
    GT_U32  index
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_TUNNEL_START_ENTRY_UNT          tunnelEntry;
    PRV_TGF_TUNNEL_START_ENTRY_UNT          tunnelEntryGet;
    CPSS_TUNNEL_TYPE_ENT                    tunnelTypeGet;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC        egressInfo;
    GT_U8                                   offsetData[4];

    PRV_UTF_LOG0_MAC("======= Setting Tunnel Configuration =======\n");


    cpssOsMemSet(&offsetData,0,sizeof(offsetData));
    /* save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[index],
                                                       &(prvTgfRestoreCfg.egressInfo[index]));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    /* Set ePort attributes configuration */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = index * 8;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort#2 attributes: */
    /* AUTODOC:   Tunnel Start=ENABLE, tunnelStartPtr = index * 8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       prvTgfPortsArray[index],
                                                       &egressInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* Calculate the offset value from input */
    prvTgfElOffsetFrame(index, offsetData);
    cpssOsMemCpy(prvTgfTunnelStartArr[index], offsetData, sizeof(offsetData));

    /*** Set a tunnel start entry ***/
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    cpssOsMemCpy(tunnelEntry.genCfg.macDa.arEther, prvTgfDaMacArr[index], sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(tunnelEntry.genCfg.data, prvTgfTunnelStartArr[index], sizeof(prvTgfTunnelStartArr[index]));
    tunnelEntry.genCfg.genericType              = PRV_TGF_TUNNEL_START_GENERIC_LONG_TYPE_E;
    tunnelEntry.genCfg.etherType                = TGF_ETHERTYPE_8847_MPLS_TAG_CNS;
    tunnelEntry.genCfg.upMarkMode               = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.genCfg.tagEnable                = GT_TRUE;
    switch(index)
    {
        case 0:
            tunnelEntry.genCfg.up               = 1;
            tunnelEntry.genCfg.vlanId           = 2;
            tunnelEntry.genCfg.retainCrc        = GT_FALSE;
            break;

        case 1:
            tunnelEntry.genCfg.up               = 0;
            tunnelEntry.genCfg.tagEnable        = GT_FALSE;
            tunnelEntry.genCfg.vlanId           = 1;
            tunnelEntry.genCfg.retainCrc        = GT_FALSE;
            break;

        case 2:
            tunnelEntry.genCfg.up               = 2;
            tunnelEntry.genCfg.vlanId           = 9;
            tunnelEntry.genCfg.retainCrc        = (!IS_SUPPORT_CRC_RETAIN)?GT_FALSE:GT_TRUE;
            break;

        case 3:
            tunnelEntry.genCfg.upMarkMode       = PRV_TGF_TUNNEL_START_MARK_FROM_PACKET_QOS_PROFILE_E;
            tunnelEntry.genCfg.up               = 0;
            tunnelEntry.genCfg.vlanId           = 31;
            tunnelEntry.genCfg.retainCrc        = (!IS_SUPPORT_CRC_RETAIN)?GT_FALSE:GT_TRUE;
            break;
        default:
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "index out of range: %d", prvTgfDevNum);
    }

    rc = prvTgfTunnelStartEntrySet(index * 8, CPSS_TUNNEL_GENERIC_E, &tunnelEntry);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d, for index[%d]", prvTgfDevNum, index);

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, index * 8, &tunnelTypeGet, &tunnelEntryGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d, for index[%d]", prvTgfDevNum, index);

    /* Check vlanId field */
    UTF_VERIFY_EQUAL1_STRING_MAC(tunnelEntryGet.genCfg.vlanId, tunnelEntry.genCfg.vlanId,
            "Vlan Id mismatch with configured value for device %d", prvTgfDevNum);

    /* Check macDa field */
    rc = cpssOsMemCmp(tunnelEntryGet.genCfg.macDa.arEther, prvTgfDaMacArr[index], sizeof(TGF_MAC_ADDR)) == 0 ?
        GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tunnelEntry.genCfg.macDa.arEther[5]: 0x%2X, for index[%d]",
            tunnelEntryGet.genCfg.macDa.arEther[5], index);

}

/* debug flag to disable using of PHA for the test */
static GT_BOOL prvTgfMplsSrSkipPha = GT_FALSE;
GT_BOOL prvTgfMplsSrSkipPhaSet(GT_BOOL newSkipPha)
{
    GT_BOOL oldSkipPha = prvTgfMplsSrSkipPha;

    prvTgfMplsSrSkipPha = newSkipPha;

    return oldSkipPha;
}

/**
* internal prvTgfPhaConfigSet function
* @endinternal
*
* @brief   PHa Configurations for MPLS Entropy Label use case
*/
static GT_VOID prvTgfPhaConfigSet
(
    GT_U32  index
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC    commonInfo;
    CPSS_DXCH_PHA_THREAD_INFO_UNT           extInfo;

    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    extInfo.notNeeded = 0;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    /* GM does not support PHA */
    if(GT_TRUE == prvUtfIsGmCompilation() || (prvTgfMplsSrSkipPha == GT_TRUE))
        return;

    /* AUTODOC: Assign thread to the ePort */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum,
                                    prvTgfPortsArray[index],
                                    GT_TRUE,
                                    prvTgfPhaArray[index]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for PHA_THREAD_ID = %d, rc = [%d]",
                    prvTgfPhaArray[index], rc);

    /* AUTODOC: Set the thread entry */
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     prvTgfPhaArray[index],
                                     &commonInfo,
                                     prvTgfPhaThreadTypeArr[index],
                                     &extInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "[TGF]: cpssDxChPhaThreadIdEntrySet FAILED for PHA_THREAD_ID = %d, rc = [%d]",
                    prvTgfPhaArray[index], rc);
}

/**
* @internal prvTgfMplsSrEntropyLabelConfigurationSet function
* @endinternal
*
* @brief   Set Entropy Label Configuration for MPLS entropy lable use case
*/
GT_VOID prvTgfMplsSrEntropyLabelConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC                 physicalInfo;
    GT_U32                                  index;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;

    PRV_UTF_LOG0_MAC("======= Setting Entropy Label Configuration =======\n");

    /* MAC Entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLANID_1_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daRoute                        = GT_TRUE;

    /* Physical port Entry */
    cpssOsMemSet(&physicalInfo, 0, sizeof(CPSS_INTERFACE_INFO_STC));
    physicalInfo.hwDevNum                   = prvTgfDevNum;
    physicalInfo.vlanId                     = PRV_TGF_VLANID_1_CNS;
    physicalInfo.devPort.hwDevNum           = prvTgfDevNum;
    physicalInfo.type                       = CPSS_INTERFACE_PORT_E;


    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* Get the general hashing mode of trunk hash generation */
        rc =  prvTgfTrunkHashGlobalModeGet(prvTgfDevNum,&prvTgfRestoreCfg.globalHashModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeGet: %d", prvTgfDevNum);

        /* Set the general hashing mode of trunk hash generation based on packet data */
        rc =  prvTgfTrunkHashGlobalModeSet(CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeSet: %d", prvTgfDevNum);
    }


    /* AUTODOC: Configure eToPhy table and FDB Entry */
    for(index = 0; index < PRV_TGF_MAX_PORT_INDEX_CNS; index++)
    {
        /* save ePort mapping configuration */
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                        prvTgfePortsArray[index],
                                                        &(prvTgfRestoreCfg.physicalInfo[index]));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableGet);

        physicalInfo.devPort.portNum = prvTgfPortsArray[index];
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                               prvTgfePortsArray[index],
                                                               &physicalInfo);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

        /* create macEntry */
        cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfDaMacArr[index], sizeof(TGF_MAC_ADDR));
        macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[index];

        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d, for index", prvTgfDevNum, index);

        /* Configure the PHA entry */
        prvTgfPhaConfigSet(index);

        /* Configure tunnel start entry */
        prvTgfTunnelConfigSet(index);
    }
}

/**
* internal prvTgfMplsSrEntropyLabelVerification function
* @endinternal
*
* @brief   MPLS entropy label use case verification
*/
GT_VOID prvTgfMplsSrEntropyLabelVerification
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
static    GT_U8                         packetBuf[PRV_TGF_BURST_COUNT_CNS][TGF_RX_BUFFER_MAX_SIZE_CNS] = {{0}, {0}, {0}, {0}};
    GT_U32                              packetLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0;
    TGF_NET_DSA_STC                     rxParam;
    GT_U8                               index = 0;
    GT_BOOL                             getFirst = GT_TRUE;
    CPSS_INTERFACE_INFO_STC             portInterface;
    GT_U32                              pktOffset = 0;
    GT_U32                              mplsEtherType = 0;
    GT_U32                              startLabel = 0;
    GT_U32                              labelData = 0;
    GT_U32                              numberOfEL = 0;
    GT_U32                              i = 0;
    GT_U32                              previousLabelTti = 0;
    GT_U32                              pktHash = 0;

    PRV_UTF_LOG0_MAC("======= Packet Verification =======\n");

    /* setup portInterface for capturing */
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;

    for (index = 0; index<PRV_TGF_BURST_COUNT_CNS; index++)
    {
        numberOfEL = 0;
        portInterface.devPort.portNum   = prvTgfPortsArray[index];
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInterface,
                                                    TGF_PACKET_TYPE_CAPTURE_E,
                                                    getFirst, GT_TRUE, packetBuf[index],
                                                    &packetLen, &packetActualLength,
                                                    &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet, for index[%d]", index);


        /* index 1 is for default vlan verification */
        pktOffset = (index == 1)?TGF_L2_HEADER_SIZE_CNS:(TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS);

        /* AUTODOC: Verify MPLS Ether Type */
        mplsEtherType = (GT_U32)(packetBuf[index][pktOffset + 1] | (packetBuf[index][pktOffset] << 8));
        UTF_VERIFY_EQUAL1_STRING_MAC(mplsEtherType,
                                     PRV_TGF_MPLS_ETHER_TYPE_CNS,
                                     "MPLS Ethertype is not as expected, for index[%d]", index);
        pktOffset +=2;

        /* Caluclate MPLS base label from L3 index
         * L3 index - 48 = position to start, if no EL
         * (index * 8)   = number of EL (Each EL is 8 Byte)
         *     -4        = Remove the Data byte from calculation */
        startLabel = ((prvTgfTunnelStartArr[index][3] - 48) + (index * 8) - 4)/4;

        /* AUTODOC: Verify the MPLS label Data */
        for(i=startLabel; i < 8; i++)
        {
            labelData = (GT_U32)(packetBuf[index][pktOffset + 3] | (packetBuf[index][pktOffset + 2] << 8) |
                    (packetBuf[index][pktOffset + 1] << 16) | (packetBuf[index][pktOffset] << 24));

            /* Entropy Label verification
             * ELI - Last 8 bits = previous MPLS Labels TTI value
             */
            if(((labelData & 0xFFFFF000) >> 12) == 7)
            {
                /* ELI Verification */
                UTF_VERIFY_EQUAL1_STRING_MAC(previousLabelTti,
                                     (labelData & 0xFF),
                                     "MPLS Entropy Label TTI value is not as expected, for index [%d]", index);
                pktOffset += 4;

                /* EL Verification - Bit19 is set & Hash Value*/
                UTF_VERIFY_EQUAL1_STRING_MAC(1,
                                        ((packetBuf[index][pktOffset] & 0x80) >> 7),
                                        "MPLS Entropy Label bit19 in not set, for index [%d]", index);

                pktHash = ((GT_U32)(packetBuf[index][pktOffset + 2] |
                                  (packetBuf[index][pktOffset + 1] << 8) |
                                  (packetBuf[index][pktOffset] << 16)) & 0x7FFFF) >> 4;
                UTF_VERIFY_EQUAL1_STRING_MAC(calculatedHash[index], pktHash,
                                            "MPLS Entropy Label hash value not macthing with calculated, for index [%d]", index);
                pktOffset += 4;

                numberOfEL += 1;
                i -= 1;
                continue;
            }
            UTF_VERIFY_EQUAL1_STRING_MAC(labelData,
                                     prvTgfTunnelStartLabelGetArr[index][i],
                                     "MPLS SR Labels is not as expected, for index [%d]", index);

            /* Capture the tti Value of current Label, will be used if the next label is ELI */
            previousLabelTti = packetBuf[index][pktOffset + 3];
            pktOffset += 4;
        }

        /* Last EL is at the end */
        if (index == 3)
        {
            labelData = (GT_U32)(packetBuf[index][pktOffset + 3] | (packetBuf[index][pktOffset + 2] << 8) |
                    (packetBuf[index][pktOffset + 1] << 16) | (packetBuf[index][pktOffset] << 24));
            if(((labelData & 0xFFFFF000) >> 12) == 7)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(previousLabelTti, (labelData & 0xFF),
                        "MPLS Entropy Label TTI value is not as expected, for index [%d]", index);
                numberOfEL += 1;
            }
        }

        UTF_VERIFY_EQUAL1_STRING_MAC(numberOfEL, index,
                           "MPLS Number of EL is not as expected, for index [%d]", index);
        getFirst = GT_FALSE;
    }
}


/**
* @internal prvTgfMplsSrEntropyLabelTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfMplsSrEntropyLabelTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          index = 0;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* AUTODOC: clear counters and capturing RxPcktTable */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* reset ethernet counters */
    rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth");

    /* AUTODOC: setup portInterface for capturing */
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;

    /* AUTODOC: enable capture on receiving ports index[0-3] */
    for(index = 0; index < PRV_TGF_MAX_PORT_INDEX_CNS; index++)
    {
        portInterface.devPort.portNum   = prvTgfPortsArray[index];
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                prvTgfDevNum, prvTgfPortsArray[index]);
    }

    /* AUTODOC: Transmit packets */
    for(index = 0; index < PRV_TGF_MAX_PORT_INDEX_CNS; index++)
    {
        /* setup packet */
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[index], sizeof(TGF_MAC_ADDR));
        calculatedHash[index] = prvTgfPacketL2Part.daMac[5] ^ prvTgfPacketL2Part.saMac[5];

        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, 1, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }

    /* AUTODOC: disable capture on receiving ports index[0-3] */
    for(index = 0; index < PRV_TGF_MAX_PORT_INDEX_CNS; index++)
    {
        portInterface.devPort.portNum   = prvTgfPortsArray[index];
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                prvTgfDevNum, prvTgfPortsArray[index]);
    }

    prvTgfMplsSrEntropyLabelVerification();
}

/**
* @internal prvTgfMplsSrEntropyLabelConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfMplsSrEntropyLabelConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32     index = 0;

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Restore Entropy Label specific Configuration
     */

    /* AUTODOC: Restore thread to the ePort and egressinfo */
    for(index = 0; index < PRV_TGF_MAX_PORT_INDEX_CNS; index++)
    {
        rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum,
                                        prvTgfPortsArray[index],
                                        GT_FALSE,
                                        prvTgfPhaArray[index]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for PHA_THREAD_ID = %d, rc = [%d]",
                prvTgfPhaArray[index], rc);

        /* Restore mapping */
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                               prvTgfePortsArray[index],
                                                               &(prvTgfRestoreCfg.physicalInfo[index]));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

        /* Restore egress info */
        rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                           prvTgfPortsArray[index],
                                                           &(prvTgfRestoreCfg.egressInfo[index]));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: restore to enhanced crc hash data */
        /* restore global hash mode */
        rc =  prvTgfTrunkHashGlobalModeSet(prvTgfRestoreCfg.globalHashModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeSet: %d", prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 2. Restore Base Configuration
     */

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfBrgFdbFlush");
}
