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
* @file prvTgfClassifierNshOverEthernetUseCase.c
*
* @brief Classifier NSH over Ethernet use cases testing.
*
*        Ports allocation:
*           ^ Port#0: Ingress port
*           ^ Port#1: Target port
*
*        Packet flow:
*           1.  Ethernet frame packet enters the device via Ingress Port#0.
*           2.  The packet is sent towards target port#1 and undergoes the following actions:
*               -   Ethernet tunnel + NSH headers are added by HA unit
*               -   PHA thread is triggered in which fw removes extra
*                   12B added by HA and updates some of NSH metadata fields
*           3.  The encapsulated packet exits the device from target port#1.
*
* @version   1
********************************************************************************
*/
#include <tunnel/prvTgfClassifierNshOverEthernetUseCase.h>
#include <oam/prvTgfOamGeneral.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>




/*************************** Constants definitions ***********************************/

/* PHA fw thread ID of Classifier NSH over Ethernet */
#define PRV_TGF_PHA_THREAD_ID_CLASSIFIER_NSH_OVER_ETHERNET_CNS  20

/* Size of output packets in bytes include CRC */
#define PRV_TGF_PACKET_SIZE_CLASSIFIER_NSH_OVER_ETHERNET_CNS   110
#define PRV_TGF_MAX_PACKET_SIZE_CNS         PRV_TGF_PACKET_SIZE_CLASSIFIER_NSH_OVER_ETHERNET_CNS

/* Number of packets */
#define PRV_TGF_BURST_COUNT_CNS             2

/* Template profile index */
#define PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS  1

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_INDEX_CNS      8

/* Ports number allocation */
#define PRV_TGF_INGRESS_PORT_IDX_CNS        0
#define PRV_TGF_TARGET_PORT_IDX_CNS         1

/* Default VLAN Id */
#define PRV_TGF_DEFAULT_VLANID_CNS          1

/* Offset to Tunnel EtherType part in the output packet */
#define PRV_TGF_TUNNEL_ETHERTYPE_PKT_OFFSET_CNS  (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS)

/* Offset to NSH part in the output packet */
#define PRV_TGF_NSH_PKT_OFFSET_CNS  (PRV_TGF_TUNNEL_ETHERTYPE_PKT_OFFSET_CNS + TGF_ETHERTYPE_SIZE_CNS)


/*************************** static DB definitions ***********************************/
/* DB to hold input packet information */
static TGF_PACKET_STC   packetInfo;

/* DB to hold target port interface */
static CPSS_INTERFACE_INFO_STC  targetPortInterface;

/* Should holds port counters */
static GT_U32    actualPacketsNum[PRV_TGF_BURST_COUNT_CNS];
static GT_U32    actualBytesNum[PRV_TGF_BURST_COUNT_CNS];

/* DB to hold relevant PHA threads information */
static CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC     commonInfo;
static CPSS_DXCH_PHA_THREAD_INFO_UNT            extInfo;
static PRV_TGF_TUNNEL_START_ENTRY_UNT           tunnelEntry;

/* Tunnel start Entry : NSH headers + NSH metadata + 12B reserved */
static GT_U8 prvTgfTunnelStartArr[36] =
    {
        0x10, 0xc0, 0x10, 0x10, /* Base header (4B):        |Ver 2b|O 1b|U 1b|TTL 6b|Length 6b|Res 4b|MD Type 4b|Next protocol 8b| */
        0x20, 0x20, 0x23, 0x23, /* Service Path header(4B): |Service Path Identifier (SPI) 24b|Service Index 8b| */
        0x30, 0x30, 0x34, 0x34, /* NSH Metadata word 3(4B): |D 1b|F 2b|R 1b|Source Node ID 12b|Source Interface ID 16b| */
        0x40, 0x40, 0x45, 0x45, /* NSH Metadata word 2(4B): |Res 8b|Tenant ID 24b| */
        0x50, 0x50, 0x56, 0x56, /* NSH Metadata word 1(4B): |Res 16b|Source Class 16b| */
        0x60, 0x60, 0x67, 0x67, /* NSH Metadata word 0(4B): |Seconds 16b|Fraction 16b */
        0xFF, 0xFF, 0xFF, 0xFF, /* Reserved (12B) Should be removed later by PHA */
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF
    };

/* Tunnel section: L2 part */
static TGF_PACKET_L2_STC prvTgfPacketTunnelL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x44, 0x04},             /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x45, 0x05}              /* saMac */
};

/* Original Ingress Packet: L2 part */
static TGF_PACKET_L2_STC prvTgfOriginalPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},             /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x03}              /* saMac */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x66, 0x66, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
    0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
    0x2e, 0x2f, 0x30, 0x31 /* length 52 bytes */
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                      /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* parameters that are needed to be restored */
static struct
{
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC                egressInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT                  tunnelEntry;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT  cfgTabAccMode;
} prvTgfRestoreCfg;


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfClassifierNshOverEthernetBridgeConfigSet function
* @endinternal
*
* @brief   Bridge test configurations
*          Forward to ePort#1 according to MAC destination
*/
static GT_VOID prvTgfClassifierNshOverEthernetBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN ID 1, target port */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfOriginalPacketL2Part.daMac,
                                          PRV_TGF_DEFAULT_VLANID_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefFdbMacEntryOnPortSet");
}


/**
* @internal prvTgfClassifierNshOverEthernetHaConfigSet function
* @endinternal
*
* @brief   HA test configurations
*          Ethernet Tunnel is added
*          PHA fw thread is updated
*/
static GT_VOID prvTgfClassifierNshOverEthernetHaConfigSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC    egressInfo;

    PRV_UTF_LOG0_MAC("======= Setting HA Configuration =======\n");

    /* AUTODOC: Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoGet");

    /* AUTODOC: Set target port attributes configuration */
    /* AUTODOC: Tunnel Start = ENABLE, tunnelStartPtr = 8, tsPassenger = ETHERNET */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                       &egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");


    /* AUTODOC: Set Tunnel Start entry 8 with: */
    /* AUTODOC: genericType = long, tagEnable = TRUE, vlanId=1 */
    /* AUTODOC: ethType = 0x894F(NSH), MACDA = 00:00:00:00:44:04 */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.genCfg.tagEnable   = GT_TRUE;
    tunnelEntry.genCfg.vlanId      = PRV_TGF_DEFAULT_VLANID_CNS;
    tunnelEntry.genCfg.genericType = PRV_TGF_TUNNEL_START_GENERIC_LONG_TYPE_E;
    tunnelEntry.genCfg.etherType   = TGF_ETHERTYPE_894F_NSH_TAG_CNS; /* 0x894F */
    cpssOsMemCpy(tunnelEntry.genCfg.data, prvTgfTunnelStartArr, sizeof(prvTgfTunnelStartArr));
    cpssOsMemCpy(tunnelEntry.genCfg.macDa.arEther, prvTgfPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_GENERIC_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");


    /**** PHA fw threads configurations ****/
    /* AUTODOC: Assign Classifier NSH over Ethernet fw thread to target port */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                        GT_TRUE, PRV_TGF_PHA_THREAD_ID_CLASSIFIER_NSH_OVER_ETHERNET_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet");

    /* AUTODOC: Set the thread entry - PRV_TGF_PHA_THREAD_ID_CLASSIFIER_NSH_OVER_ETHERNET_CNS */
    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    extInfo.notNeeded = 0;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     PRV_TGF_PHA_THREAD_ID_CLASSIFIER_NSH_OVER_ETHERNET_CNS,
                                     &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_ETHERNET_E,
                                     &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");
}


/**
* internal prvTgfClassifierNshOverEthernetConfigSet function
* @endinternal
*
* @brief   Classifier NSH over Ethernet use case configurations
*/
GT_VOID prvTgfClassifierNshOverEthernetConfigSet
(
    GT_VOID
)
{
    /* Bridge Configuration */
    prvTgfClassifierNshOverEthernetBridgeConfigSet();

    /* HA Configuration */
    prvTgfClassifierNshOverEthernetHaConfigSet();
}


/**
* @internal prvTgfClassifierNshOverEthernetConfigRestore function
* @endinternal
*
* @brief   Classifier NSH over Ethernet configurations restore.
* @note    1. Restore Bridge Configuration
*          2. Restore HA Configuration
*          3. Restore base Configuration
*/
GT_VOID prvTgfClassifierNshOverEthernetConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Restore configurations =======\n");

    /* -------------------------------------------------------------------------
     * 1. Restore Bridge Configuration
     */
    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgFdbFlush");

    /* -------------------------------------------------------------------------
     * 2. Restore HA Configuration
     */
    /* AUTODOC: restore ePort attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    /* AUTODOC: Clear PHA fw thread from target port */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                    GT_FALSE,
                                    0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet");

    /* -------------------------------------------------------------------------
     * 3. Restore base Configuration
     */
    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Disable RX capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in tgfTrafficTableRxStartCapture");

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCommonAllCntrsReset");
}


/**
* internal tgfClassifierNshOverEthernetVerification function
* @endinternal*
*         Do the following:
*         - Check Tunnel Ethertype = NSH type (0x894F)
*         - Check NSH Base header was not overwritten
*         - Check NSH Service Path header was not overwritten
*         - Check NSH metadata:
*             - Check <Source Interface ID> was overwritten
*             - Check 6 MS bits of <Source Interface ID> are cleared to zeros
*         - Check MAC addresses of passenger are not damaged
*         - Check payload data of passenger is not damaged
*/
GT_VOID tgfClassifierNshOverEthernetVerification
(
    IN  GT_U8  * packetBuf
)
{
    GT_U32              pktOffset,tsOffset,payloadPktOffset;
    GT_U32              tunnelEtherTypeActualVal,tunnelEtherTypeExpectedVal,nshValActualVal,nshValExpectedVal;
    GT_U32              i;
    GT_U32              daMacErr=0, saMacErr=0, passengerPayloadErr=0;
    TGF_PACKET_L2_STC   receivedMac;
    GT_U32              recPassengerPayload, expPassengerPayload;


    PRV_UTF_LOG0_MAC("======= Check content of output packet  =======\n");

    PRV_UTF_LOG0_MAC("======= Check NSH Ethertype  =======\n");
    /* AUTODOC: Get right offset to Tunnel Ethertype field*/
    pktOffset = PRV_TGF_TUNNEL_ETHERTYPE_PKT_OFFSET_CNS;
    /* AUTODOC: Get actual value of Tunnel Ethertype from the packet we captured */
    tunnelEtherTypeActualVal = (GT_U32)(packetBuf[pktOffset+1] | (packetBuf[pktOffset+0] << 8));
    /* AUTODOC: NSH EtherType should be 0x894F */
    tunnelEtherTypeExpectedVal = TGF_ETHERTYPE_894F_NSH_TAG_CNS;
    /* AUTODOC: Compare received actual vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(tunnelEtherTypeExpectedVal, tunnelEtherTypeActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "Tunnel EtherType is not as expected => Expected:0x%x != Received:0x%x \n",
                                 tunnelEtherTypeExpectedVal, tunnelEtherTypeActualVal);


    PRV_UTF_LOG0_MAC("======= Check NSH Base header  =======\n");
    /* AUTODOC: Update offset to NSH Base header */
    pktOffset = PRV_TGF_NSH_PKT_OFFSET_CNS;
    /* AUTODOC: Update offset to expected tunnel start array */
    tsOffset  = 0;
    /* AUTODOC: Read from packet NSH Base header (4B) */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset+3] | (packetBuf[pktOffset+2] << 8) |
                              (packetBuf[pktOffset+1] << 16) | (packetBuf[pktOffset] << 24));
    /* AUTODOC: Get the expected NSH Base header. Should be the same as we set it in HA. */
    /* AUTODOC: PHA should not touch any field */
    nshValExpectedVal = (GT_U32)(prvTgfTunnelStartArr[tsOffset+3] | (prvTgfTunnelStartArr[tsOffset+2] << 8) |
                               (prvTgfTunnelStartArr[tsOffset+1] << 16) | (prvTgfTunnelStartArr[tsOffset+0] << 24));
    /* AUTODOC: Compare received actual vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH Base header is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    PRV_UTF_LOG0_MAC("======= Check NSH Service Path header  =======\n");
    /* AUTODOC: Update offset to NSH Service Path header */
    pktOffset += 4;
    /* AUTODOC: Update offset in tunnel start array to point to expected NSH Service path header */
    tsOffset  += 4;
    /* AUTODOC: Read from packet NSH Base header (4B) */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset+3] | (packetBuf[pktOffset+2] << 8) |
                              (packetBuf[pktOffset+1] << 16) | (packetBuf[pktOffset] << 24));
    /* AUTODOC: Get the expected NSH Service Path header. Should be the same as we set it in HA. */
    /* AUTODOC: PHA should not touch any field */
    nshValExpectedVal = (GT_U32)(prvTgfTunnelStartArr[tsOffset+3] | (prvTgfTunnelStartArr[tsOffset+2] << 8) |
                               (prvTgfTunnelStartArr[tsOffset+1] << 16) | (prvTgfTunnelStartArr[tsOffset+0] << 24));
    /* AUTODOC: Compare received actual vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH Service Path header is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    PRV_UTF_LOG0_MAC("======= Check NSH Metadata  =======\n");
    /* AUTODOC: Update offset to NSH Metadata */
    pktOffset += 4;
    /* AUTODOC: Update offset in tunnel start array to point to expected NSH metadata */
    tsOffset  += 4;
    /* AUTODOC: Read <Source Interface ID> from packet NSH metadata (2B) */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset+3] | (packetBuf[pktOffset+2] << 8));

    /* AUTODOC: Set the expected <Source Interface ID>.
       Should be different than origin since PHA overwrites it with Desc<Local Dev Src Port> */
    nshValExpectedVal = (GT_U32)(prvTgfTunnelStartArr[tsOffset+3] | (prvTgfTunnelStartArr[tsOffset+2] << 8));
    /* AUTODOC: Compare received actual vs expected and notify if there is a match !!! */
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "<Source Interface ID> from NSH metadata is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);

    /* Since Desc<Local Dev Src Port> is only 10b and <Source Interface ID> is 16b
       PHA clears 6 MS bits to zero*/
    nshValActualVal &= 0xFC00 ;
    nshValExpectedVal = 0;
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "6MS bits of <Source Interface ID> from NSH metadata are not zero => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    PRV_UTF_LOG0_MAC("======= Check MAC DA SA  =======\n");

    /* Increment offset by 16B (NSH metadata size) to point to MAC DA  */
    pktOffset +=16;

    /* AUTODOC: Get actual MAC DA & MAC SA */
    for (i=0;i<(TGF_L2_HEADER_SIZE_CNS/2);i++)
    {
        receivedMac.daMac[i] = packetBuf[pktOffset + i];
        receivedMac.saMac[i] = packetBuf[pktOffset + i + sizeof(prvTgfOriginalPacketL2Part.daMac)];

        /* Decrment received with expected to check if there is any mismatch */
        daMacErr += receivedMac.daMac[i] - prvTgfOriginalPacketL2Part.daMac[i] ;
        saMacErr += receivedMac.saMac[i] - prvTgfOriginalPacketL2Part.saMac[i] ;
    }

    /* AUTODOC: compare received MAC DA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, daMacErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in MAC DA address");
    if (daMacErr != 0)
    {
        /* Print received and expected MAC DA */
        PRV_UTF_LOG12_MAC("Expected MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfOriginalPacketL2Part.daMac[0],prvTgfOriginalPacketL2Part.daMac[1],prvTgfOriginalPacketL2Part.daMac[2],
                          prvTgfOriginalPacketL2Part.daMac[3],prvTgfOriginalPacketL2Part.daMac[4],prvTgfOriginalPacketL2Part.daMac[5],
                          receivedMac.daMac[0],receivedMac.daMac[1],receivedMac.daMac[2],
                          receivedMac.daMac[3],receivedMac.daMac[4],receivedMac.daMac[5]);
    }


    /* AUTODOC: compare received MAC SA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, saMacErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in MAC SA address \n");
    if (saMacErr != 0)
    {
        /* Print received and expected MAC SA */
        PRV_UTF_LOG12_MAC("Expected MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfOriginalPacketL2Part.saMac[0],prvTgfOriginalPacketL2Part.saMac[1],prvTgfOriginalPacketL2Part.saMac[2],
                          prvTgfOriginalPacketL2Part.saMac[3],prvTgfOriginalPacketL2Part.saMac[4],prvTgfOriginalPacketL2Part.saMac[5],
                          receivedMac.saMac[0],receivedMac.saMac[1],receivedMac.saMac[2],
                          receivedMac.saMac[3],receivedMac.saMac[4],receivedMac.saMac[5]);
    }



    PRV_UTF_LOG0_MAC("======= Check Passenger payload  =======\n");

    /* Increment offset by 12B of MACs to point to passenger payload (EtherType)  */
    pktOffset +=TGF_L2_HEADER_SIZE_CNS;

    /* Anchor offset to passenger */
    payloadPktOffset = pktOffset;

    /* AUTODOC: Get actual passenger payload  */
    for (i=0;i<prvTgfPacketPayloadPart.dataLength;i+=4)
    {
        /* Update offset every 4 bytes */
        pktOffset = payloadPktOffset + i;

        /* Get actual payload */
        recPassengerPayload = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                                   (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
        /* Get expected payload */
        expPassengerPayload = (GT_U32)(prvTgfPayloadDataArr[i+3] | (prvTgfPayloadDataArr[i+2] << 8) |
                                           (prvTgfPayloadDataArr[i+1] << 16) | (prvTgfPayloadDataArr[i] << 24));

        /* Decrement received with expected to check if there is any mismatch */
        passengerPayloadErr += (recPassengerPayload - expPassengerPayload) ;
    }

    /* AUTODOC: compare received passenger payload vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, passengerPayloadErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in passenger payload data ");
}



/**
* internal tgfClassiferNshOverEthernetPortCountersVerification function
* @endinternal
*
* @brief  Compare expected vs actual target port counters
*         Target port counters were already read upon packets transmission
*/
GT_VOID tgfClassiferNshOverEthernetPortCountersVerification
(
    IN  GT_U32    expPacketsNum,
    IN  GT_U32    actualPacketsNum,
    IN  GT_U32    expBytesNum,
    IN  GT_U32    actualBytesNum
)
{
    PRV_UTF_LOG0_MAC("======= Check target port counters  =======\n\n");

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(expPacketsNum, actualPacketsNum,
                                 "--- Failure in tgfClassiferNshOverEthernetPortCountersVerification --- \n"
                                 "Packets number is not as expected => Expected:0x%x != Received:0x%x \n",
                                  expPacketsNum, actualPacketsNum);

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(expBytesNum, actualBytesNum,
                                 "--- Failure in tgfClassiferNshOverEthernetPortCountersVerification --- \n"
                                 "Bytes number is not as expected => Expected:0x%x != Received:0x%x \n",
                                  expBytesNum, actualBytesNum);
}

/**
* internal prvTgfClassifierNshOverEthernetVerification function
* @endinternal
*
* @brief  Loop over packets check packet content and target port
*         counters
*/
GT_VOID prvTgfClassifierNshOverEthernetVerification
(
    GT_VOID
)
{
    GT_STATUS          rc = GT_OK;
    GT_U8              packetBuf[PRV_TGF_BURST_COUNT_CNS][PRV_TGF_MAX_PACKET_SIZE_CNS] = {{0}, {0}};
    GT_U32             packetLen[PRV_TGF_BURST_COUNT_CNS] = {PRV_TGF_PACKET_SIZE_CLASSIFIER_NSH_OVER_ETHERNET_CNS,
                                                             PRV_TGF_PACKET_SIZE_CLASSIFIER_NSH_OVER_ETHERNET_CNS};
    GT_U32             packetActualLength = 0;
    GT_U8              queue = 0;
    GT_U8              dev = 0;
    TGF_NET_DSA_STC    rxParam;
    GT_U8              pktNum;
    GT_BOOL            getFirst = GT_TRUE;
    GT_U32             expPacketsNum, expBytesNum;


    /* Loop over packets (currently only 1 packet) */
    for (pktNum=0; pktNum<1; pktNum++)
    {
        /* Expects packet to arrive to target port */
        expPacketsNum = 1;
        /* New packet format should be as shown below with total 110 bytes
           |L2|vlan|EtherType|NSH base hdr|NSH Service Path hdr|NSH metadata|
           |payload passenger|CRC| */
        expBytesNum = packetLen[pktNum];
        /* AUTODOC: Check port counters */
        tgfClassiferNshOverEthernetPortCountersVerification(expPacketsNum,
                                                            actualPacketsNum[pktNum],
                                                            expBytesNum,
                                                            actualBytesNum[pktNum]);

        /* AUTODOC: Get the captured packets */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&targetPortInterface,
                                                    TGF_PACKET_TYPE_CAPTURE_E,
                                                    getFirst, GT_TRUE, packetBuf[pktNum],
                                                    &packetLen[pktNum], &packetActualLength,
                                                    &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet");

        /* Call the actual verification function to check this packet */
        tgfClassifierNshOverEthernetVerification(packetBuf[pktNum]);

        /* Set to false for next iteration in order to get the next packet from next Rx CPU entry */
        getFirst = GT_FALSE;
    }
}


/**
* @internal prvTgfClassifierNshOverEthernetTest function
* @endinternal
*
* @brief   Classifier NSH over Ethernet use case test.
*          Create L2 tunnel over Ethernet packet.
*          During this process PHA fw thread is triggered in which extra 12B are removed
*          and some fields of NSH metadata are updated.
*          At the end packets are captured and content is checked to verify right behaviour.
*          Plus port counters are verified as well.
*/
GT_VOID prvTgfClassifierNshOverEthernetTest
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          portIter;

    PRV_UTF_LOG0_MAC("======= Ports allocation =======\n");
    PRV_UTF_LOG1_MAC("Port [%d]: Ingress Port \n", prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG1_MAC("Port [%d]: Target Port \n", prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS]);

    /* AUTODOC: reset counters for all 4 ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* Enable capture on target port */
    targetPortInterface.type             = CPSS_INTERFACE_PORT_E;
    targetPortInterface.devPort.hwDevNum = prvTgfDevNum;
    targetPortInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /* Set input packet interface structure  */
    packetInfo.numOfParts =  sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
    packetInfo.partsArray = prvTgfPacketPartArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");


    PRV_UTF_LOG0_MAC("======= Send first packet =======\n");
    /* Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Read counters from target port and save them to be verified */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");
    actualPacketsNum[0] =  portCntrs.goodPktsRcv.l[0];
    actualBytesNum[0] = portCntrs.goodOctetsRcv.l[0];

    /* Disable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");
}




