/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalFdb.h
*
* @brief Internal header which defines API for helpers functions
* which are specific for XPS FDB Mac Table .
*
* @version   01
********************************************************************************
*/
#ifndef __cpssHalFdbh
#define __cpssHalFdbh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtOs/gtGenTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSecurityBreach.h>
#include "xpsFdb.h"

//#include <cpss/cpss/common/h/config/private/prvCpssConfigTypes.h>

/**
* @enum PRV_XPS_BRG_FDB_ACCESS_MODE_ENT
 *
 * @brief This enum defines access mode to the FDB table
*/
typedef enum
{

    /** 'by message' */
    PRV_XPS_BRG_FDB_ACCESS_MODE_BY_MESSAGE_E,

    /** 'by Index' */
    PRV_XPS_BRG_FDB_ACCESS_MODE_BY_INDEX_E

} PRV_XPS_BRG_FDB_ACCESS_MODE_ENT;

/**
* @enum XPS_FDB_ENTRY_TYPE_ENT
 *
 * @brief This enum defines the types of the FDB Entry
*/
typedef enum
{

    /** The entry is MAC Address */
    XPS_FDB_ENTRY_TYPE_MAC_ADDR_E,

    /** The entry is IPv4 Multicast */
    XPS_FDB_ENTRY_TYPE_IPV4_MCAST_E,

    /** The entry is IPv6 Multicast */
    XPS_FDB_ENTRY_TYPE_IPV6_MCAST_E,

    /** The entry is VPLS entry */
    XPS_FDB_ENTRY_TYPE_VPLS_E,

    /** The entry is IPv4 UC entry */
    XPS_FDB_ENTRY_TYPE_IPV4_UC_E,

    /** The entry is IPv6 UC address entry */
    XPS_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E,

    /** The entry is IPv6 UC data entry */
    XPS_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E,

    /** @brief The entry is MAC Address
     *  Double Tag FDB Lookup Key Mode is used
     */
    XPS_FDB_ENTRY_TYPE_MAC_ADDR_FID_VID1_E,

    /** @brief The entry is IPv4 Multicast
     *  Double Tag FDB Lookup Key Mode is used
     */
    XPS_FDB_ENTRY_TYPE_IPV4_MCAST_FID_VID1_E,

    /** @brief The entry is IPv6 Multicast
     *  Double Tag FDB Lookup Key Mode is used
     */
    XPS_FDB_ENTRY_TYPE_IPV6_MCAST_FID_VID1_E

} XPS_FDB_ENTRY_TYPE_ENT;

/**
* @struct XPS_MAC_ENTRY_KEY_STC
 *
 * @brief Key parameters of the MAC Address Entry
*/
typedef struct
{
    XPS_FDB_ENTRY_TYPE_ENT         entryType;
    GT_U32                             vid1;
    union
    {
        struct
        {
            GT_ETHERADDR     macAddr;
            GT_U16           vlanId;
        } macVlan;

        struct
        {
            GT_U8            sip[4];
            GT_U8            dip[4];
            GT_U16           vlanId;
        } ipMcast;

        struct
        {
            GT_U8            dip[4];
            GT_U32           vrfId;
        } ipv4Unicast;

        struct
        {
            GT_U8            dip[16];
            GT_U32           vrfId;
        } ipv6Unicast;

    } key;

} XPS_MAC_ENTRY_KEY_STC;

/* macro to set <currPortGroupsBmp> with the port group BMP that is relevant to globalPhyPortNum */
/* also set usePortGroupsBmp = GT_TRUE */
#define   XPS_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(devId, globalPhyPortNum) \
    {                                                                    \
        GT_U32  portGroupId;                                             \
        if(!PRV_CPSS_SIP_5_CHECK_MAC(devId))                      \
        {                                                                \
            portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devId, globalPhyPortNum); \
        }                                                                \
        else                                                             \
        {                                                                \
            portGroupId = PRV_CPSS_SIP5_GLOBAL_PHYSICAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devId, globalPhyPortNum); \
        }                                                                \
        portGroupsBmp    = (1 << portGroupId);                       \
    }

/* macro to get the total number of port groups per device.*/
#define XPS_GET_PORT_NUMBER_OF_PORT_GROUPS(devId) \
    PRV_CPSS_PP_MAC(devId)->portGroupsInfo.numOfPortGroups


/******************************************************************************\
 *                            XPS FDB MAC Entry APIs                         *
\******************************************************************************/

/**
* @internal cpssHalBrgFdbMacEntrySet function
* @endinternal
*
* @brief   Create new or update existing entry in Hardware MAC address table
* @param[in] devId         - Device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr              - (pointer to) mac table
* @param[in] fdbAccessMode        - access mode to the FDB table
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalBrgFdbMacEntrySet
(
    int                       devId,
    XPS_FDB_ENTRY_TYPE_ENT    fdbEntryType,
    void                      *macEntryPtr,
    GT_U8                     fdbAccessMode
);

/**
* @internal cpssHalBrgFdbMacEntryGet function
* @endinternal
*
* @brief   Get existing entry in Hardware MAC address table
*
* @param[in] devId         - Device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryKeyPtr           - (pointer to) mac entry key
* @param[out] macEntryPtr              - (pointer to) mac table entry
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalBrgFdbMacEntryGet
(
    int                       devId,
    XPS_FDB_ENTRY_TYPE_ENT    fdbEntryType,
    void                      *macEntryKeyPtr,
    void                      *macEntryPtr
);

/**
* @internal cpssHalBrgFdbMacEntryIndexFind_MultiHash function
* @endinternal
*
* @brief   for CRC multi hash mode.
*         function calculate hash index for this mac address , and then start to read
*         from HW the entries to find an existing entry that match the key.
* @param[in] devId         - Device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryKeyPtr           - (pointer to) Fdb mac
*       Entry
*
* @param[out] indexPtr                 - (pointer to) :
*                                      when GT_OK (found) --> index of the entry
*                                      when GT_NOT_FOUND (not found) --> first index that can be used
*                                      when GT_FULL (not found) --> index first index
*                                      on other return value --> not relevant
* @param[out] bankIndexPtr         - Bank index. Used to reference IPv6 Data in
*                                      the ipv6 Address
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found, but bucket is not FULL
* @retval GT_FULL                  - entry not found, but bucket is FULL
* @retval GT_BAD_PARAM             - wrong devId.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssHalBrgFdbMacEntryIndexFind_MultiHash
(
    int                       devId,
    XPS_FDB_ENTRY_TYPE_ENT    fdbEntryType,
    void                     *macEntryKeyPtr,
    GT_U32                   *indexPtr,
    GT_U32                   *bankIndexPtr
);

/**
* @internal cpssHalBrgFdbMacEntryHashCalc function
* @endinternal
*
* @brief   calculate the hash index for the key
*         NOTE:
*         see also function cpssHalBrgFdbMacEntryHashCalc(...)
*         that find actual place according to actual HW capacity
* @param[in] devId         - Device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryKeyPtr           - (pointer to) mac entry key
*
* @param[out] indexPtr                 - (pointer to) index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalBrgFdbMacEntryHashCalc
(
    int                       devId,
    XPS_FDB_ENTRY_TYPE_ENT    fdbEntryType,
    void                      *macEntryKeyPtr,
    GT_U32                    *indexPtr
);

/**
* @internal cpssHalBrgFdbMacEntryIndexFind function
* @endinternal
*
* @brief   function calculate hash index for this mac address , and then start to read
*         from HW the entries to find an existing entry that match the key.
* @param devId               - Device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr           - (pointer to) mac entry key
* @param[out] indexPtr                 - (pointer to) :
*                                      when GT_OK (found) --> index of the entry
*                                      when GT_NOT_FOUND (not found) --> first index that can be used
*                                      when GT_FULL (not found) --> index od start of bucket
*                                      on other return value --> not relevant
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found, but bucket is not FULL
* @retval GT_FULL                  - entry not found, but bucket is FULL
* @retval GT_BAD_PARAM             - wrong devId.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssHalBrgFdbMacEntryIndexFind
(
    int                       devId,
    XPS_FDB_ENTRY_TYPE_ENT    fdbEntryType,
    void                     *macEntryKeyPtr,
    GT_U32                   *indexPt,
    GT_U32                   *bankIndexPtr
);

/**
* @internal cpssHalBrgFdbMacEntryDelete function
* @endinternal
*
* @brief   Delete an old entry in Hardware MAC address table
*
* @param[in] devId         - Device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryKeyPtr           - (pointer to) key parameters of the mac entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devId or vlanId
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalBrgFdbMacEntryDelete
(
    int                        devId,
    XPS_FDB_ENTRY_TYPE_ENT     fdbEntryType,
    void                       *macEntryKeyPtr
);

/**
* @internal cpssHalBrgFdbMacEntryWrite function
* @endinternal
*
* @brief   Write the new entry in Hardware MAC address table in specified index.
*
* @param[in] devId            - device Id
* @param[in] index                    - hw mac entry index
* @param[in] skip                     - entry  control
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devId,saCommand,daCommand
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalBrgFdbMacEntryWrite
(
    int                           devId,
    GT_U32                        index,
    GT_BOOL                       skip,
    XPS_FDB_ENTRY_TYPE_ENT        fdbEntryType,
    void *macEntryPtr
);

/**
* @internal cpssHalBrgFdbMacEntryHwWrite function
* @endinternal
*
* @brief   Write the new entry in Hardware MAC address table in
*          specified index
*
* @param[in] CPSS Dev Num             - CPSS device number
* @param[in] index                    - hw mac entry index
* @param[in] skip                     - entry  control
*                                      GT_TRUE - used to "skip" the entry ,
*                                      the HW will treat this entry as "not used"
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,saCommand,daCommand
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalBrgFdbMacEntryHwWrite
(
    GT_U8                         cpssDevNum,
    GT_U32                        index,
    GT_BOOL                       skip,
    XPS_FDB_ENTRY_TYPE_ENT        fdbEntryType,
    void                          *macEntryPtr
);

/**
* @internal cpssHalBrgFdbMacEntryWrite_withBankIndex
* @endinternal
*
* @brief   Write the new entry in Hardware MAC address table in specified index giving bankIndex
*
* @param[in] devId            - device Id
* @param[in] index                    - hw mac entry index
* @param[in] bankIndex               - bank index
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devId,saCommand,daCommand
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalBrgFdbMacEntryWrite_withBankIndex
(
    int                           devId,
    GT_U32                        index,
    GT_U32                        bankIndex,
    GT_BOOL                       skip,
    XPS_FDB_ENTRY_TYPE_ENT        fdbEntryType,
    void                          *macEntryPtr
);

/**
* @internal cpssHalBrgFdbMacEntryRead function
* @endinternal
*
* @brief   Reads the new entry in Hardware MAC address table from specified index.
*
* @param[in] index                    - hw mac entry index
*
* @param[out] validPtr                 - (pointer to) is entry valid
* @param[out] skipPtr                  - (pointer to) is entry skip control
* @param[out] agedPtr                  - (pointer to) is entry aged
* @param[out] hwDevNumPtr              - (pointer to) is HW device Idassociated with the entry
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[out] macEntryPtr              - (pointer to) extended Mac table entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devId
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - index out of range
*/
GT_STATUS cpssHalBrgFdbMacEntryRead
(
    int                          devId,
    GT_U32                       index,
    GT_BOOL                      *validPtr,
    GT_BOOL                      *skipPtr,
    GT_BOOL                      *agedPtr,
    GT_HW_DEV_NUM                *hwDevNumPtr,
    XPS_FDB_ENTRY_TYPE_ENT       fdbEntryType,
    void                         *macEntryPtr
);

/**
* @internal cpssHalBrgFdbMacEntryInvalidate function
* @endinternal
*
* @brief   Invalidate an entry in Hardware MAC address table in specified index.
*         the invalidation done by resetting to first word of the entry
* @param[in] index                    - hw mac entry index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - index out of range
*/
GT_STATUS cpssHalBrgFdbMacEntryInvalidate
(
    int       devId,
    GT_U32    index
);

/**
* @internal cpssHalConvertGenericToDxChMacEntryKey function
* @endinternal
*
* @brief   Convert generic MAC entry key into device specific MAC entry key
*
* @param[in] devId              - Device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryKeyPtr           - (pointer to) MAC entry key parameters
* @param[in] macEntryKeyPtr           - (pointer to) MAC entry
*        Type
*
* @param[out] dxChMacEntryKeyPtr       - (pointer to) DxCh MAC entry key parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertGenericToDxChMacEntryKey
(
    int                         devId,
    XPS_FDB_ENTRY_TYPE_ENT      fdbEntryType,
    void                       *macEntryKeyPtr,
    CPSS_MAC_ENTRY_EXT_KEY_STC *dxChMacEntryKeyPtr
);

/**
* @internal cpssHalConvertGenericToDxChMacEntryKey function
* @endinternal
*
* @brief   Convert generic MAC entry key into device specific MAC entry key
*
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryKeyPtr           - (pointer to) MAC entry key parameters
* @param[in] macEntryKeyPtr           - (pointer to) MAC entry
*        Type
*
* @param[out] dxChMacEntryKeyPtr       - (pointer to) DxCh MAC entry key parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertGenericToDxChMacEntryKey
(
    int                        devId,
    XPS_FDB_ENTRY_TYPE_ENT     fdbEntryType,
    void                       *macEntryKeyPtr,
    CPSS_MAC_ENTRY_EXT_KEY_STC *dxChMacEntryKeyPtr
);

/**
* @internal cpssHalConvertXpsFdbEntryTypeToDxChMacEntryType function
* @endinternal
*
* @brief   return the FDB Mac
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] fdbEntryType           - XPS FDB Entry Type
*
* @param[out] dxChMacEntryType          - (pointer to)
*       dxChMacEntry Type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertXpsFdbEntryTypeToDxChMacEntryType
(
    GT_U8 fdbEntryType,
    GT_U8 *dxChMacEntryType
);

/**
* @internal cpssHalConvertL3HostEntryToDxChMacEntry function
* @endinternal
*
* @brief   Convert generic MAC entry into device specific MAC entry
*
* @param[in] devId                   - device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @param[out] dxChMacEntryPtr          - (pointer to) DxCh MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertL3HostEntryToDxChMacEntry
(
    int                          devId,
    XPS_FDB_ENTRY_TYPE_ENT       fdbEntryType,
    void                         *macEntryPtr,
    CPSS_MAC_ENTRY_EXT_STC       *dxChMacEntryPtr
);

/**
* @internal cpssHalConvertGenericToDxChMacEntry function
* @endinternal
*
* @brief   Convert generic MAC entry into device specific MAC entry
*
* @param[in] devId                   - device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @param[out] dxChMacEntryPtr          - (pointer to) DxCh MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertGenericToDxChMacEntry
(
    int                          devId,
    XPS_FDB_ENTRY_TYPE_ENT       fdbEntryType,
    void *macEntryPtr,
    CPSS_MAC_ENTRY_EXT_STC       *dxChMacEntryPtr
);

/**
* @internal cpssHalConvertCpssTol3HostMacEntry function
* @endinternal
*
* @brief   Convert device specific MAC entry into generic MAC entry
*
* @param[in] cpssMacEntryPtr          - (pointer to) cpss MAC entry parameters
*
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[out] macEntryPtr              - (pointer to) MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertCpssTol3HostMacEntry
(
    int                     devId,
    CPSS_MAC_ENTRY_EXT_STC *cpssMacEntryPtr,
    XPS_FDB_ENTRY_TYPE_ENT  fdbEntryType,
    void *macEntryPtr
);

/**
* @internal cpssHalConvertCpssToGenericMacEntry function
* @endinternal
*
* @brief   Convert device specific MAC entry into generic MAC entry
*
* @param[in] cpssMacEntryPtr          - (pointer to) cpss MAC entry parameters
*
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[out] macEntryPtr              - (pointer to) MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalConvertCpssToGenericMacEntry
(
    int                     devId,
    CPSS_MAC_ENTRY_EXT_STC  *cpssMacEntryPtr,
    XPS_FDB_ENTRY_TYPE_ENT  fdbEntryType,
    void                    *macEntryPtr
);

/**
* @internal cpssHalBrgFdbMacKeyCompare function
* @endinternal
*
* @brief   compare 2 mac keys.
*
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] key1Ptr                  - (pointer to) key 1
* @param[in] key2Ptr                  - (pointer to) key 2
*                                      OUTPUTS:
*                                      > 0  - if key1Ptr is  bigger than key2Ptr
*                                      == 0 - if key1Ptr is equal to str2
*                                      < 0  - if key1Ptr is smaller than key2Ptr
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
* @retval GT_TIMEOUT               - on timeout waiting for the QR
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devId
* @retval GT_FAIL                  - on error
*/
int cpssHalBrgFdbMacKeyCompare
(
    int                        devId,
    XPS_FDB_ENTRY_TYPE_ENT     fdbEntryType,
    void                       *key1Ptr,
    CPSS_MAC_ENTRY_EXT_KEY_STC *dxChMacEntryKey2
);

/**
 * @internal cpssHalSetBrgFdbMacVlanLookupMode function
 * @endinternal
 *
 * @brief   Sets the VLAN Lookup mode.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                  - device Id
 * @param[in] mode                     - lookup mode:
 *                                      CPSS_IVL_E - Both MAC and Vlan are used for MAC table lookup
 *                                      and learning.
 *                                      CPSS_SVL_E - Only MAC is used for MAC table lookup and learning.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on bad param
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalSetBrgFdbMacVlanLookupMode
(
    int              devId,
    CPSS_MAC_VL_ENT  mode
);

/**
 * @internal cpssHalSetBrgSecurBreachEventPacketCommand function
 * @endinternal
 *
 * @brief   Set for given security breach event it's packet command.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                     - device Id
 * @param[in] eventType                - security breach event type.
 *                                      The acceptable events are:
 *                                      CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
 *                                      CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
 *                                      CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E,
 *                                      CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
 *                                      CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E,
 *                                      CPSS_BRG_SECUR_BREACH_EVENTS_FRAME_TYPE,
 *                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *                                      CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
 *                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *                                      CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
 *                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *                                      CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E.
 *                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 * @param[in] command                  - packet command. (APPLICABLE VALUES:
 *                                      CPSS_PACKET_CMD_DROP_HARD_E;
 *                                      CPSS_PACKET_CMD_DROP_SOFT_E)
 *                                      for Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X : also (APPLICABLE VALUES:
 *                                      CPSS_PACKET_CMD_FORWARD_E;
 *                                      CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
 *                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E)
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - bad input parameters.
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS cpssHalSetBrgSecurBreachEventPacketCommand
(
    int                                  dev,
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT     eventType,
    CPSS_PACKET_CMD_ENT                  command
);

/**
 * @internal cpssHalSetBrgFdbActionsEnable function
 * @endinternal
 *
 * @brief   Enables/Disables FDB actions.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                  - device Id
 * @param[in] enable                   - GT_TRUE -  Actions are enabled
 *                                      GT_FALSE -  Actions are disabled
 *
 * @retval GT_OK                    - on success
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - on wrong input parameters
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalSetBrgFdbActionsEnable
(
    int      devId,
    GT_BOOL  enable
);

/**
  * @internal cpssHalSetBrgFdbNaMsgOnChainTooLong function
  * @endinternal
  *
  * @brief   Enable/Disable sending NA messages to the CPU indicating that the device
  *         cannot learn a new SA . It has reached its max hop (bucket is full).
  *
  * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
  * @note   NOT APPLICABLE DEVICES:  None.
  *
  * @param[in] devId                  - device Id
  * @param[in] enable                   - GT_TRUE  sending the message to the CPU
  *                                      GT_FALSE for disable
  *
  * @retval GT_OK                    - on success
  * @retval GT_FAIL                  - on other error.
  * @retval GT_HW_ERROR              - on hardware error
  * @retval GT_BAD_PARAM             - wrong devId
  * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
  */
GT_STATUS cpssHalSetBrgFdbNaMsgOnChainTooLong
(
    int      devId,
    GT_BOOL  enable
);

/**
 * @internal cpssHalSetBrgFdbAAandTAToCpu function
 * @endinternal
 *
 * @brief   Enable/Disable the PP to/from sending an AA and TA address
 *         update messages to the CPU.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                  - device Id
 * @param[in] enable                   -  or disable the message
 *
 * @retval GT_OK                    - on success
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - wrong devIdor auMsgType
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 * @note for multi-port groups device :
 *       Whenever applicable it's highly recommended disabling these messages,
 *       as enabling this would stress the AUQ with excessive messages
 *
 */
GT_STATUS cpssHalSetBrgFdbAAandTAToCpu
(
    int                          devId,
    GT_BOOL                      enable
);

/**
 * @internal cpssHalSetBrgFdbAuMsgRateLimit function
 * @endinternal
 *
 * @brief   Set Address Update CPU messages rate limit
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                  - device Id
 * @param[in] msgRate                  - maximal rate of AU messages per second
 * @param[in] enable                   - enable/disable message limit
 *                                      GT_TRUE - message rate is limited with msgRate
 *                                      GT_FALSE - there is no limitations, WA to CPU
 *
 * @retval GT_OK                    - on success.
 * @retval GT_HW_ERROR              - on error.
 * @retval GT_OUT_OF_RANGE          - out of range.
 * @retval GT_BAD_PARAM             - on bad parameter
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 * @note granularity - 200 messages,
 *       rate limit - 0 - 51000 messages,
 *       when DxCh Clock 200 MHz(DxCh2 220 MHz Clock) ,
 *       granularity and rate changed linearly depending on clock.
 *       for multi-port groups device :
 *       The rate-limiting is done individually in each one of the port Groups.
 *       This means that potentially configuring this value would result a rate
 *       which is up to n times bigger then expected (n is number of port groups)
 *
 */
GT_STATUS cpssHalSetBrgFdbAuMsgRateLimit
(
    int                         devId,
    GT_U32                      msgRate,
    GT_BOOL                     enable
);

/**
 * @internal cpssHalSetBrgFdbMacTriggerMode function
 * @endinternal
 *
 * @brief   Sets Mac address table Triggered\Automatic action mode.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                  - device Id
 * @param[in] mode                     - action mode:
 *                                      CPSS_ACT_AUTO_E - Action is done Automatically.
 *                                      CPSS_ACT_TRIG_E - Action is done via a trigger from CPU.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - on wrong mode
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalSetBrgFdbMacTriggerMode
(
    int                          devId,
    CPSS_MAC_ACTION_MODE_ENT     mode
);

/**
 * @internal cpssHalSetDxChBrgFdbActionMode function
 * @endinternal
 *
 * @brief   Sets FDB action mode without setting Action Trigger
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                  - device Id
 * @param[in] mode                     - FDB action mode
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on bad device or mode
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_STATE             - the setting not allowed in current state of
 *                                       configuration.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 * @note for multi-port groups device :
 *       set configuration to all portGroups with same value.
 *       1. Age with delete (may be Triggered or Auto)
 *       a. Unified FDBs mode and Unified-Linked FDB mode :
 *       This type of operation is not supported , due to the control
 *       learning being used, the risk of AA messages being dropped & the
 *       need to avoid dropping MAC entries from portGroups which are not
 *       the source of these entries (meaning they would never be refreshed)
 *       API returns GT_BAD_STATE if trying to use this action mode.
 *       b. Linked FDBs mode :
 *       Application should use this type of operation only in Automatic mode
 *       (not in Controlled mode) + Static FDB entries .
 *       In this mode automatic learnt addresses would be silently removed
 *       by PP while application addresses would be removed only by specific
 *       application API call.
 *       2. Age without delete (may be Triggered or Auto)
 *       The only type of Aging that can be used with Unified FDBs mode and
 *       Unified-Linked FDB mode (in Linked FDBs mode it should be used when
 *       using controlled mode), AA would be sent by PP to Application that
 *       would remove entry from relevant portGroups.
 *       3. Transplant (only Triggered)
 *       a. Unified-Linked FDB mode and Linked FDB mode :
 *       This action mode is not supported - API returns GT_BAD_STATE
 *       because triggering this would possibly result with entries which
 *       have their new 'source' information properly set - but these entries
 *       would not be relocated to the new source portGroups, meaning that
 *       packet received from new station location (which was supposed to
 *       be transplanted), may still create NA message.
 *       Computing new source portGroup is not always possible, especially
 *       when station is relocated to trunk or it was previously residing
 *       on multiple-portGroups.
 *       Application is required to perform Flush operation instead.
 *       This would clear previously learnt MAC
 *       4. Delete (only Triggered)
 *       supported in all FDB modes
 *
 */
GT_STATUS cpssHalSetBrgFdbActionMode
(
    int                         devId,
    CPSS_FDB_ACTION_MODE_ENT     mode
);

/**
 * @internal cpssHalSetBrgFdbUploadEnable function
 * @endinternal
 *
 * @brief   Enable/Disable reading FDB entries via AU messages to the CPU.
 *         The API only configures mode of triggered action.
 *         To execute upload use the cpssDxChBrgFdbTrigActionStart.
 *
 * @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
 *
 * @param[in] devId                  - device Id
 * @param[in] enable                   - GT_TRUE -  FDB Upload
 *                                      GT_FALSE - disable FDB upload
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 * @note There is no check done of current status of action's register
 *
 */
GT_STATUS cpssHalSetBrgFdbUploadEnable
(
    int     devId,
    GT_BOOL enable
);

/**
 * @internal cpssHalSetBrgFdbHashMode function
 * @endinternal
 *
 * @brief   Sets the FDB hash function mode.
 *         The CRC based hash function provides the best hash index distribution
 *         for random addresses and vlans.
 *         The XOR based hash function provides optimal hash index distribution
 *         for controlled testing scenarios, where sequential addresses and vlans
 *         are often used.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                  - device Id
 * @param[in] mode                     - hash function based mode
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - wrong devIdor mode
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalSetBrgFdbHashMode
(
    int                          devId,
    CPSS_MAC_HASH_FUNC_MODE_ENT  mode
);

/**
 * @internal cpssHalSetBrgFdbDropAuEnable function
 * @endinternal
 *
 * @brief   Enable/Disable dropping the Address Update messages when the queue is
 *         full .
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                  - device Id
 * @param[in] enable                   - GT_TRUE - Drop the address update messages
 *                                      GT_FALSE - Do not drop the address update messages
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalSetBrgFdbDropAuEnable
(
    int      devId,
    GT_BOOL  enable
);

/**
 * @internal cpssHalSetBrgFdbAgeBitDaRefreshEnable function
 * @endinternal
 *
 * @brief   Enables/disables destination address-based aging. When this bit is
 *         set, the aging process is done both on the source and the destination
 *         address (i.e. the age bit will be refresh when MAC DA hit occurs, as
 *         well as MAC SA hit).
 *
 * @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
 *
 * @param[in] devId                  - device Id
 * @param[in] enable                   - GT_TRUE -  refreshing
 *                                      GT_FALSE - disable refreshing
 *
 * @retval GT_OK                    - on success
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - on wrong devId
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *                                       COMMENTS
 * @retval for multi                -port groups device:
 * @retval Once enabled, MAC address is eligibly to be aged -out if no packets were
 *                                       received from it or sent to it from any other location during aging period.
 *                                       In order to properly compute this application is required to keep a
 *                                       'score board' for every regular entry (same as needed for Trunk entries),
 *                                       each bit representing age bit from each underlying portGroup which this
 * @retval entry is written to. Entry is aged -out if all relevant portGroups have
 * @retval sent AA, indicating entry was aged -out (once DA refresh is disabled -
 *                                       only the source portGroup indication interests us for aging calculations).
 *                                       Note:
 *                                       that it's highly recommended not to enable this feature, as this
 *                                       feature enables transmissions of many AA messages to the CPU
 */
GT_STATUS cpssHalSetBrgFdbAgeBitDaRefreshEnable
(
    int     devId,
    GT_BOOL enable
);

/**
 * @internal cpssHalEnableBrgGenDropInvalidSa function
 * @endinternal
 *
 * @brief   Drop all Ethernet packets with MAC SA that are Multicast (MACDA[40] = 1)
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                  - device Id
 * @param[in] enable                   - GT_TRUE,  filtering mode
 *                                      GT_FALSE, disable filtering mode
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device Id
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalEnableBrgGenDropInvalidSa
(
    int      devId,
    GT_BOOL  enable
);

/**
 * @internal cpssHalSetBrgSecurBreachMovedStaticAddr function
 * @endinternal
 *
 * @brief   When the FDB entry is signed as a static one, then this entry is not
 *         subject to aging. In addition, there may be cases where the interface
 *         does't match the interface from which this packet was received. In this
 *         case, this feature enables/disables to regard moved static adresses as
 *         a security breach event.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                     - physical device Id
 * @param[in] enable                   - GT_TRUE - moved static addresses are regarded as
 *                                      security breach event and are dropped.
 *                                      GT_FALSE - moved static addresses are not regarded
 *                                      as security breach event and are forwarded.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - bad input parameters.
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalSetBrgSecurBreachMovedStaticAddr
(
    int                  devId,
    GT_BOOL              enable
);

/**
 * @internal cpssHalSetBrgSecurBreachEventDropMode function
 * @endinternal
 *
 * @brief   Set for given security breach event it's drop mode.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                     - physical device Id
 * @param[in] eventType                - security breach event type.
 * @param[in] dropMode                 - soft or hard drop mode.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - bad input parameters.
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 * @note The acceptable events are:
 *       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E,
 *       CPSS_BRG_SECUR_BREACH_EVENTS_PORT_NOT_IN_VLAN_E,
 *       CPSS_BRG_SECUR_BREACH_EVENTS_VLAN_RANGE_DROP_E,
 *       CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
 *       CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E,
 *       CPSS_BRG_SECUR_BREACH_EVENTS_FRAME_TYPE,
 *       (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *       CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SPOOF_PROTECTION_E,
 *       (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *       CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E,
 *       (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *       CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E.
 *       (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 *
 */
GT_STATUS cpssHalSetBrgSecurBreachEventDropMode
(
    int                                  devId,
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT     eventType,
    CPSS_DROP_MODE_TYPE_ENT              dropMode
);

/**
 * @internal cpssHalSetDrvPpHwRegField function
 * @endinternal
 *
 * @brief   Write value to selected register field.
 *
 * @param[in] devId                    - The PP device Id to write to.
 * @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
 *                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
 *                                      (APPLICABLE DEVICES Lion; Lion2)
 * @param[in] regAddr                  - The register's address to write to.
 * @param[in] fieldOffset              - The start bit number in the register.
 * @param[in] fieldLength              - The number of bits to be written to register.
 * @param[in] fieldData                - Data to be written into the register.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on hardware error
 * @retval GT_NOT_INITIALIZED       - if the driver was not initialized
 * @retval GT_HW_ERROR              - on hardware error
 *
 * @note this function actually read the register modifies the requested field
 *       and writes the new value back to the HW.
 *
 */
GT_STATUS cpssHalSetDrvPpHwRegField
(
    int      devId,
    GT_U32   portGroupId,
    GT_U32   regAddr,
    GT_U32   fieldOffset,
    GT_U32   fieldLength,
    GT_U32   fieldData
);


/**
* @internal cpssHalSetBrgFdbHashMode function
* @endinternal
*
* @brief   Sets the FDB hash function mode.
*         The CRC based hash function provides the best hash index distribution
*         for random addresses and vlans.
*         The XOR based hash function provides optimal hash index distribution
*         for controlled testing scenarios, where sequential addresses and vlans
*         are often used.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devId                   - device number
* @param[in] mode                     - hash function based mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/


GT_STATUS cpssHalSetBrgFdbHashMode
(
    int                          devId,
    CPSS_MAC_HASH_FUNC_MODE_ENT  mode
);



/**
* @internal cpssHalSetBrgFdbRoutingNextHopPacketCmd function
* @endinternal
*
* @brief   Set Packet Commands for FDB routed packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
* @param[in] devId                   - device number
* @param[in] nhPacketCmd              - Route entry command. supported commands:
*                                      CPSS_PACKET_CMD_ROUTE_E,
*                                      CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
*                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E,
*                                      CPSS_PACKET_CMD_DROP_SOFT_E ,
*                                      CPSS_PACKET_CMD_DROP_HARD_E,
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/


GT_STATUS cpssHalSetBrgFdbRoutingNextHopPacketCmd
(
    int      devId,
    CPSS_PACKET_CMD_ENT         nhPacketCmd
);

GT_STATUS cpssHalFlushFdbEntry
(
    int devId,
    xpsFlushParams* flushParams
);


/**
* @internal cpssHalSetBrgFdbPortLearning function
* @endinternal
*
* @brief   Set FDB manager port learning.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devId                 - device number
* @param[in] portNum               - port number
* @param[in] unkSrcCmd             - Unknown src packet command
* @param[in] naMsgToCpuEnable      - NA message to CPU enable
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/


GT_STATUS cpssHalSetBrgFdbPortLearning
(
    int      devId,
    GT_U32   portNum,
    GT_U32   unkSrcCmd,
    GT_BOOL  naMsgToCpuEnable
);

/**
* @internal cpssHalGetBrgFdbPortLearning function
* @endinternal
*
* @brief   Get FDB manager port learning.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devId                 - device number
* @param[in] portNum               - port number
* @param[in] unkSrcCmd             - Unknown src packet command
* @param[in] naMsgToCpuEnable      - NA message to CPU enable
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/


GT_STATUS cpssHalGetBrgFdbPortLearning
(
    int      devId,
    GT_U32   portNum,
    GT_U32   *unkSrcCmd,
    GT_BOOL  *naMsgToCpuEnable
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssHalFdbh */


