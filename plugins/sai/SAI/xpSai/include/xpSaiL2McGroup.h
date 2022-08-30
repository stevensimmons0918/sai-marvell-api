// xpSaiL2McGroup.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiL2McGroup_h_
#define _xpSaiL2McGroup_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Defines default number of L2 Multicast Group Members */
#define XP_SAI_DEFAULT_MAX_L2MC_GROUP_MEMBERS 16

/** Defines default number of L2 Multicast Group entries on HW */
#define XP_SAI_DEFAULT_MAX_L2MC_GROUP_ENTRIES 16

/**
 * @brief SAI L2 Multicast Group Member Attributes struct
 */
typedef struct _xpSaiL2McGroupMemberAttributesT
{
    sai_object_id_t l2mcGroupId;
    sai_object_id_t brPortId;
    sai_ip_address_t tunnelEndpointIP;

} xpSaiL2McGroupMemberAttributesT;

/**
 * @brief SAI L2 Multicast Group Member Data Base Entry struct
 */
typedef struct _xpSaiL2McGroupMemberContextDbEntry
{
    /** L2 Multicast Group Member Id OID as a key */
    sai_object_id_t l2mcGroupMemberIdOid;

    /** L2 Multicast Group Id OID */
    sai_object_id_t l2mcGroupIdOid;

    /** Bridge Port Id OID */
    sai_object_id_t brPortIdOid;

    /** Tunnel Endpoint IP */
    sai_ip_address_t tunnelEndpointIP;

} xpSaiL2McGroupMemberContextDbEntry;

typedef struct _xpSaiL2McGroupHwEntry
{
    /** L2 Multicast Group XPS Id */
    uint32_t l2mcGroupId;

    /** L2 Multicast Group Bridge or VlanId */
    uint16_t bvId;

    /** L2 Multicast Group hardware status */
    int32_t active;

} xpSaiL2McGroupHwEntry;

/**
 * @brief SAI L2 Multicast Group Data Base Entry struct
 */
typedef struct _xpSaiL2McGroupContextDbEntry
{
    /** L2 Multicast Group Id OID as a key */
    sai_object_id_t l2mcGroupIdOid;

    /* can be VIDX or E_VIDX. For IGMP l2MC we use VIDX */
    uint32_t xpsVidx;

    /** L2 Multicast Group hardware entry */
    uint32_t numHwEntries;
    xpSaiL2McGroupHwEntry groupHwEntry[XP_SAI_DEFAULT_MAX_L2MC_GROUP_ENTRIES];

    /** Multicast Group members info */
    uint32_t numItems;
    sai_object_id_t l2mcGroupMemberIdOid[XP_SAI_DEFAULT_MAX_L2MC_GROUP_MEMBERS];

} xpSaiL2McGroupContextDbEntry;

/**
 * \brief Gets default L2 Multicast group id created on hardware.
 * \return uint32_t
 */
uint32_t xpSaiGetDefaultL2McGroupId(void);

/**
 * \brief Creates default L2 Multicast Group to be used when action is not
 *        FORWARD but XPS API requires group to passed on hardware.
 * \param [in] xpsDevId    device identifier
 * \return SAI STATUS
 */
sai_status_t xpSaiL2McCreateDefaultGroup(xpsDevice_t xpsDevId);

/**
 * \brief Activate L2 Multicast Group to be called before group is going to be
 *        used by hardware. Uses incremental mechanism, in case one of the
 *        xpSaiAdapter module starts using group, it will be created on hardware.
 * \param [in] xpsDevId    device identifier
 * \param [in] vlanIdOid   vlan object ID
 * \param [out] groupIdOid group object ID is optional
 * \return SAI STATUS
 */
sai_status_t xpSaiActivateVlanL2McGroup(xpsDevice_t xpsDevId,
                                        sai_object_id_t vlanIdOid, sai_object_id_t groupIdOid, uint32_t *groupId);

/**
 * \brief Activate L2 Multicast Group to be called before group is going to be
 *        used by hardware. Uses incremental mechanism, in case one of the
 *        xpSaiAdapter module starts using group, it will be created on hardware.
 * \param [in] xpsDevId    device identifier
 * \param [in] bridgeOid   bridge object ID
 * \param [out] groupIdOid group object ID is optional
 * \return SAI STATUS
 */
sai_status_t xpSaiActivateBridgeL2McGroup(xpsDevice_t xpsDevId,
                                          sai_object_id_t bridgeOid, sai_object_id_t groupIdOid, uint32_t *groupId);

/**
 * \brief Deactivate L2 Multicast Group to be called when group is not more used.
 *        Uses decremental mechanism, in case all xpSaiAdapter modules stop using
 *        the group, it will be removed from hardware.
 * \param [in] xpsDevId    device identifier
 * \param [in] bvIdOid     vlan or bridge object ID
 * \param [in] groupIdOid  group object ID
 * \return SAI STATUS
 */
sai_status_t xpSaiDeActivateVlanL2McGroup(xpsDevice_t xpsDevId,
                                          sai_object_id_t bvIdOid, sai_object_id_t groupIdOid);

/**
 * \brief Get L2 Multicast Group ID from group object ID
 * \param [in] xpsDevId    device identifier
 * \param [in] groupIdOid  group object ID
 * \param [in] vlanId      XPS vlan id
 * \param [out] groupId    XPS group id
 * \return sai_status_t
 */
sai_status_t xpSaiGetL2McGroupId(xpsDevice_t xpsDevId,
                                 sai_object_id_t groupIdOid, uint16_t vlanId, uint32_t *groupId);

/**
 * \brief Generate L2 Multicast Group object ID from XPS group ID
 * \param [in] xpsDevId    device identifier
 * \param [in] groupId     XPS group id
 * \param [out] groupIdOid group object ID
 * \return sai_status_t
 */
sai_status_t xpSaiGetL2McGroupIdOid(xpsDevice_t xpsDevId, uint32_t groupId,
                                    uint16_t vlanId, sai_object_id_t *groupIdOid);

/**
 * \brief Create L2 Multicast Group on hardware
 * \param [in] xpsDevId    device identifier
 * \param [in] vlanId      XPS vlan id
 * \param [in] xpsVidx      XPS VIDX id
 * \param [out] ifListId   XPS group object ID
 * \return sai_status_t
 */
sai_status_t xpSaiCreateL2McGroupOnHardware(xpsDevice_t xpsDevId,
                                            xpVlan_t vlanId,
                                            uint32_t xpsVidx,
                                            uint32_t *ifListId);

/**
 * \brief Remove L2 Multicast Group from hardware
 * \param [in] xpsDevId    device identifier
 * \param [out] ifListId   XPS group object ID
 * \return sai_status_t
 */
sai_status_t xpSaiRemoveL2McGroupFromHardware(xpsDevice_t xpsDevId,
                                              uint32_t ifListId);
/**
 * \brief Add tunnel interface ID to L2 Multicast Group
 * \param l2mcGroupId  group object ID
 * \param tnlIntfId    corresponded service instance ID
 * \return sai_status_t
 */
sai_status_t xpSaiL2McGroupAddVxTunnel(sai_object_id_t l2mcGroupId,
                                       xpsInterfaceId_t tnlIntfId);

/**
 * \brief Remove tunnel interface ID from L2 Multicast Group
 * \param l2mcGroupId  group object ID
 * \param tnlIntfId    corresponded service instance ID
 * \return sai_status_t
 */
sai_status_t xpSaiL2McGroupRemoveVxTunnel(sai_object_id_t l2mcGroupId,
                                          xpsInterfaceId_t tnlIntfId);

/**
 * \brief Standard xpSai module initialization interface
 */
sai_status_t xpSaiL2McGroupApiInit(uint64_t flag,
                                   const sai_service_method_table_t* adapHostServiceMethodTable);
sai_status_t xpSaiL2McGroupApiDeinit();
sai_status_t xpSaiL2McGroupInit(xpsDevice_t xpSaiDevId);
sai_status_t xpSaiL2McGroupDeInit(xpsDevice_t xpSaiDevId);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiL2McGroup_h_
