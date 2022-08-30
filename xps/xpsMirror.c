// xpsMirror.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsMirror.h"
#include "xpsUtil.h"
#include "xpsInit.h"
#include "xpsState.h"
#include "xpsScope.h"
#include "xpsLock.h"
#include "xpsAllocatorMgr.h"
#include "xpsAllocator.h"
#include "openXpsL3.h"
#include "xpsTunnel.h"
#include "xpsPolicer.h"
#include "xpsInternal.h"
#include "xpsLag.h"
#include "cpssHalUtil.h"
#include "cpssHalMirror.h"
#include "cpssHalL3.h"
#include "cpssHalPha.h"
#include "cpssHalAcl.h"
#include "cpssHalQos.h"
#include "xpsAcl.h"
#include "xpsErspanGre.h"
#include "xpsGlobalSwitchControl.h"

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

uint32_t egressErspanIpv4AclTableId = 0;
uint32_t egressErspanIpv6AclTableId = 0;
uint32_t egressErspanNonIpAclTableId = 0;

#define XPS_MIRROR_ANALYZER_EPORT_RANGE_END     6144

#define MIRROR_SESSION_DSCP_SHIFT    2

#ifdef __cplusplus
extern "C" {
#endif

/* ===========  FUNCTION POINTER DECLARATIONS =========== */
XP_STATUS(* xpsMirrorSetTruncateState)(xpsDevice_t deviceId,
                                       uint32_t analyzerId, uint32_t truncateSize);

XP_STATUS(* xpsMirrorSetSampleRate)(xpsDevice_t deviceId,
                                    uint32_t analyzerId, uint32_t sampleRate, bool ingress);


XP_STATUS xpsMirrorErspanOamFlowCntrConfig(xpsDevice_t deviceId);

/**
 * \var mirrorSesDbHndl maintains database per analyzer session
 *
 */
static xpsDbHandle_t mirrorSesDbHndl = XPS_STATE_INVALID_DB_HANDLE;

typedef struct
{
    uint32_t            analyzerId;
    xpsMirrorType_e     type;
    xpsMirrorData_t     data;
} xpsMirrorSessionDbEntry_t;

/**
 * \brief Statically defined key compare function for mirror
 *        manager's database
 *
 * This API is defined as static, but will be registered via
 * function pointer to be used internally by State manager to do
 * comparisons for the interface manager database
 *
 * \param [in] key1
 * \param [in] key2
 *
 * \return int32_t
 */
static int32_t mirrorKeyCompare(void* key1, void* key2)
{
    return (*(int32_t*)(key2) - *(int32_t*)(key1));
}

XP_STATUS xpsMirrorDbAddAnalyzer(xpsScope_t scopeId, uint32_t analyzerId,
                                 xpsMirrorType_e mirrorType, xpsMirrorData_t mirrorData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsMirrorDbAddAnalyzer);
    XP_STATUS status = XP_NO_ERR;
    xpsMirrorSessionDbEntry_t *dbEntry;

    // Check if the mirror session db has been created
    if (mirrorSesDbHndl == XPS_STATE_INVALID_DB_HANDLE)
    {
        XPS_FUNC_EXIT_LOG();
        return XP_ERR_INIT;
    }

    // Allocate space for the mirror session db entry
    if ((status = xpsStateHeapMalloc(sizeof(xpsMirrorSessionDbEntry_t),
                                     (void**)&dbEntry)) != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return status;
    }

    if (!dbEntry)
    {
        XPS_FUNC_EXIT_LOG();
        return XP_ERR_NULL_POINTER;
    }

    dbEntry->analyzerId = analyzerId;
    dbEntry->type = mirrorType;
    dbEntry->data = mirrorData;

    // Insert the mirror entry into the database, using analyzer id as key

    if ((status = xpsStateInsertData(scopeId, mirrorSesDbHndl,
                                     (void*)dbEntry)) != XP_NO_ERR)
    {
        // Free Allocated memory
        xpsStateHeapFree((void*)dbEntry);
        XPS_FUNC_EXIT_LOG();
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/**
 * \brief Remove an analyzer from the mirror analyzer database
 *
 * \param [in] scopeId
 * \param [in] analyzerId
 *
 * \return XP_STATUS
 */
static XP_STATUS xpsMirrorDbRemoveAnalyzer(xpsScope_t scopeId,
                                           uint32_t analyzerId)
{
    XP_STATUS status = XP_NO_ERR;
    xpsMirrorSessionDbEntry_t dbEntry;
    xpsMirrorSessionDbEntry_t *lookupEntry;

    // Check if the mirror session db has been created
    if (mirrorSesDbHndl == XPS_STATE_INVALID_DB_HANDLE)
    {
        return XP_ERR_INIT;
    }

    dbEntry.analyzerId = analyzerId;

    // Remove the mirror entry
    if ((status = xpsStateDeleteData(scopeId, mirrorSesDbHndl, (xpsDbKey_t)&dbEntry,
                                     (void**)&lookupEntry)) != XP_NO_ERR)
    {
        return status;
    }

    // Free the allocated memory
    if ((status = xpsStateHeapFree((void*)lookupEntry)) != XP_NO_ERR)
    {
        return status;
    }

    return status;
}


/**
 * \brief Obtain the analyzer data for a specific analyzer from the mirror analyzer database
 *
 * \param [in] scopeId
 * \param [in] analyzerId
 * \param [out] lookupEntry
 *
 * \return XP_STATUS
 */
static XP_STATUS xpsMirrorDbGetAnalyzerData(xpsScope_t scopeId,
                                            uint32_t analyzerId, xpsMirrorSessionDbEntry_t **lookupEntry)
{
    XP_STATUS status = XP_NO_ERR;
    xpsMirrorSessionDbEntry_t dbEntry;

    // Check if the mirror session db has been created
    if (mirrorSesDbHndl == XPS_STATE_INVALID_DB_HANDLE)
    {
        return XP_ERR_INIT;
    }

    dbEntry.analyzerId = analyzerId;

    // Look for the mirror entry
    if ((status = xpsStateSearchData(scopeId, mirrorSesDbHndl, (xpsDbKey_t)&dbEntry,
                                     (void**)lookupEntry)) != XP_NO_ERR)
    {
        return status;
    }

    if (!(*lookupEntry))
    {
        status = XP_ERR_KEY_NOT_FOUND;
    }

    return status;
}

XP_STATUS xpsMirrorErspanSessionTypeGet(xpsDevice_t deviceId,
                                        uint32_t analyzerId, bool *isIpv4,
                                        xpsMirrorType_e *mirType,
                                        uint32_t *ePort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsMirrorErspanSessionTypeGet);

    XP_STATUS retVal = XP_NO_ERR;
    xpsMirrorSessionDbEntry_t *lookupEntry = NULL;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(deviceId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              deviceId);
        return XP_ERR_INVALID_DEV_ID;
    }

    // Get scope from Device ID
    retVal = xpsScopeGetScopeId(deviceId, &scopeId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to get scope Id for device %d\n", deviceId);
        return retVal;
    }

    // Get Analyzer Data from DB
    retVal = xpsMirrorDbGetAnalyzerData(scopeId, analyzerId, &lookupEntry);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    if (lookupEntry->data.erspan2Data.ipType == XP_PREFIX_TYPE_IPV4)
    {
        *isIpv4 = true;
    }
    else
    {
        *isIpv4 = false;
    }

    if (mirType)
    {
        *mirType = lookupEntry->type;
    }

    if (ePort)
    {
        *ePort = lookupEntry->data.erspan2Data.ePort;
    }

    return retVal;
}

XP_STATUS xpsMirrorInitApi(xpsDevice_t deviceId)
{
    CPSS_PP_FAMILY_TYPE_ENT devFamilyType = cpssHalDevPPFamilyGet(deviceId);

    switch (devFamilyType)
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            xpsMirrorSetTruncateState = xpsMirrorSetTruncateStateFalcon;
            xpsMirrorSetSampleRate = xpsMirrorSetSampleRateFalcon;
            break;

        default:             /*TODO Add other devices if necessary*/
            xpsMirrorSetTruncateState = xpsMirrorSetTruncateStateLegacy;
            xpsMirrorSetSampleRate = NULL;
            break;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsMirrorInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;

    retVal = xpsMirrorInitScope(XP_SCOPE_DEFAULT);

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsMirrorInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;
    xpsDevice_t devId = 0;

    // Analyzer index 0 reserved for mirror disabled
    retVal = xpsAllocatorInitIdAllocator(scopeId,
                                         XPS_ALLOCATOR_ANALYZER,
                                         XP_MIRROR_MAX_USER_SESSION+1,
                                         XP_MIRROR_MIN_USER_SESSION);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    mirrorSesDbHndl = XPS_MIRROR_SES_DB_HNDL;
    // Create Mirror Db
    if ((retVal = xpsStateRegisterDb(scopeId, "Mirror Db", XPS_GLOBAL,
                                     &mirrorKeyCompare, mirrorSesDbHndl)) != XP_NO_ERR)
    {
        mirrorSesDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        return retVal;
    }

    cpssStatus = cpssHalMirrorInit(devId);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalMirrorInit Failed %d \n", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    if ((cpssHalDevPPFamilyGet(devId) == CPSS_PP_FAMILY_DXCH_FALCON_E) ||
        (cpssHalDevPPFamilyGet(devId) == CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        cpssStatus = cpssHalPhaInit(devId);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalPhaInit Failed %d \n", cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        cpssStatus = cpssHalMirrorErspanOAMFlowConfig(devId);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalMirrorErspanOAMFlowConfig Failed %d \n", cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        retVal = xpsMirrorErspanOamFlowCntrConfig(devId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: xpsMirrorErspanOamFlowCntrConfig %d \n", retVal);
            return retVal;
        }

        xpAclTableInfo_t     tableInfo;
        memset(&tableInfo, 0x00, sizeof(xpAclTableInfo_t));

        tableInfo.tableType = XPS_ACL_TABLE_TYPE_IPV4;
        tableInfo.stage = XPS_PCL_STAGE_EGRESS;
        retVal = xpsAclCreateTable(devId, tableInfo, &egressErspanIpv4AclTableId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: Failed to create table for the device %u \n", devId);
            return retVal;
        }

        tableInfo.tableType = XPS_ACL_TABLE_TYPE_IPV6;
        tableInfo.stage = XPS_PCL_STAGE_EGRESS;
        retVal = xpsAclCreateTable(devId, tableInfo, &egressErspanIpv6AclTableId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: Failed to create table for the device %u \n", devId);
            return retVal;
        }

        tableInfo.tableType = XPS_ACL_TABLE_TYPE_NON_IP;
        tableInfo.stage = XPS_PCL_STAGE_EGRESS;
        retVal = xpsAclCreateTable(devId, tableInfo, &egressErspanNonIpAclTableId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: Failed to create table for the device %u \n", devId);
            return retVal;
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMirrorDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;

    retVal = xpsMirrorDeInitScope(XP_SCOPE_DEFAULT);

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsMirrorDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;

    // Purge Mirror Db
    if (mirrorSesDbHndl != XPS_STATE_INVALID_DB_HANDLE)
    {
        if ((retVal = xpsStateDeRegisterDb(scopeId, &mirrorSesDbHndl)) != XP_NO_ERR)
        {
            XPS_FUNC_EXIT_LOG();
            return retVal;
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMirrorAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMirrorRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMirrorGetAnalyzerEPort(xpsScope_t scopeId, uint32_t analyzerId,
                                    uint32_t *ePort)
{
    // For now hardcoded TODO: Using allocator
    *ePort = (XPS_MIRROR_ANALYZER_EPORT_RANGE_END - 1) - analyzerId;
    return XP_NO_ERR;
}

XP_STATUS xpsMirrorCreateAnalyzerSession(xpsMirrorType_e type,
                                         xpsMirrorData_t data, uint32_t *analyzerId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status;

    XPS_LOCK(xpsMirrorCreateAnalyzerSession);

    status = xpsMirrorCreateAnalyzerSessionScope(XP_SCOPE_DEFAULT, type, data,
                                                 analyzerId);

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsMirrorCreateAnalyzerSessionScope(xpsScope_t scopeId,
                                              xpsMirrorType_e type, xpsMirrorData_t data, uint32_t *analyzerId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;
    uint32_t *eport = NULL;
    uint32_t *policerIdx = NULL;

    // Allocate analyzer Id
    retVal = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_ANALYZER, analyzerId);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    if (type == XP_MIRROR_ERSPAN2)
    {
        eport = &data.erspan2Data.ePort;
        policerIdx = &data.erspan2Data.ePortPolicerIndex;
    }
    else if (type == XP_MIRROR_LOCAL)
    {
        eport = &data.spanData.ePort;
        policerIdx = &data.spanData.ePortPolicerIndex;
    }
    else
    {
        return XP_ERR_INVALID_DATA;
    }

    // Get ePort for each analyzer session
    retVal = xpsMirrorGetAnalyzerEPort(scopeId, *analyzerId, eport);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    if (type == XP_MIRROR_ERSPAN2)
    {
        *policerIdx = (*analyzerId + CPSSHAL_ERSPAN_EPLR_FLOW_ID_BASE);
    }
    else
    {
        retVal = xpsPolicerIndexAllocate(XP_EGRESS_FLOW_POLICER, policerIdx);
        if (retVal != XP_NO_ERR)
        {
            XPS_FUNC_EXIT_LOG();
            return retVal;
        }
    }

    // Add the analyzer to the mirror db
    retVal = xpsMirrorDbAddAnalyzer(scopeId, *analyzerId, type, data);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMirrorAddAnalyzerInterface(uint32_t analyzerId,
                                        xpsInterfaceId_t analyzerIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;

    XPS_LOCK(xpsMirrorAddAnalyzerInterface);

    retVal = xpsMirrorAddAnalyzerInterfaceScope(XP_SCOPE_DEFAULT, analyzerId,
                                                analyzerIntfId);

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsMirrorAddAnalyzerInterfaceScope(xpsScope_t scopeId,
                                             uint32_t analyzerId, xpsInterfaceId_t analyzerIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    xpsMirrorSessionDbEntry_t * lookupEntry = NULL;
    XP_STATUS                   retVal = XP_NO_ERR;

    retVal = xpsMirrorDbGetAnalyzerData(scopeId, analyzerId, &lookupEntry);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    if (lookupEntry->type == XP_MIRROR_ERSPAN2)
    {
        lookupEntry->data.erspan2Data.analyzerIntfId = analyzerIntfId;
    }
    else if (lookupEntry->type == XP_MIRROR_LOCAL)
    {
        lookupEntry->data.spanData.analyzerIntfId = analyzerIntfId;
    }
    else
    {
        return XP_ERR_INVALID_DATA;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMirrorGetAnalyzerIntfPolicerIndex(uint32_t analyzerId,
                                               uint32_t *ePort, uint32_t *policerIndex)
{
    XPS_FUNC_ENTRY_LOG();

    xpsMirrorSessionDbEntry_t * lookupEntry = NULL;
    XP_STATUS                   retVal = XP_NO_ERR;

    retVal = xpsMirrorDbGetAnalyzerData(XP_SCOPE_DEFAULT, analyzerId, &lookupEntry);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    if (lookupEntry)
    {
        *ePort = lookupEntry->data.erspan2Data.ePort;
        *policerIndex = lookupEntry->data.erspan2Data.ePortPolicerIndex;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMirrorErspanOamFlowCntrConfig(xpsDevice_t deviceId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;
    uint32_t  index = 0;

    /* First 7 Egress Flow policer indices will be used for ERSPAN */
    /* index 512 to 518 */
    for (index = CPSSHAL_ERSPAN_EPLR_FLOW_ID_BASE;
         index < (CPSSHAL_ERSPAN_EPLR_FLOW_ID_BASE + XP_MIRROR_MAX_USER_SESSION + 1);
         index++)
    {

        retVal = xpsAllocatorAllocateWithId(XP_SCOPE_DEFAULT,
                                            XPS_ALLOCATOR_EGRESS_FLOW_POLICER_ENTRY, index);
        if (retVal != XP_NO_ERR)
        {
            XPS_FUNC_EXIT_LOG();
            return retVal;
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMirrorSetSampleRateFalcon(xpsDevice_t deviceId,
                                       uint32_t analyzerId, uint32_t sampleRate, bool ingress)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS cpssStatus;

    // Set truncate to Analyser Index
    cpssStatus = cpssHalMirrorAnalyzerSampleRateSet(deviceId, analyzerId,
                                                    sampleRate, (GT_BOOL)ingress);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set Truncate state for Mirror session. Error code %d \n",
              cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsMirrorSetTruncateStateFalcon(xpsDevice_t deviceId,
                                          uint32_t analyzerId, uint32_t truncateSize)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS cpssStatus;

    // Set truncate to Analyser Index
    cpssStatus = cpssHalSetMirrorTruncateState(deviceId, analyzerId,
                                               truncateSize ? GT_TRUE : GT_FALSE);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set Truncate state for Mirror session. Error code %d \n",
              cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsMirrorSetTruncateStateLegacy(xpsDevice_t deviceId,
                                          uint32_t analyzerId, uint32_t truncateSize)
{
    XPS_FUNC_ENTRY_LOG();

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "Setting truncate state for analyzer feature not supported for device\n");

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsMirrorConfigAnalyzerPortEgressMac(xpsDevice_t deviceId,
                                               xpsInterfaceId_t analyzerIntf, uint32_t ePort,
                                               macAddr_t mac, bool add)
{
    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS cpssStatus;
    xpsInterfaceType_e intfType;
    xpsLagPortIntfList_t portList;
    xpsInterfaceId_t portIntf;
    uint8_t cpssDevNum;
    uint32_t cpssPortNum;
    uint32_t i;

    retVal = xpsInterfaceGetType(analyzerIntf, &intfType);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get type for interface %u, error: %d\n", analyzerIntf, retVal);
        return retVal;
    }

    if (XPS_LAG == intfType)
    {
        memset(&portList, 0, sizeof(portList));
        retVal = xpsLagGetPortIntfList(analyzerIntf, &portList);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed, lag interface(%d)", analyzerIntf);
            return retVal;
        }

        for (i = 0; i < portList.size; i++)
        {
            portIntf = portList.portIntf[i];
            cpssDevNum = xpsGlobalIdToDevId(deviceId, portIntf);
            cpssPortNum = xpsGlobalPortToPortnum(deviceId, portIntf);
            if (add)
            {
                cpssStatus = cpssHalL3SetPortEgressMac(cpssDevNum, cpssPortNum, mac);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to set egress mac to port : %d \n", cpssPortNum);
                    return xpsConvertCpssStatusToXPStatus(cpssStatus);
                }
            }
            else
            {
                cpssStatus = cpssHalL3RemoveEgressRouterMac(cpssDevNum, GT_FALSE, cpssPortNum);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to set egress mac to port : %d \n", cpssPortNum);
                    return xpsConvertCpssStatusToXPStatus(cpssStatus);
                }
            }
        }
    }
    else if (XPS_PORT == intfType)
    {
        cpssDevNum = xpsGlobalIdToDevId(deviceId, analyzerIntf);
        cpssPortNum = xpsGlobalPortToPortnum(deviceId, analyzerIntf);
        if (add)
        {
            cpssStatus = cpssHalL3SetPortEgressMac(cpssDevNum, cpssPortNum, mac);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set egress mac to port : %d \n", cpssPortNum);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
        else
        {
            cpssStatus = cpssHalL3RemoveEgressRouterMac(cpssDevNum, GT_FALSE, cpssPortNum);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set egress mac to port : %d \n", cpssPortNum);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }
    else
    {
        return XP_ERR_INVALID_DATA;
    }

    return retVal;
}

XP_STATUS xpsMirrorErspanEnableFalcon(xpsDevice_t deviceId, uint32_t analyzerId,
                                      xpsInterfaceId_t analyzerIntf, bool enable)
{
    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS cpssStatus;
    xpsInterfaceType_e intfType;
    xpsLagPortIntfList_t portList;
    uint32_t i;

    retVal = xpsInterfaceGetType(analyzerIntf, &intfType);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get type for interface %u, error: %d\n", analyzerIntf, retVal);
        return retVal;
    }

    if (XPS_LAG == intfType)
    {
        memset(&portList, 0, sizeof(portList));
        retVal = xpsLagGetPortIntfList(analyzerIntf, &portList);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed, lag interface(%d)", analyzerIntf);
            return retVal;
        }

        for (i = 0; i < portList.size; i++)
        {
            retVal = xpsAclErspanRuleUpdate(deviceId, portList.portIntf[i], enable,
                                            analyzerId);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: xpsAclErspanRuleUpdate %d\n", retVal);
                return retVal;
            }

            cpssStatus = cpssHalAclSetEgressAclPacketType(deviceId, portList.portIntf[i],
                                                          CPSS_DXCH_PCL_EGRESS_PKT_TO_ANALYZER_E, (GT_BOOL)enable);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed cpssHalAclSetEgressAclPacketType : %d \n", cpssStatus);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }
    else if (XPS_PORT == intfType)
    {
        retVal = xpsAclErspanRuleUpdate(deviceId, analyzerIntf, enable, analyzerId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: xpsAclErspanRuleUpdate %d\n", retVal);
            return retVal;
        }

        cpssStatus = cpssHalAclSetEgressAclPacketType(deviceId, analyzerIntf,
                                                      CPSS_DXCH_PCL_EGRESS_PKT_TO_ANALYZER_E, (GT_BOOL)enable);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed cpssHalAclSetEgressAclPacketType : %d \n", cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    else
    {
        return XP_ERR_INVALID_DATA;
    }

    return retVal;
}

XP_STATUS xpsMirrorConfigAnalyzerSession(xpsDevice_t deviceId,
                                         uint32_t analyzerId, xpsMirrorType_e type, uint32_t ePort,
                                         xpsMirrorData_t * mirrorData)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS cpssStatus;
    uint32_t tnlTableIndex;
    uint32_t truncateSize = 0;
    uint32_t sampleRate = 1;
    uint8_t    tc = 0;
    uint32_t analyzerIntf = 0;
    xpsInterfaceType_e intfType;
    GT_BOOL   isIngress = GT_FALSE;

    if (type == XP_MIRROR_ERSPAN2)
    {
        analyzerIntf = mirrorData->erspan2Data.analyzerIntfId;
        tc = mirrorData->erspan2Data.tc;
        truncateSize  = mirrorData->erspan2Data.truncateSize;
        sampleRate  = mirrorData->erspan2Data.sampleRate;
        isIngress = (GT_BOOL)mirrorData->erspan2Data.isIngress;

        if ((cpssHalDevPPFamilyGet(deviceId) != CPSS_PP_FAMILY_DXCH_FALCON_E) &&
            (cpssHalDevPPFamilyGet(deviceId) != CPSS_PP_FAMILY_DXCH_AC5X_E))
        {

            // Set tos bits for tunnel
            retVal = xpsIpTunnelSetConfigByMirrorSession(deviceId,
                                                         mirrorData->erspan2Data.erspanId, mirrorData);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set TOS bits for Tunnel. Error code %d \n", retVal);
                return retVal;
            }

            // Set Egress Mac for Analyzer port
            retVal = xpsMirrorConfigAnalyzerPortEgressMac(deviceId, analyzerIntf, ePort,
                                                          mirrorData->erspan2Data.srcMacAddr, true);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set egress mac for analyzer port. Error code %d \n", retVal);
                return retVal;
            }

            // Get Tunnel Table Index
            retVal = xpsIpTunnelDbGetTunnelTermIndex(deviceId,
                                                     mirrorData->erspan2Data.erspanId, &tnlTableIndex);
            if (retVal != XP_NO_ERR)
            {
                XPS_FUNC_EXIT_LOG();
                return retVal;
            }

            // Bind Tunnel to Eport
            cpssStatus = cpssHalBindAnalyzerToTunnel(deviceId, analyzerId, tnlTableIndex,
                                                     ePort);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to bind tunnel to mirror session %d\n", analyzerId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }
    else if (type == XP_MIRROR_LOCAL)
    {
        analyzerIntf = mirrorData->spanData.analyzerIntfId;
        tc = mirrorData->spanData.tc;
        truncateSize  = mirrorData->spanData.truncateSize;
        sampleRate  = mirrorData->spanData.sampleRate;
        isIngress = (GT_BOOL)mirrorData->spanData.isIngress;
    }
    else
    {
        return XP_ERR_INVALID_DATA;
    }

    // Marking of Mirror packets
    cpssStatus = cpssHalMirrorAnalyzerDpTcSet(deviceId, isIngress, CPSS_DP_GREEN_E,
                                              tc);
    if (cpssStatus != GT_OK)
    {
        retVal = xpsConvertCpssStatusToXPStatus(cpssStatus);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set DC and TC bits for Tunnel. Error code %d \n", retVal);
        return retVal;
    }

    retVal = xpsMirrorSetTruncateState(deviceId, analyzerId, truncateSize);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set Truncate state for Mirror session %d. Error code %d \n",
              analyzerId, retVal);
        return retVal;
    }

    if (xpsMirrorSetSampleRate)
    {
        retVal = xpsMirrorSetSampleRate(deviceId, analyzerId, sampleRate, true);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set Truncate state for Mirror session %d. Error code %d \n",
                  analyzerId, retVal);
            return retVal;
        }
        retVal = xpsMirrorSetSampleRate(deviceId, analyzerId, sampleRate, false);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set Truncate state for Mirror session %d. Error code %d \n",
                  analyzerId, retVal);
            return retVal;
        }
    }

    uint32_t monitorIntf = analyzerIntf;
    CPSS_INTERFACE_TYPE_ENT portType = CPSS_INTERFACE_PORT_E;
    GT_TRUNK_ID trunkId = 0;
    uint32_t cpuPortNum  = 0;

    retVal = xpsGlobalSwitchControlGetCpuPhysicalPortNum(deviceId, &cpuPortNum);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not get CPU physical port number, error %d\n", retVal);
        return retVal;
    }

    if (monitorIntf != cpuPortNum)
    {
        retVal = xpsInterfaceGetType(analyzerIntf, &intfType);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get type for interface %u, error: %d\n", analyzerIntf, retVal);
            return retVal;
        }

        if (XPS_LAG == intfType)
        {
            xpsLagPortIntfList_t portList;
            memset(&portList, 0, sizeof(portList));
            portType = CPSS_INTERFACE_TRUNK_E;
            trunkId = xpsUtilXpstoCpssInterfaceConvert(analyzerIntf, XPS_LAG);

            retVal = xpsLagGetPortIntfList(analyzerIntf, &portList);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ports failed, lag interface(%d)", analyzerIntf);
                return retVal;
            }

            if (portList.size)
            {
                analyzerIntf = portList.portIntf[0];
            }
        }
    }
    // Bind Eport to Analyzer and Write Mirror Entry in HW
    cpssStatus = cpssHalAnalyzerPortSet(deviceId, analyzerId, portType,
                                        analyzerIntf, trunkId, ePort);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to write mirror entry\n");
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    if (type == XP_MIRROR_ERSPAN2)
    {
        if ((cpssHalDevPPFamilyGet(deviceId) == CPSS_PP_FAMILY_DXCH_FALCON_E) ||
            (cpssHalDevPPFamilyGet(deviceId) == CPSS_PP_FAMILY_DXCH_AC5X_E))
        {
            retVal = xpsMirrorErspanEnableFalcon(deviceId, analyzerId, monitorIntf, true);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: xpsMirrorErspanEnableFalcon %d\n", retVal);
                return retVal;
            }
            /* Set dummy 12B tag for V6 Session. This is required for PHA to work.
               Use Tpid index 6 (4B tag) and 7(8B tag) for this. */
            if (mirrorData->erspan2Data.ipType == XP_PREFIX_TYPE_IPV6)
            {
                cpssStatus = cpssHalMirrorErspan2V6DummyTagSet(deviceId, ePort);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssHalMirrorErspan2V6DummyTagSet Failed ePort %d ret %d \n", ePort,
                          cpssStatus);
                    return xpsConvertCpssStatusToXPStatus(cpssStatus);
                }
            }
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMirrorWriteAnalyzerSession(xpsDevice_t deviceId,
                                        uint32_t analyzerId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsMirrorWriteAnalyzerSession);

    XP_STATUS retVal = XP_NO_ERR;
    xpsMirrorSessionDbEntry_t *lookupEntry = NULL;
    xpsScope_t scopeId;
    uint32_t ePort = 0;

    if (IS_DEVICE_VALID(deviceId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              deviceId);
        return XP_ERR_INVALID_DEV_ID;
    }

    // Get scope from Device ID
    retVal = xpsScopeGetScopeId(deviceId, &scopeId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to get scope Id for device %d\n", deviceId);
        return retVal;
    }

    // Get Analyzer Data from DB
    retVal = xpsMirrorDbGetAnalyzerData(scopeId, analyzerId, &lookupEntry);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    if (lookupEntry->type == XP_MIRROR_ERSPAN2)
    {
        ePort = lookupEntry->data.erspan2Data.ePort;

        if ((cpssHalDevPPFamilyGet(deviceId) != CPSS_PP_FAMILY_DXCH_FALCON_E) &&
            (cpssHalDevPPFamilyGet(deviceId) != CPSS_PP_FAMILY_DXCH_AC5X_E))
        {
            retVal = xpsMirrorErspanTunCreate(deviceId, analyzerId, &lookupEntry->data);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: xpsMirrorErspanTunCreate %d\n", retVal);
                return retVal;
            }
        }

        retVal = xpsMirrorErspanCfgSet(deviceId, analyzerId, &lookupEntry->data);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: xpsMirrorErspanCfgSet %d\n", retVal);
            return retVal;
        }
    }
    else if (lookupEntry->type == XP_MIRROR_LOCAL)
    {
        ePort = lookupEntry->data.spanData.ePort;
    }
    else
    {
        return XP_ERR_INVALID_DATA;
    }

    // Configure mirror session
    retVal = xpsMirrorConfigAnalyzerSession(deviceId, analyzerId, lookupEntry->type,
                                            ePort, &lookupEntry->data);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMirrorUpdateAnalyzerSession(xpsDevice_t deviceId,
                                         uint32_t analyzerId, xpsMirrorData_t *mirrorData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsMirrorUpdateAnalyzerSession);

    XP_STATUS retVal = XP_NO_ERR;
    xpsMirrorSessionDbEntry_t *lookupEntry = NULL;
    xpsScope_t scopeId;
    uint32_t ePort = 0;

    if (IS_DEVICE_VALID(deviceId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              deviceId);
        return XP_ERR_INVALID_DEV_ID;
    }

    // Get scope from Device ID
    retVal = xpsScopeGetScopeId(deviceId, &scopeId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to get scopeId for device %d\n", deviceId);
        return retVal;
    }

    // Get Analyzer Data from DB
    retVal = xpsMirrorDbGetAnalyzerData(scopeId, analyzerId, &lookupEntry);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    if (lookupEntry->type == XP_MIRROR_ERSPAN2)
    {
        ePort = lookupEntry->data.erspan2Data.ePort;
        mirrorData->erspan2Data.erspanId = lookupEntry->data.erspan2Data.erspanId;
        mirrorData->erspan2Data.isIngress = lookupEntry->data.erspan2Data.isIngress;

        retVal = xpsMirrorErspanCfgSet(deviceId, analyzerId, mirrorData);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: xpsMirrorErspanCfgSet %d\n", retVal);
            return retVal;
        }

        if (cpssHalDevPPFamilyGet(deviceId) != CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            retVal = xpsMirrorConfigAnalyzerPortEgressMac(deviceId,
                                                          lookupEntry->data.erspan2Data.analyzerIntfId, ePort,
                                                          lookupEntry->data.erspan2Data.srcMacAddr, false);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set egress mac for analyzer port. Error code %d \n", retVal);
                return retVal;
            }
        }
        else
        {
            retVal = xpsMirrorErspanEnableFalcon(deviceId, analyzerId,
                                                 lookupEntry->data.erspan2Data.analyzerIntfId, false);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: xpsMirrorErspanEnableFalcon %d\n", retVal);
                return retVal;
            }
        }
    }
    else if (lookupEntry->type == XP_MIRROR_LOCAL)
    {
        ePort = lookupEntry->data.spanData.ePort;
        mirrorData->spanData.isIngress = lookupEntry->data.spanData.isIngress;
    }
    else
    {
        return XP_ERR_INVALID_DATA;
    }

    // Configure mirror session
    retVal = xpsMirrorConfigAnalyzerSession(deviceId, analyzerId, lookupEntry->type,
                                            ePort, mirrorData);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    if (lookupEntry->type == XP_MIRROR_ERSPAN2)
    {
        mirrorData->erspan2Data.ePort = lookupEntry->data.erspan2Data.ePort;
        mirrorData->erspan2Data.ePortPolicerIndex =
            lookupEntry->data.erspan2Data.ePortPolicerIndex;
    }
    else if (lookupEntry->type == XP_MIRROR_LOCAL)
    {
        mirrorData->spanData.ePort = lookupEntry->data.spanData.ePort;
        mirrorData->spanData.ePortPolicerIndex =
            lookupEntry->data.spanData.ePortPolicerIndex;
    }
    else
    {
        return XP_ERR_INVALID_DATA;
    }

    lookupEntry->data = *mirrorData;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMirrorRemoveAnalyzerSession(xpsDevice_t deviceId,
                                         uint32_t analyzerId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;
    xpsMirrorSessionDbEntry_t *lookupEntry = NULL;
    GT_STATUS cpssStatus;
    xpsScope_t scopeId;
    uint32_t policerIndex = 0;
    uint32_t analyzerIntf = 0;
    uint32_t ePort = 0;

    // Get scope from Device ID
    retVal = xpsScopeGetScopeId(deviceId, &scopeId);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    retVal = xpsMirrorDbGetAnalyzerData(scopeId, analyzerId, &lookupEntry);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    if (lookupEntry->type == XP_MIRROR_ERSPAN2)
    {
        analyzerIntf = lookupEntry->data.erspan2Data.analyzerIntfId,
        ePort = lookupEntry->data.erspan2Data.ePort;
        policerIndex = lookupEntry->data.erspan2Data.ePortPolicerIndex;

        if (cpssHalDevPPFamilyGet(deviceId) != CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            // Unbind Tunnel from ePort
            cpssStatus = cpssHalUnBindAnalyzerFromTunnel(deviceId, analyzerId, ePort);
            if (cpssStatus != GT_OK)
            {
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
        else
        {
            retVal = xpsMirrorErspanEnableFalcon(deviceId, analyzerId,
                                                 lookupEntry->data.erspan2Data.analyzerIntfId, false);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: xpsMirrorErspanEnableFalcon %d\n", retVal);
                return retVal;
            }

            cpssStatus = cpssHalMirrorErspanOAMFlowCounterClear(deviceId, analyzerId);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: cpssHalMirrorErspanOAMFlowCounterClear %d\n", cpssStatus);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }

            if (lookupEntry->data.erspan2Data.ipType == XP_PREFIX_TYPE_IPV6)
            {
                cpssStatus = cpssHalMirrorErspan2V6DummyTagClear(deviceId, ePort);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssHalMirrorErspan2V6DummyTagClear Failed ePort %d ret %d \n", ePort,
                          cpssStatus);
                    return xpsConvertCpssStatusToXPStatus(cpssStatus);
                }
            }
        }
    }
    else if (lookupEntry->type == XP_MIRROR_LOCAL)
    {
        analyzerIntf = lookupEntry->data.spanData.analyzerIntfId,
        ePort = lookupEntry->data.spanData.ePort;
        policerIndex = lookupEntry->data.spanData.ePortPolicerIndex;
    }
    else
    {
        return XP_ERR_INVALID_DATA;
    }

    // Unbind ePort from Analyzer
    cpssStatus = cpssHalAnalyzerPortClear(deviceId, analyzerId, analyzerIntf,
                                          ePort);
    if (cpssStatus != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    if (lookupEntry->type != XP_MIRROR_ERSPAN2)
    {
        retVal = xpsPolicerIndexRelease(XP_EGRESS_FLOW_POLICER, policerIndex);
        if (retVal != XP_NO_ERR)
        {
            XPS_FUNC_EXIT_LOG();
            return retVal;
        }
    }

    // Remove this analyzer from the mirror session db
    if ((retVal = xpsMirrorDbRemoveAnalyzer(scopeId, analyzerId)) != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    // Release Analyzer Id
    retVal = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_ANALYZER, analyzerId);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsMirrorGetRspanAnalyzerInterface(xpsDevice_t deviceId,
                                             xpsVlan_t vlanId, uint32_t *analyzerIntfId)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMirrorUpdLagMember(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                                xpsInterfaceId_t iface, bool removed,  uint32_t analyzerId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsMirrorUpdLagMember);

    XP_STATUS  retVal = XP_NO_ERR;
    GT_STATUS  cpssStatus;
    xpsScope_t scopeId;
    uint8_t    cpssDevNum;
    uint32_t   cpssPortNum;
    xpsMirrorSessionDbEntry_t *mirrorEntry = NULL;
    xpsMirrorSessionDbEntry_t dbEntry;
    dbEntry.analyzerId = analyzerId;

    /* Get Scope Id from devId */
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return retVal;
    }

    retVal = xpsStateSearchData(scopeId, mirrorSesDbHndl, (xpsDbKey_t)&dbEntry,
                                (void **)&mirrorEntry);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get data, return Value : %d", retVal);
        return retVal;
    }

    if (!mirrorEntry)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    if (lagIntf != mirrorEntry->data.erspan2Data.analyzerIntfId)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Monitor Intf is not lagIntf %d\n", lagIntf);
        return XP_ERR_INVALID_DATA;
    }

    if (mirrorEntry->type != XP_MIRROR_ERSPAN2)
    {
        return XP_NO_ERR;
    }

    cpssDevNum = xpsGlobalIdToDevId(devId, iface);
    cpssPortNum = xpsGlobalPortToPortnum(devId, iface);
    if (!removed)
    {
        if (cpssHalDevPPFamilyGet(devId) == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {

            retVal = xpsMirrorErspanEnableFalcon(devId, analyzerId, iface, true);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: xpsMirrorErspanEnableFalcon %d\n", retVal);
                return retVal;
            }
        }
        else
        {
            cpssStatus = cpssHalL3SetPortEgressMac(cpssDevNum, cpssPortNum,
                                                   mirrorEntry->data.erspan2Data.srcMacAddr);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set egress mac to port : %d \n", cpssPortNum);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }

            // set MAC and MTU profile for analyzer eport
            cpssStatus = cpssHalAnalyzerEPortMacSet(devId, iface,
                                                    mirrorEntry->data.erspan2Data.ePort);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set analyzer eport egress mac\n");
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }
    else
    {
        if (cpssHalDevPPFamilyGet(devId) == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            retVal = xpsMirrorErspanEnableFalcon(devId, analyzerId, iface, false);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: xpsMirrorErspanEnableFalcon %d\n", retVal);
                return retVal;
            }
        }
        else
        {
            cpssStatus = cpssHalL3RemoveEgressRouterMac(cpssDevNum, GT_FALSE, cpssPortNum);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set egress mac to port : %d \n", cpssPortNum);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMirrorErspanCfgSet(xpsDevice_t devId,
                                uint32_t analyzerId, xpsMirrorData_t *mirrorData)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS  retVal = XP_NO_ERR;
    xpsIpGreTunnelConfig_t  tunnelData;
    xpsMirrorSessionDbEntry_t *lookupEntry = NULL;
    xpsScope_t scopeId;
    GT_STATUS  cpssStatus;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    // Get scope from Device ID
    retVal = xpsScopeGetScopeId(devId, &scopeId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to get scopeId for device %d\n", devId);
        return retVal;
    }

    // Get Analyzer Data from DB
    retVal = xpsMirrorDbGetAnalyzerData(scopeId, analyzerId, &lookupEntry);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    if (lookupEntry->type != XP_MIRROR_ERSPAN2)
    {
        return retVal;
    }

    if ((cpssHalDevPPFamilyGet(devId) == CPSS_PP_FAMILY_DXCH_FALCON_E) ||
        (cpssHalDevPPFamilyGet(devId) == CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC erspanEntry;
        cpssOsMemSet(&erspanEntry, 0, sizeof(erspanEntry));

        COPY_MAC_ADDR_T(&erspanEntry.l2Info.macDa.arEther,
                        &mirrorData->erspan2Data.dstMacAddr);
        COPY_MAC_ADDR_T(&erspanEntry.l2Info.macSa.arEther,
                        &mirrorData->erspan2Data.srcMacAddr);

        erspanEntry.l2Info.tpid       = mirrorData->erspan2Data.vlanTpid;
        erspanEntry.l2Info.up         = mirrorData->erspan2Data.vlanPri;
        erspanEntry.l2Info.cfi        = mirrorData->erspan2Data.vlanCfi;
        erspanEntry.l2Info.vid        = mirrorData->erspan2Data.vlanId;

        if (mirrorData->erspan2Data.ipType == XP_PREFIX_TYPE_IPV4)
        {
            erspanEntry.protocol          = CPSS_IP_PROTOCOL_IPV4_E;
            erspanEntry.ipInfo.ipv4.dscp  = mirrorData->erspan2Data.tos >>
                                            MIRROR_SESSION_DSCP_SHIFT;
            erspanEntry.ipInfo.ipv4.flags = 0;
            erspanEntry.ipInfo.ipv4.ttl   = mirrorData->erspan2Data.ttlHopLimit;
            COPY_IPV4_ADDR_T((uint8_t*)&erspanEntry.ipInfo.ipv4.sipAddr.arIP[0],
                             (uint8_t*)&mirrorData->erspan2Data.srcIpAddr);
            COPY_IPV4_ADDR_T((uint8_t*)&erspanEntry.ipInfo.ipv4.dipAddr.arIP[0],
                             (uint8_t*)&mirrorData->erspan2Data.dstIpAddr);
        }
        else
        {
            erspanEntry.protocol             = CPSS_IP_PROTOCOL_IPV6_E;
            erspanEntry.ipInfo.ipv6.tc       = mirrorData->erspan2Data.tos;
            erspanEntry.ipInfo.ipv6.hopLimit = mirrorData->erspan2Data.ttlHopLimit;
            COPY_IPV6_ADDR_T((uint8_t*)&erspanEntry.ipInfo.ipv6.sipAddr.arIP[0],
                             (uint8_t*)&mirrorData->erspan2Data.srcIpAddr);
            COPY_IPV6_ADDR_T((uint8_t*)&erspanEntry.ipInfo.ipv6.dipAddr.arIP[0],
                             (uint8_t*)&mirrorData->erspan2Data.dstIpAddr);
        }

        if (mirrorData->erspan2Data.isIngress)
        {
            erspanEntry.ingressSessionId = analyzerId;
        }
        else
        {
            erspanEntry.egressSessionId = analyzerId;
        }

        if (cpssHalDevPPFamilyGet(devId) !=
            CPSS_PP_FAMILY_DXCH_AC5X_E) /* TODO: to be removed when CPSS supports ERSPAN PHA for AC5X */
        {
            cpssStatus = cpssHalPhaThreadInfoSet(devId, analyzerId, &erspanEntry);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed cpssHalPhaThreadInfoSet : %d \n", cpssStatus);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }
    else
    {
        memset(&tunnelData, 0, sizeof(tunnelData));
        tunnelData.protocol = mirrorData->erspan2Data.greProtoType;
        tunnelData.ttlHopLimit = mirrorData->erspan2Data.ttlHopLimit;
        tunnelData.dscp = mirrorData->erspan2Data.tos >> MIRROR_SESSION_DSCP_SHIFT;
        tunnelData.vlanId = mirrorData->erspan2Data.vlanId;
        tunnelData.vlanPri = mirrorData->erspan2Data.vlanPri;

        COPY_MAC_ADDR_T(&tunnelData.dstMacAddr, &mirrorData->erspan2Data.dstMacAddr);

        retVal = xpsErspanGreTunnelCfgSet(devId, lookupEntry->data.erspan2Data.erspanId,
                                          &tunnelData);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed xpsErspanGreTunnelCfgSet, return Value : %d", retVal);
            return retVal;
        }
    }

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsMirrorErspanTunCreate(xpsDevice_t devId,
                                   uint32_t analyzerId, xpsMirrorData_t *mirrorData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsMirrorErspanTunCreate);

    XP_STATUS retVal = XP_NO_ERR;
    xpsMirrorSessionDbEntry_t *lookupEntry = NULL;
    inetAddr_t localIp, remoteIp;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if ((cpssHalDevPPFamilyGet(devId) == CPSS_PP_FAMILY_DXCH_FALCON_E) ||
        (cpssHalDevPPFamilyGet(devId) == CPSS_PP_FAMILY_DXCH_AC5X_E))
    {
        // Pha is used for ERSPAN in falcon instead of TS
        return retVal;
    }

    // Get scope from Device ID
    retVal = xpsScopeGetScopeId(devId, &scopeId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to get scopeId for device %d\n", devId);
        return retVal;
    }

    // Get Analyzer Data from DB
    retVal = xpsMirrorDbGetAnalyzerData(scopeId, analyzerId, &lookupEntry);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    if (lookupEntry->type != XP_MIRROR_ERSPAN2)
    {
        return retVal;
    }

    remoteIp.type = localIp.type = mirrorData->erspan2Data.ipType;
    if (mirrorData->erspan2Data.ipType == XP_PREFIX_TYPE_IPV4)
    {
        COPY_IPV4_ADDR_T((uint8_t*)&localIp.addr,
                         (uint8_t*)&mirrorData->erspan2Data.srcIpAddr);
        COPY_IPV4_ADDR_T((uint8_t*)&remoteIp.addr,
                         (uint8_t*)&mirrorData->erspan2Data.dstIpAddr);
    }
    else
    {
        COPY_IPV6_ADDR_T((uint8_t*)&localIp.addr,
                         (uint8_t*)&mirrorData->erspan2Data.srcIpAddr);
        COPY_IPV6_ADDR_T((uint8_t*)&remoteIp.addr,
                         (uint8_t*)&mirrorData->erspan2Data.dstIpAddr);
    }

    retVal = xpsErspanGreCreateTunnelInterface(&localIp, &remoteIp,
                                               &lookupEntry->data.erspan2Data.erspanId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed in xpsErrspanGreCreateTunnelInterface with error %d \n",
              retVal);
        return retVal;
    }

    retVal = xpsErspanGreAddTunnelEntry(devId,
                                        lookupEntry->data.erspan2Data.erspanId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed in xpsErspanGreAddTunnelEntry with error %d\n",
              retVal);
        xpsErspanGreDestroyTunnelInterface(lookupEntry->data.erspan2Data.erspanId);
        return retVal;
    }

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsMirrorErspanTunRemove(xpsDevice_t devId,
                                   uint32_t analyzerId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsMirrorErspanTunRemove);

    XP_STATUS retVal = XP_NO_ERR;
    xpsMirrorSessionDbEntry_t *lookupEntry = NULL;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if (cpssHalDevPPFamilyGet(devId) == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        // Pha is used for ERSPAN in falcon instead of TS
        return retVal;
    }

    // Get scope from Device ID
    retVal = xpsScopeGetScopeId(devId, &scopeId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to get scopeId for device %d\n", devId);
        return retVal;
    }

    // Get Analyzer Data from DB
    retVal = xpsMirrorDbGetAnalyzerData(scopeId, analyzerId, &lookupEntry);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    if (lookupEntry->type != XP_MIRROR_ERSPAN2 ||
        lookupEntry->data.erspan2Data.erspanId == XPS_INTF_INVALID_ID)
    {
        return retVal;
    }

    retVal = xpsErspanGreRemoveTunnelEntry(devId,
                                           lookupEntry->data.erspan2Data.erspanId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to remove ERSPAN GRE tunnel entry %u with error %d\n",
              lookupEntry->data.erspan2Data.erspanId, retVal);
        return retVal;
    }

    retVal = xpsErspanGreDestroyTunnelInterface(
                 lookupEntry->data.erspan2Data.erspanId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to destroy ERSPAN GRE tunnel entry %u with error %d \n",
              lookupEntry->data.erspan2Data.erspanId, retVal);
        return retVal;
    }

    lookupEntry->data.erspan2Data.erspanId = XPS_INTF_INVALID_ID;

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsMirrorUpdateSessionId(xpsDevice_t devId, uint32_t analyzerId,
                                   bool isIngress)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;
    xpsMirrorSessionDbEntry_t *lookupEntry = NULL;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    XPS_LOCK(xpsMirrorUpdateSessionId);

    if (cpssHalDevPPFamilyGet(devId) == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC erspanEntry;
        cpssOsMemSet(&erspanEntry, 0, sizeof(erspanEntry));

        cpssStatus = cpssHalPhaThreadInfoGet(devId, analyzerId, &erspanEntry);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed cpssHalPhaThreadInfoGet : %d \n", cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
        erspanEntry.ingressSessionId = 0;
        erspanEntry.egressSessionId = 0;

        if (isIngress)
        {
            erspanEntry.ingressSessionId = analyzerId;
        }
        else
        {
            erspanEntry.egressSessionId = analyzerId;
        }

        cpssStatus = cpssHalPhaThreadInfoSet(devId, analyzerId, &erspanEntry);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed cpssHalPhaThreadInfoSet : %d \n", cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }


    /* Update mirror direction in xps DB */

    // Get scope from Device ID
    status = xpsScopeGetScopeId(devId, &scopeId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to get scopeId for device %d\n", devId);
        return status;
    }

    // Get Analyzer Data from DB
    status = xpsMirrorDbGetAnalyzerData(scopeId, analyzerId, &lookupEntry);
    if (status != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return status;
    }

    if (lookupEntry->type == XP_MIRROR_ERSPAN2)
    {
        lookupEntry->data.erspan2Data.isIngress = isIngress;
    }
    else
    {
        lookupEntry->data.spanData.isIngress = isIngress;
    }

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsMirrorVlanUpdateErSpan2Index(xpsDevice_t devId, uint32_t xpsIntfId,
                                          bool isIngress, bool enable)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;

    xpsInterfaceType_e intfType;
    xpsLagPortIntfList_t portList;
    xpsInterfaceId_t portIntf;
    uint8_t cpssDevNum;
    uint32_t cpssPortNum;
    uint32_t erSpanIdx;
    uint32_t i;
    xpsInterfaceId_t                        *intfList  = NULL;
    uint16_t                                numOfIntfs = 0;
    xpsInterfaceId_t intfId;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  srcInfoType =
        CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT tgtInfoType =
        CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT srcPortInfo;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT tgtPortInfo;

    XPS_LOCK(xpsMirrorUpdateErSpan2Index);

    memset(&portList, 0, sizeof(portList));

    if (cpssHalDevPPFamilyGet(devId) != CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        return XP_NO_ERR;
    }

    retVal = xpsVlanGetInterfaceList(devId, xpsIntfId, &intfList, &numOfIntfs);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to get interface list of vlan :%d error:%d \n", xpsIntfId, retVal);
        return retVal;
    }

    if (!numOfIntfs)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "no vlan member for vlan ::%d", xpsIntfId);
    }

    for (int i = 0; i < numOfIntfs; i++)
    {
        intfId = intfList[i];
        retVal = xpsInterfaceGetTypeScope(devId, intfId, &intfType);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Interface Get type for interface(%d) failed", intfId);
            return retVal;
        }

        if (intfType == XPS_LAG)
        {
            /* Get the Lag member port list */
            retVal = xpsLagGetPortIntfList(intfId, &portList);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to Lag port members, "
                      "device:%d, lag:%d, pcl:%d, error:%d\n ", devId, intfId, retVal);
                return retVal;
            }
        }
        else if (intfType == XPS_PORT)
        {
            portList.portIntf[portList.size] = intfId;
            (portList.size)++;
        }
    }


    for (i = 0; i < portList.size; i++)
    {
        portIntf = portList.portIntf[i];
        cpssDevNum = xpsGlobalIdToDevId(devId, portIntf);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portIntf);
        erSpanIdx = 0;
        /* Carry source/tgt port-num in index field of Erspan2 header. */
        if (enable)
        {
            erSpanIdx = cpssPortNum;
        }
        if (isIngress)
        {
            memset(&srcPortInfo, 0, sizeof(srcPortInfo));
            srcPortInfo.erspanSameDevMirror.erspanIndex = erSpanIdx;
            cpssStatus = cpssHalPhaSourcePortEntrySet(cpssDevNum, cpssPortNum, srcInfoType,
                                                      &srcPortInfo);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " cpssHalPhaSourcePortEntrySet Failed port : %d status %d \n",
                      cpssPortNum, cpssStatus);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
        else
        {
            memset(&tgtPortInfo, 0, sizeof(tgtPortInfo));
            tgtPortInfo.erspanSameDevMirror.erspanIndex = erSpanIdx;
            cpssStatus = cpssHalPhaTargetPortEntrySet(cpssDevNum, cpssPortNum, tgtInfoType,
                                                      &tgtPortInfo);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " cpssHalPhaTargetPortEntrySet Failed port : %d status %d \n",
                      cpssPortNum, cpssStatus);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }
    return retVal;
}

XP_STATUS xpsMirrorUpdateErSpan2Index(xpsDevice_t devId, uint32_t xpsIntfId,
                                      bool isIngress, bool enable)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;

    xpsInterfaceType_e intfType;
    xpsLagPortIntfList_t portList;
    xpsInterfaceId_t portIntf;
    uint8_t cpssDevNum;
    uint32_t cpssPortNum;
    uint32_t erSpanIdx;
    uint32_t i;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  srcInfoType =
        CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT tgtInfoType =
        CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT srcPortInfo;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT tgtPortInfo;

    XPS_LOCK(xpsMirrorUpdateErSpan2Index);
    if (cpssHalDevPPFamilyGet(devId) != CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        return XP_NO_ERR;
    }

    retVal = xpsInterfaceGetType(xpsIntfId, &intfType);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get type for interface %u, error: %d\n", xpsIntfId, retVal);
        return retVal;
    }

    memset(&portList, 0, sizeof(portList));
    if (XPS_LAG == intfType)
    {
        retVal = xpsLagGetPortIntfList(xpsIntfId, &portList);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed, lag interface(%d)", xpsIntfId);
            return retVal;
        }
    }
    else if (XPS_PORT == intfType)
    {
        portList.portIntf[0] = xpsIntfId;
        portList.size = 1;
    }

    for (i = 0; i < portList.size; i++)
    {
        portIntf = portList.portIntf[i];
        cpssDevNum = xpsGlobalIdToDevId(devId, portIntf);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portIntf);
        erSpanIdx = 0;
        /* Carry source/tgt port-num in index field of Erspan2 header. */
        if (enable)
        {
            erSpanIdx = cpssPortNum;
        }
        if (isIngress)
        {
            memset(&srcPortInfo, 0, sizeof(srcPortInfo));
            srcPortInfo.erspanSameDevMirror.erspanIndex = erSpanIdx;
            cpssStatus = cpssHalPhaSourcePortEntrySet(cpssDevNum, cpssPortNum, srcInfoType,
                                                      &srcPortInfo);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " cpssHalPhaSourcePortEntrySet Failed port : %d status %d \n",
                      cpssPortNum, cpssStatus);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
        else
        {
            memset(&tgtPortInfo, 0, sizeof(tgtPortInfo));
            tgtPortInfo.erspanSameDevMirror.erspanIndex = erSpanIdx;
            cpssStatus = cpssHalPhaTargetPortEntrySet(cpssDevNum, cpssPortNum, tgtInfoType,
                                                      &tgtPortInfo);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " cpssHalPhaTargetPortEntrySet Failed port : %d status %d \n",
                      cpssPortNum, cpssStatus);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }
    return retVal;
}


#ifdef __cplusplus
}
#endif
