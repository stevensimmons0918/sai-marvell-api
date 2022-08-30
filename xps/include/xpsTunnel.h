// xpsTunnel.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsTunnel.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Tunnel Manager
 */

#ifndef _xpsTunnel_h_
#define _xpsTunnel_h_

#include "xpsInterface.h"
#include "xpsState.h"
#include "xpsVlan.h"
#include "xpsVxlan.h"
#include "xpsGeneve.h"
#include "xpsNvgre.h"
#include "xpsVpnGre.h"
#include "xpTypes.h"
#include "xpsL3.h"
#include "xpsUtil.h"
#include "xpsMirror.h"
//#include "xpsIpinIp.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum xpsAddrFamily_t
{
    XPS_IP_ADDR_FAMILY_IPV4,
    XPS_IP_ADDR_FAMILY_IPV6
} xpsAddrFamily_t;

typedef union xpsAddr_t
{
    uint8_t ipv4Addr[4];
    uint8_t ipv6Addr[16];
} xpsAddr_t;

typedef struct xpsIpAddr_t
{
    xpsAddrFamily_t addrFamily;
    xpsAddr_t addr;
} xpsIpAddr_t;

typedef union xpsIpTunnelConfig_t
{
    xpsVxlanTunnelConfig_t vxlanCfg;
    xpsNvgreTunnelConfig_t nvgreCfg;
    xpsIpGreTunnelConfig_t ipgreCfg;
    xpsVpnGreTunnelConfig_t vpnGreCfg;
    xpsIpinIpTunnelConfig_t ipInIpCfg;
    xpsGeneveTunnelConfig_t geneveCfg;
} xpsIpTunnelConfig_t;


typedef struct xpsIpTunnelData_t
{
    inetAddr_t lclEpIpAddr;
    inetAddr_t rmtEpIpAddr;
    xpIpTunnelType_t type;
    XpIpTunnelMode_t ecpTtlMode;
    uint8_t  ecpTtl;
    XpIpTunnelMode_t ecpDscpMode;
    uint8_t  ecpDscp;
    XpIpTunnelEcnMode_t ecpEcnMode;
    XpIpTunnelMode_t dcpTtlMode;
    XpIpTunnelMode_t dcpDscpMode;
    XpIpTunnelEcnMode_t dcpEcnMode;
    xpsGeneveFormatType_t optionFormat; // Only for Geneve
    uint32_t vpnLabel;
    xpsIpTunnelConfig_t     cfg;
} xpsIpTunnelData_t;

#define XPS_INVALID_INSPTR 0xFFFFFFFF
#define MAX_TTI_RULE_ID 32
#define MAX_DECAP_ENTRY 8

typedef struct xpsIpTnlTermEntryMap_t
{
    uint32_t ttiRuleId;
    xpsInterfaceId_t intfId;
} xpsIpTnlTermEntryMap_t;

typedef struct xpsIpTnlDecapRuleInfo_t
{
    uint32_t aclRuleId;
    uint32_t ruleCnt;
    xpsIpTnlTermEntryMap_t rule[MAX_TTI_RULE_ID];
} xpsIpTnlDecapRuleInfo_t;

/*Decap Map entries cannot be updated/deleted once associated with tunel.
  Hence it is safe to model it as array, as the order of entries will not
  change.*/
typedef struct xpsIpTnlTermEntry_t
{
    uint32_t termEntryId; // Re-use ID allocated by upper SAI layer
    uint32_t dcpEntryCnt;
    xpsIpTnlDecapRuleInfo_t entry[MAX_DECAP_ENTRY];
} xpsIpTnlTermEntry_t;

#define MAX_EPORT_SIZE 16
typedef struct xpsVxlanEPortInfo_t
{
    uint32_t ePort; // HW ePort
    uint32_t tnlStartId;
    xpsInterfaceId_t l3IntfId; //corresponding L3 XPS InterfaceId
    xpsInterfaceId_t intfId; //corresponding Phy XPS InterfaceId
} xpsVxlanEPortInfo_t;
/* Global IP tunnel state */
typedef struct xpsIpTnlGblDbEntry_t
{
    xpsIpTunnelData_t tnlData;
    bool isLearnEnable;
    //    uint32_t tnlTermId; /* Holds CPSS logical TTI index */
    xpVif_t vifId;
    uint32_t l3HdrInsertionId;
    uint32_t l2HdrInsertionId;
    xpsRBTree_t *termEntryList;
    //    uint16_t numOfOptIntfs;
    //    xpsInterfaceId_t optIntfList[0];
    uint32_t ecmpStartIdx;
    uint32_t primaryEport;
    uint32_t numOfEports;
    xpsVxlanEPortInfo_t ePorts[MAX_EPORT_SIZE];
} xpsIpTnlGblDbEntry_t;

/* Per device tunnel state. */
typedef struct xpsTnlDbEntry_t
{
    xpVif_t vifId;
    uint32_t tnlTableIndex;
} xpsTnlDbEntry_t;

#define XP_TUNNEL_SET_PKT_CMD 1
#define XP_TUNNEL_SET_BACL_ID 2
#define XP_TUNNEL_SET_RACL_ID 4
#define XP_TUNNEL_SET_PACL_ID 8

typedef enum
{
    XP_SRV6_END_FUNCTION = 1,
    XP_SRV6_END_T_FUNCTION,
    XP_SRV6_END_X_FUNCTION,
    XP_SRV6_END_DX6_FUNCTION,
    XP_SRV6_END_DX4_FUNCTION,
    XP_SRV6_END_DX2_FUNCTION,
    XP_SRV6_END_DT6_FUNCTION,
    XP_SRV6_END_B6_FUNCTION,
    XP_SRV6_END_B6_ENCAP_FUNCTION,
    XP_SRV6_END_TM_FUNCTION,
    XP_SRV6_END_T_TMAP_FUNCTION,
    XP_SRV6_T_INS_FUNCTION,
    XP_SRV6_T_INS_ENCAP_FUNCTION,
} xpSrEndFuncType_t;


typedef struct xpLocalSidData
{
    xpSrEndFuncType_t func;       ///< func
    uint32_t args;       ///< args
    uint32_t VRFId;      ///< VRFId
    uint32_t nhId;      ///< nhId
    uint32_t ecmpEnable;      ///< ecmpEnable
    uint32_t flag;      ///< Flag
} xpSidData_t;

typedef struct
{
    xpSidData_t localSidData;
    uint8_t     localSid[XP_IPV6_ADDR_LEN];
} xpSidEntry;

typedef xpSidEntry xpsSidEntry;

typedef struct xpsSrhData_t
{
    uint32_t    numOfSegments;
    uint32_t    ins3Id;
    ipv6Addr_t  segment0;
    ipv6Addr_t  segment1;
    ipv6Addr_t  segment2;
} xpsSrhData_t;

typedef struct xpsIpMcOIFData_t
{
    xpsIpTunnelData_t tunnelData;
    xpsInterfaceId_t l3IntfId;
    xpsInterfaceId_t egressIntfId;
} xpsIpMcOIFData_t;

typedef struct xpsPbbTunnelData_t
{
    macAddr_t sMacAddr;
    macAddr_t dMacAddr;
    uint32_t bTag;
} xpsPbbTunnelData_t;

/**
 * \brief This method initializes Tunnel Mgr with system specific
 *        references to various primitive managers and their initialization per scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTunnelInitScope(xpsScope_t scopeId);

/**
 * \brief This method cleans up from the Tunnel Mgr system specific
 *        references to various primitive managers per scope.
 *
 * \param [in] scopeId
 * \return XP_STATUS
 */
XP_STATUS xpsTunnelDeInitScope(xpsScope_t scopeId);

/**
 * \brief This method configures and add device specific primitives
 *        required for the Tunnel FL manager.
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTunnelAddDevice(xpsDevice_t devId, xpsInitType_t initType);
/**
 * \public
 * \brief Enable Parsing of Tunnel Passenger Packet
 *
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */

XP_STATUS xpsTunnelEnableTermination(xpsDevice_t devId);

/**
 * \public
 * \brief Enable Parsing of Tunnel Passenger Packet
 *
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */

XP_STATUS xpsTunnelDisableTermination(xpsDevice_t devId);


/**
 * \brief This method removes device specific primitives
 *        required for the Tunnel FL manager.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTunnelRemoveDevice(xpsDevice_t devId);

/**
 * \public
 * \brief Registers a function handler and user data to handle remote vtep learning packets
 *
 *
 * \param [in] func where func is of type: int (*remoteVtepLearnPktHandler)(xpsDevice_t, struct xphRxHdr*, void*, uint16_t)
 * \param [in] *user_data
 *
 * \return XP_STATUS
 */
typedef XP_STATUS(*remoteVtepLearnPktHandler)(xpDevice_t, xphRxHdr*, void*,
                                              uint16_t, void*);
XP_STATUS xpsRegisterRemoteVtepLearnHandler(remoteVtepLearnPktHandler func,
                                            void *user_data);

/**
 * \public
 * \brief unregisters the function handler used to handle remote vtep learning packets
 *
 *
 * \return XP_STATUS
 */
XP_STATUS xpsUnregisterRemoteVtepLearnHandler();

/**
 * \brief This method initializes Tunnel Mgr with system specific
 *        references to various primitive managers and their initialization.
 *
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTunnelInit(void);

/**
 * \brief This method cleans up from the Tunnel Mgr system specific
 *        references to various primitive managers.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTunnelDeInit(void);

/**
 * \brief This method displays local vtep table
 *
 * \param [in] devId
 * \param [out] numOfValidEntries
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTunnelLocalVtepDisplayTable(xpDevice_t devId,
                                         uint32_t *numOfValidEntries);

/**
 * \brief This method displays tunnel Id table
 *
 * \param [in] devId
 * \param [out] numOfValidEntries
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTunnelTunnelIdDisplayTable(xpDevice_t devId,
                                        uint32_t *numOfValidEntries);

/**
 * \brief This method sets a field in the  tunnelIvif table
 *
 * \param [in] devId
 * \param [in] tunnelTable index
 * \param [in] fieldName
 * \param [in] fieldData
 *
 * \return XP_STATUS
 **/
XP_STATUS xpsSetIpTnlTerminationFieldData(xpsDevice_t devId, uint32_t index,
                                          uint32_t fieldName, uint32_t *fieldData);

/**
 *\brief This method retrieves the tunnel DB entry from the tunnel Interface ID
 *
 *\param [in] devId
 *\param [in] xpsInterfaceId
 *\param [out] xpsTunnelDbEntry
 *
 *\return XP_STATUS
 **/

XP_STATUS xpsIpTunnelDbGetEntry(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                xpsTnlDbEntry_t **lookupDbEntry);

/**
 *
 * \brief IpTunnel Set Config from Mirror Session data
 *
 * \param [in] devId
 * \param [in] tnlIntfId,
 * \param [in] mirrorSessionData,
 * \returns XP_STATUS
 *
 */
XP_STATUS xpsIpTunnelSetConfigByMirrorSession(xpsDevice_t devId,
                                              xpsInterfaceId_t tnlIntfId, xpsMirrorData_t * mirrorSessionData);

XP_STATUS xpsIpTunnelCreate(xpsScope_t scopeId, xpsInterfaceType_e type,
                            xpsIpTunnelData_t *data, xpsInterfaceId_t *tnlIntfId);
XP_STATUS xpsIpTunnelDelete(xpsScope_t scopeId, xpsInterfaceId_t tnlIntfId);
XP_STATUS xpsIpTunnelAdd(xpsDevice_t devId, xpsInterfaceId_t intfId);
XP_STATUS xpsIpTunnelAddOrigination(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                    xpsInterfaceId_t baseIntfId, xpsIpTunnelData_t *data);
XP_STATUS xpsErspanTunnelAdd(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId);
XP_STATUS xpsIpTunnelGetConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                               xpIpTunnelType_t type, xpsIpTunnelConfig_t *cfg);
XP_STATUS xpsIpTunnelDbAddEntry(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                uint32_t tnlTableIndex, int32_t nhId);
XP_STATUS xpsIpTunnelDbRemoveEntry(xpsDevice_t devId, xpsInterfaceId_t intfId);
XP_STATUS xpsIpTnlGblDbAddOptionalInterface(xpsDevice_t devId,
                                            xpsInterfaceId_t baseIntfId, xpsInterfaceId_t optIntfId);
XP_STATUS xpsIpTunnelAddTermination(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                    uint32_t tnlTermId);
XP_STATUS xpsIpTunnelUpdateTerminationAction(xpsDevice_t devId,
                                             xpsInterfaceId_t intfId,
                                             XpIpTunnelMode_t *decapTtlMode,
                                             XpIpTunnelMode_t *decapDscpMode);
XP_STATUS xpsIpTunnelDeleteTermination(xpsDevice_t devId,
                                       xpsInterfaceId_t intfId,
                                       uint32_t tnlTermId);

/**
 * \brief This method attaches an Ip Gre tunnel interface to an l3 interface
 *
 * \param [in] devId Device Id of device
 * \param [in] tnlIntfId Interface id of the Ip Gre tunnel
 * \param [in] l3IntfId Interface id of the l3 interface
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpGreTunnelAttachL3Intf(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId, xpsInterfaceId_t l3IntfId);

/**
 * \brief This method detaches an Ip Gre tunnel interface from the corresponding l3 interface
 *
 * \param [in] devId Device Id of device
 * \param [in] tnlIntfId Interface id of the Ip Gre tunnel
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpGreTunnelDetachL3Intf(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId);

/**
 * \brief This method attaches an Vpn Gre strcit mode tunnel interface to an l3 interface
 *
 * \param [in] devId Device Id of device
 * \param [in] tnlIntfId Interface id of the Ip Vpn Gre strict mode tunnel
 * \param [in] l3IntfId Interface id of the l3 interface
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVpnGreStrictModeTunnelAttachL3Intf(xpsDevice_t devId,
                                                xpsInterfaceId_t tnlIntfId, xpsInterfaceId_t l3IntfId);

/**
 * \brief This method detaches an Vpn Gre tunnel interface from the corresponding l3 interface
 *
 * \param [in] devId Device Id of device
 * \param [in] tnlIntfId Interface id of the Vpn Gre tunnel
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVpnGreStrictModeTunnelDetachL3Intf(xpsDevice_t devId,
                                                xpsInterfaceId_t tnlIntfId);

/**
 * \brief This method attaches an IpinIp tunnel interface to an l3 interface
 *
 * \param [in] devId Device Id of device
 * \param [in] tnlIntfId Interface id of the IpinIp tunnel
 * \param [in] l3IntfId Interface id of the l3 interface
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpinIpTunnelAttachL3Intf(xpsDevice_t devId,
                                      xpsInterfaceId_t tnlIntfId, xpsInterfaceId_t l3IntfId);

/**
 * \brief This method detaches an IpinIp tunnel interface from the corresponding l3 interface
 *
 * \param [in] devId Device Id of device
 * \param [in] tnlIntfId Interface id of the IpinIp tunnel
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpinIpTunnelDetachL3Intf(xpsDevice_t devId,
                                      xpsInterfaceId_t tnlIntfId);

/**
 *
 * \brief IpTunnel Set Config
 *
 * \param [in] devId
 * \param [in] tnlIntfId,
 * \param [in] type,
 * \param [in] cfg
 * \returns XP_STATUS
 *
 */
XP_STATUS xpsIpTunnelSetConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                               xpIpTunnelType_t type, xpsIpTunnelConfig_t * cfg);

/**
 *
 * \brief IpTunnel Update Next hop
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \returns XP_STATUS
 *
 */
XP_STATUS xpsIpTunnelUpdateNexthop(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId);

/**
 *
 * \brief IpTunnel Get Remote Ip
 *
 * \param [in] devId
 * \param [in] type,
 * \param [in] tnlIntfId,
 * \param [in] *rmtEpIpAddr
 * \returns XP_STATUS
 *
 */
XP_STATUS xpsIpTunnelGetRemoteIp(xpsDevice_t devId, xpIpTunnelType_t type,
                                 xpsInterfaceId_t tnlIntfId, ipv4Addr_t *rmtEpIpAddr);

/**
 *
 * \brief IpTunnel Remove Local Vtep
 *
 * \param [in] devId
 * \param [in] localIp,
 * \param [in] tnlType
 * \returns XP_STATUS
 *
 */
XP_STATUS xpsIpTunnelRemoveLocalVtep(xpsDevice_t devId, ipv4Addr_t localIp,
                                     xpIpTunnelType_t tnlType);

/**
 *
 * \brief IpTunnel Add Local Vtep
 *
 * \param [in] devId
 * \param [in] localIp,
 * \param [in] tnlType
 * \returns XP_STATUS
 *
 */
XP_STATUS xpsIpTunnelAddLocalVtep(xpsDevice_t devId, ipv4Addr_t localIp,
                                  xpIpTunnelType_t tnlType);

/**
 *
 * \brief IpTnl SetMc OIF
 *
 * \param [in] devId
 * \param [in] tunIntfId
 * \param [in] *oifdata
 * \returns XP_STATUS
 *
 */
XP_STATUS xpsIpTnlSetMcOIF(xpsDevice_t devId, xpsInterfaceId_t tunIntfId,
                           xpsIpMcOIFData_t *oifData);


/**
 *
 * \brief IpTnl Get L2 insertionId from tunnel Next hop Data
 *
 * \param [in] devId
 * \param [in] viffId
 * \param [in] nhId
 * \param [out] l2HdrInsrtId
 * \returns XP_STATUS
 *
 */
XP_STATUS xpsIpTnlGetL2InsId(xpsDevice_t devId, xpVif_t vifId, uint32_t nhId,
                             uint32_t *l2HdrInsrtId);

/**
 *
 * \brief IpTnl Set Next hop Data
 *
 * \param [in] devId
 * \param [in] viffId
 * \param [in] nhId
 * \returns XP_STATUS
 *
 */
XP_STATUS xpsIpTnlSetNexthopData(xpsDevice_t devId, xpVif_t vifId,
                                 uint32_t nhId);

/**
 *
 * \brief Get Tunnel Table Index
 *
 * \param [in] xpsDevice_t devId
 * \param [in] xpsInterfaceId_t intfId
 * \param [out] uint32_t *tnlTableIndex
 * \returns XP_STATUS
 *
 */
//XP_STATUS xpsIpTunnelGetTnlTableIndex(xpsDevice_t devId, xpsInterfaceId_t intfId, uint32_t *tnlTableIndex);
XP_STATUS xpsIpTunnelDbGetTunnelTermIndex(xpsDevice_t devId,
                                          xpsInterfaceId_t intfId, uint32_t *index);

XP_STATUS xpsIpTunnelClearConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                 xpIpTunnelType_t type);

XP_STATUS xpsIpTunnelGblDbGetData(xpsScope_t scopeId, xpsInterfaceId_t intfId,
                                  xpsIpTunnelData_t *data);


XP_STATUS xpsPbbTunnelCreate(xpsScope_t scopeId, xpsInterfaceType_e type,
                             xpsPbbTunnelData_t *data, xpsInterfaceId_t *tnlIntfId);
XP_STATUS xpsPbbTunnelAddOrigination(xpsDevice_t devId,
                                     xpsInterfaceId_t pbbTnlId, uint32_t insertionId, xpsPbbTunnelData_t *tnlData);
XP_STATUS xpsPbbAddTunnelTerminationEntry(xpsDevice_t devId,
                                          xpsInterfaceId_t intfId, xpsPbbTunnelData_t *tnlData);
XP_STATUS xpsPbbTunnelAddLocalEntry(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                    xpsPbbTunnelData_t *tnlData);
XP_STATUS xpsPbbTunnelDelete(xpsScope_t scopeId, xpsInterfaceId_t tnlIntfId);
XP_STATUS xpsPbbTunnelAdd(xpsDevice_t devId, xpsInterfaceId_t intfId);
XP_STATUS xpPbbTunnelRemove(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId);

XP_STATUS xpsIpTunnelUpdateTunnelStart(xpsDevice_t devId,
                                       xpsInterfaceId_t intfId,
                                       XpIpTunnelMode_t *encapTtlMode,
                                       uint32_t encapTtl,
                                       XpIpTunnelMode_t *encapDscpMode,
                                       uint32_t encapDscp,
                                       xpsVxlanUdpSrcPortMode_t *udpSrcPortMode,
                                       uint16_t udpSrcPort);

XP_STATUS xpsIpTunnelGblGetDbEntry(xpsScope_t scopeId, xpsInterfaceId_t intfId,
                                   xpsIpTnlGblDbEntry_t **ipTnlGblEntry);
XP_STATUS xpsIpTunnelFindTermEntry(xpsScope_t scopeId,
                                   xpsInterfaceId_t tnlIntfId,
                                   uint32_t termId,
                                   xpsIpTnlTermEntry_t **ppInfo);

XP_STATUS xpsIpTunnelAddToTermList(xpsScope_t scopeId,
                                   xpsInterfaceId_t tnlIntfId,
                                   uint32_t termEntryId, xpsIpTnlTermEntry_t **termInfo);

XP_STATUS xpsIpTunnelDelFromTermList(xpsScope_t scopeId,
                                     xpsInterfaceId_t tnlIntfId, uint32_t termEntryId);

XP_STATUS xpsIpTunnelGetNextTermEntry(xpsScope_t scopeId,
                                      xpsInterfaceId_t tnlIntfId,
                                      xpsIpTnlTermEntry_t **nextInfo);

XP_STATUS xpsSetTunnelEcmpConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId);
XP_STATUS xpsClearTunnelEcmpConfig(xpsDevice_t devId, uint32_t startIdx,
                                   uint32_t size);

XP_STATUS xpsIpTunnelReSize(xpsScope_t scopeId, xpsInterfaceId_t tnlIntfId,
                            xpsIpTnlGblDbEntry_t **vxlanCtxNewPtr,
                            xpsIpTnlGblDbEntry_t *vxlanCtx, bool isGrow);
XP_STATUS xpsAllocateTunnelStartIndex(xpsDevice_t xpsDevId,
                                      xpIpPrefixType_t type, uint32_t* index);
XP_STATUS xpsReleaseTunnelStartIndex(xpsDevice_t xpsDevId,
                                     uint32_t index);

#ifdef __cplusplus
}
#endif

#endif  //_xpsTunnel_h_
