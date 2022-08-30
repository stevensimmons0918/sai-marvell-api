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
* @file prvTgfMacSecUseCase.c
*
* @brief MACSec use cases testing.
*        1. Checking Egress and Ingress path using EIP-163 (Classifier) and EIP-164 (Transformer)
*           - Enabling Authentication and Encryption
*           - Enabling Authentication only (no Encryption)
*
*        Packet flow for Egress use case:
*           1.  Ethernet frame packet is sent via CPU to specific port (Egress)
*           2.  The packet should pass via Classifier and then Transformer Egress devices
*           3.  SecTag and ICV headers should be added (32B)
*           4.  Data integrity is always ensured and encryption is enabled or disabled according to user's configuration
*           5.  Packet is loopbacked and can be captured for inspection
*
*
*        Packet flow for Egress and Ingress use case:
*           1.  Ethernet frame packet is sent via CPU to specific port (Egress)
*           2.  The packet should pass via Classifier and then Transformer Egress devices
*           3.  SecTag and ICV headers should be added (32B)
*           4.  Data integrity is always ensured and encryption is enabled or disabled according to user's configuration
*           5.  Packet is loopbacked and ingress the device in which it passes via Classifier and then Transformer Ingress devices
*           6.  SecTag and ICV are removed and if encrypted packet is decrypted
*           7.  Packet is sent back to CPU and should be identical to the origin packet*
*
* @version   1
********************************************************************************
*/
#include <macSec/prvTgfMacSecUseCase.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <extUtils/trafficEngine/tgfTrafficEngine.h>
#include <common/tgfBridgeGen.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <extUtils/trafficEngine/tgfTrafficTable.h>
#include <cpss/dxCh/dxChxGen/macSec/cpssDxChMacSec.h>
#include <cpss/dxCh/dxChxGen/macSec/private/prvCpssDxChMacSec.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/events/private/prvCpssGenEvReq.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include<cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h>

/* EIP-163 devices IDs */
#define PRV_TGF_CFYE_EGRESS_DP0_DEVICE_ID_CNS   0
#define PRV_TGF_CFYE_INGRESS_DP0_DEVICE_ID_CNS  1
#define PRV_TGF_CFYE_EGRESS_DP1_DEVICE_ID_CNS   2
#define PRV_TGF_CFYE_INGRESS_DP1_DEVICE_ID_CNS  3

/* EIP-164 devices IDs */
#define PRV_TGF_SECY_EGRESS_DP0_DEVICE_ID_CNS   0
#define PRV_TGF_SECY_INGRESS_DP0_DEVICE_ID_CNS  1
#define PRV_TGF_SECY_EGRESS_DP1_DEVICE_ID_CNS   2
#define PRV_TGF_SECY_INGRESS_DP1_DEVICE_ID_CNS  3

#define PRV_TGF_MACSEC_HEADER_SIZE_CNS     32
#define PRV_TGF_MACSEC_ETHERTYPE_SIZE_CNS   2
#define PRV_TGF_MACSEC_TCI_AN_SIZE_CNS      1
#define PRV_TGF_MACSEC_SL_SIZE_CNS          1
#define PRV_TGF_MACSEC_PN_SIZE_CNS          4
#define PRV_TGF_MACSEC_SCI_SIZE_CNS         8
#define PRV_TGF_MACSEC_SECTAG_SIZE_CNS     16
#define PRV_TGF_MACSEC_ICV_SIZE_CNS        16
#define PRV_TGF_MACSEC_SECTAG_AN_CNS        2

/*************************** static DB definitions ***********************************/

/* Classifier and Transform handles */
static    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  classifyVportHandleEg;
static    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  classifyVportHandleIng;
static    CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   classifyRuleHandleEg;
static    CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   classifyRuleHandleIng;
static    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         secySAHandleEg;
static    CPSS_DXCH_MACSEC_SECY_SA_HANDLE         secySAHandleIng;
static    GT_U32                                  prvTgfMacSecDpId;

/* MACsec ICV for PN 0xb2c28464 */
static GT_U8 ICV1[] = {
    0x4f, 0x8d, 0x55, 0xe7, 0xd3, 0xf0, 0x6f, 0xd5,
    0xa1, 0x3c, 0x0c, 0x29, 0xb9, 0xd5, 0xb8, 0x80
};

/* MACsec ICV for PN 0x00000003 */
static GT_U8 ICV2[] = {
    0x56, 0x3e, 0xbf, 0xa9, 0xce, 0x27, 0x0c, 0x64,
    0x74, 0x96, 0x4c, 0xe0, 0xbd, 0xa1, 0xd3, 0xb8
};

/* MACsec ICV for PN 0xfffffffe */
static GT_U8 ICV3[] = {
    0xc2, 0x07, 0xb4, 0xfa, 0x6b, 0x3b, 0x08, 0x16,
    0x40, 0xfc, 0x24, 0xb9, 0xdc, 0x38, 0xd9, 0xe9
};

/* MACsec key */
static GT_U8 K1[] = {
    0xad, 0x7a, 0x2b, 0xd0, 0x3e, 0xac, 0x83, 0x5a,
    0x6f, 0x62, 0x0f, 0xdc, 0xb5, 0x06, 0xb3, 0x45,
};

/* MACsec SCI */
static GT_U8 SCI1[] = {
    0x12, 0x15, 0x35, 0x24, 0xc0, 0x89, 0x5e, 0x81,
};

/* Input packet to send for Egress MACSec devices */
static GT_U8 pktInForEgress[] =
{
    0xd6, 0x09, 0xb1, 0xf0,
    0x56, 0x63, 0x7a, 0x0d,
    0x46, 0xdf, 0x99, 0x8d,
    0x08, 0x00, 0x0f, 0x10,
    0x11, 0x12, 0x13, 0x14,
    0x15, 0x16, 0x17, 0x18,
    0x19, 0x1a, 0x1b, 0x1c,
    0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x2b, 0x2c,
    0x2d, 0x2e, 0x2f, 0x30,
    0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x00, 0x02
};

/* Pointer to MAC DA in Egress packet */
static GT_U8 * macDaEgPtr = &pktInForEgress[0];

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0xd6, 0x09, 0xb1, 0xf0, 0x56, 0x63},               /* dstMac */
    {0x7a, 0x0d, 0x46, 0xdf, 0x99, 0x8d}                /* srcMac */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e,
    0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e,
    0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x3a, 0x00, 0x02
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfMacSecPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

#define PRV_TGF_MACSEC_ENCRYPTED_PACKET_LEN_CNS \
    TGF_ETHERTYPE_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfMacSecPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                            /* totalLen */
    sizeof(prvTgfMacSecPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfMacSecPacketPartArray                                        /* partsArray */
};

/* Input packet to send for Ingress MACSec devices. In this case it is also used as the reference for Egress output */
static GT_U8 pktInForIngress[] =
{
    0xd6, 0x09, 0xb1, 0xf0,
    0x56, 0x63, 0x7a, 0x0d,
    0x46, 0xdf, 0x99, 0x8d,
    0x88, 0xe5, 0x2e, 0x00,
    0xb2, 0xc2, 0x84, 0x65,
    0x12, 0x15, 0x35, 0x24,
    0xc0, 0x89, 0x5e, 0x81,
    0x70, 0x1a, 0xfa, 0x1c,
    0xc0, 0x39, 0xc0, 0xd7,
    0x65, 0x12, 0x8a, 0x66,
    0x5d, 0xab, 0x69, 0x24,
    0x38, 0x99, 0xbf, 0x73,
    0x18, 0xcc, 0xdc, 0x81,
    0xc9, 0x93, 0x1d, 0xa1,
    0x7f, 0xbe, 0x8e, 0xdd,
    0x7d, 0x17, 0xcb, 0x8b,
    0x4c, 0x26, 0xfc, 0x81,
    0xe3, 0x28, 0x4f, 0x2b,
    0x7f, 0xba, 0x71, 0x3d,
    0x4f, 0x8d, 0x55, 0xe7,
    0xd3, 0xf0, 0x6f, 0xd5,
    0xa1, 0x3c, 0x0c, 0x29,
    0xb9, 0xd5, 0xb8, 0x80
};

/* Pointer to MAC DA in Ingress packet */
static GT_U8 * macDaIngPtr = &pktInForIngress[0];
/* Pointer to TCI AN fields in Ingress packet */
static GT_U8 * tciAnIngPtr = &pktInForIngress[14];
/* Pointer to SCI in Ingress packet */
static GT_U8 * sciIngPtr   = &pktInForIngress[20];

/* MACsec test port */
/* MACsec units appear to be present only in DP0 and DP1 based on test results
 * from emulator and hence for devices
 * with multiple DP instances(AC5P, harrier) we need to use ports which are present only
 * in DP instance 0 and 1. For Hawk/AC5P/SIP6_10 it is port number 1 and for
 * Harrier/SIP_6_20 it is port number 1/2/3. Phoenix/AC5X/SIP6_15 is a single
 * DP instance device and hence all test ports are applicable.
 */
typedef enum
{
    PRV_TGF_MACSEC_SEND_PORT_1_IDX_CNS    = 3,
    PRV_TGF_MACSEC_SEND_PORT_2_IDX_CNS    = 1
} PRV_TGF_MACSEC_PORT_IDX_ENT;

static GT_U32                                  packetActualLength;
static GT_U8                                   packetBuf[256];
static CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
static GT_U32                                  prvTgfSeqNrThreshold = 0;
static GT_U32                                  prvTgfSeqNumLo       = 0xb2c28464;
static GT_U32                                  prvTgfSeqNrThresholdGet;
static GT_U32                                  prvTgfSeqNumLoGet;
static GT_U32                                  prvTgfPktCntGet;
static GT_U32                                  prvTgfPktCnt         = 1;
static GT_MACSEC_UNIT_BMP                      dpBmp;
static GT_BOOL                                 prvTgfEncryptionProtect = GT_FALSE;
static GT_U32                                  prvTgfSendPort;

/* Event counter for all events */
static GT_U32          evErrorCtr = 0;

/* Callback function prototype for event counting */
typedef GT_STATUS DXCH_DATA_INTEGRITY_EVENT_CB_FUNC(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr
);

extern DXCH_DATA_INTEGRITY_EVENT_CB_FUNC               *dxChDataIntegrityEventIncrementFunc;
static DXCH_DATA_INTEGRITY_EVENT_CB_FUNC               *savedDataIntegrityErrorCb;

#ifdef CPSS_LOG_ENABLE
extern PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT[];
extern GT_U32                              prvCpssLogEnum_size_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT;
#define PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer) \
      PRV_CPSS_LOG_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer)
#else
#define PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer)
#endif

GT_STATUS prvTgfMacSecConfigParamSet
(
   IN PRV_TGF_MACSEC_CONFIG_PARAM_ENT paramId,
   IN GT_U32                          paramValue
)
{
    GT_STATUS rc = GT_OK;

    switch(paramId)
    {
        case PRV_TGF_MACSEC_CONFIG_PARAM_SEQ_NUM_LOW_E:
          prvTgfSeqNumLo = paramValue;
          break;
        case PRV_TGF_MACSEC_CONFIG_PARAM_SEQ_NUM_THR_E:
          prvTgfSeqNrThreshold = paramValue;
          break;
        case PRV_TGF_MACSEC_CONFIG_PARAM_TEST_PACKET_COUNT_E:
          prvTgfPktCnt = paramValue;
          break;
        default:
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
          break;
    }
    return rc;
}

GT_STATUS prvTgfMacSecConfigParamGet
(
   IN  PRV_TGF_MACSEC_CONFIG_PARAM_ENT paramId,
   OUT GT_U32                          *paramValuePtr
)
{
    GT_STATUS rc = GT_OK;

    switch(paramId)
    {
        case PRV_TGF_MACSEC_CONFIG_PARAM_SEQ_NUM_LOW_E:
          *paramValuePtr = prvTgfSeqNumLo;
          break;
        case PRV_TGF_MACSEC_CONFIG_PARAM_SEQ_NUM_THR_E:
          *paramValuePtr = prvTgfSeqNrThreshold;
          break;
        case PRV_TGF_MACSEC_CONFIG_PARAM_TEST_PACKET_COUNT_E:
          *paramValuePtr = prvTgfPktCnt;
          break;
        default:
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
          break;
    }
    return rc;
}


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* internal prvTgfMacSecEventConfigStore function
* @endinternal
*
* @brief   MACSec Event configuration parameters store
*/
GT_VOID prvTgfMacSecEventConfigStore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    rc = prvTgfMacSecConfigParamGet(PRV_TGF_MACSEC_CONFIG_PARAM_SEQ_NUM_LOW_E, &prvTgfSeqNumLoGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMacSecEventConfigStore param PRV_TGF_MACSEC_CONFIG_PARAM_SEQ_NUM_LOW_E\n");

    rc = prvTgfMacSecConfigParamGet(PRV_TGF_MACSEC_CONFIG_PARAM_SEQ_NUM_THR_E, &prvTgfSeqNrThresholdGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMacSecEventConfigStore param PRV_TGF_MACSEC_CONFIG_PARAM_SEQ_NUM_THR_E\n");

    rc = prvTgfMacSecConfigParamGet(PRV_TGF_MACSEC_CONFIG_PARAM_TEST_PACKET_COUNT_E, &prvTgfPktCntGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMacSecEventConfigStore param PRV_TGF_MACSEC_CONFIG_PARAM_TEST_PACKET_COUNT_E\n");

}

/**
 * @internal prvTgfMacSecEventConfigSet function
 * @endinternal
 *
 * @brief Set the test configuration parameters for MACSec event use case.
 *
 * @param[in] seqNumLo                    - sequence number lower 32 bits
 * @param[in] seqNumThr                   - sequence number threshold
 * @param[in] pktCnt                      - packet count for test
 *
 */
GT_VOID prvTgfMacSecEventConfigSet
(
    IN GT_U32 seqNumLo,
    IN GT_U32 seqNumThr,
    IN GT_U32 pktCnt
)
{
    GT_STATUS rc = GT_OK;

    rc = prvTgfMacSecConfigParamSet(PRV_TGF_MACSEC_CONFIG_PARAM_SEQ_NUM_LOW_E, seqNumLo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMacSecEventConfigRestore param PRV_TGF_MACSEC_CONFIG_PARAM_SEQ_NUM_LOW_E\n");

    rc = prvTgfMacSecConfigParamSet(PRV_TGF_MACSEC_CONFIG_PARAM_SEQ_NUM_THR_E, seqNumThr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMacSecEventConfigRestore param PRV_TGF_MACSEC_CONFIG_PARAM_SEQ_NUM_LOW_E\n");

    rc = prvTgfMacSecConfigParamSet(PRV_TGF_MACSEC_CONFIG_PARAM_TEST_PACKET_COUNT_E, pktCnt);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMacSecEventCon param PRV_TGF_MACSEC_CONFIG_PARAM_TEST_PACKET_COUNT_E\n");
}

/**
* internal prvTgfMacSecEventConfigRestore function
* @endinternal
*
* @brief   Restore the test configuration parameters
*/
GT_VOID prvTgfMacSecEventConfigRestore
(
    GT_VOID
)
{
    prvTgfMacSecEventConfigSet(prvTgfSeqNumLoGet /*sequence number*/, prvTgfSeqNrThresholdGet /* threshold number*/, prvTgfPktCntGet /* number of packets to send */);
}

/**
* internal prvTgfMacSecEgressConfigSet function
* @endinternal
*
* @brief   MACSec Egress use case configurations
*
* @param[in] encryptionProtect - enable/disable Encryption
*/
GT_VOID prvTgfMacSecEgressConfigSet
(
    IN GT_BOOL     encryptionProtect
)
{
    PRV_UTF_LOG0_MAC("======= MACSec Egress configurations =======\n");

    /* Call MACSec Egress configurations with below parameters:
       device number:      0
       Data path ID:       0
       Port number:        0
       Encryption enabled: 1        */
    prvTgfEncryptionProtect = encryptionProtect;
    prvTgfSendPort = PRV_CPSS_SIP_6_15_CHECK_MAC(prvTgfDevNum) ? PRV_TGF_MACSEC_SEND_PORT_1_IDX_CNS : PRV_TGF_MACSEC_SEND_PORT_2_IDX_CNS;

    prvCpssDxChMacSecEgressConfigSet(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort], encryptionProtect);
}

/**
* @internal prvTgfMacSecEgressConfigRestore function
* @endinternal
*
* @brief   MACSec configurations restore.
*/
GT_VOID prvTgfMacSecEgressConfigRestore
(
    GT_VOID
)
{
    PRV_UTF_LOG0_MAC("======= MACSec Egress restore configurations =======\n");

    /* Call MACSec Egress Exit function with below parameters:
       device number:      0 */
    prvCpssDxChMacSecEgressExit(prvTgfDevNum);
}

/**
* internal prvTgfMacSecEgressVerification function
* @endinternal
*
* @brief  MACSec Egress use case verification
*/
GT_VOID prvTgfMacSecEgressVerification
(
    GT_VOID
)
{
    GT_STATUS      rc = GT_OK;
    GT_U32         offset = 0;
    GT_U32         recvMacSecPktNum = 0;

    PRV_UTF_LOG0_MAC("======= MACSec Egress test verification =======\n");

    /* +-------------------+------------+-----------+-----------+------------+-------------+------------------+--------------+
     * | EtherType(2B)     | TCI(6Bits) | AN(2Bits) | SL(1B)    | PN(4B)     | SCI(8B)     |  Encrypted Frame |  ICV(16B)    |
     * +-------------------+------------+-----------+-----------+------------+-------------+------------------+--------------+
     * |      0x88E5       |            |           |           |            |             |                  |              |
     * +-------------------+------------+-----------+-----------+------------+-------------+------------------+--------------+
     */
    /* TCI Format */
    /* Field    Description
     *  V       Indicates Version Number, value is 0.
     *  ES      Indicates end station. If MPDUs are sent by an end station and the first 6B in the SCI are equal to the source
     *          MAC address,the ES can be set to 1. If the source MAC address is not used to identify the SCI,the ES is set to 0.
     *          If the ES is set to 1, the SC cannot be set to 1.
     *  SC      Indicates the secure channel. If the SCI is explicitly encapsulated in the SECTAG, the SC must be set to 1, If the
     *          SC is not set to 1, the SECTAG must not contain the SCI.
     *  SCB     Indicates the Ethernet passive optical network(EPON) broadcast identifier. The SCB is set to 1 in an MPDU only when
     *          the SC supportes EPON Single Copy Broadcast(SCB) capability. If the SCB is set to 1, the SC cannot be set to 1. If the
     *          ES is 1 and SCB is 0, the port identifier in the SCI must be 00/01. If the SCB is 1, the port identifier in the SCI is
     *          00/01, a value reserved for SCB capability.
     *   E      Indicates whether User Data is encrypted. The value 1 indiactes that user data is encrypted, and the value 0 indicates
     *          that user data is not encrypted. (If E is set to 1, user data must have been modified, so C must also be set to 1.)
     *   C      Indicates changed text. If C is set to 1, user data has been modified by an encryption or verification algorithm. Otherwise,
     *          user data has not been modified. That is, C indicates whether user data is same as secure data.
     *          E=1,C=1: indicates that a packet is encrypted, and user data must have been modified.
     *          E=1,C=0: invalid value.
     *          E=0,C=0: indicates that a packet is not encrypted, and only integrity protection is implemented. The integrity check algorithm
     *                   does not modify user data.
     *          E=0,C=1: indicates that a packet is not encrypted, and only integrity protection is implemented, but the integrity check
     *                   algorithm has modified user data.
     */

    /* +--------+-------+-------+-------+-------+-------+
     * |    V   |   ES  |  SC   |  SCB  |  E    |  C    |
     * +------------------------+---------------+-------+
     * |  Bit#7 | Bit#6 | Bit#5 | Bit#4 | Bit#3 | Bit#2 |
     * +--------+-------+-------+-------+-------+-------+
     */



    /* Compare total length of received packet including additional 32B (16B of SecTag header + 16B ICV) - should match */
    UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_PACKET_LEN_CNS + PRV_TGF_MACSEC_HEADER_SIZE_CNS + TGF_CRC_LEN_CNS,
                                 packetActualLength,
                                 "Expected Packet length [%d] different than received [%d]",
                                 PRV_TGF_PACKET_LEN_CNS + PRV_TGF_MACSEC_HEADER_SIZE_CNS + TGF_CRC_LEN_CNS, packetActualLength);

    /* Move the offset to check MACsec SecTag Header details */
    offset = TGF_L2_HEADER_SIZE_CNS;

    /* Compare MACsec etherType */
    UTF_VERIFY_EQUAL2_STRING_MAC(TGF_ETHERTYPE_88E5_MACSEC_TAG_CNS,
                                 ((packetBuf[offset] << 8) | packetBuf[offset+1]),
                                 "Expected MACsec ethertype [0x%x] different than received [0x%x]",
                                 TGF_ETHERTYPE_88E5_MACSEC_TAG_CNS, ((packetBuf[offset] << 8) | packetBuf[offset+1]));

    /* Move the offset to check MACsec SecTag TCI and AN */
    offset += PRV_TGF_MACSEC_ETHERTYPE_SIZE_CNS;

    /* Compare MACsec Version(V) - TCI : Bit#7 */
    UTF_VERIFY_EQUAL2_STRING_MAC(0,
                                 (packetBuf[offset] & 0x80) >> 7,
                                 "Expected MACsec Version [%d] different than received [%d]",
                                 0, packetBuf[offset] & 0x80);

    /* Compare MACsec End Station(ES) - TCI : BIT#6 */
    UTF_VERIFY_EQUAL2_STRING_MAC(0,
                                 (packetBuf[offset] & 0x40) >> 6,
                                 "Expected MACsec End Station [%d] different than received [%d]",
                                 0, packetBuf[offset] & 0x40);

    /* Compare MACsec Secure Channel(SC) - TCI : BIT#5 */
    UTF_VERIFY_EQUAL2_STRING_MAC(1,
                                 (packetBuf[offset] & 0x20) >> 5,
                                 "Expected MACsec Secure Channel [%d] different than received [%d]",
                                 1, packetBuf[offset] & 0x20);

    /* Compare MACsec Single Copy Broadcast(SCB) - TCI : BIT#4 */
    UTF_VERIFY_EQUAL2_STRING_MAC(0,
                                 (packetBuf[offset] & 0x10) >> 4,
                                 "Expected MACsec Single Copy Broadcast [%d] different than received [%d]",
                                 0, packetBuf[offset] & 0x10);

    if(prvTgfEncryptionProtect == GT_TRUE)
    {
        /* Compare MACsec Encryption(E) - TCI : BIT#3 */
        UTF_VERIFY_EQUAL2_STRING_MAC(1,
                                     (packetBuf[offset] & 0x8) >> 3,
                                     "Expected MACsec Encryption [%d] different than received [%d]",
                                     1, packetBuf[offset] & 0x8);

        /* Compare MACsec Changed Text(C) - TCI : BIT#2 */
        UTF_VERIFY_EQUAL2_STRING_MAC(1,
                                     (packetBuf[offset] & 0x4) >> 2,
                                     "Expected MACsec Changed Text [%d] different than received [%d]",
                                     1, packetBuf[offset] & 0x4);
    }
    else
    {
        /* Compare MACsec Encryption(E) - TCI : BIT#3 */
        UTF_VERIFY_EQUAL2_STRING_MAC(0,
                                     (packetBuf[offset] & 0x8) >> 3,
                                     "Expected MACsec Encryption [%d] different than received [%d]",
                                     0, packetBuf[offset] & 0x8);

        /* Compare MACsec Changed Text(C) - TCI : BIT#2 */
        UTF_VERIFY_EQUAL2_STRING_MAC(0,
                                     (packetBuf[offset] & 0x4) >> 2,
                                     "Expected MACsec Changed Text [%d] different than received [%d]",
                                     0, packetBuf[offset] & 0x4);
    }

    /* Compare MACsec Association Number(AN) : BIT# 1-0 */
    UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_MACSEC_SECTAG_AN_CNS,
                                 packetBuf[offset] & 0x3,
                                 "Expected MACsec AN [%d] different than received [%d]",
                                 PRV_TGF_MACSEC_SECTAG_AN_CNS, packetBuf[offset] & 0x3);

    /* Move the offset to check MACsec SecTag SL */
    offset += PRV_TGF_MACSEC_TCI_AN_SIZE_CNS;

    /* Compare MACsec Short Length */
    UTF_VERIFY_EQUAL2_STRING_MAC(0,
                                 packetBuf[offset],
                                 "Expected MACsec SL [%d] different than received [%d]",
                                 0, packetBuf[offset]);

    /* Move the offset to check MACsec SecTag PN */
    offset += PRV_TGF_MACSEC_SL_SIZE_CNS;

    /* Compare MACsec Packet Number */
    recvMacSecPktNum = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16) | (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfSeqNumLo + 1,
                                 recvMacSecPktNum,
                                 "Expected MACsec PN [%d] different than received [%d]",
                                 prvTgfSeqNumLo + 1, recvMacSecPktNum);

    /* Move the offset to check MACsec SecTag SCI */
    offset += PRV_TGF_MACSEC_PN_SIZE_CNS;

    /* Compare MACsec SCI */
    rc = cpssOsMemCmp(&packetBuf[offset], &SCI1[0], CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS) == 0 ? GT_OK: GT_FAIL;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Expected MACsec SCI different than received");

    if(prvTgfEncryptionProtect == GT_FALSE)
    {
        /* Move the offset to check Unecrypted Frame */
        offset += PRV_TGF_MACSEC_SCI_SIZE_CNS;

        /* Compare unencrypted frame etherType */
        UTF_VERIFY_EQUAL2_STRING_MAC(TGF_ETHERTYPE_0800_IPV4_TAG_CNS,
                                     ((packetBuf[offset] << 8) | packetBuf[offset+1]),
                                     "Expected unencrypted frame ethertype [0x%x] different than received [0x%x]",
                                     TGF_ETHERTYPE_0800_IPV4_TAG_CNS, ((packetBuf[offset] << 8) | packetBuf[offset+1]));

        /* Compare Unencrypted frame data */
        rc = cpssOsMemCmp(&packetBuf[offset + TGF_ETHERTYPE_SIZE_CNS], &prvTgfPayloadDataArr[0], (PRV_TGF_MACSEC_ENCRYPTED_PACKET_LEN_CNS - TGF_ETHERTYPE_SIZE_CNS))
            == 0 ? GT_OK: GT_FAIL;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Expected unencrypted frame data different than received");

        /* Move the offset to check MACsec ICV */
        offset += PRV_TGF_MACSEC_ENCRYPTED_PACKET_LEN_CNS;
    }
    else
    {
        /* Move the offset to check MACsec ICV */
        offset += PRV_TGF_MACSEC_SCI_SIZE_CNS + PRV_TGF_MACSEC_ENCRYPTED_PACKET_LEN_CNS;
    }

    switch(prvTgfSeqNumLo)
    {
        case 0xb2c28465:
            rc = cpssOsMemCmp(&packetBuf[offset], &ICV1[0], PRV_TGF_MACSEC_ICV_SIZE_CNS) == 0 ? GT_OK: GT_FAIL;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Expected MACsec ICV different than received");
            break;
        case 0x00000004:
            rc = cpssOsMemCmp(&packetBuf[offset], &ICV2[0], PRV_TGF_MACSEC_ICV_SIZE_CNS) == 0 ? GT_OK: GT_FAIL;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Expected MACsec ICV different than received");
            break;
        case 0xfffffffe:
            rc = cpssOsMemCmp(&packetBuf[offset], &ICV3[0], PRV_TGF_MACSEC_ICV_SIZE_CNS) == 0 ? GT_OK: GT_FAIL;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Expected MACsec ICV different than received");
            break;
        default:
            PRV_UTF_LOG0_MAC("Invalid PN for test\n");
            break;
    }
}

/**
* internal prvTgfMacSecEventsVerification function
* @endinternal
*
* @brief  MACSec events verification
*
* @param[in] testNo - test case number
*/
GT_VOID prvTgfMacSecEventsVerification
(
    IN GT_U8 testNo
)
{
    GT_STATUS      rc = GT_OK;
    char           *uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME}; /* Array that contains string names of unified events */
    GT_U32         counter;

    switch(testNo)
    {
        case 1:
            {
                GT_U32 saPnThrSummaryIndexesArr[10] = {0};
                GT_U32 saPnThrSummaryIndexesArrSize = 0;

                /* get packet number full events counter */
                rc=utfGenEventCounterGet(prvTgfDevNum, CPSS_PP_MACSEC_SA_PN_FULL_E, GT_TRUE, &counter);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "utfGenEventCounterGet uniEv %s count %d\n", uniEvName[CPSS_PP_MACSEC_SA_PN_FULL_E], counter);
                rc = (counter >= 1) ? GT_OK : GT_FAIL;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "packet number full events count [%d] different than expected \n", counter);

                /* call cpssDxChMacSecSAPNThresholdSummaryGet to clear SA PN Threshold summary register bits */
                rc = cpssDxChMacSecSAPNThresholdSummaryGet(prvTgfDevNum, dpBmp, saPnThrSummaryIndexesArr, &saPnThrSummaryIndexesArrSize);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChMacSecSAPNThresholdSummaryGet indexes arr count %d\n", saPnThrSummaryIndexesArrSize);

                /* get transform engine events counter */
                rc=utfGenEventCounterGet(prvTgfDevNum, CPSS_PP_MACSEC_TRANSFORM_ENGINE_ERR_E, GT_TRUE, &counter);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "utfGenEventCounterGet uniEv %s count %d\n", uniEvName[CPSS_PP_MACSEC_TRANSFORM_ENGINE_ERR_E], counter);
                UTF_VERIFY_EQUAL2_STRING_MAC(2, counter, "transform engine error events count [%d] different than expected [%d] \n", counter, 2);
                break;
            }
        case 2:
            /* get egress sequence number rollover events counter */
            rc=utfGenEventCounterGet(prvTgfDevNum, CPSS_PP_MACSEC_EGRESS_SEQ_NUM_ROLLOVER_E, GT_TRUE, &counter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "utfGenEventCounterGet uniEv %s count %d\n", uniEvName[CPSS_PP_MACSEC_EGRESS_SEQ_NUM_ROLLOVER_E], counter);
            rc = (counter >= 1) ? GT_OK : GT_FAIL;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "egress sequence number rollover events count [%d] different than expected [%d] \n", counter, 1);

            /* get transform engine events counter */
            rc=utfGenEventCounterGet(prvTgfDevNum, CPSS_PP_MACSEC_TRANSFORM_ENGINE_ERR_E, GT_TRUE, &counter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "utfGenEventCounterGet uniEv %s count %d\n", uniEvName[CPSS_PP_MACSEC_TRANSFORM_ENGINE_ERR_E], counter);
            rc = (counter >= 1) ? GT_OK : GT_FAIL;
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "transform engine error events count [%d] different than expected [%d] \n", counter, 1);
            break;
        default:
            PRV_UTF_LOG0_MAC("Invalid test case verification\n");
            break;
    }
}

/**
* @internal prvTgfMacSecEgressTest function
* @endinternal
*
* @brief   MACSec Egress use case test.
*/
GT_VOID prvTgfMacSecEgressTest
(
    GT_VOID
)
{
    GT_STATUS                          rc;
    GT_U8                              portIter;
    GT_U8                              queue = 0;
    GT_U8                              dev = 0;
    static TGF_NET_DSA_STC             rxParam;
    static CPSS_INTERFACE_INFO_STC     ingressPortInterface;
    static GT_BOOL                     getFirst = GT_TRUE;
    GT_U32                             packetLen = sizeof(packetBuf);
    GT_U32                             packetCnt;
    CPSS_PORT_MAC_COUNTER_SET_STC      portCntrs;/* current counters of the port */

    PRV_UTF_LOG0_MAC("======= MACSec test start =======\n");

    /* Start capturing Tx packets */
    ingressPortInterface.type              = CPSS_INTERFACE_PORT_E;
    ingressPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    ingressPortInterface.devPort.portNum   = prvTgfPortsArray[prvTgfSendPort];

    /* AUTODOC: enable capture on ingress port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&ingressPortInterface,
            TGF_CAPTURE_MODE_MIRRORING_E,
            GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "Failure in tgftrafficgeneratorporttxethcaptureset port - %d",
            ingressPortInterface.devPort.portNum);

    /* AUTODOC: clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    for(packetCnt = 0; packetCnt < prvTgfPktCnt; packetCnt++)
    {

        /* AUTODOC: setup Packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfMacSecPacketInfo, 1, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfSetTxSetupEth");

        /* AUTODOC: send Packet from Port 0 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in prvTgfStartTransmitingEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort]);

    }

    /* MACSec event test result in a lot of interrupts.
       Need more time to wait for packet to CPU. */
    cpssOsTimerWkAfter(500);

    /* AUTODOC: Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&ingressPortInterface,
            TGF_CAPTURE_MODE_MIRRORING_E,
            GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "Failure in tgftrafficgeneratorporttxethcaptureset port - %d",
            ingressPortInterface.devPort.portNum);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (prvTgfSendPort == portIter)
        {
            if(prvTgfSeqNumLo == 0xFFFFFFFE)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPktCnt - 1, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");
                /* check Tx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPktCnt - 1, portCntrs.goodPktsSent.l[0],
                                             "get another goodPktsSent counter than expected");
            }
            else
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPktCnt, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");
                /* check Tx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPktCnt, portCntrs.goodPktsSent.l[0],
                                             "get another goodPktsSent counter than expected");
            }
            continue;
        }
        else
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
        }
    }

    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&ingressPortInterface,
            TGF_PACKET_TYPE_CAPTURE_E,
            getFirst, GT_TRUE, packetBuf,
            &packetLen, &packetActualLength,
            &dev, &queue, &rxParam);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficTableRxPcktTblClear");
}

/**
* internal prvTgfMacSecEgressIngressConfigSet function
* @endinternal
*
* @brief   MACSec Egress & Ingress use case configurations
*/
GT_VOID prvTgfMacSecEgressIngressConfigSet
(
    GT_VOID
)
{
    PRV_UTF_LOG0_MAC("======= MACSec Egress & Ingress configurations =======\n");

    /* Call MACSec Egress & Ingress configurations with below parameters:
       device number:      0
       Data path ID:       0
       Port number:        0
       Encryption enabled: 1        */
    prvTgfSendPort = PRV_CPSS_SIP_6_15_CHECK_MAC(prvTgfDevNum) ? PRV_TGF_MACSEC_SEND_PORT_1_IDX_CNS : PRV_TGF_MACSEC_SEND_PORT_2_IDX_CNS;
    prvCpssDxChMacSecEgressIngressConfigSet(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort], GT_TRUE);
}

/**
* @internal prvTgfMacSecEgressIngressConfigRestore function
* @endinternal
*
* @brief   MACSec configurations restore.
*
*/
GT_VOID prvTgfMacSecEgressIngressConfigRestore
(
    GT_VOID
)
{
    PRV_UTF_LOG0_MAC("======= MACSec Egress & Ingress Restore configurations =======\n");

    /* Call MACSec Egress Ingress Exit function with below parameters:
       device number:      0 */
    prvCpssDxChMacSecEgressIngressExit(prvTgfDevNum);
}

/**
* internal prvTgfMacSecEgressIngressVerification function
* @endinternal
*
* @brief  MACSec Egress & Ingress use case verification
*/
GT_VOID prvTgfMacSecEgressIngressVerification
(
    GT_VOID
)
{
    GT_STATUS      rc = GT_OK;
    GT_U32         offset = 0;

    PRV_UTF_LOG0_MAC("======= MACSec Egress & Ingress test verification =======\n");

    /* Compare total length of received packet, should be same as original packet as SecTag and ICV would be removed on Ingress MAcSec */
    UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS,
                                 packetActualLength,
                                 "Expected Packet length [%d] different than received [%d]",
                                 PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS, packetActualLength);

    /* Move the offset to check MACsec SecTag Header details */
    offset = TGF_L2_HEADER_SIZE_CNS;

    /* Compare decrypted frame etherType */
    UTF_VERIFY_EQUAL2_STRING_MAC(TGF_ETHERTYPE_0800_IPV4_TAG_CNS,
                                 ((packetBuf[offset] << 8) | packetBuf[offset+1]),
                                 "Expected unencrypted frame ethertype [0x%x] different than received [0x%x]",
                                 TGF_ETHERTYPE_0800_IPV4_TAG_CNS, ((packetBuf[offset] << 8) | packetBuf[offset+1]));

    /* Compare dencrypted frame data */
    rc = cpssOsMemCmp(&packetBuf[offset + TGF_ETHERTYPE_SIZE_CNS], &prvTgfPayloadDataArr[0], (PRV_TGF_MACSEC_ENCRYPTED_PACKET_LEN_CNS - TGF_ETHERTYPE_SIZE_CNS))
        == 0 ? GT_OK: GT_FAIL;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Expected unencrypted frame data different than received");
}

/**
* @internal prvTgfMacSecEgressIngressTest function
* @endinternal
*
* @brief   MACSec Egress & Ingress use case test.
*/
GT_VOID prvTgfMacSecEgressIngressTest
(
    GT_VOID
)
{
    GT_STATUS                          rc;
    GT_U8                              portIter;
    GT_U8                              queue = 0;
    GT_U8                              dev = 0;
    static TGF_NET_DSA_STC             rxParam;
    static CPSS_INTERFACE_INFO_STC     ingressPortInterface;
    static GT_BOOL                     getFirst = GT_TRUE;
    GT_U32                             packetLen = sizeof(packetBuf);
    GT_U32                             packetCnt;
    CPSS_PORT_MAC_COUNTER_SET_STC      portCntrs;/* current counters of the port */

    PRV_UTF_LOG0_MAC("======= MACSec Egress & Ingress test start =======\n");
    PRV_UTF_LOG0_MAC("======= MACSec test start =======\n");

    /* Start capturing Tx packets */
    ingressPortInterface.type              = CPSS_INTERFACE_PORT_E;
    ingressPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    ingressPortInterface.devPort.portNum   = prvTgfPortsArray[prvTgfSendPort];

    /* AUTODOC: enable capture on ingress port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&ingressPortInterface,
            TGF_CAPTURE_MODE_MIRRORING_E,
            GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "Failure in tgftrafficgeneratorporttxethcaptureset port - %d",
            ingressPortInterface.devPort.portNum);

    /* AUTODOC: clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    for(packetCnt = 0; packetCnt < prvTgfPktCnt; packetCnt++)
    {

        /* AUTODOC: setup Packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfMacSecPacketInfo, 1, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfSetTxSetupEth");

        /* AUTODOC: send Packet from Port 0 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in prvTgfStartTransmitingEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort]);

    }

    /* AUTODOC: Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&ingressPortInterface,
            TGF_CAPTURE_MODE_MIRRORING_E,
            GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "Failure in tgftrafficgeneratorporttxethcaptureset port - %d",
            ingressPortInterface.devPort.portNum);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (prvTgfSendPort == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPktCnt, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPktCnt, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
            continue;
        }
        else
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
        }
    }

    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&ingressPortInterface,
            TGF_PACKET_TYPE_CAPTURE_E,
            getFirst, GT_TRUE, packetBuf,
            &packetLen, &packetActualLength,
            &dev, &queue, &rxParam);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficTableRxPcktTblClear");
}



/**
* @internal prvCpssDxChMacSecEgressConfigSet function
* @endinternal
*
* @brief   MACSec Egress configurations
*          - Initialize database of MACSec devices
*          - Enable Egress MACSec devices (EIP-163 & EIP-164) in PCA EXT wrapper
*          - Initilaize Egress EIP-163 device (Classifier)
*          - Initilaize Egress EIP-164 device (Transformer)
*          - Install vPort in Egress Classifier device
*          - Use the classifier vPort to install SA with Transform Record in Egress Transform device
*          - Install Rule and enable it in Egress Classifier device
*
* @param[in] devNum            - the device number
* @param[in] dpId              - data path ID
* @param[in] portNum           - port number
* @param[in] encryptionProtect - enable/disable Encryption
*
**/
GT_VOID prvCpssDxChMacSecEgressConfigSet
(
    IN GT_U8       devNum,
    IN GT_PORT_NUM portNum,
    IN GT_BOOL     encryptionProtect
)
{
    GT_STATUS                               rc;
    GT_U32                                  vPortEgPolicyId = 0;
    CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC  classifyPortCfg;
    CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC      secyPortCfg;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      ruleParams;
    GT_U32                                  i;
    GT_U32                                  channelNum;

    /* Check portNum is valid plus get Data path ID and local channel */
    rc = prvCpssDxChPortPhysicalPortMapCheckAndMacSecConvert(devNum, portNum, &prvTgfMacSecDpId, &channelNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvCpssDxChPortPhysicalPortMapCheckAndMacSecConvert");

    PRV_UTF_LOG0_MAC("\n======= Input parameters =======================\n");
    PRV_UTF_LOG1_MAC("Device number:      %d \n", devNum);
    PRV_UTF_LOG1_MAC("Data path ID:       %d \n", prvTgfMacSecDpId);
    PRV_UTF_LOG1_MAC("Port number:        %d \n", portNum);
    PRV_UTF_LOG1_MAC("Encryption enabled: %d \n", encryptionProtect);
    PRV_UTF_LOG0_MAC("==================================================\n");


/********** MACSec initialization *****************************************************************/

    PRV_UTF_LOG0_MAC("==== MACSec initialization start ==== \n");

    /* Convert Data path ID into bits map format */
    dpBmp = (BIT_0 << prvTgfMacSecDpId);

    /* Call MACSec initialization procedure */
    rc = cpssDxChMacSecInit(devNum,dpBmp);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecInit");

    PRV_UTF_LOG0_MAC("==== Set port to MACSec mode ==== \n");

    /* Since all ports after init are set to bypass mode, need to enable the requested port to work in MACSec mode */

    /* First Classifier */
    /* Clear structure before filling it  */
    cpssOsMemSet(&classifyPortCfg, 0, sizeof(classifyPortCfg));

    /* Disable MACSec bypass therefore enable MACSec mode */
    classifyPortCfg.bypassMacsecDevice = GT_FALSE;

    /* disable configuration of exception case. Leave default */
    classifyPortCfg.exceptionCfgEnable = GT_FALSE;

    /* Call CPSS API to set Classifier port with MACSec mode */
    rc = cpssDxChMacSecPortClassifyConfigSet(devNum,
                                             portNum,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             &classifyPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecPortClassifyConfigSet");


    /* Now Transformer */
    /* Clear structure before filling it  */
    cpssOsMemSet(&secyPortCfg, 0, sizeof(secyPortCfg));

    /* Get current parameters */
    rc = cpssDxChMacSecPortSecyConfigGet(devNum,
                                         portNum,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                         &secyPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecPortSecyConfigGet");

    /* Set to MACSec mode */
    secyPortCfg.bypassMacsecDevice = GT_FALSE;

    /* Set to MACSec Sequence Number Threshold */
    secyPortCfg.statCtrl.seqNrThreshold = prvTgfSeqNrThreshold;

    rc = cpssDxChMacSecPortSecyConfigSet(devNum,
                                         portNum,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                         &secyPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecPortSecyConfigSet");

    PRV_UTF_LOG0_MAC("==== MACSec initialization end ==== \n");


/** Egress Install vPort in Classifier device ******************************************************/

    PRV_UTF_LOG0_MAC("==== vPort add start ==== \n");

    /* Clear structure before filling them  */
    cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

    /* For Egress only !!! Packet expansion indication. 11b = 32 bytes (sectag header + icv) */
    vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_32B_E;

    /* Egress: SecTAG location relative to the start of the packet. 12d = Standard MACsec (no VLAN tags to be bypassed in clear) */
    vPortParams.secTagOffset = 12;

    /* Clear vPort handle */
    classifyVportHandleEg = 0;

    /* Call CPSS API to add vPort in Classifier unit */
    rc = cpssDxChMacSecClassifyVportAdd(devNum,
                                        dpBmp,
                                        CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                        &vPortParams,
                                        &classifyVportHandleEg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecClassifyVportAdd");

    /* Call CPSS API to get vPort index to use when installing SA */
    rc = cpssDxChMacSecClassifyVportIndexGet(devNum,
                                             classifyVportHandleEg,
                                             &vPortEgPolicyId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecClassifyVportIndexGet");

    PRV_UTF_LOG0_MAC("==== vPort add end ==== \n");


/** Egress Install SA with transform record in Transform device ************************************/

    PRV_UTF_LOG0_MAC("==== SA add start ==== \n");

    /* Set SA parameters */
    cpssOsMemSet(&saParams,0,sizeof(saParams));
    saParams.actionType                         = CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E;
    saParams.destPort                           = CPSS_DXCH_MACSEC_SECY_PORT_COMMON_E;
    saParams.params.egress.protectFrames        = GT_TRUE;
    saParams.params.egress.includeSci           = GT_TRUE;
    saParams.params.egress.confProtect          = (encryptionProtect) ? GT_TRUE : GT_FALSE; /* Set Encryption to on or off */
    saParams.params.egress.controlledPortEnable = GT_TRUE;
    saParams.params.egress.preSecTagAuthLength  = 12;  /* MAC DA,SA */

    /* Set transform record parameters */
    cpssOsMemSet(&trRecParams,0,sizeof(trRecParams));
    trRecParams.an              = PRV_TGF_MACSEC_SECTAG_AN_CNS;
    trRecParams.keyByteCount    = sizeof(K1);
    trRecParams.seqTypeExtended = GT_FALSE;
    trRecParams.seqNumLo        = prvTgfSeqNumLo;
    trRecParams.seqNumHi        = 0;
    trRecParams.seqMask         = 0;
    /* Set key */
    for (i=0;i<sizeof(K1);i++)
    {
        trRecParams.keyArr[i] = K1[i];
    }
    /* Set SCI */
    for (i=0;i<CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS;i++)
    {
        trRecParams.sciArr[i] = SCI1[i];
    }

    /* Clear SA handle */
    secySAHandleEg = 0;

    /* Call CPSS API to add SA */
    rc = cpssDxChMacSecSecySaAdd(devNum,
                                 dpBmp,
                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                 vPortEgPolicyId,
                                 &saParams,
                                 &trRecParams,
                                 &secySAHandleEg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecSecySaAdd");

    PRV_UTF_LOG0_MAC("==== SA add end ==== \n");


/** Egress Install Rule in Classifier device *******************************************************/

    PRV_UTF_LOG0_MAC("==== Rule add start ==== \n");

    /* Set rule parameters */
    cpssOsMemSet(&ruleParams,0,sizeof(ruleParams));
    /* Set mask. Exact match on all these fields. */
    ruleParams.mask.portNum    = 0x3f;
    ruleParams.mask.numTags    = 0x7f;
    /* Set data */
    ruleParams.key.portNum    = portNum;
    ruleParams.key.numTags    = 0x01; /* Bit[0] No VLAN tags */
    /* Set mask. Match on Destination address. */
    ruleParams.dataMask[0]=0xffffffff;
    ruleParams.dataMask[1]=0xffff0000;
    /* Set MAC DA */
    ruleParams.data[0] =  (macDaEgPtr[0]<<24) | (macDaEgPtr[1]<<16) | (macDaEgPtr[2]<<8) | macDaEgPtr[3];
    ruleParams.data[1] =  (macDaEgPtr[4]<<24) | (macDaEgPtr[5]<<16);

    /* Clear rule tag */
    classifyRuleHandleEg = 0;

    /* Call CPSS API to add rule  */
    rc = cpssDxChMacSecClassifyRuleAdd(devNum,
                                       dpBmp,
                                       CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                       classifyVportHandleEg,
                                       &ruleParams,
                                       &classifyRuleHandleEg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecClassifyRuleAdd");

    /* Call CPSS API to enable TCAM rule */
    rc = cpssDxChMacSecClassifyRuleEnable(devNum,
                                          dpBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                          classifyRuleHandleEg,
                                          GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecClassifyRuleEnable");

    PRV_UTF_LOG0_MAC("==== Rule add end ==== \n\n");

    return;
}


/**
* @internal prvCpssDxChMacSecEgressIngressConfigSet function
* @endinternal
*
* @brief   MACSec initialization configurations
*          - Initialize database of MACSec devices
*          - Enable Egress and Ingress MACSec devices (EIP-163 & EIP-164) in PCA EXT wrapper
*          - Initilaize Egress and Ingress EIP-163 device (Classifier)
*          - Initilaize Egress and Ingress EIP-164 device (Transformer)
*          - Install vPort in Egress and Ingress Classifier devices
*          - Use the classifier vPort to install SA with Transform Record in Egress and Ingress Transform devices
*          - Install Rule and enable it in Egress and Ingress Classifier devices
*
* @param[in] devNum            - the device number
* @param[in] portNum           - port number
* @param[in] encryptionProtect - enable/disable Encryption
*
*/
GT_VOID prvCpssDxChMacSecEgressIngressConfigSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      encryptionProtect
)
{
    GT_STATUS                               rc;
    GT_U32 vPortEgPolicyId                  = 0;
    GT_U32 vPortIngPolicyId                 = 0;
    CPSS_DXCH_MACSEC_SECY_SA_STC            saParams;
    CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC     trRecParams;
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      ruleParams;
    CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC  classifyPortCfg;
    CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC      secyPortCfg;
    CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     vPortParams;
    GT_U32                                  i;
    GT_U32                                  channelNum;

    /* Check portNum is valid plus get Data path ID and local channel */
    rc = prvCpssDxChPortPhysicalPortMapCheckAndMacSecConvert(devNum, portNum, &prvTgfMacSecDpId, &channelNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvCpssDxChPortPhysicalPortMapCheckAndMacSecConvert");

    PRV_UTF_LOG0_MAC("\n======= Input parameters =======================\n");
    PRV_UTF_LOG1_MAC("Device number:      %d \n", devNum);
    PRV_UTF_LOG1_MAC("Data path ID:       %d \n", prvTgfMacSecDpId);
    PRV_UTF_LOG1_MAC("Port number:        %d \n", portNum);
    PRV_UTF_LOG1_MAC("Encryption enabled: %d \n", encryptionProtect);
    PRV_UTF_LOG0_MAC("==================================================\n");


/********** MACSec initialization *****************************************************************/

    PRV_UTF_LOG0_MAC("==== MACSec initialization start ==== \n");

    /* Convert Data path ID into bits map format */
    dpBmp = (BIT_0 << prvTgfMacSecDpId);

    /* Call MACSec initialization procedure */
    rc = cpssDxChMacSecInit(devNum,dpBmp);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecInit");

    PRV_UTF_LOG0_MAC("==== Set port to MACSec mode ==== \n");

    /* Since all ports after init are set to bypass mode, need to enable the requested port to work in MACSec mode */

    /* First Classifier */

    /* Clear structure before filling them  */
    cpssOsMemSet(&classifyPortCfg, 0, sizeof(classifyPortCfg));

    /* Set to MACSec mode (Egress) */
    classifyPortCfg.bypassMacsecDevice = GT_FALSE;

    /* Disable configuration of exception case. Leave default */
    classifyPortCfg.exceptionCfgEnable = GT_FALSE;

    /* Call CPSS API to set Classifier port with MACSec mode */
    rc = cpssDxChMacSecPortClassifyConfigSet(devNum,
                                             portNum,
                                             CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                             &classifyPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecPortClassifyConfigSet");

    /* Do the same for Ingress */
    rc = cpssDxChMacSecPortClassifyConfigSet(devNum,
                                             portNum,
                                             CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                             &classifyPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecPortClassifyConfigSet");

    /* Now Transformer */
    /* Clear structure before filling it  */
    cpssOsMemSet(&secyPortCfg, 0, sizeof(secyPortCfg));

    /* Get port parameters (Egress) */
    rc = cpssDxChMacSecPortSecyConfigGet(devNum,
                                         portNum,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                         &secyPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecPortSecyConfigGet");

    /* Set to MACSec mode (Egress) */
    secyPortCfg.bypassMacsecDevice = GT_FALSE;

    rc = cpssDxChMacSecPortSecyConfigSet(devNum,
                                         portNum,
                                         CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                         &secyPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecPortSecyConfigSet");

    /* Clear structure before filling it  */
    cpssOsMemSet(&secyPortCfg, 0, sizeof(secyPortCfg));

    /* Get port parameters (Ingress) */
    rc = cpssDxChMacSecPortSecyConfigGet(devNum,
                                         portNum,
                                         CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                         &secyPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecPortSecyConfigGet");

    /* Set to MACSec mode (Ingress) */
    secyPortCfg.bypassMacsecDevice = GT_FALSE;

    rc = cpssDxChMacSecPortSecyConfigSet(devNum,
                                         portNum,
                                         CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                         &secyPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecPortSecyConfigSet");

    PRV_UTF_LOG0_MAC("==== MACSec initialization end ==== \n");


/** Egress Install vPort in Classifier device ******************************************************/

    PRV_UTF_LOG0_MAC("==== vPort add start ==== \n");

    /* Clear structure before filling them  */
    cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

    /* For Egress only !!! Packet expansion indication. 11b = 32 bytes (sectag header + icv) */
    vPortParams.pktExpansion = CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_32B_E;

    /* Egress: SecTAG location relative to the start of the packet. 12d = Standard MACsec (no VLAN tags to be bypassed in clear) */
    vPortParams.secTagOffset = 12;

    /* Clear vPort handle */
    classifyVportHandleEg = 0;

    /* Call CPSS API to add vPort in Classifier unit */
    rc = cpssDxChMacSecClassifyVportAdd(devNum,
                                        dpBmp,
                                        CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                        &vPortParams,
                                        &classifyVportHandleEg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecClassifyVportAdd");

    /* Call CPSS API to get vPort index to use when installing SA */
    rc = cpssDxChMacSecClassifyVportIndexGet(devNum,
                                             classifyVportHandleEg,
                                             &vPortEgPolicyId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecClassifyVportIndexGet");

    /* Ingress */

    /* Clear structure before filling them  */
    cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

    /* Clear vPort handle */
    classifyVportHandleIng = 0;

    /* Call CPSS API to Add vPort in Classifier unit */
    rc = cpssDxChMacSecClassifyVportAdd(devNum,
                                        dpBmp,
                                        CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                        &vPortParams,
                                        &classifyVportHandleIng);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecClassifyVportAdd");

    /* Call CPSS API to get vPort index to use when installing SA */
    rc = cpssDxChMacSecClassifyVportIndexGet(devNum,
                                             classifyVportHandleIng,
                                             &vPortIngPolicyId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecClassifyVportIndexGet");

    PRV_UTF_LOG0_MAC("==== vPort add end ==== \n");


/** Egress Install SA with transform record in Transform device ************************************/

    PRV_UTF_LOG0_MAC("==== SA add start ==== \n");

    /* Set SA parameters */
    cpssOsMemSet(&saParams,0,sizeof(saParams));
    saParams.actionType                         = CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E;
    saParams.destPort                           = CPSS_DXCH_MACSEC_SECY_PORT_COMMON_E;
    saParams.params.egress.protectFrames        = GT_TRUE;
    saParams.params.egress.includeSci           = GT_TRUE;
    saParams.params.egress.confProtect          = (encryptionProtect) ? GT_TRUE : GT_FALSE; /* Set Encryption to on or off */
    saParams.params.egress.controlledPortEnable = GT_TRUE;
    saParams.params.egress.preSecTagAuthLength  = 12;  /* MAC DA,SA */

    /* Set transform record parameters */
    cpssOsMemSet(&trRecParams,0,sizeof(trRecParams));
    trRecParams.an              = 2;
    trRecParams.keyByteCount    = sizeof(K1);
    trRecParams.seqTypeExtended = GT_FALSE;
    trRecParams.seqNumLo        = 0xb2c28464;
    trRecParams.seqNumHi        = 0;
    trRecParams.seqMask         = 0;
    /* Set key */
    for (i=0;i<sizeof(K1);i++)
    {
        trRecParams.keyArr[i] = K1[i];
    }
    /* Set SCI */
    for (i=0;i<CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS;i++)
    {
        trRecParams.sciArr[i] = SCI1[i];
    }

    /* Clear SA handle */
    secySAHandleEg = 0;

    /* Call CPSS API to add SA */
    rc = cpssDxChMacSecSecySaAdd(devNum,
                                 dpBmp,
                                 CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                 vPortEgPolicyId,
                                 &saParams,
                                 &trRecParams,
                                 &secySAHandleEg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecSecySaAdd");

    /* Ingress */

    /* Set SA parameters */
    cpssOsMemSet(&saParams,0,sizeof(saParams));
    saParams.actionType                          = CPSS_DXCH_MACSEC_SECY_SA_ACTION_INGRESS_E;
    saParams.destPort                            = CPSS_DXCH_MACSEC_SECY_PORT_CONTROLLED_E;
    saParams.params.ingress.validateFramesTagged = CPSS_DXCH_MACSEC_SECY_FRAME_VALIDATE_STRICT_E;
    saParams.params.ingress.replayProtect        = GT_TRUE;
    saParams.params.ingress.an                   = *tciAnIngPtr & 3;
    saParams.params.ingress.allowTagged          = GT_TRUE;
    saParams.params.ingress.preSecTagAuthLength  = 12; /* MAC DA,SA */
    /* Set SCI */
    for (i=0; i<CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS; i++)
    {
        saParams.params.ingress.sciArr[i]        = sciIngPtr[i];
    }

    /* Set transform record parameters */
    cpssOsMemSet(&trRecParams,0,sizeof(trRecParams));
    trRecParams.an              = 2;
    trRecParams.keyByteCount    = sizeof(K1);
    trRecParams.seqTypeExtended = GT_FALSE;
    trRecParams.seqNumLo        = 0xb2c28465;
    trRecParams.seqNumHi        = 0;
    trRecParams.seqMask         = 0;
    /* Set key */
    for (i=0;i<sizeof(K1);i++)
    {
        trRecParams.keyArr[i] = K1[i];
    }
    /* Set SCI */
    for (i=0;i<CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS;i++)
    {
        trRecParams.sciArr[i] = SCI1[i];
    }

    /* Clear SA handle */
    secySAHandleIng = 0;

    /* Call CPSS API to add SA */
    rc = cpssDxChMacSecSecySaAdd(devNum,
                                 dpBmp,
                                 CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                 vPortIngPolicyId,
                                 &saParams,&trRecParams,
                                 &secySAHandleIng);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecSecySaAdd");

    PRV_UTF_LOG0_MAC("==== SA add end ==== \n");


/** Egress Install Rule in Classifier device *******************************************************/

    PRV_UTF_LOG0_MAC("==== Rule add start ==== \n");

    /* Set rule parameters */
    cpssOsMemSet(&ruleParams,0,sizeof(ruleParams));
    /* Set mask. Exact match on all these fields. */
    ruleParams.mask.portNum    = 0x3f;
    ruleParams.mask.numTags    = 0x7f;
    /* Set data */
    ruleParams.key.portNum    = portNum;
    ruleParams.key.numTags    = 0x01; /* Bit[0] No VLAN tags */
    /* Set mask. Match on Destination address. */
    ruleParams.dataMask[0]=0xffffffff;
    ruleParams.dataMask[1]=0xffff0000;
    /* Set MAC DA */
    ruleParams.data[0] =  (macDaEgPtr[0]<<24) | (macDaEgPtr[1]<<16) | (macDaEgPtr[2]<<8) | macDaEgPtr[3];
    ruleParams.data[1] =  (macDaEgPtr[4]<<24) | (macDaEgPtr[5]<<16);

    /* Clear rule tag */
    classifyRuleHandleEg = 0;

    /* Call CPSS API to add rule  */
    rc = cpssDxChMacSecClassifyRuleAdd(devNum,
                                       dpBmp,
                                       CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                       classifyVportHandleEg,
                                       &ruleParams,
                                       &classifyRuleHandleEg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecClassifyRuleAdd");

    /* Call CPSS API to enable TCAM rule */
    rc = cpssDxChMacSecClassifyRuleEnable(devNum,
                                          dpBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                          classifyRuleHandleEg,
                                          GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecClassifyRuleEnable");

    /* Ingress */

    /* Set rule parameters */
    cpssOsMemSet(&ruleParams,0,sizeof(ruleParams));

    /* Set mask. Exact match on all these fields. */
    ruleParams.mask.portNum    = 0x3f;
    ruleParams.mask.numTags    = 0x7f;
    /* Set data */
    ruleParams.key.portNum    = portNum;
    ruleParams.key.numTags    = 0x01; /* Bit[0] No VLAN tags */
    /* Set mask. Match on Destination address. */
    ruleParams.dataMask[0]=0xffffffff;
    ruleParams.dataMask[1]=0xffff0000;
    /* Set MAC DA */
    ruleParams.data[0] =  (macDaIngPtr[0]<<24) | (macDaIngPtr[1]<<16) | (macDaIngPtr[2]<<8) | macDaIngPtr[3];
    ruleParams.data[1] =  (macDaIngPtr[4]<<24) | (macDaIngPtr[5]<<16);

    /* Clear rule tag */
    classifyRuleHandleIng = 0;

    /* Call CPSS API to add rule  */
    rc = cpssDxChMacSecClassifyRuleAdd(devNum,
                                       dpBmp,
                                       CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                       classifyVportHandleIng,
                                       &ruleParams,
                                       &classifyRuleHandleIng);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecClassifyRuleAdd");

    /* Call CPSS API to enable TCAM rule */
    rc = cpssDxChMacSecClassifyRuleEnable(devNum,
                                          dpBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                          classifyRuleHandleIng,
                                          GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMacSecClassifyRuleEnable");

    PRV_UTF_LOG0_MAC("==== Rule add end ==== \n");

    return;

}


/**
* @internal prvCpssDxChMacSecEgressExit function
* @endinternal
*
* @brief   MACSec Egress exit sequence
*
* @param[in] devNum            - the device number
*
*/
GT_VOID prvCpssDxChMacSecEgressExit
(
    IN GT_U8       devNum
)
{
    GT_STATUS           rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= MACSec Egress exit sequence =======\n");

    /* Convert Data path ID into bits map format */
    dpBmp = (BIT_0 << prvTgfMacSecDpId);

    /* Disable TCAM rule */
    rc = cpssDxChMacSecClassifyRuleEnable(devNum,
                                          dpBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                          classifyRuleHandleEg,
                                          GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChMacSecClassifyRuleEnable failure");

    /* Remove TCAM rule */
    rc = cpssDxChMacSecClassifyRuleRemove(devNum,
                                          dpBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                          classifyRuleHandleEg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChMacSecClassifyRuleRemove failure");

    /* Remove vPort */
    rc = cpssDxChMacSecClassifyVportRemove(devNum,
                                           dpBmp,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           classifyVportHandleEg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChMacSecClassifyVportRemove failure");

    /* Remove SA */
     rc = cpssDxChMacSecSecySaRemove(devNum,
                                     dpBmp,
                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                     secySAHandleEg);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChMacSecSecySaRemove failure");

     rc = prvCpssMacSecUnInit(devNum, dpBmp);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssMacSecUnInit failure");
}

/**
* @internal prvCpssDxChMacSecEgressIngressExit function
* @endinternal
*
* @brief   MACSec Egress Ingress exit sequence
*
* @param[in] devNum - the device number
*
*/
GT_VOID prvCpssDxChMacSecEgressIngressExit
(
    IN GT_U8       devNum
)
{
    GT_STATUS           rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= MACSec Egress & Ingress exit sequence =======\n");

    /* Convert Data path ID into bits map format */
    dpBmp = (BIT_0 << prvTgfMacSecDpId);

    /* Disable Egress TCAM rule */
    rc = cpssDxChMacSecClassifyRuleEnable(devNum,
                                          dpBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                          classifyRuleHandleEg,
                                          GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChMacSecClassifyRuleEnable failure");

    /* Disable Ingress TCAM rule */
    rc = cpssDxChMacSecClassifyRuleEnable(devNum,
                                          dpBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                          classifyRuleHandleIng,
                                          GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChMacSecClassifyRuleEnable failure");

    /* Remove egress TCAM rule */
    rc = cpssDxChMacSecClassifyRuleRemove(devNum,
                                          dpBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                          classifyRuleHandleEg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChMacSecClassifyRuleRemove failure");

    /* Remove ingress TCAM rule */
    rc = cpssDxChMacSecClassifyRuleRemove(devNum,
                                          dpBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                          classifyRuleHandleIng);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChMacSecClassifyRuleRemove failure");

    /* Remove egress vPort */
    rc = cpssDxChMacSecClassifyVportRemove(devNum,
                                           dpBmp,
                                           CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                           classifyVportHandleEg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChMacSecClassifyVportRemove failure");

    /* Remove ingress vPort */
    rc = cpssDxChMacSecClassifyVportRemove(devNum,
                                           dpBmp,
                                           CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                           classifyVportHandleIng);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChMacSecClassifyVportRemove failure");

    /* Remove egress SA */
     rc = cpssDxChMacSecSecySaRemove(devNum,
                                     dpBmp,
                                     CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                     secySAHandleEg);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChMacSecSecySaRemove failure");

    /* Remove ingress SA */
     rc = cpssDxChMacSecSecySaRemove(devNum,
                                     dpBmp,
                                     CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                     secySAHandleIng);
     UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChMacSecSecySaRemove failure");

    rc = prvCpssMacSecUnInit(devNum, dpBmp);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssMacSecUnInit failure");
}

/**
* @internal prvCpssDxChMacSecClassifierStatisticsGet function
* @endinternal
*
* @brief   Get Egress/Ingress MACSec Classifier statistics and print them out
*          - read TCAM hits counter
*          - read port counters
*
*          note: in prvCpssDxChMacSecEgressConfigSet & prvTgfMacSecEgressIngressConfigSet single rule is added
*                and therefore ruleId is set to 0 when reading statistics
*
* @param[in] devNum      - the device number
* @param[in] dpId        - data path ID  0:DP0, 1:DP1
* @param[in] portNum     - port physical number
* @param[in] direction   - 0:Egress, 1:Ingress
*
* @retval GT_OK          - on success.
* @retval GT_ERROR       - on failure.
**/
GT_STATUS prvCpssDxChMacSecClassifierStatisticsGet
(
    IN GT_U8          devNum,
    IN GT_U8          dpId,
    IN GT_PORT_NUM    portNum,
    IN GT_U32         direction
)
{
    GT_STATUS                                      rc;
    GT_U64                                         statTcamHitsCounter;
    CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC  portStatCounters;


    PRV_UTF_LOG0_MAC("\n======= Input parameters =======================\n");
    PRV_UTF_LOG1_MAC("Device number:      %d \n", devNum);
    PRV_UTF_LOG1_MAC("Data path ID:       %d \n", dpId);
    PRV_UTF_LOG1_MAC("Port number:        %d \n", portNum);
    PRV_UTF_LOG1_MAC("Direction:          %d \n", direction);
    PRV_UTF_LOG0_MAC("==================================================\n");

    /* Convert Data path ID into bits map format */
    dpBmp = (BIT_0 << dpId);

    PRV_UTF_LOG0_MAC("\n\n ======================================= \n");
    PRV_UTF_LOG1_MAC(" === CLASSIFIER STATISTICS [%s] ==== \n",(direction) ? "Ingress" : "Egress");
    PRV_UTF_LOG0_MAC(" ======================================= \n");

    PRV_UTF_LOG0_MAC("\n TCAM HITS STATISTICS\n");

    /* Call CPSS API to read TCAM hits statistics */
    rc = cpssDxChMacSecClassifyStatisticsTcamHitsGet(devNum,
                                                     dpBmp,
                                                     direction,
                                                     0, /* ruleId */
                                                     GT_TRUE,
                                                     &statTcamHitsCounter);
    if(rc != GT_OK)
    {
        return rc;
    }

    PRV_UTF_LOG0_MAC("\nTCAM HITS counters results\n");

    PRV_UTF_LOG2_MAC(" statTcamHitsCounter\n  high[0x%8.8x] , low[0x%8.8x] \n\n", statTcamHitsCounter.l[1],statTcamHitsCounter.l[0]);


    PRV_UTF_LOG1_MAC(" \n\nPORT STATISTICS port:[%d]\n",portNum);

    /* Call CPSS API to read PORT statistics */
    rc = cpssDxChMacSecPortClassifyStatisticsGet(devNum,
                                                 portNum,
                                                 direction,
                                                 GT_TRUE,
                                                 &portStatCounters);
    if(rc != GT_OK)
    {
        return rc;
    }

    PRV_UTF_LOG1_MAC(" \nPORT counters results\n",portNum);

    PRV_UTF_LOG2_MAC(" tcamHitMultiple:         high[0x%8.8x] , low[0x%8.8x] \n"
                     ,portStatCounters.tcamHitMultiple.l[1],portStatCounters.tcamHitMultiple.l[0]);
    PRV_UTF_LOG2_MAC(" headerParserDroppedPkts: high[0x%8.8x] , low[0x%8.8x] \n"
                     ,portStatCounters.headerParserDroppedPkts.l[1],portStatCounters.headerParserDroppedPkts.l[0]);
    PRV_UTF_LOG2_MAC(" tcamMiss:                high[0x%8.8x] , low[0x%8.8x] \n"
                     ,portStatCounters.tcamMiss.l[1],portStatCounters.tcamMiss.l[0]);
    PRV_UTF_LOG2_MAC(" pktsCtrl:                high[0x%8.8x] , low[0x%8.8x] \n"
                     ,portStatCounters.pktsCtrl.l[1],portStatCounters.pktsCtrl.l[0]);
    PRV_UTF_LOG2_MAC(" pktsData:                high[0x%8.8x] , low[0x%8.8x] \n"
                     ,portStatCounters.pktsData.l[1],portStatCounters.pktsData.l[0]);
    PRV_UTF_LOG2_MAC(" pktsDropped:             high[0x%8.8x] , low[0x%8.8x] \n"
                     ,portStatCounters.pktsDropped.l[1],portStatCounters.pktsDropped.l[0]);
    PRV_UTF_LOG2_MAC(" pktsErrIn:               high[0x%8.8x] , low[0x%8.8x] \n"
                     ,portStatCounters.pktsErrIn.l[1],portStatCounters.pktsErrIn.l[0]);

    return GT_OK;
}

/**
* @internal prvCpssDxChMacSecTransformerStatisticsGet function
* @endinternal
*
* @brief   Get Egress/Ingress MACSec Transformer statistics and print them out
*          - read SA counters
*          - read SecY counters
*          - read IFC counters
*          - read RX CAM counter
*
*          note: in prvCpssDxChMacSecEgressConfigSet & prvTgfMacSecEgressIngressConfigSet single vPort and SA
*                are added and therefore vPortId and scIndex are set to 0 when reading statistics
*
* @param[in] devNum      - the device number
* @param[in] dpId        - data path ID  0:DP0, 1:DP1
* @param[in] portNum     - port physical number
* @param[in] direction   - 0:Egress, 1:Ingress
*
* @retval GT_OK                - on success.
* @retval GT_ERROR             - on failure.
**/
GT_STATUS prvCpssDxChMacSecTransformerStatisticsGet
(
    IN GT_U8        devNum,
    IN GT_U8        dpId,
    IN GT_PORT_NUM  portNum,
    IN GT_U32       direction
)
{
    GT_STATUS                             rc;
    CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT     saStatCounters;
    CPSS_DXCH_MACSEC_SECY_SA_HANDLE       secySAHandle;
    CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT   secyStatCounters;
    CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT    ifcStatCounter;
    GT_U64                                rxCamStatCounter;


    PRV_UTF_LOG0_MAC("\n======= Input parameters =======================\n");
    PRV_UTF_LOG1_MAC("Device number:      %d \n", devNum);
    PRV_UTF_LOG1_MAC("Data path ID:       %d \n", dpId);
    PRV_UTF_LOG1_MAC("Port number:        %d \n", portNum);
    PRV_UTF_LOG1_MAC("Direction:          %d \n", direction);
    PRV_UTF_LOG0_MAC("==================================================\n");


    /* Convert Data path ID into bits map format */
    dpBmp = (BIT_0 << dpId);

    PRV_UTF_LOG0_MAC("\n\n ======================================== \n");
    PRV_UTF_LOG1_MAC(" === TRANSFORMER STATISTICS [%s] ==== \n",(direction) ? "Ingress" : "Egress");
    PRV_UTF_LOG0_MAC(" ======================================== \n");


    PRV_UTF_LOG0_MAC("\n SA STATISTICS \n");

    if (direction == CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E)
    {
        secySAHandle = secySAHandleEg;
    }
    else
    {
        secySAHandle = secySAHandleIng;
    }

    /* Call CPSS API to read SA statistics */
    rc = cpssDxChMacSecSecyStatisticsSaGet(devNum,
                                           dpBmp,
                                           direction,
                                           secySAHandle,
                                           GT_TRUE,
                                           &saStatCounters);
    if(rc != GT_OK)
    {
        return rc;
    }

    PRV_UTF_LOG0_MAC("\n SA counters results \n");

    if (direction == CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E)
    {
        PRV_UTF_LOG2_MAC(" outOctetsEncryptedProtected: high[0x%8.8x] , low[0x%8.8x] \n"
                         ,saStatCounters.egress.outOctetsEncryptedProtected.l[1],saStatCounters.egress.outOctetsEncryptedProtected.l[0]);

        PRV_UTF_LOG2_MAC(" outPktsEncryptedProtected:   high[0x%8.8x] , low[0x%8.8x] \n"
                         ,saStatCounters.egress.outPktsEncryptedProtected.l[1],saStatCounters.egress.outPktsEncryptedProtected.l[0]);

        PRV_UTF_LOG2_MAC(" outPktsSANotInUse:           high[0x%8.8x] , low[0x%8.8x] \n"
                         ,saStatCounters.egress.outPktsSANotInUse.l[1],saStatCounters.egress.outPktsSANotInUse.l[0]);

        PRV_UTF_LOG2_MAC(" outPktsTooLong:              high[0x%8.8x] , low[0x%8.8x] \n"
                         ,saStatCounters.egress.outPktsTooLong.l[1],saStatCounters.egress.outPktsTooLong.l[0]);
    }
    else
    {
        PRV_UTF_LOG2_MAC(" inOctetsDecrypted:           high[0x%8.8x] , low[0x%8.8x] \n"
                         ,saStatCounters.ingress.inOctetsDecrypted.l[1],saStatCounters.ingress.inOctetsDecrypted.l[0]);

        PRV_UTF_LOG2_MAC(" inOctetsValidated:           high[0x%8.8x] , low[0x%8.8x] \n"
                         ,saStatCounters.ingress.inOctetsValidated.l[1],saStatCounters.ingress.inOctetsValidated.l[0]);

        PRV_UTF_LOG2_MAC(" inPktsDelayed:               high[0x%8.8x] , low[0x%8.8x] \n"
                         ,saStatCounters.ingress.inPktsDelayed.l[1],saStatCounters.ingress.inPktsDelayed.l[0]);

        PRV_UTF_LOG2_MAC(" inPktsInvalid:               high[0x%8.8x] , low[0x%8.8x] \n"
                         ,saStatCounters.ingress.inPktsInvalid.l[1],saStatCounters.ingress.inPktsInvalid.l[0]);

        PRV_UTF_LOG2_MAC(" inPktsLate:                  high[0x%8.8x] , low[0x%8.8x] \n"
                         ,saStatCounters.ingress.inPktsLate.l[1],saStatCounters.ingress.inPktsLate.l[0]);

        PRV_UTF_LOG2_MAC(" inPktsNotUsingSA:            high[0x%8.8x] , low[0x%8.8x] \n"
                         ,saStatCounters.ingress.inPktsNotUsingSA.l[1],saStatCounters.ingress.inPktsNotUsingSA.l[0]);

        PRV_UTF_LOG2_MAC(" inPktsNotValid:              high[0x%8.8x] , low[0x%8.8x] \n"
                         ,saStatCounters.ingress.inPktsNotValid.l[1],saStatCounters.ingress.inPktsNotValid.l[0]);

        PRV_UTF_LOG2_MAC(" inPktsOK:                    high[0x%8.8x] , low[0x%8.8x] \n"
                         ,saStatCounters.ingress.inPktsOK.l[1],saStatCounters.ingress.inPktsOK.l[0]);

        PRV_UTF_LOG2_MAC(" inPktsUnchecked:             high[0x%8.8x] , low[0x%8.8x] \n"
                         ,saStatCounters.ingress.inPktsUnchecked.l[1],saStatCounters.ingress.inPktsUnchecked.l[0]);

        PRV_UTF_LOG2_MAC(" inPktsUnusedSA:              high[0x%8.8x] , low[0x%8.8x] \n"
                         ,saStatCounters.ingress.inPktsUnusedSA.l[1],saStatCounters.ingress.inPktsUnusedSA.l[0]);
     }


     PRV_UTF_LOG0_MAC("\n\n SecY STATISTICS \n");

     /* Call CPSS API to read SecY statistics */
     rc = cpssDxChMacSecSecyStatisticsSecyGet(devNum,
                                              dpBmp,
                                              direction,
                                              0, /* vPortId */
                                              GT_TRUE,
                                              &secyStatCounters);
     if(rc != GT_OK)
     {
         return rc;
     }

     PRV_UTF_LOG0_MAC("\n SecY counters results \n");

     if (direction == CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E)
     {
         PRV_UTF_LOG2_MAC(" outPktsControl:        high[0x%8.8x] , low[0x%8.8x] \n"
                          ,secyStatCounters.egress.outPktsControl.l[1],secyStatCounters.egress.outPktsControl.l[0]);

         PRV_UTF_LOG2_MAC(" outPktsTransformError: high[0x%8.8x] , low[0x%8.8x] \n"
                          ,secyStatCounters.egress.outPktsTransformError.l[1],secyStatCounters.egress.outPktsTransformError.l[0]);

         PRV_UTF_LOG2_MAC(" outPktsUntagged:       high[0x%8.8x] , low[0x%8.8x] \n"
                          ,secyStatCounters.egress.outPktsUntagged.l[1],secyStatCounters.egress.outPktsUntagged.l[0]);
     }
     else
     {
         PRV_UTF_LOG2_MAC(" inPktsBadTag:         high[0x%8.8x] , low[0x%8.8x] \n"
                          ,secyStatCounters.ingress.inPktsBadTag.l[1],secyStatCounters.ingress.inPktsBadTag.l[0]);

         PRV_UTF_LOG2_MAC(" inPktsControl:        high[0x%8.8x] , low[0x%8.8x] \n"
                          ,secyStatCounters.ingress.inPktsControl.l[1],secyStatCounters.ingress.inPktsControl.l[0]);

         PRV_UTF_LOG2_MAC(" inPktsNoSCI:          high[0x%8.8x] , low[0x%8.8x] \n"
                          ,secyStatCounters.ingress.inPktsNoSCI.l[1],secyStatCounters.ingress.inPktsNoSCI.l[0]);

         PRV_UTF_LOG2_MAC(" inPktsNoTag:          high[0x%8.8x] , low[0x%8.8x] \n"
                          ,secyStatCounters.ingress.inPktsNoTag.l[1],secyStatCounters.ingress.inPktsNoTag.l[0]);

         PRV_UTF_LOG2_MAC(" inPktsTaggedCtrl:     high[0x%8.8x] , low[0x%8.8x] \n"
                          ,secyStatCounters.ingress.inPktsTaggedCtrl.l[1],secyStatCounters.ingress.inPktsTaggedCtrl.l[0]);

         PRV_UTF_LOG2_MAC(" inPktsTransformError: high[0x%8.8x] , low[0x%8.8x] \n"
                          ,secyStatCounters.ingress.inPktsTransformError.l[1],secyStatCounters.ingress.inPktsTransformError.l[0]);

         PRV_UTF_LOG2_MAC(" inPktsUnknownSCI:     high[0x%8.8x] , low[0x%8.8x] \n"
                          ,secyStatCounters.ingress.inPktsUnknownSCI.l[1],secyStatCounters.ingress.inPktsUnknownSCI.l[0]);

         PRV_UTF_LOG2_MAC(" inPktsUntagged:       high[0x%8.8x] , low[0x%8.8x] \n"
                          ,secyStatCounters.ingress.inPktsUntagged.l[1],secyStatCounters.ingress.inPktsUntagged.l[0]);
      }



     PRV_UTF_LOG0_MAC("\n\n IFC STATISTICS \n");

     /* Call CPSS API to read IFC statistics */
      rc = cpssDxChMacSecSecyStatisticsIfcGet(devNum,
                                              dpBmp,
                                              direction,
                                              0, /* vPortId */
                                              GT_TRUE,
                                              &ifcStatCounter);

     PRV_UTF_LOG0_MAC("\n IFC counters results \n");

     if (direction == CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E)
     {
         PRV_UTF_LOG2_MAC(" outOctetsCommon:              high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.egress.outOctetsCommon.l[1],ifcStatCounter.egress.outOctetsCommon.l[0]);

         PRV_UTF_LOG2_MAC(" outOctetsControlled:          high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.egress.outOctetsControlled.l[1],ifcStatCounter.egress.outOctetsControlled.l[0]);

         PRV_UTF_LOG2_MAC(" outOctetsUncontrolled:        high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.egress.outOctetsUncontrolled.l[1],ifcStatCounter.egress.outOctetsUncontrolled.l[0]);

         PRV_UTF_LOG2_MAC(" outPktsBroadcastControlled:   high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.egress.outPktsBroadcastControlled.l[1],ifcStatCounter.egress.outPktsBroadcastControlled.l[0]);

         PRV_UTF_LOG2_MAC(" outPktsBroadcastUncontrolled: high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.egress.outPktsBroadcastUncontrolled.l[1],ifcStatCounter.egress.outPktsBroadcastUncontrolled.l[0]);

         PRV_UTF_LOG2_MAC(" outPktsMulticastControlled:   high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.egress.outPktsMulticastControlled.l[1],ifcStatCounter.egress.outPktsMulticastControlled.l[0]);

         PRV_UTF_LOG2_MAC(" outPktsMulticastUncontrolled: high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.egress.outPktsMulticastUncontrolled.l[1],ifcStatCounter.egress.outPktsMulticastUncontrolled.l[0]);

         PRV_UTF_LOG2_MAC(" outPktsUnicastControlled:     high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.egress.outPktsUnicastControlled.l[1],ifcStatCounter.egress.outPktsUnicastControlled.l[0]);

         PRV_UTF_LOG2_MAC(" outPktsUnicastUncontrolled:   high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.egress.outPktsUnicastUncontrolled.l[1],ifcStatCounter.egress.outPktsUnicastUncontrolled.l[0]);

     }
     else
     {
         PRV_UTF_LOG2_MAC(" inOctetsControlled:          high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.ingress.inOctetsControlled.l[1],ifcStatCounter.ingress.inOctetsControlled.l[0]);

         PRV_UTF_LOG2_MAC(" inOctetsUncontrolled:        high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.ingress.inOctetsUncontrolled.l[1],ifcStatCounter.ingress.inOctetsUncontrolled.l[0]);

         PRV_UTF_LOG2_MAC(" inPktsBroadcastControlled:   high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.ingress.inPktsBroadcastControlled.l[1],ifcStatCounter.ingress.inPktsBroadcastControlled.l[0]);

         PRV_UTF_LOG2_MAC(" inPktsBroadcastUncontrolled: high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.ingress.inPktsBroadcastUncontrolled.l[1],ifcStatCounter.ingress.inPktsBroadcastUncontrolled.l[0]);

         PRV_UTF_LOG2_MAC(" inPktsMulticastControlled:   high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.ingress.inPktsMulticastControlled.l[1],ifcStatCounter.ingress.inPktsMulticastControlled.l[0]);

         PRV_UTF_LOG2_MAC(" inPktsMulticastUncontrolled: high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.ingress.inPktsMulticastUncontrolled.l[1],ifcStatCounter.ingress.inPktsMulticastUncontrolled.l[0]);

         PRV_UTF_LOG2_MAC(" inPktsUnicastControlled:     high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.ingress.inPktsUnicastControlled.l[1],ifcStatCounter.ingress.inPktsUnicastControlled.l[0]);

         PRV_UTF_LOG2_MAC(" inPktsUnicastUncontrolled:   high[0x%8.8x] , low[0x%8.8x] \n"
                          ,ifcStatCounter.ingress.inPktsUnicastUncontrolled.l[1],ifcStatCounter.ingress.inPktsUnicastUncontrolled.l[0]);

      }



     if (direction == CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E)
     {
         PRV_UTF_LOG0_MAC("\n\n RX CAM STATISTICS \n");

         /* Call CPSS API to read IFC statistics */
         rc = cpssDxChMacSecSecyStatisticsRxCamGet(devNum,
                                                   dpBmp,
                                                   0,/* scIndex */
                                                   GT_TRUE,
                                                   &rxCamStatCounter);

         PRV_UTF_LOG0_MAC("\n\n RX CAM counters results \n");

         PRV_UTF_LOG2_MAC(" rxCamStatCounter: high[0x%8.8x] , low[0x%8.8x] \n", rxCamStatCounter.l[1], rxCamStatCounter.l[0]);
     }

    return GT_OK;
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
    GT_U8         strNameBuffer[120] = "";

    PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(eventPtr->location.ramEntryInfo.memType, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT, strNameBuffer);
    /* Print debug info */
    PRV_UTF_LOG0_MAC("======DATA INTEGRITY ERROR EVENT======\n");
    PRV_UTF_LOG2_MAC("Device     : %d - %d\n", dev, prvTgfDevNum);
    PRV_UTF_LOG1_MAC("Event type : %d\n", eventPtr->eventsType);
    PRV_UTF_LOG1_MAC("DFX RAM    : %s\n", strNameBuffer);
    PRV_UTF_LOG6_MAC("Memory     : %d {%d, %d, %d}, line # %d, hwTableType[%d]\n",
                     eventPtr->location.ramEntryInfo.memType,
                     eventPtr->location.ramEntryInfo.memLocation.dfxPipeId,
                     eventPtr->location.ramEntryInfo.memLocation.dfxClientId,
                     eventPtr->location.ramEntryInfo.memLocation.dfxMemoryId,
                     eventPtr->location.ramEntryInfo.ramRow,
                     eventPtr->location.hwEntryInfo.hwTableType);

    if((dev == prvTgfDevNum) &&
       (eventPtr->location.ramEntryInfo.memType >= CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP164_ING_IBUF_E &&
        eventPtr->location.ramEntryInfo.memType <= CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP163_ING_TPOL_RAM_E))
    {
        evErrorCtr++;
    }

    PRV_UTF_LOG1_MAC("Event error counter : %d\n", evErrorCtr);
    return GT_OK;
}

/**
* @internal prvTgfMacSecDiEventsTest function
* @endinternal
*
* @brief   MACSec DI events use case test.
*/
GT_VOID prvTgfMacSecDiEventsTest
(
    GT_VOID
)
{
    GT_STATUS                                       rc;
    GT_U8                                           portIter;
    GT_U32                                          packetCnt;
    CPSS_PORT_MAC_COUNTER_SET_STC                   portCntrs;/* current counters of the port */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            dfxMemLocationInfo; /* RAM location */
    CPSS_DXCH_LOCATION_FULL_INFO_STC                locationFull;
    static GT_U32                                   arrSize = 1;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT protectType;
    GT_U8                                           strNameBuffer[120] = "";
    GT_U32                                          memIter;
    GT_BOOL                                         macSecRamConfig[50] = {GT_FALSE};
    GT_U32                                          macSecMemIterMax;

    PRV_UTF_LOG0_MAC("======= MACSec RAM DI test start =======\n");

    /* AUTODOC: setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfMacSecPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfSetTxSetupEth");

    /* AUTODOC: Save DI error callback and set own */
    savedDataIntegrityErrorCb = dxChDataIntegrityEventIncrementFunc;
    dxChDataIntegrityEventIncrementFunc = dataIntegrityErrorHandler;

    cpssOsMemSet(&dfxMemLocationInfo, 0, sizeof(dfxMemLocationInfo));
    cpssOsMemSet(&locationFull, 0, sizeof(locationFull));

    memIter = 0;
    macSecMemIterMax = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP163_ING_TPOL_RAM_E - CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP164_ING_IBUF_E;
    for (memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP164_ING_IBUF_E;
         memType <= CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP163_ING_TPOL_RAM_E;
        memType++,memIter++)
    {
        PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(memType, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT, strNameBuffer);
        locationFull.ramEntryInfo.memType = memType;
        locationFull.portGroupsBmp        = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* fill DFX pipe, client, memory according to memType. */
        rc = prvCpssDxChDiagDataIntegrityMemoryIndexesGet(prvTgfDevNum, locationFull.ramEntryInfo.memType, GT_FALSE, 0 /* ignored */, &arrSize,
                                                          &locationFull.ramEntryInfo.memLocation, &protectType);
        if (GT_NOT_FOUND == rc)
        {
            /* RAM is not found in DataIntegrity DB */
            continue;
        }

        /* GT_BAD_SIZE is not error. We intentionally specified arrSize==1
           to get only first RAM of specified type from DB */
        if (rc != GT_OK && rc != GT_BAD_SIZE)
        {
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, prvTgfDevNum, memType, strNameBuffer);
        }

        dfxMemLocationInfo.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
        dfxMemLocationInfo.info.ramEntryInfo.memType = memType;

        /* Table found in DFX data base - unmask interrupt for DFX memory */
        rc = cpssDxChDiagDataIntegrityEventMaskSet(prvTgfDevNum, &dfxMemLocationInfo, CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E, CPSS_EVENT_UNMASK_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* Enable error injection */
        rc = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(prvTgfDevNum, &dfxMemLocationInfo, CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E, GT_TRUE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        if(memIter <= macSecMemIterMax)
        {
            macSecRamConfig[memIter] = GT_TRUE;
        }

    }

    /* Configure MacSec */
    prvTgfMacSecEgressIngressConfigSet();

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    for(packetCnt = 0; packetCnt < prvTgfPktCnt; packetCnt++)
    {
        /* Do not print packets */
        tgfTrafficTracePacketByteSet(GT_FALSE);

        /* AUTODOC: send Packet from Port 0 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in prvTgfStartTransmitingEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort]);

        /* check counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: Wait for events */
    cpssOsTimerWkAfter(100);

    /* get DI events counter */
    rc = (evErrorCtr >= 1) ? GT_OK : GT_FAIL;
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "DI events count [%d] different than expected [%d] for RAM [%s]\n", evErrorCtr, 1, strNameBuffer);
    evErrorCtr = 0;

    memIter = 0;
    for (memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP164_ING_IBUF_E; memType <= CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP163_ING_TPOL_RAM_E;
        memType++,memIter++)
    {
        if(macSecRamConfig[memIter] == GT_TRUE)
        {
            /* Disable error injection */
            rc = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(prvTgfDevNum, &dfxMemLocationInfo, CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E, GT_FALSE);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }
    }

    /* AUTODOC: Restore data integrity error handler */
    dxChDataIntegrityEventIncrementFunc = savedDataIntegrityErrorCb;
}

/**
* @internal prvTgfMacSecDiEventsRestore function
* @endinternal
*
* @brief   MACSec DI events restore configurations.
*/
GT_VOID prvTgfMacSecDiEventsRestore
(
    GT_VOID
)
{
    /* UnConfigure MacSec */
    prvTgfMacSecEgressIngressConfigRestore();
}
