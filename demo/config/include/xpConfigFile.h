// xpConfigFile.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef __XPCONFIGFILE_H__
#define __XPCONFIGFILE_H__

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include "xpTypes.h"

#define XP_IPV4_ADDR_LEN            4
#define XP_IPV6_ADDR_LEN            16
#define MAX_ENCAP_TYPE              13
#define XP_NUM_MIRROR_ENTRY         100
#define XP_MAX_QUEUES_PER_PORT     (16)
#define CONFIG_DEBUG_ENABLE         0

typedef struct _entryList
{
    void **data;
    int size;
    int count;
} entryList;

typedef struct
{
    uint16_t    port;
    uint8_t tagType;
    uint8_t numOfExtPorts;
    uint8_t stpState;
    uint8_t setIngressVif;
    uint8_t disableTunnelVif;
    uint8_t mirrorToAnalyzerMask;
    uint8_t setBd;
    uint8_t enVlanHairPining;
    uint8_t     enPbb;
    uint8_t natMode;
    uint8_t     natScope;
    uint8_t     enRouterACL;
    uint32_t    routeAclId;
    uint8_t     enBridgeACL;
    uint32_t    bridgeAclId;
    uint8_t     enIpv4Tunnel;
    uint8_t     enMplsTunnel;
    uint8_t     enOpenFlow;
    uint8_t     setEgressVif;
    uint16_t    evif;
    uint8_t     encapType;
} PortEntry;

typedef struct
{
    int32_t                index;
    int                     mdtRootIndex;
    uint32_t portList[SYSTEM_MAX_PORT];
    uint32_t    numPorts;
    int8_t vifType;
    int8_t tnlType;
    uint32_t tnlIdx;
} MvifEntry;

typedef struct
{
    uint32_t addEtagPorts[SYSTEM_MAX_PORT];
    uint32_t numOfAddEtagPorts;
    uint32_t keepEtagPorts[SYSTEM_MAX_PORT];
    uint32_t numOfKeepEtagPorts;
    uint32_t addEtagLags[SYSTEM_MAX_PORT];
    uint32_t numOfAddEtagLags;
    uint32_t keepEtagLags[SYSTEM_MAX_PORT];
    uint32_t numOfKeepEtagLags;
    uint32_t stripEtagLagVif[SYSTEM_MAX_PORT];
    uint32_t numOfStripEtagLagVif;
    uint32_t stripEtagVif[SYSTEM_MAX_PORT];
    uint32_t numOfStripEtagVif;
    uint32_t longEtagModePort[SYSTEM_MAX_PORT];
    uint32_t numOflongEtagModePort;
    uint32_t byPassPipelinePort[SYSTEM_MAX_PORT][2];
    uint32_t numOfByPassPipelinePort;

} TrustBrEntry;

typedef struct
{
    int32_t evif;
    uint32_t egressPorts[SYSTEM_MAX_PORT];
    uint32_t numOfEgressPorts;
    uint32_t egressLags[SYSTEM_MAX_PORT];
    uint32_t numOfEgressLags;
} VifEntry;

typedef struct
{
    uint32_t vrfIdx;
    uint8_t  v4RouteEn;
    uint8_t  v6RouteEn;
} VrfEntry;

typedef struct
{
    uint32_t index;
    int32_t nxtL2Ptr;
    int32_t        mvifIndex;
    uint32_t mirrorSessionId;
    uint32_t nextEngine;
    uint32_t isLast;
    uint16_t        encap;
    uint32_t  encapData;
    uint8_t disableVlanTranslation;
} l2MdtEntry;

typedef struct
{
    uint32_t index;
    int32_t nxtL3Ptr;
    int32_t nxtL2Ptr;
    uint32_t altVif;
    int32_t        mvifIndex;
    uint32_t mirrorSessionId;
    uint32_t nextEngine;
    uint32_t isLast;
    uint16_t        encap;
    uint32_t  encapData;
    uint16_t cVlan;
} l3MdtEntry;

typedef struct
{
    uint32_t            idx;
    uint8_t     sip[XP_IPV4_ADDR_LEN];
    uint8_t     grpAddr[XP_IPV4_ADDR_LEN];
    uint8_t     pktCmd;
    uint8_t     agingFlag;
    uint16_t    vlan:12;
    uint32_t    mvif;
    uint32_t    nodeIdx[10];
} Ipv4BridgeMcEntry;

typedef struct
{
    uint32_t            idx;
    uint8_t     sip[XP_IPV6_ADDR_LEN];
    uint8_t     grpAddr[XP_IPV6_ADDR_LEN];
    uint8_t     pktCmd;
    uint16_t    vlan:12;
    uint32_t    mvif;
    uint32_t    nodeIdx[10];
} Ipv6BridgeMcEntry;

typedef struct
{
    uint16_t vlanId;
    uint32_t nodeIdx[10];
} vlanToNodeIdxMap;

typedef struct
{
    uint32_t            idx;
    uint8_t     sip[XP_IPV4_ADDR_LEN];
    uint8_t     grpAddr[XP_IPV4_ADDR_LEN];
    int32_t     vrfId;
    uint32_t    pktCmd;
    uint32_t    mvif;
    uint8_t     agingFlag;
    uint32_t    rpfType;
    uint32_t    rpfValue;
    uint32_t    rpfFailCmd;
    int32_t vlanForIvifRpf;
    int32_t     isMartialIp;
    uint32_t    reasonCode;
    vlanToNodeIdxMap vlanNode[10];
} Ipv4RouteMcEntry;

typedef struct
{
    uint32_t            idx;
    uint8_t     sip[XP_IPV6_ADDR_LEN];
    uint8_t     grpAddr[XP_IPV6_ADDR_LEN];
    int32_t     vrfId;
    uint32_t    pktCmd;
    uint32_t    mvif;
    uint32_t    rpfType;
    uint32_t    rpfValue;
    uint32_t    rpfFailCmd;
    int32_t vlanForIvifRpf;
    uint32_t    isMartialIp;
    uint32_t    reasonCode;
    vlanToNodeIdxMap vlanNode[10];
} Ipv6RouteMcEntry;

/*
 *localSid data for END* fucntions
 * */
typedef struct
{
    uint32_t func;       ///< func
    uint32_t args;       ///< func
    uint32_t VRFId;      ///< VRFId
    uint32_t nhId;       ///< nhId
    uint32_t ecmpEnable;       ///< nhId
    uint32_t flag;
    uint8_t     localSid[XP_IPV6_ADDR_LEN];
} localSidEntry;

typedef struct
{
    uint32_t        vrfId;
    uint32_t        bdId;
    uint8_t     grpAddr[XP_IPV4_ADDR_LEN];
    uint32_t        rpfValue;
} Ipv4PimBiDirEntry;

typedef struct
{
    uint16_t    vlan:12;        // Maximum 4k.
    uint8_t     macSAmissCmd;
    uint8_t     broadcastCmd;
    uint8_t     ipv4ARPBCCmd;
    uint8_t     ipv4RouteEn;
    uint8_t     ipv6RouteEn;
    uint8_t     ipv4BridgeMcMode;
    uint8_t     ipv6BridgeMcMode;
    uint8_t     ipv4RouteMcEn;
    uint8_t     ipv6RouteMcEn;
    uint8_t             mplsRouteEn;
    uint8_t     ipv4RouteMcMode;
    uint8_t     ipv6RouteMcMode;
    uint8_t     vrfId;
    uint8_t     unregMcCmd;
    PortEntry portData[SYSTEM_MAX_PORT]; //TO-DO : need to change the actual size
    uint8_t     numPorts;
    uint8_t     vlanType;
    uint16_t    parentVlan:12;
    uint8_t     enL3Inf;
    uint8_t     macSALSBs;
    uint8_t     natMode;
    uint8_t     natScope;
    uint8_t     natEn;
    uint8_t     ipv4UrpfEn;
    uint8_t     ipv4UrpfMode;
    uint8_t     ipv6UrpfEn;
    uint8_t     ipv6UrpfMode;
    uint32_t    ipv4MtuLen;
    uint32_t    ipv6MtuLen;
    uint8_t     mtuPktCmd;
    uint32_t    mvifIdx[10];
    uint32_t    mcVifIdx;
    uint32_t    mcastIdx;
} VlanEntry;

typedef struct
{
    uint16_t    vlan:12;        // Maximum 4k.
    uint8_t     macSAmissCmd;
    uint8_t     broadcastCmd;
    uint8_t     ipv4ARPBCCmd;
    uint8_t     ipv4RouteEn;
    uint8_t     ipv6RouteEn;
    uint8_t     ipv4BridgeMcMode;
    uint8_t     ipv6BridgeMcMode;
    uint8_t     ipv4RouteMcEn;
    uint8_t     ipv6RouteMcEn;
    uint8_t             mplsRouteEn;
    uint8_t     ipv4RouteMcMode;
    uint8_t     ipv6RouteMcMode;
    uint8_t     vrfId;
    uint8_t     unregMcCmd;
    PortEntry portData[SYSTEM_MAX_PORT]; //TO-DO : need to change the actual size
    uint8_t     numPorts;
    uint8_t     vlanType;
    uint16_t    parentVlan:12;
    uint8_t     enL3Inf;
    uint8_t     macSALSBs;
    uint8_t     natMode;
    uint8_t     natScope;
    uint8_t     natEn;
    uint8_t     ipv4UrpfEn;
    uint8_t     ipv4UrpfMode;
    uint8_t     ipv6UrpfEn;
    uint32_t    ipv4MtuLen;
    uint32_t    ipv6MtuLen;
    uint8_t     mtuPktCmd;
    uint8_t     ipv6UrpfMode;
    uint8_t     scopeId;
} VlanScopeEntry;

typedef struct
{
    uint8_t     macAddr[XP_MAC_ADDR_LEN];
    int         vlan;
    int         port;
    uint8_t     encapType;
    uint8_t     controlMac;
    uint8_t     routerMac;
    uint8_t     pktCmd;
    int         tunnelIdx;
    uint32_t    geneveTunnelType;
    int         isLag;
    int         lagId;
    int16_t    extendedPortIdx;
    int         isMcast;
    uint32_t    mvifIdx[10];
    uint32_t    vifMcastIdx;
    uint32_t    mcastIdx;
    uint8_t agingFlag;
} MacEntry;

typedef struct
{
    uint8_t      macAddr[XP_MAC_ADDR_LEN];
    int          vlanId;
    int          reasonCode;
} FdbControlMacEntry;

typedef struct
{
    uint8_t    macAddr[XP_MAC_ADDR_LEN];
    int        vlan;
    int        port;
    uint8_t    encapType;
    uint8_t    controlMac;
    uint8_t    routerMac;
    uint8_t    pktCmd;
    int        tunnelIdx;
    uint32_t   geneveTunnelType;
    int        isLag;
    int        lagId;
    int16_t    extendedPortIdx;
    int        isMcast;
    uint32_t   mvifIdx[10];
    uint32_t   vifMcastIdx;
    uint8_t    scopeId;
    uint8_t    agingFlag;
} MacScopeEntry;

typedef struct
{
    uint8_t     ipv4Addr[XP_IPV4_ADDR_LEN];
    uint8_t     vrf;
    uint32_t    nhId;
} Ipv4HostEntry;

typedef struct
{
    uint8_t ipv4Addr[XP_IPV4_ADDR_LEN];
    uint8_t vrf;
    uint32_t    nhId;
    uint8_t scopeId;
} Ipv4HostScopeEntry;

typedef struct
{
    uint8_t     ipv4Addr[XP_IPV4_ADDR_LEN];
    uint8_t     vrf;
    uint32_t    reasonCode;
} Ipv4HostControlEntry;

typedef struct
{
    uint8_t     ipv6Addr[XP_IPV6_ADDR_LEN];
    uint8_t     vrf;
    uint32_t    reasonCode;
} Ipv6HostControlEntry;

typedef struct
{
    uint8_t     ipv4Addr[XP_IPV4_ADDR_LEN];
    uint8_t     vrf;
    uint32_t    netMask;
    uint32_t    ecmpSize;
    entryList   nhId;
} Ipv4RouteEntry;

typedef struct
{
    uint8_t     ipv4Addr[XP_IPV4_ADDR_LEN];
    uint8_t     vrf;
    uint32_t    netMask;
    uint32_t    ecmpSize;
    entryList   nhId;
    uint8_t     scopeId;
} Ipv4RouteScopeEntry;

typedef struct
{
    uint8_t     ipv6Addr[XP_IPV6_ADDR_LEN];
    uint8_t     vrf;
    uint32_t    nhId;
} Ipv6HostEntry;

typedef struct
{
    uint8_t     ipv6Addr[XP_IPV6_ADDR_LEN];
    uint8_t     vrf;
    uint32_t    netMask;
    uint32_t    ecmpSize;
    uint32_t    srIdx;
    entryList   nhId;
} Ipv6RouteEntry;

typedef struct
{
    uint8_t     macAddr[XP_MAC_ADDR_LEN];
    uint32_t    destPort;
    uint32_t    nhId;
    xpVlan_t    cTagVid;
    uint32_t    virtualId:24;
    uint32_t    isTagged;
    int32_t     tnlIdx;
    int32_t     mplsIdx;
    uint32_t    geneveTunnelType;
    uint8_t     pktCmd;
    uint8_t     procTTL;
    uint8_t     nhType;
    int32_t     lagId;
    int32_t     extendedPortIdx;
    int32_t     reasonCode;
    int32_t     allocateNhWithId;
} IpxNhEntry;

typedef struct
{
    uint8_t macAddr[XP_MAC_ADDR_LEN];
    uint32_t    destPort;
    uint32_t    nhId;
    xpVlan_t    cTagVid;
    uint32_t    virtualId:24;
    uint32_t    isTagged;
    int32_t tnlIdx;
    int32_t mplsIdx;
    uint32_t    geneveTunnelType;
    uint8_t     pktCmd;
    uint8_t     procTTL;
    uint8_t     nhType;
    int32_t     lagId;
    int32_t     extendedPortIdx;
    int32_t     reasonCode;
    uint8_t     scopeId;
} IpxNhScopeEntry;

typedef enum
{
    XP_CONFIGFILE_IPTNL_TYPE_VXLAN,
    XP_CONFIGFILE_IPTNL_TYPE_NVGRE,
    XP_CONFIGFILE_IPTNL_TYPE_GRE,
    XP_CONFIGFILE_IPTNL_TYPE_IP_OVER_IP,
    XP_CONFIGFILE_IPTNL_TYPE_GENEVE,
    XP_CONFIGFILE_IPTNL_TYPE_VPN_GRE,
    XP_CONFIGFILE_IPTNL_TYPE_VPN_GRE_LOOSE,
    XP_CONFIGFILE_IPTNL_TYPE_GRE_ERSPAN2,
    XP_CONFIGFILE_TNL_TYPE_SRH,
} IpTunnelType;

typedef struct
{
    int         tnlIdx;
    uint8_t     dmac[XP_MAC_ADDR_LEN];
    uint8_t     smac;
    uint8_t     tagType;
    uint8_t     dip[XP_IPV4_ADDR_LEN];
    uint8_t     myVtepIp[XP_IPV4_ADDR_LEN];
    int         vni;
    uint16_t    vlan;
    int         udpPort;
    int         tunnelVif;
    uint16_t    portId;
    uint16_t    bdId:12;    // Maximum 4k.
    uint8_t     isMcast;
    IpTunnelType    tnlType;
    uint8_t     setBd;
    uint16_t greProtocolId;
    uint16_t    tnlVlan:12;     // Maximum 4k.
    uint8_t     ipv4ARPBCCmd;
    uint8_t     ipv4RouteEn;
    uint8_t     ipv6RouteEn;
    uint8_t     ipv4BridgeMcMode;
    uint8_t     ipv6BridgeMcMode;
    uint8_t     ipv4RouteMcEn;
    uint8_t     ipv6RouteMcEn;
    uint8_t     mplsRouteEn;
    uint8_t     ipv4RouteMcMode;
    uint8_t     ipv6RouteMcMode;
    uint8_t     vrfId;
    uint8_t     unregMcCmd;
    uint8_t     bind2Vlan;
    uint32_t    bindVlan;
    uint32_t    nhId;
    uint32_t    pktCmd;
} TunnelEntry;

typedef struct
{
    uint8_t     tagType;
    uint16_t    sTag:12;
    uint16_t    cTag:12;
    uint16_t    vlan;
    int         tunnelVif;
    uint16_t    portId;
    uint16_t    bdId:12;    // Maximum 4k.
    uint8_t     isMcast;
} TunnelQinQEntry;

typedef struct
{
    uint8_t     bDa[XP_MAC_ADDR_LEN];
    uint8_t     bSa[XP_MAC_ADDR_LEN];
    uint16_t    bTag:12;
    uint32_t    iSID:24;
    uint16_t    portId;
    uint16_t    bdId:12;    // Maximum 4k.
    uint8_t     bind2Vlan;
    uint32_t    bindVlan;
    uint32_t    pktCmd;
    uint8_t     isEndPt;
} TunnelPBBEntry;

typedef struct
{
    uint8_t     tagType;
    uint8_t     numOfLables;
    uint32_t    label0;
    uint32_t    label1;
    uint16_t    vlan;
    int         tunnelVif;
    uint16_t    portId;
    uint16_t    bdId:12;    // Maximum 4k.
    uint32_t    mplsIdx;
    uint8_t     macSALSBs;
    uint8_t     macDA[XP_MAC_ADDR_LEN];
    uint8_t     entryType;
    uint8_t     setBd;
    uint16_t    tnlVlan:12;     // Maximum 4k.
    uint8_t     ipv4ARPBCCmd;
    uint8_t     ipv4RouteEn;
    uint8_t     ipv6RouteEn;
    uint8_t     ipv4BridgeMcMode;
    uint8_t     ipv6BridgeMcMode;
    uint8_t     ipv4RouteMcEn;
    uint8_t     ipv6RouteMcEn;
    uint8_t     mplsRouteEn;
    uint8_t     ipv4RouteMcMode;
    uint8_t     ipv6RouteMcMode;
    uint8_t     vrfId;
    uint8_t     unregMcCmd;
    uint32_t    nhId;
} TunnelMplsEntry;

typedef struct
{
    uint32_t    keyMplsLabel;
    uint8_t     keyEntryFormat;
    uint8_t     pktCmd;
    uint8_t     propTTL;
    uint16_t    CTagVid;
    uint16_t    egressVif;
    uint8_t     mplsOper;
    uint32_t    mplsSwapLabel:24;
    uint8_t     macDA[XP_MAC_ADDR_LEN];
    uint32_t    encapType;      // for egress packet outer ethernet header
    uint32_t    numOfLabel;     // for Tnl Origination
    uint32_t    nhId;
} MplsLabelEntry;

typedef struct
{
    uint16_t    bdId:12;        // Maximum 4k.
    uint16_t    floodVif:12;    // Maximum 4k.
    uint8_t     macSAmode;
    uint8_t     macSAmissCmd;
    uint8_t     broadcastCmd;
    uint8_t     ipv4ARPBCCmd;
    uint8_t     ipv4UnicastRouteEn;
    uint8_t     ipv6UnicastRouteEn;
    uint8_t     vrf;
} BdEntry;

typedef struct
{
    uint16_t    keyVlanId:12;
    uint16_t    maskVlanId:12;
    uint8_t     keyMacAddr[XP_MAC_ADDR_LEN];
    uint8_t     maskMacAddr[XP_MAC_ADDR_LEN];
    uint8_t     routerMac;
    uint8_t     controlMac;
    uint8_t     controlActionEn;
} ControlMacEntry;


typedef struct
{
    uint16_t    vlanId;
    uint32_t    tenantId;
    uint8_t     tunnelType;
} VlanTenantIdEntry;


typedef struct
{
    uint16_t    port;
    uint8_t     portState;
    uint8_t     acceptedFrameType;
    uint8_t     disableTunnelVif;
    uint8_t     disableAclPbr;
    uint8_t     setBd;
    uint16_t    defaultVlan;
    uint8_t     privateWireEn;
    uint16_t    evif;
    uint8_t     addPortBasedTag;
    uint8_t     macSaMissCmd;
    uint8_t     bumPolicerEn;
    uint8_t     portDefaultDEI;
    uint8_t     portDefaultPCP;
    uint8_t     etagExists;
} PortConfigEntry;

typedef struct
{
    uint16_t        lagId;
    uint16_t        vlan;
    xpPort_t        ports[SYSTEM_MAX_PORT];
    uint8_t         numPorts;
    uint8_t         tagType;
    uint8_t         stpState;
} PortLagEntry;

typedef enum MirrorType
{
    TYPE_PORT,
    TYPE_VLAN,
    TYPE_LAG,
    NUM_TYPES
} MirrorType;

typedef struct MirrorSource
{
    int                 type;
    const char *typeNames[NUM_TYPES];
    union
    {
        xpPort_t        port;
        xpVlan_t        vlan;
        uint32_t        lag;
    } id;
} MirrorSource;

typedef struct mirrorLocalData
{
    xpPort_t        analyzers[SYSTEM_MAX_PORT];  // Port VIFs only.
} mirrorLocalData;

typedef struct mirrorErspan2Data
{
    uint32_t        erspanSessionId;
    uint32_t        tunnelTableIndex;
} mirrorErspan2Data;

typedef union mirrorData
{
    mirrorLocalData mirrorData;
    mirrorErspan2Data erspanData;
} mirrorData;

typedef struct MirrorEntry
{
    uint32_t        sessionId;
    uint32_t        bucketId;
    uint8_t         mirrorType;
    uint8_t         dir;
    uint8_t         numAnalyzers;
    mirrorData      data;
    MirrorSource    sources[XP_NUM_MIRROR_ENTRY];
    uint8_t         numSources;
} MirrorEntry;

typedef struct
{
    ipv4Addr_t  keySrcAddress;
    xpPort_t    keySrcPort;
    ipv4Addr_t  keyDestAddress;
    xpPort_t    keyDestPort;
    uint32_t    bd;
    uint16_t    flag;
    uint8_t     protocol;
    uint8_t     pktCmd;
    ipv4Addr_t  sipAddress;
    xpPort_t    srcPort;
    ipv4Addr_t  dipAddress;
    xpPort_t    destPort;
    xpPort_t    port;
    uint8_t     mdtNatConfig;
    uint8_t     natMode;
    uint8_t     natScope;
} NatEntry;

typedef struct
{
    int8_t type;
    int32_t eAclId;
    uint8_t  macDa[XP_MAC_ADDR_LEN];
    uint8_t  macSa[XP_MAC_ADDR_LEN];
    int32_t etherType;
    ipv4Addr_t dip;
    ipv4Addr_t sip;
    ipv6Addr_t dip6;
    ipv6Addr_t sip6;
    int32_t L4DestPort;
    int32_t L4SourcePort;
    int32_t icmpMessageType;
    int32_t protocol;
    int32_t eVlan;
    int32_t egressVif;
    int32_t eRif;
    int32_t vi;
    int32_t tcpFlags;
    int32_t dscp;
    int32_t exp;
    int32_t pcp;
    int32_t reasonCode;
    uint32_t priority;
    int32_t isDmacSet;
    int32_t isSmacSet;
    int32_t isDIPSet;
    int32_t isSIPSet;
    int32_t isDIP6Set;
    int32_t isSIP6Set;
} EaclL2KeyAttribute;

typedef struct
{
    int8_t type;
    int32_t eAclId;
    uint8_t  macDa[XP_MAC_ADDR_LEN];
    uint8_t  macSa[XP_MAC_ADDR_LEN];
    int32_t etherType;
    ipv4Addr_t dip;
    ipv4Addr_t sip;
    ipv6Addr_t dip6;
    ipv6Addr_t sip6;
    int32_t L4DestPort;
    int32_t L4SourcePort;
    int32_t icmpMessageType;
    int32_t protocol;
    int32_t eVlan;
    int32_t egressVif;
    int32_t eRif;
    int32_t vi;
    int32_t tcpFlags;
    int32_t dscp;
    int32_t exp;
    int32_t pcp;
    int32_t reasonCode;
    uint32_t priority;
    int32_t isDmacSet;
    int32_t isSmacSet;
    int32_t isDIPSet;
    int32_t isSIPSet;
    int32_t isDIP6Set;
    int32_t isSIP6Set;
} EaclL2MaskAttribute;

typedef struct
{
    uint32_t enPktCmdUpd;
    uint32_t enRsnCodeUpd;
    uint32_t pktCmd;
    uint32_t rsnCode;
} EaclData;

typedef struct
{
    int32_t iAclId;
    uint8_t  macDA[XP_MAC_ADDR_LEN];
    uint8_t  macSA[XP_MAC_ADDR_LEN];
    int32_t etherType;
    int32_t cTagVid;
    int32_t cTagDei;
    int32_t cTagPcp;
    int32_t sTagVid;
    int32_t sTagDei;
    int32_t sTagPcp;
    ipv4Addr_t SIP;
    ipv4Addr_t DIP;
    int32_t L4DestPort;
    int32_t L4SourcePort;
    int32_t icmpMessageType;
    int32_t protocol;
    int32_t ipv4DfSet;
    int32_t ipv4MfSet;
    int32_t BD;
    int32_t dscp;
    int32_t iVif;
    int32_t pktHasCtag;
    int32_t pktHasStag;
    uint32_t pktLen;
    uint32_t isDIPSet;
    uint32_t isSIPSet;
    uint32_t isDmacSet;
    uint32_t isSmacSet;
    int32_t vrfId;
    uint32_t priority;
    uint32_t tableId;
} IaclL2Ipv4KeyAttribute;

typedef struct
{
    uint32_t iAclId;
    uint8_t  macDA[XP_MAC_ADDR_LEN];
    uint8_t  macSA[XP_MAC_ADDR_LEN];
    uint32_t etherType;
    uint32_t cTagVid;
    uint32_t cTagDei;
    uint32_t cTagPcp;
    uint32_t sTagVid;
    uint32_t sTagDei;
    uint32_t sTagPcp;
    ipv4Addr_t DIP;
    ipv4Addr_t SIP;
    uint32_t L4DestPort;
    uint32_t L4SourcePort;
    uint32_t icmpMessageType;
    uint32_t protocol;
    uint32_t ipv4DfSet;
    uint32_t ipv4MfSet;
    uint32_t BD;
    uint32_t dscp;
    uint32_t iVif;
    uint32_t pktHasCtag;
    uint32_t pktHasStag;
    uint32_t pktLen;
    uint32_t isDIPSet;
    uint32_t isSIPSet;
    uint32_t isDmacSet;
    uint32_t isSmacSet;
    uint32_t vrfId;
} IaclL2Ipv4MaskAttribute;

typedef struct
{
    uint32_t isTerminal;
    uint32_t enPktCmdUpd;
    uint32_t enRedirectToEvif;
    uint32_t remarkDp;
    uint32_t enPolicer;
    uint32_t qosOrVlanRw;
    uint32_t enMirrorSsnUpd;
    uint32_t remarkTc;
    uint32_t remarkDscp;
    uint32_t remarkPcp;
    uint32_t pktCmdOrEcmpSize;
    uint32_t eVifId;
    uint32_t rsnCode;
    uint32_t policerId;
    uint32_t TC;
    uint32_t PCP;
    uint32_t DSCP;
    uint32_t mirrorSessionId;
    uint32_t DP;
    uint32_t type;
    uint32_t portNum;
    uint32_t vlan;
    uint32_t encapType;
} IaclData;

typedef struct
{
    int32_t type;
    int32_t iaclId;
    int32_t ipVersion;
    int32_t aclKeyType;
    int32_t priority;
    int32_t sourcePort;
    int32_t etherType;
    int32_t cTagVid;
    int32_t cTagDei;
    int32_t cTagPcp;
    int32_t fragment;
    int32_t nextHeader;
    int32_t hopLimit;
    int32_t l4DestPort;
    int32_t l4SourcePort;
    int32_t icmpCode;
    int32_t icmpType;
    int32_t tcpFlags;
    int32_t ipType;
    int32_t DSCP;
    int32_t ECN;
    int32_t vlanId;
    int32_t egressVif;
    int32_t egressBd;
    int32_t TC;
    int32_t DP;
    int32_t isUcastRouted;
    macAddr_t macDA;
    macAddr_t macSA;
    union
    {
        ipv4Addr_t v4SIP;
        ipv6Addr_t v6SIP;
    } sip;
    union
    {
        ipv4Addr_t v4DIP;
        ipv6Addr_t v6DIP;
    } dip;
    struct
    {
        uint32_t isSIPset  : 1;
        uint32_t isDIPset  : 1;
        uint32_t isSAset   : 1;
        uint32_t isDAset   : 1;
        uint32_t reserved  : 28;
    } flags;
} IpvxIaclKeyAttribute;

typedef struct
{
    uint32_t iaclId;
    uint32_t aclKeyType;
    uint32_t sourcePort;
    uint32_t etherType;
    uint32_t cTagVid;
    uint32_t cTagDei;
    uint32_t cTagPcp;
    uint32_t fragment;
    uint32_t nextHeader;
    uint32_t hopLimit;
    uint32_t l4DestPort;
    uint32_t l4SourcePort;
    uint32_t icmpCode;
    uint32_t icmpType;
    uint32_t tcpFlags;
    uint32_t ipType;
    uint32_t DSCP;
    uint32_t ECN;
    uint32_t vlanId;
    uint32_t egressVif;
    uint32_t egressBd;
    int32_t TC;
    int32_t DP;
    int32_t isUcastRouted;
    macAddr_t macDA;
    macAddr_t macSA;
    union
    {
        ipv4Addr_t v4SIP;
        ipv6Addr_t v6SIP;
    } sip;
    union
    {
        ipv4Addr_t v4DIP;
        ipv6Addr_t v6DIP;
    } dip;
} IpvxIaclMaskAttribute;

typedef struct
{
    struct
    {
        uint32_t enCounter                : 1;
        uint32_t swicthAclCpuReasonCodeEn : 1;
        uint32_t enPolicer                : 1;
        uint32_t enSampler                : 1;
        uint32_t enTcRemark               : 1;
        uint32_t enPcpRemark              : 1;
        uint32_t enDscpRemark             : 1;
        uint32_t enPktCmdTrap             : 1;
        uint32_t enPktCmdLog              : 1;
        uint32_t enPktCmdDrop             : 1;
        uint32_t enPbr                    : 1;
        uint32_t enPktCmdUpd              : 1;
        uint32_t enDpRemark               : 1;
        uint32_t reserved                 : 19;
    } enableData;

    uint32_t type;
    uint32_t ecmpSize;
    uint32_t futureUse;
    uint32_t reasonCode;
    uint32_t mirrorSessionId;
    uint32_t vlan;
    uint32_t portNum;
    uint32_t tc;
    uint32_t pcp;
    uint32_t dscp;
    uint32_t policerId;
    uint32_t nextHopId;
    uint32_t pktCmd;
    uint32_t dp;
} IpvxIaclData;


typedef struct
{
    uint32_t nhId;
    uint32_t ecmpEn;
    uint32_t portNum;
    uint32_t vlan;
} PbrData;

typedef struct
{
    int32_t inPort;
    int32_t ipPhyPort;
    uint8_t  ethDst[XP_MAC_ADDR_LEN];
    int8_t isDestMac;
    uint8_t ethSrc[XP_MAC_ADDR_LEN];
    int8_t isSrcMac;
    int32_t ethType;
    int32_t tagging;
    int16_t vlanVID;
    int32_t vlanPCP;
    int32_t mpls;
    int16_t tpSrc;
    int16_t tpDst;
    uint8_t ip4srcAddr[4];
    int8_t isIpv4SrcAddr;
    uint8_t ip4dstAddr[4];
    int8_t isIpv4dstAddr;
    int32_t ip4tos;
    int32_t ip4proto;
    int32_t ip4icmpType;
    int32_t ip4icmpCode;
    int32_t metaData;
} OpenFlowKeyMaskEntry;

typedef struct openFlowBucketList
{
    int16_t bucketActList[10];
    int8_t bucketLen;
} openFlowBucketList;

typedef struct
{
    int16_t ofIdx;
    int16_t tableId;
    int16_t entryType;
    int16_t ofType;
    int16_t Priority;
    int16_t isGrpEntry;
    int16_t dpId;
    int16_t groupId;
    int16_t groupType;
    int16_t instType;
    int32_t metaData;
    int32_t metaDataMask;
    OpenFlowKeyMaskEntry openFlowKey;
    OpenFlowKeyMaskEntry openFlowMask;
    int16_t actSetId;
    int16_t actList[20];
    int8_t actLen;
    openFlowBucketList bucketList[10];
} OpenFlowDataEntry;

typedef struct
{
    uint8_t ethAddr[XP_MAC_ADDR_LEN];
    uint32_t ethType;
    uint16_t vlanVID;
    uint8_t vlanPCP;
    uint32_t mplsLabel;
    uint32_t mplsTC;
    uint32_t mplsBOS;
    uint32_t arpOP;
    uint8_t ipv4Addr[4];
    uint8_t ipv6Addr[16]; //TODO: Not programed
    uint8_t ipv6FlowLabel;
    uint32_t ipDSCP;
    uint32_t ipECN;
    uint32_t icmpType;
    uint32_t icmpCode;
    uint16_t l4Port;
    uint32_t pbbISID;
    uint32_t pbbUCA;
    uint32_t port;
    uint32_t pktMaxLen;
    uint32_t setTTL;
    uint32_t mpls;
    uint32_t mplsTTL;
} OpenFlowActionDataMask;

typedef struct
{
    uint32_t actId;
    uint32_t actType;
    int32_t subActType;
    OpenFlowActionDataMask ofActionData;
    OpenFlowActionDataMask ofActionMask;
} OpenFlowActionEntry;

typedef struct
{
    uint32_t    nhId;
    uint8_t ipv4Addr[XP_IPV4_ADDR_LEN];
    xpVlan_t    l3IntfId;
    uint8_t     nextHopType;
} Ipv4SaiNhEntry;

typedef struct
{
    uint32_t    nhId;
    uint8_t ipv6Addr[XP_IPV6_ADDR_LEN];
    xpVlan_t    l3IntfId;
    uint8_t     nextHopType;
} Ipv6SaiNhEntry;

typedef struct
{
    uint8_t     ipv4Addr[XP_IPV4_ADDR_LEN];
    uint32_t    l3IntfId;
    uint8_t     macSA[XP_MAC_ADDR_LEN];
    uint8_t pktCmd;
} Ipv4SaiNeighborEntry;

typedef struct
{
    uint8_t     ipv6Addr[XP_IPV6_ADDR_LEN];
    uint32_t    l3IntfId;
    uint8_t     macSA[XP_MAC_ADDR_LEN];
    uint8_t pktCmd;
} Ipv6SaiNeighborEntry;

typedef struct
{
    uint32_t    nhId;
    uint8_t ipv4Addr[XP_IPV4_ADDR_LEN];
    uint32_t    vrfId;
    uint8_t pktCmd;
    uint8_t trapPriority;
    uint32_t    nhGroupId;
    uint8_t     netMask;
} Ipv4SaiRouteEntry;

typedef struct
{
    uint32_t    nhId;
    uint8_t ipv6Addr[XP_IPV6_ADDR_LEN];
    uint32_t    vrfId;
    uint8_t pktCmd;
    uint8_t trapPriority;
    uint32_t    nhGroupId;
    uint8_t     netMask;
} Ipv6SaiRouteEntry;

typedef struct
{
    uint32_t    nhGroupId;
    uint32_t    ecmpSize;
    uint8_t     groupAttrType;
    entryList   nhIdList;
} IpvxSaiNhGroupEntry;

typedef struct
{
    uint32_t    devPortId;                        // Device port number
    uint32_t    queueNum[XP_MAX_QUEUES_PER_PORT]; // Number of Queues per Port
    uint32_t    priority;                         // Queue priority
    uint32_t
    enableSP;                         // 0 - Enable DWRR Scheduler, 1 - Enable SP Scheduler
    uint32_t    weight;                           // DWRR queue weight
} qosSchedulerDataEntry;

typedef struct
{
    uint32_t    devPort;         // Device port number
    uint32_t
    portShaperEn;    // Enables/Disables port shaping for a particular port
    uint32_t    featureEn;       // Enables/Disables port shaping feature
    uint32_t    maxBurstSize;    // Shaper Max Burst Size in Bytes
    uint64_t    rateKbps;        // Update Rate
} qosShaperPortDataEntry;

typedef struct
{
    uint32_t    devPort;         // Device port number
    uint32_t    queueNum;        // Queue number
    uint32_t    qSlowShaperEn;   // Enables/Disables queue slow shaper
    uint32_t    featureEn;       // Enables/Disables queue shaping feature
    uint32_t    maxBurstSize;    // Shaper Max Burst Size in Bytes
    uint64_t    rateKbps;        // Update Rate
} qosShaperQueueDataEntry;

typedef struct
{
    int32_t portList[SYSTEM_MAX_PORT];
    uint32_t pcp;
    uint32_t dei;
    uint32_t dscp;
    uint32_t exp;
    uint32_t tc;
    uint32_t dp;
    uint32_t l2QosEn;
    uint32_t l3QosEn;
    uint32_t mplsQosEn;
    uint32_t defaultTC;
} qosIngressMapDataEntry;

typedef struct
{
    uint32_t egressVif;
    uint32_t egressPort;
    uint32_t tc;
    uint32_t dp;
    uint32_t l2QosEn;
    uint32_t l3QosEn;
    uint32_t pcp;
    uint32_t dei;
    uint32_t dscp;
    uint32_t type;
    uint32_t idx;
    uint32_t geneveTnlType;
    uint32_t mcastType;
} qosEgressMapDataEntry;

typedef struct
{
    uint32_t      devPort;            // Device port number
    uint32_t      queueNum;           // Queue number
    uint32_t      profileId;          // Profile Id
    uint32_t      minThreshold;       // WRED Average Queue Length Minimum Threshold
    uint32_t      maxThreshold;       // WRED Average Queue Length Maximum Threshold
    uint32_t      weight;             // WRED Average Queue Size Weight
    uint32_t      maxDropProb;        // WRED Max Drop Probability Percentage
    uint32_t      lengthMaxThreshold; // Tail Drop Queue Length Maximum
    xpWredMode_e  wredMode;           // WRED mode. WRED or Tail Drop
} qosAqmQProfileDataEntry;

typedef struct
{
    uint32_t    portList[SYSTEM_MAX_PORT];        // Port list
    int32_t     queueNum[XP_MAX_QUEUES_PER_PORT]; // Number of Queues per Port
    uint32_t
    enableSP;                         // 0 - Enable DWRR Scheduler, 1 - Enable SP Scheduler
    uint32_t    weight;                           // DWRR queue weight
    uint32_t    bandwidthRate;                    // Bandwidth shape rate
    uint64_t    burstSize;                        // Burst for Bandwidth shape rate
} saiSchedulerDataEntry;

typedef struct
{
    int32_t   portList[SYSTEM_MAX_PORT];
    uint32_t  color;
    uint32_t  cbs;
    uint32_t  cir;
    uint32_t  pbs;
    uint32_t  pir;
    uint32_t  yellowPktAction;
    uint32_t  redPktAction;
} policerDataEntry;

typedef struct
{
    uint8_t     sourcePort;
    uint32_t    reasonCodeKey;
    uint8_t     enPolicer;
    uint32_t    policerId;
    uint8_t     updatePktCmd;
    uint8_t     pktCmd;
    uint8_t     updateReasonCode;
    uint32_t    reasonCode;
} CoppEntry;

typedef struct
{
    uint32_t    srIdx;
    uint8_t     numSegment;
    uint32_t    nhId;
    uint8_t     dmac[XP_MAC_ADDR_LEN];
    uint8_t     smac;
    uint8_t     tagType;
    uint16_t    vlan;
    uint16_t    portId;
    uint16_t    ins3Id;
    uint8_t     ip0[XP_IPV6_ADDR_LEN];
    uint8_t     ip1[XP_IPV6_ADDR_LEN];
    uint8_t     ip2[XP_IPV6_ADDR_LEN];
} SrhEntry;

typedef struct xpConfigFile
{
    uint8_t macSAMSBs[5];
    char    *filename;

    entryList   vlanData;
    entryList   vlanScopeData;
    entryList   macData;
    entryList   macScopeData;
    entryList   ipv4HostData;
    entryList   ipv4HostScopeData;
    entryList   ipv6HostData;
    entryList   ipv6RouteData;
    entryList   ipvxNhData;
    entryList   ipvxNhScopeData;
    entryList   ipv4RouteData;
    entryList   srhData;
    entryList   mySidData;
    entryList   ipv4RouteScopeData;
    entryList   tunnelData;
    entryList   tunnelQinQData;
    entryList   tunnelPBBData;
    entryList   tunnelMplsData;
    entryList   pbrData;
    entryList   pbrKey;
    entryList   pbrMask;
    entryList   mplsLabelData;
    entryList   bdData;
    entryList   portConfigData;
    entryList   portLagData;
    entryList   controlMacData;
    entryList   ipv4BridgeMcData;
    entryList   ipv4RouteMcData;
    entryList   ipv6BridgeMcData;
    entryList   ipv6RouteMcData;
    entryList   ipv4PimBiDirData;
    entryList   l2OIFEntry;
    entryList   l3OIFEntry;
    entryList   mvifData;
    entryList   mirrorData;
    entryList   natData;
    entryList   iaclData;
    entryList   iaclKey;
    entryList   iaclMask;
    entryList   wcmKey;
    entryList   wcmMask;
    entryList   eaclData;
    entryList   eaclKey;
    entryList   eaclMask;
    entryList   vlanTenantIdData;
    entryList   openFlowData;
    entryList   openFlowActionData;
    entryList   pureOpenFlowData;
    entryList   pureOpenFlowActionData;
    entryList   vrfData;
    entryList   ipv4SaiNhData;
    entryList   ipv6SaiNhData;
    entryList   ipv4SaiNeighborData;
    entryList   ipv6SaiNeighborData;
    entryList   ipv4SaiNhGroupData;
    entryList   ipv6SaiNhGroupData;
    entryList   ipv4SaiRouteData;
    entryList   ipv6SaiRouteData;
    entryList   ipSaiNhGroupData;
    entryList   qosSchedulerData;
    entryList   qosShaperPortData;
    entryList   qosShaperQueueData;
    entryList   qosAqmQProfileData;
    entryList   qosEgressMapData;
    entryList   saiSchedulerData;
    entryList   qosIngressMapData;
    entryList   policerData;
    entryList   fdbControlMacData;
    entryList   ipv4HostControlData;
    entryList   ipv6HostControlData;
    entryList   coppData;
    entryList   controlPcapList;
    entryList   trustBrData;
    entryList   vifData;
    entryList   ipvxIaclData;
    entryList   ipvxIaclKey;
    entryList   ipvxIaclMask;
} xpConfigFile;

extern xpConfigFile configFile;

void initEntryList(entryList *list);
extern int numOfEntry(entryList *list);
extern void* getEntry(entryList *list, int index);
void freeEntryList(entryList *list);
void addEntry(entryList *list, void *entry, uint32_t size);

XP_STATUS setMirrorSourceType(MirrorSource *src, char *myType);
XP_STATUS setMirrorSourceId(MirrorSource *src, char *myValue);

extern XP_STATUS readConfigFile(char *);
extern void initConfigEntryList(void);
extern void deInitConfigEntryList(void);
void printConfigFile(void);
void processMacData(int, char *);
void processTunnelData(int, char *);
void processTunnelQinQData(int, char *);
void processTunnelPBBData(int, char *);
void processTunnelMplsData(int, char *);
void processMplsLabelData(int, char *);
void processIpv4HostData(int, char *);
void processIpv4RouteData(int, char *);
void processIpv6HostData(int, char *);
void processIpv6RouteData(int, char *);
void processControlMacData(int, char *);
void processPortConfigData(int, char *);
void processPortLagData(int, char *);
void processVlanData(int, char *);
void processIpxNhData(int, char *);
void processMacSAMSBs(int, char *);
void processIpv4BridgeMcData(int, char *);
void processIpv4RouteMcData(int, char *);
void processIpv6BridgeMcData(int, char *);
void processIpv6RouteMcData(int, char *);
void processIpv4PimBiDirData(int, char *);
int processL2MdtNodeData(int lineNum, char *line);
int processL3MdtNodeData(int lineNum, char *line);
void processMvifData(int lineNum, char *line);
void processMirrorData(int lineNum, char *line);
void processMacSAMSBs(int lineNum, char *line);
void processNatData(int, char *);
void processL2Ipv4IaclData(int lineNum, char *line);
void processPBRData(int lineNum, char *line);
void processL2Ipv4IaclKey(int lineNum, char *line);
void processL2Ipv4WcmKey(int lineNum, char *line);
void processL2EaclData(int lineNum, char *line);
void processOpenFlowKeyMaskData(int lineNum, char *line);
void processOpenFlowActionData(int lineNum, char *line);
void processPureOfKeyMaskData(int lineNum, char *line);
void processPureOfActionData(int lineNum, char *line);
void processVlanTenantIdData(int lineNum, char *line);
void processVrfData(int lineNum, char *line);
void processIpv6SaiNhData(int lineNum, char *line);
void processIpv4SaiNhData(int lineNum, char *line);
void processIpv4SaiNeighborData(int lineNum, char *line);
void processIpv6SaiNeighborData(int lineNum, char *line);
void processIpv4SaiRouteData(int lineNum, char *line);
void processIpv6SaiRouteData(int lineNum, char *line);
void processSaiNhGroupData(int lineNum, char *line);
void processQosSchedulerData(int lineNum, char *line);
void processQosShaperPortData(int lineNum, char *line);
void processQosShaperQueueData(int lineNum, char *line);
void processQosAqmQProfileData(int lineNum, char *line);
void processFdbControlMacData(int lineNum, char *line);
void processIpv4HostControlData(int lineNum, char *line);
void processIpv6HostControlData(int lineNum, char *line);
void processCoppData(int lineNum, char *line);
void processTrustBrData(int lineNum, char *line);
void processVifData(int lineNum, char *line);
void processSRHData(int lineNum, char *line);
void processLocalSidData(int lineNum, char *line);
void processIpvxIaclData(int lineNum, char *line);
void processIpvxIaclKey(int lineNum, char *line);

#endif // __XPCONFIGFILE_H__

