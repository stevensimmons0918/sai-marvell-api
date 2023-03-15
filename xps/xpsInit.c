// xpsInit.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsInit.h"
#include "xpsIpinIp.h"
#include "xpsPort.h"
#include "xpsAcl.h"
#include "xpsLink.h"
#include "xpsTunnel.h"
#include "xpsLag.h"
#include "xpsFdb.h"
#include "xpsMpls.h"
#include "xpsAcm.h"
#include "xpsSflow.h"
#include "xpsMirror.h"
#include "xpsQos.h"
#include "xpsCopp.h"
#include "xpsPolicer.h"
#include "xpsMulticast.h"
#include "xpsOpenflow.h"
#include "xpsEgressFilter.h"
#include "xpsMtuProfile.h"
#include "xpsSr.h"
#include "xpsGlobalSwitchControl.h"
#include "xpsPacketDrv.h"
#include "xpsScope.h"
#include "xpsStp.h"
#include "xpsLock.h"
#include "xpsInt.h"
#include "xpsAllocator.h"
#include "xpsNhGrp.h"
#include "xpsVxlan.h"
#include "xpsMac.h"
#include "cpssHalUtil.h"
#include "cpssHalDev.h"
#include "cpssHalPlatform.h"
#include "cpssHalDevice.h"
#include "cpssHalQos.h"
#include "xpsNat.h"
/*MARVELL BEGIN*/
#include "cpss/generic/log/cpssLog.h"
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <gtOs/gtOsMem.h>
#include "cpssHalInit.h"
#include "cpssHalShell.h"
#include "cpssHalDevice.h"
extern xpTimeStamp_t xpTimeStamp[XP_MAX_FUNCTION];
#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <stdlib.h>
#include <unistd.h>
extern GT_STATUS cpssHalPortMgrEventRegister
(
    GT_U8                   devId
);


#include "cpssHalCascadeMgr.h"
/* TODO: Cleanup CPSS Externs */
#define XPS_INIT_API_RETURN(_text, _status) do { if ( GT_OK != _status)  return cpssHalApiReturn(__func__, __LINE__, _text, _status); } while(0)
GT_STATUS cpssHalSimInit(PROFILE_STC profile[]);
PROFILE_STC *profile = NULL;
GT_STATUS xpsCpssInit(XP_DEV_TYPE_T devType);
/*MARVELL END*/


/*Holds CPSS LPM_FDB_EM profile index.*/
static int gSharedProfId;

void SetSharedProfileIdx(int idx)
{
    gSharedProfId = idx;
}

int GetSharedProfileIdx()
{
    return gSharedProfId;
}

/*Holds max IPv4 route Number.*/
static int gRouteIpv4MaxNum;

void SetRouteIpv4MaxNum(int num)
{
    gRouteIpv4MaxNum = num;
}

int GetRouteIpv4MaxNum()
{
    return gRouteIpv4MaxNum;
}

/*Holds max IPv6 route Number.*/
static int gRouteIpv6MaxNum;

void SetRouteIpv6MaxNum(int num)
{
    gRouteIpv6MaxNum = num;
}

int GetRouteIpv6MaxNum()
{
    return gRouteIpv6MaxNum;
}

/*Holds CPSS PBR max Number.*/
static int gPbrMaxNum;

void SetPbrMaxNum(int num)
{
    gPbrMaxNum = num;
}

int GetPbrMaxNum()
{
    return gPbrMaxNum;
}

/*Holds L3 HW ID range-base that supports L3 counters*/
static int gL3CounterIdBase[5] = {-1, -1, -1, -1, -1};

void SetL3CounterIdBase(int idx, int pos)
{
    if (pos < 5)
    {
        gL3CounterIdBase[pos] = idx;
    }
}

int IsValidCounterIdBase(int pos, XP_DEV_TYPE_T  devType)
{
    /* M0 and Falcon (profiles with >25G ports) - first pos is valid
           Eg: l3_counter_index_base=0; l3_counter_index_base=4K
       Falcon10G/25G profile - first 2 pos are valid.
           Eg: l3_counter_index_base=0 3K; l3_counter_index_base=1K 4K
       Aldrin2XL- first five pos are valid.
           Eg: l3_counter_index_base=0 3K;
               l3_counter_index_base=0 1K 2K 3K 4K
     */
    if ((pos > 4) ||
        ((pos > 0) && (IS_DEVICE_AC3X(devType) || IS_DEVICE_AC5X(devType) ||
                       (IS_DEVICE_FALCON(devType) &&
                        !(IS_DEVICE_FALCON_10G_ONLY_PROFILE(devType) ||
                          IS_DEVICE_FALCON_25G_ONLY_PROFILE(devType) ||
                          IS_DEVICE_FALCON_10G_25G_ONLY_PROFILE(devType))))) ||
        ((pos > 1) && (IS_DEVICE_FALCON_10G_ONLY_PROFILE(devType) ||
                       IS_DEVICE_FALCON_25G_ONLY_PROFILE(devType) ||
                       IS_DEVICE_FALCON_10G_25G_ONLY_PROFILE(devType))))
    {
        return 0;
    }
    return 1;
}

int GetL3CounterIdBase(int pos)
{
    if (pos > 4)
    {
        return -1;
    }
    return gL3CounterIdBase[pos];
}

void xpsLinkStateNotify(void (*func)(xpsDevice_t, uint32_t, int))
{
    cpssHalLinkStateNotify = func;
}


XP_STATUS xpsSdkInit(XP_DEV_TYPE_T devType, xpsRangeProfileType_t rpType,
                     xpsInitType_t initType)
{
    cpssHalSetDeviceSwitchId(CPSS_GLOBAL_SWITCH_ID_0);
    return xpsSdkInitScope(devType, XP_SCOPE_DEFAULT, rpType, initType);
}

XP_STATUS xpsSdkInitScope(XP_DEV_TYPE_T devType, xpsScope_t scopeId,
                          xpsRangeProfileType_t rpType, xpsInitType_t initType)
{
    GT_STATUS rc = GT_OK;
    XP_STATUS status = XP_NO_ERR;
    int numberOfCscdPorts = 0;
    uint32_t cscdTrunkId = 0;
    XPS_FUNC_ENTRY_LOG();

    rc = xpsCpssInit(devType);
    status = xpsConvertCpssStatusToXPStatus(rc);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsCpssInit failed");
        return status;
    }

    if (scopeId == XP_SCOPE_DEFAULT)
    {
        xpsSetInitType(initType);

        status = xpsScopeInit(initType);
        if (status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsScopeInit failed");
            return status;
        }
    }
    if ((status=cpssHalGetDeviceType(scopeId, &devType)) != XP_NO_ERR)
    {
        cpssOsPrintf("Error <dev = %d>: xpSalGetDeviceType failed.\n", scopeId);
        return XP_ERR_INIT;
    }

    if ((status=cpssHalSetSKUmaxValues(scopeId)) != XP_NO_ERR)
    {
        cpssOsPrintf("Error <dev = %d>: cpssHalSetSKUmaxValues failed.\n", scopeId);
        return XP_ERR_INIT;
    }

    status = xpsSdkDevInitApi(scopeId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsSdkDevInitApi failed.\n");
        return status;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsScopeMgr initialized\n");

    status = xpsStateInit(scopeId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsStateInit failed");
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsStateMgr initialized\n");

    status = xpsInterfaceInitScope(scopeId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsInterfaceInitScope failed");
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsInterfaceInitScope  initialized\n");

    status = xpsMtuInitScope(scopeId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsMtuInitScope failed");
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsMtuInitScope  initialized\n");

    status = xpsEgressFilterInitScope(scopeId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsEgressFilterInitScope failed");
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpStpMgr initialized\n");

    status = xpsStpInitScope(scopeId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsStpInitScope failed");
        return status;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpStpMgr initialized\n");

    status = xpsVlanInitScope(scopeId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsVlanInitScope failed");
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsVlan initialized\n");

    status = xpsLagInitScope(scopeId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsLagInitScope failed");
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsLag initialized\n");

    //what about the handler functions?
    status = xpsFdbInitScope(scopeId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsFdbInitScope failed");
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsFdb initialized\n");

    status = xpsL3InitIpHostScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsL3IpHostInit initialized\n");

    status = xpsL3InitIpRouteScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsL3IpRouteInit initialized\n");

    status = xpsL3InitRouteNextHopGroupScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: Next Hop group initialized\n");

    status = xpsL3InitRouteNextHopScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: Next Hop initialized\n");

    status = xpsMulticastInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsMulticastMgr initialized\n");

    status = xpsMplsInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsMplsInitScope initialized\n");

    status = xpsAcmInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsAcmInitScope initialized\n");

    status = xpsSflowInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsInitSflow initialized\n");

    status = xpsAclInitScope(scopeId);
    if (status != XP_NO_ERR)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit:xpsAclInit initialized\n");

    if ((status = xpsPolicerInitScope(scopeId)) != XP_NO_ERR)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsPolicer Init initialized\n");


    status =  xpsMirrorInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsInitMirror initialized\n");


    status = xpsTunnelInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsInitTunnel initialized\n");

    if ((status = xpsQosInitScope(scopeId)) != XP_NO_ERR)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsQos Init initialized\n");

    if ((status = xpsCoppInitScope(scopeId)) != XP_NO_ERR)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsCopp Init initialized\n");

    if ((status = xpsIntInitScope(scopeId)) != XP_NO_ERR)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsInt Init initialized\n");

    status =  xpsVxlanInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsVxlanMirror initialized\n");

    status =  xpsNatInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsNat Init initialized\n");

    status = xpsLinkInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsLink initialized\n");

    status = xpsOpenflowInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsTcamMgr initialized\n");

    status = xpsSrInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsSr initialized\n");

    status = xpsIpinIpInit(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsIpinIp initialized\n");


    if ((status = xpsGlobalSwitchControlInitScope(scopeId)) != XP_NO_ERR)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkInit: xpsGlobalSwitchControlInit initialized\n");

    cpssHalGetMaxCascadedPorts(scopeId, &numberOfCscdPorts);
    if (numberOfCscdPorts > 0)
    {
        /* Reserve trunk-id 1 for cascade trunk */
        cscdTrunkId = xpsUtilCpssToXpsInterfaceConvert(CPSS_CSCD_TRUNK_ID, XPS_LAG);
        status = xpsAllocatorAllocateWithId(scopeId, XPS_ALLOCATOR_VIF_LAG,
                                            cscdTrunkId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "VIF allocation for LAG failed");
            return status;
        }

        /*do the cascade trunk init at the last*/
        rc = cpssHalCascadeMgrInitCascadeTrunk(CPSS_CSCD_TRUNK_ID);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssHalCascadeMgrInitCascadeTrunk Failed:  rc %d\n",  rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsProcessTerminationHandler()
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsSdkDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();
    XP_DEV_TYPE_T devType;
    cpssHalGetDeviceType(CPSS_GLOBAL_SWITCH_ID_0, &devType);
    XP_STATUS status = XP_NO_ERR;
    status = xpsSdkDeInitScope(XP_SCOPE_DEFAULT);
    if (status == XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "xpsSdkDeInitScope: deinited successfully\n");
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "xpsSdkDeInitScope: deinit failed %d\n", status);
    }
    /* For M0 platform, this is handled by mvDma/mvInt drv. */
    if (IS_DEVICE_FALCON(devType) || IS_DEVICE_AC5X(devType))
    {
        xpsResetSwitch(CPSS_GLOBAL_SWITCH_ID_0);
    }

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsSdkDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status =  xpsNatDeInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkDeInit: NAT cleanup done.\n");

    status = xpsAcmDeInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkDeInit: ACM cleanup done.\n");

    status = xpsMulticastDeInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkDeInit: multicast cleanup done.\n");

    status = xpsTunnelDeInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkDeInit: tunnel cleanup done.\n");

    status = xpsMplsDeInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkDeInit: mpls cleanup done.\n");

    status = xpsL3DeInitIpHostScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkDeInit: L3 Host cleanup done.\n");

    status = xpsL3DeInitIpRouteScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkDeInit: L3 Route cleanup done.\n");

    status = xpsL3DeInitRouteNextHopScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkDeInit: L3 Nh cleanup done.\n");

    status = xpsFdbDeInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE, "fdb cleanup done.\n");

    status = xpsVlanDeInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE, "vlan cleanup done.\n");

    status = xpsLagDeInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE, "lag cleanup done.\n");

    status = xpsStpDeInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE, "stp cleanup done.\n");

    status = xpsLinkDeInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE, "link cleanup done.\n");

    status = xpsMirrorDeInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkDeInit: mirror cleanup done.\n");

    status = xpsMtuDeInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE, "mtu cleanup done.\n");

    status = xpsInterfaceDeInitScope(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkDeInit: interface cleanup done.\n");

    if ((status = xpsQosDeInitScope(scopeId)) != XP_NO_ERR)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkDeInit: xpsQos cleanup done.\n");

    if ((status = xpsCoppDeInitScope(scopeId)) != XP_NO_ERR)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkDeInit: xpsCopp cleanup done.\n");

    if ((status = xpsIntDeInitScope(scopeId)) != XP_NO_ERR)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkDeInit: xpsInt cleanup done.\n");

    if ((status = xpsGlobalSwitchControlDeInitScope(scopeId)) != XP_NO_ERR)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsSdkDeInit: xpsGlobalSwitchControlDeInit cleanup done.\n");


    status = xpsStateDeInit(scopeId);
    if (status)
    {
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsStateDeInit cleanup done\n");

    if (scopeId == XP_SCOPE_DEFAULT)
    {
        status = xpsScopeDeInit();
        if (status)
        {
            return status;
        }
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "xpsSdkInit: xpsScopeMgr cleanup done.\n");
    }
    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsSdkDeviceInit(xpsDevice_t devId, xpsInitType_t initType,
                           XP_DAL_TYPE_T dalType, xpsRangeProfileType_t rtype, xpCoreClkFreq_t clkFreq,
                           xpPacketInterface packetInterface)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();
    return (xpsSdkDevInit(devId, initType, &devDefaultConfig));
}

XP_STATUS xpsSdkDeviceDefaultInit(xpsDevice_t devId, xpsInitType_t initType,
                                  xpSkuMode_e mode, xpPacketInterface packetInterface)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();
    return (xpsSdkDevInit(devId, initType, &devDefaultConfig));
}

XP_STATUS xpsSdkDefaultDevInit(xpsDevice_t devId, xpsInitType_t initType,
                               xpsDevConfigStruct_t *devConfig, xpPacketInterface packetInterface)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();
    return (xpsSdkDevInit(devId, initType, &devDefaultConfig));
}


XP_STATUS xpsSdkDevInitApi(xpsDevice_t devId)
{
    XP_STATUS status = XP_NO_ERR;

    status = xpsFdbInitApi(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsFdb Generic Init failed.\n");
        return status;
    }

    status = xpsL3InitApi(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsL3 Generic Init failed.\n");
        return status;
    }

    status = xpsMirrorInitApi(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsMirror Generic Init failed.\n");
        return status;
    }

    return status;
}


XP_STATUS xpsSdkDevInit(xpsDevice_t devId, xpsInitType_t initType,
                        xpDevConfigStruct *devConfigStruct)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status;
    XP_DEV_TYPE_T devType;
    xpsInterfaceId_t pIntfId;
    uint8_t numPorts = 0;

    if ((status=cpssHalGetDeviceType(devId, &devType)) != XP_NO_ERR)
    {
        cpssOsPrintf("Error <dev = %d>: xpSalGetDeviceType failed.\n", devId);
        return XP_ERR_INIT;
    }

    status = xpsSdkDevInitApi(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsSdkDevInitApi failed.\n");
        return status;
    }

    status = xpsStateAddDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsStateMgr addDevice failed.\n");
        return status;
    }

    status = xpsLinkAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error <dev = %d>: xpLinkManager initDevice failed.\n", devId);
        return status;
    }

    status = xpsInterfaceAddDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsInterfaceMgr addDevice failed.\n");
        return status;
    }

    //l2 domain manager: addDevice specific primitives
    status = xpsStpAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsStp addDevice failed.\n");
        return status;
    }

    //lag manager: addDevice specific primitives
    status = xpsLagAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpLagMgr addDevice failed.\n");
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> xpLagMgr initialized successfully!!!\n", devId);

    //tunnel manager: addDevice
    status = xpsTunnelAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpTunnelMgr addDevice failed.\n");
        return status;
    }

    status = xpsVlanAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpL2DomainMgr addDevice failed with error %d .\n", status);
        return status;
    }

    //fdb manager: addDevice specific primitives

    status = xpsFdbAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsFdb addDevice failed.\n");
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> vlan, stp and fdb initialized successfully!!!\n", devId);

    //ipv4 and ipv6 route manager: addDevice specific primitives
    status = xpsL3AddIpRouteDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsL3IpRoute addDevice failed.\n");
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> xpIpv4RouteMgr initialized successfully!!!\n", devId);

    status = xpsL3AddRouteNextHopDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpIpNhAddDevice addDevice failed.\n");
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> xpIpNhAddDevice initialized successfully!!!\n", devId);

    //ip host manager: addDevice specific primitives
    status = xpsL3AddIpHostDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpIpHostMgr addDevice failed.\n");
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> xpIpHostMgr (v4 and v6) initialized successfully!!!\n", devId);

    //mpls route manager: addDevice
    status = xpsMplsAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d> xpMplsRouteMgr add device failed !!!\n", devId);
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> MPLS route manager initialized successfully!!!\n", devId);

    //multicast manager: addDevice
    status = xpsMulticastAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xspMulticastMgr add device failed !!!\n", devId);
        return XP_ERR_INIT;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> XPS Multicast manager initialized successfully!!!\n", devId);

    //Segment route: addDevice
    status = xpsSrAddDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  XPS SR add device failed !!!\n", devId);
        return XP_ERR_INIT;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> XPS SR initialized successfully!!!\n", devId);


    //acm manager: addDevice
    status = xpsAcmAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xpAcmMgr add device failed !!!\n", devId);
        return status;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> Acm manager initialized successfully!!!\n", devId);

    //sflow manager: addDevice
    status = xpsSflowAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xpSflowMgr add device failed !!!\n", devId);
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> Sflow manager initialized successfully!!!\n", devId);

    //queue manager: addDevice
    if ((status = xpsQosAddDevice(devId, initType)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xpsQosAddDevice add device failed !!!\n", devId);
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> Queue manager initialized successfully!!!\n", devId);

    status = xpsCoppAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error <dev = %d>: xpCoppMgr add device failed!!!\n", devId);
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> xpCoppMgr initialized successfully!!!\n", devId);

    status = xpsIntAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error <dev = %d>: xpIntMgr add device failed!!!\n", devId);
        return status;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> xpCoppMgr initialized successfully!!!\n", devId);
    if ((status = xpsPolicerAddDevice(devId, initType)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xpsPolicerAddDevice add device failed !!!\n", devId);
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> Policer manager initialized successfully!!!\n", devId);

    status = xpsNatAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xpNatMgr add device failed !!!\n", devId);
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> NAT manager initialized successfully!!!\n", devId);

    status = xpsMirrorAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error <dev = %d>: mirrorMgr initDevice(devId) failed.\n", devId);
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> mirrorMgr initialized successfully!!!\n", devId);

    status = xpsGlobalSwitchControlAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xpsGlobalSwitchControlAddDevice add device failed !!!\n", devId);
        return status;
    }


    status = xpsL3RouteNextHopGroupAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error <dev = %d>: NH Group manager initDevice failed.\n", devId);
        return status;
    }

    if (initType == INIT_COLD)
    {
        status = xpsGlobalSwitchControlGetMaxPorts(devId, &numPorts);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to get max ports\n");
            return status;
        }

        uint32_t portId = 0;
        /*Antony, this loop is different from M0*/
        for (portId = 0; portId <(MAX_PORTNUM); portId++)
        {
            status = xpsPortInit(devId, portId, &pIntfId);
            if (status)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: xps port init failed.\n");
                return status;
            }
        }
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d> xpsPortInit done!!!\n", devId);
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "<dev = %d> xpLinkMgr initialized successfully!!!\n", devId);

    status = xpsMtuAddDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsMtuProfileMgr addDevice failed.\n");
        return status;
    }

    status = xpsAclAddDevice(devId, initType);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpAclMgr addDevice failed.\n");
        return status;
    }

    status = xpsVlanInitDefaultVlan(devId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Default Vlan Init Failed\n");
        return status;
    }

    status = xpsPacketDriverInit(devId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsPacketDriverInit  Failed\n");
        return status;
    }

    status = xpsPacketDriverNetdevInit(devId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Net Dev Library Init Failed\n");
        return status;
    }

    GT_STATUS rc = GT_OK;
    rc = cpssHalQosMapInit(devId);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalQosMapInit Failed (devId = 0x%x) done.\n", devId);
        status = xpsConvertCpssStatusToXPStatus(rc);
        return status;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsAppInitDevice (devId = 0x%x) done.\n", devId);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSdkDevDeInit(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    XP_DEV_TYPE_T devType;
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpAppDeInitDevice(devId = 0x%x)\n", devId);
    cpssOsPrintf("Deiniting dev %d", devId);

    if ((status=cpssHalGetDeviceType(devId, &devType)) != XP_NO_ERR)
    {
        cpssOsPrintf("Error <dev = %d>: xpSalGetDeviceType failed.\n", devId);
        return XP_ERR_INIT;
    }

    //link manager: removeDevice
    status = xpsLinkRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpLinkManager removeDevice failed.\n");
        return status;
    }

    //tunnel manager: removeDevice
    status = xpsTunnelRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpTunnelMgr removeDevice failed.\n");
        return status;
    }

    //fdb manager: removeDevice specific primitives
    status = xpsFdbRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpDeviceCleanup Fdb failed.\n");
        return status;
    }

    //l2 domain manager: removeDevice specific primitives
    status = xpsVlanRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpL2DomainMgr removeDevice failed.\n");
        return status;
    }

    status = xpsStpRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsStp remove Device failed.\n");
        return status;
    }

    status = xpsLagRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsLagRemoveDevice addDevice failed.\n");
        return status;
    }

    //ipv4 route manager: removeDevice specific primitives
    status = xpsL3RemoveIpRouteDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsL3IpRouteDeviceDeInit failed.\n");
        return status;
    }

    status = xpsL3RemoveRouteNextHopDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsL3RouteNextHopDeviceDeInit failed.\n");
        return status;
    }

    //ip host manager: removeDevice specific primitives
    status = xpsL3RemoveIpHostDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpIpHostMgr removeDevice failed.\n");
        return status;
    }

    //mpls route manager: removeDevice
    status = xpsMplsRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d> xpMplsRouteMgr remove device failed !!!\n", devId);
        return status;
    }

    //multicast manager: removeDevice
    status = xpsMulticastRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xpMulticastMgr remove device failed !!!\n", devId);
        return XP_ERR_INIT;
    }

    status = xpsAcmRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xpAcmMgr remove device failed !!!\n", devId);
        return status;
    }

    //sflow manager: removeDevice
    status = xpsSflowRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xpSflowMgr remove device failed !!!\n", devId);
        return status;
    }

    status = xpsPolicerRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xpsPolicerRemoveDevice remove device failed !!!\n", devId);
        return status;
    }

    //queue manager: removeDevice
    status = xpsQosRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xpsQosRemoveDevice remove device failed !!!\n", devId);
        return status;
    }

    if ((status = xpsCoppRemoveDevice(devId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xpsCoppRemoveDevice remove device failed !!!\n", devId);
        return status;
    }

    if ((status = xpsIntRemoveDevice(devId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xpsIntRemoveDevice remove device failed !!!\n", devId);
        return status;
    }
    status = xpsNatRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xps nat Mgr remove device failed !!!\n", devId);
        return status;
    }

    status = xpsMirrorRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error <dev = %d>: mirrorMgr->initDevice(devId) failed.\n", devId);
        return status;
    }

    status = xpsMtuRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsMtuProfileMgr removeDevice failed.\n");
        return status;
    }

    status = xpsL3RouteNextHopGroupRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: NH group manager removeDevice failed.\n");
        return status;
    }

    if ((status = xpsGlobalSwitchControlRemoveDevice(devId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "<dev = %d>  xpsGlobalSwitchControlRemoveDevice remove device failed !!!\n",
              devId);
        return status;
    }

    //interface manager: removeDevice
    status = xpsInterfaceRemoveDevice(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpInterfaceMgr removeDevice failed.\n");
        return status;
    }

    status = xpsPacketDriverRxDeInit(devId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsPacketDriverRxDeInit : %d \n", status);
        return status;
    }

    status = xpsPacketDriverNetdevDeInit(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsPacketDriverNetdevDeInit failed.\n");
        return status;
    }

    status = xpsPacketDriverDeInit(devId);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsPacketDriverDeInit failed.\n");
        return status;
    }

    XPS_FUNC_EXIT_LOG();
    cpssOsPrintf("Deinit successful for dev %d", devId);
    return XP_NO_ERR;
}

XP_STATUS xpsSdkDevLoadProfile(xpsDevice_t devId,
                               xpsDevProfileType_t profileType)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

const char **xpsSdkLoggerInit(void)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return NULL;
}

XP_STATUS xpsSdkSetLoggerOptions(uint32_t id, char *param)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSdkLogConsole(void)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSdkDisableLogConsole(void)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSdkLogToFile(const char* logFile)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSdkSetModuleLogLevel(char* moduleName, char* logLevel)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSdkEnableOrDisableLogModule(char* moduleName, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSdkThreadLogRegister(char* moduleName, uint32_t threadMgmtId,
                                  uint8_t mask)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSdkThreadLogStatusSet(char* moduleName, uint32_t threadMgmtId,
                                   uint8_t mask)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSdkThreadLogStatusGet(char* moduleName, uint32_t threadMgmtId,
                                   uint8_t* mask)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSdkLogVersion(void)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsTransFifoRdrBegin(xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsTransFifoRdrEnd(xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

void xpsSetInitType(xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();
    return;
}

xpsInitType_t xpsGetInitType()
{

    return INIT_COLD;
}

uint8_t xpsIsDevInitDone(xpDevice_t devId)
{
    return 1;
}

void xpsSetSalType(xpsSalType_e salType)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();
    return;
}

int xpsSalDefaultInit(void)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return 0;
}

int xpsSalInit(XP_DEV_TYPE_T devType)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return 0;
}

XP_STATUS xpsServiceCpuEnable(xpDevice_t devId, bool enable)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsServiceCpuLoadFirmware(xpDevice_t devId, const char* scpuFwPath)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

void xpsPortMappingDump(int devId)
{
    int maxTotalPorts = 0;
    int portNum = 0;

    cpssHalGetMaxGlobalPorts(&maxTotalPorts);
    cpssOsPrintf("Global maxTotalPorts = %d\n", maxTotalPorts);

    XPS_GLOBAL_PORT_ITER(portNum, maxTotalPorts)
    {
        int cpssDevNum, cpssPortNum;
        cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

        cpssOsPrintf("Global DevId %d Port %d  => Cpss DevId %d Port %d\n", devId,
                     portNum, cpssDevNum, cpssPortNum);
    }
}


/********************************************* MRVL INIT **************************************************/

PROFILE_STC * xpsPortConfigARR = NULL;

extern PROFILE_TYPE_HW_SKU_STC hw_sku_profiles_falcon[];

XP_STATUS xpsCreatePortMappingProfile(XP_DEV_TYPE_T devType, int numOfPorts)
{
    unsigned int numOfEntries = numOfPorts +
                                10; /* num of ports + cpu entries + ini file entry + delimiter */
    unsigned int idx = numOfPorts;
    xpsPortConfigARR = (PROFILE_STC *)xpMalloc(sizeof(PROFILE_STC)*numOfEntries);
    if (!xpsPortConfigARR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsCreatePortMappingProfile: failed to allocate memory %d\n", numOfEntries);
        return XP_ERR_FAILED;
    }

    memset(xpsPortConfigARR, 0, sizeof(PROFILE_STC)*numOfEntries);

    if (IS_DEVICE_FALCON_12_8(devType))
    {
        /* CPU ports for 12_8T */
        xpsPortConfigARR[idx].profileType = PROFILE_TYPE_CPU_PORT_MAP_E;
        xpsPortConfigARR[idx].profileValue.portMap.macNum = 276;
        xpsPortConfigARR[idx].profileValue.portMap.portNum = 63;
        xpsPortConfigARR[idx].profileValue.portMap.interfaceSpeed = 0;
        xpsPortConfigARR[idx].profileValue.portMap.interfaceMode =
            PROFILE_INTERFACE_MODE_KR_E;
        idx++;
        /* simulation 12_8T */
        xpsPortConfigARR[idx].profileType = PROFILE_TYPE_SIM_INIFILE_E;
        sprintf(xpsPortConfigARR[idx].profileValue.sim_inifile, "%s",
                "-i ./iniFiles/falcon_12_8_A0_wm.ini");
        idx++;
        xpsPortConfigARR[idx].profileType = PROFILE_TYPE_HW_SKU_E;
        xpsPortConfigARR[idx].profileValue.hwTableSizes = &hw_sku_profiles_falcon[2];
        idx++;
    }
    else if ((IS_DEVICE_FALCON_6_4(devType)) || (IS_DEVICE_FALCON_3_2(devType)))
    {
        /* CPU ports for 6_4T */
        xpsPortConfigARR[idx].profileType = PROFILE_TYPE_CPU_PORT_MAP_E;
        xpsPortConfigARR[idx].profileValue.portMap.macNum = 139;
        xpsPortConfigARR[idx].profileValue.portMap.portNum = 63;
        xpsPortConfigARR[idx].profileValue.portMap.interfaceSpeed = 0;
        xpsPortConfigARR[idx].profileValue.portMap.interfaceMode =
            PROFILE_INTERFACE_MODE_KR_E;
        idx++;
        xpsPortConfigARR[idx].profileType = PROFILE_TYPE_CPU_PORT_MAP_E;
        xpsPortConfigARR[idx].profileValue.portMap.macNum = 138;
        xpsPortConfigARR[idx].profileValue.portMap.portNum = 60;
        xpsPortConfigARR[idx].profileValue.portMap.interfaceSpeed = 0;
        xpsPortConfigARR[idx].profileValue.portMap.interfaceMode =
            PROFILE_INTERFACE_MODE_KR_E;
        idx++;
        xpsPortConfigARR[idx].profileType = PROFILE_TYPE_CPU_PORT_MAP_E;
        xpsPortConfigARR[idx].profileValue.portMap.macNum = 137;
        xpsPortConfigARR[idx].profileValue.portMap.portNum = 59;
        xpsPortConfigARR[idx].profileValue.portMap.interfaceSpeed = 0;
        xpsPortConfigARR[idx].profileValue.portMap.interfaceMode =
            PROFILE_INTERFACE_MODE_KR_E;
        idx++;
        xpsPortConfigARR[idx].profileType = PROFILE_TYPE_CPU_PORT_MAP_E;
        xpsPortConfigARR[idx].profileValue.portMap.macNum = 136;
        xpsPortConfigARR[idx].profileValue.portMap.portNum = 58;
        xpsPortConfigARR[idx].profileValue.portMap.interfaceSpeed = 0;
        xpsPortConfigARR[idx].profileValue.portMap.interfaceMode =
            PROFILE_INTERFACE_MODE_KR_E;
        idx++;

        /* simulation 6_4T */
        xpsPortConfigARR[idx].profileType = PROFILE_TYPE_SIM_INIFILE_E;
        if (!IS_DEVICE_EBOF_PROFILE(devType))
        {
            sprintf(xpsPortConfigARR[idx].profileValue.sim_inifile, "%s",
                    "-i ./iniFiles/falcon_6_4_A0_wm.ini");
        }
        else
        {
            sprintf(xpsPortConfigARR[idx].profileValue.sim_inifile, "%s",
                    "-i ./iniFiles/falcon_6_4_A0_ebof_wm.ini");
        }

        idx++;
        xpsPortConfigARR[idx].profileType = PROFILE_TYPE_HW_SKU_E;
        xpsPortConfigARR[idx].profileValue.hwTableSizes = &hw_sku_profiles_falcon[1];
        idx++;
    }
    else if (IS_DEVICE_FALCON_2(devType))
    {
        /* CPU ports for 2T */
        xpsPortConfigARR[idx].profileType = PROFILE_TYPE_CPU_PORT_MAP_E;
        xpsPortConfigARR[idx].profileValue.portMap.macNum = 136;
        xpsPortConfigARR[idx].profileValue.portMap.portNum = 63;
        xpsPortConfigARR[idx].profileValue.portMap.interfaceSpeed = 0;
        xpsPortConfigARR[idx].profileValue.portMap.interfaceMode =
            PROFILE_INTERFACE_MODE_KR_E;

        /* simulation 2T */
        xpsPortConfigARR[idx].profileType = PROFILE_TYPE_SIM_INIFILE_E;
        sprintf(xpsPortConfigARR[idx].profileValue.sim_inifile, "%s",
                "-i ./iniFiles/falcon_4T_A0_wm.ini");

        idx++;
        xpsPortConfigARR[idx].profileType = PROFILE_TYPE_HW_SKU_E;
        xpsPortConfigARR[idx].profileValue.hwTableSizes = &hw_sku_profiles_falcon[1];
        idx++;
    }
    else   /* unknown */
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsCreatePortMappingProfile: unknown dev type %d\n", devType);
        return XP_ERR_INVALID_ID;
    }

    /* last */
    xpsPortConfigARR[idx].profileType = PROFILE_TYPE_LAST_E;
    xpsPortConfigARR[idx].profileValue.no_param = 0;

    return XP_NO_ERR;
}

XP_STATUS xpsUpdatePortMappingProfile(XP_DEV_TYPE_T devType,
                                      int logicalPortNumber, int externalUsrPortIdx, char * infModeStr, int speed)
{
    unsigned int profileSpeed = 0;
    PROFILE_INTERFACE_MODE_ENT  interfaceMode = PROFILE_INTERFACE_MODE_1000BASE_X_E;
    static unsigned int profile_idx =
        0; /* index of the next free entry in the profile */

    if (strcmp(infModeStr, "1000BaseX") == 0)
    {
        interfaceMode = PROFILE_INTERFACE_MODE_1000BASE_X_E;
    }
    else if (strcmp(infModeStr, "SGMII") == 0)
    {
        interfaceMode = PROFILE_INTERFACE_MODE_QSGMII_E;
    }
    else  if (strcmp(infModeStr, "SR_LR") == 0)
    {
        interfaceMode = PROFILE_INTERFACE_MODE_SR_LR_E;
    }
    else  if (strcmp(infModeStr, "KR") == 0 || strcmp(infModeStr, "CR") == 0)
    {
        interfaceMode = PROFILE_INTERFACE_MODE_KR_E;
    }
    else  if (strcmp(infModeStr, "KR2") == 0)
    {
        interfaceMode = PROFILE_INTERFACE_MODE_KR2_E;
    }
    else  if (strcmp(infModeStr, "CR2") == 0)
    {
        interfaceMode = PROFILE_INTERFACE_MODE_CR2_E;
    }
    else  if (strcmp(infModeStr, "KR4") == 0)
    {
        interfaceMode = PROFILE_INTERFACE_MODE_KR4_E;
    }
    else  if (strcmp(infModeStr, "CR4") == 0)
    {
        interfaceMode = PROFILE_INTERFACE_MODE_CR4_E;
    }
    else  if (strcmp(infModeStr, "KR8") == 0 || strcmp(infModeStr, "CR8") == 0)
    {
        interfaceMode = PROFILE_INTERFACE_MODE_KR8_E;
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsUpdatePortMappingProfile: failed to convert ifMode %s to portIdxPlace %d portIdx %d\n",
              infModeStr, logicalPortNumber, externalUsrPortIdx);
    }

    switch (speed)
    {
        case XP_PG_SPEED_1G:
            profileSpeed = 10;
            break;
        case XP_PG_SPEED_10G:
            profileSpeed = 100;
            break;
        case XP_PG_SPEED_25G:
            profileSpeed = 250;
            break;
        case XP_PG_SPEED_40G:
            profileSpeed = 400;
            break;
        case XP_PG_SPEED_50G:
            profileSpeed = 500;
            break;
        case XP_PG_SPEED_100G:
            profileSpeed = 1000;
            break;
        case XP_PG_SPEED_200G:
            profileSpeed = 2000;
            break;
        default:
            printf("xpsUpdatePortMappingProfile: failed to convert speed %d to portIdxPlace %d portIdx %d\n",
                   speed, logicalPortNumber, externalUsrPortIdx);
    }

    xpsPortConfigARR[profile_idx].profileType = PROFILE_TYPE_PORT_MAP_E;
    xpsPortConfigARR[profile_idx].profileValue.portMap.macNum =
        externalUserPortIdxToMacPortMapping_MAC(devType, externalUsrPortIdx);
    xpsPortConfigARR[profile_idx].profileValue.portMap.portNum =
        logicalPortNumber; /* logical number */
    xpsPortConfigARR[profile_idx].profileValue.portMap.interfaceMode =
        interfaceMode;
    xpsPortConfigARR[profile_idx].profileValue.portMap.interfaceSpeed =
        profileSpeed;
    xpsPortConfigARR[profile_idx].profileValue.portMap.frontPanelPortIdx =
        logicalPortNumber;

    /* by default set all ports to AP */
    /*xpsMacSetPortAnEnable(0, logicalPortNumber, 1);*/

    profile_idx++;

    return XP_NO_ERR;
}

GT_STATUS xpsCpssInit(XP_DEV_TYPE_T devType)
{
    XP_STATUS   ret;
    GT_STATUS           rc;
    GT_U8           devNum;

    // Disable logging/stderr during initialization
    cmdStreamGrabSystemOutput = GT_FALSE;

    printf("Initializing Mrvl chip device type %d\n", devType);
    // Enable log
    gEnableCpssLog = 0;

    profile = cygnus;
#ifndef ASIC_SIMULATION /* Applicable only for HW */
    if (ALDB2B == devType)
    {
        rc = cpssHalPlatformDeInit(0);
        printf("cpssHalPlatformDeInit: platform deinit %d => ret %d\n", 0, rc);
    }
#endif
    if (ALDDB == devType)
    {
        profile = DB_CPSS_aldrinProfile/*aldrinProfile*/;/*AmitK*/
    }
    else if (ALD2 == devType)
    {
        profile = aldrin2Profile;
    }
    else if ((ALDB2B == devType) || (TG48M_P == devType))
    {
        profile = ac3x_b2b_profile0;
    }
    else if (CYGNUS == devType)
    {
        profile = cygnus;
    }
    else if (ALDRIN2XL == devType)
    {
        profile = aldrin2_xl_port_profile;
    }
    else if (FALCON64x100G == devType)
    {
        profile = falcon_64x100_port_profile;
    }
    else if (FALCON64x100GR4 == devType)
    {
        profile = falcon_64x100_R4_port_profile;
    }
    else if (FALCON32x25G64 == devType)
    {
        profile = falcon_32x25_6_4_port_profile;
    }
    else if (FALCON32x400G == devType)
    {
        profile = falcon_32x400_port_profile;
    }
    else if (FC32x10016x400G == devType)
    {
        profile = falcon_32x100_16x400_port_profile;
    }
    else if (FC64x100GR4IXIA == devType)
    {
        profile = falcon_64x100_ixia_port_profile;
    }
    else if (FC32x100GR4IXIA == devType)
    {
        profile = falcon_32x100_R4_6_4_port_profile_ixia;
    }
    else if (FALCON32 == devType)
    {
        profile = falcon_3_2_profile;
    }
    else if (FC24x25G8x200G == devType)
    {
        profile = falcon_24x25_8x200_port_profile;
    }
    else if (FC24x25G8x100G == devType)
    {
        profile = falcon_24x25_8x100_12_8_port_profile;
    }
    else if (FC24x100G4x400G == devType)
    {
        profile = falcon_24x100_4x400_port_profile;
    }
    else if (FC32x100G8x400G == devType)
    {
        profile = falcon_32x100_8x400_port_profile;
    }
    else if (FC48x100G8x400G == devType)
    {
        profile = falcon_48x100_8x400_port_profile;
    }
    else if (FC96x100G8x400G == devType)
    {
        profile = falcon_96x100_8x400_port_profile;
    }
    else if (FALCON128x10G == devType)
    {
        profile = falcon_128x10_port_profile;
    }
    else if (FALCON128x25G == devType)
    {
        profile = falcon_128x25_port_profile;
    }
    else if (FALCON128x50G == devType)
    {
        profile = falcon_128x50_port_profile;
    }
    else if (FALCON128x100G == devType)
    {
        profile = falcon_128x100_port_profile;
    }
    else if (FALCON128 == devType)
    {
        profile = falcon_32x25_port_profile;
    }
    else if (FALCON32X25G == devType)
    {
        profile = falcon_32x25_port_profile;
    }
    else if (FC32x25GIXIA == devType)
    {
        profile = falcon_32x25_8_ixia;
    }
    else if (FALCON64x25G == devType)
    {
        profile = falcon_64x25_port_profile;
    }
    else if (FC16x100G8x400G == devType)
    {
        profile = falcon_16x100_8x400_port_profile;
    }
    else if (FC24x258x100G64 == devType)
    {
        profile = falcon_24x25_8x100_6_4_port_profile;
    }
    else if (FC24x254x200G64 == devType)
    {
        profile = falcon_24x25_4x200_6_4_port_profile;
    }
    else if (FALCON16x400G == devType)
    {
        profile = falcon_16x400_port_profile;
    }
    else if (FC48x10G8x100G == devType)
    {
        profile = falcon_48x10_8x100_port_profile;
    }
    else if (FC48x25G8x100G == devType)
    {
        profile = falcon_48x25_8x100_port_profile;
    }
    else if (FC16x25GIXIA == devType)
    {
        profile = falcon_16x25_4_ixia;
    }
    else if (FALCON16X25G == devType)
    {
        profile = falcon_16x25_port_profile;
    }
    else if (FALCON128B2B == devType)
    {
        profile = falcon_belly2belly_profile;
    }
    else if (FC64x25G64x10G == devType)
    {
        profile = falcon_64x25_64x10_port_profile;
    }
    else if (FC32x25G8x100G == devType)
    {
        profile = falcon_32x25_8x100_port_profile;
    }
    else if (F2T80x25G == devType)
    {
        profile = falcon_2T_80x25_port_profile;
    }
    else if (F2T80x25GIXIA == devType)
    {
        profile = falcon_2T_80x25_port_profile;
    }
    else if (F2T48x25G8x100G == devType)
    {
        profile = falcon_2T_48x25_8x100_port_profile;
    }
    else if (F2T48x10G8x100G == devType)
    {
        profile = falcon_2T_48x10_8x100_port_profile;
    }
    else if (IS_DEVICE_EBOF_PROFILE(devType))
    {
        /* create dynamic falcon profile */
        if (xpsPortConfigARR != NULL)
        {
            profile = xpsPortConfigARR;
        }
        else
        {
            profile = falcon_64x25_64x10_port_profile;
        }
    }
    else if (AC5XRD == devType)
    {
        profile = ac5x_rd_board_profile;
    }
    else if (AC5X48x1G6x10G == devType)
    {
        profile = ac5x_48x1G6x10G_port_profile;
    }
    else if (ALDRIN2XLFL == devType)
    {
        profile = aldrin2_xl_fujitsu_large_profile;
    }
    else if (ALDRIN2EVAL == devType)
    {
        profile = aldrin2_xl_fujitsu_large_eval_profile;
    }
    else if (AC3XFS == devType)
    {
        profile = ac3x_fujitsu_small_profile;
    }
    else if (AC3XROB == devType)
    {
        profile = ac3x_fujitsu_small_profile;
    }
    else if (AC3XRAMAN == devType)
    {
        profile = ac3x_fujitsu_small_25_profile;
    }
    else if (AC3XMCS == devType)
    {
        profile = ac3x_fujitsu_small_25_profile;
    }
    else if (AC3XILA == devType)
    {
        profile = ac3x_fujitsu_small_ila_profile;
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid devType: %d\n", devType);
        return GT_FAIL;
    }

    rc = cpssHalInitializeValidateProfile(devType, profile);
    printf("cpssHalInitializeValidateProfile: %d\n", rc);

    rc = cpssHalSimInit(profile);
    XPS_INIT_API_RETURN("cpssHalSimInit", rc);

    rc = cpssHalInitializeSystem();
    XPS_INIT_API_RETURN("cpssHalInitializeSystem", rc);
    rc = cpssHalEnableLog(gEnableCpssLog);
    XPS_INIT_API_RETURN("cpssHalEnableLog", rc);

    /* Perform scan */
    for (devNum=0; devNum < 2; devNum++)
    {
        GT_U32 numPorts;

        if (ALDB2B == devType)
        {
            if (devNum == 1)
            {
                profile = ac3x_b2b_profile1;
            }
            else
            {
                profile = ac3x_b2b_profile0;
            }
        }
        if (TG48M_P == devType)
        {
            if (devNum == 1)
            {
                profile = ac3x_b2b_tg48mp_profile1;
            }
            else
            {
                profile = ac3x_b2b_tg48mp_profile0;
            }
        }

        if (AC5XRD == devType)
        {
            profile = ac5x_rd_board_profile;
        }

        if (AC5X48x1G6x10G == devType)
        {
            profile = ac5x_48x1G6x10G_port_profile;
        }

        if (IS_DEVICE_FUJITSU_LARGE(devType))
        {
            if (ALDRIN2XLFL == devType)
            {
                profile = aldrin2_xl_fujitsu_large_profile;
            }
            else if (ALDRIN2EVAL == devType)
            {
                profile = aldrin2_xl_fujitsu_large_eval_profile;
            }
        }

        if (IS_DEVICE_FUJITSU_SMALL(devType))
        {
            if (devType == AC3XMCS || devType ==  AC3XRAMAN) {
                profile = ac3x_fujitsu_small_25_profile;
            } 
            else if (devType == AC3XILA) {
                profile = ac3x_fujitsu_small_ila_profile;
            } 
            else 
                profile = ac3x_fujitsu_small_profile;
        }

        rc = cpssHalInitializeDeviceApi(devNum, devType, profile);
        cpssOsPrintf("cpssHalInitializeDeviceApi: %d\n", rc);

        if (rc == GT_NO_MORE) /* return status of cpssHalInitializeDeviceApi*/
        {
            // devNum has Number of PP's in a multidevice board
            ret = cpssHalSetNumDevices(devNum);
            cpssOsPrintf("cpssHalDeviceMgr: Number of Devices %d\n", devNum);
            break;
        }

        rc = cpssHalGetNumPorts(devNum, &numPorts);
        printf("cpssHalGetNumPorts: %d\n", rc);

        ret = cpssHalAddDevice(devNum);
        printf("cpssHalAddDevice: Adding device %d => ret %d\n", devNum, ret);

        ret = cpssHalInitDevice(devNum, devType, profile, numPorts);
        printf("cpssHalInitDevice: initializing device %d => ret %d\n", devNum, ret);

        if (cpssHalPortManagerIsSupported(devNum) == GT_OK)
        {
            rc = cpssHalPortMgrEventRegister(devNum);
            if (rc != GT_OK)
            {
                printf("calling cpssHalPortMgrEventRegister returned=%d", rc);
                return rc;
            }
            printf("called cpssHalPortMgrEventRegister");
        }
    }
    ret = cpssHalSetNumDevices(devNum);
    printf("cpssHalDeviceMgr: Number of Devices %d\n", devNum);
#ifdef SHELL_ENABLE
    rc = cpssHalInitializeCmdInitApi();
    cpssOsPrintf("cpssHalInitializeCmdInitApi: %d\n", rc);
#endif // SHELL_ENABLE
#ifndef ASIC_SIMULATION /* Applicable only for HW */
    if (ALDB2B == devType)
    {
        rc = cpssHalPlatformInit(0, devType);
        printf("cpssHalPlatformInit: platform init %d => ret %d\n", devNum, rc);
    }
#endif

    xpsPortMappingDump(0);
    extern GT_STATUS cpssHalPrintPortMap(int devId);
    cpssHalPrintPortMap(0);

    return rc;
}
/********************************************* MRVL INIT END **************************************************/
XP_STATUS xpsCpssCfgTableNumEntriesGet(xpsDevice_t devId,
                                       xpCfgTblEntryCountStruct *tblEntryCountPtr)
{
    XP_DEV_TYPE_T devType;
    cpssHalGetDeviceType(devId, &devType);

    tblEntryCountPtr->vlanIds = cpssHalGetSKUMaxVlanIds(devId);
    tblEntryCountPtr->lagIds = cpssHalGetSKUMaxLAGrps(devId);
    tblEntryCountPtr->lagMbrCnt = cpssHalGetSKUmaxLAGMbrPerGrp(devId);
    tblEntryCountPtr->mac2meIds = cpssHalGetSKUmaxMac2Me(devId);
    tblEntryCountPtr->nextHopEntries = cpssHalGetSKUmaxNH(devId);
    tblEntryCountPtr->arpEntries = cpssHalGetSKUmaxNeighbours(devId);
    tblEntryCountPtr->nextHopGroupEntries = cpssHalGetSKUmaxNhEcmpGrps(devId);
    tblEntryCountPtr->fdbEntries = cpssHalGetSKUmaxHashTable(devId);
    tblEntryCountPtr->aclEntries = cpssHalGetSKUmaxIACL(devId);
    if (IS_DEVICE_FALCON(devType))
    {
        tblEntryCountPtr->ipv4RouteEntries = GetRouteIpv4MaxNum();
        tblEntryCountPtr->ipv6RouteEntries = GetRouteIpv6MaxNum();
    }
    else
    {
        tblEntryCountPtr->ipv4RouteEntries = cpssHalGetSKUmaxRouteLPM(devId);
        tblEntryCountPtr->ipv6RouteEntries = cpssHalGetSKUmaxRouteLPM(
                                                 devId)/4; /*why /4 here and why /2 for ALD*/
    }

    return XP_NO_ERR;
}

void xpsResetSwitch(xpsDevice_t devId)
{

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsResetSwitch: Resetting devId %d\n", devId);
    cpssHalResetDevice((int)devId);
#if 0
    XP_DEV_TYPE_T devType;
    cpssHalGetDeviceType(devId, &devType);
    if (devType == ALDB2B)
    {
        cpssHalPlatformDeInit(devId);
    }
#endif
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsResetSwitch: Reset Done %d\n", devId);
}

XP_STATUS xpsLedModeSet(xpDevice_t devId, const char* ledModeStr)
{
    cpssHalLedProfileSet(devId, ledModeStr);
    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
