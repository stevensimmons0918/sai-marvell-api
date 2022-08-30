// xpsTcamAclShuffling.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsTcamMgr.h"
#include "xpsTcamAclShuffling.h"
#include "xpsLock.h"
#include "xpsScope.h"

#define ACL_TCAM_ORDERING_DEBUG 0

#if ACL_TCAM_ORDERING_DEBUG
#define ACL_TCAM_ORDER_TRACE printf("TOT>> ");printf
#else
#define ACL_TCAM_ORDER_TRACE(...)
#endif
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file xpsTcamAclShuffling.c
 * \brief This file contains the xps implementation of the shuffling algortihm.
 *
 */

/*
 * Some variables explanation:
 *     tableId - ID application uses to access table
 *     entryId - ID that corresponds TCAM hit index, but never changes. Application has to remeber it.
 *     tcamId  - HW hit index. Should be used for HW access.
 */

static xpsDbHandle_t tcamMgrAclShufflingCtxDbHndl = XPS_STATE_INVALID_DB_HANDLE;

static int32_t tcamMgrAclShufflingKeyComp(void* key1, void* key2)
{
    /* Key is dependent on tableId */
    return ((((xpsTcamMgrAclShufflingKeyFormatDbData_t *) key1)->tableId) - (((
                                                                                  xpsTcamMgrAclShufflingKeyFormatDbData_t *) key2)->tableId));
}

/* Static function definitions */
static XP_STATUS xpsTcamAclShufflingMoveEntries(xpsDevice_t devId,
                                                uint32_t tableId, int startEntry, int entryCnt, bool moveUp);
static XP_STATUS xpsTcamAclShufflingFindFreeEntry(xpsDevice_t devId,
                                                  uint32_t tableId, uint32_t currEntry, uint32_t prio, uint32_t* foundEntry);
static XP_STATUS xpsTcamAclShufflingFindOptimalEntry(xpsDevice_t devId,
                                                     uint32_t tableId, uint32_t priority, uint32_t* currEntry);
static XP_STATUS xpsTcamAclShufflingBalanceTable(xpsDevice_t devId,
                                                 uint32_t tableId);
static XP_STATUS xpsTcamAclShufflingTableWhiteSpace(xpsDevice_t devId,
                                                    uint32_t tableId, uint32_t* balCenter, uint32_t* whiteSpaceSize);

XP_STATUS xpsTcamAclShufflingConfigTable(xpsDevice_t devId, uint32_t tableId,
                                         uint32_t entryNum, uint32_t prioNum, xpsTcamMgrRuleMove_fn ruleMove)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *keyFormat = NULL;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *dbKey = NULL;
    xpsScope_t scopeId;

    //get scope from Device ID
    retVal = xpsScopeGetScopeId(devId, &scopeId);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    if (tcamMgrAclShufflingCtxDbHndl == XPS_STATE_INVALID_DB_HANDLE)
    {
        tcamMgrAclShufflingCtxDbHndl = XPS_TCAM_MGR_ACL_SHUFFLING_DB_HNDL;
        retVal = xpsStateRegisterDb(scopeId, "TCAM ACL Shuffling", XPS_PER_DEVICE,
                                    &tcamMgrAclShufflingKeyComp, tcamMgrAclShufflingCtxDbHndl);
        if (retVal != XP_NO_ERR)
        {
            tcamMgrAclShufflingCtxDbHndl = XPS_STATE_INVALID_DB_HANDLE;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: Failed to register state database\n");
            return retVal;
        }

        retVal = xpsStateInsertDbForDevice(devId, tcamMgrAclShufflingCtxDbHndl,
                                           tcamMgrAclShufflingKeyComp);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: Failed to add DB for device %u \n", devId);
            return retVal;
        }
    }

    dbKey = (xpsTcamMgrAclShufflingKeyFormatDbData_t *) xpMalloc(sizeof(
                                                                     xpsTcamMgrAclShufflingKeyFormatDbData_t));
    if (!dbKey)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "malloc for dbKey failed\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    dbKey->tableId = tableId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(devId, tcamMgrAclShufflingCtxDbHndl,
                                              (xpsDbKey_t)dbKey, (void**)&keyFormat)) != XP_NO_ERR)
    {
        xpFree(dbKey);
        return retVal;
    }

    xpFree(dbKey);

    if (keyFormat == NULL)
    {
        if ((retVal = xpsStateHeapMalloc(sizeof(
                                             xpsTcamMgrAclShufflingKeyFormatDbData_t), (void**)&keyFormat)) == XP_NO_ERR)
        {
            memset(keyFormat, 0, sizeof(xpsTcamMgrAclShufflingKeyFormatDbData_t));

            keyFormat->ruleMove = ruleMove;
            keyFormat->tableEntryNum = entryNum;
            keyFormat->tableId = tableId;

            memset(keyFormat->ruleMap, 0x00, entryNum * sizeof(xpsTcamMgrRuleMap_t));

            /* Insert TCAM Mgr ACL Shuffling key format structure into DB, use tableId as a key */
            retVal = xpsStateInsertDataForDevice(devId, tcamMgrAclShufflingCtxDbHndl,
                                                 (void*)keyFormat);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: Failed to add DB to the device %u \n", devId);
                xpsStateHeapFree((void*)keyFormat);
                return retVal;
            }

        }
    }

    return retVal;
}

XP_STATUS xpsTcamAclShufflingRemoveTable(xpsDevice_t devId, uint32_t tableId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *keyFormat = NULL;

    if (tcamMgrAclShufflingCtxDbHndl == XPS_STATE_INVALID_DB_HANDLE)
    {
        return XP_ERR_NOT_INITED;
    }

    retVal = xpsStateDeleteDataForDevice(devId, tcamMgrAclShufflingCtxDbHndl,
                                         (xpsDbKey_t)&tableId, (void**)&keyFormat);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to remove data from DB for device %u \n", devId);
        return retVal;
    }

    retVal = xpsStateHeapFree((void*)keyFormat);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to free state data\n");
        return retVal;
    }

    return retVal;
}

XP_STATUS xpsTcamAclShufflingAddEntry(xpsDevice_t devId, uint32_t tableId,
                                      uint32_t priority, uint32_t* entryId)
{
    XPS_LOCK(xpsTcamAclShufflingAddEntry);

    XP_STATUS retVal = XP_NO_ERR;
    uint32_t freeEntry = 0;
    int entryCnt = 0;
    bool moveUp = TRUE;
    uint32_t currEntry = 0;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *keyFormat = NULL;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *dbKey = NULL;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Calling function %s\n",
          __FUNCNAME__);

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    dbKey = (xpsTcamMgrAclShufflingKeyFormatDbData_t *) xpMalloc(sizeof(
                                                                     xpsTcamMgrAclShufflingKeyFormatDbData_t));
    if (!dbKey)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "malloc for dbKey failed\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    dbKey->tableId = tableId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(devId, tcamMgrAclShufflingCtxDbHndl,
                                              (xpsDbKey_t)dbKey, (void**)&keyFormat)) != XP_NO_ERR)
    {
        xpFree(dbKey);
        return retVal;
    }

    xpFree(dbKey);

    if (keyFormat == NULL)
    {
        retVal = XP_ERR_KEY_NOT_FOUND;
        return retVal;
    }

    /* Find optimal entry for this priority */
    retVal = xpsTcamAclShufflingFindOptimalEntry(devId, tableId, priority,
                                                 &currEntry);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to determine optimal entry in TCAM table\n");
        return retVal;
    }

    /* Check if required TCAM space is free */
    if (keyFormat->ruleMap[currEntry].isEnabled == FALSE)
    {
        /* Entry is free */
        ACL_TCAM_ORDER_TRACE("tbl %d free\n", tableId);
        /* Do nothing */
    }
    else     /* Entry is not free */
    {
        /* Find nearest free entry */
        retVal = xpsTcamAclShufflingFindFreeEntry(devId, tableId, currEntry, priority,
                                                  &freeEntry);
        if (retVal == XP_ERR_SEARCH_MISS)
        {
            /* No free entries in table */
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Not enought memory\n");
            return retVal;
        }
        if (retVal != XP_NO_ERR)
        {
            return retVal;
        }
        ACL_TCAM_ORDER_TRACE("tbl %d prio=%d curr=%u free=%u\n", tableId, priority,
                             currEntry, freeEntry);

        moveUp = (bool)(currEntry > freeEntry);
        if ((keyFormat->ruleMap[currEntry].entryPrio == priority) && (!moveUp))
        {
            entryCnt = freeEntry - currEntry - 1;
            currEntry++;
        }
        else
        {
            entryCnt = (moveUp) ? (currEntry - freeEntry) : (freeEntry - currEntry);
        }

        /* Shuffle entries to create free space */
        retVal = xpsTcamAclShufflingMoveEntries(devId, tableId, currEntry, entryCnt,
                                                moveUp);
        if (retVal != XP_NO_ERR)
        {
            return retVal;
        }
    }


    ACL_TCAM_ORDER_TRACE("tbl %d add prio=%d to=%u\n", tableId, priority,
                         currEntry);
    /* Install the entry */
    keyFormat->ruleMap[currEntry].isEnabled = TRUE;
    keyFormat->ruleMap[currEntry].entryId   = *entryId;
    keyFormat->ruleMap[currEntry].entryPrio = priority;

    /* Balance the table */
    retVal = xpsTcamAclShufflingBalanceTable(devId, tableId);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    return retVal;
}

XP_STATUS xpsTcamAclShufflingDeleteEntry(xpsDevice_t devId, uint32_t tableId,
                                         uint32_t entryId)
{
    XPS_LOCK(xpsTcamAclShufflingDeleteEntry);

    XP_STATUS retVal = XP_NO_ERR;
    uint32_t i = 0;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *keyFormat = NULL;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *dbKey = NULL;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Calling function %s\n",
          __FUNCNAME__);

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    dbKey = (xpsTcamMgrAclShufflingKeyFormatDbData_t *) xpMalloc(sizeof(
                                                                     xpsTcamMgrAclShufflingKeyFormatDbData_t));
    if (!dbKey)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "malloc for dbKey failed\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    dbKey->tableId = tableId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(devId, tcamMgrAclShufflingCtxDbHndl,
                                              (xpsDbKey_t)dbKey, (void**)&keyFormat)) != XP_NO_ERR)
    {
        xpFree(dbKey);
        return retVal;
    }

    xpFree(dbKey);

    if (keyFormat == NULL)
    {
        retVal = XP_ERR_KEY_NOT_FOUND;
        return retVal;
    }

    /* Find entry */
    for (i = 0; i < keyFormat->tableEntryNum; i++)
    {
        /* Check if this is our entry */
        if (keyFormat->ruleMap[i].entryId == entryId)
        {
            /* Delete the entry */
            keyFormat->ruleMap[i].isEnabled = FALSE;
            keyFormat->ruleMap[i].entryId   = 0;
            keyFormat->ruleMap[i].entryPrio = 0;
        }
    }


    return retVal;
}

XP_STATUS xpsTcamAclShufflingGetTcamId(xpsDevice_t devId, uint32_t tableId,
                                       uint32_t entryId, uint32_t *tcamId)
{
    XP_STATUS retVal = XP_ERR_SEARCH_MISS;
    uint32_t i = 0;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *keyFormat = NULL;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *dbKey = NULL;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Calling function %s\n",
          __FUNCNAME__);

    if (tcamId == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Invalid input parameter\n");
        return XP_ERR_INVALID_PARAMS;
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    dbKey = (xpsTcamMgrAclShufflingKeyFormatDbData_t *) xpMalloc(sizeof(
                                                                     xpsTcamMgrAclShufflingKeyFormatDbData_t));
    if (!dbKey)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "malloc for dbKey failed\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    dbKey->tableId = tableId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(devId, tcamMgrAclShufflingCtxDbHndl,
                                              (xpsDbKey_t)dbKey, (void**)&keyFormat)) != XP_NO_ERR)
    {
        xpFree(dbKey);
        return retVal;
    }

    xpFree(dbKey);

    if (keyFormat == NULL)
    {
        retVal = XP_ERR_KEY_NOT_FOUND;
        return retVal;
    }

    /* Find entry */
    for (i = 0; i < keyFormat->tableEntryNum; i++)
    {
        /* Check if entryId is valid */
        if (keyFormat->ruleMap[i].isEnabled == FALSE)
        {
            /* Pass all invalid entries */
            continue;
        }

        /* Check if this is our entry */
        if (keyFormat->ruleMap[i].entryId == entryId)
        {
            /* Return required TCAM ID index */
            *tcamId = i;
            retVal = XP_NO_ERR;
            break;
        }
    }

    return retVal;
}

XP_STATUS xpsTcamAclShufflingGetEntryId(xpsDevice_t devId, uint32_t tableId,
                                        uint32_t tcamId, uint32_t *entryId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *keyFormat = NULL;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *dbKey = NULL;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Calling function %s\n",
          __FUNCNAME__);

    if (entryId == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Invalid input parameter\n");
        return XP_ERR_INVALID_PARAMS;
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    dbKey = (xpsTcamMgrAclShufflingKeyFormatDbData_t *) xpMalloc(sizeof(
                                                                     xpsTcamMgrAclShufflingKeyFormatDbData_t));
    if (!dbKey)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "malloc for dbKey failed\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    dbKey->tableId = tableId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(devId, tcamMgrAclShufflingCtxDbHndl,
                                              (xpsDbKey_t)dbKey, (void**)&keyFormat)) != XP_NO_ERR)
    {
        xpFree(dbKey);
        return retVal;
    }

    xpFree(dbKey);

    if (keyFormat == NULL)
    {
        retVal = XP_ERR_KEY_NOT_FOUND;
        return retVal;
    }

    /* Check if entryId is valid */
    if (keyFormat->ruleMap[tcamId].isEnabled == FALSE)
    {
        retVal = XP_ERR_INVALID_ID;
        return retVal;
    }

    /* Read entry ID */
    *entryId = keyFormat->ruleMap[tcamId].entryId;

    return retVal;
}

XP_STATUS xpsTcamAclShufflingDebug(xpsDevice_t devId, uint32_t tableId)
{
    XP_STATUS retVal = XP_NO_ERR;
    uint32_t i = 0;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *keyFormat = NULL;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *dbKey = NULL;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Calling function %s\n",
          __FUNCNAME__);

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    dbKey = (xpsTcamMgrAclShufflingKeyFormatDbData_t *) xpMalloc(sizeof(
                                                                     xpsTcamMgrAclShufflingKeyFormatDbData_t));
    if (!dbKey)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "malloc for dbKey failed\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    dbKey->tableId = tableId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(devId, tcamMgrAclShufflingCtxDbHndl,
                                              (xpsDbKey_t)dbKey, (void**)&keyFormat)) != XP_NO_ERR)
    {
        xpFree(dbKey);
        return retVal;
    }

    xpFree(dbKey);

    if (keyFormat == NULL)
    {
        retVal = XP_ERR_KEY_NOT_FOUND;
        return retVal;
    }

    if (keyFormat->ruleMove == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "Error: Missing TCAM entry moving rule\n");
        return retVal;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Table %u has %u entries\n",
          tableId, keyFormat->tableEntryNum);

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "TCAM ID   Entry ID   Entry Priority\n");
    for (i = 0; i < keyFormat->tableEntryNum; i++)
    {
        if (keyFormat->ruleMap[i].isEnabled == TRUE)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "%5u %5u %5u\n", i,
                  keyFormat->ruleMap[i].entryId,
                  keyFormat->ruleMap[i].entryPrio);
        }
    }

    return retVal;
}

static XP_STATUS xpsTcamAclShufflingMoveEntries(xpsDevice_t devId,
                                                uint32_t tableId, int startEntry, int entryCnt, bool moveUp)
{
    XP_STATUS retVal = XP_NO_ERR;
    int i = 0;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *keyFormat = NULL;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *dbKey = NULL;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Calling function %s\n",
          __FUNCNAME__);

    /* No move should be done */
    if (entryCnt == 0)
    {
        return retVal;
    }

    ACL_TCAM_ORDER_TRACE("tbl %d move=%d from=%u up=%d\n", tableId, entryCnt,
                         startEntry, moveUp);

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    dbKey = (xpsTcamMgrAclShufflingKeyFormatDbData_t *) xpMalloc(sizeof(
                                                                     xpsTcamMgrAclShufflingKeyFormatDbData_t));
    if (!dbKey)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "malloc for dbKey failed\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    dbKey->tableId = tableId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(devId, tcamMgrAclShufflingCtxDbHndl,
                                              (xpsDbKey_t)dbKey, (void**)&keyFormat)) != XP_NO_ERR)
    {
        xpFree(dbKey);
        return retVal;
    }

    xpFree(dbKey);

    if (keyFormat == NULL)
    {
        retVal = XP_ERR_KEY_NOT_FOUND;
        return retVal;
    }

    /* Check if we are not exceeding the table */
    if ((!moveUp) && ((startEntry + entryCnt) >= (int)keyFormat->tableEntryNum))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Invalid input parameter\n");
        retVal = XP_ERR_INVALID_PARAMS;
        return retVal;
    }

    /* Check if we are not exceeding the table */
    if (moveUp && ((startEntry - entryCnt) < 0))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Invalid input parameter\n");
        retVal = XP_ERR_INVALID_PARAMS;
        return retVal;
    }

    /* Move the entryCnt entries up or down the TCAM table */
    if (moveUp == TRUE)
    {
        for (i = (startEntry - entryCnt); i < startEntry ; i++)
        {
            if (keyFormat->ruleMap[i].isEnabled == TRUE)
            {
                continue;
            }
            /* Perform SW move */
            memcpy(&keyFormat->ruleMap[i],         /* Destination */
                   &keyFormat->ruleMap[i+1],      /* Source */
                   sizeof(xpsTcamMgrRuleMap_t));  /* Data size */

            /* Perform HW entry move */
            keyFormat->ruleMove(devId, tableId, i, i+1);

            keyFormat->ruleMap[i+1].isEnabled = FALSE;
            keyFormat->ruleMap[i+1].entryId   = 0;
            keyFormat->ruleMap[i+1].entryPrio = 0;
        }
    }
    else
    {
        for (i = (startEntry + entryCnt - 1); i >= startEntry ; i--)
        {
            if (keyFormat->ruleMap[i+1].isEnabled == TRUE)
            {
                continue;
            }
            /* Perform SW move */
            memcpy(&keyFormat->ruleMap[i+1],       /* Destination */
                   &keyFormat->ruleMap[i],        /* Source */
                   sizeof(xpsTcamMgrRuleMap_t));  /* Data size */

            /* Perform HW entry move */
            keyFormat->ruleMove(devId, tableId, i+1, i);

            keyFormat->ruleMap[i].isEnabled = FALSE;
            keyFormat->ruleMap[i].entryId   = 0;
            keyFormat->ruleMap[i].entryPrio = 0;
        }
    }

    return retVal;
}

static XP_STATUS xpsTcamAclShufflingFindFreeEntry(xpsDevice_t devId,
                                                  uint32_t tableId, uint32_t currEntry, uint32_t prio, uint32_t *foundEntry)
{
    XP_STATUS retVal = XP_NO_ERR;
    uint32_t headIndex = 0, tailIndex = 0;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *keyFormat = NULL;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *dbKey = NULL;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Calling function %s\n",
          __FUNCNAME__);

    if (foundEntry == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Invalid input parameter\n");
        return XP_ERR_INVALID_PARAMS;
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    dbKey = (xpsTcamMgrAclShufflingKeyFormatDbData_t *) xpMalloc(sizeof(
                                                                     xpsTcamMgrAclShufflingKeyFormatDbData_t));
    if (!dbKey)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "malloc for dbKey failed\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    dbKey->tableId = tableId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(devId, tcamMgrAclShufflingCtxDbHndl,
                                              (xpsDbKey_t)dbKey, (void**)&keyFormat)) != XP_NO_ERR)
    {
        xpFree(dbKey);
        return retVal;
    }

    xpFree(dbKey);

    if (keyFormat == NULL)
    {
        retVal = XP_ERR_KEY_NOT_FOUND;
        return retVal;
    }

    while (TRUE)
    {
        /* Exit condition in case there are no free entries */
        if (((currEntry + tailIndex) >= (keyFormat->tableEntryNum - 1)) &&
            (currEntry <= headIndex))
        {
            retVal = XP_ERR_SEARCH_MISS;
            return retVal;
        }

        /* Exit condition in case algorithm exceeds table limits */
        if (currEntry <= headIndex)
        {
            retVal = XP_ERR_SEARCH_MISS;
            return retVal;
        }
        if ((currEntry + tailIndex) >= keyFormat->tableEntryNum)
        {
            retVal = XP_ERR_SEARCH_MISS;
            return retVal;
        }

        if (keyFormat->ruleMap[currEntry + tailIndex].isEnabled == FALSE)
        {
            if ((keyFormat->ruleMap[currEntry + (tailIndex - 1)].entryPrio >= prio) ||
                (keyFormat->ruleMap[currEntry + (tailIndex - 1)].isEnabled == FALSE))
            {
                *foundEntry = currEntry + tailIndex;
                return retVal;
            }
        }

        if ((currEntry - headIndex) > 0)
        {
            headIndex++;
        }

        if ((currEntry + tailIndex) < (keyFormat->tableEntryNum - 1))
        {
            tailIndex++;
        }
    }

    return retVal;
}

static XP_STATUS xpsTcamAclShufflingFindOptimalEntry(xpsDevice_t devId,
                                                     uint32_t tableId, uint32_t priority, uint32_t* currEntry)
{
    XP_STATUS retVal = XP_NO_ERR;
    uint32_t i = 0;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *keyFormat = NULL;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *dbKey = NULL;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Calling function %s\n",
          __FUNCNAME__);

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    dbKey = (xpsTcamMgrAclShufflingKeyFormatDbData_t *) xpMalloc(sizeof(
                                                                     xpsTcamMgrAclShufflingKeyFormatDbData_t));
    if (!dbKey)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "malloc for dbKey failed\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    dbKey->tableId = tableId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(devId, tcamMgrAclShufflingCtxDbHndl,
                                              (xpsDbKey_t)dbKey, (void**)&keyFormat)) != XP_NO_ERR)
    {
        xpFree(dbKey);
        return retVal;
    }

    xpFree(dbKey);

    if (keyFormat == NULL)
    {
        retVal = XP_ERR_KEY_NOT_FOUND;
        return retVal;
    }

    /* Searching for the same priority in the table */
    for (i = 0; i < keyFormat->tableEntryNum - 1; i++)
    {
        if ((keyFormat->ruleMap[i].entryPrio == priority) &&
            (keyFormat->ruleMap[i].isEnabled == TRUE))
        {
            /* If entry with searched priority already exists */
            /* Move to the last entry with this priority */
            while ((keyFormat->ruleMap[i].entryPrio == priority) &&
                   (keyFormat->ruleMap[i].isEnabled == TRUE))
            {
                i++;
            }
            *currEntry = i - 1;
            return retVal;
        }
    }

    /* In case this priority does not exist in the table */
    for (i = keyFormat->tableEntryNum - 1; i > 0; i--)
    {
        if ((keyFormat->ruleMap[i-1].entryPrio < priority) &&
            (keyFormat->ruleMap[i-1].isEnabled == TRUE))
        {
            *currEntry = i;
            return retVal;
        }
    }
    for (i = 0; i < keyFormat->tableEntryNum - 1 ; i++)
    {
        if ((keyFormat->ruleMap[i+1].entryPrio > priority) &&
            (keyFormat->ruleMap[i+1].isEnabled == TRUE))
        {
            *currEntry = i;
            return retVal;
        }
    }

    /* In case table is empty or full */
    *currEntry = keyFormat->tableEntryNum/2;

    return retVal;
}

static XP_STATUS xpsTcamAclShufflingTableWhiteSpace(xpsDevice_t devId,
                                                    uint32_t tableId, uint32_t* balCenter, uint32_t* whiteSpaceSize)
{
    XP_STATUS retVal = XP_NO_ERR;
    uint32_t i = 0;
    uint32_t maxCount = 0, maxEndIndex = 0;
    uint32_t count = 0, endIndex = 0;
    uint32_t whiteSpaceSizeTmp = 0;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *keyFormat = NULL;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *dbKey = NULL;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Calling function %s\n",
          __FUNCNAME__);

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    dbKey = (xpsTcamMgrAclShufflingKeyFormatDbData_t *) xpMalloc(sizeof(
                                                                     xpsTcamMgrAclShufflingKeyFormatDbData_t));
    if (!dbKey)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "malloc for dbKey failed\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    dbKey->tableId = tableId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(devId, tcamMgrAclShufflingCtxDbHndl,
                                              (xpsDbKey_t)dbKey, (void**)&keyFormat)) != XP_NO_ERR)
    {
        xpFree(dbKey);
        return retVal;
    }

    xpFree(dbKey);

    if (keyFormat == NULL)
    {
        retVal = XP_ERR_KEY_NOT_FOUND;
        return retVal;
    }

    /* Calculate the size of all white spaces in the table */
    for (i = 0; i < keyFormat->tableEntryNum; i++)
    {
        /* If this is whitespace, then count number of free elements */
        if (keyFormat->ruleMap[i].isEnabled == FALSE)
        {
            count++;
            endIndex = i;
            whiteSpaceSizeTmp++;
        }

        /* If we get to the end of whitespace -- compare it with other whitespaces by size */
        if ((keyFormat->ruleMap[i].isEnabled == TRUE) && (count > 0))
        {
            if (count > maxCount)
            {
                maxCount = count;
                maxEndIndex = endIndex;
            }

            count = 0;
            endIndex = 0;
        }
    }

    /* This check is required in case table tail is empty */
    if (count > maxCount)
    {
        maxCount = count;
        maxEndIndex = endIndex;
    }

    /* Calculate center of the biggest white space in this table */
    *balCenter = maxEndIndex - (maxCount/2);
    *whiteSpaceSize = whiteSpaceSizeTmp;

    return retVal;
}

static XP_STATUS xpsTcamAclShufflingBalanceTable(xpsDevice_t devId,
                                                 uint32_t tableId)
{
    XP_STATUS retVal = XP_NO_ERR;
    uint32_t balCenter = 0, whiteSpaceSize = 0;
    int i = 0;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *keyFormat = NULL;
    xpsTcamMgrAclShufflingKeyFormatDbData_t *dbKey = NULL;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Calling function %s\n",
          __FUNCNAME__);

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    dbKey = (xpsTcamMgrAclShufflingKeyFormatDbData_t *) xpMalloc(sizeof(
                                                                     xpsTcamMgrAclShufflingKeyFormatDbData_t));
    if (!dbKey)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "malloc for dbKey failed\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    dbKey->tableId = tableId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(devId, tcamMgrAclShufflingCtxDbHndl,
                                              (xpsDbKey_t)dbKey, (void**)&keyFormat)) != XP_NO_ERR)
    {
        xpFree(dbKey);
        return retVal;
    }

    xpFree(dbKey);

    if (keyFormat == NULL)
    {
        retVal = XP_ERR_KEY_NOT_FOUND;
        return retVal;
    }

    /* Assign table balancing center */
    /* Balancing center is an entry number in middle of the
     * biggest empty part of the table. */
    xpsTcamAclShufflingTableWhiteSpace(devId, tableId, &balCenter, &whiteSpaceSize);

    if (whiteSpaceSize > keyFormat->tableEntryNum/2)
    {
        return retVal;
    }

    for (i = balCenter; i > 0; i--)
    {
        if ((keyFormat->ruleMap[i].isEnabled == FALSE) &&
            (keyFormat->ruleMap[i-1].isEnabled == TRUE))
        {
            /* Moving entryCnt entries down */
            retVal = xpsTcamAclShufflingMoveEntries(devId, tableId, i-1, 1, FALSE);
            if (retVal != XP_NO_ERR)
            {
                return retVal;
            }
        }
    }

    for (i = balCenter + 1; i < (int)keyFormat->tableEntryNum - 1; i++)
    {
        if ((keyFormat->ruleMap[i].isEnabled == FALSE) &&
            (keyFormat->ruleMap[i+1].isEnabled == TRUE))
        {
            /* Moving entryCnt entries up */
            retVal = xpsTcamAclShufflingMoveEntries(devId, tableId, i+1, 1, TRUE);
            if (retVal != XP_NO_ERR)
            {
                return retVal;
            }
        }
    }

    return retVal;
}
#ifdef __cplusplus
}
#endif
