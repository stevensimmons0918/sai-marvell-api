// xpsState.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpsState_h_
#define _xpsState_h_

#include "xpTypes.h"
#include "xpsEnums.h"
#include "xpsRBTree.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def XPS_INVALID_DB_HANDLE
 *
 * Indicates an invalid database handle, used for validation
 */
#define XPS_STATE_INVALID_DB_HANDLE 0xFFFFFFFF

/**
 * \file xpsState.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS State Manager
 */

/**
 * \typedef xpsDbHandle_t
 * \brief Type used to maintain db handle
 *
 * All managers that require the State manager facility need to
 * register their Databases during init. A handle of this type
 * is returned and is to be maintained by interested managers
 *
 */
typedef uint32_t xpsDbHandle_t;

/**
 * \typedef xpsDbKey_t
 * \brief Type used to indicate the key type into a database
 *
 * Currently the key size is mapped into a uint32_t. This may be
 * changed in future releases
 */
typedef void* xpsDbKey_t;

/**
 * \enum xpsDbBucket_e
 * \brief Enum used to indicate whether a database has global or
 *        per device scope
 */
typedef enum xpsDbBucket_e
{
    XPS_GLOBAL = 0,
    XPS_PER_DEVICE,
    XPS_MAX_BUCKETS
} xpsDbBucket_e;

typedef enum xpsDbStateHandle_e
{
    XPS_GLOBAL_DB_HANDLE_BEGINS = 0,
    XPS_INTF_DB_HNDL = 1,
    XPS_LAG_DB_HNDL = 2,
    XPS_MIRROR_SES_DB_HNDL = 3,
    XPS_MIRROR_BKT_DB_HNDL = 4,
    XPS_MCL2_ENCAP_DB_HNDL = 5,
    XPS_MCL2_INTFLIST_DB_HNDL = 6,
    XPS_MCL2_TAGTYPE_DB_HNDL = 7,
    XPS_MCL2_TUNNEL_VIF_DB_HNDL = 8,
    XPS_MCL3_ENCAP_DB_HNDL = 9,
    XPS_MCL3_L3INTF_DB_HNDL = 10,
    XPS_MCL3_INTFLIST_DB_HNDL = 11,
    XPS_INTF_TO_PORTDEV_HNDL = 12,
    XPS_STG_VLANS_CTX_DB_HNDL = 13,
    XPS_STG_INTF_STATE_DB_HNDL = 14,
    XPS_TNL_NEXTHOP_ENTRY_DB_HNDL = 15,
    XPS_IP_TNL_GBL_DB_HNDL = 16,
    XPS_TNL_STATICDATA_DB_HNDL = 17,
    XPS_VLAN_STATE_DB_HNDL = 18,
    XPS_INTF_VLAN_TAG_TYPE_DB_HNDL = 19,
    XPS_PRIVATE_VLANSTATE_DB_HNDL = 20,
    XPS_LAG_VLANLIST_DB_HNDL = 21,
    XPS_STG_STATIC_DATA_DB_HNDL = 22,
    XPS_VLAN_STATIC_DATA_DB_HNDL = 23,
    XPS_OF_GROUP_STATE_DB_HNDL = 24,
    XPSAI_VRF_STATE_DB_HNDLE = 25,
    XPSAI_VRF_INTF_STATE_DB_HNDLE = 26,
    XPS_NEXTHOP_GROUP_DB_HNDL = 27,
    XPS_NEXTHOP_GROUP_STATIC_DB_HNDL = 28,
    XPSAI_ACL_DB_HNDL = 29,
    XPSAI_NEIGHBOUR_DB_HNDL = 30,
    XPSAI_NEXTHOP_DB_HNDL = 31,
    XPSAI_ROUTE_STATIC_DB_HNDL = 32,
    XPS_INTF_TO_LAG_DB_HNDL = 33,
    XPS_OF_DP_DB_HNDL = 34,
    XPS_OF_TABLE_DB_HNDL = 35,
    XPSAI_SAMPLE_PACKET_DB_HNDL = 36,
    XPSAI_SWITCH_STATIC_DB_HNDL = 37,
    XPS_MCV6_L2DOMAINID_DB_HANDLE = 38,
    XPS_MCV6_L2DOMAINIDTOVLAN_DB_HANDLE = 39,
    XPS_MCV6_L3DOMAINID_DB_HANDLE = 40,

    XPSAI_HOST_INTF_CHANNEL_DB_HNDL = 41,
    XPSAI_HOST_INTF_GROUP_DB_HNDL = 42,
    XPSAI_HOST_INTF_TRAP_DB_HNDL = 43,
    XPSAI_HOST_INTF_TABLE_DB_HNDL = 44,
    XPSAI_HOST_INTF_POLICER_DB_HNDL = 45,

    XPSAI_MIRROR_SESSION_DB_HNDL = 46,
    XPSAI_SCHEDULER_PROFILE_DB_HNDL = 47,
    XPSAI_QUEUE_QOS_DB_HNDL = 48,
    XPSAI_PORT_QOS_DB_HNDL = 49,
    XPSAI_POLICER_DB_HNDL = 50,
    XPSAI_ACL_TO_POLICER_DB_HNDL = 51,
    XPSAI_ACL_POLICER_DB_HNDL = 52,
    XPSAI_WRED_DB_HNDL = 53,
    XPSAI_QOS_MAP_DB_HNDL = 54,
    XPSAI_BUFFER_POOL_DB_HNDL = 55,
    XPSAI_BUFFER_PROFILE_DB_HNDL = 56,
    XPSAI_PORT_LAG_DB_HNDL = 57,
    XPSAI_LAG_PORT_COUNT_DB_HNDL = 58,
    XPSAI_INGRESS_PG_DB_HNDL = 59,
    XPSAI_PORT_DB_HNDL = 60,
    XPSAI_VLAN_STATE_DB_HNDL = 61,
    XPSAI_HASH_DB_HNDL = 62,
    XPSAI_ROUTER_INTERFACE_DB_HNDL = 63,
    XPSAI_ROUTE_DB_HNDL = 68,
    XPS_PBB_TNL_GBL_DB_HNDL = 69,
    XPSAI_NH_GRP_DB_HNDL = 70,
    XPS_ACL_ENTRY_INDEX_ARRAY_DB_HNDL = 71,
    XPSAI_SCHEDULER_GROUP_DB_HNDL = 72,
    XPSAI_LAG_INFO_DB_HNDL = 73,
    XPSAI_BRIDGE_DB_HNDL = 74,
    XPSAI_BRIDGE_PORT_DB_HNDL = 75,
    XPSAI_BRIDGE_MAPPING_DB_HNDL = 76,
    XPS_LAG_BRIDGEPORT_LIST_DB_HNDL = 77,
    XPSAI_L2MC_GROUP_STATE_DB_HNDL = 78,
    XPSAI_L2MC_GROUP_PORT_STATE_DB_HNDL = 79,
    XPSAI_TUNNEL_DB_HNDL = 80,
    XPSAI_TUNNEL_TERMTABLE_ENTRY_DB_HNDL = 81,
    XPSAI_TUNNEL_MAP_DB_HNDL = 82,
    XPSAI_TUNNEL_MAP_ENTRY_DB_HNDL = 83,
    XPSAI_PORT_STATISTIC_DB_HNDL = 84,
    XPSAI_NEIGHBOUR_FDB_DB_HNDL = 85,
    XPSAI_VRF_LOCAL_IP_DB_HNDL = 86,
    XPSAI_HOST_INTF_ACL_DB_HNDL = 87,
    XPSAI_IPMC_GROUP_STATE_DB_HNDL = 88,
    XPSAI_IPMC_GROUP_PORT_STATE_DB_HNDL = 89,
    XPSAI_RPF_GROUP_STATE_DB_HNDL = 90,
    XPSAI_RPF_GROUP_PORT_STATE_DB_HNDL = 91,
    XPSAI_IPMC_GROUP_VLAN_GROUP_STATE_DB_HNDL = 92,
    XPSAI_TUNNEL_BRIDGE_DB_HNDL = 93,
    XPS_ARP_DB_HNDL = 94,
    XPS_NETDEV_DB_HNDL = 95,
    XPS_NETDEV_DEFAULT_DATA_DB_HNDL = 96,
    XPS_IACL_TABLE_ID_DB_HNDL = 97,
    XPSAI_TABLE_ENTRY_COUNT_DB_HNDL = 98,
    XPS_IACL_GLOBAL_TABLE_INDEXING_DB_HNDL =99,
    XPS_ACL_TABLE_ENTRY_DB_HNDL = 100,
    XPS_ACL_TABLE_VTCAM_MAP_DB_HNDL =101,
    XPS_ACL_COUNTER_ID_MAPPING_DB_HNDL = 102,
    XPS_IP_TNL_START_GBL_DB_HNDL = 103,
    XPS_MAC2ME_DB_HNDL = 104,
    XPS_NH_DB_HNDL = 105,
    XPSAI_RIF_STATISTIC_DB_HNDL = 106,
    XPSAI_L2MC_DB_HNDL = 107,
    XPS_LAG_L2MC_GRP_LIST_DB_HNDL = 108,
    XPS_VXLAN_DB_HNDL = 109,
    XPS_VXLAN_VNI_DB_HNDL = 110,
    XPS_VXLAN_VNI_MAP_DB_HNDL = 111,
    XPS_HOST_INTF_GENETLINK_DB_HNDL = 112,
    XPS_GLOBAL_DB_HANDLE_LAST = 113, // While adding global db shift this down, and add before this

    XPS_GLOBAL_DB_HANDLE_ENDS = 4120,
    XPS_PER_DEVICE_HANDLE_BEGINS = 4121,
    XPS_EACL_KEY_DB_HNDL = 4122,
    XPS_IACL_KEY_DB_HNDL = 4123,
    XPS_PORT_DB_HNDL = 4124,
    XPS_TNL_ENTRY_DB_HNDL = 4125,
    XPS_QOS_AQM_DEV_DB_HNDL = 4126,
    XPS_INTF_VLAN_INDEX_DB_HNDL = 4127,
    XPS_LBL_ENTRY_DB_HNDL = 4128,
    XPS_VPN_ENTRY_DB_HNDL = 4129,
    XPS_TCAM_MGR_DB_HNDL = 4130,
    XPS_TCAM_MGR_ACL_SHUFFLING_DB_HNDL = 4131,
    XPS_TCAM_MGR_LIST_SHUFFLING_DB_HNDL = 4132,
    XPS_MTU_DB_HNDL = 4133,
    XPSAI_ACL_TABLE_ARRAY_DB_HNDL = 4134,
    XPSAI_ACL_COUNTERID_ARRAY_DB_HNDL = 4135,
    XPSAI_ACL_ENTRY_ID_ARRAY_DB_HNDL = 4136,
    XPS_ACL_ACM_BANKS_DB_HNDL = 4137,
    XPSAI_ACL_TCAM_ENTRY_ID_ARRAY_DB_HNDL = 4139,
    XPS_MPLS_TT_ENTRY_DB_HNDL = 4140,
    XPS_WCM_IACL_KEY_DB_HNDL = 4141,
    XPSAI_ACL_RSN_CODE_TO_QUEUE_MAP_ARRAY_DB_HNDL = 4144,
    XPS_ACL_TABLE_INDEX_ARRAY_DB_HNDL = 4145,
    XPS_SR_DB_HNDL = 4146,
    XPSAI_ACL_TABLEGROUP_ARRAY_DB_HNDL = 4147,
    XPSAI_ACL_TABLEGROUP_MEMBER_ARRAY_DB_HNDL = 4148, // Add per device db after this one
    XPSAI_ACL_STATIC_DB_HNDL = 4149,
    XPSAI_RPF_GROUP_MC_ENTRY_HW_IDX_STATE_DB_HNDL = 4150,
    XPS_MTU_L3_DB_HNDL = 4151,
    XPSAI_ACL_TCP_UDP_PORT_RANGE_DB_HNDL = 4152,
    XPS_PER_DEVICE_HANDLE_ENDS
} xpsDbStateHandle_e;
/*
 * State Management APIs
 */

/**
 * \brief API to initialize the State Manager
 *
 * This will allocate memory for global and per device data base
 * buckets
 *\param scopeId
 *\param initType
 *
 * \return int
 */
XP_STATUS xpsStateInit(xpsScope_t scopeId, xpsInitType_t initType);

/**
 * \brief API to Add Device Information to the State Manager
 *
 * This API is a NOP. We need a more explicit Add Device for Per
 * Device Databases
 *
 * \see xpStateMgrInsertDbForDevice
 *
 * \param devId
 *
 * \return int
 */
XP_STATUS xpsStateAddDevice(xpsDevice_t devId);

/**
 * \brief API to Remove Device Information from the State
 *        Manager
 *
 * This API is a NOP. We need a more explicit Remove Device for
 * Per Device Databases
 *
 * \see xpStateMgrDeleteDbForDevice
 *
 * \param devId
 *
 * \return int
 */
XP_STATUS xpsStateRemoveDevice(xpsDevice_t devId);

/**
 * \brief API do de-initialize the State Manager
 *
 * This API will release the memory allocated during init and
 * purge all states that are currently managed
 *
 * \return int
 */
XP_STATUS xpsStateDeInit(xpsScope_t scopeId);

/**
 * \brief API to register a Db for management
 *
 * This API will register interest in a database to be managed
 * by the State Manager. The bucket parameter indicates whether
 * this database is has global (system) scope or per device scope
 *
 * \param [in] * name
 * \param [in] bucket
 * \param [in] handle
 *
 * \return int
 */
XP_STATUS xpsStateRegisterDb(xpsScope_t scopeId, const char* name,
                             xpsDbBucket_e bucket, xpsComp_t comp, xpsDbHandle_t handle);

/**
 * \brief API to remove a Db from management
 *
 * Purge a database from management. The Handle will be
 * invalidated after the purge
 *
 * \param *handle
 *
 * \return int
 */
XP_STATUS xpsStateDeRegisterDb(xpsScope_t scopeId, xpsDbHandle_t *handle);

/**
 * \brief API to insert a new device database for an existing
 *        per device database
 *
 * This API will create a new device db associated with an
 * existing per device database handle
 *
 * \param [in] devId
 * \param [in] handle
 * \param [in] comp
 *
 * \return int
 */
XP_STATUS xpsStateInsertDbForDevice(xpsDevice_t devId, xpsDbHandle_t handle,
                                    xpsComp_t comp);

/**
 * \brief API to delete an existing device database associated
 *        with a per device database handle
 *
 * This API should be invoked when a device is reset or removed
 * from the system. This API will purge a per device database
 * handle's device database
 *
 * \param [in] devId
 * \param [in] handle
 *
 * \return int
 */
XP_STATUS xpsStateDeleteDbForDevice(xpsDevice_t devId, xpsDbHandle_t handle);

/**
 * \brief API to insert data into a global database
 *
 * This API will insert data passed in by the caller into a
 * database indicated by the handle. This API is to be used by
 * global databases
 *
 * \param [in] scopeId
 * \param [in] handle
 * \param [in] * data
 *
 * \return int
 */
XP_STATUS xpsStateInsertData(xpsScope_t scopeId, xpsDbHandle_t handle,
                             void *data);

/**
 * \brief API to search for and return data inside a global
 *        database indicated by a key
 *
 * This API will search and return data pointed to by a key and
 * stored in a global database. This handle passed into to this
 * API must be for a global database
 *
 * \param [in] scopeId
 * \param [in] handle
 * \param [in] key
 * \param [out] data
 *
 * \return int
 */
XP_STATUS xpsStateSearchData(xpsScope_t scopeId, xpsDbHandle_t handle,
                             xpsDbKey_t key, void **data);

/**
 * \brief API to get next data inside a global
 *        database indicated by a key
 *
 * This API will return next ascending order data pointed to by a key and
 * stored in a global database. This handle passed into to this
 * API must be for a global database
 *
 * \param [in] scopeId
 * \param [in] handle
 * \param [in] key
 * \param [out] data
 *
 * \return int
 */
XP_STATUS xpsStateGetNextData(xpsScope_t scopeId, xpsDbHandle_t handle,
                              xpsDbKey_t key, void **data);

/**
 * \brief API to remove data from a global database
 *
 * This API will remove the node pointed by the key from a
 * global database, provided it exists. It will return a pointer
 * to the data for the user to free if required
 *
 * \param [in] scopeId
 * \param [in] handle
 * \param [in] key
 * \param [out] data
 *
 * \return int
 */
XP_STATUS xpsStateDeleteData(xpsScope_t scopeId, xpsDbHandle_t handle,
                             xpsDbKey_t key, void **data);

/**
 * \brief API to insert data into a global database
 *
 * This API will insert data passed in by the caller into a
 * database indicated by the handle. This API is to be used by
 * per device databases.
 * PS: The data should be created on heap and its pointer should
 * be passed in. This is done to avoid creating copy of the
 * data. The data memory is deallocated when the tree is
 * deleted.
 *
 * \param [in] devId
 * \param [in] handle
 * \param [in] data
 *
 * \return int
 */
XP_STATUS xpsStateInsertDataForDevice(xpsDevice_t devId, xpsDbHandle_t handle,
                                      void *data);

/**
 * \brief API to search for and return data inside a per device
 *        database indicated by a key
 *
 * This API will search and return data pointed to by a key and
 * stored in a per device database. This handle passed into to
 * this API must be for a per device database.
 * PS: The data returned is the address itself. Any changes made
 * to this data will be reflected in the original data.
 *
 * \param [in] devId
 * \param [in] handle
 * \param [in] key
 * \param [out] data
 *
 * \return int
 */
XP_STATUS xpsStateSearchDataForDevice(xpsDevice_t devId, xpsDbHandle_t handle,
                                      xpsDbKey_t key, void **data);

/**
 * \brief API to get next data inside a per device
 *        database indicated by a key
 *
 * This API will return next ascending order data pointed to by a key and
 * stored in a per device database. This handle passed into to
 * this API must be for a per device database.
 * PS: The data returned is the address itself. Any changes made
 * to this data will be reflected in the original data.
 *
 * \param [in] devId
 * \param [in] handle
 * \param [in] key
 * \param [out] data
 *
 * \return int
 */
XP_STATUS xpsStateGetNextDataForDevice(xpsDevice_t devId, xpsDbHandle_t handle,
                                       xpsDbKey_t key, void **data);

/**
 * \brief API to remove data from a per device database
 *
 * This API will remove the node pointed to by the key from a
 * per device database, provided it exists. It returns a pointer
 * to the data to be freed by the user
 *
 * \param [in] devId
 * \param [in] handle
 * \param [in] key
 * \param [out] data
 *
 * \return int
 */
XP_STATUS xpsStateDeleteDataForDevice(xpsDevice_t devId, xpsDbHandle_t handle,
                                      xpsDbKey_t key, void **data);

/*
 * Memory management APIs
 */

/**
 *
 * \brief API to abstract heap memory allocation
 *
 *
 * \param [in]  size
 * \param [out] **data
 *
 * \return int
 */
XP_STATUS xpsStateHeapMalloc(size_t size, void **data);

/**
 *
 * \brief API to abstract heap memory free
 *
 *
 * \param [in] *data
 *
 * \return int
 */
XP_STATUS xpsStateHeapFree(void *data);

/**
 * \brief API to count number of objects in a global database
 *
 * \param [in] scopeId
 * \param [in] handle
 * \param [out] count
 *
 * \return int
 */

XP_STATUS xpsStateGetCount(xpsScope_t scopeId, xpsDbHandle_t handle,
                           uint32_t *count);

/**
 * \brief API to count number of objects in a per device database.
 *
 * \param [in] dbHndl
 * \param [in] devId
 * \param [out] count
 *
 * \return int
 */

XP_STATUS xpsStateGetCountForDevice(xpsDbHandle_t dbHndl, xpsDevice_t devId,
                                    uint32_t *count);

#ifdef __cplusplus
}
#endif

#endif // _xpsState_h_

