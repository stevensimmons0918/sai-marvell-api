/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssDxChExactMatchManagerTypes.h
*
* @brief Exact Match manager support - API level data type definitions
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChExactMatchManagerTypes_h
#define __cpssDxChExactMatchManagerTypes_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/ip/cpssIpTypes.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatch.h>

/* max number of banks in Exact Match hash */
#define CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS                 16

/* max number of managers supported */
#define CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS              32

/* min/max profileId value */
#define CPSS_DXCH_EXACT_MATCH_MANAGER_MIN_PROFILE_ID_CNS                 1
#define CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_PROFILE_ID_CNS                15

/* min/max expander action index value */
#define CPSS_DXCH_EXACT_MATCH_MANAGER_MIN_EXPANDED_ACTION_INDEX_CNS     0
#define CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_EXPANDED_ACTION_INDEX_CNS     15

#define CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS   16

/* the size of an action in words */
#define CPSS_DXCH_EXACT_MATCH_MANAGER_ACTION_SIZE_CNS                   8

/* max number of entries in the Exact Match profileId mapping
   16 entries for TTI and 128 entries for PCL/EPCL Falcon
   or 256 entries for PCL/EPCL for AC5P */
#define MAX_NUM_PROFILE_ID_MAPPING_ENTRIES_CNS                          256

/* max number of profiles: 1-15, 0 is used as disabled value */
#define CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_PROFILES_NUM_CNS              16

/* macro to validate the value of profile ID value */
#define CPSS_DXCH_EXACT_MATCH_MANAGER_CHECK_PROFILE_ID_VALUE_MAC(profileId)      \
    if(((profileId) < CPSS_DXCH_EXACT_MATCH_MANAGER_MIN_PROFILE_ID_CNS) ||       \
       ((profileId) > CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_PROFILE_ID_CNS))         \
    {                                                                            \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);           \
    }

/* macro to validate the value of expanded action index */
#define CPSS_DXCH_EXACT_MATCH_MANAGER_CHECK_EXPANDED_ACTION_INDEX_VALUE_MAC(expandedActionIndex)    \
    if((expandedActionIndex) > CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS)       \
    {                                                                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                              \
    }


/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC
*
*  @brief Defines a pair of device and its port group bitmap
*         (for 'manager device+portGroup list add ')
*
*/
typedef struct{
   /** device number */
   GT_U8                devNum;

   /** port group bitmap associated with the device
    *  NOTEs:
    *      1. for non multi-port groups device this parameter is IGNORED.
    *      2. for multi-port groups device :
    *        (APPLICABLE DEVICES  Falcon; AC5P; AC5X; Harrier; Ironman)
    *      bitmap must be set with at least one bit representing
    *      valid port group(s). If a bit of non valid port group
    *      is set then function returns GT_BAD_PARAM.
    *      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.*/
   GT_PORT_GROUPS_BMP   portGroupsBmp;

}CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC;


/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_HW_CAPACITY_STC
*
*  @brief Defines the Exact Match HW capacity parameters (for 'manager create')
*
* need to add correct comments & check during device add
*/
typedef struct{
    /** number of indexes in HW :
     *      Falcon, AC5P: 32K,64K,128K  */
    GT_U32   numOfHwIndexes;
    /** number of hashes (banks) : 16,8,4
    *  For SIP6 devices, the number of hashes depends on shared table mode (Exact Match size):
    *  Exact Match with 4 hashes(banks) -
    *       Falcon, AC5P: coresponding to 32K hwIndexes
    *  Exact Match with 8 hashes(banks) -
    *       Falcon, AC5P: coresponding to 64K hwIndexes
    *  Exact Match with 16 hashes(banks)-
    *       Falcon, AC5P: coresponding to 128K hwIndexes */
    GT_U32   numOfHashes;

}CPSS_DXCH_EXACT_MATCH_MANAGER_HW_CAPACITY_STC;

/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC
*
*  @brief Defines the Exact Match capacity parameters (SW and HW) (for 'manager create')
*
*/
typedef struct{
    /** @brief: Set of parameters that defines the HW capacity */
    CPSS_DXCH_EXACT_MATCH_MANAGER_HW_CAPACITY_STC  hwCapacity;
    /** @brief: Maximal total entries application would like to support (can be lower than HW capacity)
       Value must be in steps of 256 entries.(but not ZERO)
       Note: 5_BYTES  entries are counted as 1 entry
             19_BYTES entries are counted as 2 entries
             33_BYTES entries are counted as 3 entries
             47_BYTES entries are counted as 4 entries */
    GT_U32  maxTotalEntries;
    /** @brief: Maximal number of entries that will be processed per aging scan call (Not all necessary be aged-out).
       Value must be in steps of 256 entries (but not ZERO) and must be equal or smaller than maxTotalEntries.
       Together with Total max entries Affects the number of aging bins.
       See Aging bin description and Aging scan API for details*/
    GT_U32  maxEntriesPerAgingScan;
    /** @brief: Maximal number of entries that will be processed per delete scan call
       (Not all necessary be deleted).
       Value must be in steps of 256 entries (but not ZERO) and must be equal or smaller than maxTotalEntries.
       See delete scan for details.*/
    GT_U32  maxEntriesPerDeleteScan;
}CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC;

/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ENTRY_PARAMS_STC
*
*  @brief Defines the Exact Match Profile Entry parameters
*
*/
typedef struct{
    /** SIP_6_10 and above this field indicate if the profileId
     *  is valid and should be set to HW */
    GT_BOOL                                         isValidProfileId;

    /** Exact Match Profile key parameters KeySize+keyStart+mask */
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    keyParams;

    /** Exact Match Default Entry Action Type (TTI/PCL/EPCL) */
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT           defaultActionType;

    /** Exact Match Default Action */
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                defaultAction;

    /** Enable using Profile Table default Action in case there is
    *   no match in the Exact Match lookup and in the TCAM lookup */
    GT_BOOL                                         defaultActionEn;

}CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ENTRY_PARAMS_STC;

/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_TTI_STC
*
*  @brief Defines the Exact Match TTI profileId mapping
*         parameters (for 'manager create')
*
*/
typedef struct{
    /** @brief
     *  TTI key type, used as index to the mapping table
    */
    CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType;
    /** @brief
     *  enable Exact Match lookup
     *  GT_TRUE: trigger Exact Match Lookup.
     *  GT_FALSE: Do not trigger Exact Match lookup.
    */
    GT_BOOL                     enableExactMatchLookup;
    /** @brief
     *  Exact Match profile identifier, that will be assign to the
     *  packet according to the fields above
     *  (APPLICABLE RANGES: 1..15)
    */
    GT_U32                      profileId;

}CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_TTI_STC;

/**
* @struct
*         CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_PCL_STC
*
* @brief Defines the Exact Match PCL profileId mapping
*        parameters (for 'manager create')
*
*/
typedef struct{
    /** @brief
     *  PCL packet type, used as index to the mapping table
    */
    CPSS_DXCH_PCL_PACKET_TYPE_ENT   packetType;
    /** @brief
     *  pcl sub profile (APPLICABLE RANGES: 0..7)
    */
    GT_U32                          subProfileId;
    /** @brief
     *  enable Exact Match lookup
     *  GT_TRUE: trigger Exact Match Lookup.
     *  GT_FALSE: Do not trigger Exact Match lookup.
    */
    GT_BOOL                         enableExactMatchLookup;
    /** @brief
     *  Exact Match profile identifier, that will be assign to the
     *  packet according to the fields above
     *  (APPLICABLE RANGES: 1..15)
    */
    GT_U32                          profileId;
}CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_PCL_STC;

/**
 * @union CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_UNT
 *
 * @brief Exact Match profileId mapping union
*/
typedef union{
    /** TTI mapping (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman) */
    CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_TTI_STC   ttiMappingElem;

    /** PCL/EPCL mapping (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman) */
    CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_PCL_STC   pclMappingElem;

} CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_UNT;

/**
 * @struct CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_CONFIG_STC
 *
 *  @brief Defines the Exact Match lookup client parameters (for
 *  'manager create'). Maps incoming packets attributes to
 *  profileId (1-15). profileId defines for each lookup how to
 *  build Exact Match key according to packet data.
 *
 */
typedef struct{
    /** @brief
     *  global configuration to enable the lookup configuration
     */
    GT_BOOL                                     lookupEnable;

    /** @brief
     *  global configuration to determine the client of the first
     *  and second Exact Match lookup. clients can be TTI/PCL/EPCL.
     *  for example: If TTI is configured as the client for the
     *  first lookup, then firstLookupClientMappingsArray should
     *  have only elements of type CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_TTI_STC
     */
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            lookupClient;

    /** @brief
     *  number of valid elements in ProfileIdMapping array
     */
    GT_U32                                      lookupClientMappingsNum;

    /** @brief for the lookup client we configure a profileId
     *  mapping. the mapping can fit a
     *  TTI configuration: keyType --> profileId
     *  PCL/EPCL configuration: packetType+subProfileId --> profileId
     *
     */
    CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_UNT lookupClientMappingsArray[MAX_NUM_PROFILE_ID_MAPPING_ENTRIES_CNS];

}CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_CONFIG_STC;


/**
 * @struct CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC
 *
 *  @brief Defines the Exact Match lookup parameters (for
 *  'manager create'). Maps incoming packets attributes to
 *  profileId (1-15). profileId defines for each lookup how to
 *  build Exact Match key according to packet data.
 *
 */
typedef struct{
    /** @brief
     *  array of lookup configuration
     */
    CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_CONFIG_STC  lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E];

    /** @brief 1-15 profiles supported (entry 0 will stay empty
     *         for simpler  use of this array indexes) */
    CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ENTRY_PARAMS_STC   profileEntryParamsArray[CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_PROFILES_NUM_CNS];

}CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC;

/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC
*
* @brief Defines the Exact Match Expanded entry
*
*/
typedef struct{
    /** GT_TRUE:  entry is valid
     *  GT_FALSE: entry was not configured */
    GT_BOOL exactMatchExpandedEntryValid;

    /** Exact Match Expanded Action Type (TTI/PCL/EPCL) */
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT            expandedActionType;

    /** Exact Match Expanded Action */
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                 expandedAction;

    /** Whether to use the action attributes from the Exact Match
     *  rule action or from the profile  */
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT expandedActionOrigin;

}CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC;

/**
 * @struct CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC
 *
 *  @brief Defines the Exact Match Expander parameters (for 'manager create').
 *  Expander defines how to build Exact Match Entry.
 *
 */
typedef struct{
    /** @brief
     *  array of expander entries configuration (0-15 entries)
     */
    CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC  expandedArray[CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS];

}CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC;


/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC
*
* @brief Defines the Exact Match aging parameters (for 'manager create')
*
*/
typedef struct{
    /** @brief Enables/disables the refresh of the
     *  Exact Match activity bit.
     *  This should be set to true for aging scan to be
     *  done - See cpssDxChExactMatchManagerAgingScan   */
    GT_BOOL     agingRefreshEnable;

}CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC;


/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_INTERFACE_INFO_DEV_PORT_STC
*
* @brief Defines the interfaces 'devPort' info for the Exact
*        Match manager entry
*
*/
typedef struct{
    /** the HW device number associated with the Exact Match entry */
    GT_HW_DEV_NUM   hwDevNum;
    /** the port (eport) number associated with the Exact Match entry */
    GT_PORT_NUM     portNum ;
}CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_INTERFACE_INFO_DEV_PORT_STC;

/**
* @union CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_INTERFACE_INFO_UNT
*
* @brief Defines the interfaces info for the Exact Match manager
*        entry : port/trunk/vidx
*
*/
typedef union{
    /* for type CPSS_INTERFACE_PORT_E */
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_INTERFACE_INFO_DEV_PORT_STC devPort;
    /* for type CPSS_INTERFACE_TRUNK_E */
    GT_TRUNK_ID     trunkId;
    /* for type CPSS_INTERFACE_VIDX_E */
    GT_U16          vidx;
    /* NOTE: for type CPSS_INTERFACE_VID_E : no extra info needed ! */

    /* NOTE: other types not supported */
}CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_INTERFACE_INFO_UNT;

/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_INTERFACE_INFO_STC
*
*  @brief Defines the interface info for the Exact Match manager entry
*
*/
typedef struct{
    /** @brief the type of 'interface' that associated with the
     * Exact Match entry. this supporting only
     * port/trunk/vidx/vid interfaces */
    CPSS_INTERFACE_TYPE_ENT                            type;
    /** The 'interface' that associated with the Exact Match entry */
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_INTERFACE_INFO_UNT interfaceInfo;
}CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_INTERFACE_INFO_STC;

/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC
*
* @brief Exact Match manager Entry format
*/
typedef struct{
    /** Exact Match entry: key size+pattern+lookupType       */
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC       exactMatchEntry;

    /** Exact Match Action Type (TTI or PCL)                 */
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT exactMatchActionType;

    /** Exact Match Action (TTI Action or PCL Action)        */
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT      exactMatchAction;

    /** The expanded action index of the entry, used to build
     *  the reduced Exact Match entry                       */
    GT_U32                                expandedActionIndex;

    /** userDefined value used to specify the entry with a special value
       this value will be used for age_scan and delete_scan
       this is a SW database configuration
       SUPPORTED VALUES : 0..65535 (16 bits)               */
    GT_U32                                exactMatchUserDefined;

}CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC;

/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC
*
* @brief Exact Match manager add parameters
*/
typedef struct{
    /** @brief indication to enable the 're-hash' in case the new entry is not able to
        be inserted at 'first hash level' by the manager to move existing entries
        to allow place for the new entry (AKA : Cuckoo Algorithm) */
    GT_BOOL rehashEnable;

}CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC;

/**
* @enum CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_TYPE_ENT
 *
 * @brief This enum defines the type of the update message
*/
typedef enum{

    /** @brief The entry is a AGED OUT update                    */
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_OUT_E,

    /** @brief The entry is a Aged out and deleted update        */
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_DELETED_E,

    /** @brief The entry is a deleted update                     */
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_TYPE_DELETED_E,

} CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_TYPE_ENT;

/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC
*
* @brief Exact Match manager Update Event format
*/
typedef struct{

    /** @brief indicates type of update event */
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_TYPE_ENT updateType;

    /** @brief info structure for the Exact Match entry */
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC entry;

}CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC;

/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC
 *
 * @brief Exact Match manager Delete scan attributes.
*/
typedef struct{

    /* Entry Types */
    /** @brief Input to the Delete scan to include(or exclude) Lookup entries
     *        GT_TRUE   - Include Lookup type of entries in delete process.
     *        GT_FALSE  - Do not include Lookup type of entries in delete process.
     */
    GT_BOOL deleteLookupEntries[CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E];

    /* @brief Input to the Delete scan to include(or exclude) client entries(TTI/IPCL/EPCL)
     *        GT_TRUE   - Include client entries in delete process.
     *        GT_FALSE  - Do not include client entries in delete process.
     */
    GT_BOOL deleteClientEntries[CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E];

    /** @brief Input to the Delete scan to include(or exclude) entries with specific key size
     *        GT_TRUE   - Include specific key size type of entries in delete process.
     *        GT_FALSE  - Do not include specific key size type of entries in delete process.
     */
    GT_BOOL deleteKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E];

    /* Filters */
    /** @brief indicates the pattern value(To filter the entry) */
    GT_U8          pattern[CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS];

    /** @brief indicates the pattern mask bits */
    GT_U8          mask[CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS];

    /** @brief indicates the userDefined pattern value(To filter the entry) */
    GT_U32         userDefinedPattern;

    /** @brief indicates the userDefined mask bits  */
    GT_U32         userDefinedMask;

}CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC;

/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC
*
* @brief Exact Match manager aging scan attributes.
*/
typedef struct{
    /* Entry Types */
    /** @brief Input to the Aging scan to include(or exclude) Lookup entries
     *        GT_TRUE   - Include Lookup type of entries in delete process.
     *        GT_FALSE  - Do not include Lookup type of entries in aging process.
     */
    GT_BOOL checkAgeLookupEntries[CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E];

    /** @brief Input to the Aging scan to include(or exclude) client entries(TTI/IPCL/EPCL)
     *        GT_TRUE   - Include client entries in delete process.
     *        GT_FALSE  - Do not include client entries in aging process.
     */
    GT_BOOL checkAgeClientEntries[CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E];

    /** @brief Input to the Aging scan to include(or exclude) entries with specific key size
     *        GT_TRUE   - Include specific key size type of entries in delete process.
     *        GT_FALSE  - Do not include specific key size type of entries in aging process.
     */
    GT_BOOL checkAgeKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E];

    /* Filters */
    /** @brief indicates the pattern value(To filter the entry) */
    GT_U8          pattern[CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS];

    /** @brief indicates the pattern mask bits */
    GT_U8          mask[CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS];

    /** @brief indicates the userDefined pattern value(To filter the entry) */
    GT_U32         userDefinedPattern;

    /** @brief indicates the userDefined mask bits  */
    GT_U32         userDefinedMask;

    /* -------------Delete Action-------------- */
    /** @brief Input to the Aging scan, indicating aged-out specific lookup entries to delete
     *        GT_TRUE   - Delete the aged-out specific lookup entries
     *        GT_FALSE  - Do not delete the aged-out specific lookup entries.
     */
    GT_BOOL deleteAgeoutLookupEntries[CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E];

    /** @brief Input to the Aging scan, indicating aged-out specific client
     *        entries (TTI/IPCL/EPCL) to delete
     *        GT_TRUE   - Delete the aged-out specific client entries
     *        GT_FALSE  - Do not delete the aged-out specific client entries.
     */
    GT_BOOL deleteAgeoutClientEntries[CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E];

    /** @brief Input to the Aging scan, indicating aged-out specific keySize entries to delete
     *        GT_TRUE   - Delete the aged-out specific keySize entries
     *        GT_FALSE  - Do not delete the aged-out specific keySize entries.
     */
    GT_BOOL deleteAgeoutKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E];

}CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC;


/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC
 *
 * @brief Defines the Exact Match counters :
 *  1. per entry type counters
 *  2. summary counters
 *  3. per bank counter
 *
*/
typedef struct
{
    /* counter arrays for actionType and keySize */
    GT_U32 ttiClientKeySizeEntriesArray[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E];
    GT_U32 ipcl0ClientKeySizeEntriesArray[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E];
    GT_U32 ipcl1ClientKeySizeEntriesArray[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E];
    GT_U32 ipcl2ClientKeySizeEntriesArray[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E];
    GT_U32 epclClientKeySizeEntriesArray[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E];

    /* Summary of all of the above
       each above entry hold different number of index according to the keySize
       5 Bytes key size  = 1 index
       19 Bytes key size = 2 index
       33 Bytes key size = 3 index
       47 Bytes key size = 4 index */
    GT_U32  usedEntriesIndexes;

    /* The number of potential new entries to be inserted to the manager.
       calculated as the Max total entries requested by application in Create API
       minus used entries */
    GT_U32  freeEntriesIndexes;

    /* counter per bank */
    GT_U32  bankCounters[CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS];

}CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC;

/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC
 *
 * @brief Defines the Exact Match statistics per operations
 *
*/
typedef struct
{
    /* Entry successfully added with rehashing stage 0*/
    GT_U32 entryAddOkRehashingStage0;
    /* Entry successfully added with rehashing stage 1*/
    GT_U32 entryAddOkRehashingStage1;
    /* Entry successfully added with rehashing stage 2*/
    GT_U32 entryAddOkRehashingStage2;
    /* Entry successfully added with rehashing stage 3*/
    GT_U32 entryAddOkRehashingStage3;
    /* Entry successfully added with rehashing stage 4*/
    GT_U32 entryAddOkRehashingStage4;
    /* Entry successfully added with rehashing stage > 4*/
    GT_U32 entryAddOkRehashingStageMoreThanFour;
    /* Entry not added - input error */
    GT_U32 entryAddErrorInputInvalid;
    /* Entry not added - bad state */
    GT_U32 entryAddErrorBadState;
    /* Entry not added - entry exist */
    GT_U32 entryAddErrorEntryExist;
    /* Entry not added - table full */
    GT_U32 entryAddErrorTableFull;
    /* Entry not found in HA replay process */
    GT_U32 entryAddErrorReplayEntryNotFound;

    /* Entry successfully updated */
    GT_U32 entryUpdateOk;
    /* Entry not updated - input error */
    GT_U32 entryUpdateErrorInputInvalid;
    /* Entry not updated - entry not found */
    GT_U32 entryUpdateErrorNotFound;

    /* Entry successfully deleted */
    GT_U32 entryDeleteOk;
    /* Entry not deleted -  input error */
    GT_U32 entryDeleteErrorInputInvalid;
    /* Entry not deleted - entry not found */
    GT_U32 entryDeleteErrorNotfound;

    /* Scan delete successfully */
    GT_U32 scanDeleteOk;
    /* Scan delete successfully - no more */
    GT_U32 scanDeleteOkNoMore;
    /* Scan delete error - input error */
    GT_U32 scanDeleteErrorInputInvalid;
    /* Scan delete - total transplanted entries */
    GT_U32 scanDeleteTotalDeletedEntries;

    /* Scan aging successfully */
    GT_U32 scanAgingOk;
    /* Scan aging - input error */
    GT_U32 scanAgingErrorInputInvalid;
    /* Scan aging - total aged out entries */
    GT_U32 scanAgingTotalAgedOutEntries;
    /* Scan aging - total aged out deleted entries */
    GT_U32 scanAgingTotalAgedOutDeleteEntries;

    /* Entries rewrite ok */
    GT_U32 entriesRewriteOk;
    /* Entries rewrite error - input error */
    GT_U32 entriesRewriteErrorInputInvalid;
    /* Entries rewrite total rewritten entries */
    GT_U32 entriesRewriteTotalRewrite;

}CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC;

/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC
*
*  @brief Defines the Exact Match DB checks need to be done
*
*/
typedef struct
{
    /* @brief : Indicates Exact Match manager to check - Global parameters */
    GT_BOOL         globalCheckEnable;

    /* @brief : Indicates Exact Match manager to check - Counters */
    GT_BOOL         countersCheckEnable;

    /* @brief : Indicates Exact Match manager to check - Database Free List */
    GT_BOOL         dbFreeListCheckEnable;

    /* @brief : Indicates Exact Match manager to check - Database Used List */
    GT_BOOL         dbUsedListCheckEnable;

    /* @brief : Indicates Exact Match manager to check - Database index pointers */
    GT_BOOL         dbIndexPointerCheckEnable;

    /* @brief : Indicates Exact Match manager to check - Database aging bin */
    GT_BOOL         dbAgingBinCheckEnable;

    /* @brief : Indicates Exact Match manager to check - Database aging bin Usage matrix */
    GT_BOOL         dbAgingBinUsageMatrixCheckEnable;

    /* @brief : Indicates Exact Match manager to check - HW Used List */
    GT_BOOL         hwUsedListCheckEnable;

    /* @brief : Indicates Exact Match manager to check - Cuckoo Database compare to manager Database Used List */
    GT_BOOL         cuckooDbCheckEnable;

}CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC;

/**
* @struct CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT
*      Defines the Exact Match DB checks results
*
*/
typedef enum
{
    /* @brief : Indicates Exact Match manager "global parameters check" test failed */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E,

    /* @brief : Indicates Exact Match manager "counter" test failed - (per type check) */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E,
    /* @brief : Indicates Exact Match manager "counter" test failed - (per Bank check) */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E,
    /* @brief : Indicates Exact Match manager "cuckoo counter" test failed - counter mismatch */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_BANK_COUNTERS_MISMATCH_E,

    /* @brief : Indicates Exact Match manager "Free List check" test failed - 1st pointer invalid */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E,
    /* @brief : Indicates Exact Match manager "Free List check" test failed - counter number & list entries mismatch */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,

    /* @brief : Indicates Exact Match manager "Used List check" test failed - 1st pointer invalid */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E,
    /* @brief : Indicates Exact Match manager "Used List check" test failed - Last pointer invalid */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E,
    /* @brief : Indicates Exact Match manager "Used List check" test failed - Get Next failed(List corrupted) */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E,
    /* @brief : Indicates Exact Match manager "Used List check" test failed - Delete scan INVALID pointer */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E,
    /* @brief : Indicates Exact Match manager "Used List check" test failed - TBD */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E,
    /* @brief : Indicates Exact Match manager "Used List check" test failed - List number mismatch */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E,

    /* @brief : Indicates Exact Match manager "Index List check" test failed - Index pointer pointing to a invalid entry */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E,
    /* @brief : Indicates Exact Match manager "Index List check" test failed - HW index is invalid */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E,

    /* @brief : Indicates Exact Match manager "Aging bin check" test failed - age-bin counter mismatch with entries present */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,
    /* @brief : Indicates Exact Match manager "Aging bin check" test failed - total age-bin counter mismatch with total entries present */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E,
    /* @brief : Indicates Exact Match manager "Aging bin check" test failed - Entry age-bin number mismatch with the age-bin its connected */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E,
    /* @brief : Indicates Exact Match manager "Aging bin check" test failed - age-bin last scan pointer invalid */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E,

    /* @brief : Indicates Exact Match manager "Aging bin usage matrix" test failed - L1 bitmap not matching with present entries and age-bin counter */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E,
    /* @brief : Indicates Exact Match manager "Aging bin usage matrix" test failed - L2 bitmap not matching with L1 bitmaps */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E,

    /* @brief : Indicates Exact Match manager "HW Entries - Used List" test failed - Invalid Entry */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E,

    /* @brief : Indicates Exact Match manager "Lookup HW client" test failed - Invalid lookup client */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E,

    /* @brief : Indicates Exact Match manager "Lookup HW profileId" test failed - Invalid lookup profileId */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_PROFILE_ID_INVALID_E,

    /* @brief : Indicates Exact Match manager "Lookup HW key parameters" test failed - Invalid lookup key parameters */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_KEY_PARAMS_INVALID_E,

    /* @brief : Indicates Exact Match manager "Lookup HW default action enable" test failed - Invalid lookup default action enable flag */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_ENABLE_INVALID_E,

    /* @brief : Indicates Exact Match manager "Lookup HW default action" test failed - Invalid lookup default action */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_INVALID_E,

    /* @brief : Indicates Exact Match manager "Expander HW Entries - Used List" test failed - Invalid Enpander entry */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_EXPANDER_CONFIG_INVALID_E,

    /* @brief : Indicates Exact Match manager "Cuckoo DB Used List Check" test failed - Invalid Entry */
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_ENTRY_INVALID_E,

    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E
}CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChExactMatchManagerTypes_h */


