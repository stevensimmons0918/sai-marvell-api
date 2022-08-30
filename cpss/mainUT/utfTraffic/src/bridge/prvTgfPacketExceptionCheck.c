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
* @file prvTgfPacketExceptionCheck.c
*
* @brief Packet Exception Check Test
*
* @version 1
********************************************************************************
*/

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfNetIfGen.h>

#include <bridge/prvTgfPacketExceptionCheck.h>

#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>

/* Default VLAN Id */
#define PRV_TGF_DEF_VLANID_CNS                         1

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_DEF_VLANID_CNS                        /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPacketPayloadDataArr),                       /* dataLength */
    prvTgfPacketPayloadDataArr                                /* dataPtr */
};

/* PARTS of IPV4 packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of IPv4 packet */
#define PRV_TGF_TAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo =
{
    PRV_TGF_TAGGED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfTaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPartArray                                        /* partsArray */
};

/* Port Lock to CPU */
static CPSS_NET_RX_CPU_CODE_ENT portLockCpuCode = CPSS_NET_INTERVENTION_PORT_LOCK_E;

/* parameters that are needed to be restored */
static struct
{
    GT_BOOL                              statusGet;
    CPSS_PORT_LOCK_CMD_ENT               cmdGet;
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC entryGet;
    GT_U32                               srcPortGet;
    GT_U32                               tcQueueGet;
} prvTgfRestoreCfg;

/**
* @internal prvTgfPacketExceptionCheckConfigurationSet function
* @endinternal
*
* @brief   Packet exception check configurations
*
* @param[in] sendPortNum       - source port number
* @param[in] tcQueue           - traffic class queue
*
*/
GT_STATUS prvTgfPacketExceptionCheckConfigurationSet
(
    IN GT_U32 sendPortNum,
    IN GT_U8  tcQueue
)
{
    GT_STATUS                               rc;
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC    cpuCodeEntry;
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC      rxCounters;

    /* save input config parametrs for traffic test and restore */
    prvTgfRestoreCfg.srcPortGet = sendPortNum;
    prvTgfRestoreCfg.tcQueueGet = tcQueue;

    /* save state of new source MAC address learning on specified port */
    rc = cpssDxChBrgFdbPortLearnStatusGet(prvTgfDevNum, sendPortNum, &prvTgfRestoreCfg.statusGet, &prvTgfRestoreCfg.cmdGet);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("cpssDxChBrgFdbPortLearnStatusGet failed, device %d source port %d\n", prvTgfDevNum, sendPortNum);
    }

    /* set TRAP state for new source MAC address learnt on specified port */
    rc = cpssDxChBrgFdbPortLearnStatusSet(prvTgfDevNum, sendPortNum, GT_FALSE, CPSS_LOCK_TRAP_E);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("cpssDxChBrgFdbPortLearnStatusSet failed, device %d source port %d\n", prvTgfDevNum, sendPortNum);
    }

    /* save CPU code table entry for cpu code CPSS_NET_INTERVENTION_PORT_LOCK_E */
    rc = prvTgfNetIfCpuCodeTableGet(prvTgfDevNum, portLockCpuCode, &prvTgfRestoreCfg.entryGet);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("prvTgfNetIfCpuCodeTableGet failed, device %d cpuCode %d\n", prvTgfDevNum, portLockCpuCode);
    }

    /* set tcQueue for cpuCode CPSS_NET_INTERVENTION_PORT_LOCK_E to validate packet exception based on new MAC SA */
    cpssOsMemSet(&cpuCodeEntry, 0, sizeof(cpuCodeEntry));
    cpuCodeEntry.tc = tcQueue;
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum, portLockCpuCode, &cpuCodeEntry);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("prvTgfNetIfCpuCodeTableSet failed, device %d cpuCode %d\n", prvTgfDevNum, portLockCpuCode);
    }

    /* clear the queue counters */
    rc = cpssDxChNetIfSdmaRxCountersGet(prvTgfDevNum, tcQueue, &rxCounters);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("cpssDxChNetIfSdmaRxCountersGet failed, device %d tcQueue %d\n", prvTgfDevNum, tcQueue);
    }

    return GT_OK;
}

/**
* @internal prvTgfPacketExceptionCheckTrafficTest function
* @endinternal
*
* @brief   Packet exception check traffic test
*
* @param[in] sendPortNum      - port number from which traffic is ingress to device
*
*/
GT_STATUS prvTgfPacketExceptionCheckTrafficTest
(
    IN GT_U32 sendPortNum
)
{
    GT_STATUS                               rc;
    GT_U32                                  burstCount = 1;
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC      rxCounters;
    GT_U32                                  exceptionPktCount = 0;

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("tgfTrafficTableRxPcktTblClear failed, device %d\n", prvTgfDevNum);
    }

    /* configure and send test packets  */
    /* reset counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("prvTgfEthCountersReset failed, device %d\n", prvTgfDevNum);
    }

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfTaggedPacketInfo,
                             burstCount, 0, NULL);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("prvTgfSetTxSetupEth failed, device %d burst count %d\n", prvTgfDevNum, burstCount);
    }

    /* AUTODOC: send packet on port with: */
    /* AUTODOC: DA=00:00:00:00:00:04, SA=00:00:00:00:00:44, VID=1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("prvTgfStartTransmitingEth failed, device %d send port %d\n", prvTgfDevNum, sendPortNum);
    }

    /* Verify packet should reach in case of enabled queue*/
    rc = cpssDxChNetIfSdmaRxCountersGet(prvTgfDevNum, prvTgfRestoreCfg.tcQueueGet, &rxCounters);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("cpssDxChNetIfSdmaRxCountersGet failed, device %d tcQueue %d\n", prvTgfDevNum, prvTgfRestoreCfg.tcQueueGet);
    }

    exceptionPktCount = rxCounters.rxInPkts;
    rc = (exceptionPktCount == burstCount) ? GT_OK : GT_FAIL;
    if (rc != GT_OK)
    {
        PRV_UTF_LOG3_MAC("SDMA Rx count expected %d received %d tcQueue %d\n", burstCount, exceptionPktCount, prvTgfRestoreCfg.tcQueueGet);
    }
    PRV_UTF_LOG3_MAC("Packet sent %d SDMA Rx counter received %d for tcQueue %d\n", burstCount, exceptionPktCount, prvTgfRestoreCfg.tcQueueGet);

    return GT_OK;

}

/**
* @internal prvTgfPacketExceptionCheckConfigurationRestore function
* @endinternal
*
* @brief  Packet exception check configuration restore
*
*/
GT_STATUS prvTgfPacketExceptionCheckConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                           rc;
    GT_U32                              portIter;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("tgfTrafficTableRxPcktTblClear failed for device %d\n", prvTgfDevNum);
    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("prvTgfBrgFdbFlush failed for device %d\n", prvTgfDevNum);
    }

    /* restore TRAP state for new source MAC address learnt on specified port */
    rc = cpssDxChBrgFdbPortLearnStatusSet(prvTgfDevNum, prvTgfRestoreCfg.srcPortGet, prvTgfRestoreCfg.statusGet, prvTgfRestoreCfg.cmdGet);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("cpssDxChBrgFdbPortLearnStatusSet failed, device %d port %d\n", prvTgfDevNum, prvTgfRestoreCfg.srcPortGet);
    }

    /* restore CPU code table entry for cpu code CPSS_NET_INTERVENTION_PORT_LOCK_E */
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum, portLockCpuCode, &prvTgfRestoreCfg.entryGet);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("prvTgfNetIfCpuCodeTableSet failed, device %d cpuCode %d\n", prvTgfDevNum, portLockCpuCode);
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("prvTgfBrgFdbFlush failed for device %d port %d\n", prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    return GT_OK;
}
