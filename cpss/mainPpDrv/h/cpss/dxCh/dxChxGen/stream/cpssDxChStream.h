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
* @file cpssDxChStream.h
*
* @brief CPSS DxCh Stream configuration API
*   NOTE: some of the function or fields are HSR/PRP some are for the 802.1cb
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChStreamh
#define __cpssDxChStreamh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>/*for CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT*/

/** List of Acronyms used in the API names
    802.1cb    Frame Replication and Elimination for Reliability
    PRP        Parallel Redundancy Protocol
    HSR        High-availability Seamless Redundancy

    SNG        Sequence Number Generation
    IRF        Individual Recovery Function
    SRF        Sequence Recovery Function

    LE         Latent Error
*/


/**
* @enum CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT
 *
 * @brief Stream exception types
*/
typedef enum{

    /** @brief : exception type :
        By IRF : The previous packet sequence number is same as the current packet
        sequence number (of the same stream)
    */
    CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E,

    /** @brief : exception type :
        By SRF : For tag less packet for streams where tag less Packets are not accepted
    */
    CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_TAGLESS_E,

    /** @brief : exception type :
        By SRF : For rogue packets (out of the history buffer range).
    */
    CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_ROGUE_E,

    /** @brief : exception type :
        By SRF : When the packet was already received on a different path
    */
    CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_DUPLICATED_E

} CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT;


/**
* @struct CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC
 *
 * @brief Stream SNG IRF entry info
 *
 * NOTE : Info for SNG (802.1cb and HSR/PRP) and for IRF (802.1cb , but not for HSR/PRP)
 *
 * APPLICABLE DEVICES:      Ironman.
*/
typedef struct{

    /** @brief Enable/Disable the increment of <currentSeqNum>
      *  If sets, assigns <currentSeqNum>++ into the packet's Sequence Number
      *  NOTE: Info for SNG (802.1cb and HSR/PRP)
     */
    GT_BOOL        seqNumAssignmentEnable;
    /** @brief trigger reset of <currentSeqNum>.
        Resets <currentSeqNum> when transition from GT_FALSE to GT_TRUE.
        NOTE: Info for SNG (802.1cb and HSR/PRP)
     */
    GT_BOOL        resetSeqNum;
    /** @brief The current sequence number.
      *  This number will be assigned for the next packet on the stream ID of this entry.
      *  This field is 'Read Only'  (so not relevant to the 'Set' API)
      *  NOTE: Info for SNG (802.1cb and HSR/PRP)
     **/
    GT_U32         currentSeqNum;


    /** @brief Enables the Match Recovery Algorithm for this Stream
      *  NOTE: Info for IRF (802.1cb)
     **/
    GT_BOOL        individualRecoveryEnable;
    /** @brief Indicates whether the Individual recovery algorithm is to accept
      *  the next packet, no matter what the value of its sequence number
      *  NOTE: Info for IRF (802.1cb)
     **/
    GT_BOOL        takeAnySeqNum;
    /** @brief The Recovery sequence number.
      *  The RecovSeqNum variable holds the highest sequence_number value received (in 16 bits),
      *  or the value 0xFFFF, if none have been received since the Individual recovery function was reset.
      *  This field is 'Read Only'  (so not relevant to the 'Set' API)
      *  NOTE: Info for IRF (802.1cb)
     **/
    GT_U32         recoverySeqNum;

}CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC;


/**
* @struct IRF_COUNTERS_ENTRY_STC
 *
 * @brief Stream IRF counters entry info
 *
 * NOTE : counters for IRF (802.1cb , but not for HSR/PRP)
 *
 * APPLICABLE DEVICES:      Ironman.
*/
typedef struct{

    /** @brief The counters for passed packets.
        The counter is Read only
    */
    GT_U64      passedPacketCounter;
    /** @brief The counters for discarded packets and for 'sameId' packets .
        The counter is Read only
    */
    GT_U64      discardedPacketsOrSameIdCounter;
    /** @brief The counters for out of order packets.
        The counter is Read only
    */
    GT_U64      outOfOrderPacketsCounter;
    /** @brief The counters for tag less packets.
        The counter is Read only
    */
    GT_U64      taglessPacketsCounter;
    /** @brief The counters for bytes.
        The counter is Read only
    */
    GT_U64      byteCounter;

}CPSS_DXCH_STREAM_IRF_COUNTERS_ENTRY_STC;


/**
* @struct CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC
 *
 * @brief Stream SRF mapping entry info
 *
 * NOTE : Info for SRF mapping (needed for 802.1cb)
 *
 * APPLICABLE DEVICES:      Ironman.
*/
typedef struct{

    /** @brief Enables the SRF Algorithm for this Stream
     **/
    GT_BOOL     srfEnable;

    /** @brief indicate that the Stream is for multicast (multi-destination) traffic
     **/
    GT_BOOL     isMulticast;

    /** @brief the index to the SRF table for this Stream
     **/
    GT_U32      srfIndex;

    /** @brief The bitmap of egress ports associated with this Stream
     **/
    CPSS_PORTS_BMP_STC egressPortsBmp;

}CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC;

/**
* @struct CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC
 *
 * @brief Stream SRF entry info
 *        The SRF entry is pointed from the Stream by 'srfIndex'
 *
 * NOTE : Info for SRF (needed for 802.1cb)
 *
 * APPLICABLE DEVICES:      Ironman.
*/
typedef struct{

    /** @brief Indication if to use 'vector recovery algorithm' or the  'match recovery algorithm'
      *  GT_TRUE  - vector recovery algorithm
      *  GT_FALSE - match  recovery algorithm
     **/
    GT_BOOL             useVectorRecoveryAlgorithm;

    /** @brief The history length for the vector recovery algorithm
      *  APPLICABLE RANGES : 2..2K
     **/
    GT_U32              vectorSequenceHistoryLength;

    /** @brief Indicates whether packets with no sequence number are to be accepted.
     **/
    GT_BOOL             takeNoSeqNum;

    /** @brief Indicates whether the Individual recovery algorithm is to accept
      *  the next packet, no matter what the value of its sequence number
     **/
    GT_BOOL             takeAnySeqNum;

    /** @brief The first History Buffer entry number allocated for this function.
      *  APPLICABLE RANGES : 0..(1K-1)
     **/
    GT_U32              firstHistoryBuffer;

    /** @brief The number of History Buffers allocated to this function.
      *  APPLICABLE RANGES : 1..16
     **/
    GT_U32              numOfBuffers;

    /** @brief Specifies the number of times that will take the daemon to visit this entry till it is restart ,
      *  by the Recovery algorithm if no packets has been received.
      *  APPLICABLE RANGES : 0..(64K-1)
     **/
    GT_U32              restartThreshold;

    /** @brief Indicates whether the Latent Error detection is enabled or disabled
      *  GT_TRUE  - Enabled  the Latent Error detection.
      *  GT_FALSE - Disabled the Latent Error detection.
      * NOTE: used by the daemon.
     **/
    GT_BOOL             leEnable;

    /** @brief Indicates whether the Latent Error detection reset is enabled or disabled
      *  GT_TRUE  - Enabled  the Latent Error detection reset.
      *  GT_FALSE - Disabled the Latent Error detection reset.
      *  NOTE: used by the daemon.
     **/
    GT_BOOL             leResetTimeEnabled;

    /** @brief Specifies the timeout in seconds for Latent Error detection reset.
      *  Relevant when leResetTimeEnabled == GT_TRUE
      *  NOTE: used by the daemon.
      *  APPLICABLE RANGES : 1..31
     **/
    GT_U32              leResetTimeInSeconds;

}CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC;

/**
* @struct CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC
 *
 * @brief Stream SRF counters entry info
 *        The SRF counters entry is pointed from the Stream by 'srfIndex'
 *
 * NOTE : Info for SRF (needed for 802.1cb)
 *
 * APPLICABLE DEVICES:      Ironman.
*/
typedef struct{

    /** @brief Counts the number of packets that were accepted by the Recovery Algorithm.
      *  Wraparound counter.
      *  NOTE: this is read only counter. (not clear on read)
     **/
    GT_U64              passedPacketsCounter;

    /** @brief Counts the number of packets that were discarded by the Recovery Algorithm.
      *  Wraparound Counter.
      *  NOTE: this is read only counter. (not clear on read)
     **/
    GT_U64              discardedPacketsCounter;

    /** @brief Counts packets received out of order by the Recovery Algorithm.
      *  Wraparound Counter.
      *  NOTE: this is read only counter. (not clear on read)
     **/
    GT_U64              outOfOrderPacketsCounter;

    /** @brief Counts the number of the History bits that were cleared by the
      *  Vector Recovery Algorithm and were already zero.
      *  This counter gives indication to the number of lost packets.
      *  Wraparound Counter.
      *  NOTE: this is read only counter. (not clear on read)
     **/
    GT_U64              clearedZeroHistoryBitsCounter;

    /** @brief Counts packet discarded by the Vector Recovery Algorithm because
      *  its sequence number is more than History Length
      *  (<Sequence History Length>+1) from <RecovSeqNum>.
      *  Wraparound Counter.
      *  NOTE: this is read only counter. (not clear on read)
    **/
    GT_U64              roguePacketCounter;

}CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC;

/**
* @struct CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC
 *
 * @brief Stream SRF status entry info
 *        The SRF status entry is pointed from the Stream by 'srfIndex'
 *        The info is 'status' that is 'read only'
 *
 * NOTE : Info for SRF (needed for 802.1cb)
 *
 * APPLICABLE DEVICES:      Ironman.
*/
typedef struct{

    /** @brief The Recovery sequence number.
      *  The highest sequence_number value received (in 16 bits),
      *  or the value 0xFFFF, if none have been received since the Sequence recovery function was reset.
     **/
    GT_U32              recoverySeqNum;

    /** @brief The current History Buffer number associated with <recoverySeqNum>.
     **/
    GT_U32              currHistoryBuffer;

    /** @brief The bit number in currHistoryBuffer associated with <recoverySeqNum>.
     **/
    GT_U32              currHistoryBufferBit;

    /** @brief Holds the number of time in micro seconds remaining until the
        Sequence Recovery Function is restarted.
        It is decremented regularly by the Restart daemon , and reset to restartThreshold
        by the Sequence Recovery algorithm when a packet is accepted.
     **/
    GT_U32              untillRestartTimeInMicroSec;

    /** @brief Counts the number of times the Recovery Function was restarted.
        Wraparound Counter.
     **/
    GT_U32              numberOfResets;

    /** @brief The offset between the expected and the actual number of discarded packets.
        Updated each time the Latent function is reset.
     **/
    GT_U32              leBaseDifference;

    /** @brief The time in seconds since last Latent function reset.
     **/
    GT_U32              leTimeInSecSinceLastReset;

}CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC;

/**
* @struct CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC
 *
 * @brief SRF daemon info
 *
 * NOTE : Info for SRF (needed for 802.1cb)
 *
 * APPLICABLE DEVICES:      Ironman.
*/
typedef struct{

    /** @brief Enable/Disable the Restart Daemon
      *  NOTE: 1. For the Restart Daemon to work on the SRF entries once every <restartPeriodTimeInMicroSec>
      *      need : restartDaemonEnable = GT_TRUE
    */
    GT_BOOL         restartDaemonEnable;

    /** @brief Enable/Disable the LE Daemons (LE Reset Daemon  , LE detection Daemon)
      *  NOTEs: 1. For the LE Reset Daemon to work on the SRF entries once every 1 second
      *           need : latentErrorDaemonEnable = GT_TRUE and latentErrorDetectionResetEnable = GT_TRUE
      *        2.  For the LE detection Daemon to work on the SRF entries once every <latentErrorDetectionPeriodTimeInMicroSec>
      *           need : latentErrorDaemonEnable = GT_TRUE and latentErrorDetectionAlgorithmEnable = GT_TRUE
    */
    GT_BOOL         latentErrorDaemonEnable;

    /** @brief The restart daemon starts scanning time (in micro seconds)
    */
    GT_U32          restartPeriodTimeInMicroSec;

    /** @brief The latent error daemon starts scanning  (in micro seconds)
    */
    GT_U32          latentErrorDetectionPeriodTimeInMicroSec;

    /** @brief Enable/Disable LE Detection Algorithm
    */
    GT_BOOL         latentErrorDetectionAlgorithmEnable;

    /** @brief Enable/Disable LE Detection Reset
    */
    GT_BOOL         latentErrorDetectionResetEnable;

}CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC;


/**
* @struct CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC
 *
 * @brief SRF History buffer entry info
 *        Needed only for debug purposes
 *
 * NOTE : Info for SRF (needed for 802.1cb)
 *
 * APPLICABLE DEVICES:      Ironman.
*/
typedef struct{
    /** @brief array of 4 words represent 128 history bits
      *  historyBuffer[0] hold history bits 0  ..  31
      *  historyBuffer[1] hold history bits 32 ..  63
      *  historyBuffer[2] hold history bits 64 ..  95
      *  historyBuffer[3] hold history bits 96 .. 127
    */
    GT_U32          historyBuffer[4];
}CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC;

/**
* @internal cpssDxChStreamSngEnableSet function
* @endinternal
*
* @brief  Set Enable/Disable the SNG (Sequence Number Generation).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] enable         - Indication to Enable/Disable the SNG.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSngEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
);

/**
* @internal cpssDxChStreamSngEnableGet function
* @endinternal
*
* @brief  Get the Enable/Disable of the SNG (Sequence Number Generation).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] enablePtr     - (pointer to) Indication to Enable/Disable the SNG.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSngEnableGet
(
    IN  GT_U8                           devNum,
    OUT GT_BOOL                         *enablePtr
);


/**
* @internal cpssDxChStreamIrfEnableSet function
* @endinternal
*
* @brief  Set Enable/Disable the IRF (Individual Recovery Function).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] enable         - Indication to Enable/Disable the IRF.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamIrfEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
);

/**
* @internal cpssDxChStreamIrfEnableGet function
* @endinternal
*
* @brief  Get the Enable/Disable of the IRF (Individual Recovery Function).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] enablePtr     - (pointer to) Indication to Enable/Disable the IRF.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamIrfEnableGet
(
    IN  GT_U8                           devNum,
    OUT GT_BOOL                         *enablePtr
);

/**
* @internal cpssDxChStreamExceptionCommandSet function
* @endinternal
*
* @brief  Set the packet command to specific exception type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
* @param[in] command        - the packet command.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type/command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamExceptionCommandSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT      type,
    IN CPSS_PACKET_CMD_ENT                      command
);

/**
* @internal cpssDxChStreamExceptionCommandGet function
* @endinternal
*
* @brief  Get the packet command for specific exception type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
*
* @param[out] commandPtr    - (pointer to) the packet command.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamExceptionCommandGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT      type,
    OUT CPSS_PACKET_CMD_ENT                     *commandPtr
);

/**
* @internal cpssDxChStreamExceptionCpuCodeSet function
* @endinternal
*
* @brief  Set the cpu code to specific exception type.
*         Note : Relevant when the packet command for the exception is not 'FORWARD'
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
* @param[in] cpuCode        - the cpu code.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type/cpuCode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamExceptionCpuCodeSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT      type,
    IN  CPSS_NET_RX_CPU_CODE_ENT                 cpuCode
);

/**
* @internal cpssDxChStreamExceptionCpuCodeGet function
* @endinternal
*
* @brief  Get the cpu code for specific exception type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
*
* @param[out] cpuCodePtr    - (pointer to) The cpu code.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamExceptionCpuCodeGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT      type,
    OUT CPSS_NET_RX_CPU_CODE_ENT                 *cpuCodePtr
);

/**
* @internal cpssDxChStreamCountingModeSet function
* @endinternal
*
* @brief   Set the counting mode for streams.
*   NOTE: the feature not relate to the CNC unit.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] irfMode               - The mode for the IRF byte counting (L2/L3 counting)
* @param[in] srfTaglessEnable      - enable/disable the SRF tagless counting.
*                                    GT_TRUE  - enable  the SRF tagless counting
*                                    GT_FALSE - disable the SRF tagless counting
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamCountingModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT       irfMode,
    IN  GT_BOOL                                 srfTaglessEnable
);

/**
* @internal cpssDxChStreamCountingModeGet function
* @endinternal
*
* @brief   Get the counting mode for streams.
*   NOTE: the feature not relate to the CNC unit.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
*
* @param[out] irfModePtr           - (pointer to) The mode for the IRF byte counting (L2/L3 counting)
* @param[out] irfModePtr           - (pointer to) enable/disable the SRF tagless counting.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamCountingModeGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT       *irfModePtr,
    OUT GT_BOOL                                 *srfTaglessEnablePtr
);

/**
* @internal cpssDxChStreamSrfDaemonInfoSet function
* @endinternal
*
* @brief   Set the SRF daemon info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] infoPtr               - (pointer to) The info about the SRF daemon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter error
* @retval GT_OUT_OF_RANGE          - on out of range parameter error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfDaemonInfoSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC    *infoPtr
);

/**
* @internal cpssDxChStreamSrfDaemonInfoGet function
* @endinternal
*
* @brief   Get the SRF daemon info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
*
* @param[out] infoPtr              - (pointer to) The info about the SRF daemon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfDaemonInfoGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC    *infoPtr
);

/**
* @internal cpssDxChStreamEntrySet function
* @endinternal
*
* @brief  Set the stream entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index. (streamId)
*                             APPLICABLE RANGE: 0..(2K-1)
* @param[in] sngIrfInfoPtr  - (pointer to) The sng/irf entry info.
*                               ignored if NULL
* @param[in] srfMappingInfoPtr - (pointer to) The SRF mapping entry.
*                               ignored if NULL
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_OUT_OF_RANGE          - on out of range parameter error
* @retval GT_BAD_PTR               - if ALL the pointers are NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC          *sngIrfInfoPtr,
    IN  CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC      *srfMappingInfoPtr
);

/**
* @internal cpssDxChStreamEntryGet function
* @endinternal
*
* @brief  Get the stream entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index. (streamId)
*                             APPLICABLE RANGE: 0..(2K-1)
*
* @param[out] sngIrfInfoPtr - (pointer to) The sng/irf entry info.
*                               ignored if NULL
* @param[out] irfCountersPtr - (pointer to) The irf counters entry.
*                               ignored if NULL
* @param[out] srfMappingInfoPtr - (pointer to) The SRF mapping entry.
*                               ignored if NULL
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - if ALL the pointers are NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC          *sngIrfInfoPtr,
    OUT CPSS_DXCH_STREAM_IRF_COUNTERS_ENTRY_STC     *irfCountersPtr,
    OUT CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC      *srfMappingInfoPtr
);

/**
* @internal cpssDxChStreamAgingGroupSet function
* @endinternal
*
* @brief  Set group of 32 bits bitmap of aging bits (for 32 consecutive streams).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] startIndex     - The start index. (streamId)
*                             in steps of 32 : 0,32,64 .. (2K-32) for corresponding indexes in cpssDxChStreamEntrySet/Get
*                             APPLICABLE RANGE: 0,32,64 .. (2K-32)
* @param[in] agingGroupBmp  - 32 bits bitmap of aging bits from startIndex .. (startIndex + 31)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or startIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamAgingGroupSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    IN  GT_U32                          agingGroupBmp
);

/**
* @internal cpssDxChStreamAgingGroupGet function
* @endinternal
*
* @brief  Get group of 32 bits bitmap of aging bits (for 32 consecutive streams).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] startIndex     - The start index. (streamId)
*                             in steps of 32 : 0,32,64 .. (2K-32) for corresponding indexes in cpssDxChStreamEntrySet/Get
*                             APPLICABLE RANGE: 0,32,64 .. (2K-32)
*
* @param[out] agingGroupBmpPtr  - (pointer to) 32 bits bitmap of aging bits from startIndex .. (startIndex + 31)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or startIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamAgingGroupGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    OUT GT_U32                          *agingGroupBmpPtr
);

/**
* @internal cpssDxChStreamSrfEntrySet function
* @endinternal
*
* @brief  Set the stream SRF entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The SRF index. (pointed by srfIndex)
*                             APPLICABLE RANGE: 0..(1K-1)
* @param[in] srfConfigInfoPtr - (pointer to) The SRF config entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_OUT_OF_RANGE          - on out of range parameter error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC       *srfConfigInfoPtr
);

/**
* @internal cpssDxChStreamSrfEntryGet function
* @endinternal
*
* @brief  Get the stream SRF entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The SRF index. (pointed by srfIndex)
*                             APPLICABLE RANGE: 0..(1K-1)
*
* @param[out] srfConfigInfoPtr - (pointer to) The SRF config entry info.
*                               ignored if NULL
* @param[out] srfCountersPtr   - (pointer to) The SRF counters entry.
*                               ignored if NULL
* @param[out] srfStatusInfoPtr - (pointer to) The SRF status entry info.
*                               ignored if NULL
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - if ALL the pointers are NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC       *srfConfigInfoPtr,
    OUT CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC     *srfCountersPtr,
    OUT CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC       *srfStatusInfoPtr
);

/**
* @internal cpssDxChStreamSrfLatentErrorDetectedGroupGet function
* @endinternal
*
* @brief  Get group of 32 bits bitmap of error bits (for 32 consecutive SRF indexes).
*   NOTE: 1. can call this function on event of type CPSS_PP_STREAM_LATENT_ERROR_E
*         to get the streams with latent error
*         2. the registers are Read only and clear after read.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] startIndex     - The start SRF index. (pointed by srfIndex)
*                             in steps of 32  : 0,32,64 .. (1K-32) for corresponding indexes in cpssDxChStreamSrfEntrySet/Get
*                             APPLICABLE RANGE: 0,32,64 .. (1K-32)
*
* @param[out] errorGroupBmpPtr  - (pointer to) 32 bits bitmap of error bits from startIndex .. (startIndex + 31)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or startIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfLatentErrorDetectedGroupGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    OUT GT_U32                          *errorGroupBmpPtr
);

/**
* @internal cpssDxChStreamSrfHistoryBufferEntrySet function
* @endinternal
*
* @brief  Set the stream SRF history buffer entry info. (bitmap of 128 history bits)
*         Needed only for debug purposes.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The SRF History buffer index. (pointed by 'currHistoryBuffer' or 'firstHistoryBuffer')
*                             APPLICABLE RANGE: 0..(1K-1)
* @param[in] histBuffPtr    - (pointer to) The entry info. (bitmap of 128 history bits)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfHistoryBufferEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC   *histBuffPtr
);

/**
* @internal cpssDxChStreamSrfHistoryBufferEntryGet function
* @endinternal
*
* @brief  Get the stream SRF history buffer entry info. (bitmap of 128 history bits)
*         Needed only for debug purposes.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The SRF History buffer index. (pointed by 'currHistoryBuffer' or 'firstHistoryBuffer')
*                             APPLICABLE RANGE: 0..(1K-1)
*
* @param[out] histBuffPtr    - (pointer to) The entry info. (bitmap of 128 history bits)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfHistoryBufferEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC   *histBuffPtr
);

/**
* @internal cpssDxChStreamSrfZeroBitVectorGroupEntrySet function
* @endinternal
*
* @brief  Set the stream SRF Zero bit vector entry info. (bitmap of 32 bits)
*         Needed only for debug purposes.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] startIndex     - The SRF History buffer index. (pointed by 'currHistoryBuffer' or 'firstHistoryBuffer')
*                             in steps of 32 : 0,32,64 .. (1K-32) for corresponding indexes of 'currHistoryBuffer' or 'firstHistoryBuffer'
*                             APPLICABLE RANGE: 0,32,64 .. (1K-32)
* @param[in] vector          - the vector of the 32 bits
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or startIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfZeroBitVectorGroupEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    IN  GT_U32                          vector
);

/**
* @internal cpssDxChStreamSrfZeroBitVectorGroupEntryGet function
* @endinternal
*
* @brief  Get the stream SRF Zero bit vector entry info. (bitmap of 32 bits)
*         Needed only for debug purposes.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] startIndex     - The SRF History buffer index. (pointed by 'currHistoryBuffer' or 'firstHistoryBuffer')
*                             in steps of 32 : 0,32,64 .. (1K-32) for corresponding indexes of 'currHistoryBuffer' or 'firstHistoryBuffer'
*                             APPLICABLE RANGE: 0,32,64 .. (1K-32)
*
* @param[out] vectorPtr       - (point to) the vector of the 32 bits
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or startIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfZeroBitVectorGroupEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    OUT GT_U32                          *vectorPtr
);

/**
* @internal cpssDxChStreamSrfTrgPortMappedMulticastIndexSet function
* @endinternal
*
* @brief  Set the mapped target physical port used for calculating the SRF index.
*         Relevant for SRF Multicast Streams only.
*         For LAGs, all the ports in the LAG should be mapped to a single port L
*         in the LAG.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The target physical port number.
* @param[in] index          - The index that represent the physical port in multicast stream.
*                             APPLICABLE RANGES : 0..63 (according to range of physical port)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or or portNum
* @retval GT_OUT_OF_RANGE          - on out of range parameter error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfTrgPortMappedMulticastIndexSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          index
);

/**
* @internal cpssDxChStreamSrfTrgPortMappedMulticastIndexGet function
* @endinternal
*
* @brief  Get the mapped target physical port used for calculating the SRF index.
*         Relevant for SRF Multicast Streams only.
*         For LAGs, all the ports in the LAG should be mapped to a single port L
*         in the LAG.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The target physical port number.
*
* @param[out] indexPtr      - (pointer to) The index that represent the physical port in multicast stream.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfTrgPortMappedMulticastIndexGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *indexPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChStreamh */

