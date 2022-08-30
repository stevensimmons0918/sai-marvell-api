// xpSaiTunnel.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiTunnel_h_
#define _xpSaiTunnel_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/**
 * \brief Standard xpSai module initialization interface
 */
XP_STATUS xpSaiTunnelApiInit(uint64_t flag,
                             const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiTunnelApiDeinit();
XP_STATUS xpSaiTunnelInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiTunnelDeInit(xpsDevice_t xpSaiDevId);

/**
 * \brief API to notify tunnel module that bridgePortId based on tunnelId is created
 *        and store bridgePortId associated with that tunnel in the Tunnel DB.
 * \param bridgeId  bridge ID
 * \param bridgeId  bridgePort ID
 * \param tunnelId  tunnel ID
 * \return sai_status_t
 */
sai_status_t xpSaiTunnelOnBridgePortAdded(sai_object_id_t bridgeId,
                                          sai_object_id_t bridgePortId, sai_object_id_t tunnelId);

/**
 * \brief API to notify tunnel module that bridgePortId based on tunnelId is removed
 *        and remove bridgePortId associated with that tunnel from the Tunnel DB.
 * \param bridgeId  bridge ID
 * \param bridgeId  bridgePort ID
 * \param tunnelId  tunnel ID
 * \return sai_status_t
 */
sai_status_t xpSaiTunnelOnBridgePortRemoved(sai_object_id_t bridgeId,
                                            sai_object_id_t bridgePortId, sai_object_id_t tunnelId);

/**
 * \brief Get HW tunnel interface list by tunnel ID
 * \param xpsDevId  deviceID
 * \param tunnelId  tunnel ID
 * \param bridgeId  bridge ID/VlanId
 * \param servInstanceId service instanceId
 * \return sai_status_t
 */
sai_status_t xpSaiTunnelGetEgressEncapInfoFromTunnelIdandBridgeId(
    xpsDevice_t xpsDevId, sai_object_id_t tunnelId, sai_object_id_t bridgeId,
    uint32_t* servInstanceId);

/**
 * \brief Get HW tunnel interface list by tunnel ID
 * \param xpsDevId  deviceID
 * \param tunnelId  tunnel ID
 * \param termEntryCount number of termination entry associated with tunnelId
 * \param tnlIntfIdList tunnelInterfaceId list
 * \return sai_status_t
 */
sai_status_t xpSaiTunnelGetTunnelInterfaceListForTunnelId(xpsDevice_t xpsDevId,
                                                          sai_object_id_t tunnelId, uint32_t termEntryCount,
                                                          xpsInterfaceId_t* tnlIntfIdList);

/**
 * \brief Get HW tunnel interface by tunnel ID and RemoteVtepIp
 * \param xpsDevId  deviceID
 * \param tunnelId  tunnel ID
 * \param remoteVtepIp remote endpoint Ip
 * \param intfId    pointer to HW tunnel interface
 * \return sai_status_t
 */
sai_status_t xpSaiTunnelGetHwTunnelIntfIdByTunnIdAndRemoteVtepIp(
    xpsDevice_t xpsDevId, sai_object_id_t tunnelId, sai_ip_address_t remoteVtepIp,
    xpsInterfaceId_t *tnlIntfId);

/**
 * \brief API to notify tunnel module that the bridgePort based on tunnelId is added
 *        as memeber to multicast group and store multicast Endpoint IP in the Tunnel DB.
 * \param tunnelId  tunnel ID
 * \param brPortId  bridgePort ID
 * \param mcGroupId L2 Multicast Group ID
 * \param epIpAddr  Endpoint IP
 * \return sai_status_t
 */
sai_status_t xpSaiTunnelIntfOnMcMemberAdded(sai_object_id_t tunnelId,
                                            sai_object_id_t brPortId, sai_object_id_t mcGroupId, sai_ip_address_t epIpAddr);

/**
 * \brief API to notify tunnel module that the bridgePort based on tunnelId is removed
 *        as memeber from multicast group and remove multicast Endpoint IP from the Tunnel DB.
 * \param tunnelId  tunnel ID
 * \param brPortId  bridgePort ID
 * \param mcGroupId L2 Multicast Group ID
 * \param epIpAddr  Endpoint IP
 * \return sai_status_t
 */
sai_status_t xpSaiTunnelIntfOnMcMemberRemoved(sai_object_id_t tunnelId,
                                              sai_object_id_t brPortId, sai_object_id_t mcGroupId, sai_ip_address_t epIpAddr);

/**
 * \brief API to notify tunnel module that next hop has been modified.
 * \param nextHopId   next hop ID
 * \return sai_status_t
 */
sai_status_t xpSaiTunnelOnNextHopChanged(uint32_t nextHopId);

/**
 * \brief Set the SAI FDB Learning mode to the PktCmd of Tunnel BridgePort
 * \param [in]  xpsDevId   Device ID
 * \param [in]  saiTnlId  Tunnel ID
 * \param [in]  tnlTermEntryId  TunnelTermEntry ID
 * \param [in]  learnMode  learn mode value in terms of SAI attribute
 * \param [in]  is_trm_entry_nw Whether its a new TUnnel Term entry or not
 * \return sai_status_t
 */
sai_status_t xpSaiTunnelBridgePortSetFdbLearningMode(xpsDevice_t xpsDevId,
                                                     sai_object_id_t saiTnlId,
                                                     xpPktCmd_e learnModer);

/**
 * \brief To get the XPS Interface from TunnelId and BridgeId
 * \param [in] ingressVif     IngressVif
 * \param [out] saiTnlId  Tunnel Id
 * \return sai_status_t
 */
sai_status_t xpSaiTunnelGetTunnelId(uint32_t ingressVif,
                                    sai_object_id_t *saiTnlId);

sai_status_t xpSaiTunnelGetTunnelIntf(xpsDevice_t xpsDevId,
                                      sai_object_id_t tunnelId,
                                      uint32_t *tunIntf);
#define XP_SAI_TUNNEL_THRESHOLD              (8)
#define XP_SAI_TUNNEL_ENCAP_MAP_MAX_NUM      (4)             ///< Max possible number of encap mappers based on map types attached to tunnel
#define XP_SAI_TUNNEL_DECAP_MAP_MAX_NUM      (4)             ///< Max possible number of decap mappers based on map types attached to tunnel

typedef struct _xpSaiTunnelDbEntry
{
    /* Key */
    uint32_t
    tunnelId;                                    ///< Tunnel identifier

    /* Context */
    int32_t
    tunnelType;                                  ///< Tunnel type
    sai_object_id_t
    uLayIntfId;                                  ///< Underlay router interface
    sai_object_id_t
    oLayIntfId;                                  ///< Overlay router interface
    sai_ip_address_t
    tnlSrcIp;                                    ///< Tunnel src IP
    sai_ip_address_t
    tnlDstIp;                                    ///< Tunnel dst IP
    int32_t
    ecpTtlMode;                                  ///< Tunnel TTL mode (pipe or uniform model)
    uint8_t
    ecpTtl;                                      ///< Tunnel TTL value
    int32_t
    ecpDscpMode;                                 ///< Tunnel DSCP mode (pipe or uniform model)
    uint8_t
    ecpDscp;                                     ///< Tunnel DSCP value (6 bits)
    bool
    tnlGreKeyValid;                              ///< Is Tunnel GRE key valid
    uint32_t
    tnlGreKey;                                   ///< Tunnel GRE key
    int32_t
    ecpEcnMode;                                  ///< Tunnel encap ECN mode
    uint32_t peerMode;
    uint32_t
    ecpMapListCnt;                               ///< Tunnel encap Mappers object list count
    sai_object_id_t
    ecpMapList[XP_SAI_TUNNEL_ENCAP_MAP_MAX_NUM]; ///< Tunnel encap Mappers object list
    int32_t
    dcpEcnMode;                                  ///< Tunnel decap ECN mode
    uint32_t
    dcpMapListCnt;                               ///< Tunnel decap Mappers object list count
    sai_object_id_t
    dcpMapList[XP_SAI_TUNNEL_DECAP_MAP_MAX_NUM]; ///< Tunnel decap Mappers object list
    int32_t
    dcpTtlMode;                                  ///< Tunnel TTL mode (pipe or uniform model)
    int32_t
    dcpDscpMode;                                 ///< Tunnel DSCP mode (pipe or uniform model)
    xpPktCmd_e
    inrSaMissCmd;                                ///< To provide inner src mac control on Tunnel
    uint32_t udpSrcPortMode;
    uint32_t udpSrcPort;
    uint32_t tnlIntfId;
    uint32_t
    entryCnt;                                    ///< Tunnel term entries count
    sai_object_id_t
    entryId[XP_SAI_TUNNEL_THRESHOLD];   ///< Tunnel term entries associated with tunnel
} xpSaiTunnelInfoT;


XP_STATUS xpSaiTunnelGetCtxDb(xpsDevice_t xpsDevId,
                              sai_object_id_t tunnelId, xpSaiTunnelInfoT **pTunnelEntry);



#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*_xpSaiTunnel_h_*/
