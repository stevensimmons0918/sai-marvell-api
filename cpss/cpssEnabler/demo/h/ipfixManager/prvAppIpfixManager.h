/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file prvAppIpfixManager.h
*
* @brief App demo Ipfix manager header file.
*
* @version   1
********************************************************************************/

#ifndef __prvAppIpfixManagerh
#define __prvAppIpfixManagerh

#include <gtOs/gtOsTask.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/prvCpssDxChIpfixManager.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/cpssDxChIpfixManager.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define   PRV_APP_IPFIX_MGR_SERVICE_CPU_NUM_CNS        16
#define   PRV_APP_IPFIX_MGR_DATA_PKTS_QUEUE_NUM_CNS    3
#define   PRV_APP_IPFIX_MGR_FIRST_PKTS_QUEUE_NUM_CNS   4

#define   PRV_APP_IPFIX_MGR_FLOWS_PER_PORT_GROUP_MAX_CNS _4K
#define   PRV_APP_IPFIX_MGR_PKT_BUFF_LEN           1150

/* Config Params */
#define   PRV_APP_IPFIX_MGR_ACTIVE_TIMEOUT_CNS     30
#define   PRV_APP_IPFIX_MGR_DATA_PKTS_MTU_CNS      1150
#define   PRV_APP_IPFIX_MGR_IDLE_TIMEOUT_CNS       30

/* Burst sizes */
#define   PRV_APP_IPFIX_MGR_NEW_FLOWS_BURST_SIZE_CNS   _1K
#define   PRV_APP_IPFIX_MGR_DATA_PKTS_BURST_SIZE_CNS   _1K
#define   PRV_APP_IPFIX_MGR_IPC_EVENTS_BURST_SIZE_CNS  _1K

/* sleep time in milli seconds */
#define   PRV_APP_IPFIX_MGR_NEW_FLOWS_SLEEP_TIME_CNS   0
#define   PRV_APP_IPFIX_MGR_DATA_PKTS_SLEEP_TIME_CNS   1000
#define   PRV_APP_IPFIX_MGR_IPC_EVENTS_SLEEP_TIME_CNS  10
#define   PRV_APP_IPFIX_MGR_WAIT_AFTER_TASK_CREATE_CNS 10
#define   PRV_APP_IPFIX_MGR_WAIT_TIME_FOR_IPC_MSG      10

#define   PRV_APP_IPFIX_MGR_EXPORT_COMPLETE_TIME_PER_TIK 10
#define   PRV_APP_IPFIX_MGR_EXPORT_COMPLETE_WAIT_TIME   5000

#define   PRV_APP_IPFIX_MGR_EVENT_ARRAY_SIZE_CNS       _1K
#define   PRV_APP_IPFIX_MGR_IPFIX_INDEX_MAX_CNS        _4K
#define   PRV_APP_IPFIX_MGR_MAX_FLOWS_CNS              _4K

#define   PRV_APP_IPFIX_MGR_EMM_ENTRIES_MAX_CNS        _4K

#define PRV_APP_IPFIX_MGR_HW_INDEXES_MAX_CNS PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum
#define PRV_APP_IPFIX_MGR_HW_HASHES_MAX_CNS  PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.exactMatchNumOfBanks

#define PRV_APP_IPFIX_MGR_FLOW_KEY_SIZE_MAX_CNS   47

#define PRV_APP_IPFIX_MGR_DATA_RECORDS_PER_PKT_MAX_CNS  _1K

#define PRV_APP_IPFIX_MGR_EM_PROFILE_IDX_CNS        8
#define PRV_APP_IPFIX_MGR_EM_EXPANDED_ACTION_IDX_CNS        1
#define PRV_APP_IPFIX_MGR_MAX_NUM_ENTRIES_STEPS_CNS 256
#define PRV_APP_IPFIX_MGR_VLAN_ID_CNS               1
#define PRV_APP_IPFIX_MGR_FIRST_PKTS_CPU_CODE       (CPSS_NET_FIRST_USER_DEFINED_E + 2)
#define PRV_APP_IPFIX_MGR_DATA_PKTS_CPU_CODE        (CPSS_NET_FIRST_USER_DEFINED_E + 3)
#define PRV_APP_IPFIX_MGR_MAX_PORT_NUM_CNS          128

/* Age limit in seconds */
#define PRV_APP_IPFIX_MGR_ENTRY_IDLE_TIMEOUT_CNS       10
#define PRV_APP_IPFIX_MGR_ENTRY_ACTIVE_TIMEOUT_CNS     10

/* default PCL id for PCL-ID table configuration */
#define PRV_APP_IPFIX_MANAGER_PCL_DEFAULT_ID_MAC(_direction, _lookup, _port) \
    ((_direction * 128) + (_lookup * 64) + _port)

#define __IPFIX_MANAGER_LOG(x,...)                              \
    if ((appIpfixManagerDbPtr != NULL) &&                       \
        (appIpfixManagerDbPtr->debugPrintsEnable == GT_TRUE))   \
    {                                                           \
        cpssOsPrintf("Error: %s: %d\n  " x, __FILE__, __LINE__,##__VA_ARGS__); \
    }

#define __IPFIX_MANAGER_DBG_LOG(x,...)                              \
    if ((appIpfixManagerDbPtr != NULL) &&                       \
        (appIpfixManagerDbPtr->debugPrintsEnable == GT_TRUE))   \
    {                                                           \
        cpssOsPrintf(x, ##__VA_ARGS__); \
    }

#define __IPFIX_MANAGER_DBG_COUNTER_INCREMENT(_counterName)       \
    if ((appIpfixManagerDbPtr != NULL) &&                         \
        (appIpfixManagerDbPtr->debugCountersEnable == GT_TRUE))   \
    {                                                             \
        appIpfixManagerDbPtr->dbgCounters._counterName ++;        \
    }

#define __IPFIX_MANAGER_DBG_COUNTER_DECREMENT(_counterName)       \
    if ((appIpfixManagerDbPtr != NULL) &&                         \
        (appIpfixManagerDbPtr->debugCountersEnable == GT_TRUE))   \
    {                                                             \
        appIpfixManagerDbPtr->dbgCounters._counterName --;        \
    }

#define __IPFIX_MANAGER_DBG_COUNTER_SET(_counterName, _val)       \
    if ((appIpfixManagerDbPtr != NULL) &&                         \
        (appIpfixManagerDbPtr->debugCountersEnable == GT_TRUE))   \
    {                                                             \
        appIpfixManagerDbPtr->dbgCounters._counterName = _val;     \
    }

#define __IPFIX_MANAGER_DBG_COUNTER_PRINT(_counterName)               \
    if ((appIpfixManagerDbPtr != NULL) &&                             \
        (appIpfixManagerDbPtr->debugPrintsEnable == GT_TRUE))         \
    {                                                                 \
        cpssOsPrintf("    %-32s: %d\n", #_counterName,              \
                     appIpfixManagerDbPtr->dbgCounters._counterName);  \
    }

#define __IPFIX_MANAGER_VAL_COMPARE(_val1, _val2)                                   \
    if (_val1 != _val2)                                                             \
    {                                                                               \
        __IPFIX_MANAGER_LOG("Not matching: \n    " #_val1 " : %d\n    " #_val2 " : %d\n",   \
                            _val1, _val2);                                          \
        return GT_BAD_VALUE;                                                        \
    }

/**
 * @struct PRV_APP_IPFIX_MGR_DBG_COUNTERS_STC
 *
 * @brief Structure for capturing various debug counters
 */
typedef struct PRV_APP_IPFIX_MGR_DBG_COUNTERS_STCT {

    /* Number of first packets mirrored to host */
    GT_U32      pktsMirroredToHost;

    /* Number of first packets fetched by host */
    GT_U32      pktsFetchedByHost;

    /* Number of first packets filtered out by host because the flow is already learned */
    GT_U32      pktsFilteredByHost;

    /* Number of mirrored first packets dropped by the switch because of congestion */
    GT_U32      pktsDrops;

    /* Number of active entries */
    GT_U32      activeEntries;

    /* Number of flows terminated by Idle timeout */
    GT_U32      flowsTerminatedByIdleTimeout;

    /* Number of flows terminated by Active timeout */
    GT_U32      flowsTerminatedByActiveTimeout;

    /* Number of new flows that cannot be learned
       because of hash collision (despite using Cuckoo) */
    GT_U32      flowsLearningFails;

    /* Number of DataGetAll requests issued */
    GT_U32      dataGetAllRequests;

    /* Number of export complete message */
    GT_U32      exportCompleteMsgs;

    /* Number of IPFIX data packets fetched by the host */
    GT_U32      dataPktsFetchedByHost;

    /* Number of IPFIX data packets sent by FW */
    GT_U32      dataPktsSentByFw;

    /* Number of IPFIX data packets dropped by the switch because of congestion */
    GT_U32      dataPktsDrops;

    /* Time between a DataGetAll request is issued to CPSS and
          The last packet of the sequence is fetched by the App.
          The ExportCompletion event is issued to the App */
    GT_U32      timeForLastDataGetAllCompletion;

} PRV_APP_IPFIX_MGR_DBG_COUNTERS_STC;


typedef struct PRV_APP_IPFIX_MGR_DEFAULT_SWITCH_CFG_STCT
{
    CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profile;
    CPSS_PORT_TX_Q_ARB_GROUP_ENT            arbGroup;
} PRV_APP_IPFIX_MGR_DEFAULT_SWITCH_CFG_STC;

/**
 * @enum PRV_APP_IPFIX_MGR_PACKET_TYPE_ENT
 *
 * @brief enumerator for packet type
 */
typedef enum{
    /* IPv4-TCP */
    APP_IPFIX_MGR_PACKET_TYPE_IPV4_TCP_E,

    /* IPv4-UDP */
    APP_IPFIX_MGR_PACKET_TYPE_IPV4_UDP_E,

    /* IPv6-TCP */
    APP_IPFIX_MGR_PACKET_TYPE_IPV6_TCP_E,

    /* IPv6-UDP */
    APP_IPFIX_MGR_PACKET_TYPE_IPV6_UDP_E,

    /* OTHER */
    APP_IPFIX_MGR_PACKET_TYPE_OTHER_E

} PRV_APP_IPFIX_MGR_PACKET_TYPE_ENT;

typedef enum {

    /* new entry */
    PRV_APP_IPFIX_MGR_ENTRY_STATUS_NEW_E,

    /* entry marked for pending termination */
    PRV_APP_IPFIX_MGR_ENTRY_STATUS_PENDING_TERMINATION_E,

    /* active entry */
    PRV_APP_IPFIX_MGR_ENTRY_STATUS_ACTIVE_E

} PRV_APP_IPFIX_MGR_ENTRY_STATUS_ENT;


typedef struct PRV_APP_IPFIX_MGR_DATA_PKT_HEADER_STCT {

    /* packet type (8 bits) */
    GT_U8       packetType;

    /* sequence number (16 bits) */
    GT_U16      seqNum;

    /* number of data records exported (16 bits) */
    GT_U16      numExported;

    /* flowId of the first data record exported (16 bits) */
    GT_U16      firstFlowId;

    /* flowId of the lat data record exported (16 bits) */
    GT_U16      lastFlowId;

    /* time of the day sampled while forming the packet (7 bytes) */
    GT_U32      tod[2];

} PRV_APP_IPFIX_MGR_DATA_PKT_HEADER_STC;

/**
 * @struct PRV_APP_IPFIX_MGR_DATA_RECORD_STC
 *
 * @brief structure to hold IPFIX flow data record
 */
typedef struct PRV_APP_IPFIX_MGR_DATA_RECORD_STCT {
    /* flow identifier (16 bits) */
    GT_U32      flowId;

    /* Time Stamp of the first packet in the flow (20 bits)*/
    GT_U32      firstTs;

    /* Time stamp of the last packet in the flow (20 bits) */
    GT_U32      lastTs;

    /* packet count (40 bits) */
    GT_U32      packetCount[2];

    /* drop count (40 bits) */
    GT_U32      dropCount[2];

    /* byte count (46 bits)*/
    GT_U32      byteCount[2];

    /* entry status (2 bits) */
    PRV_APP_IPFIX_MGR_ENTRY_STATUS_ENT  entryStatus;

} PRV_APP_IPFIX_MGR_DATA_RECORD_STC;


typedef struct PRV_APP_IPFIX_MGR_DATA_PKT_STCT {

    PRV_APP_IPFIX_MGR_DATA_PKT_HEADER_STC header;

    PRV_APP_IPFIX_MGR_DATA_RECORD_STC dataRecord[PRV_APP_IPFIX_MGR_DATA_RECORDS_PER_PKT_MAX_CNS];

} PRV_APP_IPFIX_MGR_DATA_PKT_STC;

/**
 * @struct PRV_APP_IPFIX_MGR_FLOWS_STC
 *
 * @brief structure for IPFIX manager flows database per control pipe
 */
typedef struct PRV_APP_IPFIX_MGR_FLOWS_STCT {

    /* is active */
    GT_BOOL                           isactive;

    /* packet type */
    PRV_APP_IPFIX_MGR_PACKET_TYPE_ENT pktType;

    /* flow key */
    CPSS_DXCH_EXACT_MATCH_KEY_STC     key;

    /* data Record */
    PRV_APP_IPFIX_MGR_DATA_RECORD_STC data;

} PRV_APP_IPFIX_MGR_FLOWS_STC;

typedef struct PRV_APP_IPFIX_MGR_THREAD_PARAMS_STCT {
    /* task id */
    GT_TASK   taskId;

    /* terminate newFlowsHandle task */
    GT_BOOL   taskTerminate;

    /* is task active */
    GT_BOOL   isTaskActive;

    /* burst size */
    GT_U32    burstSize;

} PRV_APP_IPFIX_MGR_THREAD_PARAMS_STC;

typedef struct PRV_APP_IPFIX_MGR_EVENTS_STCT {

    /* IPC event array */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_RECV_UNT ipcMsg[PRV_APP_IPFIX_MGR_EVENT_ARRAY_SIZE_CNS];

    /* set to 1 if a new config return is recieved */
    GT_BOOL     configReturnReadStatus;

    /* last config return */
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_CONFIG_STC lastConfigReturn;

    /* indicator for wait for export complete event */
    GT_BOOL     waitForExportComplete;

    /* indicator that config error event occured */
    GT_BOOL     configErrorOccured;

    /* config Error bitmap */
    GT_BOOL     configErrorBitmap;

    /* start index */
    GT_U32      startIndex;

    /* end index */
    GT_U32      endIndex;

    /* is events array full */
    GT_BOOL     isEventsBuffFull;


} PRV_APP_IPFIX_MGR_EVENTS_STC;

typedef struct PRV_APP_IPFIX_MGR_TEST_PARAMS_EMM_STCT
{
    GT_U32                                expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT      lookupNum;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT      action;
    GT_U8                                 keyMask[CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS];
} PRV_APP_IPFIX_MGR_TEST_PARAMS_EMM_STC;

typedef struct PRV_APP_IPFIX_MGR_TEST_PARAMS_PCL_STCT
{
    CPSS_PCL_DIRECTION_ENT             direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT         lookupNum;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT      packetType;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormatType;
} PRV_APP_IPFIX_MGR_TEST_PARAMS_PCL_STC;

typedef struct PRV_APP_IPFIX_MGR_TEST_PARAMS_STCT {

    GT_PORT_NUM     egressPort;

    /* poll interval in milli seconds*/
    GT_U32          newFlowsPollInterval;
    GT_U32          dataPktsPollInterval;
    GT_U32          ipcFetchPollInterval;
    GT_U32          waitTimeForExportComplete;

    PRV_APP_IPFIX_MGR_TEST_PARAMS_EMM_STC   emm;
    PRV_APP_IPFIX_MGR_TEST_PARAMS_PCL_STC   pcl;

    GT_U8          newFlowsSdmaQNum;
    GT_U8          dataPktsSdmaQNum;

    GT_U32          newFlowsCpuCode;
    GT_U32          dataPktsCpuCode;

    GT_U32          activeTimeout;
    GT_U32          idleTimeout;

} PRV_APP_IPFIX_MGR_TEST_PARAMS_STC;

/**
 * @struct PRV_APP_IPFIX_MGR_DB_STC
 *
 * @brief structure for IPFIX manager database
 */
typedef struct PRV_APP_IPFIX_MGR_DB_STCT {

    /* device number */
    GT_U8                                         devNum;

    /* ipfix enable */
    GT_BOOL                                       ipfixEnable;

    /* attributes */
    CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC        attributes;

    /* global configuration */
    CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC     globalCfg;

    /* port group configuration*/
    CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC portGroupCfg[CPSS_DXCH_MAX_PORT_GROUPS_CNS];

    /* flow database */
    PRV_APP_IPFIX_MGR_FLOWS_STC
        flowsDb[CPSS_DXCH_MAX_PORT_GROUPS_CNS][PRV_APP_IPFIX_MGR_FLOWS_PER_PORT_GROUP_MAX_CNS];

    /* First packets queue number */
    GT_U32    firstPktsQueueNum;

    /* exact match manager id */
    GT_U32    exactMatchManagerId[CPSS_DXCH_TILES_MAX_CNS];

    /* terminate all tasks*/
    GT_BOOL   allTasksTerminate;

    /* new flows handle thread parameters */
    PRV_APP_IPFIX_MGR_THREAD_PARAMS_STC newFlows;

    /* data packets handle thread parameters */
    PRV_APP_IPFIX_MGR_THREAD_PARAMS_STC dataPkts;

    /* ipc events handle thread parameters */
    PRV_APP_IPFIX_MGR_THREAD_PARAMS_STC ipcFetch;

    /* Events */
    PRV_APP_IPFIX_MGR_EVENTS_STC   pendingEvents;

    /* aging enable */
    GT_BOOL             agingEnable;

    /* debug prints enable */
    GT_BOOL             debugPrintsEnable;

    /* debug counters enable */
    GT_BOOL             debugCountersEnable;

    /* cpu Rx dump enable */
    GT_BOOL             cpuRxDumpEnable;

    /* debug counters */
    PRV_APP_IPFIX_MGR_DBG_COUNTERS_STC dbgCounters;

    /* test params */
    PRV_APP_IPFIX_MGR_TEST_PARAMS_STC testParams;

    /* default switch config params*/
    PRV_APP_IPFIX_MGR_DEFAULT_SWITCH_CFG_STC   defaultSwitchCfg;
} PRV_APP_IPFIX_MGR_DB_STC;

/**
 * @internal appDemoIpfixManagerDbCreate function
 * @endinternal
 *
 * @brief  Create IPFIX application database and initialize parameters
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerDbCreate
(
    GT_U8           devNum
);

/**
 * @internal appDemoIpfixManagerDbDelete function
 * @endinternal
 *
 * @brief  Delete IPFIX application database
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerDbDelete
(
    GT_U8           devNum
);

/**
 * @internal appDemoIpfixManagerEmmInit function
 * @endinternal
 *
 * @brief  Initialize Exact Match Maanger for IPFIX manager
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerEmmInit
(
    IN GT_U8           devNum
);

/**
 * @internal appDemoIpfixManagerEmmRestore function
 * @endinternal
 *
 * @brief  Restore Exact Match Maanger configuration
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerEmmRestore
(
    IN  GT_U8           devNum
);

/**
 * @internal appDemoIpfixManagerTxInit function
 * @endinternal
 *
 * @brief  Initialize/Configure TX queue configuration
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerTxInit
(
    IN      GT_U8           devNum
);

/**
 * @internal appDemoIpfixManagerTxRestore function
 * @endinternal
 *
 * @brief  Restore TX queue configuration
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerTxRestore
(
    IN      GT_U8           devNum
);

/**
 * @internal appDemoIpfixManagerPolicerInit function
 * @endinternal
 *
 * @brief  Initialize policer engine for IPFIX manager
 *
 * @param[in] devNum - device number
 * @param[in] policerStage - policer stage ingress/egress
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerPolicerInit
(
    IN  GT_U8                            devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT policerStage
);

/**
 * @internal appDemoIpfixManagerPolicerRestore function
 * @endinternal
 *
 * @brief  Restore policer configuration
 *
 * @param[in] devNum - device number
 * @param[in] policerStage - policer stage ingress/egress
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerPolicerRestore
(
    GT_U8                            devNum,
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT policerStage
);

/**
 * @internal appDemoIpfixManagerPclInit function
 * @endinternal
 *
 * @brief  Init/Configure UDB configuration for PCL Keys
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerPclInit
(
    GT_U8           devNum
);

/**
 * @internal appDemoIpfixManagerPclRestore function
 * @endinternal
 *
 * @brief  Restore UDB configuration for PCL Key
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerPclRestore
(
    GT_U8           devNum
);

/**
 * @internal   appDemoIpfixManagerFlowDelete function
 * @endinternal
 *
 * @brief Delete one flow
 *
 * @param[in]  devNum        - device number
 * @param[in]  portGroupId   - port group identifier
 * @param[in]  flowId        - flow identifier
 *
 * @return GT_OK    - on success
 */
GT_STATUS appDemoIpfixManagerFlowDelete
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 flowId
);

/**
 * @internal   appDemoIpfixManagerFlowDeleteAll function
 * @endinternal
 *
 * @brief Delete all flows
 *
 * @param[in]  devNum        - device number
 *
 * @return GT_OK    - on success
 */
GT_STATUS appDemoIpfixManagerFlowDeleteAll
(
    IN  GT_U8     devNum
);

/**
 * @internal prvAppDemoIpfixManagerSingleEventHandle function
 * @endinternal
 *
 * @brief  Handle a single event.
 *
 * @param[in] handledIpcMsgPtr - (pointer to) IPC message that we handle.
 *
 * @return GT_OK    -  on success
 */
GT_STATUS prvAppDemoIpfixManagerSingleEventHandle
(
    OUT CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_RECV_UNT *handledIpcMsgPtr
);

/**
 * @internal   appDemoIpfixManagerThreadsStart function
 * @endinternal
 *
 * @brief Start IPFIX Manager tasks
 *
 * @param[in] devNum  - device number
 *
 * @return GT_OK    - on success
 */
GT_STATUS appDemoIpfixManagerThreadsStart
(
    IN  GT_U8   devNum
);

/**
 * @internal appDemoIpfixManagerThreadsStop
 * @endinternal
 *
 * @brief   Stop all IPFIX Manager tasks
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK  - on success
 */
GT_STATUS appDemoIpfixManagerThreadsStop
(
    IN  GT_U8   devNum
);

/**
 * @internal    appDemoIpfixManagerMain function
 * @endinternal
 *
 * @brief   Perform all initializations and start IPFIX
 *          Manager tasks
 *
 * @param[in] devNum   -   device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerMain
(
    IN    GT_U8   devNum
);

/**
 * @internal appDemoIpfixManagerClear function
 * @endinternal
 *
 * @brief   Stop IPFIX Manager tasks and restore configuration
 *
 * @param[in] devNum         - device number
 *
 * @retval   GT_OK  - on success
 */
GT_STATUS appDemoIpfixManagerClear
(
    IN  GT_U8   devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvAppIpfixManagerh */
