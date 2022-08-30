// xpsTcamListShuffling.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsTcamMgr.h"
#include "xpsTcamListShuffling.h"
#include "xpsLock.h"
#include "xpsScope.h"

#ifdef __cplusplus
extern "C" {
#endif
static const uint32_t XPS_TCAM_INVALID_ENTRY_ID = 0xFFFFFFFF;
static const uint32_t XPS_TCAM_ENTRIES_MAX = 6 * 1024;

typedef struct xpsTcamGroupEntry_t
{
    int32_t prev;           // Previous group ID
    int32_t next;           // Next group ID
    uint32_t firstTcamId;   // First TCAM entry ID of this group
    uint32_t lastTcamId;    // Last TCAM entry ID of this group
    uint32_t priority;      // Priority level
} xpsTcamGroupEntry_t;

typedef struct xpsTcamTableEntry_t
{
    int32_t group;
    uint32_t logicalEntryId;
} xpsTcamTableEntry_t;

typedef struct xpsTcamLogicalEntry_t
{
    uint32_t tcamEntryId;
    int32_t next;
} xpsTcamLogicalEntry_t;

/**
 * \struct xpsTcamMgrAclShufflingKeyFormatDbData_t
 *         This structure maintains mapping between tableId -> TCAM Mgr related data
 *
 */
typedef struct xpsTcamListShufflingDbEntry_t
{
    // Key
    uint32_t tableId;

    // Data
    uint32_t prioMax;           // Max. priority for this TCAM table
    uint32_t size;              // Max. number of entries

    int32_t groupHead;          // The beginning of TCAM group list

    int32_t logicalEntryPool;   // Points to the first free TCAM logical entry
    int32_t groupEntryPool;     // Points to the first free TCAM group entry

    xpsTcamTableEntry_t phyEntry[XPS_TCAM_ENTRIES_MAX];
    xpsTcamLogicalEntry_t logicalEntry[XPS_TCAM_ENTRIES_MAX];
    xpsTcamGroupEntry_t groupEntry[XPS_TCAM_ENTRIES_MAX];

    xpsTcamMgrRuleMove_fn ruleMoveFunc;

} xpsTcamListShufflingDbEntry_t;

static xpsDbHandle_t tcamMgrListShufflingCtxDbHndl =
    XPS_STATE_INVALID_DB_HANDLE;



static int32_t listShufflingKeyComp(void* key1, void* key2)
{
    /* Key is dependent on tableId */
    return (*(uint32_t *)key1 - *(uint32_t *)key2);
}

XP_STATUS xpsTcamListShufflingConfigTable(xpsDevice_t devId, uint32_t tableId,
                                          uint32_t entryNum, uint32_t prioNum, xpsTcamMgrRuleMove_fn ruleMove)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsTcamListShufflingDbEntry_t *nodeEntry = NULL;
    xpsScope_t scopeId;

    //get scope from Device ID
    retVal = xpsScopeGetScopeId(devId, &scopeId);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    if (tcamMgrListShufflingCtxDbHndl == XPS_STATE_INVALID_DB_HANDLE)
    {
        tcamMgrListShufflingCtxDbHndl = XPS_TCAM_MGR_LIST_SHUFFLING_DB_HNDL;
        retVal = xpsStateRegisterDb(scopeId, "TCAM List-based Shuffling",
                                    XPS_PER_DEVICE, &listShufflingKeyComp, tcamMgrListShufflingCtxDbHndl);
        if (retVal != XP_NO_ERR)
        {
            tcamMgrListShufflingCtxDbHndl = XPS_STATE_INVALID_DB_HANDLE;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: Failed to register state database\n");
            return retVal;
        }

        retVal = xpsStateInsertDbForDevice(devId, tcamMgrListShufflingCtxDbHndl,
                                           listShufflingKeyComp);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: Failed to add DB for device %u \n", devId);
            return retVal;
        }
    }

    /* Search corresponding table API */
    retVal = xpsStateSearchDataForDevice(devId, tcamMgrListShufflingCtxDbHndl,
                                         (xpsDbKey_t)&tableId, (void**)&nodeEntry);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    if (nodeEntry == NULL)
    {
        if (entryNum > XPS_TCAM_ENTRIES_MAX)
        {
            retVal = XP_ERR_INVALID_PARAMS;
            return retVal;
        }

        retVal = xpsStateHeapMalloc(sizeof(xpsTcamListShufflingDbEntry_t),
                                    (void**)&nodeEntry);
        if (retVal == XP_NO_ERR)
        {
            memset(nodeEntry, 0, sizeof(xpsTcamListShufflingDbEntry_t));

            nodeEntry->tableId = tableId;
            nodeEntry->prioMax = prioNum - 1;
            nodeEntry->size = entryNum;
            nodeEntry->ruleMoveFunc = ruleMove;
            nodeEntry->groupHead = -1;

            for (uint32_t i = 0; i < entryNum; i++)
            {
                nodeEntry->phyEntry[i].group = -1;
                nodeEntry->phyEntry[i].logicalEntryId = 0;

                /* Create a chain from TCAM logical entries */
                nodeEntry->logicalEntry[i].next = i + 1;
                nodeEntry->logicalEntry[i].tcamEntryId = XPS_TCAM_INVALID_ENTRY_ID;

                /* Create a chain from TCAM group entries */
                nodeEntry->groupEntry[i].next = i + 1;
                nodeEntry->groupEntry[i].prev = -1;
            }
            /* The last entry in the chain must point to the invalid entry (-1) */
            nodeEntry->logicalEntry[entryNum - 1].next = -1;
            nodeEntry->groupEntry[entryNum - 1].next = -1;

            /* Point to the first entry in the chain */
            nodeEntry->groupEntryPool = 0;
            nodeEntry->logicalEntryPool = 0;

            /* Insert TCAM Mgr List Shuffling key format structure into DB, use tableId as a key */
            retVal = xpsStateInsertDataForDevice(devId, tcamMgrListShufflingCtxDbHndl,
                                                 (void*)nodeEntry);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: Failed to add DB to the device %u \n", devId);
                xpsStateHeapFree((void *)nodeEntry);
                return retVal;
            }
        }
    }

    return retVal;
}

XP_STATUS xpsTcamListShufflingDeleteTable(xpsDevice_t devId, uint32_t tableId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsTcamListShufflingDbEntry_t *nodeEntry = NULL;

    if (tcamMgrListShufflingCtxDbHndl == XPS_STATE_INVALID_DB_HANDLE)
    {
        return XP_ERR_NOT_INITED;
    }

    retVal = xpsStateDeleteDataForDevice(devId, tcamMgrListShufflingCtxDbHndl,
                                         (xpsDbKey_t)&tableId, (void**)&nodeEntry);
    if (retVal != XP_NO_ERR)
    {
        //LOGFN()
        return retVal;
    }

    retVal = xpsStateHeapFree((void*)nodeEntry);
    if (retVal != XP_NO_ERR)
    {
        //LOGFN()
        return retVal;
    }

    return retVal;
}

XP_STATUS xpsTcamListShufflingGetTcamId(xpsDevice_t devId, uint32_t tableId,
                                        uint32_t entryId, uint32_t *tcamId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsTcamListShufflingDbEntry_t *nodeEntry = NULL;

    if (tcamId == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Invalid TCAM entry ID pointer\n");
        return XP_ERR_NULL_POINTER;
    }


    retVal = xpsStateSearchDataForDevice(devId, tcamMgrListShufflingCtxDbHndl,
                                         (xpsDbKey_t)&tableId, (void**)&nodeEntry);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    if (nodeEntry == NULL)
    {
        retVal = XP_ERR_KEY_NOT_FOUND;
        return retVal;
    }

    if (entryId >= nodeEntry->size)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid entry ID\n");
        retVal = XP_ERR_INVALID_PARAMS;
        return retVal;
    }

    *tcamId = nodeEntry->logicalEntry[entryId].tcamEntryId;

    return retVal;
}

XP_STATUS xpsTcamListShufflingGetEntryId(xpsDevice_t devId, uint32_t tableId,
                                         uint32_t tcamId, uint32_t *entryId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsTcamListShufflingDbEntry_t *nodeEntry = NULL;

    if (entryId == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Invalid Entry ID pointer\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsStateSearchDataForDevice(devId, tcamMgrListShufflingCtxDbHndl,
                                         (xpsDbKey_t)&tableId, (void**)&nodeEntry);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    if (nodeEntry == NULL)
    {
        retVal = XP_ERR_KEY_NOT_FOUND;
        return retVal;
    }

    if (tcamId >= nodeEntry->size)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid TCAM ID\n");
        retVal = XP_ERR_INVALID_PARAMS;
        return retVal;
    }

    *entryId = nodeEntry->phyEntry[tcamId].logicalEntryId;

    return retVal;
}

static xpsTcamGroupEntry_t *allocTcamGroupEntry(xpsTcamListShufflingDbEntry_t
                                                *nodeEntry)
{
    xpsTcamGroupEntry_t *group;

    if (nodeEntry == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Invalid state node entry pointer\n");
        return NULL;
    }

    if (nodeEntry->groupEntryPool == -1)
    {
        // No free group entry left
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: No free group entry left\n");
        return NULL;
    }

    group = &nodeEntry->groupEntry[(uint32_t)nodeEntry->groupEntryPool];
    nodeEntry->groupEntryPool = group->next;

    return group;
}

static void freeTcamGroupEntry(xpsTcamListShufflingDbEntry_t *nodeEntry,
                               xpsTcamGroupEntry_t *group)
{
    if (nodeEntry == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Invalid state node entry pointer\n");
        return;
    }

    if (group == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Invalid group entry pointer\n");
        return;
    }

    group->next = nodeEntry->groupEntryPool;
    nodeEntry->groupEntryPool = group - nodeEntry->groupEntry;
}

static uint32_t allocTcamLogicalEntryId(xpsTcamListShufflingDbEntry_t
                                        *nodeEntry)
{
    uint32_t entryId;

    if (nodeEntry == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Invalid state node entry pointer\n");
        return XPS_TCAM_INVALID_ENTRY_ID;
    }

    if (nodeEntry->logicalEntryPool == -1)
    {
        // No free group entry left
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: No free logical entry left\n");
        return XPS_TCAM_INVALID_ENTRY_ID;
    }

    entryId = (uint32_t)nodeEntry->logicalEntryPool;
    nodeEntry->logicalEntryPool = nodeEntry->logicalEntry[entryId].next;

    return entryId;
}

static void freeTcamLogicalEntryId(xpsTcamListShufflingDbEntry_t *nodeEntry,
                                   uint32_t entryId)
{
    if (nodeEntry == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Invalid state node entry pointer\n");
        return;
    }

    nodeEntry->logicalEntry[entryId].next = nodeEntry->logicalEntryPool;
    nodeEntry->logicalEntryPool = (int32_t)entryId;
}

static XP_STATUS copyTcamEntry(xpsDevice_t devId,
                               xpsTcamListShufflingDbEntry_t *nodeEntry, uint32_t dst, uint32_t src)
{
    XP_STATUS status = XP_NO_ERR;
    uint32_t logicalEntryId;

    status = nodeEntry->ruleMoveFunc(devId, nodeEntry->tableId, dst, src);
    if (status == XP_NO_ERR)
    {
        nodeEntry->phyEntry[dst].logicalEntryId =
            nodeEntry->phyEntry[src].logicalEntryId;
        nodeEntry->phyEntry[dst].group = nodeEntry->phyEntry[src].group;
        logicalEntryId = nodeEntry->phyEntry[src].logicalEntryId;
        nodeEntry->logicalEntry[logicalEntryId].tcamEntryId = dst;
    }
    return status;
}

static XP_STATUS shiftTcamGroupLeft(xpsDevice_t devId,
                                    xpsTcamListShufflingDbEntry_t *nodeEntry, xpsTcamGroupEntry_t *group,
                                    uint32_t shift)
{
    XP_STATUS status = XP_NO_ERR;
    uint32_t dst, src;
    uint32_t cnt;

    if (group == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    dst = group->firstTcamId - shift;
    cnt = (shift < (group->lastTcamId - group->firstTcamId + 1)) ?
          shift : (group->lastTcamId - group->firstTcamId + 1);

    for (uint32_t i = 0; i < cnt; i++)
    {
        src = group->lastTcamId - i;
        status = copyTcamEntry(devId, nodeEntry, dst, src);
        if (status != XP_NO_ERR)
        {
            return status;
        }
        ++dst;
    }
    group->firstTcamId -= shift;
    group->lastTcamId -= shift;
    return status;
}

static XP_STATUS shiftTcamGroupRight(xpsDevice_t devId,
                                     xpsTcamListShufflingDbEntry_t *nodeEntry, xpsTcamGroupEntry_t *group,
                                     uint32_t shift)
{
    XP_STATUS status = XP_NO_ERR;
    uint32_t dst, src;
    uint32_t cnt;

    if (group == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    dst = group->lastTcamId + shift;
    cnt = (shift < (group->lastTcamId - group->firstTcamId + 1)) ?
          shift : (group->lastTcamId - group->firstTcamId + 1);

    for (uint32_t i = 0; i < cnt; i++)
    {
        src = group->firstTcamId + i;
        status = copyTcamEntry(devId, nodeEntry, dst, src);
        if (status != XP_NO_ERR)
        {
            return status;
        }
        --dst;
    }
    group->lastTcamId += shift;
    group->firstTcamId += shift;
    return status;
}

static void findShortestShift(xpsTcamListShufflingDbEntry_t *nodeEntry,
                              xpsTcamGroupEntry_t *group,
                              xpsTcamGroupEntry_t **shiftGroup,
                              bool *leftShift, uint32_t *freeEntries)
{
    int32_t prev = group->prev;
    int32_t next = group->next;
    uint32_t leftEntryNum = 0;
    uint32_t rightEntryNum = 0;
    uint32_t tailRoom = 0;
    uint32_t headRoom = 0;

    while ((prev != -1) || (next != -1))
    {
        if ((prev != -1) && (headRoom == 0))
        {
            xpsTcamGroupEntry_t *prevGrp = &nodeEntry->groupEntry[(uint32_t)prev];
            headRoom = (prevGrp->prev != -1) ?
                       (prevGrp->firstTcamId - nodeEntry->groupEntry[(uint32_t)
                                                                     prevGrp->prev].lastTcamId - 1) :
                       (prevGrp->firstTcamId);
            leftEntryNum += prevGrp->lastTcamId - prevGrp->firstTcamId + 1;
            prev = headRoom ? prev : prevGrp->prev;
        }

        if ((next != -1) && (tailRoom == 0))
        {
            xpsTcamGroupEntry_t *nextGrp = &nodeEntry->groupEntry[(uint32_t)next];
            tailRoom = (nextGrp->next != -1) ?
                       (nodeEntry->groupEntry[(uint32_t)nextGrp->next].firstTcamId -
                        nextGrp->lastTcamId - 1) :
                       (nodeEntry->size - nextGrp->lastTcamId - 1);
            rightEntryNum += nextGrp->lastTcamId - nextGrp->firstTcamId + 1;
            next = tailRoom ? next : nextGrp->next;
        }

        if (headRoom && (leftEntryNum <= rightEntryNum || next == -1))
        {
            *leftShift = true;
            *freeEntries = headRoom;
            *shiftGroup = &nodeEntry->groupEntry[(uint32_t)prev];
            return;
        }
        else if (tailRoom && (rightEntryNum <= leftEntryNum || prev == -1))
        {
            *leftShift = false;
            *freeEntries = tailRoom;
            *shiftGroup = &nodeEntry->groupEntry[(uint32_t)next];
            return;
        }
    }
    *leftShift = false;
    *shiftGroup = NULL;
}

static void mergeTcamGroups(xpsTcamListShufflingDbEntry_t *nodeEntry,
                            xpsTcamGroupEntry_t *group, xpsTcamGroupEntry_t *victim)
{
    // Update TCAM entries to point to the new group
    for (uint32_t i = victim->firstTcamId; i <= victim->lastTcamId; i++)
    {
        nodeEntry->phyEntry[i].group = group - nodeEntry->groupEntry;
    }

    group->lastTcamId = victim->lastTcamId;
    group->next = victim->next;
    if (group->next != -1)
    {
        nodeEntry->groupEntry[(uint32_t)group->next].prev = group -
                                                            nodeEntry->groupEntry;
    }

    freeTcamGroupEntry(nodeEntry, victim);
}

static XP_STATUS shiftTcamGroup(xpsDevice_t devId,
                                xpsTcamListShufflingDbEntry_t *nodeEntry, xpsTcamGroupEntry_t *group,
                                uint32_t *headRoom, uint32_t *tailRoom)
{
    XP_STATUS status = XP_NO_ERR;
    xpsTcamGroupEntry_t *shiftGroup;
    xpsTcamGroupEntry_t *head = NULL;
    bool leftShift;
    uint32_t freeEntries;

    findShortestShift(nodeEntry, group, &shiftGroup, &leftShift, &freeEntries);
    if (!shiftGroup)
    {
        return XP_ERR_RESOURCE_NOT_AVAILABLE;
    }

    head = shiftGroup;
    if (leftShift)
    {
        while (shiftGroup != group)
        {
            status = shiftTcamGroupLeft(devId, nodeEntry, shiftGroup, 1);
            if (status != XP_NO_ERR)
            {
                return status;
            }
            shiftGroup = (shiftGroup->next != -1) ?
                         &nodeEntry->groupEntry[(uint32_t)shiftGroup->next] : NULL;
        }
        *headRoom = 1;
        *tailRoom = 0;

        if (head->prev != -1)
        {
            xpsTcamGroupEntry_t *prevGrp = &nodeEntry->groupEntry[(uint32_t)head->prev];

            // Merge adjacent groups with equal priorities
            if ((prevGrp->priority == head->priority) &&
                (prevGrp->lastTcamId + 1 == head->firstTcamId))
            {
                mergeTcamGroups(nodeEntry, prevGrp, head);
            }
        }
    }
    else
    {
        while (shiftGroup != group)
        {
            status = shiftTcamGroupRight(devId, nodeEntry, shiftGroup, 1);
            if (status != XP_NO_ERR)
            {
                return status;
            }
            shiftGroup = (shiftGroup->prev != -1) ?
                         &nodeEntry->groupEntry[(uint32_t)shiftGroup->prev] : NULL;
        }
        *headRoom = 0;
        *tailRoom = 1;

        if (head->next != -1)
        {
            xpsTcamGroupEntry_t *nextGrp = &nodeEntry->groupEntry[(uint32_t)head->next];

            // Merge adjacent groups with equal priorities
            if ((head->priority == nextGrp->priority) &&
                (head->lastTcamId + 1 == nextGrp->firstTcamId))
            {
                mergeTcamGroups(nodeEntry, head, nextGrp);
            }
        }
    }
    return status;
}

static void addEntryAfter(xpsTcamListShufflingDbEntry_t *nodeEntry,
                          xpsTcamGroupEntry_t *group, uint32_t *entryId)
{
    *entryId = allocTcamLogicalEntryId(nodeEntry);
    if (*entryId != XPS_TCAM_INVALID_ENTRY_ID)
    {
        ++(group->lastTcamId);
        nodeEntry->phyEntry[group->lastTcamId].logicalEntryId = *entryId;
        nodeEntry->phyEntry[group->lastTcamId].group = group - nodeEntry->groupEntry;
        nodeEntry->logicalEntry[*entryId].tcamEntryId = group->lastTcamId;
    }
}

static void addEntryBefore(xpsTcamListShufflingDbEntry_t *nodeEntry,
                           xpsTcamGroupEntry_t *group, uint32_t *entryId)
{
    *entryId = allocTcamLogicalEntryId(nodeEntry);
    if (*entryId != XPS_TCAM_INVALID_ENTRY_ID)
    {
        --(group->firstTcamId);
        nodeEntry->phyEntry[group->firstTcamId].logicalEntryId = *entryId;
        nodeEntry->phyEntry[group->firstTcamId].group = group - nodeEntry->groupEntry;
        nodeEntry->logicalEntry[*entryId].tcamEntryId = group->firstTcamId;
    }
}

static XP_STATUS addEntry(xpsDevice_t devId,
                          xpsTcamListShufflingDbEntry_t *nodeEntry, xpsTcamGroupEntry_t *group,
                          uint32_t *entryId, bool newGroup)
{
    XP_STATUS status = XP_NO_ERR;
    uint32_t tailRoom = 0;
    uint32_t headRoom = 0;

    if (!newGroup)
    {
        if (group->next != -1)
        {
            xpsTcamGroupEntry_t *nextGrp = &nodeEntry->groupEntry[(uint32_t)group->next];

            if (nextGrp->priority == group->priority)
            {
                // Add new entry to the end of the group
                addEntryAfter(nodeEntry, group, entryId);
                if (*entryId == XPS_TCAM_INVALID_ENTRY_ID)
                {
                    return XP_ERR_RESOURCE_NOT_AVAILABLE;
                }

                // Merge adjacent groups with equal priorities
                if (group->lastTcamId + 1 == nextGrp->firstTcamId)
                {
                    mergeTcamGroups(nodeEntry, group, nextGrp);
                }
                return status;
            }
        }

        tailRoom = (group->next != -1) ?
                   (nodeEntry->groupEntry[(uint32_t)group->next].firstTcamId - group->lastTcamId -
                    1) :
                   (nodeEntry->size - group->lastTcamId - 1);

        headRoom = (group->prev != -1) ?
                   (group->firstTcamId - nodeEntry->groupEntry[(uint32_t)group->prev].lastTcamId -
                    1) :
                   (group->firstTcamId);

        // Check for free space and allocate if needed
        if (!tailRoom && !headRoom)
        {
            status = shiftTcamGroup(devId, nodeEntry, group, &headRoom, &tailRoom);
            if (status != XP_NO_ERR)
            {
                return status;
            }
        }

        // Add entry to the group
        if (tailRoom >= headRoom)
        {
            addEntryAfter(nodeEntry, group, entryId);
        }
        else
        {
            addEntryBefore(nodeEntry, group, entryId);
        }

        if (*entryId == XPS_TCAM_INVALID_ENTRY_ID)
        {
            return XP_ERR_RESOURCE_NOT_AVAILABLE;
        }
    }
    else
    {
        uint32_t entryIdx = 0;
        uint32_t priority = nodeEntry->prioMax;
        uint32_t numOfEntries = nodeEntry->size;
        uint32_t numOfPrio = nodeEntry->prioMax + 1;

        if (group->prev != -1)
        {
            xpsTcamGroupEntry_t *prevGrp = &nodeEntry->groupEntry[(uint32_t)group->prev];

            numOfPrio = prevGrp->priority;
            numOfEntries -= (prevGrp->lastTcamId + 1);
            priority = prevGrp->priority - 1;
            entryIdx = prevGrp->lastTcamId + 1;
        }

        if (group->next != -1)
        {
            xpsTcamGroupEntry_t *nextGrp = &nodeEntry->groupEntry[(uint32_t)group->next];

            numOfPrio -= (nextGrp->priority + 1);
            numOfEntries -= (nodeEntry->size - nextGrp->firstTcamId);
        }

        if (numOfEntries == 0)
        {
            status = shiftTcamGroup(devId, nodeEntry, group, &headRoom, &tailRoom);
            if (status != XP_NO_ERR)
            {
                return status;
            }

            entryIdx = tailRoom ?
                       (nodeEntry->groupEntry[(uint32_t)group->next].firstTcamId - 1) :
                       (nodeEntry->groupEntry[(uint32_t)group->prev].lastTcamId + 1);
        }
        else
        {
            priority -= group->priority;
            if (numOfEntries >= numOfPrio)
            {
                uint32_t entriesPerPriority = numOfEntries / numOfPrio;
                entryIdx += priority * entriesPerPriority;
                entryIdx += (entriesPerPriority - 1) / 2;
            }
            else
            {
                uint32_t prioritiesPerEntry = numOfPrio / numOfEntries;
                if (numOfPrio % numOfEntries)
                {
                    ++prioritiesPerEntry;

                    if (priority > prioritiesPerEntry * (numOfPrio % numOfEntries))
                    {
                        entryIdx += (numOfPrio % numOfEntries);
                        priority -= prioritiesPerEntry * (numOfPrio % numOfEntries);
                        --prioritiesPerEntry;
                    }
                }

                entryIdx += priority / prioritiesPerEntry;
            }
        }

        *entryId = allocTcamLogicalEntryId(nodeEntry);
        if (*entryId == XPS_TCAM_INVALID_ENTRY_ID)
        {
            return XP_ERR_RESOURCE_NOT_AVAILABLE;
        }

        nodeEntry->phyEntry[entryIdx].logicalEntryId = *entryId;
        nodeEntry->phyEntry[entryIdx].group = group - nodeEntry->groupEntry;
        nodeEntry->logicalEntry[*entryId].tcamEntryId = entryIdx;
        group->firstTcamId = entryIdx;
        group->lastTcamId = entryIdx;
    }

    return status;
}

XP_STATUS xpsTcamListShufflingAddEntry(xpsDevice_t devId, uint32_t tableId,
                                       uint32_t priority, uint32_t* entryId)
{
    XP_STATUS status = XP_NO_ERR;
    xpsTcamListShufflingDbEntry_t *nodeEntry = NULL;
    xpsTcamGroupEntry_t *group;
    xpsTcamGroupEntry_t *prevGroup = NULL;
    xpsTcamGroupEntry_t *newGroup = NULL;
    bool newGroupCreated = false;

    status = xpsStateSearchDataForDevice(devId, tcamMgrListShufflingCtxDbHndl,
                                         (xpsDbKey_t)&tableId, (void**)&nodeEntry);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    if (nodeEntry == NULL)
    {
        status = XP_ERR_KEY_NOT_FOUND;
        return status;
    }

    if (priority > nodeEntry->prioMax)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Too big entry priority %u. Max value %u.\n", priority,
              nodeEntry->prioMax);
        status = XP_ERR_INVALID_PARAMS;
        return status;
    }
    group = (nodeEntry->groupHead != -1) ?
            &nodeEntry->groupEntry[(uint32_t)nodeEntry->groupHead] :
            NULL;

    while (group)
    {
        if (group->priority == priority)
        {
            status = addEntry(devId, nodeEntry, group, entryId, false);
            return status;
        }
        else if (priority > group->priority)
        {
            // Add new group before current group
            newGroup = allocTcamGroupEntry(nodeEntry);
            if (newGroup == NULL)
            {
                status = XP_ERR_RESOURCE_NOT_AVAILABLE;
                return status;
            }

            newGroup->prev = group->prev;
            newGroup->next = group - nodeEntry->groupEntry;
            if (group->prev != -1)
            {
                nodeEntry->groupEntry[(uint32_t)group->prev].next = newGroup -
                                                                    nodeEntry->groupEntry;
                group->prev = newGroup - nodeEntry->groupEntry;
            }
            else
            {
                nodeEntry->groupHead = newGroup - nodeEntry->groupEntry;
                group->prev = newGroup - nodeEntry->groupEntry;
            }
            newGroup->priority = priority;
            group = newGroup;
            newGroupCreated = true;
            break;
        }
        else
        {
            prevGroup = group;
            group = (group->next != -1) ? &nodeEntry->groupEntry[(uint32_t)group->next] :
                    NULL;
        }
    }

    if (group == NULL)
    {
        // Add new group to the list tail
        group = allocTcamGroupEntry(nodeEntry);
        if (group == NULL)
        {
            status = XP_ERR_RESOURCE_NOT_AVAILABLE;
            return status;
        }

        group->next = -1;
        group->priority = priority;
        if (prevGroup == NULL)
        {
            group->prev = -1;
            nodeEntry->groupHead = group - nodeEntry->groupEntry;
        }
        else
        {
            group->prev = prevGroup - nodeEntry->groupEntry;
            prevGroup->next = group - nodeEntry->groupEntry;
        }
        newGroupCreated = true;
    }

    status = addEntry(devId, nodeEntry, group, entryId, true);
    if (status != XP_NO_ERR && newGroupCreated)
    {
        // Remove empty group
        if (group->prev != -1)
        {
            nodeEntry->groupEntry[(uint32_t)group->prev].next = group->next;
        }
        else
        {
            nodeEntry->groupHead = group->next;
        }

        if (group->next != -1)
        {
            nodeEntry->groupEntry[(uint32_t)group->next].prev = group->prev;
        }
        freeTcamGroupEntry(nodeEntry, group);
    }

    return status;
}

XP_STATUS xpsTcamListShufflingDeleteEntry(xpsDevice_t devId, uint32_t tableId,
                                          uint32_t entryId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsTcamGroupEntry_t *group;
    xpsTcamGroupEntry_t *newGroup;
    uint32_t tcamEntryId;
    xpsTcamListShufflingDbEntry_t *nodeEntry = NULL;

    retVal = xpsStateSearchDataForDevice(devId, tcamMgrListShufflingCtxDbHndl,
                                         (xpsDbKey_t)&tableId, (void**)&nodeEntry);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    if (nodeEntry == NULL)
    {
        retVal = XP_ERR_KEY_NOT_FOUND;
        return retVal;
    }

    if (entryId >= nodeEntry->size)
    {
        retVal = XP_ERR_INVALID_PARAMS;
        return retVal;
    }

    tcamEntryId = nodeEntry->logicalEntry[entryId].tcamEntryId;

    if (tcamEntryId == XPS_TCAM_INVALID_ENTRY_ID)
    {
        retVal = XP_ERR_INVALID_PARAMS;
        return retVal;
    }

    group = &nodeEntry->groupEntry[(uint32_t)
                                   nodeEntry->phyEntry[tcamEntryId].group];
    nodeEntry->logicalEntry[entryId].tcamEntryId = XPS_TCAM_INVALID_ENTRY_ID;
    nodeEntry->logicalEntry[entryId].next = -1;

    freeTcamLogicalEntryId(nodeEntry, entryId);

    if (group->firstTcamId == group->lastTcamId)
    {
        if (group->prev != -1)
        {
            nodeEntry->groupEntry[(uint32_t)group->prev].next = group->next;
        }
        else
        {
            nodeEntry->groupHead = group->next;
        }

        if (group->next != -1)
        {
            nodeEntry->groupEntry[(uint32_t)group->next].prev = group->prev;
        }

        freeTcamGroupEntry(nodeEntry, group);
    }
    else if (group->firstTcamId == tcamEntryId)
    {
        ++group->firstTcamId;
    }
    else if (group->lastTcamId == tcamEntryId)
    {
        --group->lastTcamId;
    }
    else // Within the group
    {
        newGroup = allocTcamGroupEntry(nodeEntry);
        if (newGroup == NULL)
        {
            retVal = XP_ERR_RESOURCE_NOT_AVAILABLE;
            return retVal;
        }

        for (uint32_t i = group->firstTcamId; i < tcamEntryId; i++)
        {
            nodeEntry->phyEntry[i].group = newGroup - nodeEntry->groupEntry;
        }

        newGroup->firstTcamId = group->firstTcamId;
        newGroup->lastTcamId = tcamEntryId - 1;
        newGroup->priority = group->priority;
        newGroup->prev = group->prev;
        newGroup->next = group - nodeEntry->groupEntry;
        if (newGroup->prev != -1)
        {
            nodeEntry->groupEntry[(uint32_t)newGroup->prev].next = newGroup -
                                                                   nodeEntry->groupEntry;
        }
        else
        {
            nodeEntry->groupHead = newGroup - nodeEntry->groupEntry;
        }

        group->firstTcamId = tcamEntryId + 1;
        group->prev = newGroup - nodeEntry->groupEntry;
    }

    return retVal;
}


#ifdef __cplusplus
}
#endif
