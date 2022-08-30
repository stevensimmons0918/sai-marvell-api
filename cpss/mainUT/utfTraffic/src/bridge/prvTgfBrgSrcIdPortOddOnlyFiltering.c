/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfBrgSrcIdPortOddOnlyFiltering.c
*
* DESCRIPTION:
*       per egress ePort Source ID filtering for odd Source-ID values.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <extUtils/trafficEngine/private/prvTgfTrafficParser.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCscdGen.h>
#include <bridge/prvTgfBrgSrcIdPortOddOnlyFiltering.h>

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSrcId.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS        6

/* target port */
#define EGR_LEAF_PORT_IDX_CNS    3
#define EGR_ROOT_PORT_IDX_CNS    2
#define TX_PORT_IDX_CNS     1
#define EGR_FDB_PORT_IDX_CNS     0

/* default FDB entry macAddr */
static TGF_MAC_ADDR prvTgfFdbEntryMacAddr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x08};
static TGF_MAC_ADDR prvTgfMcMacAddr = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

/* src-Id */
#define EVEN_SRC_ID_CNS  0
#define ODD_SRC_ID_CNS   1

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/******************************* Test packets *********************************/

static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x08}                 /* saMac */
};

/* VLAN_TAG0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, 0                            /* pri, cfi, VlanId */
};

/* DATA of bypass packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* Bypass PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + 2 * TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/******************************************************************************\
 *                   Private variables & definitions                          *
\******************************************************************************/

/* Parameters needed to be restored */
static GT_BOOL                           origLeafFilterEnable;      /* original filter mode for "Leaf" port */
static GT_BOOL                           origRootFilterEnable;      /* original filter mode for "Root" port */
static GT_BOOL                           origFdbRootFilterEnable;   /* original filter mode for port from which UC packet should egress */
static GT_U32                            origSrcId;                 /* original source-id */

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfBrgSrcIdPortOddOnlyFilteringConfigurationSet function
* @endinternal
*
* @brief Function save and set test configurations
*
*/
GT_VOID prvTgfBrgSrcIdPortOddOnlyFilteringConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK; /* return code */

    /* AUTODOC: create VLAN 6 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgDefVlanEntryWrite");

    /* AUTODOC: save configurations */
    rc = cpssDxChBrgSrcIdPortOddOnlyFilterEnableGet(prvTgfDevNum, prvTgfPortsArray[EGR_ROOT_PORT_IDX_CNS],
                                                    &origRootFilterEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdPortOddOnlyFilterEnableGet: portNum = %d",
                                 prvTgfPortsArray[EGR_ROOT_PORT_IDX_CNS]);

    rc = cpssDxChBrgSrcIdPortOddOnlyFilterEnableGet(prvTgfDevNum, prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS],
                                                    &origFdbRootFilterEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdPortOddOnlyFilterEnableGet: portNum = %d",
                                 prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS]);

    rc = cpssDxChBrgSrcIdPortOddOnlyFilterEnableGet(prvTgfDevNum, prvTgfPortsArray[EGR_LEAF_PORT_IDX_CNS],
                                                    &origLeafFilterEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdPortOddOnlyFilterEnableGet: portNum = %d",
                                 prvTgfPortsArray[EGR_LEAF_PORT_IDX_CNS]);

    rc = prvTgfBrgSrcIdPortDefaultSrcIdGet(prvTgfDevNum,prvTgfPortsArray[TX_PORT_IDX_CNS], &origSrcId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortDefaultSrcIdGet: portNum = %d",
                                 prvTgfPortsArray[TX_PORT_IDX_CNS]);

    /* AUTODOC: disable "odd src-id" filtering on prvTgfPortsArray[EGR_ROOT_PORT_IDX_CNS] */
    rc = cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet(prvTgfDevNum, prvTgfPortsArray[EGR_ROOT_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet: portNum = %d",
                                 prvTgfPortsArray[EGR_ROOT_PORT_IDX_CNS]);

    /* AUTODOC: enable "odd src-id" filtering on prvTgfPortsArray[EGR_LEAF_PORT_IDX_CNS] */
    rc = cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet(prvTgfDevNum, prvTgfPortsArray[EGR_LEAF_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet: portNum = %d",
                                 prvTgfPortsArray[EGR_LEAF_PORT_IDX_CNS]);

    /* AUTODOC: enable "odd src-id" filtering on prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS] */
    rc = cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet(prvTgfDevNum, prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet: portNum = %d",
                                 prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS]);

}

/**
* @internal prvTgfBrgSrcIdPortOddOnlyFilteringPacketSendAndCheck function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number to send traffic from
* @param[in] packetInfoPtr            - PACKET to send
*                                       None
*/
static GT_VOID prvTgfBrgSrcIdPortOddOnlyFilteringPacketSendAndCheck
(
    IN GT_U32 caseNumber
)
{
    GT_STATUS    rc = GT_OK;    /* return code */
    GT_U32       portIter = 0;  /* itertore over test's ports */
    GT_U32       expectedGoodOctetsValue = 0;   /* expected good octets */
    GT_U32       expectedGoodPctsValue = 0;     /* expected good packets */
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* send Packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[TX_PORT_IDX_CNS]);

    /* AUTODOC: verify packets */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (portIter == TX_PORT_IDX_CNS)
        {
            expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS) * prvTgfBurstCount + 4;
            expectedGoodPctsValue = prvTgfBurstCount;
        }
        else
        {
            switch (caseNumber)
            {
                case 1:
                    /* packet egress from all the test ports */
                    expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS)*prvTgfBurstCount;
                    expectedGoodPctsValue = prvTgfBurstCount;
                    break;
                case 2:
                    /* packet filtered from the leaf ports */
                    if (portIter == EGR_LEAF_PORT_IDX_CNS || portIter == EGR_FDB_PORT_IDX_CNS)
                    {
                        expectedGoodOctetsValue = 0;
                        expectedGoodPctsValue = 0;
                    }
                    else
                    {
                        expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS)*prvTgfBurstCount;
                        expectedGoodPctsValue = prvTgfBurstCount;
                    }
                    break;
                case 3:
                    /* UC packet filtered from the EGR_FDB_PORT_IDX_CNS only */
                    expectedGoodOctetsValue = 0;
                    expectedGoodPctsValue = 0;
                    break;
            }
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodOctetsValue, portCntrs.goodOctetsSent.l[0],
                                    "get another goodOctetsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodPctsValue, portCntrs.goodPktsSent.l[0],
                                    "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfBrgSrcIdPortOddOnlyFilteringTrafficGenerate function
* @endinternal
*
* @brief   Function generate traffic and check counters
*
*/
GT_VOID prvTgfBrgSrcIdPortOddOnlyFilteringTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc; /* return code */

    /* AUTODOC: setup Packet */

   cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfMcMacAddr,
                 sizeof(prvTgfPacketL2Part.daMac));

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* AUTODOC: Part 1: even source-ID - packet should forwarded in all egress ports */
    PRV_UTF_LOG0_MAC("\n[TGF]: Part 1 - even source-ID\n");

    /* AUTODOC: set even source-ID */
    prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum, prvTgfPortsArray[TX_PORT_IDX_CNS], EVEN_SRC_ID_CNS);

    /* AUTODOC: send traffic and check results */
    prvTgfBrgSrcIdPortOddOnlyFilteringPacketSendAndCheck(1);

    /* AUTODOC: Part 2: odd source-ID - MC packet should be egress only from
       prvTgfPortsArray[EGR_ROOT_PORT_IDX_CNS] and filtered in
       prvTgfPortsArray[EGR_LEAF_PORT_IDX_CNS] and prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS] */
    PRV_UTF_LOG0_MAC("\n[TGF]: Part 2 - odd source-ID\n");

    /* AUTODOC: set odd source-ID */
    prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum, prvTgfPortsArray[TX_PORT_IDX_CNS], ODD_SRC_ID_CNS);

    /* AUTODOC: send traffic and check results */
    prvTgfBrgSrcIdPortOddOnlyFilteringPacketSendAndCheck(2);

    /* AUTODOC: Part 3: odd source-ID -
       add FDB entry with MAC 00:00:00:00:00:02, VLAN 6, port 0
       send UC packet to with macDA 00:00:00:00:00:02
       Filtered only in: and prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS] */
    PRV_UTF_LOG0_MAC("\n[TGF]: Part 3 - odd source-ID, sending UC traffic\n");

    /* AUTODOC: set PVID 6 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[TX_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgVlanPortVidSet");


    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:02, VLAN 6, port 0 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfFdbEntryMacAddr,
                                          PRV_TGF_VLANID_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS],
                                          GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgDefFdbMacEntryOnPortSet");

    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfFdbEntryMacAddr,
                 sizeof(prvTgfPacketL2Part.daMac));

    /* AUTODOC: setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* AUTODOC: set odd source-ID */
    prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum, prvTgfPortsArray[TX_PORT_IDX_CNS], ODD_SRC_ID_CNS);

    /* AUTODOC: send traffic and check results */
    prvTgfBrgSrcIdPortOddOnlyFilteringPacketSendAndCheck(3);
}

/**
* @internal prvTgfBrgSrcIdPortOddOnlyFilteringRestore function
* @endinternal
*
* @brief   Restore test configurations
*
*/
GT_VOID prvTgfBrgSrcIdPortOddOnlyFilteringRestore
(
    GT_VOID
)
{
    GT_STATUS rc; /* return code */
    /* AUTODOC: invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore configurations */
    rc = cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet(prvTgfDevNum, prvTgfPortsArray[EGR_ROOT_PORT_IDX_CNS],
                                                    origRootFilterEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet: portNum = %d",
                                 prvTgfPortsArray[EGR_ROOT_PORT_IDX_CNS]);

    rc = cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet(prvTgfDevNum, prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS],
                                                    origFdbRootFilterEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet: portNum = %d",
                                 prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS]);

    rc = cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet(prvTgfDevNum, prvTgfPortsArray[EGR_LEAF_PORT_IDX_CNS],
                                                    origLeafFilterEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet: portNum = %d",
                                 prvTgfPortsArray[EGR_LEAF_PORT_IDX_CNS]);

    rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum,prvTgfPortsArray[TX_PORT_IDX_CNS], origSrcId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortDefaultSrcIdSet: portNum = %d",
                                 prvTgfPortsArray[TX_PORT_IDX_CNS]);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

}

