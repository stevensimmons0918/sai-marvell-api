// xpsAging.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsAging.h"
#include "xpsCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpsAgeFifoHandler(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAgeFifoNumHandler(xpDevice_t devId, uint32_t fifoNum)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsReadAgeFifoMessage(xpsDevice_t devId, xpAgeFifoData *fifoInfo)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsProcessAgeFifoMessage(xpsDevice_t devId, xpAgeFifoData *fifoInfo)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSetAgingMode(xpsDevice_t devId, XP_AGE_MODE_T ageMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsGetAgingMode(xpsDevice_t devId, XP_AGE_MODE_T* ageMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsSetAgingCycleUnitTime(xpsDevice_t devId, uint32_t unitTime)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS  xpsGetAgingCycleUnitTime(xpsDevice_t devId, uint32_t* unitTime)
{
    return XP_NO_ERR;
}



XP_STATUS xpsDisableRefreshPause(xpsDevice_t devId, uint8_t disable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


#ifdef __cplusplus
}
#endif
