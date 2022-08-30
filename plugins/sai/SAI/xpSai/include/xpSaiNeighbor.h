// xpSaiNeighbor.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiNeighbor_h_
#define _xpSaiNeighbor_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct _xpSaiNeighborAttributesT
{
    sai_attribute_value_t dstMacAddress;
    sai_attribute_value_t packetAction;
    sai_attribute_value_t noHostRoute;
} xpSaiNeighborAttributesT;

/* keys are ip addr and vrfId */
typedef struct xpSaiNeighborEntry_t
{
    /* TODO: Clean up the this entry with consistent types */
    /* Key */
    sai_ip_addr_family_t type;
    ipv4Addr_t ipv4Addr;
    ipv6Addr_t ipv6Addr;
    sai_object_id_t rifId;

    /* Data */
    macAddr_t macDa;
    xpsPktCmd_e
    userPktCmd;       /* Packet command(derived from SAI Packet action) configured by user through API */
    xpsPktCmd_e
    resolvedPktCmd;   /* Final packet command resolved from Used packet command and FDB packet command. Refer:xpSaiNeighborResolvePktCmd */
    xpsInterfaceId_t
    xpsEgressIntf; /* Egress interface for the corresponding XPS host entry */
    uint32_t   nhId;
    bool noHostEntry;
    xpSaiNeighborState_t state;
    xpsInterfaceId_t l3IntfId; /*TODO: Redundant. Remove this */
} xpSaiNeighborEntry_t;

typedef struct _xpSaiNeighborProperties_t
{
    /* Flags to indicate the properties that got changed */
    uint8_t macDaValid;
    uint8_t pktCmdValid;
    uint8_t egrIntfValid;
    uint8_t stateValid;
    uint8_t serviceInstIdValid;
    /* Properties */
    macAddr_t macDa;
    /* Physical egress Interface */
    xpsInterfaceId_t egressIntf;
    /* Aggregated packet action */
    xpsPktCmd_e pktCmd;
    /* Neighbor status */
    xpSaiNeighborState_t state;
    uint32_t serviceInstId;
} xpSaiNeighborProperties_t;

/*
 * Neighbor change notification to dependent modules.
 * The notification is result of either direct change to negihbor or
 * result of modules further in pipeline fdb etc.
 *
 * All the properties bundled in single notification to enable fast update.
 */
typedef struct _xpSaiNeighborChangeNotif_t
{
    /* Neighbor that got changed */
    const sai_neighbor_entry_t *neighborKey;
    /* Notification type */
    xpSaiNotifType_t notifType;
    /* Properties */
    xpSaiNeighborProperties_t props;
} xpSaiNeighborChangeNotif_t;

XP_STATUS xpSaiNeighborInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiNeighborDeInit(xpsDevice_t xpSaiDevId);

XP_STATUS xpSaiNeighborApiInit(uint64_t flag,
                               const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiNeighborApiDeinit();

sai_status_t xpSaiNeighborEntryGetFirst(xpSaiNeighborEntry_t *entry);
sai_status_t xpSaiNeighborEntryGetNext(xpSaiNeighborEntry_t *entry,
                                       xpSaiNeighborEntry_t *nextEntry);
sai_status_t xpSaiNeighborEntrySet(xpSaiNeighborEntry_t *entry);
sai_status_t xpSaiNeighborEntryGet(xpSaiNeighborEntry_t *entry);

/**
 * Handle a FDB change notification
 */
sai_status_t xpSaiNeighborHandleFdbNotification(const xpSaiFdbChangeNotif_t
                                                *notif);

/*
 * Fetch the dest mac address corresponding to neighbor IP address
 * Return SAI_STATUS_ADDR_NOT_FOUND if neighbor is not yet resolved.
 */
sai_status_t xpSaiNeighborGetMacDa(sai_neighbor_entry_t *neighborKey,
                                   macAddr_t *macDa);

/*
 * Fetch final XPS egress interface id from neighbor IP address.
 * Return SAI_STATUS_ADDR_NOT_FOUND if neighbor is not yet resolved.
 */
sai_status_t xpSaiNeighborGetXpsEgrIntfId(sai_neighbor_entry_t *neighborKey,
                                          xpsInterfaceId_t *egressIntfId);

/*
 * Fetch the XPS Pkt cmd for a neighbor. This takes consideration of the pktCmd in case of a Vlan rif.
 * Return SAI_STATUS_ADDR_NOT_FOUND if neighbor is not yet resolved.
 */
sai_status_t xpSaiNeighborGetXpsPktCmd(sai_neighbor_entry_t *neighborKey,
                                       xpsPktCmd_e *pktCmd);

/*
 * Fetch the XPS NH data corresponding to a neighbor entry.
 * Return SAI_STATUS_ADDR_NOT_FOUND if neighbor is not yet resolved.
 */
sai_status_t xpSaiNeighborGetXpsNhData(sai_neighbor_entry_t *neighborKey,
                                       macAddr_t *macDa, xpsInterfaceId_t *egressIntfId, xpsPktCmd_e *pktCmd);

/*
 * Returns if a neighbor is resolved or not. This takes care if the DA is learnt in case of Vlan RIF.
 */
uint8_t xpSaiNeighborIsResolved(sai_neighbor_entry_t *neighborKey);

sai_status_t xpSaiMaxCountNeighborAttribute(uint32_t *count);
sai_status_t xpSaiCountNeighborObjects(uint32_t *count);
sai_status_t xpSaiGetNeighborObjectList(uint32_t *object_count,
                                        sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetNeighborAttributes(const sai_neighbor_entry_t*
                                            neighbor_entry, uint32_t *attr_count, sai_attribute_t *attr_list);
sai_status_t xpSaiNeighborGetXpsHostEntry(const xpSaiNeighborEntry_t *nbrEntry,
                                          xpsL3HostEntry_t *xpsHostEntry);

/*
 * Fetch all the unresolved neighbors (no FDB) and update the pkt action.
 */
sai_status_t xpSaiUpdateNbr(xpsDevice_t devId, xpsPktCmd_e unknownUcCmd);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiNeighbor_h_
