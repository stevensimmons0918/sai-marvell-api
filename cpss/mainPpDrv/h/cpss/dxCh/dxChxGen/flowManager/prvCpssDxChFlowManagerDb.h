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
* @file prvCpssDxChFlowManagerDb.h
*
* @brief The CPSS DXCH Flow Manager internal batabase
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChFlowManagerDb_h
#define __prvCpssDxChFlowManagerDb_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/flowManager/cpssDxChFlowManager.h>

/* maximal amount of Flow Managers - Id range 0..127 */
#define PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS 127

/* maximal amount of flows in cpss flow db */
#define PRV_CPSS_DXCH_FLOW_DB_MAX_CNS (_64K)

/* maximal hash list size */
#define PRV_NUM_OF_HASH_LISTS_CNS 8192

/* maximal flow key size */
#define PRV_CPSS_DXCH_FLOW_KEY_SIZE_CNS 60

/* maximal key table index */
#define PRV_CPSS_DXCH_KEY_TABLE_INDEX_CNS 16

/* maximal fw flow indexes for aldrin2 device */
#define MAX_NUM_OF_FW_FLOW_INDEX_CNS _8K

/* Invalid Flow Index */
#define PRV_CPSS_FLOW_MANAGER_INVALID_FLOW_INDEX_CNS 0xFFFFFFFF

/* bitmap of devices used by Flow Manager */
typedef GT_U32 PRV_CPSS_DXCH_FLOW_MNG_DEVS_BITMAP[PRV_CPSS_MAX_PP_DEVICES_CNS/32];

/* macro PRV_CPSS_FLOW_MNG_DEV_BMP_SET_MAC
    to set a device in bitmap of devices in the FLOW manager database

  devsBitmap - of type PRV_CPSS_DXCH_FLOW_MNG_DEVS_BITMAP
                device bitmap

  devNum - the device num to set in the device bitmap
*/
#define PRV_CPSS_FLOW_MNG_DEV_BMP_SET_MAC(devsBitmap, devNum)   \
    (devsBitmap)[(devNum)>>5] |= 1 << ((devNum) & 0x1f)

/* macro PRV_CPSS_FLOW_MNG_IS_DEV_BMP_SET_MAC
 * to check if device is set in the bitmap of devices in the flow manager
 * database
 *
 * devsBitmap - of type PRV_CPSS_DXCH_FLOW_MNG_DEVS_BITMAP
 *              device bitmap
 *
 * devNum - the device num to set in the device bitmap
 *
 * return 0 -- device not set in bmp
 * return 1 -- device set in bmp
 */
#define PRV_CPSS_FLOW_MNG_IS_DEV_BMP_SET_MAC(devsBitmap, devNum)   \
    (((devsBitmap)[(devNum)>>5] & (1 << ((devNum) & 0x1f))) ? 1 : 0)

/* macro PRV_CPSS_FLOW_MNG_DEV_BMP_CLEAR_MAC
 * to clear a device from the bitmap of devices in the FLOW manager
 * database
 *
 * devsBitmap - of type PRV_CPSS_DXCH_FLOW_MNG_DEVS_BITMAP
 *              device bitmap
 *
 * devNum - the device num to set in the device bitmap
 */
#define PRV_CPSS_FLOW_MNG_DEV_BMP_CLEAR_MAC(devsBitmap, devNum)   \
    (devsBitmap)[(devNum)>>5] &= ~(1 << ((devNum) & 0x1f))

/**
* @struct PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC
 *
 * @brief Flow management structure.
*/
typedef struct PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC{

    /** @brief IPFIX Flow index in CPSS Flow DB.
    */
    GT_U32 cpssFlowIndex;

    /** @brief Next ptr for Used and Free linked list.
     */
    struct PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC *next;

    /** @brief Prev ptr for Used and Free linked list.
     */
    struct PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC *prev;

    /** @brief Next ptr for Hash list to track next hash index.
     */
    struct PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC *nextInHashList;

    /** @brief Prev ptr for Hash list to track prev hash index.
     */
    struct PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC *prevInHashList;

} PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC;

/**
* @struct PRV_CPSS_DXCH_FLOW_MANAGER_UDB_KEY_STC
 *
 * @brief Flow Manager UDB key configuration structure.
*/
typedef struct {

    /** @brief The triplet <anchor type, offset, mask> is used to derive
     *  the flow key by FW configured by cpss/application and will be
     *  upto keySize length i.e. a short tcam rule of size 20B can have
     *  upto 20 triplets combination.
     */
    CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT anchor;

    /** @brief Explanation as above for anchor type.
     */
    GT_U8                             offsetOrPattern;

    /** @brief Explanation as above for anchor type.
     */
    GT_U8                             mask;

} PRV_CPSS_DXCH_FLOW_MANAGER_UDB_KEY_STC;

/**
* @struct PRV_CPSS_DXCH_FLOW_MANAGER_KEY_TABLE_CFG_STC
 *
 * @brief Flow Manager key table configuration structure.
*/
typedef struct{

    /** @brief PCL key size in bytes.
     */
    CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_ENT keySize;

    /** @brief Triplet <anchor, offset, mask> configuration structure.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_UDB_KEY_STC content[CPSS_FLOW_MANAGER_MAX_KEY_LEN_CNS];

} PRV_CPSS_DXCH_FLOW_MANAGER_KEY_TABLE_CFG_STC;

/**
* @struct PRV_CPSS_DXCH_FLOW_MANAGER_KEY_INDEX_MAPPING_STC
 *
 * @brief Flow Manager packet type to key index mapping structure.
*/
typedef struct{

    /** @brief Key index to packet type mapping.
     */
    GT_U8          keyIndexArr[PRV_CPSS_DXCH_KEY_TABLE_INDEX_CNS];

    /** @brief Feature bitmap per packet type per port.
     */
    GT_U8          enableBitmap;

} PRV_CPSS_DXCH_FLOW_MANAGER_KEY_INDEX_MAPPING_STC;

/**
* @struct PRV_CPSS_DXCH_FLOW_MANAGER_SOURCE_PORT_TO_DEV_NUM_MAPPING_STC
 *
 * @brief Flow Manager soource port to dev num mapping for key index
 * configuration.
*/
typedef struct{

    /** @brief IPC object used for communicationg with service cpu.
     */
    GT_UINTPTR                                       fwChannel;

    /** @brief Packet type to key index mapping.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_KEY_INDEX_MAPPING_STC keyIndexMapCfg[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];

} PRV_CPSS_DXCH_FLOW_MANAGER_SOURCE_PORT_TO_DEV_NUM_MAPPING_STC;

/**
* @struct PRV_CPSS_DXCH_FLOW_MANAGER_REMOVED_FLOW_COUNTER_STC
 *
 * @brief The flow counter structure for removed flows.
*/
typedef struct{

    /** @brief Byte count of the removed IPFIX flow in the CPSS DB.
     */
    GT_UL64 byteCount;

    /** @brief Packet count of the removed IPFIX flow in the CPSS DB.
     */
    GT_UL64 packetCount;

    /** @brief Drop count of the removed ipfix flow in the CPSS DB.
     */
    GT_UL64 dropCount;

} PRV_CPSS_DXCH_FLOW_MANAGER_REMOVED_FLOW_ENTRY_STC;

/**
* @struct PRV_CPSS_DXCH_FLOW_MANAGER_STC
 *
 * @brief Flow Manager structure.
*/
typedef struct{

    /** @brief Flow Manager Unique ID.
     */
    GT_U32                                            flowMngId;

    /** @brief Flow Manager Unique ID.
     */
    GT_BOOL                                           flowMngEnable;

    /** @brief Devices bitmap.
     */
    PRV_CPSS_DXCH_FLOW_MNG_DEVS_BITMAP                devsBitmap;

    /** @brief Number of devices in the system.
     */
    GT_U32                                            numOfDevices;

    /** @brief Device family type.
     */
    CPSS_PP_FAMILY_TYPE_ENT                           devFamily;

    /** @brief The FW configuration specific parameters.
     */
    CPSS_DXCH_FLOW_MANAGER_FW_CFG_STC                 cpssFwCfg;

    /** @brief CPSS Flow DB size.
     */
    GT_U32                                            flowDbSize;

    /** @brief Size of short rule 20/30B for ipv4 packets.
     */
    GT_U32                                            tcamShortRuleSize;

    /** @brief Flow data db for flows.
     */
    CPSS_DXCH_FLOW_MANAGER_FLOW_STC                   *flowDataDb;

    /** @brief Flow counter db for flows.
     */
    CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC             *flowCounterDb;

    /** @brief Flow counter db for removed flows.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_REMOVED_FLOW_ENTRY_STC *removedFlowCounterDb;

    /** @brief Flow hash for DB management of flows.
     */
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC                 *flowHashDb;

    /** @brief Hash list for flows stored in CPSS DB.
    */
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC                 *hashList[PRV_NUM_OF_HASH_LISTS_CNS];

    /** @brief List of all the free entries in DB.
    */
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC                 *cpssFlowFreeListHeader;

    /** @brief Used list entries header pointer.
    */
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC                 *cpssFlowUsedListHeader;

    /** @brief Used list entries tail pointer.
    */
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC                 *cpssFlowUsedListTail;

    /** @brief Flow index mapping from FW DB to CPSS DB.
    */
    GT_U32                                            *fwToCpssFlowDb;

    /** @brief Application callback to notify about the newly
     *  learnt flows..
     */
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC           appCbFlowNotificaion;

    /** @brief Callback to notify application about aged flows..
     */
    CPSS_DXCH_FLOW_MANAGER_AGED_FLOW_CB_FUNC          appCbOldFlowRemovalNotificaion;

    /** @brief Flag to denote flow state in CPSS DB with reference to FW DB.
    */
    GT_BOOL                                           *flowState;

    /** @brief Pointer to store complete packet received from FW.
    */
    GT_U8                                             *reassemblyBufferPtr;

    /** @brief Key table index based udb configuration for flows classification.
    */
    PRV_CPSS_DXCH_FLOW_MANAGER_KEY_TABLE_CFG_STC      keyTableUdbCfg[PRV_CPSS_DXCH_KEY_TABLE_INDEX_CNS];

    /** @brief Key table index mapping per source port per device.
    */
    PRV_CPSS_DXCH_FLOW_MANAGER_SOURCE_PORT_TO_DEV_NUM_MAPPING_STC *devInfo[PRV_CPSS_MAX_PP_DEVICES_CNS];

    /** @brief To track the count of new flows received in message packet.
    */
    GT_U32                                            messageNewFlowCount;

    /** @brief To track the count of removed flows received in message packet.
    */
    GT_U32                                            messageRemovedFlowCount;

    /** @brief To track the count of flow data get response in received message
     * packet.
    */
    GT_U32                                            messageFlowDataGetCount;

    /** @brief To track the count of re-sync.
    */
    GT_U32                                            reSyncCount;

    /** @brief Pointer to track the used list inorder to get the flow ids per
     * port.
    */
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC                 *usedListPtr;

    /** @brief Global Pointer to track the used list inorder to get the next flow ids.
    */
    PRV_CPSS_DXCH_FLOW_HASH_ENTRY_STC                 *globalUsedListPtr;

    /** @brief Free running number on cpss end to validate incoming isf sanity.
    */
    GT_U32                                            seqNoCpss;

    /** @brief To track the count of new flows received in message packet post
     *  re-sync.
    */
    GT_U32                                            messageNewFlowCountAfterResync;

    /** @brief To track the count of removed flows in message packet post
     *  re-sync.
    */
    GT_U32                                            messageRemovedFlowCountAfterResync;

    /** @brief To track the message packet sequence number causing re-sync.
    */
    GT_U32                                            messageSequenceNumberForResync;

    /** @brief To track the message packet sequence number processed at CPSS
     *  end and used at FW for BTL implementation.
    */
    GT_U32                                            lastReadIsfMsgSeqPtr;

    /** @brief To track the total count of new flows added in DB.
    */
    GT_U32                                       dbAddedFlowCount;

    /** @brief To track the total count of removed flows from DB.
    */
    GT_U32                                       dbRemovedFlowCount;

    /** @brief To enable debugging of ISF received frame.
    */
    GT_BOOL                                      debug_dumpIsfPacket;

} PRV_CPSS_DXCH_FLOW_MANAGER_STC;

typedef struct{

    /** @brief Specific opcode for individual FW init parameter.
     */
    char opcode;

    /** @brief FW init parameters value.
     */
    GT_U8 data[190];

} PRV_CPSS_DXCH_IPC_MSG_STC;

typedef struct{

      /** @brief FW ipc return value.
       */
      GT_U8 data[190];

} PRV_CPSS_DXCH_IPC_RECV_MSG_STC;

typedef struct{

    /** @brief Specific opcode for individual FW init parameter.
     */
    GT_U8  rc;

    /** @brief Specific opcode for individual FW init parameter.
     */
    GT_U8  size;

    /** @brief FW init parameters value.
     */
    GT_U8 data[190];

} PRV_CPSS_DXCH_IPC_FW_PTRS_RECV_MSG_STC;

typedef struct{

    /** @brief FW pointer corresponding to init configuration.
     */
    GT_U32 fwInitCfgPtr;

    /** @brief FW pointer corresponding to flow tuple entries.
     */
    GT_UINTPTR fwFlowTuplePtr;

    /** @brief FW pointer corresponding to flow counters entries.
     */
    GT_U32 fwFlowCntrPtr;

    /** @brief FW pointer corresponding to hash lists entries.
     */
    GT_U32 fwHashListPtr;

    /** @brief FW pointer correspoding to free list enrteis.
     */
    GT_U32 fwFreeListPtr;

    /** @brief FW pointer corresponding to used list entries.
     */
    GT_U32 fwUsedListPtr;

    /** @brief FW pointer corresponding to PCL key configuration.
     */
    GT_U32 fwPclKeyCfgPtr;

    /** @brief FW pointer corresponding to Port bind keys configuration.
     */
    GT_U32 fwPortBindKeyCfgPtr;

    /** @brief FW pointer corresponding to TCAM to FW DB mapping.
     */
    GT_U32 fwTcamToFwDbMappingPtr;

} PRV_CPSS_DXCH_FW_PTRS_ADDRESS_STC;

/* typedef for pointer to structure */
typedef PRV_CPSS_DXCH_FLOW_MANAGER_STC
    *CPSS_DXCH_FLOW_MNG_PTR;

/**
* @enum PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_ENT
 *
 * @brief Enumeration for IPC opcode for communicating with FW.
*/
typedef enum{

    /** @brief IPC opcode to configure FW related parameters.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_CONFIG_E,

    /** @brief IPC opcode to configure udb indexes for key index
     *  table to be used by FW for flow classification.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_SET_UDB_KEY_E,

    /** @brief IPC opcode to configure key index to port mapping for
     *  different flow packet types.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_SET_PORT_KEY_E,

    /** @brief IPC opcode to request flow data from FW.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_GET_FLOW_DATA_E,

    /** @brief IPC opcode to re-sync CPSS DB with FW DB once ISF
     *  packet is missed by host cpu.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_MESSAGE_PACKET_SYNC_E,

    /** @brief IPC opcode to start flow manager service.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_START_E,

    /** @brief IPC opcode to stop flow manager service.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_STOP_E,

    /** @brief IPC opcode to get FW pointers.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_GET_FW_PTRS_E

} PRV_CPSS_DXCH_FLOW_MANAGER_IPC_OP_CODE_ENT;

/**
* @enum PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_ENT
 *
 * @brief Enumeration for FW configration parameter type.
*/
typedef enum{

    /** @brief Message opcode to signify no more configuration parameters from
     *  Application.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_NO_MORE_E,

    /** @brief Message opcode to configure Rx SDMA queue for FW.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_RX_SDMA_QUEUE_E,

    /** @brief Message opcode to configure Tx SDMA queue for FW.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_TX_SDMA_QUEUE_E,

    /** @brief Message opcode to configure logical TCAM start index for FW.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_TCAM_START_INDEX_E,

    /** @brief Message opcode to configure IPFIX start index for FW.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_IPFIX_START_INDEX_E,

    /** @brief Message opcode to configure TCAM size for FW.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_TCAM_SIZE_E,

    /** @brief Message opcode to configure TCAM short rule size for flows.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_SHORT_RULE_SIZE_E,

    /** @brief Message opcode to configure reduced rule size for FW sent ISF
     *  for new flows.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_REDUCED_RULE_SIZE_E,

    /** @brief Message opcode to configure FW Flow DB size.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_FW_FLOW_DB_SIZE_E,

    /** @brief Message opcode to configure FW short DB size.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_FW_SHORT_DB_SIZE_E,

    /** @brief Message opcode to configure CPU code for FW to identify flow
     *  packet types.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_RX_CPU_CODE_START_E,

    /** @brief Message opcode to configure DSA tag to be used for sending
     *  message packets from FW to Application.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_TX_DSA_TAG_E,

    /** @brief Message opcode to configure MGCAM channel for FW to access TCAM
     *  and Policer tables.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_MGCAM_CHANNEL_E,

    /** @brief Message opcode to configure FW working mode - HW,
     *         cached or mixed.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_MODE_E,

    /** @brief Message opcode to configure TCAM size for HW working
     *         mode.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_TCAM_SIZE_FOR_HW_MODE_E,

    /** @brief Message opcode to configure backpressure queue limit
     *         size.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_BACKPRESSURE_THRESHOLD_E,

} PRV_CPSS_DXCH_FLOW_MANAGER_CONFIG_ENT;

/**
* @enum PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_ENT
 *
 * @brief Enumeration for ISF opcode sent in messages.
*/
typedef enum{

    /** @brief Message packet opcode for new flows sent from FW.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_NEW_FLOW_E = 1,

    /** @brief Message packet opcode for removed flows from FW.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_REMOVED_FLOW_E,

    /** @brief Message packet opcode for existing flows in FW.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_FLOW_DATA_E,

    /** @brief Message packet opcode to denote end of flows in ISF frame.
     */
    PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_END_E = 0xFF

} PRV_CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_OP_CODE_ENT;

/**
* @internal prvCpssDxChFlowManagerDbInfoGet function
* @endinternal
*
* @brief   To get flow manager db related information.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  flowMngId                - unique flow manager id.
* @param[out] flowMngInfoPtr           - ptr to flow manager configuration structure.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
*/
GT_STATUS prvCpssDxChFlowManagerDbInfoGet
(
    IN  GT_U32                            flowMngId,
    OUT PRV_CPSS_DXCH_FLOW_MANAGER_STC    *flowMngInfoPtr
);

/**
* @internal prvCpssDxChFlowManagerParseMessagePacketAndAddRule function
* @endinternal
*
* @brief To parse received packet and based on opcode add rule to cpss db or update the db.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  devNum         - device number.
* @param [in] flowMngDbPtr   - (pointer to) flow manager db
* @param [in] bufferPtr      - (pointer to) packet buffer data
* @param [in] opcode         - message opcode
* @param [in] flowId         - flow id
* @param [in] flowData       - (pointer to) flow data
*
* @retval GT_OK                        - on success.
* @retval GT_BAD_PTR                        - on NULL input ptr.
*/
GT_STATUS prvCpssDxChFlowManagerParseMessagePacketAndAddRule
(
    IN GT_U8                                   devNum,
    IN CPSS_DXCH_FLOW_MNG_PTR                  flowMngDbPtr,
    IN GT_U8                                   *bufferPtr,
    IN GT_U8                                   opcode,
    IN GT_U32                                  flowId,
    IN CPSS_DXCH_FLOW_MANAGER_FLOW_STC         *flowData
);

/**
* @internal prvCpssDxChFlowManagerCalcFlowHashAndKey function
* @endinternal
*
* @brief To calculate the flow hash value and its key from the received packet.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in]  flowMngDbPtr   - (pointer to) flow manager db
* @param [in]  flowData       - (pointer to) flow data
* @param [out] flowHashPtr    - (pointer to) flow hash
* @param [out] flowKeyPtr     - (pointer to) flow key
* @param [out] flowKeySizePtr - (pointer to) flow key size
*
* @retval GT_OK                        - on success.
* @retval GT_BAD_PTR                        - on NULL input ptr.
*/
GT_STATUS prvCpssDxChFlowManagerCalcFlowHashAndKey
(
    IN  CPSS_DXCH_FLOW_MNG_PTR           flowMngDbPtr,
    IN  CPSS_DXCH_FLOW_MANAGER_FLOW_STC  *flowData,
    OUT GT_U16                           *flowHashPtr,
    OUT GT_U8                            *flowKeyPtr,
    OUT GT_U8                            *flowKeySizePtr
);

/**
* @internal prvCpssDxChFlowManagerGetFlowIndex function
* @endinternal
*
* @brief To get flow index of the existing flow based on flow hash and flow key.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngDbPtr          - (pointer to) flow manager db
* @param [in] flow_hash             - flow hash
* @param [in] flow_key              - (pointer to) flow key
* @param [out]flowIndexPtr          - (pointer to) flow index
*
* @retval GT_OK                         - on success.
* @retval GT_BAD_PTR                        - on NULL input ptr.
*/
GT_STATUS prvCpssDxChFlowManagerGetFlowIndex
(
    IN  CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN  GT_U16                 flow_hash,
    IN  GT_U8                  *flow_key,
    OUT GT_U32                 *flowIndexPtr
);

/**
* @internal prvCpssDxChFlowManagerGetFlowDbKey function
* @endinternal
*
* @brief To get flow key of the flow based on the flow index.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngDbPtr       - (pointer to) flow manager db
* @param [in] fw_flow_index      - flow index for which flow key has to be determined
* @param [out]flowKeyPtr         - (pointer to) flow key
*
* @retval GT_OK                          - on success.
* @retval GT_BAD_PTR                        - on NULL input ptr.
*/
GT_STATUS prvCpssDxChFlowManagerGetFlowDbKey
(
    IN  CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN  GT_U32                 fw_flow_index,
    OUT GT_U8                  *flowKeyPtr
);

/**
* @internal prvCpssDxChFlowManagerAddFlowToDb function
* @endinternal
*
* @brief To add new flow to cpss db and also to its hash list based on hash index.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngDbPtr          - (pointer to) flow manager db
* @param [in] flow_hash             - flow hash
* @param [in] flowKeyPtr            - (pointer to) flow key
* @param [in] fwFlowId              - flow id
*
* @retval GT_OK                           - on success.
* @retval GT_BAD_PTR                        - on NULL input ptr.
*/
GT_STATUS prvCpssDxChFlowManagerAddFlowToDb
(
    IN CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN GT_U16                 flow_hash,
    IN GT_U8                  *flowKeyPtr,
    IN GT_U32                 fwFlowId
);

/**
* @internal prvCpssDxChFlowManagerAddEntryToDb function
* @endinternal
*
* @brief To add/update the ipfix entry for the given flow index in cpss db.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  devNum                - device number.
* @param [in] flowMngDbPtr          - (pointer to) flow manager db
* @param [in] flowIndex             - flow index value
* @param [in] bufferPtr             - (pointer to) packet buffer
*
* @retval GT_OK                           - on success.
* @retval GT_BAD_PTR                      - on NULL input ptr.
*/
GT_STATUS prvCpssDxChFlowManagerAddEntryToDb
(
    IN GT_U8                  devNum,
    IN CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN GT_U32                 flowIndex,
    IN GT_U8                  *bufferPtr
);


/**
* @internal prvCpssDxChFlowManagerAddRemovedEntryToDb function
* @endinternal
*
* @brief To add/update the ipfix entry for the given removed flow index in cpss db.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngDbPtr          - (pointer to) flow manager db
* @param [in] flowIndex             - flow index value
*
* @retval GT_OK                           - on success.
* @retval GT_BAD_PTR                      - on NULL input ptr.
*/
GT_STATUS prvCpssDxChFlowManagerAddRemovedFlowEntryToDb
(
    IN CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN GT_U32                 flowIndex
);

/**
* @internal prvCpssDxChFlowManagerSetFlowDbKey function
* @endinternal
*
* @brief To set the flow key in the cpss db based on the specified flow index and tcam rule size.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngDbPtr        - (pointer to) flow manager db
* @param [in] fw_flow_index       - flow index
* @param [in] flowKeyPtr          - (pointer to) flow key
*
* @retval GT_OK                           - on success.
* @retval GT_BAD_PTR                        - on NULL input ptr.
*/
GT_STATUS prvCpssDxChFlowManagerSetFlowDbKey
(
    IN CPSS_DXCH_FLOW_MNG_PTR flowMngDbPtr,
    IN GT_U32                 flowIndex,
    IN GT_U8                  *flowKeyPtr
);

/**
* @internal prvCpssDxChFlowManagerConfigGet function
* @endinternal
*
* @brief To get the configuration data sent from the cpss to FW for flow manager service
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId            - unique flow manager id
*                                      (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
*/
GT_STATUS prvCpssDxChFlowManagerConfigGet
(
    IN GT_U32    flowMngId
);

/**
* @internal prvCpssDxChFlowManagerDbFlowMngGet function
* @endinternal
*
* @brief To get DB info about Flow Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] flowMngId                - unique flow manager id.
*                                      (APPLICABLE RANGES: 0..127)
*
* RETURNS:
*       pointer to the DB of the Flow Manager.
*       NULL on error
*
*/
PRV_CPSS_DXCH_FLOW_MANAGER_STC *prvCpssDxChFlowManagerDbFlowMngGet
(
    IN GT_U32  flowMngId
);

/**
* @internal prvCpssDxChFlowManagerCpssDbDump function
* @endinternal
*
* @brief To dump the entire cpss flow db.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId            - unique flow manager id
*                                      (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_NOT_FOUND             - if the Flow Manager DB is not found.
*/
GT_STATUS prvCpssDxChFlowManagerCpssDbDump
(
    IN GT_U32  flowMngId
);

/**
* @internal prvCpssDxChFlowManagerUsedListDump function
* @endinternal
*
* @brief To dump the used list maintained in order to track the flows activity.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId              - unique flow manager id
* @param [in] direction              - to dump from start or end of list
* @param [in] numEntries             - number of entries to print
*
* @retval GT_OK                           - on success.
*/
GT_STATUS prvCpssDxChFlowManagerUsedListDump
(
    IN GT_U32    flowMngId,
    IN GT_U32    direction,
    IN GT_U32    numEntries
);

/**
* @internal prvCpssDxChFlowManagerFreeListDump function
* @endinternal
*
* @brief To dump the free list based on which entries to new flows are allocated.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in]  flowMngId                - unique flow manager id
* @param [in]  direction                - to dump from start or end of list
* @param [in]  numEntries               - number of entries to print
*
* @retval GT_OK                          - on success.
*/
GT_STATUS prvCpssDxChFlowManagerFreeListDump
(
    IN GT_U32    flowMngId,
    IN GT_U32    direction,
    IN GT_U32    numEntries
);

/**
* @internal prvCpssDxChFlowManagerHashListDump function
* @endinternal
*
* @brief To dump the hash list maintained for faster db management.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId            - unique flow manager id
* @param [in] startHash            - start hash index
* @param [in] numOfLists           - number of hash lists to dump
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_FOUND             - if the Flow Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChFlowManagerHashListDump
(
    IN GT_U32    flowMngId,
    IN GT_U32    startHash,
    IN GT_U32    numOfLists
);

/**
* @internal prvCpssDxChFlowManagerFlowEntryDump function
* @endinternal
*
* @brief To dump the flow entries maintained in order to track the flows activity.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId              - unique flow manager id
* @param [in] startIndex             - to dump from this inex onwards
* @param [in] numEntries             - number of entries to print
*
* @retval GT_OK                           - on success.
*/
GT_STATUS prvCpssDxChFlowManagerFlowEntryDump
(
    IN GT_U32    flowMngId,
    IN GT_U32    startIndex,
    IN GT_U32    numEntries
);

/**
* @internal serviceCpuFlowManagerFwInit function
* @endinternal
*
* @brief Flow Manager Firmware loading and Activate
* @param[in] devNum                - device number
* @param[in] sCpuId                - service CPU ID
* @param[out] fwChannel            - IPC channel
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*/
GT_STATUS serviceCpuFlowManagerFwInit
(
     IN  GT_U8      devNum,
     IN  GT_U32     sCpuId,
     OUT GT_UINTPTR *fwChannel
);

/**
* @internal prvCpssDxChFlowManagerFwToCpssIndexDump function
* @endinternal
*
* @brief To dump the fw and cpss flow index mapping.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId              - unique flow manager id
* @param [in] startIndex             - to dump from this index onwards
* @param [in] numEntries             - number of entries to print
*
* @retval GT_OK                          - on success.
*/
GT_STATUS prvCpssDxChFlowManagerFwToCpssIndexDump
(
    IN GT_U32    flowMngId,
    IN GT_U32    startIndex,
    IN GT_U32    numEntries
);

/**
* @internal prvCpssDxChFlowManagerMessageStatisticsClear function
* @endinternal
*
* @brief Debug api to clear all the message packet statistics.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param [in] flowMngId              - unique flow manager id
*
* @retval GT_OK                      - on success.
*/
GT_STATUS prvCpssDxChFlowManagerMessageStatisticsClear
(
    IN GT_U32    flowMngId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChFlowManagerDbh */


