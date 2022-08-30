// xpsPort.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsPort.h"
#include "xpsState.h"
#include "xpsInternal.h"
#include "xpsScope.h"
#include "xpsLock.h"
#include "gtEnvDep.h"
#include "cpssHalPort.h"
#include "cpssHalDevice.h"
#include "cpssHalQos.h"
#include "cpssHalVlan.h"
#include "cpssHalTunnel.h"
#include "cpssHalUtil.h"
#include "cpssHalFdb.h"
#include "xpsLink.h"
#include "xpsAcl.h"
#include "xpsTunnel.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void cpssHalDumputilsPortDump(uint8_t devId, uint32_t portNum,
                                     bool detail);

static xpsDbHandle_t portDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t intfToPortDevHndl = XPS_STATE_INVALID_DB_HANDLE;

/**
 * \brief Statically defined key compare function for port's
 *        database
 *
 * This API is defined as static, but will be registered via
 * function pointer to be used internally by State manager to do
 * comparisons for the port  database
 *
 * \param [in] key1
 * \param [in] key2
 *
 * \return int32_t
 */

static int32_t portKeyCompare(void *key1, void *key2)
{
    return ((((xpsPortIntfMap_t *) key1)->portNum) - (((xpsPortIntfMap_t *)
                                                       key2)->portNum));
}


/**
 * \brief Statically defined key compare function for the
 *        portInterface to Port/Device database
 *
 *
 * \param [in] key1
 * \param [in] key2
 *
 * \return int32_t
 */

static int32_t portDevKeyCompare(void *key1, void *key2)
{
    return (((xpsIntfPortMap_t*)key1)->intfId - ((xpsIntfPortMap_t*)key2)->intfId);
}


XP_STATUS xpsPortDbInit(void)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();
    return xpsPortDbInitScope(XP_SCOPE_DEFAULT);
}

XP_STATUS xpsPortDbInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    portDbHndl = XPS_PORT_DB_HNDL;

    xpsLockCreate(XP_LOCKINDEX_XPS_PORT, 1);

    // Create PER_DEVICE Port Db
    if ((status = xpsStateRegisterDb(scopeId, "Port Db", XPS_PER_DEVICE,
                                     &portKeyCompare, portDbHndl)) != XP_NO_ERR)
    {
        portDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        return status;
    }

    intfToPortDevHndl = XPS_INTF_TO_PORTDEV_HNDL;
    // Create a XPS_GLOBAL Port Interface to Port/Device Db
    if ((status = xpsStateRegisterDb(scopeId, "Intf to PortDevice Db", XPS_GLOBAL,
                                     &portDevKeyCompare, intfToPortDevHndl)) != XP_NO_ERR)
    {
        intfToPortDevHndl = XPS_STATE_INVALID_DB_HANDLE;
        return status;
    }

    return status;

    XPS_FUNC_EXIT_LOG();
}

XP_STATUS xpsPortDbDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return xpsPortDbDeInitScope(XP_SCOPE_DEFAULT);
}

XP_STATUS xpsPortDbDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;

    // Purge PER_DEVICE Port Db
    if ((status = xpsStateDeRegisterDb(scopeId, &portDbHndl)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateDeRegisterDb failed for portDbHndl");
        return status;
    }

    if ((status = xpsStateDeRegisterDb(scopeId, &intfToPortDevHndl)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsStateDeRegisterDb failed for intfToPortDevHndl");
        return status;
    }

    xpsLockDestroy(XP_LOCKINDEX_XPS_PORT);

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsPortCreateDbForDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsPortCreateDbForDevice);
    XP_STATUS status = XP_NO_ERR;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid device id");
        return XP_ERR_INVALID_DEV_ID;
    }

    status = xpsStateInsertDbForDevice(devId, portDbHndl, portKeyCompare);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "State insert failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsPortDeleteDbForDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsPortDeleteDbForDevice);
    XP_STATUS status = XP_NO_ERR;
    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid device id");
        return XP_ERR_INVALID_DEV_ID;
    }

    status = xpsStateDeleteDbForDevice(devId, portDbHndl);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "State delete failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsPortInit(xpsDevice_t devId, xpsPort_t portNum,
                      xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status       = XP_NO_ERR;
    xpsPortIntfMap_t *portInfo = NULL;
    xpsIntfPortMap_t *intfInfo = NULL;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid device id");
        return XP_ERR_INVALID_DEV_ID;
    }
    else if (portNum > XP_MAX_PHY_PORTS)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid port number");
        return XP_ERR_INVALID_PORTNUM;
    }

    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return status;
    }

    if (!(xpsInterfaceCreateScope(scopeId, XPS_PORT, intfId)))
    {
        if ((status = xpsStateHeapMalloc(sizeof(xpsPortIntfMap_t),
                                         (void **)&portInfo)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Memory allocation for port info failed");
            return status;
        }
        if (!portInfo)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
            status = XP_ERR_NULL_POINTER;
            return status;
        }
        portInfo->devId       =  devId;
        portInfo->portNum     =  portNum;
        portInfo->intfId      =  *intfId;

        if ((status = xpsStateHeapMalloc(sizeof(xpsIntfPortMap_t),
                                         (void **)&intfInfo)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Memory allocation for interface info failed");
            xpsStateHeapFree((void *)portInfo);
            return status;
        }
        if (!intfInfo)
        {
            status = XP_ERR_NULL_POINTER;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
            xpsStateHeapFree((void *)portInfo);
            return status;
        }
        intfInfo->devId       =  devId;
        intfInfo->portNum     =  portNum;
        intfInfo->intfId      =  *intfId;

        // Insert the portNum,deviceId info structure into the database, using the portNum as a key
        if ((status = xpsStateInsertDataForDevice(devId, portDbHndl,
                                                  (void *)portInfo)) != XP_NO_ERR)
        {
            // Free Allocated memory
            xpsStateHeapFree((void *)portInfo);
            xpsStateHeapFree((void *)intfInfo);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "State insert call failed");
            return status;
        }

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "intfId = %d - devId = %d, portNum = %d\n", *intfId, devId, portNum);
        // Insert the same data into the other DB for cross referencing. Assumption here is these VIFs will never be released
        if ((status = xpsStateInsertData(scopeId, intfToPortDevHndl,
                                         (void*)intfInfo)) != XP_NO_ERR)
        {
            // Free Allocated Memory
            xpsStateHeapFree((void*)intfInfo);
            xpsStateHeapFree((void *)portInfo);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "State insert failed");
            return status;
        }
        return status;
    }

    XPS_FUNC_EXIT_LOG();
    return status;

}


XP_STATUS xpsPortGetPortIntfId(xpsDevice_t devId, xpsPort_t portNum,
                               xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsPortGetPortIntfId);

    XP_STATUS status = XP_NO_ERR;
    xpsPortIntfMap_t portInfo;
    xpsPortIntfMap_t *info = NULL;

    portInfo.portNum     =  portNum;
    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid device id");
        return XP_ERR_INVALID_DEV_ID;
    }
    else if (portNum > XP_MAX_PHY_PORTS)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid port number");
        return XP_ERR_INVALID_PORTNUM;
    }

    if ((status = xpsStateSearchDataForDevice(devId, portDbHndl,
                                              (xpsDbKey_t) &portInfo, (void **)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "State search data failed");
        return status;
    }
    if (info != NULL)
    {
        *intfId = info->intfId;
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        status = XP_ERR_NULL_POINTER;
    }

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsPortGetCPUPortIntfId(xpsDevice_t devId, xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPortGetSCPUPortIntfId(xpsDevice_t devId, xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPortGetLoopback0PortIntfId(xpsDevice_t devId,
                                        xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPortGetLoopback1PortIntfId(xpsDevice_t devId,
                                        xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPortIsExist(xpsInterfaceId_t portIfId)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return xpsPortIsExistScope(XP_SCOPE_DEFAULT, portIfId);

}

XP_STATUS xpsPortIsExistScope(xpsScope_t scopeId, xpsInterfaceId_t portIfId)
{
    XPS_FUNC_ENTRY_LOG();

    xpsDevice_t devId;
    xpsPort_t portNum;

    XPS_FUNC_EXIT_LOG();

    return xpsPortGetDevAndPortNumFromIntfScope(scopeId, portIfId, &devId,
                                                &portNum);
}

XP_STATUS xpsPortGetDevAndPortNumFromIntf(xpsInterfaceId_t intfId,
                                          xpsDevice_t *devId, xpsPort_t *portNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsPortGetDevAndPortNumFromIntf);

    XPS_FUNC_EXIT_LOG();
    return xpsPortGetDevAndPortNumFromIntfScope(XP_SCOPE_DEFAULT, intfId, devId,
                                                portNum);
}

XP_STATUS xpsPortGetDevAndPortNumFromIntfScope(xpsScope_t scopeId,
                                               xpsInterfaceId_t intfId, xpsDevice_t *devId, xpsPort_t *portNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsPortGetDevAndPortNumFromIntfScope);

    XP_STATUS status = XP_NO_ERR;
    xpsIntfPortMap_t intfInfo;
    xpsIntfPortMap_t *info = NULL;
    xpsInterfaceType_e intfType;

    if ((status = xpsInterfaceGetTypeScope(scopeId, intfId,
                                           &intfType)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interfaceType for interface(%d) failed:%d", intfId, status);
        return status;
    }

    intfId = (intfType == XPS_BRIDGE_PORT) ? XPS_INTF_MAP_BRIDGE_PORT_TO_INTF(
                 intfId) : intfId;
    intfInfo.intfId = intfId;

    if ((status = xpsStateSearchData(scopeId, intfToPortDevHndl,
                                     (xpsDbKey_t)&intfInfo, (void **)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "State search data failed");
        return status;
    }
    if (info != NULL)
    {
        *devId   = info->devId;
        *portNum = info->portNum;
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        status = XP_ERR_NULL_POINTER;
    }

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsPortSetField(xpsDevice_t devId, xpsInterfaceId_t portIfId,
                          xpsPortConfigFieldList_t fNum, uint32_t fData)
{
    XPS_FUNC_ENTRY_LOG();
    GT_STATUS rc = GT_OK;
    XP_STATUS ret = XP_NO_ERR;
    GT_U8   cpssDevId;
    GT_U32  cpssPortNum;

    cpssDevId = xpsGlobalIdToDevId(devId, portIfId);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portIfId);
    switch (fNum)
    {
        case XPS_PORT_PVID:
            rc = cpssDxChBrgVlanPortVidSet(cpssDevId, cpssPortNum, CPSS_DIRECTION_INGRESS_E,
                                           (GT_U16)fData);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Set pvid failed for portNum(%d)", portIfId);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
            break;

        case XPS_PORT_MAC_SAMISSCMD:
            ret = xpsPortSetFdbLearnMode(cpssDevId, cpssPortNum, fData);
            if (ret != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to configure FDB Learn mode for cpssport %d - portIfId %d",
                      cpssPortNum, portIfId);
                return ret;
            }
            break;

        default:
            break;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsPortGetField(xpsDevice_t devId, xpsInterfaceId_t portIfId,
                          xpsPortConfigFieldList_t fNum, uint32_t *fData)
{
    XPS_FUNC_ENTRY_LOG();
    GT_STATUS rc = GT_OK;
    XP_STATUS ret = XP_NO_ERR;
    GT_U8   cpssDevId;
    GT_U32  cpssPortNum;

    cpssDevId = xpsGlobalIdToDevId(devId, portIfId);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portIfId);
    switch (fNum)
    {
        case XPS_PORT_PVID:
            rc = cpssDxChBrgVlanPortVidGet(cpssDevId, cpssPortNum, CPSS_DIRECTION_INGRESS_E,
                                           (GT_U16 *)fData);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get pvid failed for portNum(%d)", portIfId);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
            break;

        case XPS_PORT_MAC_SAMISSCMD:
            ret = xpsPortGetFdbLearnMode(cpssDevId, cpssPortNum, fData);
            if (ret != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get FDB Learn mode for cpssport %d - portIfId %d",
                      cpssPortNum, portIfId);
                return ret;
            }
            break;

        default:
            break;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

//Port config APIs
XP_STATUS xpsPortSetConfig(xpsDevice_t devId, xpsInterfaceId_t portIfId,
                           xpsPortConfig_t *pConfig)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPortGetConfig(xpsDevice_t devId, xpsInterfaceId_t portIfId,
                           xpsPortConfig_t *pConfig)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPortSetPolicingEnable(xpsDevice_t devId, xpsInterfaceId_t port,
                                   xpsPolicerType_e policerType, bool enable)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS rc = GT_OK;

    if (policerType == XP_INGRESS_PORT_POLICER)
    {
        rc = cpssHalPolicerPortMeteringEnableSet(devId,
                                                 CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, port, (GT_BOOL)enable);
    }
    else if (policerType == XP_EGRESS_PORT_POLICER)
    {
        rc = cpssHalPolicerPortMeteringEnableSet(devId,
                                                 CPSS_DXCH_POLICER_STAGE_EGRESS_E, port, (GT_BOOL)enable);
    }
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalPolicerPortMeteringEnableSet rc %d, stage %d port %d\n",
              rc, policerType, port);
        xpsConvertCpssStatusToXPStatus(rc);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPortEnableMirroring(xpsInterfaceId_t portIfId,
                                 uint32_t analyzerId,
                                 bool ingress, bool isErspan, bool enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsPortEnableMirroring);

    XP_STATUS ret = XP_NO_ERR;

    ret = xpsPortEnableMirroringScope(XP_SCOPE_DEFAULT, portIfId,
                                      analyzerId,
                                      ingress, isErspan, enable);

    XPS_FUNC_EXIT_LOG();

    return ret;
}

XP_STATUS xpsPortEnableMirroringScope(xpsScope_t scopeId,
                                      xpsInterfaceId_t portIfId, uint32_t analyzerId,
                                      bool ingress, bool isErspan, bool enable)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS      ret       = XP_NO_ERR;
    xpsDevice_t    portDevId = 0;
    xpsPort_t      portId    = 0;

    if ((ret = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portIfId, &portDevId,
                                                    &portId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get port Id failed with error code %d\n", ret);
        return ret;
    }

    ret = cpssHalPortMirrorEnable(portDevId, portId, analyzerId, ingress, enable);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to enable mirror on port %d with error code %d\n", portDevId, ret);
        return ret;
    }

    if (!ingress && isErspan)
    {
        ret = xpsAclErspanRuleUpdate(portDevId, portId, enable, analyzerId);
        if (ret != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to enable mirror on port %d with error code %d\n", portDevId, ret);
            return ret;
        }
    }

    if (isErspan)
    {
        ret = xpsMirrorUpdateErSpan2Index(portDevId, portIfId,
                                          ingress, enable);
        if (ret != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsMirrorUpdateErSpan2Index Failed port %d ret %d\n", portIfId, ret);
            return (ret);
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPortGetFirst(xpsInterfaceId_t *portIfId)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();
    return xpsPortGetFirstScope(XP_SCOPE_DEFAULT, portIfId);
}

XP_STATUS xpsPortGetFirstScope(xpsScope_t scopeId, xpsInterfaceId_t *portIfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsPortGetFirst);

    XP_STATUS status = XP_NO_ERR;
    xpsIntfPortMap_t *info = NULL;
    xpsDevice_t devId = 0;
    uint32_t portNum = 0;
    xpsIntfPortMap_t intfInfo;
    /* Parameter validation */
    if (portIfId == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }


    /* Get first {IntfId -> port,deviceID} entry from the interfaceToPortDb */
    if ((status = xpsStateGetNextData(scopeId, intfToPortDevHndl, (xpsDbKey_t)NULL,
                                      (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get first InterfaceId entry from the interfaceToPortDb failed");
        return status;
    }
    /* Interface Info found ? */
    if (info == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Interface info not found");
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    while (info)
    {
        *portIfId = info->intfId;
        portNum  = info->portNum;
        devId    = info->devId;
        status = xpsLinkManagerIsPortNumValid(devId, portNum);
        if (status == XP_NO_ERR)
        {
            break;
        }

        intfInfo.intfId = *portIfId;
        if ((status = xpsStateGetNextData(scopeId, intfToPortDevHndl,
                                          (xpsDbKey_t)&intfInfo, (void**)&info)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get InterfaceId entry from the interfaceToPortDb failed");
            return status;
        }
    }

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsPortGetNext(xpsInterfaceId_t portIfId,
                         xpsInterfaceId_t *portIfIdNext)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsPortGetNext);

    XPS_FUNC_EXIT_LOG();
    return xpsPortGetNextScope(XP_SCOPE_DEFAULT, portIfId, portIfIdNext);

}

XP_STATUS xpsPortGetNextScope(xpsScope_t scopeId, xpsInterfaceId_t portIfId,
                              xpsInterfaceId_t *portIfIdNext)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;
    xpsIntfPortMap_t intfInfo;
    xpsIntfPortMap_t *info = NULL;
    xpsDevice_t devId = 0;
    uint32_t portNum = 0;
    intfInfo.intfId = portIfId;
    /* Parameter validation */
    if (portIfIdNext == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }

    /* get next {IntfId -> port,deviceID} entry from the interfaceToPortDb */
    if ((status = xpsStateGetNextData(scopeId, intfToPortDevHndl,
                                      (xpsDbKey_t)&intfInfo, (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get next {port,deviceID} entry from the interfaceToPortDb failed");
        return status;
    }

    /* Interface Info found ?*/
    if (info == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "Interface %u  info does not exist", portIfId);
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    while (info)
    {
        *portIfIdNext = info->intfId;
        portNum  = info->portNum;
        devId    = info->devId;
        status = xpsLinkManagerIsPortNumValid(devId, portNum);
        if (status == XP_NO_ERR)
        {
            break;
        }
        intfInfo.intfId = *portIfIdNext;
        if ((status = xpsStateGetNextData(scopeId, intfToPortDevHndl,
                                          (xpsDbKey_t)&intfInfo, (void**)&info)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get InterfaceId entry from the interfaceToPortDb failed");
            return status;
        }
    }

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsPortSetRandomHash(xpsDevice_t devId, xpsPort_t port,
                               uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPortGetRandomHash(xpsDevice_t devId, xpsPort_t port,
                               uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPortSetStaticHash(xpsDevice_t devId, xpsPort_t port,
                               xpHashType_e hashType, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPortGetStaticHash(xpsDevice_t devId, xpsPort_t port,
                               xpHashType_e hashType, uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsPortKeepEtag(xpsDevice_t devId, xpsInterfaceId_t portIfId,
                          uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPortAddEtag(xpsDevice_t devId, xpsInterfaceId_t portIfId,
                         uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPortEnableLongEtagMode(xpsDevice_t devId,
                                    xpsInterfaceId_t portIfId, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPortBridgePortSetMacSaMissCmd(xpsDevice_t devId,
                                           xpsInterfaceId_t bridgePortId, xpsPktCmd_e saMissCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPortGetPortIntfCount(xpsScope_t scopeId, uint32_t *count)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsPortGetPortIntfCount);

    XP_STATUS result = XP_NO_ERR;

    result = xpsStateGetCount(scopeId, intfToPortDevHndl, count);
    XPS_FUNC_EXIT_LOG();
    return result;
}

/**
 * \brief Get device maximum port number
 *
 * \param [in]  devId       device id
 * \param [out] maxPortNum  maximum port number
 *
 * \return XP_N0_ERR if there is no error
 */
XP_STATUS xpsPortGetMaxNum(xpsDevice_t devId, uint32_t *maxPortNum)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid device id");
        return XP_ERR_INVALID_DEV_ID;
    }

    if (maxPortNum == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }


    status = cpssHalGetMaxGlobalPorts((int*) maxPortNum);
    XPS_FUNC_EXIT_LOG();
    return status;
}

/**
 * \brief Dump port info
 *
 * \param [in] devId    device id
 * \param [in] portNum  port number
 * \param [in] detail
 * \return XP_N0_ERR if there is no error
 */
XP_STATUS xpsPortDump(xpsDevice_t devId, xpsInterfaceId_t portNum, bool detail)
{
    XPS_FUNC_ENTRY_LOG();
    cpssHalDumputilsPortDump(devId, portNum, detail);
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

/**
 * \brief Dump port info
 *
 * \param [in] devId    device id
 * \param [in] portNum  port number
 * \param [in] index
 * \param [out] channelSwapNum
 * \return XP_N0_ERR if there is no error
 */
XP_STATUS xpsLinkManagerPlatformGetSerdesLaneSwapInfo(xpsDevice_t devId,
                                                      uint32_t portNum, uint32_t index,
                                                      uint32_t* channelSwapNum)   //TODO revisit */ as TX swap info is used and Rx swap info is ignored
{
    uint32_t*           rxSerdesLaneArr;
    uint32_t*           txSerdesLaneArr;
    uint32_t            rxArr[8]; //CPSS_DXCH_PORT_XG_PSC_LANES_NUM_CNS=4
    uint32_t            txArr[8];
    XP_STATUS ret = XP_NO_ERR;
    rxSerdesLaneArr = rxArr;
    txSerdesLaneArr = txArr;

    if ((ret = cpssHalPortPlatformGetSerdesLaneSwapInfo(devId, portNum,
                                                        rxSerdesLaneArr, txSerdesLaneArr)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get serdes lane swap info failed");
        return ret;
    }

    *channelSwapNum = txSerdesLaneArr[index];
    return ret;
}

/**
 * \brief This method returns Start Port number
 */

uint32_t xpsGetGlobalStartPort(void)
{
    return CPSS_GLOBAL_START_PORT;
}

/**
 * \brief Get per device maximum port number
 *
 * \param [in]  devId       device id
 * \param [out] maxPortNum  maximum port number per device
 *
 * \return XP_N0_ERR if there is no error
 */
XP_STATUS xpsPortGetPerDeviceMaxNum(xpsDevice_t devId, uint8_t *maxPortNum)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid device id");
        return XP_ERR_INVALID_DEV_ID;
    }

    if (maxPortNum == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }

    status = cpssHalGetMaxPorts((int) devId, (int*) maxPortNum);

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsPortSAMacBaseSet(xpsDevice_t devId, macAddr_t mac)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    GT_ETHERADDR devMac;

    memcpy(devMac.arEther, mac, sizeof(devMac.arEther));

    status = cpssHalPortMacSaBaseSet(devId, &devMac);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalPortMacSaBaseSet Failed (%d)\n", status);
        return status;
    }
    return status;

}

XP_STATUS xpsPortSAMacLsbSet(xpsDevice_t devId, uint32_t portNum,
                             uint8_t macLsb)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    status = cpssHalPortMacSaLsbSet(devId, portNum, macLsb);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalPortMacSaLsbSet Port(%d) Failed (%d)\n", portNum, status);
        return status;
    }

    return status;
}

XP_STATUS xpsPortUpdateTunnelMtu(xpsDevice_t devId, xpsInterfaceId_t portIntf)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    uint32_t swL3IntfId = INVALID_L3_INDEX;
    xpsScope_t scopeId;
    xpsInterfaceInfo_t *l3IntfInfo = NULL;
    xpsInterfaceInfo_t *portIntfInfo = NULL;
    xpsInterfaceChildInfo_t *nextInfo = NULL;
    xpsInterfaceChildInfo_t *nextPortChildInfo = NULL;
    xpsIpTnlGblDbEntry_t *lookupEntry = NULL;

    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    status = xpsInterfaceGetInfoScope(scopeId, portIntf, &portIntfInfo);
    if ((status != XP_NO_ERR) || (portIntfInfo == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface portInfo : %d\n", portIntf);
        return status;
    }
    status = xpsL3RetrieveInterfaceMapping(scopeId, portIntf, &swL3IntfId);
    if (status == XP_NO_ERR)
    {
        status = xpsInterfaceGetInfoScope(scopeId, swL3IntfId, &l3IntfInfo);
        if ((status != XP_NO_ERR) || (l3IntfInfo == NULL))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                  "L3 not mapped : %d\n", portIntf);
        }

        while (xpsIntfChildGetNext(l3IntfInfo, &nextInfo) == XP_NO_ERR)
        {
            /* For now only Vxlan tunnels have the ePort support.*/
            status = xpsIpTunnelGblGetDbEntry(scopeId, nextInfo->keyIntfId, &lookupEntry);
            if ((status != XP_NO_ERR) || (lookupEntry == NULL))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Vxlan Tunnel Not found :%d \n", nextInfo->keyIntfId);
                return status;
            }

            for (uint32_t i= 0; i < lookupEntry->numOfEports; i++)
            {
                rc = cpssHalTunnelEPorTMtuProfileSet(devId, portIntf,
                                                     lookupEntry->ePorts[i].ePort);
                if (rc != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "EPorTMtuProfileSet Port(%d) Failed (%d)\n", portIntf, rc);
                    xpsConvertCpssStatusToXPStatus(rc);
                }
            }

            rc = cpssHalTunnelEPorTMtuProfileSet(devId, portIntf,
                                                 lookupEntry->primaryEport);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed cpssHalTunnelEPorTMtuProfileSet %d\n", rc);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }

        return XP_NO_ERR;
    }

    while (xpsIntfChildGetNext(portIntfInfo, &nextPortChildInfo) == XP_NO_ERR)
    {
        if (nextPortChildInfo->type == XPS_LAG)
        {
            status = xpsL3RetrieveInterfaceMapping(scopeId, nextPortChildInfo->keyIntfId,
                                                   &swL3IntfId);
            if (status == XP_ERR_INVALID_PARAMS)
            {
                continue;
            }
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed lag to l3 map %d\n", nextPortChildInfo->keyIntfId);
                return status;
            }
        }
        else if (nextPortChildInfo->type == XPS_VLAN)
        {
            swL3IntfId = XPS_INTF_MAP_BD_TO_INTF(nextPortChildInfo->keyIntfId);
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Invalid L2 type : %d\n", nextPortChildInfo->type);
            return XP_ERR_INVALID_INTERFACE;
        }
        status = xpsInterfaceGetInfoScope(scopeId, swL3IntfId, &l3IntfInfo);
        if ((status == XP_NO_ERR) && (l3IntfInfo == NULL))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                  "No L3 mapped : %d\n", swL3IntfId);
            continue;
        }

        while (xpsIntfChildGetNext(l3IntfInfo, &nextInfo) == XP_NO_ERR)
        {
            status = xpsIpTunnelGblGetDbEntry(scopeId, nextInfo->keyIntfId, &lookupEntry);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Vxlan Tunnel Not found :%d \n", nextInfo->keyIntfId);
                return status;
            }

            for (uint32_t i= 0; i < lookupEntry->numOfEports; i++)
            {
                rc = cpssHalTunnelEPorTMtuProfileSet(devId, portIntf,
                                                     lookupEntry->ePorts[i].ePort);
                if (rc != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "EPorTMtuProfileSet Port(%d) Failed (%d)\n", portIntf, rc);
                    xpsConvertCpssStatusToXPStatus(rc);
                }
            }

            rc = cpssHalTunnelEPorTMtuProfileSet(devId, portIntf,
                                                 lookupEntry->primaryEport);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed cpssHalTunnelEPorTMtuProfileSet %d\n", rc);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }
    }

    return XP_NO_ERR;
}

void xpsPortFecParamSet(xpsDevice_t devId, xpsPort_t port, uint32_t fecMode)
{
    if (cpssHalIsEbofDevice(devId))
    {
        xpFecMode mode = MAX_FEC_MODE;
        bool valid = 1;
        if (fecMode <= MAX_FEC_MODE)
        {
            switch (fecMode)
            {
                case RS_FEC_MODE:
                    mode = RS_FEC_MODE;
                    break;
                case FC_FEC_MODE:
                    mode = FC_FEC_MODE;
                    break;
                case RS_544_514_FEC_MODE:
                    mode = RS_544_514_FEC_MODE;
                    break;
                case MAX_FEC_MODE:
                    mode = MAX_FEC_MODE;
                    break;
            }
        }
        else
        {
            valid = 0;
        }

        cpssHalPortFecParamSet(devId, port, mode, valid);
    }
}


XP_STATUS xpsPortSetFdbLearnMode(GT_U8 devId, GT_U32 port,
                                 uint32_t learnMode)
{
    XPS_FUNC_ENTRY_LOG();
    GT_U32  unkSrcCmd           = 0;
    GT_BOOL naMsgToCpuEnable    = GT_FALSE;
    GT_STATUS rc                = GT_OK;

    if (learnMode == XP_PKTCMD_DROP)
    {
        unkSrcCmd           = CPSS_LOCK_SOFT_DROP_E;
        naMsgToCpuEnable    = GT_FALSE;
    }
    else if (learnMode == XP_PKTCMD_FWD)
    {
        unkSrcCmd           = CPSS_LOCK_FRWRD_E;
        naMsgToCpuEnable    = GT_FALSE;
    }
    else if (learnMode == XP_PKTCMD_TRAP)
    {
        unkSrcCmd           = CPSS_LOCK_TRAP_E;
        naMsgToCpuEnable    = GT_FALSE;
    }
    else if (learnMode == XP_PKTCMD_FWD_MIRROR)
    {
        unkSrcCmd           = CPSS_LOCK_MIRROR_E;
        naMsgToCpuEnable    = GT_FALSE;
    }
    else
    {
        /*
           TODO: WA to set ports naMsgToCpu to false as part of default bridge port creation.
           HW/HW-notify learn mode will hit here.
           naMsgToCpu must be set based on port is part of vlan ?
        */
        unkSrcCmd           = CPSS_LOCK_FRWRD_E;
        naMsgToCpuEnable    = GT_TRUE;
    }

    rc = cpssHalSetBrgFdbPortLearning(devId, port, unkSrcCmd, naMsgToCpuEnable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalSetBrgFdbPortLearning rc %d, port %d learnMode %d\n",
              rc, port, learnMode);
    }

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(rc);
}

XP_STATUS xpsPortGetFdbLearnMode(GT_U8 devId, GT_U32 port,
                                 uint32_t *learnMode)
{
    XPS_FUNC_ENTRY_LOG();
    GT_U32  unkSrcCmd           = 0;
    GT_BOOL naMsgToCpuEnable    = GT_FALSE;
    GT_STATUS rc                = GT_OK;

    if (learnMode == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsPortSetFdbLearnMode - learnMode is NULL\n");
        return XP_ERR_NULL_POINTER;
    }

    rc = cpssHalGetBrgFdbPortLearning(devId, port, &unkSrcCmd, &naMsgToCpuEnable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalSetBrgFdbPortLearning rc %d, port %d learnMode %d\n",
              rc, port, learnMode);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    if (naMsgToCpuEnable == GT_FALSE)
    {
        if (unkSrcCmd == CPSS_LOCK_DROP_E)
        {
            *learnMode = XP_PKTCMD_DROP;
        }
        else if (unkSrcCmd == CPSS_LOCK_TRAP_E)
        {
            *learnMode = XP_PKTCMD_TRAP;
        }
        else if (unkSrcCmd == CPSS_LOCK_FRWRD_E)
        {
            *learnMode = XP_PKTCMD_FWD;
        }
    }
    else
    {
        if (unkSrcCmd == CPSS_LOCK_FRWRD_E)
        {
            *learnMode = XP_PKTCMD_MAX;
        }
        else
        {
            *learnMode = XP_PKTCMD_FWD_MIRROR;
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


#ifdef __cplusplus
}
#endif
