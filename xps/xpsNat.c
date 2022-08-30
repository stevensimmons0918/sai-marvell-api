// xpNat.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsNat.h"
#include "xpsInit.h"
#include "xpsInternal.h"
#include "xpsLock.h"


#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpsNatInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNatInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsNatDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNatDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsNatAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNatRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNatSetMdtConfig(xpsDevice_t devId, uint32_t index, uint32_t value)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNatDeleteEntry(xpsDevice_t devId, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNatGetEntry(xpsDevice_t devId, uint32_t index,
                         xpsNatEntryKey_t *natKey, xpsNatEntryMask_t *natMask,
                         xpsNatEntryData_t *natData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNatAddDoubleEntry(xpsDevice_t devId, uint32_t index,
                               xpsNatEntryKey_t *natKey, xpsNatEntryMask_t* natMask,
                               xpsNatEntryData_t *natData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNatAddInternalEntry(xpsDevice_t devId, uint32_t index,
                                 xpsNatEntryKey_t *natKey, xpsNatEntryMask_t* natMask,
                                 xpsNatEntryData_t *natData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNatAddExternalEntry(xpsDevice_t devId, uint32_t index,
                                 xpsNatEntryKey_t *natKey, xpsNatEntryMask_t* natMask,
                                 xpsNatEntryData_t *natData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNatAddFilterRule(xpsDevice_t devId, uint32_t index,
                              xpsNatEntryKey_t *natKey, xpsNatEntryMask_t* natMask,
                              xpsNatEntryData_t *natData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
