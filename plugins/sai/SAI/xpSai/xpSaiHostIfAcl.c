// xpSaiHostAcl.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiHostInterface.h"

XP_SAI_LOG_REGISTER_API(SAI_API_HOSTIF);

#define XP_SAI_HOST_IF_ACL_IPV4_KEY_FLD_NUM 6
#define XP_SAI_HOST_IF_ACL_IPV4_KEY_SIZE WIDTH_128

#define XP_SAI_HOST_IF_ACL_IPV6_KEY_FLD_NUM 4
#define XP_SAI_HOST_IF_ACL_IPV6_KEY_SIZE WIDTH_128

static sai_status_t xpSaiHostIfAclTableCreate(xpIaclType_e tblType,
                                              xpIaclKeyType keyType,
                                              uint32_t keySize, uint32_t numTables, uint32_t numDb,
                                              xpIaclkeyField_t* iaclFields, uint32_t numFlds);

static sai_status_t xpSaiHostIfAclPortAclIdInit();

static sai_status_t xpSaiHostIfAclIpv4TableCreate();
static sai_status_t xpSaiHostIfAclIpv6TableCreate();

static sai_status_t xpSaiHostIfAclEntryCreate(uint32_t rule_id,
                                              xpsIaclData_t* pIaclData, xpIaclKeyType keyType,
                                              xpIaclkeyField_t* iaclFields, uint32_t numFlds);

//Func: xpSaiHostIfAclInit

sai_status_t xpSaiHostIfAclInit()
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    saiRetVal = xpSaiHostIfAclPortAclIdInit();
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostIfAclPortAclIdInit() failed with error %d \n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiHostIfAclIpv4TableCreate();
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostIfAclIpv4TableCreate() failed with error %d \n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiHostIfAclIpv6TableCreate();
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHostIfAclIpv6TableCreate() failed with error %d \n",
                       saiRetVal);
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostIfAclIpv4TableCreate

static sai_status_t xpSaiHostIfAclIpv4TableCreate()
{
    uint32_t fieldsNum = 0;
    xpIaclkeyField_t iaclFields[XP_SAI_HOST_IF_ACL_IPV4_KEY_FLD_NUM];

    memset(&iaclFields, 0, sizeof(iaclFields));

    iaclFields[fieldsNum++].fld.v4Fld   = XP_IACL_KEY_TYPE_V4;
    iaclFields[fieldsNum++].fld.v4Fld            = XP_IACL_ID;
    iaclFields[fieldsNum++].fld.v4Fld        = XP_IACL_MAC_DA;
    iaclFields[fieldsNum++].fld.v4Fld = XP_IACL_V4_ETHER_TYPE;
    iaclFields[fieldsNum++].fld.v4Fld      = XP_IACL_PROTOCOL;
    iaclFields[fieldsNum++].fld.v4Fld  = XP_IACL_L4_DEST_PORT;

    return xpSaiHostIfAclTableCreate(XP_IACL0, XP_IACL_V4_TYPE,
                                     XP_SAI_HOST_IF_ACL_IPV4_KEY_SIZE, 1, 1, iaclFields, fieldsNum);
}

//Func: xpSaiHostIfAclIpv6TableCreate

static sai_status_t xpSaiHostIfAclIpv6TableCreate()
{
    return SAI_STATUS_SUCCESS;

#if 0
ToDo:
    IPv6 ACL code requires implementation
    uint32_t fieldsNum = 0;
    xpIaclkeyField_t iaclFields[XP_SAI_HOST_IF_ACL_IPV6_KEY_FLD_NUM];

    memset(&iaclFields, 0, sizeof(iaclFields));

    iaclFields[fieldsNum++].fld.v6Fld      = XP_IACL_KEY_TYPE_V6;
    iaclFields[fieldsNum++].fld.v6Fld            = XP_IACL_V6_ID;
    iaclFields[fieldsNum++].fld.v6Fld  = XP_IACL_L4_V6_DEST_PORT;

    return xpSaiHostIfAclTableCreate(XP_IACL0, XP_IACL_V6_TYPE,
                                     XP_SAI_HOST_IF_ACL_IPV6_KEY_SIZE, 1, 1, iaclFields, fieldsNum);
#endif
}

//Func: xpSaiHostIfAclTableCreate

static sai_status_t xpSaiHostIfAclTableCreate(xpIaclType_e tblType,
                                              xpIaclKeyType keyType,
                                              uint32_t keySize, uint32_t numTables, uint32_t numDb,
                                              xpIaclkeyField_t* iaclFields, uint32_t numFlds)
{
    XP_STATUS retVal = XP_NO_ERR;

    xpIaclTableProfile_t iaclTblProfile;
    xpIaclkeyFieldList_t iaclFldList;

    memset(&iaclTblProfile, 0, sizeof(iaclTblProfile));
    memset(&iaclFldList, 0, sizeof(iaclFldList));

    xpsDevice_t xpsDevId = xpSaiGetDevId();

    iaclTblProfile.numTables = numTables;
    iaclTblProfile.tableProfile[0].tblType = tblType;
    iaclTblProfile.tableProfile[0].keySize = keySize;
    iaclTblProfile.tableProfile[0].numDb = numDb;

    retVal = xpsIaclCreateTable(xpsDevId, iaclTblProfile);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsIaclDefinePaclKey() failed with error %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    iaclFldList.numFlds = numFlds;
    iaclFldList.type    = keyType;
    iaclFldList.fldList = iaclFields;
    iaclFldList.isValid = TRUE;

    retVal = xpsIaclDefinePaclKey(xpsDevId, keyType, &iaclFldList);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsIaclDefinePaclKey() failed with error %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostIfAclPortAclIdInit

static sai_status_t xpSaiHostIfAclPortAclIdInit()
{
    XP_STATUS        retVal = XP_NO_ERR;
    xpsInterfaceId_t intfId = 0;

    xpsDevice_t xpsDevId = xpSaiGetDevId();

    retVal = xpsPortGetFirst(&intfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsPortGetFirst() failed with error %d \n", retVal);
        return  xpsStatus2SaiStatus(retVal);
    }

    do
    {
        XP_SAI_LOG_DBG("Set acl_id %d to the port %d\n", intfId, intfId);

        retVal = xpsPortSetField(xpsDevId, intfId, XPS_PORT_INGRESS_VIF, 1);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsPortSetField(XPS_PORT_INGRESS_VIF) failed with error %d \n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        retVal = xpsPortSetField(xpsDevId, intfId, XPS_PORT_ACL_ID, intfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsPortSetField(XPS_PORT_ACL_ID) failed with error %d \n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        retVal = xpsPortSetField(xpsDevId, intfId, XPS_PORT_ACL_EN, TRUE);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsPortSetField(XPS_PORT_ACL_EN) failed with error %d \n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }
    while (xpsPortGetNext(intfId, &intfId) == XP_NO_ERR);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostIfAclEntryCreate

static sai_status_t xpSaiHostIfAclEntryCreate(uint32_t rule_id,
                                              xpsIaclData_t* pIaclData, xpIaclKeyType keyType,
                                              xpIaclkeyField_t* iaclFields, uint32_t numFlds)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpIaclkeyFieldList_t iaclFldList;

    memset(&iaclFldList, 0, sizeof(iaclFldList));

    xpsDevice_t xpsDevId = xpSaiGetDevId();

    iaclFldList.numFlds = numFlds;
    iaclFldList.type    = keyType;
    iaclFldList.fldList = iaclFields;
    iaclFldList.isValid = TRUE;

    retVal = xpsIaclWritePaclKey(xpsDevId, rule_id, &iaclFldList);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsIaclWritePaclKey() failed with error %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsIaclWritePaclData(xpsDevId, rule_id, pIaclData);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsIaclWritePaclData() failed with error %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiHostIfAclIpv4EntryCreate

sai_status_t xpSaiHostIfAclIpv4EntryCreate(uint32_t rule_id, uint32_t acl_id,
                                           xpPktCmd_e pktCmd, sai_mac_t mac, uint16_t etherType, uint8_t ipProto,
                                           uint16_t dstPort)
{
    uint32_t fieldsNum      = 0;
    uint8_t  set_mask[6]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t  ignore_mask[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t  type           = XP_IACL_KEY_TYPE_V4;

    xpIaclkeyField_t iaclFields[XP_SAI_HOST_IF_ACL_IPV4_KEY_FLD_NUM];
    xpsIaclData_t iaclDataEntry;

    memset(&iaclFields, 0, sizeof(iaclFields));
    memset(&iaclDataEntry, 0, sizeof(iaclDataEntry));

    iaclFields[fieldsNum].fld.v4Fld = XP_IACL_KEY_TYPE_V4;
    iaclFields[fieldsNum].value     = (uint8_t *) &type;
    iaclFields[fieldsNum++].mask    = set_mask;

    iaclFields[fieldsNum].fld.v4Fld = XP_IACL_ID;
    iaclFields[fieldsNum].value     = (uint8_t *) &acl_id;
    iaclFields[fieldsNum++].mask    = set_mask;

    iaclFields[fieldsNum].fld.v4Fld = XP_IACL_MAC_DA;
    iaclFields[fieldsNum].value     = (uint8_t *) mac;
    iaclFields[fieldsNum++].mask    = mac[0] ? set_mask : ignore_mask;

    iaclFields[fieldsNum].fld.v4Fld = XP_IACL_V4_ETHER_TYPE;
    iaclFields[fieldsNum].value     = (uint8_t *) &etherType;
    iaclFields[fieldsNum++].mask    = etherType ? set_mask : ignore_mask;

    iaclFields[fieldsNum].fld.v4Fld = XP_IACL_PROTOCOL;
    iaclFields[fieldsNum].value     = (uint8_t *) &ipProto;
    iaclFields[fieldsNum++].mask    = ipProto ? set_mask : ignore_mask;

    iaclFields[fieldsNum].fld.v4Fld  = XP_IACL_L4_DEST_PORT;
    iaclFields[fieldsNum].value     = (uint8_t *) &dstPort;
    iaclFields[fieldsNum++].mask    = dstPort ? set_mask : ignore_mask;

    iaclDataEntry.iAclDataType.aclData.enPktCmdUpd       = TRUE;
    iaclDataEntry.iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command           =
        pktCmd;

    XP_SAI_LOG_DBG("New IPv4 rule, rule_id %d, acl_id %d, pktCmd %d, etherType %d, ipProto %d, dstPort %d, fieldsNum %d\n",
                   rule_id, acl_id, pktCmd, etherType, ipProto, dstPort, fieldsNum);

    return xpSaiHostIfAclEntryCreate(rule_id, &iaclDataEntry, XP_IACL_V4_TYPE,
                                     iaclFields, fieldsNum);
}

//Func: xpSaiHostIfAclIpv6EntryCreate

sai_status_t xpSaiHostIfAclIpv6EntryCreate(uint32_t rule_id, uint32_t acl_id,
                                           xpPktCmd_e pktCmd)
{
    uint32_t acl_id_mask = 0;
    uint32_t fieldsNum   = 0;

    xpIaclkeyField_t iaclFields[XP_SAI_HOST_IF_ACL_IPV6_KEY_FLD_NUM];
    xpsIaclData_t iaclDataEntry;

    memset(&iaclFields, 0, sizeof(iaclFields));
    memset(&iaclDataEntry, 0, sizeof(iaclDataEntry));

    iaclFields[fieldsNum].fld.v6Fld = XP_IACL_V6_ID;
    iaclFields[fieldsNum].value     = (uint8_t *) &acl_id;
    iaclFields[fieldsNum++].mask    = (uint8_t *) &acl_id_mask;

    iaclDataEntry.iAclDataType.aclData.enPktCmdUpd = TRUE;
    iaclDataEntry.iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command      =
        pktCmd;

    return xpSaiHostIfAclEntryCreate(rule_id, &iaclDataEntry, XP_IACL_V6_TYPE,
                                     iaclFields, fieldsNum);
}
