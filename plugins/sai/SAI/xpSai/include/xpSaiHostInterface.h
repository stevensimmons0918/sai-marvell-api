// xpSaiHostInterface.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiHostInterface_h_
#define _xpSaiHostInterface_h_

#include "xpSai.h"

#define XPSAI_HOSTIF_TRAP_REASON_CODE_MAX_NUM    10

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _xpSaiHostInterfaceAttributesT
{
    sai_attribute_value_t type;
    sai_attribute_value_t ifId;
    sai_attribute_value_t name;
    sai_attribute_value_t mcgrpName;  //Only for GENETLINK type
} xpSaiHostInterfaceAttributesT;

typedef struct _xpSaiHostInterfaceChannelDbEntryT
{
    // DB Key
    uint32_t          keyId;        // Host Interface unique ID

    // DB Data
    uint32_t              intfFd;
    sai_hostif_type_t     type;
    sai_hostif_vlan_tag_t vlanTagType;
    char                  name[SAI_HOSTIF_NAME_SIZE];
    char                  mcgrpName[SAI_HOSTIF_GENETLINK_MCGRP_NAME_SIZE];
    uint32_t              mcgrpId;
    uint16_t              genetlinkFamilyId;
} xpSaiHostInterfaceChannelDbEntryT;

typedef struct _xpSaiHostInterfaceTrapAttributesT
{
    sai_attribute_value_t trapType;
    sai_attribute_value_t packetAction;
    sai_attribute_value_t trapPriority;
    sai_attribute_value_t excludePortList;
    sai_attribute_value_t trapGroup;
} xpSaiHostInterfaceTrapAttributesT;

typedef struct _xpSaiHostInterfaceTrapGroupAttributesT
{
    sai_attribute_value_t adminState;
    sai_attribute_value_t prio;
    sai_attribute_value_t queue;
    sai_attribute_value_t policer;
} xpSaiHostInterfaceTrapGroupAttributesT;

typedef struct _xpSaiHostInterfaceGroupDbEntryT
{
    // DB Key
    uint32_t  keyId;                // Host Interface Group unique ID

    // DB Data
    bool      adminState;
    sai_object_id_t policerOid;
    uint32_t  queue;
} xpSaiHostInterfaceGroupDbEntryT;

typedef struct _xpSaiHostInterfaceTrapDbEntryT
{
    // DB Key
    sai_object_id_t
    trapOid;        // SAI_OBJECT_TYPE_HOSTIF_TRAP or SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP

    // DB Data
    sai_packet_action_t
    action;         // valid for SAI_OBJECT_TYPE_HOSTIF_TRAP only
    uint32_t                    priority;
    sai_object_id_t             groupId;
    bool                        *validPort;
    uint8_t
    reasonCodeNum;  // Number of XDK reason codes mapped to SAI trap
    uint32_t                    reasonCode[XPSAI_HOSTIF_TRAP_REASON_CODE_MAX_NUM];
} xpSaiHostInterfaceTrapDbEntryT;

typedef struct _xpSaiHostInterfaceTrapDefaultT
{
    sai_hostif_trap_type_t      trapType;       // Trap type
    sai_packet_action_t         action;         // Default trap action
    sai_mac_t                   mac;            // L2 control MAC
    uint8_t
    reasonCodeNum;  // Number of XDK reason codes mapped to SAI trap
    uint32_t                    reasonCode[XPSAI_HOSTIF_TRAP_REASON_CODE_MAX_NUM];
    bool                        enable;         // Enable implemented/supported trap
} xpSaiHostInterfaceTrapDefaultT;

typedef struct _xpSaiHostInterfacePolicerDbEntry
{
    sai_object_id_t keyOid; ///< SAI Policer object ID

    struct
    {
        sai_uint32_t acmIndex; ///< SAI Policer entry ID
        sai_size_t   refCount; ///< SAI Policer entry reference counter
    } data;
} xpSaiHostInterfacePolicerDbEntry_t;

typedef struct xpTrapConfig
{
    uint32_t trapId;
    uint32_t reasonCode;
    sai_hostif_table_entry_channel_type_t channel;
    sai_hostif_table_entry_type_t intfType;
    uint32_t fd;
} xpTrapConfig;

typedef struct _xpSaiHostInterfaceTableAttributesT
{
    sai_attribute_value_t type;
    sai_attribute_value_t objId;
    sai_attribute_value_t trapId;
    sai_attribute_value_t channelType;
    sai_attribute_value_t hostIf;
} xpSaiHostInterfaceTableAttributesT;

typedef struct _xpSaiHostInterfaceTableEntryT
{
    uint32_t tableEntryId;
    uint32_t reasonCode;
} xpSaiHostInterfaceTableEntryT;

typedef struct _xpSaiHostInterfaceTableDbEntryT
{
    // DB Key
    uint32_t                        keyId;  // Host Interface Table Entry unique ID

    // DB Data
    sai_hostif_table_entry_type_t   type;
    sai_object_id_t                 interfaceObjId;     /* PORT | LAG | VLAN */
    sai_object_id_t                 trapId;
    sai_hostif_table_entry_channel_type_t   channelType;
    sai_object_id_t
    hostFd;             /* SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_FD */
    uint8_t                         tableEntryNum;
    xpSaiHostInterfaceTableEntryT
    tableEntry[XPSAI_HOSTIF_TRAP_REASON_CODE_MAX_NUM];
} xpSaiHostInterfaceTableDbEntryT;

XP_STATUS xpSaiHostInterfaceApiInit(uint64_t flag,
                                    const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiHostInterfaceApiDeinit();

void *xpSaiHostInterfaceCbThread(void *arg);
void *xpSaiHostInterfaceTapThread(void *arg);

sai_status_t xpSaiHostInterfaceInit(xpsDevice_t devId);
sai_status_t xpSaiHostInterfaceDeInit(xpsDevice_t devId);

XP_STATUS xpSaiHostInterfaceNetdevInit(void);
XP_STATUS xpSaiHostInterfaceNetdevDeInit(void);

sai_status_t xpSaiConvertHostIntfOid(sai_object_id_t hif_id, uint32_t* pIntf);
sai_status_t xpSaiHostInterfaceTrapIdParse(xpsDevice_t devId,
                                           uint32_t reasonCode, sai_attribute_t *attr);
sai_status_t xpSaiHostInterfacePortTypeParse(xpsDevice_t devId,
                                             xpsInterfaceType_e intfType, xpsInterfaceId_t intfId, sai_attribute_t *attr);

sai_status_t xpSaiHostIntfChannelDbInfoGet(sai_object_id_t interfaceOid,
                                           xpSaiHostInterfaceChannelDbEntryT **ppChannelEntry);


XP_STATUS xpSaiHostIfNetdevCreate(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                  uint8_t *intfName);
XP_STATUS xpSaiHostIfNetdevDelete(xpsDevice_t devId, xpsInterfaceId_t intfId);
XP_STATUS xpSaiHostIfNetdevTrapSet(xpsDevice_t devId, xpTrapConfig *trapConf,
                                   bool create);
XP_STATUS xpSaiHostIfLinkStatusSet(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                   bool status);
XP_STATUS xpSaiHostIfVlanTagTypeSet(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                    uint32_t tagType);


sai_status_t xpSaiSetSwitchMac(uint8_t* ifName);
sai_status_t xpSaiFdObjCreate(uint32_t *pFd);
sai_status_t xpSaiFdObjDelete(uint32_t fd);
sai_status_t xpSaiFdPacketRecv(sai_object_id_t  hif_id,
                               sai_size_t *buffer_size,
                               void *buffer,
                               uint32_t *attr_count,
                               sai_attribute_t *attr_list
                              );
sai_status_t xpSaiFdPacketSend(sai_object_id_t  hif_id,
                               sai_size_t buffer_size,
                               const void *buffer,
                               uint32_t attr_count,
                               const sai_attribute_t *attr_list
                              );

sai_status_t xpSaiGenetlinkObjCreate(uint8_t *genetlinkName,
                                     uint8_t *mcgrpName,
                                     uint32_t *pFd, uint16_t *familyId, uint32_t *mcgrpId);
sai_status_t xpSaiGenetlinkObjDelete(uint32_t fd);


sai_status_t xpSaiMaxCountHostInterfaceAttribute(uint32_t *count);
sai_status_t xpSaiCountHostInterfaceObjects(uint32_t *count);
sai_status_t xpSaiGetHostInterfaceObjectList(uint32_t *object_count,
                                             sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetHostInterfaceAttributes(sai_object_id_t id,
                                                 uint32_t *attr_count, sai_attribute_t *attr_list);

sai_status_t xpSaiMaxCountHostInterfaceTrapAttribute(uint32_t *count);
sai_status_t xpSaiCountHostInterfaceTrapObjects(uint32_t *count);
sai_status_t xpSaiGetHostInterfaceTrapObjectList(uint32_t *object_count,
                                                 sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetHostInterfaceTrapAttributes(sai_object_id_t id,
                                                     uint32_t *attr_count, sai_attribute_t *attr_list);

sai_status_t xpSaiMaxCountHostInterfaceTrapGroupAttribute(uint32_t *count);
sai_status_t xpSaiCountHostInterfaceTrapGroupObjects(uint32_t *count);
sai_status_t xpSaiGetHostInterfaceTrapGroupObjectList(uint32_t *object_count,
                                                      sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetHostInterfaceTrapGroupAttributes(sai_object_id_t id,
                                                          uint32_t *attr_count, sai_attribute_t *attr_list);

sai_status_t xpSaiMaxCountHostInterfaceTableEntryAttribute(uint32_t *count);
sai_status_t xpSaiCountHostInterfaceTableEntryObjects(uint32_t *count);
sai_status_t xpSaiGetHostInterfaceTableEntryObjectList(uint32_t *object_count,
                                                       sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetHostInterfaceTableEntryAttributes(sai_object_id_t id,
                                                           uint32_t *attr_count, sai_attribute_t *attr_list);

sai_status_t xpSaiMaxCountHostInterfaceUserTrapAttribute(uint32_t *count);
sai_status_t xpSaiCountHostInterfaceUserTrapObjects(uint32_t *count);
sai_status_t xpSaiGetHostInterfaceUserTrapObjectList(uint32_t *object_count,
                                                     sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetHostInterfaceUserTrapAttributes(sai_object_id_t id,
                                                         uint32_t *attr_count, sai_attribute_t *attr_list);

sai_status_t xpSaiHostInterfaceUserTrapIdRangeGet(uint32_t *trapIdMin,
                                                  uint32_t *trapIdMax);
sai_status_t xpSaiHostInterfaceUserTrapReasonCodeByOid(sai_object_id_t
                                                       userTrapOid, uint32_t *reasonCode);
sai_status_t xpSaiHostInterfaceGetXpsCoppCtrlPktType(
    sai_hostif_trap_type_t saiTrap, xpsCoppCtrlPkt_e* pXpsTrap);

sai_status_t xpSaiHostIntfTrapDbInfoGet(sai_object_id_t trapOid,
                                        xpSaiHostInterfaceTrapDbEntryT **ppTrapEntry);

sai_status_t xpSaiHostIntfGroupDbInfoGet(sai_object_id_t trap_group_id,
                                         xpSaiHostInterfaceGroupDbEntryT **ppGrpEntry);

xpSaiHostInterfaceTrapDefaultT *xpSaiHostIntfDefaultTrapGet(
    sai_hostif_trap_type_t trapType);


#define XP_DEFAULT_CPU_QUEUE   0
#define XP_SFLOW_CPU_QUEUE     1

void xpSaiSetSoftFwdInfo(sai_mac_t dstMac, sai_object_id_t outIntf);
void xpSaiResetSoftFwdInfo();


sai_status_t xpSaiHostInterfaceTrapQueueDropCounter(uint32_t queue,
                                                    uint64_t *DropPkts);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiHostInterface_h_
