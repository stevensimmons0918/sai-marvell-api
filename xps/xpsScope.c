// xpsScope.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsScope.h"
#include "xpsState.h"
#include "xpsCommon.h"

#ifdef __cplusplus
extern "C" {
#endif


static xpsScope_t *xpsScopeArray = NULL;


XP_STATUS xpsScopeInit(xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    uint64_t uniqueObjId = 0;

    //Persistence allocaation for scope array
    if (!xpsScopeArray)
    {
        uniqueObjId = XP_ALLOCATOR_OID(0, (uint64_t)XPS_SCOPE_ARRAY_OID);
        xpsScopeArray = (xpsScope_t *)XP_ID_BASED_PERSISTENT_MALLOC(uniqueObjId,
                                                                    sizeof(xpsScope_t) * XP_MAX_DEVICES);
        if (!xpsScopeArray)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Malloc for Scope array failed");
            return XP_ERR_OUT_OF_MEM;
        }

        if (initType == INIT_COLD)
        {
            // Initialize the scope array
            memset(xpsScopeArray, 0, sizeof(xpsScope_t) * XP_MAX_DEVICES);
        }
    }
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsScopeDeInit()
{
    XPS_FUNC_ENTRY_LOG();



    if (xpsScopeArray)
    {
        // Release the memory allocated for scope array
        XP_ID_BASED_PERSISTENT_FREE(xpsScopeArray);
        xpsScopeArray = NULL;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsScopeAddDevice(xpsDevice_t devId, xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();


    if (!xpsScopeArray)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Scope array is NULL");
        return XP_ERR_NULL_POINTER;
    }

    //Populate the scope array
    xpsScopeArray[devId] = scopeId;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsScopeRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();


    xpsScope_t oldScopeId = 0;
    uint8_t devIndex = 0;

    if (!xpsScopeArray)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Scope array is NULL");
        return XP_ERR_NULL_POINTER;
    }
    oldScopeId = xpsScopeArray[devId];

    //Reset the scope for the device
    xpsScopeArray[devId] = XP_SCOPE_DEFAULT;

    //Release the allocators for the scope if no device is bound to this. Needed ?
    for (devIndex = 0; devIndex < XP_MAX_DEVICES; devIndex++)
    {
        if (xpsScopeArray[devIndex] == oldScopeId)
        {
            break;
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsScopeGetScopeId(xpsDevice_t devId, xpsScope_t *pScopeId)
{
    XPS_FUNC_ENTRY_LOG();

    *pScopeId = XP_SCOPE_DEFAULT;

    return XP_NO_ERR;
}

XP_STATUS xpsScopeGetFirstDevice(xpsScope_t scopeId, xpsDevice_t *firstDevId)
{
    XPS_FUNC_ENTRY_LOG();


    xpsDevice_t devId = 0;
    *firstDevId = XP_MAX_DEVICES;

    if (!xpsScopeArray)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Scope array is NULL");
        return XP_ERR_NULL_POINTER;
    }

    for (devId = 0; devId < XP_MAX_DEVICES; devId++)
    {
        if (IS_DEVICE_VALID(devId) == 0)
        {
            continue;
        }

        if (xpsScopeArray[devId] == scopeId)
        {
            *firstDevId = devId;
            break;
        }
    }

    //No device in this scope
    if (*firstDevId == XP_MAX_DEVICES)
    {
        return XP_ERR_NOT_FOUND;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsScopeGetNextDevice(xpsScope_t scopeId, xpsDevice_t curDevId,
                                xpsDevice_t *nextDevId)
{
    XPS_FUNC_ENTRY_LOG();


    *nextDevId = XP_MAX_DEVICES;

    if (!xpsScopeArray)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Scope array is NULL");
        return XP_ERR_NULL_POINTER;
    }

    if (xpsScopeArray[curDevId] != scopeId)
    {
        return XP_ERR_INVALID_ARG;
    }

    curDevId++;

    for (; curDevId < XP_MAX_DEVICES; curDevId++)
    {
        if (IS_DEVICE_VALID(curDevId) == 0)
        {
            continue;
        }

        if (xpsScopeArray[curDevId] == scopeId)
        {
            *nextDevId = curDevId;
            break;
        }
    }

    //No next device in this scope
    if (curDevId == XP_MAX_DEVICES)
    {
        return XP_ERR_NOT_FOUND;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/**
 * \brief This method returns the next unused scope index.
 *
 * \param [out] *freeScope
 *
 * \return XP_STATUS
 */
XP_STATUS xpsScopeGetFreeScope(xpsScope_t *freeScope)
{
    XPS_FUNC_ENTRY_LOG();


    uint32_t majorNum = 0;

    if (freeScope == 0)
    {
        return XP_ERR_NULL_POINTER;
    }

    for (uint32_t i = 0; i < XP_MAX_DEVICES; i++)
    {
        if (xpsScopeArray[i] > majorNum)
        {
            majorNum = xpsScopeArray[i];
        }
    }

    majorNum++;

    if (majorNum >= XP_MAX_DEVICES)
    {
        return XP_ERR_NOT_FOUND;
    }

    *freeScope = majorNum;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif

