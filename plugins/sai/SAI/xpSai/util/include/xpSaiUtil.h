// xpSaiUtil.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiUtil_h_
#define _xpSaiUtil_h_

#include "xpTypes.h"

#ifdef __cplusplus
extern "C" {
#endif  /*__cplusplus*/

#include "sai.h"

#define XPS_APP_SAI_PROFILE_ARRAY_SIZE      40
#define XPS_APP_SAI_PROFILE_NUMBER          2
#define XPS_APP_SAI_PROFILE_VALUE_STR_SIZE  20
#define XP_SAI_OBJ_ATTR_INVALID             (0xFFFFFFFF)
#define XP_SAI_ATTR_ANY                     (-1)
#define XP_SAI_NO_CONDITION                 (0xFFFF)

/**
 * @def SAI_SCPU_FW_FILE
 * Service CPU Firmware File
 */
#define SAI_SCPU_FW_FILE                            "scpuFwFile"

/**
 * @def SAI_ENABLE_SCPU_FW
 * Enable SCPU to run firmware
 */
#define SAI_ENABLE_SCPU_FW                          "enableScpuFw"

/**
 * @def SAI_Z80_AN_FW_FILE
 * Z80 Ucode Firmware File
 */
#define SAI_Z80_AN_FW_FILE                          "z80ANFwFile"

#define XP_SAI_MAC_ADDR_FMT \
    "%02" PRIx8 ":%02" PRIx8 ":%02" PRIx8 ":%02" PRIx8 ":%02" PRIx8 ":%02" PRIx8

#define XP_SAI_MAC_ADDR_ARGS(mac) \
    (mac)[0], (mac)[1], (mac)[2], (mac)[3], (mac)[4], (mac)[5]
#define XP_SAI_XP_MAC_ADDR_ARGS(mac) \
    (mac)[5], (mac)[4], (mac)[3], (mac)[2], (mac)[1], (mac)[0]

#define XP_SAI_IP_ADDR_FMT "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8

#define XP_SAI_IP_ADDR_ARGS(ip) \
    (ip)[0], (ip)[1], (ip)[2], (ip)[3]
#define XP_SAI_XP_IP_ADDR_ARGS(ip) \
    (ip)[3], (ip)[2], (ip)[1], (ip)[0]

#define XP_SAI_IPv6_ADDR_FMT \
    "%02" PRIx8 "%02" PRIx8 ":%02" PRIx8 "%02" PRIx8 ":%02" PRIx8 "%02" PRIx8 ":%02" PRIx8 "%02" PRIx8 \
    ":%02" PRIx8 "%02" PRIx8 ":%02" PRIx8 "%02" PRIx8 ":%02" PRIx8 "%02" PRIx8 ":%02" PRIx8 "%02" PRIx8

#define XP_SAI_IPv6_ADDR_ARGS(ip) \
    (ip)[0], (ip)[1], (ip)[2], (ip)[3], (ip)[4], (ip)[5], (ip)[6], (ip)[7], \
    (ip)[8], (ip)[9], (ip)[10], (ip)[11], (ip)[12], (ip)[13], (ip)[14], (ip)[15]
#define XP_SAI_XP_IPv6_ADDR_ARGS(ip) \
    (ip)[15], (ip)[14], (ip)[13], (ip)[12], (ip)[11], (ip)[10], (ip)[9], (ip)[8], \
    (ip)[7], (ip)[6], (ip)[5], (ip)[4], (ip)[3], (ip)[2], (ip)[1], (ip)[0]


typedef struct xpSaiAppSaiKeyValue_t
{
    char* key;
    char* value;
} xpSaiAppSaiKeyValue_t;

typedef struct xpSaiAttrEntry_t
{
    sai_attr_id_t   id;
    int             attrValue;
    bool            xpSaiSupport;
    bool            mandatoryOnCreate;
    bool            validForCreate;
    bool            validForSet;
    bool            validForGet;
    const char      *name;
    sai_attr_id_t   conditionType;
    int             conditionValue;
    int             conditionValType;
} xpSaiAttrEntry_t;

/**
 *@brief Attribute types
 */
typedef enum _attr_val_type_t
{
    XP_SAI_TYPE_U8 = 0,
    XP_SAI_TYPE_S8,
    XP_SAI_TYPE_U16,
    XP_SAI_TYPE_S16,
    XP_SAI_TYPE_U32,
    XP_SAI_TYPE_S32,
    XP_SAI_TYPE_U64,
    XP_SAI_TYPE_S64,
    XP_SAI_TYPE_BOOL,
    XP_SAI_TYPE_INVALID

} attr_val_type_t;

sai_status_t xpSaiAttrCheckImpl(uint32_t attr_count,
                                const sai_attribute_t *attr_list,
                                uint32_t object_attr_count, const xpSaiAttrEntry_t *object_attr_list,
                                sai_common_api_t oper);

bool xpSaicustIsL2MCAttrbiutesSupportedForcustProfile(xpsDevice_t devId,
                                                      uint32_t attr_count, const sai_attribute_t *attr_list);
bool xpSaicustIsL2MCGroupAttrbiutesSupportedForcustProfile(xpsDevice_t devId,
                                                           uint32_t attr_count, const sai_attribute_t *attr_list);
bool xpSaicustIsL2MCGroupMemberAttrSupportedForcustProfile(xpsDevice_t devId,
                                                           uint32_t attr_count, const sai_attribute_t *attr_list);
bool xpSaicustIsVlanAttrSupportedForcustProfile(xpsDevice_t devId,
                                                uint32_t attr_count, const sai_attribute_t *attr_list);
bool xpSaicustIsBridgeAttrSupportedForcustProfile(xpsDevice_t devId,
                                                  uint32_t attr_count, const sai_attribute_t *attr_list);
bool xpSaicustIsFdbAttrSupportedForcustProfile(xpsDevice_t devId,
                                               uint32_t attr_count, const sai_attribute_t *attr_list);

#define xpSaiAttrCheck(ATTR_COUNT, ATTR_LIST, OBJECT_ATTR_COUNT, OBJECT_ATTR_LIST, OPER)                                                                        \
    (((OBJECT_ATTR_LIST == l2mc_entry_attribs) && (!xpSaicustIsL2MCAttrbiutesSupportedForcustProfile(xpSaiGetDevId(), ATTR_COUNT, ATTR_LIST))) ||               \
     ((OBJECT_ATTR_LIST == l2mc_group_attribs) && (!xpSaicustIsL2MCGroupAttrbiutesSupportedForcustProfile(xpSaiGetDevId(), ATTR_COUNT, ATTR_LIST))) ||          \
     ((OBJECT_ATTR_LIST == l2mc_group_member_attribs) && (!xpSaicustIsL2MCGroupMemberAttrSupportedForcustProfile(xpSaiGetDevId(), ATTR_COUNT, ATTR_LIST))) ||   \
     ((OBJECT_ATTR_LIST == bridge_attribs) && (!xpSaicustIsBridgeAttrSupportedForcustProfile(xpSaiGetDevId(), ATTR_COUNT, ATTR_LIST))) ||                       \
     ((OBJECT_ATTR_LIST == vlan_attribs) && (!xpSaicustIsVlanAttrSupportedForcustProfile(xpSaiGetDevId(), ATTR_COUNT, ATTR_LIST))) ||                           \
     ((OBJECT_ATTR_LIST == mcast_fdb_entry_attribs) && (!xpSaicustIsFdbAttrSupportedForcustProfile(xpSaiGetDevId(), ATTR_COUNT, ATTR_LIST))))                   \
    ? SAI_STATUS_NOT_SUPPORTED :                                                                                                                                \
    xpSaiAttrCheckImpl(ATTR_COUNT, ATTR_LIST, OBJECT_ATTR_COUNT, OBJECT_ATTR_LIST, OPER);

static inline bool xpSaiEthAddrIsZero(const sai_mac_t ea)
{
    return !(ea[0] | ea[1] | ea[2] | ea[3] | ea[4] | ea[5]);
}

static inline bool xpSaiEthAddrIsEqual(const sai_mac_t a, const sai_mac_t b)
{
    return !memcmp(a, b, sizeof(sai_mac_t));
}

/* Copies src mac into dst mac in reverse order. */
static inline void xpSaiMacCopyAndReverse(macAddr_t dstMac,
                                          const macAddr_t srcMac)
{
    uint8_t i = 0;
    uint8_t j = 0;

    for (i = 0, j = (XP_MAC_ADDR_LEN - 1); i < XP_MAC_ADDR_LEN; i++, j--)
    {
        dstMac[i] = srcMac[j];
    }
}

/* Copies src mac into dst mac in same order. */
static inline void xpSaiMacCopy(macAddr_t dstMac, const macAddr_t srcMac)
{
    uint8_t i = 0;

    for (i = 0; i < XP_MAC_ADDR_LEN; i++)
    {
        dstMac[i] = srcMac[i];
    }
}

/*Check whether the type of mac address is multicast, unicast or broadcast*/
bool xpSaiUtilIsMulticastMac(const sai_mac_t mac);
bool xpSaiUtilIsUnicastMac(const sai_mac_t mac);
bool xpSaiUtilIsBroadcastMac(const sai_mac_t mac);

/* Copies src mac into dst mac in reverse order. */
static inline void xpSaiIpCopyAndReverse(uint8_t *dstIp, const uint8_t *srcIp,
                                         sai_ip_addr_family_t type)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t length = (type == SAI_IP_ADDR_FAMILY_IPV4) ? XP_IPV4_ADDR_LEN :
                     XP_IPV6_ADDR_LEN;

    for (i = 0, j = (length - 1); i < length; i++, j--)
    {
        dstIp[i] = srcIp[j];
    }
}


static inline void xpSaiIpCopy(uint8_t *dstIp, const uint8_t *srcIp,
                               sai_ip_addr_family_t type)
{
    uint8_t i = 0;
    uint8_t length = (type == SAI_IP_ADDR_FAMILY_IPV4) ? XP_IPV4_ADDR_LEN :
                     XP_IPV6_ADDR_LEN;

    for (i = 0; i < length; i++)
    {
        dstIp[i] = srcIp[i];
    }
}

static inline bool xpSaiIpIsEqual(sai_ip_address_t *firstIp,
                                  sai_ip_address_t *secondIp)
{
    uint8_t length;

    if (!firstIp || !secondIp || firstIp->addr_family != secondIp->addr_family)
    {
        return false;
    }

    length = (firstIp->addr_family == SAI_IP_ADDR_FAMILY_IPV4) ? XP_IPV4_ADDR_LEN :
             XP_IPV6_ADDR_LEN;
    return (memcmp(&firstIp->addr, &secondIp->addr, length) == 0);
}

XP_STATUS xpSaiAppSaiProfileKeyValueAdd(sai_switch_profile_id_t profile_id,
                                        const char* key, char* value);
sai_status_t xpSaiAppSaiProfileKeyIntValueAdd(sai_switch_profile_id_t
                                              profile_id, const char* key, uint32_t value);

/* Get variable value given its name */
const char* xpSaiAppSaiProfileGetValue(sai_switch_profile_id_t profile_id,
                                       const char* key);
/* try to load variable from locally stored aray */
const char* xpSaiProfileKeyValueGet(sai_switch_profile_id_t profile_id,
                                    const char* key);
/* try to load variable from file pointed by 'fileName'*/
XP_STATUS xpSaiProfileConfigLoadFromFile(sai_switch_profile_id_t profile_id,
                                         const char *fileName);

/* Enumerate all the K/V pairs in a profile.
   Pointer to NULL passed as variable restarts enumeration.
   Function returns 0 if next value exists, -1 at the end of the list. */
int xpSaiAppSaiProfileGetNextValue(sai_switch_profile_id_t profile_id,
                                   const char** variable, const char** value);

/**
 * \brief Find attribute in the list of attributes by attribute ID
 *
 * This API is used to find an attribute in the list of attributes
 * by attribute ID. Returns either pointer to the attribute or NULL
 * in case of error.
 *
 * \param [in] sai_attr_id_t id
 * \param [in] uint32_t attr_count
 * \param [in] const sai_attribute_t* attr_list
 * \param [out] uint32_t *attr_idx
 *
 * \return sai_attribute_t* or NULL
 */
const sai_attribute_t *xpSaiFindAttrById(sai_attr_id_t id, uint32_t attr_count,
                                         const sai_attribute_t *attr_list, uint32_t *attr_idx);

/**
 * \brief Get the saiApp status
 *
 * This API is used to find application status.
 * Get value 0 : saiApp running
 * Get value > 0 : saiApp in exit mode
 *
 * \return int
*/
int xpSaiGetExitStatus();

/**
 * \brief Set the saiApp in exit state
 *
 * This API is used to set the application in exit state.
 *
 * \return void
*/
void xpSaiSetExitStatus();

/**
 * \brief Block operation for a number of milliseconds
 *
 * \return void
*/
void xpSaiSleepMsec(uint32_t msec);

/**
 * \brief SAI wrapper to xpsDAIsCtxGrowthNeeded function
 *
 * \param [in] numOfArrayElements  Number of elements in array
 * \param [in] defaultArraySize    Default array size
 *
 * \return non-zero if array should be grown, zero if growth is not needed
 */
uint32_t xpSaiCtxGrowthNeeded(uint16_t numOfArrayElements,
                              uint16_t defaultArraySize);

/**
 * \brief SAI wrapper to xpsDAIsCtxShrinkNeeded function
 *
 * \param [in] numOfArrayElements  Number of elements in array
 * \param [in] defaultArraySize    Default array size
 *
 * \return non-zero if array should be shrinked, zero if shrink is not needed
 */
uint32_t xpSaiCtxShrinkNeeded(uint16_t numOfArrayElements,
                              uint16_t defaultArraySize);

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
sai_status_t xpSaiAddPortToPortList(xpsPortList_t *portList, xpsPort_t port);

/**
 * \brief Remove port to portList
 *
 * \param [in,out] xpsPortList_t *portList
 * \param [in] xpsPort_t port
 *
 * Removed the port from port list, replaces with last port in port list
 *
 * \return sai_status_t
 */
sai_status_t xpSaiRemovePortFromPortList(xpsPortList_t *portList,
                                         xpsPort_t port);

/**
 * \brief xpSaiGetProfileType
 * \param devId
 *
 * Returns the profile type from device default config
 * Currently no check for devId, because missing implementation of multidevice.
 *
 * \return XP_PROFILE_TYPE
 */
XP_PROFILE_TYPE xpSaiGetProfileType(xpsDevice_t devId);

/**
 * \brief xpSaiUtilGetSubIntervals
 * \param lowerLimit
 * \param upperLimit
 * \param *subFrom
 * \param *subMask
 * \param *pNumSubIntervals
 *
 * This utility function takes in parameters as two limits of a range
 * and breaks them down into multiple subintervals of powers of 2 and
 * returns value & mask pairs
 *
 * \return sai_status_t
 */

sai_status_t xpSaiUtilGetSubIntervals(sai_uint16_t  lowerLimit,
                                      sai_uint16_t  upperLimit,
                                      sai_uint16_t *subFrom,
                                      sai_uint16_t *subMask,
                                      sai_uint16_t *pNumSubIntervals);



#define XPSAI_SCHEDULER_MAX_PROFILES 16
#define XPSAI_MAX_TC_VALUE 16

/**
 * Below functions are used for mapping and managing
 * CPSS port profiles (like shceduler, taildrop)
 * and respective SAI objects.
 */

typedef struct xpSaiPortProfile_t
{
    sai_object_id_t profileObjId[XPSAI_MAX_TC_VALUE];
    sai_uint32_t    portList[SYSTEM_MAX_PORT];
    sai_uint32_t    refCount;
} xpSaiPortProfile_t;

sai_status_t xpSaiProfileMgrBindProfileToPort(xpSaiPortProfile_t *profileList,
                                              sai_uint32_t profileIdx, sai_uint32_t portNum);

sai_status_t xpSaiProfileMgrUnbindProfileToPort(xpSaiPortProfile_t *profileList,
                                                sai_uint32_t profileIdx, sai_uint32_t portNum);

sai_status_t xpSaiProfileMgrFindFreeIdx(xpSaiPortProfile_t *profileList,
                                        sai_uint32_t profileListSize, sai_uint32_t *freeIdx);

sai_status_t xpSaiProfileMgrFindMatch(xpSaiPortProfile_t *profileList,
                                      sai_uint32_t profileListSize, sai_object_id_t *newProfile,
                                      sai_uint32_t *newIdx);
void xpSaiSignalRegister();

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*_xpSaiUtil_h_*/
