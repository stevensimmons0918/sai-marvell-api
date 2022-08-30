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
* @file cpssDxChBrgFdbManagerTypes.h
*
* @brief FDB manager support - API level data type definitions
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChBrgFdbManagerTypes_h
#define __cpssDxChBrgFdbManagerTypes_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/ip/cpssIpTypes.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>

/** max number of managers supported */
#define CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS    32

/**
* @enum CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT
 *
 * @brief An enum for FDB manager to indicate the Shadow DB type.
 *
*/
typedef enum{
     /** @brief SIP6 type of DB,
      *  Upto 256K entries, upto 16 banks and multi hash value
      *  APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman */
    CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E,

    /** @brief SIP6 type of DB,
      *  Upto 256K entries, supports only 16 banks and multi hash mode
      *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X */
    CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E,

     /** @brief SIP6 and SIP5 hybrid type of DB,
      *  Upto 256K entries, supports only 16 banks and multi hash mode
      *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman */
    CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E,

    /** @brief SIP4 type of DB,
     *  Upto 16K entries, with 4 banks and single hash value
     *  APPLICABLE DEVICES: AC5 */
    CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E
} CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_HW_CAPACITY_STC
 *
 * @brief Defines the FDB HW capacity parameters (for 'manager create')
 *
*/
typedef struct{
    /** number of indexes in HW : 8K,16K,32K,64K,128K,256K */
    GT_U32   numOfHwIndexes;
    /** @brief number of hashes (banks) : 16,8,4
     *  For SIP6 devices, the number of hashes depends on shared table mode (FDB size):
     *  For SIP5 devices, 16 Hashes only supported
     *  In case of SIP5_SIP6_HYBRID shadow type - 16 Hashes only supported
     *  FDB 8K, 16K and 32K     -  4 hashes(banks)
     *  FDB 64K                 -  8 hashes(banks)
     *  FDB 128K and 256K       - 16 hashes(banks)
     *  For AC5 device          -  1 Hash(4 Banks) */
    GT_U32   numOfHashes;

    /** @brief: Hash function modes
     *  Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon, AC5X, AC5P ignores this field
     *  APPLICABLE DEVICES: AC5 */
    CPSS_MAC_HASH_FUNC_MODE_ENT mode;
}CPSS_DXCH_BRG_FDB_MANAGER_HW_CAPACITY_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC
 *
 * @brief Defines the FDB capacity parameters (SW and HW) (for 'manager create')
 *
*/
typedef struct{
    /** @brief: Set of parameters that defines the HW capacity */
    CPSS_DXCH_BRG_FDB_MANAGER_HW_CAPACITY_STC  hwCapacity;
    /** @brief: Maximal total entries application would like to support (can be lower than HW capacity)
       Value must be in steps of 256 entries.(but not ZERO)
       Note: IPv6 UC are counted as two entries */
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
    /** @brief: Maximal number of entries that will be processed per transplant scan call
       (Not all necessary be transplanted).
       Value must be in steps of 256 entries (but not ZERO) and must be equal or smaller than maxTotalEntries.
       See transplant scan for details.*/
    GT_U32  maxEntriesPerTransplantScan;
    /** @brief: Maximal number of entries that will be processed per learning scan call
       (the number of NA messages processed).
       Value must be in steps of 256 entries (but not ZERO) and must be equal or smaller than maxTotalEntries.
       See learning scan for details.*/
    GT_U32  maxEntriesPerLearningScan;

    /** @brief: indication that 'learning limits' feature is enabled/disabled.
     *          GT_TRUE  - the 'learning limits' feature is enabled.
     *          GT_FALSE - the 'learning limits' feature is disabled and no
     *                      DB allocation done for it.
     */
    GT_BOOL enableLearningLimits;
    /** @brief: FDB Manager 'learning limits' - Global .
     * NOTE : the limit for all Dynamic UC Macs.
     */
    GT_U32  maxDynamicUcMacGlobalLimit;
    /** @brief: FDB Manager 'learning limits' - Fid.
     * NOTE : the default limit for Dynamic UC Macs in each FID unless specified explicitly.
     */
    GT_U32  maxDynamicUcMacFidLimit;
    /** @brief: FDB Manager 'learning limits' - Global-Eports.
     * NOTE : the default limit for Dynamic UC Macs in each Global Eport unless specified explicitly.
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
     */
    GT_U32  maxDynamicUcMacGlobalEportLimit;
    /** @brief: FDB Manager 'learning limits' - Trunks.
     * NOTE : the default limit for Dynamic UC Macs in each Trunk unless specified explicitly.
     */
    GT_U32  maxDynamicUcMacTrunkLimit;
    /** @brief: FDB Manager 'learning limits' - ports.
     * NOTE : the default limit for Dynamic UC Macs in each port unless specified explicitly.
     */
    GT_U32  maxDynamicUcMacPortLimit;

}CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC;

/**
* @enum CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_ENT
 *
 * @brief Defines the types of the limits for dynamic UC Mac entries.
 *
*/
typedef enum{
    /** global limit */
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_E,
    /** limit per FID */
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E,
    /** limit per trunkId */
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_TRUNK_E,
    /** limit per global-eport */
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E,
    /** limit per port */
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E,

    /** 'last enumeration value' not to used */
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE__LAST__E
}CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_ENT;

/**
* @union CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_UNT
 *
 * @brief Defines the 'indexes' for the types of the limits for dynamic UC Mac entries.
 *
*/
typedef union{
    /** for type CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E */
    GT_PORT_NUM globalEport;
    /** for type CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_TRUNK_E */
    GT_TRUNK_ID trunkId;
    /** for type CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E */
    GT_U32      fid;
    /** for type CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E */
    CPSS_INTERFACE_DEV_PORT_STC     devPort;
}CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_UNT;


/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC
 *
 * @brief Defines the types and the 'indexes' for the limits for dynamic UC Mac entries.
 *
*/
typedef struct{
    /** type of limit */
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_ENT limitType;
    /** index of limit */
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_UNT      limitIndex;
}CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC;

/**
* @enum CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_ENT
 *
 * @brief An enum to state FDB mac entry muxing modes.
 * how the 12 bits that are muxed in the FDB mac entry.
 *
 * APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
 * AC5 ignores this field
 *
*/
typedef enum{
    /** 12 bits of 'TAG1_VID' */
    CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E,
    /** 12 bits of 'SRC_ID' */
    CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E,
    /** 10 bits of UDB , 1 bit of 'SRC_ID' , 1 bit of 'DA_ACCESS_LEVEL' */
    CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_10_SRC_ID_1_DA_ACCESS_LEVEL_E,
    /** 8 bits of UDB , 3 bits of 'SRC_ID' , 1 bit of 'DA_ACCESS_LEVEL' */
    CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_8_SRC_ID_3_DA_ACCESS_LEVEL_E,
    /** not to be used */
    CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE__MUST_BE_LAST__E
} CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_ENT;

/**
* @enum CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ENT
 *
 * @brief An enum for FDB manager mirroring modes.
 *
*/
typedef enum{
    /** @brief DO not mirror the packet
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; AC5 */
    CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E,

    /** @brief Mirror to configured Analyzer port, on packets DA or SA match
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; AC5 */
    CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E,

     /** @brief Mirror to configured Analyzer port, on packets DA match
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; */
    CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_DA_E,

    /** @brief Mirror to configured Analyzer port, on packets SA match
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; */
    CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_E
} CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ENT;

/**
* @enum CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_ENT
 *
 * @brief An enum to state IPMC (IPv4/Ipv6 MC) entry muxing modes.
 * how the 7 bits that are muxed in the FDB IPMC (IPv4/Ipv6 MC) entry.
 *
 * APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
 * AC5 ignores this field
 *
*/
typedef enum{
    /** 7 bits of 'SRC_ID' */
    CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E,
    /** 4 bits of UDB , 3 bits of 'SRC_ID'*/
    CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_4_SRC_ID_3_E,
    /** 7 bits of UDB */
    CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_E,
    /** not to be used */
    CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE__MUST_BE_LAST__E
} CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_ENT;


/**
* @enum CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE
 *
 * @brief An enum to state entry muxing modes.
 * how the srcId length and tag1 VID are muxed in the FDB entry.
 *
 * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X.
 *
*/
typedef enum{
     /** @brief
      *  <SrcID>                - 9 bits
      *  <UDB>                  - 8 bits
      *  <Tag1 VID>             - Not supported
      *  <SA/DA Security Level> - Supported
      */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E,

     /** @brief
      *  <SrcID>                - 12 bits
      *  <UDB>                  - 5 bits
      *  <Tag1 VID>             - Not supported
      *  <SA/DA Security Level> - Supported
      */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E,

     /** @brief
      *  <SrcID>                - 6 bits
      *  <UDB>                  - 8 bits
      *  <Tag1 VID>             - Supported
      *  <SA/DA Security Level> - Not supported
      */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E,

     /** @brief
      *  <SrcID>                - 6 bits
      *  <UDB>                  - 5 bits
      *  <Tag1 VID>             - Supported
      *  <SA/DA Security Level> - Not supported
      */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E,

    /** not to be used */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE__MUST_BE_LAST__E
} CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_ENT;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC
 *
 * @brief Defines the FDB entry attributes (for 'manager create')
 *
*/
typedef struct{
    /** @brief Application can select how MAC entry MUX fields would be used (SRD-ID, TAG1 etc.)
     *  Note: if VID1 mode is selected VID1 assignment in NA messages should be internally enabled too.
     * APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman; */
    CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_ENT macEntryMuxingMode;

    /** @brief Application can select how IPMC entry MUX fields would be used (SRD-ID, UDB etc.)
     * APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman. */
    CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_ENT ipmcEntryMuxingMode;

    /** @brief Application can select how entry MUX fields would be used (SRD-ID, UDB etc.)
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_ENT     entryMuxingMode;

    /** @brief Application can decide how HW will treat entries SA commands - HARD or SOFT drops
     * APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman; */
    CPSS_PACKET_CMD_ENT                              saDropCommand;

    /** @brief Application can decide how HW will treat entries DA commands - HARD or SOFT drops
     * APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman. */
    CPSS_PACKET_CMD_ENT                              daDropCommand;

    /** @brief Application can decide how HW will treat routed IP packets - one of :
        CPSS_PACKET_CMD_ROUTE_E
        CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E
        CPSS_PACKET_CMD_TRAP_TO_CPU_E
        CPSS_PACKET_CMD_DROP_HARD_E
        CPSS_PACKET_CMD_DROP_SOFT_E
        APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
    */
    CPSS_PACKET_CMD_ENT                              ipNhPacketcommand;

    /** @brief: FDB Manager software shadow DB type */
    CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT        shadowType;

    /** @brief: FDB Manager 'Global Eports' range.
     * NOTE : should use the same value used for cpssDxChCfgGlobalEportSet() in globalPtr param
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
     */
    CPSS_DXCH_CFG_GLOBAL_EPORT_STC  globalEportInfo;

}CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC;

/**
* @enum CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_ENT
 *
 * @brief Defines the MAC entry hash types
 *
*/
typedef enum{
    /** MAC entry without FID for hash (FID must be ZERO) */
    CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_ONLY_E,
    /** @brief MAC entry with FID for hash
     * In case of AC5 - VID is used in place of FID */
    CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E,
    /** @brief MAC entry with FID for hash and with VID1 for hash
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman; */
    CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E
}CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_ENT;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC
 *
 * @brief Defines the FDB learning parameters (for 'manager create')
 *
*/
typedef struct{
    /** @brief Application can decide to enable or not sending to CPSS FDB Manager NA messages of MAC entries that
     *  could not be learnt due to hash collisions.
     *  Enabling this activates the Learning throttling mechanism - a logic in FDB Manager that internally
     *  controls the rate of AU messages and enable/ disable HW NA-chain-too-long.
     *  The motivation for enabling it is that FDB Manager will try to find other free location by using Cuckoo.
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman; */
    GT_BOOL macNoSpaceUpdatesEnable;

    /** @brief Application can select to globally enable or disable MAC learning of routed packets. */
    GT_BOOL  macRoutedLearningEnable;

    /** @brief Application can select (none, 1 or 2 VLANs) - SVL, IVL single tag or IVL double tag
        (key will be: MAC, MAC+FID, or MAC+FID+VID1 respectively)
        Affect entry format and how hashing is performed for FDB MAC addresses.
        Note: while selection between SVL and IVL is mapped to HW setting, the selection between single or double tag
        is done at the HW per eVLAN, regardless, we'll support it as global mode to overcome the missing indication in NA updates.
        Application that want to work in double Tag, must select the entry format that includes VLAN1 (check it during create API)
        and enable it per eVLAN (Application responsibility).
    */
    CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_ENT macVlanLookupMode;
}CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC;


/**
* @enum CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ENT
 *
 * @brief Enumeration how the multi-CRC hash will use 16 'most upper bits' for
 *  lookup key of type 'MAC_AND_FID' (CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E)
 * relevant when using CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E
*/
typedef enum{
    /** use 16 bits 0. */
    CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E,
    /** use 16 bits of FID. */
    CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_USE_FID_E,
    /** use 16 LSBits of MAC. */
    CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_USE_MAC_E
} CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ENT;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC
 *
 * @brief Defines the FDB lookup parameters (for 'manager create')
 * APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman;
 * Note - AC5 ignores these fields
 *
*/
typedef struct{
    /** @brief Application can select the method for selecting the 16 MSB bits out of
        the 80 hash bits, when working in IVL single tag (MAC+FID) MAC VLAN Lookup mode. */
    CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ENT  crcHashUpperBitsMode;
    /** @brief Application can set a global IPv4 UC route prefix length entries. HW will use
        this setting to look for entries which not match.
        (applicable values: 1..32)
    */
    GT_U32      ipv4PrefixLength;
    /** @brief Application can set a global IPv6 UC route prefix length entries. HW will use
        this setting to look for entries which not match.
        (applicable values: 1..127)
    */
    GT_U32      ipv6PrefixLength;
}CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC;


/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC
 *
 * @brief Defines the FDB aging/refresh parameters (for 'manager create')
 *
*/
typedef struct{
    /** @brief Application can Enable refresh by destination mac address - UC packets
        (in addition to refresh by source mac address , that is always done) */
    GT_BOOL         destinationUcRefreshEnable;
    /** @brief Application can Enable refresh by destination mac address - MC packets */
    GT_BOOL         destinationMcRefreshEnable;
    /** @brief Refresh IP UC FDB entries when entry is used for routing.
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman; */
    GT_BOOL         ipUcRefreshEnable;
}CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_DEV_PORT_STC
 *
 * @brief Defines the interfaces 'devPort' info for the FDB manager entry
 *
*/
typedef CPSS_INTERFACE_DEV_PORT_STC  CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_DEV_PORT_STC;

/**
* @union CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_UNT
 *
 * @brief Defines the interfaces info for the FDB manager entry : port/trunk/vidx
 *
*/
typedef union{
    /** for type CPSS_INTERFACE_PORT_E */
    CPSS_INTERFACE_DEV_PORT_STC devPort;
    /** for type CPSS_INTERFACE_TRUNK_E */
    GT_TRUNK_ID     trunkId;
    /** for type CPSS_INTERFACE_VIDX_E */
    GT_U16          vidx;
    /* NOTE: for type CPSS_INTERFACE_VID_E : no extra info needed ! */

    /* NOTE: other types not supported */
}CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_UNT;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC
 *
 * @brief Defines the interface info for the FDB manager entry
 *
*/
typedef struct{
    /** @brief the type of 'interface' that associated with the FDB entry .
       this supporting only port/trunk/vidx/vid interfaces */
    CPSS_INTERFACE_TYPE_ENT                            type;
    /** The 'interface' that associated with the FDB entry */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_UNT interfaceInfo;
}CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC
 *
 * @brief Fdb manager Entry format matching : CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E
*/
typedef struct {
    /** @brief the mac address of the entry.
        it is part of the 'hash key'
    */
    GT_ETHERADDR     macAddr;
    /** @brief the FDB ID that allow several vlans to share the same entries.
        it is part of the 'hash key'
        in case of AC5 - fid holds the VID value.
    */
    GT_U32           fid;
    /** @brief vid1 , optionally part of the 'hash key'
     *  NOTE: part of muxed fields (see macEntryMuxingMode , ipmcEntryMuxingMode)
     *  APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X;
     */
    GT_U32           vid1;

    /** @brief destination interface : port/trunk/vidx/vlan
     */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC dstInterface;

    /** @brief security level assigned to the MAC DA that matches this entry.
     *  (APPLICABLE RANGES: In case of SIP5 and SIP4 shadow type: 0..0x7;
     *                      In case of SIP5_SIP6_HYBRID & SIP6 shadow type: 0..1)
     *
     *  NOTE: part of muxed fields (see macEntryMuxingMode)
     */
    GT_U32 daSecurityLevel;

    /** @brief security level assigned to the MAC SA that matches this entry.
     *  (APPLICABLE RANGES: In case of SIP5 and SIP4 shadow type: 0..0x7;
     *                      In case of SIP5_SIP6_HYBRID & SIP6 shadow type: 0..1)
     */
    GT_U32 saSecurityLevel;

    /** @brief Source ID.
     *
     *  NOTE: part of muxed fields (see macEntryMuxingMode)
     */
    GT_U32 sourceID;

    /** @brief user defined field.
     *  NOTE: part of muxed fields (see macEntryMuxingMode)
     */
    GT_U32 userDefined;

    /** @brief action taken when a packet's DA matches this entry
     *
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; AC5; Falcon; AC5P; AC5X; Harrier; Ironman;
     *  Supported values: FORWARD, MIRROR_TO_CPU, TRAP_TO_CPU, DROP_HARD/SOFT_HARD
     *    IMPORTANT: In case of SIP6 & SIP5_SIP6_HYBRID shadow type
     *               using DROP_HARD/SOFT_HARD will act according
     *               CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC::daDropCommand
     */
    CPSS_PACKET_CMD_ENT daCommand;

    /** @brief action taken when a packet's SA matches this entry.
     *
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; AC5; Falcon; AC5P; AC5X; Harrier; Ironman;
     *  Supported values: FORWARD, MIRROR_TO_CPU, TRAP_TO_CPU, DROP_HARD/SOFT_HARD
     *    IMPORTANT: In case of SIP6 & SIP5_SIP6_HYBRID shadow type
     *               using DROP_HARD/SOFT_HARD will act according
     *               CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC::saDropCommand
     *               MIRROR_TO_CPU, TRAP_TO_CPU - is not applicable.
     */
    CPSS_PACKET_CMD_ENT saCommand;

    /** @brief static/dynamic entry
     */
    GT_BOOL isStatic;

    /** @brief Age flag that is used for the aging state. the Used for the two-step Aging process.
        The device sets this bit to 1 (refreshed) when this MAC Entry is accessed as a source MAC Address.
        GT_FALSE - Entry is marked for aging in next age pass.
        GT_TRUE  - Entry is marked for aging in two age-passes.
     */
    GT_BOOL age;

    /** @brief indication to do 'Routing' if mac DA matches
     */
    GT_BOOL daRoute;

    /** @brief If set, and the entry <DA command> is TRAP or MIRROR then the CPU
     * code may be overwritten according to the Application Specific CPU Code assignment
     *  mechanism.
     */
    GT_BOOL appSpecificCpuCode;

    /** @brief the packet is marked by the FDB for mirroring to the Analyzer port.
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; AC5
     *  APPLICABLE VALUES:  In case of SIP5_SIP6_HYBRID shadow type:
     *                          CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E
     *                      In case of SIP5 shadow type:
     *                          CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E
     *                          CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_DA_E
     *                          CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_E
     *                          CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E
     *                      In case of SIP4 shadow type:
     *                          CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E
     */
    CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ENT mirrorToAnalyzerPort;

    /** @brief QOS parameters update for MAC DA
     * If set to 0, no QoS parameters are modified,
     * when this entry matches the destination lookup for either MAC DA or IPv4/6 Multicast entry.
     * Else the parameters are taken from QoS parameter set<index>
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; AC5
     * APPLICABLE RANGES:  In case of SIP6 & SIP5_SIP6_HYBRID shadow type: 0
     *                     In case of SIP5 & SIP4 shadow type: 0..0x7
     */
    GT_U32  daQoSParameterSetIndex;

    /** @brief QOS parameters update for MAC SA
     * If set to 0, no QoS parameters are modified,
     * when this entry matches the destination lookup for either MAC SA or IPv4/6 Multicast entry.
     * Else the parameters are taken from QoS parameter set<index>
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; AC5
     * APPLICABLE RANGES:  In case of SIP6 & SIP5_SIP6_HYBRID shadow type: 0
     *                     In case of SIP5 & SIP4 shadow type: 0..0x7
     */
    GT_U32  saQoSParameterSetIndex;

}CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC
 *
 * @brief Fdb manager Entry format matching : CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E / CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E
*/
typedef struct {
    /** @brief the SIP address of the entry.
        it is a part of the 'hash key'
        IPv4 = SIP[31:0], where SIP[0] is the least significant bit of the IPv4 destination address.
        IPv6 = SelectedSIPByte0[7:0] and SelectedSIPByte1[7:0], SelectedSIPByte2[7:0] and SelectedSIPByte3[7:0]
    */
    GT_U8   sipAddr[4];

    /** @brief the DIP address of the entry.
        it is a part of the 'hash key'
        IPv4 = DIP[31:0], where DIP[0] is the least significant bit of the IPv4 destination address.
        IPv6 = SelectedDIPByte0[7:0] and SelectedDIPByte1[7:0], SelectedDIPByte2[7:0] and SelectedDIPByte3[7:0]
    */
    GT_U8   dipAddr[4];

    /** @brief the FDB ID that allows several vlans to share the same entries.
        it is a part of the 'hash key'
        In case of AC5 - FID holds value of VID.
    */
    GT_U32      fid;

    /** @brief destination interface : only vidx and vlan are valid for this entry type
     */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC dstInterface;

    /** @brief security level assigned to the MAC DA that matches this entry.
     * APPLICABLE RANGES:  In case of SIP6 & SIP5_SIP6_HYBRID shadow type: 0..1
     *                     In case of SIP5 & SIP4 shadow type: 0..0x7
     *
     *  NOTE: part of muxed fields (see ipMcEntryMuxingMode)
     */
    GT_U32 daSecurityLevel;

    /** @brief Source ID.
     *
     *  NOTE: part of muxed fields (see ipMcEntryMuxingMode)
     */
    GT_U32 sourceID;

    /** @brief user defined field.
     *  NOTE: part of muxed fields (see ipMcEntryMuxingMode)
     */
    GT_U32 userDefined;

    /** @brief action taken when a packet's DA matches this entry
     *
     *  Supported values: FORWARD,MIRROR_TO_CPU,TRAP_TO_CPU, DROP_HARD/SOFT_HARD
     *              IMPORTANT: if DROP_HARD/SOFT_HARD must match
     *              CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC::daDropCommand
     */
    CPSS_PACKET_CMD_ENT daCommand;

    /** @brief static/dynamic entry
     */
    GT_BOOL isStatic;

    /** @brief Age flag that is used for the aging state. the Used for the two-step Aging process.
        The device sets this bit to 1 (refreshed) when this MAC Entry is accessed as a source MAC Address.
        GT_FALSE - Entry is marked for aging in next age pass.
        GT_TRUE  - Entry is marked for aging in two age-passes.
     */
    GT_BOOL age;

    /** @brief indication to do 'Routing' if mac DA matches
     */
    GT_BOOL daRoute;

    /** @brief If set, and the entry <DA command> is TRAP or MIRROR then the CPU
     * code may be overwritten according to the Application Specific CPU Code assignment
     *  mechanism.
     */
    GT_BOOL appSpecificCpuCode;

    /** @brief the packet is marked by the FDB for mirroring to the Analyzer port.
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; AC5 *
     *  APPLICABLE VALUES:  In case of SIP5_SIP6_HYBRID shadow type:
     *                          CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E
     *                      In case of SIP5 shadow type:
     *                          CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E
     *                          CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_DA_E
     *                          CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_E
     *                          CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E
     *                      In case of SIP4 shadow type:
     *                          CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E
     */
    CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ENT mirrorToAnalyzerPort;

    /** @brief QOS parameters update for MAC DA
     *  GT_TRUE  - QOS parameters are modified when this entry matches the destination lookup for either MAC DA or IPv4/6 Multicast entry
     *  GT_FALSE - no QoS parameters are modified.
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; AC5
     *  APPLICABLE RANGES:  In case of SIP6 & SIP5_SIP6_HYBRID shadow type: 0
     *                      In case of SIP5 & SIP4 shadow type: 0..0x7
     */
    GT_U32  daQoSParameterSetIndex;

    /** @brief QOS parameters update for MAC SA
     *  GT_TRUE  - QOS parameters are modified when this entry matches the destination lookup for either MAC DA or IPv4/6 Multicast entry
     *  GT_FALSE - no QoS parameters are modified.
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; AC5
     *  APPLICABLE RANGES:  In case of SIP6 & SIP5_SIP6_HYBRID shadow type: 0
     *                      In case of SIP5 & SIP4 shadow type: 0..0x7
     */
    GT_U32  saQoSParameterSetIndex;

    /** @brief vid1 , optionally part of the 'hash key'
     *  NOTE: part of muxed fields (see entryMuxingMode)
     *  APPLICABLE RANGES:  In case of SIP6, SIP4 & SIP5_SIP6_HYBRID shadow type: 0
     *                      In case of SIP5 shadow type: 0..0xFFF
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X.
     *  SIP_6 and SIP_4 shadow type should ignore this value. Only SIP_5 and HYBRID need to check validity
     */
    GT_U32           vid1;
}CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC;

/**
* @enum CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT
 *
 * @brief Defines the types of info in the FDB entry for FDB UC routed packets
 * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
 *
*/
typedef enum
{
    /** the FDB entry hold full NH entry with info : tunnel start */
    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E ,
    /** the FDB entry hold full NH entry with info : NAT */
    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E          ,
    /** the FDB entry hold full NH entry with info : ARP */
    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E          ,
    /** @brief the FDB entry hold minimal info and point to the NH in the 'router' for additional info
     * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.)
     */
    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E                 ,
    /** @brief the FDB entry hold minimal info and point to the ECMP in the 'router' for additional info
     * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.)
     */
    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E
}CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT;


/**
* @union CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FIELD_POINTER_UNT
 *
 * @brief Field 'pointer' in the UC Routing Entry in the FDB.
 *      match to types: CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E
 *                      CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E
 *                      CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E
 * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
 *
*/
typedef union{
    /** @brief The field is 'pointer' to the tunnel start entry.
        info for type : CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E*/
    GT_U32      tunnelStartPointer;
    /** @brief The field is 'pointer' to the NAT entry.
        info for type : CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E*/
    GT_U32      natPointer;
    /** @brief The field is 'pointer' to the ARP entry.
        info for type : CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E*/
    GT_U32      arpPointer;
}CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FIELD_POINTER_UNT;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_IPV6_EXT_INFO_STC
 *
 * @brief IPv6 specific info in the UC Routing Entry in the FDB
 *      relevant to CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E
 *
*/
typedef struct{
    /** @brief The IPv6 site id of this route entry. */
    CPSS_IP_SITE_ID_ENT siteId;

    /** @brief Enable IPv6 Scope Checking. */
    GT_BOOL scopeCheckingEnable;
}CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_IPV6_EXT_INFO_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_STC
 *
 * @brief UC Routing Entry in the FDB : full info in the FDB entry
 *      match to types: CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E
 *                      CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E
 *                      CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E
 * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
 *
*/
typedef struct{
    /** next hop vlanId */
    GT_U32 nextHopVlanId;

    /** @brief Enable TTL/Hop Limit Decrement */
    GT_BOOL ttlHopLimitDecEnable;

    /** @brief Enable TTL/HopLimit Decrement and option/Extension
     *         check bypass.
     */
    GT_BOOL ttlHopLimDecOptionsExtChkByPass;

    /** @brief the counter set this route entry is linked to */
    CPSS_IP_CNT_SET_ENT countSet;

    /** @brief Enable performing ICMP Redirect Exception
     *         Mirroring. */
    GT_BOOL ICMPRedirectEnable;

    /** @brief One of the global configurable MTU sizes
     *  (APPLICABLE RANGES: Falcon, AC5P, AC5X, Harrier, Ironman 0..1)
     *  (APPLICABLE RANGES: Bobcat2, Caelum, Bobcat3, Aldrin, AC3X, Aldrin2 0..7)
     */
    GT_U32 mtuProfileIndex;

    /** @brief destination interface : port/trunk/vidx/vlan
     */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC dstInterface;

    /** @brief : info about 'pointer' (TS/NAT/ARP)
     *      match to types: CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E
     *                      CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E
     *                      CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E
    */
    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FIELD_POINTER_UNT pointer;

    /** Extended information that relevant to IPv6 entry only */
    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_IPV6_EXT_INFO_STC ipv6ExtInfo;

}CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT
 *
 * @brief UC Routing Entry next hop in the FDB
 * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*/
typedef union{
    /**   @brief The next hop info is fully in the FDB entry.
     *   match to types:
     *   CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E
     *   CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E
     *   CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E
    */
    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_STC fullFdbInfo;

    /**  @brief info for type : CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E
     * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.)
     */
    GT_U32      nextHopPointerToRouter;
    /**  @brief info for type : CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E
     * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.)
     */
    GT_U32      ecmpPointerToRouter;
}CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_IP_UC_COMMON_STC
 *
 * @brief IPv4/IPv6 UC Entry info
 * APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; AC3X.
 * APPLICABLE SHADOW_TYPES: SIP5; SIP5_SIP6_HYBRID
*/
typedef struct{

    /** @brief Enables the remarking of the QoS Profile assigned to the packet.
     *         GT_FALSE  - KeepPrevious : Keep previous QoSProfile setting
     *         GT_TRUE   - Remark       : Assign <QoSProfile> to the packet.
     *         APPLICABLE VALUES: in case of SIP5_SIP6_HYBRID shadow type - GT_FALSE
     *                            in case of SIP5 shadow type - GT_TRUE and GT_FALSE
     */
    GT_BOOL     qosProfileMarkingEnable;

    /** @brief Only relevant if the QoS precedence of the previous QoS Assignment Mechanisms is soft and
     *         <QoS Profile MarkingEn> is set to 1.
     *         If this is the QoSProfile that is assigned to the packet at the end of the ingress pipe,
     *         <QoSProfile> is used to index the QoSProfile to QoS Table Entry<n> and extract the QoS Parameters for the packet,
     *         which are TC, DP, UP and DSCP
     *  (APPLICABLE RANGES: 0..0x7F)
     */
    GT_U32      qosProfileIndex;

    /** @brief The precedence level of the QoS Profile assignment of the entry.
     *  APPLICABLE VALUES: In case of SIP5_SIP6_HYBRID shadow type:
     *                          CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
     *                     In case of SIP5 shadow type
     *                          CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E
     *                          CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E
     */
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosProfilePrecedence;

    /** @brief Enables the modification of the User Priority assigned to the packet.
     *  APPLICABLE VALUES: In case of SIP5_SIP6_HYBRID shadow type:
     *                          CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E
     *                     In case of SIP5 shadow type:
     *                          CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E
     *                          CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E
     *                          CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E
     */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT      modifyUp;

    /** @brief Enables the modification of the DSCP of the packet.
     *  APPLICABLE VALUES: In case of SIP5_SIP6_HYBRID shadow type:
     *                          CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E
     *                    In case of SIP5 shadow type:
     *                          CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E
     *                          CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E
     *                          CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E
     */
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT      modifyDscp;

    /** @brief Enables trapping or mirroring ARP broadcast packets.
     *  GT_FALSE - ARP Broadcast packets matching this route entry are not Trapped or Mirrored to the CPU,
     *             and they are forwarded as specified by previous engines.
     *  GT_TRUE  - An ARP Broadcast packet matching this entry is Trapped or Mirrored to the CPU,
     *             according to the configuration of <ARPBCMode> field
     *  APPLICABLE VALUES: In case of SIP5_SIP6_HYBRID shadow type - GT_FALSE
     *                     In case of SIP5 shadow type - GT_TRUE and GT_FALSE
     */
    GT_BOOL     arpBcTrapMirrorEnable;

    /** @brief The Access Level of routed Unicast packets with a DIP matching this entry,
     *  used to access the L3 Access matrix
     *  APPLICABLE RANGE: In case of SIP5_SIP6_HYBRID shadow type - 0
     *                    In case of SIP5 shadow type - 0..0x3F
     */
    GT_U32      dipAccessLevel;

    /** @brief Enables mirroring the packet to an Ingress Analyzer interface.
     *  GT_TRUE  - Ingress mirroring to analyzer is enabled
     *  GT_FALSE - Ingress mirroring to analyzer is disabled
     *  APPLICABLE VALUES: In case of SIP5_SIP6_HYBRID shadow type - GT_FALSE
     *                     In case of SIP5 shadow type - GT_TRUE and GT_FALSE
     */
    GT_BOOL ingressMirrorToAnalyzerEnable;

    /** @brief Enables mirroring the packet to an Ingress Analyzer interface.
     *  Only relevant if the ingressMirrorToAnalyzerEnable is enabled
     * (APPLICABLE RANGES: 0..0x6)
     */
    GT_U32 ingressMirrorToAnalyzerIndex;
}CPSS_DXCH_BRG_FDB_MANAGER_IP_UC_COMMON_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC
 *
 * @brief IPv4 UC Routing Entry in the FDB
 * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*/
typedef struct{
    /** @brief the IPv4 address .
        it is part of the 'hash key'
    */
    GT_IPADDR   ipv4Addr;
    /** @brief the VRF ID .
        it is part of the 'hash key'
    */
    GT_U32      vrfId;

    /** @brief Age flag that is used for the aging state. the Used for the two-step Aging process.
        The device sets this bit to 1 (refreshed) when this MAC Entry is accessed as a source MAC Address.
        GT_FALSE - Entry is marked for aging in next age pass.
        GT_TRUE  - Entry is marked for aging in two age-passes.
     */
    GT_BOOL age;

    /** state the UC route type */
    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT ucRouteType;

    /** the next hop info */
    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT ucRouteInfo;

    /** @brief the IP unicast related info
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X
     */
    CPSS_DXCH_BRG_FDB_MANAGER_IP_UC_COMMON_STC ucCommonInfo;

}CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC
 *
 * @brief IPv6 UC Routing Entry in the FDB
 * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*/
typedef struct{
    /** @brief the IPv6 address .
        it is part of the 'hash key'
    */
    GT_IPV6ADDR   ipv6Addr;
    /** @brief the VRF ID .
        it is part of the 'hash key'
    */
    GT_U32      vrfId;

    /** @brief Age flag that is used for the aging state. the Used for the two-step Aging process.
        The device sets this bit to 1 (refreshed) when this MAC Entry is accessed as a source MAC Address.
        GT_FALSE - Entry is marked for aging in next age pass.
        GT_TRUE  - Entry is marked for aging in two age-passes.
     */
    GT_BOOL age;

    /** state the UC route type */
    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT ucRouteType;

    /** the next hop info */
    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT ucRouteInfo;

    /** @brief the IP unicast related info
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X
     */
    CPSS_DXCH_BRG_FDB_MANAGER_IP_UC_COMMON_STC ucCommonInfo;

}CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC;

/**
* @enum CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT
 *
 * @brief This enum defines the type of the FDB Entry
*/
typedef enum{
    /** @brief The entry is MAC Address entry. (with/without : FID , Vid1) */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E,

    /** @brief The entry is IPv4 Multicast entry (IGMP Snooping). */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E,

    /** @brief The entry is IPv6 Multicast entry (MLD Snooping). */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E,

    /** @brief The entry is IPv4 Unicast entry.
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E,

    /** @brief The entry is IPv6 Unicast address entry.
     * NOTE: such entry occupy 2 entries in HW (and in shadow)
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E,

    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE__LAST__E /* not to be used */

} CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT;


/**
* @union CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_FORMAT_UNT
 *
 * @brief The formats of the types in the FDB table
 *
*/
typedef union{

    /** entry format matching : CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC fdbEntryMacAddrFormat;
    /** entry format matching : CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC fdbEntryIpv4McFormat;
    /** entry format matching : CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC fdbEntryIpv6McFormat;
    /** @brief entry format matching : CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC  fdbEntryIpv4UcFormat;
    /** @brief entry format matching : CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC  fdbEntryIpv6UcFormat;

} CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_FORMAT_UNT;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC
 *
 * @brief Fdb manager Entry format
*/
typedef struct{
    /** the FDB entry type */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT  fdbEntryType;

    /** the format of the FDB entry */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_FORMAT_UNT format;

}CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC;


/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC
 *
 * @brief Fdb manager Entry format
*/
typedef struct{
    /** @brief indication to use 'tempEntryOffset' as this entry expected to override 'SP unknown' entry (relevant to 'MAC entry' format) */
    GT_BOOL tempEntryExist;
    /** @brief the specific Bank Id in which the 'SP unknown' entry exists (this value
        is taken from the 'CPSS_MAC_UPDATE_MSG_EXT_STC::entryOffset' of the 'NA message')
        (relevant to 'MAC entry' format)
    */
    GT_U32  tempEntryOffset;
    /** @brief indication to enable the 're-hash' in case the new entry is not able to
        be inserted at 'first hash level' by the manager to move existing entries
        to allow place for the new entry (AKA : Cuckoo Algorithm)
        APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    GT_BOOL rehashEnable;

}CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC;

/**
* @enum CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_ENT
 *
 * @brief This enum defines the type of the update message
*/
typedef enum{

    /** @brief The entry is a New Address update. */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_E,

    /** @brief The entry is a Moved Address update. */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_E,

    /** @brief The entry is a No Space update
     *  NOTE: At this point FDB does not create any SP entry for this */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NO_SPACE_E,

    /** @brief The entry is a AGED OUT update */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_OUT_E,

    /** @brief The entry is a Aged out and deleted update */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_DELETED_E,

    /** @brief The entry is a deleted update */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_DELETED_E,

    /** @brief The entry is a Transplanted update */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_TRANSPLANTED_E,

    /** @brief The entry was deleted due to move/transplant that reach one of dynamic mac Uc limits */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_REACH_LIMIT_AND_DELETED_E,

    /** @brief The entry was not learned due to reach one of dynamic mac Uc limits */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_REACH_LIMIT_E,

} CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_ENT;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_NEW_FORMAT_STC
 *
 * @brief Fdb manager "Update entry" format for "NEW Addresses"
*/
typedef struct{

    /** @brief indicates temp entry offset (The bank in which hardware placed the entry at)*/
    GT_U32          tempEntryOffset;
} CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_NEW_FORMAT_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_MOVED_FORMAT_STC
 *
 * @brief Fdb manager "Update entry" format for "MOVED Addresses"
*/
typedef struct{

    /** @brief indicates the old interface details, the entry was attached with */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC oldInterface;

    /** @brief indicates source ID */
    GT_U32          oldsrcId;

}CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_MOVED_FORMAT_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_MOVED_REACH_LIMIT_AND_DELETED_FORMAT_STC
 *
 * @brief Fdb manager "learn limit reached" format for UC dynamic mac entries that reach the leamit
*/
typedef struct{
    /** the limit info it reached */
    CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC  limitInfo;
    /** the limit value it reached */
    GT_U32                                              limitValue;
}CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_MOVED_REACH_LIMIT_AND_DELETED_FORMAT_STC;

/**
* @union CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_INFO_UNT
 *
 * @brief The formats of the Update info types.
 *        Note: NEW and MOVED entries will have data, other entries do not have any data.
 *
*/
typedef union{

    /** @brief info structure for the update entry, when entry type is NEW */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_NEW_FORMAT_STC   newEntryFormat;

    /** @brief info structure for the update entry, when entry type is MOVED */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_MOVED_FORMAT_STC movedEntryFormat;

    /** @brief info structure for the update entry, when entry type is :
        CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_REACH_LIMIT_AND_DELETED_E or
        CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_REACH_LIMIT_E
    */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_MOVED_REACH_LIMIT_AND_DELETED_FORMAT_STC movedReachLimitEntryFormat;

} CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_INFO_UNT;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC
 *
 * @brief Fdb manager Update Event format
*/
typedef struct{

    /** @brief indicates type of update event */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_ENT updateType;

    /** @brief info structure for the update event */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_INFO_UNT updateInfo;

    /** @brief info structure for the FDB entry */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC entry;

}CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC
 *
 * @brief Fdb manager per port routing attributes.
          APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*/
typedef struct{

    /** @brief IPv6 Routing status */
    GT_BOOL unicastIpv6RoutingEn;

    /** @brief IPv4 Routing status */
    GT_BOOL unicastIpv4RoutingEn;

}CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC
 *
 * @brief Fdb manager per port learning attributes.
*/
typedef struct{

    /** @brief Enable sending new address messages to CPU */
    GT_BOOL                 naMsgToCpuEnable;

    /** @brief Unknown source address packet command */
    CPSS_PORT_LOCK_CMD_ENT  unkSrcAddrCmd;

    /** @brief Learning priority for the port
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.*/
    CPSS_DXCH_FDB_LEARN_PRIORITY_ENT learnPriority;

    /** @brief User Group of the port
     *    APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
     *   (APPLICABLE RANGES: 0..15)
     **/
    GT_U32 userGroup;

    /** @brief packet command when the packet's SA address is associated (in the FDB)
     *        with another port.
     *        one of : CPSS_PACKET_CMD_FORWARD_E .. CPSS_PACKET_CMD_DROP_SOFT_E
    */
    CPSS_PACKET_CMD_ENT      movedMacSaCmd;

}CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_NO_SPACE_CACHE_STC
*
* @brief FDB manager no space event cache
*/
typedef struct{

    /** @brief last 4 no-space entries.
    *   NOTE : non-valid entry specified as :
    *   noSpaceEntryCache[index].fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE__LAST__E
    */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC      noSpaceEntryCache[4];
}CPSS_DXCH_BRG_FDB_MANAGER_NO_SPACE_CACHE_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC
*
* @brief Fdb manager learning throttling attributes.
*/
typedef struct{
    /** @brief Mechanism enabled true / false (per Create API setting) */
    GT_BOOL   macNoSpaceUpdatesEnable;

    /** @brief The predefined occupancy level, which will be tuned by FDB Manager to get maximum performance */
    GT_U32  thresholdB;
    GT_U32  thresholdC;

    /** @brief Current used entries */
    GT_U32   currentUsedEntries;

    /** @brief The entries in the filtering cache */
    CPSS_DXCH_BRG_FDB_MANAGER_NO_SPACE_CACHE_STC  noSpaceCache;

    /** @brief AU current message rate - number of messages per second */
    GT_U32 auMsgRate;

    /** @brief AU message rate is limited with
     *        GT_TRUE  - Au messages are subjected to throttling with the auMsgRate.
     *        GT_FALSE - Au messages are not subjected to throttling.
     */
    GT_BOOL auMsgRateLimitEn;

    /** @brief NA hash-to-long enabled */
    GT_BOOL hashTooLongEnabled;

}CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC
 *
 * @brief Fdb manager Learning scan attributes.
*/
typedef struct{

    /** @brief Indicates FDB updating status for NA events
     *        GT_TRUE  - Update DB & FDB while processing the event,
     *                   Copy to OUT event array only if success.
     *        GT_FALSE - Do not update DB/FDB, Copy to OUT event array
     */
    GT_BOOL addNewMacUcEntries;

    /** @brief indicates if to try rehashing (Cuckoo) on additions
       APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.*/
    GT_BOOL addWithRehashEnable;

    /** @brief Indicates FDB updating status for Moved events
     *        GT_TRUE  - Update DB & FDB while processing the event,
     *                   Copy to OUT event array only if success.
     *        GT_FALSE - Do not update DB/FDB, Copy to OUT event array
     *        APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
     */
    GT_BOOL updateMovedMacUcEntries;
}CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC
 *
 * @brief Fdb manager Transplant scan attributes.
*/
typedef struct{

    /* Entry Types */
    /** @brief Input to the Transplant scan to include(or exclude) MAC unicast entries
     *        GT_TRUE   - Include MAC unicast type of entries in transplanting process.
     *        GT_FALSE  - Do not include MAC unicast type of entries in transplanting process.
     */
    GT_BOOL transplantMacUcEntries;

    /** @brief Input to the Transplant scan to include(or exclude) IP unicast entries
     *        GT_TRUE   - Include IP unicast type of entries in transplanting process.
     *        GT_FALSE  - Do not include IP unicast type of entries in transplanting process.
     *        APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
     */
    GT_BOOL transplantIpUcEntries;

    /** @brief Input to the Transplant scan to include(or exclude) static entries
     *        GT_TRUE   - Include static entries in transplanting process.
     *        GT_FALSE  - Do not include static entries in transplanting process.
     */
    GT_BOOL transplantStaticEntries;

    /* Filters */
    /** @brief indicates the fid value(To filter the entry)
     * In case of AC5 - VID is used in place of FID */
    GT_U32  fid;

    /** @brief indicates the fid mask bits */
    GT_U32  fidMask;

    /** @brief  indicates the old interface info to filter the entries */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC oldInterface;

    /** @brief indicates the new interface info to apply to the transplanted entries */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC newInterface;
}CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC
 *
 * @brief Fdb manager Delete scan attributes.
*/
typedef struct{

    /* Entry Types */
    /** @brief Input to the Delete scan to include(or exclude) MAC unicast entries
     *        GT_TRUE   - Include MAC unicast type of entries in delete process.
     *        GT_FALSE  - Do not include MAC unicast type of entries in delete process.
     */
    GT_BOOL deleteMacUcEntries;

    /** @brief Input to the Delete scan to include(or exclude) MAC multicast entries
     *        GT_TRUE   - Include MAC multicast type of entries in delete process.
     *        GT_FALSE  - Do not include MAC multicast type of entries in delete process.
     */
    GT_BOOL deleteMacMcEntries;

    /** @brief Input to the Delete scan to include(or exclude) IP unicast entries
     *        GT_TRUE   - Include IP unicast type of entries in delete process.
     *        GT_FALSE  - Do not include IP unicast type of entries in delete process.
     *        APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
     */
    GT_BOOL deleteIpUcEntries;

    /** @brief Input to the Delete scan to include(or exclude) IP multicast entries
     *        GT_TRUE   - Include IP multicast type of entries in delete process.
     *        GT_FALSE  - Do not include IP multicast type of entries in delete process.
     */
    GT_BOOL deleteIpMcEntries;

    /** @brief Input to the Delete scan to include(or exclude) static entries
     *        GT_TRUE   - Include static entries in delete process.
     *        GT_FALSE  - Do not include static entries in delete process.
     */
    GT_BOOL deleteStaticEntries;

    /* Filters */
    /** @brief indicates the fid value(To filter the entry)
     * In case of AC5 - VID is used in place of FID */
    GT_U32          fid;

    /** @brief indicates the fid mask bits */
    GT_U32          fidMask;

    /** @brief indicates the device number of the entry */
    GT_U32          hwDevNum;

    /** @brief certain dev / all dev entries */
    GT_U32          hwDevNumMask;

    /** @brief indicates the ePort/Trunk number */
    GT_U32          ePortTrunkNum;

    /** @brief indicates the ePort/Trunk number mask bits */
    GT_U32          ePortTrunkNumMask;

    /** @brief indicates if the interface is trunk or not */
    GT_BOOL         isTrunk;

    /** @brief indicates the isTrunk mask bits */
    GT_BOOL          isTrunkMask;

    /** @brief indicates the vid1 value
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    GT_U16          vid1;

    /** @brief indicates the vid1 mask bits */
    GT_U16          vid1Mask;

    /** @brief indicates the userDefined value */
    GT_U32          userDefined;

    /** @brief indicates the userDefined mask bits */
    GT_U32          userDefinedMask;

    /** @brief relevant only to Dynamic UC MAC entries
        GT_TRUE - only dynamic UC MAC entries are being examined & API
                  will check the port/Trunk/ global ePort, FID and global
                  limits this entry is assigned to - then it should check
                  the corresponding current & max learning counters -
                  such that only if current > max the entry is eligible
                  for delete
        GT_FALSE - don't check ONLY Dynamic UC MAC entries that are over one of their limits.
                   meaning apply no extra checks.
    */
    GT_BOOL onlyAboveLearnLimit;

}CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC
 *
 * @brief Fdb manager aging scan attributes.
*/
typedef struct{

    /* Entry Types */
    /** @brief Input to the Aging scan to include(or exclude) MAC unicast entries
     *        GT_TRUE   - Include MAC unicast type of entries in aging process.
     *        GT_FALSE  - Do not include MAC unicast type of entries in aging process.
     */
    GT_BOOL checkAgeMacUcEntries;

    /** @brief Input to the Aging scan to include(or exclude) MAC multicast entries
     *        GT_TRUE   - Include MAC multicast type of entries in aging process.
     *        GT_FALSE  - Do not include MAC multicast type of entries in aging process.
     */
    GT_BOOL checkAgeMacMcEntries;

    /** @brief Input to the Aging scan to include(or exclude) IP unicast entries
     *        GT_TRUE   - Include IP unicast type of entries in aging process.
     *        GT_FALSE  - Do not include IP unicast type of entries in aging process.
     *        APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
     */
    GT_BOOL checkAgeIpUcEntries;

    /** @brief Input to the Aging scan to include(or exclude) IP multicast entries
     *        GT_TRUE   - Include IP multicast type of entries in aging process.
     *        GT_FALSE  - Do not include IP multicast type of entries in aging process.
     */
    GT_BOOL checkAgeIpMcEntries;

    /* Filters */
    /** @brief indicates the fid value(To filter the entry)
     * In case of AC5 - VID is used in place of FID */
    GT_U32          fid;

    /** @brief indicates the fid mask bits */
    GT_U32          fidMask;

    /** @brief indicates the ePort/Trunk number */
    GT_U32          ePortTrunkNum;

    /** @brief indicates the ePort/Trunk number mask bits */
    GT_U32          ePortTrunkNumMask;

    /** @brief indicates if the interface is trunk or not */
    GT_BOOL         isTrunk;

    /** @brief indicates the isTrunk mask bits */
    GT_BOOL         isTrunkMask;

    /** @brief indicates the vid1 value
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    GT_U16          vid1;

    /** @brief indicates the vid1 mask bits */
    GT_U16          vid1Mask;

    /** @brief indicates the userDefined value */
    GT_U32          userDefined;

    /** @brief indicates the userDefined mask bits */
    GT_U32          userDefinedMask;

    /** @brief indicates the device number of the entry */
    GT_U32          hwDevNum;

    /** @brief UC-ePort entries - certain dev / all */
    GT_U32          hwDevNumMask_ePort;

    /** @brief UC-trunk entries - certain dev / all */
    GT_U32          hwDevNumMask_trunk;

    /* -------------Delete Action-------------- */
    /** @brief Input to the Aging scan, indicating aged-out MAC unicast(ePort) entries to delete
     *        GT_TRUE   - Delete the aged-out MAC unicast(ePort) entries
     *        GT_FALSE  - Do not delete the aged-out MAC unicast(ePort) entries.
     */
    GT_BOOL     deleteAgeoutMacUcEportEntries;

    /** @brief Input to the Aging scan, indicating aged-out MAC unicast(trunk) entries to delete
     *        GT_TRUE   - Delete the aged-out MAC unicast(trunk) entries
     *        GT_FALSE  - Do not delete the aged-out MAC unicast(trunk) entries.
     */
    GT_BOOL     deleteAgeoutMacUcTrunkEntries;

    /** @brief Input to the Aging scan, indicating aged-out MAC multicast entries to delete
     *        GT_TRUE   - Delete the aged-out MAC multicast entries
     *        GT_FALSE  - Do not delete the aged-out MAC multicast entries.
     */
    GT_BOOL     deleteAgeoutMacMcEntries;

    /** @brief Input to the Aging scan, indicating aged-out IP unicast entries to delete
     *        GT_TRUE   - Delete the aged-out IP unicast entries
     *        GT_FALSE  - Do not delete the aged-out IP unicast entries.
     *        APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
     */
    GT_BOOL     deleteAgeoutIpUcEntries;

    /** @brief Input to the Aging scan, indicating aged-out IP multicast entries to delete
     *        GT_TRUE   - Delete the aged-out IP multicast entries
     *        GT_FALSE  - Do not delete the aged-out IP multicast entries.
     */
    GT_BOOL     deleteAgeoutIpMcEntries;
}CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC;


/** max number of banks in FDB hash */
#define CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS     16

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC
 *
 * @brief Defines the FDB counters :
 *  1. per entry type counters
 *  2. summary counters
 *  3. per bank counter
 *
*/
typedef struct
{
    /** counter for type : MAC unicast dynamic  */
    GT_U32  macUnicastDynamic;
    /** counter for type : MAC unicast static   */
    GT_U32  macUnicastStatic;
    /** counter for type : MAC multicast dynamic */
    GT_U32  macMulticastDynamic;
    /** counter for type : MAC multicast static  */
    GT_U32  macMulticastStatic;
    /** counter for type : IPv4 multicast */
    GT_U32  ipv4Multicast;
    /** counter for type : IPv6 multicast */
    GT_U32  ipv6Multicast;
    /** @brief counter for type : IPv4 unicast routing
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    GT_U32  ipv4UnicastRoute;
    /** @brief counter for type : IPv6 unicast routing : the 'key' part
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    GT_U32  ipv6UnicastRouteKey;
    /** @brief counter for type : IPv6 unicast routing : the 'data' part
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    GT_U32  ipv6UnicastRouteData;

    /** Summary of all of the above */
    GT_U32  usedEntries;
    /** @brief : the number of potential new entries to be inserted to the manager.
       calculated as the Max total entries requested by application in Create API
       minus used entries */
    GT_U32  freeEntries;

    /** counter per bank */
    GT_U32  bankCounters[CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS];

}CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC
 *
 * @brief FDB manager update entry parameters
*/
typedef struct{
    /** @brief Flag that is used for the FDB entry updating.
        GT_FALSE - Whole FDB entry will be updated.
        GT_TRUE  - Only source interface in FDB entry will be updated.
     */
    GT_BOOL updateOnlySrcInterface;

}CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC
 *
 * @brief Defines the FDB statistics per operations
 *
*/
typedef struct
{
    /** Entry successfully added with rehashing stage 0*/
    GT_U32 entryAddOkRehashingStage0;
    /** @brief Entry successfully added with rehashing stage 1
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    GT_U32 entryAddOkRehashingStage1;
    /** @brief Entry successfully added with rehashing stage 2
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    GT_U32 entryAddOkRehashingStage2;
    /** @brief Entry successfully added with rehashing stage 3
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    GT_U32 entryAddOkRehashingStage3;
    /** @brief Entry successfully added with rehashing stage 4
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    GT_U32 entryAddOkRehashingStage4;
    /** @brief Entry successfully added with rehashing stage > 4
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    GT_U32 entryAddOkRehashingStageMoreThanFour;
    /** Entry not added - input error */
    GT_U32 entryAddErrorInputInvalid;
    /** Entry not added - bad state */
    GT_U32 entryAddErrorBadState;
    /** Entry not added - entry exist */
    GT_U32 entryAddErrorEntryExist;
    /** Entry not added - table full */
    GT_U32 entryAddErrorTableFull;
    /** Entry not added - hw error */
    GT_U32 entryAddErrorHwUpdate;
    /** Entry not added - number of errors due to reach limit on any port */
    GT_U32  entryAddErrorPortLimit;
    /** Entry not added - number of errors due to reach limit on any trunk */
    GT_U32  entryAddErrorTrunkLimit;
    /** Entry not added - number of errors due to reach limit on any global eport */
    GT_U32  entryAddErrorGlobalEPortLimit;
    /** Entry not added - number of errors due to reach limit on any Fid */
    GT_U32  entryAddErrorFidLimit;
    /** Entry not added - number of errors due to reach the global limit */
    GT_U32  entryAddErrorGlobalLimit;

    /** Entry successfully updated */
    GT_U32 entryUpdateOk;
    /** Entry not updated - input error */
    GT_U32 entryUpdateErrorInputInvalid;
    /** Entry not updated - entry not found */
    GT_U32 entryUpdateErrorNotFound;
    /** Entry not update - DB coherency problem : 'bad state' */
    GT_U32 entryUpdateErrorBadState;
    /** Entry not update - hw error */
    GT_U32 entryUpdateErrorHwUpdate;
    /** Entry not updated - number of errors due to reach limit on any port */
    GT_U32  entryUpdateErrorPortLimit;
    /** Entry not updated - number of errors due to reach limit on any trunk */
    GT_U32  entryUpdateErrorTrunkLimit;
    /** Entry not updated - number of errors due to reach limit on any global eport */
    GT_U32  entryUpdateErrorGlobalEPortLimit;
    /** Entry not update - number of errors due to reach limit on any Fid */
    GT_U32  entryUpdateErrorFidLimit;

    /** Entry successfully deleted */
    GT_U32 entryDeleteOk;
    /** Entry not deleted -  input error */
    GT_U32 entryDeleteErrorInputInvalid;
    /** Entry not deleted - entry not found */
    GT_U32 entryDeleteErrorNotfound;
    /** Entry not deleted - hw error */
    GT_U32 entryDeleteErrorHwUpdate;
    /** Entry not deleted - DB coherency problem : 'bad state' */
    GT_U32 entryDeleteErrorBadState;

    /** Temporary entry successfully deleted */
    GT_U32 entryTempDeleteOk;
    /** Temporary entry not deleted -  input error */
    GT_U32 entryTempDeleteErrorInputInvalid;

    /** Scan transplant successfully */
    GT_U32 scanTransplantOk;
    /** Scan transplant successfully - no more */
    GT_U32 scanTransplantOkNoMore;
    /** Scan transplant error - input error */
    GT_U32 scanTransplantErrorInputInvalid;
    /** Scan transplant error - failed table update */
    GT_U32 scanTransplantErrorFailedTableUpdate;
    /** Scan transplant - total transplanted entries */
    GT_U32 scanTransplantTotalTransplantedEntries;
    /** Scan transplant - total transplanted that reached limit and had to be deleted */
    GT_U32 scanTransplantTotalTransplantReachLimitDeletedEntries;

    /** Scan delete successfully */
    GT_U32 scanDeleteOk;
    /** Scan delete successfully - no more */
    GT_U32 scanDeleteOkNoMore;
    /** Scan delete error - failed table update */
    GT_U32 scanDeleteErrorFailedTableUpdate;
    /** Scan delete error - input error */
    GT_U32 scanDeleteErrorInputInvalid;
    /** Scan delete - total transplanted entries */
    GT_U32 scanDeleteTotalDeletedEntries;

    /** Scan aging successfully */
    GT_U32 scanAgingOk;
    /** Scan aging - input error */
    GT_U32 scanAgingErrorInputInvalid;
    /** Scan aging error - failed table update */
    GT_U32 scanAgingErrorFailedTableUpdate;
    /** Scan aging - total aged out entries */
    GT_U32 scanAgingTotalAgedOutEntries;
    /** Scan aging - total aged out deleted entries */
    GT_U32 scanAgingTotalAgedOutDeleteEntries;

    /** Scan learning successully */
    GT_U32 scanLearningOk;
    /** Scan learning successully - no more */
    GT_U32 scanLearningOkNoMore;
    /** Scan learning error - input error */
    GT_U32 scanLearningErrorInputInvalid;
    /** Scan learning error - failed table update */
    GT_U32 scanLearningErrorFailedTableUpdate;
    /** Scan learning - total HW entry new messages */
    GT_U32 scanLearningTotalHwEntryNewMessages;
    /** Scan learning - total HW entry moved messages */
    GT_U32 scanLearningTotalHwEntryMovedMessages;
    /** Scan learning - total HW entry no space messages */
    GT_U32 scanLearningTotalHwEntryNoSpaceMessages;
    /** Scan learning - total HW entry no space filtered messages */
    GT_U32 scanLearningTotalHwEntryNoSpaceFilteredMessages;

    /** HA Sync total valid */
    GT_U32 haSyncTotalValid;
    /** HA Sync total IN valid */
    GT_U32 haSyncTotalInvalid;
    /** HA Sync total SP entry found */
    GT_U32 haSyncTotalSPEntry;
    /** HA Sync total IPv6 Invalid data part */
    GT_U32 haSyncTotalIPv6DataInvalid;

    /** Entries rewrite ok */
    GT_U32 entriesRewriteOk;
    /** Entries rewrite error - input error */
    GT_U32 entriesRewriteErrorInputInvalid;
    /** Entries rewrite total rewritten entries */
    GT_U32 entriesRewriteTotalRewrite;

    /** 'limit set' API : number of success to set limit on any port */
    GT_U32  limitSetOkPortLimit;
    /** 'limit set' API : number of success to set limit on any trunk */
    GT_U32  limitSetOkTrunkLimit;
    /** 'limit set' API : number of success to set limit on any global eport */
    GT_U32  limitSetOkGlobalEPortLimit;
    /** 'limit set' API : number of success to set limit on any Fid */
    GT_U32  limitSetOkFidLimit;
    /** 'limit set' API : number of success to set the global limit  */
    GT_U32  limitSetOkGlobalLimit;
    /** 'limit set' API : number of errors on input parameter(s)  */
    GT_U32  limitSetErrorInput;

}CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC
 *
 * @brief Defines the types of tests to perform
 *
*/
typedef struct
{
    /** @brief : Indicates FDB manager to check - Global parameters */
    GT_BOOL         globalCheckEnable;

    /** @brief : Indicates FDB manager to check - Counters */
    GT_BOOL         countersCheckEnable;

    /** @brief : Indicates FDB manager to check - Database Free List */
    GT_BOOL         dbFreeListCheckEnable;

    /** @brief : Indicates FDB manager to check - Database Used List */
    GT_BOOL         dbUsedListCheckEnable;

    /** @brief : Indicates FDB manager to check - Database index pointers */
    GT_BOOL         dbIndexPointerCheckEnable;

    /** @brief : Indicates FDB manager to check - Database aging bin */
    GT_BOOL         dbAgingBinCheckEnable;

    /** @brief : Indicates FDB manager to check - Database aging bin Usage matrix */
    GT_BOOL         dbAgingBinUsageMatrixCheckEnable;

    /** @brief : Indicates FDB manager to check - HW Used List */
    GT_BOOL         hwUsedListCheckEnable;

    /** @brief : Indicates FDB manager to check - HW Free List (Ignore SP) */
    GT_BOOL         hwFreeListCheckEnable;

    /** @brief : Indicates FDB manager to check - HW Free List (do not ignore SP) */
    GT_BOOL         hwFreeListWithSpCheckEnable;

    /** @brief :  Global learn limit - check global limit current entries as follows:
     *  The sum of current entries as counted by the global counter must be equal to the field:
     *  MAC Unicast - dynamic already exist in Counters structure.
     *  corresponding error : CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_GLOBAL_INVALID_E
     */
    GT_BOOL         learnLimitGlobalCheckEnable;

    /** @brief :  FID learn limit - check all global FID limit current entries as follows:
     *  The total sum of all current entries from all FID entries equals the Global counter.
     *  corresponding error : CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_FID_INVALID_E
     */
    GT_BOOL         learnLimitFidCheckEnable;

    /** @brief :  Global ePort learn limit - check all global ePorts limit current entries as follows:
     *  The total sum of all current entries from all Global ePort must be smaller or equal the Global counter.
     *  corresponding error : CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_GLOBAL_EPORT_INVALID_E
     */
    GT_BOOL         learnLimitGlobalEportCheckEnable;

    /** @brief :  Trunk learn limit - check all global Trunk limit current entries as follows:
     *  The total sum of all current entries from all Trunk must be smaller or equal the Global counter.
     *  corresponding error : CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_TRUNK_INVALID_E
     */
    GT_BOOL         learnLimitTrunkCheckEnable;

    /** @brief :  Ports learn limit - check all per device Port limit current entries as follows:
     *  The total sum of all current entries from all ports must be smaller or equal the Global counter
     *  corresponding error : CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_PORT_INVALID_E
     */
    GT_BOOL         learnLimitPortCheckEnable;

    /** @brief :  Ports + Trunk + Global ePort learn limit - check the integrity
     *  of current in both port, Trunk, Global ePort together
     *  The total sum of all current entries from all ports, Trunk & Global ePorts
     *  together must be equal to the Global counter
     *  corresponding error : CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_SRC_INTERFACES_CURRENT_INVALID_E
     */
    GT_BOOL         learnLimitSrcInterfacesCheckEnable;

} CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC;

/**
 * @define CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC
 *
 * @brief Define MACRO to fill the stc of CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC
 *        with 'check ALL' available checkers.
 *      IN param : _dbCheckParam - the variable of type CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC
*/
#define CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(_dbCheckParam) \
    _dbCheckParam.globalCheckEnable                   = GT_TRUE;      \
    _dbCheckParam.countersCheckEnable                 = GT_TRUE;      \
    _dbCheckParam.dbFreeListCheckEnable               = GT_TRUE;      \
    _dbCheckParam.dbUsedListCheckEnable               = GT_TRUE;      \
    _dbCheckParam.dbIndexPointerCheckEnable           = GT_TRUE;      \
    _dbCheckParam.dbAgingBinCheckEnable               = GT_TRUE;      \
    _dbCheckParam.dbAgingBinUsageMatrixCheckEnable    = GT_TRUE;      \
    _dbCheckParam.hwUsedListCheckEnable               = GT_TRUE;      \
    _dbCheckParam.hwFreeListCheckEnable               = GT_TRUE;      \
    _dbCheckParam.hwFreeListWithSpCheckEnable         = GT_TRUE;      \
    _dbCheckParam.learnLimitGlobalCheckEnable         = GT_TRUE;      \
    _dbCheckParam.learnLimitFidCheckEnable            = GT_TRUE;      \
    _dbCheckParam.learnLimitGlobalEportCheckEnable    = GT_TRUE;      \
    _dbCheckParam.learnLimitTrunkCheckEnable          = GT_TRUE;      \
    _dbCheckParam.learnLimitPortCheckEnable           = GT_TRUE;      \
    _dbCheckParam.learnLimitSrcInterfacesCheckEnable  = GT_TRUE


/**
 * * @enum CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULTS_ENT
 *
 *  @brief This enum defines the type of test failure, for FDB self-test
 */
typedef enum{
    /** @brief : Indicates FDB manager "global parameters check" test failed */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E,

    /** @brief : Indicates FDB manager "counter" test failed - (per type check) */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E,
    /** @brief : Indicates FDB manager "counter" test failed - (per Bank check) */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E,
    /** @brief : Indicates FDB manager "counter" test failed - HW/SW counter mismatch */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_SW_BANK_COUNTERS_MISMATCH_E,

    /** @brief : Indicates FDB manager "Free List check" test failed - 1st pointer invalid */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E,
    /** @brief : Indicates FDB manager "Free List check" test failed - counter number & list entries mismatch */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,

    /** @brief : Indicates FDB manager "Used List check" test failed - 1st pointer invalid */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E,
    /** @brief : Indicates FDB manager "Used List check" test failed - Last pointer invalid */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E,
    /** @brief : Indicates FDB manager "Used List check" test failed - Get Next failed(List corrupted) */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E,
    /** @brief : Indicates FDB manager "Used List check" test failed - Transplant scan INVALID pointer */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_TRANS_SCAN_PTR_INVALID_E,
    /** @brief : Indicates FDB manager "Used List check" test failed - Delete scan INVALID pointer */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E,
    /** @brief : Indicates FDB manager "Used List check" test failed - TBD */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E,
    /** @brief : Indicates FDB manager "Used List check" test failed - List number mismatch */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E,

    /** @brief : Indicates FDB manager "Index List check" test failed - Index pointer pointing to a invalid entry */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E,
    /** @brief : Indicates FDB manager "Index List check" test failed - HW index is invalid */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E,

    /** @brief : Indicates FDB manager "Aging bin check" test failed - age-bin counter mismatch with entries present */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,
    /** @brief : Indicates FDB manager "Aging bin check" test failed - total age-bin counter mismatch with total entries present */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E,
    /** @brief : Indicates FDB manager "Aging bin check" test failed - Entry age-bin number mismatch with the age-bin its connected */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E,
    /** @brief : Indicates FDB manager "Aging bin check" test failed - age-bin last scan pointer invalid */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E,

    /** @brief : Indicates FDB manager "Aging bin usage matrix" test failed - L1 bitmap not matching with present entries and age-bin counter */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E,
    /** @brief : Indicates FDB manager "Aging bin usage matrix" test failed - L2 bitmap not matching with L1 bitmaps */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E,

    /** @brief : Indicates FDB manager "HW Entries - Used List" test failed - Invalid Entry */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E,

    /** @brief : Indicates FDB manager "HW Entries - Free List" test failed - Valid entry(Non-SP) in place of FREE */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E,

    /** @brief : Indicates FDB manager "HW Entries - Free List" test failed - SP entry in place of FREE */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E,

    /** @brief :  Global learn limit - check global limit current entries as follows:
     *  The sum of current entries as counted by the global counter must be equal to the field:
     *  MAC Unicast - dynamic already exist in Counters structure.
     */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_GLOBAL_INVALID_E,

    /** @brief :  FID learn limit - check all global FID limit current entries as follows:
     *  The total sum of all current entries from all FID entries equals the Global counter.
     */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_FID_INVALID_E,

    /** @brief :  Global ePort learn limit - check all global ePorts limit current entries as follows:
     *  The total sum of all current entries from all Global ePort must be smaller or equal the Global counter.
     */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_GLOBAL_EPORT_INVALID_E,

    /** @brief :  Trunk learn limit - check all global Trunk limit current entries as follows:
     *  The total sum of all current entries from all Trunk must be smaller or equal the Global counter.
     */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_TRUNK_INVALID_E,

    /** @brief :  Ports learn limit - check all per device Port limit current entries as follows:
     *  The total sum of all current entries from all ports must be smaller or equal the Global counter
     */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_PORT_INVALID_E,

    /** @brief :  Ports + Trunk + Global ePort learn limit - check the integrity
     *  of current in both port, Trunk, Global ePort together
     *  The total sum of all current entries from all ports, Trunk & Global ePorts
     *  together must be equal to the Global counter
     */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_SRC_INTERFACES_CURRENT_INVALID_E,


    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E
} CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC
 *
 * @brief Defines the FDB entry attributes that can be changed after manager creation.
 *       (for 'manager config update' API)
 *
 *  NOTE : this is subset of parameters from : CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC
 *
*/
typedef struct{
    /** @brief Application can decide how HW will treat entries SA commands - HARD or SOFT drops
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman; */
    CPSS_PACKET_CMD_ENT                              saDropCommand;

    /** @brief Application can decide how HW will treat entries DA commands - HARD or SOFT drops
     * APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    CPSS_PACKET_CMD_ENT                              daDropCommand;

    /** @brief Application can decide how HW will treat routed IP packets - one of :
        CPSS_PACKET_CMD_ROUTE_E
        CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E
        CPSS_PACKET_CMD_TRAP_TO_CPU_E
        CPSS_PACKET_CMD_DROP_HARD_E
        CPSS_PACKET_CMD_DROP_SOFT_E
        APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
    */
    CPSS_PACKET_CMD_ENT                              ipNhPacketcommand;

}CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC
 *
 * @brief Defines the FDB learning parameters that can be changed after manager creation.
 *       (for 'manager config update' API)
 *
 *  NOTE : this is subset of parameters from : CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC
 *
 *
*/
typedef struct{
    /** @brief Application can select to globally enable or disable MAC learning of routed packets. */
    GT_BOOL  macRoutedLearningEnable;

}CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC;

/**
* @struct CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC
 *
 * @brief Defines the FDB aging/refresh parameters that can be changed after manager creation.
 *       (for 'manager config update' API)
 *
 *  NOTE : this is subset of parameters from : CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC
 *
*/
typedef struct{
    /** @brief Application can Enable refresh by destination mac address - UC packets
     *  (in addition to refresh by source mac address , that is always done)
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman; */
    GT_BOOL         destinationUcRefreshEnable;
    /** @brief Application can Enable refresh by destination mac address - MC packets
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman; */
    GT_BOOL         destinationMcRefreshEnable;
    /** @brief Refresh IP UC FDB entries when entry is used for routing.
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman; */
    GT_BOOL         ipUcRefreshEnable;
}CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgFdbManagerTypes_h */


