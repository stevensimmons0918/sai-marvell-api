// xpsStp.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsStp.h"
#include "xpsInternal.h"
#include "xpsInit.h"
#include "xpsInterface.h"
#include "xpsScope.h"
#include "xpsEgressFilter.h"
#include "xpsLock.h"
#include "xpsMac.h"
#include "xpsGlobalSwitchControl.h"
#include "xpsUtil.h"
#include "xpsLag.h"
#include "xpsPort.h"
#include "cpssHalDevice.h"
//#include "xpsAppUtil.h"
#include "openXpsTypes.h"
#include "openXpsLag.h"
#include "gtEnvDep.h"
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include "xpsAllocator.h"
#include "cpssHalUtil.h"
#include "cpssHalStp.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

xpsStpMode_e stpMode_g = XPS_STP_MODE_PORT_E;

#define SYSTEM_DEFAULT_STG  0
#define SYSTEM_MAX_EGRESS_FILTER_ID 255
#define XP_STP_MAX_IDS(devId)                 (cpssHalGetSKUmaxSTPgrps(devId))
#define XP_STP_RANGE_START             0

// used for stp to vlan mapping
static xpsDbHandle_t stgCtxDbHndl = XPS_STATE_INVALID_DB_HANDLE;

// used for stp interface state
static xpsDbHandle_t stgIntfStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;

//used for stp static variables
static xpsDbHandle_t stgStaticDataDbHndl = XPS_STATE_INVALID_DB_HANDLE;

#define XPS_DEFAULT_NUM_VLANS_IN_STG 16

/* This DS is common to all STP variants*/
typedef struct xpsStgDbEntry
{
    //Key
    xpsStp_t stgId;

    xpsStpMode_e mode;

    //Data
    uint16_t numOfVlans;
    xpsVlan_t vlans[XPS_DEFAULT_NUM_VLANS_IN_STG];
} xpsStgDbEntry;



static int32_t xpsStpKeyComp(void* key1, void* key2)
{
    return ((((xpsStgDbEntry *) key1)->stgId) - (((xpsStgDbEntry *)
                                                  key2)->stgId));
}

static int32_t stpMgrIntfKeyComp(void* key1, void* key2)
{
    return ((((xpsStgIntfStateDbEntry *) key1)->stgIntfDbKey) - (((
                                                                      xpsStgIntfStateDbEntry *) key2)->stgIntfDbKey));
}

static int32_t stpMgrStaticVarKeyComp(void* key1, void* key2)
{
    return ((((xpsStgStaticDbEntry *) key1)->staticDataType) - (((
                                                                     xpsStgStaticDbEntry *) key2)->staticDataType));
}


/* State maintainence functions*/

//Static variables DB functions

XP_STATUS xpsStgGetStaticVariablesDb(xpsScope_t scopeId,
                                     xpsStgStaticDbEntry ** staticVarDbPtr)
{
    XP_STATUS result = XP_NO_ERR;
    xpsStgStaticDbEntry keyStaticVarDb;

    keyStaticVarDb.staticDataType = XPS_STG_STATIC_VARIABLES;

    if ((result = xpsStateSearchData(scopeId, stgStaticDataDbHndl,
                                     (xpsDbKey_t)&keyStaticVarDb, (void**)staticVarDbPtr)) != XP_NO_ERR)
    {
        //LOGFN()
        return result;
    }

    if (!(*staticVarDbPtr))
    {
        //LOGFN()
        return XP_ERR_KEY_NOT_FOUND;
    }

    return result;
}

static XP_STATUS xpsStgRemoveStaticVariablesDb(xpsScope_t scopeId)
{

    XP_STATUS result = XP_NO_ERR;
    xpsStgStaticDbEntry * staticVarDb = NULL;
    xpsStgStaticDbEntry keyStaticVarDb;

    result = xpsStgGetStaticVariablesDb(scopeId, &staticVarDb);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStgGetStaticVariablesDb failed");
        return result;
    }

    result =  xpsStpDestroyScope(scopeId, staticVarDb->defaultStg);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsStpDestroyScope failed");
        return result;
    }

    keyStaticVarDb.staticDataType = XPS_STG_STATIC_VARIABLES;
    if ((result = xpsStateDeleteData(scopeId, stgStaticDataDbHndl,
                                     (xpsDbKey_t)&keyStaticVarDb, (void**)&staticVarDb)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsStateDeleteData failed");
        return result;
    }

    if ((result = xpsStateHeapFree((void*)staticVarDb)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsStateHeapFree failed");
        return result;
    }

    return result;
}

static XP_STATUS xpsStgInsertStaticVariablesDb(xpsScope_t scopeId)
{

    XP_STATUS result = XP_NO_ERR;
    xpsStgStaticDbEntry *staticVarDb;

    if ((result = xpsStateHeapMalloc(sizeof(xpsStgStaticDbEntry),
                                     (void**)&staticVarDb)) != XP_NO_ERR)
    {
        //LOGFN()
        return result;
    }
    if (staticVarDb == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    memset(staticVarDb, 0, sizeof(xpsStgStaticDbEntry));
    staticVarDb->staticDataType = XPS_STG_STATIC_VARIABLES;

    staticVarDb->defaultStg = SYSTEM_DEFAULT_STG;

    // Insert the stg static variable into the database
    if ((result = xpsStateInsertData(scopeId, stgStaticDataDbHndl,
                                     (void*)staticVarDb)) != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)staticVarDb);
        staticVarDb = NULL;
        return result;
    }

    result =  xpsStpReserveScope(scopeId, staticVarDb->defaultStg);
    if (result != XP_NO_ERR)
    {
        //LOGFN()
        return result;
    }

    return result;
}

static XP_STATUS xpsStgGetDb(xpsScope_t scopeId, xpsStp_t stgId,
                             xpsStgDbEntry ** stgEntryPtr)
{
    XP_STATUS result = XP_NO_ERR;
    xpsStgDbEntry keyStgEntry;

    memset(&keyStgEntry, 0x0, sizeof(xpsStgDbEntry));
    keyStgEntry.stgId = stgId;

    if ((result = xpsStateSearchData(scopeId, stgCtxDbHndl,
                                     (xpsDbKey_t)&keyStgEntry, (void**)stgEntryPtr)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, stgId(%d)", stgId);
        return result;
    }
    if (!(*stgEntryPtr))
    {

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Stg does not exist");
        return XP_ERR_KEY_NOT_FOUND;
    }

    return result;
}

static XP_STATUS xpsStgRemoveDb(xpsScope_t scopeId, xpsStp_t stgId)
{

    XP_STATUS result = XP_NO_ERR;
    xpsStgDbEntry * stgEntry = NULL;
    xpsStgDbEntry keyStgEntry;

    memset(&keyStgEntry, 0x0, sizeof(xpsStgDbEntry));
    keyStgEntry.stgId = stgId;

    if ((result = xpsStateDeleteData(scopeId, stgCtxDbHndl,
                                     (xpsDbKey_t)&keyStgEntry, (void**)&stgEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Delete stg from Vlan list failed");
        return result;
    }

    if ((result = xpsStateHeapFree((void*)stgEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Heap Freeing failed");
        return result;
    }

    return result;
}

static XP_STATUS xpsStgInsertDb(xpsScope_t scopeId, xpsStp_t stgId,
                                xpsStgDbEntry ** stgEntryPtr)
{

    XP_STATUS result = XP_NO_ERR;

    if ((result = xpsStateHeapMalloc(sizeof(xpsStgDbEntry),
                                     (void**)stgEntryPtr)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Memory allocation for stg failed");
        return result;
    }
    if (*stgEntryPtr == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    memset(*stgEntryPtr, 0, sizeof(xpsStgDbEntry));

    (*stgEntryPtr)->stgId = stgId;
    (*stgEntryPtr)->numOfVlans = 0;
    (*stgEntryPtr)->mode = XPS_STP_MODE_PORT_E;
    if (stpMode_g==XPS_STP_MODE_VLAN_E)
    {
        (*stgEntryPtr)->mode = XPS_STP_MODE_VLAN_E;
    }

    // Insert the stg -> vlans mapping into the database, stgId is the key
    if ((result = xpsStateInsertData(scopeId, stgCtxDbHndl,
                                     (void*)*stgEntryPtr)) != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)*stgEntryPtr);
        return result;
    }

    return result;
}

XP_STATUS xpsStgAddVlanDb(xpsScope_t scopeId, xpsStp_t stgId, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsStgAddVlanDb);
    XP_STATUS result = XP_NO_ERR;
    xpsStgDbEntry * stgEntryNew, * stgEntry;
    uint16_t numOfVlans, index;

    result = xpsStgGetDb(scopeId, stgId, &stgEntry);

    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get stg from vlan list failed, "
              "stgId(%d) vlanId(%d) ", stgId, vlanId);
        return result;
    }

    numOfVlans = stgEntry->numOfVlans;

    //Check whether vlan is already part of stg
    for (index = 0; index < numOfVlans; index++)
    {
        if (stgEntry->vlans[index] == vlanId)
        {
            result = XP_NO_ERR;
            return result;
        }
    }

    //Number of vlans increase exponentially starting from XPS_DEFAULT_NUM_VLANS_IN_STG by default
    //Everytime the number reaches the current size, the size is doubled
    if (xpsDAIsCtxGrowthNeeded(numOfVlans, XPS_DEFAULT_NUM_VLANS_IN_STG))
    {
        //Allocate a new context with double interface array size
        uint16_t numOfBytes = xpsDAGetCtxSizeWhileGrow(sizeof(xpsStgDbEntry),
                                                       sizeof(uint16_t), numOfVlans, XPS_DEFAULT_NUM_VLANS_IN_STG);
        if ((result = xpsStateHeapMalloc(numOfBytes,
                                         (void**)&stgEntryNew)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Heap Allocation failed, stgId(%d) vlanId(%d) ", stgId, vlanId);
            return result;
        }
        if (stgEntryNew == NULL)
        {
            result = XP_ERR_NULL_POINTER;
            return result;
        }


        memset(stgEntryNew, 0, sizeof(xpsStgDbEntry));

        //Copy old context into new context (should copy old context size - smaller one)
        numOfBytes = sizeof(xpsStgDbEntry)+ sizeof(uint16_t)*
                     (numOfVlans - XPS_DEFAULT_NUM_VLANS_IN_STG);
        memcpy(stgEntryNew, stgEntry, numOfBytes);

        //Delete the old stgVlansCtx
        result = xpsStgRemoveDb(scopeId, stgId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Delete stg VlanCtx failed, stgId(%d)", stgId);
            xpsStateHeapFree((void*)stgEntryNew);
            return result;
        }
        // Insert the stg -> vlans mapping into the database, stgId is the key
        if ((result = xpsStateInsertData(scopeId, stgCtxDbHndl,
                                         (void*)stgEntryNew)) != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)stgEntryNew);
            return result;
        }

        stgEntry = stgEntryNew;
    }

    stgEntry->vlans[numOfVlans] = vlanId;
    stgEntry->numOfVlans += 1;

    XPS_FUNC_EXIT_LOG();

    return result;
}

XP_STATUS xpsStgRemoveVlanDb(xpsScope_t scopeId, xpsStp_t stgId,
                             xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsStgRemoveVlanDb);
    XP_STATUS result = XP_NO_ERR;
    xpsStgDbEntry * stgEntryNew, * stgEntry;
    uint16_t j = 0, numOfVlans;
    uint8_t vlanPresent = 0;

    result = xpsStgGetDb(scopeId, stgId, &stgEntry);
    if (result != XP_NO_ERR)
    {
        return result;
    }

    numOfVlans = stgEntry->numOfVlans;
    if (!numOfVlans)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Does not exist, stgId(%d)",
              stgId);
        result = XP_ERR_KEY_NOT_FOUND;
        return result;
    }

    stgEntry->numOfVlans = numOfVlans - 1;
    for (j = 0; j < numOfVlans; j++)
    {
        if (!vlanPresent)
        {
            if (stgEntry->vlans[j] == vlanId)
            {
                vlanPresent = 1;
            }
            continue;
        }
        else if (j != 0)
        {
            stgEntry->vlans[j-1] = stgEntry->vlans[j];
        }
    }

    if (!vlanPresent)
    {
        result = XP_ERR_KEY_NOT_FOUND;
        return result;
    }

    if (xpsDAIsCtxShrinkNeeded(numOfVlans, XPS_DEFAULT_NUM_VLANS_IN_STG))
    {
        //Allocate a new context with double interface array size
        uint16_t numOfBytes = xpsDAGetCtxSizeWhileShrink(sizeof(xpsStgDbEntry),
                                                         sizeof(uint16_t), numOfVlans, XPS_DEFAULT_NUM_VLANS_IN_STG);

        if ((result = xpsStateHeapMalloc(numOfBytes,
                                         (void**)&stgEntryNew)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Heap allocation failed, stgId(%)", stgId);
            return result;
        }
        if (stgEntryNew == NULL)
        {
            result = XP_ERR_NULL_POINTER;
            return result;
        }

        memset(stgEntryNew, 0, sizeof(xpsStgDbEntry));

        //Copy old context into new context (should copy new context size - smaller)
        memcpy(stgEntryNew, stgEntry, numOfBytes);

        //Delete the old stgVlansCtx
        result = xpsStgRemoveDb(scopeId, stgId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Remove stg from vlan list failed");
            xpsStateHeapFree((void*)stgEntryNew);
            return result;
        }
        // Insert the stg -> vlans mapping into the database, stgId is the key
        if ((result = xpsStateInsertData(scopeId, stgCtxDbHndl,
                                         (void*)stgEntryNew)) != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)stgEntryNew);
            return result;
        }
    }

    XPS_FUNC_EXIT_LOG();
    return result;

}

//###################STG-PORT stp state APIs##########################################
XP_STATUS xpsStgGetIntfStateDb(xpsScope_t scopeId, xpsStp_t stgId,
                               xpsInterfaceId_t intfId, xpsStpState_e *stpState)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    xpsStgIntfStateDbEntry * stgIntfState = NULL;
    xpsStgIntfStateDbEntry keyStgIntfState;
    uint64_t stpIntfDbKey = XPS_STP_GET_STGINTFSTATE_DBKEY(intfId, stgId);

    memset(&keyStgIntfState, 0x0, sizeof(xpsStgIntfStateDbEntry));
    keyStgIntfState.stgIntfDbKey = stpIntfDbKey;

    if ((result = xpsStateSearchData(scopeId, stgIntfStateDbHndl,
                                     (xpsDbKey_t)&keyStgIntfState, (void**)&stgIntfState)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, stpId(%d)", stgId);
        return result;
    }

    if (!stgIntfState)
    {
        *stpState = SPAN_STATE_DISABLE; //By default putting it in disable state
    }
    else
    {
        *stpState = stgIntfState->state;
    }

    XPS_FUNC_EXIT_LOG();
    return result;

}

XP_STATUS xpsStgGetStgIntfEntryDb(xpsScope_t scopeId, xpsStp_t stgId,
                                  xpsInterfaceId_t intfId, xpsStgIntfStateDbEntry **stgIntfStatePtr)
{
    XP_STATUS result = XP_NO_ERR;
    xpsStgIntfStateDbEntry keyStgIntfState;
    uint64_t stpIntfDbKey = XPS_STP_GET_STGINTFSTATE_DBKEY(intfId, stgId);

    memset(&keyStgIntfState, 0x0, sizeof(xpsStgIntfStateDbEntry));
    keyStgIntfState.stgIntfDbKey = stpIntfDbKey;

    if ((result = xpsStateSearchData(scopeId, stgIntfStateDbHndl,
                                     (xpsDbKey_t)&keyStgIntfState, (void**)stgIntfStatePtr)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, stpId(%d)", stgId);
        return result;
    }

    return result;
}

XP_STATUS xpsStgRemoveIntfStateDb(xpsScope_t scopeId, xpsStp_t stgId,
                                  xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    xpsStgIntfStateDbEntry * stgIntfState = NULL;
    xpsStgIntfStateDbEntry keyStgIntfState;
    uint64_t stpIntfDbKey = XPS_STP_GET_STGINTFSTATE_DBKEY(intfId, stgId);

    memset(&keyStgIntfState, 0x0, sizeof(xpsStgIntfStateDbEntry));
    keyStgIntfState.stgIntfDbKey = stpIntfDbKey;

    if ((result = xpsStateDeleteData(scopeId, stgIntfStateDbHndl,
                                     (xpsDbKey_t)&keyStgIntfState, (void**)&stgIntfState)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Delete data failed, stgId(%d)", stgId);
        return result;
    }

    if ((result = xpsStateHeapFree((void*)stgIntfState)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Heap free failed, stgId(%d)",
              stgId);
        return result;
    }

    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsStgInsertIntfStateDb(xpsScope_t scopeId, xpsStp_t stgId,
                                  xpsInterfaceId_t intfId, xpsStpState_e stpState)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    xpsStgIntfStateDbEntry * stgIntfState = NULL;
    uint64_t stpIntfDbKey = XPS_STP_GET_STGINTFSTATE_DBKEY(intfId, stgId);

    if ((result = xpsStateHeapMalloc(sizeof(xpsStgIntfStateDbEntry),
                                     (void**)&stgIntfState)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Heap Allocation failed, stgId(%d), interface(%d)", stgId, intfId);
        return result;
    }
    if (stgIntfState == NULL)
    {
        result = XP_ERR_NULL_POINTER;
        return result;
    }

    memset(stgIntfState, 0, sizeof(xpsStgIntfStateDbEntry));

    stgIntfState->stgIntfDbKey = stpIntfDbKey;
    stgIntfState->state = stpState;

    // Insert the stg + intf to stpstate mapping into the database, stgId and port form the key
    if ((result = xpsStateInsertData(scopeId, stgIntfStateDbHndl,
                                     (void*)stgIntfState)) != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)stgIntfState);
        return result;
    }

    XPS_FUNC_EXIT_LOG();
    return result;

}

/*User APIs*/
XP_STATUS xpsStgGetVlanList(xpsDevice_t devId, xpsStp_t stpId,
                            xpsVlan_t **vlanList, uint16_t *numOfVlans)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    xpsStgDbEntry * stgEntry = NULL;
    xpsScope_t scopeId;
    *numOfVlans = 0;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID:%d", devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    /* Get Scope Id from devId */
    if ((result = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return result;
    }
    result = xpsStgGetDb(scopeId, stpId, &stgEntry);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vlan from list failed, stpId(%d)", stpId);
        return result;
    }
    if (stgEntry && stgEntry->mode == XPS_STP_MODE_VLAN_E)
    {
        *vlanList = stgEntry->vlans;
        *numOfVlans= stgEntry->numOfVlans;
    }

    XPS_FUNC_EXIT_LOG();
    return result;

}

XP_STATUS xpsStpInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return xpsStpInitScope(XP_SCOPE_DEFAULT);
}

XP_STATUS xpsStpInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    xpsLockCreate(XP_LOCKINDEX_XPS_STP, 1);

    XP_STATUS result = XP_NO_ERR;

    /* initialize the Stp Id Allocator*/
    result = xpsAllocatorInitIdAllocator(scopeId, XPS_ALLOCATOR_STP_ID,
                                         XP_STP_MAX_IDS(0), XP_STP_RANGE_START);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to initialize XPS Stp allocator\n");
        return result;
    }

    stgCtxDbHndl = XPS_STG_VLANS_CTX_DB_HNDL;
    result = xpsStateRegisterDb(scopeId, "STG-VLANS", XPS_GLOBAL, &xpsStpKeyComp,
                                stgCtxDbHndl);
    if (result != XP_NO_ERR)
    {
        stgCtxDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Registering STG-VLANS failed");
        return result;
    }

    stgIntfStateDbHndl = XPS_STG_INTF_STATE_DB_HNDL;
    result = xpsStateRegisterDb(scopeId, "Intf STP state", XPS_GLOBAL,
                                &stpMgrIntfKeyComp, stgIntfStateDbHndl);
    if (result != XP_NO_ERR)
    {
        stgIntfStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Registering Intf STP state failed");
        return result;
    }
    stgStaticDataDbHndl = XPS_STG_STATIC_DATA_DB_HNDL;
    result = xpsStateRegisterDb(scopeId, "STG global variables", XPS_GLOBAL,
                                &stpMgrStaticVarKeyComp, stgStaticDataDbHndl);
    if (result != XP_NO_ERR)
    {
        stgStaticDataDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        //LOGFN()
        return result;
    }

    if (xpsGetInitType() == INIT_COLD)
    {
        result = xpsStgInsertStaticVariablesDb(scopeId);
        if (result != XP_NO_ERR)
        {
            //LOGFN()
            return result;
        }
    }

    XPS_FUNC_EXIT_LOG();
    return result;

}

XP_STATUS xpsStpDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return xpsStpDeInitScope(XP_SCOPE_DEFAULT);
}

XP_STATUS xpsStpDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;

    result = xpsStgRemoveStaticVariablesDb(scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStgRemoveStaticVariablesDb failed");
        return result;
    }

    result = xpsStateDeRegisterDb(scopeId, &stgCtxDbHndl);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "De-resgistering STG-VLANS failed");
        return result;
    }

    result = xpsStateDeRegisterDb(scopeId, &stgIntfStateDbHndl);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "De-resgistering Intf STP state failed");
        return result;
    }

    result = xpsStateDeRegisterDb(scopeId, &stgStaticDataDbHndl);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsStateDeRegisterDb failed");
        return result;
    }

    xpsLockDestroy(XP_LOCKINDEX_XPS_STP);

    XPS_FUNC_EXIT_LOG();
    return result;

}

XP_STATUS xpsStpAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    xpsPort_t portNum = 0;
    xpsStgStaticDbEntry *staticVarDb;
    xpsScope_t scopeId;
    CPSS_BRG_STP_STATE_MODE_ENT stpMode;
    GT_STATUS status = GT_OK;
    uint8_t numPorts = 0;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID:%d", devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    /* Get Scope Id from devId */
    if ((result = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return result;
    }

    if (initType != INIT_COLD)
    {
        return XP_NO_ERR;
    }

    //Get the default stg
    result =  xpsStgGetStaticVariablesDb(scopeId, &staticVarDb);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Default Stg creation failed");
        return result;
    }

    result = xpsGlobalSwitchControlGetMaxPorts(devId, &numPorts);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to get max ports\n");
        return result;
    }

    if (stpMode_g==XPS_STP_MODE_VLAN_E)
    {
        stpMode = CPSS_BRG_STP_E_VLAN_MODE_E;
    }
    else
    {
        stpMode = CPSS_BRG_STP_E_PORT_MODE_E;
    }

    XPS_GLOBAL_PORT_ITER(portNum, numPorts)
    {
        status = cpssHalSetBrgPortStpMode(devId, portNum, stpMode);
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set Brg mode failed ,"
                  " devId(%d)  interface(%d)", devId,  portNum);
            return xpsConvertCpssStatusToXPStatus(status);
        }

        /* set all ports in default STP forwarding state */
        if (stpMode_g==XPS_STP_MODE_VLAN_E)
        {

            status = cpssHalSetBrgStpState(devId, portNum, staticVarDb->defaultStg,
                                           CPSS_STP_FRWRD_E);
            if (GT_OK != status)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalSetBrgStpState  port=%d stpId=%d", portNum, staticVarDb->defaultStg);
                return xpsConvertCpssStatusToXPStatus(status);
            }
        }
    }
    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsStpRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsStpIsExist(xpsScope_t scopeId, xpsStp_t stpId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    xpsStgDbEntry * stgEntry = NULL;
    xpsStgDbEntry keyStgEntry;

    memset(&keyStgEntry, 0x0, sizeof(xpsStgDbEntry));
    keyStgEntry.stgId = stpId;

    if ((result = xpsStateSearchData(scopeId, stgCtxDbHndl,
                                     (xpsDbKey_t)&keyStgEntry, (void**)&stgEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, stpId(%d)", stpId);
        return result;
    }

    if (!stgEntry)
    {
        result = XP_ERR_RESOURCE_NOT_AVAILABLE;
        return result;
    }

    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsValidatePortStpId(xpsDevice_t devId, xpsInterfaceId_t intfId,
                               xpsStp_t stpId)
{
    XP_STATUS result = XP_NO_ERR;
    xpsStp_t portStpId = XPS_INVALID_STP;
    if (stpMode_g==XPS_STP_MODE_VLAN_E)
    {
        return XP_NO_ERR;
    }
    result = xpsGetPortStpId(devId, intfId, &portStpId);
    if (result != XP_NO_ERR)
    {
        return result;
    }

    if (portStpId != XPS_INVALID_STP && portStpId != stpId)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "STP Port Already Set,"
              " devId(%d) PrevstpId(%d) interface(%d)", devId, portStpId, intfId);
        return XP_ERR_RESOURCE_BUSY;
    }
    return result;
}

XP_STATUS xpsGetPortStpId(xpsDevice_t devId, xpsInterfaceId_t intfId,
                          xpsStp_t *stpId)
{
    XP_STATUS result = XP_NO_ERR;
    xpsInterfaceInfo_t *intfInfo = NULL;

    if (!stpId)
    {
        return XP_ERR_NULL_POINTER;
    }
    //get the interface db and update the corresponding STP id for same
    result = xpsInterfaceGetInfo(intfId, &intfInfo);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsGetPortStpId failed,"
              " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
        return result;
    }
    if (intfInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interface info failed recieved Null Pointer,"
              " devId(%d)  interface(%d)", devId, intfId);
        return XP_ERR_NULL_POINTER;
    }
    *stpId = intfInfo->stpId;
    return result;
}

XP_STATUS xpsStpIsExistOnDevice(xpsDevice_t devId, xpsStp_t stpId)
{
    XPS_FUNC_ENTRY_LOG();

    xpsScope_t scopeId = 0;
    XP_STATUS result = XP_NO_ERR;

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

    XPS_FUNC_EXIT_LOG();
    return xpsStpIsExist(scopeId, stpId);
}

//Default stp is a global variable
XP_STATUS xpsStpSetDefault(xpsDevice_t devId, xpsStp_t stpId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    xpsStgStaticDbEntry *staticVarDb;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID:%d", devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    /* Get Scope Id from devId */
    if ((result = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return result;
    }

    result = xpsStpIsExist(scopeId, stpId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Stp Does not exit");
        return result;
    }

    //Get the default stg
    result =  xpsStgGetStaticVariablesDb(scopeId, &staticVarDb);
    if (result != XP_NO_ERR)
    {
        //LOGFN()
        return result;
    }

    staticVarDb->defaultStg = stpId;

    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsStpGetDefault(xpsDevice_t devId, xpsStp_t *stpId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    xpsStgStaticDbEntry *staticVarDb;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID:%d", devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    /* Get Scope Id from devId */
    if ((result = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return result;
    }

    //Get the default stg
    result =  xpsStgGetStaticVariablesDb(scopeId, &staticVarDb);
    if (result != XP_NO_ERR)
    {
        //LOGFN()
        return result;
    }

    *stpId = staticVarDb->defaultStg;

    XPS_FUNC_EXIT_LOG();
    return result;

}

GT_U32 xpsConvertStpPortStateCpssToXps(CPSS_STP_STATE_ENT cpssStpState,
                                       xpVlanStgState_e* xpsStpState)
{
    XPS_FUNC_ENTRY_LOG();
    //Converts the stp port state from CPSS_STP_STATE_ENT to xpVlanStgState_e
    switch (cpssStpState)
    {
        case CPSS_STP_BLCK_LSTN_E:
            {
                *xpsStpState = SPAN_STATE_BLOCK;
                break;
            }
        case CPSS_STP_LRN_E:
            {
                *xpsStpState = SPAN_STATE_LEARN;
                break;
            }
        case CPSS_STP_FRWRD_E:
            {
                *xpsStpState = SPAN_STATE_FORWARD;
                break;
            }
        case CPSS_STP_DISABLED_E:
            {
                *xpsStpState = SPAN_STATE_DISABLE;
                break;
            }
        default:
            {
                cpssOsPrintf("Unknown cpssStpState %d\n", cpssStpState);
                return GT_BAD_VALUE;
            }
    }
    XPS_FUNC_EXIT_LOG();
    return GT_OK;
}

XP_STATUS xpsStpCreate(xpsStp_t *stpId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsStpCreate);

    XPS_FUNC_EXIT_LOG();
    return xpsStpCreateScope(XP_SCOPE_DEFAULT, stpId);

}

XP_STATUS xpsStpCreateScope(xpsScope_t scopeId, xpsStp_t *stpId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsStpCreate);

    XPS_FUNC_EXIT_LOG();
    return xpsStpCreateWithState(XP_SCOPE_DEFAULT, SPAN_STATE_FORWARD, stpId);

}

XP_STATUS xpsStpCreateWithState(xpsScope_t scopeId, xpsStpState_e stpDefState,
                                xpsStp_t *stpId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsStpCreate);
    XP_STATUS result;
    xpsStgDbEntry * stgEntry = NULL;
    xpsDevice_t devId;
    GT_U16 stpIdVal;
    GT_STATUS status;
    uint32_t portId = 0;
    CPSS_STP_STATE_ENT   state = CPSS_STP_DISABLED_E;
    uint8_t numPorts = 0;
    //GT_U32      stpEntryState[] = {00, 55, aa, ff} ;

    result = xpsScopeGetFirstDevice(scopeId, &devId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsScopeGetFirstDevice() Failed %d\n", result);
        return result;
    }

    result = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_STP_ID, stpId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Allocate stg failed");
        return result;
    }

    result = xpsStgInsertDb(scopeId, *stpId, &stgEntry);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Insert into vlan list failed");
        return result;
    }
    stpIdVal = (uint16_t)(*stpId);

    if (stpDefState >= SPAN_STATE_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid STP state input");
        return XP_ERR_INVALID_ARG;
    }

    //As its dynamic allocation the db cant already exist. Create DB.
    if (stgEntry->mode == XPS_STP_MODE_VLAN_E)
    {
        result = xpsGlobalSwitchControlGetMaxPorts(devId, &numPorts);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to get max ports\n");
            return result;
        }

        status = cpssHalConvertStpPortStateXpsToCpss(stpDefState, &state);
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Stp State:%d",
                  stpDefState);
            return xpsConvertCpssStatusToXPStatus(status);
        }

        /*Set egress filter table to default state if its not forward*/
        // per vlan STP
        XPS_GLOBAL_PORT_ITER(portId, numPorts)
        {
            status = cpssHalSetBrgStpState(devId, portId, (GT_U16) stpIdVal, state);
            if (GT_OK != status)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress state failed,"
                      " devId(%d) stpId(%d) interface(%d)", devId, stpId, portId);
                return xpsConvertCpssStatusToXPStatus(status);
            }
        }
    }

#if 0
    //memset(stpEntry, (stpDefState | (stpDefState << 2)), sizeof(stpEntry));
    //memset(stpEntry, stpEntryState[stpDefState], sizeof(stpEntry));
    //memset(stpEntry, 0xff, sizeof(stpEntry));
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        cpssOsMemSet(stpEntry, 0x0, sizeof(stpEntry));
        /* call cpss api function */
        status = cpssDxChBrgStpEntryWrite(devNum, stpIdVal, stpEntry);
        if (status != GT_OK)
        {
            result = xpsConvertCpssStatusToXPStatus(status);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChBrgStpEntryWrite() Failed %d\n", result);
            }
            return result;
        }
    }
#endif
    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsStpReserve(xpsStp_t stpId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsStpReserve);

    XPS_FUNC_EXIT_LOG();
    return xpsStpReserveScope(XP_SCOPE_DEFAULT,  stpId);

}

XP_STATUS xpsStpReserveScope(xpsScope_t scopeId, xpsStp_t stpId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result;
    xpsStgDbEntry * stgEntry = NULL;
    xpsStgDbEntry keyStgEntry;

    if (stpId > SYSTEM_MAX_EGRESS_FILTER_ID)
    {
        return XP_ERR_INVALID_ARG;
    }

    memset(&keyStgEntry, 0x0, sizeof(xpsStgDbEntry));
    keyStgEntry.stgId = stpId;

    if ((result = xpsStateSearchData(scopeId, stgCtxDbHndl,
                                     (xpsDbKey_t)&keyStgEntry, (void**)&stgEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, stpId(%d)", stpId);
        return result;
    }

    if (stgEntry)
    {
        result = XP_ERR_KEY_EXISTS;
        return result;
    }

    result = xpsAllocatorAllocateWithId(scopeId, XPS_ALLOCATOR_STP_ID, stpId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Stg allocation with Id failed, stpId(%d)", stpId);
        return result;
    }

    //    if (stpMode_g == XPS_STP_MODE_VLAN_E)
    {
        result=  xpsStgInsertDb(scopeId, stpId, &stgEntry);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Inserting to Vlan list failed, stpId(%d)", stpId);
            return result;
        }
    }

    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsStpDestroy(xpsStp_t stpId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsStpDestroy);

    XPS_FUNC_EXIT_LOG();

    return xpsStpDestroyScope(XP_SCOPE_DEFAULT, stpId);
}

XP_STATUS xpsStpDestroyScope(xpsScope_t scopeId, xpsStp_t stpId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result;
    uint32_t prevNodeStpId;
    xpsStgDbEntry *stgEntry;
    xpsStgIntfStateDbEntry *curStgIntfStatePtr, *prevStgIntfStatePtr;
    result = xpsStgGetDb(scopeId, stpId, &stgEntry);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " Vlan list failed, stpId(%d)",
              stpId);
        return result;
    }
#if 1
    if (stgEntry->mode == XPS_STP_MODE_VLAN_E && stgEntry->numOfVlans)
    {
        //LOGFN(); Please cleanup vlan subscriptions to this stg
        result = XP_ERR_RESOURCE_BUSY;
        return result;
    }
#endif
    //de-allocate the stg
    result = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_STP_ID, stpId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Release of Interface Id failed");
        return result;
    }

    /* Loop through all the xpsStgIntfStateDbEntry in RB tree and delete entries with matching stpId*/
    curStgIntfStatePtr = NULL;
    prevStgIntfStatePtr = NULL;
    xpsStgIntfStateDbEntry keyPrevStgIntfState;
    prevNodeStpId = 0xFFFFFFFF;

    memset(&keyPrevStgIntfState, 0x0, sizeof(xpsStgIntfStateDbEntry));

    //Get the first tree node for in-order traversal with NULL key
    result=  xpsStateGetNextData(scopeId, stgIntfStateDbHndl, NULL,
                                 (void * *)&curStgIntfStatePtr);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get Data failed");
        return result;
    }

    while (curStgIntfStatePtr)
    {
        //Delete the previous node if stpId matches
        if (prevNodeStpId == stpId)
        {
            result=  xpsStateDeleteData(scopeId, stgIntfStateDbHndl,
                                        (xpsDbKey_t)&keyPrevStgIntfState, (void**)&prevStgIntfStatePtr);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Delete data failed");
                return result;
            }

            result=  xpsStateHeapFree((void*)prevStgIntfStatePtr);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Heap de-allocation  failed");
                return result;
            }
        }

        //Store the node information in previous and get-next
        prevStgIntfStatePtr = curStgIntfStatePtr;
        keyPrevStgIntfState.stgIntfDbKey = prevStgIntfStatePtr->stgIntfDbKey;
        prevNodeStpId = XPS_STP_GET_STPID_FROM_DBKEY(keyPrevStgIntfState.stgIntfDbKey);

        //get-next node
        result=  xpsStateGetNextData(scopeId, stgIntfStateDbHndl,
                                     (xpsDbKey_t)&keyPrevStgIntfState, (void * *)&curStgIntfStatePtr);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get next node failed");
            return result;
        }
    }

    //Delete the last node of the tree if stpId matches.
    if (prevNodeStpId == stpId)
    {
        result=  xpsStateDeleteData(scopeId, stgIntfStateDbHndl,
                                    (xpsDbKey_t)&keyPrevStgIntfState, (void**)&prevStgIntfStatePtr);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Delete data failed");
            return result;
        }

        result=  xpsStateHeapFree((void*)prevStgIntfStatePtr);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Heap de-allocation  failed");
            return result;
        }
    }
    //    if (stpMode_g==XPS_STP_MODE_VLAN_E)
    {

        result = xpsStgRemoveDb(scopeId, stpId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Stg Remove from vlan list, stpId(%d)", stpId);
            return result;
        }
    }
    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsStpUpdateIntfStateDb(xpsDevice_t devId, xpsStp_t stpId,
                                  xpsInterfaceId_t intfId, xpsStpState_e stpState)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsStpUpdateIntfStateDb);
    XP_STATUS result = XP_NO_ERR;
    xpsStgIntfStateDbEntry * stgIntfState = NULL;
    xpsScope_t scopeId;

    /* Get Scope Id from devId */
    if ((result = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return result;
    }

    // Create/update the software state
    result = xpsStgGetStgIntfEntryDb(scopeId, stpId, intfId, &stgIntfState);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get interface entry failed,"
              " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
        return result;
    }

    if (stgIntfState)
    {
        stgIntfState->state = stpState;
    }
    else
    {
        result = xpsStgInsertIntfStateDb(scopeId, stpId, intfId, stpState);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Insert interface state failed,"
                  " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
            return result;
        }
    }

    XPS_FUNC_EXIT_LOG();
    return result;
}


/* For all isolated vlans in a private vlan one stg must be created and for that all ports' egress stpstate
must be blocked */
XP_STATUS xpsStpSetState(xpsDevice_t devId, xpsStp_t stpId,
                         xpsInterfaceId_t intfId, xpsStpState_e stpState)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result;

    CPSS_STP_STATE_ENT   state = CPSS_STP_DISABLED_E;
    GT_STATUS status;
    xpsInterfaceId_t portIntf;
    xpsLagPortIntfList_t lagPortList;
    xpsInterfaceType_e  portIntfType;
    uint32_t i = 0;
    xpsInterfaceInfo_t *intfInfo = NULL;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID:%d", devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    status = cpssHalConvertStpPortStateXpsToCpss(stpState, &state);
    if (GT_OK != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Stp State:%d",
              stpState);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    /*check interface type for input port to decide whether it is single port or LAG*/
    result = xpsInterfaceGetType(intfId, &portIntfType);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Get Interface type failed, interface(%d)", intfId);
        return result;
    }

    if (portIntfType == XPS_PORT)
    {
        if (stpMode_g==XPS_STP_MODE_VLAN_E)
        {
            status = cpssHalSetBrgStpState(devId, intfId, (GT_U16) stpId, state);
            if (GT_OK != status)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress state failed,"
                      " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
                return xpsConvertCpssStatusToXPStatus(status);
            }
        }
        else
        {
            status = cpssHalSetBrgPortStpState(devId, intfId, state);
            if (GT_OK != status)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress state failed,"
                      " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
                return xpsConvertCpssStatusToXPStatus(status);
            }
        }
    }
    else if (portIntfType == XPS_LAG)
    {
        memset(&lagPortList, 0, sizeof(lagPortList));
        result = xpsLagGetPortIntfList(intfId, &lagPortList);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed, lag interface(%d)", intfId);
            return result;
        }

        for (i = 0; i < lagPortList.size; i++)
        {
            portIntf = lagPortList.portIntf[i];

            if (stpMode_g==XPS_STP_MODE_VLAN_E)
            {
                status = cpssHalSetBrgStpState(devId, portIntf, (GT_U16) stpId, state);
                if (GT_OK != status)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress state failed,"
                          " devId(%d) stpId(%d) interface(%d)", devId, stpId, portIntf);
                    return xpsConvertCpssStatusToXPStatus(status);
                }
            }
            else
            {
                status = cpssHalSetBrgPortStpState(devId, portIntf, state);
                if (GT_OK != status)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress state failed,"
                          " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
                    return xpsConvertCpssStatusToXPStatus(status);
                }
            }
        }
    }

    //  Below code is for updating the state in local db
    result = xpsStpUpdateIntfStateDb(devId, stpId, intfId, stpState);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress state failed,"
              " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
        return result;
    }
    //get the interface db and update the corresponding STP id for same
    result = xpsInterfaceGetInfo(intfId, &intfInfo);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress state failed,"
              " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
        return result;
    }
    if (intfInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interface info failed recieved Null Pointer,"
              " devId(%d)  interface(%d)", devId, intfId);
        return XP_ERR_NULL_POINTER;
    }
    intfInfo->stpId = stpId;

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsStpGetState(xpsDevice_t devId, xpsStp_t stpId,
                         xpsInterfaceId_t intfId, xpsStpState_e *stpState)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result;
    xpsScope_t scopeId;

    /* Get Scope Id from devId */
    if ((result = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return result;
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID:%d", devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    result = xpsStgGetIntfStateDb(scopeId, stpId, intfId, stpState);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get interface state failed,"
              " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
        return result;
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

static XP_STATUS xpsStpResetIntfStateDb(xpsDevice_t devId, xpsStp_t stpId,
                                        xpsInterfaceId_t intfId, xpsStpState_e defaultStpState)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS result = XP_NO_ERR;
    xpsVlan_t i = 0;
    xpsStgDbEntry * stgEntry;
    xpsScope_t scopeId;

    /* Get Scope Id from devId */
    if ((result = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return result;
    }

    /*1. Fetch vlans and then get the vlan, then set intf+vlan state (ingress default state) in that. */
    result = xpsStgGetDb(scopeId, stpId, &stgEntry);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get vlan list failed, stpId intfId", stpId, intfId);
        return result;
    }

    if (stgEntry-> mode == XPS_STP_MODE_VLAN_E)
    {
        for (i=0; i < stgEntry->numOfVlans; i++)
        {
            result = xpsVlanSetIngStpState(devId, stgEntry->vlans[i], intfId,
                                           defaultStpState);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Vlan set Ing stp state failed,"
                      " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
                return result;
            }
        }
    }

    /*2. delete the software state*/
    result = xpsStgRemoveIntfStateDb(scopeId, stpId, intfId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Remove interface entry failed,"
              " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
        return result;
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}


XP_STATUS xpsStgGetStgCount(xpsScope_t scopeId, uint32_t *count)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    result = xpsStateGetCount(scopeId, stgCtxDbHndl, count);

    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsStpGetStpIdList(xpsScope_t scopeId, xpsStp_t* stpId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    xpsStgDbEntry *curStgEntry = NULL;
    xpsStgDbEntry *prevStgEntry = NULL;
    uint32_t prevStgDbKey = 0xFFFFFFFF;//stgId is the key
    uint32_t count = 0;
    result=  xpsStateGetNextData(scopeId, stgCtxDbHndl, NULL,
                                 (void **)&curStgEntry);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get data, return Value : %d", result);
        return result;
    }
    while (curStgEntry)
    {
        //Store the node information in previous and get-next
        prevStgEntry = curStgEntry;
        prevStgDbKey  = prevStgEntry->stgId;
        stpId[count] = prevStgDbKey;
        result=  xpsStateGetNextData(scopeId, stgCtxDbHndl,
                                     (xpsDbKey_t)&prevStgDbKey, (void **)&curStgEntry);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get data, return Value : %d", result);
            return result;
        }
        count++;
    }

    XPS_FUNC_EXIT_LOG();
    return result;

}

/*Clears the software states and resets the stp state to default */
XP_STATUS xpsStpResetState(xpsDevice_t devId, xpsStp_t stpId,
                           xpsInterfaceId_t intfId, xpsStpState_e defaultStpState)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result;

    CPSS_STP_STATE_ENT state = CPSS_STP_DISABLED_E;
    GT_STATUS status;
    xpsInterfaceType_e  portIntfType;
    xpsLagPortIntfList_t lagPortList;
    xpsInterfaceId_t portIntf;
    xpsInterfaceInfo_t *intfInfo = NULL;
    uint32_t i = 0;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID:%d", devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    status = cpssHalConvertStpPortStateXpsToCpss(defaultStpState, &state);
    if (GT_OK != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Stp State:%d",
              defaultStpState);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    /*check interface type for input port to decide whether it is single port or LAG*/
    result = xpsInterfaceGetType(intfId, &portIntfType);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Get Interface type failed, interface(%d)", intfId);
        return result;
    }

    if (portIntfType == XPS_PORT)
    {
        if (stpMode_g==XPS_STP_MODE_VLAN_E)
        {
            status = cpssHalSetBrgStpState(devId, intfId, (GT_U16) stpId, state);
            if (GT_OK != status)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress state failed,"
                      " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
                return xpsConvertCpssStatusToXPStatus(status);
            }
        }
        else
        {
            status = cpssHalSetBrgPortStpState(devId, intfId, state);
            if (GT_OK != status)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress state failed,"
                      " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
                return xpsConvertCpssStatusToXPStatus(status);
            }
        }
    }
    else if (portIntfType == XPS_LAG)
    {
        memset(&lagPortList, 0, sizeof(lagPortList));
        result = xpsLagGetPortIntfList(intfId, &lagPortList);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed, lag interface(%d)", intfId);
            return result;
        }

        for (i = 0; i < lagPortList.size; i++)
        {
            portIntf = lagPortList.portIntf[i];
            if (stpMode_g==XPS_STP_MODE_VLAN_E)
            {
                status = cpssHalSetBrgStpState(devId, portIntf, (GT_U16) stpId, state);
                if (GT_OK != status)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress state failed,"
                          " devId(%d) stpId(%d) interface(%d)", devId, stpId, portIntf);
                    return xpsConvertCpssStatusToXPStatus(status);
                }
            }
            else
            {
                status = cpssHalSetBrgPortStpState(devId, portIntf, state);
                if (GT_OK != status)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress state failed,"
                          " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
                    return xpsConvertCpssStatusToXPStatus(status);
                }
            }


        }
    }
    /*Reset port vlan table to default and delete software states*/
    result = xpsStpResetIntfStateDb(devId, stpId, intfId, defaultStpState);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress state failed,"
              " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
        return result;
    }

    //get the interface db and update the corresponding STP id for same
    result = xpsInterfaceGetInfo(intfId, &intfInfo);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress state failed,"
              " devId(%d) stpId(%d) interface(%d)", devId, stpId, intfId);
        return result;
    }
    if (intfInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interface info failed recieved Null Pointer,"
              " devId(%d)  interface(%d)", devId, intfId);
        return XP_ERR_NULL_POINTER;
    }
    intfInfo->stpId = XPS_INVALID_STP;

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;

}

XP_STATUS xpsStpGetInterfaceList(xpsDevice_t devId, xpsStp_t stpId,
                                 xpsInterfaceId_t **interfaceList, uint16_t *numOfIntfs)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result;
    uint32_t curNodeStpId;
    uint16_t intfCount = 0;
    xpsScope_t scopeId;
    xpsInterfaceId_t *intfList, curNodeIntfId;
    xpsStgIntfStateDbEntry *curStgIntfStatePtr, keyStgIntfState;
    int maxPorts = 0;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Device ID:%d", devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    /* Get Scope Id from devId */
    if ((result = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return result;
    }

    /* Loop through all the xpsStgIntfStateDbEntry in RB tree and delete entries with matching stpId*/
    curStgIntfStatePtr = NULL;
    curNodeStpId = 0xFFFFFFFF;
    curNodeIntfId = 0xFFFFFFFF;

    memset(&keyStgIntfState, 0x0, sizeof(xpsStgIntfStateDbEntry));

    result = cpssHalGetMaxGlobalPorts(&maxPorts);

    /*Allocate interface array*/
    if ((result = xpsStateHeapMalloc(sizeof(xpsInterfaceId_t)*maxPorts,
                                     (void**)&intfList)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "XPS state Heap Allocation failed");
        return result;
    }
    if (intfList == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    memset(intfList, 0, sizeof(xpsInterfaceId_t));

    //Get the first tree node for in-order traversal with NULL key
    result=  xpsStateGetNextData(scopeId, stgIntfStateDbHndl, NULL,
                                 (void **)&curStgIntfStatePtr);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get Data failed");
        xpsStateHeapFree((void *)intfList);
        return result;
    }

    while (curStgIntfStatePtr)
    {
        curNodeStpId = XPS_STP_GET_STPID_FROM_DBKEY(curStgIntfStatePtr->stgIntfDbKey);
        curNodeIntfId = XPS_STP_GET_INTFID_FROM_DBKEY(curStgIntfStatePtr->stgIntfDbKey);

        //Fill the interface in the list if stpId matches
        if (curNodeStpId == stpId)
        {
            intfList[intfCount] = curNodeIntfId;
            intfCount++;
        }

        //Build key from current node and get-next
        keyStgIntfState.stgIntfDbKey = curStgIntfStatePtr->stgIntfDbKey;

        //get-next node
        result=  xpsStateGetNextData(scopeId, stgIntfStateDbHndl,
                                     (xpsDbKey_t)&keyStgIntfState, (void * *)&curStgIntfStatePtr);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get next node failed");
            xpsStateHeapFree((void *)intfList);
            return result;
        }
    }

    *numOfIntfs = intfCount;
    *interfaceList = intfList;

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsVlanInterfaceDefaultIngressStpSet(xpsDevice_t devId,
                                               xpsInterfaceId_t intfId, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    xpsStp_t stpId;
    xpsStpState_e stpState;
    if (stpMode_g!=XPS_STP_MODE_VLAN_E)
    {
        return result;
    }
    result = xpsVlanGetStp(devId, vlanId, &stpId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get stp failed, devId(%d) vlanId(%d)", devId, vlanId);
        return result;
    }
    result = xpsStpGetState(devId, stpId, intfId, &stpState);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "stp state get failed");
        return result;
    }

    result = xpsStpUpdateIntfStateDb(devId, stpId, intfId, stpState);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "stp Ingress state set failed");
        return result;
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsStpUpdLagMember(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                             xpsInterfaceId_t iface, bool removed)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsStpUpdLagMember);

    XP_STATUS result = XP_NO_ERR;
    xpsStgIntfStateDbEntry *stgIntfState = NULL;
    GT_STATUS                    status;
    CPSS_STP_STATE_ENT   state = CPSS_STP_DISABLED_E;

    xpsStgDbEntry *curStgEntry = NULL;
    uint32_t stgId = 0;
    xpsScope_t scopeId;
    xpsStpState_e stpState = SPAN_STATE_DISABLE;
    xpsStp_t stpId;
    xpsInterfaceInfo_t *intfInfo = NULL;
    xpsInterfaceId_t stpPortIntf;

    /* If port is removed from LAG - use STP state of port
     * If port is added to LAG - use STP state of LAG */
    if (removed)
    {
        stpPortIntf = iface;
    }
    else
    {
        stpPortIntf = lagIntf;
    }

    /* Get Scope Id from devId */
    if ((result = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return result;
    }
    if (stpMode_g==XPS_STP_MODE_VLAN_E)
    {
        result = xpsStateGetNextData(scopeId, stgCtxDbHndl, NULL,
                                     (void **)&curStgEntry);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get data, return Value : %d", result);
            return result;
        }

        while (curStgEntry)
        {
            stgIntfState = NULL;

            stgId = curStgEntry->stgId;
            result = xpsStateGetNextData(scopeId, stgCtxDbHndl, (xpsDbKey_t)&stgId,
                                         (void **)&curStgEntry);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get data, return Value : %d", result);
                return result;
            }

            result = xpsStgGetStgIntfEntryDb(scopeId, stgId, stpPortIntf, &stgIntfState);
            if (result != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get interface stg data, return Value : %d", result);
                return result;
            }

            if (! stgIntfState)
            {
                continue;
            }

            switch (stgIntfState->state)
            {
                case SPAN_STATE_LEARN:
                case SPAN_STATE_BLOCK:
                case SPAN_STATE_DISABLE:
                case SPAN_STATE_FORWARD:
                    break;
                default:
                    return XP_ERR_INVALID_ARG;
            }

            status = cpssHalConvertStpPortStateXpsToCpss(stgIntfState->state, &state);
            if (GT_OK != status)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Stp State:%d",
                      stgIntfState->state);
                return xpsConvertCpssStatusToXPStatus(status);
            }

            status = cpssHalSetBrgStpState(devId, iface, (GT_U16) stgId, state);
            if (GT_OK != status)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set stg state failed,"
                      " devId(%d) stpId(%d) interface(%d)", devId, stgId, iface);
                return xpsConvertCpssStatusToXPStatus(status);
            }
        }
    }
    else
    {
        result = xpsInterfaceGetInfo(stpPortIntf, &intfInfo);
        if (result != XP_NO_ERR || intfInfo == NULL)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get interface info failed,"
                  " devId(%d)  interface(%d)", devId, stpPortIntf);
            return result;
        }
        stpId = intfInfo->stpId;
        if ((result = xpsStpGetState(devId, stpId, stpPortIntf,
                                     &stpState)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "STP get failed for (%d)",
                  stpPortIntf);
            stpState = SPAN_STATE_DISABLE;
        }

        status = cpssHalConvertStpPortStateXpsToCpss(stpState, &state);
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Stp State:%d",
                  stpState);
            return xpsConvertCpssStatusToXPStatus(status);
        }

        status = cpssHalSetBrgPortStpState(devId, iface, state);
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set ingress state failed,"
                  " devId(%d) stpId(%d) interface(%d)", devId, stgId, stgId);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }
    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsStpSetPortStpState(xpsDevice_t devId, xpsInterfaceId_t iface,
                                xpVlanStgState_e stgState)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS            status = GT_OK;
    CPSS_STP_STATE_ENT   state  = CPSS_STP_DISABLED_E;

    status = cpssHalConvertStpPortStateXpsToCpss(stgState, &state);
    if (GT_OK != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Stp State:%d",
              stgState);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    status = cpssHalSetBrgPortStpMode(devId, iface, CPSS_BRG_STP_E_PORT_MODE_E);
    if (GT_OK != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set port stp mode failed,"
              " devId(%d) interface(%d)", devId, iface);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    status = cpssHalSetBrgPortStpState(devId, iface, state);
    if (GT_OK != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set stg state failed,"
              " devId(%d) state(%d) interface(%d)", devId, state, iface);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
