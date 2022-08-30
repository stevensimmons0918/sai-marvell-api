// xpSaiHash.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiHash.h"
#include "xpSaiValidationArrays.h"
#include "xpsAllocator.h"

XP_SAI_LOG_REGISTER_API(SAI_API_HASH);
#define XP_SAI_HASH_MAX_IDS     128
#define XP_SAI_HASH_RANGE_START 0

static sai_hash_api_t* _xpSaiHashApi;
static xpsDbHandle_t sXpSaiHashDbHndl = XPS_STATE_INVALID_DB_HANDLE;

/*
 * Static Function Prototypes
 */

static sai_status_t xpSaiHashCreate(sai_object_id_t *hash_id,
                                    sai_object_id_t switch_id,
                                    uint32_t attr_count, const sai_attribute_t *attr_list);
static sai_status_t xpSaiHashRemove(sai_object_id_t hash_id);
static sai_status_t xpSaiHashInfoInit(xpSaiHashDbEntry_t *info);
static int32_t saiHashKeyComp(void* key1, void* key2);
static sai_status_t xpSaiHashInfoGet(sai_object_id_t hashId,
                                     xpSaiHashDbEntry_t **ppInfo);
static sai_status_t xpSaiHashOidConvert(sai_object_id_t hashOid,
                                        uint32_t* pHashId);
static sai_status_t xpSaiHashAttributeListGet(sai_object_id_t hash_id,
                                              uint32_t attr_count, sai_attribute_t *attr_list);
static sai_status_t xpSaiHashFieldListGet(sai_object_id_t hash_id,
                                          sai_attribute_value_t *fieldList);
static sai_status_t xpSaiHashDefaultObjCreate(void);
static sai_status_t xpSaiHashDefaultObjRemove(void);
static sai_status_t xpSaiHashLagFieldSet(sai_object_id_t hashId);
static sai_status_t xpSaiHashLagFieldGet(sai_attribute_value_t *fieldList);
static sai_status_t xpSaiConvertNativeHashFields(xpSaiHashDbEntry_t* pHashInfo,
                                                 const sai_attribute_value_t *fieldList);
static sai_status_t xpSaiHashCrcFieldSet(uint32_t hashType,
                                         sai_object_id_t hash_id);
//default Obj for Default/IPv4/IPv6 hashing
static sai_object_id_t xpSaiHashObj[XPSAI_MAX_HASH_OBJECT_INDEX] = {SAI_NULL_OBJECT_ID, SAI_NULL_OBJECT_ID, SAI_NULL_OBJECT_ID};

sai_status_t xpSaiSwitchHashGet(uint32_t hashType, sai_object_id_t *phashId)
{
    if ((hashType == XPSAI_L2_HASH_OBJECT_INDEX) ||
        (hashType == XPSAI_IPV4_HASH_OBJECT_INDEX) ||
        (hashType == XPSAI_IPV6_HASH_OBJECT_INDEX))
    {
        *phashId = xpSaiHashObj[hashType];
        return SAI_STATUS_SUCCESS;
    }
    return SAI_STATUS_FAILURE;
}
/**
 * \brief Initializes the HASH info structure
 *
 * \param [in] info
 *
 * \return XP_STATUS
 */
static sai_status_t xpSaiHashInfoInit(xpSaiHashDbEntry_t *info)
{
    if (info == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(info, 0, sizeof(xpSaiHashDbEntry_t));

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief SAI Hash State Key Compare function
 *
 * This API is used by the state manager as a method to compare
 * keys
 *
 * \param [in] void* key1
 * \param [in] void* key2
 *
 * \return int32_t
 */
static int32_t saiHashKeyComp(void* key1, void* key2)
{
    if (((xpSaiHashDbEntry_t*)key1)->keyHashId >= ((xpSaiHashDbEntry_t*)
                                                   key2)->keyHashId)
    {
        return (((xpSaiHashDbEntry_t*)key1)->keyHashId > ((xpSaiHashDbEntry_t*)
                                                          key2)->keyHashId);
    }

    return -1;
}

/**
 * \brief SAI Hash OID to local id converting function
 *
 * This API is used as a method to convert Hash OID to local hash id
 * keys
 *
 * \param [in] sai_object_id_t hashOid
 * \param [out] uint32_t* pHashId
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiHashOidConvert(sai_object_id_t hashOid,
                                        uint32_t* pHashId)
{
    if (!XDK_SAI_OBJID_TYPE_CHECK(hashOid, SAI_OBJECT_TYPE_HASH))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n", xpSaiObjIdTypeGet(hashOid));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *pHashId = (uint32_t)xpSaiObjIdValueGet(hashOid);

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief SAI Hash create
 *
 * This API is used as a method to create Hash object, will not apply the nativeHashFields to hardware
 * use xpSaiHashAttributeSet to apply the nativeHashFields.
 *
 * \param[out] sai_object_id_t *hash_id
 * \param[in] sai_object_id_t switch_id
 * \param[in] uint32_t attr_count
 * \param[in] const sai_attribute_t *attr_list
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiHashCreate(sai_object_id_t *hash_id,
                                    sai_object_id_t switch_id,
                                    uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS status = XP_NO_ERR;
    uint32_t xpSaiHashId = 0;
    xpSaiHashDbEntry_t *hashInfo = NULL;
    sai_uint32_t dev_id =  xpSaiObjIdSwitchGet(switch_id);

    XP_SAI_LOG_DBG("Calling xpSaiHashCreate\n");

    if (hash_id == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    status = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_HASH,
                                    &xpSaiHashId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate Hash id, retVal %d\n", status);
        return xpsStatus2SaiStatus(status);
    }

    XP_SAI_LOG_DBG("Allocated xpSaiHashId = %d\n", xpSaiHashId);

    // Create a new HASH Info structure
    if ((status = xpsStateHeapMalloc(sizeof(xpSaiHashDbEntry_t),
                                     (void**)&hashInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate HASH structure, HASH ID %d\n", xpSaiHashId);
        return SAI_STATUS_FAILURE;
    }

    // Initialize the allocated state
    if ((saiRetVal = xpSaiHashInfoInit(hashInfo)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not initialize HASH structure, HASH ID %d\n",
                       xpSaiHashId);
        xpsStateHeapFree((void*)hashInfo);
        return saiRetVal;
    }

    //write SAI Hash fields to hashInfo
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST:
                saiRetVal = xpSaiConvertNativeHashFields(hashInfo,
                                                         (const sai_attribute_value_t *) &attr_list[count].value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Could not set hashInfo, HASH ID %d\n", xpSaiHashId);
                    xpsStateHeapFree((void*)hashInfo);
                    return SAI_STATUS_FAILURE;
                }
                break;
            default:
                break;
        }
    }

    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_HASH, dev_id, (sai_uint64_t)xpSaiHashId,
                         hash_id) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object could not be created.\n");
        xpsStateHeapFree((void*)hashInfo);
        return SAI_STATUS_FAILURE;
    }

    hashInfo->keyHashId = *hash_id;
    // Insert the state into the database
    if ((status = xpsStateInsertData(XP_SCOPE_DEFAULT, sXpSaiHashDbHndl,
                                     (void*)hashInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to insert data into SAI HASH DB\n");
        xpsStateHeapFree((void*)hashInfo);
        return xpsStatus2SaiStatus(status);
    }
    return saiRetVal;
}

static sai_status_t xpSaiHashObjRemove(sai_object_id_t hash_id)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS retVal = XP_NO_ERR;
    uint32_t xpSaiHashId = 0;
    xpSaiHashDbEntry_t* hashInfo  = NULL;
    xpSaiHashDbEntry_t *key = NULL;

    XP_SAI_LOG_DBG("Calling xpSaiHashObjRemove\n");

    key = (xpSaiHashDbEntry_t *)xpMalloc(sizeof(xpSaiHashDbEntry_t));
    if (!key)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for key\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(key, 0x00, sizeof(xpSaiHashDbEntry_t));

    if ((saiRetVal = xpSaiHashOidConvert(hash_id,
                                         &xpSaiHashId)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to convert SAI HASH OID\n");
        xpFree(key);
        return saiRetVal;
    }

    // Remove Hash entry from the database
    key->keyHashId = hash_id;

    if ((retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, sXpSaiHashDbHndl, key,
                                     (void**)&hashInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to delete data from SAI HASH DB\n");
        xpFree(key);
        return xpsStatus2SaiStatus(retVal);
    }

    // Free the memory allocated for the Hash entry
    if ((retVal = xpsStateHeapFree(hashInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to to free allocated memory\n");
        xpFree(key);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_HASH,
                                   xpSaiHashId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not release Hash id %d\n", xpSaiHashId);
        xpFree(key);
        return retVal;
    }

    xpFree(key);
    return saiRetVal;
}

/**
 * \brief SAI Hash Object remove
 *
 * This API is used as a method to remove Hash object
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiHashRemove(sai_object_id_t hash_id)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiHashRemove\n");

    if ((hash_id == xpSaiHashObj[XPSAI_L2_HASH_OBJECT_INDEX]) ||
        (hash_id == xpSaiHashObj[XPSAI_IPV4_HASH_OBJECT_INDEX]) ||
        (hash_id == xpSaiHashObj[XPSAI_IPV6_HASH_OBJECT_INDEX]))
    {
        XP_SAI_LOG_ERR("Error: Cannot remove hash, because it is in used");
        return XP_ERR_OP_NOT_SUPPORTED;
    }

    saiRetVal = xpSaiHashObjRemove(hash_id);
    if (saiRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Cannot remove hash object");
    }
    return saiRetVal;
}


/**
 * \brief SAI Hash DB entry getting function
 *
 * This API is used as a method to search and return a pointer to
 * Hash entry in DB
 *
 * \param [in] sai_object_id_t hashId
 * \param [out] xpSaiHashDbEntry_t **ppInfo
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiHashInfoGet(sai_object_id_t hashId,
                                     xpSaiHashDbEntry_t **ppInfo)
{
    XP_STATUS      status   = XP_NO_ERR;
    xpSaiHashDbEntry_t *key = NULL;

    key = (xpSaiHashDbEntry_t *)xpMalloc(sizeof(xpSaiHashDbEntry_t));
    if (!key)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for key\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(key, 0x00, sizeof(xpSaiHashDbEntry_t));

    key->keyHashId = hashId;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, sXpSaiHashDbHndl, key,
                                (void**)ppInfo);

    if (*ppInfo == NULL)
    {
        XP_SAI_LOG_DBG("Could not find HASH info");
        xpFree(key);
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }

    xpFree(key);
    return xpsStatus2SaiStatus(status);
}

/**
 * \brief SAI Hash field setting function
 *
 * This API is used as a method to write SAI Hash fields to hardware
 *
 * \param [in] sai_object_id_t hashId
 * \param [in] sai_object_id_t hashId
 *
 * \return sai_status_t
 */
sai_status_t xpSaiHashFieldSet(uint32_t hashType, sai_object_id_t hashId,
                               bool setXOR, bool setCRC)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    XP_SAI_LOG_DBG("xpSaiHashFieldSet()\n");

    if (setXOR)
    {
        /*xpSaiHashLagFieldSet configure the gobal simple hash, only  need by default hash*/
        if ((hashType == XPSAI_L2_HASH_OBJECT_INDEX) &&
            (hashId == xpSaiHashObj[XPSAI_L2_HASH_OBJECT_INDEX]))
        {
            saiStatus = xpSaiHashLagFieldSet(hashId);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not set XOR Hash(sample hash) Fields list\n");
                return saiStatus;
            }
        }
        else
        {
            XP_SAI_LOG_ERR("Could not set XOR Hash(sample hash) Fields list for non-default hash\n")
            return SAI_STATUS_NOT_SUPPORTED;
        }
    }

    if (setCRC)
    {
        saiStatus = xpSaiHashCrcFieldSet(hashType, hashId);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not set Hash Crc Fields list\n");
            return saiStatus;
        }
    }

    return saiStatus;
}

/**
 * \brief SAI Hash field converting function
 *
 * This API is used as a method to convert SAI Hash fields to HashInfo struct
 *
 * \param [out] xpSaiHashDbEntry_t* pHashInfo
 * \param [in] const sai_attribute_value_t *fieldList
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiConvertNativeHashFields(xpSaiHashDbEntry_t* pHashInfo,
                                                 const sai_attribute_value_t *fieldList)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    if (pHashInfo == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(pHashInfo->nativeHashFields, 0, sizeof(pHashInfo->nativeHashFields));
    pHashInfo->hashFieldNumber = 0;

    for (uint32_t i = 0; i < fieldList->s32list.count; i++)
    {
        sai_native_hash_field_t hashFieldId = (sai_native_hash_field_t)
                                              fieldList->s32list.list[i];

        if ((hashFieldId == SAI_NATIVE_HASH_FIELD_INNER_SRC_IP) ||
            (hashFieldId == SAI_NATIVE_HASH_FIELD_INNER_DST_IP) ||
            (hashFieldId == SAI_NATIVE_HASH_FIELD_INNER_SRC_IPV4) ||
            (hashFieldId == SAI_NATIVE_HASH_FIELD_INNER_DST_IPV4) ||
            (hashFieldId == SAI_NATIVE_HASH_FIELD_INNER_SRC_IPV6) ||
            (hashFieldId == SAI_NATIVE_HASH_FIELD_INNER_DST_IPV6) ||
            (hashFieldId >= XP_SAI_HASH_NATIVE_FIELDS_COUNT))
        {
            /*these fields are invalid for both CRC and XOR, so we do not insert it to DB*/
            XP_SAI_LOG_WARNING("Warning: hashFieldId %d not supported\n", hashFieldId);
            continue;
        }

        pHashInfo->nativeHashFields[hashFieldId] = true;
        pHashInfo->hashFieldNumber ++;
    }

    return saiStatus;
}

/*field setting for sample hash*/
static sai_status_t xpSaiHashLagFieldSet(sai_object_id_t hashId)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS status = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpSaiHashDbEntry_t* pHashInfo = NULL;

    saiRetVal = xpSaiHashInfoGet(hashId, &pHashInfo);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find HASH info in DB\n");
        return saiRetVal;
    }

    /*check valid of nativeHashFields,for the xor algorithm, only supports (dmac,smac),(dip,sip),
    (dip,sip,l4dport,l4sport),(dmac,smac,dip,sip)，(dmac,smac,dip,sip,l4dport,l4sport ) these five combinations.*/

    if ((pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_SRC_IP] ^
         pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_DST_IP])
        ||(pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_L4_SRC_PORT] ^
           pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_L4_DST_PORT])
        ||(pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_SRC_MAC] ^
           pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_DST_MAC]))
    {
        XP_SAI_LOG_WARNING("check XOR hash fields failed, the src and dst must be set synchronously\n");
    }
    int invalidFieldCount = 0;
    for (int i = SAI_NATIVE_HASH_FIELD_SRC_IP; i < XP_SAI_HASH_NATIVE_FIELDS_COUNT;
         i++)
    {
        if ((i == SAI_NATIVE_HASH_FIELD_SRC_IP)||(i == SAI_NATIVE_HASH_FIELD_DST_IP)||
            (i == SAI_NATIVE_HASH_FIELD_L4_SRC_PORT)
            ||(i == SAI_NATIVE_HASH_FIELD_L4_DST_PORT)||
            (i == SAI_NATIVE_HASH_FIELD_SRC_MAC)||(i == SAI_NATIVE_HASH_FIELD_DST_MAC))
        {
            continue;
        }

        if (pHashInfo->nativeHashFields[i] == true)
        {
            invalidFieldCount++;
        }
    }
    if (invalidFieldCount != 0)
    {
        XP_SAI_LOG_WARNING("check XOR hash fields failed, please remove the invalid fields(only sip, dip, l4_sport, l4_dport, smac and dmac is valid)\n");
    }
    if ((pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_L4_SRC_PORT] == true) &&
        (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_SRC_IP] != true))
    {
        XP_SAI_LOG_WARNING("check XOR hash fields failed, L4_port should be set with IP together\n");
    }

    xpHashField*  fields;
    fields = (xpHashField*)xpMalloc(sizeof(xpHashField)*XP_NUM_HASH_FIELDS);
    if (!fields)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for Hash Fields\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(fields, 0, sizeof(xpHashField)*XP_NUM_HASH_FIELDS);

    uint8_t index = 0;

    if ((pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_SRC_IP] == true) &&
        (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_DST_IP] == true))
    {
        fields[index++] = XP_IPV4_SOURCE_IP_ADDRESS;
        fields[index++] = XP_IPV4_DESTINATION_IP_ADDRESS;
        fields[index++] = XP_IPV6_SPLIT_SOURCE_IP_ADDRESS;
        fields[index++] = XP_IPV6_SPLIT_DESTINATION_IP_ADDRESS;
        fields[index++] = XP_IPV6_FLOW_LABEL;

        /*L4_port required that IP be set*/
        if ((pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_L4_SRC_PORT] == true) &&
            (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_L4_DST_PORT] == true))
        {
            fields[index++] = XP_UDP_SOURCE_PORT;
            fields[index++] = XP_UDP_DESTINATION_PORT;
            fields[index++] = XP_TCP_SOURCE_PORT;
            fields[index++] = XP_TCP_DESTINATION_PORT;
        }
    }

    if ((pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_SRC_MAC] == true) &&
        (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_DST_MAC] == true))
    {
        fields[index++]= XP_ETHERNET_MAC_DA;
        fields[index++]= XP_ETHERNET_MAC_SA;
    }

    size_t size = index;

    status = xpsLagSetHashFields(xpsDevId, fields, size);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Setting Hash Fields failed");
        xpFree(fields);
        return  xpsStatus2SaiStatus(status);
    }

    xpFree(fields);
    return SAI_STATUS_SUCCESS;
}

/*field setting for crc hash*/
static sai_status_t xpSaiHashCrcFieldSet(uint32_t hashType,
                                         sai_object_id_t hash_id)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS status = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpSaiHashDbEntry_t* pHashInfo = NULL;
    bool symmetricHashEnable = false;
    bool defaultHashSetV4Tables = false;
    bool defaultHashSetV6Tables = false;

    /*SymmetricHashEnable based on current DB setting*/
    saiRetVal = xpSaiSwitchSymmetricHashStatusGet(&symmetricHashEnable);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("failed to get symmetricHashEnable\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiHashInfoGet(hash_id, &pHashInfo);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find HASH info in DB\n");
        return saiRetVal;
    }

    xpHashField*  fields;
    fields = (xpHashField*)xpMalloc(sizeof(xpHashField)*XP_NUM_HASH_FIELDS);
    if (!fields)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for Hash Fields\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(fields, 0, sizeof(xpHashField)*XP_NUM_HASH_FIELDS);
    uint8_t index = 0;

    if (hashType == XPSAI_L2_HASH_OBJECT_INDEX)
    {
        //tabel is XP_HASH_MASK_TABLE_ETHERNET (xpHashMaskTableIndex)
        //field is dMac, sMac, vlan id , inport, etherType

        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_DST_MAC] == true)
        {
            fields[index++] = XP_ETHERNET_MAC_DA;
        }
        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_SRC_MAC] == true)
        {
            fields[index++] = XP_ETHERNET_MAC_SA;
        }
        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_VLAN_ID] == true)
        {
            fields[index++] = XP_ETHERNET_S_TAG_VID;
        }
        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_IN_PORT] == true)
        {
            fields[index++] = XP_IN_PORT;
        }
        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_ETHERTYPE] == true)
        {
            fields[index++] = XP_ETHERNET_ETHERTYPE;
        }
        status = xpsLagSetHashFieldsWithHashId(xpsDevId, fields, index,
                                               XP_HASH_MASK_TABLE_ETHERNET, symmetricHashEnable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting Hash Mask Table fields failed");
            xpFree(fields);
            return  xpsStatus2SaiStatus(status);
        }
    }

    memset(fields, 0, sizeof(xpHashField)*XP_NUM_HASH_FIELDS);
    index = 0;

    if ((hashType == XPSAI_L2_HASH_OBJECT_INDEX) &&
        ((hash_id == xpSaiHashObj[XPSAI_IPV4_HASH_OBJECT_INDEX]) ||
         (SAI_NULL_OBJECT_ID == xpSaiHashObj[XPSAI_IPV4_HASH_OBJECT_INDEX])))
    {
        defaultHashSetV4Tables = true;
    }

    if ((hashType == XPSAI_L2_HASH_OBJECT_INDEX) &&
        ((hash_id == xpSaiHashObj[XPSAI_IPV6_HASH_OBJECT_INDEX]) ||
         (SAI_NULL_OBJECT_ID == xpSaiHashObj[XPSAI_IPV6_HASH_OBJECT_INDEX])))
    {
        defaultHashSetV6Tables = true;
    }

    if ((defaultHashSetV4Tables == true) ||
        (hashType == XPSAI_IPV4_HASH_OBJECT_INDEX))
    {
        //tabel is XP_HASH_MASK_TABLE_IPV4_TCP, XP_HASH_MASK_TABLE_IPV4_UDP, XP_HASH_MASK_TABLE_IPV4_FRAGMENT, XP_HASH_MASK_TABLE_IPV4_OTHER
        //field is dMac, sMac, vlan id , dip ,sip, dport, sport, ipProtocol, inport
        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_DST_MAC] == true)
        {
            fields[index++] = XP_ETHERNET_MAC_DA;
        }
        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_SRC_MAC] == true)
        {
            fields[index++] = XP_ETHERNET_MAC_SA;
        }
        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_VLAN_ID] == true)
        {
            fields[index++] = XP_ETHERNET_S_TAG_VID;
        }

        if ((pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_DST_IP] == true)
            ||(pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_DST_IPV4] == true))
        {
            fields[index++] = XP_IPV4_DESTINATION_IP_ADDRESS;
        }
        if ((pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_SRC_IP] == true)
            ||(pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_SRC_IPV4] == true))
        {
            fields[index++] = XP_IPV4_SOURCE_IP_ADDRESS;
        }

        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_L4_DST_PORT] == true)
        {
            fields[index++] = XP_TCP_DESTINATION_PORT;
            fields[index++] = XP_UDP_DESTINATION_PORT;
        }
        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_L4_SRC_PORT] == true)
        {
            fields[index++] = XP_TCP_SOURCE_PORT;
            fields[index++] = XP_UDP_SOURCE_PORT;
        }

        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_IN_PORT] == true)
        {
            fields[index++] = XP_IN_PORT;
        }
        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_IP_PROTOCOL] == true)
        {
            fields[index++] = XP_IPV4_PROTOCOL;
        }


        status = xpsLagSetHashFieldsWithHashId(xpsDevId, fields, index,
                                               XP_HASH_MASK_TABLE_IPV4_TCP, symmetricHashEnable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting Hash Mask Table fields failed");
            xpFree(fields);
            return  xpsStatus2SaiStatus(status);
        }
        status = xpsLagSetHashFieldsWithHashId(xpsDevId, fields, index,
                                               XP_HASH_MASK_TABLE_IPV4_UDP, symmetricHashEnable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting Hash Mask Table fields failed");
            xpFree(fields);
            return  xpsStatus2SaiStatus(status);
        }
        status = xpsLagSetHashFieldsWithHashId(xpsDevId, fields, index,
                                               XP_HASH_MASK_TABLE_IPV4_FRAGMENT, symmetricHashEnable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting Hash Mask Table fields failed");
            xpFree(fields);
            return  xpsStatus2SaiStatus(status);
        }
        status = xpsLagSetHashFieldsWithHashId(xpsDevId, fields, index,
                                               XP_HASH_MASK_TABLE_IPV4_OTHER, symmetricHashEnable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting Hash Mask Table fields failed");
            xpFree(fields);
            return  xpsStatus2SaiStatus(status);
        }
    }

    memset(fields, 0, sizeof(xpHashField)*XP_NUM_HASH_FIELDS);
    index = 0;
    if ((defaultHashSetV6Tables == true) ||
        (hashType == XPSAI_IPV6_HASH_OBJECT_INDEX))
    {
        //tabel is XP_HASH_MASK_TABLE_IPV6, XP_HASH_MASK_TABLE_IPV6_TCP, XP_HASH_MASK_TABLE_IPV6_UDP
        //field is dMac, sMac, vlan id , dip ,sip, dport, sport, ipProtocol, inport
        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_DST_MAC] == true)
        {
            fields[index++] = XP_ETHERNET_MAC_DA;
        }
        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_SRC_MAC] == true)
        {
            fields[index++] = XP_ETHERNET_MAC_SA;
        }
        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_VLAN_ID] == true)
        {
            fields[index++] = XP_ETHERNET_S_TAG_VID;
        }

        if ((pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_DST_IP] == true)
            ||(pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_DST_IPV6] == true))
        {
            fields[index++] = XP_IPV6_SPLIT_DESTINATION_IP_ADDRESS;
        }
        if ((pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_SRC_IP] == true)
            ||(pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_SRC_IPV6] == true))
        {
            fields[index++] = XP_IPV6_SPLIT_SOURCE_IP_ADDRESS;
        }

        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_L4_DST_PORT] == true)
        {
            fields[index++] = XP_TCP_DESTINATION_PORT;
            fields[index++] = XP_UDP_DESTINATION_PORT;
        }
        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_L4_SRC_PORT] == true)
        {
            fields[index++] = XP_TCP_SOURCE_PORT;
            fields[index++] = XP_UDP_SOURCE_PORT;
        }

        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_IN_PORT] == true)
        {
            fields[index++] = XP_IN_PORT;
        }

        if (pHashInfo->nativeHashFields[SAI_NATIVE_HASH_FIELD_IP_PROTOCOL] == true)
        {
            fields[index++] = XP_IPV6_NEXT_HEADER;
        }

        status = xpsLagSetHashFieldsWithHashId(xpsDevId, fields, index,
                                               XP_HASH_MASK_TABLE_IPV6, symmetricHashEnable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting Hash Mask Table fields failed");
            xpFree(fields);
            return  xpsStatus2SaiStatus(status);
        }
        status = xpsLagSetHashFieldsWithHashId(xpsDevId, fields, index,
                                               XP_HASH_MASK_TABLE_IPV6_TCP, symmetricHashEnable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting Hash Mask Table fields failed");
            xpFree(fields);
            return  xpsStatus2SaiStatus(status);
        }
        status = xpsLagSetHashFieldsWithHashId(xpsDevId, fields, index,
                                               XP_HASH_MASK_TABLE_IPV6_UDP, symmetricHashEnable);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting Hash Mask Table fields failed");
            xpFree(fields);
            return  xpsStatus2SaiStatus(status);
        }
    }
    xpFree(fields);
    return SAI_STATUS_SUCCESS;
}

/**
 *  xpSaiHashAttributeSet - sai_set_hash_attribute_fn
 */
sai_status_t xpSaiHashAttributeSet(sai_object_id_t hash_id,
                                   const sai_attribute_t *attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiHashDbEntry_t* pHashInfo = NULL;
    xpSaiHashDbEntry_t tmpHashInfo;
    uint8_t hashType = XPSAI_MAX_HASH_OBJECT_INDEX;
    bool needSetXORField = false;
    bool needSetCRCField = false;
    XP_STATUS status = XP_NO_ERR;
    xpsDevice_t devId =  xpSaiGetDevId();

    XP_SAI_LOG_DBG("Calling xpSaiHashAttributeSet\n");

    saiRetVal = xpSaiAttrCheck(1, attr,
                               HASH_VALIDATION_ARRAY_SIZE, hash_attribs,
                               SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d \n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiHashInfoGet(hash_id, &pHashInfo);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find HASH info in DB\n");
        return saiRetVal;
    }

    memcpy(&tmpHashInfo, pHashInfo, sizeof(xpSaiHashDbEntry_t));

    xpsLagHashAlgoType_e xpsHashAlgo;
    status = xpsLagGetHashAlgo(devId, &xpsHashAlgo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsLagGetHashAlgo failed devId %d \n", devId);
        return xpsStatus2SaiStatus(status);
    }

    switch (attr->id)
    {
        case SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST:
            {
                saiRetVal = xpSaiConvertNativeHashFields(pHashInfo, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("HASH field convert error\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_HASH_ATTR_UDF_GROUP_LIST:
            {
                return SAI_STATUS_ATTR_NOT_SUPPORTED_0;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown attribute %d\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }

    if (hash_id == xpSaiHashObj[XPSAI_L2_HASH_OBJECT_INDEX])
    {
        /*if the algo is XOR, the sampleHash fields of default hash can be set*/
        if (xpsHashAlgo == XPS_LAG_HASH_ALGO_XOR_E)
        {
            hashType = XPSAI_L2_HASH_OBJECT_INDEX;
            needSetXORField = true;
        }
        /*if the algo is CRC, the hashMasktables of default hash can be set*/
        else if (xpsHashAlgo == XPS_LAG_HASH_ALGO_CRC_E)
        {
            hashType = XPSAI_L2_HASH_OBJECT_INDEX;
            needSetCRCField = true;
        }
    }
    /*whether the algo is XOR or CRC, the hashMasktables of V4hash and V6hash can be set*/
    else if (hash_id == xpSaiHashObj[XPSAI_IPV4_HASH_OBJECT_INDEX])
    {
        hashType = XPSAI_IPV4_HASH_OBJECT_INDEX;
        needSetCRCField = true;
    }
    else if (hash_id == xpSaiHashObj[XPSAI_IPV6_HASH_OBJECT_INDEX])
    {
        hashType = XPSAI_IPV6_HASH_OBJECT_INDEX;
        needSetCRCField = true;
    }

    if (needSetXORField || needSetCRCField)
    {
        saiRetVal = xpSaiHashFieldSet(hashType, hash_id, needSetXORField,
                                      needSetCRCField);
        if (saiRetVal!= SAI_STATUS_SUCCESS)
        {
            /*the hashInfo should change to the original value if failed*/
            memcpy(pHashInfo, &tmpHashInfo, sizeof(xpSaiHashDbEntry_t));
            XP_SAI_LOG_ERR("Failed to Set HasField for SAI HASH\n");
            return saiRetVal;
        }
    }

    return saiRetVal;
}


/**
 * \brief SAI Hash field list getting function
 *
 * This API is used as a method to get a list of SAI Hash fields
 *
 * \param [in] sai_object_id_t hash_id
 * \param [in] sai_attribute_t *fieldList
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiHashFieldListGet(sai_object_id_t hash_id,
                                          sai_attribute_value_t *fieldList)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS status = XP_NO_ERR;
    xpsDevice_t devId =  xpSaiGetDevId();
    xpSaiHashDbEntry_t* pHashInfo = NULL;
    uint32_t index = 0;

    xpsLagHashAlgoType_e xpsHashAlgo;
    status =  xpsLagGetHashAlgo(devId, &xpsHashAlgo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsLagGetHashAlgo failed devId %d \n", devId);
        return xpsStatus2SaiStatus(status);
    }

    if ((hash_id == xpSaiHashObj[XPSAI_L2_HASH_OBJECT_INDEX]) &&
        (xpsHashAlgo == XPS_LAG_HASH_ALGO_XOR_E))
    {
        /*get simple hash field only if (algo==XOR)&&(hashId==defaultHash) */
        saiStatus = xpSaiHashLagFieldGet(fieldList);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not get Lag Hash Fields list\n");
            return saiStatus;
        }
    }
    else
    {
        /*get hash mask table field*/
        saiStatus = xpSaiHashInfoGet(hash_id, &pHashInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not find HASH info in DB\n");
            return saiStatus;
        }

        for (uint8_t i = 0; i < XP_SAI_HASH_NATIVE_FIELDS_COUNT; i++)
        {
            if (fieldList->s32list.count < index)
            {
                return SAI_STATUS_BUFFER_OVERFLOW;
            }

            if (pHashInfo->nativeHashFields[i] == true)
            {
                fieldList->s32list.list[index++] = i;
            }
        }
        fieldList->s32list.count = index;
    }

    return saiStatus;
}

static sai_status_t xpSaiHashLagFieldGet(sai_attribute_value_t *fieldList)
{
    uint32_t j = 0;
    XP_STATUS status = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpHashField* fields;
    uint32_t size;

    fields = (xpHashField*)xpMalloc(sizeof(xpHashField)*XP_NUM_HASH_FIELDS);
    if (!fields)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for Hash Fields\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    memset(fields, 0, sizeof(xpHashField)*XP_NUM_HASH_FIELDS);

    status = xpsLagGetHashFields(xpsDevId, fields, &size);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting Hash Fields failed");
        xpFree(fields);
        return  xpsStatus2SaiStatus(status);

    }

    bool lagHashFieldArray[XP_NUM_HASH_FIELDS] = {false};
    for (uint8_t i = 0; i<(uint8_t)size; i++)
    {
        lagHashFieldArray[fields[i]] = true;
    }

    if ((lagHashFieldArray[XP_ETHERNET_MAC_DA]==true)&&
        (lagHashFieldArray[XP_ETHERNET_MAC_SA]==true))

    {
        if (fieldList->s32list.count < j+1)
        {
            xpFree(fields);
            return SAI_STATUS_BUFFER_OVERFLOW;
        }
        fieldList->s32list.list[j++] = SAI_NATIVE_HASH_FIELD_SRC_MAC;
        fieldList->s32list.list[j++] = SAI_NATIVE_HASH_FIELD_DST_MAC;
    }

    if (((lagHashFieldArray[XP_IPV4_SOURCE_IP_ADDRESS]==true)&&
         (lagHashFieldArray[XP_IPV4_DESTINATION_IP_ADDRESS]==true)) ||
        ((lagHashFieldArray[XP_IPV6_FLOW_LABEL]==true)&&
         (lagHashFieldArray[XP_IPV6_SPLIT_SOURCE_IP_ADDRESS]==true)&&
         (lagHashFieldArray[XP_IPV6_SPLIT_DESTINATION_IP_ADDRESS]==true)))
    {

        if ((lagHashFieldArray[XP_UDP_SOURCE_PORT]==true)&&
            (lagHashFieldArray[XP_UDP_DESTINATION_PORT]==true)&&
            (lagHashFieldArray[XP_TCP_SOURCE_PORT]==true)&&
            (lagHashFieldArray[XP_TCP_DESTINATION_PORT]==true))
        {
            if (fieldList->s32list.count < j+3)
            {
                xpFree(fields);
                return SAI_STATUS_BUFFER_OVERFLOW;
            }
            fieldList->s32list.list[j++] = SAI_NATIVE_HASH_FIELD_SRC_IP;
            fieldList->s32list.list[j++] = SAI_NATIVE_HASH_FIELD_DST_IP;
            fieldList->s32list.list[j++] = SAI_NATIVE_HASH_FIELD_L4_SRC_PORT;
            fieldList->s32list.list[j++] = SAI_NATIVE_HASH_FIELD_L4_DST_PORT;

        }
        else
        {
            if (fieldList->s32list.count < j+1)
            {
                xpFree(fields);
                return SAI_STATUS_BUFFER_OVERFLOW;
            }
            fieldList->s32list.list[j++] = SAI_NATIVE_HASH_FIELD_SRC_IP;
            fieldList->s32list.list[j++] = SAI_NATIVE_HASH_FIELD_DST_IP;
        }
    }

    if (!j)
    {
        XP_SAI_LOG_ERR("Could not get HASH field\n");
        xpFree(fields);
        return SAI_STATUS_FAILURE;
    }

    fieldList->s32list.count = j;
    xpFree(fields);

    return  SAI_STATUS_SUCCESS;
}
/**
 * \brief SAI Hash Attribute getting function
 *
 * This API is used as a method to get SAI Hash Attribute
 *
 * \param [in] sai_object_id_t hash_id
 * \param [in] sai_attribute_t *attr
 * \param [in] uint32_t attr_index
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetHashAttribute(sai_object_id_t hash_id,
                                   sai_attribute_t *attr, uint32_t attr_index)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiGetHashAttribute\n");

    switch (attr->id)
    {
        case SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST:
            {
                saiRetVal = xpSaiHashFieldListGet(hash_id, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Could not get HASH field\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_HASH_ATTR_UDF_GROUP_LIST:
            {
                return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(attr_index);
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown attribute %d\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index);
            }
    }

    return SAI_STATUS_SUCCESS;
}

/**
 *  xpSaiHashAttributeListGet - implements sai_get_hash_attribute_fn
 */
sai_status_t xpSaiHashAttributeListGet(sai_object_id_t hash_id,
                                       uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiHashAttributeListGet\n");

    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetHashAttribute(hash_id, &attr_list[count], count);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not get HASH field Attribute\n");
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief SAI Hash default object creating function
 *
 * This API is used as a method to create SAI Hash default object
 * with default attributes.
 *
 * Default value after switch initialization:
 *
 * #SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST = \[#SAI_NATIVE_HASH_FIELD_SRC_MAC,
 * #SAI_NATIVE_HASH_FIELD_DST_MAC, #SAI_NATIVE_HASH_FIELD_IN_PORT,
 * #SAI_NATIVE_HASH_FIELD_ETHERTYPE\]
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiHashDefaultObjCreate(void)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    sai_object_id_t hash_oid = SAI_NULL_OBJECT_ID;
    sai_attribute_t hash_attr_list[1];
    sai_int32_t default_hash_fields[] = {SAI_NATIVE_HASH_FIELD_SRC_MAC, SAI_NATIVE_HASH_FIELD_DST_MAC, \
                                         SAI_NATIVE_HASH_FIELD_IN_PORT, SAI_NATIVE_HASH_FIELD_ETHERTYPE
                                        };

    memset(&hash_attr_list, 0, sizeof(hash_attr_list));
    hash_attr_list[0].id = SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST;
    hash_attr_list[0].value.s32list.count = sizeof(default_hash_fields)/sizeof(
                                                sai_int32_t);
    hash_attr_list[0].value.s32list.list = default_hash_fields;

    saiRetVal = xpSaiHashCreate(&hash_oid, 0, 1, hash_attr_list);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not create default HASH. saiRetVal = %d\n", saiRetVal);
        return saiRetVal;
    }

    //Set Attributes List
    saiRetVal = xpSaiHashFieldSet(XPSAI_L2_HASH_OBJECT_INDEX, hash_oid, false,
                                  true);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to Set HasField for SAI HASH\n");
        return saiRetVal;
    }

    xpSaiHashObj[XPSAI_L2_HASH_OBJECT_INDEX] = hash_oid;

    return SAI_STATUS_SUCCESS;
}


/**
 * \brief SAI Hash default object removing function
 *
 *  This API is used as a method to remove SAI Hash default object
 *  and revert all what were done in default object creating function.
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiHashDefaultObjRemove(void)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    for (uint8_t i = XPSAI_L2_HASH_OBJECT_INDEX; i < XPSAI_MAX_HASH_OBJECT_INDEX ;
         i++)
    {
        if (xpSaiHashObj[i] == SAI_NULL_OBJECT_ID)
        {
            continue;
        }
        saiRetVal = xpSaiHashObjRemove(xpSaiHashObj[i]);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not Could not remove default HASH. saiRetVal = %d\n",
                           saiRetVal);
            return saiRetVal;
        }

        saiRetVal = xpSaiSwitchDefaultHashSet(SAI_NULL_OBJECT_ID);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not set to switch default HASH. saiRetVal = %d\n",
                           saiRetVal);
            return saiRetVal;
        }
        xpSaiHashObj[i] = SAI_NULL_OBJECT_ID;
    }
    return SAI_STATUS_SUCCESS;
}


/**
 * \brief SAI Hash fields to XP Hash fields converting function
 *
 * This API is used as a method to convert SAI Hash fields to XP Hash fields
 *
 * \param [in] const sai_int32_t *saiFields
 * \param [out] xpHashField *xpsFieldList
 * \param [inout] uint32_t *count
 *
 * \return sai_status_t
 */
sai_status_t xpSaiHashFieldsToXpHashFieldsConvert(const sai_int32_t *saiFields,
                                                  uint32_t count, xpSaiHwHashState_t *state, sai_object_id_t hashId)
{
    XP_SAI_LOG_DBG("xpSaiHashFieldsToXpHashFieldsConvert()\n");
    uint8_t j=0;

    for (uint32_t i = 0; i < count; i++)
    {
        switch (saiFields[i])
        {
            case SAI_NATIVE_HASH_FIELD_SRC_IP:
                {
                    if (xpSaiHashObj[XPSAI_IPV4_HASH_OBJECT_INDEX] == hashId)
                    {
                        for (j=0; j < state[IPv4].fieldsNumber; j++)
                        {
                            if (state[IPv4].fieldList[j] == XP_IPV4_SOURCE_IP_ADDRESS)
                            {
                                break;
                            }
                        }
                        if (state[IPv4].fieldsNumber == j)
                        {
                            state[IPv4].fieldList[state[IPv4].fieldsNumber] = XP_IPV4_SOURCE_IP_ADDRESS;
                            state[IPv4].fieldsNumber++;
                            state[IPv4].enable = true;
                        }
                    }

                    if (xpSaiHashObj[XPSAI_IPV6_HASH_OBJECT_INDEX] == hashId)
                    {
                        for (j=0; j < state[IPv6_SPLIT].fieldsNumber; j++)
                            if (state[IPv6_SPLIT].fieldList[j] == XP_IPV6_SPLIT_SOURCE_IP_ADDRESS)
                            {
                                break;
                            }
                        if (state[IPv6_SPLIT].fieldsNumber == j)
                        {
                            state[IPv6_SPLIT].fieldList[state[IPv6_SPLIT].fieldsNumber] =
                                XP_IPV6_SPLIT_SOURCE_IP_ADDRESS;
                            state[IPv6_SPLIT].fieldsNumber++;
                            state[IPv6_SPLIT].enable = true;
                        }
                    }
                    break;
                }
            case SAI_NATIVE_HASH_FIELD_DST_IP:
                {
                    if (xpSaiHashObj[XPSAI_IPV4_HASH_OBJECT_INDEX] == hashId)
                    {
                        for (j=0; j < state[IPv4].fieldsNumber; j++)
                            if (state[IPv4].fieldList[j] == XP_IPV4_DESTINATION_IP_ADDRESS)
                            {
                                break;
                            }
                        if (state[IPv4].fieldsNumber == j)
                        {
                            state[IPv4].fieldList[state[IPv4].fieldsNumber] =
                                XP_IPV4_DESTINATION_IP_ADDRESS;
                            state[IPv4].fieldsNumber++;
                            state[IPv4].enable = true;
                        }
                    }

                    if (xpSaiHashObj[XPSAI_IPV6_HASH_OBJECT_INDEX] == hashId)
                    {
                        for (j=0; j < state[IPv6_SPLIT].fieldsNumber; j++)
                            if (state[IPv6_SPLIT].fieldList[j] == XP_IPV6_SPLIT_DESTINATION_IP_ADDRESS)
                            {
                                break;
                            }
                        if (state[IPv6_SPLIT].fieldsNumber == j)
                        {
                            state[IPv6_SPLIT].fieldList[state[IPv6_SPLIT].fieldsNumber] =
                                XP_IPV6_SPLIT_DESTINATION_IP_ADDRESS;
                            state[IPv6_SPLIT].fieldsNumber++;
                            state[IPv6_SPLIT].enable = true;
                        }
                    }
                    break;
                }
            case SAI_NATIVE_HASH_FIELD_INNER_SRC_IP:
                {
                    XP_SAI_LOG_ERR("Hash field %d is no supported.\n", saiFields[i]);
                    return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(i);
                }
            case SAI_NATIVE_HASH_FIELD_INNER_DST_IP:
                {
                    XP_SAI_LOG_ERR("Hash field %d is no supported.\n", saiFields[i]);
                    return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(i);
                }
            case SAI_NATIVE_HASH_FIELD_VLAN_ID:
                {
                    for (j=0; j < state[ETHERNET].fieldsNumber; j++)
                        if (state[ETHERNET].fieldList[j] == XP_ETHERNET_C_TAG_VID)
                        {
                            break;
                        }
                    if (state[ETHERNET].fieldsNumber == j)
                    {
                        state[ETHERNET].fieldList[state[ETHERNET].fieldsNumber] = XP_ETHERNET_C_TAG_VID;
                        state[ETHERNET].fieldsNumber++;
                        state[ETHERNET].enable = true;
                    }
                    break;
                }
            case SAI_NATIVE_HASH_FIELD_IP_PROTOCOL:
                {
                    for (j=0; j < state[IPv4].fieldsNumber; j++)
                        if (state[IPv4].fieldList[j] == XP_IPV4_PROTOCOL)
                        {
                            break;
                        }
                    if (state[IPv4].fieldsNumber == j)
                    {
                        state[IPv4].fieldList[state[IPv4].fieldsNumber] = XP_IPV4_PROTOCOL;
                        state[IPv4].fieldsNumber++;
                        state[IPv4].enable = true;
                    }
                    break;
                }
            case SAI_NATIVE_HASH_FIELD_ETHERTYPE:
                {
                    for (j=0; j < state[ETHERNET].fieldsNumber; j++)
                        if (state[ETHERNET].fieldList[j] == XP_ETHERNET_ETHERTYPE)
                        {
                            break;
                        }
                    if (state[ETHERNET].fieldsNumber == j)
                    {
                        state[ETHERNET].fieldList[state[ETHERNET].fieldsNumber] = XP_ETHERNET_ETHERTYPE;
                        state[ETHERNET].fieldsNumber++;
                        state[ETHERNET].enable = true;
                    }
                    break;
                }
            case SAI_NATIVE_HASH_FIELD_L4_SRC_PORT:
                {
                    for (j=0; j < state[TCP].fieldsNumber; j++)
                        if (state[TCP].fieldList[j] == XP_TCP_SOURCE_PORT)
                        {
                            break;
                        }
                    if (state[TCP].fieldsNumber == j)
                    {
                        state[TCP].fieldList[state[TCP].fieldsNumber] = XP_TCP_SOURCE_PORT;
                        state[TCP].fieldsNumber++;
                        state[TCP].enable = true;

                        state[UDP].fieldList[state[UDP].fieldsNumber] = XP_UDP_SOURCE_PORT;
                        state[UDP].fieldsNumber++;
                        state[UDP].enable = true;
                    }
                    break;
                }
            case SAI_NATIVE_HASH_FIELD_L4_DST_PORT:
                {
                    for (j=0; j < state[TCP].fieldsNumber; j++)
                        if (state[TCP].fieldList[j] == XP_TCP_DESTINATION_PORT)
                        {
                            break;
                        }
                    if (state[TCP].fieldsNumber == j)
                    {
                        state[TCP].fieldList[state[TCP].fieldsNumber] = XP_TCP_DESTINATION_PORT;
                        state[TCP].fieldsNumber++;
                        state[TCP].enable = true;

                        state[UDP].fieldList[state[UDP].fieldsNumber] = XP_UDP_DESTINATION_PORT;
                        state[UDP].fieldsNumber++;
                        state[UDP].enable = true;
                    }
                    break;
                }
            case SAI_NATIVE_HASH_FIELD_SRC_MAC:
                {
                    for (j=0; j < state[ETHERNET].fieldsNumber; j++)
                        if (state[ETHERNET].fieldList[j] == XP_ETHERNET_MAC_SA)
                        {
                            break;
                        }
                    if (state[ETHERNET].fieldsNumber == j)
                    {
                        state[ETHERNET].fieldList[state[ETHERNET].fieldsNumber] = XP_ETHERNET_MAC_SA;
                        state[ETHERNET].fieldsNumber++;
                        state[ETHERNET].enable = true;
                    }
                    break;
                }
            case SAI_NATIVE_HASH_FIELD_DST_MAC:
                {
                    for (j=0; j < state[ETHERNET].fieldsNumber; j++)
                        if (state[ETHERNET].fieldList[j] == XP_ETHERNET_MAC_DA)
                        {
                            break;
                        }
                    if (state[ETHERNET].fieldsNumber == j)
                    {
                        state[ETHERNET].fieldList[state[ETHERNET].fieldsNumber] = XP_ETHERNET_MAC_DA;
                        state[ETHERNET].fieldsNumber++;
                        state[ETHERNET].enable = true;
                    }
                    break;
                }
            case SAI_NATIVE_HASH_FIELD_IN_PORT:
                {
                    XP_SAI_LOG_ERR("Hash field %d is no supported.\n", saiFields[i]);
                    return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(i);
                }
            default:
                {
                    XP_SAI_LOG_ERR("Unknown field %d\n", saiFields[i]);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
    }

    return SAI_STATUS_SUCCESS;
}


/**
 * \brief SAI switch setting IPV4Hash or IPV6Hash function
 *
 * This API is used as a method to set switch attr of IPV4 hash or IPV6 hash
 *
 * \param [in] uint32_t hashType
 * \param [in] sai_object_id_t hash_oid
 *
 * \return sai_status_t
 */
sai_status_t xpSaiHashSetSwitchHash(uint32_t hashType, sai_object_id_t hash_oid)
{
    sai_status_t    saiRetVal        = SAI_STATUS_SUCCESS;
    if ((hashType != XPSAI_IPV4_HASH_OBJECT_INDEX) &&
        (hashType != XPSAI_IPV6_HASH_OBJECT_INDEX))
    {
        XP_SAI_LOG_ERR("hashType %d can not set to switch hash\n", hashType);
    }

    if (((hash_oid == xpSaiHashObj[XPSAI_IPV4_HASH_OBJECT_INDEX]) &&
         (XPSAI_IPV4_HASH_OBJECT_INDEX == hashType))
        || ((hash_oid == xpSaiHashObj[XPSAI_IPV6_HASH_OBJECT_INDEX]) &&
            (XPSAI_IPV6_HASH_OBJECT_INDEX == hashType)))
    {
        XP_SAI_LOG_DBG("new hash object is same as current hash object, hashType:%d\n",
                       hashType);
        return SAI_STATUS_SUCCESS;
    }
    //Set Attributes List

    if (hash_oid != SAI_NULL_OBJECT_ID)
    {
        saiRetVal = xpSaiHashFieldSet(hashType, hash_oid, false, true);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to Set HasField for SAI HASH\n");
            return saiRetVal;
        }
    }
    else
    {
        saiRetVal = xpSaiHashFieldSet(hashType,
                                      xpSaiHashObj[XPSAI_L2_HASH_OBJECT_INDEX], false, true);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to Set default hash for SAI HASH\n");
            return saiRetVal;
        }
    }
    xpSaiHashObj[hashType] = hash_oid;
    return SAI_STATUS_SUCCESS;
}


/**
 * \brief SAI crc hash mask table rewriting function
 *
 * This API is used rewrite all crc hash mask table, based on current DB setting.
 *
 *
 * \return sai_status_t
 */
sai_status_t xpSaiHashReconfigueHashMaskTables(void)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    if (xpSaiHashObj[XPSAI_L2_HASH_OBJECT_INDEX] != SAI_NULL_OBJECT_ID)
    {
        saiStatus = xpSaiHashCrcFieldSet(XPSAI_L2_HASH_OBJECT_INDEX,
                                         xpSaiHashObj[XPSAI_L2_HASH_OBJECT_INDEX]);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not set Hash Crc Fields list for default hash\n");
            return saiStatus;
        }
    }
    if (xpSaiHashObj[XPSAI_IPV4_HASH_OBJECT_INDEX] != SAI_NULL_OBJECT_ID)
    {
        saiStatus = xpSaiHashCrcFieldSet(XPSAI_IPV4_HASH_OBJECT_INDEX,
                                         xpSaiHashObj[XPSAI_IPV4_HASH_OBJECT_INDEX]);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not set Hash Crc Fields list for IPV4 hash\n");
            return saiStatus;
        }
    }
    if (xpSaiHashObj[XPSAI_IPV6_HASH_OBJECT_INDEX] != SAI_NULL_OBJECT_ID)
    {
        saiStatus = xpSaiHashCrcFieldSet(XPSAI_IPV6_HASH_OBJECT_INDEX,
                                         xpSaiHashObj[XPSAI_IPV6_HASH_OBJECT_INDEX]);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not set Hash Crc Fields list for IPV6 hash\n");
            return saiStatus;
        }
    }

    return saiStatus;
}

sai_status_t xpSaiHashInit(xpsDevice_t xpsDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    retVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_HASH,
                                         XP_SAI_HASH_MAX_IDS, XP_SAI_HASH_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI HASH ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }

    sXpSaiHashDbHndl = XPSAI_HASH_DB_HNDL;
    retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "xpSaiHashDb state", XPS_GLOBAL,
                                &saiHashKeyComp, sXpSaiHashDbHndl);
    if (retVal != XP_NO_ERR)
    {
        sXpSaiHashDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Failed to register SAI HASH DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    for (int i = 0; i < NUM_LAYERS; i++)
    {
        retVal = xpsGlobalSwitchControlDisableLayerHash(xpsDevId, (xpLayerType_t) i);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_DBG("Failed execute to xpsGlobalSwitchControlDisableLayerHash()\n");
            return xpsStatus2SaiStatus(retVal);
        }
    }

    saiRetVal = xpSaiHashDefaultObjCreate();
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not create default HASH.\n");
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiHashDeInit(xpsDevice_t devId)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    saiRetVal = xpSaiHashDefaultObjRemove();
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not remove default HASH.\n");
        return saiRetVal;
    }

    retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &sXpSaiHashDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to De-register SAI HASH INFO DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

XP_STATUS xpSaiHashApiInit(uint64_t flag,
                           const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiHashApiInit\n");

    _xpSaiHashApi = (sai_hash_api_t *)xpMalloc(sizeof(sai_hash_api_t));
    if (NULL == _xpSaiHashApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiHashApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiHashApi->create_hash = xpSaiHashCreate;
    _xpSaiHashApi->remove_hash = xpSaiHashRemove;
    _xpSaiHashApi->set_hash_attribute = xpSaiHashAttributeSet;
    _xpSaiHashApi->get_hash_attribute = xpSaiHashAttributeListGet;

    saiRetVal = xpSaiApiRegister(SAI_API_HASH, (void*)_xpSaiHashApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register SAI HASH API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return  retVal;
}

XP_STATUS xpSaiHashApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiHashApiDeinit\n");

    xpFree(_xpSaiHashApi);
    _xpSaiHashApi = NULL;

    return  retVal;
}
