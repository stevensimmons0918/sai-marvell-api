// xpSaiLag.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiLag_h_
#define _xpSaiLag_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

typedef struct _xpSaiLagMemberAttributesT
{
    sai_attribute_value_t lagId;
    sai_attribute_value_t portId;
    sai_attribute_value_t egressDisable;
    sai_attribute_value_t ingressDisable;
} xpSaiLagMemberAttributesT;
/*
16 bits - obj type
48 bits - obj value ~ Reffering to XDK_SAI_OBJID_VALUE_BITS
   Divided:
      24 bits - LAG ID
      24 bits - PORT ID
*/
#define SAI_LAG_MEMBER_SUB_OBJID_VALUE_BITS 24
#define SAI_LAG_MEMBER_OBJ_VALUE_MASK 0x0000000000FFFFFFULL

typedef struct xpSaiPortLagInfo_t
{
    xpsInterfaceId_t intfId;
    xpsInterfaceId_t lagId;
    sai_uint16_t portVlanId;
    sai_uint8_t defaultVlanPriority;
    bool dropTagged;
    bool dropUntagged;
    xpsPktCmd_e saMissCmd;
    bool autoDistribute;
    bool egressDisable;
    bool ingressDisable;
} xpSaiPortLagInfo_t;

typedef struct xpSaiLagPortCountInfo_t
{
    xpsInterfaceId_t lagId;   /*DB Key*/
    xpsVlan_t vlanId;         /*DB Key*/
    uint32_t membersPortCount;
} xpSaiLagPortCountInfo_t;

typedef struct xpSaiPrimaryLagIdNode
{
    uint32_t lagId;
    struct xpSaiPrimaryLagIdNode * next;
}  xpSaiPrimaryLagIdNode_t;

typedef struct xpSaiLagInfo_t
{
    xpsInterfaceId_t lagId;                        /* DB Key */
    xpsInterfaceId_t
    primaryLagId;                 /* Interface ID of the primary LAG(valid for the secondary LAG only) */
    bool
    isLagEgressRedirectEnabled;   /* Flag indicating that LAG egress redirect is enabled. */
    xpsPortList_t
    primaryLagEgressPortList;     /* Egress port list of primary LAG */
    bool
    isEgressBlockListEn;          /* Blocked port list flag. When true -- ingress blocking is active. Used for egress blocking only */
    xpsPortList_t
    egressBlockPortList;          /* Blocked port list. Used for egress blocking only */
    uint32_t
    lagPortBitMapPtrIdx;          /* Lag's PortBitMap Pointer Index */
    uint32_t
    secondaryLagPortBitMapPtrIdx;  /* Secondary Lag's PortBitMap Pointer Index  (Valid Only for Primary Lag Only) */
    uint8_t
    secondaryLagRedirectCount;     /* This field holds the value of number of Number of Primary Lags that are in Redirect Enable Mode. This value is valid only for Secondary Lag*/
    xpSaiPrimaryLagIdNode_t
    *primaryLagIdList;    /* Value valid for Secodary Lag only. Stores all the primaryLagIds that are redirect enabled*/
    uint32_t
    refCnt;                        /* Counter of direct references to this LAG */
    xpSaiPortLagInfo_t
    lagConfig;            /* Common config for all LAG members */
    sai_object_id_t  ingressAclId;
    sai_object_id_t  egressAclId;
    uint16_t pvidUserSetting;   /*keep the pvid that user set, needed when remove untagged vlan or remove from L3*/
} xpSaiLagInfo_t;

sai_int32_t xpSaiLagPortCountKeyComp(void *key1, void *key2);
sai_status_t xpSaiLagPortCountInfoGet(xpsInterfaceId_t lagId, xpsVlan_t vlanId,
                                      xpSaiLagPortCountInfo_t **ppSaiLagPortCountInfoEntry);

sai_uint64_t xpSaiLagMemberLagIdValueGet(sai_object_id_t sai_object_id);
sai_uint64_t xpSaiLagMemberPortIdValueGet(sai_object_id_t sai_object_id);
sai_status_t xpSaiLagMemberObjIdCreate(sai_object_type_t sai_object_type,
                                       sai_uint64_t lag_id,  sai_uint64_t port_id, sai_object_id_t *sai_object_id);

XP_STATUS xpSaiLagApiInit(uint64_t flag,
                          const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiLagApiDeinit();

sai_status_t xpSaiMaxCountLagAttribute(uint32_t *count);
sai_status_t xpSaiMaxCountLagMemberAttribute(uint32_t *count);

sai_status_t xpSaiCountLagObjects(uint32_t *count);
sai_status_t xpSaiCountLagMemberObjects(uint32_t *count);

sai_status_t xpSaiGetLagObjectList(uint32_t *object_count,
                                   sai_object_key_t *object_list);
sai_status_t xpSaiGetLagMemberObjectList(uint32_t *object_count,
                                         sai_object_key_t *object_list);

sai_status_t xpSaiGetLagAttributes(sai_object_id_t lag_id, uint32_t attr_count,
                                   sai_attribute_t *attr_list);
sai_status_t xpSaiBulkGetLagMemberAttributes(sai_object_id_t id,
                                             uint32_t *attr_count, sai_attribute_t *attr_list);

XP_STATUS xpSaiLagInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiLagDeInit(xpsDevice_t xpSaiDevId);
sai_status_t xpSaiLagInfoBlockedPortListSet(xpsInterfaceId_t lagId,
                                            xpsPortList_t *blockedPortList, uint32_t enable);
sai_status_t xpSaiLagInfoBlockedPortListGet(xpsInterfaceId_t lagId,
                                            xpsPortList_t *blockedPortList);

sai_status_t xpSaiLagRef(xpsInterfaceId_t xpsLagId);
sai_status_t xpSaiLagUnref(xpsInterfaceId_t xpsLagId);
XP_STATUS xpSaiLagSetSaMissCmd(xpsInterfaceId_t xpsLagId, xpsPktCmd_e pktCmd);

sai_status_t xpSaiSetLagAttrPortVlanId(sai_object_id_t lagId,
                                       sai_attribute_value_t value);
sai_status_t xpSaiGetLagAttrPortVlanId(sai_object_id_t lagId,
                                       sai_attribute_value_t* value);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _xpSaiLag_h_ */
