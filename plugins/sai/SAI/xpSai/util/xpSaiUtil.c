// xpSaiUtil.c

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#include <execinfo.h>
#include <signal.h>
#include "xpSaiUtil.h"
#include "xpSaiDev.h"
#include "xpSai.h"

#include <unistd.h>
#include <poll.h>
#include "xpSaiValidationArrays.h"

#define SAI_MAX_PROFILE_VALUE_LEN 11
bool gResetInProgress = 0;
static xpSaiAppSaiKeyValue_t
xpSaiAppSaiProfileSet[XPS_APP_SAI_PROFILE_NUMBER][XPS_APP_SAI_PROFILE_ARRAY_SIZE]
    = {};
XP_SAI_LOG_REGISTER_API(SAI_API_UNSPECIFIED);


bool xpSaiUtilIsMulticastMac(const sai_mac_t mac)
{
    return !xpSaiUtilIsBroadcastMac(mac) && (mac[0] & 0x01);
}

bool xpSaiUtilIsUnicastMac(const sai_mac_t mac)
{
    return !xpSaiUtilIsBroadcastMac(mac) && !xpSaiUtilIsMulticastMac(mac);
}

bool xpSaiUtilIsBroadcastMac(const sai_mac_t mac)
{
    return (mac[0] & mac[1] & mac[2] & mac[3] & mac[4] & mac[5]) == 0xFF;
}

XP_STATUS xpSaiAppSaiProfileKeyValueAdd(sai_switch_profile_id_t profile_id,
                                        const char* key, char* value)
{
    int i        = 0;
    int keyLen   = 0;
    int valueLen = 0;
    static int keyIdx = 0;

    if (profile_id >= XPS_APP_SAI_PROFILE_NUMBER)
    {
        XP_SAI_LOG_ERR("Invalid profile id %d.\n", profile_id);
        return XP_ERR_INVALID_PARAMS;
    }

    xpSaiAppSaiKeyValue_t* xpSaiAppSaiProfileArray =
        xpSaiAppSaiProfileSet[profile_id];

    if (keyIdx < XPS_APP_SAI_PROFILE_ARRAY_SIZE)
    {
        i = keyIdx;
        if (xpSaiAppSaiProfileArray[i].key == NULL)
        {
            keyLen = strlen(key) + 1;
            valueLen = strlen(value) + 1;

            XP_SAI_LOG_NOTICE("Adding Profile Config [%d] key '%s' value '%s'.\n", keyIdx,
                              key, value);

            xpSaiAppSaiProfileArray[i].key = (char*)xpMalloc(keyLen);
            xpSaiAppSaiProfileArray[i].value = (char*)xpMalloc(valueLen);

            memset(xpSaiAppSaiProfileArray[i].key, 0, keyLen);
            memset(xpSaiAppSaiProfileArray[i].value, 0, valueLen);

            strncpy(xpSaiAppSaiProfileArray[i].key, key, keyLen);
            strncpy(xpSaiAppSaiProfileArray[i].value, value, valueLen);

            keyIdx++;

            return XP_NO_ERR;
        }
    }
    else
    {
        XP_SAI_LOG_ERR("Cannot Add Profile config [%d] %s:%s, max config allowed is %d\n",
                       keyIdx, key, value, XPS_APP_SAI_PROFILE_ARRAY_SIZE);
    }


    return XP_ERR_OUT_OF_MEM;
}

sai_status_t xpSaiAppSaiProfileKeyIntValueAdd(sai_switch_profile_id_t
                                              profile_id, const char* key, uint32_t value)
{
    XP_STATUS retVal = XP_NO_ERR;

    char strValue[SAI_MAX_PROFILE_VALUE_LEN];

    memset(&strValue, 0x00, sizeof(strValue));

    snprintf(strValue, sizeof(strValue), "%d", value);

    retVal = xpSaiAppSaiProfileKeyValueAdd(profile_id, key, strValue);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to add profile id %d.\n", profile_id);
    }
    return xpsStatus2SaiStatus(retVal);
}

const char* xpSaiAppSaiProfileGetValue(sai_switch_profile_id_t profile_id,
                                       const char* key)
{
    int                  i                     = 0;
    xpSaiAppSaiKeyValue_t* xpSaiAppSaiProfileArray = NULL;

    if (profile_id >= XPS_APP_SAI_PROFILE_NUMBER)
    {
        XP_SAI_LOG_ERR("Invalid profile id %d.\n", profile_id);
        return NULL;
    }

    xpSaiAppSaiProfileArray = xpSaiAppSaiProfileSet[profile_id];

    for (i = 0; i < XPS_APP_SAI_PROFILE_ARRAY_SIZE; i++)
    {
        if (xpSaiAppSaiProfileArray[i].key != NULL)
        {
            if (strncmp(xpSaiAppSaiProfileArray[i].key, key,
                        XPS_APP_SAI_PROFILE_VALUE_STR_SIZE) == 0)
            {
                return xpSaiAppSaiProfileArray[i].value;
            }
        }
    }

    return NULL;
}

int xpSaiAppSaiProfileGetNextValue(sai_switch_profile_id_t profile_id,
                                   const char** variable, const char** value)
{
    /* TBD */
    return 0;
}

XP_STATUS xpSaiProfileConfigLoadFromFile(sai_switch_profile_id_t profile_id,
                                         const char *fileName)
{
    XP_STATUS retVal  = XP_NO_ERR;
    FILE*     cfgFile = NULL;

    if (NULL == fileName)
    {
        XP_SAI_LOG_ERR("Configuration file name not defined\n");
        return XP_ERR_INVALID_ARG;
    }

    cfgFile = fopen(fileName, "r");
    if (NULL == cfgFile)
    {
        XP_SAI_LOG_ERR("Can't open SAI config file %s\n", fileName);
        return XP_ERR_FILE_OPEN;
    }

    while (0 == feof(cfgFile))  // While not at the end of the file:
    {
        char line[XP_MAX_CONFIG_LINE_LEN];

        if (fgets(line, XP_MAX_CONFIG_LINE_LEN, cfgFile) != NULL)
        {
            char *key = NULL;
            char *val = NULL;

            if (('#' == line[0]) || (';' == line[0]))
            {
                continue;
            }

            key = strtok(line, "=");
            val = strtok(NULL, "=");

            if ((NULL == key) || (NULL == val))
            {
                continue;
            }

            retVal = xpSaiAppSaiProfileKeyValueAdd(profile_id, key, val);
            if (retVal != XP_NO_ERR)
            {
                break;
            }
        }
    }

    fclose(cfgFile);

    return retVal;
}

const char* xpSaiProfileKeyValueGet(sai_switch_profile_id_t profile_id,
                                    const char* key)
{
    return xpSaiAppSaiProfileGetValue(profile_id, key);
}

/**
 * \brief Find attribute index
 *
 * \param id                - sai attribute id to search
 * \param value             - sai attribute value to search
 * \param object_attr_count - size of all sai feature attributes
 * \param object_attr_list  - list of sai feature attributes and their configuration
 * \param(out) index        - index of found attribute in a list
 *
 * \return sai_status_t
 */
static sai_status_t xpSaiAttrIndexFind(sai_attr_id_t id,
                                       sai_attribute_value_t value, uint32_t object_attr_count,
                                       const xpSaiAttrEntry_t *object_attr_list, sai_common_api_t oper,
                                       uint32_t *index)
{
    bool idMatch = false;

    if (NULL == object_attr_list)
    {
        XP_SAI_LOG_ERR("NULL value object attribute\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == index)
    {
        XP_SAI_LOG_ERR("NULL value index\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t i = 0; i < object_attr_count; i++)
    {
        if (id == object_attr_list[i].id)
        {
            idMatch = true;
            if ((SAI_COMMON_API_GET == oper) ||
                ((object_attr_list[i].attrValue == XP_SAI_ATTR_ANY) ||
                 (object_attr_list[i].attrValue == value.s32)))
            {
                *index = i;
                return SAI_STATUS_SUCCESS;
            }
        }
    }

    if (idMatch)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
}

/**
 * \brief Verify SAI attributes prerequisites
 *
 * \param object_attr_list  - list of sai feature attributes and their configuration
 * \param object_attr_count - size of all sai feature attributes
 * \param attr_present      - list of present attributes
 *
 * \return bool
 */
static bool xpSaiAttrCheckConditionsSucceed(const xpSaiAttrEntry_t
                                            *object_attr_list,
                                            uint32_t object_attr_count, uint32_t attr_id,
                                            bool *attr_present)
{
    sai_attr_id_t condType = 0;
    int condValue = 0;
    uint32_t i = 0;

    condType = object_attr_list[attr_id].conditionType;
    condValue = object_attr_list[attr_id].conditionValue;

    for (i = 0; i < object_attr_count; i++)
    {
        if ((object_attr_list[i].id == condType) &&
            (attr_present[i]))
        {
            if ((object_attr_list[i].attrValue == XP_SAI_ATTR_ANY) ||
                (object_attr_list[i].attrValue ==
                 condValue)) // condValue might be XP_SAI_ATTR_ANY
            {
                return true;
            }
        }
    }

    return false;
}

/**
 * \brief Verify all mandatory attributes present
 *
 * \param object_attr_count - size of all sai feature attributes
 * \param object_attr_list  - list of sai feature attributes and their configuration
 * \param attr_present      - list of present attributes
 *
 * \return bool
 */
static bool xpSaiAttrCheckMandatorySucceed(const xpSaiAttrEntry_t
                                           *object_attr_list,
                                           uint32_t object_attr_count, bool *attr_present)
{
    uint32_t i = 0, j = 0;
    bool mandatoryAttrPresent = true;

    // Either process present flags or process mandatory flags
    // value ANY can speed-up processing
    for (i = 0; i < object_attr_count; i++)
    {
        // Miss the check if attribute is not supported
        if (!object_attr_list[i].xpSaiSupport)
        {
            continue;
        }

        // Miss the check if conditions are false
        if (object_attr_list[i].conditionType !=
            XP_SAI_NO_CONDITION) // We have conditional attribute
        {
            // In case condition does not work, no check is required
            if (!xpSaiAttrCheckConditionsSucceed(object_attr_list, object_attr_count, i,
                                                 attr_present))
            {
                continue;
            }
        }

        // Perform mandatory attributes verification
        if (object_attr_list[i].mandatoryOnCreate && !attr_present[i])
        {
            mandatoryAttrPresent = false;
            for (j = 0; j < object_attr_count; j++)
            {
                if (object_attr_list[i].id == object_attr_list[j].id)
                {
                    mandatoryAttrPresent = mandatoryAttrPresent | attr_present[j];
                }
            }
            if (!mandatoryAttrPresent)
            {
                XP_SAI_LOG_ERR("Missing mandatory attribute %s \n", object_attr_list[i].name);
                return mandatoryAttrPresent;
            }
        }
    }

    return mandatoryAttrPresent;
}

/**
 * \brief Verify SAI attributes
 *
 * \param attr_count        - size of sai attribute list
 * \param attr_list         - pointer to sai attribute list
 * \param object_attr_count - size of all sai feature attributes
 * \param object_attr_list  - list of sai feature attributes and their configuration
 * \param oper              - saii API type
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAttrCheckImpl(uint32_t attr_count,
                                const sai_attribute_t *attr_list,
                                uint32_t object_attr_count, const xpSaiAttrEntry_t *object_attr_list,
                                sai_common_api_t oper)
{
    uint32_t index;
    bool *attr_present;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;


    if ((attr_count > 0) && (NULL == attr_list))
    {
        XP_SAI_LOG_ERR("attr_list is NULL and attr_count is non-zero\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == object_attr_list)
    {
        XP_SAI_LOG_ERR("object_attr_list is NULL\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((SAI_COMMON_API_MAX <= oper) || (oper < 0))
    {
        XP_SAI_LOG_ERR("operation %d out of range\n", oper);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (oper)
    {
        case SAI_COMMON_API_REMOVE:
            {
                XP_SAI_LOG_ERR("SAI_COMMON_API_REMOVE not implemented\n", oper);
                return SAI_STATUS_NOT_IMPLEMENTED;
            }

        case SAI_COMMON_API_SET:
            {
                if (1 != attr_count)
                {
                    XP_SAI_LOG_ERR("Trying to set more than one attribute\n");
                    return SAI_STATUS_INVALID_PARAMETER;
                }
                // FALLTHROUGH
            }
        default: // FALLTHROUGH
            {}
    }

    attr_present = (bool *)calloc(object_attr_count, sizeof(bool));
    if (NULL == attr_present)
    {
        XP_SAI_LOG_ERR("Can't allocate memory for attr_present array\n");
        return SAI_STATUS_NO_MEMORY;
    }

    for (uint32_t i = 0; i < attr_count; i++)
    {
        saiRetVal = xpSaiAttrIndexFind(attr_list[i].id, attr_list[i].value,
                                       object_attr_count, object_attr_list, oper, &index);

        switch (saiRetVal)
        {
            case SAI_STATUS_UNKNOWN_ATTRIBUTE_0:
                {
                    XP_SAI_LOG_ERR("Unknown attribute %d with value %d \n", attr_list[i].id,
                                   attr_list[i].value.s16);
                    free(attr_present);
                    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(i);
                }
            case SAI_STATUS_INVALID_ATTR_VALUE_0:
                {
                    XP_SAI_LOG_ERR("Unknown attribute %d with value %d \n", attr_list[i].id,
                                   attr_list[i].value.s16);
                    free(attr_present);
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(i);
                }
        }

        bool attrError = false;
        switch (oper)
        {
            case SAI_COMMON_API_CREATE:
                {
                    if (!object_attr_list[index].validForCreate)
                    {
                        XP_SAI_LOG_ERR("Invalid attribute %s for create\n",
                                       object_attr_list[index].name);
                        attrError=true;
                    }
                    break;
                }

            case SAI_COMMON_API_SET:
                {
                    if (!object_attr_list[index].validForSet)
                    {
                        XP_SAI_LOG_ERR("Invalid attribute %s for set\n", object_attr_list[index].name);
                        attrError=true;
                    }
                    break;
                }

            case SAI_COMMON_API_GET:
                {
                    if (!object_attr_list[index].validForGet)
                    {
                        XP_SAI_LOG_ERR("Invalid attribute %s for get\n", object_attr_list[index].name);
                        attrError=true;
                    }
                    break;
                }
            default: // FALLTHROUGH
                {}
        }
        if (attrError)
        {
            free(attr_present);
            return SAI_STATUS_INVALID_ATTRIBUTE_0 + SAI_STATUS_CODE(i);
        }

        if (attr_present[index])
        {
            XP_SAI_LOG_ERR("Attribute %s appears twice in attribute list at index %d\n",
                           object_attr_list[index].name, i);
            free(attr_present);
            return SAI_STATUS_INVALID_ATTRIBUTE_0 + SAI_STATUS_CODE(i);
        }

        // Process non-supported case
        if (!object_attr_list[index].xpSaiSupport)
        {
            XP_SAI_LOG_INFO("[skip] Unsupported attribute %s \n",
                            object_attr_list[index].name);
            free(attr_present);
            return SAI_STATUS_SUCCESS;
        }

        attr_present[index] = true;
    }

    if (SAI_COMMON_API_CREATE == oper)
    {
        if (!xpSaiAttrCheckMandatorySucceed(object_attr_list, object_attr_count,
                                            attr_present))
        {
            free(attr_present);
            return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
        }
    }

    free(attr_present);

    return SAI_STATUS_SUCCESS;
}

const sai_attribute_t *xpSaiFindAttrById(sai_attr_id_t id, uint32_t attr_count,
                                         const sai_attribute_t *attr_list, uint32_t *attr_idx)
{
    if (NULL == attr_list)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return NULL;
    }

    for (uint32_t i = 0; i < attr_count; i++)
    {
        if (id == attr_list[i].id)
        {
            if (NULL != attr_idx)
            {
                *attr_idx = i;
            }
            return &attr_list[i];
        }
    }

    return NULL;
}

int xpSaiGetExitStatus()
{
    return (xpsGetExitStatus());
}

void xpSaiSetExitStatus()
{
    xpsSetExitStatus();
}

void xpSaiSleepMsec(uint32_t msec)
{
    usleep(msec*1000);
}

/**
 * \brief Add port to portList
 *
 * \param [in,out] xpsPortList_t *portList
 * \param [in] xpsPort_t port
 *
 * Update the port list with new port, does not perform sorting
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAddPortToPortList(xpsPortList_t *portList, xpsPort_t port)
{
    sai_uint8_t  countIdx = 0;

    for (countIdx = 0; countIdx < portList->size; countIdx++)
    {
        if (port == portList->portList[countIdx])
        {
            // port is already present in the port list
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }
    }

    if (portList->size == SYSTEM_MAX_PORT)
    {
        // portList is full, can't append more ports
        return SAI_STATUS_NO_MEMORY;
    }

    portList->portList[portList->size++] = port;

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief Remove port from portList
 *
 * \param [in,out] xpsPortList_t *portList
 * \param [in] xpsPort_t port
 *
 * Removed the port from port list, replaces with last port in port list
 *
 * \return sai_status_t
 */
sai_status_t xpSaiRemovePortFromPortList(xpsPortList_t *portList,
                                         xpsPort_t port)
{
    sai_uint8_t  countIdx = 0;

    for (countIdx = 0; countIdx < portList->size; countIdx++)
    {
        if (port == portList->portList[countIdx])
        {
            // Replace removed port with last port in portList
            portList->portList[countIdx] = portList->portList[--portList->size];
            return SAI_STATUS_SUCCESS;
        }
    }

    return SAI_STATUS_ITEM_NOT_FOUND;
}

uint32_t xpSaiCtxGrowthNeeded(uint16_t numOfArrayElements,
                              uint16_t defaultArraySize)
{
    return xpsDAIsCtxGrowthNeeded(numOfArrayElements, defaultArraySize);
}

uint32_t xpSaiCtxShrinkNeeded(uint16_t numOfArrayElements,
                              uint16_t defaultArraySize)
{
    return xpsDAIsCtxShrinkNeeded(numOfArrayElements, defaultArraySize);
}

XP_PROFILE_TYPE xpSaiGetProfileType(xpsDevice_t devId)
{
    //TODO: When multidevice is done, devId should be used
    (void) devId;

    return devDefaultConfig.profileType;
}

bool xpSaicustIsL2MCAttrbiutesSupportedForcustProfile(xpsDevice_t devId,
                                                      uint32_t attr_count, const sai_attribute_t *attr_list)
{
    if (xpSaiGetProfileType(devId) == XP_QACL_SAI_PROFILE)
    {
        for (uint32_t i = 0; i < attr_count; ++i)
        {
            switch (attr_list[i].id)
            {
                case (SAI_L2MC_ENTRY_ATTR_PACKET_ACTION):
                case (SAI_L2MC_ENTRY_ATTR_OUTPUT_GROUP_ID):
                case (SAI_L2MC_ENTRY_ATTR_CUSTOM_RANGE_START):
                    {
                        return false;
                    }
                default:
                    {
                        break;
                    }
            }
        }
    }
    return true;
}

bool xpSaicustIsL2MCGroupAttrbiutesSupportedForcustProfile(xpsDevice_t devId,
                                                           uint32_t attr_count, const sai_attribute_t *attr_list)
{
    if (xpSaiGetProfileType(devId) == XP_QACL_SAI_PROFILE)
    {
        for (uint32_t i = 0; i < attr_count; ++i)
        {
            switch (attr_list[i].id)
            {
                case (SAI_L2MC_GROUP_ATTR_L2MC_OUTPUT_COUNT):
                case (SAI_L2MC_GROUP_ATTR_L2MC_MEMBER_LIST):
                    {
                        return false;
                    }
            }
        }
    }
    return true;
}

bool xpSaicustIsL2MCGroupMemberAttrSupportedForcustProfile(xpsDevice_t devId,
                                                           uint32_t attr_count, const sai_attribute_t *attr_list)
{
    if (xpSaiGetProfileType(devId) == XP_QACL_SAI_PROFILE)
    {
        for (uint32_t i = 0; i < attr_count; ++i)
        {
            switch (attr_list[i].id)
            {
                case (SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID):
                case (SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID):
                case (SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_ENDPOINT_IP):
                case (SAI_L2MC_GROUP_MEMBER_ATTR_CUSTOM_RANGE_START):
                    {
                        return false;
                    }
                default:
                    {
                        break;
                    }
            }
        }
    }
    return true;
}

bool xpSaicustIsVlanAttrSupportedForcustProfile(xpsDevice_t devId,
                                                uint32_t attr_count, const sai_attribute_t *attr_list)
{
    if (xpSaiGetProfileType(devId) == XP_QACL_SAI_PROFILE)
    {
        for (uint32_t i = 0; i < attr_count; ++i)
        {
            switch (attr_list[i].id)
            {
                case (SAI_VLAN_ATTR_CUSTOM_IGMP_SNOOPING_ENABLE):
                case (SAI_VLAN_ATTR_UNKNOWN_NON_IP_MCAST_OUTPUT_GROUP_ID):
                case (SAI_VLAN_ATTR_UNKNOWN_IPV4_MCAST_OUTPUT_GROUP_ID):
                case (SAI_VLAN_ATTR_UNKNOWN_IPV6_MCAST_OUTPUT_GROUP_ID):
                case (SAI_VLAN_ATTR_UNKNOWN_LINKLOCAL_MCAST_OUTPUT_GROUP_ID):
                case (SAI_VLAN_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP):
                case (SAI_VLAN_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP):
                case (SAI_VLAN_ATTR_BROADCAST_FLOOD_GROUP):
                    {
                        return false;
                    }
                default:
                    {
                        break;
                    }
            }
        }
    }
    return true;
}

bool xpSaicustIsBridgeAttrSupportedForcustProfile(xpsDevice_t devId,
                                                  uint32_t attr_count, const sai_attribute_t *attr_list)
{
    if (xpSaiGetProfileType(devId) == XP_QACL_SAI_PROFILE)
    {
        for (uint32_t i = 0; i < attr_count; ++i)
        {
            switch (attr_list[i].id)
            {
                case (SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP):
                case (SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP):
                case (SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP):
                    {
                        return false;
                    }
                default:
                    {
                        break;
                    }
            }
        }
    }
    return true;
}

bool xpSaicustIsFdbAttrSupportedForcustProfile(xpsDevice_t devId,
                                               uint32_t attr_count, const sai_attribute_t *attr_list)
{
    if (xpSaiGetProfileType(devId) == XP_QACL_SAI_PROFILE)
    {
        for (uint32_t i = 0; i < attr_count; ++i)
        {
            switch (attr_list[i].id)
            {
                case (SAI_MCAST_FDB_ENTRY_ATTR_GROUP_ID):
                case (SAI_MCAST_FDB_ENTRY_ATTR_PACKET_ACTION):
                case (SAI_MCAST_FDB_ENTRY_ATTR_META_DATA):
                case (SAI_MCAST_FDB_ENTRY_ATTR_CUSTOM_RANGE_START):
                    {
                        return false;
                    }
                default:
                    {
                        break;
                    }
            }
        }
    }
    return true;
}


sai_status_t xpSaiUtilGetSubIntervals(sai_uint16_t  lowerLimit,
                                      sai_uint16_t  upperLimit,
                                      sai_uint16_t *subFrom,
                                      sai_uint16_t *subMask,
                                      sai_uint16_t *pNumSubIntervals)
{
    /* Interval decomposition into power of two intervals */
    sai_uint32_t                                      currentFrom;
    sai_uint32_t                                      currentTo;
    sai_uint32_t                                      currentBit;
    sai_uint32_t                                      currentMask;
    sai_uint32_t                                      numSubIntervals;

    XP_SAI_LOG_NOTICE("\nThe value mask pairs below for from %d to %d\n",
                      lowerLimit, upperLimit);
    /* Decompose interval into a number of sub-intervals, each of a power of two length */
    currentFrom = (((sai_uint32_t)((sai_uint16_t) -1)) + 1) + lowerLimit;
    currentTo = (((sai_uint32_t)((sai_uint16_t) -1)) + 1) + upperLimit;
    currentBit = 1;
    currentMask = (sai_uint32_t) -1;
    numSubIntervals = 0;
    while (currentFrom <= currentTo)
    {
        if ((currentFrom & currentBit) != 0)
        {
            subFrom[numSubIntervals] = (sai_uint16_t) currentFrom;
            subMask[numSubIntervals] = (sai_uint16_t) currentMask;
            numSubIntervals++;
            currentFrom += currentBit;
        }

        if ((currentTo & currentBit) == 0)
        {
            subFrom[numSubIntervals] = (sai_uint16_t)(currentTo & currentMask);
            subMask[numSubIntervals] = (sai_uint16_t) currentMask;
            numSubIntervals++;
            currentTo -= currentBit;
        }

        currentMask &= ~currentBit;
        currentBit <<= 1;
    }

    *pNumSubIntervals = numSubIntervals;

    return SAI_STATUS_SUCCESS;

}

sai_status_t xpSaiProfileMgrBindProfileToPort(xpSaiPortProfile_t *profileList,
                                              sai_uint32_t profileIdx, sai_uint32_t portNum)
{
    uint32_t ref_count = profileList[profileIdx].refCount;
    profileList[profileIdx].portList[ref_count] = portNum;
    profileList[profileIdx].refCount++;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiProfileMgrUnbindProfileToPort(xpSaiPortProfile_t *profileList,
                                                sai_uint32_t profileIdx, sai_uint32_t portNum)
{
    uint32_t count = 0;
    bool portPresent = false;

    for (count = 0; count < profileList[profileIdx].refCount; count++)
    {
        if (!portPresent)
        {
            if (profileList[profileIdx].portList[count] == portNum)
            {
                portPresent = true;
            }
            continue;
        }
        else
        {
            profileList[profileIdx].portList[count-1] =
                profileList[profileIdx].portList[count];
        }
    }


    if (!portPresent)
    {
        XP_SAI_LOG_ERR("Error: Could not find port_obj in profiles list");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    profileList[profileIdx].refCount -= 1; // Update refCount

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiProfileMgrFindFreeIdx(xpSaiPortProfile_t *profileList,
                                        sai_uint32_t profileListSize,
                                        sai_uint32_t *freeIdx)
{
    uint32_t idx = 0;

    /* Find a match for new scheduler profile */
    for (idx = 0; idx < profileListSize; idx++)
    {
        if (profileList[idx].refCount == 0)
        {
            *freeIdx = idx;
            return SAI_STATUS_SUCCESS;
        }
    }

    return SAI_STATUS_FAILURE;
}

sai_status_t xpSaiProfileMgrFindMatch(xpSaiPortProfile_t *profileList,
                                      sai_uint32_t profileListSize,
                                      sai_object_id_t *newProfile, sai_uint32_t *newIdx)
{
    uint32_t idx = 0;
    uint32_t qNum = 0;

    /* Find a match for new scheduler profile */
    for (idx = 0; idx < profileListSize; idx++)
    {
        if (profileList[idx].refCount == 0)
        {
            continue;
        }
        for (qNum = 0; qNum < XPSAI_MAX_TC_VALUE; qNum++)
        {
            if (profileList[idx].profileObjId[qNum] != newProfile[qNum])
            {
                break;
            }
        }
        if (qNum == XPSAI_MAX_TC_VALUE)
        {
            /* profile match found */
            *newIdx = idx;
            return SAI_STATUS_SUCCESS;
        }
    }

    return SAI_STATUS_FAILURE;
}

/* Syncd SIGNAL handlers. Capture the Back trace for the registered
 * signal and gracefully reset the device.
 */

void xpSaiSignalHandler(int sig)
{
    //    uint32_t devId = 0;

    //cpssOsPrintf("Error: signal %d\n", sig);

    gResetInProgress = 1;

    /*
     * Ensure none of the application threads access CPSS APIs
     * before xpsResetSwitch and printf used only when error is seen.
     * This is handled in mvDma and mvInt driver. So commenting here.
     * */
    //    xpsResetSwitch(devId, true);

    signal(sig, SIG_DFL);
    kill(getpid(), sig);
}

void xpSaiSignalRegister()
{
    signal(SIGSEGV, xpSaiSignalHandler);
    signal(SIGABRT, xpSaiSignalHandler);
    signal(SIGTERM, xpSaiSignalHandler);
    signal(SIGINT, xpSaiSignalHandler);
}
