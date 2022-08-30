// xps8021Br.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsInterface.h"
#include "xpsPort.h"
#include "xps8021Br.h"
#include "xpsInternal.h"
#include "xpsLock.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file xps8021Br.c
 * \brief Provides xps a utility to manage Extended ports.
 *        Extends xpsInterface.c
 *
 * This file manages extended interfaces using xpsInterface.c
 *
 */

#define XPS_IF_LOG(type, ...)  LOGFN(xpLogModXps, XP_SUBMOD_MAIN, type, ##__VA_ARGS__)

/*
 * XPS 802.1Br Private APIs
 */

/**
 * \private
 * \brief Enables or Disables E-TAG support on cascade port
 *
 * \param [in]  xpsInterfaceId_t intfId
 * \param [in]          enable
 *
 * \return XP_STATUS
 */


/**
 * \private
 * \brief Get pointer to the number of usages of port as cascade
 *        port
 *
 * \param [in]  xpsInterfaceId_t intfId
 * \param [in] **      usages
 *
 * \return XP_STATUS
 */


/**
 * \private
 * \brief Increase number of usages of port as cascade port
 *
 * \param [in]  xpsInterfaceId_t intfId
 *
 * \return XP_STATUS
 */


/**
 * \private
 * \brief Decrease number of usages of port as cascade port
 *
 * \param [in]  xpsInterfaceId_t intfId
 *
 * \return XP_STATUS
 */


/*
 * XPS 802.1Br Public APIs
 */

XP_STATUS xps8021BrCreatePortExtenderGroup(uint16_t numOfPorts,
                                           xpsPeg_t *groupId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xps8021BrCreatePortExtenderGroupScope(xpsScope_t scopeId,
                                                uint16_t numOfPorts, xpsPeg_t *groupId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrDeletePortExtenderGroup(xpsPeg_t groupId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrDeletePortExtenderGroupScope(xpsScope_t scopeId,
                                                xpsPeg_t groupId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrCreateExtendedPort(xpsPeg_t groupId,
                                      uint32_t maxCascadePorts, xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrCreateExtendedPortScope(xpsScope_t scopeId, xpsPeg_t groupId,
                                           uint32_t maxCascadePorts, xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrInitExtendedPort(xpsDevice_t devId, xpsPeg_t groupId,
                                    xpsInterfaceId_t extendedPort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrBindExtendedPort(xpsPeg_t groupId,
                                    xpsInterfaceId_t extendedPort, xpsInterfaceId_t cascadePort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrBindExtendedPortScope(xpsScope_t scopeId, xpsPeg_t groupId,
                                         xpsInterfaceId_t extendedPort, xpsInterfaceId_t cascadePort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrUnBindExtendedPort(xpsPeg_t groupId,
                                      xpsInterfaceId_t extendedPort, xpsInterfaceId_t cascadePort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrUnBindExtendedPortScope(xpsScope_t scopeId, xpsPeg_t groupId,
                                           xpsInterfaceId_t extendedPort, xpsInterfaceId_t cascadePort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrDeleteExtendedPort(xpsPeg_t groupId, xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrDeleteExtendedPortScope(xpsScope_t scopeId, xpsPeg_t groupId,
                                           xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrIsExtendedPortValid(xpsPeg_t groupId,
                                       xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrIsExtendedPortValidScope(xpsScope_t scopeId,
                                            xpsPeg_t groupId, xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrIsCascadePortBindToExtendedPort(xpsInterfaceId_t
                                                   extendedPort, xpsInterfaceId_t cascadePort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrIsCascadePortBindToExtendedPortScope(xpsScope_t scopeId,
                                                        xpsInterfaceId_t extendedPort, xpsInterfaceId_t cascadePort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrGetVlanMulticastInterface(xpsVlan_t vlan,
                                             xpL2EncapType_e encapType, xpsInterfaceId_t *mcIntf)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrGetVlanMulticastInterfaceScope(xpsScope_t scopeId,
                                                  xpsVlan_t vlan, xpL2EncapType_e encapType, xpsInterfaceId_t *mcIntf)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrGetL2MulticastInterface(xpsMcL2InterfaceListId_t ifListId,
                                           xpL2EncapType_e encapType, xpsInterfaceId_t *mcIntf)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrGetL2MulticastInterfaceScope(xpsScope_t scopeId,
                                                xpsMcL2InterfaceListId_t ifListId, xpL2EncapType_e encapType,
                                                xpsInterfaceId_t *mcIntf)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrGetL3MulticastInterface(xpsMcL3InterfaceListId_t ifListId,
                                           xpsInterfaceId_t l3IntfId, xpL3EncapType_e encapType, xpsInterfaceId_t *mcIntf)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xps8021BrGetL3MulticastInterfaceScope(xpsScope_t scopeId,
                                                xpsMcL3InterfaceListId_t ifListId, xpsInterfaceId_t l3IntfId,
                                                xpL3EncapType_e encapType, xpsInterfaceId_t *mcIntf)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


#ifdef __cplusplus
}
#endif

