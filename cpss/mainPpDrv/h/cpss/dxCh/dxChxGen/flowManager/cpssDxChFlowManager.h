/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file cpssDxChFlowManager.h
*
* @brief Structures, enums and CPSS APIs declarations for Flow Manager.
*
* @version   1
*********************************************************************************
**/

#ifndef __cpssDxChFlowManagerh
#define __cpssDxChFlowManagerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>

/* maximal size of pcl udb key */
#define CPSS_FLOW_MANAGER_MAX_KEY_LEN_CNS 60

/**
* @struct CPSS_DXCH_FLOW_MANAGER_FLOW_COUNTER_STC
 *
 * @brief The flow counter structure.
*/
typedef struct{

    /** @brief The unique flow identifier for every IPFIX flow
     *  in the CPSS DB.
     */
    GT_U32 flowId;

    /** @brief Byte count of the IPFIX flow in the CPSS DB.
     */
    GT_UL64 byteCount;

    /** @brief Packet count of the IPFIX flow in the CPSS DB.
     */
    GT_UL64 packetCount;

    /** @brief Drop count of the ipfix flow in the CPSS DB.
     */
    GT_UL64 dropCount;

    /** @brief Time stamp when flow was learned. 64 bits including
     *  LSB 32 bits as nano second timer and MSB 32 bits as second timer.
     */
    GT_UL64 firstTimeStamp;

    /** @brief Time stamp of the last packet belonging to a specific
     *  flow. 64 bits including LSB 32 bits as nano second timer and MSB 32 bits
     *  as second timer.
     */
    GT_UL64 lastTimeStamp;

} CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC;

/**
* @struct CPSS_DXCH_FLOW_MANAGER_FLOW_STC
 *
 * @brief Message structure for new flow.
*/
typedef struct{

    /** @brief Unique flow identifier for every IPFIX flow.
     */
    GT_U32 flowId;

    /** @brief Bit 16-23 of Word1 represents Key Size.
     */
    GT_U8  keySize;

    /** @brief Bit 24-31 of Word1 represents Key ID.
     */
    GT_U8  keyId;

    /** @brief Flow Key Content.
     */
    GT_U8  keyData[CPSS_FLOW_MANAGER_MAX_KEY_LEN_CNS];

} CPSS_DXCH_FLOW_MANAGER_FLOW_STC;

/*******************************************************************************
* CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC
*
* DESCRIPTION:
*       Application callback function used by the Ipfix Flow Manager to notify
*       about the newly learnt ipfix flows.
*
* INPUTS:
*       devNum           - device number
*       flowMngId        - unique flow manager id
*                         (APPLICABLE RANGES: 0..127)
*       newFlow          - points to the new flow based structure
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK             - on success
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC)
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              flowMngId,
    IN  CPSS_DXCH_FLOW_MANAGER_FLOW_STC     *newFlow
);

/*******************************************************************************
* CPSS_DXCH_FLOW_MANAGER_AGED_FLOW_CB_FUNC
*
* DESCRIPTION:
*       Application callback function used by the Flow Manager to notify
*       about the aged ipfix flows.
*
* INPUTS:
*       flowMngId           - unique flow manager id
*       flowIndex           - flow index which is aged out from cpss db.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK             - on success
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_DXCH_FLOW_MANAGER_AGED_FLOW_CB_FUNC)
(
    IN  GT_U32          flowMngId,
    IN  GT_U32          flowIndex
);

/**
* @enum CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT
 *
 * @brief Enumeration for key sizes for flows.
*/
typedef enum{

    /** @brief Flow key size to be used 20 Bytes
    */
    CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_20B_E = 20,

    /** @brief Flow key size to be used 30 Bytes
    */
    CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E = 30,

    /** @brief Flow key size to be used 60 Bytes
    */
    CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_60B_E = 60

} CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT;

/**
* @enum CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT
 *
 * @brief Enumeration for anchor types.
*/
typedef enum{

    /** @brief L2 anchor type for flow classification
    */
    CPSS_DXCH_FLOW_MANAGER_ANCHOR_L2_E = 0,

    /** @brief L3 anchor type for flow classification
    */
    CPSS_DXCH_FLOW_MANAGER_ANCHOR_L3_E,

    /** @brief L4 anchor type for flow classification
    */
    CPSS_DXCH_FLOW_MANAGER_ANCHOR_L4_E,

    /** @brief Source Port anchor type for flow classification
    */
    CPSS_DXCH_FLOW_MANAGER_ANCHOR_SRC_PORT_E,

    /** @brief Fixed anchor type for flow classification
    */
    CPSS_DXCH_FLOW_MANAGER_ANCHOR_FIXED_E,

    /** @brief Not used anchor type for flow classification
    */
    CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_USED_E,

    /** @brief Not valid anchor type for flow classification
    */
    CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_VALID_E,

   /** @brief VLAN ID anchor type for flow classification
    *  OffsetOrPattern value 0 includes UP + CFI + 4MSB of VLAN ID 12 bits
    *  OffsetOrPattern value 1 includes 8LSB of VLAN ID 12 bits
   */
    CPSS_DXCH_FLOW_MANAGER_ANCHOR_VID_E

} CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT;

/**
* @enum CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_ENT
 *
 * @brief Enumeration for Flow packet types.
*/
typedef enum{

    /** @brief IPV4 TCP packet type for flows
    */
    CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E = 0,

    /** @brief IPV4 UDP packet type for flows
    */
    CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E,

    /** @brief IPV6 TCP packet type for flows
    */
    CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E,

    /** @brief IPV6 UDP packet type for flows
    */
    CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E,

    /** @brief Packet type others for flows
    */
    CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E,

    /** @brief End of packet type enumerator
    */
    CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E

} CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_ENT;

/**
* @struct CPSS_DXCH_FW_CFG_STC
 *
 * @brief FW initialization parameters structure.
*/
typedef struct{

    /** @brief The Rx SDMA queue number used for capturing
     *  new flows to the FW.
     */
    GT_U32 rxSdmaQ;

    /** @brief The Tx SDMA queue number used for passing
     *  message to the application.
     */
    GT_U32 txSdmaQ;

    /** @brief User defined cpu code to identify the
     *  packets received at the application.
     */
    CPSS_NET_RX_CPU_CODE_ENT messagePacketCpuCode;

    /** @brief User defined cpu code to identify the ipv4
     *  packets received by the FW for lookup.
     */
    CPSS_NET_RX_CPU_CODE_ENT cpuCodeStart;

    /** @brief The logical start index of the TCAM area
     *  reserved for the FW.
     */
    GT_U32 tcamStartIndex;

    /** @brief The start index of the IPFIX area reserved
     *  for the FW.
     */
    GT_U32 ipfixStartIndex;

    /** @brief The reduced rule size for the ISF sent from FW for every new
     *  flow. It denotes the actual number of bytes which influence the match
     *  and can be smaller than the actual PCL flow key size 20B/30B/60B since
     *  some UDBs are not used. Reduced size is achieved due to the design that
     *  each key starts with 2B PCL-ID which FW does not have and so
     *  effectively each flow key size gets reduced to 2B lesser than the
     *  actual flow key size. Further the bytes regarded as NOT_VALID anchor
     *  types are not considered for reduced size.
     *  (APPLICABLE RANGES: Flow Key Size   Reduced Rule Size)
     *                            20B          (1-18)B
     *                            30B          (1-28)B
     *                            60B          (1-58)B
     */
    GT_U32 reducedRuleSize;

    /** @brief TCAM size reserved for FW in chunks of short
     *  rules size i.e. 20/30B.
     */
    GT_U32 tcamNumOfShortRules;

    /** @brief Size of short rule 20/30B for ipv4 packets.
     */
    CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT tcamShortRuleSize;

    /** @brief FW Flow DB size in chunks of short rules 20B/30B.
     * Eg: If value is 6K, than up to 6K IPv4 flows can be maintained in the FW
     * DB.
     */
    GT_U32 flowDBSize;

    /** @brief FW flow DB size in chunks of short rules that
     *  will be used for short rules 20B/30B.
     *  Eg: If value of whole DB is 8K and this parameter is 6K, than upto 6K
     *  IPv4 flows can be maintained and for long rules IPv6 we can have 1K or
     *  666 rules depending if short rule size is 20B/30B
     */
    GT_U32 flowShortDBSize;

} CPSS_DXCH_FLOW_MANAGER_FW_CFG_STC;

/**
* @struct CPSS_DXCH_FLOW_MANAGER_STC
 *
 * @brief FW initialization parameters structure.
*/
typedef struct{

    /** @brief The FW configuration specific parameters.
     */
    CPSS_DXCH_FLOW_MANAGER_FW_CFG_STC           cpssFwCfg;

    /** @brief CPSS Flow DB entries.
     */
    GT_U32                                      flowDbSize;

    /** @brief Application callback to notify about the newly
     *  leanrt flows..
     */
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC     appCbFlowNotificaion;

    /** @brief Callback to notify application about aged flows.
     */
    CPSS_DXCH_FLOW_MANAGER_AGED_FLOW_CB_FUNC    appCbOldFlowRemovalNotificaion;


} CPSS_DXCH_FLOW_MANAGER_STC;

/**
* @struct PRV_CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC
 *
 * @brief Flow Manager structure to track flows learnt per source port.
*/
typedef struct{

    /** @brief To track ipv4 tcp flows leant.
     */
    GT_U32  ipv4TcpFlowsLearnt;

    /** @brief To track ipv4 udp flows leant.
     */
    GT_U32  ipv4UdpFlowsLearnt;

    /** @brief To track ipv6 tcp flows leant.
     */
    GT_U32  ipv6TcpFlowsLearnt;

    /** @brief To track ipv6 udp flows leant.
     */
    GT_U32  ipv6UdpFlowsLearnt;

    /** @brief To track other packet type flows leant.
     */
    GT_U32  otherTypeFlowsLearnt;

} CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC;

/**
* @struct CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_STATISTICS_STC
 *
 * @brief Flow Manager message packet statistics structure.
*/
typedef struct{

    /** @brief To track the count of new flows received in message packet..
     */
    GT_U32  messageNewFlowCount;

    /** @brief To track the count of removed flows received in message packet.
     */
    GT_U32  messageRemovedFlowCount;

    /** @brief To track the count of flow data get response in received message packet.
     */
    GT_U32  messageFlowDataGetCount;

    /** @brief To track the count of re-sync.
     */
    GT_U32  reSyncCount;

    /** @brief To track the message packet sequence number.
     */
    GT_U32  sequenceNumber;

    /** @brief To track the count of new flows received in message packet post
     *  re-sync.
     */
    GT_U32  messageNewFlowCountAfterResync;

    /** @brief To track the count of removed flows received in message packet
     *  post re-sync.
     */
    GT_U32  messageRemovedFlowCountAfterResync;

    /** @brief To track the message packet sequence number causing re-sync.
     */
    GT_U32  messageSequenceNumberForResync;

    /** @brief To track the total count of new flows added in DB.
     */
    GT_U32  dbAddedFlowCount;

    /** @brief To track the total count of removed flows from DB.
     */
    GT_U32  dbRemovedFlowCount;

} CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_STATISTICS_STC;

/**
* @internal cpssDxChFlowManagerCreate function
* @endinternal
*
* @brief   Create flow manager and configure the required parameters to FW inorder to assist
           the FW to start the flow manager service.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId              - unique flow manager id.
* @param[in] flowMngInfoPtr         - ptr to flow manager configuration structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_ALREADY_EXIST         - if already exist Flow Manager with given Id
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_CPU_MEM        - on CPU memory allocation fail
*/
GT_STATUS cpssDxChFlowManagerCreate
(
    IN GT_U32                        flowMngId,
    IN CPSS_DXCH_FLOW_MANAGER_STC    *flowMngInfoPtr
);

/**
* @internal cpssDxChFlowManagerDevListAdd function
* @endinternal
*
* @brief   Adds devices to the existing flow manager.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                - unique flow manager id.
* @param[in] devListArr[]             - array of device list associated to flow manager.
* @param[in] numberOfDevs             - number of devices in system.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Flow Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
*/
GT_STATUS cpssDxChFlowManagerDevListAdd
(
    IN GT_U32                           flowMngId,
    IN GT_U8                            devListArr[], /*arrSizeVarName=numberOfDevs*/
    IN GT_U32                           numberOfDevs
);

/**
* @internal cpssDxChFlowManagerDevListRemove function
* @endinternal
*
* @brief   Remove devices from the existing flow manager.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                - unique flow manager id.
* @param[in] devListArr[]             - array of device list associated to flow manager.
* @param[in] numberOfDevs             - number of devices in system.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Flow Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_NOT_INITIALIZED       - if there are no devices in DB
* @retval GT_BAD_STATE             - on no devices in DB to remove
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChFlowManagerDevListRemove
(
    IN GT_U32                           flowMngId,
    IN GT_U8                            devListArr[], /*arrSizeVarName=numberOfDevs*/
    IN GT_U32                           numberOfDevs
);

/**
* @internal cpssDxChFlowManagerDelete function
* @endinternal
*
* @brief   Delete the flow manager.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId             - unique flow manager id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on devices still attached to Flow manager
* @retval GT_NOT_FOUND             - on Flow manager not found
*/
GT_STATUS cpssDxChFlowManagerDelete
(
    IN GT_U32                           flowMngId
);

/**
* @internal cpssDxChFlowManagerMessageParseAndCpssDbUpdate function
* @endinternal
*
* @brief   Parse the received frames and update the CPSS DB accordingly.
*          Also notify the application about the newly learnt flows
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] numOfBuff                - Num of buffs in packetBuffsArrPtr.
* @param[in] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[in] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssDxChFlowManagerMessageParseAndCpssDbUpdate
(
    IN    GT_U8                                      devNum,
    IN    GT_U32                                     numOfBuff,
    IN    GT_U8                                      *packetBuffsArrPtr[], /*arrSizeVarName=numOfBuff*/
    IN    GT_U32                                     buffLenArr[] /*arrSizeVarName=numOfBuff*/
);

/**
* @internal cpssDxChFlowManagerEnableSet function
* @endinternal
*
* @brief   Enabling Flow Manager feature
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                - unique flow manager id
*                                      (APPLICABLE RANGES: 0..127)
* @param[in] enable                   - GT_TRUE  - enable Flow Manager feature
*                                       GT_FALSE - disable Flow Manager feature
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_FOUND             - on Flow manager not found
* @retval GT_BAD_VALUE             - on illegal value returned by FW
*/
GT_STATUS cpssDxChFlowManagerEnableSet
(
    IN  GT_U32   flowMngId,
    IN  GT_BOOL  enable
);

/**
* @internal cpssDxChFlowManagerEnableGet function
* @endinternal
*
* @brief   Indicates if DBA feature is enabled
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                 - unique flow manager id
*                                       (APPLICABLE RANGES: 0..127)
* @param[out] enablePtr                - (pointer to) GT_TRUE  - Flow Manager feature enabled
*                                        GT_FALSE - no Flow Manager feature support (or disabled)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_FOUND             - on Flow manager not found
*/
GT_STATUS cpssDxChFlowManagerEnableGet
(
    IN  GT_U32   flowMngId,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChFlowManagerKeyEntryUserDefinedByteSet function
* @endinternal
*
* @brief Set the anchor type, offset and mask at given udb index for key table index.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId               - flow manager id
*                                      (APPLICABLE RANGES: 0..127)
* @param [in] keyTableIndex           - key table index
* @param [in] keySize                 - key size
* @param [in] udbIndex                - udb index
* @param [in] offsetType              - the type of offset
* @param [in] offset                  - the offset of UDB in the offset type
* @param [in] mask                    - mask to use to match the udb fields
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
*/
GT_STATUS cpssDxChFlowManagerKeyEntryUserDefinedByteSet
(
    IN GT_U32                                  flowMngId,
    IN GT_U32                                  keyTableIndex,
    IN CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT     keySize,
    IN GT_U32                                  udbIndex,
    IN CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT       offsetType,
    IN GT_U8                                   offsetOrPattern,
    IN GT_U8                                   mask
);

/**
* @internal cpssDxChFlowManagerKeyEntryUserDefinedByteGet function
* @endinternal
*
* @brief   Get the UDB fields programmed to the FW for the given key and udb index.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                - flow manager id.
* @param[in] keyTableIndex            - key table index.
* @param[in] udbIndex                 - udb index.
* @param[out]keySizePtr               - (pointer to) PCL key size.
* @param[out]offsetTypePtr            - (pointer to) The type of offset.
* @param[out]offsetPtr                - (pointer to) The offset of UDB, in
*                                       bytes, from the place in packet
*                                       indicated by offset type.
* @param[out]maskPtr                  - (pointer to) The mask type.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_OUT_OF_RANGE          - parameter is out of range.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssDxChFlowManagerKeyEntryUserDefinedByteGet
(
    IN  GT_U32                                     flowMngId,
    IN  GT_U32                                     keyTableIndex,
    IN  GT_U32                                     udbIndex,
    OUT CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT        *keySizePtr,
    OUT CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT          *offsetTypePtr,
    OUT GT_U8                                      *offsetPtr,
    OUT GT_U8                                      *maskPtr
);

/**
* @internal cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet function
* @endinternal
*
* @brief Set the source port, packet type mapping to the key index.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  devNum                - device number.
* @param [in] sourcePort            - source port number
* @param [in] keyIndexArr           - array of key index based on packet type, index to packet mapping
*                                     index    packet type
*                                       0       IPV4 TCP
*                                       1       IPV4 UDP
*                                       2       IPV6 TCP
*                                       3       IPV6 UDP
*                                       4        OTHER
*                                     APPLICABLE RANGES 0...115
* @param [in] enableBitmap          - enable/disable featur bitmap per packet type per port
*                                       BIT 0 - IPV4 TCP
*                                       BIT 1 - IPV4 UDP
*                                       BIT 2 - IPV6 TCP
*                                       BIT 3 - IPV6 UDP
*                                       BIT 4 - OTHER
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
*/
GT_STATUS cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet
(
    IN GT_U8          devNum,
    IN GT_PORT_NUM    sourcePort,
    IN GT_U32         keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E],
    IN GT_U32         enableBitmap
);

/**
* @internal cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet function
* @endinternal
*
* @brief Get the key index based on the source port.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]   devNum                - device number.
* @param [in]  sourcePort            - source port number
* @param [out] keyIndexArr           - key index array per packet type
* @param [out] enableBitmapPtr       - bitmap ptr for enable/disable per packet type
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
*/
GT_STATUS cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet
(
    IN GT_U8            devNum,
    IN  GT_PORT_NUM     sourcePort,
    OUT GT_U32          keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E],
    OUT GT_U32          *enableBitmapPtr
);

/**
* @internal cpssDxChFlowManagerFlowCountersToDbGet function
* @endinternal
*
* @brief To send flow counters get request to FW to retrieve the IPFIX HW counters regularly and
*        store it in CPSS DB for further usage of application.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId             - unique flow manager id
*                                    (APPLICABLE RANGES: 0..127)
* @param [in] flowIdCount           - number of flow ids for which counters is requested
* @param [in] flowIdArr             - array of flow ids for which counter is requested from FW
*
* @retval GT_OK                    - on success
* @retval GT_TIMEOUT               - on timeout
* @retval GT_FAIL                  - on error
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
* @retval GT_NO_MORE               - no messages ready
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssDxChFlowManagerFlowCountersToDbGet
(
    IN  GT_U32   flowMngId,
    IN  GT_U32   flowIdCount,
    IN  GT_U32   flowIdArr[] /*arrSizeVarName=flowIdCount*/
);

/**
* @internal cpssDxChFlowManagerFlowDataGet function
* @endinternal
*
* @brief To get flow data for an existing flow in cpss db.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in]  flowMngId             - unique flow manager id
*                                     (APPLICABLE RANGES: 0..127)
* @param [in]  flowId                - flow id for which data is needed
* @param [out] flowEntryPtr          - ptr to flow entry structure
* @param [out] flowDataPtr           - ptr to flow data structure
*
* @retval GT_OK                      - on success.
* @retval GT_BAD_PARAM               - one of the input parameters is not valid.
* @retval GT_NOT_FOUND               - if the Flow Manager DB is not found.
* @retval GT_NO_SUCH                 - on no such flow id exists.
* @retval GT_BAD_PTR                 - on NULL pointer.
*/
GT_STATUS cpssDxChFlowManagerFlowDataGet
(
    IN  GT_U32                                  flowMngId,
    IN  GT_U32                                  flowId,
    OUT CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC   *flowEntryPtr,
    OUT CPSS_DXCH_FLOW_MANAGER_FLOW_STC         *flowDataPtr
);

/**
* @internal cpssDxChFlowManagerFlowIdGetNext function
* @endinternal
*
* @brief To get the flow id from the list of flow ids maintained in cpss db. The flow ids are maintained
*        in the list based on their insertion time order and the get request can be either from the
*        beginning of the list or can continue from where it last left.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId             - unique flow manager id
*                                     (APPLICABLE RANGES: 0..127)
* @param[in]  getFirst              - GT_TRUE - get first flow id
*                                    GT_FALSE - get subsequent flow ids
* @param [out]nextFlowIdPtr         - retrieve the existing flow id
*
* @retval GT_OK                      - on success.
* @retval GT_BAD_PARAM               - one of the input parameters is not valid.
* @retval GT_NOT_FOUND               - if the Flow Manager DB is not found.
* @retval GT_BAD_PTR                 - on NULL pointer.
* @retval GT_NO_MORE                 - no flow id exists in DB.
*/
GT_STATUS cpssDxChFlowManagerFlowIdGetNext
(
    IN  GT_U32  flowMngId,
    IN  GT_BOOL getFirst,
    OUT GT_U32  *nextFlowIdPtr
);

/**
* @internal cpssDxChFlowManagerSrcPortFlowLearntGet function
* @endinternal
*
* @brief To get flows learnt per source port for statistics/monitoring purpose.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in]  devNum                - unique device number
* @param [in]  sourcePort            - source port number
* @param [out] flowsLearntPtr        - (pointer to) Number of learnt flows per packet type
*
* @retval GT_OK                      - on success.
* @retval GT_BAD_PARAM               - one of the input parameters is not valid.
* @retval GT_NOT_FOUND               - if the Flow Manager DB is not found.
* @retval GT_BAD_PTR                 - on NULL pointer.
*/
GT_STATUS cpssDxChFlowManagerSrcPortFlowLearntGet
(
    IN  GT_U8                                                devNum,
    IN  GT_PORT_NUM                                          sourcePort,
    OUT CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC         *flowsLearntPtr
);

/**
* @internal cpssDxChFlowManagerSrcPortFlowIdGetNext function
* @endinternal
*
* @brief To get flows id learnt for the given source port from the list of flow ids maintained in cpss db.
*        The flow ids are maintained in form of list in insertion time based order and the request can be
*        either from the beginning of the list or can continue from where it last left.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat     3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                  - unique device number
* @param[in] sourcePort              - source port number
* @param[in] getFirst                - GT_TRUE - get first flow id
*                                      GT_FALSE - get subsequent flow ids
* @param[in] flowIdPtr               - (pointer to) flow ids learnt per source port
*
* @retval GT_OK                      - on success.
* @retval GT_BAD_PARAM               - one of the input parameters is not valid.
* @retval GT_NOT_FOUND               - if the Flow Manager DB is not found.
* @retval GT_BAD_PTR                 - on NULL pointer.
* @retval GT_NO_MORE                 - on no more flow ids present.
*/
GT_STATUS cpssDxChFlowManagerSrcPortFlowIdGetNext
(
    IN    GT_U8                                   devNum,
    IN    GT_PORT_NUM                             sourcePort,
    IN    GT_BOOL                                 getFirst,
    OUT   GT_U32                                  *flowIdPtr
);

/**
* @internal cpssDxChFlowManagerMessagePacketStatisticsGet function
* @endinternal
*
* @brief   Debug api to get all the message packet related statistics.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId             - flow manager id.
* @param[out]messagePacketStatsPtr - (pointer to) message packet statistics structure.
*
* @retval GT_OK                      - on success.
* @retval GT_BAD_PARAM               - one of the input parameters is not valid.
* @retval GT_NOT_FOUND               - if the Flow Manager DB is not found.
* @retval GT_BAD_PTR                 - on NULL pointer.
*/
GT_STATUS cpssDxChFlowManagerMessagePacketStatisticsGet
(
    IN  GT_U32                                                flowMngId,
    OUT CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_STATISTICS_STC  *messagePacketStats
);

/**
* @internal cpssDxChFlowManagerReSync function
* @endinternal
*
* @brief   Re-Sync CPSS DB with FW DB due to sequence number miss.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId             - flow manager id.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_FOUND             - on Flow manager not found
* @retval GT_BAD_VALUE             - on illegal value returned by FW
*/
GT_STATUS cpssDxChFlowManagerReSync
(
    IN GT_U32 flowMngId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChFlowManagerh */

