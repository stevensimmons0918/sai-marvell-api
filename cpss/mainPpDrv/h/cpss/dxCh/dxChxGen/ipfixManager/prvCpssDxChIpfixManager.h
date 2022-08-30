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
* @file prvCpssDxChIpfixManager.h
*
* @brief Private structures, enums and CPSS APIs declarations for Ipfix Manager.
*
* @version   1
*********************************************************************************
**/

#ifndef __prvCpssDxChIpfixManagerh
#define __prvCpssDxChIpfixManagerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/cpssDxChIpfixManager.h>

#define PRV_CPSS_DXCH_SDMA_QUEUE_MAX_CNS                                128
#define PRV_CPSS_DXCH_SERVICE_CPU_NUM_MIN_CNS                           16
#define PRV_CPSS_DXCH_SERVICE_CPU_NUM_MAX_CNS                           19
#define PRV_CPSS_DXCH_IPFIX_MANAGER_MAX_TILES_CNS                       4
#define PRV_CPSS_DXCH_IPFIX_MANAGER_MAX_FLOWS_PER_TILE_CNS              (8*_1K)
#define PRV_CPSS_DXCH_IPFIX_MANAGER_MAX_FLOWS_PER_PORT_GROUP_CNS         _4K
#define PRV_CPSS_DXCH_IPFIX_MANAGER_FREE_FLOW_ID_POOL_DB_ARR_SIZE_CNS         \
            (PRV_CPSS_DXCH_IPFIX_MANAGER_MAX_FLOWS_PER_PORT_GROUP_CNS / 32 + 1)

/* In Bytes */
#define PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_MAX_SIZE_CNS 40

#define PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_MAX_CNS (_1K)

/* 1024 bits. i.e. 128 bytes*/
#define PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_FETCH_SIZE_CNS  200
#define PRV_CPSS_DXCH_IPFIX_MANAGER_FETCH_ALL_IPC_READS_MAX_CNS 50
#define PRV_CPSS_DXCH_IPFIX_MANAGER_FETCH_ALL_IPC_DATA_SIZE             \
            ( PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_FETCH_SIZE_CNS *      \
              PRV_CPSS_DXCH_IPFIX_MANAGER_FETCH_ALL_IPC_READS_MAX_CNS )

#define PRV_CPSS_DXCH_IPFIX_MANAGER_DEFAULT_DATA_QUEUE_NUM_CNS  7
#define PRV_CPSS_DXCH_IPFIX_MANAGER_DEFAULT_DATA_PKT_MTU_CNS    1150
#define PRV_CPSS_DXCH_IPFIX_MANAGER_DEFAULT_IDLE_TIMEOUT_CNS    30
#define PRV_CPSS_DXCH_IPFIX_MANAGER_DEFAULT_ACTIVE_TIMEOUT_CNS  30
#define PRV_CPSS_DXCH_IPFIX_MANAGER_DEFAULT_IDLE_TIMEOUT_MAX_CNS    (65*60)
#define PRV_CPSS_DXCH_IPFIX_MANAGER_DEFAULT_ACTIVE_TIMEOUT_MAX_CNS  (65*60)
#define PRV_CPSS_DXCH_IPFIX_MANAGER_DEFAULT_DATA_PKT_MTU_MAX_CNS    1150

/**
 * @struct PRV_CPSS_DXCH_IPFIX_MANAGER_FLOW_ID_DB_STC
 *
 * @brief Structure to hold free flow ids database
*/
typedef struct PRV_CPSS_DXCH_IPFIX_MANAGER_FLOW_ID_DB_STCT {

    /* Bit mask that holds free flowId information 0 = Free, 1 = Taken */
    GT_U32  activeFlowIdPool[PRV_CPSS_DXCH_IPFIX_MANAGER_FREE_FLOW_ID_POOL_DB_ARR_SIZE_CNS];

} PRV_CPSS_DXCH_IPFIX_MANAGER_FLOW_ID_DB_STC;

/**
 * @struct PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_STC
 *
 * @brief structure for IPC communication
 */
typedef struct PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_STCT {

    /* Firmware Channel fo IPC messaging */
    GT_UINTPTR  fwChannel;

    /* Last parsed IPC data */
    GT_U32      lastIpcData[20];

    /* Last parsed IPC data valid */
    GT_U32      lastIpcDataValid;

} PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_STC;

/**
 * @struct PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC
 *
 * @brief structure for holding IPFIX Manager Global Variables and DB
 */
typedef struct PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STCT {

    /* enable/disable ipfix manager */
    GT_BOOL                                      ipfixEnable;

    /* ipfix manager attributes */
    CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC       attributes;

    /* ipc messaging */
    PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_STC          ipc;

    /* ipfix global configuration */
    CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC    globalConfig;

    /* ipfix port group configuration */
    CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC portGroupConfig[CPSS_DXCH_MAX_PORT_GROUPS_CNS];

    /* ipfix flowId database */
    PRV_CPSS_DXCH_IPFIX_MANAGER_FLOW_ID_DB_STC   flowIdDb[CPSS_DXCH_MAX_PORT_GROUPS_CNS];

    /* Even notification callback function pointer */
    CPSS_DXCH_IPFIX_MANAGER_EVENT_FUNC           eventNotifyFunc;

} PRV_CPSS_DXCH_IPFIX_MANAGER_DB_STC;

/**
 * @struct PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_ADD_STC
 *
 * @brief structure for Entry Add IPC message
 */
typedef struct PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_ADD_STCT {

    /* Opcode Word0[7:0] (8 bits) */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENT      opcode;

    /* Flow ID Word0[31:16] (16 bits). flowId[14:13] - tileNum, flowId[12:0] flowId */
    GT_U32                                          flowId;

    /* First TS Word1[14:0] (15 bits)*/
    GT_U16                                          firstTs;

    /* First TS Valid (1 bit) Word1[15] */
    GT_BOOL                                         firstTsValid;

} PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_ADD_STC;

/**
 * @struct PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_DELETE_STC
 *
 * @brief structure for Entry Delete IPC message
 */
typedef struct PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_DELETE_STCT {

    /* Opcode Word0[7:0] (8 bits) */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENT      opcode;

    /* Flow ID Word0[31:16] (16 bits). flowId[14:13] - tileNum, flowId[12:0] flowId */
    GT_U32                                          flowId;

} PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_DELETE_STC;

/**
 * @struct PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_DELETE_ALL_STC
 *
 * @brief structure for EntryDeleteAll IPC message
 */
typedef struct PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_DELETE_ALL_STCT {

    /* Opcode Word0[7:0] (8 bits) */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENT      opcode;

} PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_DELETE_ALL_STC;

/**
 * @struct PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_DATA_GET_STC
 *
 * @brief structure for DataGet IPC message
 */
typedef struct PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_DATA_GET_STCT {

    /* Opcode Word0[7:0] (8 bits) */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENT      opcode;

    /* Flow ID Word0[31:16] (16 bits). flowId[14:13] - tileNum, flowId[12:0] flowId */
    GT_U32                                          flowId;

} PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_DATA_GET_STC;

/**
 * @struct PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_DATA_GET_ALL_STC
 *
 * @brief structure for DataGetAll IPC message
 */
typedef struct PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_DATA_GET_ALL_STCT {

    /* Opcode Word0[7:0] (8 bits) */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENT      opcode;

} PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_DATA_GET_ALL_STC;

/**
 * @struct PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_CONFIG_GET_STC
 *
 * @brief structure for ConfigGet IPC message
 */
typedef struct PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_CONFIG_GET_STCT {

    /* Opcode  (8 bits) */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENT      opcode;

} PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_CONFIG_GET_STC;

/**
 * @union PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_SEND_UNT
 *
 * @brief union of IPC messages
 */
typedef union {

    /* dummy structure to get opcode of any IPC message */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_GET_STC           opcodeGet;

    /* EntryAdd IPC message */
    PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_ADD_STC        entryAdd;

    /* EntryDelete IPC message */
    PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_DELETE_STC     entryDelete;

    /* EntryDeleteAll IPC message*/
    PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_ENTRY_DELETE_ALL_STC entryDeleteAll;

    /* ConfigSet IPC message */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_CONFIG_STC               configSet;

    /* DataGet IPC message */
    PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_DATA_GET_STC         dataGet;

    /* DataGetAll IPC message */
    PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_DATA_GET_ALL_STC     dataGetAll;

    /* ConfigGet IPC message */
    PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_CONFIG_GET_STC        configGet;

} PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_SEND_UNT;



/**PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_FETCH_MAX_CNS
 * @internal prvCpssDxChIpfixManagerIpcMessageSend function
 * @endinternal
 *
 * @param[in] devNum   - device number
 * @param[in] ipcMsg  - IPC messae
 *
 * @retVal    GT_OK     - on success
 */
GT_STATUS prvCpssDxChIpfixManagerIpcMessageSend
(
    IN  GT_U8                                   devNum,
    IN  PRV_CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_SEND_UNT *ipcMsg
);

/**
 * @internal prvCpssDxChIpfixManagerIpcMsgFetchAndHandleAll function
 * @endinternal
 *
 * @brief  Fetch and handle all the pending IPC message (up to 1K max)
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in]  devNum                 - device number
 * @param[in[  maxMsgToFetch             - max number of messages to fetch
 * @param[out] numOfMsgFetchedPtr        - (pointer to) number of fetched messages.
 *                                         Range: 0 to 1K.
 *
 * @retval GT_OK       - on success
 */
GT_STATUS prvCpssDxChIpfixManagerIpcMsgFetchAndHandleAll
(
    IN  GT_U8     devNum,
    IN  GT_U32     maxMsgToFetch,
    OUT GT_U32    *numOfMsgFetchedPtr
);

/**
 * @internal prvCpssDxChIpfixManagerFreeFlowIdGet function
 *
 * @brief Get Free Flow ID
 *
 * @param[in]  devNum      -  device number
 * @param[in]  portGroupId     -  port group Id
 * @param[out] flowIdPtr   -  (pointer to) free flowId
 *
 * @retval GT_OK        - on success
 * @retval GT_BAD_PARAM - parameter out of range
 * @retval GT_FULL      - flowId database is full
 */
GT_STATUS prvCpssDxChIpfixManagerFreeFlowIdGet
(
    IN  GT_U8          devNum,
    IN  GT_U32          portGroupId,
    OUT GT_U32         *flowIdPtr
);

/**
 * @internal prvCpssDxChIpfixManagerIsFlowIdActive function
 *
 * @brief   Check if Flow ID is active.
 *
 * @param[in]  devNum      -  device number
 * @param[in]  portGroupId -  port group id
 * @param[in]  flowId      -  flowId
 * @param[in]  isFlowIdTaken - flowId status
 *
 * @retval GT_OK        - on success
 * @retval GT_BAD_PARAM - parameter out of range
 */
GT_STATUS prvCpssDxChIpfixManagerIsFlowIdActive
(
    IN  GT_U8          devNum,
    IN  GT_U32         portGroupId,
    IN  GT_U32         flowId,
    OUT GT_BOOL        *isFlowIdTaken
);

/**
 * @internal prvCpssDxChIpfixManagerFlowIdRelease function
 *
 * @brief Release a Flow ID and mark as free
 *
 * @param[in]  devNum      -  device number
 * @param[in]  portGroupId -  port group id
 * @param[out] flowId      -  flow id to be marked as free
 *
 * @retval GT_OK        - on success
 * @retval GT_BAD_PARAM - parameter out of range
 */
GT_STATUS prvCpssDxChIpfixManagerFlowIdRelease
(
    IN  GT_U8          devNum,
    IN  GT_U32         portGroupId,
    IN  GT_U32         flowId
);

/**
 * @internal prvCpssDxChIpfixManagerActiveFlowIdDump function
 *
 * @brief Dump all active flowIds
 *
 * @param[in]  devNum      -  device number
 *
 * @retval GT_OK        - on success
 */
GT_STATUS prvCpssDxChIpfixManagerActiveFlowIdDump
(
    IN  GT_U8          devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChIpfixManagerh */
