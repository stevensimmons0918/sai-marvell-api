// xpsOpenflow.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsEnums.h"
#include "xpsUtil.h"
#include "xpsState.h"
#include "xpsInternal.h"
#include "xpsOpenflow.h"
#include "xpsInit.h"
#include "xpsInterface.h"
#include "xpsScope.h"
#include "xpsLock.h"
#include "xpsGlobalSwitchControl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief State structure maintained by XPS OpenFlow
 *
 * This state contains the relationship between a OpenFlow data path
 * and service ports that are related to it (these ports are xpOfPortNo_e to
 * Vif's mapping to perform translation from user selection to internal
 * used data)
 *
 * This state is internal to XPS and is not exposed to the user
 */
typedef struct xpsOfDpDbEntry
{
    // Key
    xpsDevice_t devId;
    xpDatapath_t dpId;

} xpsOfDpDbEntry;

/**
 * \brief State structure maintained by XPS OpenFlow
 *
 * This state contains the relationship between a table
 * and the table key that is used to specify matching fields
 * in OpenFlow table
 *
 * This state is internal to XPS and is not exposed to the user
 */
typedef struct xpsOfTableDbEntry
{
    // Key
    uint32_t tableId;

} xpsOfTableDbEntry;






/**
 * This enum appointed for indexing locks of each type of DB.
 */
typedef enum xpsLockIndex_e
{
    xpsOfGroupDb = 0,
    xpsOfDpDb,
    xpsOfTableDb,
    // Maximum lock index
    xpsOfMaxIndex

} xpsLockIndex_e;


typedef struct xpsOfGroupDbEntry
{
    // Key
    uint32_t groupId;
    uint32_t subGroupId;
    uint32_t shadow;

    // Data
    xpOfGroupCtx_t groupCtx;
} xpsOfGroupDbEntry;














/* Following APIs for OpenFlow data path state maintenance */









/* Following APIs for OpenFlow table state maintenance */






















XP_STATUS xpsOfAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfSetPortEnable(xpsDevice_t devId, xpOfPortNo_e ofppPort,
                             xpsInterfaceId_t portIntf, uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfGetPortEnable(xpsDevice_t devId, xpOfPortNo_e ofppPort,
                             xpsInterfaceId_t portIntf, uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfUpdateMemoryAddressInOfGroupDbEntry(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfGroupInit(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfGroupDeInit(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOpenflowInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOpenflowInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOpenflowDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOpenflowDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}



XP_STATUS xpsOfCreateTable(xpsDevice_t devId, xpOpenFlowTableProfile_t *tblInfo)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfDeleteTable(xpsDevice_t devId,
                           const xpOpenFlowTableProfile_t *tblInfo)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfSetTableKeyFormat(xpsDevice_t devId, uint32_t tableId,
                                 xpOfTableType_e tableType,
                                 const xpOxmType_e *keyList, uint32_t keyListLen)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfEnableTableLookup(xpsDevice_t devId, uint32_t tableId,
                                 xpOfTableType_e tableType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfDisableTableLookup(xpsDevice_t devId, uint32_t tableId,
                                  xpOfTableType_e tableType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfApplyActions(xpsDevice_t devId, xpDatapath_t dpId,
                            uint32_t inPort, uint8_t* packet,
                            uint32_t packetLen, xpOfAction_t* actionList, uint32_t actionListLen)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfGetTableStats(xpsDevice_t devId, xpDatapath_t dpId,
                             uint8_t tableId, xpOfTableStats_t *tableStats)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfAddFlow(xpsDevice_t devId, xpDatapath_t dpId,
                       xpFlowEntry_t *flowEntry, xpsOfFlowIdList_t *flowIdList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfModifyFlowById(xpsDevice_t devId, xpDatapath_t dpId,
                              uint8_t tableId, const xpOfFlowId_t *flowId,
                              const xpOfInstructions_t *instructions)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfModifyFlow(xpsDevice_t devId, xpDatapath_t dpId,
                          const xpFlowEntry_t *flowEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfRemoveFlowById(xpsDevice_t devId, xpDatapath_t dpId,
                              uint8_t tableId, const xpOfFlowId_t *flowId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfRemoveFlow(xpsDevice_t devId, xpDatapath_t dpId,
                          const xpFlowEntry_t *flowEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfGetFlowStats(xpsDevice_t devId, xpDatapath_t dpId,
                            uint8_t tableId, const xpOfFlowId_t *flowId, xpOfStats_t *flowStats)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfRegisterPacketInHandler(xpsDevice_t devId, xpDatapath_t dpId,
                                       xpOfPacketInHandler func, void *userData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfUnregisterPacketInHandler(xpsDevice_t devId, xpDatapath_t dpId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfRegisterFlowStatsFlushHandler(xpOfFlowStatsFlushHandler func,
                                             void *userData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfUnregisterFlowStatsFlushHandler(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfGetOfppLocalIntfId(xpsDevice_t devId, xpDatapath_t dpId,
                                  xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfGetOfppControllerIntfId(xpsDevice_t devId, xpDatapath_t dpId,
                                       xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}




XP_STATUS xpsOfAddGroup(xpsDevice_t devId, xpDatapath_t dpId, uint32_t groupId,
                        xpOfGroupType_e groupType, xpOfBucket_t* buckets, uint32_t bucketsNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfSwitchGroupShadowState(xpsScope_t scopeId, uint32_t groupId,
                                      uint32_t subGroupId, uint32_t shadow)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfModifyGroup(xpsDevice_t devId, xpDatapath_t dpId,
                           uint32_t groupId, xpOfGroupType_e groupType,
                           xpOfBucket_t* buckets, uint32_t bucketsNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}



XP_STATUS xpsOfRemoveGroup(xpsDevice_t devId, xpDatapath_t dpId,
                           uint32_t groupId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfGetGroupStats(xpsDevice_t devId, xpDatapath_t dpId,
                             uint32_t groupId, xpOfStats_t *groupStats)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfGetBucketStats(xpsDevice_t devId, xpDatapath_t dpId,
                              uint32_t groupId, uint32_t bucketsNum, xpOfStats_t *bucketStats)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfDisplayTable(xpDevice_t devId, xpDatapath_t dpId,
                            uint32_t tableId, xpOfTableType_e tableType,
                            uint32_t *numOfValidEntries, uint32_t startIndex, uint32_t endIndex,
                            char * logFile, uint32_t detailFormat, uint32_t silentMode, uint8_t tblCopyIdx)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfSetRehashLevel(xpDevice_t devId, uint32_t tableId,
                              uint8_t numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsOfGetRehashLevel(xpDevice_t devId, uint32_t tableId,
                              uint8_t* numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
