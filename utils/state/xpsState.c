// xpsState.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsState.h"
#include "xpsInterface.h"
#ifdef __cplusplus
extern "C" {
#endif
extern xpsInitType_t xpsGetInitType();

#define LOGFN(MOD, SUB, LVL, ...)  do{ }while(0);



/**
 * \def XPS_STATE_MAX_GLOBAL_DB
 *
 * Define to indicate the maximum number of global DB handles we
 * want to maintain
 */

#define XPS_STATE_MAX_GLOBAL_DB  (XPS_GLOBAL_DB_HANDLE_LAST - XPS_GLOBAL_DB_HANDLE_BEGINS)
#define XPS_STATE_MAX_PER_DEVICE_DB (XPS_PER_DEVICE_HANDLE_ENDS - XPS_PER_DEVICE_HANDLE_BEGINS)

#define XPS_STATE_PER_DEVICE_DB_HANDLE_INDEX(dbHndl) ((dbHndl) - XPS_PER_DEVICE_HANDLE_BEGINS)

/**
 * \def XPS_STATE_DB_HANDLE_BARRIER
 *
 * This Macro marks the barrier between Global and Per Device
 * Database Handles
 */
#define XPS_STATE_DB_HANDLE_BARRIER XPS_GLOBAL_DB_HANDLE_ENDS

/**
 * \def XPS_STATE_DB_HANDLE_IS_VALID
 *
 * Returns false if the handle passed in is invalid
 *
 * \param [in] dbHndl
 */
#define XPS_STATE_DB_HANDLE_IS_VALID(dbHndl) ((dbHndl != XPS_STATE_INVALID_DB_HANDLE) ? 1 : 0)

/**
 * \def XPS_STATE_DB_HANDLE_IS_PER_DEVICE
 *
 * Returns fals if the handle is not a per device handle
 *
 * \param [in] dbHndl
 */
#define XPS_STATE_DB_HANDLE_IS_PER_DEVICE(dbHndl) ((dbHndl < XPS_GLOBAL_DB_HANDLE_ENDS) ? 0 : 1)

#define XPS_STATE_GET_DB_BUCKET(dbHndl) ((dbHndl > XPS_GLOBAL_DB_HANDLE_ENDS ) ? XPS_PER_DEVICE : XPS_GLOBAL)

/**
 * \var gBucket
 *
 * \brief Pointer to the global database bucket
 *
 * We use a static triple pointer to point to the head of the
 * global bucket array. Effectively we point to an array of
 * pointers to Red-Black Tree structs on a global basis
 *
 * A global db is one which is same across devices.
 * A scope is a collection of devices. And there is no
 * DB which will be valid across scopes. Hence a global
 * DB in true sense is a scope DB. Terms might be used
 * interchangeably in the file and elsewhere in XPS.
 *
 *  s denotes scope
 *  p denotes the pointer
 * gBucket ---
 *           |
 *           |
  *          |
  *  ---------------------
 *  | s1 | s2 | s3 | s4 |
 *  ---------------------
  *           |
 *   ---------------------
 *   | p1 | p2 | p3 | p4 |
 *   ---------------------
   *           |
 *             |
 *   ---------------------------------
 *  | rb-tree pointer for scope 2, handle p2 |
 *   ---------------------------------
 *  Max size of gBucket =
 */
static xpsRBTree_t***   gBucket = NULL;

/**
 * \var pdBucket
 * \brief Pointer to the per device database bucket
 *
 * We use a static triple pointer to point to an array of
 * database arrays of pointers to Red-Black Tree structs. This
 * allows us to maintain the following relationship:
 *
 * pdBucket ---
 *            |
 *            |
 *           ---------------------
 *           | p1 | p2 | p3 | p4 |
 *           ---------------------
 *                        |
 *                        |
 *                ---------------------
 *                | d0 | d1 | d2 | d3 |
 *                ---------------------
 *                                 |
 *                                 |
 *                    -----------------------------------------
 *                    | rb-tree pointer for handle p3, dev d3 |
 *                    -----------------------------------------
 */
static xpsRBTree_t***  pdBucket = NULL;

static XP_STATUS xpsStateUpdateMemoryAddressesForPerDeviceDb(xpsDevice_t devId);
static XP_STATUS xpsStateUpdateMemoryAddressesForGlobalDb(xpsScope_t scopeId);
static XP_STATUS xpsStateUpdateMemoryAddressesForGBucketScopeArray();
static XP_STATUS xpsStateUpdateMemoryAddressesForPdBucketArray();

XP_STATUS xpsStateInit(xpsScope_t scopeId, xpsInitType_t initType)
{
    XP_STATUS status = XP_NO_ERR;
    uint64_t uniqueObjId = 0;
    uint64_t offset = 0;
    uint8_t  dir = 0;

    // Pre Allocate space for the pointer arrays for both the global and
    // Per device buckets
    uniqueObjId = XP_ALLOCATOR_OID(0, (uint64_t)XPS_STATE_GLOBAL_DB_BUCKET_OID);

    if (!gBucket)
    {
        gBucket  = (xpsRBTree_t***)XP_ID_BASED_PERSISTENT_MALLOC(uniqueObjId,
                                                                 sizeof(xpsRBTree_t**) * XP_MAX_SCOPES);
        /*Store the global bucket in persistent memory at known address*/
        if (!gBucket)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Malloc for Global DB failed");
            return XP_ERR_OUT_OF_MEM;
        }
        if (initType == INIT_WARM)
        {
            status = XP_PERSISTENT_GET_OFFSET_DIR(&offset, &dir);
            if (offset != 0)
            {
                status = xpsStateUpdateMemoryAddressesForGBucketScopeArray();
                /*Update memory addresses stored in the state db after process restart*/
                status = xpsStateUpdateMemoryAddressesForGlobalDb(scopeId);
                if (status != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update mem address for global db failed");
                    return status;
                }
            }
        }
        if (initType == INIT_COLD)
        {
            // Initialize the global bucket to empty
            memset(gBucket, 0, sizeof(xpsRBTree_t**) * XP_MAX_SCOPES);
        }
    }

    if (!gBucket[scopeId])
    {
        gBucket[scopeId]  = (xpsRBTree_t**)XP_PERSISTENT_MALLOC(sizeof(
                                                                    xpsRBTree_t*) * XPS_STATE_MAX_GLOBAL_DB);
        /*Store the global bucket in persistent memory at known address*/
        if (!gBucket[scopeId])
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Malloc for Global DB failed");
            return XP_ERR_OUT_OF_MEM;
        }
        if (initType == INIT_COLD)
        {
            // Initialize the global bucket to empty
            memset(gBucket[scopeId], 0, sizeof(xpsRBTree_t*) * XPS_STATE_MAX_GLOBAL_DB);

        }
    }


    //This call is per scope. Allocate pdBucket for all handles only for first time.
    if (!pdBucket)
    {
        uniqueObjId = XP_ALLOCATOR_OID(0, (uint64_t)XPS_STATE_PERDEV_DB_BUCKET_OID);
        pdBucket = (xpsRBTree_t***)XP_ID_BASED_PERSISTENT_MALLOC(uniqueObjId,
                                                                 sizeof(xpsRBTree_t**) * XPS_STATE_MAX_PER_DEVICE_DB);
        if (!pdBucket)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Malloc for Per Device failed");
            return XP_ERR_OUT_OF_MEM;
        }

        if (initType == INIT_COLD)
        {
            // Initialize the per device bucket to empty
            memset(pdBucket, 0, sizeof(xpsRBTree_t**) * XPS_STATE_MAX_PER_DEVICE_DB);
        }
    }
    return status;
}

XP_STATUS xpsStateAddDevice(xpsDevice_t devId)
{
    uint64_t offset = 0;
    uint8_t dir = 0;
    XP_STATUS status = XP_NO_ERR;

    xpsInitType_t initType = xpsGetInitType();
    if (initType == INIT_WARM)
    {
        status = XP_PERSISTENT_GET_OFFSET_DIR(&offset, &dir);
        if (offset != 0)
        {
            status = xpsStateUpdateMemoryAddressesForPdBucketArray();
            status = xpsStateUpdateMemoryAddressesForPerDeviceDb(devId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Update mem address for perDevice db failed with Error code: %d\n", status);
                return status;
            }
        }
    }
    return status;
}

XP_STATUS xpsStateRemoveDevice(xpsDevice_t devId)
{
    return XP_NO_ERR;
}

XP_STATUS xpsStateDeInit(xpsScope_t scopeId)
{
    XP_STATUS status = XP_NO_ERR;
    uint32_t i = 0;

    if (!gBucket[scopeId] || !pdBucket)
    {
        return XP_ERR_NULL_POINTER;
    }

    // Cycle thorugh the global databases purging the trees
    for (i = 0; i < XPS_STATE_MAX_GLOBAL_DB; i++)
    {
        if (gBucket[scopeId][i])
        {
            // Delete the tree
            xpsRBTDelete(gBucket[scopeId][i]);
            gBucket[scopeId][i] = NULL;
        }
    }

    // Release the memory allocated for the global bucket
    XP_PERSISTENT_FREE(gBucket[scopeId]);
    gBucket[scopeId] = NULL;

    //If there is no scope anymore, free gBucket.
    for (i = 0; i < XP_MAX_SCOPES; i++)
    {
        if (gBucket[scopeId] != NULL)
        {
            break;
        }
    }

    if (i == XP_MAX_SCOPES)
    {
        XP_ID_BASED_PERSISTENT_FREE(gBucket);
        gBucket = NULL;
    }

    // Cycle through the per device databases purging each tree per device
    for (uint32_t i = 0; i < XPS_STATE_MAX_PER_DEVICE_DB; i++)
    {
        if (pdBucket[i])
        {
            for (uint32_t j = 0; j < XP_MAX_DEVICES; j++)
            {
                if (pdBucket[i][j])
                {
                    // Delete the tree
                    xpsRBTDelete(pdBucket[i][j]);
                    pdBucket[i][j] = NULL;
                }
            }
            XP_PERSISTENT_FREE(pdBucket[i]);
            pdBucket[i] = NULL;
        }
    }

    // Release the memory allocated for the per device pucket
    XP_ID_BASED_PERSISTENT_FREE(pdBucket);
    pdBucket = NULL;

    return status;
}

XP_STATUS xpsStateRegisterDb(xpsScope_t scopeId, const char* name,
                             xpsDbBucket_e bucket, xpsComp_t comp, xpsDbHandle_t handle)
{
    XP_STATUS    status = XP_NO_ERR;
    xpsRBTree_t *rbtree = NULL;
    xpsDbHandle_t pdHandleIndex = XPS_STATE_PER_DEVICE_DB_HANDLE_INDEX(handle);
    static void *p = NULL;
    if (NULL == p)
    {
        p =&(gBucket[0][1]);
    }

    switch (bucket)
    {
        case XPS_GLOBAL:

            if (gBucket[scopeId][handle] == NULL)
            {
                // Insert Tree corresponding with the allocated dbHandle
                if ((status = xpsRBTInit(&rbtree, comp, XPS_VERSION)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Global DB initialization failed");
                    return status;
                }
                if (!rbtree)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Global DB, out of memory");
                    return XP_ERR_OUT_OF_MEM;
                }

                gBucket[scopeId][handle]  = rbtree;
            }
            else
            {
                rbtree = gBucket[scopeId][handle];
                if (rbtree == NULL)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Global DB, out of memory");
                    return XP_ERR_OUT_OF_MEM;
                }
                rbtree->comp = comp;
            }


            break;

        case XPS_PER_DEVICE:
            // Per device databases will ignore the setting of the compare function in this API
            // Different device types may have different compare functions
            // Allocate space for the Per Device State Array corresponding to this handle
            if (pdBucket[pdHandleIndex] == NULL)
            {
                pdBucket[pdHandleIndex] = (xpsRBTree_t**)XP_PERSISTENT_MALLOC(sizeof(
                                                                                  xpsRBTree_t*) * XP_MAX_DEVICES);

                if (!pdBucket[pdHandleIndex])
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Per device DB, out of memory");
                    return XP_ERR_OUT_OF_MEM;
                }
                for (uint8_t i = 0; i < XP_MAX_DEVICES; i++)
                {
                    pdBucket[pdHandleIndex][i] = NULL;
                }
            }
            break;

        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Parameter");
            return XP_ERR_INVALID_PARAMS;
    }
    return XP_NO_ERR;
}

inline static xpsRBTree_t* xpsStateGetDbForDevice(xpsDbHandle_t dbHndl,
                                                  xpsDevice_t devId)
{
    xpsRBTree_t *rbtree = NULL;
    xpsDbHandle_t pdHandleIndex = XPS_STATE_PER_DEVICE_DB_HANDLE_INDEX(dbHndl);

    if (dbHndl > XPS_GLOBAL_DB_HANDLE_ENDS)
    {
        if (pdBucket[pdHandleIndex] == NULL)
        {
            return rbtree;
        }
        else
        {
            rbtree = pdBucket[pdHandleIndex][devId];
        }
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid DB handle");
        return NULL;
    }

    return rbtree;
}

inline static xpsRBTree_t* xpsStateGetDb(xpsDbHandle_t dbHndl,
                                         xpsScope_t scopeId)
{
    xpsRBTree_t *rbtree = NULL;
    if (gBucket && dbHndl < XPS_GLOBAL_DB_HANDLE_ENDS)
    {
        if (gBucket[scopeId] == NULL)
        {
            return rbtree;
        }
        else
        {
            rbtree = gBucket[scopeId][dbHndl];
        }
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid DB handle");
        return NULL;
    }
    return rbtree;
}
XP_STATUS xpsStateUpdateMemoryAddressesForGBucketScopeArray()
{
    XP_STATUS status = XP_NO_ERR;
    xpsScope_t scopeId = 0;
    void *inAddress = NULL;
    void *outAddress = NULL;

    for (scopeId = 0; scopeId < XP_MAX_SCOPES; scopeId ++)
    {
        inAddress = (void *)gBucket[scopeId];
        outAddress  = NULL;
        status = XP_PERSISTENT_ADDRESS_TRANSLATION(inAddress, &outAddress);
        if (status != XP_NO_ERR)
        {
            return status;
        }
        gBucket[scopeId] = (xpsRBTree_t **)outAddress;

    }
    return status;
}

XP_STATUS xpsStateUpdateMemoryAddressesForGlobalDb(xpsScope_t scopeId)
{
    XP_STATUS status = XP_NO_ERR;
    xpsRBTree_t *rbtree = NULL;
    xpsDbHandle_t dbHndl = XPS_GLOBAL_DB_HANDLE_BEGINS;
    void *inAddress = NULL;
    void *outAddress = NULL;

    for (dbHndl = dbHndl+1 ; dbHndl < XPS_GLOBAL_DB_HANDLE_LAST; dbHndl ++)
    {
        rbtree = NULL;
        if (gBucket[scopeId][dbHndl])
        {
            inAddress = (void *) gBucket[scopeId][dbHndl];
            outAddress  = NULL;
            status = XP_PERSISTENT_ADDRESS_TRANSLATION(inAddress, &outAddress);
            if (status != XP_NO_ERR)
            {
                return status;
            }
            rbtree = (xpsRBTree_t *)outAddress;
            gBucket[scopeId][dbHndl] = rbtree;
        }
        if (rbtree != NULL)
        {
            if (rbtree->root != NULL)
            {
                inAddress = (void *) rbtree->root;
                outAddress  = NULL;
                status = XP_PERSISTENT_ADDRESS_TRANSLATION(inAddress, &outAddress);
                if (status != XP_NO_ERR)
                {
                    return status;
                }
                rbtree->root = (xpsRBNode_t *)outAddress;
            }
            status = xpsRBTUpdateMemoryAddress(rbtree);
            if (status != XP_NO_ERR)
            {
                return status;
            }
        }
    }
    return status;
}

XP_STATUS xpsStateUpdateMemoryAddressesForPdBucketArray()
{
    XP_STATUS status = XP_NO_ERR;
    void *inAddress = NULL;
    void *outAddress = NULL;

    for (uint32_t i = 1; i < XPS_STATE_MAX_PER_DEVICE_DB; i++)
    {
        if (pdBucket[i])
        {
            inAddress = (void *)pdBucket[i];
            outAddress  = NULL;
            status = XP_PERSISTENT_ADDRESS_TRANSLATION(inAddress, &outAddress);
            if (status != XP_NO_ERR)
            {
                return status;
            }
            pdBucket[i] = (xpsRBTree_t **)outAddress;
        }
    }
    return status;
}

XP_STATUS xpsStateUpdateMemoryAddressesForPerDeviceDb(xpsDevice_t devId)
{
    XP_STATUS status = XP_NO_ERR;
    xpsRBTree_t *rbtree = NULL;
    xpsDbHandle_t dbHndl = 0;
    void *inAddress = NULL;
    void *outAddress = NULL;

    for (dbHndl = dbHndl +1; dbHndl < XPS_STATE_MAX_PER_DEVICE_DB; dbHndl ++)
    {
        rbtree = NULL;
        if (pdBucket[dbHndl])
        {
            inAddress = (void *) pdBucket[dbHndl][devId];

            /* Check for per device db, skip Address Translation if not inited for perticular db Handle. */
            if (inAddress != NULL)
            {
                outAddress  = NULL;
                status = XP_PERSISTENT_ADDRESS_TRANSLATION(inAddress, &outAddress);
                if (status != XP_NO_ERR)
                {
                    return status;
                }
                rbtree = (xpsRBTree_t *)outAddress;
                pdBucket[dbHndl][devId] = rbtree;
            }
        }

        if (rbtree != NULL)
        {
            if (rbtree->root != NULL)
            {
                inAddress = (void *) rbtree->root;
                outAddress  = NULL;
                status = XP_PERSISTENT_ADDRESS_TRANSLATION(inAddress, &outAddress);
                if (status != XP_NO_ERR)
                {
                    return status;
                }
                rbtree->root = (xpsRBNode_t *)outAddress;
            }
            status = xpsRBTUpdateMemoryAddress(rbtree);
            if (status != XP_NO_ERR)
            {
                return status;
            }
        }

    }
    return status;
}

XP_STATUS xpsStateDeRegisterDb(xpsScope_t scopeId, xpsDbHandle_t *handle)
{
    XP_STATUS status = XP_NO_ERR;
    xpsRBTree_t *rbtree = NULL;
    xpsDbHandle_t pdHandleIndex = XPS_STATE_PER_DEVICE_DB_HANDLE_INDEX(*handle);

    // Verify handle validity
    if (!XPS_STATE_DB_HANDLE_IS_VALID(*handle))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid DB handle");
        return XP_ERR_INVALID_PARAMS;
    }

    if (XPS_STATE_GET_DB_BUCKET(*handle) == XPS_GLOBAL)
    {
        // Device is hard coded to 0 here as it's irrelevant
        rbtree = xpsStateGetDb(*handle, scopeId);
        if (!rbtree)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get global DB failed");
            return XP_ERR_NULL_POINTER;
        }

        // Delete the tree
        xpsRBTDelete(rbtree);
        gBucket[scopeId][*handle] = NULL;
        *handle = XPS_STATE_INVALID_DB_HANDLE;
    }
    else if (XPS_STATE_GET_DB_BUCKET(*handle) == XPS_PER_DEVICE)
    {
        // Loop through all devices
        for (uint32_t i = 0; i < XP_MAX_DEVICES; i++)
        {
            rbtree = xpsStateGetDbForDevice(*handle, i);
            if (!rbtree)
            {
                continue;
            }

            // Purge the tree for this device
            xpsRBTDelete(rbtree);
            pdBucket[pdHandleIndex][i] = NULL;
        }

        // Free the tree device list
        XP_PERSISTENT_FREE(pdBucket[pdHandleIndex]);
        pdBucket[pdHandleIndex] = NULL;
        *handle = XPS_STATE_INVALID_DB_HANDLE;
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid Parameter");
        status = XP_ERR_INVALID_PARAMS;
    }

    return status;
}

XP_STATUS xpsStateInsertDbForDevice(xpsDevice_t devId, xpsDbHandle_t handle,
                                    xpsComp_t comp)
{
    XP_STATUS status = XP_NO_ERR;
    xpsDbHandle_t pdHandleIndex = XPS_STATE_PER_DEVICE_DB_HANDLE_INDEX(handle);

    if (pdBucket[pdHandleIndex][devId] == NULL)
    {
        // Verify handle validity
        if (!XPS_STATE_DB_HANDLE_IS_VALID(handle))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid DB handle");
            return XP_ERR_INVALID_PARAMS;
        }

        // Verify handle is per device
        if (!XPS_STATE_DB_HANDLE_IS_PER_DEVICE(handle))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "DB handle is not per device");
            return XP_ERR_INVALID_PARAMS;
        }

        // Ensure that there is no existing Database for this device
        xpsRBTree_t *rbtree = xpsStateGetDbForDevice(handle, devId);
        if (rbtree)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Database exist for the device");
            return XP_ERR_PM_DEV_ADD_DNE;
        }

        // Allocate space for the device db
        if ((status = xpsRBTInit(&rbtree, comp, XPS_VERSION)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "DB allocation failed");
            return status;
        }
        if (!rbtree)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "DB allocation, out of memory");
            return XP_ERR_OUT_OF_MEM;
        }

        // Update the reference in handle's pd array
        pdBucket[pdHandleIndex][devId] = rbtree;
    }
    else
    {
        xpsRBTree_t *rbtree = NULL;
        rbtree = pdBucket[pdHandleIndex][devId];
        rbtree->comp = comp;
    }
    return status;
}

XP_STATUS xpsStateDeleteDbForDevice(xpsDevice_t devId, xpsDbHandle_t handle)
{
    xpsDbHandle_t pdHandleIndex = XPS_STATE_PER_DEVICE_DB_HANDLE_INDEX(handle);

    // Verify handle validity
    if (!XPS_STATE_DB_HANDLE_IS_VALID(handle))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid DB handle");
        return XP_ERR_INVALID_PARAMS;
    }

    // Verify handle is per device
    if (!XPS_STATE_DB_HANDLE_IS_PER_DEVICE(handle))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "DB handle is not per device");
        return XP_ERR_INVALID_PARAMS;
    }

    // Ensure that there is an existing Database for this device
    xpsRBTree_t *rbtree = xpsStateGetDbForDevice(handle, devId);
    if (!rbtree)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "DB not exists");
        return XP_ERR_NULL_POINTER;
    }

    // Delete the tree
    xpsRBTDelete(rbtree);
    pdBucket[pdHandleIndex][devId] = NULL;

    return XP_NO_ERR;
}

XP_STATUS xpsStateInsertData(xpsScope_t scopeId, xpsDbHandle_t handle,
                             void *data)
{
    XP_STATUS status = XP_NO_ERR;
    xpsRBTree_t *rbtree = NULL;

    // Verify handle validity
    if (!XPS_STATE_DB_HANDLE_IS_VALID(handle))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid DB handle");
        return XP_ERR_INVALID_PARAMS;
    }

    // Retrieve the tree from the appropriate bucket (devId hardcoded to 0 for global)
    rbtree = xpsStateGetDb(handle, scopeId);
    if (!rbtree)
    {
        return XP_ERR_NULL_POINTER;
    }

    // Add a new node
    status = xpsRBTAddNode(rbtree, data);
    if ((status != XP_NO_ERR) && (status != XP_ERR_KEY_EXISTS))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Add node failed");
        return status;
    }

    return status;
}

XP_STATUS xpsStateSearchData(xpsScope_t scopeId, xpsDbHandle_t handle,
                             xpsDbKey_t key, void **data)
{
    XP_STATUS status = XP_NO_ERR;
    xpsRBTree_t *rbtree = NULL;

    // Verify handle validity
    if (!XPS_STATE_DB_HANDLE_IS_VALID(handle))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid DB handle");
        return XP_ERR_INVALID_PARAMS;
    }

    // Retrieve the tree form the appropriate bucket (devId hardcoded to 0 for global)
    rbtree = xpsStateGetDb(handle, scopeId);
    if (!rbtree)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Retrieve rbtree from appropriate bucket failed");
        return XP_ERR_NULL_POINTER;
    }

    // Search for the node
    *data = xpsRBTSearchNode(rbtree, key);

    return status;
}

XP_STATUS xpsStateGetNextData(xpsScope_t scopeId, xpsDbHandle_t handle,
                              xpsDbKey_t key, void **data)
{
    XP_STATUS status = XP_NO_ERR;
    xpsRBTree_t *rbtree = NULL;

    // Verify handle validity
    if (!XPS_STATE_DB_HANDLE_IS_VALID(handle))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid DB handle");
        return XP_ERR_INVALID_PARAMS;
    }

    // Retrieve the tree form the appropriate bucket (devId hardcoded to 0 for global)
    rbtree = xpsStateGetDb(handle, scopeId);
    if (!rbtree)
    {
        return XP_ERR_NULL_POINTER;
    }

    // Get next ascending order data for the key.
    *data = xpsRBTGetInorderSuccessor(rbtree, key);

    return status;
}

XP_STATUS xpsStateDeleteData(xpsScope_t scopeId, xpsDbHandle_t handle,
                             xpsDbKey_t key, void **data)
{
    XP_STATUS status = XP_NO_ERR;
    xpsRBTree_t *rbtree = NULL;

    // Verify handle validity
    if (!XPS_STATE_DB_HANDLE_IS_VALID(handle))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid DB handle");
        return XP_ERR_INVALID_PARAMS;
    }

    // Retrieve the tree form the appropriate bucket (devId hardcoded to 0 for global)
    rbtree = xpsStateGetDb(handle, scopeId);
    if (!rbtree)
    {
        return XP_ERR_NULL_POINTER;
    }

    // Delete the node if it exists, the memory allocated for data will be deleted by the caller
    *data = xpsRBTDeleteNode(rbtree, key);

    return status;
}

XP_STATUS xpsStateInsertDataForDevice(xpsDevice_t devId, xpsDbHandle_t handle,
                                      void *data)
{
    XP_STATUS status = XP_NO_ERR;
    xpsRBTree_t *rbtree = NULL;

    // Verify handle validity
    if (!XPS_STATE_DB_HANDLE_IS_VALID(handle))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid DB handle");
        return XP_ERR_INVALID_PARAMS;
    }

    // Retrieve the tree form the appropriate bucket (devId hardcoded to 0 for global)
    rbtree = xpsStateGetDbForDevice(handle, devId);
    if (!rbtree)
    {
        return XP_ERR_NULL_POINTER;
    }

    // Add a new node
    if ((status = xpsRBTAddNode(rbtree, data)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Add node to RBTree failed");
    }

    return status;
}

XP_STATUS xpsStateSearchDataForDevice(xpsDevice_t devId, xpsDbHandle_t handle,
                                      xpsDbKey_t key, void **data)
{
    XP_STATUS status = XP_NO_ERR;
    xpsRBTree_t *rbtree = NULL;

    // Verify handle validity
    if (!XPS_STATE_DB_HANDLE_IS_VALID(handle))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid DB handle");
        return XP_ERR_INVALID_PARAMS;
    }

    // Retrieve the tree form the appropriate bucket (devId hardcoded to 0 for global)
    rbtree = xpsStateGetDbForDevice(handle, devId);
    if (!rbtree)
    {
        return XP_ERR_NULL_POINTER;
    }

    // Search for the node
    *data = xpsRBTSearchNode(rbtree, key);

    return status;
}

XP_STATUS xpsStateGetNextDataForDevice(xpsDevice_t devId, xpsDbHandle_t handle,
                                       xpsDbKey_t key, void **data)
{
    XP_STATUS status = XP_NO_ERR;
    xpsRBTree_t *rbtree = NULL;

    // Verify handle validity
    if (!XPS_STATE_DB_HANDLE_IS_VALID(handle))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid DB handle");
        return XP_ERR_INVALID_PARAMS;
    }

    // Retrieve the tree form the appropriate bucket (devId hardcoded to 0 for global)
    rbtree = xpsStateGetDbForDevice(handle, devId);
    if (!rbtree)
    {
        return XP_ERR_NULL_POINTER;
    }

    // Get next ascending order data for the key.
    *data = xpsRBTGetInorderSuccessor(rbtree, key);

    return status;
}

XP_STATUS xpsStateDeleteDataForDevice(xpsDevice_t devId, xpsDbHandle_t handle,
                                      xpsDbKey_t key, void **data)
{
    XP_STATUS status = XP_NO_ERR;
    xpsRBTree_t *rbtree = NULL;

    // Verify handle validity
    if (!XPS_STATE_DB_HANDLE_IS_VALID(handle))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid DB handle");
        return XP_ERR_INVALID_PARAMS;
    }

    // Retrieve the tree form the appropriate bucket (devId hardcoded to 0 for global)
    rbtree = xpsStateGetDbForDevice(handle, devId);
    if (!rbtree)
    {
        return XP_ERR_NULL_POINTER;
    }

    // Delete the node if it exists, the memory allocated for data will be deleted by the caller
    *data = xpsRBTDeleteNode(rbtree, key);

    return status;
}

/*
 * Memory management APIs
 */

XP_STATUS xpsStateHeapMalloc(size_t size, void **data)
{
    *data = XP_PERSISTENT_MALLOC(size);
    if (!(*data))
    {
        return XP_ERR_OUT_OF_MEM;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsStateHeapFree(void *data)
{
    if (data)
    {
        XP_PERSISTENT_FREE(data);
        return XP_NO_ERR;
    }

    return XP_ERR_NULL_POINTER;
}

XP_STATUS xpsStateGetCount(xpsScope_t scopeId, xpsDbHandle_t handle,
                           uint32_t *count)
{
    XP_STATUS status = XP_NO_ERR;
    xpsRBTree_t *rbtree = NULL;
    // Retrieve the tree form the appropriate bucket (devId hardcoded to 0 for global)
    rbtree = xpsStateGetDb(handle, scopeId);
    if (!rbtree)
    {
        return XP_ERR_NULL_POINTER;
    }
    *count = rbtree->count;
    return status;
}

XP_STATUS xpsStateGetCountForDevice(xpsDbHandle_t dbHndl, xpsDevice_t devId,
                                    uint32_t *count)
{
    XP_STATUS status = XP_NO_ERR;
    xpsRBTree_t *rbtree = NULL;
    // Retrieve the tree form the appropriate bucket (devId hardcoded to 0 for global)
    rbtree = xpsStateGetDbForDevice(dbHndl, devId);
    if (!rbtree)
    {
        return XP_ERR_NULL_POINTER;
    }
    *count = rbtree->count;
    return status;
}

#ifdef __cplusplus
}
#endif
