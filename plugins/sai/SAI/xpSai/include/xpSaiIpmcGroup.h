// xpSaiIpmcGroup.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiIpmcGroup_h_
#define _xpSaiIpmcGroup_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Defines default number of IP Multicast Group Members */
#define XP_SAI_DEFAULT_MAX_IPMC_GROUP_MEMBERS 16

/**
 * @brief SAI IP Multicast Group Member Attributes struct
 */
typedef struct _xpSaiIpmcGroupMemberAttributesT
{
    sai_object_id_t groupId;
    sai_object_id_t outputId;

} xpSaiIpmcGroupMemberAttributesT;

/**
 * @brief SAI IP Multicast VLAN to Group Content DB
 */
typedef struct _xpSaiIpmcGroupVlan2GroupContextDbEntry
{
    /** Bridge or VlanId */
    uint16_t bvId;

    /** L2 Multicast Group XPS Id */
    uint32_t l2mcGroupId;

    /** L3 Multicast Group XPS info */
    uint32_t numItems;
    uint32_t l3mcGroupId[XP_SAI_DEFAULT_MAX_IPMC_GROUP_MEMBERS];

} xpSaiIpmcGroupVlan2GroupContextDbEntry;

/**
 * @brief SAI IP Multicast Group Member RIF type VLAN
 */
typedef struct _xpSaiIpmcGroupMemberRifTypeVlan
{
    /** L2 Multicast Group XPS Id */
    uint32_t l2mcGroupId;

    /** L2 Multicast Group Bridge or VlanId */
    uint16_t bvId;

} xpSaiIpmcGroupMemberRifTypeVlan;

/**
 * @brief SAI IP Multicast Group Member Data Base Entry struct
 */
typedef struct _xpSaiIpmcGroupMemberContextDbEntry
{
    /** IP Multicast Group Member Id OID as a key */
    sai_object_id_t groupMemberIdOid;

    /** IP Multicast Group Id OID */
    sai_object_id_t groupIdOid;

    /** Output Id OID */
    sai_object_id_t outputIdOid;

} xpSaiIpmcGroupMemberContextDbEntry;

/**
 * @brief SAI IP Multicast Group Data Base Entry struct
 */
typedef struct _xpSaiIpmcGroupContextDbEntry
{
    /** IP Multicast Group Id OID as a key */
    sai_object_id_t groupIdOid;

    /** IP Multicast Group RIF Type */
    int32_t ipmcRifType;

    /** Multicast Group members info */
    uint32_t numItems;
    sai_object_id_t groupMemberIdOid[XP_SAI_DEFAULT_MAX_IPMC_GROUP_MEMBERS];

} xpSaiIpmcGroupContextDbEntry;

sai_status_t xpSaiIpmcGroupOnVlanMemberUpdateCallback(xpsDevice_t xpsDevId,
                                                      uint32_t portId, uint16_t vlanId, bool create);

/**
 * \brief Standard xpSai module initialization interface
 */
sai_status_t xpSaiIpmcGroupApiInit(uint64_t flag,
                                   const sai_service_method_table_t* adapHostServiceMethodTable);
sai_status_t xpSaiIpmcGroupApiDeinit();
sai_status_t xpSaiIpmcGroupInit(xpsDevice_t xpSaiDevId);
sai_status_t xpSaiIpmcGroupDeInit(xpsDevice_t xpSaiDevId);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiIpmcGroup_h_
