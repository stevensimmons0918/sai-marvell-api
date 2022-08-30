/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
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
* @file cpssDxChIpfixManager.h
*
* @brief Structures, enums and CPSS APIs declarations for Ipfix Manager.
*
* @version   1
*********************************************************************************
**/

#ifndef __cpssDxChIpfixManagerh
#define __cpssDxChIpfixManagerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/*global variables macros*/
#define PRV_CPSS_DXCH_IPFIX_MANAGER_DB_PTR_GET(_devNum, _var)\
    _var = PRV_CPSS_DXCH_PP_MAC(_devNum)->ipfixMgrDbPtr

#define CPSS_DXCH_MAX_PORT_GROUPS_CNS 8
#define CPSS_DXCH_TILES_MAX_CNS 4
#define CPSS_DXCH_IPFIX_MANAGER_FLOWS_PER_PORT_GROUP_MAX_CNS _4K
#define CPSS_DXCH_IPFIX_MANAGER_FIRST_TS_MAX_CNS  (1<<15)
#define CPSS_DXCH_IPFIX_MANAGER_DATA_PKTS_CPU_CODE_DEFAULT (CPSS_NET_FIRST_USER_DEFINED_E + 3)

/**
 * @enum CPSS_DXCH_IPFIX_MANAGER_FLOW_END_REASON_ENT
 *
 * @brief enumerator for active/idle timeout
 */
typedef enum {
    /** idle timeout */
    CPSS_DXCH_IPFIX_MANAGER_FLOW_END_REASON_IDLE_TIMEOUT_E,

    /** active timeout */
    CPSS_DXCH_IPFIX_MANAGER_FLOW_END_REASON_ACTIVE_TIMEOUT_E,

} CPSS_DXCH_IPFIX_MANAGER_FLOW_END_REASON_ENT;

/**
 * @enum CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENT
 *
 * @brief enumerator for IPC message opcodes
 */
typedef enum {
    /** Invalid */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_NONE_E,

    /** Entry Add */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENTRY_ADD_E,

    /** Entry Delete */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENTRY_DELETE_E,

    /** Entry Delete All*/
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENTRY_DELETE_ALL_E,

    /** Config Set */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_SET_E,

    /** Data Get */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_DATA_GET_E,

    /** Data Get all*/
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_DATA_GET_ALL_E,

    /** Entry Invalidate */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENTRY_INVALIDATE_E,

    /** Export Completion */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_EXPORT_COMPLETION_E,

    /** config get */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_GET_E,

    /** config return */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_RETURN_E,

    /** config return */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_ERROR_E

} CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENT;

/**
 * @enum  CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_ENT
 *
 * @brief enumerator for number of IPFIX entries per flow
 */
typedef enum {

    /** 2 entries per flow */
    CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_IS_TWO_E,

    /** 1 entry per flow */
    CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_IS_ONE_E

} CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_ENT;

/**
 * @struct CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC
 *
 * @brief ipfix manager create attributes
 */
typedef struct CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STCT {

    /** Service CPU (CM3) number */
    GT_U8   serviceCpuNum;

} CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC;

/**
 * @struct CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC
 *
 * @brief IPFIX Entry parameters
 */
typedef struct CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STCT {

    /** first time stamp (Value of the 15-bits long) prepended with 0's) which
       was extracted from the TO CPU eDSA tag of the first packet */
    GT_U16      firstTs;

    /** True/False. If set to false, it means the first field shall be copied
       from TOD by FW at the time it processes the command */
    GT_BOOL     firstTsValid;
} CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC;

/**
 * @struct CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC
 *
 * @brief IPFIX manager global confiuration parameters structure.
 */
typedef struct CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STCT {

    /** policer stage Word0[10:9] (2 bits) */
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT            policerStage;

    /** enable/disable the flow monitoring termination by FW */
    GT_BOOL     agingOffload;

    /** enable/disable the extension of IPFIX counter/TS in FW SRAM */
    GT_BOOL     monitoringOffload;

    /** ipfix entries per flow Word0[16] (1-bit) */
    /** APPLICABLE DEVICES: Falcon */
    CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_ENT ipfixEntriesPerFlow;

    /** Range: 0 to 65min, in sec units. Typical is 30sec
       Meaningless when agingOffload is disabled
       If set to 0, flows are marked as pending termination for Idle timeout
         as soon as they are learned. It means the entry will be terminated
         by the coming aging scan cycle
       If set above 65min, reported timestamps may wrap-around
       If set to all 1's, inactive flows are terminated only by removing
         them manually via cpssDxChIpfixManagerEntryDelete API
         This is the required setting when Aging offload is disabled */
    GT_U32      idleTimeout;

    /**  Range: 0 to 65min, in sec units. Typical is 200sec
        Meaningless when agingOffload is disabled
        If set to 0, flows are marked as pending termination for Active timeout
          as soon as they are learned. It means the entry will be terminated by
          the coming aging scan cycle
        If set above 65min, reported timestamps may wrap-around */
    GT_U32      activeTimeout;

    /** Enable/disable mode.
       When enabled, on every export cycle, FW clears the data counters
       (not the timestamps) in IPFIX Table and in SRAM Extension Table*/
    GT_BOOL     deltaMode;

    /** Maximum number of L2 payload bytes allowed in an IPFIX data packet */
    GT_U32      dataPktMtu;

    /** 16B eDSA tag to be inserted by FW in IPFIX data packets sent to host */
    GT_U8      txDsaTag[16];

    /** IPFIX Data Queue number */
    GT_U8   ipfixDataQueueNum;

} CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC;

/**
 * @struct CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC
 *
 * @brief IPFIX manager port group confiuration parameters structure.
 */
typedef struct CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STCT {
    /** Max index used by IPFIX Manager in each 8K IPFIX Table.
       May be a different index per Tile.
       It is assumed that all entries below this index (included)
         are used for IPFIX, while higher entries may be used for billing and/or
         packet sampling. Entry at index 0 is always kept free */
    GT_U32      maxIpfixIndex;

} CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC;

/**
 * @struct CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_GET_STC
 *
 * @brief dummy structure to read opcode of IPC message
 */
typedef struct CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_GET_STCT {

    /** Opcode Word0[7:0] (8 bits) */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENT      opcode;

} CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_GET_STC;

/**
 * @struct CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_EXPORT_COMPLETION_STC
 *
 * @brief structure for ExportCompletion IPC message
 */
typedef struct CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_EXPORT_COMPLETION_STCT {

    /** Opcode  Word0[7:0] (8 bits) */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENT      opcode;

    /** number of active entries Word0[31:16] (16 bits) */
    GT_U16                                          numActive;

    /** last active number. Word1[15:0] (16 bits) */
    GT_U16                                          lastActiveNum;

    /** last packet number. Word1[31:16] (16 bits) */
    GT_U16                                          lastPktNum;

    /** TOD in 4ms units for last sampled packet. Word2[31:0] and Word3[23:0] (56 bits)*/
    GT_U32                                          lastTod[2];

} CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_EXPORT_COMPLETION_STC;

/**
 * @struct CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_INVALIDATE_STC
 *
 * @brief structure for EntryInvalidate IPC message
 */
typedef struct CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_INVALIDATE_STCT {

    /** Opcode  Word0[7:0] (8 bits) */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENT      opcode;

    /** end reason - Idle/active timeout. Word0[8] (1-bit) */
    CPSS_DXCH_IPFIX_MANAGER_FLOW_END_REASON_ENT     endReason;

    /** last packet command Word0[15:13] */
    /** APPLICABLE DEVICES : AC5X/P */
    GT_U8                                           lastPktCmd;

    /** Flow ID Word0[31:16] (16 bits) flowId[14:13] - tileNum, flowId[12:0] flowId */
    GT_U32                                          flowId;

    /** first time stamp Word1[19:0] (20-bits) */
    GT_U32                                          firstTs;

    /** last time stamp Word1[31:20] and Word2[7:0] (20-bits) */
    GT_U32                                          lastTs;

    /** Packet Count Word2[31:8] and Word3[15:0] (40-bits) */
    GT_U32                                          packetCount[2];

    /** Drop Count Word3[31:16] and Word4[23:0] (40-bits) */
    GT_U32                                          dropCount[2];

    /** Byte Count Word4[31:24], Word5[32:0] and Word6[5:0] (46-bits) */
    GT_U32                                          byteCount[2];

    /** last CPU code Word6[31:24] (8 bits) */
    /** APPLICABLE DEVICES : AC5X/P */
    GT_U8                                           lastCpuCode;

} CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_INVALIDATE_STC;

/**
 * @struct CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_CONFIG_STC
 *
 * @brief structure for ConfigSet/ConfigReturn IPC message
 */
typedef struct CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_CONFIG_STCT {

    /** Opcode Word0[7:0] (8 bits) */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENT  opcode;

    /** ipfix enable Word0[8] (1 bit)  */
    GT_BOOL                                     ipfixEnable;

    /** policer stage Word0[10:9] (2 bits) */
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT            policerStage;

    /** agingOffload Word0[11] (1-bit) */
    GT_BOOL                                     agingOffload;

    /** monitoringOffload Word0[12] (1-bit) */
    GT_BOOL                                     monitoringOffload;

    /** deltaMode Word0[13] (1-bit) */
    GT_BOOL                                     deltaMode;

    /** ipfix entries per flow Word0[16] (1-bit) */
    /** APPLICABLE DEVICES: Falcon */
    CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_ENT ipfixEntriesPerFlow;

    /** learming mode. Word0[29] */
    /** APPLICABLE DEVICES: AC5X/P */
    GT_BOOL                                     learningMode;

    /** export last cpu code Word0[30] */
    /** APPLICABLE DEVICES: AC5X/P */
    GT_BOOL                                     exportLastCpuCode;

    /** Last Okt Command Word0[31] */
    /** APPLICABLE DEVICES: AC5X/P */
    GT_BOOL                                     exportLastOktCmd;

    /** idleTimeout Word1[11:0] (12 bits) */
    GT_U32                                      idleTimeout;

    /** activeTimeout Word1[23:12] (12 bits) */
    GT_U32                                      activeTimeout;

    /** Data packet MTU Word2[13:0] (14 bits) */
    GT_U16                                      dataPktMtu;

    /** Local Queue Number Word2[23:16] (8 bits) */
    GT_U8                                       localQueueNum;

    /** MG number Word2[31:24] (8 bits) */
    GT_U8                                       mgNum;

    /** TX DSA Tag Word<6-3> (4 * 32 bits)*/
    GT_U8                                      txDsaTag[16];

    /** 4 x maxIpfixIndex Word<8-7> (4 * 16 bits) */
    GT_U16                                      maxIpfixIndex[CPSS_DXCH_MAX_PORT_GROUPS_CNS];

    /** maximum in transit index Word9[15:0] 16 bits */
    /** APPLICABLE DEVICES: AC5X/P */
    GT_U16                                      maxIntransitIndex;

} CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_CONFIG_STC;


/**
 * @struct CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_ERROR_STC
 *
 * @brief structure for config error IPC message
 */
typedef struct CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_ERROR_STCT {

    /** Opcode Word0[7:0] (8 bits) */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENT      opcode;

    /** return Code Word0[13:8] */
    GT_U32                                          errBmp;

} CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_ERROR_STC;

/**
 * @union PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_UNT
 *
 * @brief union of IPC messages
 */
typedef union {

    /** dummy structure to get opcode of any IPC message */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_GET_STC        opcodeGet;

    /** ExportCompletion IPC message */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_EXPORT_COMPLETION_STC exportCompletion;

    /** EntryInvalidate IPC message */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_INVALIDATE_STC  entryInvalidate;

    /** ConfigReturn IPC message */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_CONFIG_STC            configReturn;

    /** configError IPC message */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_ERROR_STC configError;

} CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_RECV_UNT;

/**
 * @internal  CPSS_DXCH_IPFIX_MANAGER_EVENT_FUNC callback function
 * @endinternal
 *
 * @brief callback function for notifying event
 *
 * @param[in] ipcMsgPtr  - (pointer to) ipc message and data corresponding to the event
 */
typedef void (*CPSS_DXCH_IPFIX_MANAGER_EVENT_FUNC) (
        IN CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_RECV_UNT *ipcMsgPtr
);

/**
 * @internal cpssDxChIpfixManagerCreate function
 * @endinternal
 *
 * @brief Create IPFIX Manager and IPFIX Assistant FW entities
 *        per device, and configure the running parameters
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum            - device number
 * @param[in] attributesPtr     - (pointer to) ipfix manager attributes
 *
 * @retVal GT_OK                - on success
 */
GT_STATUS cpssDxChIpfixManagerCreate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC  *attributesPtr
);

/**
 * @internal cpssDxChIpfixManagerEnableSet function
 * @endinternal
 *
 * @brief Enable/Disable IPFIX Manager
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum       - device number
 * @param[in] ipfixEnable  - GT_TRUE - enable
 *                           GT_FALSE - disable
 *
 * @retVal GT_OK                - on success
 */
GT_STATUS cpssDxChIpfixManagerEnableSet
(
    IN  GT_U8        devNum,
    IN  GT_BOOL      ipfixEnable
);

/**
 * @internal cpssDxChIpfixManagerDelete function
 * @endinternal
 *
 * @brief Delete IPFIX Manager
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum            - device number
 *
 * @retVal GT_OK                - on success
 */
GT_STATUS cpssDxChIpfixManagerDelete
(
    IN  GT_U8                                   devNum
);

/**
 * @internal cpssDxChIpfixManagerGlobalConfigSet function
 * @endinternal
 *
 * @brief  Configure IPFIX Manager Global configuration and its IPFIX Assistant
 *         FW on a specific device
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum            - device Number
 * @param[in] globalCfgPtr      - (pointer to) structure with ipfix manager
 *                                global configuration.
 *
 * @retval  GT_OK               - on success.
 */
GT_STATUS cpssDxChIpfixManagerGlobalConfigSet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC  *globalCfgPtr
);

/**
 * @internal cpssDxChIpfixManagerPortGroupConfigSet function
 * @endinternal
 *
 * @brief  Configure IPFIX Manager port group configuration and its IPFIX Assistant
 *         FW on a specific device
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum            - device Number
 * @param[in] portGroupsBmp     - port groups bitmap
 * @param[in] portGroupCfgPtr    - (pointer to) structure with ipfix manager
 *                                port group configuration.
 *
 * @retval  GT_OK               - on success.
 */
GT_STATUS cpssDxChIpfixManagerPortGroupConfigSet
(
    IN  GT_U8                                          devNum,
    IN  GT_PORT_GROUPS_BMP                             portGroupsBmp,
    IN  CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC  *portGroupCfgPtr
);

/**
 * @internal cpssDxChIpfixManagerConfigGet function
 * @endinternal
 *
 * @brief   Request the settings configured by cpssDxChIpfixManagerConfigSet
 *          to the FW
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum           - device Number
 *
 * @retval GT_OK                - on success
 */
GT_STATUS cpssDxChIpfixManagerConfigGet
(
    IN  GT_U8                               devNum
);

/**
 * @internal cpssDxChIpfixManagerEntryAdd function
 * @endinternal
 *
 * @brief   Add a new IPFIX flow to a port group, specifying its First TS
 *          extracted from the first packet mirrored to the App
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum         - device number
 * @param[in]  portGroupsBmp   - port groups bitmap
 * @param[in]  entryParamsPtr - (pointer to) IPFIX entry input params
 * @param[out] flowIdPtr      - (pointer to) flow identifier. In the range
 *                              between 1 to tileMaxIpfixIndex (included).
 *                              Returns 0 when no free entry is available
 *
 * @retval  GT_OK               - on success
 */
GT_STATUS cpssDxChIpfixManagerEntryAdd
(
    IN  GT_U8                                    devNum,
    IN  GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN  CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC *entryParamsPtr,
    OUT GT_U32                                   *flowIdPtr
);

/**
 * @internal cpssDxChIpfixManagerEntryDelete function
 * @endinternal
 *
 * @brief   Release the FlowID of an active entry in a Tile
 *          and terminate its IPFIX monitoring
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum         -  device number
 * @param[in]  portGroupsBmp  -  port groups bitmap
 * @param[in]  flowId         -  flow identifier
 *
 * @retval GT_OK       - on success
 */
GT_STATUS cpssDxChIpfixManagerEntryDelete
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  flowId
);

/**
 * @internal cpssDxChIpfixManagerEntryDeleteAll function
 * @endinternal
 *
 * @brief   Release all FlowIDs of all active entries in
 *          the device and terminate all IPFIX monitoring.
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum          - device number
 * @param[out[ numDeletedPtr   - number of active entries deleted
 *
 * @retval GT_OK       - on success
 */
GT_STATUS cpssDxChIpfixManagerEntryDeleteAll
(
    IN  GT_U8     devNum,
    OUT GT_U32    *numDeletedPtr
);

/**
 * @internal cpssDxChIpfixManagerDataGet function
 * @endinternal
 *
 * @brief   Pull the IPFIX data for a single entry in a Tile.
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum         - device number
 * @param[in]  portGroupsBmp  - port groups bitmap
 * @param[in]  flowId         - flow identifier
 *
 * @retval GT_OK       - on success
 */
GT_STATUS cpssDxChIpfixManagerIpfixDataGet
(
    IN  GT_U8                                  devNum,
    IN  GT_PORT_GROUPS_BMP                     portGroupsBmp,
    IN  GT_U32                                 flowId
);

/**
 * @internal cpssDxChIpfixManagerDataGetAll function
 * @endinternal
 *
 * @brief   Pull the IPFIX data for all active entries of the device.
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum           - device number
 *
 * @retval GT_OK       - on success
 */
GT_STATUS cpssDxChIpfixManagerIpfixDataGetAll
(
    IN  GT_U8                                  devNum
);

/**
 * @internal cpssDxChIpfixManagerIpcMsgFetch function
 * @endinternal
 *
 * @brief  Trigger IPFIX Manager to fetch the next pending
 *         IPC message
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum              - device number
 * @param[out] numOfMsgFetchedPtr  - (pointer to) 0 or 1, depending if the Tx
 *                                   IPC memory was empty or not
 *
 * @retval GT_OK       - on success
 */
GT_STATUS cpssDxChIpfixManagerIpcMsgFetch
(
    IN  GT_U8     devNum,
    OUT GT_U32    *numOfMsgFetchedPtr
);

/**
 * @internal cpssDxChIpfixManagerIpcMsgFetchAll function
 * @endinternal
 *
 * @brief  Trigger IPFIX Manager to fetch all the pending IPC
 *         message (up to 1K max)
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum                 - device number
 * @param[out] numOfMsgFetchedPtr        - (pointer to) number of fetched messages.
 *                                         Range: 0 to 1K
 *
 * @retval GT_OK       - on success
 */
GT_STATUS cpssDxChIpfixManagerIpcMsgFetchAll
(
    IN  GT_U8     devNum,
    OUT GT_U32    *numOfMsgFetchedPtr
);

/**
 * @internal cpssDxChIpfixManagerEventNotifyBind function
 * @endinternal
 *
 * @brief  Register call back function for even notification
 *
 * @param[in] devNum           - device number
 * @param[in[ notifyFunc       - (pointer to)event notification function callback
 *
 * @retval GT_OK     - on success
 */
GT_STATUS cpssDxChIpfixManagerEventNotifyBind
(
    IN   GT_U8                                 devNum,
    IN   CPSS_DXCH_IPFIX_MANAGER_EVENT_FUNC    notifyFunc
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChIpfixManagerh */

