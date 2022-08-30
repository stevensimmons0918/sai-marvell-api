// xpSai.c

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSai_h_
#define _xpSai_h_

#include <stdio.h>
#include <string.h>
#include "xpEnums.h"
#include "xpTypes.h"
//Init manager
#include "xpsInit.h"
//Interface manager
#include "xpsInterface.h"
#include "xpsPort.h"
#include "xpsMac.h"
#include "xpsLink.h"
//Layer 2 managers
#include "xpsFdb.h"
#include "xpsStp.h"
#include "xpsVlan.h"
#include "xpsLag.h"
#include "xpsInternal.h"
//Layer 3 managers
#include "xpsL3.h"
#include "xpsNhGrp.h"
//Tunnel managers
#include "xpsTunnel.h"
//Multicast managers
#include "xpsMulticast.h"
//Counting/Policing/Sampling manager (infrastructure)
#include "xpsAcm.h"
#include "xpsSflow.h"
//NAT manager
#include "xpsNat.h"
//ACL manager
#include "xpsAcl.h"
// QoS
#include "xpsQos.h"
//Packet driver
#include "xpsPacketDrv.h"
//Mirror manager
#include "xpsMirror.h"
#include "xpsErspanGre.h"
#include "xpsScope.h"
#include "xpDevTypes.h"
#include "xpsMtuProfile.h"
#include "xpsGlobalSwitchControl.h"
#include "xpSaiLog.h"

#define INVALID_INDEX 0xffffffff

#ifdef __cplusplus
extern "C" {
#endif

/*Below macro is introduced in saiversion.h from 1.8.1*/
#if !defined(SAI_1_7_1)
#include "saiversion.h"
#else

#ifndef SAI_VERSION
#define SAI_VERSION(major, minor, revision) (10000 * (major) + 100 * (minor) + (revision))
#endif

#ifndef SAI_API_VERSION
#define SAI_MAJOR 1
#define SAI_MINOR 7
#define SAI_REVISION 1
#define SAI_API_VERSION SAI_VERSION(SAI_MAJOR, SAI_MINOR, SAI_REVISION)
#endif

#endif

#define LONG_INT_FORMAT(_longint) *(((int*)(&_longint))+1), _longint

#define _STR_VAL(x) {x,#x}

/*
 * FIXME:
 * Need create a SAI internal include file and place these definitions in that.
 */
/*
 * XP Sai notification type.
 * Used among internal modules of SAI adaptor for notificaitons.
 */
typedef enum _xpSaiNotifType_t
{
    XP_SAI_CREATED = 0,
    XP_SAI_PROPERTY_CHANGED,
    XP_SAI_REMOVED
} xpSaiNotifType_t;

/*
 * Status of the the neighbor entry
 */
typedef enum _xpSaiNeighborState_t
{
    XP_SAI_NEIGHBOR_FULLY_RESOLVED,
    XP_SAI_NEIGHBOR_PHYS_INTF_UNRESOLVED,
    XP_SAI_NEIGHBOR_UNRESOLVED
} xpSaiNeighborState_t;

/**
 * \brief SAI ACL global variable DB programming
 *
 */
typedef enum sXpSaiAclStaticDataType_e
{
    SAI_ACL_STATIC_VARIABLES,
} sXpSaiAclStaticDataType_e;
/**
 * \brief State structure maintained by ACL
 *
 * This state contains the ACL global variables
 *
 * This state is internal to SAI layer and may be changed by SAI layer routines only
 */
typedef struct xpSaiAclEntry_t
{
    //Key
    sXpSaiAclStaticDataType_e keyStaticDataType;

    //Data
    /* Table priority parameters */
    uint32_t saiAclTableMinimumPriority;
    uint32_t saiAclTableMaximumPriority;

    /* Entry priority parameters */
    uint32_t saiAclEntryMinimumPriority;
    uint32_t saiAclEntryMaximumPriority;
} xpSaiAclEntry_t;

/**
 * \brief Structure maintained by Vlan
 *
 *  This contains vlan related information
 *
 * This state is internal to SAI layer and may be changed by SAI layer routines only
 */
typedef struct _xpSaiVlanContextDbEntry
{
    //Key
    xpsVlan_t    vlanId;

    //Data
    uint64_t ingPkts;
    uint64_t ingOctets;
    uint64_t egrPkts;
    uint64_t egrOctets;

    uint32_t egrCounterId;

    //Max limit of forwarding entries in this vlan
    uint32_t maxLearnedAddresses;

    //ACL
    sai_object_id_t ingressAclId;
    sai_object_id_t egressAclId;

    //RSPAN
    uint32_t isRspanVlan;

    //Flood control type
    int32_t unknownUnicastFloodType;
    int32_t broadcastFloodType;
    int32_t unregMulticastFloodType;

    //ERSPAN
    uint32_t monitorPortRefCount;
} xpSaiVlanContextDbEntry;

/**
 * \brief Structure to maintain number of
 *        entries in each table.
 *
 *  These counts are internal to SAI layer.
 *  Counts are updated by SAI layer routines only.
 */
typedef struct _xpSaiTableEntryCountDbEntry
{
    //Key
    xpsDevice_t keyDevId;

    //Data
    uint32_t max_ipv4RouteEntries;
    uint32_t max_ipv6RouteEntries;
    uint32_t max_nextHopEntries;
    uint32_t max_nextHopGroupEntries;
    uint32_t max_nextHopGroupMemberEntries;
    uint32_t max_arpEntries;
    uint32_t max_fdbEntries;
    uint32_t max_aclTables;
    uint32_t max_aclTableGroups;
    uint32_t max_aclEntries;
    uint32_t max_aclCounters;
    uint32_t max_lagMemberPerGroup;
    uint32_t max_lagGroups;

    //Data
    uint32_t ipv4RouteEntries;
    uint32_t ipv6RouteEntries;
    uint32_t ipv4HostEntries;
    uint32_t ipv6HostEntries;
    uint32_t nextHopEntries;
    uint32_t nextHopGroupEntries;
    uint32_t nextHopGroupMemberEntries;
    uint32_t arpEntries;
    uint32_t fdbEntries;
    uint32_t aclTables;
    uint32_t aclTableGroups;
    uint32_t aclEntries;
    uint32_t aclCounters;

} xpSaiTableEntryCountDbEntry;

sai_status_t xpSaiGetTableEntryCountCtxDb(xpsDevice_t xpSaiDevId,
                                          xpSaiTableEntryCountDbEntry **entryCountCtxPtr);

#include "xpSaiFdb.h"
#include "xpSaiNeighbor.h"
#include "sai.h"
#include "saiobject.h"
#include "../experimental/saiextensions.h"
#include "xpSaiSwitch.h"
#include "xpSaiScheduler.h"
#include "xpSaiSchedulerGroup.h"
#include "xpSaiPort.h"
#include "xpSaiFdb.h"
#include "xpSaiVlan.h"
#include "xpSaiVirtualRouter.h"
#include "xpSaiRoute.h"
#include "xpSaiNextHop.h"
#include "xpSaiNextHopGroup.h"
#include "xpSaiRouterInterface.h"
#include "xpSaiQosMap.h"
#include "xpSaiPolicer.h"
#include "xpSaiWred.h"
#include "xpSaiQueue.h"
#include "xpSaiBuffer.h"
#include "xpSaiAcl.h"
#include "xpSaiHostInterface.h"
#include "xpSaiStp.h"
#include "xpSaiLag.h"
#include "xpSaiSamplePacket.h"
#include "xpSaiMirror.h"
#include "xpSaiHash.h"
#include "xpSaiUdf.h"
#include "xpSaiObject.h"
#include "xpSaiVrf.h"
#include "xpSaiCopp.h"
#include "xpSaiBridge.h"
#include "xpsAllocator.h"
#include "xpSaiMcastFdb.h"
#include "xpSaiL2Mc.h"
#include "xpSaiL2McGroup.h"
#include "xpSaiTunnel.h"
#include "xpSaiIpmc.h"
#include "xpSaiIpmcGroup.h"
#include "xpSaiRpfGroup.h"
#include "xpSaiCounter.h"

#if !defined(SAI_PORT_INDEX)

#define SAI_PORT_INDEX 0

#endif

/*
* Defining a value today. Ideally this should be part of sai_api_t enum.
* Should be in sync with max value in sai_api_t enum.
*/
#define XP_SAI_SWITCH_ACL_ID 128 //for enabling switch ACL MSB bit should be 1
#define XP_SAI_PACL_ID_SET   0x100
#define XP_SAI_BACL_ID_SET   0x200
#define XP_SAI_RACL_ID_SET   0x400
#define XP_SAI_ACL_ID_MASK   0XFF
#define XP_SAI_IACL0_ID_SET   0x10000
#define XP_SAI_IACL1_ID_SET   0x20000
#define XP_SAI_IACL2_ID_SET   0x40000
#define XP_SAI_IACL3_ID_SET   0x80000
#define XP_SAI_IACL4_ID_SET   0x100000
#define XP_SAI_EACL_ID_SET    0x200000
#define XP_SAI_TABLE_ID_MASK  0XFFFF
#define XP_SAI_ACL_TABLE_NUMBER   32      /// 3 IACL + 1 EACL table.

#define MAX_STR_LEN_64  64

#define XP_SAI_USER_DEFINED_TRAP_RC_NUM   32
#define XP_SAI_USER_DEFINED_TRAP_RC_MIN   855
#define XP_SAI_USER_DEFINED_TRAP_RC_MAX   (XP_SAI_USER_DEFINED_TRAP_RC_MIN + XP_SAI_USER_DEFINED_TRAP_RC_NUM - 1)
#define XP_SAI_CONTROL_TRAP_RC_NUM        128
#define XP_SAI_CONTROL_TRAP_RC_MIN        (XP_SAI_USER_DEFINED_TRAP_RC_MAX + 1)
#define XP_SAI_CONTROL_TRAP_RC_MAX        (XP_SAI_CONTROL_TRAP_RC_MIN + XP_SAI_CONTROL_TRAP_RC_NUM -1)

#if SAI_API_VERSION > SAI_VERSION(1,7,1)
#define MAX_SAI_API_EXTENSION SAI_API_EXTENSIONS_RANGE_END
#else
#define MAX_SAI_API_EXTENSION SAI_API_EXTENSIONS_RANGE_START_END
#endif

extern void* xpSaiApiTableArr[MAX_SAI_API_EXTENSION];
extern const sai_service_method_table_t* adapHostServiceMethodTable;

typedef XP_STATUS(*xpSdkDevConfigFuncPtr_t)(xpsDevice_t deviceId,
                                            xpInitType_t initType);

#define XDK_SAI_OBJID_VALUE_BITS  48
#define XDK_SAI_OBJID_VALUE_MASK  0x000000FFFFFFFFFFULL
#define XDK_SAI_OBJID_SWITCH_BITS 40
#define XDK_SAI_OBJID_SWITCH_MASK 0x0000FF0000000000ULL
#define XDK_SAI_OBJID_SWITCH_MAX_VAL 0xFF
#define XDK_SAI_OBJID_TUNNEL_BRIDGE_ID_BITS 16

typedef enum xpSaiMemPoolId
{
    XP_SAI_MEM_POOL_QOS_ING_PORT_PFL = XPS_MEM_POOL_ID_START,
} xpSaiMemPoolId;

/*
* Routine Description:
*     Compares received object type with type got as parameter.
*     Returns "true" if object belongs to provided type and "false in oposite case".
*
* Arguments:
*     [in] sai_object_id : object
*     [in] sai_object_type : object type
*
* Return Values:
*    bool
*/
#define XDK_SAI_OBJID_TYPE_CHECK(sai_object_id, sai_object_type) (sai_object_type == sai_object_type_query(sai_object_id) ? true : false)

/*
* Routine Description:
*     Creates SAI Object ID which contains switch ID
*
* Arguments:
*     [in] sai_object_type_t : object type
*     [in] sai_uint32_t      : switch ID
*     [in] sai_uint64_t      : only 40 bits of income value are being used
*     [out] sai_object_id_t
*
* Return Values:
*    sai_status_t
*/
sai_status_t xpSaiObjIdCreate(sai_object_type_t sai_object_type,
                              sai_uint32_t dev_id,
                              sai_uint64_t local_id, sai_object_id_t *sai_object_id);

/*
* Routine Description:
*     Retrieves switch ID from SAI Object ID
*
* Arguments:
*     [in] sai_object_id_t : object type
*
* Return Values:
*    sai_uint32_t
*/
sai_uint32_t xpSaiObjIdSwitchGet(sai_object_id_t sai_object_id);

/*
* Routine Description:
*     Retrieves scope ID from SAI Object ID
*
* Arguments:
*     [in] sai_object_type_t : object type
*
* Return Values:
*    sai_uint32_t
*/
sai_uint32_t xpSaiObjIdScopeGet(sai_object_id_t sai_object_id);

/*
* Routine Description:
*     Query sai object value.
*
* Arguments:
*     [out] sai_object_id_t
*
* Return Values:
*    sai_uint64_t : only 48 bits of income value are usable
*/
sai_uint64_t xpSaiObjIdValueGet(sai_object_id_t sai_object_id);

/*
* Routine Description:
*     Query sai object type.
*
* Arguments:
*     [in] sai_object_id_t
*
* Return Values:
*    Return SAI_OBJECT_TYPE_NULL when sai_object_id is not valid.
*    Otherwise, return a valid sai object type SAI_OBJECT_TYPE_XXX
*/
sai_object_type_t xpSaiObjIdTypeGet(sai_object_id_t sai_object_id);

/*
* Routine Description:
*     Convert device ID to scope ID
*
* Arguments:
*     [in] xpsDevice_t
*
* Return Values:
*    Return xpsScope_t
*/
xpsScope_t xpSaiScopeFromDevGet(xpsDevice_t xpsDevId);

xpsDevice_t xpSaiGetDevId();
sai_status_t cpssStatus2SaiStatus(GT_STATUS status);
sai_status_t xpsStatus2SaiStatus(XP_STATUS status);
sai_status_t xpSaiConvertXpsPacketAction2Sai(xpPktCmd_e xpPktCmd,
                                             sai_packet_action_t* pPacketAction);
sai_status_t xpSaiApiRegister(sai_api_t sai_api_id, void* xpSaiApi);
sai_status_t xpSaiConvertSaiPacketAction2xps(sai_packet_action_t
                                             sai_packet_action, xpPktCmd_e* pXpPktCmd);

/*
* Routine Description: Reverse MAC address.
*
* Arguments:
*     [inout] sai_mac_t mac - MAC address to be reversed
*
* Return Values:
*    Return: Pointer to reversed MAC address.
*/
uint8_t* xpSaiMacReverse(sai_mac_t mac);

/*
* Routine Description: Reverse IPv4 address.
*
* Arguments:
*     [in] ipv4Addr - address
*     [out] buffer - reversed byte array
*
*/
void xpSaiIPv4ReverseCopy(sai_ip4_t ipv4Addr, uint8_t* buffer);

/*
* Routine Description: Reverse IPv6 address.
*
* Arguments:
*     [inout] ipv6Addr - address to be reversed
*
* Return Values:
*    Return: Pointer to reversed address.
*/
uint8_t* xpSaiIPv6Reverse(sai_ip6_t ipv6Addr);

/*
* Routine Description: Negate mask.
*
* Arguments:
*     [inout] mask - mask to be reversed
*     [in] size - size
*
* Return Values:
*    Return: Pointer to mask.
*/
uint8_t* xpSaiNegMask(uint8_t* mask, uint8_t size);

XP_STATUS xpSaiDynamicArrayGrow(void **ppNewData, void *pOldData,
                                uint32_t sizeOfStruct, uint16_t sizeOfElement, uint32_t numOfElements,
                                uint32_t defaultSize);
XP_STATUS xpSaiDynamicArrayShrink(void **ppNewData, void *pOldData,
                                  uint32_t sizeOfStruct, uint16_t sizeOfElement, uint32_t numOfElements,
                                  uint32_t defaultSize);

/*
* Routine Description: Configure Egress ACL entry
*
* Arguments:
*     [in] eVif - egress vif
*     [in] tableId - virtural table Id
*     [in] aclBindPoint - bind point type
*
* Return Values:
*    Return: SAI status
*/
sai_status_t xpSaiAclEgressAclIdSet(xpsInterfaceId_t eVif, uint32_t tableId,
                                    sai_acl_bind_point_type_t aclBindPoint);
/*
* Routine Description: Configure Router Port ACL entry
*
* Arguments:
*     [in] l3IntfId - Router Vif
*     [in] aclId - virtural table Id will use as ACL ID
*     [in] enable - Enable/Disable
*
* Return Values:
*    Return: SAI status
*/

sai_status_t xpSaiAclRouterAclIdSet(xpsInterfaceId_t l3IntfId, uint32_t aclId,
                                    uint32_t groupId, bool enable);
/*
* Routine Description: Configure Bridge ACL entry.
*
* Arguments:
*     [in] vlanId - vlan Id
*     [in] aclId  - virtural table Id will use as ACL ID
*     [in] enable - Enable/Disable
*
* Return Values:
*    Return: SAI status
*/
sai_status_t xpSaiAclBridgeAclIdSet(xpsVlan_t vlanId, uint32_t aclId,
                                    bool enable);
/*
* Routine Description: Configure Port ACL entry.
*
* Arguments:
*     [in] portId - portId
*     [in] aclId  - virtural table Id will use as ACL ID
*     [in] enable - Enable/Disable
*
* Return Values:
*    Return: SAI status
*/
sai_status_t xpSaiAclPortAclIdSet(xpsInterfaceId_t portId, uint32_t aclId,
                                  bool enable);
/**
 * \brief API to set static variables for ACL
 *
 * \param [in] xpSaiAclEntryVar structure to be saved for ACL usage
 *
 * \return XP_STATUS
 */
sai_status_t xpSaiAclStaticVariablesSet(xpSaiAclEntry_t xpSaiAclEntryVar);

/**
 * \brief API to get static variables for ACL
 *
 * \param [out] ppSaiAclEntryVar pointer to structure with ACL variables
 *
 * \return XP_STATUS
 */
sai_status_t xpSaiAclStaticVariablesGet(xpSaiAclEntry_t* ppSaiAclEntryVar);

/*
* Routine Description: Check for valid MAC
*
* Arguments:
*     [in] mac - MAC to be validate
*
* Return Values:
*    Return: SUCCESS for valid MAC or appropriate error if not
*/
sai_status_t xpSaiIsValidUcastMac(sai_mac_t mac);

/*
* Routine Description: Generic method to insert context DB entry by
*                      specifying DB handler and key
*
* Arguments:
*     [in] scopeId - scope Id
*     [in] xpSaiDbHndl  - database handler
*     [in] entryCtxData - pointer to key of context data
*     [in] size - context data len
*     [out] entryCtxPtr - pointer to allocated data structure
*
* Return Values:
*    Return: SAI status
*/
sai_status_t xpSaiInsertCtxDb(xpsScope_t scopeId, xpsDbHandle_t xpSaiDbHndl,
                              void *entryCtxData, size_t size, void **entryCtxPtr);
/*
* Routine Description: Generic method to remove context DB entry by specifying the key
*
* Arguments:
*     [in] scopeId - scope Id
*     [in] xpSaiDbHndl  - database handler
*     [in] key - pointer to key of context data
*
* Return Values:
*    Return: SAI status
*/
sai_status_t xpSaiRemoveCtxDb(xpsScope_t scopeId, xpsDbHandle_t xpSaiDbHndl,
                              void *key);

/*
* Routine Description: Generic method to get context DB entry by specifying the key
*
* Arguments:
*     [in] scopeId - scope Id
*     [in] xpSaiDbHndl  - database handler
*     [in] key - pointer to key of context data
*     [out] entryCtxPtr - pointer to data structure
*
* Return Values:
*    Return: SAI status
*/
sai_status_t xpSaiGetCtxDb(xpsScope_t scopeId, xpsDbHandle_t xpSaiDbHndl,
                           void *key, void **ctxPtr);

/*
* Routine Description: Generic method to getnext context DB entry by specifying the key
*
* Arguments:
*     [in] scopeId - scope Id
*     [in] xpSaiDbHndl  - database handler
*     [in] key - pointer to key of context data
*     [out] entryCtxPtr - pointer to data structure
*
* Return Values:
*    Return: SAI status
*/
sai_status_t xpSaiGetNextCtxDb(xpsScope_t scopeId, xpsDbHandle_t xpSaiDbHndl,
                               void *key, void **ctxPtr);

/*
* Routine Description: Generic method to count number of context DB entries
*
* Arguments:
*     [in] scopeId - scope Id
*     [in] xpSaiDbHndl  - database handler
*     [out] count - number of entries
*
* Return Values:
*    Return: SAI status
*/
sai_status_t xpSaiCountCtxDbObjects(xpsScope_t scopeId,
                                    xpsDbHandle_t xpSaiDbHndl, uint32_t *count);

xpsL2EncapType_e xpSaiConvertRspanTagging(xpsL2EncapType_e tagging_mode);

xpsL2EncapType_e xpSaiRevertRspanTagging(xpsL2EncapType_e tagging_mode);

XP_STATUS xpSaiVlanGetVlanCtxDb(xpsScope_t scopeId, xpsVlan_t vlanId,
                                xpSaiVlanContextDbEntry ** vlanCtxPtr);

#ifdef __cplusplus
}
#endif
#endif //_xpSai_h_


