/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfCncGen.h
*
* DESCRIPTION:
*       Precision Time Protocol (PTP)
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*
*******************************************************************************/
#ifndef __prvTgfPtpGen_h
#define __prvTgfPtpGen_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* packet VID */
#define PRV_TGF_PTP_GEN_VLAN_CNS  2

extern GT_U32 prvTgfSendPortIdx; /* 1 */
extern GT_U32 prvTgfReceivePortIdx; /* 3 */
/* send port index */
#define PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS (prvTgfSendPortIdx)

/* receive port index */
#define PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS (prvTgfReceivePortIdx)

/* PCL rule index */
#define PRV_TGF_PTP_GEN_PCL_RULE_INDEX_CNS  8

/* TTI rule index */
#define PRV_TGF_PTP_GEN_TTI_RULE_INDEX_CNS  12

/********************************************************************************/
/*                                     UTILS                                    */
/********************************************************************************/
/**
* @internal prvTgfPtpFrameCounterEnable function
* @endinternal
*
* @brief   Enables frame counter for particulat packet type.
*
* @retval none
*/
GT_STATUS prvTgfPtpFrameCounterEnable
(
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  PRV_TGF_PTP_TS_PACKET_TYPE_ENT          format,
    IN  PRV_TGF_PTP_TS_ACTION_ENT               action
);

/**
* @internal prvTgfPtpFrameCounterVerifyAndRestore function
* @endinternal
*
* @brief  Verify the frame counter for particulat packet type and restore counter control
*
* @retval none
*/
GT_VOID prvTgfPtpFrameCounterVerifyAndRestore
(
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32               pktCnt
);

/**
* @internal prvTgfPtpGenUtilTaiTodCounterSet function
* @endinternal
*
* @brief   Set TOD Counter current time.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
* @param[in] tgfTodValuePtr           - (pointer to) TOD value.
*                                       relevant returned code
*/
GT_STATUS prvTgfPtpGenUtilTaiTodCounterSet
(
    IN  PRV_TGF_PTP_TAI_ID_STC              *tgfTaiIdPtr,
    IN  PRV_TGF_PTP_TOD_COUNT_STC           *tgfTodValuePtr
);

/**
* @internal prvTgfPtpGenUtilTaiTodCounterGet function
* @endinternal
*
* @brief   Get TOD Counter current time.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*
* @param[out] tgfTodValuePtr           - (pointer to) TOD value.
*                                       relevant returned code
*/
GT_STATUS prvTgfPtpGenUtilTaiTodCounterGet
(
    IN  PRV_TGF_PTP_TAI_ID_STC              *tgfTaiIdPtr,
    OUT PRV_TGF_PTP_TOD_COUNT_STC           *tgfTodValuePtr
);

/**
* @internal prvTgfPtpGenUtilTaiSpeedMeasure function
* @endinternal
*
* @brief   Measure TAI speed relative to CPU system time.
*
* @param[in] tgfTaiIdPtr              - (pointer to) TAI Units identification.
*
* @param[out] nanoPerMilliSecPtr      - (pointer to) amount of TAI nanoseconds
*                                       in CPU system time millisecond.
*/
GT_STATUS prvTgfPtpGenUtilTaiSpeedMeasure
(
    IN PRV_TGF_PTP_TAI_ID_STC     *tgfTaiIdPtr,
    OUT GT_U32                    *nanoPerMilliSecPtr
);

/**
* @internal prvTgfPtpGenUtilPortTaiSpeedMeasure function
* @endinternal
*
* @brief   Measure TAI speed relative to CPU system time.
*
* @param[in] portNum              - port number.
* @param[in] taiNun               - TAI number.
*
* @param[out] nanoPerMilliSecPtr      - (pointer to) amount of TAI nanoseconds
*                                       in CPU system time millisecond.
*/
GT_STATUS prvTgfPtpGenUtilPortTaiSpeedMeasure
(
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  GT_U32                    taiNum,
    OUT GT_U32                    *nanoPerMilliSecPtr
);

/********************************************************************************/
/*                                Test Functions                                */
/********************************************************************************/

/**
* @internal prvTgfPtpGenVidAndFdbDefCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpGenVidAndFdbDefCfgSet
(
    GT_VOID
);

/**
* @internal prvTgfPtpGenVidAndFdbDefCfgRestore function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpGenVidAndFdbDefCfgRestore
(
    GT_VOID
);


/**
* @internal prvTgfPtpGenVidAndFdbDefCfgRestoreEgress function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpGenVidAndFdbDefCfgRestoreEgress
(
    GT_VOID
);

/**
* @internal prvTgfPtpGenAddTimeAfterPtpHeaderCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpGenAddTimeAfterPtpHeaderCfgSet
(
    GT_VOID
);

/**
* @internal prvTgfPtpGenIngressEgressCaptureCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpGenIngressEgressCaptureCfgSet
(
     GT_VOID
);

/**
* @internal prvTgfPtpGenIngressEgressCaptureRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpGenIngressEgressCaptureRestore
(
     GT_VOID
);

/**
* @internal prvTgfPtpGenIngressCaptureCfgSet function
* @endinternal
*
* @brief   None
*
* @param[in] receive_port_index       -Egress port index [0,3]
*                                       None
*/
GT_STATUS prvTgfPtpGenIngressCaptureCfgSet
(
     IN GT_U32              receive_port_index
);

/**
* @internal prvTgfPtpGenIngressCaptureRestore function
* @endinternal
*
* @brief   None
*
* @param[in] receive_port_index       -Egress port index [0,3]
*                                       None
*/
GT_STATUS prvTgfPtpGenIngressCaptureRestore
(
        IN GT_U32 receive_port_index
);

/**
* @internal prvTgfPtpGenTsTagPortCfgPiggyBackSet function
* @endinternal
*
* @brief   set per port configuration timestamp tag Piggyback
*
* @param[in] port- port number
*
*/
GT_STATUS prvTgfPtpGenTsTagPortCfgPiggyBackSet
(
        IN GT_U32 port
);

/**
* @internal prvTgfPtpGenTsTagPortCfgPiggyBackRestore function
* @endinternal
*
* @brief   restore per port configuration timestamp tag Piggyback
*
* @param[in] port- port number
*
*/
GT_STATUS prvTgfPtpGenTsTagPortCfgPiggyBackRestore
(
        IN GT_U32 port
);

/**
* @internal prvTgfPtpGenFdbDefCfgSet function
* @endinternal
*
* @brief   None
*
* @param[in] receive_port_index       -Egress port index [0,3]
*                                       None
*/

GT_STATUS prvTgfPtpGenFdbDefCfgSet
(
    IN GT_U32              receive_port_index

);


/**
* @internal prvTgfPtpGenEgressCaptureCfgSet function
* @endinternal
*
* @param[in] receive_port_index       -Egress port index [0,3]
*                                       None
*/
GT_STATUS prvTgfPtpGenEgressCaptureCfgSet
(
    IN GT_U32              receive_port_index
);

/**
* @internal prvTgfPtpGenAddCorrectedTimeCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpGenAddCorrectedTimeCfgSet
(
    GT_VOID
);


/**
* @internal prvTgfPtpGenEgressCaptureQueue1Check function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpGenEgressCaptureQueue1Check
(
     IN GT_U32 expected_valid
);

/**
* @internal prvTgfPtpGenIngressCaptureQueue1Check function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpGenIngressCaptureQueue1Check
(
     IN GT_U32 expected_valid
);

/**
* @internal prvTgfPtpGenCfgRestore function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpGenCfgRestore
(
    GT_VOID
);

/**
* @internal prvTgfPtpGenAddTimeAfterPtpHeaderTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpGenAddTimeAfterPtpHeaderTrafficGenerateAndCheck
(
    GT_VOID
);

/**
* @internal prvTgfPtpGenPiggyBackTimestampTrafficGenerateAndCheck function
* @endinternal
*
* @brief  Generate traffic:
*         Send to device's port given packet under capture.
*         Check captured packet.
*/
GT_VOID prvTgfPtpGenPiggyBackTimestampTrafficGenerateAndCheck
(
    GT_VOID
);

/**
* @internal prvTgfPtpGenAddIngressEgressTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpGenAddIngressEgressTrafficGenerateAndCheck
(
    GT_VOID
);



/**
* @internal prvTgfPtpGenTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet.
*/
GT_STATUS prvTgfPtpGenTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfPtpGenAddCorrectionTimeTrafficCheck function
* @endinternal
*
* @brief   Send traffic and check results
*/
GT_VOID prvTgfPtpGenAddCorrectionTimeTrafficCheck
(
    GT_VOID
);

/**
* @internal prvTgfPtpGenAddCorrectedTimeCfgRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpGenAddCorrectedTimeCfgRestore
(
    GT_VOID
);

/**
* @internal prvTgfPtpGenGetEvent function
* @endinternal
*
* @brief   check events number
*/
GT_VOID prvTgfPtpGenGetEvent
(
    IN  CPSS_UNI_EV_CAUSE_ENT uniEvent,
    IN  GT_U32  eventsNum,
    IN  GT_BOOL check
);

/**
* @internal prvTgfPtpGenEgressTimestempPortsConfig function
* @endinternal
*
* @brief   Configure tested ports for specific systems
*/
GT_VOID  prvTgfPtpGenEgressTimestempPortsConfig
(
    IN GT_U32 egressPortIndexPipe0,
    IN GT_U32 egressPortIndexPipe1
);

/**
* @internal prvTgfPtpGenEgressTimestempQueueInteruptsConfig function
* @endinternal
*
* @brief   set ermrk ptp queue size.
*/
GT_VOID  prvTgfPtpGenEgressTimestempQueueInteruptsConfig
(
    IN GT_U32 egressPortIndexPipe0,
    IN GT_U32 egressPortIndexPipe1
);

/**
* @internal prvTgfPtpGenEgressTimestempQueueInteruptsRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID  prvTgfPtpGenEgressTimestempQueueInteruptsRestore
(
    GT_VOID
);

/**
* @internal prvTgfPtpGenAddTimeStampTagAndMirrorSet function
* @endinternal
*
* @brief   1)config ptp action to timestamp
*         2)config oam ptp table
*         3)config pcl rule to mirror and oam action
*         4)config timestamp tag format
*/
GT_VOID prvTgfPtpGenAddTimeStampTagAndMirrorSet
(
    GT_VOID
);

/**
* @internal prvTgfPtpGenTrafficAndCheckCpuGetTimeStampTag function
* @endinternal
*
* @brief   send traffic , get packet from the cpu and check the timestamp tag.
*/
GT_VOID prvTgfPtpGenTrafficAndCheckCpuGetTimeStampTag
(
    GT_VOID
);

/**
* @internal prvTgfPtpGenAddTimeStampTagAndMirrorrRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpGenAddTimeStampTagAndMirrorrRestore
(
    GT_VOID
);

/**
* @internal prvTgfPtpEgresssExceptionCfgSet function
* @endinternal
*
* @brief  Configuration to receive PTP Packet
*/
GT_VOID prvTgfPtpEgresssExceptionCfgSet
(
    GT_VOID
);

/**
* @internal prvTgfPtpEgressExceptionTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpEgressExceptionTrafficGenerateAndCheck
(
    GT_VOID
);

/**
* @internal prvTgfPtpEgressExceptionTestRestore function
* @endinternal
*
* @brief  Restore the Configuration
*/
GT_VOID prvTgfPtpEgressExceptionTestRestore
(
    GT_VOID
);

/**
* @internal prvTgfPtpInvalidPtpCfgSet function
* @endinternal
*
* @brief  Configuration to receive PTP Packet
*/

GT_VOID prvTgfPtpInvalidPtpCfgSet
(
    GT_VOID
);

/**
* @internal prvTgfPtpInvalidPtpTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet
*/
GT_VOID prvTgfPtpInvalidPtpTrafficGenerateAndCheck
(
    GT_VOID
);

/**
* @internal prvTgfPtpInvalidPtpCfgRestore function
* @endinternal
*
* @brief  Restore Configuration
*/
GT_VOID prvTgfPtpInvalidPtpCfgRestore
(
    GT_VOID
);

/**
* @internal prvTgfPtpCapturePreviousModeCfgAndVerifyCheck function
* @endinternal
*
* @brief  Set the TOD
*         Enable capture previous mode:
*         Set the TOD+1000 sec
*         Verify the capture value.
*/
GT_VOID prvTgfPtpCapturePreviousModeCfgAndVerifyCheck
(
    GT_VOID
);

/**
* @internal prvTgfPtpCapturePreviousModeCfgRestore function
* @endinternal
*
* @brief  Restore Configuration
*/
GT_VOID prvTgfPtpCapturePreviousModeCfgRestore
(
    GT_VOID
);

/**
* @internal prvTgfPtpOamBasedTimestampTriggerSet function
* @endinternal
*
* @brief  1)config oam ptp table
*         3)config pcl rule to mirror and oam action
*
* @param[in] oamPtpIndex    - OAM/PTP index
* @param[in] actionPktCmd   - PCL Action command
*/
GT_VOID prvTgfPtpOamBasedTimestampTriggerSet
(
    GT_U32              oamPtpIndex,
    CPSS_PACKET_CMD_ENT actionPktCmd
);

/**
* @internal prvTgfPtpGenTsxDisable function
* @endinternal
*
* @brief   Disable TSX module in HW
*/
GT_STATUS prvTgfPtpGenTsxDisable
(
    GT_PHYSICAL_PORT_NUM  ingressPortNum,
    GT_PHYSICAL_PORT_NUM  egressPortNum
);

/**
* @internal prvTgfPtpGenTsxRestore function
* @endinternal
*
* @brief   Restore TXS module configuration
*/
GT_STATUS prvTgfPtpGenTsxRestore
(
    GT_PHYSICAL_PORT_NUM  ingressPortNum,
    GT_PHYSICAL_PORT_NUM  egressPortNum
);

#if 0
/**
* @internal prvTgfPtpGenTimeStampUDBRestore function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpGenTimeStampUDBRestore
(
    GT_VOID
);

/**
* @internal prvTgfPtpGenTimeStampUDBCfgSet function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpGenTimeStampUDBCfgSet
(
    GT_VOID
);

/**
* @internal prvTgfPtpGenTimeStampTrafficGenerateAndCheck function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpGenTimeStampTrafficGenerateAndCheck
(
    GT_VOID
);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPtpGen_h */


