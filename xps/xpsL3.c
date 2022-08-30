// xpsL3.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsL3.h"
#include "xpsInit.h"
#include "xpsInternal.h"
#include "xpsNhGrp.h"
#include "xpsInterface.h"
#include "xpsState.h"
#include "xpsVlan.h"
#include "xpsPort.h"
#include "xpsLag.h"
#include "xpsIpGre.h"
#include "xpsSr.h"
#include "xpsIpinIp.h"
#include "xpsScope.h"
#include "xpsLock.h"
#include "xpsGlobalSwitchControl.h"
#include "cpssHalFdb.h"
#include "cpssHalVlan.h"
#include "cpssHalCtrlPkt.h"
#include "cpssHalCopp.h"
#include "cpssHalCounter.h"
#include "xpsAllocator.h"
#include "cpssDxChIp.h"
#include "cpssDxChIpTypes.h"
#include "cpssDxChIpCtrl.h"
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include "cpssHalUtil.h"
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpmTypes.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChIpLpmDbg.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamCommonTypes.h>
#include <cpssCommon/private/prvCpssSkipList.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbRouting.h>
#include "cpssHalDevice.h"
#include "xpsMtuProfile.h"
#include "xpsFdbMgr.h"
#include "cpssHalFdbMgr.h"
#include "cpssHalHostIf.h"
#include "cpssHalMulticast.h"
#include "xpsAcl.h"
#ifdef XP_HEADERS
#include "xpIpv6RouteMgr.h"
#include "xpIpv4RouteMgr.h"
#include "xpIpHostMgr.h"
#include "xpIpNhMgr.h"
#include "xpCtrlMacMgr.h"
#include "xpAllocatorMgr.h"
#include "xpL2DomainMgr.h"
#include "xpLogModXps.h"
#endif
#include <iostream>
#include <iterator>
#include <map>
using namespace std;

map<uint32_t, uint32_t> *gHwNhId2SwNhId = NULL;
XP_STATUS xpsL3InitNhIdMap()
{
    gHwNhId2SwNhId = new map<uint32_t, uint32_t>;
    if (NULL == gHwNhId2SwNhId)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Alloc failed for Nh map");
        return XP_ERR_NULL_POINTER;
    }
    return XP_NO_ERR;
}
XP_STATUS xpsL3GetHwId2SwId(uint32_t hwNhId, uint32_t* swNhId)
{
    if (NULL == gHwNhId2SwNhId)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Nh map not initialized");
        return XP_ERR_NULL_POINTER;
    }
    if ((*gHwNhId2SwNhId).find(hwNhId) != (*gHwNhId2SwNhId).end())
    {
        *swNhId = (*gHwNhId2SwNhId)[hwNhId];
        return XP_NO_ERR;
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "no entry for hwNhId[%u] in HW", hwNhId);
        *swNhId = INVALID_L3_INDEX;
        return XP_ERR_KEY_NOT_FOUND;
    }
    return XP_NO_ERR;
}
XP_STATUS xpsL3SetHwId2SwId(uint32_t hwNhId, uint32_t swNhId)
{
    pair<map<uint32_t, uint32_t>::iterator, bool> ret;
    if (NULL == gHwNhId2SwNhId)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Nh map not initialized");
        return XP_ERR_NULL_POINTER;
    }

    ret = (*gHwNhId2SwNhId).insert(pair<uint32_t, uint32_t>(hwNhId, swNhId));
    if (ret.second==false)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "hwNhId[%u]->swNhId[%u] already exists", ret.first->first, ret.first->second);
        return XP_ERR_KEY_EXISTS;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "new map hwNhId[%u]->swNhId[%u] ", hwNhId, swNhId);
    return XP_NO_ERR;
}
XP_STATUS xpsL3DelHwId2SwId(uint32_t hwNhId)
{
    uint32_t cnt;
    if (NULL == gHwNhId2SwNhId)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Nh map not initialized");
        return XP_ERR_NULL_POINTER;
    }
    cnt = (*gHwNhId2SwNhId).erase(hwNhId);
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "removed %d maps", cnt);
    return XP_NO_ERR;
}

XP_STATUS xpsL3GetHwNhIdCount(uint32_t *cnt)
{
    if (NULL == gHwNhId2SwNhId)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Nh map not initialized");
        return XP_ERR_NULL_POINTER;
    }
    *cnt = (*gHwNhId2SwNhId).size();
    return XP_NO_ERR;
}

extern uint32_t nhGrpMaxSize;
#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include "cpssHalL3.h"

/**********Define all Static Data Structure and Function which are private to this File ******///
/* no initialized parameter value */
#define NOT_INIT_CNS    0xFFFFFFFF

/********** ARP (DaMac) DB For Host and NextHop IPv4/6 Unicast Entries API's which are private to this file ******///

int GetL3CounterIdBase(int pos);
int GetPbrMaxNum();
static xpsDbHandle_t arpDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t mac2meDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t nhSwDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static bool xpsEnableL3FdbMgr = GT_TRUE;

/***********  API Prototypes ***********************/


static XP_STATUS xpsArpInitScope(xpsScope_t scopeId);

static XP_STATUS xpsArpCreateScope(xpsScope_t scopeId, macAddr_t arpDaMac,
                                   xpsArpState_t arpState, xpsArpPointer_t *routerArpIndex);

static XP_STATUS xpsArpDestroyScope(xpsScope_t scopeId,  macAddr_t arpDaMac,
                                    xpsArpState_t arpState);
static XP_STATUS xpsArpGetInfoScope(xpsScope_t scopeId, macAddr_t arpDaMac,
                                    xpsArpInfo_t **info);

static XP_STATUS xpsArpGetArpPointerScope(xpsScope_t scopeId,
                                          macAddr_t arpDaMac, xpsArpPointer_t *routerArpIndex);
static XP_STATUS xpsIpRouterArpAddWrite(xpDevice_t devId,
                                        xpsArpPointer_t routerArpIndex, macAddr_t arpDaMac);
static XP_STATUS xpsIpRouterArpAddDel(xpDevice_t devId,
                                      xpsArpPointer_t routerArpIndex);
static XP_STATUS xpsL3UpdateIpv6UcHostEntry(xpsDevice_t devId, uint32_t index,
                                            xpsL3HostEntry_t *pL3HostEntry);
static XP_STATUS xpsInitMac2MeAllocator(xpsDevice_t xpsDevId);
static XP_STATUS xpsL3SetupRouterVlan(xpsDevice_t devId, xpsVlan_t vlanId);

static XP_STATUS xpsL3AddIpv6UcHostEntry(xpsDevice_t devId, uint32_t *index,
                                         xpsL3HostEntry_t *pL3HostEntry);
XP_STATUS xpsL3ConvertXpsNhEntryToCpssNhEntry(xpsDevice_t devId,
                                              xpsL3NextHopEntry_t *xpsNhEntry, CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *cpssNhEntry);
XP_STATUS xpsL3SetRouteNextHopHw(xpsDevice_t devId, uint32_t hwNhId,
                                 xpsL3NextHopEntry_t *pL3NextHopEntry);
XP_STATUS xpsL3RemoveRouteNhHw(xpsDevice_t devId, uint32_t hwNhId);

/***********  End of API Prototypes ***********************/


XP_STATUS xpsL3InitApi(xpsDevice_t devId)
{
    XP_STATUS                           xpsStatus = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    static bool initL3Done = false;

    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC cpssNhEntry;
    memset(&cpssNhEntry, 0, sizeof(cpssNhEntry));


    if (initL3Done == true)
    {
        return xpsStatus;
    }

    /* LPM miss will point NH index 0 which is reserved for UC.
       Set the default action to DROP. Note, this is different from default
       drop NH created from init, which is used by SONiC Drop pkt-action. */
    cpssNhEntry.entry.regularEntry.cmd = CPSS_PACKET_CMD_DROP_HARD_E;
    rc = cpssHalWriteIpUcRouteEntries(devId, 0, &cpssNhEntry, 1);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to add default UC route nexthop entry :%d ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalCncL3Init(devId);
    if (rc != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Set IP Exceptions pkt cmd to drop.
    NOTE: some execp like TTL, MTU exceed are managed from
    copp rules. Set the rest here. */

    rc = cpssHalIpExceptionCommandSet(devId, CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E,
                                      CPSS_IP_PROTOCOL_IPV4V6_E,
                                      CPSS_PACKET_CMD_DROP_HARD_E);
    if (rc != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(rc);
    }
    rc = cpssHalIpExceptionCommandSet(devId, CPSS_DXCH_IP_EXCP_MC_HDR_ERROR_E,
                                      CPSS_IP_PROTOCOL_IPV4V6_E,
                                      CPSS_PACKET_CMD_DROP_HARD_E);
    if (rc != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(rc);
    }
    rc = cpssHalIpExceptionCommandSet(devId, CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E,
                                      CPSS_IP_PROTOCOL_IPV4V6_E,
                                      CPSS_PACKET_CMD_DROP_HARD_E);
    if (rc != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalIpExceptionCommandSet(devId, CPSS_DXCH_IP_EXCP_MC_ILLEGAL_ADDRESS_E,
                                      CPSS_IP_PROTOCOL_IPV4V6_E,
                                      CPSS_PACKET_CMD_DROP_HARD_E);
    if (rc != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(rc);
    }
    rc = cpssHalIpExceptionCommandSet(devId, CPSS_DXCH_IP_EXCP_UC_ALL_ZERO_SIP_E,
                                      CPSS_IP_PROTOCOL_IPV4V6_E,
                                      CPSS_PACKET_CMD_DROP_HARD_E);
    if (rc != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Init RIF HW IDs*/

    xpsStatus = xpsAllocatorInitIdAllocator(devId,
                                            XP_ALLOC_RIF_HW_ENTRY,
                                            XPS_L3_RESERVED_ROUTER_VLAN(devId),
                                            XPS_L3_RESERVED_ROUTER_VLAN(devId));
    if (xpsStatus != XP_NO_ERR)
    {
        return xpsStatus;
    }

    rc = cpssHalIpPbrBypassRouterTriggerRequirementsEnableSet(devId, GT_TRUE);
    if (rc != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(rc);
    }
    /* Init PBR Leaf Entry IDs*/

    xpsStatus = xpsAllocatorInitIdAllocator(devId,
                                            XP_ALLOC_PBR_HW_ENTRY,
                                            GetPbrMaxNum(),
                                            XPS_L3_PBR_LEAF_RANGE_START);
    if (xpsStatus != XP_NO_ERR)
    {
        return xpsStatus;
    }

    initL3Done = true;
    return xpsStatus;
}


/**
 * \private
 * \fn xpsInitRouterArpDaAndTunnelStartAllocator
 * \brief initialize the allocator for Router Arp Da and Tunnel Start
 *
 * Note: Tunnel Start is not supported.
 *
 * \param [in] devId
 *
 * \return xp_status
 */
static XP_STATUS xpsInitRouterArpDaAndTunnelStartAllocator(xpsDevice_t xpsDevId)
{
    XP_STATUS status = XP_NO_ERR;
    xpsScope_t scope = xpsDevId;

    status = xpsAllocatorInitIdAllocator(scope,
                                         XPS_ALLOCATOR_ROUTER_ARP_DA_AND_TUNNL_START,
                                         XP_ROUTER_ARP_DA_AND_TUNNEL_START_MAX_IDS(xpsDevId),
                                         XP_ROUTER_ARP_DA_AND_TUNNEL_START_RANGE_START);

    return status;
}

/**
 * \private
 * \fn xpsAllocateRouterArpDaAndTunnelStartIndex
 * \brief allocate Arp Da Index
 *
 * Note: Tunnel Start is not supported.
 *
 * \param [in] devId
 * * \param [out] index
 * \return xp_status
 */
static XP_STATUS xpsAllocateRouterArpDaAndTunnelStartIndex(xpsDevice_t xpsDevId,
                                                           uint32_t* index)
{
    xpsScope_t scope = xpsDevId;

    return xpsAllocatorAllocateId(scope,
                                  XPS_ALLOCATOR_ROUTER_ARP_DA_AND_TUNNL_START, index);

}

/**
 * \private
 * \fn static XP_STATUS xpsReleaseRouterArpDaAndTunnelStartIndex(xpsDevice_t xpsDevId, uint32_t index)

 * \brief release Arp Da Index
 *
 * Note: Tunnel Start is not supported.
 *
 * \param [in] devId
 * * \param [in] index
 * \return xp_status
 */
static XP_STATUS xpsReleaseRouterArpDaAndTunnelStartIndex(xpsDevice_t xpsDevId,
                                                          uint32_t index)
{
    xpsScope_t scope = xpsDevId;

    return xpsAllocatorReleaseId(scope, XPS_ALLOCATOR_ROUTER_ARP_DA_AND_TUNNL_START,
                                 index);
}

/**
 * \private
 * \fn arpKeyCompare
 * \brief Statically defined key compare function for Da Mac's
 *        database
 *
 * This API is defined as static, but will be registered via
 * function pointer to be used internally by State manager to do
 * comparisons for the DaMac database
 *
 * \param [in] void* key1
 * \param [in] void* key2
 *
 * \return int32_t
 */
static int32_t arpKeyCompare(void* key1, void* key2)
{
    return (int32_t)(memcmp((macAddr_t *)key1, (macAddr_t *) key2,
                            sizeof(macAddr_t)));
}

/**
 * \private
 * \brief Init API for Arp DB for a scope
 *
 * This API will create and initialize the global arp info
 * database, that will be used to maintain the Arp Pointer (index) to
 * Da MAC relationship
 *
 * \param [in] scopeId
 *
 * \return int
 */
static XP_STATUS xpsArpInitScope(xpsScope_t scopeId)
{

    XP_STATUS status = XP_NO_ERR;

    xpsLockCreate(XP_LOCKINDEX_ARP_LOCK, 1);

    arpDbHndl = XPS_ARP_DB_HNDL;
    // Create Global Interface Db
    if ((status = xpsStateRegisterDb(scopeId, "Arp Db", XPS_GLOBAL, &arpKeyCompare,
                                     arpDbHndl)) != XP_NO_ERR)
    {
        arpDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Global Arp creation failed");
        return status;
    }

    /* initialize the allocator for Router ARP Da*/
    if ((status = xpsInitRouterArpDaAndTunnelStartAllocator(scopeId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to initialize XPS Router ARP DA and Tunnel Start allocator\n");
    }

    return status;
}

/**
 * \private
 * \brief Create a new ARP pointer in the SW DB for a scope.
 *
 * Creates a new ARP Info and stores its ARP Pointer information
 * inside the database
 *
 * \param [in] scopeId
 * \param [in] arpDaMac
 * \param [in] arpState          - indicates if entry is used by Neighbor and/or Host
 * \param [out] routerArpIndex
 *
 * \return int
 */
static XP_STATUS xpsArpCreateScope(xpsScope_t scopeId, macAddr_t arpDaMac,
                                   xpsArpState_t arpState, xpsArpPointer_t *routerArpIndex)
{
    XP_STATUS status = XP_NO_ERR;
    xpsArpInfo_t *info = NULL;

    /* Creates ARP entry for neighbor or host type*/
    if (arpState == ARP_NOT_ACTIVE)
    {
        return XP_ERR_INVALID_VALUE;
    }


    //Check first if Entry already exists. If Exist, mark its state to be Neibhor/NextHop or both.
    status =  xpsArpGetInfoScope(scopeId, arpDaMac, &info);


    if (info != NULL)
    {
        info->arpState |= arpState;
        info->refCount++;

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "updt arp " FMT_MAC" st %d cnt %d",
              PRI_MAC(arpDaMac),
              info->arpState, info->refCount);
        *routerArpIndex = info->arpPointer;
        return XP_ERR_KEY_EXISTS;
    }

    if ((status = xpsAllocateRouterArpDaAndTunnelStartIndex(scopeId,
                                                            routerArpIndex)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Arp Next Hop Pointer allocation is failed");
        return status;
    }

    // Allocate space for the ARP info struct
    if ((status = xpsStateHeapMalloc(sizeof(xpsArpInfo_t),
                                     (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Space allocation for ARP info failed");
        return status;
    }

    if (!info)
    {
        status = XP_ERR_NULL_POINTER;
        return status;
    }
    memset(info, 0, sizeof(xpsArpInfo_t));

    info->arpPointer = *routerArpIndex;
    memcpy(info->keyDaMac, arpDaMac, sizeof(macAddr_t));
    info->arpState |= arpState;
    info->refCount++;

    // Insert the arp info structure into the database, using the Da Mac as a key
    if ((status = xpsStateInsertData(scopeId, arpDbHndl, (void*)info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Insertion of arp pointer info failed");
        // Free Allocated memory
        xpsStateHeapFree((void*)info);
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "create arp " FMT_MAC " st %d cnt %d",
          PRI_MAC(arpDaMac),
          info->arpState, info->refCount);

    return status;
}

/**
 * \private
 * \brief Create a new ARP pointer in the SW DB && HW for a scope.
 *
 * Creates a new ARP Info and stores its ARP Pointer information
 * inside the database
 *
 * \param [in] devId
 * \param [in] arpDaMac
 * \param [in] arpState          - indicates if entry is used by Neighbor and/or Host
 * \param [out] routerArpIndex
 *
 * \return int
 */

static XP_STATUS xpsArpCreate(xpDevice_t devId, macAddr_t arpDaMac,
                              xpsArpState_t arpState, xpsArpPointer_t *routerArpIndex)
{
    XP_STATUS status;

    status = xpsArpCreateScope(XP_SCOPE_DEFAULT, arpDaMac, arpState,
                               routerArpIndex);

    /* Don't add to HW if Entry exists or due to some other failures*/
    if (status == XP_ERR_KEY_EXISTS)
    {
        return XP_NO_ERR;
    }
    if (status != XP_NO_ERR)
    {
        return status;
    }

    /* write the arpDaMac to HW for this devId*/
    status =  xpsIpRouterArpAddWrite(devId, *routerArpIndex, arpDaMac);

    return status;
}

/**
 * \private
 * \brief Destroys an existing ARP Pointer in the system for a
 *        scope
 *
 * Destroys an existing arp Pointer and removes the information
 * stored from the database
 *
 * \param [in] scopeId
 * \param [in] arpDaMac
 *
 * \return int
 */
static XP_STATUS xpsArpDestroyScope(xpsScope_t scopeId, macAddr_t arpDaMac,
                                    xpsArpState_t arpState)
{
    XP_STATUS status = XP_NO_ERR;
    xpsArpInfo_t arpInfoKey;
    xpsArpInfo_t *info = NULL;
    xpsArpPointer_t routerArpIndex;

    /* Deletes ARP entry for neighbor or host type only */
    if (arpState == ARP_NOT_ACTIVE)
    {
        return XP_ERR_INVALID_VALUE;
    }

    //Check first if Entry is already exists. If Exists, remove its state.Otherwise, destroy it.
    status =  xpsArpGetInfoScope(scopeId, arpDaMac, &info);

    /*ArpInfo is not found for arpDaMac */
    if (info == NULL)
    {
        return XP_ERR_NOT_FOUND;
    }

    // First clear its state as requested.
    info->refCount--;
    info->arpState &= ~arpState;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "destroy arp " FMT_MAC "st %d cnt %d",
          PRI_MAC(arpDaMac),
          info->arpState, info->refCount);
    /* Check if it is still used by host or neighbor*/
    if ((info->arpState != ARP_NOT_ACTIVE) || info->refCount)
    {
        return XP_ERR_KEY_EXISTS;
    }

    memcpy(arpInfoKey.keyDaMac, arpDaMac, sizeof(macAddr_t));

    // Remove the interface from the db
    if ((status = xpsStateDeleteData(scopeId, arpDbHndl, (xpsDbKey_t)&arpInfoKey,
                                     (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Removal of Arp info failed \n");
        return status;
    }

    if (info == NULL)
    {
        return XP_ERR_NOT_FOUND;
    }

    routerArpIndex = info->arpPointer;

    // Release the Nexthop Arp Pointer back into the free pool
    if ((status = xpsReleaseRouterArpDaAndTunnelStartIndex(scopeId,
                                                           routerArpIndex)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Release of Nexthop Arp pointer %d failed", routerArpIndex);
        return status;
    }

    // Free the allocated memory
    if ((status = xpsStateHeapFree((void*)info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Freeing allocated memory failed");
    }

    return status;
}

/**
 * \private xpsArpDelete
 * \brief Deletes ARP pointer from SW DB && HW
 *
 * Clears the state of the MAC DA in SW DB. If DAMAC is not used by neighbor and host.
 * It will remove it from SW DB and HW.
 *
 * \param [in] devId
 * \param [in] arpDaMac
 * \param [in] arpState          - indicates if entry is used by Neighbor and/or Host
 * \param [out] routerArpIndex
 *
 * \return int
 */

static XP_STATUS xpsArpDelete(xpDevice_t devId, macAddr_t arpDaMac,
                              xpsArpState_t arpState)
{
    XP_STATUS status;
    xpsArpPointer_t routerArpIndex;

    /* Find the routerArpIndex first*/
    if ((status = xpsArpGetArpPointerScope(XP_SCOPE_DEFAULT, arpDaMac,
                                           &routerArpIndex)) == XP_NO_ERR)
    {
        // Delete Arp Entry only if entry exist
        status = xpsArpDestroyScope(XP_SCOPE_DEFAULT, arpDaMac, arpState);

        /* Don't delete from HW if Entry already still being used or due to some other failures*/
        if (status != XP_NO_ERR)
        {
            /* Entry is still being used by either host or neighbor so clear its status*/
            if (status == XP_ERR_KEY_EXISTS)
            {
                return XP_NO_ERR;
            }
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsArpDestroyScope Failed : %d \n", status);
            return status;
        }

        /* write the ARP DA MAC to HW for this devId*/
        status =  xpsIpRouterArpAddDel(devId, routerArpIndex);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Ip router arp address delete failed");
            return status;
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpsArpGetArpPointer(xpsDevice_t devId, macAddr_t arpDaMac,
                              xpsArpPointer_t *routerArpIndex)
{
    return xpsArpGetArpPointerScope(XP_SCOPE_DEFAULT, arpDaMac, routerArpIndex);
}

/**
 * \private
 * \brief Retrieve an ARP Information's properties for a scope
 *
 * Fetches information about an existing ARP created in
 * the system
 *
 *
 * \param [in] scopeId
 * \param [in] arpDaMac
 * \param [out] info
 *
 * \return int
 */
static XP_STATUS xpsArpGetInfoScope(xpsScope_t scopeId, macAddr_t arpDaMac,
                                    xpsArpInfo_t **info)
{
    XP_STATUS status = XP_NO_ERR;
    xpsArpInfo_t arpInfoKey;

    memcpy(arpInfoKey.keyDaMac, arpDaMac, sizeof(macAddr_t));

    if ((status = xpsStateSearchData(scopeId, arpDbHndl, (xpsDbKey_t)&arpInfoKey,
                                     (void**)info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search Arp Pointer in list, Da MAC Address " FMT_MAC "\n",
              PRI_MAC(arpDaMac));
    }

    return status;
}

/**
 * \private
 * \brief Retrieve's an ARP's Next Hop ARP pointer information
 *        for a scope
 *
 * There is no set ARP Pointer API. In order to change an ARP's
 * Next Hop Arp Pointer, the ARP Node must be destroyed and
 * recreated
 *
 * \param [in] scopeId
 * \param [in] arpDaMac
 * \param [out]  routerArpIndex
 *
 * \return int
 */
static XP_STATUS xpsArpGetArpPointerScope(xpsScope_t scopeId,
                                          macAddr_t arpDaMac, xpsArpPointer_t *routerArpIndex)
{
    XP_STATUS status = XP_NO_ERR;
    xpsArpInfo_t *info = NULL;
    xpsArpInfo_t arpInfoKey;

    memcpy(arpInfoKey.keyDaMac, arpDaMac, sizeof(macAddr_t));

    if ((status = xpsStateSearchData(scopeId, arpDbHndl, (xpsDbKey_t)&arpInfoKey,
                                     (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search Arp Pointer in list, Da MAC Address " FMT_MAC " \n",
              PRI_MAC(arpDaMac));
        return status;
    }

    if (!info)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "Info not found DMAC  %2x:%2x:%2x:%2x:%2x:%2x \n",
              arpDaMac[5], arpDaMac[4], arpDaMac[3], arpDaMac[2], arpDaMac[1], arpDaMac[0]);
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    *routerArpIndex = info->arpPointer;

    return status;

}

/**
 *  xpsIpRouterArpAddrWrite function
 * @endinternal
 *
 * @brief   write a ARP MAC address to the router ARP / Tunnel start / NAT table (for NAT capable devices)
 *
 * @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                    - the device number
 * @param[in] routerArpIndex           - The Arp Address index (to be inserted later
 *                                      in the UC Route entry Arp nextHopARPPointer
 *                                       field)
 * @param[in] arpMacAddrPtr            - the ARP MAC address to write
 */
static XP_STATUS xpsIpRouterArpAddWrite(xpDevice_t devId,
                                        xpsArpPointer_t routerArpIndex, macAddr_t arpDaMac)
{
    GT_STATUS cpssStatus = GT_OK;

    cpssStatus = cpssHalWriteL3IpRouterArpAddress(devId, routerArpIndex, arpDaMac);
    return xpsConvertCpssStatusToXPStatus(cpssStatus);
}

/**
 *  xpsIpRouterArpAddrRead function
 * @endinternal
 *
 * @brief   read a ARP MAC address to the router ARP / Tunnel
 *          start / NAT table (for NAT capable devices)
 *
 * @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                    - the device number
 * @param[in] routerArpIndex           - The Arp Address index (to be inserted later
 *                                      in the UC Route entry Arp nextHopARPPointer
 *                                       field)
 * @param[out] arpMacAddrPtr            - the ARP MAC address to
 *       write
 */
XP_STATUS xpsIpRouterArpAddRead(xpDevice_t devId,
                                xpsArpPointer_t routerArpIndex, macAddr_t *arpDaMac)
{
    GT_ETHERADDR arpMacAddr;
    GT_STATUS cpss_status;

    /* call device specific API */
    cpss_status = cpssDxChIpRouterArpAddrRead(devId, routerArpIndex, &arpMacAddr);

    if (cpss_status == GT_OK)
    {
        /* write a ARP MAC address to the Router ARP Table */
        memcpy(arpDaMac, arpMacAddr.arEther, sizeof(macAddr_t));
    }

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

/**
 *  xpsIpRouterArpAddrDel function
 * @endinternal
 *
 * @brief   Deletes ARP MAC address to the router ARP / Tunnel start / NAT table (for NAT capable devices)
 *
 * @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                    - the device number
 * @param[in] routerArpIndex           - The Arp Address index (to be inserted later
 *                                      in the UC Route entry Arp nextHopARPPointer
 *                                       field)
 */
static XP_STATUS xpsIpRouterArpAddDel(xpDevice_t devId,
                                      xpsArpPointer_t routerArpIndex)
{
    macAddr_t arpDaMac;
    GT_STATUS cpssStatus = GT_OK;

    memset(&arpDaMac, 0, sizeof(macAddr_t));

    cpssStatus = cpssHalWriteL3IpRouterArpAddress(devId, routerArpIndex, arpDaMac);

    return xpsConvertCpssStatusToXPStatus(cpssStatus);
}






///************* L3 subinterface Create/Delete/Properties Set/Get Start ***************************
XP_STATUS xpsL3CreateSubIntf(xpsInterfaceId_t *l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3CreateSubIntfScope(xpsScope_t scopeId,
                                  xpsInterfaceId_t *l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3DestroySubIntf(xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3DestroySubIntfScope(xpsScope_t scopeId,
                                   xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3InitSubIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3DeInitSubIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3BindSubIntf(xpsInterfaceId_t portIntfId,
                           xpsInterfaceId_t l3IntfId, xpsVlan_t encapId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3BindSubIntfScope(xpsScope_t scopeId, xpsInterfaceId_t portIntfId,
                                xpsInterfaceId_t l3IntfId, xpsVlan_t encapId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3UnBindSubIntf(xpsInterfaceId_t portIntfId,
                             xpsInterfaceId_t l3IntfId, xpsVlan_t encapId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3UnBindSubIntfScope(xpsScope_t scopeId,
                                  xpsInterfaceId_t portIntfId, xpsInterfaceId_t l3IntfId, xpsVlan_t encapId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


/////********************************L3 Interface Create/Delete/Properties Set/Get  Start********************************/////

////*********************************L3 Port Interface Create/Remove/Init/DeInit/Bind/UnBind****************************/////

static XP_STATUS xpsL3ResetInterfaceMapping(xpsScope_t scopeId,
                                            xpsInterfaceInfo_t *intfInfo)
{
    /* Check if pointers are valid*/
    if (!intfInfo)
    {
        return XP_ERR_NULL_POINTER;
    }

    /* Clear interface mapping information*/
    intfInfo->intfMappingInfo.type = XPS_INVALID_IF_TYPE;
    intfInfo->intfMappingInfo.keyIntfId = XPS_INTF_INVALID_ID;

    return XP_NO_ERR;
}
static XP_STATUS xpsL3ValidateInterfaceMapping(xpsScope_t scopeId,
                                               xpsInterfaceInfo_t *l3IntfInfo, xpsInterfaceInfo_t *portIntfInfo)
{
    /* Check if pointers are valid*/
    if (!l3IntfInfo || !portIntfInfo)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (portIntfInfo->keyIntfId == 0)
    {
        return XP_NO_ERR;
    }

    /*Check if portInterface is already mapped to L3Interface */
    if ((l3IntfInfo->intfMappingInfo.type == portIntfInfo->type) &&
        (l3IntfInfo->intfMappingInfo.keyIntfId == portIntfInfo->keyIntfId))
    {
        return XP_ERR_FAILED;
    }

    /*Check if L3interface is already mapped to portInterface */
    if ((portIntfInfo->intfMappingInfo.type == l3IntfInfo->type) &&
        (portIntfInfo->intfMappingInfo.keyIntfId == l3IntfInfo->keyIntfId))
    {
        return XP_ERR_FAILED;
    }

    return XP_NO_ERR;
}
/* Store the mapping between an L3 interface to ePort interface in both directions*/
static XP_STATUS xpsL3StoreInterfaceMapping(xpsScope_t scopeId,
                                            xpsInterfaceInfo_t *l3IntfInfo, xpsInterfaceInfo_t *portIntfInfo)
{
    /* Check if pointers are valid*/
    if (!l3IntfInfo || !portIntfInfo)
    {
        return XP_ERR_NULL_POINTER;
    }

    /* Map PortInterface to L3Interface*/
    l3IntfInfo->intfMappingInfo.type = portIntfInfo->type;
    l3IntfInfo->intfMappingInfo.keyIntfId = portIntfInfo->keyIntfId;

    /* Map L3Interface with portInterface*/
    portIntfInfo->intfMappingInfo.type = l3IntfInfo->type;
    portIntfInfo->intfMappingInfo.keyIntfId = l3IntfInfo->keyIntfId;

    return XP_NO_ERR;
}

/* Retrieve the physical interface corresponding to an l3 interface */
XP_STATUS xpsL3RetrieveInterfaceMapping(xpsScope_t scopeId,
                                        xpsInterfaceId_t keyIntfId, xpsInterfaceId_t *mappedIntfId)
{
    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t *intfInfo = NULL;

    /* Check if pointer is valid*/
    if (!mappedIntfId)
    {
        return XP_ERR_NULL_POINTER;
    }

    /* get the interface information for the mapped interface*/
    status = xpsInterfaceGetInfoScope(scopeId, keyIntfId, &intfInfo);

    if ((status != XP_NO_ERR) || (intfInfo == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface info for Interface id : %d\n", keyIntfId);
        return status;
    }

    /* Check if there is an inteface mapping on this interface Information*/
    if (intfInfo->intfMappingInfo.type == XPS_INVALID_IF_TYPE)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    *mappedIntfId = intfInfo->intfMappingInfo.keyIntfId;

    return XP_NO_ERR;
}

static XP_STATUS xpsL3RemoveInterfaceMapping(xpsScope_t scopeId,
                                             xpsInterfaceInfo_t *intfInfo)
{
    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceId_t mappedKeyIntfId;
    xpsInterfaceInfo_t *mappedIntfInfo = NULL;

    if (!intfInfo)
    {
        return XP_ERR_NULL_POINTER;
    }

    //Interface is not mapped
    if (intfInfo->intfMappingInfo.type == XPS_INVALID_IF_TYPE)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    mappedKeyIntfId = intfInfo->intfMappingInfo.keyIntfId;

    /* get the interface information for the mapped interface*/
    status = xpsInterfaceGetInfoScope(scopeId, mappedKeyIntfId, &mappedIntfInfo);

    if ((status != XP_NO_ERR) || (mappedIntfInfo == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface info for Interface id : %d\n", mappedKeyIntfId);
        return status;
    }

    /* Clear the mapping on this interface information*/
    xpsL3ResetInterfaceMapping(scopeId, intfInfo);

    /*Clear the mapping on the mapped interface*/
    xpsL3ResetInterfaceMapping(scopeId, mappedIntfInfo);

    return XP_NO_ERR;
}

XP_STATUS xpsL3CreatePortIntf(xpsInterfaceId_t *l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;
    status = xpsL3CreatePortIntfScope(XP_SCOPE_DEFAULT, l3IntfId);

    // We are just creating an interface, no need to update LPM routes.
    // we are calling to program the mtu profile index in interface
    if (status == XP_NO_ERR)
    {
        status = xpsMtuSetDefaultL3InterfaceMtuSize(0, *l3IntfId);
    }
    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsL3CreatePortIntfScope(xpsScope_t scopeId,
                                   xpsInterfaceId_t *l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    if (!l3IntfId)
    {
        return XP_ERR_NULL_POINTER;
    }

    status = xpsInterfaceCreateScope(scopeId, XPS_PORT_ROUTER, l3IntfId);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsL3DestroyPortIntf(xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status = xpsL3DestroyPortIntfScope(XP_SCOPE_DEFAULT, l3IntfId);

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsL3DestroyPortIntfScope(xpsScope_t scopeId,
                                    xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status = xpsInterfaceDestroyScope(scopeId, l3IntfId);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsL3InitPortIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3DeInitPortIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3BindPortIntf(xpsInterfaceId_t portIntfId,
                            xpsInterfaceId_t l3IntfId)
{
    return xpsL3BindPortIntfScope(XP_SCOPE_DEFAULT, portIntfId, l3IntfId);
}

XP_STATUS xpsL3GetBindPortIntf(xpsInterfaceId_t portIntfId,
                               xpsInterfaceId_t *l3IntfId)
{
    return xpsL3GetBindPortIntfScope(XP_SCOPE_DEFAULT, portIntfId, l3IntfId);
}

XP_STATUS xpsL3BindPortIntfScope(xpsScope_t scopeId,
                                 xpsInterfaceId_t portIntfId, xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceType_e l3IntfType, portIntfType;
    xpsDevice_t    portDevId = 0;
    xpPort_t      portId = 0;
    xpsInterfaceInfo_t *l3IntfInfo = NULL, *portIntfInfo = NULL;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum, i;
    xpsLagPortIntfList_t lagPortList;
    xpsInterfaceId_t bindPortIntf;
    GT_STATUS cpssStatus = GT_OK;
    uint32_t vlanId = 0;

    memset(&lagPortList, 0, sizeof(lagPortList));
    // Check if both the interface types are valid
    status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &l3IntfInfo);
    if ((status != XP_NO_ERR) || (l3IntfInfo == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface information for L3 Interface id : %d\n", l3IntfId);
        return status;
    }

    l3IntfType = l3IntfInfo->type;

    if (l3IntfType != XPS_PORT_ROUTER)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Interface %d is not t port router interface \n", l3IntfId);
        return XP_ERR_INVALID_PARAMS;
    }

    status = xpsInterfaceGetInfoScope(scopeId, portIntfId, &portIntfInfo);

    if ((status != XP_NO_ERR) || (portIntfInfo == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface information for Port Interface id : %d\n", portIntfId);
        return status;
    }

    portIntfType = portIntfInfo->type;
    if (portIntfType != XPS_PORT && portIntfType != XPS_LAG)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Interface %d is not a port interface \n", l3IntfId);
        return XP_ERR_INVALID_PARAMS;
    }

    /* Check if same interface mapping is exist*/
    status = xpsL3ValidateInterfaceMapping(scopeId, l3IntfInfo, portIntfInfo);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Inteface is already mapped\n");
        return status;
    }

    status = xpsAllocatorAllocateId(scopeId, XP_ALLOC_RIF_HW_ENTRY, &vlanId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAllocatorAllocateId Failed : %d\n", status);
        return status;
    }

    if (portIntfType == XPS_PORT)
    {
        lagPortList.portIntf[0] = portIntfId;
        lagPortList.size = 1;
    }
    else if (portIntfType == XPS_LAG)
    {
        memset(&lagPortList, 0, sizeof(lagPortList));
        if ((status = xpsLagGetPortIntfListScope(scopeId, portIntfId,
                                                 &lagPortList)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed, lag interface(%d)", portIntfId);
            return status;
        }
        /* Update L3 interface value to Lag DB so that it will be used to update BD of newly added ports
         * after L3 interface is created on LAG
         */
        if ((status = xpsLagSetL3IntfToDbScope(scopeId, portIntfId,
                                               l3IntfId)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Updating L3 interface to, lag interface(%d) DB failed", portIntfId);
            return status;
        }
    }

    if ((status = xpsL3SetupRouterVlan(portDevId, vlanId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Setting up Router Vlan failed for vlanId: %d", vlanId);
        return status;
    }
    for (i = 0; i < lagPortList.size; i++)
    {
        bindPortIntf = lagPortList.portIntf[i];

        // Get device and local port id for the device the port belongs to
        // TODO: Is this call required? or xpsPort DB to be obsoleted?
        status = xpsPortGetDevAndPortNumFromIntfScope(scopeId, bindPortIntf, &portDevId,
                                                      &portId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port and dev Id from port intf : %d \n", bindPortIntf);
            return status;
        }

        cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
        cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

        cpssStatus = cpssHalL3BindPortIntf(cpssDevId, cpssPortNum, vlanId);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to bind l3 to port : %d \n", portIntfId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    /* Store bdHWId in xpsRIf DB*/
    l3IntfInfo->bdHWId = vlanId;
    // Store the mapping in state
    status = xpsL3StoreInterfaceMapping(scopeId, l3IntfInfo, portIntfInfo);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to store mapping for L3 intf %d and Port Intf %d\n", l3IntfId,
              portIntfId);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetBindPortIntfScope(xpsScope_t scopeId,
                                    xpsInterfaceId_t portIntfId, xpsInterfaceId_t *l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;

    status = xpsL3RetrieveInterfaceMapping(scopeId, portIntfId, l3IntfId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get L3 interface Mapping Information for interface %d\n",
              portIntfId);
        return status;
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsL3UnBindPortIntf(xpsInterfaceId_t portIntfId)
{
    return xpsL3UnBindPortIntfScope(XP_SCOPE_DEFAULT, portIntfId);
}

XP_STATUS xpsL3UnBindPortIntfScope(xpsScope_t scopeId,
                                   xpsInterfaceId_t portIntfId)
{
    XPS_FUNC_ENTRY_LOG();
    xpsInterfaceType_e portIntfType;
    XP_STATUS status = XP_NO_ERR;
    xpsDevice_t portDevId;
    xpsPort_t portId;
    xpsInterfaceInfo_t *portIntfInfo = NULL;
    xpsInterfaceInfo_t *l3IntfInfo = NULL;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum, i;
    xpsLagPortIntfList_t lagPortList;
    xpsInterfaceId_t bindPortIntf;
    GT_STATUS cpssStatus = GT_OK;
    xpsInterfaceId_t l3IntfId = 0;
    memset(&lagPortList, 0, sizeof(lagPortList));

    // Check if both the interface types are valid
    status = xpsInterfaceGetInfoScope(scopeId, portIntfId, &portIntfInfo);
    if ((status != XP_NO_ERR) || (portIntfInfo == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface information for Interface id : %d\n", portIntfId);
        return status;
    }

    l3IntfId = portIntfInfo->intfMappingInfo.keyIntfId;

    // Check if both the interface types are valid
    status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &l3IntfInfo);
    if ((status != XP_NO_ERR) || (l3IntfInfo == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface information for L3 Interface id : %d\n", l3IntfId);
        return status;
    }

    portIntfType = portIntfInfo->type;

    if (portIntfType != XPS_PORT && portIntfType != XPS_LAG)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid interface type %d for port interface id %d\n", portIntfType,
              portIntfId);
        return XP_ERR_INVALID_ARG;
    }
    if (portIntfType == XPS_PORT)
    {
        lagPortList.portIntf[0] = portIntfId;
        lagPortList.size = 1;
    }
    else if (portIntfType == XPS_LAG)
    {
        if ((status = xpsLagGetPortIntfListScope(scopeId, portIntfId,
                                                 &lagPortList)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed, lag interface(%d)", portIntfId);
            return status;
        }

        /* Set Invalid  L3 interface value to LAG DB */
        if ((status = xpsLagSetL3IntfToDbScope(scopeId, portIntfId,
                                               XPS_INTF_INVALID_ID)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " Setting Invalid L3 interface to lag interface(%d) DB failed", portIntfId);
            return status;
        }
    }

    for (i = 0; i < lagPortList.size; i++)
    {
        bindPortIntf = lagPortList.portIntf[i];

        // Retrieve the port and devId from the port interface
        // TODO: Is this call required? or xpsPort DB to be obsoleted?
        status = xpsPortGetDevAndPortNumFromIntfScope(scopeId, bindPortIntf, &portDevId,
                                                      &portId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port and dev Id from port intf : %d \n", bindPortIntf);
            return status;
        }

        cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
        cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

        cpssStatus = cpssHalL3UnBindPortIntf(cpssDevId, cpssPortNum);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to unbind l3 to port intf : %d \n", portIntfId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    // Remove the interface mapping information from the state
    status = xpsL3RemoveInterfaceMapping(scopeId, portIntfInfo);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to remove interface mapping information for interface Id : %d",
              portIntfId);
        return status;
    }

    status = xpsAllocatorReleaseId(scopeId, XP_ALLOC_RIF_HW_ENTRY,
                                   l3IntfInfo->bdHWId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to release RIF HW index. Status : %d\n", status);
        return status;
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

////*********************************L3 Port Interface End ****************************/////
////*********************************L3 VLAN Interface Create/Remove/Init/DeInit/Bind/UnBind****************************/////
XP_STATUS xpsL3CreateVlanIntf(xpsVlan_t vlanId, xpsInterfaceId_t *l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;
    status = xpsL3CreateVlanIntfScope(XP_SCOPE_DEFAULT, vlanId, l3IntfId);

    if (status == XP_NO_ERR)
    {
        status = xpsMtuSetDefaultL3InterfaceMtuSize(0, *l3IntfId);
    }

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsL3CreateVlanIntfScope(xpsScope_t scopeId, xpsVlan_t vlanId,
                                   xpsInterfaceId_t *l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();
    GT_STATUS cpss_status = GT_OK;
    XP_STATUS status = XP_NO_ERR;
    int devId = 0;

    if (!l3IntfId)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }

    status = xpsVlanIsExist(scopeId, vlanId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan does not exist");
        return status;
    }

    status = xpsInterfaceCreateRouterOverVlanScope(scopeId, (uint32_t)vlanId,
                                                   l3IntfId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Create router over vlan failed");
        return status;
    }

    cpss_status = cpssHalBrgVlanIpUcRouteEnable(devId, vlanId);
    if (xpsConvertCpssStatusToXPStatus(cpss_status) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "enabling routing on vlan failed ");
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    cpss_status = cpssHalBrgVlanIpMcRouteEnable(devId, vlanId,
                                                CPSS_IP_PROTOCOL_IPV4V6_E,
                                                GT_TRUE);
    if (cpss_status !=GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "IPMC Route failed");
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    if (cpssHalIsEbofDevice(devId))
    {
        cpss_status = cpssHalEnableBrgVlanIpV6IcmpToCpu(devId, vlanId, GT_TRUE);
        if (cpss_status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Program ICMPv6 Cmd failed for vlan  %d: Cpss RC: %d\n, vlanId, cpssStatus");
            return xpsConvertCpssStatusToXPStatus(cpss_status);
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3DestroyVlanIntf(xpsVlan_t vlanId, xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();
    GT_STATUS cpss_status = GT_OK;
    XP_STATUS status = XP_NO_ERR;
    xpsDevice_t devId =0;

    status =  xpsL3DestroyVlanIntfScope(XP_SCOPE_DEFAULT, vlanId, l3IntfId);
    if (status != XP_NO_ERR)
    {
        return status;
    }
    // TODO: Need to loop/ITER through the deviceID
    cpss_status = cpssDxChBrgVlanIpUcRouteEnable(0, vlanId,
                                                 CPSS_IP_PROTOCOL_IPV4V6_E,
                                                 GT_FALSE);

    cpss_status = cpssHalBrgVlanIpMcRouteEnable(devId, vlanId,
                                                CPSS_IP_PROTOCOL_IPV4V6_E,
                                                GT_FALSE);
    if (cpss_status !=GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "IPMC Route failed");
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    if (cpssHalIsEbofDevice(devId))
    {
        cpss_status = cpssHalEnableBrgVlanIpV6IcmpToCpu(devId, vlanId, GT_FALSE);
        if (cpss_status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Program ICMPv6 Cmd failed for vlan  %d: Cpss RC: %d\n, vlanId, cpssStatus");
            return xpsConvertCpssStatusToXPStatus(cpss_status);
        }
    }

    status = xpsVlanSetRoutingEnable(devId, vlanId, XPS_PROTOCOL_IPV4_E, false,
                                     false);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Updating Vlan Context Db failed, vlanId(%d)", vlanId);
        return status;
    }
    status = xpsVlanSetRoutingEnable(devId, vlanId, XPS_PROTOCOL_IPV6_E, false,
                                     false);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Updating Vlan Context Db failed, vlanId(%d)", vlanId);
        return status;
    }

    status = xpsVlanSetRoutingEnable(devId, vlanId, XPS_PROTOCOL_IPV4_E, true,
                                     false);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Updating Vlan Context Db failed, vlanId(%d)", vlanId);
        return status;
    }
    status = xpsVlanSetRoutingEnable(devId, vlanId, XPS_PROTOCOL_IPV6_E, true,
                                     false);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Updating Vlan Context Db failed, vlanId(%d)", vlanId);
        return status;
    }
#if 0

    cpss_status = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                               CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_BCAST_E,
                                               CPSS_PACKET_CMD_FORWARD_E);
    if (cpss_status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalVlanUnkUnregFilterSet Failed");
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    cpss_status = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                               CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_MCAST_E,
                                               CPSS_PACKET_CMD_FORWARD_E);
    if (cpss_status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalVlanUnkUnregFilterSet Failed");
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    cpss_status = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                               CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV6_MCAST_E,
                                               CPSS_PACKET_CMD_FORWARD_E);
    if (cpss_status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalVlanUnkUnregFilterSet Failed");
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    cpss_status = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                               CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IPV4_BCAST_E,
                                               CPSS_PACKET_CMD_FORWARD_E);
    if (cpss_status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalVlanUnkUnregFilterSet Failed");
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }
#endif

    cpss_status = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                               CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E,
                                               CPSS_PACKET_CMD_FORWARD_E);
    if (cpss_status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalVlanUnkUnregFilterSet Failed");
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }


    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

XP_STATUS xpsL3DestroyVlanIntfScope(xpsScope_t scopeId, xpsVlan_t vlanId,
                                    xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status = xpsVlanIsExist(scopeId, vlanId);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    status = xpsInterfaceDestroyRouterOverVlanScope(scopeId, (uint32_t)vlanId,
                                                    l3IntfId);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    //status = xpsMcDestroyL3DomainId(XPS_INTF_MAP_INTF_TO_BD(l3IntfId));

    XPS_FUNC_EXIT_LOG();

    return status;
}

////*********************************L3 VLAN Interface End****************************/////
////*********************************L3 Tunnel Interface Create/Remove/Init/DeInit/Bind/UnBind****************************/////
XP_STATUS xpsL3CreateTunnelIntf(xpsInterfaceId_t *l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3CreateTunnelIntfScope(xpsScope_t scopeId,
                                     xpsInterfaceId_t *l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3DestroyTunnelIntf(xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3DestroyTunnelIntfScope(xpsScope_t scopeId,
                                      xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3InitTunnelIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3DeInitTunnelIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3BindTunnelIntf(xpsDevice_t devId, xpsInterfaceId_t tnnlIntfId,
                              xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3UnBindTunnelIntf(xpsDevice_t devId, xpsInterfaceId_t tnnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

////*********************************L3 Tunnel Interface End****************************/////
////*********************************VPN Interface Start****************************/////
XP_STATUS xpsL3CreateVpnIntf(xpsInterfaceId_t *vpnIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3CreateVpnIntfScope(xpsScope_t scopeId,
                                  xpsInterfaceId_t *vpnIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3DestroyVpnIntf(xpsInterfaceId_t vpnIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3DestroyVpnIntfScope(xpsScope_t scopeId,
                                   xpsInterfaceId_t vpnIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3InitVpnIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3DeInitVpnIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3BindVpnIntfToLabel(xpsDevice_t devId,  uint32_t vpnLabel,
                                  xpsInterfaceId_t vpnIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3UnBindVpnIntfFromLabel(xpsDevice_t devId, uint32_t vpnLabel)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetVpnIntfVrf(xpsDevice_t devId, xpsInterfaceId_t vpnIntfId,
                             xpsInterfaceId_t vrfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetVpnIntfVrf(xpsDevice_t devId, xpsInterfaceId_t vpnIntfId,
                             uint32_t *vrfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


////*********************************VPN Interface End****************************/////

//// ********************************L3 Interface Properties ***************************////

static int32_t mac2meKeyCompare(void* key1, void* key2)
{
    xpsMac2MeInfo_t *entry1 = (xpsMac2MeInfo_t *)key1;
    xpsMac2MeInfo_t *entry2 = (xpsMac2MeInfo_t *)key2;

    if (entry1->l3IntfId < entry2->l3IntfId)
    {
        return -1;
    }
    else if (entry1->l3IntfId > entry2->l3IntfId)
    {
        return 1;
    }
    else
    {
        return (int32_t)(memcmp((macAddr_t *)entry1->mac, (macAddr_t *) entry2->mac,
                                sizeof(macAddr_t)));
    }
}

static int32_t xpsNhKeyComp(void* key1, void* key2)
{
    return ((int32_t)(((xpsL3NhSwDbEntry_t*)(key1))->swNhId - ((
                                                                   xpsL3NhSwDbEntry_t*)(key2))->swNhId));
}

static XP_STATUS xpsInitMac2MeAllocator(xpsDevice_t xpsDevId)
{
    uint32_t mac2MeDepth = 0;
    GT_STATUS cpssStatus = GT_OK;
    XP_STATUS status = XP_NO_ERR;

    // Get MAC2ME table depth
    cpssStatus = cpssDxChCfgTableNumEntriesGet(xpsDevId,
                                               CPSS_DXCH_CFG_TABLE_MAC_TO_ME_E, &mac2MeDepth);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get MAC2ME table depth. Device : %d Status : %d\n", xpsDevId,
              cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    mac2meDbHndl = XPS_MAC2ME_DB_HNDL;
    if ((status = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Mac2Me Db", XPS_GLOBAL,
                                     &mac2meKeyCompare,
                                     mac2meDbHndl)) != XP_NO_ERR)
    {
        mac2meDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Mac2me Db creation failed");
        return status;
    }

    return xpsAllocatorInitIdAllocator(xpsDevId, XPS_ALLOCATOR_MAC2ME_TBL,
                                       mac2MeDepth,
                                       XPS_L3_MAC2ME_RANGE_START);
}
static XP_STATUS xpsInitNhAllocator(xpsDevice_t xpsDevId)
{
    XP_STATUS status = XP_NO_ERR;

    nhSwDbHndl = XPS_NH_DB_HNDL;
    if ((status = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "xps_nh_db", XPS_GLOBAL,
                                     &xpsNhKeyComp,
                                     nhSwDbHndl)) != XP_NO_ERR)
    {
        nhSwDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Mac2me Db creation failed");
    }
    return status;

}

/* Allocate a MAC2ME table index */
static XP_STATUS xpsL3AllocateMacToMeIndex(xpsDevice_t devId,
                                           xpsInterfaceId_t l3IntfId,
                                           macAddr_t mac,
                                           uint32_t *index)
{
    XP_STATUS status = XP_NO_ERR;
    xpsMac2MeInfo_t *info = NULL;
    xpsMac2MeInfo_t mac2meInfoKey;
    memset(&mac2meInfoKey, 0, sizeof(xpsMac2MeInfo_t));

    memcpy(mac2meInfoKey.mac, mac, sizeof(macAddr_t));
    mac2meInfoKey.l3IntfId = l3IntfId;

    if ((status = xpsStateSearchData(XP_SCOPE_DEFAULT, mac2meDbHndl,
                                     (xpsDbKey_t)&mac2meInfoKey,
                                     (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search Mac2Me db fail mac " FMT_MAC "\n", PRI_MAC(mac));
        return status;
    }
    if (info)
    {
        /* Entry found */
        info->refCount++;
        *index = info->mac2meIndex;
    }
    else
    {
        /* Create new Entry */

        status = xpsAllocatorAllocateId(devId, XPS_ALLOCATOR_MAC2ME_TBL, index);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to allocate MAC2ME table index\n");
            return status;
        }

        if ((status = xpsStateHeapMalloc(sizeof(xpsMac2MeInfo_t),
                                         (void**)&info)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Space allocation for mac2me info failed");
            return status;
        }
        if (!info)
        {
            return XP_ERR_NULL_POINTER;
        }
        memset(info, 0, sizeof(xpsMac2MeInfo_t));

        info->mac2meIndex = *index;
        info->refCount = 1;
        memcpy(info->mac, mac, sizeof(macAddr_t));
        info->l3IntfId = l3IntfId;

        if ((status = xpsStateInsertData(XP_SCOPE_DEFAULT, mac2meDbHndl,
                                         (void*)info)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Insertion of mac2me pointer info failed");
            // Free Allocated memory
            xpsStateHeapFree((void*)info);
        }
    }

    return status;
}

/* Release a MAC2ME table index */
static XP_STATUS xpsL3ReleaseMacToMeIndex(xpsDevice_t devId,
                                          xpsInterfaceId_t l3IntfId,
                                          macAddr_t mac,
                                          int32_t index)
{
    XP_STATUS status = XP_NO_ERR;
    xpsMac2MeInfo_t *info = NULL;
    xpsMac2MeInfo_t mac2meInfoKey;
    memset(&mac2meInfoKey, 0, sizeof(xpsMac2MeInfo_t));

    memcpy(mac2meInfoKey.mac, mac, sizeof(macAddr_t));
    mac2meInfoKey.l3IntfId = l3IntfId;

    if ((status = xpsStateSearchData(XP_SCOPE_DEFAULT, mac2meDbHndl,
                                     (xpsDbKey_t)&mac2meInfoKey,
                                     (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search Mac2Me db fail mac " FMT_MAC "\n",
              PRI_MAC(mac));
        return status;
    }

    if (info)
    {
        info->refCount--;
        if (info->refCount == 0)
        {
            if ((status = xpsStateDeleteData(XP_SCOPE_DEFAULT, mac2meDbHndl,
                                             (xpsDbKey_t)&mac2meInfoKey,
                                             (void**)&info)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Removal of mac2me info failed \n");
                return status;
            }

            if (info == NULL)
            {
                return XP_ERR_NOT_FOUND;
            }

            // Free memory
            if ((status = xpsStateHeapFree((void*)info)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Freeing allocated memory failed");
            }

            return xpsAllocatorReleaseId(devId, XPS_ALLOCATOR_MAC2ME_TBL, index);
        }
    }
    else
    {
        status = XP_ERR_NOT_FOUND;
    }
    return status;
}

XP_STATUS xpsL3AddIngressRouterMac(xpsDevice_t devId, macAddr_t mac)
{
    GT_STATUS cpssStatus = GT_OK;
    XP_STATUS status = XP_NO_ERR;
    uint32_t mac2MeIndex = 0;
    CPSS_DXCH_TTI_MAC_VLAN_STC value, mask;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC intfInfo;
    GT_PORT_GROUPS_BMP pgBmp;
    xpsScope_t scopeId;

    XPS_LOCK(xpsL3AddIngressRouterMac);

    memset(&intfInfo, 0, sizeof(CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC));
    memset(&value, 0, sizeof(CPSS_DXCH_TTI_MAC_VLAN_STC));
    memset(&mask, 0xff, sizeof(CPSS_DXCH_TTI_MAC_VLAN_STC));
    memset(&pgBmp, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
           sizeof(GT_PORT_GROUPS_BMP));   // All port groups

    // Get scope id from device id
    status = xpsScopeGetScopeId(devId, &scopeId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get scope id for dev id: %d\n", devId);
        return status;
    }

    //Allocate an index in MAC2ME table
    status = xpsL3AllocateMacToMeIndex(devId, 0, mac, &mac2MeIndex);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to allocate MAC2ME table index\n");
        return status;
    }

    //set the values
    value.vlanId = 0;
    mask.vlanId = 0;
    memcpy(value.mac.arEther, mac, sizeof(value.mac.arEther));
    memset(mask.mac.arEther, 0xFF, sizeof(mask.mac.arEther));
    intfInfo.includeSrcInterface = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;

    // Write the entry to the table
    cpssStatus = cpssHalSetTtiPortGroupMacToMe(devId, pgBmp, mac2MeIndex, &value,
                                               &mask, &intfInfo);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to add MAC2ME table entry at index %d\n", mac2MeIndex);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    return XP_NO_ERR;
}


XP_STATUS xpsL3RemoveIngressRouterMac(xpsDevice_t devId, macAddr_t mac)
{
    GT_STATUS cpssStatus = GT_OK;
    GT_PORT_GROUPS_BMP pgBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    CPSS_DXCH_TTI_MAC_VLAN_STC value, mask, cmpMask, cmpValue;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC intfInfo, cmpIntfInfo;
    XP_STATUS status = XP_NO_ERR;
    uint32_t mac2MeDepth = 0;

    memset(&cmpIntfInfo, 0, sizeof(CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC));
    memset(&cmpValue, 0, sizeof(CPSS_DXCH_TTI_MAC_VLAN_STC));
    memset(&cmpMask, 0xff, sizeof(CPSS_DXCH_TTI_MAC_VLAN_STC));

    // Form the entry to be matched against with
    cmpValue.vlanId = 0;
    cmpMask.vlanId = 0;
    memcpy(cmpValue.mac.arEther, mac, sizeof(cmpValue.mac.arEther));
    memset(cmpMask.mac.arEther, 0xFF, sizeof(cmpMask.mac.arEther));
    cmpIntfInfo.includeSrcInterface = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;

    // TODO: Optimize this function

    // Get MAC2ME table depth
    cpssStatus = cpssDxChCfgTableNumEntriesGet(devId,
                                               CPSS_DXCH_CFG_TABLE_MAC_TO_ME_E, &mac2MeDepth);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get MAC2ME table depth. Device : %d Status : %d\n", devId,
              cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    // Loop find the matching index
    for (uint32_t index = 0; index < mac2MeDepth; index++)
    {
        // Get the MAC2ME entry at the index
        cpssStatus = cpssDxChTtiPortGroupMacToMeGet(devId, pgBmp, index, &value, &mask,
                                                    &intfInfo);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to read MAC2ME at index %d\n", index);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        //Update cmpIntfInfo if it is to be excluded so that the matched entry is hit.
        if (cmpIntfInfo.includeSrcInterface ==
            CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E)
        {
            cmpIntfInfo.srcIsTrunk  = intfInfo.srcIsTrunk ;
            cmpIntfInfo.srcHwDevice = intfInfo.srcHwDevice;
            cmpIntfInfo.srcPortTrunk  = intfInfo.srcPortTrunk ;
        }

        // Check if matching
        if (!memcmp(&cmpValue, &value, sizeof(value)) &&
            !memcmp(&cmpMask, &mask, sizeof(mask)) &&
            !memcmp(&cmpIntfInfo, &intfInfo, sizeof(intfInfo)))
        {
            // Found the entry
            // Invalidate the entry (as per CPSS user guide)
            memset(cmpValue.mac.arEther, 0, sizeof(cmpValue.mac.arEther));
            memset(cmpMask.mac.arEther, 0xFF, sizeof(cmpMask.mac.arEther));
            cmpValue.vlanId = 0xFFF;
            cmpMask.vlanId = 0x1FFF;

            // Release the id
            status = xpsL3ReleaseMacToMeIndex(devId, 0, value.mac.arEther, index);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to release MAC2ME index %d\n", index);
                return status;
            }

            return XP_NO_ERR;
        }
    }

    return XP_ERR_NOT_FOUND;
}

XP_STATUS xpsL3AddIngressRouterVlanMac(xpsDevice_t devId, xpsVlan_t vlan,
                                       macAddr_t mac)
{
    XPS_FUNC_ENTRY_LOG();
    // TODO
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsL3RemoveIngressRouterVlanMac(xpsDevice_t devId, xpsVlan_t vlan,
                                          macAddr_t mac)
{
    XPS_FUNC_ENTRY_LOG();
    //TODO
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3AddIntfIngressRouterMac(xpsDevice_t devId,
                                       xpsInterfaceId_t l3IntfId, macAddr_t mac)
{
    XPS_FUNC_ENTRY_LOG();
    GT_STATUS cpssStatus = GT_OK;
    XP_STATUS status = XP_NO_ERR;
    uint32_t mac2MeIndex = 0;
    CPSS_DXCH_TTI_MAC_VLAN_STC value, mask;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC intfInfo;
    GT_PORT_GROUPS_BMP pgBmp;
    xpsScope_t scopeId;
    xpsInterfaceType_e l3IntfType;
    xpsInterfaceInfo_t *l3Info = NULL;
    XPS_LOCK(xpsL3AddIntfIngressRouterMac);

    memset(&intfInfo, 0, sizeof(CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC));
    memset(&value, 0, sizeof(CPSS_DXCH_TTI_MAC_VLAN_STC));
    memset(&mask, 0xff, sizeof(CPSS_DXCH_TTI_MAC_VLAN_STC));
    memset(&pgBmp, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
           sizeof(GT_PORT_GROUPS_BMP));   // All port groups

    // Get scope id from device id
    status = xpsScopeGetScopeId(devId, &scopeId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get scope id for dev id: %d\n", devId);
        return status;
    }

    status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &l3Info);

    if ((status != XP_NO_ERR) || (l3Info == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface info for Interface id : %d\n", l3IntfId);
        return status;
    }
    l3IntfType = l3Info->type;

    if (l3IntfType != XPS_PORT_ROUTER && (l3IntfType != XPS_VLAN_ROUTER))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "L3 interface %d is not a port router interface\n", l3IntfId);
        return XP_ERR_INVALID_ARG;
    }

    if (l3IntfType == XPS_VLAN_ROUTER)
    {
        value.vlanId = l3Info->keyIntfId;
    }
    else
    {
        //set the values
        value.vlanId = l3Info->bdHWId;
    }
    mask.vlanId = 0x1FFF;
    memcpy(value.mac.arEther, mac, sizeof(value.mac.arEther));
    memset(mask.mac.arEther, 0xFF, sizeof(mask.mac.arEther));

    intfInfo.includeSrcInterface = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;

    //Allocate an index in MAC2ME table
    status = xpsL3AllocateMacToMeIndex(devId, l3IntfId, mac, &mac2MeIndex);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to allocate MAC2ME table index\n");
        return status;
    }

    // Write the entry to the table
    cpssStatus = cpssHalSetTtiPortGroupMacToMe(devId, pgBmp, mac2MeIndex, &value,
                                               &mask, &intfInfo);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to add MAC2ME table entry at index %d\n", mac2MeIndex);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
    l3Info->mac2meHwIdx = mac2MeIndex;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3RemoveIntfIngressRouterMac(xpsDevice_t devId,
                                          xpsInterfaceId_t l3IntfId, macAddr_t mac)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsL3RemoveIntfIngressRouterMac);
    GT_STATUS cpssStatus = GT_OK;
    GT_PORT_GROUPS_BMP pgBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    CPSS_DXCH_TTI_MAC_VLAN_STC value, mask, cmpMask, cmpValue;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC intfInfo, cmpIntfInfo;
    XP_STATUS status = XP_NO_ERR;
    xpsScope_t scopeId;
    xpsInterfaceType_e l3IntfType;
    xpsInterfaceInfo_t *l3Info = NULL;

    memset(&cmpIntfInfo, 0, sizeof(CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC));
    memset(&cmpValue, 0, sizeof(CPSS_DXCH_TTI_MAC_VLAN_STC));
    memset(&cmpMask, 0xff, sizeof(CPSS_DXCH_TTI_MAC_VLAN_STC));

    // Get scope id from device id
    status = xpsScopeGetScopeId(devId, &scopeId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get scope id for dev id: %d\n", devId);
        return status;
    }

    status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &l3Info);

    if ((status != XP_NO_ERR) || (l3Info == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface info for Interface id : %d\n", l3IntfId);
        return status;
    }
    l3IntfType = l3Info->type;

    if (l3IntfType != XPS_PORT_ROUTER && (l3IntfType != XPS_VLAN_ROUTER))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "L3 interface %d is not a port router interface\n", l3IntfId);
        return XP_ERR_INVALID_ARG;
    }

    if (l3IntfType != XPS_PORT_ROUTER  && (l3IntfType != XPS_VLAN_ROUTER))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "L3 interface %d is not a port router interface\n", l3IntfId);
        return XP_ERR_INVALID_ARG;
    }

    if (l3IntfType == XPS_VLAN_ROUTER)
    {
        cmpValue.vlanId = l3Info->keyIntfId;
    }
    else
    {
        cmpValue.vlanId = l3Info->bdHWId;
    }
    cmpMask.vlanId = XPS_VLAN_MASK;
    memcpy(cmpValue.mac.arEther, mac, sizeof(cmpValue.mac.arEther));
    memset(cmpMask.mac.arEther, 0xFF, sizeof(cmpMask.mac.arEther));
    cmpIntfInfo.includeSrcInterface = CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E;

    // Get the MAC2ME entry at the index
    cpssStatus = cpssDxChTtiPortGroupMacToMeGet(devId, pgBmp, l3Info->mac2meHwIdx,
                                                &value, &mask,
                                                &intfInfo);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to read MAC2ME at index %d\n", l3Info->mac2meHwIdx);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    //Update the srcHwDevice if it is to be excluded so that the matched entry is hit.
    if (cmpIntfInfo.includeSrcInterface ==
        CPSS_DXCH_TTI_USE_SRC_INTERFACE_FIELDS_EXCLUDE_SRC_DEVICE_E)
    {
        cmpIntfInfo.srcHwDevice = intfInfo.srcHwDevice;
    }

    if (cmpIntfInfo.includeSrcInterface ==
        CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E)
    {
        cmpIntfInfo.srcIsTrunk  = intfInfo.srcIsTrunk ;
        cmpIntfInfo.srcHwDevice = intfInfo.srcHwDevice;
        cmpIntfInfo.srcPortTrunk  = intfInfo.srcPortTrunk ;
    }

    // Check if matching
    if (!memcmp(&cmpValue, &value, sizeof(value)) &&
        !memcmp(&cmpMask, &mask, sizeof(mask)) &&
        !memcmp(&cmpIntfInfo, &intfInfo, sizeof(intfInfo)))
    {
        // Found the entry
        // Invalidate the entry (as per CPSS user guide)
        memset(cmpValue.mac.arEther, 0, sizeof(cmpValue.mac.arEther));
        memset(cmpMask.mac.arEther, 0xFF, sizeof(cmpMask.mac.arEther));
        cmpValue.vlanId = 0xFFF;
        cmpMask.vlanId = 0x1FFF;

        // Release the id
        status = xpsL3ReleaseMacToMeIndex(devId, l3IntfId, value.mac.arEther,
                                          l3Info->mac2meHwIdx);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to release MAC2ME index %d\n", l3Info->mac2meHwIdx);
            return status;
        }

        return XP_NO_ERR;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_ERR_NOT_FOUND;
}

XP_STATUS xpsL3SetEgressRouterMac(xpsDevice_t devID, xpsInterfaceId_t l3IntfId,
                                  macAddr_t egressMac)
{
    xpsInterfaceId_t portInterfaceId;
    xpPort_t portId;
    xpsInterfaceType_e l3IntfType, intfType;
    XP_STATUS status = XP_NO_ERR;
    xpsScope_t scopeId;
    xpsDevice_t portDevId;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum, i;
    xpsLagPortIntfList_t lagPortList;
    xpsInterfaceInfo_t *l3IntfInfo = NULL;
    GT_STATUS cpssStatus = GT_OK;
    xpsVlan_t vlanId = 0;
    GT_BOOL isVlan = GT_FALSE;
    GT_U8 cpssDevNum = 0;
    memset(&lagPortList, 0, sizeof(lagPortList));


    // Get scope id from device id
    status = xpsScopeGetScopeId(devID, &scopeId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get scope id for dev id: %d\n", devID);
        return status;
    }
    status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &l3IntfInfo);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to fetch interface type from l3 interface id %d\n", l3IntfId);
        return status;
    }
    l3IntfType = l3IntfInfo->type;
    // Check if the L3 inteface is of right type
    if (l3IntfType != XPS_PORT_ROUTER && (l3IntfType != XPS_VLAN_ROUTER))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid L3 interface type for L3 interface ; %d\n", l3IntfType);
        return XP_ERR_INVALID_ARG;
    }

    if (l3IntfType == XPS_VLAN_ROUTER)
    {
        isVlan = GT_TRUE;
        vlanId = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Vlan Id : %d\n", vlanId);

        if (vlanId > XPS_VLANID_MAX)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlan id %d  .",
                  vlanId);
            return XP_ERR_INVALID_VLAN_ID;
        }

        XPS_DEVICES_PER_SWITCH_ITER(devID, cpssDevNum)
        {
            cpssStatus = cpssHalL3SetEgressRouterMac(cpssDevNum, isVlan, vlanId, egressMac);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set egress mac to vlanId : %d \n", vlanId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }

    }

    else
    {
        // Fetch the port interface from the L3 interface
        status = xpsL3RetrieveInterfaceMapping(scopeId, l3IntfId, &portInterfaceId);

        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port interface id from L3 interface id %d\n", l3IntfId);
            return status;
        }
        status = xpsInterfaceGetTypeScope(scopeId, portInterfaceId, &intfType);

        if (status != XP_NO_ERR)
        {
            return status;
        }
        if (intfType == XPS_PORT)
        {
            // Get the port interface id from L3 interface id
            lagPortList.portIntf[0] = portInterfaceId;
            lagPortList.size = 1;
        }
        else if (intfType == XPS_LAG)
        {
            if ((status = xpsLagGetPortIntfListScope(scopeId, portInterfaceId,
                                                     &lagPortList)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ports failed, lag interface(%d)", portInterfaceId);
                return status;
            }
        }
        for (i = 0; i < lagPortList.size; i++)
        {
            portInterfaceId = lagPortList.portIntf[i];

            // Get the port and dev id corresponding the port interface id
            status = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portInterfaceId,
                                                          &portDevId, &portId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get port id and dev id for port interface id %d\n", portInterfaceId);
                return status;
            }

            cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
            cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

            cpssStatus = cpssHalL3SetEgressRouterMac(cpssDevId, isVlan, cpssPortNum,
                                                     egressMac);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set egress mac to port intf : %d \n", portInterfaceId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }
    /*save the mac in l3 interface info*/
    memcpy(l3IntfInfo->egressSAMac, egressMac, sizeof(macAddr_t));
    return XP_NO_ERR;
}

XP_STATUS xpsL3RemoveEgressRouterMac(xpsDevice_t devID,
                                     xpsInterfaceId_t l3IntfId)
{
    xpsInterfaceId_t portInterfaceId;
    xpPort_t portId;
    xpsInterfaceType_e l3IntfType, portIntfType;
    XP_STATUS status = XP_NO_ERR;
    xpsScope_t scopeId;
    xpsDevice_t portDevId;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum, i;
    xpsLagPortIntfList_t lagPortList;
    xpsInterfaceInfo_t *l3IntfInfo = NULL;
    macAddr_t egressMac;
    memset(&egressMac, 0, sizeof(macAddr_t));
    GT_STATUS cpssStatus = GT_OK;
    GT_BOOL isVlan = GT_FALSE;
    xpsVlan_t vlanId;
    GT_U8 cpssDevNum;

    memset(&egressMac, 0, sizeof(macAddr_t));
    memset(&lagPortList, 0, sizeof(lagPortList));

    // Get scope id from device id
    status = xpsScopeGetScopeId(devID, &scopeId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get scope id for dev id: %d\n", devID);
        return status;
    }

    status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &l3IntfInfo);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to fetch interface type from l3 interface id %d\n", l3IntfId);
        return status;
    }
    l3IntfType = l3IntfInfo->type;
    // Check if the L3 inteface is of right type

    if (l3IntfType != XPS_PORT_ROUTER || (l3IntfType != XPS_VLAN_ROUTER))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid L3 interface type for L3 interface ; %d\n", l3IntfType);
        return XP_ERR_INVALID_ARG;
    }

    if (l3IntfType == XPS_VLAN_ROUTER)
    {
        isVlan = GT_TRUE;
        // Retrieve the corresponding vlan id
        vlanId = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Vlan Id : %d\n", vlanId);

        if (vlanId > XPS_VLANID_MAX)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlan id %d  .",
                  vlanId);
            return XP_ERR_INVALID_VLAN_ID;
        }

        XPS_DEVICES_PER_SWITCH_ITER(devID, cpssDevNum)
        {
            cpssStatus = cpssHalL3RemoveEgressRouterMac(cpssDevNum, isVlan, vlanId);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to remove egress mac for vlan : %d \n", vlanId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }
    else
    {
        // Fetch the port interface from the L3 interface
        status = xpsL3RetrieveInterfaceMapping(scopeId, l3IntfId, &portInterfaceId);

        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port interface id from L3 interface id %d\n", l3IntfId);
            return status;
        }

        status = xpsInterfaceGetType(portInterfaceId, &portIntfType);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to fetch interface type from interface id %d\n", portInterfaceId);
            return status;
        }
        if (portIntfType == XPS_PORT)
        {
            lagPortList.portIntf[0] = portInterfaceId;
            lagPortList.size = 1;
        }
        else if (portIntfType == XPS_LAG)
        {
            if ((status = xpsLagGetPortIntfListScope(scopeId, portInterfaceId,
                                                     &lagPortList)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ports failed, lag interface(%d)", portInterfaceId);
                return status;
            }

            /* Set Invalid  L3 interface value to LAG DB */
            if ((status = xpsLagSetL3IntfToDbScope(scopeId, portInterfaceId,
                                                   XPS_INTF_INVALID_ID)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " Setting Invalid L3 interface to lag interface(%d) DB failed",
                      portInterfaceId);
                return status;
            }
        }

        for (i = 0; i < lagPortList.size; i++)
        {
            portInterfaceId = lagPortList.portIntf[i];
            // Get the port and dev id corresponding the port interface id
            status = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portInterfaceId,
                                                          &portDevId, &portId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get port id and dev id for port interface id %d\n", portInterfaceId);
                return status;
            }

            cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
            cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

            cpssStatus = cpssHalL3RemoveEgressRouterMac(cpssDevId, isVlan, cpssPortNum);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to remove egress mac for port intf : %d \n", portInterfaceId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }
    memcpy(l3IntfInfo->egressSAMac, egressMac, sizeof(macAddr_t));
    return XP_NO_ERR;
}

XP_STATUS xpsL3SetEgressRouterMacMSbs(xpsDevice_t devId, macAddrHigh_t macHi)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetEgressRouterMacMSbs(xpsDevice_t devId, macAddrHigh_t *macHi)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIntfEgressRouterMacLSB(xpsDevice_t devId,
                                         xpsInterfaceId_t l3IntfId, macAddrLow_t macSa)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIntfEgressRouterMacLSB(xpsDevice_t devId,
                                         xpsInterfaceId_t l3IntfId, macAddrLow_t *macSa)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIntfVrf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                          uint32_t vrfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsL3SetIntfVrf);

    XP_STATUS status = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;
    xpsInterfaceType_e intfType;
    xpsInterfaceInfo_t *l3Info = NULL;
    xpsScope_t scopeId;
    xpsVlan_t vlanId = 0;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &l3Info);

    if ((status != XP_NO_ERR) || (l3Info == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface info for Interface id : %d\n", l3IntfId);
        return status;
    }

    intfType = l3Info->type;
    if ((intfType != XPS_PORT_ROUTER) && (intfType != XPS_SUBINTERFACE_ROUTER) &&
        (intfType != XPS_VLAN_ROUTER) && (intfType != XPS_TUNNEL_ROUTER))
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if (l3Info->type == XPS_PORT_ROUTER)
    {
        vlanId = l3Info->bdHWId;
    }
    else
    {
        vlanId = l3Info->keyIntfId;
    }

    cpssStatus = cpssHalBrgVlanVrfIdSet(devId, vlanId, vrfId);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalBrgVlanVrfIdSet Failed VlanId %d Vrf %d\n", vlanId, vrfId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    l3Info->vrfId = vrfId;

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsL3GetIntfVrf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                          uint32_t *vrfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsL3GetIntfVrf);

    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceType_e intfType;
    xpsInterfaceInfo_t *l3Info = NULL;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &l3Info);

    if ((status != XP_NO_ERR) || (l3Info == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface info for Interface id : %d\n", l3IntfId);
        return status;
    }

    intfType = l3Info->type;
    if ((intfType != XPS_PORT_ROUTER) && (intfType != XPS_SUBINTERFACE_ROUTER) &&
        (intfType != XPS_VLAN_ROUTER) && (intfType != XPS_TUNNEL_ROUTER))
    {
        return XP_ERR_INVALID_PARAMS;
    }

    *vrfId = l3Info->vrfId;

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsL3SetIntfIpv4UcRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsL3SetIntfIpv4UcRoutingEn);

    XP_STATUS status     = XP_NO_ERR;
    xpsInterfaceType_e intfType;
    xpsScope_t scopeId;
    xpsInterfaceId_t portInterfaceId = XPS_INTF_INVALID_ID;
    xpsDevice_t portDevId;
    xpPort_t portId;
    GT_BOOL cpssEn = GT_FALSE;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum, i;
    xpsLagPortIntfList_t lagPortList;
    GT_STATUS cpssStatus = GT_OK;
    CPSS_PACKET_CMD_ENT pktCmd;

    cpssEn = (enable == 0) ? GT_FALSE : GT_TRUE;
    pktCmd = (enable == 0) ? CPSS_PACKET_CMD_FORWARD_E:CPSS_PACKET_CMD_DROP_HARD_E;

    xpsVlan_t vlanId = 0;
    int port = 0;
    int maxTotalPorts = 0;
    CPSS_PORTS_BMP_STC                   globalPortMembers;
    CPSS_PORTS_BMP_STC                   portsMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_BOOL                              isValid = GT_FALSE;

    memset(&portsMembers, 0, sizeof(portsMembers));
    memset(&portsTagging, 0, sizeof(portsTagging));
    memset(&vlanInfo, 0, sizeof(vlanInfo));
    memset(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));
    memset(&lagPortList, 0, sizeof(lagPortList));

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }


    status = xpsInterfaceGetTypeScope(scopeId, l3IntfId, &intfType);

    if (status != XP_NO_ERR)
    {
        return status;
    }

    if ((intfType != XPS_PORT_ROUTER) && (intfType != XPS_SUBINTERFACE_ROUTER) &&
        (intfType != XPS_VLAN_ROUTER) && (intfType != XPS_TUNNEL_ROUTER))
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if (intfType == XPS_VLAN_ROUTER)
    {
        vlanId = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Vlan Id : %d\n", vlanId);

        if ((vlanId <= XPS_VLANID_MIN) && (vlanId > XPS_VLANID_MAX))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Invalid vlan id %d maxVlan %d .", vlanId, XPS_VLANID_MAX);
            return XP_ERR_INVALID_VLAN_ID;
        }
        status = xpsVlanSetRoutingEnable(devId, vlanId, XPS_PROTOCOL_IPV4_E, false,
                                         enable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Updating Vlan Context Db failed, vlanId(%d)", vlanId);
            return status;
        }
        cpssHalGetMaxGlobalPorts(&maxTotalPorts);

        memset(&globalPortMembers, 0, sizeof(globalPortMembers));
        cpssStatus = cpssHalReadBrgVlanEntry(devId, (GT_U16)vlanId, &portsMembers,
                                             &portsTagging, &vlanInfo, &isValid,
                                             &portsTaggingCmd, &globalPortMembers);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Read vlan %d failed .",
                  vlanId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        /* Enabling UC routing on all member ports of the vlan*/
        XPS_GLOBAL_PORT_ITER(port, maxTotalPorts)
        {
            cpssDevId = xpsGlobalIdToDevId(devId, port);
            cpssPortNum = xpsGlobalPortToPortnum(devId, port);
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&globalPortMembers, cpssPortNum))
            {
                cpssStatus = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                            CPSS_IP_PROTOCOL_IPV4_E, cpssEn, GT_TRUE);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to update ipv4 UC routing for port intf : %d \n", cpssPortNum);
                    return xpsConvertCpssStatusToXPStatus(cpssStatus);
                }
            }
        }
#if 0
        /*TODO : Revisit this on supporting IPMC */
        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_BCAST_E,
                                                  pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_MCAST_E,
                                                  pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV6_MCAST_E,
                                                  pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IPV4_BCAST_E,
                                                  (pktCmd == CPSS_PACKET_CMD_DROP_HARD_E) ? CPSS_PACKET_CMD_DROP_SOFT_E : pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
#endif
        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E,
                                                  pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    else
    {
        status = xpsL3RetrieveInterfaceMapping(scopeId, l3IntfId, &portInterfaceId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port interface id for L3 interface: %d\n", l3IntfId);
            return status;
        }
        status = xpsInterfaceGetTypeScope(scopeId, portInterfaceId, &intfType);

        if (status != XP_NO_ERR)
        {
            return status;
        }
        if (intfType == XPS_PORT)
        {

            // Get the port interface id from L3 interface id
            lagPortList.portIntf[0] = portInterfaceId;
            lagPortList.size = 1;
        }
        else if (intfType == XPS_LAG)
        {
            if ((status = xpsLagGetPortIntfListScope(scopeId, portInterfaceId,
                                                     &lagPortList)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ports failed, lag interface(%d)", portInterfaceId);
                return status;
            }
            if ((status = xpsLagIpV4RouteEnable(scopeId, portInterfaceId,
                                                false, enable)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Set ipv4 Route Failed, lag interface(%d)", portInterfaceId);
                return status;
            }
        }
        for (i = 0; i < lagPortList.size; i++)
        {
            portInterfaceId = lagPortList.portIntf[i];

            // Get the port id and dev id from the port interface id
            // TODO: is this call required? Or XPS port db to be obsoleted?
            status = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portInterfaceId,
                                                          &portDevId, &portId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get physical port id and device from the port interface %d \n",
                      portInterfaceId);
                return status;
            }

            cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
            cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);
            cpssStatus = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                        CPSS_IP_PROTOCOL_IPV4_E, cpssEn, GT_FALSE);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to update ipv4 UC routing for port intf : %d \n", portInterfaceId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
            // Disable  FDB NA AU msg
            cpssStatus = cpssDxChBrgFdbNaToCpuPerPortSet(cpssDevId, cpssPortNum,
                                                         cpssEn? GT_FALSE:GT_TRUE);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set the FDB NA to CPU disable for port %d\n", cpssPortNum);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
#if 0
            cpssStatus = cpssDxChBrgFdbPortLearnStatusSet(cpssDevId, cpssPortNum, GT_FALSE,
                                                          cpssEn? CPSS_LOCK_DROP_E:CPSS_LOCK_FRWRD_E);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "failed to disable learning of new SMAC on  port %d\n", cpssPortNum);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
#endif
        }
    }
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIntfIpv4UcRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsL3GetIntfIpv4UcRoutingEn);

    XP_STATUS status = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;
    xpsInterfaceType_e intfType;
    xpsScope_t scopeId;
    xpsInterfaceId_t portInterfaceId;
    xpsDevice_t portDevId;
    xpPort_t portId;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    xpsVlan_t vlanId;
    bool ipEnable = false;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }
    status = xpsInterfaceGetTypeScope(scopeId, l3IntfId, &intfType);

    if (status)
    {
        return status;
    }

    if ((intfType != XPS_PORT_ROUTER) && (intfType != XPS_SUBINTERFACE_ROUTER) &&
        (intfType != XPS_VLAN_ROUTER) && (intfType != XPS_TUNNEL_ROUTER))
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if ((enable == NULL))
    {
        return XP_ERR_NULL_POINTER;
    }

    if (intfType == XPS_VLAN_ROUTER)
    {
        /* Retrieving vlan-id from input L3 interface*/
        vlanId = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Vlan Id : %d\n", vlanId);

        if (vlanId > XPS_VLANID_MAX)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlan id %d  .",
                  vlanId);
            return XP_ERR_INVALID_VLAN_ID;
        }

        /* Check if IPv4 Routing Enabled on Vlan Router Interface. */
        status = xpsVlanGetRoutingEnable(devId, vlanId, XPS_PROTOCOL_IPV4_E, false,
                                         &ipEnable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get Vlan IPv4 Routing Failed, vlanId(%d)", vlanId);
            return status;
        }

        *enable = (uint32_t)ipEnable;

    }
    else
    {
        // Get the port interface id from L3 interface id
        status = xpsL3RetrieveInterfaceMapping(scopeId, l3IntfId, &portInterfaceId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port interface id for L3 interface: %d\n", l3IntfId);
            return status;
        }
        status = xpsInterfaceGetTypeScope(scopeId, portInterfaceId, &intfType);

        if (status != XP_NO_ERR)
        {
            return status;
        }
        if (intfType == XPS_LAG)
        {
            // If LAG get from the DB as we cannot get from the member port
            if ((status = xpsLagIpV4RouteEnableGet(scopeId, portInterfaceId,
                                                   false, enable)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ipv4 Route Failed, lag interface(%d)", portInterfaceId);
            }
            return status;
        }

        // Get the port id and dev id from the port interface id
        // TODO: is this call required?
        status = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portInterfaceId,
                                                      &portDevId, &portId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get physical port id and device from the port interface %d \n",
                  portInterfaceId);
            return status;
        }

        cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
        cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

        cpssStatus = cpssDxChIpPortRoutingEnableGet(cpssDevId, cpssPortNum,
                                                    CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, (GT_BOOL *)enable);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get V4 UC enable on port %d\n", cpssPortNum);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIntfIpv6UcRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsL3SetIntfIpv6UcRoutingEn);

    XP_STATUS status     = XP_NO_ERR;
    xpsInterfaceType_e intfType;
    xpsScope_t scopeId;
    xpsInterfaceId_t portInterfaceId = XPS_INTF_INVALID_ID;
    xpsDevice_t portDevId;
    xpPort_t portId;
    GT_BOOL cpssEn = GT_FALSE;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum, i;
    xpsLagPortIntfList_t lagPortList;
    GT_STATUS cpssStatus = GT_OK;
    CPSS_PACKET_CMD_ENT pktCmd;

    cpssEn = (enable == 0) ? GT_FALSE : GT_TRUE;
    pktCmd = (enable == 0) ? CPSS_PACKET_CMD_FORWARD_E:CPSS_PACKET_CMD_DROP_HARD_E;

    xpsVlan_t vlanId = 0;
    int port = 0;
    int maxTotalPorts = 0;
    CPSS_PORTS_BMP_STC                   globalPortMembers;
    CPSS_PORTS_BMP_STC                   portsMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_BOOL                              isValid = GT_FALSE;
    memset(&lagPortList, 0, sizeof(lagPortList));


    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    status = xpsInterfaceGetTypeScope(scopeId, l3IntfId, &intfType);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    if ((intfType != XPS_PORT_ROUTER) && (intfType != XPS_SUBINTERFACE_ROUTER) &&
        (intfType != XPS_VLAN_ROUTER) && (intfType != XPS_TUNNEL_ROUTER))
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if (intfType == XPS_VLAN_ROUTER)
    {
        vlanId = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Vlan Id : %d\n", vlanId);

        if ((vlanId <= XPS_VLANID_MIN) && (vlanId > XPS_VLANID_MAX))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Invalid vlan id %d maxVlan %d .", vlanId, XPS_VLANID_MAX);
            return XP_ERR_INVALID_VLAN_ID;
        }
        status = xpsVlanSetRoutingEnable(devId, vlanId, XPS_PROTOCOL_IPV6_E, false,
                                         enable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Updating Vlan Context Db failed, vlanId(%d)", vlanId);
            return status;
        }

        cpssHalGetMaxGlobalPorts(&maxTotalPorts);

        memset(&globalPortMembers, 0, sizeof(globalPortMembers));
        cpssStatus = cpssHalReadBrgVlanEntry(devId, (GT_U16)vlanId, &portsMembers,
                                             &portsTagging, &vlanInfo, &isValid,
                                             &portsTaggingCmd, &globalPortMembers);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Read vlan %d failed .",
                  vlanId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        /* Enabling UC routing on all member ports of the vlan*/
        XPS_GLOBAL_PORT_ITER(port, maxTotalPorts)
        {
            cpssDevId = xpsGlobalIdToDevId(devId, port);
            cpssPortNum = xpsGlobalPortToPortnum(devId, port);
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&globalPortMembers, cpssPortNum))
            {
                cpssStatus = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                            CPSS_IP_PROTOCOL_IPV6_E, cpssEn, GT_TRUE);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to update ipv6 UC routing for port intf : %d \n", cpssPortNum);
                    return xpsConvertCpssStatusToXPStatus(cpssStatus);
                }
            }
        }

#if 0
        /*TODO : Revisit this on supporting IPMC */
        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_BCAST_E,
                                                  pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_MCAST_E,
                                                  pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV6_MCAST_E,
                                                  pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IPV4_BCAST_E,
                                                  (pktCmd == CPSS_PACKET_CMD_DROP_HARD_E) ? CPSS_PACKET_CMD_DROP_SOFT_E : pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

#endif
        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E,
                                                  pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    else
    {
        // Get the port interface id from L3 interface id
        status = xpsL3RetrieveInterfaceMapping(scopeId, l3IntfId, &portInterfaceId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port interface id for L3 interface: %d\n", l3IntfId);
            return status;
        }

        status = xpsInterfaceGetTypeScope(scopeId, portInterfaceId, &intfType);

        if (status != XP_NO_ERR)
        {
            return status;
        }
        if (intfType == XPS_PORT)
        {

            // Get the port interface id from L3 interface id
            lagPortList.portIntf[0] = portInterfaceId;
            lagPortList.size = 1;
        }
        else if (intfType == XPS_LAG)
        {
            if ((status = xpsLagGetPortIntfListScope(scopeId, portInterfaceId,
                                                     &lagPortList)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ports failed, lag interface(%d)", portInterfaceId);
                return status;
            }
            if ((status = xpsLagIpV6RouteEnable(scopeId, portInterfaceId,
                                                false, enable)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Set ipv6 Route Failed, lag interface(%d)", portInterfaceId);
                return status;
            }
        }
        for (i = 0; i < lagPortList.size; i++)
        {
            portInterfaceId = lagPortList.portIntf[i];
            // Get the port id and dev id from the port interface id
            // TODO: is this call required
            status = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portInterfaceId,
                                                          &portDevId, &portId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get physical port id and device from the port interface %d \n",
                      portInterfaceId);
                return status;
            }

            cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
            cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

            cpssStatus = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                        CPSS_IP_PROTOCOL_IPV6_E, cpssEn, GT_FALSE);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to update ipv4 UC routing for port intf : %d \n", portInterfaceId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIntfIpv6UcRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsL3GetIntfIpv6UcRoutingEn);

    XP_STATUS status = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;
    xpsInterfaceType_e intfType;
    xpsScope_t scopeId;
    xpsInterfaceId_t portInterfaceId;
    xpsDevice_t portDevId;
    xpPort_t portId;
    GT_U8 cpssDevId;
    GT_U32 cpssPortId;
    xpsVlan_t vlanId;
    bool ipEnable = false;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }
    status = xpsInterfaceGetTypeScope(scopeId, l3IntfId, &intfType);

    if (status)
    {
        return status;
    }

    if ((intfType != XPS_PORT_ROUTER) && (intfType != XPS_SUBINTERFACE_ROUTER) &&
        (intfType != XPS_VLAN_ROUTER) && (intfType != XPS_TUNNEL_ROUTER))
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if ((enable == NULL))
    {
        return XP_ERR_NULL_POINTER;
    }

    if (intfType == XPS_VLAN_ROUTER)
    {
        /* Retrieving vlan-id from input L3 interface*/
        vlanId = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Vlan Id : %d\n", vlanId);

        if (vlanId > XPS_VLANID_MAX)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlan id %d  .",
                  vlanId);
            return XP_ERR_INVALID_VLAN_ID;
        }

        /* Check if IPv6 Routing Enabled on Vlan Router Interface. */
        status = xpsVlanGetRoutingEnable(devId, vlanId, XPS_PROTOCOL_IPV6_E, false,
                                         &ipEnable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get Vlan IPv4 Routing Failed, vlanId(%d)", vlanId);
            return status;
        }

        *enable = (uint32_t)ipEnable;

    }
    else
    {
        // Get the port interface id from L3 interface id
        status = xpsL3RetrieveInterfaceMapping(scopeId, l3IntfId, &portInterfaceId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port interface id for L3 interface: %d\n", l3IntfId);
            return status;
        }

        status = xpsInterfaceGetTypeScope(scopeId, portInterfaceId, &intfType);

        if (status != XP_NO_ERR)
        {
            return status;
        }
        if (intfType == XPS_LAG)
        {
            // If LAG get from the DB as we cannot get from the member port
            if ((status = xpsLagIpV6RouteEnableGet(scopeId, portInterfaceId,
                                                   false, enable)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ipv6 Route Failed, lag interface(%d)", portInterfaceId);
            }
            return status;
        }

        // Get the port id and dev id from the port interface id
        status = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portInterfaceId,
                                                      &portDevId, &portId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get physical port id and device from the port interface %d \n",
                  portInterfaceId);
            return status;
        }

        cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
        cpssPortId = xpsGlobalPortToPortnum(portDevId, portId);

        cpssStatus = cpssDxChIpPortRoutingEnableGet(cpssDevId, cpssPortId,
                                                    CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV6_E, (GT_BOOL *)enable);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get V4 UC enable on port %d\n", cpssPortId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIntfMplsRoutingEn(xpsDevice_t devId,
                                    xpsInterfaceId_t l3IntfId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIntfMplsRoutingEn(xpsDevice_t devId,
                                    xpsInterfaceId_t l3IntfId, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIntfIpv4McRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsL3SetIntfIpv4UcRoutingEn);

    XP_STATUS status     = XP_NO_ERR;
    xpsInterfaceType_e intfType;
    xpsScope_t scopeId;
    xpsInterfaceId_t portInterfaceId = XPS_INTF_INVALID_ID;
    xpsDevice_t portDevId;
    xpPort_t portId;
    GT_BOOL cpssEn = GT_FALSE;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum, i;
    xpsLagPortIntfList_t lagPortList;
    GT_STATUS cpssStatus = GT_OK;
    CPSS_PACKET_CMD_ENT pktCmd;

    cpssEn = (enable == 0) ? GT_FALSE : GT_TRUE;
    pktCmd = (enable == 0) ? CPSS_PACKET_CMD_FORWARD_E:CPSS_PACKET_CMD_DROP_HARD_E;

    xpsVlan_t vlanId = 0;
    int port = 0;
    int maxTotalPorts = 0;
    CPSS_PORTS_BMP_STC                   globalPortMembers;
    CPSS_PORTS_BMP_STC                   portsMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_BOOL                              isValid = GT_FALSE;

    memset(&portsMembers, 0, sizeof(portsMembers));
    memset(&portsTagging, 0, sizeof(portsTagging));
    memset(&vlanInfo, 0, sizeof(vlanInfo));
    memset(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));
    memset(&lagPortList, 0, sizeof(lagPortList));

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }


    status = xpsInterfaceGetTypeScope(scopeId, l3IntfId, &intfType);

    if (status != XP_NO_ERR)
    {
        return status;
    }

    if ((intfType != XPS_PORT_ROUTER) && (intfType != XPS_SUBINTERFACE_ROUTER) &&
        (intfType != XPS_VLAN_ROUTER) && (intfType != XPS_TUNNEL_ROUTER))
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if (intfType == XPS_VLAN_ROUTER)
    {
        vlanId = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Vlan Id : %d\n", vlanId);

        if ((vlanId <= XPS_VLANID_MIN) && (vlanId > XPS_VLANID_MAX))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Invalid vlan id %d maxVlan %d .", vlanId, XPS_VLANID_MAX);
            return XP_ERR_INVALID_VLAN_ID;
        }
        status = xpsVlanSetRoutingEnable(devId, vlanId, XPS_PROTOCOL_IPV4_E, true,
                                         enable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Updating Vlan Context Db failed, vlanId(%d)", vlanId);
            return status;
        }
        cpssHalGetMaxGlobalPorts(&maxTotalPorts);

        memset(&globalPortMembers, 0, sizeof(globalPortMembers));
        cpssStatus = cpssHalReadBrgVlanEntry(devId, (GT_U16)vlanId, &portsMembers,
                                             &portsTagging, &vlanInfo, &isValid,
                                             &portsTaggingCmd, &globalPortMembers);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Read vlan %d failed .",
                  vlanId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        /* Enabling UC routing on all member ports of the vlan*/
        XPS_GLOBAL_PORT_ITER(port, maxTotalPorts)
        {
            cpssDevId = xpsGlobalIdToDevId(devId, port);
            cpssPortNum = xpsGlobalPortToPortnum(devId, port);
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&globalPortMembers, cpssPortNum))
            {
                cpssStatus = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId, cpssPortNum,
                                                          CPSS_IP_PROTOCOL_IPV4_E, cpssEn);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to update ipv4 UC routing for port intf : %d \n", cpssPortNum);
                    return xpsConvertCpssStatusToXPStatus(cpssStatus);
                }
            }
        }

        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E,
                                                  pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    else
    {
        status = xpsL3RetrieveInterfaceMapping(scopeId, l3IntfId, &portInterfaceId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port interface id for L3 interface: %d\n", l3IntfId);
            return status;
        }
        status = xpsInterfaceGetTypeScope(scopeId, portInterfaceId, &intfType);

        if (status != XP_NO_ERR)
        {
            return status;
        }
        if (intfType == XPS_PORT)
        {

            // Get the port interface id from L3 interface id
            lagPortList.portIntf[0] = portInterfaceId;
            lagPortList.size = 1;
        }
        else if (intfType == XPS_LAG)
        {
            if ((status = xpsLagGetPortIntfListScope(scopeId, portInterfaceId,
                                                     &lagPortList)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ports failed, lag interface(%d)", portInterfaceId);
                return status;
            }
            if ((status = xpsLagIpV4RouteEnable(scopeId, portInterfaceId,
                                                true, enable)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Set ipv4 Route Failed, lag interface(%d)", portInterfaceId);
                return status;
            }
        }
        for (i = 0; i < lagPortList.size; i++)
        {
            portInterfaceId = lagPortList.portIntf[i];

            // Get the port id and dev id from the port interface id
            // TODO: is this call required? Or XPS port db to be obsoleted?
            status = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portInterfaceId,
                                                          &portDevId, &portId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get physical port id and device from the port interface %d \n",
                      portInterfaceId);
                return status;
            }

            cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
            cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);
            cpssStatus = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId, cpssPortNum,
                                                      CPSS_IP_PROTOCOL_IPV4_E, cpssEn);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to update ipv4 UC routing for port intf : %d \n", portInterfaceId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIntfIpv4McRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsL3GetIntfIpv4McRoutingEn);

    XP_STATUS status = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;
    xpsInterfaceType_e intfType;
    xpsScope_t scopeId;
    xpsInterfaceId_t portInterfaceId;
    xpsDevice_t portDevId;
    xpPort_t portId;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    xpsVlan_t vlanId;
    bool ipEnable = false;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }
    status = xpsInterfaceGetTypeScope(scopeId, l3IntfId, &intfType);

    if (status)
    {
        return status;
    }

    if ((intfType != XPS_PORT_ROUTER) && (intfType != XPS_SUBINTERFACE_ROUTER) &&
        (intfType != XPS_VLAN_ROUTER) && (intfType != XPS_TUNNEL_ROUTER))
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if ((enable == NULL))
    {
        return XP_ERR_NULL_POINTER;
    }

    if (intfType == XPS_VLAN_ROUTER)
    {
        /* Retrieving vlan-id from input L3 interface*/
        vlanId = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Vlan Id : %d\n", vlanId);

        if (vlanId > XPS_VLANID_MAX)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlan id %d  .",
                  vlanId);
            return XP_ERR_INVALID_VLAN_ID;
        }

        /* Check if IPv4 Routing Enabled on Vlan Router Interface. */
        status = xpsVlanGetRoutingEnable(devId, vlanId, XPS_PROTOCOL_IPV4_E, true,
                                         &ipEnable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get Vlan IPv4 MC Routing Failed, vlanId(%d)", vlanId);
            return status;
        }

        *enable = (uint32_t)ipEnable;

    }
    else
    {
        // Get the port interface id from L3 interface id
        status = xpsL3RetrieveInterfaceMapping(scopeId, l3IntfId, &portInterfaceId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port interface id for L3 interface: %d\n", l3IntfId);
            return status;
        }
        status = xpsInterfaceGetTypeScope(scopeId, portInterfaceId, &intfType);

        if (status != XP_NO_ERR)
        {
            return status;
        }
        if (intfType == XPS_LAG)
        {
            // If LAG get from the DB as we cannot get from the member port
            if ((status = xpsLagIpV4RouteEnableGet(scopeId, portInterfaceId,
                                                   true, enable)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ipv4 Route Failed, lag interface(%d)", portInterfaceId);
            }
            return status;
        }

        // Get the port id and dev id from the port interface id
        // TODO: is this call required?
        status = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portInterfaceId,
                                                      &portDevId, &portId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get physical port id and device from the port interface %d \n",
                  portInterfaceId);
            return status;
        }

        cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
        cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

        cpssStatus = cpssDxChIpPortRoutingEnableGet(cpssDevId, cpssPortNum,
                                                    CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4_E, (GT_BOOL *)enable);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get V4 UC enable on port %d\n", cpssPortNum);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIntfIpv6McRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsL3SetIntfIpv6McRoutingEn);

    XP_STATUS status     = XP_NO_ERR;
    xpsInterfaceType_e intfType;
    xpsScope_t scopeId;
    xpsInterfaceId_t portInterfaceId = XPS_INTF_INVALID_ID;
    xpsDevice_t portDevId;
    xpPort_t portId;
    GT_BOOL cpssEn = GT_FALSE;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum, i;
    xpsLagPortIntfList_t lagPortList;
    GT_STATUS cpssStatus = GT_OK;
    CPSS_PACKET_CMD_ENT pktCmd;

    cpssEn = (enable == 0) ? GT_FALSE : GT_TRUE;
    pktCmd = (enable == 0) ? CPSS_PACKET_CMD_FORWARD_E:CPSS_PACKET_CMD_DROP_HARD_E;

    xpsVlan_t vlanId = 0;
    int port = 0;
    int maxTotalPorts = 0;
    CPSS_PORTS_BMP_STC                   globalPortMembers;
    CPSS_PORTS_BMP_STC                   portsMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_BOOL                              isValid = GT_FALSE;
    memset(&lagPortList, 0, sizeof(lagPortList));


    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    status = xpsInterfaceGetTypeScope(scopeId, l3IntfId, &intfType);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    if ((intfType != XPS_PORT_ROUTER) && (intfType != XPS_SUBINTERFACE_ROUTER) &&
        (intfType != XPS_VLAN_ROUTER) && (intfType != XPS_TUNNEL_ROUTER))
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if (intfType == XPS_VLAN_ROUTER)
    {
        vlanId = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Vlan Id : %d\n", vlanId);

        if ((vlanId <= XPS_VLANID_MIN) && (vlanId > XPS_VLANID_MAX))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Invalid vlan id %d maxVlan %d .", vlanId, XPS_VLANID_MAX);
            return XP_ERR_INVALID_VLAN_ID;
        }
        status = xpsVlanSetRoutingEnable(devId, vlanId, XPS_PROTOCOL_IPV6_E, true,
                                         enable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Updating Vlan Context Db failed, vlanId(%d)", vlanId);
            return status;
        }

        cpssHalGetMaxGlobalPorts(&maxTotalPorts);

        memset(&globalPortMembers, 0, sizeof(globalPortMembers));
        cpssStatus = cpssHalReadBrgVlanEntry(devId, (GT_U16)vlanId, &portsMembers,
                                             &portsTagging, &vlanInfo, &isValid,
                                             &portsTaggingCmd, &globalPortMembers);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Read vlan %d failed .",
                  vlanId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        /* Enabling UC routing on all member ports of the vlan*/
        XPS_GLOBAL_PORT_ITER(port, maxTotalPorts)
        {
            cpssDevId = xpsGlobalIdToDevId(devId, port);
            cpssPortNum = xpsGlobalPortToPortnum(devId, port);
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&globalPortMembers, cpssPortNum))
            {
                cpssStatus = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId, cpssPortNum,
                                                          CPSS_IP_PROTOCOL_IPV6_E, cpssEn);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to update ipv6 MC routing for port intf : %d \n", cpssPortNum);
                    return xpsConvertCpssStatusToXPStatus(cpssStatus);
                }
            }
        }

#if 0
        /*TODO : Revisit this on supporting IPMC */
        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_BCAST_E,
                                                  pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_MCAST_E,
                                                  pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV6_MCAST_E,
                                                  pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IPV4_BCAST_E,
                                                  (pktCmd == CPSS_PACKET_CMD_DROP_HARD_E) ? CPSS_PACKET_CMD_DROP_SOFT_E : pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

#endif
        cpssStatus = cpssHalVlanUnkUnregFilterSet(devId, vlanId,
                                                  CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E,
                                                  pktCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVlanUnkUnregFilterSet Failed");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    else
    {
        // Get the port interface id from L3 interface id
        status = xpsL3RetrieveInterfaceMapping(scopeId, l3IntfId, &portInterfaceId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port interface id for L3 interface: %d\n", l3IntfId);
            return status;
        }

        status = xpsInterfaceGetTypeScope(scopeId, portInterfaceId, &intfType);

        if (status != XP_NO_ERR)
        {
            return status;
        }
        if (intfType == XPS_PORT)
        {

            // Get the port interface id from L3 interface id
            lagPortList.portIntf[0] = portInterfaceId;
            lagPortList.size = 1;
        }
        else if (intfType == XPS_LAG)
        {
            if ((status = xpsLagGetPortIntfListScope(scopeId, portInterfaceId,
                                                     &lagPortList)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ports failed, lag interface(%d)", portInterfaceId);
                return status;
            }
            if ((status = xpsLagIpV6RouteEnable(scopeId, portInterfaceId,
                                                true, enable)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Set ipv6 Route Failed, lag interface(%d)", portInterfaceId);
                return status;
            }
        }
        for (i = 0; i < lagPortList.size; i++)
        {
            portInterfaceId = lagPortList.portIntf[i];
            // Get the port id and dev id from the port interface id
            // TODO: is this call required
            status = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portInterfaceId,
                                                          &portDevId, &portId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get physical port id and device from the port interface %d \n",
                      portInterfaceId);
                return status;
            }

            cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
            cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

            cpssStatus = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId, cpssPortNum,
                                                      CPSS_IP_PROTOCOL_IPV6_E, cpssEn);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to update ipv4 MC routing for port intf : %d \n", portInterfaceId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIntfIpv6McRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsL3GetIntfIpv6McRoutingEn);

    XP_STATUS status = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;
    xpsInterfaceType_e intfType;
    xpsScope_t scopeId;
    xpsInterfaceId_t portInterfaceId;
    xpsDevice_t portDevId;
    xpPort_t portId;
    GT_U8 cpssDevId;
    GT_U32 cpssPortId;
    xpsVlan_t vlanId;
    bool ipEnable = false;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }
    status = xpsInterfaceGetTypeScope(scopeId, l3IntfId, &intfType);

    if (status)
    {
        return status;
    }

    if ((intfType != XPS_PORT_ROUTER) && (intfType != XPS_SUBINTERFACE_ROUTER) &&
        (intfType != XPS_VLAN_ROUTER) && (intfType != XPS_TUNNEL_ROUTER))
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if ((enable == NULL))
    {
        return XP_ERR_NULL_POINTER;
    }

    if (intfType == XPS_VLAN_ROUTER)
    {
        /* Retrieving vlan-id from input L3 interface*/
        vlanId = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Vlan Id : %d\n", vlanId);

        if (vlanId > XPS_VLANID_MAX)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlan id %d  .",
                  vlanId);
            return XP_ERR_INVALID_VLAN_ID;
        }

        /* Check if IPv6 Routing Enabled on Vlan Router Interface. */
        status = xpsVlanGetRoutingEnable(devId, vlanId, XPS_PROTOCOL_IPV6_E, true,
                                         &ipEnable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get Vlan IPv4 Routing Failed, vlanId(%d)", vlanId);
            return status;
        }

        *enable = (uint32_t)ipEnable;

    }
    else
    {
        // Get the port interface id from L3 interface id
        status = xpsL3RetrieveInterfaceMapping(scopeId, l3IntfId, &portInterfaceId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port interface id for L3 interface: %d\n", l3IntfId);
            return status;
        }

        status = xpsInterfaceGetTypeScope(scopeId, portInterfaceId, &intfType);

        if (status != XP_NO_ERR)
        {
            return status;
        }
        if (intfType == XPS_LAG)
        {
            // If LAG get from the DB as we cannot get from the member port
            if ((status = xpsLagIpV6RouteEnableGet(scopeId, portInterfaceId,
                                                   true, enable)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ipv6 Route Failed, lag interface(%d)", portInterfaceId);
            }
            return status;
        }

        // Get the port id and dev id from the port interface id
        status = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portInterfaceId,
                                                      &portDevId, &portId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get physical port id and device from the port interface %d \n",
                  portInterfaceId);
            return status;
        }

        cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
        cpssPortId = xpsGlobalPortToPortnum(portDevId, portId);

        cpssStatus = cpssDxChIpPortRoutingEnableGet(cpssDevId, cpssPortId,
                                                    CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV6_E, (GT_BOOL *)enable);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get V4 UC enable on port %d\n", cpssPortId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIntfCounterEn(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                                xpsDirection_t dir, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsL3SetIntfCounterEn);
    GT_STATUS cpss_status = GT_OK;
    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceType_e intfType;
    xpsScope_t scopeId;
    CPSS_IP_CNT_SET_ENT cntSet;
    CPSS_DXCH_IP_CNT_SET_MODE_ENT              cntSetMode;
    CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC interfaceModeCfg;
    CPSS_DXCH_IP_COUNTER_SET_STC               ipCounters;
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }
    status = xpsInterfaceGetTypeScope(scopeId, l3IntfId, &intfType);

    if (status)
    {
        return status;
    }

    /*
     * Count mode configurations for vlan router interfaces better configured via a single manager.
     * Else, both xpsVlanMgr and xpsL3Mgr would meddle with same config in BD table
     */
    if (intfType == XPS_VLAN_ROUTER)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Counter for vlan based router interfaces needs to be configured via xpsVlanMgr");
        return XP_ERR_INVALID_ARG;
    }

    if ((intfType != XPS_PORT_ROUTER) && (intfType != XPS_SUBINTERFACE_ROUTER) &&
        (intfType != XPS_VLAN_ROUTER) && (intfType != XPS_TUNNEL_ROUTER))
    {
        return XP_ERR_INVALID_PARAMS;
    }

    cntSet = CPSS_IP_CNT_SET0_E;
    cntSetMode = CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E;
    interfaceModeCfg.ipMode = CPSS_IP_PROTOCOL_IPV4_E;
    interfaceModeCfg.portTrunkCntMode =
        CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E;
    interfaceModeCfg.vlanMode = CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E;

    cpss_status = cpssDxChIpCntSetModeSet(devId, cntSet, cntSetMode,
                                          &interfaceModeCfg);
    if (cpss_status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssDxChIpCntSetModeSet FAILED, status = [%d]", cpss_status);

        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    /* reset IP couters */
    cpss_status = cpssDxChIpCntSet(devId, cntSet, &ipCounters);
    if (cpss_status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssDxChIpCntSet FAILED, rc = [%d]", cpss_status);

        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }
    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

XP_STATUS xpsL3GetIntfCounterEn(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                                xpsDirection_t dir, uint32_t * enable)
{
    XPS_FUNC_ENTRY_LOG();

#ifdef TOBD_INTERFACE

    XPS_LOCK(xpsL3GetIntfCounterEn);

    XP_STATUS status = XP_NO_ERR;

    xpL2DomainCtx_t l2Ctx;
    xpsInterfaceType_e intfType;
    xpsScope_t scopeId;
    xpBdCountMode_e mode = BD_COUNT_MODE_NONE;
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }
    status = xpsInterfaceGetTypeScope(scopeId, l3IntfId, &intfType);

    if (status)
    {
        return status;
    }

    /*
     * Count mode configurations for vlan router interfaces better configured via a single manager.
     * Else, both xpsVlanMgr and xpsL3Mgr would meddle with same config in BD table
     */
    if (intfType == XPS_VLAN_ROUTER)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Counter for vlan based router interfaces needs to be configured via xpsVlanMgr");
        return XP_ERR_INVALID_ARG;
    }

    if ((intfType != XPS_PORT_ROUTER) && (intfType != XPS_SUBINTERFACE_ROUTER) &&
        (intfType != XPS_VLAN_ROUTER) && (intfType != XPS_TUNNEL_ROUTER))
    {
        return XP_ERR_INVALID_PARAMS;
    }
    if ((enable == NULL))
    {
        return XP_ERR_NULL_POINTER;
    }

    memset(&l2Ctx, 0, sizeof(xpL2DomainCtx_t));

    l2Ctx.bdId = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);

    status  = xpL2DomainMgr::instance()->getCountMode(devId, l2Ctx, dir, mode);
    if (status)
    {
        return status;
    }

    *enable = (mode == BD_COUNT_MODE_NONE)?0:1;

    return status;


#endif
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIntfEgressCounterId(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t counterId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIntfEgressCounterId(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t * counterId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIntfIpv4McRouteMode(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, xpVlanRouteMcMode_t mode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIntfIpv4McRouteMode(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, xpVlanRouteMcMode_t *mode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIntfIpv6McRouteMode(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, xpVlanRouteMcMode_t mode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIntfIpv6McRouteMode(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, xpVlanRouteMcMode_t *mode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsL3SetIpv4UrpfEnable(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                                 uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIpv4UrpfMode(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                               xpsUrpfMode_e urpfMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIpv4UrpfEnable(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                                 uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIpv4UrpfMode(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                               xpsUrpfMode_e *urpfMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsL3SetIpv6UrpfEnable(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                                 uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIpv6UrpfMode(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                               xpsUrpfMode_e urpfMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIpv6UrpfEnable(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                                 uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIpv6UrpfMode(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                               xpsUrpfMode_e *urpfMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIntfEgressVif(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                                xpsInterfaceId_t *egressVif)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsL3SetIntfNatMode(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                              xpsVlanNatMode_e mode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIntfNatMode(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                              xpsVlanNatMode_e *mode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIntfNatScope(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                               uint32_t scope)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIntfNatScope(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                               uint32_t *scope)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetRouterAclEnable(xpsDevice_t devId,
                                  xpsInterfaceId_t l3IntfId, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();


    XP_STATUS result = XP_NO_ERR;
    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceType_e intfType;
    xpsScope_t scopeId;
    xpsInterfaceInfo_t *intfInfo;
    bool isAclEnable = false;

    /* Get Scope Id from devId */
    if ((result = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return result;
    }

    result = xpsInterfaceGetTypeScope(scopeId, l3IntfId, &intfType);
    switch (intfType)
    {
        case XPS_PORT_ROUTER:
            {
                /* get the interface information for the mapped interface*/
                status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &intfInfo);

                if ((status != XP_NO_ERR) || (intfInfo == NULL))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to get interface info for Interface id : %d\n", l3IntfId);
                    return status;
                }

                isAclEnable = intfInfo->isAclEnable;

                if (isAclEnable != enable)
                {
                    intfInfo->isAclEnable = enable;
                }
            }
            break;

        case XPS_SUBINTERFACE_ROUTER:
            result = XP_ERR_INVALID_ARG;
            break;
        case XPS_VLAN_ROUTER:
            /* get the interface information for the mapped interface*/
            status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &intfInfo);

            if ((status != XP_NO_ERR) || (intfInfo == NULL))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get interface info for Interface id : %d\n", l3IntfId);
                return status;
            }

            isAclEnable = intfInfo->isAclEnable;

            if (isAclEnable != enable)
            {
                intfInfo->isAclEnable = enable;
            }

            break;
        case XPS_TUNNEL_ROUTER:
            result = XP_ERR_INVALID_ARG;
            break;
        default:
            result = XP_ERR_INVALID_ARG;
    }

    return result;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetRouterAclId(xpsDevice_t devId,
                              xpsInterfaceId_t l3IntfId,
                              uint32_t groupId, uint32_t tableId)
{

    XP_STATUS result = XP_NO_ERR;
    XP_STATUS status = XP_NO_ERR;
    uint32_t bdId = 0;
    uint32_t xpsPortId = 0;
    xpsInterfaceType_e intfType;
    xpsScope_t scopeId;
    uint32_t trunkId = 0, portDevId = 0, portId = 0;
    xpsInterfaceInfo_t *intfInfo;

    /* Get Scope Id from devId */
    if ((result = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return result;
    }

    result = xpsInterfaceGetTypeScope(scopeId, l3IntfId, &intfType);
    switch (intfType)
    {
        case XPS_PORT_ROUTER:
            {

                /* get the interface information for the mapped interface*/
                status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &intfInfo);

                if ((status != XP_NO_ERR) || (intfInfo == NULL))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to get interface info for Interface id : %d\n", l3IntfId);
                    return status;
                }

                result = xpsL3RetrieveInterfaceMapping(scopeId, l3IntfId, &xpsPortId);
                if (result != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to get port interface id for L3 interface: %d\n", l3IntfId);
                    return result;
                }

                result = xpsInterfaceGetTypeScope(scopeId, xpsPortId, &intfType);

                if (result != XP_NO_ERR)
                {
                    return result;
                }

                if (intfType == XPS_LAG)
                {
                    trunkId = xpsPortId;
                    if (intfInfo->isAclEnable)
                    {
                        if (intfInfo->aclTableId == tableId)
                        {
                            return XP_NO_ERR;
                        }
                        result = xpsAclSetLagAcl(devId, trunkId, tableId, groupId);

                        if (result != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "xpsAclSetLagIngressAcl Failed on device Id %d lag %d "
                                  "tableId %d with error %d\n", devId, trunkId, tableId, result);
                            return result;
                        }
                        intfInfo->aclTableId = tableId;
                        intfInfo->aclGroupId = groupId;
                    }
                    else /* unbind */
                    {
                        tableId = intfInfo->aclTableId;
                        groupId = intfInfo->aclGroupId;

                        result = xpsAclLagUnbind(devId, trunkId, groupId, tableId);

                        if (result != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "xpsAclSetLagUnbind Failed on device Id %d lag %d "
                                  "tableId %d with error %d\n", devId, trunkId, tableId, result);
                            return result;
                        }
                    }
                }
                else /* Port */
                {
                    result = xpsPortGetDevAndPortNumFromIntfScope(scopeId, xpsPortId, &portDevId,
                                                                  &portId);
                    if (result != XP_NO_ERR)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Failed to get physical port id and device from the port interface %d \n",
                              portId);
                        return result;
                    }

                    if (intfInfo->isAclEnable)
                    {
                        if (intfInfo->aclTableId == tableId)
                        {
                            return XP_NO_ERR;
                        }
                        result = xpsAclSetPortAcl(devId, portId, tableId, groupId);

                        if (result != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "xpsAclSetPortIngressAcl Failed on device Id %d port %d "
                                  "tableId %d with error %d\n", devId, l3IntfId, tableId, result);
                            return result;
                        }

                        intfInfo->aclTableId = tableId;
                        intfInfo->aclGroupId = groupId;
                    }
                    else /* Unbind */
                    {
                        tableId = intfInfo->aclTableId;
                        groupId = intfInfo->aclGroupId;
                        result = xpsAclPortUnbind(devId, portId, groupId, tableId);

                        if (result != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "xpsAclSetPortUnbind Failed on device Id %d port %d "
                                  "tableId %d with error %d\n", devId, l3IntfId, tableId, result);
                            return result;
                        }
                    }
                }
            }
            break;

        case XPS_SUBINTERFACE_ROUTER:
            result = XP_ERR_INVALID_ARG;
            break;
        case XPS_VLAN_ROUTER:
            {
                status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &intfInfo);

                if ((status != XP_NO_ERR) || (intfInfo == NULL))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to get interface info for Interface id : %d\n", l3IntfId);
                    return status;
                }

                bdId = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);
                if (bdId >= XP_MAX_VLAN_NUM)
                {
                    return XP_ERR_INVALID_ARG;
                }
                result = xpsAclSetVlanAcl(devId, bdId, tableId, groupId);

                if (intfInfo->isAclEnable)
                {
                    if (intfInfo->aclTableId == tableId)
                    {
                        return XP_NO_ERR;
                    }

                    if (result != XP_NO_ERR)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "xpsAclSetVlanIngressAcl Failed on device Id %d port %d "
                              "tableId %d with error %d\n", devId, l3IntfId, tableId, result);
                        return result;
                    }
                    intfInfo->aclTableId = tableId;
                    intfInfo->aclGroupId = groupId;
                }
                else /* Unbind */
                {
                    tableId = intfInfo->aclTableId;
                    groupId = intfInfo->aclGroupId;

                    result = xpsAclVlanUnbind(devId, bdId, groupId, tableId);

                    if (result != XP_NO_ERR)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "xpsAclSetVlanUnbind Failed on device Id %d vlan %d "
                              "tableId %d with error %d\n", devId, bdId, tableId, result);
                        return result;
                    }
                }

            }
            break;
        case XPS_TUNNEL_ROUTER:
            result = XP_ERR_INVALID_ARG;
            break;
        default:
            result = XP_ERR_INVALID_ARG;
    }

    return result;
}

XP_STATUS xpsL3GetRouterAclId(xpsDevice_t devId,
                              xpsInterfaceId_t l3IntfId, uint32_t *aclId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceType_e intfType;
    xpsScope_t scopeId;
    xpsInterfaceInfo_t *intfInfo;

    /* Get Scope Id from devId */
    if ((result = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return result;
    }

    result = xpsInterfaceGetTypeScope(scopeId, l3IntfId, &intfType);
    switch (intfType)
    {
        case XPS_PORT_ROUTER:
            {
                /* get the interface information for the mapped interface*/
                status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &intfInfo);

                if ((status != XP_NO_ERR) || (intfInfo == NULL))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to get interface info for Interface id : %d\n", l3IntfId);
                    return status;
                }

                if (intfInfo->isAclEnable)
                {
                    *aclId = intfInfo->aclTableId;
                }
                else
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to get aclId for Interface id : %d\n", l3IntfId);
                    return status;
                }
            }
            break;
        case XPS_SUBINTERFACE_ROUTER:
            result = XP_ERR_INVALID_ARG;
            break;
        case XPS_VLAN_ROUTER:
            /* get the interface information for the mapped interface*/
            status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &intfInfo);

            if ((status != XP_NO_ERR) || (intfInfo == NULL))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get interface info for Interface id : %d\n", l3IntfId);
                return status;
            }

            if (intfInfo->isAclEnable)
            {
                *aclId = intfInfo->aclTableId;
            }
            else
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get aclId for Interface id : %d\n", l3IntfId);
                return status;
            }
            break;
        case XPS_TUNNEL_ROUTER:
            result = XP_ERR_INVALID_ARG;
            break;
        default:
            result = XP_ERR_INVALID_ARG;
    }

    return result;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetRouterAclIdLegacy(xpsDevice_t devId,
                                    xpsInterfaceId_t l3IntfId, uint32_t *aclId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetRouterAclEnable(xpsDevice_t devId,
                                  xpsInterfaceId_t l3IntfId, uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();


    XP_STATUS result = XP_NO_ERR;
    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceType_e intfType;
    xpsScope_t scopeId;
    xpsInterfaceInfo_t *intfInfo;

    /* Get Scope Id from devId */
    if ((result = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return result;
    }

    result = xpsInterfaceGetTypeScope(scopeId, l3IntfId, &intfType);
    switch (intfType)
    {
        case XPS_PORT_ROUTER:
            {
                /* get the interface information for the mapped interface*/
                status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &intfInfo);

                if ((status != XP_NO_ERR) || (intfInfo == NULL))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to get interface info for Interface id : %d\n", l3IntfId);
                    return status;
                }

                *enable = intfInfo->isAclEnable;
            }

            break;

        case XPS_SUBINTERFACE_ROUTER:
            result = XP_ERR_INVALID_ARG;
            break;

        case XPS_VLAN_ROUTER:
            {
                /* get the interface information for the mapped interface*/
                status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &intfInfo);

                if ((status != XP_NO_ERR) || (intfInfo == NULL))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to get interface info for Interface id : %d\n", l3IntfId);
                    return status;
                }

                *enable = intfInfo->isAclEnable;
            }
            break;

        case XPS_TUNNEL_ROUTER:
            result = XP_ERR_INVALID_ARG;
            break;
        default:
            result = XP_ERR_INVALID_ARG;
    }


    XPS_FUNC_EXIT_LOG();

    return result;
}


XP_STATUS xpsL3GetRouterAclEnableLegacy(xpsDevice_t devId,
                                        xpsInterfaceId_t l3IntfId, uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/////*******************************L3 Host Management API's  ********************************/////

static XP_STATUS xpsL3FdbMgrEnable(xpsDevice_t devId, bool enable)
{
    XPS_FUNC_ENTRY_LOG();
#if TODO
    /* Enable, Disable FdbManager for L3 Neighbor*/
#endif
    xpsEnableL3FdbMgr = enable;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3InitIpHost(void)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsL3InitIpHostScope(XP_SCOPE_DEFAULT);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3InitIpHostScope(xpsScope_t scopeId)
{
    GT_STATUS cpss_status;
    XP_DEV_TYPE_T devType;
    cpssHalGetDeviceType((xpsDevice_t)scopeId, &devType);

    if (devType == ALDB2B || devType == ALDRIN2XL || devType == TG48M_P)
    {
        xpsL3FdbMgrEnable((xpsDevice_t)scopeId, FALSE);
    }

    cpss_status = cpssHalL3IpHeaderErrorMaskSet(scopeId);
    if (cpss_status != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }
    return XP_NO_ERR;
}

static bool xpsIsL3FdbMgrEnabled(xpsDevice_t devId)
{
    return xpsEnableL3FdbMgr;
}

XP_STATUS xpsL3DeInitIpHost(void)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsL3DeInitIpHostScope(XP_SCOPE_DEFAULT);

    XPS_FUNC_EXIT_LOG();
}

XP_STATUS xpsL3DeInitIpHostScope(xpsScope_t scopeId)
{
    return XP_NO_ERR;
}

XP_STATUS xpsL3AddIpHostDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    GT_STATUS cpss_status = GT_OK;
    XP_STATUS status = XP_NO_ERR;

#if TBD_HOST
    /*
    *  For IPv4/Ipv6 Host Entries with pkt commands other than FWD, should be downloaded in LPM.
    *  FDB Table supports only one command at a time. Not per Entry.
    */
#endif

    /* Initialize MAC2Me Allocator*/
    if ((status = xpsInitMac2MeAllocator(devId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: failded to initialize Mac2Me Allocator on device %d\n", devId);
        return status;
    }
    /* For B2B support: Enabling Fdb routing command and MultiHash Mode*/
    /* Set Packet Commands for FDB routed packets */
    cpss_status = cpssHalSetBrgFdbRoutingNextHopPacketCmd(devId,
                                                          CPSS_PACKET_CMD_ROUTE_E);

    if (cpss_status != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

XP_STATUS xpsL3RemoveIpHostDevice(xpsDevice_t devId)
{
    return XP_NO_ERR;
}

/////******************************** Internal L3 Host FDB Manager Management Start ********************************/////

/**
 * \brief to add IPv4/Ipv6 Neighbor entry to FDB that is managed by FDB Manager
 *
 *\param [in] devId           Device Id
 *\param [in] pL3HostEntry    Neighbor Entry
 *
 *
 *\returns XP_STATUS indicating success or failure
 **/
static XP_STATUS xpsL3AddIpHostEntryUsingFdbMgr(xpsDevice_t devId,
                                                xpsL3HostEntry_t *pL3HostEntry)
{
    XPS_FUNC_ENTRY_LOG();
    GT_STATUS cpssStatus = GT_OK;
    XP_STATUS xpsStatus = XP_NO_ERR;
    xpsArpPointer_t arpPointer;
    uint32_t fdbManagerId = xpsFdbGetManagerId();
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV4_UC_E;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    // Null Pointer Check
    if (pL3HostEntry == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (pL3HostEntry->type >= XP_PREFIX_TYPE_MAX)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if (pL3HostEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
    }

    /*Create MAC Da in SW & HW Mac Da Table (arp). Mark it as NextHop Active in SW*/
    if ((xpsStatus = xpsArpCreate(devId, pL3HostEntry->nhEntry.nextHop.macDa,
                                  ARP_NEIGHBOR_ACTIVE, &arpPointer)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsArpCreate Failed on device Id %d", devId);
        return xpsStatus;
    }

    /* CPSS HAL FDB Manager handles IPv4 and IPv6 neighbor based on actual address type in pL3HostEntry*/
    cpssStatus = cpssHalFdbMgrMacEntryWrite(devId, fdbManagerId, fdbEntryType,
                                            (void *) pL3HostEntry);
    XPS_FUNC_EXIT_LOG();
    return xpsConvertCpssStatusToXPStatus(cpssStatus);
}

/**
 *\brief to update IPv4/Ipv6 Neighbor entry in FDB that is managed by FDB Manager
 *
 *\param [in] devId           Device Id
 *\param [in] pL3HostEntry    Neighbor Entry
 *
 *\returns XP_STATUS indicating success or failure
 **/
static XP_STATUS xpsL3UpdateIpHostEntryUsingFdbMgr(xpsDevice_t devId,
                                                   xpsL3HostEntry_t *pL3HostEntry)
{
    XPS_FUNC_ENTRY_LOG();
    GT_STATUS cpssStatus = GT_OK;
    XP_STATUS xpsStatus = XP_NO_ERR;
    xpsL3HostEntry_t readL3HostEntry;
    GT_BOOL isEqual = GT_FALSE;
    xpsArpPointer_t arpPointer;
    uint32_t fdbManagerId = xpsFdbGetManagerId();
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV4_UC_E;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    // Null Pointer Check
    if (pL3HostEntry == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (pL3HostEntry->type >= XP_PREFIX_TYPE_MAX)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    memcpy(&readL3HostEntry, pL3HostEntry, sizeof(xpsL3HostEntry_t));

    if (pL3HostEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
    }

    cpssStatus = cpssHalFdbMgrMacEntryGet(devId, fdbManagerId, fdbEntryType,
                                          (void *)&readL3HostEntry);

    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to update Host Entry due to an issue while reading it in device Id %d \n",
              devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    /* Check if MAC address has changed or not */
    isEqual = (0 == cpssOsMemCmp((GT_VOID*) &pL3HostEntry->nhEntry.nextHop.macDa,
                                 (GT_VOID*) &readL3HostEntry.nhEntry.nextHop.macDa,
                                 sizeof(macAddr_t))) ? GT_TRUE : GT_FALSE;

    /* if MAC DA has changed. Then delete old one and create a new one*/
    if (isEqual != GT_TRUE)
    {
        if ((xpsStatus = xpsArpDelete(devId, readL3HostEntry.nhEntry.nextHop.macDa,
                                      ARP_NEIGHBOR_ACTIVE)) != XP_NO_ERR)
        {
            return xpsStatus;
        }

        /*Create MAC Da in SW & HW Mac Da Table (arp). Mark it as NextHop Active in SW*/
        if ((xpsStatus = xpsArpCreate(devId, pL3HostEntry->nhEntry.nextHop.macDa,
                                      ARP_NEIGHBOR_ACTIVE, &arpPointer)) != XP_NO_ERR)
        {
            return xpsStatus;
        }
    }

    /* CPSS HAL FDB Manager handles IPv4 and IPv6 neighbor based on actual address type.  */
    cpssStatus = cpssHalFdbMgrMacEntryUpdate(devId, fdbManagerId, fdbEntryType,
                                             (void *) pL3HostEntry);
    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpssStatus);
}

/**
 *\brief to remove IPv4/Ipv6 Neighbor entry from FDB that is managed by FDB Manager
 *\param [in] devId         Device Id
 *\param [in] pL3HostEntry  Neighbor Entry's Key
 *
 *\returns XP_STATUS indicating success or failure
 **/
static XP_STATUS xpsL3RemoveIpHostEntryUsingFdbMgr(xpsDevice_t devId,
                                                   xpsL3HostEntry_t *pL3HostEntry)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS cpssStatus = GT_OK;
    XP_STATUS xpsStatus = XP_NO_ERR;
    uint32_t fdbManagerId = xpsFdbGetManagerId();
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV4_UC_E;
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    // Null Pointer Check
    if (pL3HostEntry == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (pL3HostEntry->type >= XP_PREFIX_TYPE_MAX)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if (pL3HostEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
    }
    cpssStatus = cpssHalFdbMgrMacEntryDelete(devId, fdbManagerId, fdbEntryType,
                                             (void *) pL3HostEntry);

    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: could not remove IP host entry in device %d", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    if ((xpsStatus = xpsArpDelete(devId, pL3HostEntry->nhEntry.nextHop.macDa,
                                  ARP_NEIGHBOR_ACTIVE)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsArpDelete is not Success in device Id %d", devId);
        return xpsStatus;
    }

    XPS_FUNC_EXIT_LOG();
    return xpsConvertCpssStatusToXPStatus(cpssStatus);
}

/**
 *\brief to retrieve IPv4/Ipv6 Neighbor entry from FDB that is managed by FDB Manager
 *\param [in] devId             Device Id
 *\param [in out] pL3HostEntry [in] Neighbor Entry Key [out] Neighbor Entry
 *\returns XP_STATUS indicating success or failure
 **/
static XP_STATUS xpsL3GetIpHostEntryUsingFdbMgr(xpsDevice_t devId,
                                                xpsL3HostEntry_t *pL3HostEntry)
{
    XPS_FUNC_ENTRY_LOG();

    uint32_t fdbManagerId = xpsFdbGetManagerId();
    GT_STATUS cpssStatus = GT_OK;
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV4_UC_E;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    // Null Pointer Check
    if (pL3HostEntry == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (pL3HostEntry->type >= XP_PREFIX_TYPE_MAX)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if (pL3HostEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
    }

    /* CPSS HAL FDB Manager handles IPv4 and IPv6 neighbor based on actual address type in pL3HostEntry*/
    cpssStatus = cpssHalFdbMgrMacEntryGet(devId, fdbManagerId, fdbEntryType,
                                          (void *)pL3HostEntry);

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpssStatus);
}

/**
 *\brief to add IPv4/IPv6 neighbor entry for control to FDB that is managed by FDB Manager
 *
 *\param [in] devId           Device Id
 *\param [in] pL3HostEntry    Neighbor Entry's key
 *\returns XP_STATUS indicating success or failure
 **/
static XP_STATUS xpsL3AddIpHostControlEntryUsingFdbMgr(xpsDevice_t devId,
                                                       xpsL3HostEntry_t *pL3HostEntry)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS cpssStatus = GT_OK;
    uint32_t fdbManagerId = xpsFdbGetManagerId();
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV4_UC_E;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    // Null Pointer Check
    if (pL3HostEntry == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (pL3HostEntry->type >= XP_PREFIX_TYPE_MAX)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    /* Required to make it Control Entry*/
    pL3HostEntry->nhEntry.pktCmd = XP_PKTCMD_TRAP;

    if (pL3HostEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
    }

    /* CPSS HAL FDB Manager handles IPv4 and IPv6 neighbor based on actual address type.  */
    cpssStatus = cpssHalFdbMgrMacEntryWrite(devId, fdbManagerId, fdbEntryType,
                                            (void *) pL3HostEntry);

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpssStatus);
}

/*********** End of IP Host FDB Manager APIs **************************/

XP_STATUS xpsL3AddIpHostEntry(xpsDevice_t devId, xpsL3HostEntry_t *pL3HostEntry,
                              xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS cpss_status = GT_OK;
    XP_STATUS xps_status = XP_NO_ERR;
    uint32_t index, bankIndex = 0;
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV4_UC_E;
    xpsArpPointer_t arpPointer;

    /*************************************************/
    /* CPSS FDB Manager manages IP Host Entries      */
    /*************************************************/
    if (xpsIsL3FdbMgrEnabled(devId) == TRUE)
    {
        if (indexList != NULL)
        {
            /* reset the index*/
            indexList->size = 0;
        }
        return xpsL3AddIpHostEntryUsingFdbMgr(devId, pL3HostEntry);
    }

    if (indexList == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }
    /* reset the index*/
    indexList->size = 0;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    // Null Pointer Check
    if (!pL3HostEntry)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (pL3HostEntry->type >= XP_PREFIX_TYPE_MAX)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    /*Create MAC Da in SW & HW Mac Da Table (arp). Mark it as NextHop Active in SW*/
    if ((xps_status = xpsArpCreate(devId, pL3HostEntry->nhEntry.nextHop.macDa,
                                   ARP_NEIGHBOR_ACTIVE, &arpPointer)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsArpCreate Failed with return Value= %d", xps_status);
        return xps_status;
    }

    if (pL3HostEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        /* Add IPv6 Address and Data */
        cpss_status = xpsL3AddIpv6UcHostEntry(devId, &index, pL3HostEntry);
    }
    else
    {
        cpss_status = cpssHalBrgFdbMacEntryIndexFind(devId, fdbEntryType, pL3HostEntry,
                                                     &index, &bankIndex);
        /* Check if something went wrong or table is full*/
        if (cpss_status != GT_OK && cpss_status != GT_NOT_FOUND)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalBrgFdbMacEntryIndexFind returned with cpss_status= %d", cpss_status);
            return xpsConvertCpssStatusToXPStatus(cpss_status);
        }
        /*Create a new Host Fdb Mac Entry at the specified index */
        cpss_status = cpssHalBrgFdbMacEntryWrite(devId, index, GT_FALSE, fdbEntryType,
                                                 pL3HostEntry);
    }

    /* return the Index for Ipv4 Host Entry*/
    indexList->size = 1;
    indexList->index[0] = index;

    XPS_FUNC_EXIT_LOG();
    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

XP_STATUS xpsL3UpdateIpHostEntry(xpsDevice_t devId,
                                 xpsL3HostEntry_t *pL3HostEntry)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS cpss_status = GT_OK;
    XP_STATUS xps_status = XP_NO_ERR;
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV4_UC_E;
    xpsL3HostEntry_t readL3HostEntry;
    xpsArpPointer_t arpPointer;
    GT_BOOL isEqual = GT_FALSE;

    /*************************************************/
    /* CPSS FDB Manager manages IP Host Entries      */
    /*************************************************/
    if (xpsIsL3FdbMgrEnabled(devId) == TRUE)
    {
        return xpsL3UpdateIpHostEntryUsingFdbMgr(devId, pL3HostEntry);
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    // Null Pointer Check
    if (!pL3HostEntry)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (pL3HostEntry->type >= XP_PREFIX_TYPE_MAX)
    {
        return XP_ERR_INVALID_PARAMS;
    }
    if (pL3HostEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E;
    }

    cpss_status = cpssHalBrgFdbMacEntryGet(devId, fdbEntryType, pL3HostEntry,
                                           &readL3HostEntry);

    if (cpss_status != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }


    /* Verifying values */
    isEqual = (0 == cpssOsMemCmp((GT_VOID*) &pL3HostEntry->nhEntry.nextHop.macDa,
                                 (GT_VOID*) &readL3HostEntry.nhEntry.nextHop.macDa,
                                 sizeof(macAddr_t))) ? GT_TRUE : GT_FALSE;

    /* if MAC DA has changed. Then delete old one and create a new one*/
    if (isEqual != GT_TRUE)
    {
        if ((xps_status = xpsArpDelete(devId, readL3HostEntry.nhEntry.nextHop.macDa,
                                       ARP_NEIGHBOR_ACTIVE)) != XP_NO_ERR)
        {
            return xps_status;
        }

        /*Create MAC Da in SW & HW Mac Da Table (arp). Mark it as NextHop Active in SW*/
        if ((xps_status = xpsArpCreate(devId, pL3HostEntry->nhEntry.nextHop.macDa,
                                       ARP_NEIGHBOR_ACTIVE, &arpPointer)) != XP_NO_ERR)
        {
            return xps_status;
        }
    }

    /* No need to setup Next Hop Entry. It is done as part of FdbMacEntrySet*/
    //status = xpsL3SetupNextHopEntry(devId, &(pL3HostEntry->nhEntry), &nextHopT, &nhType);

    if (pL3HostEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        return xpsL3UpdateIpv6UcHostEntry(devId, NOT_INIT_CNS, pL3HostEntry);
    }

    /* Find the Host Fdb Mac Entry Index and update it, if exists */
    cpss_status = cpssHalBrgFdbMacEntrySet(devId, fdbEntryType,
                                           (void *) pL3HostEntry, PRV_XPS_BRG_FDB_ACCESS_MODE_BY_INDEX_E);

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

/*
*   Updates only IPv6 data Entry for Host.
*
*/
static XP_STATUS xpsL3UpdateIpv6UcHostEntry(xpsDevice_t devId, uint32_t index,
                                            xpsL3HostEntry_t *pL3HostEntry)
{
    GT_STATUS cpss_status = GT_OK;
    uint32_t ipv6UcFdbEntryIndex = 0, ipv6UFdbEntryDataIndex = 0;
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType;
    uint32_t dataBankIndex;
    /* Index for IPv6 address Entry is provided lets verify if exists.*/
    if (index != NOT_INIT_CNS)
    {
        fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
        /*Lets find the data Entry index. Update only happens on IPv6 Data Entry*/
        cpss_status = cpssHalBrgFdbMacEntryIndexFind(devId, fdbEntryType, pL3HostEntry,
                                                     &ipv6UcFdbEntryIndex, &dataBankIndex);
        /* Entry is not found */
        if (cpss_status != GT_OK)
        {
            return xpsConvertCpssStatusToXPStatus(cpss_status);
        }

        if (ipv6UcFdbEntryIndex != index)
        {
            return XP_ERR_INVALID_DATA;
        }
    }

    fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E;
    /*Lets find the data Entry index. Update only happens on IPv6 Data Entry*/
    cpss_status = cpssHalBrgFdbMacEntryIndexFind(devId, fdbEntryType, pL3HostEntry,
                                                 &ipv6UcFdbEntryIndex, &dataBankIndex);

    /* Entry is not found */
    if (cpss_status != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devId) == GT_FALSE)
    {
        ipv6UFdbEntryDataIndex = ipv6UcFdbEntryIndex;
    }
    else
    {
        /* On SIP-6, IPV6 UC Address entry must be in even bank, and Data entry immediately follows */
        //ipv6UFdbEntryDataIndex = ipv6UcFdbEntryIndex + 1; /* Not supported yet*/
    }

    /* Set ipv6 uc route DATA entry in FDB. The dataBank Index should not be used in Ipv6 Data Entry. It's ignored inside the API*/
    cpss_status = cpssHalBrgFdbMacEntryWrite_withBankIndex(devId,
                                                           ipv6UFdbEntryDataIndex, dataBankIndex, GT_FALSE, fdbEntryType,
                                                           (void *) pL3HostEntry);

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

/*
*   This API writes the IPv6 data and IPv6 address for Host where the address entry points to its data using Next
*   Hop Bank Number.
*   In case there is any failure in writing IPv6 address, it removes IPv6 Data Entry.
*
*/
static XP_STATUS xpsL3AddIpv6UcHostEntry(xpsDevice_t devId, uint32_t *index,
                                         xpsL3HostEntry_t *pL3HostEntry)
{
    GT_STATUS cpss_status = GT_OK;
    uint32_t ipv6UcFdbEntryIndex = 0, ipv6UFdbEntryDataIndex = 0;
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType;
    uint32_t dataBankIndex, addressBankIndex;
    /* set entry type */
    fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;

    /* Calculate index for ipv6 uc route DATA entry in FDB and get it's dataBank Index */
    cpss_status = cpssHalBrgFdbMacEntryIndexFind(devId, fdbEntryType, pL3HostEntry,
                                                 &ipv6UcFdbEntryIndex, &dataBankIndex);
    if (cpss_status != GT_OK && cpss_status != GT_NOT_FOUND)
    {
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    cpss_status = GT_OK; /* in case GT_NOT_FOUND*/

    if (PRV_CPSS_SIP_6_CHECK_MAC(devId) == GT_FALSE)
    {
        ipv6UFdbEntryDataIndex = ipv6UcFdbEntryIndex;
    }
    else
    {
        /* On SIP-6, IPV6 UC Address entry must be in even bank, and Data entry immediately follows */
        //ipv6UFdbEntryDataIndex = ipv6UcFdbEntryIndex + 1; /* Not Supported */
    }

    fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E;

    /* Set ipv6 uc route DATA entry in FDB. The dataBank Index should not be used in Ipv6 Data Entry. It's ignored inside the API*/
    cpss_status = cpssHalBrgFdbMacEntryWrite_withBankIndex(devId,
                                                           ipv6UFdbEntryDataIndex, dataBankIndex, GT_FALSE, fdbEntryType,
                                                           (void *) pL3HostEntry);

    if (cpss_status != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    /* set entry type */
    fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;

    /* Calculates an index for IPv6 'Address Entry for SIP5. No need to calculate an index for IPv6 Address in SIP6 as IPv6 Data follows IPv6 Address Entry*/
    if (PRV_CPSS_SIP_6_CHECK_MAC(devId) == GT_FALSE)
    {
        cpss_status = cpssHalBrgFdbMacEntryIndexFind(devId, fdbEntryType, pL3HostEntry,
                                                     &ipv6UcFdbEntryIndex, &addressBankIndex);

        /* Check if something went wrong or It's Full*/
        if (cpss_status != GT_OK && cpss_status != GT_NOT_FOUND)
        {
            GT_STATUS rc = GT_OK;
            /* Deletes IPv6 Host 'Data' Entry as could not write IPv6 Host 'Address' entry*/
            if ((rc = cpssHalBrgFdbMacEntryInvalidate(devId,
                                                      ipv6UFdbEntryDataIndex)) != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to remove IPv6 Host Data Entry\n");
                return xpsConvertCpssStatusToXPStatus(cpss_status);
            }

            return xpsConvertCpssStatusToXPStatus(cpss_status);
        }
        cpss_status = GT_OK; /* in case GT_NOT_FOUND*/
    }

    /* Set ipv6 uc route ADDRESS entry in FDB. Use dataBank Index to reference data Entry in IPv6 Address Entry*/
    cpss_status = cpssHalBrgFdbMacEntryWrite_withBankIndex(devId,
                                                           ipv6UcFdbEntryIndex, dataBankIndex,  GT_FALSE, fdbEntryType,
                                                           (void *) pL3HostEntry);

    if (cpss_status != GT_OK)
    {
        GT_STATUS rc = GT_OK;
        /* Deletes IPv6 Host 'Data' Entry as could not write IPv6 Host 'Address' entry*/
        if ((rc = cpssHalBrgFdbMacEntryInvalidate(devId,
                                                  ipv6UFdbEntryDataIndex)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to remove IPv6 Host Data Entry\n");
            return xpsConvertCpssStatusToXPStatus(cpss_status);
        }
    }

    /* Return to the Application the IPv6 Data Entry Index. Should not update Address entry in subsequance calls*/
    *index = ipv6UcFdbEntryIndex;


    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

XP_STATUS xpsL3UpdateIpHostEntryByIndex(xpsDevice_t devId, uint32_t index,
                                        xpsL3HostEntry_t *pL3HostEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV4_UC_E;
    GT_STATUS cpss_status = GT_OK;

    /*************************************************/
    /* CPSS FDB Manager manages IP Host Entries      */
    /*************************************************/
    if (xpsIsL3FdbMgrEnabled(devId) == TRUE)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: updating IP Host using an index is not supported on device %d\n",
              devId);
        return XP_ERR_NOT_SUPPORTED;
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    // Null Pointer Check
    if (!pL3HostEntry)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (pL3HostEntry->type >= XP_PREFIX_TYPE_MAX)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if (pL3HostEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        return xpsL3UpdateIpv6UcHostEntry(devId, index,  pL3HostEntry);
    }

    /* Update the Host FdbMac Entry at specified index*/
    cpss_status = cpssHalBrgFdbMacEntryWrite(devId, index, GT_FALSE, fdbEntryType,
                                             (void *) pL3HostEntry);

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

XP_STATUS xpsL3GetNumOfValidIpHostEntries(xpsDevice_t devId,
                                          xpIpPrefixType_t type, uint32_t *numOfValidEntries)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetNumOfValidNhEntries(xpsDevice_t devId,
                                      uint32_t *numOfValidEntries)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3NhGetTableSize(xpsDevice_t devId, uint32_t *tsize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIpHostEntry(xpsDevice_t devId, xpsL3HostEntry_t *pL3HostEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV4_UC_E;
    GT_STATUS cpss_status = GT_OK;

    /*************************************************/
    /* CPSS FDB Manager manages IP Host Fdb based    */
    /*************************************************/
    if (xpsIsL3FdbMgrEnabled(devId) == TRUE)
    {
        return xpsL3GetIpHostEntryUsingFdbMgr(devId, pL3HostEntry);
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    // Null Pointer Check
    if (!pL3HostEntry)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (pL3HostEntry->type >= XP_PREFIX_TYPE_MAX)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if (pL3HostEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E;
        xpsL3HostEntry_t pL3HostEntryKey;

        memcpy(pL3HostEntryKey.ipv6Addr, pL3HostEntry->ipv6Addr, sizeof(ipv6Addr_t));
        pL3HostEntryKey.vrfId = pL3HostEntry->vrfId;

        // Hash the key and find entry if exists. pL3HostEntry acts as a Key. The found entry is findL3HostEntry
        cpss_status = cpssHalBrgFdbMacEntryGet(devId, fdbEntryType,
                                               (void *)&pL3HostEntryKey, (void *)pL3HostEntry);

        /* Copy Back Ipv6 Address Entry as requested to read IPv6 Data only*/
        memcpy(pL3HostEntry->ipv6Addr, pL3HostEntryKey.ipv6Addr, sizeof(ipv6Addr_t));
        pL3HostEntry->vrfId = pL3HostEntryKey.vrfId;
    }
    else
    {

        // Hash the key and find entry if exists. pL3HostEntry acts as a Key. The found entry is findL3HostEntry
        cpss_status = cpssHalBrgFdbMacEntryGet(devId, fdbEntryType,
                                               (void *)pL3HostEntry, (void *)pL3HostEntry);
    }
    //Check if Entry is found before it is assigned.
    if (cpss_status != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

XP_STATUS xpsL3GetIpHostEntryByIndex(xpsDevice_t devId, uint32_t index,
                                     xpsL3HostEntry_t *pL3HostEntry)
{
    XPS_FUNC_ENTRY_LOG();

    GT_BOOL valid = GT_FALSE;
    GT_BOOL skip  = GT_FALSE;
    GT_BOOL aged  = GT_FALSE;
    GT_HW_DEV_NUM hwDev = 0;
    GT_STATUS cpss_status = GT_OK;
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV4_UC_E;
    uint32_t ipv6UcFdbEntryIndex, dataBankIndex;
    xpsL3HostEntry_t ipv6L3HostAddEntry;

    /*************************************************/
    /* CPSS FDB Manager manages IP Host Entries      */
    /*************************************************/
    if (xpsIsL3FdbMgrEnabled(devId) == TRUE)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: retrieving IP Host using an index is not supported on device %d\n",
              devId);
        return XP_ERR_NOT_SUPPORTED;
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    // Null Pointer Check
    if (!pL3HostEntry)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (pL3HostEntry->type >= XP_PREFIX_TYPE_MAX)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if (pL3HostEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
        /* read MAC entry at specified index*/
        cpss_status = cpssHalBrgFdbMacEntryRead(devId, index, &valid, &skip, &aged,
                                                &hwDev, fdbEntryType, (void *)&ipv6L3HostAddEntry);

        fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E;
        /*Lets find the data Entry index. Update only happens on IPv6 Data Entry*/
        cpss_status = cpssHalBrgFdbMacEntryIndexFind(devId, fdbEntryType,
                                                     (void *) &ipv6L3HostAddEntry, &ipv6UcFdbEntryIndex, &dataBankIndex);
        /* Entry is not found */
        if (cpss_status != GT_OK)
        {
            return xpsConvertCpssStatusToXPStatus(cpss_status);
        }

        /* read IPv6 Data Entry*/
        cpss_status = cpssHalBrgFdbMacEntryRead(devId, ipv6UcFdbEntryIndex, &valid,
                                                &skip, &aged, &hwDev, fdbEntryType, pL3HostEntry);

        /* Entry is not found */
        if (cpss_status != GT_OK)
        {
            return xpsConvertCpssStatusToXPStatus(cpss_status);
        }

        //Combine both IPv6 address Host and Data Entries
        memcpy(pL3HostEntry->ipv6Addr, ipv6L3HostAddEntry.ipv6Addr, sizeof(ipv6Addr_t));
    }
    else
    {
        /* read MAC entry at specified index*/
        cpss_status = cpssHalBrgFdbMacEntryRead(devId, index, &valid, &skip, &aged,
                                                &hwDev, fdbEntryType, pL3HostEntry);
    }

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

XP_STATUS xpsL3RemoveIpHostEntry(xpsDevice_t devId,
                                 xpsL3HostEntry_t *pL3HostEntry)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS cpss_status = GT_OK;
    XP_STATUS xps_status = XP_NO_ERR;
    uint32_t index = 0;
    uint32_t dataBankIndex, ipv6UcFdbEntryIndex;
    GT_U8    devNum;

    /*************************************************/
    /* CPSS FDB Manager manages IP Host Entries      */
    /*************************************************/
    if (xpsIsL3FdbMgrEnabled(devId) == TRUE)
    {
        return xpsL3RemoveIpHostEntryUsingFdbMgr(devId, pL3HostEntry);
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    // Null Pointer Check
    if (!pL3HostEntry)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (pL3HostEntry->type >= XP_PREFIX_TYPE_MAX)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if (pL3HostEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E;
        /*Find the dataEntry Index*/
        cpss_status = cpssHalBrgFdbMacEntryIndexFind(devId, fdbEntryType, pL3HostEntry,
                                                     &ipv6UcFdbEntryIndex, &dataBankIndex);
        /* Entry is not found */
        if (cpss_status != GT_OK)
        {
            return xpsConvertCpssStatusToXPStatus(cpss_status);
        }


        /* Delete The IPv6 Host Unicast Entry for IPv6 Data Entry*/
        cpss_status = cpssHalBrgFdbMacEntryInvalidate(devId, ipv6UcFdbEntryIndex);
        if (cpss_status != GT_OK)
        {
            return xpsConvertCpssStatusToXPStatus(cpss_status);
        }

        /*Find Ipv6 Address Entry index */
        cpss_status = xpsL3FindIpHostEntry(devId, pL3HostEntry, (int *)&index);
        if (cpss_status != GT_OK)
        {
            return xpsConvertCpssStatusToXPStatus(cpss_status);
        }

        /* Delete The IPv6 Host Unicast Entry for Ipv6 Address*/
        cpss_status = cpssHalBrgFdbMacEntryInvalidate(devId, index);


    }
    else
    {
        XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
        {
            cpss_status = xpsL3FindIpHostEntry(devNum, pL3HostEntry, (int *)&index);
            if (cpss_status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to find IP Host Entry " FMT_IP4
                      " :xp status=%d at index=%d in device=%d",
                      PRI_IP4(pL3HostEntry->ipv4Addr), cpss_status, index, devNum);
                //return XP_NO_ERR;
            }
            else
            {
                break;
            }
        }

        /* Delete The IPv4/IPv6 Host Unicast Entry*/
        cpss_status = cpssHalBrgFdbMacEntryInvalidate(devId, index);
        if (cpss_status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalBrgFdbMacEntryInvalidate failed for device=%d and Index=%d: cpss_status=%d",
                  devId, index, cpss_status);
        }
    }

    if ((xps_status = xpsArpDelete(devId, pL3HostEntry->nhEntry.nextHop.macDa,
                                   ARP_NEIGHBOR_ACTIVE)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsArpDelete is not Success with return value=%d", xps_status);
        return xps_status;
    }

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

XP_STATUS xpsL3RemoveIpHostEntryByIndex(xpsDevice_t devId, uint32_t index,
                                        xpIpPrefixType_t pL3PrefixType)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS cpss_status = GT_OK;

    /*************************************************/
    /* CPSS FDB Manager manages IP Host Entries      */
    /*************************************************/
    if (xpsIsL3FdbMgrEnabled(devId) == TRUE)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: removing IP Host using an index is not supported on device %d\n",
              devId);
        return XP_ERR_NOT_SUPPORTED;
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    if (pL3PrefixType >= XP_PREFIX_TYPE_MAX)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if (pL3PrefixType == XP_PREFIX_TYPE_IPV6)
    {
        GT_BOOL valid = GT_FALSE;
        GT_BOOL skip  = GT_FALSE;
        GT_BOOL aged  = GT_FALSE;
        GT_HW_DEV_NUM hwDev = 0;
        uint32_t ipv6UcFdbEntryIndex, dataBankIndex;
        XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
        xpsL3HostEntry_t ipv6L3HostAddEntry;

        /* read MAC entry at specified index*/
        cpss_status = cpssHalBrgFdbMacEntryRead(devId, index, &valid, &skip, &aged,
                                                &hwDev, fdbEntryType, (void *)&ipv6L3HostAddEntry);

        fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E;
        /*Lets find the data Entry index. Update only happens on IPv6 Data Entry*/
        cpss_status = cpssHalBrgFdbMacEntryIndexFind(devId, fdbEntryType,
                                                     (void *) &ipv6L3HostAddEntry, &ipv6UcFdbEntryIndex, &dataBankIndex);
        /* Entry is not found */
        if (cpss_status != GT_OK)
        {
            return xpsConvertCpssStatusToXPStatus(cpss_status);
        }

        /* Delete The IPv6 Host Data*/
        cpss_status = cpssHalBrgFdbMacEntryInvalidate(devId, ipv6UcFdbEntryIndex);

        /* Entry is not found */
        if (cpss_status != GT_OK)
        {
            return xpsConvertCpssStatusToXPStatus(cpss_status);
        }
        /*Delete IPv6 Host Address*/
        cpss_status = cpssHalBrgFdbMacEntryInvalidate(devId, index);
    }
    else
    {
        /* Delete The IPv4/IPv6 Host Unicast Entry*/
        cpss_status = cpssHalBrgFdbMacEntryInvalidate(devId, index);
    }
    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

XP_STATUS xpsL3FindIpHostEntry(xpsDevice_t devId,
                               xpsL3HostEntry_t *pL3HostEntry, int *index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsL3FindIpHostEntry);

    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV4_UC_E;
    GT_STATUS cpss_status = GT_OK;
    GT_U32 bankIndex = 0;

    /*************************************************/
    /* CPSS FDB Manager manages IP Host Entries      */
    /*************************************************/
    if (xpsIsL3FdbMgrEnabled(devId) == TRUE)
    {
        //FDB Manager does not return entry's index.
        if (index != NULL)
        {
            *index = NOT_INIT_CNS;
        }

        return xpsL3GetIpHostEntry(devId, pL3HostEntry);
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    // Null Pointer Check
    if ((!pL3HostEntry) || (!index))
    {
        return XP_ERR_NULL_POINTER;
    }

    if (pL3HostEntry->type >= XP_PREFIX_TYPE_MAX)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    if (pL3HostEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
    }

    cpss_status = cpssHalBrgFdbMacEntryIndexFind(devId, fdbEntryType,
                                                 (void *) pL3HostEntry, (uint32_t *)index, &bankIndex);

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

XP_STATUS xpsL3FlushIpHostEntries(xpDevice_t devId,
                                  xpsL3HostEntry_t *pL3HostEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3FlushIpHostEntriesWithShadowUpdate(xpDevice_t devId,
                                                  xpsL3HostEntry_t *pL3HostEntry, uint8_t updateShadow)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3AddIpHostControlEntry(xpsDevice_t devId,
                                     xpsL3HostEntry_t *pL3HostEntry, xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS cpss_status = GT_OK;
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV4_UC_E;
    uint32_t index, bankIndex = 0;

    /*************************************************/
    /* CPSS FDB Manager manages IP Host Entries      */
    /*************************************************/
    if (xpsIsL3FdbMgrEnabled(devId) == TRUE)
    {
        if (indexList != NULL)
        {
            /* reset the index*/
            indexList->size = 0;
        }
        return xpsL3AddIpHostControlEntryUsingFdbMgr(devId, pL3HostEntry);
    }

    if (indexList == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }


    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    // Null Pointer Check
    if (!pL3HostEntry)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (pL3HostEntry->type >= XP_PREFIX_TYPE_MAX)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    /* Required to make it Control Entry*/
    pL3HostEntry->nhEntry.pktCmd = XP_PKTCMD_TRAP;

    if (pL3HostEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        fdbEntryType = XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
    }

    cpss_status = cpssHalBrgFdbMacEntryIndexFind(devId, fdbEntryType,
                                                 (void *) pL3HostEntry, &index, &bankIndex);

    if (cpss_status != GT_OK && cpss_status != GT_NOT_FOUND)
    {
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    cpss_status = cpssHalBrgFdbMacEntryWrite(devId, index, GT_FALSE, fdbEntryType,
                                             (void *) pL3HostEntry);

    indexList->size = 1;
    indexList->index[0] = index;

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

#if TOD_FDB

/*
 * Need to check, why XPS is using egressIntId for the reasonCode
 */
#endif
XP_STATUS xpsL3GetIpHostControlEntryReasonCode(xpsDevice_t devId,
                                               xpsL3HostEntry_t *pL3HostEntry, uint32_t *reasonCode)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_LOCK(xpsL3GetIpHostControlEntryReasonCode);

    XP_STATUS status = XP_NO_ERR;

    if (!reasonCode)
    {
        return XP_ERR_NULL_POINTER;
    }

    if ((status = xpsL3GetIpHostEntry(devId, pL3HostEntry)) != XP_NO_ERR)
    {
        return status;
    }

    *reasonCode = pL3HostEntry->nhEntry.nextHop.egressIntfId;
    return status;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*********** API for L3  Route Management *////////////////////////////////////

XP_STATUS xpsL3InitIpRoute(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3InitIpRouteScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;


    status = xpsArpInitScope(scopeId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: failded to initialize Arp scope on device ");
        return status;
    }

    /* Initialize NH Allocator*/
    if ((status = xpsInitNhAllocator((xpsDevice_t)scopeId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: failded to initialize NH Allocator on device ");
        return status;
    }
    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsL3DeInitIpRoute(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3DeInitIpRouteScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3AddIpRouteDevice(xpsDevice_t devId, xpsInitType_t initType)
{


    XP_STATUS status = XP_NO_ERR;
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    status  = xpsIpRouteInit(devId);

    if (status)
    {
        return status;
    }

    return status;
}

XP_STATUS xpsL3RemoveIpRouteDevice(xpsDevice_t devId)
{


    XP_STATUS status = XP_NO_ERR;
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    status = xpsIpRouteDeinit(devId);

    if (status)
    {

        return status;
    }

    return XP_NO_ERR;
}


// API to Add Route Entry by subTrieMode
XP_STATUS xpsL3AddIpRouteEntryBySubTrieMode(xpsDevice_t devId,
                                            xpsL3RouteEntry_t *pL3RouteEntry, uint32_t *prfxBucketIdx,
                                            xpLpmSubTrieMode subTrieMode)
{
    XPS_FUNC_ENTRY_LOG();
    return XP_NO_ERR;
}

// API to Remove Route Entry By Index
XP_STATUS xpsL3RemoveIpRouteEntryByIndex(xpsDevice_t devId,
                                         uint32_t prfxBucketIdx, xpsL3RouteEntry_t *pL3RouteEntry)
{
    XPS_FUNC_ENTRY_LOG();

    return XP_NO_ERR;

}

XP_STATUS xpsL3UpdateIpRouteEntryByIndex(xpsDevice_t devId,
                                         uint32_t prfxBucketIdx, xpsL3RouteEntry_t *pL3RouteEntry)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsL3FindIpRouteLpmEntry(xpsDevice_t devId,
                                   xpsL3RouteEntry_t *pL3RouteEntry, uint32_t *prfxBucketIdx)
{
    XPS_FUNC_ENTRY_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3FlushIpRouteEntries(xpDevice_t devId,
                                   xpsL3RouteEntry_t *pL3RouteEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

//Gets the number of Ip route entries
XP_STATUS xpsL3GetNumOfValidIpRouteEntries(xpsDevice_t devId,
                                           xpIpPrefixType_t type, uint32_t *numOfValidEntries)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetNumRehashLevel(xpsDevice_t devId,
                                 xpsIpPrefixType_e prefixType, uint32_t numRehashLevel)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetNumRehashLevel(xpsDevice_t devId,
                                 xpsIpPrefixType_e prefixType, uint32_t *numRehashLevel)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

extern void cpssHalDumputilsFdbDump(uint8_t);
XP_STATUS xpsDisplayRouteTable(xpDevice_t devId, xpIpPrefixType_t type,
                               uint32_t endIndex)
{
    XPS_FUNC_ENTRY_LOG();
    cpssHalDumputilsFdbDump(devId);
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/////********************************L3 Route Next Hop Management Start ********************************/////

XP_STATUS xpsL3InitRouteNextHop(void)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsL3InitRouteNextHopScope(XP_SCOPE_DEFAULT);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3InitRouteNextHopScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    uint32_t defNhId = 0;
    xpsL3NextHopEntry_t defaultNh;
    uint32_t tblSize = 0;
    GT_STATUS rc = GT_OK;
    xpsDevice_t devId = scopeId;

    rc = cpssDxChCfgTableNumEntriesGet(0, CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E,
                                       &tblSize);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to Max NH Entries Error :%d \n", rc);
        return XP_ERR_INIT;
    }

    /* initialize the Router Next Hop Id Allocator*/
    status = xpsAllocatorInitIdAllocator(scopeId, XPS_ALLOCATOR_L3_NEXT_HOP_TBL,
                                         tblSize,
                                         XPS_l3_NEXTHOP_RANGE_START);

    status = xpsAllocatorInitIdAllocator(scopeId, XPS_ALLOCATOR_L3_NEIGH_TBL,
                                         XP_ROUTER_NEIGHBOUR_ENTRY_MAX(devId),
                                         XPS_l3_NEXTHOP_RANGE_START);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to initialize XPS NextHop Id allocator\n");
        return status;
    }
    status = xpsL3InitNhIdMap();
    if (status != XP_NO_ERR)
    {
        return status;
    }

    // Create the default NH and and set the pktCmd to trap
    status = xpsL3CreateRouteNextHop(1, &defNhId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to create default NH\n");
        return status;
    }

    // Set the default NH parameters
    memset(&defaultNh, 0, sizeof(defaultNh));
    defaultNh.pktCmd = XP_PKTCMD_DROP;

    //TODO the default egress interface was 0. This caused issue because port 0 is not mappted to a device.
    //To unblock,making the egress intf to a port which is in the port range of device 0. Need to
    // fix it properly
    defaultNh.nextHop.egressIntfId = 17;

    // Program default NH
    status = xpsL3SetRouteNextHopDb(0, defNhId, &defaultNh);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set default NH DB\n");
        return status;
    }
    status = xpsL3SetRouteNextHop(0, defNhId, &defaultNh);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to set default NH\n");
        return status;
    }

    //Set Packet Command to DROP as default behvior to drop the packet exceeding MTU
    status = xpsL3SetMtuPktCmdForInterface(0, 0, XP_PKTCMD_DROP);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set default MTU Pkt command\n");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsL3DeInitRouteNextHop(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3DeInitRouteNextHopScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3AddRouteNextHopDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3RemoveRouteNextHopDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsL3CreateRouteNextHop(uint32_t nhEcmpSize, uint32_t *nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsL3CreateRouteNextHop);

    return xpsL3CreateRouteNextHopScope(XP_SCOPE_DEFAULT, nhEcmpSize, nhId);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsL3CreateRouteNhHwTable(xpsScope_t scopeId, uint32_t nhEcmpSize,
                                    uint32_t *nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;
    uint32_t maxNHEntries = 0;
    uint32_t nhIdBase = 0;
    uint32_t nhIdAllocated = 0;

    /* allocate NextHop Id. Ecmp is not supported.*/
    status = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_L3_NEXT_HOP_TBL, nhId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAllocatorAllocateId Failed : %d\n", status);
        return status;
    }

    /* (1) NextHop table limit is 24576. The Ids in the table are shared
           between individual NH and NHGroup members.
       (2) We have removed the old design of fixed partitioning of this memory.
       (3) New approach will allow user to configure all 24K regular NH or
           384 NH-group with 64 NH-members.
       (4) For individual NH, ID allocation starts from 0 to 24K-1.
       (5) For NH Group members, starts from 24K-(i*64)-64 to 0, in offset of 64, where i is NH  Group Id [0...383]
       (6) Whenever individual NH is created, we allocate the id and see if it is reserved
           by NH-Group, if we crossed the tbl limit. The same we do when we allocate
           NH-group.
       */

    // Get NH table depth
    cpssStatus = cpssDxChCfgTableNumEntriesGet(0,
                                               CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E, &maxNHEntries);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get NH table depth Status %d\n", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    nhIdBase = (*nhId - ((*nhId)%nhGrpMaxSize));
    nhIdAllocated = (maxNHEntries - (nhIdBase+nhGrpMaxSize))/nhGrpMaxSize;

    /* Check if the allocated ID falls in NH Group Ids. If so, TABLE FULL.
     * Return Error.
     */
    status = xpsAllocatorAllocateWithId(scopeId, XPS_ALLOCATOR_L3_NEXT_HOP_GRP,
                                        nhIdAllocated);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "NHId %d already reserved by NH-GroupBase : %d Status :%d \n", *nhId,
              nhIdAllocated, status);
        return status;
    }
    else
    {
        status = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_L3_NEXT_HOP_GRP,
                                       nhIdAllocated);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to release ECMP table index. Status : %d\n", status);
            return status;
        }
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "NH ID :%d Base %d CheckId :%d ",
          *nhId, nhIdBase, nhIdAllocated);

    XPS_FUNC_EXIT_LOG();

    return status;
}
XP_STATUS xpsL3CreateRouteNextHopScope(xpsScope_t scopeId, uint32_t nhEcmpSize,
                                       uint32_t *nhId)
{
    XP_STATUS status = XP_NO_ERR;
    status = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_L3_NEIGH_TBL, nhId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAllocatorAllocateId Failed : %d\n", status);
        return status;
    }
    return XP_NO_ERR;
}

XP_STATUS xpsL3CreateRouteNextHopWithId(uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3CreateRouteNextHopWithIdScope(xpsScope_t scopeId, uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetRouteNextHopAndNextHopGroup(xpsDevice_t devId, uint32_t nhId,
                                              xpsL3NextHopEntry_t *pL3NextHopEntry)
{
    XPS_FUNC_ENTRY_LOG();
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC cpssNhEntry;
    XP_STATUS                       xpsStatus = XP_NO_ERR;

    memset(&cpssNhEntry, 0, sizeof(cpssNhEntry));

    /* Check if the device is valid */
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    xpsStatus = xpsL3SetRouteNextHopDb(devId, nhId, pL3NextHopEntry);
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to create db nhid %u ret %d\n", nhId, xpsStatus);
        return xpsStatus;
    }
#if 0
    /* Convert the XPS Nexthop data to the CPSS format */
    xpsStatus = xpsL3ConvertXpsNhEntryToCpssNhEntry(devId, pL3NextHopEntry,
                                                    &cpssNhEntry);
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to convert XPS NH entry to CPSS entry for NH: %d", nhId);
        return xpsStatus;
    }

    /* Set the flag to support Application Specific CPU CODE Assignment */
    cpssNhEntry.entry.regularEntry.appSpecificCpuCodeEnable = GT_TRUE;

    /* Create the NH entry using CPSS */
    cpssStatus = cpssHalWriteIpUcRouteEntries(devId, nhId, &cpssNhEntry, 1);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to add route nexthop entry for NH : %d", nhId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
#endif

    /* Set the nexthop to the nexthop groups which have this NH */
    xpsStatus = xpsL3SetRouteNextHopNextHopGroup(devId, nhId, pL3NextHopEntry);
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set NH to NH groups for NH: %d", nhId);
        return xpsStatus;
    }

    return xpsStatus;
}

XP_STATUS xpsL3ConvertXpsNhEntryToCpssNhEntry(xpsDevice_t devId,
                                              xpsL3NextHopEntry_t *xpsNhEntry, CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *cpssNhEntry)
{
    uint32_t arpPointer;
    XP_STATUS xpsStatus = XP_NO_ERR;
    xpsInterfaceType_e l3IntfType;
    xpsInterfaceType_e egressIntfType;
    xpsScope_t scopeId;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    xpsPort_t portId;
    xpsDevice_t portDevId;
    xpsInterfaceInfo_t *intfInfo;
    xpsInterfaceInfo_t *l3IntfInfo;
    xpsVlan_t vlanId;
    xpsInterfaceId_t l3IntfId = 0;
    xpsVniDbEntry_t             *vniCtx = NULL;
    uint32_t                    vniId     = 0;

    if (xpsNhEntry->pktCmd == XP_PKTCMD_DROP ||
        xpsNhEntry->pktCmd == XP_PKTCMD_TRAP)
    {
        // Drop or trap NH doesnt require any egress NH parameters
        // TODO: Derive cpuCode from reason code (for Trap)
        cpssNhEntry->entry.regularEntry.cmd  = xpsConvertPktCmdToCpssPktCmd(
                                                   xpsNhEntry->pktCmd);
        return XP_NO_ERR;
    }

    /* Get the scope id for the device */
    xpsStatus = xpsScopeGetScopeId(devId, &scopeId);
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get scope id for device %d\n", devId);
        return xpsStatus;
    }

    /* Fetch the arp pointer corresponding NH Dmac */
    //xpsStatus = xpsArpGetArpPointer(devId, xpsNhEntry->nextHop.macDa, &arpPointer);
    //if (xpsStatus != XP_NO_ERR)
    //{
    xpsStatus = xpsArpCreate(devId, xpsNhEntry->nextHop.macDa, ARP_NEXTHOP_ACTIVE,
                             &arpPointer);
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to create ARP for NH\n");
        return xpsStatus;
    }
    //}

    /* Get the type of the router interface */
    xpsStatus = xpsInterfaceGetInfo(xpsNhEntry->nextHop.l3InterfaceId, &intfInfo);
    //Check if router interface is valid and not NULL
    if (xpsStatus != XP_NO_ERR || intfInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface type for L3 interface %d\n",
              xpsNhEntry->nextHop.l3InterfaceId);
        return xpsStatus;
    }

    l3IntfType = intfInfo->type;

    //Set The MTU Profile Index;
    cpssNhEntry->entry.regularEntry.mtuProfileIndex = intfInfo->mtuPorfileIndex;
    /* Get the type of the egress interface */
    xpsStatus = xpsInterfaceGetType(xpsNhEntry->nextHop.egressIntfId,
                                    &egressIntfType);
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface type for egress interface %d\n",
              xpsNhEntry->nextHop.egressIntfId);
        return xpsStatus;
    }

    /* Populate CPSS parameters according to the egress L3 interface type */
    switch (l3IntfType)
    {
        case XPS_PORT_ROUTER:
        case XPS_SUBINTERFACE_ROUTER:
        case XPS_VLAN_ROUTER:
            {
                if (egressIntfType == XPS_PORT)
                {
                    cpssNhEntry->entry.regularEntry.nextHopInterface.type = CPSS_INTERFACE_PORT_E;
                    // Get the port id and device id from the egress port interface
                    xpsStatus = xpsPortGetDevAndPortNumFromIntf(xpsNhEntry->nextHop.egressIntfId,
                                                                &portDevId, &portId);
                    if (xpsStatus != XP_NO_ERR)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Failed to get physical port id and device from the egress interface id %d. Status : %d\n",
                              xpsNhEntry->nextHop.egressIntfId, xpsStatus);
                        return xpsStatus;
                    }

                    cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
                    cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

                    // TODO: Change this to destination dev id when B2B support is done
                    cpssNhEntry->entry.regularEntry.nextHopInterface.devPort.hwDevNum =
                        (GT_HW_DEV_NUM)cpssDevId;
                    // Convert egressIntfId to cpss port num
                    cpssNhEntry->entry.regularEntry.nextHopInterface.devPort.portNum =
                        (GT_PORT_NUM)cpssPortNum;

                }
                else if (egressIntfType == XPS_LAG)
                {
                    cpssNhEntry->entry.regularEntry.nextHopInterface.type = CPSS_INTERFACE_TRUNK_E;
                    cpssNhEntry->entry.regularEntry.nextHopInterface.trunkId =
                        (GT_TRUNK_ID)xpsUtilXpstoCpssInterfaceConvert(xpsNhEntry->nextHop.egressIntfId,
                                                                      egressIntfType);
                }

                else if (egressIntfType == XPS_VLAN_ROUTER)
                {
                    vlanId = XPS_INTF_MAP_INTF_TO_BD(xpsNhEntry->nextHop.l3InterfaceId);
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Vlan Id : %d\n", vlanId);
                    cpssNhEntry->entry.regularEntry.nextHopInterface.vlanId = vlanId;
                    cpssNhEntry->entry.regularEntry.nextHopInterface.type = CPSS_INTERFACE_VID_E;

                }

                else
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Invalid egress interface type %d : Egress interface : %d\n", egressIntfType,
                          xpsNhEntry->nextHop.egressIntfId);
                    return XP_ERR_INVALID_ARG;
                }

                break;
            }
        case XPS_TUNNEL_VXLAN:
            {
                xpsIpTnlGblDbEntry_t *lookupEntry = NULL;
                xpsStatus = xpsIpTunnelGblGetDbEntry(scopeId, xpsNhEntry->nextHop.l3InterfaceId,
                                                     &lookupEntry);
                if (xpsStatus != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Vxlan Tunnel Not found :%d \n", xpsNhEntry->nextHop.l3InterfaceId);
                    return xpsStatus;
                }
                cpssNhEntry->entry.regularEntry.nextHopInterface.type = CPSS_INTERFACE_PORT_E;
                cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);
                cpssNhEntry->entry.regularEntry.nextHopInterface.devPort.hwDevNum =
                    (GT_HW_DEV_NUM)devId;
                cpssNhEntry->entry.regularEntry.nextHopInterface.devPort.portNum =
                    (GT_PORT_NUM)lookupEntry->primaryEport;

                /* Get VNI DB entry */
                vniId = xpsNhEntry->serviceInstId;
                xpsStatus = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
                if (xpsStatus != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Error in xpsVniGetDbEntry an entry: error code: %d\n",
                          xpsStatus);
                }
                else
                {
                    cpssNhEntry->entry.regularEntry.nextHopVlanId = vniCtx->hwEVlanId;
                }
                break;
            }
        // Other L3 interface types...

        default:
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Unsupported L3 interface type received : %d L3 Interface Id : %d\n",
                      l3IntfType,
                      xpsNhEntry->nextHop.l3InterfaceId);
                return XP_ERR_INVALID_ARG;
            }

    }
    cpssNhEntry->type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;
    cpssNhEntry->entry.regularEntry.cmd  = xpsConvertPktCmdToCpssPktCmd(
                                               xpsNhEntry->pktCmd);

    // In L3 route context, CPSS_PACKET_CMD_FORWARD_E is invalid. Changing it to right one
    if (cpssNhEntry->entry.regularEntry.cmd == CPSS_PACKET_CMD_FORWARD_E)
    {
        cpssNhEntry->entry.regularEntry.cmd = CPSS_PACKET_CMD_ROUTE_E;
    }

    cpssNhEntry->entry.regularEntry.isTunnelStart = GT_FALSE;
    if (l3IntfType == XPS_VLAN_ROUTER)
    {
        vlanId = XPS_INTF_MAP_INTF_TO_BD(xpsNhEntry->nextHop.l3InterfaceId);
        cpssNhEntry->entry.regularEntry.nextHopVlanId = vlanId;
    }
    else
    {
        if ((egressIntfType == XPS_PORT || egressIntfType == XPS_LAG) &&
            l3IntfType != XPS_TUNNEL_VXLAN)
        {
            xpsStatus = xpsL3GetBindPortIntf(xpsNhEntry->nextHop.egressIntfId, &l3IntfId);
            if (xpsStatus != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "xpsL3GetBindPortIntf: %d\n", xpsStatus);
                return xpsStatus;
            }

            xpsStatus = xpsInterfaceGetInfo(l3IntfId, &l3IntfInfo);
            if ((xpsStatus != XP_NO_ERR) || (l3IntfInfo == NULL))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get interface information for L3 Interface id : %d\n", l3IntfId);
                return xpsStatus;
            }

            cpssNhEntry->entry.regularEntry.nextHopVlanId = l3IntfInfo->bdHWId;
        }
    }
    cpssNhEntry->entry.regularEntry.nextHopARPPointer = arpPointer;
    cpssNhEntry->entry.regularEntry.isNat = GT_FALSE;
    cpssNhEntry->entry.regularEntry.ttlHopLimitDecEnable = GT_TRUE;

    return XP_NO_ERR;
}

XP_STATUS xpsL3ConvertCpssNhEntryToXpsNhEntry(xpsDevice_t devId,
                                              CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *cpssNhEntry, xpsL3NextHopEntry_t *xpsNhEntry)
{
    GT_STATUS cpssStatus = GT_OK;
    GT_ETHERADDR arpDmac;
    xpsInterfaceId_t l3IntfId;
    XP_STATUS status = XP_NO_ERR;
    xpsScope_t scopeId;
    GT_U32 globalPortNum;

    memset(xpsNhEntry, 0, sizeof(xpsL3NextHopEntry_t));

    // Read the Arp entry from HW
    cpssStatus = cpssDxChIpRouterArpAddrRead(devId,
                                             cpssNhEntry->entry.regularEntry.nextHopARPPointer, &arpDmac);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to read ARP entry from the ARP table index : %d\n",
              cpssNhEntry->entry.regularEntry.nextHopARPPointer);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    status = xpsScopeGetScopeId(devId, &scopeId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get scope for devId : %d\n", devId);
        return status;
    }

    if (cpssNhEntry->entry.regularEntry.nextHopInterface.type ==
        CPSS_INTERFACE_TRUNK_E)
    {
        xpsNhEntry->nextHop.egressIntfId = xpsUtilCpssToXpsInterfaceConvert(
                                               cpssNhEntry->entry.regularEntry.nextHopInterface.trunkId, XPS_LAG);
    }
    else
    {
        globalPortNum = xpsLocalPortToGlobalPortnum(
                            cpssNhEntry->entry.regularEntry.nextHopInterface.devPort.hwDevNum,
                            cpssNhEntry->entry.regularEntry.nextHopInterface.devPort.portNum);

        // Fetch the global port id from the local port id and dev id
        status = xpsPortGetPortIntfId(devId, globalPortNum,
                                      &xpsNhEntry->nextHop.egressIntfId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port interface id from devid : %d global port id : %d status : %d\n",
                  0, globalPortNum, status);
            return status;
        }
    }

    if (cpssNhEntry->entry.regularEntry.nextHopVlanId < XPS_L3_RESERVED_ROUTER_VLAN(
            devId))
    {
        // Vlan router interface case
        // Convert the Vlan router's Vlan to the L3 interface id
        l3IntfId = XPS_INTF_MAP_BD_TO_INTF(
                       cpssNhEntry->entry.regularEntry.nextHopVlanId);
    }
    else
    {
        // Port router interface case
        // Fetch the L3 interface corresponding to port interface id
        status = xpsL3RetrieveInterfaceMapping(scopeId,
                                               xpsNhEntry->nextHop.egressIntfId, &l3IntfId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get mapped L3 interface with port inteface %d\n",
                  xpsNhEntry->nextHop.egressIntfId);
            return status;
        }
    }

    xpsNhEntry->serviceInstId = cpssNhEntry->entry.regularEntry.nextHopVlanId;
    xpsNhEntry->pktCmd = xpsConvertPktCmdToXpsPktCmd(
                             cpssNhEntry->entry.regularEntry.cmd);
    // TODO: Take care B2B case
    xpsNhEntry->nextHop.l3InterfaceId = l3IntfId;
    memcpy(&xpsNhEntry->nextHop.macDa, &arpDmac.arEther, sizeof(macAddr_t));

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetRouteNextHopDb(xpsDevice_t devId, uint32_t nhId,
                                 xpsL3NextHopEntry_t *pL3NextHopEntry)
{
    xpsL3NhSwDbEntry_t* pXpsNhSwDbInfo = NULL;
    xpsL3NhSwDbEntry_t tempXpsNhSwDbInfo = {0};
    XP_STATUS status = XP_NO_ERR;

    tempXpsNhSwDbInfo.swNhId = nhId;
    if ((status = xpsStateSearchData(XP_SCOPE_DEFAULT, nhSwDbHndl,
                                     (xpsDbKey_t)&tempXpsNhSwDbInfo,
                                     (void**)&pXpsNhSwDbInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "eror getting sw instance for NH id %u ret %d", nhId, status);
        return status;
    }
    if (pXpsNhSwDbInfo)
    {
        if (memcmp(&pXpsNhSwDbInfo->nhEntry, pL3NextHopEntry,
                   sizeof(xpsL3NextHopEntry_t)) != 0)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                  "update nhId %u [ hwId %d]", nhId, pXpsNhSwDbInfo->hwNhId);
            memcpy(&pXpsNhSwDbInfo->nhEntry,
                   pL3NextHopEntry, sizeof(xpsL3NextHopEntry_t));
            if (pXpsNhSwDbInfo->hwNhId != INVALID_L3_INDEX)
            {
                xpsL3RemoveRouteNhHw(devId, pXpsNhSwDbInfo->hwNhId);
                xpsL3SetRouteNextHopHw(devId, pXpsNhSwDbInfo->hwNhId, pL3NextHopEntry);
            }
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                  "nothing to update nhId %u", nhId);
        }
        return XP_NO_ERR;
    }

    if ((status = xpsStateHeapMalloc(sizeof(xpsL3NhSwDbEntry_t),
                                     (void**)&pXpsNhSwDbInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xps NH db alloc failed");
        return status;
    }
    if (!pXpsNhSwDbInfo)
    {
        return XP_ERR_NULL_POINTER;
    }
    memset(pXpsNhSwDbInfo, 0, sizeof(xpsL3NhSwDbEntry_t));
    pXpsNhSwDbInfo->swNhId = nhId;
    pXpsNhSwDbInfo->hwNhId = INVALID_L3_INDEX;
    pXpsNhSwDbInfo->hwPbrLeafId = XPS_INTF_INVALID_ID;
    memcpy(&pXpsNhSwDbInfo->nhEntry,
           pL3NextHopEntry, sizeof(xpsL3NextHopEntry_t));

    if ((status = xpsStateInsertData(XP_SCOPE_DEFAULT, nhSwDbHndl,
                                     (void*)pXpsNhSwDbInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Insertion of xps NH data failed");
        // Free Allocated memory
        xpsStateHeapFree((void*)pXpsNhSwDbInfo);
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "Inserted new DB nhid %d", nhId);

    return status;
}

XP_STATUS xpsL3GetRouteNextHopSwDb(xpsScope_t scopeId, uint32_t nhId,
                                   xpsL3NhSwDbEntry_t **pxpsNhSwDbInfo)
{
    xpsL3NhSwDbEntry_t tempXpsNhSwDbInfo = {0};
    XP_STATUS status = XP_NO_ERR;

    tempXpsNhSwDbInfo.swNhId = nhId;
    if ((status = xpsStateSearchData(scopeId, nhSwDbHndl,
                                     (xpsDbKey_t)&tempXpsNhSwDbInfo,
                                     (void**)pxpsNhSwDbInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "eror getting sw instance for NH id %u ret %d", nhId, status);
        return status;
    }
    if (NULL == *pxpsNhSwDbInfo)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "sw instance for NH id %u not found", nhId);
        return XP_ERR_KEY_NOT_FOUND;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetRouteNextHopDb(xpsDevice_t devId, uint32_t nhId,
                                 xpsL3NextHopEntry_t *pL3NextHopEntry)
{
    xpsL3NhSwDbEntry_t* pxpsNhSwDbInfo = NULL;
    xpsL3NhSwDbEntry_t tempXpsNhSwDbInfo = {0};
    XP_STATUS status = XP_NO_ERR;
    if (pL3NextHopEntry ==  NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsL3GetRouteNextHopDb received NULL");
        return XP_ERR_NULL_POINTER;

    }
    tempXpsNhSwDbInfo.swNhId = nhId;
    if ((status = xpsStateSearchData(XP_SCOPE_DEFAULT, nhSwDbHndl,
                                     (xpsDbKey_t)&tempXpsNhSwDbInfo,
                                     (void**)&pxpsNhSwDbInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "eror getting sw instance for NH id %u ret %d", nhId, status);
        return status;
    }
    if (NULL == pxpsNhSwDbInfo)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "sw instance for NH id %u not found", nhId);
        return XP_ERR_KEY_NOT_FOUND;
    }
    /*copy only xpsL3NextHopEntry from db */
    memcpy(pL3NextHopEntry, &pxpsNhSwDbInfo->nhEntry, sizeof(xpsL3NextHopEntry_t));
    return XP_NO_ERR;
}
XP_STATUS xpsL3RemoveRouteNextHopDb(xpsDevice_t devId, uint32_t nhId)
{
    xpsL3NhSwDbEntry_t* pxpsNhSwDbInfo = NULL;
    xpsL3NhSwDbEntry_t tempXpsNhSwDbInfo = {0};
    XP_STATUS status = XP_NO_ERR;

    tempXpsNhSwDbInfo.swNhId = nhId;
    /*TODO if a hw map entry exists?*/
    if ((status = xpsStateDeleteData(XP_SCOPE_DEFAULT, nhSwDbHndl,
                                     (xpsDbKey_t)&tempXpsNhSwDbInfo,
                                     (void**)&pxpsNhSwDbInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "eror getting sw instance for NH id %u ret %d", nhId, status);
        return status;
    }
    if (NULL == pxpsNhSwDbInfo)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "sw instance for NH id %u not found", nhId);
        return XP_ERR_KEY_NOT_FOUND;
    }

    xpsStateHeapFree((void*)pxpsNhSwDbInfo);
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "sw db destroyed for NH id %u ", nhId);
    return XP_NO_ERR;
}

/*before invoking this api, use xpsL3NextHopMapGet */
XP_STATUS xpsL3NextHopMapInsert(uint32_t swNhId, uint32_t hwNhId)
{
    xpsL3NhSwDbEntry_t* pxpsNhSwDbInfo = NULL;
    xpsL3NhSwDbEntry_t tempXpsNhSwDbInfo = {0};
    XP_STATUS status = XP_NO_ERR;

    tempXpsNhSwDbInfo.swNhId = swNhId;
    if ((status = xpsStateSearchData(XP_SCOPE_DEFAULT, nhSwDbHndl,
                                     (xpsDbKey_t)&tempXpsNhSwDbInfo,
                                     (void**)&pxpsNhSwDbInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "eror getting sw instance for NH id %u ret %d", swNhId, status);
        return status;
    }
    if (NULL == pxpsNhSwDbInfo)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "sw instance for NH id %u not found", swNhId);
        return XP_ERR_KEY_NOT_FOUND;
    }
    if (pxpsNhSwDbInfo->hwNhId == INVALID_L3_INDEX)
    {
        pxpsNhSwDbInfo->hwNhId = hwNhId ;
        pxpsNhSwDbInfo->hwRefCnt++;
        status = xpsL3SetHwId2SwId(hwNhId, swNhId);
        if (status != XP_NO_ERR)
        {
            return status;
        }
        /*create NH in hw*/
        xpsL3SetRouteNextHopHw(0, hwNhId, &pxpsNhSwDbInfo->nhEntry);
    }
    else if (pxpsNhSwDbInfo->hwNhId == hwNhId)
    {
        pxpsNhSwDbInfo->hwRefCnt++;
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "swNhId %u already mapped to hwId %d [Cnt %d] can't map new hwId %d",
              swNhId, pxpsNhSwDbInfo->hwNhId, pxpsNhSwDbInfo->hwRefCnt, hwNhId);
        return XP_ERR_KEY_EXISTS;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "map NH id %u to hwId %d refCnt %d", swNhId, hwNhId, pxpsNhSwDbInfo->hwRefCnt);
    return XP_NO_ERR;
}
XP_STATUS xpsL3NextHopMapRemove(uint32_t swNhId, uint32_t hwNhId)
{
    xpsL3NhSwDbEntry_t* pxpsNhSwDbInfo = NULL;
    xpsL3NhSwDbEntry_t tempXpsNhSwDbInfo = {0};
    XP_STATUS status = XP_NO_ERR;

    tempXpsNhSwDbInfo.swNhId = swNhId;
    if ((status = xpsStateSearchData(XP_SCOPE_DEFAULT, nhSwDbHndl,
                                     (xpsDbKey_t)&tempXpsNhSwDbInfo,
                                     (void**)&pxpsNhSwDbInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "eror getting sw instance for NH id %u ret %d", swNhId, status);
        return status;
    }
    if (NULL == pxpsNhSwDbInfo)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "sw instance for NH id %u not found", swNhId);
        return XP_ERR_KEY_NOT_FOUND;
    }
    if (pxpsNhSwDbInfo->hwNhId == INVALID_L3_INDEX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "no Hw entry exists for nhId %u", swNhId);
        return XP_ERR_KEY_NOT_FOUND;
    }
    pxpsNhSwDbInfo->hwRefCnt--;
    if ((pxpsNhSwDbInfo->hwRefCnt == 0))
    {
        pxpsNhSwDbInfo->hwNhId = INVALID_L3_INDEX ;
        /*if no route use the NH, remove from HW*/
        status =  xpsL3RemoveRouteNhHw(0, hwNhId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "error removing hw nhid %u ret %d", hwNhId, status);
            return status;
        }
        status = xpsL3DelHwId2SwId(hwNhId);
        status = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XPS_ALLOCATOR_L3_NEXT_HOP_TBL,
                                       hwNhId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to release NH table index %d. ret %d", hwNhId, status);
            return status;
        }
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "unmap nhId %u to hwId %d refCount %u ", swNhId, hwNhId,
          pxpsNhSwDbInfo->hwRefCnt);
    return XP_NO_ERR;
}
XP_STATUS xpsL3NextHopMapGet(uint32_t swNhId, uint32_t* hwNhId)
{
    xpsL3NhSwDbEntry_t* pxpsNhSwDbInfo = NULL;
    xpsL3NhSwDbEntry_t tempXpsNhSwDbInfo = {0};
    XP_STATUS status = XP_NO_ERR;

    tempXpsNhSwDbInfo.swNhId = swNhId;
    if ((status = xpsStateSearchData(XP_SCOPE_DEFAULT, nhSwDbHndl,
                                     (xpsDbKey_t)&tempXpsNhSwDbInfo,
                                     (void**)&pxpsNhSwDbInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "eror getting sw instance for NH id %u ret %d", swNhId, status);
        return status;
    }
    if (NULL == pxpsNhSwDbInfo)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "sw instance for NH id %u not found", swNhId);
        return XP_ERR_KEY_NOT_FOUND;
    }
    *hwNhId = pxpsNhSwDbInfo->hwNhId;
    return XP_NO_ERR;
}

XP_STATUS xpsL3SetRouteNextHop(xpsDevice_t devId, uint32_t nhId,
                               xpsL3NextHopEntry_t *pL3NextHopEntry)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS                       xpsStatus = XP_NO_ERR;
    uint32_t nhHwId;

    /* Check if the device is valid */
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    /*get HWID from SW NHID*/
    xpsStatus = xpsL3NextHopMapGet(nhId, &nhHwId);
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "get NH map failed");
        return xpsStatus;
    }

    if (nhHwId  == INVALID_L3_INDEX)
    {
        /*get the hw id for new entry to be written*/
        xpsStatus = xpsL3CreateRouteNhHwTable((xpsScope_t)devId, 1, &nhHwId);
        if (xpsStatus != XP_NO_ERR)
        {

            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsL3CreateRouteNhHwTable failed ret %d", xpsStatus);
            return xpsStatus;
        }

        //return xpsL3NextHopMapInsert(nhId, nhHwId);
    }
    else
    {
        /*if the map exists, indicates already HW id is programmed.*/
        /*update the ref count*/
        //return xpsL3NextHopMapInsert(nhId, nhHwId);
    }
    return xpsL3NextHopMapInsert(nhId, nhHwId);

    //return xpsL3SetRouteNextHopHw(devId, nhHwId, pL3NextHopEntry);
}
XP_STATUS xpsL3SetRouteNextHopHw(xpsDevice_t devId, uint32_t hwNhId,
                                 xpsL3NextHopEntry_t *pL3NextHopEntry)
{
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC cpssNhEntry;
    XP_STATUS                       xpsStatus = XP_NO_ERR;
    GT_STATUS                       cpssStatus = GT_OK;
    memset(&cpssNhEntry, 0, sizeof(cpssNhEntry));

    /* Convert the XPS Nexthop data to the CPSS format */
    xpsStatus = xpsL3ConvertXpsNhEntryToCpssNhEntry(devId, pL3NextHopEntry,
                                                    &cpssNhEntry);
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to convert XPS NH entry to CPSS entry for NH: %d", hwNhId);
        return xpsStatus;
    }

    /* Set the flag to support Application Specific CPU CODE Assignment */
    cpssNhEntry.entry.regularEntry.appSpecificCpuCodeEnable = GT_TRUE;

    /* Create the NH entry using CPSS */
    cpssStatus = cpssHalWriteIpUcRouteEntries(devId, hwNhId, &cpssNhEntry, 1);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to add route nexthop entry for NH : %d", hwNhId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsL3SetRouteNextHopHw hwId %u updated", hwNhId);
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetRouteNextHop(xpsDevice_t devId, uint32_t nhId,
                               xpsL3NextHopEntry_t *pL3NextHopEntry)
{
    return xpsL3GetRouteNextHopDb(devId, nhId, pL3NextHopEntry);
}
XP_STATUS xpsL3DestroyRouteNextHop(uint32_t nhEcmpSize, uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsL3DestroyRouteNextHop);

    return  xpsL3DestroyRouteNextHopScope(XP_SCOPE_DEFAULT, nhEcmpSize, nhId);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3DestroyRouteNextHopScope(xpsScope_t scopeId, uint32_t nhEcmpSize,
                                        uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    /* Release NextHop Id. Ecmp is not supported*/
    status = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_L3_NEIGH_TBL, nhId);

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsL3CreateNextHopGroupScope(xpsScope_t scopeId, uint32_t nhEcmpSize,
                                       uint32_t *nhGrpId)
{
    XP_STATUS status = XP_NO_ERR;
    XPS_FUNC_ENTRY_LOG();

    status = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_L3_NEXT_HOP_GRP,
                                    nhGrpId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to allocate NH group. Status : %d\n", status);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3CreateNextHopGroup(uint32_t nhEcmpSize, uint32_t *nhGrpId)
{
    XPS_LOCK(xpsL3CreateNextHopGroup);

    return xpsL3CreateNextHopGroupScope(XP_SCOPE_DEFAULT, nhEcmpSize, nhGrpId);
}

XP_STATUS xpsL3DestroyNextHopGroupScope(xpsScope_t scopeId, uint32_t nhEcmpSize,
                                        uint32_t nhGrpId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;

    status = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_L3_NEXT_HOP_GRP, nhGrpId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to release NH grp %d. Status : %d\n", nhGrpId, status);
        return status;
    }
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3DestroyNextHopGroup(uint32_t nhEcmpSize, uint32_t nhGrpId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetHashFields(xpsDevice_t deviceId, xpHashField* fields,
                             size_t size)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3ClearRouteNextHop(xpsDevice_t devId, uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;
    uint32_t hwNhId;

    /* Check if the device is valid */
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }
    status = xpsL3NextHopMapGet(nhId, &hwNhId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get sw DB for nh %u", nhId);
        return status;
    }
    if (hwNhId != INVALID_L3_INDEX)
    {
        status = xpsL3RemoveRouteNhHw(devId, hwNhId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsL3RemoveRouteNhHw remove hwnhid %u  ret %d", hwNhId, status);
            return status;
        }
    }

    status = xpsL3RemoveRouteNextHopDb(devId, nhId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "remove nhid %u  ret %d", nhId,
              status);
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE, "removed nhid %u and hwId %u",
          nhId, hwNhId);
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3RemoveRouteNhHw(xpsDevice_t devId, uint32_t hwNhId)
{
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC cpssNhEntry;
    GT_ETHERADDR arpEntry;
    GT_STATUS cpssStatus = GT_OK;
    XP_STATUS status = XP_NO_ERR;

    memset(&cpssNhEntry, 0, sizeof(cpssNhEntry));

    /* Read the CPSS Nexthop entry from the HW */
    cpssStatus = cpssDxChIpUcRouteEntriesRead(devId, hwNhId, &cpssNhEntry, 1);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to read the NH entry from Hw : NH id : %d", hwNhId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    cpssStatus = cpssDxChIpRouterArpAddrRead(devId,
                                             cpssNhEntry.entry.regularEntry.nextHopARPPointer, &arpEntry);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get arp entry at index %d\n",
              cpssNhEntry.entry.regularEntry.nextHopARPPointer);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    /* Release the ARP pointer used by this NH entry */
    status = xpsArpDelete(devId, arpEntry.arEther, ARP_NEXTHOP_ACTIVE);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to destroy ARP\n");
        return status;
    }

    /*
     * Clear the Route Nexthop entry in HW. There is no specific API to clear the NH in cpss.
     * cpssDxChIpUcRouteEntriesWrite expects valid values. so, setting pktCmd tp drop
     * and setting to HW.
     */
    cpssNhEntry.entry.regularEntry.cmd = CPSS_PACKET_CMD_DROP_HARD_E;
    cpssNhEntry.entry.regularEntry.countSet = CPSS_IP_CNT_NO_SET_E;
    memset(&cpssNhEntry.entry.regularEntry.nextHopInterface, 0,
           sizeof(cpssNhEntry.entry.regularEntry.nextHopInterface));
    cpssNhEntry.entry.regularEntry.countSet = CPSS_IP_CNT_NO_SET_E;
    cpssNhEntry.entry.regularEntry.mtuProfileIndex = 0;
    cpssNhEntry.entry.regularEntry.nextHopVlanId = 0;

    /* Unset the flag which supports Application Specific CPU CODE Assignment */
    cpssNhEntry.entry.regularEntry.appSpecificCpuCodeEnable = GT_FALSE;

    cpssStatus = cpssHalWriteIpUcRouteEntries(devId, hwNhId, &cpssNhEntry, 1);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to clear route nexthop entry for NH : %d", hwNhId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "removed hwNhId %d", hwNhId);
    return XP_NO_ERR;
}

/*This api is used to remove the ECMP member.
The caller of this function, calls with the NH hw ID.
Upon removing the Nh entry from NH grp,
need to update reference for ARP pointer also*/
XP_STATUS xpsL3ClearEcmpRouteNextHop(xpsDevice_t devId, uint32_t nhId)
{
    XP_STATUS status = XP_NO_ERR;
    XPS_FUNC_ENTRY_LOG();

    status = xpsL3RemoveRouteNhHw(devId, nhId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to destroy hw nhId %u",
              nhId);
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE, "removed hw nhId %u", nhId);
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
/////******************************** L3 Route Next Hop Management End ********************************/////

///********************************* MPLS L3 VPN  Management API Start *******************************////
XP_STATUS xpsL3SetRouteVpnNextHop(xpsDevice_t devId, uint32_t nhId,
                                  xpsL3NextHopEntry_t *pL3NextHopEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

///********************************* MPLS L3 VPN  Management API End *******************************////
///********************************* IPV6 Mcast Routing Domain Management APi start******************///

XP_STATUS xpsL3SetV6McL3DomainIdForInterface(xpDevice_t devId,
                                             xpsInterfaceId_t intfId, xpMcastDomainId_t mcL3DomainId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsL3GetV6McL3DomainIdForInterface(xpDevice_t devId,
                                             xpsInterfaceId_t intfId, xpMcastDomainId_t *mcL3DomainId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3IPMtuProfileWrite(uint8_t devId, uint32_t hwNhIdx,
                                 uint32_t mtuProfileIndex,
                                 CPSS_INTERFACE_TYPE_ENT cpssIntfType,
                                 GT_PORT_NUM     cpssIfNum)
{
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC cpssNhEntry;
    GT_STATUS cpssStatus = GT_OK;
    int  cpssDevNum;
    GT_BOOL foundEntry = GT_FALSE;

    memset(&cpssNhEntry, 0, sizeof(cpssNhEntry));
    /* Read the CPSS Route Nexthop entrddy from HW */
    cpssStatus = cpssDxChIpUcRouteEntriesRead(devId, hwNhIdx, &cpssNhEntry, 1);

    /* skip entry if could not read or not same as requested input interface type*/
    if (cpssStatus != GT_OK ||
        cpssNhEntry.entry.regularEntry.nextHopInterface.type != cpssIntfType)
    {
        return XP_NO_ERR;
    }

    switch (cpssIntfType)
    {
        case CPSS_INTERFACE_TRUNK_E:
            {
                if (cpssNhEntry.entry.regularEntry.nextHopInterface.trunkId == cpssIfNum)
                {
                    foundEntry = GT_TRUE;
                }
                break;
            }
        case CPSS_INTERFACE_PORT_E:
            {
                if (cpssNhEntry.entry.regularEntry.nextHopInterface.devPort.portNum ==
                    cpssIfNum)
                {
                    foundEntry = GT_TRUE;
                }
                break;
            }
        case CPSS_INTERFACE_VID_E:
            {
                /* nextHopInterface.vlanId field is not updated by CPSS,
                  hence check on nextHopVlanId for SVI */
                if (cpssNhEntry.entry.regularEntry.nextHopVlanId == cpssIfNum)
                {
                    foundEntry = GT_TRUE;
                }
                break;
            }
        default:
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Faild to set MTU profile intf %u type: %d",
                      cpssIfNum, cpssIntfType);
            }
    }

    if (foundEntry == GT_TRUE)
    {
        foundEntry = GT_FALSE;  /* reset found entry to find next matched entry */
        cpssNhEntry.entry.regularEntry.mtuProfileIndex = mtuProfileIndex;
        /* Supporting B2B device to update MTU for corrosponding Nexthop in that device*/
        XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
        {
            /* Read the CPSS Route Nexthop entry from HW */
            cpssStatus = cpssDxChIpUcRouteEntriesWrite(cpssDevNum, hwNhIdx, &cpssNhEntry,
                                                       1);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to update MTU profile index for Unicast Nexthop entry at index: %d",
                      hwNhIdx);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }
    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIPMtuRouteEntries(uint8_t devId, uint32_t mtuProfileIndex,
                                    CPSS_INTERFACE_TYPE_ENT cpssIntfType,
                                    GT_PORT_NUM     cpssIfNum)
{
    XP_STATUS status = XP_NO_ERR;
    map<uint32_t, uint32_t>::iterator iter;
    xpsL3NextHopGroupEntry_t* pNhGrpEntry   = NULL;
    xpsScope_t                scopeId;

    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    if (NULL == gHwNhId2SwNhId)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Nh map not initialized");
        return XP_ERR_NULL_POINTER;
    }

    /*Iterate the programmed NHIds and update the MTU profile index.*/
    for (iter = (*gHwNhId2SwNhId).begin(); iter != (*gHwNhId2SwNhId).end(); iter++)
    {
        status = xpsL3IPMtuProfileWrite(devId, iter->first, mtuProfileIndex,
                                        cpssIntfType, cpssIfNum);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "set MTU Failed %d err %d \n ", iter->first, status);
            return XP_ERR_NULL_POINTER;
        }
    }
    /*Iterate programmed NHGroups and update its members*/
    while (xpsL3GetNextRouteNextHopGroup(scopeId, pNhGrpEntry,
                                         &pNhGrpEntry) == XP_NO_ERR)
    {
        for (uint32_t i = 0; i < pNhGrpEntry->groupSize; i++)
        {
            status = xpsL3IPMtuProfileWrite(devId, (pNhGrpEntry->baseNhId + i),
                                            mtuProfileIndex, cpssIntfType, cpssIfNum);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "set MTU Failed %d err %d \n ", (pNhGrpEntry->baseNhId + i), status);
                return XP_ERR_NULL_POINTER;
            }
        }
    }
    return status;
}

XP_STATUS xpsL3SetMtuLenForInterface(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                     uint32_t mtuLen)
{
    xpsInterfaceId_t portInterfaceId;
    xpsInterfaceType_e intfType;
    GT_U32 mtuProfileIndex;
    XP_STATUS status = XP_NO_ERR;
    xpPort_t portId;
    xpsDevice_t portDevId = 0;
    GT_PORT_NUM cpssIfNum; /* Value could be vlan Id, port-Id, or Trunk-Id*/
    CPSS_INTERFACE_TYPE_ENT cpssIntfType;
    GT_TRUNK_ID trunkId;
    xpsVlan_t vlanId;
    xpsScope_t scopeId;

    // Get scope id from device id
    status = xpsScopeGetScopeId(devId, &scopeId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get scope id for dev id: %d\n", devId);
        return status;
    }

    /* Add the MTU to L3 Interface and MTU Table, if not exists*/
    status = xpsMtuSetIL3nterfaceMtuSize(devId, intfId, mtuLen);

    if (status != XP_NO_ERR)
    {
        return status;
    }

    /* Get the type of the egress interface */
    status = xpsInterfaceGetType(intfId, &intfType);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set IP MTU on interface %d due to failure in retrieving interface type\n",
              intfId);
        return status;
    }

    if (intfType == XPS_VLAN_ROUTER) /* vlan */
    {
        // Retrieve the corresponding vlan id
        vlanId = XPS_INTF_MAP_INTF_TO_BD(intfId);

        if (vlanId > XPS_VLANID_MAX)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set IP MTU on interface %d due to invalid vlan id %d\n", intfId,
                  vlanId);
            return XP_ERR_INVALID_VLAN_ID;
        }
        cpssIntfType = CPSS_INTERFACE_VID_E;
        cpssIfNum = (GT_PORT_NUM) vlanId;
    }
    else
    {
        status = xpsL3RetrieveInterfaceMapping(scopeId, intfId, &portInterfaceId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Filed to set IP MTU on interface %d dueo to failure in retrieving port interface id for L3 interface\n",
                  intfId);
            return status;
        }

        /* Get the type of the egress interface */
        status = xpsInterfaceGetType(portInterfaceId, &intfType);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set IP MTU on interface %d due to failure in retrieving interface type\n",
                  intfId);
            return status;
        }


        if (intfType == XPS_LAG)
        {
            trunkId = xpsUtilXpstoCpssInterfaceConvert(portInterfaceId, intfType);
            cpssIntfType = CPSS_INTERFACE_TRUNK_E;
            cpssIfNum = (GT_PORT_NUM) trunkId;
        }
        else
        {

            status = xpsPortGetDevAndPortNumFromIntf(portInterfaceId, &portDevId, &portId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set IP MTU on interface %d due to failure in retrieving physical port id and device from port interface %d \n",
                      intfId, portInterfaceId);
                return status;
            }

            cpssIfNum = (GT_PORT_NUM) xpsGlobalPortToPortnum(portDevId, portId);
            cpssIntfType = CPSS_INTERFACE_PORT_E;
        }

    }

    if ((status = xpsInterfaceGetIpMtuProfileIndex(intfId,
                                                   &mtuProfileIndex)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interface IP MTU profile index on interface(%d) failed: %d", intfId,
              status);
        return status;
    }

    /* Update the MTU Profile index for all matched Unicast Next Hop entries.
       Iterate and configure the programmed NHs*/
    status =  xpsL3SetIPMtuRouteEntries(devId, mtuProfileIndex,
                                        cpssIntfType, cpssIfNum);

    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set IP MTU for UC Route Entries on interface %d type %d with status %d ",
              intfId, intfType, status);
        return status;
    }

    return status;
}

XP_STATUS xpsL3SetIpv4MtuLenForInterface(xpsDevice_t devId,
                                         xpsInterfaceId_t intfId, uint32_t ipv4MtuLen)
{
    XPS_FUNC_ENTRY_LOG();
    uint32_t  currentMtuLen = 0;
    XP_STATUS status = XP_NO_ERR;

    XPS_LOCK(xpsL3SetIpv4MtuLenForInterface);

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    status = xpsL3GetIpv4MtuLenForInterface(devId, intfId, &currentMtuLen);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get ipv4 mtu length for interface %d device %d\n", intfId, devId);
        return status;
    }

    if (currentMtuLen == ipv4MtuLen)
    {
        // In case of reassigning the same mtu, don't increment the refCount, just return success
        return XP_NO_ERR;
    }

    XPS_FUNC_EXIT_LOG();
    return xpsL3SetMtuLenForInterface(devId, intfId, ipv4MtuLen);
}

XP_STATUS xpsL3GetIpv4MtuLenForInterface(xpsDevice_t devId,
                                         xpsInterfaceId_t intfId, uint32_t *ipv4MtuLen)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsL3GetIpv4MtuLenForInterface);
    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t *intfInfo;

    GT_STATUS  cpssStatus = GT_OK;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }


    if ((status = xpsInterfaceGetInfo(intfId, &intfInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interfaceType for interface(%d) failed:%d", intfId, status);
        return status;
    }

    if (intfInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interfaceType for interface(%d) failed:%d", intfId, status);
        return status;
    }


    cpssStatus = cpssDxChIpMtuProfileGet(devId, intfInfo->mtuPorfileIndex,
                                         ipv4MtuLen);

    XPS_FUNC_EXIT_LOG();
    return xpsConvertCpssStatusToXPStatus(cpssStatus);
}

XP_STATUS xpsL3SetIpv6MtuLenForInterface(xpsDevice_t devId,
                                         xpsInterfaceId_t intfId, uint32_t ipv6MtuLen)
{
    XPS_FUNC_ENTRY_LOG();
    uint32_t  currentMtuLen = 0;
    XP_STATUS status = XP_NO_ERR;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    status = xpsL3GetIpv4MtuLenForInterface(devId, intfId, &currentMtuLen);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get ipv4 mtu length for interface %d device %d\n", intfId, devId);
        return status;
    }

    if (currentMtuLen == ipv6MtuLen)
    {
        // In case of reassigning the same mtu, don't increment the refCount, just return success
        return XP_NO_ERR;
    }

    XPS_FUNC_EXIT_LOG();
    return xpsL3SetMtuLenForInterface(devId, intfId, ipv6MtuLen);
}

XP_STATUS xpsL3GetIpv6MtuLenForInterface(xpsDevice_t devId,
                                         xpsInterfaceId_t intfId, uint32_t *ipv6MtuLen)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t *intfInfo;
    GT_STATUS  cpssStatus = GT_OK;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    if (((status = xpsInterfaceGetInfo(intfId, &intfInfo)) != XP_NO_ERR) ||
        (intfInfo == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get IPv6 MTU for Interface %d due to failure in retrieving interface info",
              intfId, status);
        return status;
    }

    cpssStatus = cpssDxChIpMtuProfileGet(devId, intfInfo->mtuPorfileIndex,
                                         ipv6MtuLen);

    XPS_FUNC_EXIT_LOG();
    return xpsConvertCpssStatusToXPStatus(cpssStatus);
}

XP_STATUS xpsL3SetMtuPktCmdForInterface(xpsDevice_t devId,
                                        xpsInterfaceId_t intfId, uint32_t mtuPktCmd)
{
    XPS_FUNC_ENTRY_LOG();
    GT_STATUS  cpssStatus = GT_OK;
    CPSS_DXCH_IP_EXCEPTION_TYPE_ENT exceptionType =
        CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_E;
    CPSS_IP_PROTOCOL_STACK_ENT protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
    CPSS_PACKET_CMD_ENT exceptionCmd;
    int  cpssDevNum;

    if ((mtuPktCmd != XP_PKTCMD_TRAP) && (mtuPktCmd != XP_PKTCMD_DROP) &&
        (mtuPktCmd != XP_PKTCMD_FWD))
    {
        LOGFN(xpLogModeXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "pktCmd can only be set as DROP/TRAP/FWD");
        return XP_ERR_INVALID_ARG;
    }

    exceptionCmd = xpsConvertPktCmdToCpssPktCmd((xpsPktCmd_e) mtuPktCmd);

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        cpssStatus = cpssDxChIpExceptionCommandSet(cpssDevNum, exceptionType,
                                                   protocolStack, exceptionCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Could not set MTU error Packet Command on device %d \n", cpssDevNum);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpssStatus);
}

XP_STATUS xpsL3GetMtuPktCmdForInterface(xpsDevice_t devId,
                                        xpsInterfaceId_t intfId, uint32_t *mtuPktCmd)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS  cpssStatus = GT_OK;
    CPSS_DXCH_IP_EXCEPTION_TYPE_ENT exceptionType =
        CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_E;
    CPSS_IP_PROTOCOL_STACK_ENT protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
    CPSS_PACKET_CMD_ENT exceptionCmd;

    cpssStatus = cpssDxChIpExceptionCommandGet(devId, exceptionType, protocolStack,
                                               &exceptionCmd);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not get MTU error Packet Command on device %d \n", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    XPS_FUNC_EXIT_LOG();

    *mtuPktCmd = (uint32_t) exceptionCmd;

    return xpsConvertCpssStatusToXPStatus(cpssStatus);
}

XP_STATUS xpsL3GetVlanIdForL3Interface(xpsInterfaceId_t intfId,
                                       xpsVlan_t* vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetVlanIdForL3InterfaceScope(xpsScope_t scopeId,
                                            xpsInterfaceId_t intfId, xpsVlan_t* vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIgmpCmd(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                          xpsPktCmd_e pktCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIgmpCmd(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                          xpsPktCmd_e *pktCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIcmpv6Cmd(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                            xpsPktCmd_e pktCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIcmpv6Cmd(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                            xpsPktCmd_e *pktCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

///********************************* IPV6 Mcast Routing Domain Management APi End *******************///

XP_STATUS xpsL3SetIpv4RehashLevel(xpDevice_t devId, uint8_t numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIpv4RehashLevel(xpDevice_t devId, uint8_t* numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3SetIpv6RehashLevel(xpDevice_t devId, uint8_t numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3GetIpv6RehashLevel(xpDevice_t devId, uint8_t* numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsL3Ipv4HostGetTableSize(xpsDevice_t devId, uint32_t *tsize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3Ipv6HostGetTableSize(xpsDevice_t devId, uint32_t *tsize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3Ipv4HostClearBucketState(xpDevice_t devId, uint8_t tblCopyIdx)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3Ipv6HostClearBucketState(xpDevice_t devId, uint8_t tblCopyIdx)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3Ipv4RouteClearBucketState(xpDevice_t devId, uint8_t tblCopyIdx)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsL3Ipv6RouteClearBucketState(xpDevice_t devId, uint8_t tblCopyIdx)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}



/** CPSS Changes for LPM IPV4/V6 Route Area **/


static void xpsIpRouteDefaultIpLttEntrySet
(
    GT_U8                         devId,
    CPSS_DXCH_IP_LTT_ENTRY_STC*   ipLttEntry
)
{
    if (NULL == ipLttEntry)
    {
        return;
    }

    cpssOsBzero((GT_CHAR*) ipLttEntry, sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
#if 0
    if ((PRV_CPSS_SIP_5_CHECK_MAC(devId)))
    {
        ipLttEntry->routeType            = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }
    else
    {
        ipLttEntry->routeType            = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    }
    ipLttEntry->numOfPaths               = 1;
    ipLttEntry->routeEntryBaseIndex      = 0;
    ipLttEntry->ucRPFCheckEnable         = GT_FALSE;
    ipLttEntry->sipSaCheckMismatchEnable = GT_FALSE;
    ipLttEntry->ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
}
#endif
ipLttEntry->routeType                =
    CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
ipLttEntry->numOfPaths               = 0;
ipLttEntry->routeEntryBaseIndex      = 0;
ipLttEntry->ucRPFCheckEnable         = GT_FALSE;
ipLttEntry->sipSaCheckMismatchEnable = GT_FALSE;
ipLttEntry->ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
}


static void  xpsIpRouteDefaultVrfConfigSet(xpDevice_t devId, GT_U32 vrfId,
                                           CPSS_DXCH_IP_LPM_VR_CONFIG_STC *vrConfigInfo)
{

    /* Setting the Default Nexthop and Route Of the Default VRF */
    vrConfigInfo->supportIpv4Mc = GT_TRUE;
    vrConfigInfo->supportIpv6Mc = GT_TRUE;
    vrConfigInfo->supportIpv4Uc = GT_TRUE; /*Enabling IPV4 UC on Def VRF*/
    vrConfigInfo->supportIpv6Uc = GT_TRUE; /*Enabling IPV6 UC on Def VRF*/
    vrConfigInfo->supportFcoe   = GT_FALSE;

    xpsIpRouteDefaultIpLttEntrySet(devId,
                                   &vrConfigInfo->defIpv4UcNextHopInfo.ipLttEntry);
    xpsIpRouteDefaultIpLttEntrySet(devId,
                                   &vrConfigInfo->defIpv6UcNextHopInfo.ipLttEntry);
    xpsIpRouteDefaultIpLttEntrySet(devId, &vrConfigInfo->defIpv4McRouteLttEntry);
    vrConfigInfo->defIpv4McRouteLttEntry.routeEntryBaseIndex =
        XPS_l3_DEFAULT_MC_NEXTHOP;
    xpsIpRouteDefaultIpLttEntrySet(devId, &vrConfigInfo->defIpv6McRouteLttEntry);
    vrConfigInfo->defIpv6McRouteLttEntry.routeEntryBaseIndex =
        XPS_l3_DEFAULT_MC_NEXTHOP;

    /* We can enable FCOE/V4/6 MC based on Need
    xpsIpRouteDefaultIpLttEntrySet(dev, &vrConfigInfo.defaultFcoeForwardingNextHopInfo.ipLttEntry);
    */
}

GT_STATUS xpsIpRouteGetLpmDefaultCfg(xpDevice_t devId,
                                     CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT *shadowType,
                                     CPSS_DXCH_LPM_RAM_CONFIG_STC *ramDbCfgPtr)
{
    GT_U32 i=0;
    GT_U32 blockSizeInBytes;
    GT_U32 blockSizeInLines;
    GT_U32 lpmRamNumOfLines;
    GT_U32 lastBlockSizeInLines;
    GT_U32 lastBlockSizeInBytes;
    GT_U32 numOfPbrBlocks;
    GT_U32 maxNumOfPbrEntries;
    GT_U32 maxNumOfPbrEntriesToUse ;
    CPSS_PP_FAMILY_TYPE_ENT devFamilyType;
    CPSS_PP_DEVICE_TYPE devType;

    devFamilyType = cpssHalDevPPFamilyGet(devId);
    if (devFamilyType == CPSS_MAX_FAMILY)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to get dev info");
        return xpsConvertCpssStatusToXPStatus(GT_FAIL);
    }

    devType = cpssHalDevPPTypeGet(devId);
    if (devType == 0xFFFFFFFF)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to get dev type");
        return xpsConvertCpssStatusToXPStatus(GT_FAIL);
    }
    maxNumOfPbrEntriesToUse = maxNumOfPbrEntries = PRV_CPSS_DXCH_PP_MAC(
                                                       devId)->moduleCfg.ip.maxNumOfPbrEntries;

    if (devFamilyType == CPSS_PP_FAMILY_DXCH_AC3X_E ||
        devFamilyType == CPSS_PP_FAMILY_DXCH_ALDRIN_E ||
        devFamilyType == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
        devFamilyType == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        *shadowType      = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
        ramDbCfgPtr->lpmMemMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;

        lpmRamNumOfLines = PRV_CPSS_DXCH_LPM_RAM_GET_NUM_OF_LINES_MAC(
                               PRV_CPSS_DXCH_PP_MAC(devId)->fineTuning.tableSize.lpmRam);

        /*if we are working in half memory mode - then do all the calculations for half size,return to the real values later in the code*/
        if (ramDbCfgPtr->lpmMemMode == CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
        {
            lpmRamNumOfLines/=2;
        }

        blockSizeInLines = (lpmRamNumOfLines/XPS_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS);
        if (blockSizeInLines==0)
        {
            /* can not create a shadow with the current lpmRam size */
            return GT_FAIL;
        }
        blockSizeInBytes = blockSizeInLines * 4;

        if (maxNumOfPbrEntriesToUse >= lpmRamNumOfLines)
        {
            /* No memory for Ip LPM */
            return GT_FAIL;
        }
        if (maxNumOfPbrEntriesToUse > blockSizeInLines)
        {
            numOfPbrBlocks = (maxNumOfPbrEntriesToUse + blockSizeInLines - 1) /
                             blockSizeInLines;
            lastBlockSizeInLines = (numOfPbrBlocks*blockSizeInLines)
                                   -maxNumOfPbrEntriesToUse;
            if (lastBlockSizeInLines==0)/* PBR will fit exactly in numOfPbrBlocks */
            {
                ramDbCfgPtr->numOfBlocks = XPS_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS - numOfPbrBlocks;
                lastBlockSizeInLines = blockSizeInLines; /* all of last block for IP LPM */
            }
            else/* PBR will not fit exactly in numOfPbrBlocks and we will have in the last block LPM lines together with PBR lines*/
            {
                ramDbCfgPtr->numOfBlocks = XPS_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS - numOfPbrBlocks +
                                           1;
            }
        }
        else
        {
            if (maxNumOfPbrEntriesToUse == blockSizeInLines)
            {
                ramDbCfgPtr->numOfBlocks = XPS_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS-1;
                lastBlockSizeInLines = blockSizeInLines;
            }
            else
            {
                ramDbCfgPtr->numOfBlocks = XPS_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS;
                lastBlockSizeInLines = blockSizeInLines - maxNumOfPbrEntriesToUse;
            }
        }

        /* number of LPM bytes ONLY when last block is shared between LPM and PBR */
        lastBlockSizeInBytes = lastBlockSizeInLines * 4;

        for (i=0; i<ramDbCfgPtr->numOfBlocks-1; i++)
        {
            ramDbCfgPtr->blocksSizeArray[i] = blockSizeInBytes;
        }
        ramDbCfgPtr->blocksSizeArray[ramDbCfgPtr->numOfBlocks-1] =
            lastBlockSizeInBytes == 0 ?
            blockSizeInBytes :   /* last block is fully LPM (not PBR) */
            lastBlockSizeInBytes;/* last block uses 'x' for LPM , rest for PBR */

        /* reset other sections */
        i = ramDbCfgPtr->numOfBlocks;

        for (/*continue i*/; i<XPS_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS; i++)
        {
            ramDbCfgPtr->blocksSizeArray[i] = 0;
        }

        //        ramDbCfgPtr->blocksAllocationMethod =
        //            CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E;
        ramDbCfgPtr->blocksAllocationMethod =
            CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;

    }
    else if (devFamilyType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
             devFamilyType == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        *shadowType = CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E;
        ramDbCfgPtr->maxNumOfPbrEntries = maxNumOfPbrEntries;
        ramDbCfgPtr->lpmRamConfigInfo[0].devType = devType;
        ramDbCfgPtr->lpmRamConfigInfo[0].sharedMemCnfg = PRV_CPSS_DXCH_PP_MAC(
                                                             devId)->moduleCfg.ip.sharedTableMode;
        ramDbCfgPtr->lpmRamConfigInfoNumOfElements=1;
        ramDbCfgPtr->lpmMemMode =
            CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;/*the only mode for Falcon*/
        ramDbCfgPtr->blocksAllocationMethod  =
            CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;
        return GT_OK;
    }
    else
    {
        return GT_FAIL;
    }
    return GT_OK;


}


static XP_STATUS xpsL3SetupRouterVlan(xpsDevice_t devId, xpsVlan_t vlanId)
{
    GT_STATUS cpssStatus = GT_OK;
    CPSS_PORTS_BMP_STC portMembers;
    CPSS_PORTS_BMP_STC portTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portTagCmd;
    //xpsVlan_t vlanId = XPS_L3_RESERVED_ROUTER_VLAN;
    int devNum;
    int portNum;
    int cpssCscdPort;
    int cpssCscdMaxPorts;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT cpssTagType =
        CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
    GT_BOOL         tag = GT_TRUE;

    memset(&portMembers, 0, sizeof(CPSS_PORTS_BMP_STC));
    memset(&portTagging, 0, sizeof(portTagging));
    memset(&vlanInfo, 0, sizeof(CPSS_DXCH_BRG_VLAN_INFO_STC));
    memset(&portTagCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    vlanInfo.autoLearnDisable = GT_TRUE;
    vlanInfo.bcastUdpTrapMirrEn = GT_FALSE;
    vlanInfo.fcoeForwardingEn = GT_FALSE;
    vlanInfo.ipv4UcastRouteEn = GT_TRUE;
    vlanInfo.ipv4McastRouteEn = GT_TRUE;
    vlanInfo.ipv6McastRouteEn = GT_TRUE;
    vlanInfo.ipv6UcastRouteEn = GT_TRUE;
    //vlanInfo.fidValue = XPS_L3_RESERVED_ROUTER_VLAN;
    vlanInfo.vrfId = XPS_L3_DEFAULT_VRF;
    /* TODO :Now MC DMAC with no FDB entries can be dropped.
             Revisit this on supporting IPMC */
    vlanInfo.unregNonIpMcastCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    /*ARP/ND Fwd pkt-action will fail as it rely on Bridge engine result*/
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    /*Vlan flooding of BC DA MAC and BC DIP on SVI. Here route-trigger check
      will fail as DMAC is BC, hence rely on bridge for flooding.
      TODO : pkts with DMAC and UC DIP are forwarded now because of this.
      Check how it can be done.
      */
    vlanInfo.unregIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    /* Required when L3 enabled on router port for IGMP. */
    vlanInfo.unregIpv4McastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv6McastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unkUcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IpmBrgMode = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv4IpmBrgEn = GT_TRUE;
    vlanInfo.ipv6IpmBrgEn = GT_TRUE;
    vlanInfo.floodVidx = 0xFFF;
    vlanInfo.floodVidxMode = CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E;
    vlanInfo.naMsgToCpuEn = GT_TRUE;
    vlanInfo.unregIpmEVidx = 0xFFF;
    vlanInfo.unregIpmEVidxMode = CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E;
    vlanInfo.ipv4IgmpToCpuEn = GT_FALSE;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {

        vlanInfo.fidValue = vlanId;

        cpssStatus = cpssDxChBrgVlanEntryWrite(devNum, vlanId,
                                               &portMembers, &portTagging, &vlanInfo, &portTagCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to create default router vlan %d: Cpss RC: %d\n", vlanId, cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
        if ((cpssStatus = cpssDxChBrgVlanMruProfileIdxSet(devNum, vlanId, 0)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set MRU default router vlan %d: Cpss RC: %d\n", vlanId, cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        if ((cpssStatus = cpssDxChBrgVlanForwardingIdSet(devNum, vlanId,
                                                         vlanId)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set frwding router vlan %d: Cpss RC: %d\n", vlanId, cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    /* Add Cascade Ports to Router  Vlan*/
    XPS_B2B_CSCD_PORTS_ITER(devNum, portNum, cpssCscdPort, cpssCscdMaxPorts)
    {
        if ((cpssDxChBrgVlanMemberAdd(devNum, vlanId, cpssCscdPort, tag,
                                      cpssTagType)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpss vlanmember add failed for devNum %d vlan %d port %d", devNum, vlanId,
                  cpssCscdPort);
        }
    }
    /*
       if((status = xpsCascadeAddPortsToVlan(vlanId))!= XP_NO_ERR)
       {
       LOGFN(xpLogModXps,XP_SUBMOD_MAIN,XP_LOG_ERROR, "Failed to add cascade ports to  default router vlan: XP RC: %d\n", status);
       return status;
       }
       */

#if 0 // TODO: disable BUM on L3 interface
    if (XP_NO_ERR != xpsVlanSetUnknownUcCmd(devId, XPS_L3_RESERVED_ROUTER_VLAN,
                                            XP_PKTCMD_DROP))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Setting unknown Uc cmd failed for vlan (%d)", vlanId);
    }
#endif //0 
    return XP_NO_ERR;
}


XP_STATUS xpsIpRouteInit(xpDevice_t devId)
{
    GT_STATUS                                    st              = GT_OK;
    XP_STATUS                                    status          = XP_NO_ERR;
    GT_U32                                       lpmDBId         =
        XPS_CPSS_DEFAULT_LPM_DB_ID;
    //    GT_U32                   vrfId       = XPS_CPSS_DEFAULT_VRF_ID;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT             shadowType      ;
    CPSS_IP_PROTOCOL_STACK_ENT                   protocolStack   =
        CPSS_IP_PROTOCOL_IPV4V6_E;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT           lpmMemoryConfig;
    //    CPSS_DXCH_IP_LPM_VR_CONFIG_STC               vrConfigInfo;
    GT_BOOL                                      enableRouting = GT_TRUE;
    GT_U8                    devNum;


    cpssOsBzero((GT_CHAR*)&lpmMemoryConfig, sizeof(lpmMemoryConfig));

    st = xpsIpRouteGetLpmDefaultCfg(devId, &shadowType, &lpmMemoryConfig.ramDbCfg);
    if ((status = xpsConvertCpssStatusToXPStatus(st))!=XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error Getting Default LPM Config for the deviceId = %d\n", devId);
        return status;
    }

    st = cpssDxChIpLpmDBCreate(lpmDBId, shadowType, protocolStack,
                               &lpmMemoryConfig);
    if ((status = xpsConvertCpssStatusToXPStatus(st))!=XP_NO_ERR)
    {
        if (status == XP_ERR_KEY_EXISTS)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "Device Id = %d Already added to Lpm DB Id = %d shadowType = %d, protocolStack = %d\n",
                  devId, lpmDBId, shadowType, protocolStack);
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Device Id = %d Couldnt be Added to Lpm DB Id = %d"
                  "shadowType = %d, protocolStack = %d \n",
                  devId, lpmDBId, shadowType, protocolStack);
            return status;
        }
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "Device Id = %d Successfully Added to Lpm DB Id = %d"
              "shadowType = %d, protocolStack = %d \n",
              devId, lpmDBId, shadowType, protocolStack);
        //Print Log Message for Status of Tcam Based DB Creation
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        st = cpssDxChIpLpmDBDevListAdd(lpmDBId, (GT_U8*)&devNum, 1);
        if ((status = xpsConvertCpssStatusToXPStatus(st))!=XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Device Id = %d Failed to Add to Lpm DB Id = %d", devNum, lpmDBId);
            return status;
        }
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        st = cpssDxChIpRoutingEnable(devNum, enableRouting);
        if ((status = xpsConvertCpssStatusToXPStatus(st))!=XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Routing Enable on Device Id  = %d Failed!!!\n", devNum);
            return status;
        }
    }


    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "Routing Enable on Device Id  = %d Successful!!!\n", devId);
    return status;
}

XP_STATUS xpsIpRouteDeinit(xpDevice_t devId)
{
    XP_STATUS                                    status    = XP_NO_ERR;
    GT_STATUS                                    cpss_status        = GT_OK;
    GT_U32                                       lpmDBId   =
        XPS_CPSS_DEFAULT_LPM_DB_ID;
    //    GT_U32                   vrfId       = XPS_CPSS_DEFAULT_VRF_ID;
    GT_BOOL                                      enableRouting = GT_FALSE;
    GT_U8                    devNum = 0xFF;

    /*Unconfigure Routing GLobally*/
    cpss_status = cpssHalEnableIpRouting(devId, enableRouting);
    if ((status = xpsConvertCpssStatusToXPStatus(cpss_status))!=XP_NO_ERR)
    {
        /*Pring Error Log Message with DevId and routing Flag*/
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Routing Disable on Device Id  = %d Failed!!!\n", devId);
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "Routing Disable on Device Id  = %d Successful!!!\n", devId);
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        cpss_status = cpssDxChIpLpmDBDevsListRemove(lpmDBId, (GT_U8*)&devNum, 1);
        if ((status = xpsConvertCpssStatusToXPStatus(cpss_status))!=XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Device Id = %d Failed to Remove to Lpm DB Id = %d", devNum, lpmDBId);
            return status;
        }
    }

    /*Delete the LPM Database*/
    cpss_status = cpssDxChIpLpmDBDelete(lpmDBId);
    if ((status = xpsConvertCpssStatusToXPStatus(cpss_status))!=XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Lpm Id  = %d Delete Failed !!!\n", lpmDBId);
        return status;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "Lpm Id  = %d Delete Successful !!!\n", lpmDBId);
    return XP_NO_ERR;
}


XP_STATUS xpsL3AddDirectedBroadcast(xpsDevice_t devId,
                                    xpsL3HostEntry_t xpsL3NeighborEntry)
{
    XP_STATUS                       status = XP_NO_ERR;
    xpsHashIndexList_t              indexList;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC cpssNhEntry;
    xpsInterfaceInfo_t              *intfInfo;

    /* Check if the device is valid */
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    memset(&cpssNhEntry, 0, sizeof(cpssNhEntry));
    memset(&indexList, 0, sizeof(xpsHashIndexList_t));

    for (int i =0; i<6; i++)
    {
        xpsL3NeighborEntry.nhEntry.nextHop.macDa[i]=0xff;
    }
    xpsL3NeighborEntry.ipv4Addr[2] |=
        7; //TODO: Need to fix this hardcode by getting prefix and generating broadcast ip
    xpsL3NeighborEntry.ipv4Addr[3]= 255;
    status = xpsInterfaceGetInfo(xpsL3NeighborEntry.nhEntry.nextHop.l3InterfaceId,
                                 &intfInfo);
    if (status != XP_NO_ERR || intfInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface type for L3 interface %d\n",
              xpsL3NeighborEntry.nhEntry.nextHop.l3InterfaceId);
        return status;
    }

    if (intfInfo->type == XPS_VLAN_ROUTER)
    {
        xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId =
            xpsL3NeighborEntry.nhEntry.nextHop.l3InterfaceId;
    }

    if ((status = xpsL3AddIpHostEntry(devId, &xpsL3NeighborEntry,
                                      &indexList))!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "adding host entry in fdb for directed bcast failed\n");
        return status;
    }

    return XP_NO_ERR;


}

XP_STATUS xpsL3WriteIpClassERouteEntry(xpsDevice_t devId,
                                       xpsL3RouteEntry_t *pL3RouteEntry, bool newEntry)
{
    XP_STATUS           status      = XP_NO_ERR;
    uint32_t            prfxBucketIdx = 0;
    xpsL3RouteEntry_t   pL3ClassERouteEntry;

    memcpy(&pL3ClassERouteEntry, pL3RouteEntry, sizeof(xpsL3RouteEntry_t));
    pL3ClassERouteEntry.ipv4Addr[0] = 240;
    pL3ClassERouteEntry.ipMaskLen = 4;

    if (newEntry)
    {
        status = xpsL3AddIpRouteEntry(devId, &pL3ClassERouteEntry, &prfxBucketIdx,
                                      NULL);
        if (status !=XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsL3AddIpRouteEntry Failed for classE : %d\n", status);
            return status;
        }
    }
    else
    {
        status = xpsL3UpdateIpRouteEntry(devId, &pL3ClassERouteEntry, NULL);
        if (status !=XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsL3UpdateIpRouteEntry Failed for classE : %d\n", status);
            return status;
        }
    }
    return XP_NO_ERR;
}

// API to Add Route Entry
XP_STATUS xpsL3AddIpRouteEntry(xpsDevice_t devId,
                               xpsL3RouteEntry_t *pL3RouteEntry, uint32_t *prfxBucketIdx,
                               xpsL3RouteBulkInfo_t *routeInfo)
{


    XP_STATUS                              status      = XP_NO_ERR;
    //uint32_t prfxIdx = 0xFFFFFFFF;
    GT_U32                                 lpmDBId     = XPS_CPSS_DEFAULT_LPM_DB_ID;
    GT_U32                                 vrId        = XPS_CPSS_DEFAULT_VRF_ID;
    GT_IPADDR                              ipAddr;
    GT_IPV6ADDR                            ipv6Addr;
    GT_U32                                 prefixLen   = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_BOOL                                override    = GT_TRUE;
    GT_BOOL                                defragmentationEnable = GT_TRUE;

    GT_STATUS st= GT_OK;
    static uint32_t failCnt = 0;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    if (!pL3RouteEntry)
    {
        return XP_ERR_NULL_POINTER;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "Entered func : xpsL3AddIpRouteEntry , type %d nhId %d ecmpsize %d",
          pL3RouteEntry->type, pL3RouteEntry->nhId, pL3RouteEntry->nhEcmpSize);

    if (pL3RouteEntry->vrfId > XPS_CPSS_DEFAULT_VRF_ID)
    {
        vrId = pL3RouteEntry->vrfId;
    }

    if (pL3RouteEntry->type)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "xpsL3AddIpRouteEntry  " FMT_IP6 " /%d ",
              PRI_IP6(pL3RouteEntry->ipv6Addr),
              pL3RouteEntry->ipMaskLen);
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "xpsL3AddIpRouteEntry " FMT_IP4 " /%d ",
              PRI_IP4(pL3RouteEntry->ipv4Addr),
              pL3RouteEntry->ipMaskLen);
    }

    cpssOsBzero((GT_CHAR*)&nextHopInfo, sizeof(nextHopInfo));
    prefixLen = pL3RouteEntry->ipMaskLen;
    xpsIpRouteDefaultIpLttEntrySet(devId, &nextHopInfo.ipLttEntry);

    /*
     * FIXME: This would not be a proper way of determining if ECMP or not.
     * Need to conclude on the changing the xpsL3RouteEntry_t to reflect this.
     * nhEcmpSize in xpsL3RouteEntry_t is not relevant as the NH ecmp size is
     * processed and set during NH group creation
     */
    if (pL3RouteEntry->nhEcmpSize > 1)
    {
        nextHopInfo.ipLttEntry.routeEntryBaseIndex      = pL3RouteEntry->nhId;
        // Check if the NH group exists
        if (xpsL3ExistsRouteNextHopGroup(pL3RouteEntry->nhId) == false)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "NH group %d does not exist\n",
                  pL3RouteEntry->nhId);
            return XP_ERR_NOT_FOUND;
        }

        if (PRV_CPSS_SIP_6_CHECK_MAC(devId))
        {
            nextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E;
        }
        else
        {
            nextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        }
    }
    else
    {
        /*if not ecmp, convert to hw Id*/

        status = xpsL3NextHopMapGet(pL3RouteEntry->nhId,
                                    &nextHopInfo.ipLttEntry.routeEntryBaseIndex);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsL3AddIpRouteEntry mapGet failed nhId  %u ret %d\n", pL3RouteEntry->nhId,
                  status);
            return status;
        }
        nextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }

    if (routeInfo)
    {
        routeInfo->vrfId = vrId;
        routeInfo->type = pL3RouteEntry->type;
        memcpy(routeInfo->ipv4Addr, pL3RouteEntry->ipv4Addr, sizeof(ipv4Addr_t));
        memcpy(routeInfo->ipv6Addr, pL3RouteEntry->ipv6Addr, sizeof(ipv6Addr_t));
        routeInfo->override = (bool)override;
        routeInfo->ipMaskLen = prefixLen;
        memcpy(&(routeInfo->nextHopInfo), &nextHopInfo, sizeof(nextHopInfo));
        return XP_NO_ERR;
    }

    if (pL3RouteEntry->type == XP_PREFIX_TYPE_IPV4)
    {
        memcpy(ipAddr.arIP, pL3RouteEntry->ipv4Addr, sizeof(ipv4Addr_t));

        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen,
                                          &nextHopInfo, override, defragmentationEnable);
        if ((status = xpsConvertCpssStatusToXPStatus(st))!=XP_NO_ERR)
        {
            if ((st == GT_OUT_OF_PP_MEM) && (failCnt == 0))
            {
                // Print Log Message saying that the IPV4 Route Entry didnt get added.
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "func : xpsL3AddIpV4RouteEntry ret %d prefixLen %d, added ipv4 " FMT_IP4" \n",
                      st, prefixLen,
                      PRI_IP4(pL3RouteEntry->ipv4Addr));
                /* Collect LPM Usage */
                st = cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(lpmDBId);
                if (GT_OK != st)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters FAILED, st = [%d]",
                          st);
                }

                st = cpssDxChIpLpmDbgHwOctetPerProtocolPrint(lpmDBId);
                if (GT_OK != st)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChIpLpmDbgHwOctetPerProtocolPrint FAILED, st = [%d]", st);
                }
                failCnt = 1;
            }
            return status;
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "xpsL3AddIpV4RouteEntry add success %d prefixLen %d, added ipv4 " FMT_IP4" \n",
                  st, prefixLen,
                  PRI_IP4(pL3RouteEntry->ipv4Addr));
        }
    }
    else if (pL3RouteEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        memcpy(ipv6Addr.arIP, pL3RouteEntry->ipv6Addr, sizeof(ipv6Addr_t));

        st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipv6Addr, prefixLen,
                                          &nextHopInfo, override, defragmentationEnable);
        if ((status = xpsConvertCpssStatusToXPStatus(st))!=XP_NO_ERR)
        {
            if ((st == GT_OUT_OF_PP_MEM) && (failCnt == 0))
            {
                // Print Log Message saying that the IPV6 Route Entry didnt get added.
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "func : xpsL3AddIpV6RouteEntry ret %d prefixLen %d, added ipv6 " FMT_IP6"\n",
                      st, prefixLen, PRI_IP6(pL3RouteEntry->ipv6Addr));

                /* Collect LPM Usage */
                st = cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(lpmDBId);
                if (GT_OK != st)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters FAILED, st = [%d]",
                          st);
                }

                st = cpssDxChIpLpmDbgHwOctetPerProtocolPrint(lpmDBId);
                if (GT_OK != st)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChIpLpmDbgHwOctetPerProtocolPrint FAILED, st = [%d]", st);
                }
                failCnt = 1;
            }
            return status;
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "xpsL3AddIpV6RouteEntry add success %d prefixLen %d, added ipv6 " FMT_IP6"\n",
                  st, prefixLen, PRI_IP6(pL3RouteEntry->ipv6Addr));
        }
    }
    else
    {
        return XP_ERR_INVALID_PARAMS;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "exit func : xpsL3AddIpRouteEntry , type %d", pL3RouteEntry->type);
    return status;
}

// API to Remove Route Entry
XP_STATUS xpsL3RemoveIpRouteEntry(xpsDevice_t devId,
                                  xpsL3RouteEntry_t *pL3RouteEntry,
                                  xpsL3RouteBulkInfo_t *routeInfo)
{
    GT_U32                                 lpmDBId     = XPS_CPSS_DEFAULT_LPM_DB_ID;
    GT_U32                                 vrId        = XPS_CPSS_DEFAULT_VRF_ID;
    GT_IPADDR                              ipAddr;
    GT_IPV6ADDR                            ipv6Addr;
    GT_U32                                 prefixLen   = 0;
    GT_STATUS                  st          = GT_OK;
    XP_STATUS                              status      = XP_NO_ERR;
    xpsL3RouteEntry_t                      l3RouteEntry;
    uint32_t                               defNhId = 0;
    uint32_t                               hwNhId;
    xpsL3NextHopEntry_t                    defaultNh;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    if (!pL3RouteEntry)
    {
        return XP_ERR_NULL_POINTER;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "Entered func :xpsL3RemoveIpRouteEntry , type %d nhId %d ecmpSize %d",
          pL3RouteEntry->type, pL3RouteEntry->nhId, pL3RouteEntry->nhEcmpSize);

    if (pL3RouteEntry->vrfId > XPS_CPSS_DEFAULT_VRF_ID)
    {
        vrId = pL3RouteEntry->vrfId;
    }

    if (pL3RouteEntry->type)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "xpsL3RemoveIpRouteEntry " FMT_IP6" /%d ",
              PRI_IP6(pL3RouteEntry->ipv6Addr),
              pL3RouteEntry->ipMaskLen);
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "xpsL3RemoveIpRouteEntry " FMT_IP4 "/%d ",
              PRI_IP4(pL3RouteEntry->ipv4Addr),
              pL3RouteEntry->ipMaskLen);
    }
    memcpy(&l3RouteEntry, pL3RouteEntry, sizeof(xpsL3RouteEntry_t));
    /*decrement ref count of the sw nh id and del if 0*/
    if (pL3RouteEntry->nhEcmpSize == 1)
    {
        status = xpsL3NextHopMapGet(pL3RouteEntry->nhId, &hwNhId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsL3RemoveIpRouteEntry mapGet failed nhId  %u ret %d\n", pL3RouteEntry->nhId,
                  status);
            return status;
        }
        if (hwNhId != INVALID_L3_INDEX)
        {
            status =   xpsL3NextHopMapRemove(pL3RouteEntry->nhId,  hwNhId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "xpsL3RemoveIpRouteEntry remMap failed nhId  %u hwNhId %u ret %d\n",
                      pL3RouteEntry->nhId, hwNhId, status);
                return status;
            }
        }
    }

    if (routeInfo)
    {
        routeInfo->vrfId = vrId;
        routeInfo->type = pL3RouteEntry->type;
        memcpy(routeInfo->ipv4Addr, pL3RouteEntry->ipv4Addr, sizeof(ipv4Addr_t));
        memcpy(routeInfo->ipv6Addr, pL3RouteEntry->ipv6Addr, sizeof(ipv6Addr_t));
        routeInfo->override = GT_FALSE;
        routeInfo->ipMaskLen = pL3RouteEntry->ipMaskLen;
        return XP_NO_ERR;
    }

    if (pL3RouteEntry->type == XP_PREFIX_TYPE_IPV4)
    {
        memcpy(ipAddr.arIP, pL3RouteEntry->ipv4Addr, sizeof(ipv4Addr_t));
        prefixLen = pL3RouteEntry->ipMaskLen;

        GT_U32 ipAddrU32 = *((GT_U32 *)(ipAddr.arIP));
        if (ipAddrU32 == 0 && prefixLen == 0)
        {
            memset(&defaultNh, 0, sizeof(defaultNh));
            defaultNh.pktCmd = XP_PKTCMD_DROP;

            //TODO the default egress interface was 0. This caused issue because port 0 is not mappted to a device.
            //To unblock,making the egress intf to a port which is in the port range of device 0. Need to
            // fix it properly

            defaultNh.nextHop.egressIntfId = 17;   // from default nh settings

            status = xpsL3SetRouteNextHop(0, defNhId, &defaultNh);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to set default NH\n");
                return status;
            }
        }

        else
        {
            st = cpssDxChIpLpmIpv4UcPrefixDel(lpmDBId, vrId, &ipAddr, prefixLen);
            if ((status = xpsConvertCpssStatusToXPStatus(st))!=XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "IPv4 entry Failed to delete the route:rc=%d.\n", st);
                return status;
            }
            else
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                      "xpsL3RemoveIpRouteEntry success " FMT_IP4 "/%d ",
                      PRI_IP4(pL3RouteEntry->ipv4Addr),
                      pL3RouteEntry->ipMaskLen);
            }
        }
    }
    else if (pL3RouteEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        memcpy(ipv6Addr.arIP, pL3RouteEntry->ipv6Addr, sizeof(ipv6Addr_t));
        prefixLen = pL3RouteEntry->ipMaskLen;
        st = cpssDxChIpLpmIpv6UcPrefixDel(lpmDBId, vrId, &ipv6Addr, prefixLen);
        if ((status = xpsConvertCpssStatusToXPStatus(st))!=XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "IPv6 entry Failed to delete the route:rc=%d.\n", st);
            return status;
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "IPv6 xpsL3RemoveIpRouteEntry Sucess " FMT_IP6" /%d ",
                  PRI_IP6(pL3RouteEntry->ipv6Addr),
                  pL3RouteEntry->ipMaskLen);
        }
    }
    else
    {
        return XP_ERR_INVALID_PARAMS;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "exit func : xpsL3RemoveIpRouteEntry , type %d\n", pL3RouteEntry->type);

    return XP_NO_ERR;
}

GT_STATUS xpsL3IpRouteInfoGetHw(xpsDevice_t devId, inetAddr_t* ipAddress,
                                GT_U32 prefixLen,
                                uint32_t vrfId,
                                CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT *nextHopInfoGet)
{
    GT_U32                                 lpmDBId     = XPS_CPSS_DEFAULT_LPM_DB_ID;
    GT_IPADDR                              ipAddr;
    GT_IPV6ADDR                            ipv6Addr;
    GT_U32                                 tcamRowIndex = 0;
    GT_U32                                 tcamColumIndex = 0;
    GT_STATUS                               st;

    if (ipAddress->type == XP_PREFIX_TYPE_IPV4)
    {
        memcpy(ipAddr.arIP, ipAddress->addr.ipv4Addr, sizeof(ipv4Addr_t));

        st = cpssDxChIpLpmIpv4UcPrefixSearch(lpmDBId, vrfId, &ipAddr, prefixLen,
                                             nextHopInfoGet, &tcamRowIndex,
                                             &tcamColumIndex);
        if (st!= GT_OK)
        {
            //Print Error Message when we cant fin the IPV4 Route Entry
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                  "Getting Ip addr from HW ret %d", st);
            return st;
        }
    }
    else if (ipAddress->type == XP_PREFIX_TYPE_IPV6)
    {
        memcpy(ipv6Addr.arIP, ipAddress->addr.ipv6Addr, sizeof(ipv6Addr_t));

        st = cpssDxChIpLpmIpv6UcPrefixSearch(lpmDBId, vrfId, &ipv6Addr, prefixLen,
                                             nextHopInfoGet, &tcamRowIndex,
                                             &tcamColumIndex);
        //    st = cpssDxChIpLpmIpv6UcPrefixGet(lpmDBId, vrfId, &ipAddr, &prefixLen,
        //                                          &nextHopInfoGet, &tcamRowIndex,
        //                                          &tcamColumIndex);
        if (st!= GT_OK)
        {
            //Print Error Message when we cant fin the IPV6 Route Entry
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                  "Getting Ip addr from HW ret %d", st);
            return st;
        }
    }
    else
    {
        return GT_BAD_VALUE;
    }
    return GT_OK;
}

XP_STATUS xpsL3FindIpRouteEntry(xpsDevice_t devId,
                                xpsL3RouteEntry_t *pL3RouteEntry, uint32_t *prfxBucketIdx)
{

    XP_STATUS                              status      = XP_NO_ERR;
    GT_STATUS                  st          = GT_OK;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfoGet;
    inetAddr_t ipAddr;


    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    if ((!pL3RouteEntry) || (!prfxBucketIdx))
    {
        return XP_ERR_NULL_POINTER;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "Entered func :xpsL3FindIpRouteEntry , type %d nhId %d ecmpSize %d",
          pL3RouteEntry->type, pL3RouteEntry->nhId, pL3RouteEntry->nhEcmpSize);
    if (pL3RouteEntry->type)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "xpsL3FindIpRouteEntry " FMT_IP6 "/%d ",
              PRI_IP6(pL3RouteEntry->ipv6Addr),
              pL3RouteEntry->ipMaskLen);
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "xpsL3FindIpRouteEntry " FMT_IP4" /%d ",
              PRI_IP4(pL3RouteEntry->ipv4Addr),
              pL3RouteEntry->ipMaskLen);
    }

    cpssOsBzero((GT_CHAR*)&nextHopInfoGet, sizeof(nextHopInfoGet));
    cpssOsBzero((GT_CHAR*)&ipAddr, sizeof(ipAddr));

    if ((ipAddr.type = pL3RouteEntry->type) == XP_PREFIX_TYPE_IPV4)
    {
        memcpy(ipAddr.addr.ipv4Addr, pL3RouteEntry->ipv4Addr, sizeof(ipv4Addr_t));
    }
    else
    {
        memcpy(ipAddr.addr.ipv6Addr, pL3RouteEntry->ipv6Addr, sizeof(ipv6Addr_t));
    }

    st =  xpsL3IpRouteInfoGetHw(devId, &ipAddr, pL3RouteEntry->ipMaskLen,
                                pL3RouteEntry->vrfId, &nextHopInfoGet);
    if ((status = xpsConvertCpssStatusToXPStatus(st))!=XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "Exit func :xpsL3FindIpRouteEntry route get  ret %d", status);
        return status;
    }
    if (nextHopInfoGet.ipLttEntry.routeType ==
        CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E)
    {
        pL3RouteEntry->nhEcmpSize = 1;
        status = xpsL3GetHwId2SwId(nextHopInfoGet.ipLttEntry.routeEntryBaseIndex,
                                   &pL3RouteEntry->nhId);
        if (status != XP_NO_ERR)
        {
            return status;
        }
    }
    else /*PBR not supported*/
    {
        pL3RouteEntry->nhId =nextHopInfoGet.ipLttEntry.routeEntryBaseIndex;
        pL3RouteEntry->nhEcmpSize = nhGrpMaxSize;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "Exit func :xpsL3FindIpRouteEntry  nhid %d ecmpSize %d",
          pL3RouteEntry->nhId, pL3RouteEntry->nhEcmpSize);
    return XP_NO_ERR;
}



XP_STATUS xpsL3UpdateIpRouteEntry(xpsDevice_t devId,
                                  xpsL3RouteEntry_t *pL3RouteEntry,
                                  xpsL3RouteBulkInfo_t *routeInfo)
{
    XP_STATUS                              status      = XP_NO_ERR;
    GT_STATUS                  st          = GT_OK;
    GT_U32                                 lpmDBId     = XPS_CPSS_DEFAULT_LPM_DB_ID;
    GT_U32                                 vrId        = XPS_CPSS_DEFAULT_VRF_ID;
    GT_IPADDR                              ipAddr;
    GT_IPV6ADDR                            ipv6Addr;
    GT_U32                                 prefixLen   = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_BOOL                                override    = GT_FALSE;
    GT_BOOL                                defragmentationEnable = GT_TRUE;
    uint32_t                               nhHwId;
    inetAddr_t                              ipAddress;
    uint32_t oldSwNhId;
    uint32_t oldHwNhId;
    uint8_t    updateNhMap = 0;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    if (!pL3RouteEntry)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (pL3RouteEntry->vrfId > XPS_CPSS_DEFAULT_VRF_ID)
    {
        vrId = pL3RouteEntry->vrfId;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "Entered func :xpsL3UpdateIpRouteEntry , type %d nhId %d ecmpSize %d",
          pL3RouteEntry->type, pL3RouteEntry->nhId, pL3RouteEntry->nhEcmpSize);
    if (pL3RouteEntry->type)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "xpsL3UpdateIpRouteEntry " FMT_IP6 " /%d ",
              PRI_IP6(pL3RouteEntry->ipv6Addr),
              pL3RouteEntry->ipMaskLen);
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "xpsL3UpdateIpRouteEntry " FMT_IP4" /%d ",
              PRI_IP4(pL3RouteEntry->ipv4Addr),
              pL3RouteEntry->ipMaskLen);
    }
    /*release the old NH id( not ecmp) used by route*/
    cpssOsBzero((GT_CHAR*)&ipAddress, sizeof(ipAddress));
    cpssOsBzero((GT_CHAR*)&nextHopInfo, sizeof(nextHopInfo));

    if ((ipAddress.type = pL3RouteEntry->type) == XP_PREFIX_TYPE_IPV4)
    {
        memcpy(ipAddress.addr.ipv4Addr, pL3RouteEntry->ipv4Addr, sizeof(ipv4Addr_t));
    }
    else
    {
        memcpy(ipAddress.addr.ipv6Addr, pL3RouteEntry->ipv6Addr, sizeof(ipv6Addr_t));
    }

    st =  xpsL3IpRouteInfoGetHw(devId, &ipAddress, pL3RouteEntry->ipMaskLen,
                                pL3RouteEntry->vrfId, &nextHopInfo);
    if ((status = xpsConvertCpssStatusToXPStatus(st))!=XP_NO_ERR)
    {
        return status;
    }
    /*update SW db maps*/
    if (nextHopInfo.ipLttEntry.routeType ==
        CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E)
    {
        updateNhMap = 1;
        xpsL3GetHwId2SwId(nextHopInfo.ipLttEntry.routeEntryBaseIndex, &oldSwNhId);
        oldHwNhId = nextHopInfo.ipLttEntry.routeEntryBaseIndex;
    }

    cpssOsBzero((GT_CHAR*)&nextHopInfo, sizeof(nextHopInfo));
    xpsIpRouteDefaultIpLttEntrySet(devId, &nextHopInfo.ipLttEntry);

    if (pL3RouteEntry->nhEcmpSize > 1)
    {
        nextHopInfo.ipLttEntry.routeEntryBaseIndex = pL3RouteEntry->nhId;
    }
    else
    {
        /*Get HW Id, if exists increment the refcount
            else create new Hw id and use it*/
        status = xpsL3NextHopMapGet(pL3RouteEntry->nhId, &nhHwId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "get NH map failed ret %d",
                  status);
            return status;
        }

        if (nhHwId  == INVALID_L3_INDEX)
        {
            /*get the hw id for new entry to be written*/
            status = xpsL3CreateRouteNhHwTable((xpsScope_t)devId, 1, &nhHwId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "xpsL3CreateRouteNhHwTable failed ret %d", status);
                return status;
            }
        }
        xpsL3NextHopMapInsert(pL3RouteEntry->nhId, nhHwId);
        nextHopInfo.ipLttEntry.routeEntryBaseIndex = nhHwId;
    }

    /*
     * FIXME: This would not be a proper way of determining if ECMP or not.
     * Need to conclude on the changing the xpsL3RouteEntry_t to reflect this.
     * nhEcmpSize in xpsL3RouteEntry_t is not relevant as the NH ecmp size is
     * processed and set during NH group creation
     */
    if (pL3RouteEntry->nhEcmpSize > 1)
    {
        // Check if the NH group exists
        if (xpsL3ExistsRouteNextHopGroup(pL3RouteEntry->nhId) == false)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "NH group %d does not exist\n",
                  pL3RouteEntry->nhId);
            return XP_ERR_NOT_FOUND;
        }
        if (PRV_CPSS_SIP_6_CHECK_MAC(devId))
        {
            nextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E;
        }
        else
        {
            nextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        }
    }

    /*For Bulk case, just save all the data and write and end.*/
    if (routeInfo)
    {
        override  = GT_TRUE;
        routeInfo->vrfId = vrId;
        routeInfo->type = pL3RouteEntry->type;
        memcpy(routeInfo->ipv4Addr, pL3RouteEntry->ipv4Addr, sizeof(ipv4Addr_t));
        memcpy(routeInfo->ipv6Addr, pL3RouteEntry->ipv6Addr, sizeof(ipv6Addr_t));
        routeInfo->override = (bool)override;
        routeInfo->ipMaskLen = pL3RouteEntry->ipMaskLen;
        memcpy(&(routeInfo->nextHopInfo), &nextHopInfo, sizeof(nextHopInfo));
        return XP_NO_ERR;
    }

    if (pL3RouteEntry->type == XP_PREFIX_TYPE_IPV4)
    {
        override  = GT_TRUE;
        memcpy(ipAddr.arIP, pL3RouteEntry->ipv4Addr, sizeof(ipv4Addr_t));
        prefixLen = pL3RouteEntry->ipMaskLen;

        st = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, &ipAddr, prefixLen,
                                          &nextHopInfo, override, defragmentationEnable);
        if ((status = xpsConvertCpssStatusToXPStatus(st))!=XP_NO_ERR)
        {
            // Print Log Message saying that the IPV4 Route Entry didnt get added.
            return status;
        }
    }

    else if (pL3RouteEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        override  = GT_TRUE;
        memcpy(ipv6Addr.arIP, pL3RouteEntry->ipv6Addr, sizeof(ipv6Addr_t));
        prefixLen = pL3RouteEntry->ipMaskLen;

        st = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, vrId, &ipv6Addr, prefixLen,
                                          &nextHopInfo, override, defragmentationEnable);
        if ((status = xpsConvertCpssStatusToXPStatus(st))!=XP_NO_ERR)
        {
            // Print Log Message saying that the IPV6 Route Entry didnt get added.
            return status;
        }
    }
    else
    {
        return XP_ERR_INVALID_PARAMS;
    }

    /*release old entry now*/
    if (updateNhMap)
    {
        xpsL3NextHopMapRemove(oldSwNhId, oldHwNhId);
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "Exit func :xpsL3UpdateIpRouteEntry  nhid %d ecmpSize %d",
          pL3RouteEntry->nhId, pL3RouteEntry->nhEcmpSize);
    return status;
}


XP_STATUS xpsL3UpdateL3VlanMember(xpsDevice_t devId, xpsVlan_t vlanId,
                                  xpsInterfaceId_t intfId, bool isIpv4, bool isIpv6)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS result = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsInterfaceId_t    l3IntfId;
    xpsInterfaceInfo_t *l3IntfInfo = NULL;
    uint32_t cpssDevId;
    uint32_t cpssPortNum ;

    l3IntfId = XPS_INTF_MAP_BD_TO_INTF(vlanId);
    result = xpsInterfaceGetInfoScope(devId, l3IntfId, &l3IntfInfo);
    if ((result != XP_NO_ERR) || (l3IntfInfo == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface information for L3 Interface id : %d\n", l3IntfId);
        return result;
    }

    cpssDevId = xpsGlobalIdToDevId(devId, intfId);
    cpssPortNum = xpsGlobalPortToPortnum(devId, intfId);

    rc = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                        CPSS_IP_PROTOCOL_IPV4_E, (GT_BOOL)isIpv4, GT_TRUE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to update ipv4 UC routing for port intf : %d \n", cpssPortNum);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                        CPSS_IP_PROTOCOL_IPV6_E, (GT_BOOL)isIpv6, GT_TRUE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to update ipv4 UC routing for port intf : %d \n", cpssPortNum);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalL3SetEgressRouterMac(cpssDevId, (GT_BOOL)false, cpssPortNum,
                                     l3IntfInfo->egressSAMac);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set egress mac to port intf : %d \n", cpssPortNum);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    return XP_NO_ERR;
}


XP_STATUS xpsL3UpdateLagMember(xpsDevice_t devId, xpsInterfaceId_t lagIntfId,
                               xpsInterfaceId_t portIntfId, bool add)
{
    GT_U8               cpssDevId;
    GT_U32              cpssPortNum;
    xpsDevice_t         portDevId = 0;
    xpPort_t            portId = 0;
    xpsScope_t          scopeId;
    XP_STATUS           result = XP_NO_ERR;
    xpsInterfaceId_t    l3IntfId;
    xpsInterfaceInfo_t *l3IntfInfo = NULL;
    uint32_t            v4Enable = FALSE;
    uint32_t            v6Enable = FALSE;
    uint32_t            v4MCEnable = FALSE;
    uint32_t            v6MCEnable = FALSE;
    GT_BOOL              isVlan = GT_FALSE;
    GT_STATUS cpssStatus = GT_OK;

    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        return result;
    }
    result = xpsLagGetL3IntfFromDbScope(scopeId, lagIntfId, &l3IntfId);
    if ((result != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get l3 information for lag Interface id : %d\n", lagIntfId);
        return result;
    }
    // No L3 enabled on LAG interface.
    if (l3IntfId == XPS_INTF_INVALID_ID)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_WARNING,
              "No l3 information for lag Interface id : %d\n", lagIntfId);
        return result;
    }

    /*l3 binding exists for Lag interface, procedd further for updating port attributes of new lag member*/
    result = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portIntfId, &portDevId,
                                                  &portId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get port and dev Id from port intf : %d \n", portIntfId);
        return result;
    }
    cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
    cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

    /*bind sequence*/
    result = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &l3IntfInfo);
    if ((result != XP_NO_ERR) || (l3IntfInfo == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface information for L3 Interface id : %d\n", l3IntfId);
        return result;
    }

    if (add == true)
    {
        /*can this be done better by having a SW state for UC/BC v4/v6 ?*/
        result = xpsL3GetIntfIpv4UcRoutingEn(devId, l3IntfId, &v4Enable);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get ipv4 UC enable status for intf : %d \n", l3IntfId);
            return result;
        }
        if (v4Enable == TRUE)
        {
            cpssStatus = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                        CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE, GT_FALSE);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to update uc routing en to port : %d \n", portIntfId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "IPv4 uc routing for port:%d DISABLED\n", portIntfId);
        }

        result = xpsL3GetIntfIpv6UcRoutingEn(devId, l3IntfId, &v6Enable);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get ipv6 UC enable status for intf : %d \n", l3IntfId);
            return result;
        }
        if (v6Enable == TRUE)
        {
            cpssStatus = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                        CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE, GT_FALSE);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to update uc routing en to port : %d \n", portIntfId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }

        result = xpsL3GetIntfIpv4McRoutingEn(devId, l3IntfId, &v4MCEnable);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get ipv4 MC enable status for intf : %d \n", l3IntfId);
            return result;
        }
        if (v4Enable == TRUE)
        {
            cpssStatus = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId, cpssPortNum,
                                                      CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to update mc routing en to port : %d \n", portIntfId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                  "IPv4 mc routing for port:%d DISABLED\n", portIntfId);
        }

        result = xpsL3GetIntfIpv6McRoutingEn(devId, l3IntfId, &v6MCEnable);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get ipv6 MC enable status for intf : %d \n", l3IntfId);
            return result;
        }
        if (v6Enable == TRUE)
        {
            cpssStatus = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId, cpssPortNum,
                                                      CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to update uc routing en to port : %d \n", portIntfId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "IPv6 mc routing for port:%d DISABLED\n", portIntfId);
        }

        cpssStatus = cpssHalL3BindPortIntf(cpssDevId, cpssPortNum, l3IntfInfo->bdHWId);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to bind l3 to port : %d \n", portIntfId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        cpssStatus = cpssHalL3SetEgressRouterMac(cpssDevId, isVlan, cpssPortNum,
                                                 l3IntfInfo->egressSAMac);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set egress mac to port intf : %d \n", portIntfId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        result = xpsVxlanUpdateNNIPortOnLagRIf(devId, l3IntfInfo, portIntfId, add);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsVxlanUpdateNNIPortOnLagRIf Failed : %d \n", portIntfId);
            return result;
        }
    }
    else
    {
        cpssStatus = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                    CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE, GT_FALSE);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to update uc routing en to port : %d \n", portIntfId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
        cpssStatus = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                    CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE, GT_FALSE);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to update uc routing en to port : %d \n", portIntfId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        cpssStatus = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId, cpssPortNum,
                                                  CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to update mc routing en to port : %d \n", portIntfId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        cpssStatus = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId, cpssPortNum,
                                                  CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to update mc routing en to port : %d \n", portIntfId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
        cpssStatus = cpssHalL3UnBindPortIntf(cpssDevId, cpssPortNum);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to unbind l3 to port intf : %d \n", portIntfId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        cpssStatus = cpssHalL3RemoveEgressRouterMac(cpssDevId, isVlan, cpssPortNum);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to remove egress mac for port intf : %d \n", portIntfId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        result = xpsVxlanUpdateNNIPortOnLagRIf(devId, l3IntfInfo, portIntfId, add);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsVxlanUpdateNNIPortOnLagRIf Failed : %d \n", portIntfId);
            return result;
        }
    }

    result = xpsAclCommonDropRuleUpdate(devId, cpssPortNum, add);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAclCommonDropRuleUpdate Failed : %d \n", cpssPortNum);
        return result;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsUpdateIpCtrlTrapOnL3Entry(xpsDevice_t xpsDevId,
                                       xpsCoppCtrlPkt_e pktType,
                                       xpsPktCmd_e pktCmd,
                                       xpsInterfaceId_t l3Intf,
                                       xpsInterfaceInfo_t *l3Info,
                                       bool isIntfDel)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    GT_BOOL enable = GT_FALSE;
    uint32_t vlanId;
    xpsScope_t scopeId;

    //get scope from Device ID
    result = xpsScopeGetScopeId(xpsDevId, &scopeId);
    if (result != XP_NO_ERR)
    {
        return result;
    }
    if (l3Intf != XPS_INTF_INVALID_ID && l3Info == NULL)
    {
        result = xpsInterfaceGetInfoScope(scopeId, l3Intf, &l3Info);

        if ((result != XP_NO_ERR) || (l3Info == NULL))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get interface info for Interface id : %d\n", l3Intf);
            return result;
        }

    }
    else if (l3Intf == XPS_INTF_INVALID_ID && l3Info != NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "Valid L3 Info \n");
    }
    else
    {
        return XP_ERR_NULL_POINTER;
    }

    if (l3Info->type == XPS_PORT_ROUTER)
    {
        vlanId = l3Info->bdHWId;
    }
    else
    {
        vlanId = l3Info->keyIntfId;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "Vlan : %d pktTye : %d pktCmd : %d \n",
          vlanId, pktType, pktCmd);

    if (isIntfDel == false)
    {
        enable = GT_TRUE;
    }

    if (pktType == XPS_COPP_CTRL_PKT_ARP_RESPONSE)
    {
        /*
           TRAP/COPY_TO_CPU - Set action as TRAP, as it is UC pkt.
           FWD - Set action as TRAP and DROP it in CPU, as it is invalid to flood known UC pkt.
           DROP - Set action as TRAP and DROP it in CPU
           NOTE: Drop is handled by trapping the packets and setting
           the statistical index value to 0 for ARP_REPLY cpu code.
         */
        if (isIntfDel == true)
        {
            pktCmd = XP_PKTCMD_FWD;
        }
        else
        {
            pktCmd = XP_PKTCMD_TRAP;
        }
    }

    result = xpsCoppEnableCtrlPktTrapOnVlan(xpsDevId, pktType, vlanId,
                                            l3Info->egressSAMac, pktCmd, (uint8_t)enable);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed CtrlPktTrap : %d result :%d \n", pktType, result);
        return result;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsL3IntfStatsReadForNon64Bit(xpsDevice_t xpsDevId,
                                        xpsInterfaceId_t l3IntfId,
                                        xpsL3StatsInfo_t *xpsRifStatistics)
{
    GT_STATUS rc = GT_OK;
    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t *l3IntfInfo = NULL;
    xpsScope_t scopeId;
    uint32_t eVlanId = 0;
    uint32_t ingVlanIdPassIdx = 0;
    uint32_t ingVlanIdDropIdx = 0;
    uint32_t egrVlanIdPassIdx = 0;
    uint32_t egrVlanIdDropIdx = 0;
    uint64_t l3IngVlanPktCnt = 0;
    uint64_t l3IngVlanByteCnt = 0;
    //    uint64_t ingVlanPassPktCnt = 0;
    //    uint64_t ingVlanPassByteCnt = 0;
    uint64_t ingVlanDropPktCnt = 0;
    uint64_t ingVlanDropByteCnt = 0;
    uint64_t egrVlanPassPktCnt = 0;
    uint64_t egrVlanPassByteCnt = 0;
    uint64_t egrVlanDropPktCnt = 0;
    uint64_t egrVlanDropByteCnt = 0;
    uint32_t pos = 0;
    uint32_t isFound = 0;
    uint32_t blockStart = 0;
    int eVlanIdxBase = 0;
    uint32_t egrPassOffset = 0;
    uint32_t egrDropOffset = 0;


    if ((status = xpsScopeGetScopeId(xpsDevId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &l3IntfInfo);
    if ((status != XP_NO_ERR) || (l3IntfInfo == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface information for L3 Interface id : %d\n", l3IntfId);
        return status;
    }
    if (l3IntfInfo->type == XPS_VLAN_ROUTER)
    {
        eVlanId = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);
    }
    else
    {
        eVlanId = l3IntfInfo->bdHWId;
    }

    for (pos = 0 ; pos < 5 ; pos++)
    {
        eVlanIdxBase = GetL3CounterIdBase(pos);
        if ((eVlanIdxBase == -1) || (eVlanId < (uint32_t)eVlanIdxBase) ||
            (eVlanId >= ((uint32_t)eVlanIdxBase+XPS_CPSS_CNC_COUNTER_PER_BLK)))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "Invalid Counter Range :%d for HwId :%d \n", eVlanIdxBase, eVlanId);
            continue;
        }
        else
        {
            isFound = 1;
            if (cpssHalDevPPFamilyGet(xpsDevId) == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
            {
                blockStart = ALDRIN2_RIF_CNC_BLOCK_START +
                             (pos*MAX_RIF_CLINETS_NON_64_BIT_MODE);
            }
            else if (cpssHalDevPPFamilyGet(xpsDevId) == CPSS_PP_FAMILY_DXCH_FALCON_E)
            {
                blockStart = FALCON_RIF_CNC_BLOCK_START +
                             (pos*MAX_RIF_CLINETS_NON_64_BIT_MODE);
            }
            else if (cpssHalDevPPFamilyGet(xpsDevId) == CPSS_PP_FAMILY_DXCH_AC5X_E)
            {
                blockStart = AC5X_RIF_CNC_BLOCK_START +
                             (pos*MAX_RIF_CLINETS_NON_64_BIT_MODE);
            }
            else
            {
                blockStart = CPSS_CNC_BLOCK_1;
            }

            break;
        }
    }

    if (!isFound)
    {
        return XP_NO_ERR;
    }

    ingVlanIdPassIdx = (eVlanId % XPS_CPSS_CNC_COUNTER_PER_BLK);
    ingVlanIdDropIdx = ((1 << 14) | eVlanId)%XPS_CPSS_CNC_COUNTER_PER_BLK;

    if (cpssHalDevPPFamilyGet(xpsDevId) == CPSS_PP_FAMILY_DXCH_FALCON_E ||
        cpssHalDevPPFamilyGet(xpsDevId) == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        egrVlanIdPassIdx = ((eVlanId << 1)% XPS_CPSS_CNC_COUNTER_PER_BLK);
        egrVlanIdDropIdx = (((eVlanId << 1) | 1)% XPS_CPSS_CNC_COUNTER_PER_BLK);
        if (eVlanId < (uint32_t)(eVlanIdxBase+512))
        {
            egrPassOffset = CNC_CLIENT_EGRESS_VLAN_PASS_BLK_OFFSET;
            egrDropOffset = CNC_CLIENT_EGRESS_VLAN_PASS_BLK_OFFSET;
        }
        else
        {
            egrPassOffset = CNC_CLIENT_EGRESS_VLAN_DROP_BLK_OFFSET;
            egrDropOffset = CNC_CLIENT_EGRESS_VLAN_DROP_BLK_OFFSET;
        }
    }
    else
    {
        egrVlanIdPassIdx = ingVlanIdPassIdx;
        egrVlanIdDropIdx = ingVlanIdDropIdx;
        egrPassOffset = CNC_CLIENT_EGRESS_VLAN_PASS_BLK_OFFSET;
        egrDropOffset = CNC_CLIENT_EGRESS_VLAN_DROP_BLK_OFFSET;
    }

    /* Get Stats from L2/L3 Ingress Vlan client */
    rc = cpssHalCncCounterGet(xpsDevId,
                              (blockStart + CNC_CLIENT_L2L3_INGRESS_VLAN_BLK_OFFSET),
                              ingVlanIdPassIdx,
                              CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E,
                              &l3IngVlanPktCnt, &l3IngVlanByteCnt, NULL);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "CNC_BLOCK_1 CounterGet Failed :%d for HwId :%d \n", eVlanIdxBase, eVlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Get Stats from Ingress Pass/Drop Vlan client for Drop Block */
    rc = cpssHalCncCounterGet(xpsDevId,
                              (blockStart + CNC_CLIENT_INGRESS_VLAN_DROP_BLK_OFFSET),
                              ingVlanIdDropIdx,
                              CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E,
                              &ingVlanDropPktCnt, &ingVlanDropByteCnt, NULL);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "CNC_BLOCK_2 CounterGet Failed :%d for HwId :%d \n", eVlanIdxBase, eVlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Get Stats from Egress Pass/Drop Vlan client for Pass Block */
    rc = cpssHalCncCounterGet(xpsDevId,
                              (blockStart + egrPassOffset),
                              egrVlanIdPassIdx,
                              CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E,
                              &egrVlanPassPktCnt, &egrVlanPassByteCnt, NULL);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "CNC_BLOCK_3 CounterGet Failed :%d for HwId :%d \n", eVlanIdxBase, eVlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Get Stats from Egress Pass/Drop Vlan client for Drop Block */
    rc = cpssHalCncCounterGet(xpsDevId,
                              (blockStart + egrDropOffset),
                              egrVlanIdDropIdx,
                              CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E,
                              &egrVlanDropPktCnt, &egrVlanDropByteCnt, NULL);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "CNC_BLOCK_4 CounterGet Failed :%d for HwId :%d \n", eVlanIdxBase, eVlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    xpsRifStatistics->rxPkts = (l3IngVlanPktCnt - ingVlanDropPktCnt);
    xpsRifStatistics->rxErrPkts = ingVlanDropPktCnt;
    xpsRifStatistics->rxOctets = (l3IngVlanByteCnt - ingVlanDropByteCnt);
    xpsRifStatistics->rxErrOctets = ingVlanDropByteCnt;
    xpsRifStatistics->txPkts = egrVlanPassPktCnt;
    xpsRifStatistics->txOctets = egrVlanPassByteCnt;
    xpsRifStatistics->txErrPkts = egrVlanDropPktCnt;
    xpsRifStatistics->txErrOctets = egrVlanDropByteCnt;

    return XP_NO_ERR;
}

XP_STATUS xpsL3IntfStatsReadFor64Bit(xpsDevice_t xpsDevId,
                                     xpsInterfaceId_t l3IntfId,
                                     xpsL3StatsInfo_t *xpsRifStatistics)
{
    GT_STATUS rc = GT_OK;
    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t *l3IntfInfo = NULL;
    xpsScope_t scopeId;
    uint32_t eVlanId = 0;
    uint32_t ingVlanIdPassIdx = 0;
    uint32_t ingVlanIdDropIdx = 0;
    uint32_t egrVlanIdPassIdx = 0;
    uint32_t egrVlanIdDropIdx = 0;
    uint64_t l3IngVlanPktCnt = 0;
    uint64_t l3IngVlanByteCnt = 0;
    //    uint64_t ingVlanPassPktCnt = 0;
    //    uint64_t ingVlanPassByteCnt = 0;
    uint64_t ingVlanDropPktCnt = 0;
    uint64_t ingVlanDropByteCnt = 0;
    uint64_t egrVlanPassPktCnt = 0;
    uint64_t egrVlanPassByteCnt = 0;
    uint64_t egrVlanDropPktCnt = 0;
    uint64_t egrVlanDropByteCnt = 0;
    uint32_t pos = 0;
    uint32_t isFound = 0;
    uint32_t blockStart = 0;
    int eVlanIdxBase = 0;
    uint32_t  egrPassPktBlkOffset = 0;
    uint32_t egrDropPktBlkOffset = 0;
    uint32_t egrPassByteBlkOffset = 0;
    uint32_t egrDropByteBlkOffset = 0;
    CPSS_PP_FAMILY_TYPE_ENT ppFamily = cpssHalDevPPFamilyGet(xpsDevId);

    if (ppFamily != CPSS_PP_FAMILY_DXCH_FALCON_E &&
        ppFamily != CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        return XP_NO_ERR;
    }

    if ((status = xpsScopeGetScopeId(xpsDevId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    status = xpsInterfaceGetInfoScope(scopeId, l3IntfId, &l3IntfInfo);
    if ((status != XP_NO_ERR) || (l3IntfInfo == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface information for L3 Interface id : %d\n", l3IntfId);
        return status;
    }
    if (l3IntfInfo->type == XPS_VLAN_ROUTER)
    {
        eVlanId = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);
    }
    else
    {
        eVlanId = l3IntfInfo->bdHWId;
    }

    for (pos = 0 ; pos < 5 ; pos++)
    {
        eVlanIdxBase = GetL3CounterIdBase(pos);
        if ((eVlanIdxBase == -1) || (eVlanId < (uint32_t)eVlanIdxBase) ||
            (eVlanId >= ((uint32_t)eVlanIdxBase+XPS_CPSS_CNC_COUNTER_PER_BLK)))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "Invalid Counter Range :%d for HwId :%d \n", eVlanIdxBase, eVlanId);
            continue;
        }
        else
        {
            isFound = 1;
            if (ppFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
            {
                blockStart = FALCON_RIF_CNC_BLOCK_START +
                             (pos*MAX_RIF_CLINETS_64_BIT_MODE);
            }
            else
            {
                blockStart = AC5X_RIF_CNC_BLOCK_START +
                             (pos*MAX_RIF_CLINETS_64_BIT_MODE);
            }

            break;
        }
    }
    if (!isFound)
    {
        return XP_NO_ERR;
    }

    ingVlanIdPassIdx = (eVlanId % XPS_CPSS_CNC_COUNTER_PER_BLK);
    ingVlanIdDropIdx = ((1 << 14) | eVlanId)%XPS_CPSS_CNC_COUNTER_PER_BLK;

    if (ppFamily == CPSS_PP_FAMILY_DXCH_FALCON_E ||
        ppFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        egrVlanIdPassIdx = ((eVlanId << 1)% XPS_CPSS_CNC_COUNTER_PER_BLK);
        egrVlanIdDropIdx = (((eVlanId << 1) | 1)% XPS_CPSS_CNC_COUNTER_PER_BLK);
        if (eVlanId < (uint32_t)(eVlanIdxBase+512))
        {
            egrPassPktBlkOffset = CNC_CLIENT_EGRESS_VLAN_PASS_PKT_BLK_OFFSET;
            egrDropPktBlkOffset = CNC_CLIENT_EGRESS_VLAN_PASS_PKT_BLK_OFFSET;
            egrPassByteBlkOffset = CNC_CLIENT_EGRESS_VLAN_PASS_BYTE_BLK_OFFSET;
            egrDropByteBlkOffset = CNC_CLIENT_EGRESS_VLAN_PASS_BYTE_BLK_OFFSET;
        }
        else
        {
            egrPassPktBlkOffset = CNC_CLIENT_EGRESS_VLAN_DROP_PKT_BLK_OFFSET;
            egrDropPktBlkOffset = CNC_CLIENT_EGRESS_VLAN_DROP_PKT_BLK_OFFSET;
            egrPassByteBlkOffset = CNC_CLIENT_EGRESS_VLAN_DROP_BYTE_BLK_OFFSET;
            egrDropByteBlkOffset = CNC_CLIENT_EGRESS_VLAN_DROP_BYTE_BLK_OFFSET;
        }
    }
    else
    {
        egrVlanIdPassIdx = ingVlanIdPassIdx;
        egrVlanIdDropIdx = ingVlanIdDropIdx;
        egrPassPktBlkOffset = CNC_CLIENT_EGRESS_VLAN_PASS_PKT_BLK_OFFSET;
        egrDropPktBlkOffset = CNC_CLIENT_EGRESS_VLAN_DROP_PKT_BLK_OFFSET;
        egrPassByteBlkOffset = CNC_CLIENT_EGRESS_VLAN_PASS_BYTE_BLK_OFFSET;
        egrDropByteBlkOffset = CNC_CLIENT_EGRESS_VLAN_DROP_BYTE_BLK_OFFSET;
    }

    /* Get Stats from L2/L3 Ingress Vlan client for PKt */
    rc = cpssHalCncCounterGet(xpsDevId,
                              (blockStart + CNC_CLIENT_L2L3_INGRESS_VLAN_PKT_BLK_OFFSET),
                              ingVlanIdPassIdx,
                              CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E,
                              &l3IngVlanPktCnt, NULL, NULL);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "CNC_CLIENT_L2L3 Pkt CounterGet Failed :%d for HwId :%d \n", eVlanIdxBase,
              eVlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Get Stats from L2/L3 Ingress Vlan client for Byte */
    rc = cpssHalCncCounterGet(xpsDevId,
                              (blockStart + CNC_CLIENT_L2L3_INGRESS_VLAN_BYTE_BLK_OFFSET),
                              ingVlanIdPassIdx,
                              CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E,
                              NULL, &l3IngVlanByteCnt, NULL);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "CNC_CLIENT_L2L3 Byte CounterGet Failed :%d for HwId :%d \n", eVlanIdxBase,
              eVlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Get Stats from Ingress Pass/Drop Vlan client for Drop Block (Pkt) */
    rc = cpssHalCncCounterGet(xpsDevId,
                              (blockStart + CNC_CLIENT_INGRESS_VLAN_DROP_PKT_BLK_OFFSET),
                              ingVlanIdDropIdx,
                              CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E,
                              &ingVlanDropPktCnt, NULL, NULL);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " INGRESS_VLAN_DROP Pkt CounterGet Failed :%d for HwId :%d \n", eVlanIdxBase,
              eVlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Get Stats from Ingress Pass/Drop Vlan client for Drop Block (Byte) */
    rc = cpssHalCncCounterGet(xpsDevId,
                              (blockStart + CNC_CLIENT_INGRESS_VLAN_DROP_BYTE_BLK_OFFSET),
                              ingVlanIdDropIdx,
                              CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E,
                              NULL, &ingVlanDropByteCnt, NULL);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " INGRESS_VLAN_DROP Byte CounterGet Failed :%d for HwId :%d \n", eVlanIdxBase,
              eVlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }
    /* Get Stats from Egress Pass/Drop Vlan client for Pass Block (Pkt) */
    rc = cpssHalCncCounterGet(xpsDevId,
                              (blockStart + egrPassPktBlkOffset),
                              egrVlanIdPassIdx,
                              CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E,
                              &egrVlanPassPktCnt, NULL, NULL);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "EGRESS_VLAN_PASS Pkt CounterGet Failed :%d for HwId :%d \n", eVlanIdxBase,
              eVlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Get Stats from Egress Pass/Drop Vlan client for Drop Block (Pkt)*/
    rc = cpssHalCncCounterGet(xpsDevId,
                              (blockStart + egrDropPktBlkOffset),
                              egrVlanIdDropIdx,
                              CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E,
                              &egrVlanDropPktCnt, NULL, NULL);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "EGRESS_VLAN_DROP Pkt CounterGet Failed :%d for HwId :%d \n", eVlanIdxBase,
              eVlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }
    /* Get Stats from Egress Pass/Drop Vlan client for Pass Block (Byte)*/
    rc = cpssHalCncCounterGet(xpsDevId,
                              (blockStart + egrPassByteBlkOffset),
                              egrVlanIdPassIdx,
                              CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E,
                              NULL, &egrVlanPassByteCnt, NULL);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "EGRESS_VLAN_PASS Byte CounterGet Failed :%d for HwId :%d \n", eVlanIdxBase,
              eVlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Get Stats from Egress Pass/Drop Vlan client for Drop Block (Byte)*/
    rc = cpssHalCncCounterGet(xpsDevId,
                              (blockStart + egrDropByteBlkOffset),
                              egrVlanIdDropIdx,
                              CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E,
                              NULL, &egrVlanDropByteCnt, NULL);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "EGRESS_VLAN_DROP Byte CounterGet Failed :%d for HwId :%d \n", eVlanIdxBase,
              eVlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    xpsRifStatistics->rxPkts = (l3IngVlanPktCnt - ingVlanDropPktCnt);
    xpsRifStatistics->rxErrPkts = ingVlanDropPktCnt;
    xpsRifStatistics->rxOctets = (l3IngVlanByteCnt - ingVlanDropByteCnt);
    xpsRifStatistics->rxErrOctets = ingVlanDropByteCnt;
    xpsRifStatistics->txPkts = egrVlanPassPktCnt;
    xpsRifStatistics->txOctets = egrVlanPassByteCnt;
    xpsRifStatistics->txErrPkts = egrVlanDropPktCnt;
    xpsRifStatistics->txErrOctets = egrVlanDropByteCnt;

    return XP_NO_ERR;
}
XP_STATUS xpsL3IntfStatsRead(xpsDevice_t xpsDevId,
                             xpsInterfaceId_t l3IntfId,
                             xpsL3StatsInfo_t *xpsRifStatistics)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    XP_DEV_TYPE_T devType;
    cpssHalGetDeviceType(xpsDevId, &devType);

    if ((IS_DEVICE_FALCON(devType) &&
         !(IS_DEVICE_FALCON_10G_ONLY_PROFILE(devType) ||
           IS_DEVICE_FALCON_25G_ONLY_PROFILE(devType) ||
           IS_DEVICE_FALCON_10G_25G_ONLY_PROFILE(devType))))
    {
        xpStatus = xpsL3IntfStatsReadFor64Bit(xpsDevId, l3IntfId, xpsRifStatistics);
        if (xpStatus != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: Failed to get xps rif statistic data, xpStatus: %d\n", xpStatus);
            return xpStatus;
        }
    }
    else
    {
        xpStatus = xpsL3IntfStatsReadForNon64Bit(xpsDevId, l3IntfId, xpsRifStatistics);
        if (xpStatus != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: Failed to get xps rif statistic data, xpStatus: %d\n", xpStatus);
            return xpStatus;
        }
    }
    return xpStatus;
}

XP_STATUS xpsL3VrfCreate(xpDevice_t devId, uint32_t vrfId,
                         uint32_t *ucNhId, uint32_t *mcNhId)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC               vrConfigInfo;
    uint32_t defUcNhId = XPS_l3_DEFAULT_UC_NEXTHOP;
    uint32_t defMcNhId = XPS_l3_DEFAULT_MC_NEXTHOP;
    uint32_t lpmDbId = XPS_CPSS_DEFAULT_LPM_DB_ID;
    xpsScope_t scopeId;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC cpssNhEntry;
    CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC routeEntryPtr;
    memset(&cpssNhEntry, 0, sizeof(cpssNhEntry));
    memset(&routeEntryPtr, 0, sizeof(CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC));

    /*Add a Default Virtual Router into the LPM DB thats created*/
    cpssOsBzero((GT_CHAR*) &vrConfigInfo, sizeof(CPSS_DXCH_IP_LPM_VR_CONFIG_STC));
    xpsIpRouteDefaultVrfConfigSet(devId, vrfId, &vrConfigInfo);

    if (vrfId > XPS_L3_DEFAULT_VRF)
    {
        if ((xpStatus = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
        {
            return xpStatus;
        }

        /*No SAI attr. For now return default-vrf UC NH id for all non-default Vrfs to SAI. */
#ifdef DEFAULT_UC_VRF
        /* Allocate NH-idx for default UC for non-global VRF (non-zero)*/
        xpStatus = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_L3_NEXT_HOP_TBL,
                                          &defUcNhId);
        if (xpStatus != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsAllocatorAllocateId Failed : %d\n", xpStatus);
            return xpStatus;
        }
#endif
        /* Allocate NH-idx for default MC for non-global VRF (non-zero)*/
        xpStatus = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_L3_NEXT_HOP_TBL,
                                          &defMcNhId);
        if (xpStatus != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsAllocatorAllocateId Failed : %d\n", xpStatus);
            return xpStatus;
        }

        vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = defUcNhId;
        vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry.routeEntryBaseIndex = defUcNhId;

        vrConfigInfo.defIpv4McRouteLttEntry.routeEntryBaseIndex = defMcNhId;
        vrConfigInfo.defIpv6McRouteLttEntry.routeEntryBaseIndex = defMcNhId;
    }

    rc  = cpssHalIpLpmVirtualRouterAdd(lpmDbId, vrfId, &vrConfigInfo);
    if ((xpStatus = xpsConvertCpssStatusToXPStatus(rc))!=XP_NO_ERR)
    {
        if (xpStatus == XP_ERR_KEY_EXISTS)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "VRF Id  = %d Already Added to Lpm DB Id = %d \n",
                  lpmDbId, vrfId);
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "VRF Id  = %d Couldnt be Added to Lpm DB Id = %d rc :%d\n",
                  lpmDbId, vrfId, rc);
            return xpStatus;
        }
    }

    if (vrfId > XPS_L3_DEFAULT_VRF)
    {
        cpssNhEntry.entry.regularEntry.cmd = CPSS_PACKET_CMD_DROP_HARD_E;
        rc = cpssHalWriteIpUcRouteEntries(devId, defUcNhId, &cpssNhEntry, 1);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to add default UC route nexthop entry :%d ", rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

        routeEntryPtr.cmd = CPSS_PACKET_CMD_DROP_HARD_E;
        rc = cpssHalIpMcRouteEntriesWrite(devId, defMcNhId,
                                          &routeEntryPtr);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " DefMC Entry cpssHalIpMcRouteEntriesWrite failed rc : %d ", rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

    }
    *ucNhId = defUcNhId;
    *mcNhId = defMcNhId;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "VRF Id  = %d Successfully Added to Lpm DB Id = %d \n",
          lpmDbId, vrfId);

    return xpStatus;

}

XP_STATUS xpsL3VrfRemove(xpDevice_t devId, uint32_t vrfId,
                         uint32_t ucNhId, uint32_t mcNhId)
{
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    uint32_t lpmDbId = XPS_CPSS_DEFAULT_LPM_DB_ID;

    rc = cpssDxChIpLpmVirtualRouterDel(lpmDbId, vrfId);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Remove Failed VRF Id  = %d to Lpm DB Id = %d rc :%d\n",
              lpmDbId, vrfId, rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    if (vrfId > XPS_L3_DEFAULT_VRF)
    {
#ifdef DEFAULT_UC_VRF
        status = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XPS_ALLOCATOR_L3_NEXT_HOP_TBL,
                                       ucNhId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to release VRF UC NH table index %d. ret %d", ucNhId, status);
            return status;
        }
#endif
        status = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XPS_ALLOCATOR_L3_NEXT_HOP_TBL,
                                       mcNhId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to release VRF MC NH table index %d. ret %d", ucNhId, status);
            return status;
        }
    }
    return status;
}

XP_STATUS xpsL3GetMaxVrf(xpDevice_t devId, uint32_t *vrfSize)
{
    XP_STATUS status = XP_NO_ERR;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    *vrfSize = cpssHalGetSKUmaxVRF(devId);

    return status;

}

XP_STATUS xpsL3UpdateMcNHPktAction(xpDevice_t devId, uint32_t mcNhId,
                                   xpsPktCmd_e cmd)
{
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    CPSS_PACKET_CMD_ENT cpssPktCmd = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC routeEntryPtr;
    memset(&routeEntryPtr, 0, sizeof(CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC));


    rc = cpssHalIpMcRouteEntriesRead(devId, mcNhId,
                                     &routeEntryPtr);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " MC Entry Read failed rc : %d ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    cpssPktCmd = xpsConvertPktCmdToCpssPktCmd(cmd);
    if (cpssPktCmd == CPSS_PACKET_CMD_MIRROR_TO_CPU_E)
    {
        cpssPktCmd = CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
    }
    else if (cpssPktCmd == CPSS_PACKET_CMD_FORWARD_E)
    {
        cpssPktCmd = CPSS_PACKET_CMD_ROUTE_E;
    }

    routeEntryPtr.cmd = cpssPktCmd;
    rc = cpssHalIpMcRouteEntriesWrite(devId, mcNhId,
                                      &routeEntryPtr);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalIpMcRouteEntriesWrite failed rc : %d ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    return status;
}

XP_STATUS xpsL3PbrEntrySet(xpDevice_t devId, uint32_t hwLeafId, uint32_t nhId,
                           bool isNhGrp)
{
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;
    CPSS_DXCH_LPM_LEAF_ENTRY_STC leafEntry;
    memset(&leafEntry, 0, sizeof(leafEntry));

    leafEntry.index = nhId;
    leafEntry.applyPbr = GT_TRUE;;

    if (isNhGrp)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devId))
        {
            leafEntry.entryType = CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E;
        }
        else
        {
            leafEntry.entryType = CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E;
        }
    }
    else
    {
        leafEntry.entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
    }
    cpssStatus = cpssHalLpmLeafEntryWrite(devId, hwLeafId, &leafEntry);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalLpmLeafEntryWrite failed hwLeafId %d rc %d ", hwLeafId, cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
    return status;
}

XP_STATUS xpsL3RouteBulkAdd(xpsL3RouteBulkInfo_t *routeInfo, uint32_t count)
{
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    uint32_t v4Count = 0;
    uint32_t v6Count = 0;
    GT_U32 lpmDBId     = XPS_CPSS_DEFAULT_LPM_DB_ID;
    GT_BOOL defragmentationEnable = GT_TRUE;

    CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC *v4Ptr = NULL;
    CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC *v6Ptr = NULL;

    if (!count || !routeInfo)
    {
        return XP_ERR_INVALID_ARG;
    }

    v4Ptr = (CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC*)xpMalloc(sizeof(
                                                               CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC)*count);
    if (NULL == v4Ptr)
    {
        return XP_ERR_OUT_OF_MEM;
    }

    v6Ptr = (CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC*)xpMalloc(sizeof(
                                                               CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC)*count);
    if (NULL == v6Ptr)
    {
        return XP_ERR_OUT_OF_MEM;
    }

    uint32_t v4ObjIdx[count] = {0};
    uint32_t v6ObjIdx[count] = {0};
    uint32_t i = 0;
    uint32_t objIdx = 0;

    for (i = 0; i < count; i++)
    {
        if (routeInfo[i].retStatus == 0xFFFFFFFF)
        {
            continue;
        }
        if (routeInfo[i].type == XP_PREFIX_TYPE_IPV4)
        {
            v4Ptr[v4Count].vrId = routeInfo[i].vrfId;
            v4Ptr[v4Count].override = (GT_BOOL)routeInfo[i].override;
            v4Ptr[v4Count].prefixLen = routeInfo[i].ipMaskLen;
            memcpy(v4Ptr[v4Count].ipAddr.arIP, routeInfo[i].ipv4Addr, sizeof(ipv4Addr_t));
            memcpy(&v4Ptr[v4Count].nextHopInfo, &routeInfo[i].nextHopInfo,
                   sizeof(v4Ptr[v4Count].nextHopInfo));
            v4ObjIdx[v4Count] = i;
            v4Count++;
        }
        else
        {
            v6Ptr[v6Count].vrId = routeInfo[i].vrfId;
            v6Ptr[v6Count].override = (GT_BOOL)routeInfo[i].override;
            v6Ptr[v6Count].prefixLen = routeInfo[i].ipMaskLen;
            memcpy(v6Ptr[v6Count].ipAddr.arIP, routeInfo[i].ipv6Addr, sizeof(ipv6Addr_t));
            memcpy(&v6Ptr[v6Count].nextHopInfo, &routeInfo[i].nextHopInfo,
                   sizeof(v6Ptr[v6Count].nextHopInfo));
            v6ObjIdx[v6Count] = i;
            v6Count++;
        }
    }

    if (v4Count)
    {
        rc = cpssDxChIpLpmIpv4UcPrefixAddBulk(lpmDBId, v4Count, v4Ptr);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "V4 BulkAdd failed %d \n", rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                  "V4 BulkAdd success count=%d \n", v4Count);
        }
        for (i = 0; i < v4Count; i++)
        {
            objIdx = v4ObjIdx[i];
            if (v4Ptr[i].returnStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "V4BulkAdd failed ret %d prefixLen %d, prefix " FMT_IP4" \n",
                      v4Ptr[i].returnStatus, v4Ptr[i]. prefixLen,
                      PRI_IP4(v4Ptr[i].ipAddr.arIP));
                /*Try Single write with defrag option*/
                rc = cpssDxChIpLpmIpv4UcPrefixAdd(lpmDBId, v4Ptr[i].vrId, &(v4Ptr[i].ipAddr),
                                                  v4Ptr[i].prefixLen,
                                                  &(v4Ptr[i].nextHopInfo),
                                                  v4Ptr[i].override, defragmentationEnable);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "V4 BulkAdd SinglePrefix failed %d prefix " FMT_IP4 "\n",
                          rc, PRI_IP4(v4Ptr[i].ipAddr.arIP));
                    routeInfo[objIdx].retStatus = rc;
                }
                else
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                          "V4BulkAdd success ret %d prefixLen %d, prefix " FMT_IP4" \n",
                          v4Ptr[i].returnStatus, v4Ptr[i].prefixLen,
                          PRI_IP4(v4Ptr[i].ipAddr.arIP));
                }
            }
            routeInfo[objIdx].retStatus = v4Ptr[i].returnStatus;
        }
    }

    if (v6Count)
    {
        rc = cpssDxChIpLpmIpv6UcPrefixAddBulk(lpmDBId, v6Count, v6Ptr, GT_FALSE);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "V6 BulkAdd failed %d \n", rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                  "V6 BulkAdd success count=%d \n", v6Count);
        }

        for (i = 0; i < v6Count; i++)
        {
            objIdx = v6ObjIdx[i];
            if (v6Ptr[i].returnStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "V6BulkAdd failed ret %d prefixLen %d, prefix " FMT_IP6 " \n",
                      v6Ptr[i].returnStatus, v6Ptr[i]. prefixLen,
                      PRI_IP6(v6Ptr[i].ipAddr.arIP));
                /*Try Single write with defrag option*/
                rc = cpssDxChIpLpmIpv6UcPrefixAdd(lpmDBId, v6Ptr[i].vrId, &(v6Ptr[i].ipAddr),
                                                  v6Ptr[i].prefixLen,
                                                  &(v6Ptr[i].nextHopInfo),
                                                  v6Ptr[i].override, defragmentationEnable);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "V6 BulkAdd SinglePrefix failed %d \n", rc);
                    routeInfo[objIdx].retStatus = rc;
                }
                else
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                          "V6BulkAdd success ret %d prefixLen %d, prefix " FMT_IP6 " \n",
                          v6Ptr[i].returnStatus, v6Ptr[i].prefixLen,
                          PRI_IP6(v6Ptr[i].ipAddr.arIP));
                }
            }
            routeInfo[objIdx].retStatus = v6Ptr[i].returnStatus;
        }
    }
    if (v4Ptr)
    {
        xpFree(v4Ptr);
    }

    if (v6Ptr)
    {
        xpFree(v6Ptr);
    }
    return status;
}

XP_STATUS xpsL3RouteBulkDel(xpsL3RouteBulkInfo_t *routeInfo, uint32_t count)
{
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    uint32_t v4Count = 0;
    uint32_t v6Count = 0;
    GT_U32 lpmDBId     = XPS_CPSS_DEFAULT_LPM_DB_ID;

    CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC *v4Ptr = NULL;
    CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC *v6Ptr = NULL;

    if (!count || !routeInfo)
    {
        return XP_ERR_INVALID_ARG;
    }

    v4Ptr = (CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC*)xpMalloc(sizeof(
                                                               CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC)*count);
    if (NULL == v4Ptr)
    {
        return XP_ERR_OUT_OF_MEM;
    }

    v6Ptr = (CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC*)xpMalloc(sizeof(
                                                               CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC)*count);
    if (NULL == v6Ptr)
    {
        return XP_ERR_OUT_OF_MEM;
    }

    uint32_t v4ObjIdx[count] = {0};
    uint32_t v6ObjIdx[count] = {0};
    uint32_t i = 0;
    uint32_t objIdx = 0;

    for (i = 0; i < count; i++)
    {
        if (routeInfo[i].retStatus == 0xFFFFFFFF)
        {
            continue;
        }
        if (routeInfo[i].type == XP_PREFIX_TYPE_IPV4)
        {
            v4Ptr[v4Count].vrId = routeInfo[i].vrfId;
            v4Ptr[v4Count].override = (GT_BOOL)routeInfo[i].override;
            v4Ptr[v4Count].prefixLen = routeInfo[i].ipMaskLen;
            memcpy(v4Ptr[v4Count].ipAddr.arIP, routeInfo[i].ipv4Addr, sizeof(ipv4Addr_t));
            v4ObjIdx[v4Count] = i;
            v4Count++;
        }
        else
        {
            v6Ptr[v6Count].vrId = routeInfo[i].vrfId;
            v6Ptr[v6Count].override = (GT_BOOL)routeInfo[i].override;
            v6Ptr[v6Count].prefixLen = routeInfo[i].ipMaskLen;
            memcpy(v6Ptr[v6Count].ipAddr.arIP, routeInfo[i].ipv6Addr, sizeof(ipv6Addr_t));
            v6ObjIdx[v6Count] = i;
            v6Count++;
        }
    }

    if (v4Count)
    {
        rc = cpssDxChIpLpmIpv4UcPrefixDelBulk(lpmDBId, v4Count, v4Ptr);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "V4 BulkDel failed %d \n", rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                  "V4 BulkDel success count=%d \n", v4Count);
        }

        for (i = 0; i < v4Count; i++)
        {
            objIdx = v4ObjIdx[i];
            routeInfo[objIdx].retStatus = v4Ptr[i].returnStatus;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "V4BulkDel success ret %d prefixLen %d, prefix " FMT_IP4" \n",
                  v4Ptr[i].returnStatus, v4Ptr[i].prefixLen,
                  PRI_IP4(v4Ptr[i].ipAddr.arIP));
        }
    }

    if (v6Count)
    {
        rc = cpssDxChIpLpmIpv6UcPrefixDelBulk(lpmDBId, v6Count, v6Ptr);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "V6 BulkDel failed %d \n", rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                  "V6 BulkDel success count=%d \n", v6Count);
        }

        for (i = 0; i < v6Count; i++)
        {
            objIdx = v6ObjIdx[i];
            routeInfo[objIdx].retStatus = v6Ptr[i].returnStatus;

            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "V6BulkDel success ret %d prefixLen %d, prefix " FMT_IP6 " \n",
                  v6Ptr[i].returnStatus, v6Ptr[i].prefixLen,
                  PRI_IP6(v6Ptr[i].ipAddr.arIP));
        }
    }

    if (v4Ptr)
    {
        xpFree(v4Ptr);
    }

    if (v6Ptr)
    {
        xpFree(v6Ptr);
    }

    return status;
}

#ifdef __cplusplus
}
#endif
