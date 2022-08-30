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
* @file prvTgfOamDelayMeasurement.c
*
* @brief OAM Delay Measurement features testing
*
* @version   9
********************************************************************************
*/
#include <oam/prvTgfOamDelayMeasurement.h>
#include <cpssCommon/private/prvCpssMisc.h>
#include <ptp/prvTgfPtpGen.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* OAM EPCL Lookup Configuration */
extern PRV_TGF_OAM_EPCL_RULE_CONFIG_STC oamEPclRuleInfo[];

/* OAM packet command profile */
#define PRV_TGF_OAM_PACKET_CMD_PROFILE_STC              5
/* EPCL configuration index for send/recieve port of CPU0 */
#define PRV_TGF_OAM_EPCL_DM_RULE_CPU0_PORT_INDEX_STC    0
/* EPCL configuration index for send/recieve port of CPU1 */
#define PRV_TGF_OAM_EPCL_DM_RULE_CPU1_PORT_INDEX_STC    2

/* EPCL configuration index for send/recieve port of PP01 */
#define PRV_TGF_OAM_EPCL_DM_RULE_PP1_PORT_INDEX_STC     1
/* EPCL configuration index for send/recieve port of PP0 */
#define PRV_TGF_OAM_EPCL_DM_RULE_PP0_PORT_INDEX_STC     3

extern TGF_PACKET_STC * capturePacketsArray[];

/* Burst packets number */
extern GT_U32 prvTgfOamBurstCount;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal tgfOamDelayMeasurementTest function
* @endinternal
*
* @brief   Delay Measurement Verification
*/
GT_VOID tgfOamDelayMeasurementTest
(
    GT_VOID
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          oamIndex;           /* OAM entry index */
    GT_U32          opCodeIndex;        /* Opcode index */
    GT_U32          opCode;             /* Opcode value */
    PRV_TGF_PTP_TS_CFG_ENTRY_STC timeStampCfgEntry; /* TS configuiration table structure */
    PRV_TGF_OAM_COMMON_CONFIG_STC oamConfig; /* OAM configuration */
    PRV_TGF_OAM_PDU_HEADER_DATA_STC pduHeaderData; /* PDU header data */
    GT_U32          secondsDelay;       /* delay in seconds */
    GT_U64          txTimeStampf;       /* Tx timestamp */
    GT_U64          rxTimeb;            /* Rx timestamp */
    GT_U32          secondsFromTimestamps; /* seconds from timestamp */
    GT_U8           *tlvEntryPtr;       /* pointer to PDU TLV data */
    GT_U32          nanoPerMilliSec; /* TAI nano-seconds in CPU mili-second */
    PRV_CPSS_TIMESTAMP_STC pp0RxTs;     /*PP0 RX timestamp retrieved from packet */
    PRV_CPSS_TIMESTAMP_STC pp0TxTs;     /*PP0 TX timestamp retrieved from packet */
    PRV_CPSS_TIMESTAMP_STC pp1RxTs;     /*PP1 RX timestamp retrieved from packet */
    PRV_CPSS_TIMESTAMP_STC pp1TxTs;     /*PP1 TX timestamp retrieved from packet */

    cpssOsMemSet(&timeStampCfgEntry, 0, sizeof(timeStampCfgEntry));
    cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));
    cpssOsMemSet(&pduHeaderData, 0, sizeof(pduHeaderData));

    /* AUTODOC: Set DMM/DMR Opcodes */
    for(opCodeIndex = 0; opCodeIndex < 3; opCodeIndex++)
    {
        switch(opCodeIndex)
        {
            case 0:
                /* Set OAM DM0 Opcode */
                opCode = PRV_TGF_DM_OPCODE_BYTE_45_CNS;
                break;
            case 1:
                /* Set OAM DM1 Opcode */
                opCode = PRV_TGF_DM_OPCODE_BYTE_46_CNS;
                break;
            default:
                /* Set OAM DM2 Opcode */
                opCode = PRV_TGF_DM_OPCODE_BYTE_47_CNS;
                break;
        }

        rc = prvTgfOamOpcodeSet(
                    PRV_TGF_OAM_STAGE_TYPE_EGRESS_E,
                    PRV_TGF_OAM_OPCODE_TYPE_DM_E,
                    opCodeIndex, opCode);

        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc,
                    "prvTgfOamOpcodeSet");
    }

    /* AUTODOC: Set EOAM configuration and OAM timestamp table */
    for(oamIndex = 0; oamIndex < PRV_TGF_EOAM_DM_PCL_RULE_NUM_STC; oamIndex++)
    {
        oamConfig.oamEntryIndex = PRV_EOAM_DM_ENTRY_START_INDEX_CNS + oamIndex;
        /* Get OAM common configuration */
        prvTgfOamConfigurationGet(&oamConfig);

        oamConfig.oamEntry.opcodeParsingEnable = GT_FALSE;
        oamConfig.oamEntry.timestampEnable = GT_TRUE;
        oamConfig.oamEntry.oamPtpOffsetIndex = oamIndex;

        /* Set common OAM configurations */
        prvTgfOamConfigurationSet(&oamConfig);

        timeStampCfgEntry.packetFormat = PRV_TGF_PTP_TS_PACKET_TYPE_Y1731_E;
        timeStampCfgEntry.tsMode = PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E;
        timeStampCfgEntry.tsAction = PRV_TGF_PTP_TS_ACTION_ADD_TIME_E;
        timeStampCfgEntry.offset =
            PRV_TGF_OAM_TLV_DATA_OFFSET_STC + 8 * oamIndex;
        /* Set OAM Timestamp table */
        prvTgfPtpTsCfgTableSet(oamIndex, &timeStampCfgEntry);
    }

    /* AUTODOC: Set IOAM configuration */
    for(oamIndex = 0; oamIndex < PRV_TGF_IOAM_PCL_RULE_NUM_STC; oamIndex++)
    {

        oamConfig.oamEntryIndex = PRV_IOAM_ENTRY_START_INDEX_CNS + oamIndex;
        /* Get OAM common configuration */
        prvTgfOamConfigurationGet(&oamConfig);

        /* Enable Opcode parsing */
        oamConfig.oamEntry.opcodeParsingEnable = GT_TRUE;
        oamConfig.oamEntry.packetCommandProfile =
                    PRV_TGF_OAM_PACKET_CMD_PROFILE_STC;

        /* Set common OAM configurations */
        prvTgfOamConfigurationSet(&oamConfig);
    }

    /* AUTODOC: Set DMM Opcode/Profile Packet Command for IOAM packets */
    rc = prvTgfOamOpcodeProfilePacketCommandEntrySet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    oamEPclRuleInfo[PRV_TGF_OAM_EPCL_DM_RULE_PP1_PORT_INDEX_STC].pduHeaderByteValue,
                    PRV_TGF_OAM_PACKET_CMD_PROFILE_STC,
                    CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                    2);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc,
                    "prvTgfOamOpcodeProfilePacketCommandEntrySet");

    /* AUTODOC: Set DMR Opcode/Profile Packet Command for IOAM packets */
    rc = prvTgfOamOpcodeProfilePacketCommandEntrySet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    oamEPclRuleInfo[PRV_TGF_OAM_EPCL_DM_RULE_PP0_PORT_INDEX_STC].pduHeaderByteValue,
                    PRV_TGF_OAM_PACKET_CMD_PROFILE_STC,
                    CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                    2);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc,
                    "prvTgfOamOpcodeProfilePacketCommandEntrySet");

    /* AUTODOC: Set OAM CPU code base fro DMM/DMR packets trapped to CPU */
    rc = prvTgfOamCpuCodeBaseSet(
                    PRV_TGF_OAM_STAGE_TYPE_INGRESS_E,
                    CPSS_NET_FIRST_USER_DEFINED_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamCpuCodeBaseSet");

    /* AUTODOC: Phase 1: Send DMM packet from "CPU0" to "CPU1" */

    /* AUTODOC: Set MEL = 3 */
    pduHeaderData.megLevel = 3;
    /* Set TLV offset */
    pduHeaderData.tlvOffset = 32;

    /* AUTODOC: Set DMM OAM Opcode - CPU0*/
    pduHeaderData.opcode = PRV_TGF_DM_OPCODE_BYTE_45_CNS;

    /* AUTODOC: Set OAM opcode into source MAC address for EPCL rule cath */
    prvTgfOamPacketHeaderSrcMacByteSet(
                    PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                    pduHeaderData.opcode);

    prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);

    /* AUTODOC: Check results - Trapped to CPU */
    prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_LAST_E,
                    0, /* Number of sent packets */
                    prvTgfOamBurstCount, /* Packets trapped to CPU */
                    prvTgfOamBurstCount, /* Exceptions */
                    0, /* Billing counters */
                    GT_TRUE /*usePortGroupsBmp*/);

    /* AUTODOC: Parse from CPU0 OAM PDU header - DMM */
    prvTgfOamRxPduHeaderParse(capturePacketsArray[0], &pduHeaderData);
    UTF_VERIFY_EQUAL2_STRING_MAC(
                    3, pduHeaderData.megLevel, "Form CPU0 MEL Error: %d, &d",
                    prvTgfDevsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(
                    PRV_TGF_DM_OPCODE_BYTE_45_CNS, pduHeaderData.opcode, "From CPU0 Opcode Error: %d, &d",
                    prvTgfDevsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS]);

    /* AUTODOC: Swap MAC addresses */
    prvTgfOamPacketHeaderMacSwap();

    /* AUTODOC: Phase 2: Send DMR packet from "CPU1" to "CPU0" */
    secondsDelay = 2;
    /* Wait 2 seconds */
    cpssOsTimerWkAfter(secondsDelay * 1000);

    /* AUTODOC: Set DMR OAM Opcode - "CPU1" */
    pduHeaderData.opcode = PRV_TGF_DM_OPCODE_BYTE_46_CNS;
    /* AUTODOC: Set source mac address byte */
    prvTgfOamPacketHeaderSrcMacByteSet(
                    PRV_TGF_OAM_SRC_MAC_BYTE_INDEX_STC,
                    pduHeaderData.opcode);

    prvTgfOamTransmitPacket(
                    &pduHeaderData,
                    PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS,
                    PRV_TGF_OAM_BURST_COUNT_CNS,
                    PRV_TGF_OAM_NO_DELAY_CNS);


    /* AUTODOC: Check results - Trapped to CPU */
    prvTgfDefOamGeneralTrafficCheck(
                    PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS,
                    PRV_TGF_OAM_EXCEPTION_TYPE_LAST_E,
                    0, /* Number of sent packets */
                    prvTgfOamBurstCount, /* Packets trapped to CPU */
                    prvTgfOamBurstCount, /* Exceptions */
                    0, /* Billing counters */
                    GT_TRUE /*usePortGroupsBmp*/);

    /* AUTODOC: Parse from "CPU1" OAM PDU header - DMM */
    prvTgfOamRxPduHeaderParse(capturePacketsArray[0], &pduHeaderData);
    UTF_VERIFY_EQUAL2_STRING_MAC(
                    3, pduHeaderData.megLevel, "Form CPU1 MEL Error: %d, &d",
                    prvTgfDevsArray[PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(
                    PRV_TGF_DM_OPCODE_BYTE_46_CNS, pduHeaderData.opcode, "From CPU1 Opcode Error: %d, &d",
                    prvTgfDevsArray[PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS],
                    prvTgfPortsArray[PRV_TGF_FROM_CPU1_EGRESS_PORT_IDX_CNS]);

    tlvEntryPtr = &pduHeaderData.tlvDataArr[0];

    /* AUTODOC: Timestamp PP0 Tx (seconds) offset - 0  */
    rc = prvCpssTimesampLoad(
        "SSSSNNNN"/*typestampFormat*/, (const char*)&(tlvEntryPtr[0]), &pp0TxTs);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvCpssTimesampLoad");

    /* AUTODOC: Timestamp PP1 Rx (seconds) offset - 8  */
    rc = prvCpssTimesampLoad(
        "SSSSNNNN"/*typestampFormat*/, (const char*)&(tlvEntryPtr[8]), &pp1RxTs);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvCpssTimesampLoad");

    /* AUTODOC: Timestamp PP1 Tx (seconds) offset - 16 */
    rc = prvCpssTimesampLoad(
        "SSSSNNNN"/*typestampFormat*/, (const char*)&(tlvEntryPtr[16]), &pp1TxTs);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvCpssTimesampLoad");

    /* AUTODOC: Timestamp PP0 Rx (seconds) offset - 24 */
    rc = prvCpssTimesampLoad(
        "SSSSNNNN"/*typestampFormat*/, (const char*)&(tlvEntryPtr[24]), &pp0RxTs);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvCpssTimesampLoad");

    PRV_UTF_LOG3_MAC(
        "pp0RxTs: secondsHigh %d secondsLow %d nanoseconds %d\n",
        pp0RxTs.secondsHigh, pp0RxTs.secondsLow, pp0RxTs.nanoseconds);
    PRV_UTF_LOG3_MAC(
        "pp0TxTs: secondsHigh %d secondsLow %d nanoseconds %d\n",
        pp0TxTs.secondsHigh, pp0TxTs.secondsLow, pp0TxTs.nanoseconds);
    PRV_UTF_LOG3_MAC(
        "pp1RxTs: secondsHigh %d secondsLow %d nanoseconds %d\n",
        pp1RxTs.secondsHigh, pp1RxTs.secondsLow, pp1RxTs.nanoseconds);
    PRV_UTF_LOG3_MAC(
        "pp1TxTs: secondsHigh %d secondsLow %d nanoseconds %d\n",
        pp1TxTs.secondsHigh, pp1TxTs.secondsLow, pp1TxTs.nanoseconds);

    /* the TAI inits shows time 2000 times slower then CPU System time */
    /* below timestamps from packet scaled to CPU System time units    */
    if (cpssDeviceRunCheck_onEmulator())
    {
        nanoPerMilliSec = 1000000; /*default*/
        rc = prvTgfPtpGenUtilPortTaiSpeedMeasure(
            prvTgfPortsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS],
            0 /*taiNum*/, &nanoPerMilliSec);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc,
                        "prvTgfPtpGenUtilPortTaiSpeedMeasure");

        prvCpssTimesampScale(&pp0RxTs, 1000000, nanoPerMilliSec, &pp0RxTs);
        prvCpssTimesampScale(&pp0TxTs, 1000000, nanoPerMilliSec, &pp0TxTs);
        prvCpssTimesampScale(&pp1RxTs, 1000000, nanoPerMilliSec, &pp1RxTs);
        prvCpssTimesampScale(&pp1TxTs, 1000000, nanoPerMilliSec, &pp1TxTs);

        PRV_UTF_LOG0_MAC("Timestamps after scaling\n");
        PRV_UTF_LOG3_MAC(
            "pp0RxTs: secondsHigh %d secondsLow %d nanoseconds %d\n",
            pp0RxTs.secondsHigh, pp0RxTs.secondsLow, pp0RxTs.nanoseconds);
        PRV_UTF_LOG3_MAC(
            "pp0TxTs: secondsHigh %d secondsLow %d nanoseconds %d\n",
            pp0TxTs.secondsHigh, pp0TxTs.secondsLow, pp0TxTs.nanoseconds);
        PRV_UTF_LOG3_MAC(
            "pp1RxTs: secondsHigh %d secondsLow %d nanoseconds %d\n",
            pp1RxTs.secondsHigh, pp1RxTs.secondsLow, pp1RxTs.nanoseconds);
        PRV_UTF_LOG3_MAC(
            "pp1TxTs: secondsHigh %d secondsLow %d nanoseconds %d\n",
            pp1TxTs.secondsHigh, pp1TxTs.secondsLow, pp1TxTs.nanoseconds);
    }


    /* AUTODOC: Timestamp PP0 Tx (seconds) */
    txTimeStampf.l[0] = pp0TxTs.secondsLow;
    /* AUTODOC: Timestamp PP1 Rx (seconds) */
    rxTimeb.l[0]      = pp1RxTs.secondsLow;
    tlvEntryPtr = &pduHeaderData.tlvDataArr[16];
    /* AUTODOC: Timestamp PP1 Tx (seconds) */
    txTimeStampf.l[1] = pp1TxTs.secondsLow;
    /* AUTODOC: Timestamp PP0 Rx (seconds) */
    rxTimeb.l[1] = pp0RxTs .secondsLow;

    /* TAI time speed near to CPU system time speed */
    if(rxTimeb.l[1] > txTimeStampf.l[0])
    {
        secondsFromTimestamps =
                        rxTimeb.l[1] - txTimeStampf.l[0];
    }
    else if(txTimeStampf.l[1] > txTimeStampf.l[0])
    {
        secondsFromTimestamps =
                        txTimeStampf.l[1] - txTimeStampf.l[0];
    }
    else
    {
        secondsFromTimestamps = 0;
    }

    PRV_UTF_LOG1_MAC(" secondsDelay = %d\n",
                         secondsDelay);
    PRV_UTF_LOG1_MAC(" secondsFromTimestamps = %d\n",
                         secondsFromTimestamps);

    /* AUTODOC: Check PDU timestamp results */
    if( (secondsFromTimestamps) &&
        (secondsFromTimestamps > secondsDelay) )
    {
        secondsFromTimestamps = secondsDelay;
    }

    UTF_VERIFY_EQUAL2_STRING_MAC(
                secondsDelay, secondsFromTimestamps,
                "Unexpected delay (in seconds) :%d, port %d",
                secondsFromTimestamps,
                prvTgfPortsArray[PRV_TGF_FROM_CPU0_EGRESS_PORT_IDX_CNS]);


    /* AUTODOC: Restore original MAC addresses */
    prvTgfOamPacketHeaderDataReset();
}


