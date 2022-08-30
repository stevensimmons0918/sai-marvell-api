// xpsEnums.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsEnums.h
 * \brief This file contains Enum definitions for various XPS
 *        XPS Features
 */

#ifndef _xpsEnums_h_
#define _xpsEnums_h_

#include "xpEnums.h"
#include "xpTypes.h"
#include "openXpsEnums.h"

typedef enum
{
    VLAN_NONE = 0,
    VLAN_PRIMARY,
    VLAN_COMMUNITY,
    VLAN_ISOLATED,
    VLAN_MAX
} xpsPrivateVlanType_e;


typedef enum
{
    XPS_FDB_PKT_CMD,
    XPS_FDB_IS_CONTROL_MAC,
    XPS_FDB_IS_ROUTER_MAC,
    XPS_FDB_IS_STATIC_MAC
} xpsFdbAttribute_e;

typedef enum
{
    XPS_VXLAN_TUNNEL= 0,
    XPS_NVGRE_TUNNEL,
    XPS_GENEVE_TUNNEL,
    XPS_PBB_TUNNEL,
    XPS_INVALID_TUNNEL
} xpsServiceInstanceType_e;

typedef enum
{
    XPS_AC_ALL_TAGGED,
    XPS_AC_ALL_UNTAGGED,
    XPS_AC_OUTER_VID,
    XPS_AC_OUTER_VID_ONLY_TAGGED,
    XPS_AC_DEFAULT,
    XPS_AC_MAX_TYPES
} xpsAcMatchType_e;

typedef xpL3EncapType_e xpsL3EncapType_e;
typedef xpVlanNatMode_e xpsVlanNatMode_e;
typedef xpUrpfMode_e xpsUrpfMode_e;
typedef xpFdbEntryAction_e xpsFdbEntryAction_e;

// typedef uint32_t xpsStp_t;
typedef xpEgressFilter_t xpsEgressFilter_t;
typedef xpPeg_t xpsPeg_t;
typedef XP_DIR_E xpsDirection_t;
typedef  XP_DEV_TYPE_T xpsDeviceType_e;
#endif  //_xpsEnums_h_
