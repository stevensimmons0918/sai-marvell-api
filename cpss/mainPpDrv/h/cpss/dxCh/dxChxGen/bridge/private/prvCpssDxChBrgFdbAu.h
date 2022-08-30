/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file prvCpssDxChBrgFdbAu.h
*
* @brief MAC hash struct implementation.
*
* @version   8
********************************************************************************
*/
#ifndef __prvCpssDxChBrgFdbAuh
#define __prvCpssDxChBrgFdbAuh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/common/config/private/prvCpssGenIntDefs.h>


/**
* @enum MESSAGE_QUEUE_ENT
 *
 * @brief hold the different types of message queues that cpss SW uses
*/
typedef enum{

    /** the primary AUQ */
    MESSAGE_QUEUE_PRIMARY_AUQ_E,

    /** the primary FUQ */
    MESSAGE_QUEUE_PRIMARY_FUQ_E,

    /** the secondary AUQ */
    MESSAGE_QUEUE_SECONDARY_AUQ_E,

    /** the FUQ for CNC2,3 */
    MESSAGE_QUEUE_CNC23_FUQ_E


} MESSAGE_QUEUE_ENT;

#define HW_FORMAT_2_CMD_MAC(hwData,macCmd)                                 \
    macCmd = (((hwData) == 0) ? CPSS_MAC_TABLE_FRWRD_E  :                  \
                    (((hwData) == 1) ? CPSS_MAC_TABLE_MIRROR_TO_CPU_E :    \
                    (((hwData) == 2) ? CPSS_MAC_TABLE_CNTL_E :             \
                    (((hwData) == 3) ? CPSS_MAC_TABLE_DROP_E :             \
                    (((hwData) == 4) ? CPSS_MAC_TABLE_SOFT_DROP_E :        \
                                       CPSS_MAC_TABLE_FRWRD_E)))));

/**
* @internal auqFuqClearUnreadMessages function
* @endinternal
*
* @brief   The function scan the AU/FU queues, clear all unread
*         AU/FU messages in the queue and calculate number of messages
*         that remained till the end of the queue.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which AU are counted
* @param[in] portGroupId              - the  - for multi-port-groups support
* @param[in] queueType                - AUQ or FUQ. FUQ valid for DxCh2 and above.
*
* @param[out] numOfAuPtr               - (pointer to) number of AU messages processed in the specified queue.
* @param[out] numMsgTillQueueEndPtr    - (pointer to) number of AU messages remained to fill the queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, queueType.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on not initialized queue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In order to have the accurate number of entries application should
*       protect Mutual exclusion between HW access to the AUQ/FUQ
*       SW access to the AUQ/FUQ and calling to this API.
*       i.e. application should stop the PP from sending AU messages to CPU.
*       and should not call the api's
*       cpssDxChBrgFdbFuMsgBlockGet, cpssDxChBrgFdbFuMsgBlockGet
*
*/
GT_STATUS auqFuqClearUnreadMessages
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  MESSAGE_QUEUE_ENT             queueType,
    OUT GT_U32                       *numOfAuPtr,
    OUT GT_U32                       *numMsgTillQueueEndPtr
);

/**
* @internal auqFuqMessagesNumberGet function
* @endinternal
*
* @brief   The function scan the AU/FU queues and returns the number of
*         AU/FU messages in the queue.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number on which AU are counted
* @param[in] portGroupId              - the  - for multi-port-groups support
* @param[in] queueType                - AUQ or FUQ. FUQ valid for DxCh2 and above.
*
* @param[out] numOfAuPtr               - (pointer to) number of AU messages in the specified queue.
* @param[out] endOfQueueReachedPtr     - (pointer to) GT_TRUE: The queue reached to the end.
*                                      GT_FALSE: else
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, queueType.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on not initialized queue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In order to have the accurate number of entries application should
*       protect Mutual exclusion between HW access to the AUQ/FUQ
*       SW access to the AUQ/FUQ and calling to this API.
*       i.e. application should stop the PP from sending AU messages to CPU.
*       and should not call the api's
*       cpssDxChBrgFdbFuMsgBlockGet, cpssDxChBrgFdbFuMsgBlockGet
*
*/
GT_STATUS auqFuqMessagesNumberGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  MESSAGE_QUEUE_ENT             queueType,
    OUT GT_U32                       *numOfAuPtr,
    OUT GT_BOOL                      *endOfQueueReachedPtr
);

/**
* @internal auDesc2UpdMsg function
* @endinternal
*
* @brief   This function is called whenever an address update message is received.
*         It translates the descriptor into CPSS_MAC_UPDATE_MSG_EXT_STC format
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - number of device whose descriptor queue is processed
* @param[in] portGroupId              - the  - for multi-port-groups support
* @param[in] auDescExtPtr             - (pointer to) the AU descriptor filled by PP.
* @param[in] auDescrFromDma           - AU descriptor is from DMA queue
*                                      GT_TRUE - AU descriptor is from DMA queue
*                                      GT_FALSE - AU descriptor is from FIFO but not DMA queue
*
* @param[out] addrUpMsgPtr             - MAC format of entry
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS auDesc2UpdMsg
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  PRV_CPSS_AU_DESC_EXT_8_STC    *auDescExtPtr,
    IN  GT_BOOL                       auDescrFromDma,
    OUT CPSS_MAC_UPDATE_MSG_EXT_STC   *addrUpMsgPtr
);

/**
* @internal prvCpssDxChBrgFdbAuFuMessageToCpuOnNonLocalMaskEnableSet function
* @endinternal
*
* @brief   Set the field mask for 'MAC entry does NOT reside on the local port group'
*         When Enabled - AU/FU messages are not sent to the CPU if the MAC entry
*         does NOT reside on the local portgroup, i.e.
*         the entry port[5:4] != LocalPortGroup
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - mask enable/disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on bad device number.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device (device is not multi-port groups device)
*
* @note none
*
*/
GT_STATUS prvCpssDxChBrgFdbAuFuMessageToCpuOnNonLocalMaskEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal prvCpssDxChBrgFdbAuFuMessageToCpuOnNonLocalMaskEnableGet function
* @endinternal
*
* @brief   Get the field mask for 'MAC entry does NOT reside on the local port group'
*         When Enabled - AU/FU messages are not sent to the CPU if the MAC entry
*         does NOT reside on the local core, i.e.
*         the entry port[5:4] != LocalPortGroup
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) mask enable/disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on bad device number.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device (device is not multi-port groups device)
*
* @note none
*
*/
GT_STATUS prvCpssDxChBrgFdbAuFuMessageToCpuOnNonLocalMaskEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal prvDxChBrgFdbAuFuMsgBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of AU / FU messages , the max number
*         of elements defined by the caller
*         for multi-port groups device :
*         AU messages -
*         1. Unified FDB mode:
*         portGroup indication is required , for:
*         a. Aging calculations of Trunks
*         b. Aging calculations of regular entries when DA refresh is enabled.
*         In these 2 cases entries are considered as aged-out only if AA is
*         receives from all 4 portGroups.
*         2. In Unified-Linked FDB mode and Linked FDB 128K mode:
*         portGroup indication is required for these reasons:
*         a. Aging calculations of Trunk and regular entries which have
*         been written to multiple portGroups.
*         b. New addresses (NA) source portGroup indication is required
*         so application can add new entry to the exact portGroup
*         which saw this station (especially on trunk entries).
*         c. Indication on which portGroup has removed this address when
*         AA of delete is fetched (relevant when same entry was written
*         to multiple portGroups).
*         d. Indication on which portGroup currently has this address
*         when QR (query reply) is being fetched (relevant when same
*         entry was written to multiple portGroups).
*         the function fetch AU messages from the queues in a round robin manner
*         (in steps of one entry).the function store which queue was last checked,
*         such that next message should be fetched from the next queue that
*         hold messages until the number of requested messages could be
*         supplied to application.
*         As one can understand, algorithm described above might create
*         undesired functional behaviors for the application:
*         1. Out-of-order messages -
*         since the fetching of messages from the 4 portGroups, is not
*         synchronized with messages' arrival order, possibly, application
*         would treat the updates not in the right order -
*         for example station X has been relocated to location A and then
*         location B - this will create 2 NA messages, NA with location A and
*         then NA with location B, if we'll treat these messages in wrong
*         order - we'll end up configuring station to location B and then A.
*         (specifically this example is not so bad cause even we do set station
*         with location A additional NA would be sent repeating location B
*         until rightly configured).
*         Note that there is no WA that can fix this undesired behavior.
*         FU messages -
*         since FU is triggered on all port groups , 'duplicated' messages may
*         appear on entries that reside on several/all port groups.
*         the function fetch FU messages from the queues in a round robin manner
*         (in steps of one entry).the function store which queue was last checked,
*         such that next message should be fetched from the next queue that
*         hold messages until the number of requested messages could be
*         supplied to application.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number from which AU / FU are taken
* @param[in] queueType                - AUQ or FUQ. FUQ valid for DxCh2 and above.
* @param[in,out] numOfAuFuPtr             - (pointer to)actual number of AU / FU messages that
*                                      were received
*
* @param[out] auFuMessagesPtr          - array that holds received AU / FU messages
*                                      pointer is allocated by the caller , with enough memory
*                                      for numOfAuPtr entries that was given as INPUT
*                                      parameter.
* @param[out] hwAuMessagesPtr          - array that holds received AU messages in HW format
*                                      pointer is allocated by the caller , with enough memory
*                                      for numOfAuPtr entries that was given as INPUT
*                                      parameter.
* @param[out] portGroupIdPtr           - array that holds portGroupIds, from where AU messages received.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       AU messages
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - not all results of the CNC block upload
*                                       retrieved from the common used FU and CNC
*                                       upload queue
*/
GT_STATUS prvDxChBrgFdbAuFuMsgBlockGet
(
    IN      GT_U8                        devNum,
    IN      MESSAGE_QUEUE_ENT            queueType,
    INOUT   GT_U32                      *numOfAuFuPtr,
    OUT     CPSS_MAC_UPDATE_MSG_EXT_STC *auFuMessagesPtr,
    OUT     PRV_CPSS_AU_DESC_EXT_8_STC  *hwAuMessagesPtr,
    OUT     GT_U32                      *portGroupIdPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChBrgFdbAuh */

