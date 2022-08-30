/************************************************************************
 * Copyright (c) Marvell [2000-2020].
 * All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may
 *   not use this file except in compliance with the License. You may obtain
 *   a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.
 *
 *   THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 *   CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *   LIMITATIONS ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 *   FOR PARTICULAR PURPOSE, MERCHANTABILITY OR NON-INFRINGEMENT.
 *
 *   See the Apache Version 2.0 License for specific language governing
 *   permissions and limitations under the License.
 *
 *   FILE : openXpsReasonCodeTable.h
 *
 *   Abstract: This file defines the reason codes in OpenXPS.
 ************************************************************************/
/**
 * \file openXpsReasonCodeTable.h
 * \brief This file contains API prototypes and type definitions
 *        for the openXps Reason Code Table
 * \Copyright (c) Marvell [2000-2020]
 */

#ifndef _openXpsReasonCodeTable_h_
#define _openXpsReasonCodeTable_h_

#define XP_BRIDGE_MAC_SA_NEW                    11  ///< FDB MAC learning: New MAC SA indication
#define XP_BRIDGE_MAC_SA_MOVE                   12  ///< FDB MAC learning: MAC SA has been moved indication

#define XP_ROUTE_RC_NH_TABLE_HIT                60  ///< NextHope table hit indication

#define XP_IVIF_RC_BPDU                         103 ///< BPDU

#define XP_BRIDGE_RC_IVIF_ARPIGMPICMP_CMD       206 ///< ARP Broadcast Ingress VIF control
#define XP_BRIDGE_RC_IVIF_SA_MISS               211 ///< MAC SA Miss Ingress VIF control

#define XP_ROUTE_RC_HOST_TABLE_HIT              302 ///< Host Table hit indication
#define XP_ROUTE_RC_UC_TABLE_MISS               307 ///< Miss when Unicast Routing is enabled
#define XP_ROUTE_RC_ROUTE_NOT_POSSIBLE          312 ///< IP Routing failure
#define XP_ROUTE_RC_TTL1_OR_IP_OPTION           313 ///< IP Options or TTL=1
#define XP_ROUTE_RC_ARP_MY_UNICAST              316 ///< Route ARP my unicast

#endif
