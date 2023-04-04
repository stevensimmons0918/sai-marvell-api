// xpsVlan.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsVlan.h"
#include "xpsEgressFilter.h"
#include "xpsUtil.h"
#include "xpsLock.h"
#include "xpsInternal.h"
#include "xpsStp.h"
#include "xpsScope.h"
#include "xpsInterface.h"
#include "xpsInit.h"
#include "xpsPort.h"
#include "xpsMac.h"
#include "xpsGlobalSwitchControl.h"
#include "xpsLag.h"
#include "xpsVxlan.h"
#include "xpsAcl.h"
#include "xpsAllocator.h"

#include "gtGenTypes.h"
#include "gtEnvDep.h"
#include "cpssDxChBrgGen.h"
#include "cpssDxChBrgVlan.h"
#include "cpssHalUtil.h"
#include "cpssHalDevice.h"
#include "cpssHalVlan.h"
#include "cpssHalL3.h"
#include "cpssHalTunnel.h"
#include "cpssHalMulticast.h"
#include "cpssDxChPortCtrl.h"


#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgNestVlan.h>

extern xpsStpMode_e stpMode_g;

#include "cpssHalCascadeMgr.h"
#define IS_TNL_INTF(intfType) (((intfType) == XPS_TUNNEL_MPLS) || ((intfType) == XPS_TUNNEL_VXLAN) || \
        ((intfType) == XPS_TUNNEL_NVGRE) || ((intfType) == XPS_TUNNEL_GENEVE) || ((intfType) == XPS_TUNNEL_PBB))
#define IS_ENCAP_VALID(encapType) ((((encapType) < XP_L2_ENCAP_MAX) && (encapType) > XP_L2_ENCAP_INVALID))

#define XPS_VLAN_DEFAULT_INTFS 32
#define XPS_VLAN_DEFAULT_SECVLANS 8
#define XPS_VLAN_DEFAULT_VLANS_IN_LAG 8
#define XPS_VLAN_INVALID_ID 0xFFFF
#define XPS_VLAN_DEFAULT_ID 1
#define XPS_VLAN_MAX_BUF_SIZE 120
#define XPS_VLAN_SHOW_ALL 5000

//cpss defines
#define XPS_VLAN_FLOOD_VIDX 0xFFF
#define XPS_VLAN_L3_DEFAULT_VRID 0
//TODO set marvell fdb size
#define XPS_MAXFDB_LIMIT (32*1024)

#define XP_VLAN_STACK_RANGE_START             0
#define XP_VLAN_STACK_MAX_IDS 128

extern GT_STATUS cpssHalL3UpdtIntfIpUcRoutingEn(GT_U8 cpssDevId,
                                                GT_U32 cpssPortNum,
                                                CPSS_IP_PROTOCOL_STACK_ENT protocol,
                                                GT_BOOL enable,
                                                GT_BOOL isSVI);
static xpsDbHandle_t vlanStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;

static xpsDbHandle_t intfVlanIndexDbHndl = XPS_STATE_INVALID_DB_HANDLE;

static xpsDbHandle_t intfVlanTagTypeDbHndl = XPS_STATE_INVALID_DB_HANDLE;


static xpsDbHandle_t lagVlanListDbHndl = XPS_STATE_INVALID_DB_HANDLE;


static xpsDbHandle_t vlanStaticDataDbHndl = XPS_STATE_INVALID_DB_HANDLE;

static xpsDbHandle_t vlanStackDataDbHndl = XPS_STATE_INVALID_DB_HANDLE;

/*  xpsVlanContextDbEntry

        devBitMap - Bitmap to indicate if vlan is configured in a particular device
        primaryVlanId - Needed for private vlan; To fetch the primary id of a secondary vlan.
        l2DomainContext - Structure holding bdId, and encap nodes
        intfList[] - List of interfaces added to the Vlan
*/
typedef struct xpsVlanContextDbEntry
{
    //Key
    xpsVlan_t    vlanId;

    //Data
    xpsDeviceMap         deviceMap;

    //Private Vlan
    xpsPrivateVlanType_e vlanType;
    xpsVlan_t primaryVlanId;

    xpsVlanBridgeMcMode_e mcKeyType;
    bool igmpSnoopEnable;

    //Fdb limit
    uint32_t fdbCount;
    uint32_t fdbLimit;

    //Acl Id
    uint32_t baclId;
    uint32_t raclId;

    //Acl Status
    uint8_t baclEnable;
    uint8_t raclEnable;

    //Interface list
    uint16_t numOfIntfs;
    bool     isIpv4RoutingEn;
    bool     isIpv6RoutingEn;
    bool     isIpv4McRoutingEn;
    bool     isIpv6McRoutingEn;

    uint32_t mappedVni;
    xpsInterfaceId_t intfList[XPS_VLAN_DEFAULT_INTFS];
} xpsVlanContextDbEntry;


typedef struct xpsIntfVlanIndexDbEntry
{
    //Key
    uint32_t           intfVlanKey;

    //Data
    uint32_t           pVlanVifIdx;
} xpsIntfVlanIndexDbEntry;

/*  xpsIntfVlanEncapInfoDbEntry - Global structure to store vlan specific interface
                                        info.
        intfVlanKey: vlan+intfId (the key)

        devBitMap - Bitmap to indicate if this interface is configured in a particular device
        data: This is vni/tni/isid etc. Its egressEncapId for non-tunnel interfaces
        tagType: tagType of the interface (for tunnels its l2EncapType)
*/
typedef struct xpsIntfVlanEncapInfoDbEntry
{
    //Key
    uint32_t           intfVlanKey;

    //Data
    xpsDeviceMap         deviceMap;
    uint32_t           data;
    xpL2EncapType_e    tagType;
} xpsIntfVlanEncapInfoDbEntry;

typedef struct xpsPVlanCtxDbEntry
{
    //Key
    xpsVlan_t priVlanId;

    //Data
    xpsDeviceMap         deviceMap;
    uint8_t numOfSecVlans;
    uint32_t secVlanIds[XPS_VLAN_DEFAULT_SECVLANS];
} xpsPVlanCtxDbEntry;

typedef struct xpsLagVlanListDbEntry
{
    //Key
    xpsInterfaceId_t lagIntfId; //lagId or lag's bridgePortId

    //Data
    uint32_t pvid; /*variable to store the pvid if lag is untagged member of any vlan, default is invalid vlanId*/
    uint16_t numOfVlans;
    uint32_t vlans[XPS_VLAN_DEFAULT_VLANS_IN_LAG];
} xpsLagVlanListDbEntry;

typedef struct xpsLagBridgePortListDbEntry
{
    //Key
    xpsInterfaceId_t lagIntfId; //lagId

    //Data
    uint16_t numOfBridgePorts;
    uint32_t bridgePort[XPS_VLAN_DEFAULT_INTFS];
} xpsLagBridgePortListDbEntry;

typedef enum xpsVlanStaticDataType_e
{
    XPS_VLAN_STATIC_VARIABLES,
} xpsVlanStaticDataType_e;

//Holds the system defaultVlan value.
//Also holds the nullPortVif value. The same nullPortVif is used
//for hairpinning purpose for all ports in all vlans in system.
typedef struct xpsVlanStaticDbEntry
{
    //Key
    xpsVlanStaticDataType_e staticDataType;

    //Data
    xpsVlan_t    defaultVlan;
    uint32_t     nullPortVif;
} xpsVlanStaticDbEntry;

typedef struct xpsVlanStackDbEntry
{
    // Key
    xpsVlanStack_t vlanStackId;

    // Data
    xpsVlanStackStage_e xpsVlanStackStage;
    xpsVlanStackAction_e xpsVlanStackAction;
    xpsInterfaceId_t xpsIntf;
} xpsVlanStackDbEntry;

static uint32_t xpsIntfVlanDbKey(xpsScope_t scopeId, xpsInterfaceId_t intfId,
                                 xpsVlan_t vlanId)
{
    XP_STATUS result = XP_NO_ERR;
    xpsInterfaceType_e intfType;

    if ((result = xpsInterfaceGetTypeScope(scopeId, intfId,
                                           &intfType)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interfaceType for interface(%d) failed:%d", intfId, result);
        return 0;
    }
#ifdef DEBUG_FLAG
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "IntfId:%d vlanId:%d", intfId,
          vlanId);
#endif
    return ((intfId << 12) | vlanId);

}



static int32_t vlanCtxKeyComp(void* key1, void* key2)
{
    return ((((xpsVlanContextDbEntry *) key1)->vlanId) - (((
                                                               xpsVlanContextDbEntry *) key2)->vlanId));
}




static int32_t intfVlanKeyComp(void* key1, void* key2)
{
    return ((((xpsIntfVlanEncapInfoDbEntry *) key1)->intfVlanKey)
            - (((xpsIntfVlanEncapInfoDbEntry *) key2)->intfVlanKey));
}

static int32_t intfVlanIndexKeyComp(void* key1, void* key2)
{
    return ((((xpsIntfVlanIndexDbEntry *) key1)->intfVlanKey)
            - (((xpsIntfVlanIndexDbEntry *) key2)->intfVlanKey));
}

static int32_t lagVlanListKeyComp(void* key1, void* key2)
{
    return ((((xpsLagVlanListDbEntry *) key1)->lagIntfId) - (((
                                                                  xpsLagVlanListDbEntry *) key2)->lagIntfId));
}




static int32_t vlanStaticVarKeyComp(void* key1, void* key2)
{
    return ((((xpsVlanStaticDbEntry *) key1)->staticDataType) - (((
                                                                      xpsVlanStaticDbEntry *) key2)->staticDataType));
}

static int32_t vlanStackVarKeyComp(void *key1, void *key2)
{
    return ((((xpsVlanStackDbEntry *)key1)->vlanStackId) - (((
                                                                 xpsVlanStackDbEntry *)key2)
                                                                ->vlanStackId));
}

//Static variables DB functions
static XP_STATUS xpsVlanGetStaticVariablesDb(xpsScope_t scopeId,
                                             xpsVlanStaticDbEntry ** staticVarDbPtr)
{
    XP_STATUS result = XP_NO_ERR;
    xpsVlanStaticDbEntry keyStaticVarDb;

    memset(&keyStaticVarDb, 0x0, sizeof(xpsVlanStaticDbEntry));
    keyStaticVarDb.staticDataType = XPS_VLAN_STATIC_VARIABLES;
    if ((result = xpsStateSearchData(scopeId, vlanStaticDataDbHndl,
                                     (xpsDbKey_t)&keyStaticVarDb, (void**)staticVarDbPtr)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsStateSearchData failed");
        return result;
    }

    if (!(*staticVarDbPtr))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Key not found");
        return XP_ERR_KEY_NOT_FOUND;
    }

    return result;
}


static XP_STATUS xpsVlanInsertStaticVariablesDb(xpsScope_t scopeId)
{

    XP_STATUS result = XP_NO_ERR;
    uint32_t npVlanVif = 0;
    xpsVlanStaticDbEntry *staticVarDb;

    if ((result = xpsStateHeapMalloc(sizeof(xpsVlanStaticDbEntry),
                                     (void**)&staticVarDb)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Heap malloc failed");
        return result;
    }
    if (staticVarDb == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }

    memset(staticVarDb, 0, sizeof(xpsVlanStaticDbEntry));
    staticVarDb->staticDataType = XPS_VLAN_STATIC_VARIABLES;


    staticVarDb->nullPortVif = npVlanVif;
    staticVarDb->defaultVlan = XPS_VLAN_DEFAULT_ID;

    // Insert the vlan static variables into the database
    if ((result = xpsStateInsertData(scopeId, vlanStaticDataDbHndl,
                                     (void*)staticVarDb)) != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)staticVarDb);
        staticVarDb = NULL;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Inserting data in state failed");
        return result;
    }

    return result;
}

/* Following APIs for Lag BridgePort */








/* Following APIs for LAG*/
static XP_STATUS xpsVlanGetLagVlanListDb(xpsScope_t scopeId,
                                         xpsInterfaceId_t lagIntfId, xpsLagVlanListDbEntry ** lagVlanListEntryPtr)
{
    XP_STATUS result = XP_NO_ERR;
    xpsLagVlanListDbEntry keyLagVlanListEntry;

    memset(&keyLagVlanListEntry, 0x0, sizeof(xpsLagVlanListDbEntry));
    keyLagVlanListEntry.lagIntfId = lagIntfId;
    if ((result = xpsStateSearchData(scopeId, lagVlanListDbHndl,
                                     (xpsDbKey_t)&keyLagVlanListEntry, (void**)lagVlanListEntryPtr)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Search data failed");
        return result;
    }

    return result;
}

static XP_STATUS xpsVlanRemoveLagVlanListDb(xpsScope_t scopeId,
                                            xpsInterfaceId_t lagIntfId)
{

    XP_STATUS result = XP_NO_ERR;
    xpsLagVlanListDbEntry * lagVlanListEntry = NULL;
    xpsLagVlanListDbEntry keyLagVlanListEntry;

    memset(&keyLagVlanListEntry, 0x0, sizeof(xpsLagVlanListDbEntry));
    keyLagVlanListEntry.lagIntfId = lagIntfId;

    if ((result = xpsStateDeleteData(scopeId, lagVlanListDbHndl,
                                     (xpsDbKey_t)&keyLagVlanListEntry, (void**)&lagVlanListEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Delete data for lag interface(%d) failed", lagIntfId);
        return result;
    }

    if ((result = xpsStateHeapFree((void*)lagVlanListEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "XPS state Heap Free failed");
        return result;
    }

    return result;
}


static XP_STATUS xpsVlanAddVlanInLagDb(xpsScope_t scopeId,
                                       xpsInterfaceId_t lagIntfId, xpsVlan_t vlanId, xpsL2EncapType_e tagType)
{

    XP_STATUS result = XP_NO_ERR;
    xpsLagVlanListDbEntry * lagVlanListEntryNew = NULL, * lagVlanListEntry = NULL;
    uint16_t numOfVlans = 0;

    result = xpsVlanGetLagVlanListDb(scopeId, lagIntfId, &lagVlanListEntry);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Lag Vlan List for lag interface(%d) failed", lagIntfId);
        return result;
    }

    //If first vlan for lag, then need to create the db entry.
    if (NULL == lagVlanListEntry)
    {
        if ((result = xpsStateHeapMalloc(sizeof(xpsLagVlanListDbEntry),
                                         (void**)&lagVlanListEntryNew)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "XPS state Heap Allocation failed");
            return result;
        }
        if (lagVlanListEntryNew == NULL)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Recieved null pointer");
            return XP_ERR_NULL_POINTER;
        }

        memset(lagVlanListEntryNew, 0, sizeof(xpsLagVlanListDbEntry));

        //First vlan addition in lag.
        lagVlanListEntryNew->lagIntfId = lagIntfId;
        lagVlanListEntryNew->numOfVlans = 1;
        lagVlanListEntryNew->vlans[0] = vlanId;

        if (tagType == XP_L2_ENCAP_DOT1Q_UNTAGGED)
        {
            lagVlanListEntryNew->pvid =
                vlanId; /*set pvid to the current vlanId as lag is untagged */
        }
        else
        {
            lagVlanListEntryNew->pvid =
                XP_MAX_VLAN_NUM; /*set default to invalid vlanId indicating lag is not untagged intf to any vlan*/
        }

        if ((result = xpsStateInsertData(scopeId, lagVlanListDbHndl,
                                         (void*)lagVlanListEntryNew)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Inserting data in state failed");
            xpsStateHeapFree((void*)lagVlanListEntryNew);
            return result;
        }

        return XP_NO_ERR;
    }

    numOfVlans = lagVlanListEntry->numOfVlans;

    //Create a new context with double dynamic array size. Also delete old context
    if (xpsDAIsCtxGrowthNeeded(numOfVlans, XPS_VLAN_DEFAULT_VLANS_IN_LAG))
    {
        result = xpsDynamicArrayGrow((void**)&lagVlanListEntryNew, lagVlanListEntry,
                                     sizeof(xpsLagVlanListDbEntry),
                                     sizeof(uint32_t), numOfVlans, XPS_VLAN_DEFAULT_VLANS_IN_LAG);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Dynamic array grow failed");
            return result;
        }

        //Delete the old context
        result = xpsVlanRemoveLagVlanListDb(scopeId, lagIntfId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Delete lag from vlan list failed, lag interface(%d)", lagIntfId);
            xpsStateHeapFree((void*)lagVlanListEntryNew);
            return result;
        }
        if ((result = xpsStateInsertData(scopeId, lagVlanListDbHndl,
                                         (void*)lagVlanListEntryNew)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Inserting data in state failed");
            xpsStateHeapFree((void*)lagVlanListEntryNew);
            return result;
        }
        lagVlanListEntry = lagVlanListEntryNew;
    }

    lagVlanListEntry->numOfVlans += 1;
    lagVlanListEntry->vlans[numOfVlans] = vlanId;

    if (tagType == XP_L2_ENCAP_DOT1Q_UNTAGGED)
    {
        lagVlanListEntry->pvid =
            vlanId; /*set pvid to the current vlanId as lag is untagged */
    }

    return result;
}

static XP_STATUS xpsVlanRemoveVlanFromLagDb(xpsScope_t scopeId,
                                            xpsInterfaceId_t lagIntfId, xpsVlan_t vlanId)
{
    XP_STATUS result = XP_NO_ERR;
    xpsLagVlanListDbEntry * lagVlanListEntryNew = NULL, * lagVlanListEntry = NULL;
    uint16_t j = 0, numOfVlans, vlanPresent = 0;

    result = xpsVlanGetLagVlanListDb(scopeId, lagIntfId, &lagVlanListEntry);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Lag Vlan List for lag interface(%d) failed", lagIntfId);
        return result;
    }

    if (lagVlanListEntry == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid arguments");
        return XP_ERR_INVALID_ARG;
    }

    numOfVlans = lagVlanListEntry->numOfVlans;

    //If no vlans or only one vlan and invalid input - return error
    if ((!numOfVlans) || ((numOfVlans == 1) &&
                          (lagVlanListEntry->vlans[0] != vlanId)))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlan id");
        return XP_ERR_INVALID_ARG;
    }

    /*check if the lag is untagged member of vlan, if so update pvid to Invalid*/
    if (lagVlanListEntry->pvid == vlanId)
    {
        /*reset pvid to default vlanId*/
        lagVlanListEntry->pvid = XP_MAX_VLAN_NUM;
    }

    if ((numOfVlans == 1) && (lagVlanListEntry->vlans[0] == vlanId))
    {
        //Only vlan in the lag matches the vlanId to be removed then delete the context
        result = xpsVlanRemoveLagVlanListDb(scopeId, lagIntfId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Remove the Lag from vlan list,"
                  " lag interface(%d)", lagIntfId);
            return result;
        }

        //Lag-vlan Db deleted. Return
        return XP_NO_ERR;
    }

    //Delete the vlan from the context
    lagVlanListEntry->numOfVlans = numOfVlans - 1;
    for (j = 0; j < numOfVlans; j++)
    {
        if (!vlanPresent)
        {
            if (lagVlanListEntry->vlans[j] == vlanId)
            {
                vlanPresent = 1;
            }
            continue;
        }
        else if (j !=0)
        {
            lagVlanListEntry->vlans[j-1] = lagVlanListEntry->vlans[j];
        }
    }

    if (!vlanPresent)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid arguments");
        return XP_ERR_INVALID_ARG;
    }

    if (xpsDAIsCtxShrinkNeeded(numOfVlans, XPS_VLAN_DEFAULT_VLANS_IN_LAG))
    {
        result = xpsDynamicArrayShrink((void**)&lagVlanListEntryNew,
                                       lagVlanListEntry, sizeof(xpsLagVlanListDbEntry),
                                       sizeof(uint32_t), numOfVlans, XPS_VLAN_DEFAULT_VLANS_IN_LAG);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Dynamic array shrink failed");
            return result;
        }

        result = xpsVlanRemoveLagVlanListDb(scopeId, lagIntfId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Remove the Lag from vlan list,"
                  " lag interface(%d)", lagIntfId);
            xpsStateHeapFree((void*)lagVlanListEntryNew);
            lagVlanListEntryNew = NULL;
            return result;
        }

        if ((result = xpsStateInsertData(scopeId, lagVlanListDbHndl,
                                         (void*)lagVlanListEntryNew)) != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)lagVlanListEntryNew);
            lagVlanListEntryNew = NULL;
            return result;
        }
    }

    return result;
}

/* Following APIs for private vlan state maintainence*/










static XP_STATUS xpsVlanGetVlanCtxDb(xpsScope_t scopeId, xpsVlan_t vlanId,
                                     xpsVlanContextDbEntry ** vlanCtxPtr)
{
    XP_STATUS result = XP_NO_ERR;
    xpsVlanContextDbEntry keyVlanCtx;

    memset(&keyVlanCtx, 0x0, sizeof(xpsVlanContextDbEntry));
    keyVlanCtx.vlanId= vlanId;
    if ((result = xpsStateSearchData(scopeId, vlanStateDbHndl,
                                     (xpsDbKey_t)&keyVlanCtx, (void**)vlanCtxPtr)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, vlanId(%d)", vlanId);
        return result;
    }

    if (!(*vlanCtxPtr))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan does not exist");
        return XP_ERR_KEY_NOT_FOUND;
    }

    return result;
}



XP_STATUS xpsVlanUpdateMemoryAddressInVlanCtxDb(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetVlanFloodVIF(xpsScope_t scopeId, xpsVlan_t vlanId,
                                 xpVif_t *floodVif)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsVlanGetVlanMulticastVIF(xpsScope_t scopeId, xpsVlan_t vlanId,
                                     xpL2EncapType_e encapType, xpVif_t *mcVif)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

static XP_STATUS xpsVlanRemoveVlanCtxDb(xpsScope_t scopeId, xpsVlan_t vlanId)
{

    XP_STATUS result = XP_NO_ERR;
    xpsVlanContextDbEntry * vlanCtx = NULL;
    xpsVlanContextDbEntry keyVlanCtx;

    memset(&keyVlanCtx, 0x0, sizeof(xpsVlanContextDbEntry));
    keyVlanCtx.vlanId= vlanId;

    if ((result = xpsStateDeleteData(scopeId, vlanStateDbHndl,
                                     (xpsDbKey_t)&keyVlanCtx, (void**)&vlanCtx)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Delete vlan data failed, vlanId(%d)", vlanId);
        return result;
    }

    if ((result = xpsStateHeapFree((void*)vlanCtx)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "XPS state Heap Free failed");
        return result;
    }

    return result;
}

static XP_STATUS xpsVlanInsertVlanCtxDb(xpsScope_t scopeId, xpsVlan_t vlanId,
                                        xpsVlanContextDbEntry ** vlanCtxPtr)
{

    XP_STATUS result = XP_NO_ERR;

    if ((result = xpsStateHeapMalloc(sizeof(xpsVlanContextDbEntry),
                                     (void**)vlanCtxPtr)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "XPS state Heap Allocation failed");
        return result;
    }
    if (*vlanCtxPtr == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Recieved null pointer");
        return XP_ERR_NULL_POINTER;
    }

    memset(*vlanCtxPtr, 0, sizeof(xpsVlanContextDbEntry));

    (*vlanCtxPtr)->vlanId = vlanId;

    // Insert the vlan context into the database, vlanId is the key
    if ((result = xpsStateInsertData(scopeId, vlanStateDbHndl,
                                     (void*)*vlanCtxPtr)) != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)(*vlanCtxPtr));
        *vlanCtxPtr = NULL;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Inserting data in state failed");
        return result;
    }

    return result;
}




static XP_STATUS xpsVlanAddIntfDb(xpsScope_t scopeId, xpsVlan_t vlanId,
                                  xpsInterfaceId_t intfId, xpsVlanContextDbEntry **vlanCtxNewPtr)
{

    XP_STATUS result = XP_NO_ERR;
    xpsVlanContextDbEntry *vlanCtx = NULL;
    uint16_t numOfIntfs;

    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }

    numOfIntfs = vlanCtx->numOfIntfs;

    //Number of interfaces increase exponentially starting from XPS_VLAN_DEFAULT_INTFS by default
    //Everytime the number reaches the current size, the size is doubled
    if (xpsDAIsCtxGrowthNeeded(numOfIntfs, XPS_VLAN_DEFAULT_INTFS))
    {
        result = xpsDynamicArrayGrow((void **)vlanCtxNewPtr, (void *)vlanCtx,
                                     sizeof(xpsVlanContextDbEntry),
                                     sizeof(xpsInterfaceId_t), numOfIntfs, XPS_VLAN_DEFAULT_INTFS);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Dynamic array grow failed");
            return result;
        }

        //Delete the old vlanCtx
        result = xpsVlanRemoveVlanCtxDb(scopeId, vlanId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Delete the vlan context failed,"
                  "vlanId(%d)", vlanId);
            xpsStateHeapFree((void *)(*vlanCtxNewPtr));
            return result;
        }

        //Insert the new context
        if ((result = xpsStateInsertData(scopeId, vlanStateDbHndl,
                                         (void*)(*vlanCtxNewPtr))) != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)(*vlanCtxNewPtr));
            (*vlanCtxNewPtr) = NULL;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Inserting data in state failed");
            return result;
        }
        vlanCtx = *vlanCtxNewPtr;
    }

    vlanCtx->intfList[numOfIntfs] = intfId;
    vlanCtx->numOfIntfs += 1;

    return result;
}

static XP_STATUS xpsVlanRemoveIntfDb(xpsScope_t scopeId, xpsVlan_t vlanId,
                                     xpsInterfaceId_t intfId, xpsVlanContextDbEntry **vlanCtxNew)
{

    XP_STATUS result = XP_NO_ERR;
    xpsVlanContextDbEntry *vlanCtx = NULL;
    uint32_t j = 0, numOfIntfs, intfPresent = 0;

    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }

    numOfIntfs = vlanCtx->numOfIntfs;

    if (!numOfIntfs)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vlan does not have any interface");
        return XP_ERR_INVALID_ARG;
    }

    //Delete the element from the context
    vlanCtx->numOfIntfs = numOfIntfs - 1;
    for (j = 0; j < numOfIntfs; j++)
    {
        if (!intfPresent)
        {
            if (vlanCtx->intfList[j] == intfId)
            {
                intfPresent = 1;
            }
            continue;
        }
        else if (j != 0)
        {
            vlanCtx->intfList[j-1] = vlanCtx->intfList[j];
        }
    }

    if (!intfPresent)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid arguments");
        return XP_ERR_INVALID_ARG;
    }

    if (xpsDAIsCtxShrinkNeeded(numOfIntfs, XPS_VLAN_DEFAULT_INTFS))
    {
        result = xpsDynamicArrayShrink((void **)vlanCtxNew, (void *)vlanCtx,
                                       sizeof(xpsVlanContextDbEntry),
                                       sizeof(xpsInterfaceId_t), numOfIntfs, XPS_VLAN_DEFAULT_INTFS);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Dynamic array shrink failed");
            return result;
        }

        result = xpsVlanRemoveVlanCtxDb(scopeId, vlanId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Delete the vlan context failed,"
                  "vlanId(%d)", vlanId);
            xpsStateHeapFree((void*)(*vlanCtxNew));
            *vlanCtxNew = NULL;
            return result;
        }

        if ((result = xpsStateInsertData(scopeId, vlanStateDbHndl,
                                         (void*)*vlanCtxNew)) != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)(*vlanCtxNew));
            *vlanCtxNew = NULL;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Inserting data in state failed");
            return result;
        }
    }

    return result;
}


XP_STATUS xpsVlanGetEndpointInfo(xpsScope_t scopeId, xpsVlan_t vlanId,
                                 xpsInterfaceId_t intfId, xpsL2EncapType_e *tagType, uint32_t *data)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result;
    xpsIntfVlanEncapInfoDbEntry *intfVlan = NULL, keyIntfVlan;
    uint32_t intfVlanKey = xpsIntfVlanDbKey(scopeId, intfId, vlanId);

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_ARG;
    }

    memset(&keyIntfVlan, 0x0, sizeof(xpsIntfVlanEncapInfoDbEntry));
    keyIntfVlan.intfVlanKey= intfVlanKey;

    if ((result = xpsStateSearchData(scopeId, intfVlanTagTypeDbHndl,
                                     (xpsDbKey_t)&keyIntfVlan, (void**)&intfVlan)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, vlanId(%d) interface(%d)", vlanId, intfId);
        return result;
    }

    if (!intfVlan)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Interface is not added in vlanId(%d)", vlanId);
        result = XP_ERR_INVALID_ARG;
        return result;
    }

    *tagType = intfVlan->tagType;
    // In case of .1D bridge, this data represents the bridgeId instead of encapVlan.
    // For encapVlan derivation, XPS_INTF_MAP_BRIDGE_PORT_TO_VLAN(bridgePortId) can be used.
    *data = intfVlan->data;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetIntfTagTypeScope(xpsScope_t scopeId, xpsVlan_t vlanId,
                                     xpsInterfaceId_t intfId, xpsL2EncapType_e *tagType)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    xpsIntfVlanEncapInfoDbEntry *intfVlan = NULL, keyIntfVlan;
    uint32_t intfVlanKey = xpsIntfVlanDbKey(scopeId, intfId, vlanId);

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_ARG;
    }

    memset(&keyIntfVlan, 0x0, sizeof(xpsIntfVlanEncapInfoDbEntry));
    keyIntfVlan.intfVlanKey= intfVlanKey;

    if ((result = xpsStateSearchData(scopeId, intfVlanTagTypeDbHndl,
                                     (xpsDbKey_t)&keyIntfVlan, (void**)&intfVlan)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, vlanId(%d) interface(%d)", vlanId, intfId);
        return result;
    }

    if (!intfVlan)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Interface is not added in vlanId(%d)", vlanId);
        result = XP_ERR_INVALID_ARG;
        return result;
    }

    *tagType = intfVlan->tagType;

    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsVlanGetIntfTagType(xpsVlan_t vlanId, xpsInterfaceId_t intfId,
                                xpsL2EncapType_e *tagType)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsVlanGetIntfTagTypeScope(XP_SCOPE_DEFAULT, vlanId, intfId, tagType);

    XPS_FUNC_EXIT_LOG();
}

static XP_STATUS xpsVlanGetIntfEntryDb(xpsScope_t scopeId, xpsVlan_t vlanId,
                                       xpsInterfaceId_t intfId, xpsIntfVlanEncapInfoDbEntry ** intfVlanPtr)
{
    XP_STATUS result = XP_NO_ERR;
    uint32_t intfVlanKey = xpsIntfVlanDbKey(scopeId, intfId, vlanId);
    xpsIntfVlanEncapInfoDbEntry keyIntfVlan;

    memset(&keyIntfVlan, 0x0, sizeof(xpsIntfVlanEncapInfoDbEntry));
    keyIntfVlan.intfVlanKey= intfVlanKey;

    if ((result = xpsStateSearchData(scopeId, intfVlanTagTypeDbHndl,
                                     (xpsDbKey_t)&keyIntfVlan, (void**)intfVlanPtr)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, vlanId(%d) interface(%d)", vlanId, intfId);
        return result;
    }

    return result;
}




static XP_STATUS xpsVlanRemoveIntfEntryDb(xpsScope_t scopeId, xpsVlan_t vlanId,
                                          xpsInterfaceId_t intfId)
{
    XP_STATUS result = XP_NO_ERR;
    uint32_t intfVlanKey = xpsIntfVlanDbKey(scopeId, intfId, vlanId);
    xpsIntfVlanEncapInfoDbEntry * intfVlan = NULL, keyIntfVlan;

    memset(&keyIntfVlan, 0x0, sizeof(xpsIntfVlanEncapInfoDbEntry));
    keyIntfVlan.intfVlanKey= intfVlanKey;

    if ((result = xpsStateSearchData(scopeId, intfVlanTagTypeDbHndl,
                                     (xpsDbKey_t)&keyIntfVlan, (void**)&intfVlan)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, vlanId(%d) interface(%d)", vlanId, intfId);
        return result;
    }

    if (intfVlan)
    {
        //Delete from RB tree
        if ((result = xpsStateDeleteData(scopeId, intfVlanTagTypeDbHndl,
                                         (xpsDbKey_t)&keyIntfVlan, (void**)&intfVlan)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Delete Data for device (id: %d) failed,"
                  " vlanId(%d) interface(%d)", vlanId, intfId);
            return result;
        }

        //Free the node memory
        if ((result = xpsStateHeapFree((void*)intfVlan)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Heap Free failed");
            return result;
        }
    }

    return result;
}



static XP_STATUS xpsVlanInsertIntfEntryDb(xpsScope_t scopeId, xpsVlan_t vlanId,
                                          xpsInterfaceId_t intfId, xpL2EncapType_e tagType, uint32_t data,
                                          xpsIntfVlanEncapInfoDbEntry ** intfVlanPtr)
{
    XP_STATUS result = XP_NO_ERR;
    uint32_t intfVlanKey = xpsIntfVlanDbKey(scopeId, intfId, vlanId);

    if ((result = xpsStateHeapMalloc(sizeof(xpsIntfVlanEncapInfoDbEntry),
                                     (void**)intfVlanPtr)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Heap Allocation failed");
        return result;
    }

    memset(*intfVlanPtr, 0, sizeof(xpsIntfVlanEncapInfoDbEntry));

    (*intfVlanPtr)->intfVlanKey = intfVlanKey;
    // In case of .1D bridge, this data represents the bridgeId instead of encapVlan.
    // For encapVlan derivation, XPS_INTF_MAP_BRIDGE_PORT_TO_VLAN(bridgePortId) can be used.
    (*intfVlanPtr)->data = data;
    (*intfVlanPtr)->tagType  = tagType;

    // Insert the interface info structure into the database, using the vif as a key
    if ((result = xpsStateInsertData(scopeId, intfVlanTagTypeDbHndl,
                                     (void*)*intfVlanPtr)) != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)(*intfVlanPtr));
        (*intfVlanPtr) = NULL;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Inserting data in state failed");
        return result;
    }

    return result;
}



//Per device APIs for portVlanIvifIndex in hw for an interface and vlan for a particular device.
static XP_STATUS xpsVlanGetIntfIndexDb(xpsDevice_t devId, xpsVlan_t vlanId,
                                       xpsInterfaceId_t intfId, xpsIntfVlanIndexDbEntry ** intfVlanPtr)
{
    XP_STATUS result = XP_NO_ERR;
    xpsScope_t scopeId;
    uint32_t intfVlanKey = 0;
    xpsIntfVlanIndexDbEntry keyIntfVlan;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    intfVlanKey = xpsIntfVlanDbKey(scopeId, intfId, vlanId);

    memset(&keyIntfVlan, 0x0, sizeof(xpsIntfVlanIndexDbEntry));
    keyIntfVlan.intfVlanKey= intfVlanKey;

    if ((result = xpsStateSearchDataForDevice(devId, intfVlanIndexDbHndl,
                                              (xpsDbKey_t)&keyIntfVlan, (void**)intfVlanPtr)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, vlanId(%d) interface(%d)", vlanId, intfId);
        return result;
    }

    return result;
}



XP_STATUS xpsVlanGetIntfIndex(xpsDevice_t devId, xpsVlan_t vlanId,
                              xpsInterfaceId_t intfId, uint32_t *pVlanIvifIdx)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

static XP_STATUS xpsVlanStackInsertDb(xpsScope_t scopeId, xpsVlanStack_t vlanStackId,
                                      xpsVlanStackDbEntry **vlanStackEntry)
{

    XP_STATUS result = XP_NO_ERR;

    if ((result = xpsStateHeapMalloc(sizeof(xpsVlanStackDbEntry),
                                     (void **)vlanStackEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Memory allocation for vlan stack failed");
        return result;
    }
    if (*vlanStackEntry == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    memset(*vlanStackEntry, 0, sizeof(vlanStackEntry));

    (*vlanStackEntry)->vlanStackId = vlanStackId;

    // Insert the vlan stack mapping into the database, vlanStackId is the key
    if ((result = xpsStateInsertData(scopeId, vlanStackDataDbHndl,
                                     (void *)*vlanStackEntry)) != XP_NO_ERR)
    {
        xpsStateHeapFree((void *)*vlanStackEntry);
        return result;
    }

    return result;
}

static XP_STATUS xpsVlanStackGetDb(xpsScope_t scopeId, xpsVlanStack_t vlanStackId,
                                   xpsVlanStackDbEntry **vlanStackEntry)
{

    XP_STATUS result = XP_NO_ERR;
    xpsVlanStackDbEntry keyVlanStackEntry;

    memset(&keyVlanStackEntry, 0x0, sizeof(keyVlanStackEntry));
    keyVlanStackEntry.vlanStackId = vlanStackId;

    if ((result = xpsStateSearchData(scopeId, vlanStackDataDbHndl,
                                     (xpsDbKey_t)&keyVlanStackEntry, (void **)vlanStackEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, vlanStackId(%d)", vlanStackId);
        return result;
    }
    if (!(*vlanStackEntry))
    {

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan stack entry does not exist");
        return XP_ERR_KEY_NOT_FOUND;
    }

    return result;
}

static XP_STATUS xpsVlanStackRemoveDb(xpsScope_t scopeId, xpsVlanStack_t vlanStackId)
{

    XP_STATUS result = XP_NO_ERR;
    xpsVlanStackDbEntry keyVlanStackEntry;
    xpsVlanStackDbEntry *vlanStackEntry = NULL;
    memset(&keyVlanStackEntry, 0x0, sizeof(keyVlanStackEntry));
    keyVlanStackEntry.vlanStackId = vlanStackId;

    if ((result = xpsStateDeleteData(scopeId, vlanStackDataDbHndl,
                                     (xpsDbKey_t)&keyVlanStackEntry, (void **)&vlanStackEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Delete vlan stack failed, vlanStackId(%d)", vlanStackId);
        return result;
    }
    if ((result = xpsStateHeapFree((void *)vlanStackEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Heap Freeing failed");
        return result;
    }

    return result;
}

//This API not exposed to user. For internal purpose



/*
 * Vlan 1 is created by default.
 */
XP_STATUS xpsVlanInitDefaultVlan(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    uint8_t maxTotalPorts = 0;
#if 0
    int portNum;
#endif

    if ((result = xpsGlobalSwitchControlGetMaxPorts(devId,
                                                    &maxTotalPorts)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get max physical port number. error code:%d\n", result);
    }
    result = xpsVlanCreate(devId, XPS_VLAN_DEFAULT_ID);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Creating Default Vlan failed, ");
        return result;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          " Not adding member ports for default vlan, ");

#if 0
    XPS_GLOBAL_PORT_ITER(portNum, maxTotalPorts)
    {
        if ((result = xpsVlanAddInterface(devId, XPS_VLAN_DEFAULT_ID, portNum,
                                          XP_L2_ENCAP_DOT1Q_UNTAGGED)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Can't add interface to default vlan. error code:%d\n", result);
        }
    }
#endif

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanInit(void)
{
    return xpsVlanInitScope(XP_SCOPE_DEFAULT);
}

XP_STATUS xpsVlanInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();
    xpsDevice_t devId = 0;
    GT_STATUS status = GT_OK;
    int maxTotalPorts =0;
    int portNum = 0;
    XP_STATUS result;
    xpsLockCreate(XP_LOCKINDEX_XPS_VLAN, 1);
    vlanStateDbHndl = XPS_VLAN_STATE_DB_HNDL;

    status = cpssHalUpdateMaxVlanCountInHw(scopeId, XPS_VLAN_MAX_IDS);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Updating max vlan count in hardware failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    /* GLOBAL states initialization*/
    result = xpsStateRegisterDb(scopeId, "Vlan state", XPS_GLOBAL, &vlanCtxKeyComp,
                                vlanStateDbHndl);
    if (result != XP_NO_ERR)
    {
        vlanStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vlan state DB register failed");
        return result;
    }
    if ((cpssHalGetMaxGlobalPorts(&maxTotalPorts) != XP_NO_ERR))
    {
        cpssOsPrintf("%s:Error: Can't get max physical port number. error code:%d\n",
                     __FUNCNAME__, result);
    }

    intfVlanTagTypeDbHndl = XPS_INTF_VLAN_TAG_TYPE_DB_HNDL;
    result = xpsStateRegisterDb(scopeId, "Port Vlan TagType state", XPS_GLOBAL,
                                &intfVlanKeyComp, intfVlanTagTypeDbHndl);
    if (result != XP_NO_ERR)
    {
        intfVlanTagTypeDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Port Vlan TagType state DB register failed");
        return result;
    }
    vlanStaticDataDbHndl = XPS_VLAN_STATIC_DATA_DB_HNDL;
    result = xpsStateRegisterDb(scopeId, "Vlan global variables", XPS_GLOBAL,
                                &vlanStaticVarKeyComp, vlanStaticDataDbHndl);
    if (result != XP_NO_ERR)
    {
        vlanStaticDataDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Register db call failed");
        return result;
    }

    intfVlanIndexDbHndl = XPS_INTF_VLAN_INDEX_DB_HNDL;
    /* PER-DEVICE state initialization*/
    result = xpsStateRegisterDb(scopeId, "Port Vlan Index state", XPS_PER_DEVICE,
                                &intfVlanIndexKeyComp, intfVlanIndexDbHndl);
    if (result != XP_NO_ERR)
    {
        intfVlanIndexDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Port Vlan Index state DB register failed");
        return result;
    }

    result = xpsVlanInsertStaticVariablesDb(scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Inserting static variables failed");
        return result;
    }

    lagVlanListDbHndl = XPS_LAG_VLANLIST_DB_HNDL;
    result = xpsStateRegisterDb(scopeId, "Lag - vlanList state", XPS_GLOBAL,
                                &lagVlanListKeyComp, lagVlanListDbHndl);
    if (result != XP_NO_ERR)
    {
        lagVlanListDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Lag - vlanList state DB register failed");
        return result;
    }

    vlanStackDataDbHndl = XPS_VLAN_STACK_DATA_DB_HNDL;
    result = xpsStateRegisterDb(scopeId, "Vlan stack variables", XPS_GLOBAL,
                                &vlanStackVarKeyComp, vlanStackDataDbHndl);
    if (result != XP_NO_ERR)
    {
        vlanStackDataDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Register db call failed");
        return result;
    }

    XPS_GLOBAL_PORT_ITER(portNum, maxTotalPorts)
    {
        status = cpssHalEnableBrgVlanPortIngFlt(devId, portNum, GT_TRUE);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Ingress filtering for the port number (%d) failed", portNum);
            return xpsConvertCpssStatusToXPStatus(status);
        }

    }

    //set mru value for jumbo frame.
    if ((status = cpssHalSetBrgVlanMruProfileValue(devId, 0,
                                                   CPSS_DXCH_PORT_MAX_MRU_CNS)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Mru profile value set failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }


    /* Set TPID bitmap for TAG1 to 0, since SONIC doesn't support QinQ.
     * Supported Config : 8 TPID entries; 8 profiles.
     * By default all profiles are set to tpidBmp 0xF for both ingress and egress.
     * And all ports by default use profile 0.
     * For now, disable profile 0, TAG1 tpidbitmap by setting to 0.
     * TODO:Revisit this code when S/C tag configurations are allowed from SONIC.
     */

    status = cpssHalBrgVlanIngressTpidProfileSet(devId, 0, CPSS_VLAN_ETHERTYPE1_E,
                                                 0x0);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "VlanIngressTpidProfileSet failed");
        return xpsConvertCpssStatusToXPStatus(status);

    }

    /* initialize the Vlan Stack Id Allocator*/
    result = xpsAllocatorInitIdAllocator(scopeId, XP_ALLOCATOR_VLAN_STACK,
                                         XP_VLAN_STACK_MAX_IDS, XP_VLAN_STACK_RANGE_START);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to initialize XPS Vlan Stack allocator\n");
        return result;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpsVlanDeInitScope(xpsScope_t scopeId)
{
    XP_STATUS result = XP_NO_ERR;
    XPS_FUNC_ENTRY_LOG();

    /* GLOBAL states de-initialization*/
    result = xpsStateDeRegisterDb(scopeId, &vlanStateDbHndl);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "vlan state De-Register DB handler failed");
        return result;
    }
    result = xpsStateDeRegisterDb(scopeId, &intfVlanTagTypeDbHndl);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Port vlan Tag state De-Register DB handler failed");
        return result;
    }


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();


    XP_STATUS result = XP_NO_ERR;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }


    cpssDxChBrgVlanBridgingModeSet(devId, CPSS_BRG_MODE_802_1Q_E);

    result = xpsStateInsertDbForDevice(devId, intfVlanIndexDbHndl,
                                       &intfVlanIndexKeyComp);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Adding port vlan index for device(ID: %d) failed", devId);
        return result;
    }


    return XP_NO_ERR;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}



XP_STATUS xpsVlanRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanIsExist(xpsScope_t scopeId, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanIsExistOnDevice(xpsDevice_t devId, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanIsEndpointExist(xpsDevice_t devId, xpsVlan_t vlanId,
                                 xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    xpsIntfVlanEncapInfoDbEntry *intfVlan = NULL, keyIntfVlan;
    uint32_t intfVlanKey = 0;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    intfVlanKey = xpsIntfVlanDbKey(scopeId, intfId, vlanId);

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    memset(&keyIntfVlan, 0x0, sizeof(xpsIntfVlanEncapInfoDbEntry));
    keyIntfVlan.intfVlanKey = intfVlanKey;

    if ((result = xpsStateSearchData(scopeId, intfVlanTagTypeDbHndl,
                                     (xpsDbKey_t)&keyIntfVlan, (void**)&intfVlan)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, vlanId(%d) interface(%d)", vlanId, intfId);
        return result;
    }

    if ((!intfVlan) || (!xpsDeviceMapTestDeviceBit(&intfVlan->deviceMap, devId)))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT, "Resource not available");
        result = XP_ERR_RESOURCE_NOT_AVAILABLE;
        return result;
    }


    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsVlanIsIntfExist(xpsDevice_t devId, xpsVlan_t vlanId,
                             xpsInterfaceId_t intfId)
{
    XP_STATUS rc = XP_NO_ERR;
    XPS_FUNC_ENTRY_LOG();

    rc =  xpsVlanIsEndpointExist(devId, vlanId, intfId);

    XPS_FUNC_EXIT_LOG();
    return rc;
}

/*
   API has logic to get alll ports of a interface id. The usage of this api is only in vlan as of now.
   later if other features(l2mc,l3mc) also need similar api we can move to xpsInterface.c
   As of now we dont read individual ports from hw using vifmgr like in xp. There is no proper reason
   to have xpsInterfaceSetPort api(no  vif write reqd).   Hence not implemented in xpsInterface.
 */
static XP_STATUS xpsVlanGetInterfacePorts(xpsDevice_t devId,
                                          xpsInterfaceId_t intfId, xpsPortList_t *portList)
{
    XP_STATUS result = XP_NO_ERR;
    xpsScope_t scopeId;
    xpsLagPortIntfList_t portIntfList;
    xpsInterfaceType_e intfType;

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    // handle getting ports of each interface type as required
    //Get interfacetype
    result = xpsInterfaceGetTypeScope(scopeId, intfId, &intfType);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interface type failed for(%d)", intfId);
        return result;
    }
    if (intfType == XPS_PORT)
    {
        portList->portList[0]= intfId;
        portList->size=1;
    }
    else if (intfType == XPS_LAG)
    {
        cpssOsMemSet(&portIntfList, 0, sizeof(portIntfList));
        result = xpsLagGetPortIntfList(intfId, &portIntfList);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get ports failed,"
                  " devId(%d) for lag interface(%d)", devId, intfId);
            return result;
        }
        for (uint32_t i=0; i<portIntfList.size; i++)
        {
            portList->portList[i]=portIntfList.portIntf[i];
        }
        portList->size=portIntfList.size;
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsVlanGetInterfacePorts for %d failed - invalid type", intfId);
        return XP_ERR_INVALID_ARG;
    }

    return result;
}

XP_STATUS xpsVlanUpdatePvid(xpsDevice_t devId, xpsInterfaceId_t intfId,
                            xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    xpsPortList_t devPortList;
    XP_STATUS result = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    uint32_t cpssDevId;

    cpssOsMemSet(&devPortList, 0, sizeof(devPortList));

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    result = xpsVlanGetInterfacePorts(devId, intfId, &devPortList);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get ports failed,"
              " devId(%d) for interface(%d)", devId, intfId);
        return result;
    }

    for (uint32_t i = 0; i < devPortList.size; i++)
    {
        cpssDevId =   xpsGlobalIdToDevId(devId, devPortList.portList[i]);
        int port = xpsGlobalPortToPortnum(devId, devPortList.portList[i]);
        rc = cpssDxChBrgVlanPortVidSet(cpssDevId, port, CPSS_DIRECTION_INGRESS_E,
                                       vlanId);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Set pvid failed for portNum(%d)", intfId);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }
    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsVlanGetPvid(xpsDevice_t devId, xpsInterfaceId_t intfId,
                         xpsVlan_t *pvid)
{
    XPS_FUNC_ENTRY_LOG();
    xpsPortList_t devPortList;
    XP_STATUS result = XP_NO_ERR;
    GT_STATUS rc = GT_OK;

    cpssOsMemSet(&devPortList, 0, sizeof(devPortList));

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }
    result = xpsVlanGetInterfacePorts(devId, intfId, &devPortList);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get ports failed,"
              " devId(%d) for interface(%d)", devId, intfId);
        return result;
    }

    for (uint32_t i = 0; i < devPortList.size; i++)
    {
        int cpssDevId =   xpsGlobalIdToDevId(devId, devPortList.portList[i]);
        int port = xpsGlobalPortToPortnum(devId, devPortList.portList[i]);
        rc = cpssDxChBrgVlanPortVidGet(cpssDevId, port, CPSS_DIRECTION_INGRESS_E, pvid);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Set pvid failed for portNum(%d)", devPortList.portList[i]);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
        /*break after getting for first port in the interface*/
        break;
    }
    XPS_FUNC_EXIT_LOG();
    return result;
}




/* API makes the vlan as the default vlan of the system.

Updates the pvid of the ports to the default vlan unless a port is untagged
in another vlan. By default all ports pvid is set to 100 in FL which gDefaultVlan is also initialized
to 100. When a port is added to a vlan as untagged its pvid is updated. Hence such
ports' pvid must not be overwritten.

*/

XP_STATUS xpsVlanSetDefault(xpsDevice_t devId, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVlanSetDefault);

    int portNum = 0;
    XP_STATUS result = XP_NO_ERR;
    xpsVlan_t pvid;
    xpsVlanContextDbEntry *vlanCtx = NULL;
    xpsVlanStaticDbEntry * staticVarDb;
    xpsScope_t scopeId;
    uint8_t maxTotalPorts = 0;
    GT_STATUS rc = GT_OK;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if ((result = xpsGlobalSwitchControlGetMaxPorts(devId,
                                                    &maxTotalPorts)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get max physical port number. error code:%d\n", result);
    }
    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }

    result = xpsVlanGetStaticVariablesDb(scopeId, &staticVarDb);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get static variables db failed");
        return result;
    }

    //Secondary vlans cant be made default
    if ((vlanCtx->vlanType != VLAN_NONE)
        &&(vlanCtx->vlanType != VLAN_PRIMARY))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid arguments");
        result = XP_ERR_INVALID_ARG;
        return result;
    }

    XPS_GLOBAL_PORT_ITER(portNum, maxTotalPorts)
    {
        int port;
        port = xpsGlobalPortToPortnum(devId, portNum);
        rc = cpssDxChBrgVlanPortVidGet(devId, port, CPSS_DIRECTION_INGRESS_E, &pvid);

        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get port config, portNum(%d)",
                  portNum);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

        //Port is untagged in a vlan other than default. Dont overwrite pvid.
        if (staticVarDb->defaultVlan != pvid)
        {
            continue;
        }

        pvid= vlanId;

        result = xpsVlanUpdatePvid(devId, portNum, vlanId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set port config, portNum(%d)",
                  portNum);
            return result;
        }
    }

    //Set the global variable
    staticVarDb->defaultVlan = vlanId;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetDefault(xpsDevice_t devId, xpsVlan_t *vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    xpsVlanStaticDbEntry * staticVarDb;
    xpsScope_t scopeId;

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    result = xpsVlanGetStaticVariablesDb(scopeId, &staticVarDb);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get static variables db failed");
        return result;
    }

    *vlanId = staticVarDb->defaultVlan;
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanIsBd(xpsScope_t scopeId, xpsVlan_t vlanId, bool *isVlanBd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsVlanCreateBd(xpsDevice_t devId, xpsVlan_t *bdId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanCreate(xpsDevice_t devId, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsVlanCreate);

    XP_STATUS result = XP_NO_ERR;
    xpsVlanContextDbEntry *vlanCtx = NULL, keyVlanCtx;
    xpsStp_t defaultStg;
    xpsScope_t scopeId=0;

    CPSS_PORTS_BMP_STC  portsMembers;
    CPSS_PORTS_BMP_STC  portsTagging;
    GT_STATUS           rc = GT_OK;
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpss_vlan_info;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC
    ports_tagging_cmd; /* ports tagging command */
    uint32_t            entries = 0;

    memset(&cpss_vlan_info, 0, sizeof(cpss_vlan_info));
    memset(&portsMembers, 0, sizeof(CPSS_PORTS_BMP_STC));
    memset(&portsTagging, 0, sizeof(CPSS_PORTS_BMP_STC));
    memset(&ports_tagging_cmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* Fill Vlan info */
    cpss_vlan_info.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    cpss_vlan_info.ipv4IgmpToCpuEn      = GT_FALSE;
    cpss_vlan_info.ipv4McastRouteEn     = GT_FALSE;
    cpss_vlan_info.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    if (!cpssHalIsEbofDevice(devId))
    {
        cpss_vlan_info.ipv6IcmpToCpuEn      = GT_FALSE;
    }
    else
    {
        cpss_vlan_info.ipv6IcmpToCpuEn      = GT_TRUE;
    }
    cpss_vlan_info.ipv6McastRouteEn     = GT_FALSE;
    cpss_vlan_info.ipv4UcastRouteEn     = GT_TRUE;
    cpss_vlan_info.ipv6UcastRouteEn     = GT_TRUE;
    cpss_vlan_info.vrfId = XPS_VLAN_L3_DEFAULT_VRID;

    cpss_vlan_info.ipv4IpmBrgMode         = CPSS_BRG_IPM_SGV_E;
    cpss_vlan_info.ipv6IpmBrgMode         = CPSS_BRG_IPM_SGV_E;
    cpss_vlan_info.ipv4IpmBrgEn           = GT_TRUE;
    cpss_vlan_info.ipv6IpmBrgEn           = GT_TRUE;

    cpss_vlan_info.naMsgToCpuEn           = GT_TRUE;
    cpss_vlan_info.autoLearnDisable       = GT_FALSE;
    cpss_vlan_info.unkUcastCmd            = CPSS_PACKET_CMD_FORWARD_E;
    cpss_vlan_info.unregNonIpMcastCmd     = CPSS_PACKET_CMD_FORWARD_E;
    cpss_vlan_info.unregNonIpv4BcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    cpss_vlan_info.unregIpv4BcastCmd      = CPSS_PACKET_CMD_FORWARD_E;
    cpss_vlan_info.portIsolationMode      =
        CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    cpss_vlan_info.bcastUdpTrapMirrEn     = GT_FALSE;

    cpss_vlan_info.floodVidx              = XPS_VLAN_FLOOD_VIDX;
    cpss_vlan_info.floodVidxMode          =
        CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E;

    cpss_vlan_info.unregIpmEVidx          = XPS_VLAN_FLOOD_VIDX;
    cpss_vlan_info.unregIpmEVidxMode      =
        CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_BOTH_E;


    /* By defualt Control packets are not trapped to CPU
     * when control protocols are enabled, corresponding cfg  will be enabled
     * with this configuration contols packets are still switched with in a VLAN
     */
    if (!cpssHalIsEbofDevice(devId))
    {
        /* will be changed using the control packet table*/
        cpss_vlan_info.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_NONE_E;
    }
    else
    {
        cpss_vlan_info.ipCtrlToCpuEn = CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E;
    }
    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if ((vlanId <= XPS_VLANID_MIN) || (vlanId > XPS_VLANID_MAX))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    //Get default stg. All vlans created will be bound to default stg.
    //User can bind vlan to a new stp/ disable stp with explicit api calls
    if (stpMode_g==XPS_STP_MODE_VLAN_E)
    {
        result = xpsStpGetDefault(devId, &defaultStg);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting default stp failed");
            return result;
        }
    }
    memset(&keyVlanCtx, 0x0, sizeof(xpsVlanContextDbEntry));
    keyVlanCtx.vlanId= vlanId;

    if ((result = xpsStateSearchData(scopeId, vlanStateDbHndl,
                                     (xpsDbKey_t)&keyVlanCtx, (void**)&vlanCtx)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, vlanId(%d)", vlanId);
        return result;
    }

    if (!vlanCtx)
    {
        result = xpsVlanInsertVlanCtxDb(scopeId, vlanId, &vlanCtx);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Insert vlan context failed, vlanId(%d)", vlanId);
            return result;
        }

        //During vlan creation a vlan is not a private vlan. Need to call private vlan APIs to make it a part of private vlan.
        vlanCtx->vlanType = VLAN_NONE;
        vlanCtx->primaryVlanId = vlanId;

        vlanCtx->fdbLimit = XPS_MAXFDB_LIMIT;

        //Add the vlan into default-stg's vlan list
        if (stpMode_g==XPS_STP_MODE_VLAN_E)
        {
            result = xpsStgAddVlanDb(scopeId, defaultStg, vlanId);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsStgAddVlanD failed");
                return result;
            }
        }
    }

    /*If vlan is already present in the device then return*/
    if (xpsDeviceMapTestDeviceBit(&vlanCtx->deviceMap, devId))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Key already exists");
        result = XP_ERR_KEY_EXISTS;
        return result;
    }

    if ((rc = cpssHalWriteBrgVlanEntry(devId, vlanId, &portsMembers,
                                       &portsTagging, &cpss_vlan_info, &ports_tagging_cmd)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan %d create failed",
              vlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&portsMembers, entries);
    if ((rc = cpssHalSetBrgVlanMruProfileIdx(devId, vlanId, 0)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Mru profile ID 0 set failed");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    if ((rc = cpssHalSetBrgVlanForwardingId(devId, vlanId, vlanId)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "VlanForwardingIdSet failed");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    if (stpMode_g==XPS_STP_MODE_VLAN_E)
    {
        if ((rc = cpssHalBindBrgVlanToStpId(devId, vlanId,
                                            (GT_U16)defaultStg)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan to Stp bind failed");
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }
    //Set the bit for this device
    xpsDeviceMapSetDeviceBit(&vlanCtx->deviceMap, devId);

    if ((result= cpssHalCascadeAddPortsToVlan(vlanId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Vlan add cascade for devId(%d) vlanId(%d)", devId, vlanId);
        return result;
    }

    if ((rc = cpssHalEnableBrgVlanNaToCpu(devId, vlanId, GT_TRUE)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan Update to CPU failed");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanDestroy(xpsDevice_t devId, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVlanDestroy);

    XP_STATUS result = XP_NO_ERR;
    xpsVlanContextDbEntry *vlanCtx = NULL;
    xpsInterfaceId_t *intfList = NULL;
    uint32_t numOfIntfs = 0, i = 0, stgId;
    xpsVlanBridgeMcMode_e bridgeMode;
    xpsScope_t scopeId;
    GT_STATUS status = GT_OK;
    xpsVlan_t defaultVlanId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    //get default vlan and check for the requested vlan matches default vlan
    result = xpsVlanGetDefault(devId, &defaultVlanId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get defualt VlanId failed");
        return result;
    }
#if 0
    if (vlanId == defaultVlanId)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Deletion of default vlan is not supported");
        return XP_ERR_NOT_SUPPORTED;
    }
#endif
    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }

    result = xpsVlanGetIpv6McBridgeMode(devId, vlanId, &bridgeMode);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan IPv6 Bridge Mode  failed, vlanId(%d)", vlanId);
        return result;
    }
    result = xpsMcReleaseDestroyL2DomainId(devId, vlanId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Set ipv6 mc bridge mode failed as we couldnt release a L2 Domain Id");
        return result;
    }

    /*If vlan is not present in the device then return*/
    if (!xpsDeviceMapTestDeviceBit(&vlanCtx->deviceMap, devId))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Key is not found");
        result = XP_ERR_KEY_NOT_FOUND;
        return result;
    }

    //Get the stgId. Used later in the function to remove the vlan from stg-vlan list.
    if (stpMode_g==XPS_STP_MODE_VLAN_E)
    {
        result = xpsVlanGetStp(devId, vlanId, &stgId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get stp failed, devId(%d) vlanId(%d)", devId, vlanId);
            return result;
        }
    }
    /*Remove Port Vlans for the device
        Because of dynamic array, there is a chance that the remove interface may create a new vlan context
        So Need to store the interface list in another array*/

    numOfIntfs = vlanCtx->numOfIntfs;
    if ((result = xpsStateHeapMalloc(numOfIntfs*sizeof(xpsInterfaceId_t),
                                     (void **)&intfList)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Heap malloc failed");
        return result;
    }
    memcpy(intfList, vlanCtx->intfList, numOfIntfs*sizeof(xpsInterfaceId_t));

    for (i = 0; i < numOfIntfs; i++)
    {
        result = xpsVlanRemoveInterface(devId, vlanId, intfList[i]);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " Vlan remove interface for devId(%d) vlanId(%d)", devId, vlanId);
            xpsStateHeapFree((void*) intfList);
            return result;
        }
    }
    if ((result= cpssHalCascadeRemovePortsFromVlan(vlanId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Vlan remove cascade for devId(%d) vlanId(%d)", devId, vlanId);
        xpsStateHeapFree((void*) intfList);
        return result;
    }
    //Get vlanCtx again which is probably changed
    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get vlan from db failed");
        xpsStateHeapFree((void*) intfList);
        return result;
    }

    //Free the local interface list
    if ((result = xpsStateHeapFree((void*) intfList)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Heap freeing faild");
        return result;
    }

    status = cpssHalInvalidateBrgVlanEntry(devId, vlanId);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Destroy vlan failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }


    /*Clear the device bit for this vlan*/
    xpsDeviceMapClrDeviceBit(&vlanCtx->deviceMap, devId);

    /*Clear software state if necessary*/
    if (xpsDeviceMapIsEmpty(&vlanCtx->deviceMap))
    {
        //Add the vlan into default-stg's vlan list
        if (stpMode_g==XPS_STP_MODE_VLAN_E)
        {
            result = xpsStgRemoveVlanDb(scopeId, stgId, vlanId);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsStgRemoveVlanDb failed");
                return result;
            }
        }
        result = xpsVlanRemoveVlanCtxDb(scopeId, vlanId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Remove Interface entry failed"
                  "vlanId(%d) ", vlanId);
            return result;
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

// Removes all VLANs on specified devId instance.
XP_STATUS xpsVlanDeleteAll(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS result = XP_NO_ERR;
    xpsVlan_t devicePvid = 0;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if (xpsVlanGetDefault(devId, &devicePvid) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting default vlan failed");
        result = XP_ERR_RESOURCE_NOT_AVAILABLE;
        return result;
    }

    for (uint16_t vlanIndx = XPS_VLANID_MIN; vlanIndx <= XPS_VLANID_MAX; vlanIndx++)
    {
        // Default VLAN should not be removed
        if (devicePvid == vlanIndx)
        {
            continue;
        }

        if (xpsVlanDestroy(devId, vlanIndx) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Deleting operation of vlan failed");
            result = xpsVlanDestroy(devId, vlanIndx);
            return result;
        }
    }

    XPS_FUNC_EXIT_LOG();
    return result;
}



/*
VLAN show command.
*/

const char *xpsVlanGetEncapTypeString(xpL2EncapType_e encapType)
{
    switch (encapType)
    {
        case XP_L2_ENCAP_DOT1Q_UNTAGGED:
            return "802.1Q Untagged";

        case XP_L2_ENCAP_DOT1Q_TAGGED:
            return "802.1Q tagged";

        case XP_L2_ENCAP_DOT1Q_PRIOTAGGED:
            return "802.1Q Priority Tagged";

        case XP_L2_ENCAP_QINQ_UNTAGGED:
            return "QinQ Untagged";

        case XP_L2_ENCAP_QINQ_CTAGGED:
            return "QinQ C_Tagged";

        case XP_L2_ENCAP_QINQ_STAGGED:
            return "QinQ S_Tagged";

        case XP_L2_ENCAP_VXLAN:
            return "Vxlan";

        case XP_L2_ENCAP_NVGRE:
            return "Nvgre";

        case XP_L2_ENCAP_PBB:
            return "Pbb";

        case XP_L2_ENCAP_GENEVE:
            return "Geneve";

        case XP_L2_ENCAP_MPLS:
            return "Mpls";

        case XP_L2_ENCAP_NO_OP:
            return "No op";

        case XP_L2_ENCAP_MAX:
        default:
            return "Invalid type";
    }
}

XP_STATUS xpsConvertPktCmdToCpssEnable(xpsPktCmd_e xpsPktCmd, GT_BOOL* enable)
{
    switch (xpsPktCmd)
    {
        case XP_PKTCMD_FWD_MIRROR:
            *enable = GT_TRUE;
            break;
        case XP_PKTCMD_FWD:
            *enable = GT_FALSE;
            break;
        default:
            /* trapping or mirroring to cpu is allowed */
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Only XP_PKTCMD_TRAP(enable)/XP_PKTCMD_DROP(disable) pkt cmd is supported");
            return XP_ERR_INVALID_PARAMS;
    }
    return XP_NO_ERR;
}

XP_STATUS xpsConvertCpssEnableToPktCmd(xpsPktCmd_e* xpsPktCmd, GT_BOOL enable)
{
    switch (enable)
    {
        case GT_TRUE:
            *xpsPktCmd = XP_PKTCMD_FWD_MIRROR;
            break;
        case GT_FALSE:
            *xpsPktCmd = XP_PKTCMD_FWD;
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid params");
            return XP_ERR_INVALID_PARAMS;
    }
    return XP_NO_ERR;
}

XP_STATUS xpsVlanShow(xpsDevice_t devId, xpsVlan_t vlanId)
{

    XPS_FUNC_ENTRY_LOG();
    uint8_t maxTotalPorts = 0;
    int                                ret;
    xpsVlan_t prevVlanId = 0;
    CPSS_PORTS_BMP_STC                   globalPortMembers;
    CPSS_PORTS_BMP_STC                   portsMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    GT_BOOL                              isValid;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    uint8_t isShowAll = 0;
    XP_STATUS result = XP_NO_ERR;
    int     port                                 = 0;  /* current port number */
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT currTagCmd =
        CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    int maxPorts;
    GT_PORT_NUM     cpssPortNum;
    cpssHalGetMaxGlobalPorts(&maxPorts);

    memset(&portsMembers, 0, sizeof(portsMembers));
    memset(&portsTagging, 0, sizeof(portsTagging));
    memset(&vlanInfo, 0, sizeof(vlanInfo));
    memset(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));

    if ((result = xpsGlobalSwitchControlGetMaxPorts(devId,
                                                    &maxTotalPorts)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get max physical port number. error code:%d\n", result);
        return result;
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid device id");
        return XP_ERR_INVALID_DEV_ID;
    }

    if ((vlanId > XPS_VLANID_MAX) && (vlanId != XPS_VLAN_SHOW_ALL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlan id");
        return XP_ERR_INVALID_VLAN_ID;
    }


    if (vlanId == XPS_VLAN_SHOW_ALL)
    {
        result = xpsVlanGetFirst(devId, &vlanId);
        if ((result != XP_NO_ERR) && (result != XP_ERR_NOT_FOUND))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting first vlan failed");
            return result;
        }

        isShowAll = 1;
    }

    while (1)
    {

        memset(&globalPortMembers, 0, sizeof(globalPortMembers));

        cpssOsPrintf("\n------------------------------------------------------------------");
        cpssOsPrintf("\n802.1Q Tag             Port members                    ");
        cpssOsPrintf("\n------------------------------------------------------------------");
        cpssOsPrintf("\n%9d\t", vlanId);

        if ((ret = cpssHalReadBrgVlanEntry(devId, (GT_U16)vlanId, &portsMembers,
                                           &portsTagging, &vlanInfo, &isValid, &portsTaggingCmd,
                                           &globalPortMembers)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Read vlan %d failed .",
                  vlanId);
            return xpsConvertCpssStatusToXPStatus(ret);
        }

        XPS_GLOBAL_PORT_ITER(port, maxPorts)
        {
            cpssPortNum = xpsGlobalPortToPortnum(devId, port);
            if (cpssPortNum == 0xffff)
            {
                cpssOsPrintf("Invalid Port %d,", port);
                return xpsConvertCpssStatusToXPStatus(GT_FAIL);
            }
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&globalPortMembers, cpssPortNum))
            {
                cpssOsPrintf("%d,", cpssPortNum);
            }
        }

        cpssOsPrintf("\n-------------------------\n");
        cpssOsPrintf("Dev/Port Tagging commands\n");
        cpssOsPrintf("-------------------------\n");
        devId =0;
        XPS_GLOBAL_PORT_ITER(port, maxTotalPorts)
        {
            cpssPortNum = xpsGlobalPortToPortnum(devId, port);
            if (cpssPortNum == 0xffff)
            {
                cpssOsPrintf("Invalid Port %d,", port);
                return xpsConvertCpssStatusToXPStatus(GT_FAIL);
            }
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&globalPortMembers, cpssPortNum))
            {
                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
                devId = xpsGlobalIdToDevId(devId, port);
                if ((ret = cpssDxChBrgVlanEntryRead(devId, (GT_U16)vlanId, &portsMembers,
                                                    &portsTagging, &vlanInfo, &isValid, &portsTaggingCmd)) != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Read vlan %d failed .",
                          vlanId);
                    return xpsConvertCpssStatusToXPStatus(ret);
                }

                currTagCmd = portsTaggingCmd.portsCmd[port];
                cpssOsPrintf("  %d/%02d   %d\n", devId, port, currTagCmd);
            }
        }

        cpssOsPrintf("\n-------------------------\n");
        if (!isShowAll)
        {
            break;
        }
        prevVlanId = vlanId;
        result = xpsVlanGetNext(devId, prevVlanId, &vlanId);
        if (result == XP_ERR_NOT_FOUND)
        {
            break;
        }
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting next vlan failed");
            return result;
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;

}


/*
VLAN show detail command.
*/

XP_STATUS xpsVlanShowDetail(xpsDevice_t devId, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();


    //XPS_LOCK(xpsVlanShowDetail);
    xpsPktCmd_e ipv6IcmpToCpuCmd;
    xpsPktCmd_e ipv4IgmpToCpuCmd;
    uint8_t maxTotalPorts =0;
    CPSS_PORTS_BMP_STC                   portsMembers;
    CPSS_PORTS_BMP_STC                   globalPortMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    GT_BOOL                              isValid;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    int     port                                 = 0;
    int ret;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT currTagCmd =
        CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;

    xpsScope_t scopeId;
    XP_STATUS result = XP_NO_ERR;
    xpsVlanContextDbEntry *vlanCtx = NULL;
    xpsVlan_t prevVlanId = 0;
    uint8_t isShowAll = 0;
    GT_PORT_NUM     cpssPortNum;
    memset(&portsMembers, 0, sizeof(portsMembers));
    memset(&portsTagging, 0, sizeof(portsTagging));
    memset(&vlanInfo, 0, sizeof(vlanInfo));
    memset(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));

    if ((result = xpsGlobalSwitchControlGetMaxPorts(devId,
                                                    &maxTotalPorts)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get max physical port number. error code:%d\n", result);
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid device id");
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }


    if ((vlanId > XPS_VLANID_MAX) && (vlanId != XPS_VLAN_SHOW_ALL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlan id");
        return XP_ERR_INVALID_VLAN_ID;
    }


    if (vlanId == XPS_VLAN_SHOW_ALL)
    {
        result = xpsVlanGetFirst(devId, &vlanId);
        if ((result != XP_NO_ERR) && (result != XP_ERR_NOT_FOUND))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting first vlan failed");
            return result;
        }

        isShowAll = 1;
    }

    while (1)
    {

        memset(&globalPortMembers, 0, sizeof(globalPortMembers));

        result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting vlan from db failed");
            return result;
        }

        cpssOsPrintf("\n------------------------------------------------------------------");
        cpssOsPrintf("\n802.1Q Tag             Port members                    ");
        cpssOsPrintf("\n------------------------------------------------------------------");
        cpssOsPrintf("\n%9d\t", vlanId);
        if ((ret = cpssHalReadBrgVlanEntry(devId, (GT_U16)vlanId, &portsMembers,
                                           &portsTagging, &vlanInfo, &isValid, &portsTaggingCmd,
                                           &globalPortMembers)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Read vlan %d failed .",
                  vlanId);
            return xpsConvertCpssStatusToXPStatus(ret);
        }


        XPS_GLOBAL_PORT_ITER(port, maxTotalPorts)
        {
            cpssPortNum = xpsGlobalPortToPortnum(devId, port);
            if (cpssPortNum == 0xffff)
            {
                cpssOsPrintf("Invalid Port %d,", port);
                return xpsConvertCpssStatusToXPStatus(GT_FAIL);
            }
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&globalPortMembers, cpssPortNum))
            {
                cpssOsPrintf("%d", cpssPortNum);
            }
        }
        if ((xpsConvertCpssEnableToPktCmd(&ipv6IcmpToCpuCmd,
                                          vlanInfo.ipv6IcmpToCpuEn) != XP_NO_ERR))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " %d ", XP_ERR_INVALID_DATA);
        }

        if ((xpsConvertCpssEnableToPktCmd(&ipv4IgmpToCpuCmd,
                                          vlanInfo.ipv4IgmpToCpuEn) != XP_NO_ERR))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " %d ", XP_ERR_INVALID_DATA);
        }

        cpssOsPrintf("\n-----------------------------------------------------\n");
        cpssOsPrintf("vlan Info\n");
        cpssOsPrintf("-----------------------------------------------------\n");

        cpssOsPrintf("unkSrcAddrSecBreach  =%d\t", vlanInfo.unkSrcAddrSecBreach);
        cpssOsPrintf("ipv4UcastRouteEn     =%d\n", vlanInfo.ipv4UcastRouteEn);
        cpssOsPrintf("unregNonIpMcastCmd   =%d\t",
                     xpsConvertPktCmdToXpsPktCmd(vlanInfo.unregNonIpMcastCmd));
        cpssOsPrintf("ipv4McastRouteEn     =%d\n", vlanInfo.ipv4McastRouteEn);
        cpssOsPrintf("unregIpv4McastCmd    =%d\t",
                     xpsConvertPktCmdToXpsPktCmd(vlanInfo.unregIpv4McastCmd));
        cpssOsPrintf("ipv6UcastRouteEn     =%d\n", vlanInfo.ipv6UcastRouteEn);
        cpssOsPrintf("unregIpv6McastCmd    =%d\t",
                     xpsConvertPktCmdToXpsPktCmd(vlanInfo.unregIpv6McastCmd));
        cpssOsPrintf("ipv6McastRouteEn     =%d\n", vlanInfo.ipv6McastRouteEn);
        cpssOsPrintf("unkUcastCmd          =%d\t",
                     xpsConvertPktCmdToXpsPktCmd(vlanInfo.unkUcastCmd));
        cpssOsPrintf("stgId                =%d\n", vlanInfo.stgId);
        cpssOsPrintf("unregIpv4BcastCmd    =%d\t",
                     xpsConvertPktCmdToXpsPktCmd(vlanInfo.unregIpv4BcastCmd));
        cpssOsPrintf("autoLearnDisable     =%d\n", vlanInfo.autoLearnDisable);
        cpssOsPrintf("unregNonIpv4BcastCmd =%d\t",
                     xpsConvertPktCmdToXpsPktCmd(vlanInfo.unregNonIpv4BcastCmd));
        cpssOsPrintf("naMsgToCpuEn         =%d\n", vlanInfo.naMsgToCpuEn);
        cpssOsPrintf("ipv4IgmpToCpuEn      =%d\t", ipv4IgmpToCpuCmd);
        cpssOsPrintf("mruIdx               =%d\n", (int)vlanInfo.mruIdx);
        cpssOsPrintf("mirrToRxAnalyzerEn   =%d\t", vlanInfo.mirrToRxAnalyzerEn);
        cpssOsPrintf("bcastUdpTrapMirrEn   =%d\n", vlanInfo.bcastUdpTrapMirrEn);
        cpssOsPrintf("ipv6IcmpToCpuEn      =%d\t", ipv6IcmpToCpuCmd);
        cpssOsPrintf("vrfId                =%d\n", (int)vlanInfo.vrfId);
        cpssOsPrintf("ipCtrlToCpuEn        =%d\t", vlanInfo.ipCtrlToCpuEn);
        cpssOsPrintf("floodVidx            =%d\n", vlanInfo.floodVidx);
        cpssOsPrintf("ipv4IpmBrgMode       =%d\t", vlanInfo.ipv4IpmBrgMode);
        cpssOsPrintf("floodVidxMode        =%d\n", vlanInfo.floodVidxMode);
        cpssOsPrintf("ipv6IpmBrgMode       =%d\t", vlanInfo.ipv6IpmBrgMode);
        cpssOsPrintf("portIsolationMode    =%d\n", vlanInfo.portIsolationMode);
        cpssOsPrintf("ipv4IpmBrgEn         =%d\t", vlanInfo.ipv4IpmBrgEn);
        cpssOsPrintf("ucastLocalSwitchingEn=%d\n", vlanInfo.ucastLocalSwitchingEn);
        cpssOsPrintf("ipv6IpmBrgEn         =%d\t", vlanInfo.ipv6IpmBrgEn);
        cpssOsPrintf("mcastLocalSwitchingEn=%d\n", vlanInfo.mcastLocalSwitchingEn);
        cpssOsPrintf("ipv6SiteIdMode       =%d\n", vlanInfo.ipv6SiteIdMode);

        cpssOsPrintf("\n-----------------------------------------------------\n");

        cpssOsPrintf("Usage:\n");
        cpssOsPrintf("\n");
        cpssOsPrintf("%d Untagged     - ifTag0 and/or Tag1 were classified in the incoming packet,      \n",
                     CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);
        cpssOsPrintf("                  they are removed from the packet.                               \n");
        cpssOsPrintf("%d Tag0         - Packet egress with Tag0 (Tag0 as defined in ingress pipe). \n",
                     CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E);
        cpssOsPrintf("%d Tag1         - Packet egress with Tag1 (Tag1 as defined in ingress pipe). \n",
                     CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E);
        cpssOsPrintf("%d Out_T0_in_T1 - Outer Tag0, Inner Tag1 (tag swap).                   \n",
                     CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
        cpssOsPrintf("%d Out_T1_in_T0 - Outer Tag1, Inner Tag0 (tag swap).                    \n",
                     CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E);
        cpssOsPrintf("%d Push_Tag0    - TAG0 is added to the packet regardless of whether Tag0 and TAG1 \n",
                     CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E);
        cpssOsPrintf("                  were classified in the incoming packet.                         \n");
        cpssOsPrintf("%d Pop_Out_Tag  - The incoming packet outer tag is removed,                      \n",
                     CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E);
        cpssOsPrintf("                  regardless of whether it is Tag0 or Tag1.                       \n");
        cpssOsPrintf("                  This operation is a NOP ifthe packet arrived with neither       \n");
        cpssOsPrintf("                  Tag0 nor Tag1 classified.                                       \n");
        cpssOsPrintf("\n-------------------------\n");
        cpssOsPrintf("Dev/Port Tagging commands\n");
        cpssOsPrintf("-------------------------\n");
        /*Macro iterates through all the ports on that device*/
        /*TODO to get max no of ports through cpss call*/
        XPS_GLOBAL_PORT_ITER(port, maxTotalPorts)
        {
            cpssPortNum = xpsGlobalPortToPortnum(devId, port);
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&globalPortMembers, cpssPortNum))
            {
                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
                if ((ret = cpssDxChBrgVlanEntryRead(devId, (GT_U16)vlanId, &portsMembers,
                                                    &portsTagging, &vlanInfo, &isValid, &portsTaggingCmd)) != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Read vlan %d failed .",
                          vlanId);
                    return xpsConvertCpssStatusToXPStatus(ret);
                }

                currTagCmd = portsTaggingCmd.portsCmd[cpssPortNum];
                cpssOsPrintf("  %d/%02d   %d\n", devId, cpssPortNum, currTagCmd);
            }
        }
        cpssOsPrintf("\n-------------------------\n");

        if (!isShowAll)
        {
            break;
        }
        prevVlanId = vlanId;
        result = xpsVlanGetNext(devId, prevVlanId, &vlanId);
        if (result == XP_ERR_NOT_FOUND)
        {
            break;
        }
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting next vlan failed");
            return result;
        }

        result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Getting vlan info from db failed");
            return result;
        }

    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;


}

XP_STATUS xpsVlanRemoveTunnelFromSWStateOnly(xpsDevice_t devId,
                                             xpsVlan_t vlanId, xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanAddTunnelToSWStateOnly(xpsDevice_t devId, xpsVlan_t vlanId,
                                        xpsInterfaceId_t intfId, xpsL2EncapType_e tagType, uint32_t data)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetRoutingEnable(xpsDevice_t devId, xpsVlan_t vlanId,
                                  XPS_PROTOCOL_ENT protocol, bool isMcast,
                                  bool *enable)
{
    xpsVlanContextDbEntry * vlanCtx = NULL;
    XP_STATUS               result = XP_NO_ERR;
    xpsScope_t scopeId;

    result = xpsScopeGetScopeId(devId, &scopeId);

    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (vlanCtx == NULL || result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }

    switch (protocol)
    {
        case XPS_PROTOCOL_IPV4_E:
            if (isMcast)
            {
                *enable = vlanCtx->isIpv4McRoutingEn;
            }
            else
            {
                *enable = vlanCtx->isIpv4RoutingEn;
            }
            break;
        case XPS_PROTOCOL_IPV6_E:
            if (isMcast)
            {
                *enable = vlanCtx->isIpv6McRoutingEn;
            }
            else
            {
                *enable = vlanCtx->isIpv6RoutingEn;
            }
            break;
        default:
            return XP_ERR_INVALID_DATA;
            break;
    }
    return result;
}

XP_STATUS xpsVlanSetRoutingEnable(xpsDevice_t devId, xpsVlan_t vlanId,
                                  XPS_PROTOCOL_ENT protocol, bool isMcast, bool enable)
{
    xpsVlanContextDbEntry * vlanCtx = NULL;
    XP_STATUS               result = XP_NO_ERR;
    xpsScope_t scopeId;

    result = xpsScopeGetScopeId(devId, &scopeId);
    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }
    switch (protocol)
    {
        case XPS_PROTOCOL_IPV4_E:
            if (isMcast)
            {
                vlanCtx->isIpv4McRoutingEn = enable;
            }
            else
            {
                vlanCtx->isIpv4RoutingEn = enable;
            }
            break;
        case XPS_PROTOCOL_IPV6_E:
            if (isMcast)
            {
                vlanCtx->isIpv6McRoutingEn = enable;
            }
            else
            {
                vlanCtx->isIpv6RoutingEn = enable;
            }
            break;
        default:
            return XP_ERR_INVALID_DATA;
            break;

    }

    return result;
}

XP_STATUS xpsVlanAddEndpoint(xpsDevice_t devId, xpsVlan_t vlanId,
                             xpsInterfaceId_t intfId, xpsL2EncapType_e tagType, uint32_t data)
{
    XPS_FUNC_ENTRY_LOG();
    GT_STATUS rc = GT_OK;
    XP_STATUS result = XP_NO_ERR;
    xpsInterfaceType_e intfType = XPS_PORT;
    xpsVlanContextDbEntry * vlanCtx = NULL;
    xpsScope_t scopeId;
    xpLagDbEntry_t     lagInfoKey;
    xpLagDbEntry_t    *lagInfo = NULL;
    uint32_t cpssDevId;
    uint32_t cpssPortNum ;
    if (IS_ENCAP_VALID(tagType) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid tagType (%d)",
              tagType);
        return XP_ERR_INVALID_ARG;
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }

    xpsIntfVlanEncapInfoDbEntry *intfVlan = NULL;
    /*This function could be called -
      1. very first time(allocate + add + software states, set bit)
      2. not first time another device (add , set bit)
      3. not first time same device ( check device and return)*/
    result = xpsVlanGetIntfEntryDb(scopeId, vlanId, intfId, &intfVlan);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get interface entry failed,"
              "vlanId(%d) interface(%d)", vlanId, intfId);
        return result;
    }
    if (intfVlan)
    {
        /*If vlan is already present in the device then return*/
        if (xpsDeviceMapTestDeviceBit(&intfVlan->deviceMap, devId))
        {
            return XP_ERR_KEY_EXISTS;
        }

        if (intfVlan->tagType != tagType)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid tag type");
            result = XP_ERR_INVALID_ARG;
            return result;
        }
    }

    xpsInterfaceInfo_t *intfInfo = NULL;
    result = xpsInterfaceGetInfoScope(scopeId, intfId, &intfInfo);
    if (result != XP_NO_ERR || intfInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get intf id %d\n", intfId);
        return result;
    }
    //Get interfacetype
    intfType = intfInfo->type;

    if (!intfVlan)
    {

        /*Interface is not present in any device. Create the software states and set the device mask*/

        result = xpsVlanInsertIntfEntryDb(scopeId, vlanId, intfId, tagType, data,
                                          &intfVlan);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " Insert interface entry failed, "
                  "vlanId(%d) interface(%d)", vlanId, intfId);
            return result;
        }

        if ((intfType == XPS_LAG))
        {
            result = xpsVlanAddVlanInLagDb(scopeId, intfId, vlanId, tagType);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Add vlan in lag db failed"
                      "vlanId(%d) interface(%d)", vlanId, intfId);
                return result;
            }
        }

        result = xpsVlanAddIntfDb(scopeId, vlanId, intfId, &vlanCtx);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Add interface db failed"
                  "vlanId(%d) interface(%d)", vlanId, intfId);
            return result;
        }

        xpsDeviceMapSetDeviceBit(&intfVlan->deviceMap, devId);
    }


    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT cpssTagType =
        CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    GT_BOOL         tag = GT_FALSE;

    /*According to interface type switch to different l2domain apis*/
    switch (intfType)
    {
        case XPS_PORT:
        case XPS_EXTENDED_PORT:
            if (tagType == XP_L2_ENCAP_DOT1Q_TAGGED)
            {
                cpssTagType = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
                tag = GT_TRUE;
            }

            /*Update port's default VLAN for untagged ports*/
            if (tagType == XP_L2_ENCAP_DOT1Q_UNTAGGED)
            {
                result = xpsVlanUpdatePvid(devId, intfId, vlanId);
                if (result != XP_NO_ERR)
                {
                    return result;
                }
            }

            cpssDevId = xpsGlobalIdToDevId(devId, intfId);
            cpssPortNum = xpsGlobalPortToPortnum(devId, intfId);
            // TODO: need to get Interface ID for logical ports like Lag/..
            if ((rc = cpssDxChBrgVlanMemberAdd(cpssDevId, vlanId, cpssPortNum, tag,
                                               cpssTagType)) != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpss vlanmemberadd failed for vlan %d", vlanId);
                return xpsConvertCpssStatusToXPStatus(rc);
            }

            // Set accepted frame type to ALL
            rc = cpssDxChBrgVlanPortAccFrameTypeSet(cpssDevId, cpssPortNum,
                                                    CPSS_PORT_ACCEPT_FRAME_ALL_E);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set accepted frame type for port id %d\n", cpssPortNum);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
            rc = cpssDxChBrgFdbNaToCpuPerPortSet(cpssDevId, cpssPortNum, GT_TRUE);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "failed to enable forwarding of new MAC to CPUfor packets recieved on port %d\n",
                      cpssPortNum);
                return xpsConvertCpssStatusToXPStatus(rc);
            }


            if (vlanCtx->isIpv4RoutingEn || vlanCtx->isIpv6RoutingEn)
            {
                if ((result = xpsL3UpdateL3VlanMember(devId, vlanId, intfId,
                                                      vlanCtx->isIpv4RoutingEn, vlanCtx->isIpv6RoutingEn)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update l3 vlan member (%d) failed for vlan (%d) \n", intfId, vlanId);
                    return result;
                }
            }
            if (vlanCtx->isIpv4McRoutingEn || (vlanCtx->mcKeyType > MC_BRIDGE_MODE_FDB &&
                                               vlanCtx->mcKeyType < MC_BRIDGE_MODE_MAX))
            {
                if ((rc = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId,
                                                       cpssPortNum,
                                                       CPSS_IP_PROTOCOL_IPV4_E,
                                                       GT_TRUE)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update MC Routing vlan member (%d) failed for vlan (%d) \n", intfId, vlanId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            if (vlanCtx->isIpv6McRoutingEn)
            {
                if ((rc = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId,
                                                       cpssPortNum,
                                                       CPSS_IP_PROTOCOL_IPV6_E,
                                                       GT_TRUE)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update MC Routing vlan member (%d) failed for vlan (%d) \n", intfId, vlanId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }

            if (vlanCtx->isIpv4RoutingEn || vlanCtx->isIpv6RoutingEn ||
                vlanCtx->isIpv4McRoutingEn || vlanCtx->isIpv6McRoutingEn)
            {
                if ((result = xpsAclCommonDropRuleUpdate(cpssDevId, cpssPortNum,
                                                         true)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "xpsAclCommonDropRuleUpdate port (%d) failed for vlan (%d) \n",
                          cpssPortNum, vlanId);
                    return result;
                }
            }

            result = xpsIntfChildAdd(intfInfo, vlanId, XPS_VLAN, vlanId);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to add vlan %d to port %d\n", vlanId,
                      intfId);
                return result;
            }

            break;
        case XPS_LAG:

            if (tagType == XP_L2_ENCAP_DOT1Q_TAGGED)
            {
                cpssTagType = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
                tag = GT_TRUE;
            }
            lagInfoKey.lagIntf = intfId;
            // Retrieve the lag information for this id if it exists
            if ((result = xpsStateSearchData(scopeId, lagDbHandle, (xpsDbKey_t)&lagInfoKey,
                                             (void**)&lagInfo)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Search for Lag interface failed, lag interface(%d)", intfId);
                return result;
            }
            if (!lagInfo)
            {
                result = XP_ERR_NOT_FOUND;
                return result;
            }
            for (uint32_t i = 0; i < XPS_LAG_MAX_PORTS; i++)
            {
                if ((lagInfo->portIntf[i] != XPS_INTF_INVALID_ID))
                {
                    cpssDevId = xpsGlobalIdToDevId(devId, lagInfo->portIntf[i]);
                    cpssPortNum = xpsGlobalPortToPortnum(devId, lagInfo->portIntf[i]);
                    // TODO: need to get Interface ID for logical ports like Lag/..
                    if ((rc = cpssDxChBrgVlanMemberAdd(cpssDevId, vlanId, cpssPortNum, tag,
                                                       cpssTagType)) != GT_OK)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "cpss vlanmemberadd failed for vlan %d", vlanId);
                        return xpsConvertCpssStatusToXPStatus(rc);
                    }
                    /*Update port's default VLAN for untagged ports*/
                    if (tagType == XP_L2_ENCAP_DOT1Q_UNTAGGED)
                    {
                        result = xpsVlanUpdatePvid(devId, lagInfo->portIntf[i], vlanId);
                        if (result != XP_NO_ERR)
                        {
                            return result;
                        }
                    }

                    // Set accepted frame type to ALL
                    rc = cpssDxChBrgVlanPortAccFrameTypeSet(cpssDevId, cpssPortNum,
                                                            CPSS_PORT_ACCEPT_FRAME_ALL_E);
                    if (rc != GT_OK)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Failed to set accepted frame type for port id %d\n", cpssPortNum);
                        return xpsConvertCpssStatusToXPStatus(rc);
                    }

                    rc = cpssDxChBrgFdbNaToCpuPerPortSet(cpssDevId, cpssPortNum, GT_TRUE);
                    if (rc != GT_OK)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "failed to enable forwarding of new MAC to CPUfor packets recieved on port %d\n",
                              cpssPortNum);
                        return xpsConvertCpssStatusToXPStatus(rc);
                    }

                    if (vlanCtx->isIpv4RoutingEn || vlanCtx->isIpv6RoutingEn)
                    {
                        if ((result = xpsL3UpdateL3VlanMember(devId, vlanId, lagInfo->portIntf[i],
                                                              vlanCtx->isIpv4RoutingEn, vlanCtx->isIpv6RoutingEn)) != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "Update l3 vlan member (%d) failed for vlan (%d) \n", intfId, vlanId);
                            return result;
                        }
                    }
                    if ((vlanCtx->isIpv4McRoutingEn) || (vlanCtx->mcKeyType > MC_BRIDGE_MODE_FDB &&
                                                         vlanCtx->mcKeyType < MC_BRIDGE_MODE_MAX))
                    {
                        if ((rc = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId,
                                                               cpssPortNum,
                                                               CPSS_IP_PROTOCOL_IPV4_E,
                                                               GT_TRUE)) != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "Update MC Routing vlan member (%d) failed for vlan (%d) \n",
                                  intfId, vlanId);
                            return xpsConvertCpssStatusToXPStatus(rc);
                        }
                    }
                    if (vlanCtx->isIpv6McRoutingEn)
                    {
                        if ((rc = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId,
                                                               cpssPortNum,
                                                               CPSS_IP_PROTOCOL_IPV6_E,
                                                               GT_TRUE)) != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "Update MC Routing vlan member (%d) failed for vlan (%d) \n",
                                  intfId, vlanId);
                            return xpsConvertCpssStatusToXPStatus(rc);
                        }
                    }

                    if (vlanCtx->isIpv4RoutingEn || vlanCtx->isIpv6RoutingEn ||
                        vlanCtx->isIpv4McRoutingEn || vlanCtx->isIpv6McRoutingEn)
                    {
                        if ((result = xpsAclCommonDropRuleUpdate(cpssDevId,
                                                                 cpssPortNum, true)) != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "xpsAclCommonDropRuleUpdate port (%d) failed for vlan (%d) \n",
                                  lagInfo->portIntf[i], vlanId);
                            return result;
                        }
                    }
                }
            }
            break;
        case XPS_TUNNEL_MPLS:
        case XPS_TUNNEL_VXLAN:
        case XPS_TUNNEL_NVGRE:
        case XPS_TUNNEL_GENEVE:
        case XPS_TUNNEL_PBB:
            /*if ((result = xpL2DomainMgr::instance()->addTunnel(devId,(xpTunnel_t)intfId, vlanCtx->l2DomainContext,(xpL2EncapType_e)tagType, l2EncapData)) != XP_NO_ERR)
              {
              LOGFN(xpLogModXps,XP_SUBMOD_MAIN,XP_LOG_ERROR,"Add tunnel failed, interface(%d)", intfId);
              return result;
              }*/
            break;
        default:
            break;
    }

    if ((vlanCtx->mappedVni) && (intfType == XPS_PORT || intfType == XPS_LAG ||
                                 intfType == XPS_TUNNEL_VXLAN))
    {
        result = xpsVxlanAddMemberToVniDbEntry(devId, vlanCtx->mappedVni,
                                               vlanId, intfId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " Vlan to Vni failed, "
                  "vlanId(%d) vniId(%d)", vlanId, vlanCtx->mappedVni);
            return result;
        }
    }
    else if (vlanCtx->isIpv4RoutingEn || vlanCtx->isIpv6RoutingEn) /*NNI SVI*/
    {
        xpsInterfaceId_t    l3IntfId;
        xpsInterfaceInfo_t *l3IntfInfo = NULL;

        l3IntfId = XPS_INTF_MAP_BD_TO_INTF(vlanId);
        result = xpsInterfaceGetInfoScope(devId, l3IntfId, &l3IntfInfo);
        if ((result != XP_NO_ERR) || (l3IntfInfo == NULL))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get interface information for L3 Interface id : %d\n", l3IntfId);
            return result;
        }

        result = xpsVxlanUpdateNNIPortOnSVI(devId, l3IntfInfo, intfId, true);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsVxlanUpdateNNIPortOnSVI Failed : %d \n", intfId);
            return result;
        }
    }

    XPS_FUNC_EXIT_LOG();

    return result;
}


XP_STATUS xpsVlanAddInterface(xpsDevice_t devId, xpsVlan_t vlanId,
                              xpsInterfaceId_t intfId, xpsL2EncapType_e tagType)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS result = XP_NO_ERR;
    uint32_t serviceId = vlanId;
    xpsScope_t scopeId = XP_SCOPE_DEFAULT;

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }


    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    result = xpsVlanAddEndpoint(devId, vlanId, intfId, tagType, serviceId);

    if (result!=XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan add endpoint failed");
        return result;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanRemoveEndpoint(xpsDevice_t devId, xpsVlan_t vlanId,
                                xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS result = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsInterfaceType_e intfType= XPS_PORT;
    xpsIntfVlanEncapInfoDbEntry *intfVlan = NULL;
    xpsVlanContextDbEntry *vlanCtx = NULL;
    xpL2EncapType_e tagType;
    xpsScope_t scopeId;
    xpLagDbEntry_t     lagInfoKey;
    xpLagDbEntry_t    *lagInfo = NULL;
    int cpssDevId;
    int cpssPortNum ;
    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }


    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }

    result = xpsVlanGetIntfEntryDb(scopeId, vlanId, intfId, &intfVlan);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interface entry failed, interface(%d)", intfId);
        return result;
    }

    if (!intfVlan)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Interface is not added in vlanId(%d)", vlanId);
        result = XP_ERR_KEY_NOT_FOUND;
        /*VLAN member already removed */
        result = XP_NO_ERR;
        return result;
    }
    else
    {
        /*If not present in this device then return*/
        if (!xpsDeviceMapTestDeviceBit(&intfVlan->deviceMap, devId))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Key not found");
            result = XP_ERR_KEY_NOT_FOUND;
            return result;
        }
        xpsDeviceMapClrDeviceBit(&intfVlan->deviceMap, devId);
    }

    tagType = intfVlan->tagType;

    xpsInterfaceInfo_t *intfInfo = NULL;
    result = xpsInterfaceGetInfoScope(scopeId, intfId, &intfInfo);
    if (result != XP_NO_ERR || intfInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get intf id %d\n", intfId);
        return result;
    }
    //Get interfacetype
    intfType = intfInfo->type;

    xpsVlan_t defaultVlanId;
    result = xpsVlanGetDefault(devId, &defaultVlanId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get default vlan failed");
        return result;
    }

    switch (intfType)
    {
        case XPS_PORT:
        case XPS_EXTENDED_PORT:
            cpssDevId = xpsGlobalIdToDevId(devId, intfId);
            cpssPortNum = xpsGlobalPortToPortnum(devId, intfId);
            rc = cpssDxChBrgVlanPortDelete(cpssDevId, vlanId, cpssPortNum);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Remove Port(intfId %d) failed", intfId);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
            if (vlanCtx->isIpv4RoutingEn)
            {
                rc = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                    CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE, GT_TRUE);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to update ipv4 UC routing for port intf : %d \n", cpssPortNum);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            if (vlanCtx->isIpv6RoutingEn)
            {
                rc = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                    CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE, GT_TRUE);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to update ipv4 UC routing for port intf : %d \n", cpssPortNum);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }

            if ((vlanCtx->isIpv4McRoutingEn) || (vlanCtx->mcKeyType > MC_BRIDGE_MODE_FDB &&
                                                 vlanCtx->mcKeyType < MC_BRIDGE_MODE_MAX))
            {
                if ((rc = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId,
                                                       cpssPortNum,
                                                       CPSS_IP_PROTOCOL_IPV4_E,
                                                       GT_FALSE)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update MC Routing vlan member (%d) failed for vlan (%d) \n", intfId, vlanId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            if (vlanCtx->isIpv6McRoutingEn)
            {
                if ((rc = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId,
                                                       cpssPortNum,
                                                       CPSS_IP_PROTOCOL_IPV6_E,
                                                       GT_FALSE)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update MC Routing vlan member (%d) failed for vlan (%d) \n", intfId, vlanId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }

            if (vlanCtx->isIpv4RoutingEn || vlanCtx->isIpv6RoutingEn ||
                vlanCtx->isIpv4McRoutingEn || vlanCtx->isIpv6McRoutingEn)
            {
                if ((rc = xpsAclCommonDropRuleUpdate(devId, cpssPortNum, false)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "xpsAclCommonDropRuleUpdate port (%d) failed \n", cpssPortNum);
                    return result;
                }
            }

            result = xpsIntfChildRemove(intfInfo, vlanId);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to remove vlan %d from port %d\n", vlanId,
                      intfId);
                return result;
            }

            break;
        case XPS_LAG:
            cpssOsMemSet(&lagInfoKey, 0, sizeof(lagInfoKey));
            lagInfoKey.lagIntf = intfId;
            // Retrieve the lag information for this id if it exists
            if ((result = xpsStateSearchData(scopeId, lagDbHandle, (xpsDbKey_t)&lagInfoKey,
                                             (void**)&lagInfo)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Search for Lag interface failed, lag interface(%d)", intfId);
                return result;
            }
            if (!lagInfo)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Lag info not found");
                result = XP_ERR_NOT_FOUND;
                return result;
            }
            for (uint32_t i = 0; i < XPS_LAG_MAX_PORTS; i++)
            {
                if ((lagInfo->portIntf[i] != XPS_INTF_INVALID_ID))
                {
                    cpssDevId = xpsGlobalIdToDevId(devId, lagInfo->portIntf[i]);
                    cpssPortNum = xpsGlobalPortToPortnum(devId, lagInfo->portIntf[i]);

                    if ((rc =  cpssDxChBrgVlanPortDelete(cpssDevId, vlanId, cpssPortNum)) != GT_OK)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "cpss vlanmemberdelete failed for vlan %d", vlanId);
                        return xpsConvertCpssStatusToXPStatus(rc);
                    }

                    /*Update port's default VLAN for untagged ports*/
                    if (tagType == XP_L2_ENCAP_DOT1Q_UNTAGGED)
                    {
                        result = xpsVlanUpdatePvid(devId, lagInfo->portIntf[i], defaultVlanId);
                        if (result != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Pvid updation failed");
                            return result;
                        }
                    }
                    if (vlanCtx->isIpv4RoutingEn)
                    {
                        rc = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                            CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE, GT_TRUE);
                        if (rc != GT_OK)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "Failed to update ipv4 UC routing for port intf : %d \n", cpssPortNum);
                            return xpsConvertCpssStatusToXPStatus(rc);
                        }

                    }
                    if (vlanCtx->isIpv6RoutingEn)
                    {
                        rc = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                            CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE, GT_TRUE);
                        if (rc != GT_OK)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "Failed to update ipv4 UC routing for port intf : %d \n", cpssPortNum);
                            return xpsConvertCpssStatusToXPStatus(rc);
                        }
                    }
                    if ((vlanCtx->isIpv4McRoutingEn) || (vlanCtx->mcKeyType > MC_BRIDGE_MODE_FDB &&
                                                         vlanCtx->mcKeyType < MC_BRIDGE_MODE_MAX))
                    {
                        if ((rc = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId,
                                                               cpssPortNum,
                                                               CPSS_IP_PROTOCOL_IPV4_E,
                                                               GT_FALSE)) != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "Update MC Routing vlan member (%d) failed for vlan (%d) \n",
                                  intfId, vlanId);
                            return xpsConvertCpssStatusToXPStatus(rc);
                        }
                    }
                    if (vlanCtx->isIpv6McRoutingEn)
                    {
                        if ((rc = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId,
                                                               cpssPortNum,
                                                               CPSS_IP_PROTOCOL_IPV6_E,
                                                               GT_FALSE)) != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "Update MC Routing vlan member (%d) failed for vlan (%d) \n",
                                  intfId, vlanId);
                            return xpsConvertCpssStatusToXPStatus(rc);
                        }
                    }

                    if (vlanCtx->isIpv4RoutingEn || vlanCtx->isIpv6RoutingEn ||
                        vlanCtx->isIpv4McRoutingEn || vlanCtx->isIpv6McRoutingEn)
                    {
                        if ((result = xpsAclCommonDropRuleUpdate(cpssDevId, cpssPortNum,
                                                                 false)) != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "xpsAclCommonDropRuleUpdate port (%d) failed \n", lagInfo->portIntf[i]);
                            return result;
                        }
                    }
                }
            }
            break;
        case XPS_TUNNEL_MPLS:
        case XPS_TUNNEL_VXLAN:
        case XPS_TUNNEL_NVGRE:
        case XPS_TUNNEL_GENEVE:
        case XPS_TUNNEL_PBB:
            /* if ((result = xpL2DomainMgr::instance()->removeTunnel(devId,(xpTunnel_t)intfId, vlanCtx->l2DomainContext,(xpL2EncapType_e)tagType, l2EncapData)) != XP_NO_ERR)
               {
               LOGFN(xpLogModXps,XP_SUBMOD_MAIN,XP_LOG_ERROR,"Remove Tunnel(intfId %d) failed", intfId);
               return result;
               }*/
            break;
        default:
            break;
    }

    if (intfType == XPS_PORT || intfType == XPS_LAG ||
        intfType == XPS_EXTENDED_PORT)
    {
        /* Update port's default VLAN for untagged ports */
        if (tagType == XP_L2_ENCAP_DOT1Q_UNTAGGED)
        {
            xpsVlan_t defaultVlanId;
            result = xpsVlanGetDefault(devId, &defaultVlanId);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get default vlan failed");
                return result;
            }

            result = xpsVlanUpdatePvid(devId, intfId, defaultVlanId);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Pvid updation failed");
                return result;
            }
        }
    }


    if (xpsDeviceMapIsEmpty(&intfVlan->deviceMap))
    {
        result = xpsVlanRemoveIntfDb(scopeId, vlanId, intfId, &vlanCtx);

        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Remove interface failed, vlanId(%d) interface(%d)", vlanId, intfId);
            return result;
        }

        // Removing IntfVlan entry can use either (bridgeId & bridgePortId) or (real intfId and vlanId).
        result = xpsVlanRemoveIntfEntryDb(scopeId, vlanId, intfId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Remove interface entry failed, vlanId(%d) interface(%d)", vlanId, intfId);
            xpsStateHeapFree((void *)vlanCtx);
            return result;
        }

    }
    if (intfType == XPS_LAG)
    {
        result = xpsVlanRemoveVlanFromLagDb(scopeId, intfId, vlanId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Remove vlan from lag failed, vlanId(%d) interface(%d)", vlanId, intfId);
            return result;
        }
    }

    if ((vlanCtx->mappedVni) && (intfType == XPS_PORT || intfType == XPS_LAG ||
                                 intfType == XPS_TUNNEL_VXLAN))
    {
        result = xpsVxlanRemoveMemberToVniDbEntry(devId, vlanCtx->mappedVni,
                                                  vlanId, intfId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " Vlan to Vni failed, "
                  "vlanId(%d) vniId(%d)", vlanId, vlanCtx->mappedVni);
            return result;
        }
    }
    else if (vlanCtx->isIpv4RoutingEn || vlanCtx->isIpv6RoutingEn) /*NNI SVI*/
    {
        xpsInterfaceId_t    l3IntfId;
        xpsInterfaceInfo_t *l3IntfInfo = NULL;

        l3IntfId = XPS_INTF_MAP_BD_TO_INTF(vlanId);
        result = xpsInterfaceGetInfoScope(devId, l3IntfId, &l3IntfInfo);
        if ((result != XP_NO_ERR) || (l3IntfInfo == NULL))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get interface information for L3 Interface id : %d\n", l3IntfId);
            return result;
        }

        result = xpsVxlanUpdateNNIPortOnSVI(devId, l3IntfInfo, intfId, false);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsVxlanUpdateNNIPortOnSVI Failed : %d \n", intfId);
            return result;
        }
    }
    XPS_FUNC_EXIT_LOG();

    return result;
}

XP_STATUS xpsVlanRemoveInterface(xpsDevice_t devId, xpsVlan_t vlanId,
                                 xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    result = xpsVlanRemoveEndpoint(devId, vlanId, intfId);

    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsVlanGetBridgePortFromInterface(xpsScope_t scopeId,
                                            xpsVlan_t vlanId, xpsInterfaceId_t intfId, xpsInterfaceId_t *bridgePort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanAddPortToLag(xpsDevice_t devId, xpsInterfaceId_t lagIntfId,
                              xpsInterfaceId_t portIntfId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS result = XP_NO_ERR;
    xpsVlanContextDbEntry *vlanCtx = NULL;
    xpL2EncapType_e tagType;
    xpL2Encapdata_t l2EncapData;
    uint16_t i, port;
    xpsScope_t scopeId;
    xpsInterfaceType_e intfType;
    xpsLagBridgePortListDbEntry *lagBridgePortIntfList = NULL, lagPortIntfList;
    uint32_t cpssDevId;
    uint32_t cpssPortNum ;
    GT_STATUS rc = GT_OK;
    xpsInterfaceId_t    l3IntfId;
    xpsInterfaceInfo_t *l3IntfInfo = NULL;

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    /*Assumption: portIntfId is a valid portVif, error checking should have happened in caller*/
#if 0
    if ((result = xpsVlanGetLagBridgePortListDb(scopeId, lagIntfId,
                                                &lagBridgePortIntfList)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get lagIntfList for lag interface(%d) failed", lagIntfId);
        return result;
    }
#endif
    if (lagBridgePortIntfList == NULL)
    {
        memset(&lagPortIntfList, 0, sizeof(xpsLagBridgePortListDbEntry));
        lagPortIntfList.lagIntfId = lagIntfId;
        lagPortIntfList.numOfBridgePorts = 1;
        lagPortIntfList.bridgePort[0] = lagIntfId;
        lagBridgePortIntfList = &lagPortIntfList;
    }

    cpssDevId = xpsGlobalIdToDevId(devId, portIntfId);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portIntfId);

    for (port = 0; port < lagBridgePortIntfList->numOfBridgePorts; port++)
    {
        lagIntfId = lagBridgePortIntfList->bridgePort[port];
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Derived lagIntfId:%d",
              lagIntfId);

        xpsLagVlanListDbEntry * lagVlanListEntry;
        result = xpsVlanGetLagVlanListDb(scopeId, lagIntfId, &lagVlanListEntry);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get Lag Vlan List for lag interface(%d) failed", lagIntfId);
            return result;
        }

        if (!lagVlanListEntry)
        {
            result = XP_NO_ERR;
            return result;
        }

        for (i = 0; i < lagVlanListEntry->numOfVlans; i++)
        {
            CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT cpssTagType =
                CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
            GT_BOOL tag = GT_FALSE;

            result = xpsVlanGetVlanCtxDb(scopeId, lagVlanListEntry->vlans[i], &vlanCtx);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get Vlan Context Db failed");
                return result;
            }

            if ((result = xpsInterfaceGetTypeScope(scopeId, lagIntfId,
                                                   &intfType)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get interfaceType for interface(%d) failed:%d", lagIntfId, result);
                return result;
            }

            if (intfType == XPS_BRIDGE_PORT)
            {
                l2EncapData.vlanData.vlanId = XPS_INTF_MAP_BRIDGE_PORT_TO_VLAN(lagIntfId);
                lagIntfId = XPS_INTF_MAP_BRIDGE_PORT_TO_INTF(lagIntfId);
            }
            else
            {
                l2EncapData.vlanData.vlanId = lagVlanListEntry->vlans[i];
            }

            result = xpsVlanGetIntfTagTypeScope(scopeId, l2EncapData.vlanData.vlanId,
                                                lagIntfId, &tagType);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get interface tag type failed");
                return result;
            }

            if (tagType == XP_L2_ENCAP_DOT1Q_TAGGED)
            {
                cpssTagType = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
                tag = GT_TRUE;
            }

            if ((rc = cpssDxChBrgVlanMemberAdd(cpssDevId, lagVlanListEntry->vlans[i],
                                               cpssPortNum, tag, cpssTagType)) != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpss vlanmemberadd failed for vlan %d", lagVlanListEntry->vlans[i]);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
            /*update pvid of member port, if the lag is untagged interface of the vlan*/
            if (lagVlanListEntry->pvid == lagVlanListEntry->vlans[i])
            {
                result = xpsVlanUpdatePvid(devId, portIntfId, lagVlanListEntry->vlans[i]);
                if (result != XP_NO_ERR)
                {
                    return result;
                }
            }

            rc = cpssDxChBrgFdbNaToCpuPerPortSet(cpssDevId, cpssPortNum, GT_TRUE);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "failed to enable forwarding of new MAC to CPUfor packets recieved on port %d\n",
                      cpssPortNum);
                return xpsConvertCpssStatusToXPStatus(rc);
            }

            /* L3 Part. Check if routing enabled on this Vlan and update IP Config on
               lag member.*/

            if (vlanCtx->isIpv4RoutingEn)
            {
                rc = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                    CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE, GT_TRUE);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to update uc routing en to port : %d \n", portIntfId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            if (vlanCtx->isIpv6RoutingEn)
            {
                rc = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                    CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE, GT_TRUE);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to update uc routing en to port : %d \n", portIntfId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }

            /* Update Egress MAC only when routing enabled on Vlan*/
            if (vlanCtx->isIpv4RoutingEn || vlanCtx->isIpv6RoutingEn)
            {
                l3IntfId = XPS_INTF_MAP_BD_TO_INTF(lagVlanListEntry->vlans[i]);
                result = xpsInterfaceGetInfoScope(devId, l3IntfId, &l3IntfInfo);
                if ((result != XP_NO_ERR) || (l3IntfInfo == NULL))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to get interface information for L3 Interface id : %d\n", l3IntfId);
                    return result;
                }

                rc = cpssHalL3SetEgressRouterMac(cpssDevId, GT_FALSE, cpssPortNum,
                                                 l3IntfInfo->egressSAMac);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to set egress mac to port intf : %d \n", portIntfId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            if ((vlanCtx->isIpv4McRoutingEn) || (vlanCtx->mcKeyType > MC_BRIDGE_MODE_FDB &&
                                                 vlanCtx->mcKeyType < MC_BRIDGE_MODE_MAX))
            {
                if ((rc = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId,
                                                       cpssPortNum,
                                                       CPSS_IP_PROTOCOL_IPV4_E,
                                                       GT_TRUE)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update MC Routing failed for port (%d) \n", portIntfId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            if (vlanCtx->isIpv6McRoutingEn)
            {
                if ((rc = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId,
                                                       cpssPortNum,
                                                       CPSS_IP_PROTOCOL_IPV6_E,
                                                       GT_TRUE)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update MC V6 Routing failed for port (%d) \n", portIntfId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            /*If Vlan is mapped to VNI. Add this port to eVlan (UNI)*/
            if (vlanCtx->mappedVni)
            {
                xpsVniDbEntry_t *vniCtx = NULL;
                result = xpsVniGetDbEntry(scopeId, vlanCtx->mappedVni, &vniCtx);
                if (result != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Get VNI Context Db failed, vni(%d)", vlanCtx->mappedVni);
                    return result;
                }

                rc = cpssHalVlanMemberAdd(cpssDevId, vniCtx->hwEVlanId,
                                          cpssPortNum, GT_FALSE,
                                          CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "eVlan member add failed (%d)", vniCtx->hwEVlanId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
                rc = cpssHalTtiPortLookupEnableSet(cpssDevId, cpssPortNum,
                                                   CPSS_DXCH_TTI_KEY_ETH_E, GT_TRUE);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssHalTtiPortLookupEnableSet failed err: %d \n ", rc);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }

                rc = cpssHalVlanPortTranslationEnableSet(cpssDevId, cpssPortNum,
                                                         CPSS_DIRECTION_EGRESS_E,
                                                         CPSS_DXCH_BRG_VLAN_TRANSLATION_VID0_E);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssHalVlanPortTranslationEnableSet failed port (%d)err: %d \n ", cpssPortNum,
                          rc);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            else if ((vlanCtx->isIpv4RoutingEn || vlanCtx->isIpv6RoutingEn) && l3IntfInfo)
            {
                result = xpsVxlanUpdateNNIPortOnLagRIf(devId, l3IntfInfo, portIntfId, true);
                if (result != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update Lag member(%d) Failed", portIntfId);
                    return result;
                }
            }
        }
    }

    XPS_FUNC_EXIT_LOG();
    return result;
}


XP_STATUS xpsVlanRemovePortFromLag(xpsDevice_t devId,
                                   xpsInterfaceId_t lagIntfId, xpsInterfaceId_t portIntfId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS result = XP_NO_ERR;
    xpsVlanContextDbEntry *vlanCtx = NULL;
    xpL2EncapType_e tagType;
    xpL2Encapdata_t l2EncapData;
    uint16_t i, port;
    xpsScope_t scopeId;
    xpsInterfaceType_e intfType;
    xpsLagBridgePortListDbEntry *lagBridgePortIntfList = NULL, lagPortIntfList;
    uint32_t cpssDevId;
    uint32_t cpssPortNum ;
    GT_STATUS rc = GT_OK;
    xpsInterfaceId_t    l3IntfId;
    xpsInterfaceInfo_t *l3IntfInfo = NULL;

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

#if 0
    if ((result = xpsVlanGetLagBridgePortListDb(scopeId, lagIntfId,
                                                &lagBridgePortIntfList)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get lagIntfList for lag interface(%d) failed", lagIntfId);
        return result;
    }
#endif

    if (lagBridgePortIntfList == NULL)
    {
        memset(&lagPortIntfList, 0, sizeof(xpsLagBridgePortListDbEntry));
        lagPortIntfList.lagIntfId = lagIntfId;
        lagPortIntfList.numOfBridgePorts = 1;
        lagPortIntfList.bridgePort[0] = lagIntfId;
        lagBridgePortIntfList = &lagPortIntfList;
    }

    for (port = 0; port < lagBridgePortIntfList->numOfBridgePorts; port++)
    {
        lagIntfId = lagBridgePortIntfList->bridgePort[port];
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Derived lagIntfId:%d",
              lagIntfId);

        xpsLagVlanListDbEntry * lagVlanListEntry;
        result = xpsVlanGetLagVlanListDb(scopeId, lagIntfId, &lagVlanListEntry);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get Lag Vlan List for lag interface(%d) failed", lagIntfId);
            return result;
        }

        if (!lagVlanListEntry)
        {
            result = XP_NO_ERR;
            return result;
        }

        //Remove the port from lag from all the vlans the lag is part of
        for (i = 0; i < lagVlanListEntry->numOfVlans; i++)
        {
            result = xpsVlanGetVlanCtxDb(scopeId, lagVlanListEntry->vlans[i], &vlanCtx);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get Vlan Context Db failed");
                return result;
            }

            /*check if LAG is untagged member of this vlan, if so we need to reset pvid to default on removed port*/
            if (lagVlanListEntry->pvid == lagVlanListEntry->vlans[i])
            {
                xpsVlan_t defaultVlanId;

                if ((result = xpsVlanGetDefault(devId, &defaultVlanId)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get defual VlanId failed");
                    return result;
                }

                result = xpsVlanUpdatePvid(devId, portIntfId, defaultVlanId);
                if (result != XP_NO_ERR)
                {
                    return result;
                }
            }

            if ((result = xpsInterfaceGetTypeScope(scopeId, lagIntfId,
                                                   &intfType)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get interfaceType for interface(%d) failed:%d", lagIntfId, result);
                return result;
            }

            if (intfType == XPS_BRIDGE_PORT)
            {
                l2EncapData.vlanData.vlanId = XPS_INTF_MAP_BRIDGE_PORT_TO_VLAN(lagIntfId);
                lagIntfId = XPS_INTF_MAP_BRIDGE_PORT_TO_INTF(lagIntfId);
            }
            else
            {
                l2EncapData.vlanData.vlanId = lagVlanListEntry->vlans[i];
            }

            result = xpsVlanGetIntfTagTypeScope(scopeId, l2EncapData.vlanData.vlanId,
                                                lagIntfId, &tagType);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get interface tag type failed");
                return result;
            }
            cpssDevId = xpsGlobalIdToDevId(devId, portIntfId);
            cpssPortNum = xpsGlobalPortToPortnum(devId, portIntfId);



            /* L3 Part. Check if routing enabled on this Vlan and update IP Config on
               lag member.*/
            if (vlanCtx->isIpv4RoutingEn)
            {
                rc = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                    CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE, GT_TRUE);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to update uc routing en to port : %d \n", portIntfId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            if (vlanCtx->isIpv6RoutingEn)
            {
                rc = cpssHalL3UpdtIntfIpUcRoutingEn(cpssDevId, cpssPortNum,
                                                    CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE, GT_TRUE);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to update uc routing en to port : %d \n", portIntfId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }

            /* Update Egress MAC only when routing enabled on Vlan*/
            if (vlanCtx->isIpv4RoutingEn || vlanCtx->isIpv6RoutingEn)
            {
                l3IntfId = XPS_INTF_MAP_BD_TO_INTF(lagVlanListEntry->vlans[i]);
                result = xpsInterfaceGetInfoScope(devId, l3IntfId, &l3IntfInfo);
                if ((result != XP_NO_ERR) || (l3IntfInfo == NULL))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to get interface information for L3 Interface id : %d\n", l3IntfId);
                    return result;
                }

                rc = cpssHalL3RemoveEgressRouterMac(cpssDevId, GT_FALSE, cpssPortNum);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to set egress mac to port intf : %d \n", portIntfId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            if ((vlanCtx->isIpv4McRoutingEn) || (vlanCtx->mcKeyType > MC_BRIDGE_MODE_FDB &&
                                                 vlanCtx->mcKeyType < MC_BRIDGE_MODE_MAX))
            {
                if ((rc = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId,
                                                       cpssPortNum,
                                                       CPSS_IP_PROTOCOL_IPV4_E,
                                                       GT_FALSE)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update MC Routing failed for port (%d) \n", portIntfId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            if (vlanCtx->isIpv6McRoutingEn)
            {
                if ((rc = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId,
                                                       cpssPortNum,
                                                       CPSS_IP_PROTOCOL_IPV6_E,
                                                       GT_FALSE)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update MC V6 Routing failed for port (%d) \n", portIntfId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }

            if ((rc =  cpssDxChBrgVlanPortDelete(cpssDevId, lagVlanListEntry->vlans[i],
                                                 cpssPortNum)) != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpss vlanmemberdelete failed for vlan %d", lagVlanListEntry->vlans[i]);
                return xpsConvertCpssStatusToXPStatus(rc);
            }

            /*If Vlan is mapped to VNI. Add this port to eVlan*/
            if (vlanCtx->mappedVni)
            {
                xpsVniDbEntry_t *vniCtx = NULL;
                result = xpsVniGetDbEntry(scopeId, vlanCtx->mappedVni, &vniCtx);
                if (result != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Get VNI Context Db failed, vni(%d)", vlanCtx->mappedVni);
                    return result;
                }

                rc = cpssHalBrgVlanPortDelete(cpssDevId, vniCtx->hwEVlanId,
                                              cpssPortNum);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "eVlan member add failed (%d)", vniCtx->hwEVlanId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }

                rc = cpssHalTtiPortLookupEnableSet(cpssDevId, cpssPortNum,
                                                   CPSS_DXCH_TTI_KEY_ETH_E, GT_FALSE);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssHalTtiPortLookupEnableSet failed err: %d \n ", rc);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
                rc = cpssHalVlanPortTranslationEnableSet(cpssDevId, cpssPortNum,
                                                         CPSS_DIRECTION_EGRESS_E,
                                                         CPSS_DXCH_BRG_VLAN_TRANSLATION_DISABLE_E);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssHalVlanPortTranslationEnableSet failed port (%d)err: %d \n ",
                          cpssPortNum, rc);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            else if ((vlanCtx->isIpv4RoutingEn || vlanCtx->isIpv6RoutingEn) && l3IntfInfo)
            {
                result = xpsVxlanUpdateNNIPortOnLagRIf(devId, l3IntfInfo, portIntfId, false);
                if (result != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update Lag member(%d) Failed", portIntfId);
                    return result;
                }
            }
        }
    }

    XPS_FUNC_EXIT_LOG();
    return result;
}


XP_STATUS xpsVlanGetInterfaceList(xpsDevice_t devId, xpsVlan_t vlanId,
                                  xpsInterfaceId_t **intfList, uint16_t *numOfIntfs)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS result = XP_NO_ERR;
    xpsVlanContextDbEntry * vlanCtx = NULL;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_ARG;
    }

    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }

    *intfList = vlanCtx->intfList;
    *numOfIntfs = vlanCtx->numOfIntfs;


    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsVlanSetIpv4McBridgeMode(xpsDevice_t devId, xpsVlan_t vlanId,
                                     xpsVlanBridgeMcMode_e bridgeMode)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_LOCK(xpsVlanSetIpv4McBridgeMode);

    XP_STATUS result = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsVlanContextDbEntry * vlanCtx = NULL;
    xpsScope_t scopeId;
    GT_BOOL enable = GT_FALSE;

    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    GT_U32 port;
    int maxTotalPorts = 0;
    CPSS_PORTS_BMP_STC                   globalPortMembers;
    CPSS_PORTS_BMP_STC                   portsMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_BOOL                              isValid = GT_FALSE;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }

    if (bridgeMode > MC_BRIDGE_MODE_FDB &&
        bridgeMode < MC_BRIDGE_MODE_MAX)
    {
        enable = GT_TRUE;
        rc = cpssHalBrgVlanIpmBridgingModeSet(devId, vlanId, CPSS_IP_PROTOCOL_IPV4_E,
                                              CPSS_BRG_IPM_SGV_E);
        if (rc !=GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "IPM Bridging Mode failed");
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }

    /*
    NOTE: We use Routing Engine instead of Bridge engine for L2 MC.
    Make sure to disable IPMC in Bridge engine.
    */

    rc = cpssHalBrgVlanIpmBridgingEnable(devId, vlanId, CPSS_IP_PROTOCOL_IPV4_E,
                                         (enable)?GT_FALSE:GT_TRUE);
    if (rc !=GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "IPM Bridging failed");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalBrgVlanIpMcRouteEnable(devId, vlanId, CPSS_IP_PROTOCOL_IPV4_E,
                                       enable);
    if (rc !=GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "IPMC Route failed");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    cpssHalGetMaxGlobalPorts(&maxTotalPorts);

    memset(&globalPortMembers, 0, sizeof(globalPortMembers));
    rc = cpssHalReadBrgVlanEntry(devId, (GT_U16)vlanId, &portsMembers,
                                 &portsTagging, &vlanInfo, &isValid,
                                 &portsTaggingCmd, &globalPortMembers);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Read vlan %d failed .",
              vlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Enabling UC routing on all member ports of the vlan*/
    XPS_GLOBAL_PORT_ITER(port, maxTotalPorts)
    {
        cpssDevId = xpsGlobalIdToDevId(devId, port);
        cpssPortNum = xpsGlobalPortToPortnum(devId, port);
        if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&globalPortMembers, cpssPortNum))
        {
            if ((rc = cpssHalUpdtIntfIpMcRoutingEn(cpssDevId,
                                                   cpssPortNum,
                                                   CPSS_IP_PROTOCOL_IPV4_E,
                                                   enable)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Update MC Routing failed for port (%d) \n", cpssPortNum);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }
    }

    vlanCtx->mcKeyType = bridgeMode;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetIpv6McBridgeMode(xpsDevice_t devId, xpsVlan_t vlanId,
                                     xpsVlanBridgeMcMode_e bridgeMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetUnregMcCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                               xpsPktCmd_e unRegMcCmd)
{
    GT_STATUS status = GT_OK;
    CPSS_PACKET_CMD_ENT command;
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVlanSetUnregMcCmd);

    XP_STATUS result = XP_NO_ERR;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    command = xpsConvertPktCmdToCpssPktCmd(unRegMcCmd);
    //program Unregistered IPv4 multicast Cmd
    status = cpssHalSetBrgVlanUnkUnregFilter(devId, vlanId,
                                             CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_MCAST_E,
                                             command);
    if (status != GT_OK)
    {

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Set Unregistered IPv4  multicast Cmd failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    //program Unregistered IPv6 multicast Cmd
    status = cpssHalSetBrgVlanUnkUnregFilter(devId, vlanId,
                                             CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV6_MCAST_E,
                                             command);
    if (status != GT_OK)
    {

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Set Unregistered IPv6 multicast Cmd failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    //program Unregistered non IP multicast Cmd
    status = cpssHalSetBrgVlanUnkUnregFilter(devId, vlanId,
                                             CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IP_MCAST_E,
                                             command);
    if (status != GT_OK)
    {

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Set Unregistered non IP multicast Cmd failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetIpv4McBridgeMode(xpsDevice_t devId, xpsVlan_t vlanId,
                                     xpsVlanBridgeMcMode_e *bridgeMode)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_LOCK(xpsVlanGetIpv4McBridgeMode);

    XP_STATUS result = XP_NO_ERR;
    xpsVlanContextDbEntry * vlanCtx = NULL;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }

    *bridgeMode = vlanCtx->mcKeyType;
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetIpv6McBridgeMode(xpsDevice_t devId, xpsVlan_t vlanId,
                                     xpsVlanBridgeMcMode_e *bridgeMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetUnregMcCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                               xpsPktCmd_e *unRegMcCmd)
{
    GT_STATUS status = GT_OK;
    CPSS_DXCH_BRG_VLAN_INFO_STC vlanInfo;
    GT_BOOL isValidPtr;
    CPSS_PORTS_BMP_STC portsMembersPtr;
    CPSS_PORTS_BMP_STC portsTaggingPtr;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmdPtr;
    xpsPktCmd_e cmd;

    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsVlanGetUnregMcCmd);

    XP_STATUS result = XP_NO_ERR;
    xpsScope_t scopeId;

    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsMembersPtr, 0, sizeof(portsMembersPtr));
    cpssOsMemSet(&portsTaggingPtr, 0, sizeof(portsTaggingPtr));
    cpssOsMemSet(&portsTaggingCmdPtr, 0, sizeof(portsTaggingCmdPtr));

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }
    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    status = cpssDxChBrgVlanEntryRead(devId, vlanId, &portsMembersPtr,
                                      &portsTaggingPtr, &vlanInfo, &isValidPtr, &portsTaggingCmdPtr);

    if (GT_OK != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Reading vlan entry failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    if (isValidPtr != GT_TRUE)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlan id");
        return XP_ERR_INVALID_VLAN_ID;
    }

    //get unregisterd multicast cmd
    cmd = xpsConvertPktCmdToXpsPktCmd(vlanInfo.unregIpv4McastCmd);
    *unRegMcCmd = cmd;

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}


XP_STATUS xpsVlanSetArpBcCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                             xpsPktCmd_e arpBcCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVlanSetArpBcCmd);

    XP_STATUS result = XP_NO_ERR;
    xpsVlanContextDbEntry * vlanCtx = NULL;
    xpsScope_t scopeId;
    GT_STATUS rc = GT_OK;
    CPSS_PACKET_CMD_ENT cmd;
    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }
    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }

    cmd = xpsConvertPktCmdToCpssPktCmd(arpBcCmd);
    rc = cpssHalSetBrgGenArpBcastToCpuCmd(devId,
                                          CPSS_DXCH_ARP_BCAST_CMD_MODE_VLAN_E, cmd);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Set arp broadcast cmd failed");
        return  xpsConvertCpssStatusToXPStatus(rc);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsVlanSetUnknownUcCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                                 xpsPktCmd_e unknownUcCmd)
{
    GT_STATUS status = GT_OK;
    CPSS_PACKET_CMD_ENT command;

    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVlanSetUnknownUcCmd);

    XP_STATUS result = XP_NO_ERR;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    //program unknown unicast cmd
    command = xpsConvertPktCmdToCpssPktCmd(unknownUcCmd);
    status = cpssHalSetBrgVlanUnkUnregFilter(devId, vlanId,
                                             CPSS_DXCH_BRG_VLAN_PACKET_UNK_UCAST_E,
                                             command);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Program unknown unicast cmd failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetBcCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                          xpsPktCmd_e bcCmd)
{
    GT_STATUS status = GT_OK;
    CPSS_PACKET_CMD_ENT command;
    XPS_FUNC_ENTRY_LOG();


    XPS_LOCK(xpsVlanSetBcCmd);

    XP_STATUS result = XP_NO_ERR;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }
    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    command = xpsConvertPktCmdToCpssPktCmd(bcCmd);
    //program IPv4 BROADCAST Cmd
    status = cpssHalSetBrgVlanUnkUnregFilter(devId, vlanId,
                                             CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_BCAST_E,
                                             command);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Program IPv4 BROADCAST Cmd failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    //program non IPv4 BROADCAST Cmd
    status = cpssHalSetBrgVlanUnkUnregFilter(devId, vlanId,
                                             CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IPV4_BCAST_E,
                                             command);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Program non IPv4 BROADCAST Cmd failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }


    XPS_FUNC_EXIT_LOG();

    return result;
}

XP_STATUS xpsVlanSetUnknownSaCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                                 xpsPktCmd_e saMissCmd)
{
    GT_STATUS status = GT_OK;
    CPSS_PACKET_CMD_ENT command;
    XPS_FUNC_ENTRY_LOG();


    XPS_LOCK(xpsVlanSetUnknownSaCmd);

    XP_STATUS result = XP_NO_ERR;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }
    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    command = xpsConvertPktCmdToCpssPktCmd(saMissCmd);
    /*Set unknown SA cmd to XP_PKTCMD_FWD_MIRROR so that it doesn't drop the packet. By default it drops for packet with unknown SA*/
    status = cpssHalSetBrgVlanUnknownMacSaCommand(devId, vlanId, command);
    if (GT_OK != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set unknown SA cmd failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return result;
}


XP_STATUS xpsVlanSetMirrorToAnalyzer(xpsDevice_t devId, xpsVlan_t vlanId,
                                     uint32_t enable, uint32_t analyzerId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*API to bulk set config for vlan*/
XP_STATUS xpsVlanSetConfig(xpsDevice_t devId, xpsVlan_t vlanId,
                           xpsVlanConfig_t *vlanConfig)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetArpBcCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                             xpsPktCmd_e *arpBcCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsVlanGetUnknownUcCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                                 xpsPktCmd_e *unknownUcCmd)
{
    GT_STATUS status = GT_OK;
    CPSS_DXCH_BRG_VLAN_INFO_STC vlanInfo;
    GT_BOOL isValidPtr;
    CPSS_PORTS_BMP_STC portsMembersPtr;
    CPSS_PORTS_BMP_STC portsTaggingPtr;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmdPtr;
    xpsPktCmd_e cmd;

    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsVlanGetUnknownUcCmd);

    XP_STATUS result = XP_NO_ERR;
    xpsScope_t scopeId;

    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsMembersPtr, 0, sizeof(portsMembersPtr));
    cpssOsMemSet(&portsTaggingPtr, 0, sizeof(portsTaggingPtr));
    cpssOsMemSet(&portsTaggingCmdPtr, 0, sizeof(portsTaggingCmdPtr));

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }
    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    status = cpssDxChBrgVlanEntryRead(devId, vlanId, &portsMembersPtr,
                                      &portsTaggingPtr, &vlanInfo, &isValidPtr, &portsTaggingCmdPtr);

    if (GT_OK != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Reading vlan entry failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    if (isValidPtr != GT_TRUE)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlan id");
        return XP_ERR_INVALID_VLAN_ID;
    }

    //get unknown unicast cmd
    cmd = xpsConvertPktCmdToXpsPktCmd(vlanInfo.unkUcastCmd);
    *unknownUcCmd = cmd;

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetBcCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                          xpsPktCmd_e *bcCmd)
{
    GT_STATUS status = GT_OK;
    CPSS_DXCH_BRG_VLAN_INFO_STC vlanInfo;
    GT_BOOL isValidPtr;
    CPSS_PORTS_BMP_STC portsMembersPtr;
    CPSS_PORTS_BMP_STC portsTaggingPtr;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmdPtr;
    xpsPktCmd_e cmd;

    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsVlanGetBcCmd);

    XP_STATUS result = XP_NO_ERR;
    xpsScope_t scopeId;

    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsMembersPtr, 0, sizeof(portsMembersPtr));
    cpssOsMemSet(&portsTaggingPtr, 0, sizeof(portsTaggingPtr));
    cpssOsMemSet(&portsTaggingCmdPtr, 0, sizeof(portsTaggingCmdPtr));

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    status = cpssDxChBrgVlanEntryRead(devId, vlanId, &portsMembersPtr,
                                      &portsTaggingPtr, &vlanInfo, &isValidPtr, &portsTaggingCmdPtr);

    if (GT_OK != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Reading vlan entry failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    if (isValidPtr != GT_TRUE)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlan id");
        return XP_ERR_INVALID_VLAN_ID;
    }

    //get broadcast cmd
    cmd = xpsConvertPktCmdToXpsPktCmd(vlanInfo.unregIpv4BcastCmd);
    *bcCmd = cmd;

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetUnknownSaCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                                 xpsPktCmd_e *saMissCmd)
{
    GT_STATUS status = GT_OK;
    CPSS_PACKET_CMD_ENT command;
    xpsPktCmd_e cmd;
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVlanGetUnknownSaCmd);

    XP_STATUS result = XP_NO_ERR;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    /* Get unknown SA command */
    status = cpssDxChBrgVlanUnknownMacSaCommandGet(devId, vlanId, &command);
    if (GT_OK != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get operation for unknown SA cmd failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    cmd = xpsConvertPktCmdToXpsPktCmd(command);
    *saMissCmd = cmd;
    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsVlanGetMirrorAnalyzerId(xpsDevice_t devId, xpsVlan_t vlanId,
                                     uint32_t *enable, uint32_t *analyzerId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


//get scope from Device ID
/*API to get bulk config of vlan*/
XP_STATUS xpsVlanGetConfig(xpsDevice_t devId, xpsVlan_t vlanId,
                           xpsVlanConfig_t *vlanConfig)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

//per-port and per-vlan configuration
XP_STATUS xpsVlanSetIntfCountMode(xpsDevice_t devId, xpsVlan_t vlanId,
                                  xpsInterfaceId_t intfId, xpsCountMode_e countMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetIntfCountMode(xpsDevice_t devId, xpsVlan_t vlanId,
                                  xpsInterfaceId_t intfId, xpsCountMode_e *countMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

//hairpinning
XP_STATUS xpsVlanSetHairpin(xpsDevice_t devId, xpsVlan_t vlanId,
                            xpsInterfaceId_t intfId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*##################Service Instance programming######################*/



XP_STATUS xpsVlanServiceInstanceCreate(xpsDevice_t devId, uint32_t serviceId,
                                       xpsServiceInstanceType_e serviceType, xpsServiceIdData_t *data)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanServiceInstanceSetConfig(xpsDevice_t devId, uint32_t serviceId,
                                          xpsServiceInstanceType_e serviceType, xpsServiceIdData_t *data)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsVlanServiceInstanceRemove(xpsDevice_t devId, uint32_t serviceId,
                                       xpsServiceInstanceType_e serviceType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanServiceInstanceGetConfig(xpsDevice_t devId, uint32_t serviceId,
                                          xpsServiceInstanceType_e serviceType, xpsServiceIdData_t *data)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*##################SPANNING TREE##########################*/

XP_STATUS xpsVlanBindStp(xpsDevice_t devId, xpsVlan_t vlanId, xpsStp_t stpId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVlanBindStp);

    XP_STATUS result = XP_NO_ERR;
    uint32_t pVlanIvifIdx, i = 0;
    xpsStpState_e stpState;
    xpsVlanContextDbEntry * vlanCtx = NULL;
    xpsInterfaceId_t intfId;
    xpsScope_t scopeId;
    GT_STATUS status;
    GT_U16   stpIdPtr;
    xpsStp_t oldStpId;

    if (stpMode_g != XPS_STP_MODE_VLAN_E)
    {
        return XP_NO_ERR;
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }

    //Stp should be disabled for isolated vlan.
    if (vlanCtx->vlanType == VLAN_ISOLATED)
    {
        return result;
    }

    result = xpsStpIsExist(scopeId, stpId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Stp does not exist, stpId(%d)", stpId);
        return result;
    }

    status = cpssDxChBrgVlanStpIdGet(devId, vlanId, &stpIdPtr);
    if (status != GT_OK)
    {
        result = xpsConvertCpssStatusToXPStatus(status);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan get Stp Id failed");
            return result;
        }
    }
    oldStpId = (xpsStp_t) stpIdPtr;

    //Return smoothly if same stpId is passed
    if (oldStpId == stpId)
    {
        return result;
    }

    //Cleanup xps state if vlan is already part of another stp. In most cases the state should be present.
    //If the state is not present then api is called for same stp+vlan with a different device. Proceed normally then.
    result = xpsStgRemoveVlanDb(scopeId, oldStpId, vlanId);
    if ((result != XP_NO_ERR) && (result != XP_ERR_KEY_NOT_FOUND))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "");
        return result;
    }

    status = cpssHalBindBrgVlanToStpId(devId, vlanId, (GT_U16)stpId);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan to Stp bind failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    result = xpsStgAddVlanDb(scopeId, stpId, vlanId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Adding stg to vlan db failed");
        return result;
    }

    //Get portList of the vlan. Query stpstate and update the ingress stp states.
    //Egress stp state is from stg/egressFilter
    for (i = 0; i < vlanCtx->numOfIntfs; i++)
    {

        intfId = vlanCtx->intfList[i];
        result = xpsVlanGetIntfIndex(devId, vlanId, intfId, &pVlanIvifIdx);

        if (result == XP_ERR_KEY_NOT_FOUND)
        {
            continue;
        }

        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan get interface Index"
                  " vlanId(%d) interface(%d) failed", vlanId, intfId);
            return result;
        }

        result = xpsStgGetIntfStateDb(scopeId, stpId, intfId, &stpState);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get interface state failed,"
                  " stpId(%d) interface(%d)", stpId, intfId);
            return result;
        }

        result = xpsVlanSetIngStpState(devId, vlanId, intfId, stpState);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress stp failed,"
                  " stpId(%d) interface(%d)", stpId, intfId);
            return result;
        }

    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;

}

XP_STATUS xpsVlanUnbindStp(xpsDevice_t devId, xpsVlan_t vlanId, xpsStp_t stpId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVlanUnbindStp);

    XP_STATUS               result = XP_NO_ERR;
    xpsStp_t                defaultStg, stgId;
    uint32_t                pVlanIvifIdx, i = 0;
    xpsStpState_e           stpState;
    xpsVlanContextDbEntry * vlanCtx = NULL;
    xpsInterfaceId_t        intfId;
    xpsScope_t              scopeId;
    GT_STATUS               status;
    GT_U16                  vlanStpId;

    if (stpMode_g != XPS_STP_MODE_VLAN_E)
    {
        return XP_NO_ERR;
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }

    status = cpssDxChBrgVlanStpIdGet(devId, vlanId, &vlanStpId);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan get Stp Id failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    stgId = (xpsStp_t)vlanStpId;
    if (stpId != stgId)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vlan not bound to input STP, stpId(%d)", stpId);
        result = XP_ERR_INVALID_ARG;
        return result;
    }
    //Get default stg. All vlans created will be bound to default stg.
    result = xpsStpGetDefault(devId, &defaultStg);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting default stp failed");
        return result;
    }

    status = cpssHalBindBrgVlanToStpId(devId, vlanId, (GT_U16)defaultStg);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan to Stp bind failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    result = xpsStgAddVlanDb(scopeId, defaultStg, vlanId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Adding stg to vlan db failed");
        return result;
    }

    result = xpsStgRemoveVlanDb(scopeId, stpId, vlanId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Removing stg from vlan db failed");
        return result;
    }

    //Get portList of the vlan. Query stpstate and update the ingress stp states.
    for (i = 0; i < vlanCtx->numOfIntfs; i++)
    {

        intfId = vlanCtx->intfList[i];

        result = xpsVlanGetIntfIndex(devId, vlanId, intfId, &pVlanIvifIdx);

        if (result == XP_ERR_KEY_NOT_FOUND)
        {
            continue;
        }

        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Vlan get interface Index vlanId(%d) interface(%d) failed", vlanId, intfId);
            return result;
        }

        result = xpsStgGetIntfStateDb(scopeId, defaultStg, intfId, &stpState);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get interface state failed,"
                  " interface(%d)", intfId);
            return result;
        }

        result = xpsVlanSetIngStpState(devId, vlanId, intfId, stpState);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress stp failed,"
                  " vlanId(%d) interface(%d)", vlanId, intfId);
            return result;
        }

    }


    XPS_FUNC_EXIT_LOG();
    return result;

}

XP_STATUS xpsVlanGetStp(xpsDevice_t devId, xpsVlan_t vlanId, xpsStp_t *stpId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVlanGetStp);

    XP_STATUS  result = XP_NO_ERR;
    xpsVlanContextDbEntry * vlanCtx = NULL;
    xpsScope_t scopeId;
    GT_STATUS  status = GT_OK;
    GT_U16     vlanStpId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }

    status = cpssDxChBrgVlanStpIdGet(devId, vlanId, &vlanStpId);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan get Stp Id failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    *stpId = (xpsStp_t)vlanStpId;

    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsVlanSetIngStpState(xpsDevice_t devId, xpsVlan_t vlanId,
                                xpsInterfaceId_t intfId, xpsStpState_e stpState)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    //uint32_t pVlanIvifIdx;
    xpsIntfVlanIndexDbEntry *intfVlan = NULL;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    result = xpsVlanGetIntfIndexDb(devId, vlanId, intfId, &intfVlan);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vlan get interface index db  vlanId(%d) interface(%d) failed", vlanId, intfId);
        return result;
    }

    //If no intfVlan then should gracefully return without error.
    if (!intfVlan)
    {
        result = XP_NO_ERR;
        return result;
    }
#if 0
    pVlanIvifIdx = intfVlan->pVlanVifIdx;

    result = xpL2DomainMgr::instance()->setIngStgState(devId, pVlanIvifIdx,
                                                       stpState);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress stg failed");
        return result;
    }

#endif // if 0

    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsVlanGetIngStpState(xpsDevice_t devId, xpsVlan_t vlanId,
                                xpsInterfaceId_t intfId, xpsStpState_e *stpState)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetStpState(xpsDevice_t devId, xpsVlan_t vlanId,
                             xpsInterfaceId_t intfId, xpsStpState_e stpState)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetStpState(xpsDevice_t devId, xpsVlan_t vlanId,
                             xpsInterfaceId_t intfId, xpsStpState_e *stpState)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
/*#################PRIVATE VLAN########################*/

XP_STATUS xpsPVlanCreatePrimary(xpsDevice_t devId, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*This API destroys the private Vlan with primary vlanId as the vlanId
This API does not delete the vlan from the device. But resets the vlan's
type to a normal vlan from private vlan. User needs to delete the vlan
through the normal vlan delete API if he wishes.*/
XP_STATUS xpsPVlanDestroyPrimary(xpsDevice_t devId, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


/*This API only adds the interface in the floodList. But does not create an entry in portVlan table
Should be called only when a port of one vlan (say community) is added to the floodList of another
vlan( say primary)*/






XP_STATUS xpsPVlanCreateSecondary(xpsDevice_t devId, xpsVlan_t secVlanId,
                                  xpsVlan_t priVlanId, xpsPrivateVlanType_e vlanType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*  This API removes a secondary vlan from the private vlan.
But does not delete the vlan from the system. To delete the vlan
itself user needs call xpsVlanDestroy API.

This API Assumes symmetric programming of vlans across devices.
There is no reason why a vlan would be secondary in one device and
not secondary in another device.
*/
XP_STATUS xpsPVlanDestroySecondary(xpsDevice_t devId, xpsVlan_t secVlanId,
                                   xpsVlan_t priVlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPVlanAddInterfacePrimary(xpsDevice_t devId, xpsVlan_t priVlanId,
                                      xpsInterfaceId_t intfId, xpsL2EncapType_e tagType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPVlanRemoveInterfacePrimary(xpsDevice_t devId, xpsVlan_t priVlanId,
                                         xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}







XP_STATUS xpsPVlanAddInterfaceCommunity(xpsDevice_t devId, xpsVlan_t priVlanId,
                                        xpsVlan_t secVlanId, xpsInterfaceId_t intfId, xpsL2EncapType_e tagType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPVlanRemoveInterfaceCommunity(xpsDevice_t devId,
                                           xpsVlan_t priVlanId, xpsVlan_t secVlanId, xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPVlanAddInterfaceIsolated(xpsDevice_t devId, xpsVlan_t priVlanId,
                                       xpsVlan_t secVlanId, xpsInterfaceId_t intfId, xpsL2EncapType_e tagType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPVlanRemoveInterfaceIsolated(xpsDevice_t devId,
                                          xpsVlan_t priVlanId, xpsVlan_t secVlanId, xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPVlanGetType(xpsDevice_t devId, xpsVlan_t vlanId,
                          xpsPrivateVlanType_e *vlanType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}



// PRIVATE vlan fdb learning api.
XP_STATUS xpsPVlanAddFdb(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                         xpsPrivateVlanType_e vlanType, xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetBridgeAclEnable(xpsDevice_t devId, xpsVlan_t vlanId,
                                    uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetRouterAclEnable(xpsDevice_t devId, xpsVlan_t vlanId,
                                    uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsVlanSetBridgeAclId(xpsDevice_t devId, xpsVlan_t vlanId,
                                uint32_t aclId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetRouterAclId(xpsDevice_t devId, xpsVlan_t vlanId,
                                uint32_t aclId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetBridgeAclEnable(xpsDevice_t devId, xpsVlan_t vlanId,
                                    uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetRouterAclEnable(xpsDevice_t devId, xpsVlan_t vlanId,
                                    uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsVlanGetBridgeAclId(xpsDevice_t devId, xpsVlan_t vlanId,
                                uint32_t *aclId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetRouterAclId(xpsDevice_t devId, xpsVlan_t vlanId,
                                uint32_t *aclId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetPerVlanControlMac(xpsDevice_t devId, xpsVlan_t vlanId,
                                      macAddr_t mac)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetPerVlanControlMacWithMask(xpsDevice_t devId,
                                              xpsVlan_t vlanId, macAddr_t mac, macAddr_t macMask)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanClearPerVlanControlMac(xpsDevice_t devId, xpsVlan_t vlanId,
                                        macAddr_t mac)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanClearPerVlanControlMacWithMask(xpsDevice_t devId,
                                                xpsVlan_t vlanId, macAddr_t mac, macAddr_t macMask)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetGlobalControlMac(xpsDevice_t devId, macAddr_t mac)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetGlobalControlMacWithMask(xpsDevice_t devId, macAddr_t mac,
                                             macAddr_t mask)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanClearGlobalControlMacWithMask(xpsDevice_t devId, macAddr_t mac,
                                               macAddr_t mask)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetGlobalControlMacWithAction(xpsDevice_t devId, macAddr_t mac)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanClearGlobalControlMac(xpsDevice_t devId, macAddr_t mac)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetFirst(xpsDevice_t devId, xpsVlan_t *vlanId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;
    xpsVlanContextDbEntry *vlanCtx = NULL;
    xpsVlanContextDbEntry keyVlanCtxTmp;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    status = xpsScopeGetScopeId(devId, &scopeId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return status;
    }

    if (vlanId == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }

    memset(&keyVlanCtxTmp, 0x0, sizeof(xpsVlanContextDbEntry));

    /* Get first vlan entry from the list */
    if ((status = xpsStateGetNextData(scopeId, vlanStateDbHndl, (xpsDbKey_t)NULL,
                                      (void**)&vlanCtx)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get next vlan from list failed");
        return status;
    }

    /* iterate and pick first vlan for this device */
    while (vlanCtx != NULL)
    {
        if (!xpsDeviceMapTestDeviceBit(&vlanCtx->deviceMap, devId))
        {
            keyVlanCtxTmp.vlanId= vlanCtx->vlanId;
            /* try to get next vlan from the list */
            if ((status = xpsStateGetNextData(scopeId, vlanStateDbHndl,
                                              (xpsDbKey_t)&keyVlanCtxTmp, (void**)&vlanCtx)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get next vlan from list failed");
                return status;
            }
        }
        else
        {
            /* we have found vlan for this device */
            break;
        }
    }

    /* vlan context found after iteration ? */
    if (vlanCtx == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan does not exist");
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    *vlanId = vlanCtx->vlanId;

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsVlanGetNext(xpsDevice_t devId, xpsVlan_t vlanId,
                         xpsVlan_t *vlanIdNext)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsVlanContextDbEntry *vlanCtx = NULL;
    xpsVlanContextDbEntry keyVlanCtxTmp;
    xpsScope_t scopeId;

    memset(&keyVlanCtxTmp, 0x0, sizeof(xpsVlanContextDbEntry));
    keyVlanCtxTmp.vlanId = vlanId;


    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    status = xpsScopeGetScopeId(devId, &scopeId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return status;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    if (vlanIdNext == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }

    /* get next vlan entry from the list */
    if ((status = xpsStateGetNextData(scopeId, vlanStateDbHndl,
                                      (xpsDbKey_t)&keyVlanCtxTmp, (void**)&vlanCtx)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get next vlan from list failed");
        return status;
    }

    /* iterate and pick next vlan for this device */
    while (vlanCtx != NULL)
    {
        if (!xpsDeviceMapTestDeviceBit(&vlanCtx->deviceMap, devId))
        {
            keyVlanCtxTmp.vlanId = vlanCtx->vlanId;
            /* try to get next vlan from the list */
            if ((status = xpsStateGetNextData(scopeId, vlanStateDbHndl,
                                              (xpsDbKey_t)&keyVlanCtxTmp, (void**)&vlanCtx)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get next vlan from list failed");
                return status;
            }
        }
        else
        {
            /* we have found vlan for this device */
            break;
        }
    }

    /* vlan context found ? */
    if (vlanCtx == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT, "Vlan context is not found");
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    *vlanIdNext = vlanCtx->vlanId;


    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsVlanSetOpenFlowEnable(xpsDevice_t devId, xpsVlan_t vlanId,
                                   xpsInterfaceId_t intfId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsVlanSetRedirectToInterface(xpsDevice_t devId, xpsVlan_t vlanId,
                                        xpsInterfaceId_t intfId, uint32_t enRedirect, xpsInterfaceId_t destIntfId,
                                        xpVlanEncap_t encapType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetOpenFlowEnable(xpsDevice_t devId, xpsVlan_t vlanId,
                                   xpsInterfaceId_t intfId, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/*FDB limit feature*/
XP_STATUS xpsVlanGetFdbCount(xpsDevice_t devId, xpsVlan_t vlanId,
                             uint32_t *numFdbEntries)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsVlanIncrementFdbCount(xpsDevice_t devId, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsVlanDecrementFdbCount(xpsDevice_t devId, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanClearFdbCountByVlan(xpsDevice_t devId, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanClearFdbCountForAllVlan(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanIsFdbLimitReached(xpsDevice_t devId, xpsVlan_t vlanId,
                                   uint8_t *isFdbLimitReached)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsVlanSetFdbLimit(xpsDevice_t devId, xpsVlan_t vlanId,
                             uint32_t fdbLimit)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsVlanGetFdbLimit(xpsDevice_t devId, xpsVlan_t vlanId,
                             uint32_t *fdbLimit)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/* per-Vlan BUM policer apis */
XP_STATUS xpsVlanSetBumPolicerEn(xpsDevice_t devId, xpsVlan_t vlanId,
                                 uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetBumPolicerEn(xpsDevice_t devId, xpsVlan_t vlanId,
                                 uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetBumPolicerId(xpsDevice_t devId, xpsVlan_t vlanId,
                                 uint32_t policerId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetBumPolicerId(xpsDevice_t devId, xpsVlan_t vlanId,
                                 uint32_t *policerId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetStagTpid(xpsDevice_t devId, uint16_t *tpId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetStagTpid(xpsDevice_t devId, uint16_t tpId,
                             xpsDirection_t dir)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetCountMode(xpDevice_t devId, xpsVlan_t vlanId,
                              xpsVlanCountMode_e mode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetCountMode(xpDevice_t devId, xpsVlan_t vlanId,
                              xpsVlanCountMode_e *mode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetEgressCounterId(xpDevice_t devId, xpsVlan_t vlanId,
                                    uint32_t counterId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetEgressCounterId(xpDevice_t devId, xpsVlan_t vlanId,
                                    uint32_t *counterId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetCountingEnable(xpsDevice_t devId, xpsDirection_t dir,
                                   uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetIntfCountingEnable(xpsDevice_t devId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetBumPolicingEnable(xpsDevice_t devId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetFloodL2InterfaceList(xpsDevice_t devId, xpsVlan_t vlanId,
                                         xpsMcL2InterfaceListId_t listId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetFloodL2InterfaceList(xpsDevice_t devId, xpsVlan_t vlanId,
                                         xpsMcL2InterfaceListId_t *listId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanResetFloodL2InterfaceList(xpsDevice_t devId, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetIPv4UnregMcastL2InterfaceList(xpsDevice_t devId,
                                                  xpsVlan_t vlanId, xpsMcL2InterfaceListId_t listId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetIPv4UnregMcastL2InterfaceList(xpsDevice_t devId,
                                                  xpsVlan_t vlanId, xpsMcL2InterfaceListId_t *listId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanResetIPv4UnregMcastL2InterfaceList(xpsDevice_t devId,
                                                    xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetIPv6UnregMcastL2InterfaceList(xpsDevice_t devId,
                                                  xpsVlan_t vlanId, xpsMcL2InterfaceListId_t listId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetIPv6UnregMcastL2InterfaceList(xpsDevice_t devId,
                                                  xpsVlan_t vlanId, xpsMcL2InterfaceListId_t *listId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanResetIPv6UnregMcastL2InterfaceList(xpsDevice_t devId,
                                                    xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetRehashLevel(xpDevice_t devId, uint8_t numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetRehashLevel(xpDevice_t devId, uint8_t* numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsCountVlanObjects(xpsDevice_t xpsDevId, uint32_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS  result = XP_NO_ERR;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(xpsDevId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              xpsDevId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(xpsDevId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    result = xpsStateGetCount(scopeId, vlanStateDbHndl, count);

    return result;

    XPS_FUNC_EXIT_LOG();

}

XP_STATUS xpsCountVlanMemberObjects(xpsDevice_t xpsDevId, uint32_t *count)
{
    XP_STATUS  result = XP_NO_ERR;
    xpsScope_t scopeId;
    XPS_FUNC_ENTRY_LOG();
    //get scope from Device ID
    result = xpsScopeGetScopeId(xpsDevId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    result = xpsStateGetCount(scopeId, intfVlanTagTypeDbHndl, count);
    return result;



    if (IS_DEVICE_VALID(xpsDevId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              xpsDevId);
        return XP_ERR_INVALID_DEV_ID;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanGetVlanIdList(xpsDevice_t xpsDevId, xpsVlan_t* vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS              result                 = XP_NO_ERR;
    xpsVlanContextDbEntry *curVlanContextDbEntry  = NULL;
    xpsVlanContextDbEntry *prevVlanContextDbEntry = NULL;
    uint32_t               count                  = 0;
    xpsScope_t             scopeId;

    if (IS_DEVICE_VALID(xpsDevId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              xpsDevId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(xpsDevId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }
    result=  xpsStateGetNextData(scopeId, vlanStateDbHndl, NULL,
                                 (void **)&curVlanContextDbEntry);

    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get data , retVal : %d", result);
        return result;
    }

    while (curVlanContextDbEntry)
    {
        //Store the node information in previous and get-next
        prevVlanContextDbEntry  = curVlanContextDbEntry;
        vlanId[count]           = prevVlanContextDbEntry->vlanId;

        result =  xpsStateGetNextData(scopeId, vlanStateDbHndl,
                                      (xpsDbKey_t)&prevVlanContextDbEntry->vlanId, (void **)&curVlanContextDbEntry);

        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get data , retVal : %d", result);
            return result;
        }

        count++;
    }

    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsVlanGetVlanMemberIdList(xpsDevice_t xpsDevId,
                                     uint32_t* vlanMemberId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS                    result                 = XP_NO_ERR;
    xpsIntfVlanEncapInfoDbEntry *curVlanContextDbEntry  = NULL;
    xpsIntfVlanEncapInfoDbEntry *prevVlanContextDbEntry = NULL;
    uint32_t                     count                  = 0;
    xpsScope_t                   scopeId;

    if (IS_DEVICE_VALID(xpsDevId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              xpsDevId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(xpsDevId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }
    result=  xpsStateGetNextData(scopeId, intfVlanTagTypeDbHndl, NULL,
                                 (void **)&curVlanContextDbEntry);

    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get data , retVal : %d", result);
        return result;
    }

    while (curVlanContextDbEntry)
    {
        //Store the node information in previous and get-next
        prevVlanContextDbEntry  = curVlanContextDbEntry;
        vlanMemberId[count]     = prevVlanContextDbEntry->intfVlanKey;

        result =  xpsStateGetNextData(scopeId, intfVlanTagTypeDbHndl,
                                      (xpsDbKey_t)&prevVlanContextDbEntry->intfVlanKey,
                                      (void **)&curVlanContextDbEntry);

        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get data , retVal : %d", result);
            return result;
        }

        count++;
    }

    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsVlanSetIgmpCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                            xpsPktCmd_e igmpCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVlanSetIgmpCmd);

    XP_STATUS result = XP_NO_ERR;
    xpsScope_t scopeId;
    GT_STATUS rc = GT_OK;
    GT_BOOL enable = GT_FALSE;
    xpsVlanContextDbEntry * vlanCtx = NULL;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    result = xpsVlanGetVlanCtxDb(scopeId, vlanId, &vlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan Context Db failed, vlanId(%d)", vlanId);
        return result;
    }

    result = xpsConvertPktCmdToCpssEnable(igmpCmd, &enable);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Conversion of packet command to cpss enable failed");
        return result;
    }

    rc = cpssHalEnableBrgVlanIgmpSnooping(devId, vlanId, enable);
    if (rc !=GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Program IGMP Cmd failed");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    vlanCtx->igmpSnoopEnable = enable;

    XPS_FUNC_EXIT_LOG();
    return result;
}


XP_STATUS xpsVlanSetIcmpv6Cmd(xpsDevice_t devId, xpsVlan_t vlanId,
                              xpsPktCmd_e icmpv6Cmd)
{
    XPS_FUNC_ENTRY_LOG();
    GT_BOOL enable= GT_FALSE;
    GT_STATUS rc = GT_OK;
    XPS_LOCK(xpsVlanSetIcmpv6Cmd);

    XP_STATUS result = XP_NO_ERR;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }
    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    result = xpsConvertPktCmdToCpssEnable(icmpv6Cmd, &enable);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Conversion of packet command to cpss enable failed");
        return result;
    }

    //program ICMPv6 Cmd
    rc = cpssHalEnableBrgVlanIpV6IcmpToCpu(devId, vlanId, enable);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Program ICMPv6 Cmd failed");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsVlanGetIgmpCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                            xpsPktCmd_e *igmpCmd)
{
    XPS_FUNC_ENTRY_LOG();
    xpsPktCmd_e cmd;
    GT_BOOL enable;
    GT_STATUS rc = GT_OK;
    XPS_LOCK(xpsVlanGetIgmpCmd);

    XP_STATUS result = XP_NO_ERR;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }
    //get IGMP Cmd
    rc = cpssDxChBrgVlanIgmpSnoopingEnableGet(devId, vlanId, &enable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting IGMP Cmd failed");
        return xpsConvertCpssStatusToXPStatus(rc);
    }
    result = xpsConvertCpssEnableToPktCmd(&cmd, enable);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Conversion of cpss enable to packet command failed");
        return result;
    }
    *igmpCmd = cmd;
    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsVlanGetIcmpv6Cmd(xpsDevice_t devId, xpsVlan_t vlanId,
                              xpsPktCmd_e *icmpv6Cmd)
{
    XPS_FUNC_ENTRY_LOG();
    xpsPktCmd_e cmd;
    GT_BOOL enable;
    GT_STATUS rc = GT_OK;
    XPS_LOCK(xpsVlanGetIcmpv6Cmd);

    XP_STATUS result = XP_NO_ERR;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID(%d)",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    if (vlanId > XPS_VLANID_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlanId.");
        return XP_ERR_INVALID_VLAN_ID;
    }

    //get ICMPv6 Cmd
    rc = cpssDxChBrgVlanIpV6IcmpToCpuEnableGet((GT_U8)devId, vlanId, &enable);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting ICMPv6 Cmd failed");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    result = xpsConvertCpssEnableToPktCmd(&cmd, enable);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Conversion of cpss enable to packet command failed");
        return result;
    }
    *icmpv6Cmd = cmd;
    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsVlanHandlePortVlanTableRehash(xpsDevice_t devId,
                                           xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpscustRemoveLagFromVlan(xpsScope_t scopeId,
                                   xpsInterfaceId_t lagIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVlanSetVniInVlanDb(xpsScope_t scopeId, xpsVlan_t vlanId,
                                uint32_t vniId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;
    xpsVlanContextDbEntry *entry;

    XPS_LOCK(xpsVlanMapVniinVlanDb);

    status = xpsVlanGetVlanCtxDb(scopeId, vlanId, &entry);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    entry->mappedVni = vniId;

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsVlanGetVniInVlanDb(xpsScope_t scopeId, xpsVlan_t vlanId,
                                uint32_t *vniId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;
    xpsVlanContextDbEntry *entry;

    XPS_LOCK(xpsVlanMapVniinVlanDb);

    status = xpsVlanGetVlanCtxDb(scopeId, vlanId, &entry);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    *vniId = entry->mappedVni;

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsVlanCreateStack(xpsScope_t scopeId, xpsVlanStackStage_e xpsVlanStackStage,
                             xpsVlanStackAction_e xpsVlanStackAction, xpsInterfaceId_t intfId,
                             xpsVlanStack_t *vlanStackId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVlanCreateStack);
    XP_STATUS result;
    xpsDevice_t devId;
    GT_STATUS rc = GT_OK;
    xpsVlanStackDbEntry *vlanStackEntry;
    xpsInterfaceType_e intfType = XPS_PORT;
    xpsInterfaceInfo_t *intfInfo = NULL;
    uint32_t cpssDevId;
    uint32_t cpssPortNum;
    xpsVlan_t prevVlanId = 0;
    xpsVlan_t vlanId = 0;

    result = xpsScopeGetFirstDevice(scopeId, &devId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsScopeGetFirstDevice() Failed %d\n", result);
        return result;
    }

    result = xpsAllocatorAllocateId(scopeId, XP_ALLOCATOR_VLAN_STACK, vlanStackId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Allocate vlan stack failed");
        return result;
    }

    result = xpsVlanStackInsertDb(scopeId, *vlanStackId, &vlanStackEntry);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Insert into vlanStack list failed");
        return result;
    }

    result = xpsInterfaceGetInfoScope(scopeId, intfId, &intfInfo);
    if (result != XP_NO_ERR || intfInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get intf id %d\n", intfId);
        return result;
    }
    // Get interfacetype
    intfType = intfInfo->type;

    /*According to interface type switch to different l2domain apis*/
    switch (intfType)
    {
    case XPS_PORT:
    case XPS_EXTENDED_PORT:
        cpssDevId = xpsGlobalIdToDevId(devId, intfId);
        cpssPortNum = xpsGlobalPortToPortnum(devId, intfId);

        if (xpsVlanStackStage == XP_INGRESS_PORT_STACK)
        {
            if (xpsVlanStackAction == XP_ACTION_PUSH)
            {
                if ((rc = cpssDxChBrgNestVlanAccessPortSet(cpssDevId, cpssPortNum, GT_TRUE)) != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpss cpssDxChBrgNestVlanAccessPortSet failed for port %d", cpssPortNum);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
        }
        else
        {
            if (xpsVlanStackAction == XP_ACTION_POP)
            {
                result = xpsVlanGetFirst(devId, &vlanId);
                if ((result != XP_NO_ERR) && (result != XP_ERR_NOT_FOUND))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting first vlan failed");
                    return result;
                }
                /* Iterate all vlans and set tagging mode of the port if it is a member of this vlan*/
                while (1)
                {
                    if (xpsVlanIsIntfExist(devId, vlanId, intfId) == XP_NO_ERR)
                    {
                        if ((rc = cpssDxChBrgVlanMemberSet(cpssDevId, vlanId, cpssPortNum,
                                                           GT_TRUE, GT_TRUE, CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E)) != GT_OK)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "cpss cpssDxChBrgNestVlanAccessPortSet failed for port %d", cpssPortNum);
                            return xpsConvertCpssStatusToXPStatus(rc);
                        }
                    }
                    prevVlanId = vlanId;
                    result = xpsVlanGetNext(devId, prevVlanId, &vlanId);
                    if (result == XP_ERR_NOT_FOUND)
                    {
                        result = XP_NO_ERR;
                        break;
                    }
                }
            }
        }
        break;
    default:
        break;
    }

    vlanStackEntry->xpsIntf = intfId;
    vlanStackEntry->xpsVlanStackAction = xpsVlanStackAction;
    vlanStackEntry->xpsVlanStackStage = xpsVlanStackStage;

    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsVlanRemoveStack(xpsScope_t scopeId, xpsVlanStack_t vlanStackId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVlanRemoveStack);
    XP_STATUS result;
    xpsDevice_t devId;
    GT_STATUS rc = GT_OK;
    xpsVlanStackDbEntry *vlanStackEntry;
    xpsInterfaceType_e intfType = XPS_PORT;
    xpsInterfaceInfo_t *intfInfo = NULL;
    uint32_t cpssDevId;
    uint32_t cpssPortNum;
    xpsVlan_t prevVlanId = 0;
    xpsVlan_t vlanId = 0;

    result = xpsScopeGetFirstDevice(scopeId, &devId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsScopeGetFirstDevice() Failed %d\n", result);
        return result;
    }

    result = xpsVlanStackGetDb(scopeId, vlanStackId, &vlanStackEntry);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " xpsVlanStackGetDb, vlanStackId(%d)",
              vlanStackId);
        return result;
    }

    result = xpsInterfaceGetInfoScope(scopeId, vlanStackEntry->xpsIntf, &intfInfo);
    if (result != XP_NO_ERR || intfInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get intf id %d\n", vlanStackEntry->xpsIntf);
        return result;
    }
    // Get interfacetype
    intfType = intfInfo->type;

    /*According to interface type switch to different l2domain apis*/
    switch (intfType)
    {
    case XPS_PORT:
    case XPS_EXTENDED_PORT:
        cpssDevId = xpsGlobalIdToDevId(devId, vlanStackEntry->xpsIntf);
        cpssPortNum = xpsGlobalPortToPortnum(devId, vlanStackEntry->xpsIntf);

        if (vlanStackEntry->xpsVlanStackStage == XP_INGRESS_PORT_STACK)
        {
            if (vlanStackEntry->xpsVlanStackAction == XP_ACTION_PUSH)
            {
                if ((rc = cpssDxChBrgNestVlanAccessPortSet(cpssDevId, cpssPortNum, GT_FALSE)) != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpss cpssDxChBrgNestVlanAccessPortSet failed for port %d", cpssPortNum);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
        }
        else
        {
            if (vlanStackEntry->xpsVlanStackAction == XP_ACTION_POP)
            {
                result = xpsVlanGetFirst(devId, &vlanId);
                if ((result != XP_NO_ERR) && (result != XP_ERR_NOT_FOUND))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting first vlan failed");
                    return result;
                }
                /* Iterate all vlans and set tagging mode of the port if it is a member of this vlan*/
                while (1)
                {
                    if (xpsVlanIsIntfExist(devId, vlanId, vlanStackEntry->xpsIntf) == XP_NO_ERR)
                    {
                        if ((rc = cpssDxChBrgVlanMemberSet(cpssDevId, vlanId, cpssPortNum,
                                                           GT_TRUE, GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E)) != GT_OK)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "cpss cpssDxChBrgNestVlanAccessPortSet failed for port %d", cpssPortNum);
                            return xpsConvertCpssStatusToXPStatus(rc);
                        }
                    }
                    prevVlanId = vlanId;
                    result = xpsVlanGetNext(devId, prevVlanId, &vlanId);
                    if (result == XP_ERR_NOT_FOUND)
                    {
                        result = XP_NO_ERR;
                        break;
                    }
                }
            }
        }
        break;
    default:
        break;
    }

    // de-allocate the vlan stack
    result = xpsAllocatorReleaseId(scopeId, XP_ALLOCATOR_VLAN_STACK, vlanStackId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Release of Interface Id failed");
        return result;
    }

    result = xpsVlanStackRemoveDb(scopeId, vlanStackId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vlan Stack Remove failed, vlanStackId(%d)", vlanStackId);
        return result;
    }

    XPS_FUNC_EXIT_LOG();
    return result;
}

#ifdef __cplusplus
}
#endif

