/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsServiceCpuFw.c
*
* DESCRIPTION:
*           This file contains HWS Firmware loading support
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
******************************************************************************/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include "mvHwsServiceCpuInt.h"

#define INTR_SIMULATION

#ifdef INTR_SIMULATION
MV_HWS_SERV_CPU_INT servCpuIntrMapSim;
#endif /* INTR_SIMULATION */

MV_HWS_SERV_CPU_INT *servCpuIntrMap;

GT_VOID mvHwsServCpuLock(GT_VOID);
GT_VOID mvHwsServCpuUnLock(GT_VOID);

/**
* @internal mvHwsServCpuIntrInit function
* @endinternal
*
* @brief   HWS Service CPU interrupt init
*/
GT_VOID mvHwsServCpuIntrInit(GT_U32 sharedMemBaseAddr)
{
#ifdef INTR_SIMULATION
    sharedMemBaseAddr = sharedMemBaseAddr;/*Avoid warning*/
    servCpuIntrMap = &servCpuIntrMapSim;
#else
    servCpuIntrMap = (void*)sharedMemBaseAddr;
#endif
}

/**
* @internal mvHwsServCpuIntrSet function
* @endinternal
*
* @brief   HWS Service CPU interrupt config
*         Update port interrupt status & event, including event summary
*/
GT_VOID mvHwsServCpuIntrSet(GT_U32 port, GT_U32 intr)
{
    if ((port >= MV_HWS_PORT_INT_MAX_NUM) ||
        (intr > MV_HWS_PORT_LINK_EVENT))
    {
        return;
    }

    if (!(port & servCpuIntrMap->portIntMask))
    {
        return;
    }

    mvHwsServCpuLock();

    /* Update port interrupt status */
    if (intr & MV_HWS_PORT_HCD_EVENT)
    {
        servCpuIntrMap->portInt[port].portIntStatus &= ~(MV_HWS_PORT_HCD_MASK);
    }
    servCpuIntrMap->portInt[port].portIntStatus |= intr;

    /* Update port interrupt event */
    servCpuIntrMap->portInt[port].portIntStatus |= (intr & MV_HWS_PORT_EVENT_MASK);

    /* Update Service CPU interrupt event */
    servCpuIntrMap->portIntEvent |= 1 << port;

    /* Update Service CPU interrupt event summary */
    servCpuIntrMap->portIntSummary |= 1 << (port / MV_HWS_GROUP_EVENT_NUM);

    mvHwsServCpuUnLock();

    /* Trigger Doorbell interrupt */
}

/**
* @internal mvHwsServCpuIntrGet function
* @endinternal
*
* @brief   HWS Service CPU interrupt retrive
*         Return interrupt map to calling interrupt handler
*         Clear active interrupt events
*/
GT_VOID mvHwsServCpuIntrGet(MV_HWS_SERV_CPU_INT *intrMap)
{
    mvHwsServCpuLock();

    /* Copy Interrupt map to Interrupt handler */
    hwsOsMemCopyFuncPtr(intrMap, servCpuIntrMap, sizeof(MV_HWS_SERV_CPU_INT));

    /* Clear active interrupt events */
    mvHwsServCpuIntrClear();

    mvHwsServCpuUnLock();
}

/**
* @internal mvHwsServCpuIntrClear function
* @endinternal
*
* @brief   HWS Service CPU interrupt clear
*         Clear port interrupt event
*         Clear Service CPU interrupt event
*         Clear Service CPU interrupt event summary
*/
GT_VOID mvHwsServCpuIntrClear(void)
{
    GT_U32 groupIndex;
    GT_U32 portIndex;
    GT_U32 portBase;

    for (groupIndex = 0; groupIndex < MV_HWS_GROUP_EVENT_NUM; groupIndex++)
    {
        if (servCpuIntrMap->portIntSummary & (1 << groupIndex))
        {
            portBase = groupIndex * MV_HWS_GROUP_EVENT_NUM;

            for (portIndex = 0; portIndex < MV_HWS_PORT_EVENT_NUM; portIndex++)
            {
                if (servCpuIntrMap->portIntEvent & (1 << (portIndex + portBase)))
                {
                    /* Clear port interrupt event */
                    servCpuIntrMap->portInt[portIndex].portIntEvent &= ~(MV_HWS_PORT_EVENT_MASK);
                }
            }
        }
    }

    /* Clear Service CPU interrupt event */
    servCpuIntrMap->portIntEvent = 0;

    /* Clear Service CPU interrupt event summary */
    servCpuIntrMap->portIntSummary = 0;
}

/**
* @internal mvHwsServCpuIntrStatusReset function
* @endinternal
*
* @brief   HWS Service CPU interrupt status reset
*         Updates only interrupt status Not event
*/
GT_VOID mvHwsServCpuIntrStatusReset(GT_U32 port)
{
    if (port >= MV_HWS_PORT_INT_MAX_NUM)
    {
        return;
    }

    if (!(port & servCpuIntrMap->portIntMask))
    {
        return;
    }

    mvHwsServCpuLock();

    /* Reset port interrupt status */
    servCpuIntrMap->portInt[port].portIntStatus = 0;

    mvHwsServCpuUnLock();
}

/**
* @internal mvHwsServCpuIntrHandler function
* @endinternal
*
* @brief   HWS Service CPU interrupt handler
*         This API should be called from the interrupt handler after retriving
*         interrupt map via mvHwsServCpuIntrGet
*/
GT_VOID mvHwsServCpuIntrHandler(void)
{
    GT_U32 groupIndex;
    GT_U32 portIndex;
    GT_U32 portBase;

    for (groupIndex = 0; groupIndex < MV_HWS_GROUP_EVENT_NUM; groupIndex++)
    {
        if (servCpuIntrMap->portIntSummary & (1 << groupIndex))
        {
            portBase = groupIndex * MV_HWS_GROUP_EVENT_NUM;

            for (portIndex = 0; portIndex < MV_HWS_PORT_EVENT_NUM; portIndex++)
            {
                if ((servCpuIntrMap->portIntEvent) & (1 << (portIndex + portBase)))
                {
                    if (servCpuIntrMap->portInt[portIndex].portIntEvent & MV_HWS_PORT_HCD_EVENT)
                    {
                        /*
                        ** Handle HCD Event
                        ** ================
                        ** Port = Port Index (0-31) + Port Base(Base AP physical port e.g: Bobcat2 = 48)
                        **
                        ** Type = ((servCpuIntrMap->portInt[portIndex].portIntStatus & MV_HWS_PORT_HCD_MASK) >>
                        **                                                                       MV_HWS_PORT_HCD_SHIFT)
                        ** HCD Types: Port_1000Base_KX = 0
                        **            Port_10GBase_KX4 = 1
                        **            Port_10GBase_R   = 2
                        **            Port_40GBase_R   = 3
                        */
                    }
                    if (servCpuIntrMap->portInt[portIndex].portIntEvent & MV_HWS_PORT_LINK_EVENT)
                    {
                        /*
                        ** Handle Link Event
                        ** ================
                        ** Port = Port Index (0-31) + Port Base(Base AP physical port e.g: Bobcat2 = 48)
                        */
                    }
                }
            }
        }
    }
}

/**
* @internal mvHwsServCpuLock function
* @endinternal
*
* @brief   HWS Service CPU Lock
*         Hw Semaphore for interupt event update
*/
GT_VOID mvHwsServCpuLock(GT_VOID)
{

}

/**
* @internal mvHwsServCpuUnLock function
* @endinternal
*
* @brief   HWS Service CPU UnLock
*         Hw Semaphore for interupt event update
*/
GT_VOID mvHwsServCpuUnLock(GT_VOID)
{

}


