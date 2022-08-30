// xpSaiSwitch.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiSwitch_h_
#define _xpSaiSwitch_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

/*Default SAI adapter VLAN ID*/
#define XPSAI_DEFAULT_VLAN_ID    1
#define XPSAI_SWITCH_HW_NAME_LEN 16
#define XPSAI_TABLETIME 10000000
#define NV_MEMPOOLSIZE 1024*1024*5
#define MIN_INTERVAL_RESTART 10*1000
#define XP_SAI_MAX_MIRROR_SESSIONS_NUM (XP_MIRROR_MAX_USER_SESSION + 1)
#define XPSAI_SWITCH_MAX_MTU 16383
#define XPSAI_ACL_TABLE_SUPPORT 15
#define XPSAI_ACL_MANDATORY_CAPABILITY_NUM 1
#define XPSAI_INGRESS_ACL_TABLE_MAX   5    // Actual supported tables.
#define XPSAI_ACL_DB_MAX      6   // Actual supported tables.
#define XPSAI_MAX_PRIORITIES_SUPPORTED 8


typedef enum
{
    XPSAI_L2_HASH_OBJECT_INDEX,
    XPSAI_IPV4_HASH_OBJECT_INDEX,
    XPSAI_IPV6_HASH_OBJECT_INDEX,
    XPSAI_MAX_HASH_OBJECT_INDEX
} xpSaiHashType;

typedef enum
{
    XP_SAI_UNINITIALIZED,
    XP_SAI_INITIALIZED
} xpSaiInitStat;

typedef struct _xpSaiSwitchAttributesT
{
    sai_attribute_value_t portNumber;
    sai_attribute_value_t cpuPort;
    sai_attribute_value_t maxVirtualRouters;
    sai_attribute_value_t fdbTableSize;
    sai_attribute_value_t numberOfUnicastQueues;
    sai_attribute_value_t numberOfMulticastQueues;
    sai_attribute_value_t numberOfQueues;
    sai_attribute_value_t numberOfCpuQueues;
    sai_attribute_value_t onLinkRouteSupported;
    sai_attribute_value_t operStatus;
    sai_attribute_value_t maxTemp;
    sai_attribute_value_t qosMaxNumberOfSchedulerGroupHierarchyLevels;
    sai_attribute_value_t qosMaxNumberOfSchedulerGroupsPerHierarchyLevel;
    sai_attribute_value_t qosMaxNumberOfChildsPerSchedulerGroup;
    sai_attribute_value_t totalBufferSize;
    sai_attribute_value_t ingressBufferPoolNum;
    sai_attribute_value_t egressBufferPoolNum;
    sai_attribute_value_t switchingMode;
    sai_attribute_value_t bcastCpuFloodEnable;
    sai_attribute_value_t mcastCpuFloodEnable;
    sai_attribute_value_t violationTtl1Action;
    sai_attribute_value_t defaultPortVlanId;
    sai_attribute_value_t srcMacAddress;
    sai_attribute_value_t maxLearnedAddresses;
    sai_attribute_value_t fdbAgingTime;
    sai_attribute_value_t fdbUnicastMissAction;
    sai_attribute_value_t fdbBroadcastMissAction;
    sai_attribute_value_t fdbMulticastMissAction;
    sai_attribute_value_t ecmpHashSeed;
    sai_attribute_value_t ecmpHashType;
    sai_attribute_value_t ecmpHashFields;
    sai_attribute_value_t ecmpMaxPaths;
    sai_attribute_value_t qosDefaultTc;
    sai_attribute_value_t qosDot1pToTcMap;
    sai_attribute_value_t qosDot1pToColorMap;
    sai_attribute_value_t qosDscpToTcMap;
    sai_attribute_value_t qosDscpToColorMap;
    sai_attribute_value_t qosTcToQueueMap;
    sai_attribute_value_t qosTcAndColorToDot1pMap;
    sai_attribute_value_t qosTcAndColorToDscpMap;
    sai_attribute_value_t qosMaxNumberOfTrafficClasses;
} xpSaiSwitchAttributesT;

typedef enum xpSaiSwitchStaticDataType_e
{
    SAI_SWITCH_STATIC_VARIABLES,
} xpSaiSwitchStaticDataType_e;

// Enum: Ibuffer Config File Parse tokens
typedef enum xpSaiSwitchIbufferParseTkns
{
    XPSAI_IB_PRS_TKN_SPEED,         /// Parse Token: speed
    XPSAI_IB_PRS_TKN_LOSSLESS,      /// Parse Token: lossless
    XPSAI_IB_PRS_TKN_XOFF,          /// Parse Token: xoff
    XPSAI_IB_PRS_TKN_XON,           /// Parse Token: xon
    XPSAI_IB_PRS_TKN_LOSSY,         /// Parse Token: lossy
    XPSAI_IB_PRS_TKN_LYDROP,        /// Parse Token: lydrop
    XPSAI_IB_PRS_TKN_MAX,           /// Parse Token: Unsupported token (sentinel)
} xpSaiSwitchIbufferParseTkns_t;

// Struct: Ibuffer Config from file
typedef struct xpSaiSwitchIbufferCfg
{
    XP_SPEED       speed;
    uint32_t       losslessAlloc;
    uint32_t       losslessXoff;
    uint32_t       losslessXon;
    uint32_t       lossyAlloc;
    uint32_t       lossyDrop[XPSAI_MAX_PRIORITIES_SUPPORTED];
} xpSaiSwitchIbufferCfg_t;

/**
 * \brief xpSaiSwitchQoSMapIds_t
 *
 * This struct contains differend QoS Map Ids applied for a Switch.
 *
 * Used in xpSaiSwitchEntry_t.
 */
typedef struct xpSaiSwitchQoSMapIds_t
{
    sai_object_id_t dot1pToTcMapId;
    sai_object_id_t dot1pToColorMapId;
    sai_object_id_t dscpToTcMapId;
    sai_object_id_t dscpToColorMapId;
    sai_object_id_t tcToQueueMapId;
    sai_object_id_t tcAndColorToDot1pMapId;
    sai_object_id_t tcAndColorToDscpMapId;
} xpSaiSwitchQoSMapIds_t;

/**
 * \brief State structure maintained by Switch
 *
 * This state contains the Switch global variables
 *
 * This state is internal to SAI layer and may be changed by SAI layer routines only
 */
typedef struct xpSaiSwitchEntry_t
{
    xpSaiSwitchStaticDataType_e keyStaticDataType; /// Table Key

    sai_mac_t                   switchSrcMacAddress; /// Switch default MAC address

    sai_object_id_t             defaultTrapGroup; /// Switch default Trap group

    sai_object_id_t             default1QBridgeId;  /// Switch default Bridge OID

    sai_object_id_t             defaultVrf;         /// Switch default VRF

    bool
    mirrorSessionIds[XP_SAI_MAX_MIRROR_SESSIONS_NUM]; /// List of mirror session IDs

    bool
    sampleSessionIds[XP_SAI_MAX_MIRROR_SESSIONS_NUM]; /// List of sample session IDs

    uint8_t                     defaultTc;          /// Default Traffic Class value

    sai_object_id_t             defaultHash;  /// Switch default HASH
    bool
    symmetricHashEnabled;    /// Switch symmetric HASH mode
    uint32_t                    lagHashSeed;         /// Switch LAG hash seed
    uint32_t                    ecmpHashSeed;        /// Switch ECMP hash seed
    uint8_t
    lagHashAlgorithm;         /// Switch LAG hash algorithm
    uint8_t
    ecmpHashAlgorithm;        /// Switch ECMP hash algorithm
    sai_switch_restart_type_t
    restartTypeSupported;  ///restart Type support(Planned & Unplanned)
    uint32_t
    nv_storage_size_warmboot; ///Switch non volatile memory size
    uint32_t                    min_planned_restart_interval;
    sai_int32_t                 switch_oper_status;
    sai_packet_action_t         fdbUnicastMissAction;
    sai_uint32_t
    fdbMaxLearnedEntries; /// Maximum number of learned MAC addresses
    uint32_t
    fdbCtrlMacEntries;    /// Track all Control MAC entries added by SAI
    bool
    bcastCpuFloodEnable;  /// L2 broadcast flood control to CPU port (Bcast and DA miss)
    bool
    mcastCpuFloodEnable;  /// L2 multicast flood control to CPU port
    uint32_t                    numUcastQueues;       // SAI_NUM_UNICAST_QUEUES
    uint32_t                    numMcastQueues;       // SAI_NUM_MULTICAST_QUEUES
    uint32_t                    numQueues;            // SAI_NUM_QUEUES
    uint32_t                    numCpuQueues;         // SAI_NUM_CPU_QUEUES

    sai_object_id_t             defaultScheduler;     // Switch default Scheduler
    xpSaiSwitchQoSMapIds_t      qosMapIds;
    sai_object_id_t
    h1UcastScheduler;     // Switch default Scheduler for parent H1 node of UC Queues
    sai_object_id_t
    h1CtrlScheduler;      // Switch default Scheduler for parent H1 node of Control Queues
    sai_object_id_t
    h1McastScheduler;     // Switch default Scheduler for parent H1 node of MC Queues
    xpSaiSwitchIbufferCfg_t
    ibufferConfig[XP_PG_SPEED_MISC];        /// Default Ibuffer Configuration for devices which support it (XP_PG_SPEED_MISC, is the max number of speeds)
    uint32_t                    numNhGrp;             // SAI_NUM_ECMP_GROUPS
    sai_uint32_t
    profileId;            // SAI_SWITCH_ATTR_SWITCH_PROFILE_ID
    sai_object_id_t             ingressAclObjId;      // Ingress ACL Object Id
    sai_object_id_t             egressAclObjId;       // Egress ACL Object Id
    uint16_t
    vxlan_udp_dst_port;       // Switch default VXLAN tunnel header default udp destination port
    macAddr_t
    vxlan_default_router_mac; // Switch default router mac is used to set destination inner default mac address
} xpSaiSwitchEntry_t;

typedef struct xpSaiSwitchProfile_s
{
    xpInitType_t   initType;
    XP_DEV_TYPE_T  devType;
    bool           diagMode;
    uint32_t       userCli;
    xpSdkDevType_t sdkDevType;
    char           hwCfgPath[XP_MAX_FILE_NAME_LEN];
    sai_vlan_id_t  defaultVlanId;
    uint32_t       portScanMs;
    char           hwId[XPSAI_SWITCH_HW_NAME_LEN];
    uint8_t        withBillingCntrs;
    uint8_t        warmBoot;
    uint32_t       numUcastQueues;      // SAI_NUM_UNICAST_QUEUES
    uint32_t       numMcastQueues;      // SAI_NUM_MULTICAST_QUEUES
    uint32_t       numQueues;           // SAI_NUM_QUEUES
    uint32_t       numCpuQueues;        // SAI_NUM_CPU_QUEUES
    uint32_t       numNhGrpEcmpMember;  // SAI_NUM_ECMP_MEMBERS
    uint32_t       numNhGrp;            // SAI_NUM_ECMP_GROUPS
    sai_uint32_t   profileId;           // SAI_SWITCH_ATTR_SWITCH_PROFILE_ID
    xpSaiLogDest_t logDest;             // Log to file or stdout
    char           warmBootReadFile[XP_MAX_FILE_NAME_LEN];
    char           warmBootWriteFile[XP_MAX_FILE_NAME_LEN];
    char
    ibufConfigFile[XP_MAX_FILE_NAME_LEN];    // Keyed by SAI_XP_IBUF_CONFIG_READ_FILE
    xpSaiSwitchIbufferCfg_t ibufConfig[XP_PG_SPEED_MISC];
    char           scpuFwFile[XP_MAX_FILE_NAME_LEN];        // SAI_SCPU_FW_FILE
    bool           enableScpuFw;                            // SAI_ENABLE_SCPU_FW
    char           z80ANFwFile[XP_MAX_FILE_NAME_LEN];       // SAI_Z80_AN_FW_FILE
} xpSaiSwitchProfile_t;

typedef struct _sai_switch_notification_t
{
    sai_switch_state_change_notification_fn      on_switch_state_change;
    sai_fdb_event_notification_fn                on_fdb_event;
    sai_port_state_change_notification_fn        on_port_state_change;
    sai_switch_shutdown_request_notification_fn  on_switch_shutdown_request;
    sai_packet_event_notification_fn             on_packet_event;
    sai_queue_pfc_deadlock_notification_fn       on_queue_deadlock_event;
    sai_bfd_session_state_change_notification_fn on_bfd_state_change;
} sai_switch_notification_t;

typedef enum xpSaiSchedulerProfileType
{
    XP_SAI_SCHEDULER_PROFILE_TYPE_DEFAULT,
    XP_SAI_SCHEDULER_PROFILE_TYPE_H1UC,
    XP_SAI_SCHEDULER_PROFILE_TYPE_H1CTRL,
    XP_SAI_SCHEDULER_PROFILE_TYPE_H1MC,
    XP_SAI_SCHEDULER_PROFILE_TYPE_NONE,
} xpSaiSchedulerProfileType_t;

XP_STATUS xpSaiSwitchApiInit(uint64_t flag,
                             const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiSwitchApiDeinit();

sai_status_t xpSaiSwitchProfileCharValueGet(sai_switch_profile_id_t profile_id,
                                            const char* key, uint8_t* pValue);
sai_status_t xpSaiSwitchProfileShortValueGet(sai_switch_profile_id_t profile_id,
                                             const char* key, uint16_t* pValue);
sai_status_t xpSaiSwitchProfileIntValueGet(sai_switch_profile_id_t profile_id,
                                           const char* key, int32_t* pValue);
sai_status_t xpSaiSwitchProfileStrValueGet(sai_switch_profile_id_t profile_id,
                                           const char* key, char* pValue, uint32_t size);

sai_status_t xpSaiSwitchProfileIdGet(sai_uint32_t *profileId);
sai_status_t xpSaiGetSwitchSrcMacAddress(sai_mac_t mac);
sai_status_t xpSaiSwitchDefaultTrapGroupSet(sai_object_id_t group);
sai_status_t xpSaiSwitchDefaultTrapGroupGet(sai_object_id_t* pGroup);
sai_status_t xpSaiDefaultPortConfigApply(xpsDevice_t xpsDevId,
                                         xpsInterfaceId_t xpsIntf);
sai_status_t xpSaiSwitchDefaultHashGet(sai_object_id_t* pHashId);
sai_status_t xpSaiSwitchHashGet(uint32_t hashType, sai_object_id_t* pHashId);
sai_status_t xpSaiSwitchDefaultHashSet(sai_object_id_t hashId);
sai_status_t xpSaiSwitchDefaultBridgeIdGet(sai_object_id_t* pBridgeId);
sai_status_t xpSaiSwitchDefaultBridgeIdSet(sai_object_id_t bridgeId);
sai_status_t xpSaiSwitchDefaultVrfGet(sai_object_id_t *pVrfId);
sai_status_t xpSaiSwitchDefaultVrfSet(sai_object_id_t vrfId);
sai_status_t xpSaiSwitchMirrorSessionIdAllocate(uint32_t* sessionId);
void xpSaiSwitchMirrorSessionIdRelease(uint32_t sessionId);
sai_status_t xpSaiSwitchSymmetricHashStatusGet(bool* pEnabled);
sai_status_t xpSaiSwitchSymmetricHashStatusSet(bool enable);
sai_status_t xpSaiSwitchGetDefaultScheduler(sai_object_id_t *defSchedObjId);
sai_status_t xpSaiSwitchSetDefaultScheduler(sai_object_id_t defSchedObjId);
sai_switch_notification_t* xpSaiGetSwitchNotifications(xpsDevice_t devId);
XP_DEV_TYPE_T xpSaiSwitchDevTypeGet();
uint32_t xpSaiPortCycleTimeGet();

uint32_t xpSaiGetSwitchInbandMgmtInf();

sai_status_t xpSaiGetSwitchAttrFdbUnicastMissAction(sai_attribute_value_t
                                                    *value);

sai_status_t xpSaiSetSwitchAttribute(sai_object_id_t port_id,
                                     const sai_attribute_t *attr);

/**
 * \brief Get the amount of ibuffer allocated for this port for
 *        lossless
 *
 *
 * \param devId     - device Id
 * \param speed     - port speed
 * \param allocation - Buffer allocation
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchGetIbufferLosslessAllocation(xpsDevice_t devId,
                                                     XP_SPEED speed, sai_uint32_t *allocation);

/**
 * \brief Get the xoff threshold for the ibuffer out of the
 *        lossless allocation
 *
 *
 * \param devId     - deviceId
 * \param speed     - port speed
 * \param xoff      - xoff threshold
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchGetIBufferLosslessXoff(xpsDevice_t devId,
                                               XP_SPEED speed, sai_uint32_t *xoff);

/**
 * \brief Get the xon threshold for the ibuffer out of the
 *        lossless allocation
 *
 *
 * \param devId     - deviceId
 * \param speed     - port speed
 * \param xon       - xon threshold
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchGetIbufferLosslessXon(xpsDevice_t devId, XP_SPEED speed,
                                              sai_uint32_t *xon);

/**
 * \brief Get the amount of ibuffer allocated for this port for
 *        lossy
 *
 * \param devId     - deviceId
 * \param speed     - port speed
 * \param allocation - Buffer allocation
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchGetIBufferLossyAllocation(xpsDevice_t devId,
                                                  XP_SPEED speed, sai_uint32_t *allocation);

/**
 * \brief Get the lossy drop levels for each priority in the
 *        ibuffer
 *
 *
 * \param devId     - deviceId
 * \param speed     - port speed
 * \param priority  - lossy priority of interest
 * \param level     - lossy drop level threshold
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchGetIBufferLossyDropLevel(xpsDevice_t devId,
                                                 XP_SPEED speed, sai_uint32_t priority, sai_uint32_t *level);

/**
 * \brief Get maximum number of learned FDB entries
 *
 * \param devId         - device id
 * \param maxFdbEntries - placeholder for max FDB entries
 *
 * This API is used to retrieve maximum allowed FDB entries to be learned from SAI database
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchMaxFdbEntriesGet(xpsDevice_t devId,
                                         sai_uint32_t *maxFdbEntries);

/**
 * \brief Set maximum number of learned FDB entries
 *
 * \param devId         - device id
 * \param maxFdbEntries - max FDB entries
 *
 * This API is used to set maximum allowed FDB entries that can be learned by device
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchMaxFdbEntriesSet(xpsDevice_t devId,
                                         sai_uint32_t maxFdbEntries);

/**
 * \brief Get number of Control MAC entries
 *
 * \param devId          - device id
 * \param ctrlMacEntries - placeholder for Control MAC entries
 *
 * This API is used to retrieve current number of Control MAC entries
 * which were added by SAI adapter
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiSwitchCtrlMacEntriesGet(xpsDevice_t devId,
                                       uint32_t *ctrlMacEntries);

/**
 * \brief Increment Control MAC entries counter
 *
 * \param devId - device id
 *
 * This API is used to increment Control MAC entries counter when SAI adapter
 * adds new entry
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiSwitchNewCtrlMacEntryAdd(xpsDevice_t devId);

extern void xpSaiSwitchThreadsWaitForInit();

void xpSaiSwitchLinkUpEventNotification(xpsDevice_t xpsDevId,
                                        uint32_t port_num);
void xpSaiSwitchLinkDownEventNotification(xpsDevice_t xpsDevId,
                                          uint32_t port_num);

/**
 * \param devId         - device id
 * \param value         - Flag value: true(enable) or false(disable)
 *
 * This API is used to get the enable flag for L2 broadcast flood control to CPU port
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetSwitchAttrBcastCpuFloodEnable(xpsDevice_t devId,
                                                   bool* value);

/**
 * \param devId         - device id
 * \param value         - Flag value: true(enable) or false(disable)
 *
 * This API is used to get the enable flag for L2 multicast flood control to CPU port
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetSwitchAttrMcastCpuFloodEnable(xpsDevice_t devId,
                                                   bool* value);

/**
 * \param value         - Out parameter to fetch number of nhGrps used
 *
 * This API is used to get the number of next hop groups used in system
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetSwitchAttrNumberOfNhGroups(sai_attribute_value_t *value);

sai_status_t xpSaiSwitchCreateH1SchedulerBasedOnType(sai_object_id_t
                                                     switchObjId,
                                                     xpSaiSchedulerProfileType_t schedProfileType);
sai_status_t xpSaiSwitchSetSchedulerProfileBasedonType(
    xpSaiSchedulerProfileType_t schedProfileType,
    sai_object_id_t schedObjId);
sai_status_t xpSaiSwitchGetSchedulerProfileBasedonType(
    xpSaiSchedulerProfileType_t schedProfileType,
    sai_object_id_t *schedObjId);

/**
 * \brief Get hash algorithm from DB
 *
 * \param sai_int32_t* hashAlgotiryhm    -hashAlgotiryhm
 * \param bool isLag - lag or ecmp
 *
 * This API is used to get hash algorithm from DB
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchHashAlgorithmGet(sai_int32_t* hashAlgotiryhm,
                                         bool isLag);

sai_status_t xpSaiGetSwitchAttrCPUPort(sai_object_id_t *cpuPort);

sai_status_t xpSaiSwitchSampleSessionIdAllocate(uint32_t* sessionId);

void xpSaiSwitchSampleSessionIdRelease(uint32_t sessionId);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiSwitch_h_
