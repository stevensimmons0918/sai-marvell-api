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
* @file prvCpssDxChBrgFdbManager_db.h
*
* @brief FDB manager support.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChBrgFdbManager_db_h
#define __prvCpssDxChBrgFdbManager_db_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/bridgeFdbManager/cpssDxChBrgFdbManager.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrg.h>

#define PRV_SHARED_FDB_MANAGER_DB_VAR_SET(_var,_value)\
      PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.fdbManagerDir._var,_value)

#define PRV_SHARED_FDB_MANAGER_DB_VAR_GET(_var)\
      PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.fdbManagerDir._var)

/* get pointer to the FDB manager */
#define MANAGER_GET_MAC(_fdbManagerId)  PRV_SHARED_FDB_MANAGER_DB_VAR_GET(prvCpssDxChFdbManagerDbArr)[_fdbManagerId]

/* macro to check and return error when fdbManagerId is not valid */
#define FDB_MANAGER_ID_CHECK(_fdbManagerId)                         \
    if((_fdbManagerId) >= CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS)     \
    {                                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,                 \
            "fdbManagerId [%d] >= [%d] (out of range)",             \
            (_fdbManagerId),                                        \
            CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS);        \
    }                                                               \
    else if(NULL == PRV_SHARED_FDB_MANAGER_DB_VAR_GET(prvCpssDxChFdbManagerDbArr)[_fdbManagerId])              \
    {                                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,           \
            "fdbManagerId [%d] was not initialized ",               \
            (_fdbManagerId));                                       \
    }

/* number of words needed for 'BMP' of devices */
#define NUM_WORDS_DEVS_BMP_CNS  ((PRV_CPSS_MAX_PP_DEVICES_CNS+31)/32)

/* we force application to give 'num of entries' in steps of 256 */
#define PRV_FDB_MANAGER_NUM_ENTRIES_STEPS_CNS 256

/* FDB maximal entries per age-bin */
#define PRV_CPSS_DXCH_BRG_FDB_MANAGER_MAX_AGE_BIN_ID_CNS    \
    (_256K / PRV_FDB_MANAGER_NUM_ENTRIES_STEPS_CNS)

/* Number of AU messages to read from AUQ (Per each prvDxChBrgFdbAuFuMsgBlockGet call)*/
#define AU_MAX_NUM_CNS                                      256

/* Max number of rehashing stages used in cuckoo' algorithm  */
#define PRV_CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_REHASHING_STAGES_CNS     100

/* Total stages for 4 bank hash calculation */
#define PRV_CPSS_DXCH_BRG_FDB_MANAGER_FOUR_BANKS_TOTAL_STAGES_CNS        5
/* Total stages for 8 bank hash calculation */
#define PRV_CPSS_DXCH_BRG_FDB_MANAGER_EIGHT_BANKS_TOTAL_STAGES_CNS       4
/* Total stages for 16 bank hash calculation */
#define PRV_CPSS_DXCH_BRG_FDB_MANAGER_SIXTEEN_BANKS_TOTAL_STAGES_CNS     3
/* Allow HW bank counters update */
/* #define PRV_CPSS_DXCH_BRG_FDB_MANAGER_ALLOW_HW_BANK_COUNTER_UPDATE_CNS */
/**
* @union PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_RESTORE_INFO_UNT
 *
 * @brief internal FDB manager restore info.
*/
typedef union {
    /* USED_LIST_1ST_PTR_INVALID */
    /* USED_LIST_TRANS_SCAN_PTR_INVALID, USED_LIST_DELETE_SCAN_PTR_INVALID */
    struct{
        GT_U32  entryPointer;
        GT_BOOL isValid_entryPointer;
        GT_BOOL isUsedEntry;
    }nodeInfo;

    /* FREE_LIST_1ST_PTR_INVALID,       INDEX_POINT_INVALID,            USED_LIST_GETNEXT_PTR_INVALID,
     * AGING_SCAN_PTR_INVALID,          HW_FREE_ENTRY_VALID_EXIST,      USED_LIST_ENTRY_PREV_PTR_INVALID,
     */
    GT_BOOL entryStatus;

    /* AGING_USAGE_MATRIX_L2_INVALID */
    GT_BOOL l2BitStatus;

    /* TYPE_COUNTER_INVALID,            BANK_COUNTER_INVALID,           HW_SW_COUNTER_MISMATCH
     * FREE_LIST_ENTRIES_NUM_MISMATCH,  USED_LIST_LAST_PTR_INVALID_E,   USED_LIST_ENTRIES_NUM_MISMATCH,
     * AGING_TOTAL_NUM_MISMATCH,        AGING_BIN_ENTRIES_NUM_MISMATCH  AGING_USAGE_MATRIX_L1_INVALID
     * GLOBAL_PARAM_HW_INVALID
     */
    GT_U32  counter;

    /* ENTRY_INDEX_INVALID */
    GT_U32  hwIndex;

    /* AGING_ENTRY_BIN_INDEX_INVALID */
    GT_U32  ageBinIndex;

    /* HW_ENTRY_INVALID */
    GT_U32  macAddr_low_32;
}PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_RESTORE_INFO_UNT;

/**
* @enum PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_ENT
 *
 * @brief types of HW FDB entries and of manager format.
 *
*/
typedef enum{
    PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E                = 0,
    PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E              = 1,
    PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E              = 2,
    /*APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.*/
    PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E                 = 3,
    /*APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.*/
    PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E       = 4,
    /*APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.*/
    PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E      = 5,
    /*APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.*/
    PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_FCOE_UC_E                 = 6
} PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_ENT;

/**
* @union PRV_CPSS_DXCH_FDB_MANAGER_DB_INTERFACE_INFO_UNT
 *
 * @brief internal Fdb manager DB (database) Entry , target interface : port/trunk/vidx/vid
 *
*/
typedef union{
    /* for type CPSS_INTERFACE_PORT_E */
    struct{
        /* compress 2 fields to 32 bits : using bit fields */
        GT_HW_DEV_NUM   hwDevNum:10;
        GT_PORT_NUM     portNum :22;
    }devPort;
    /* for type CPSS_INTERFACE_TRUNK_E */
    GT_TRUNK_ID     trunkId;
    /* for type CPSS_INTERFACE_VIDX_E */
    GT_U16          vidx;
    /* NOTE: for type CPSS_INTERFACE_VID_E : no extra info needed ! */

    /* NOTE: other types not supported */
}PRV_CPSS_DXCH_FDB_MANAGER_DB_INTERFACE_INFO_UNT;


/**
* @struct PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC
 *
 * @brief internal Fdb manager DB (database) Entry format matching :
 *      PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E
*/
typedef struct{
    /* the mac address (48 bits) */
    GT_U32  macAddr_low_32;     /* in network order those bits hold are the bytes [2..5] */
                                /* in terms of the device those bits hold bits 31..0     */
/* aligned to 32 bits */
    GT_U32  macAddr_high_16 :16;/* in network order those bits hold are the bytes [0..1] */
                                /* in terms of the device those bits hold bits 47..32    */
    GT_U32  valid           : 1;
    GT_U32  skip            : 1;
    GT_U32  age             : 1;
    GT_U32  fid             :13;
/* aligned to 32 bits */
    GT_U32  vid1            :12;/* depend on mux mode : vid1,srcId,userDefined */
    GT_U32  srcId           :12;/* depend on mux mode : vid1,srcId,userDefined */
    GT_U32  daCommand       : 3;
    GT_U32  saCommand       : 3;
    GT_U32  isStatic        : 1;
    GT_U32  daRoute         : 1;
/* aligned to 32 bits */

    GT_U32  daAccessLevel           : 3;
    GT_U32  saAccessLevel           : 3;
    GT_U32  saQosIndex              : 3;
    GT_U32  daQosIndex              : 3;
    GT_U32  spUnknown               : 1;

    /* In case of AC5 - it used for both SA/DA lookup
     * saLookupIngressMirrorToAnalyzerPort - mirrorToAnalyzerPort */
    GT_U32  saLookupIngressMirrorToAnalyzerPort    : 1;
    GT_U32  daLookupIngressMirrorToAnalyzerPort    : 1;
    GT_U32  appSpecificCpuCode      : 1;
    GT_U32  userDefined             :11;/* depend on mux mode : vid1,srcId,userDefined */
    /** @brief destination interface : port/trunk/vidx/vlan */
    GT_U32 dstInterface_type        : 2; /* CPSS_INTERFACE_TYPE_ENT */

/* left with 'spare' of : 32-(3+3+3+3+1+1+1+1+11+2) = 3 bits */

    /* the 'union' open new 32 bits alignment */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INTERFACE_INFO_UNT dstInterface;
}PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC;

/**
* @struct PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC
 *
 * @brief internal Fdb manager DB (database) Entry format matching :
*       PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E and
*       PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E
*/
typedef struct{
    GT_U32  sipAddr;        /* IPv4 = SIP[31:0], where SIP[0] is the least significant bit of the IPv4 destination address.
                               IPv6 = SelectedSIPByte0[7:0] and SelectedSIPByte1[7:0], SelectedSIPByte2[7:0] and SelectedSIPByte3[7:0] */
/* aligned to 32 bits */

    GT_U32  dipAddr;        /* IPv4 = DIP[31:0], where DIP[0] is the least significant bit of the IPv4 destination address.
                               IPv6 = SelectedDIPByte0[7:0] and SelectedDIPByte1[7:0], SelectedDIPByte2[7:0] and SelectedDIPByte3[7:0] */
/* aligned to 32 bits */

    GT_U32  valid           : 1;
    GT_U32  skip            : 1;
    GT_U32  age             : 1;
    GT_U32  fid             :13;
    GT_U32  srcId           :12;
    GT_U32  daCommand       : 3;
    GT_U32  isStatic        : 1;
/* aligned to 32 bits */

    GT_U32  daAccessLevel           : 3;
    GT_U32  spUnknown               : 1;
     /* In case of AC5 - it used for both SA/DA lookup
     * saLookupIngressMirrorToAnalyzerPort - mirrorToAnalyzerPort */
    GT_U32  saLookupIngressMirrorToAnalyzerPort    : 1;
    GT_U32  daLookupIngressMirrorToAnalyzerPort    : 1;
    GT_U32  appSpecificCpuCode      : 1;
    GT_U32  userDefined             :11;
    GT_U32  vid1                    :12;    /* depend on mux mode : vid1_assignment_mode */
    /** @brief destination interface : port/trunk/vidx/vlan */
    GT_U32 dstInterface_type        : 2;    /* CPSS_INTERFACE_TYPE_ENT */

/* aligned to 32 bits */
    GT_U32  daQosIndex              : 3;
    GT_U32  saQosIndex              : 3;
    GT_U32  daRoute                 : 1;

/* left with 'spare' of : 32-(3+3+1) = 25 bits */

    /* the 'union' open new 32 bits alignment */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INTERFACE_INFO_UNT dstInterface;
}PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC;

/**
* @struct PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV_UC_COMMON_FORMAT_STC
 *
 * @brief internal Fdb manager DB (database) Entry format matching :
*       PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E and
*       PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E
        APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*/
typedef struct{
    GT_U32  vrfId                   :12;
    /* the meaning of : pointerInfo
        case of CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E --> tunnelStartPointer
        case of CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E          --> natPointer
        case of CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E          --> arpPointer
        case of CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E                 --> nextHopPointerToRouter
        case of CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E                     --> ecmpPointerToRouter
    */
    GT_U32  pointerInfo             :20;
/* aligned to 32 bits */
    GT_U32  ucRouteExtType          :3;/*of type CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT*/

    GT_U32  valid                   :1;
    GT_U32  skip                    :1;
    GT_U32  age                     :1;


    /* next not relevant to :
        case of CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E
        case of CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E
    */

    GT_U32  nextHopVlanId           :13; /*for ucRouteExtType one of TS/NAT/ARP */
    GT_U32  ttlHopLimitDecEnable    :1;
    GT_U32  ttlHopLimDecOptionsExtChkByPass:1;
    GT_U32  ICMPRedirectEnable      :1;
    GT_U32  mtuProfileIndex         :1;
    GT_U32  countSet                :3;
    GT_U32  qosProfilePrecedence    :1;
    GT_U32  arpBcTrapMirrorEn       :1;
    GT_U32  modifyUp                :2;
    GT_U32  modifyDscp              :2;
/* aligned to 32 bits */

    GT_U32  ingressMirrorToAnalyzerIndex :3;
    GT_U32  qosProfileMarkingEn     :1;
    GT_U32  qosProfileIndex         :7;
    GT_U32  dipAccessLevel          :6;

    /** @brief destination interface : port/trunk/vidx/vlan */
    GT_U32 dstInterface_type        :2; /* CPSS_INTERFACE_TYPE_ENT */

/* aligned to 32 bits - used 19 bits (spare 13 bits) */

    /* the 'union' open new 32 bits alignment */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INTERFACE_INFO_UNT dstInterface;
}PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV_UC_COMMON_FORMAT_STC;

/**
* @struct PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV4_UC_FORMAT_STC
 *
 * @brief internal Fdb manager DB (database) Entry format matching :
 *      PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E
        APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*/
typedef struct{
    GT_U32  ipAddr;
/* aligned to 32 bits */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV_UC_COMMON_FORMAT_STC ipUcCommonInfo;
}PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV4_UC_FORMAT_STC;

/**
* @struct PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_DATA_FORMAT_STC
 *
 * @brief internal Fdb manager DB (database) Entry format matching :
*       PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E
        APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*/
typedef struct{
    GT_U32  ipAddr_127_106          :22;
    GT_U32  ipv6ScopeCheck          :1;
    GT_U32  ipv6DestSiteId          :1;
/* aligned to 32 bits */

    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV_UC_COMMON_FORMAT_STC ipUcCommonInfo;
}PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_DATA_FORMAT_STC;

/**
* @struct PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_KEY_FORMAT_STC
 *
 * @brief internal Fdb manager DB (database) Entry format matching :
*       PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E
        APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*/
typedef struct{
    GT_U32  ipAddr_31_0;
/* aligned to 32 bits */
    GT_U32  ipAddr_63_32;
/* aligned to 32 bits */
    GT_U32  ipAddr_95_64;
/* aligned to 32 bits */
    GT_U32  ipAddr_105_96           :10;

    GT_U32  valid                   :1;
    GT_U32  skip                    :1;
    GT_U32  age                     :1;
/* aligned to 32 bits - used 13 bits (spare 19 bits) */
}PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_KEY_FORMAT_STC;

/* NOTE: we not use 'real pointers' because :
    1. in 64 bits CPU the sizeof(pointer) is 8 bytes while in 32bits CPU it is 4 bytes
    2. and event in 32 bits CPU we can 'compress' it when several pointers needed
       in the same entry
*/
/* 20 bits index - supporting 1M values */
#define ENTRY_PTR_H_NUM_BITS 10 /* 10 bits 'high' (MSBits) */
#define ENTRY_PTR_L_NUM_BITS 10 /* 10 bits 'Low' (LSBits) */
#define ENTRY_PTR_NUM_BITS   (ENTRY_PTR_L_NUM_BITS + ENTRY_PTR_H_NUM_BITS)
/* in many cases should be use as :
    ENTRY_PTR nextEntryPointer : ENTRY_PTR_NUM_BITS;

    in order to 'compress' the 'pointer'
*/
typedef GT_U32  ENTRY_PTR;
typedef GT_U32  ENTRY_PTR_H;
typedef GT_U32  ENTRY_PTR_L;

#define SET_nextEntryPointer_MAC(dbPtr,index) \
    dbPtr->nextEntryPointer_high = (index >> ENTRY_PTR_L_NUM_BITS); \
    dbPtr->nextEntryPointer_low  = (index)/* compiler will chop the extra bits anyway ! */

#define GET_nextEntryPointer_MAC(dbPtr,index) \
    index = (dbPtr->nextEntryPointer_high << ENTRY_PTR_L_NUM_BITS) | \
            dbPtr->nextEntryPointer_low

/* 1 bit field  */
#define GT_BIT_NUM_BITS 1
/* single bit to indicate */
typedef GT_U32  GT_BIT;

/* 20 bits index - supporting 1M values */
#define HW_INDEX_NUM_BITS 20

/* num of bits to the 'age bin' */
#define AGE_BIN_INDEX_NUM_BITS (ENTRY_PTR_NUM_BITS - 8) /* 8 comes from 2^8 = 256 = PRV_FDB_MANAGER_NUM_ENTRIES_STEPS_CNS */

/* the specific 'entry format' */
typedef union{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC    prvMacEntryFormat;
    /* APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV4_UC_FORMAT_STC     prvIpv4UcEntryFormat;
    /* APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_DATA_FORMAT_STC prvIpv6UcDataEntryFormat;
    /* APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman. */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_KEY_FORMAT_STC prvIpv6UcKeyEntryFormat;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC  prvIpv4McEntryFormat;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC  prvIpv6McEntryFormat;
}PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_SPECIFIC_FORMAT;


/**
* @struct PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC
 *
 * @brief internal Fdb manager DB (database) Entry format
 *
 *  NOTE: the format expected to be 'as compressed as possible'
*/
typedef struct{
    /** HW index of the entry */
    GT_U32                     hwIndex               : HW_INDEX_NUM_BITS/*20*/;
    /** Age-list (age-bin) number */
    GT_U32                     ageBinIndex           : AGE_BIN_INDEX_NUM_BITS/*12*/;
/* aligned to 32 bits */
    /** 'entry pointer' to the previous entry */
    GT_BIT                     isValid_prevEntryPointer : GT_BIT_NUM_BITS/*1*/;
    ENTRY_PTR                  prevEntryPointer         : ENTRY_PTR_NUM_BITS/*20*/;

    /** 'entry pointer' to the next     entry */
    GT_BIT                     isValid_nextEntryPointer : GT_BIT_NUM_BITS/*1*/;
    ENTRY_PTR_H                nextEntryPointer_high    : ENTRY_PTR_H_NUM_BITS/*10*/;
/* aligned to 32 bits */
    ENTRY_PTR_L                nextEntryPointer_low     : ENTRY_PTR_L_NUM_BITS/*10*/;

    /** 'entry pointer' to the previous entry in 'age-list' */
    GT_BIT                     isValid_age_prevEntryPointer : GT_BIT_NUM_BITS/*1*/;
    ENTRY_PTR                  age_prevEntryPointer     : ENTRY_PTR_NUM_BITS/*20*/;

    /** 'entry pointer' to the next     entry in 'age-list' */
    GT_BIT                     isValid_age_nextEntryPointer : GT_BIT_NUM_BITS/*1*/;
/* aligned to 32 bits */
    ENTRY_PTR                  age_nextEntryPointer     : ENTRY_PTR_NUM_BITS/*20*/;

    GT_BIT                     isUsedEntry           :GT_BIT_NUM_BITS/*1*/;
    /* metadata common to all entries */
    /** indication about the format of the 'HW' , and about 'specificFormat' ,
        value is one of : PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_ENT */
    GT_U32                     hwFdbEntryType        : 3;

/* aligned to 32 bits (10+10+1+3 = 24) --> spare 8 bits  */

    /* the 'entry format' */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_SPECIFIC_FORMAT                specificFormat;
}PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC;

/*
 * @struct PRV_CPSS_DXCH_FDB_MANAGER_DB_INDEX_STC
 *
 * @brief internal Fdb manager DB (database) 'index' format
 *
 *  NOTE: the format expected to be 'as compressed as possible'
*/
typedef struct{
    /** 'entry pointer' */
    GT_BIT                     isValid_entryPointer: GT_BIT_NUM_BITS/*1*/;
    ENTRY_PTR                  entryPointer        : ENTRY_PTR_NUM_BITS/*20*/;
}PRV_CPSS_DXCH_FDB_MANAGER_DB_INDEX_STC;


/*
 * @struct PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC
 *
 * @brief internal Fdb manager DB (database) 'pointer to node' in the DB
 *
*/
typedef struct{
    /** 'entry pointer' */
    GT_BIT                     isValid_entryPointer: GT_BIT_NUM_BITS/*1*/;
    ENTRY_PTR                  entryPointer        : ENTRY_PTR_NUM_BITS/*20*/;
}PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC;

/* number of words in the mac entry */
#define FDB_ENTRY_WORDS_SIZE_CNS        5
/* number of words in the mac entry */
#define SIP6_FDB_ENTRY_WORDS_SIZE_CNS   4

typedef struct{
    /* temp place to hold the info for:
       cpssDxChBrgFdbManagerEntryAdd(...) :
       format of the HW needed for the    : for SIP 4/5/6 devices
       sip6_hwFormatWords & sip6_ipv6Ext_hwFormatWords - Will be used for SIP4 also.
       Device specific buffers will be used while writing, while reading common sip6 buffer will be used
    */
    GT_U32                          sip6_hwFormatWords           [FDB_ENTRY_WORDS_SIZE_CNS];
    GT_U32                          sip6_ipv6Ext_hwFormatWords   [FDB_ENTRY_WORDS_SIZE_CNS];
    GT_U32                          sip5_20_hwFormatWords        [FDB_ENTRY_WORDS_SIZE_CNS];
    GT_U32                          sip5_20_ipv6Ext_hwFormatWords[FDB_ENTRY_WORDS_SIZE_CNS];
    GT_U32                          sip5_hwFormatWords           [FDB_ENTRY_WORDS_SIZE_CNS];
    GT_U32                          sip5_ipv6Ext_hwFormatWords   [FDB_ENTRY_WORDS_SIZE_CNS];

    /* @brief Temp variable to store the HW AUQ messages, in case of Learning scan */
    PRV_CPSS_AU_DESC_EXT_8_STC      hwAuMessages[AU_MAX_NUM_CNS];

    /* @brief Temp variable to store the last index & total index to hw AU Messages,
     * In case messages read but not processed */
    GT_U32                          lastHwAuMessageIndex;
    GT_U32                          lastScanTotalNumOfAUs;

    /* @brief Temp variable to store the portGroup info
     * relevant ONLY for update events */
    GT_U32                          portGroupIds[AU_MAX_NUM_CNS];

}PRV_CPSS_DXCH_FDB_MANAGER_DB_TEMP_OUT_OF_STACK_INFO_STC;

/**
* @struct PRV_CPSS_DXCH_BRG_FDB_MANAGER_NO_SPACE_CACHE_STC
*
* @brief FDB manager no space event cache
*/
typedef struct{

    /* @brief Indicates the used entry keys, to be compared
     * (APPLICABLE RANGES: 1..4) */
    GT_U8       usedEntriesCnt;

    /* @brief Indicates the index updated last time,
     *        In case cache is full (usedEntryBitMap == 0xF), overwrite the next entry of lastUpdatedEntry
     *  (APPLICABLE RANGES: 0..3)
     */
    GT_U8       lastUpdatedEntry;

    /* @brief last 4 no-space entry
           Each Entry will have the key value only (Other fields will be masked while storing) */
    GT_U32      noSpaceKeyCache[4][CPSS_DXCH_FDB_AU_MSG_WORDS_SIZE_CNS];

} PRV_CPSS_DXCH_BRG_FDB_MANAGER_NO_SPACE_CACHE_STC;

/**
* @enum PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_ENT
 *
 * @brief This enum defines the type of threshold
*/
typedef enum{

    /** @brief The FDB utilization is under Threshold_A */
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_A_E,

    /** @brief The FDB utilization in under Threshold_B */
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_B_E,

    /** @brief The FDB utilization is under Threshold_C */
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_C_E,

    /** @brief The FDB utilization in not monitored */
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_NOT_VALID_E

} PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_ENT;

/**
* @enum PRV_CPSS_DXCH_BRG_FDB_MANAGER_AGING_BIN_STC
 *
 * @brief FDB Manager aging bin
*/
typedef struct{
    /** @brief indicates age-bin's occupancy level */
    GT_U32                                      totalUsedEntriesCnt;

    /** @brief indicates age-bin's starting entry pointer */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC   headOfAgeBinEntryList;
}PRV_CPSS_DXCH_BRG_FDB_MANAGER_AGING_BIN_STC;

/**
* @enum PRV_CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC
 *
 * @brief FDB Manager aging bin related info
 *        These structures are allocated at the time of FDB instance creation
 *        and freed at the time of delete instance
*/
typedef struct{
    /** @brief indicates the age-bin ID to be scanned,
     *         aging scan API updates this at the end of each aging process*/
    GT_U32     currentAgingScanBinID;

    /** @brief level1 age bin usage matrix
     *         Range [0-128 Bytes]
     *         in case of AC5 [0-1 Bytes] */
    GT_U32    *level1UsageBitmapPtr;

    /** @brief level2 age bin usage matrix
     *         Range [0-4 Bytes] */
    GT_U32    level2UsageBitmap;

    /** @brief indicates the number of age-bins created at the manager creation time
     *         Range [0-1024]
     *         in case of AC5 [0-64] */
    GT_U32     totalAgeBinAllocated;

    /** @brief list of age bins, array index is the age-bin ID.
     *         Range [0-1024]
     *         in case of AC5 [0-64] */
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_AGING_BIN_STC *ageBinListPtr;
}PRV_CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC;

/* counter for each entry type */
typedef enum{
     PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_STATIC_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_MULTICAST_DYNAMIC_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_MULTICAST_STATIC_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV4_MULTICAST_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_MULTICAST_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV4_UNICAST_ROUTE_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_UNICAST_ROUTE_KEY_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_UNICAST_ROUTE_DATA_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_FCOE_UNICAST_ROUTE_E

    ,PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE__LAST__E
}PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_ENT;

/*
 * @struct PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TYPE_COUNTERS_STC
 *
 * @brief internal Fdb manager DB (database) 'per entry type' counters
 *
*/
typedef struct{
    /** number of entries of the entry type */
    GT_U32              entryTypePopulation;
}PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TYPE_COUNTERS_STC;

/*
 * @struct PRV_CPSS_DXCH_FDB_MANAGER_BANK_INFO_STC
 *
 * @brief internal Fdb manager DB (database) 'per bank' info
 *
*/
typedef struct{
    /** number of entries in the bank (counter) */
    GT_U32              bankPopulation;
}PRV_CPSS_DXCH_FDB_MANAGER_BANK_INFO_STC;


/* statistics about the 'entry add'  API cpssDxChBrgFdbManagerEntryAdd(...) */
typedef enum{
     PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_BAD_STATE_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_ENTRY_EXIST_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TABLE_FULL_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_HW_UPDATE_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_PORT_LIMIT_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TRUNK_LIMIT_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_FID_LIMIT_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_LIMIT_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_0_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_1_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_2_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_3_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_4_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_5_AND_ABOVE_E


    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ENT;

/* statistics about the 'entry delete'  API cpssDxChBrgFdbManagerEntryDelete(...) */
typedef enum{
     PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_BAD_STATE_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_ENTRY_NOT_FOUND_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_HW_UPDATE_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_OK_E

    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ENT;

/* statistics about the 'temp entry delete'  API cpssDxChBrgFdbManagerEntryTempDelete(...) */
typedef enum{
     PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TEMP_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TEMP_DELETE_API_STATISTICS_OK_E

    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TEMP_DELETE_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TEMP_DELETE_API_STATISTICS_ENT;

/* statistics about the 'entry update'  API cpssDxChBrgFdbManagerEntryUpdate(...) */
typedef enum{
     PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_BAD_STATE_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_NOT_FOUND_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_HW_UPDATE_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_PORT_LIMIT_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_TRUNK_LIMIT_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_FID_LIMIT_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_OK_E

    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ENT;

/* statistics about the 'Learning Scan'  API cpssDxChBrgFdbManagerLearningScan(...) */
typedef enum{
     PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_ERROR_FAILED_TABLE_UPDATE_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NO_SPACE_MSG_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NO_SPACE_FILTERED_MSG_E

    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_ENT;

/* statistics about the 'Transplant Scan'  API cpssDxChBrgFdbManagerTransplantScan(...) */
typedef enum{
     PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_OK_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_OK_NO_MORE_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_ERROR_FAILED_TABLE_UPDATE_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_TOTAL_TRANSPLANTED_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_TOTAL_TRANSPLANTED_REACH_LIMIT_DELETED_E

    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_ENT;

/* statistics about the 'Delete Scan'  API cpssDxChBrgFdbManagerDeleteScan(...) */
typedef enum{
     PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_OK_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_OK_NO_MORE_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_ERROR_FAILED_TABLE_UPDATE_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_TOTAL_DELETED_E

    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_ENT;

/* statistics about the 'Age Scan'  API cpssDxChBrgFdbManagerAgingScan(...) */
typedef enum{
     PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_OK_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_ERROR_FAILED_TABLE_UPDATE_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_DELETED_E

    ,PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_ENT;

/* statistics about the 'HA Sync'  API cpssDxChBrgFdbManagerHighAvailabilityEntriesSync(...) */
typedef enum{
     PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_VALID_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_INVALID_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_SP_ENTRY_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_IPV6_DATA_INVALID_ENTRY_E

    ,PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_ENT;

/* statistics about the 'Entries Rewrite'  API cpssDxChBrgFdbManagerEntryRewrite(...) */
typedef enum{
     PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS_OK_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS_TOTAL_REWRITE_E

    ,PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS_ENT;

/* statistics about the 'limit set'  API cpssDxChBrgFdbManagerLimitSet(...) */
typedef enum{
     PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_PORT_LIMIT_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_TRUNK_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_GLOBAL_EPORT_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_FID_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_GLOBAL_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_ERROR_INPUT_E

    ,PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_ENT;

/*
 * @enum PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_ENT
 *
 * @brief internal Fdb manager DB (database) hash calc result for existing/new entry
 *
*/
typedef enum{
    /** @brief there is free palce in HASH for FDB entry */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_FREE_E,
    /** @brief the FDB entry was found but there is no free place in HASH */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E,
    /** @brief neither FDB entry no free place was found in HASH */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_NOT_FOUND_AND_NO_FREE_E
}PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_ENT;

/* The enumeration describes state of current bank when searching for FDB entry in database */
typedef enum {
    /** IPv6->IPv6 - Lookup/Move Bank[0,1] */
    PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_IPV6_TO_IPV6_E,
    /** IPv6->Non-IPv6 - Move Bank[0] */
    PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_IPV6_TO_BANK_0_NON_IPV6_E,
    /** IPv6->Non-IPv6 - Move Bank[1] */
    PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_IPV6_TO_BANK_1_NON_IPV6_E,
    /** IPv6->2*Empty - Free found */
    PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_IPV6_TWO_BANKS_FREE_E,
    /** Non-IPv6->IPv6 - Move Bank[0,1] */
    PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_NON_IPV6_TO_IPV6_E,
    /** Non-IPv6->Non-IPv6 - Lookup/Move Bank[0] */
    PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_NON_IPV6_TO_NON_IPV6_E,
    /** Non-IPv6->Empty - Free found */
    PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_NON_IPV6_BANK_FREE_E,
    /** The banks are occupied but not matched neither for lookup nor to move - skip to next entry*/
    PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_NOT_MATCHED_E,
    /** Not valid banks - should not happen */
    PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_NOT_VALID_BANKS_E
}PRV_CPSS_DXCH_FDB_MANAGER_CALC_ENTRY_STATE_ENT;

/*
 * @struct PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC
 *
 * @brief internal Fdb manager DB (database) hash calc info for existing/new entry
 *
*/
typedef struct{
    /** the final result of the 'lookup' in the DB */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_ENT calcEntryType;
    /** if found this is the pointer to the entry in the DB */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC *dbEntryPtr;
    /** if found this is the pointer to the extended entry in the DB - for IPv6 UC entry */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC *dbEntryExtPtr;
    /** the hash results for the key of the entry */
    GT_U32                              crcMultiHashArr[CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS];
    /** the banks that are already used in the DB , and can't be used for new entry
        the array filled with info for indexes : 0..lastBankIdChecked (inclusive)
        and for index 'paramsPtr->tempEntryOffset' (relevant to 'entrtyAdd')
    */
    GT_U32                              usedBanksArr[CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS];
    /** the last iteration that the 'usedBanksArr' was filled with.
        NOTE: when entry not found this value will hold :
              (fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes - 1) */
    GT_U32                              lastBankIdChecked;
    /** the bankId that on which we found existing entry , or on which we want
        to use for new entry after we checked for 'most populated' bank */
    GT_U32                              selectedBankId;

    /**  the step  between banks - for IPv6 UC entry the step is 2, otherwise the step is 1 */
    GT_U32                              bankStep;
    /** type of entry for statistics */
    PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_ENT   counterType;

    /** depth of Cuckoo -- needed for statistics */
    GT_U32                              rehashStageId;

    /** the bank with fdbEntry on wich rehashing should be applied */
    GT_U32                              rehashStageBank;

    /** the banks steps calculated for hash results for the key of the entry */
    GT_U32                              bankStepArr[CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS];
}PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC;

/*
 * @struct PRV_CPSS_DXCH_FDB_MANAGER_DB_REHASHING_INFO_STC
 *
 * @brief internal structure holds info of rehashing FDB entry.
          Used by 'cuckoo' algorithm to relocate entries in FDB manger database and HW.
 *
*/
typedef struct {
    /** @brief FDB entry needed relocation */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                 entry;
    /** @brief Recalculated hash info for entry to be relocated */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC    calcInfo;
    /** @brief FDB entry HW index. Used in rehashing logic to prevent crosslinks */
    GT_U32                                              entryHwIndex;
}PRV_CPSS_DXCH_FDB_MANAGER_DB_REHASHING_INFO_STC;


/** @brief - define for the max number of FIDs that the manager may hold */
#define PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_FID_CNS              _8K
/** @brief - define for the max number of trunks that the manager may hold */
#define PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_TRUNK_ID_CNS         _4K
/** @brief - define for the max number of hwDevNum that the manager may hold */
#define PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_HW_DEV_NUM_CNS       _1K /*10 bits in HW */
/** @brief - define for the max number of global-eports that the manager may hold */
#define PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_EPORT_CNS            _16K

/*
 * @struct PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_STC
 *
 * @brief internal structure holds info of dynamic UC mac limits.
 *
*/
typedef struct{
    /** @brief the limit (threshold) as was set by the application */
    GT_U32      limitThreshold;
    /** @brief the actual current counter that relate to the threshold */
    GT_U32      actualCounter;
}PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_STC;

/*
 * @struct PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_PER_HW_DEVICE_STC
 *
 * @brief internal structure holds info of dynamic UC mac limits , per hwDevNum
 *
*/
typedef struct{
    /** @brief - the number of ports in perEportPtr */
    GT_U32      numPorts;
    /** @brief - dynamic UC mac number of entries - per eport (on the device)
    *   dynamic allocated according to the 'actual eport' supported.
    *   converting the 'devNum' to 'hwDevNum' and get number of eports from new devices during
    *   cpssDxChBrgFdbManagerDevListAdd(...)
    **/
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_STC   *perEportPtr;

}PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_PER_HW_DEVICE_STC;

typedef struct{
    /*  @brief - hold last error that caused by limit check
        on of the errors :
        GT_LEARN_LIMIT_PORT_ERROR
        GT_LEARN_LIMIT_TRUNK_ERROR
        GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR
        GT_LEARN_LIMIT_FID_ERROR
        GT_LEARN_LIMIT_GLOBAL_ERROR

        if GT_OK - means that no limit violation.
    */
    GT_STATUS       rcError;

    /**  @brief - when limitCountNeeded == GT_TRUE and rcError == GT_OK then the next hold the
        needed to be updated counters:
        globalEport,trunkId,fid,devPort.hwDevNum,devPort.portNum
        if any of those with value GT_NA , means that it is not updated.
    */
    GT_BOOL         limitCountNeeded;

    CPSS_INTERFACE_TYPE_ENT interfaceType;

    GT_PORT_NUM     globalEport;
    GT_U32          trunkId;
    GT_U32          fid;
    CPSS_INTERFACE_DEV_PORT_STC     devPort;
}PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_LAST_LIMIT_INFO_STC;

typedef struct{
    /** @brief - info relevant to add/remove entry .
        in case of transplant/moved entry , the info here is of the 'new interface'
    */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_LAST_LIMIT_INFO_STC lastLimitInfo;
    /** @brief - info relevant to transplant/moved entry .
        the info here is the 'old interface' */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_LAST_LIMIT_INFO_STC oldInterfaceLimitInfo;

    /** @brief - dynamic UC mac number of entries - global limit */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_STC dynamicUcMacLimit_global;
    /** @brief - dynamic UC mac number of entries - per fid */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_STC dynamicUcMacLimit_fid[PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_FID_CNS];
    /** @brief - dynamic UC mac number of entries - per trunk */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_STC dynamicUcMacLimit_trunkId[PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_TRUNK_ID_CNS];
    /** @brief - dynamic UC mac number of entries - per global eport
    *   dynamic allocated according to the 'global eport' range
    **/
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_STC *dynamicUcMacLimit_globalEportPtr;
    /** @brief - the base global eport (lowest eport in the range)
    *   to allow to access the dynamicUcMacLimit_globalEportPtr with index = (eport-base).
    **/
    GT_U32                                                dynamicUcMacLimit_globalEport_base;
    /** @brief - the number of global eport (from base)
     *  this is the number of eports in dynamicUcMacLimit_globalEportPtr
    **/
    GT_U32                                                dynamicUcMacLimit_globalEport_num;
    /** @brief - array of pointers to dynamic UC mac number of entries - per hwDevNum
    *   Each pointer gets allocated during cpssDxChBrgFdbManagerDevListAdd(...)
    *   by converting the 'devNum' to 'hwDevNum'.
    *   the function cpssDxChBrgFdbManagerLimitSet will fail for 'unknown' hwDevNum
    *   (that got dynamicUcMacLimit_perHwDev[hwDevNum] == NULL)
    **/
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_PER_HW_DEVICE_STC* dynamicUcMacLimit_perHwDev[PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_HW_DEV_NUM_CNS];

}PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_INFO_STC;

/**
* @struct PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC
 *
 * @brief internal Fdb manager 'per' instance info
 *      dynamically allocated during cpssDxChBrgFdbManagerCreate(...)
 *      and freed during cpssDxChBrgFdbManagerDelete(...)
*/
typedef struct{
    /* 'external' info from cpssDxChBrgFdbManagerCreate(...) */
    CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC          capacityInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC  entryAttrInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC          learningInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC            lookupInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC             agingInfo;

    /** internal info */
    /* the 'pool of entries' according to size : capacityInfo.maxTotalEntries
       dynamic allocated during 'create manager' and released during 'delete manager'

       index to this array is 'DB index' (0..capacityInfo.maxTotalEntries-1)
    */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *entryPoolPtr;
    /* the 'index array' according to size : capacityInfo.hwCapacity.numOfHwIndexes
       dynamic allocated during 'create manager' and released during 'delete manager'

       index to this array is 'HW index' (0..capacityInfo.hwCapacity.numOfHwIndexes-1)
    */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INDEX_STC          *indexArr;

    /* 'semi pointer' to head of free 'list' of entries in entryPoolPtr */
    /* the head hold the last one that was freed */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC       headOfFreeList;
    /* 'semi pointer' to head of used 'list' of entries in entryPoolPtr */
    /* the head hold the 'oldest' entry added to 'used' list */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC       headOfUsedList;
    /* 'semi pointer' to tail of used 'list' of entries in entryPoolPtr */
    /* the tail hold the 'newest' entry added to 'used' list */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC       tailOfUsedList;

    /* values calculated once instead of every time need to do hash calculations */
    CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC               cpssHashParams;

    /* info about the banks : counters , etc... */
    PRV_CPSS_DXCH_FDB_MANAGER_BANK_INFO_STC         bankInfoArr[CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS];

    /* number of HW indexes per bank = numOfHwIndexes/numOfHashes */
    /* calculated once ... used many times */
    GT_U32                                          maxPopulationPerBank;

    /* Number of banks,
     *  for AC5                  - 4
     *  for Falcon, AC5x, AC5P   - number of hashes */
    GT_U32                                          numOfBanks;

    /** number of entries in the FDB manager (must be in range : 0.. maxTotalEntries) */
    GT_U32                                          totalPopulation;

    /* the counters per entry type */
    PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TYPE_COUNTERS_STC typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE__LAST__E];

    /* statistics about the 'entry add'     API cpssDxChBrgFdbManagerEntryAdd(...) */
    GT_U32                                          apiEntryAddStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E];
    /* statistics about the 'entry delete'  API cpssDxChBrgFdbManagerEntryDelete(...) */
    GT_U32                                          apiEntryDeleteStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS___LAST___E];
    /* statistics about the 'temp entry delete'  API cpssDxChBrgFdbManagerEntryTempDelete(...) */
    GT_U32                                          apiEntryTempDeleteStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TEMP_DELETE_API_STATISTICS___LAST___E];
    /* statistics about the 'entry update'  API cpssDxChBrgFdbManagerEntryUpdate(...) */
    GT_U32                                          apiEntryUpdateStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS___LAST___E];
    /* statistics about the 'Learning scan' API cpssDxChBrgFdbManagerLearningScan(...) */
    GT_U32                                          apiLearningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E];
    /* statistics about the 'Transplant scan' API cpssDxChBrgFdbManagerTransplantScan(...) */
    GT_U32                                          apiTransplantScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS___LAST___E];
    /* statistics about the 'Delete scan' API cpssDxChBrgFdbManagerDeleteScan(...) */
    GT_U32                                          apiDeleteScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS___LAST___E];
    /* statistics about the 'Aging scan' API cpssDxChBrgFdbManagerAgingScan(...) */
    GT_U32                                          apiAgingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS___LAST___E];
    /* statistics about the 'HA scan' API cpssDxChBrgFdbManagerHighAvailabilityEntriesSync(...) */
    GT_U32                                          apiHASyncStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS___LAST___E];
    /* statistics about the 'Entry rewrite' API cpssDxChBrgFdbManagerEntryRewrite(...) */
    GT_U32                                          apiEntriesRewriteStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS___LAST___E];
    /* statistics about the 'Limit Set' API cpssDxChBrgFdbManagerLimitSet(...) */
    GT_U32                                          apiLimitSetArr[PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS___LAST___E];

    /* bitmap of devices registered to the manager */
    GT_U32                                          devsBmpArr[NUM_WORDS_DEVS_BMP_CNS];
    /* number of devices registered to the manager */
    GT_U32                                          numOfDevices;

    /** temp place to hold extra info temporarily needed by implementation of APIs */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_TEMP_OUT_OF_STACK_INFO_STC   tempInfo;

    /** define the supported 'SIP' for the manager , so only relevant devices can registered to it */
    GT_U32                                          supportedSipArr[CPSS_GEN_SIP_BMP_MAX_NUM_CNS];

    /** scan pointers : getNext , transplant , delete */
    /* 'semi pointer' to getNext 'last' entry in entryPoolPtr */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC       lastGetNextInfo;
    /* 'semi pointer' to transplant 'last' entry in entryPoolPtr */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC       lastTranslplantInfo;
    /* 'semi pointer' to delete 'last' entry in entryPoolPtr */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC       lastDeleteInfo;

    /** sanity manager of the number of 'malloc' pointers that are associated
        with this manager
        -- incremented for every 'malloc' related to this manager.
        -- decremented for every 'free' related to this manager.
        -- checked by the 'manager delete' that we are ZERO at the end of operation.
    */
    GT_U32                                          numOfAllocatedPointers;

    /**
     *          ThresholdB  ThresholdC
     * MHT-2    30%         90%
     * MHT-4    40%         90%
     * MHT-8    60%         95%
     * MHT-16   90%         95%
     * e.g for numOfHwIndexes=256k & numOfHashes=16, The calulation is
     *      thresholdB = [90% of 256k] = 230400
     *      thresholdC = [95% of 256k] = 243200
     **/
    GT_U32  thresholdB;
    GT_U32  thresholdC;

    /* @brief the entries in the filtering cache in HW format */
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_NO_SPACE_CACHE_STC noSpaceCacheEntry;

    /* @brief the entries in the filtering cache */
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_ENT currentThresholdType;

    /* @brief the aging bin related info */
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC agingBinInfo;

    /* @brief number of rehashing stages (must be in range : 0.. CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_REHASHING_STAGES_CNS) */
    GT_U32 totalStages;

    /* @brief trace of rehashing FDB entries with rehashing info */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_REHASHING_INFO_STC rehashingTraceArray[PRV_CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_REHASHING_STAGES_CNS];

    /* @brief Self test restore info */
    PRV_CPSS_DXCH_FDB_MANAGER_DEBUG_RESTORE_INFO_UNT selfTestRestoreInfo[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];

    /** @brief indicates the test cases, which are currently error state.(injected by prvCpssDxChFdbManagerDebugSelfTest)
     *         Range [0-3 Bytes] */
    GT_U32    selfTestBitmap;

    /** @brief the info allocated in runtime only
        if the application set : fdbManagerPtr->capacityInfo.enableLearningLimits == GT_TRUE
        this to support BWC on the memory consumption.
    */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_INFO_STC  *dynamicUcMacLimitPtr;

    /** @brief content of HW entry used for flush operation:
     *  SIP_4  - first word 0x3 (Valid but Skipped), other words are zero
     *  other device - all words are zero. */
    GT_U32   hwEntryForFlush[FDB_ENTRY_WORDS_SIZE_CNS];

    /** @brief Valid incase shadow type is Hybrid/SIP5:
     *  Indicates types of devices added to the hybrid/SIP5 FDB Manager
     *  Hybrid FDB manager can hold - SIP5, SIP5_20, SIP6 devices
     *  SIP5   FDB manager can hold - SIP5, SIP5_20 devices
     *  Bits are set according to PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT */
    GT_U32   hwDeviceTypesBitmap;

}PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC;

/**
* @enum PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT
 *
 * @brief This enum defines the HW format type
*/
typedef enum{

    /** @brief Indicates the HW format is for SIP6 devices */
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E = 1,

    /** @brief Indicates the HW format is for SIP5 devices */
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_E = 2,

    /** @brief Indicates the HW format is for SIP5_20 devices */
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E = 4,

    /** @brief Indicates the HW format is for SIP4 devices */
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP4_E = 8

} PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT;

/**
* @internal prvCpssDxChFdbManagerDbCreate function
* @endinternal
*
* @brief  The function creates the FDB Manager and its databases according to
*        input capacity structure. Internally all FDB global parameters / modes
*        initialized to their defaults (No HW access - just SW defaults that will
*        be later applied to registered devices).
*
*        NOTE: the function doing only DB operations (no HW operations)
*
*        NOTE: incase of 'error' : if the manager was allocated the caller must
*           call prvCpssDxChFdbManagerDbDelete(...) to 'undo' the partial create.
*
* @param[in] fdbManagerId          - the FDB Manager id to associate with the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] capacityPtr           - (pointer to) parameters of the HW structure and capacity of its managed PPs FDBs –
*                                    like number of indexes, the amount of hashes etc.
*                                    As FDB Manager is created and entries may be added before PP registered –
*                                    we must know in advance the relevant capacity –
*                                    in Falcon for example we must know the shared memory allocation mode.
* @param[in] entryAttrPtr          - (pointer to) parameters that affects how HW/SW entries are
*                                    structured and specific fields behavior – including:
*                                    MUX'd fields in MAC / IPMC entries format, SA / DA drop commands mode &
*                                    IP NH packet command.
* @param[in] learningPtr           - (pointer to) parameters that affects which sort of entries to learn etc.
*                                    (Learn MAC of Routed packets and MAC no-space updates).
* @param[in] lookupPtr             - (pointer to) parameters that affect lookup of entries: MAC VLAN Lookup mode
*                                    (none, single or double tags), IVL single-tag MSB mode.
*                                    And two new Falcon setting: IPv4 UC and IPv6 UC masks
*                                    (allow using FDB for prefixes lookup and not just exact match)
* @param[in] agingPtr              - (pointer to) parameters that affect refresh and aging
*                                      (Refresh Destination UC, Refresh Destination MC &Refresh IP UC)
* @param[out] managerAllocatedPtr  - (pointer to) indication that the manager was allocated.
*                                    so on error need to do 'clean-up'
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_CPU_MEM        - on fail to do Cpu memory allocation.
* @retval GT_ALREADY_EXIST         - if the FDB Manager id already exists.
*/
GT_STATUS prvCpssDxChFdbManagerDbCreate
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           *capacityPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             *lookupPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              *agingPtr,
    OUT GT_BOOL                                         *managerAllocatedPtr
);

/**
* @internal prvCpssDxChFdbManagerDbDelete function
* @endinternal
*
* @brief  The function de-allocates specific FDB Manager Instance.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerId            - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
*/
GT_STATUS prvCpssDxChFdbManagerDbDelete
(
    IN GT_U32 fdbManagerId
);

/**
* @internal prvCpssDxChFdbManagerDbDevListAdd function
* @endinternal
*
* @brief   This function adding (registering) device(s) to specific FDB Manager Instance.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] devListArr[]          - the array of device ids to add to the FDB Manager.
* @param[in] numOfDevs             - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerDbDevListAdd
(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[], /*arrSizeVarName=numOfDevs*/
    IN GT_U32 numOfDevs
);

/**
* @internal prvCpssDxChFdbManagerDbDevListRemove function
* @endinternal
*
* @brief   This function removing (unregistering) device(s) from specific FDB Manager Instance.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] devListArr[]          - the array of device ids to remove from the FDB Manager.
* @param[in] numOfDevs             - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NO_SUCH               - if one of the devices not in DB.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerDbDevListRemove
(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[], /*arrSizeVarName=numOfDevs*/
    IN GT_U32 numOfDevs
);


/**
* @internal prvCpssDxChFdbManagerDbIsAnyDeviceRegistred function
* @endinternal
*
* @brief   This function check if is there are registered device.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
*
* @retval 0 - no device registered.
* @retval 1 - at least one device registered.
*/
GT_U32 prvCpssDxChFdbManagerDbIsAnyDeviceRegistred
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr
);

/**
* @internal prvCpssDxChFdbManagerDbDeviceIteratorGetNext function
* @endinternal
*
* @brief   This function get first/next registered device.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] isFirst               - indication to get First (GT_TRUE)  or the next (GT_FALSE)
* @param[in] devNumPtr             - (pointer to) the 'current device' for which we ask for 'next'
*                                     relevant only when isFirst == GT_FALSE (ask for next)
* @param[out] devNumPtr            - (pointer to) the 'first/next device'
*
* @retval GT_OK - there is 'first/next device'
* @retval GT_NO_MORE - there are no more devices
*                if isFirst == GT_TRUE --> no devices at all
*                else                  --> no more devices after 'current' (IN *devNumPtr)
*/
GT_STATUS prvCpssDxChFdbManagerDbDeviceIteratorGetNext
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr,
    IN  GT_BOOL                                 isFirst,
    INOUT GT_U8                                 *devNumPtr
);

/**
* @internal prvCpssDxChFdbManagerDbFdbEntryIteratorGetNext function
* @endinternal
*
* @brief   This function get first/next valid (pointer to) entry from the manager.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] isFirst               - indication to get First (GT_TRUE)  or the next (GT_FALSE)
* @param[in] devNumPtr             - (pointer to) the '(pointer to)current entry' for which we ask for 'next'
*                                     relevant only when isFirst == GT_FALSE (ask for next)
* @param[out] devNumPtr            - (pointer to) the '(pointer to)first/next entry'
*
*
* @retval GT_OK - there is 'first/next valid entry' from the manager
* @retval GT_NO_MORE - there are no more entries
*                      if isFirst == GT_TRUE --> no entries at all
*                      else                  --> no more entries after 'current' (IN *dbEntryPtrPtr)
*         GT_BAD_STATE - there is error in DB management.
*
*/
GT_STATUS prvCpssDxChFdbManagerDbFdbEntryIteratorGetNext
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN  GT_BOOL                                         isFirst,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC* *dbEntryPtrPtr
);

/**
* @internal prvCpssDxChFdbManagerDbCalcHashAndLookupResult function
* @endinternal
*
* @brief   This function hash calculate place for the new/existing entry.
*          the HASH info filled into calcInfoPtr , with indication if entry found ,
*          or not found and no place for it ,
*          or not found but have place for it.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*                                     the function actually uses only the 'key' of the entry.
* @param[in] addParamsPtr          - (pointer to) info given for 'entry add' that
*                                     may use specific bank from the hash.
*                                     NOTE: ignored if NULL
* @param[out] calcInfoPtr          - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS   prvCpssDxChFdbManagerDbCalcHashAndLookupResult
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *addParamsPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr
);

/**
* @internal prvCpssDxChFdbManagerDbGetMostPopulatedBankId function
* @endinternal
*
* @brief   This function choose the best bank to use for the entry.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] calcInfoPtr           - using calcInfoPtr->usedBanksArr[] ,
*                                    using calcInfoPtr->lastBankIdChecked
* @param[out] calcInfoPtr          - updating calcInfoPtr->selectedBankId
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - not found bank to use
*
*/
GT_STATUS   prvCpssDxChFdbManagerDbGetMostPopulatedBankId
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC       *fdbManagerPtr,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC  *calcInfoPtr
);

/**
* @internal prvCpssDxChFdbManagerDbAddNewEntry function
* @endinternal
*
* @brief   This function get free entry to be used for new added entry.
*           adding new entry :
*           update metadata DB :indexArr[] , bankInfoArr[] , typeCountersArr[]
*           update headOfFreeList , headOfUsedList , tailOfUsedList
*           update calcInfoPtr->dbEntryPtr as valid pointer
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] primaryBank           - selected bank to adding entry:
*                                     GT_TRUE  - primary bank ID
*                                     GT_FALSE - secondary bank ID - for IPv6 FDB entry type only
* @param[in] calcInfoPtr           - using calcInfoPtr->selectedBankId ,
*                                          calcInfoPtr->counterType ,
*                                          calcInfoPtr->crcMultiHashArr[] ,
* @param[out] calcInfoPtr          - updating calcInfoPtr->dbEntryPtr
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE /GT_BAD_PARAM - non-consistency between different parts of
*                                    the manager.
*
*/
GT_STATUS   prvCpssDxChFdbManagerDbAddNewEntry
(
    IN    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN    GT_BOOL                                                 primaryBank,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC        *calcInfoPtr
);

/**
* @internal prvCpssDxChFdbManagerDbDeleteOldEntry function
* @endinternal
*
* @brief   This function return entry to the free list removing from the used list.
*           delete old entry :
*           update metadata DB :indexArr[] , bankInfoArr[] , typeCountersArr[]
*           update headOfFreeList , headOfUsedList , tailOfUsedList
*           update calcInfoPtr->dbEntryPtr about valid pointers
*           lastGetNextInfo , lastTranslplantInfo , lastDeleteInfo
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] primaryBank           - selected bank to adding entry:
*                                     GT_TRUE  - primary bank ID
*                                     GT_FALSE - secondary bank ID - for IPv6 FDB entry type only
* @param[in] calcInfoPtr           - using calcInfoPtr->selectedBankId ,
*                                          calcInfoPtr->counterType ,
*                                          calcInfoPtr->dbEntryPtr ,
*                                          calcInfoPtr->crcMultiHashArr[] ,
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE /GT_BAD_PARAM - non-consistency between different parts of
*                                    the manager.
*
*/
GT_STATUS   prvCpssDxChFdbManagerDbDeleteOldEntry
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN GT_BOOL                                                 primaryBank,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC        *calcInfoPtr
);

/**
* @internal prvCpssDxChFdbManagerDbTableFullDiagnostic function
* @endinternal
*
* @brief   This function do diagnostic to 'FDB full' state
*          the caller function will return this error to the application
*          NOTE: the function always return 'ERROR'. the 'GT_FULL' is expected ,
*                other values are not
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
*
* @retval GT_BAD_STATE             - non-consistency between different parts of
*                                    the manager.
* @retval GT_FULL                  - the table is really FULL (all banks are full).
*
*/
GT_STATUS   prvCpssDxChFdbManagerDbTableFullDiagnostic
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr
);


/*
* @internal prvCpssDxChFdbManagerDbApplicationIteratorEntryGetNext function
* @endinternal
*
* @brief   The function return the first/next (pointer to) entry.
*          it is based on 'internally managed last entry' that the manager
*          returned 'last time' to the application.
*
*          NOTE: this function is to be used only for APPLICATION specific 'get next'
*                and not for iterators inside the manager.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[in] getFirst              - indication to get First (GT_TRUE)  or the next (GT_FALSE)
* @param[inout] lastGetNextInfoPtr - Indicates the last DB node fetched by this scan
*                                    OR - DB pointer from where to start the next search.
* @param[out] dbEntryPtrPtr        - (pointer to)  the '(pointer to)first/next entry'
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_BAD_PARAM             - the iterator is not valid for the 'get next' operation
*                                    the logic found coherency issue.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerDbApplicationIteratorEntryGetNext
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr,
    IN GT_BOOL                                          getFirst,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC    *lastGetNextInfoPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC **dbEntryPtrPtr
);

/*
* @internal prvCpssDxChFdbManagerDbIsEntryKeyMatchDbEntryKey function
* @endinternal
*
* @brief   The function check if the key of the entry from application (entryPtr)
*          match                 the key of the entry from the manager DB (dbEntryPtr).
*          the result retrieved in (*isMatchPtr)
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*                                     the function actually uses only the 'key' of the entry.
* @param[in] dbEntryPtr            - (pointer to) the entry (manager format)
*                                     the function actually uses only the 'key' of the entry.
* @param[out] isMatchPtr           - (pointer to) is the key match ?
*                                    GT_TRUE  - the key     match
*                                    GT_FALSE - the key not match
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerDbIsEntryKeyMatchDbEntryKey
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                *entryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC      *dbEntryPtr,
    OUT GT_BOOL                                           *isMatchPtr
);

/*
* @internal prvCpssDxChFdbManagerDbIsIpv6UcEntryKeyMatchDbEntryKey function
* @endinternal
*
* @brief   The function checks is the key of the IPv6 UC entry from application (entryPtr)
*          match                 the key of the IPv6 UC entry from the manager DB (dbEntryPtr).
*          the result retrieved in (*isMatchPtr)
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] dbEntryPtr            - (pointer to) the entry (manager format)
*                                     the function actually uses only the 'key' of the entry.
* @param[in] dbEntryExtPtr         - (pointer to) the extended entry (manager format)
*                                     valid for Ipv6 UC entriy or NULL otherwise
* @param[out] isMatchPtr           - (pointer to) is the key match ?
*                                    GT_TRUE  - the key     match
*                                    GT_FALSE - the key not match
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerDbIsIpv6UcEntryKeyMatchDbEntryKey
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                *entryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC      *dbEntryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC      *dbEntryExtPtr,
    OUT GT_BOOL                                           *isMatchPtr
);

/*
* @internal prvCpssDxChBrgFdbManagerDbDevListGet function
* @endinternal
*
* @brief The function returns current FDB Manager registered devices (added by 'add device' API).
*
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[inout] numOfDevicesPtr    - (pointer to) the number of devices in the array deviceListArray as 'input parameter'
*                                    and actual number of devices as 'output parameter'.
* @param[out] deviceListArray[]    - array of registered devices (no more than list length).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS prvCpssDxChBrgFdbManagerDbDevListGet
(
    IN GT_U32           fdbManagerId,
    INOUT GT_U32        *numOfDevicesPtr,
    OUT GT_U8           deviceListArray[] /*arrSizeVarName=*numOfDevicesPtr*/
);

/*
* @internal prvCpssDxChBrgFdbManagerDbConfigGet function
* @endinternal
*
* @brief The function fetches current FDB Manager setting as they were defined in FDB Manager Create.
*
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] capacityPtr          - (pointer to) parameters of the HW structure and capacity of its managed PPs FDBs:
*                                    like number of indexes, the amount of hashes etc.
*                                    As FDB Manager is created and entries may be added before PP registered,
*                                    we must know in advance the relevant capacity.
*                                    In Falcon, for example, we must know the shared memory allocation mode.
* @param[out] entryAttrPtr         - (pointer to) parameters that affects how HW/SW entries are
*                                    structured and specific fields behavior including:
*                                    MUX'd fields in MAC / IPMC entries format, SA / DA drop commands mode and
*                                    IP NH packet command.
* @param[out] learningPtr          - (pointer to) parameters that affects which sort of entries to learn etc.
*                                    (Learn MAC of Routed packets and MAC no-space updates).
* @param[out] lookupPtr            - (pointer to) parameters that affect lookup of entries: MAC VLAN Lookup mode
*                                    (none, single or double tags), IVL single-tag MSB mode.
*                                    And two new Falcon setting: IPv4 UC and IPv6 UC masks
*                                    (allow using FDB for prefixes lookup and not just exact match)
* @param[out] agingPtr             - (pointer to) parameters that affect refresh and aging
*                                      (Refresh Destination UC, Refresh Destination MC &Refresh IP UC)
*
*/
void prvCpssDxChBrgFdbManagerDbConfigGet
(
    IN  GT_U32                                          fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC          *capacityPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC          *learningPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC            *lookupPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC             *agingPtr
);

/*
* @prvCpssDxChBrgFdbManagerDbCountersGet function
* @endinternal
*
* @brief The function fetches FDB Manager counters.
*
*
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] countersPtr          - (pointer to) FDB Manager Counters.
*
*/
void prvCpssDxChBrgFdbManagerDbCountersGet
(
    IN GT_U32                                           fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC          *countersPtr
);

/*
* @prvCpssDxChBrgFdbManagerDbStatisticsGet function
* @endinternal
*
* @brief The function fetches FDB Manager statistics.
*
*
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] statisticsPtr        - (pointer to) FDB Manager Statistics.
*
*/
void prvCpssDxChBrgFdbManagerDbStatisticsGet
(
    IN GT_U32                                           fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC        *statisticsPtr
);

/*
* @prvCpssDxChBrgFdbManagerDbStatisticsClear function
* @endinternal
*
* @brief The function clears FDB Manager statistics.
*
*
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*/
void prvCpssDxChBrgFdbManagerDbStatisticsClear
(
    IN GT_U32                                           fdbManagerId
);

/*
* @internal prvCpssDxChFdbManagerDbAgeBinEntryGetNext function
* @endinternal
*
* @brief   The function return the first/next (pointer to) age-bin entry.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[in] getFirst              - indication to get First (GT_TRUE)  or the next (GT_FALSE)
* @param[inout] lastGetNextInfoPtr - Indicates the last DB node in the ageBin fetched by this agingscan
*                                    OR - DB pointer from where to start the next search in the ageBin.
* @param[out] dbEntryPtrPtr        - (pointer to)  the '(pointer to)first/next age-bin entry'
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_BAD_PARAM             - the iterator is not valid for the 'get next' operation
*                                    the logic found coherency issue.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerDbAgeBinEntryGetNext
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN GT_BOOL                                          getFirst,
    IN GT_U32                                           ageBinID,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC     *lastGetNextInfoPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   **dbEntryPtrPtr
);

/**
* @internal prvCpssDxChBrgFdbManagerRehashBankFdbEntryGet function
* @endinternal
*
* @brief   This function retrieves FDB entry from calcInfo per bank from HASH calculated in prvCpssDxChFdbManagerDbHashCalculate.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB manager
* @param[in] bankId                 - bank ID to get rehashing calc info 
* @param[in] bankStep               - rehashing entry bank step
* @param[in] calcInfoPtr            - (pointer to) the calculated hash and lookup results
* @param[out] entryPtr              - (pointer to) the current FDB entry that need to be rehashed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS  prvCpssDxChBrgFdbManagerRehashBankFdbEntryGet
(
    IN      PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN      GT_U32                                            bankId,
    IN      GT_U32                                            bankStep,
    IN      PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr,
    OUT     CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr
);

/**
* @internal prvCpssDxChFdbManagerDbRehashEntry function
* @endinternal
*
* @brief   This function re-hash index for the new entry that can't be added to HW
*          when all banks for the current hash index are occupied.
*
*          NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB manager.
* @param[in] rehashStageId          - rehashing stage level 
* @param[out] calcInfoPtr           - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                   - on operation fail
*
*/
GT_STATUS   prvCpssDxChFdbManagerDbRehashEntry
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN GT_U32                                           rehashStageId,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr
);

/**
* @internal prvCpssDxChBrgFdbManagerRestoreEntryIpV6UcFormatParamsFromDb function
* @endinternal
*
* @brief  function to restore values of the FDB entry for IpV6 UC from DB
*         copy values from manager format to application format
*         the 'opposite' logic of saveEntryFormatParamsToDb(...)
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
* @param[in] dbEntryExtPtr          - (pointer to) the manager extended entry format (manager format)
* @param[out] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS   prvCpssDxChBrgFdbManagerRestoreEntryIpV6UcFormatParamsFromDb
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC  *dbEntryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC  *dbEntryExtPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC           *entryPtr
);

/**
* @internal prvCpssDxChBrgFdbManagerRestoreEntryFormatParamsFromDb function
* @endinternal
*
* @brief  function to restore values of the FDB entry from DB
*         copy values from manager format to application format
*         the 'opposite' logic of saveEntryFormatParamsToDb(...)
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
* @param[out] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS   prvCpssDxChBrgFdbManagerRestoreEntryFormatParamsFromDb
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC  *dbEntryPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC           *entryPtr
);

/*
* @internal prvCpssDxChBrgFdbManagerDbCheck function
* @endinternal
*
* @brief The function verifies the FDB manager DB.
*
* @param[in]  fdbManagerId         - The FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in]  checksPtr            - (Pointer to) DB check attributes
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS prvCpssDxChBrgFdbManagerDbCheck
(
    IN  GT_U32                                           fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC          *checksPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT    resultArray[],
    OUT GT_U32                                           *errorNumPtr
);

/*
* @internal prvCpssDxChFdbManagerDebugSelfTest function
* @endinternal
*
* @brief The function injects/corrects error in FDB manager DB, as mentioned in expResultArray
*
* @param[in] fdbManagerId         - The FDB Manager id.
*                                   (APPLICABLE RANGES : 0..31)
* @param[in] expResultArray       - (pointer to) Test error types array.
* @param[in] errorNumPtr          - (pointer to) Number of error types filled in expResultArray
* @param[in] errorEnable          - Status value indicates error/correct state
*                                   GT_TRUE  - Error need to injected according to expResultArray and expErrorNum
*                                   GT_FALSE - Error need to be corrected according to expResultArray and expErrorNum
*
* @retval GT_OK                   - on success
* @retval GT_FAIL                 - on error
*
*/
GT_STATUS prvCpssDxChFdbManagerDebugSelfTest
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT   expResultArray[],
    IN GT_U32                                           expErrorNum,
    IN GT_BOOL                                          errorEnable
);

/**
* @internal prvCpssDxChFdbManagerDbBuildDbFormatFromHwEntry function
* @endinternal
*
* @brief  function to build the DB entry
*         the IN  format is 'HW format'
*         the OUT format is 'DB format'
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[out] dbEntryPtr           - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS prvCpssDxChFdbManagerDbBuildDbFormatFromHwEntry
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN  PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT  hwFormatType,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *dbEntryPtr
);

/**
* @internal prvCpssDxChFdbManagerDbHashCalculate function
* @endinternal
*
* @brief   This function calculates hash for the new/existing entry.
*
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*                                     the function actually uses only the 'key' of the entry.
* @param[out] calcInfoPtr          - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS   prvCpssDxChFdbManagerDbHashCalculate
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC    *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC               *entryPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr
);

typedef enum{
    /**@brief check if new entry will cause over the limit
      *     or update counter on new entry
    */
     LIMIT_OPER_ADD_NEW_E
    /**@brief check if current state is over the limit */
    ,LIMIT_OPER_CHECK_CURRENT_E
    /**@brief update counter on delete */
    ,LIMIT_OPER_DELETE_OLD_E
    /**@brief get info on the old interface */
    ,LIMIT_OPER_INTERFACE_UPDATE_OLD_PART_INFO_GET_E
    /**@brief check counters on the new interface , only for those differ
       from the old part */
    ,LIMIT_OPER_INTERFACE_UPDATE_NEW_PART_INFO_GET_E
    /**@brief update counters (--) on the old interface
     *        update counters (++) on the new interface
    */
    ,LIMIT_OPER_INTERFACE_UPDATE_E
}LIMIT_OPER_ENT;

/**
* @internal prvCpssDxChFdbManagerDbIsOverLimitCheck function
* @endinternal
*
* @brief  function to check if the entry violate any of the dynamic UC MAC limits.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] oper                  - options are :
*                                     1. LIMIT_OPER_CHECK_CURRENT_E
*                                     2. LIMIT_OPER_INTERFACE_UPDATE_OLD_PART_INFO_GET_E
* @param[in] dbEntryIpUcPtr        - (pointer to) the MAC format (DB entry)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_TRUE              - the entry     violate one of the limits
* @retval GT_FALSE             - the entry not violate any of the limits
*/
GT_BOOL prvCpssDxChFdbManagerDbIsOverLimitCheck(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN LIMIT_OPER_ENT                                           oper,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC  *dbEntryMacPtr
);

/**
* @internal prvCpssDxChFdbManagerDbLimitPerHwDevNumPerPort function
* @endinternal
*
* @brief  the function increment/decrement from the per {hwDevNum,portNum} current
*           counter of the dynamic UC mac limit.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] oper                  - options are :
*                                     1. LIMIT_OPER_ADD_NEW_E
* @param[in] dbEntryIpUcPtr        - (pointer to) the MAC format (DB entry)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                   - on success
* @retval GT_FAIL                 - on error
*/
GT_STATUS prvCpssDxChFdbManagerDbLimitPerHwDevNumPerPort(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN LIMIT_OPER_ENT                                          oper,
    IN GT_HW_DEV_NUM                                           hwDevNum,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC  *dbEntryMacPtr
);

/**
* @internal prvCpssDxChFdbManagerDbIsGlobalEport function
* @endinternal
*
* @brief  function to check if the portNum is in the global eport range/mask
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] portNum               - the portNum to check
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_TRUE              - the portNum is     considered 'Global Eport'
* @retval GT_FALSE             - the portNum is not considered 'Global Eport'
*/
GT_BOOL prvCpssDxChFdbManagerDbIsGlobalEport(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN GT_PORT_NUM                                              portNum
);


/**
* @internal prvCpssDxChFdbManagerDbIsOverLimitCheck_applicationFormat function
* @endinternal
*
* @brief  function to check if the entry violate any of the dynamic UC MAC limits.
*           the entry in in 'application format'
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] oper                  - options are :
*                                     1. LIMIT_OPER_ADD_NEW_E
*                                     2. LIMIT_OPER_INTERFACE_UPDATE_NEW_PART_INFO_GET_E
* @param[in] entryMacPtr           - (pointer to) the MAC format (DB entry)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_TRUE              - the entry     violate one of the limits
* @retval GT_FALSE             - the entry not violate any of the limits
*/
GT_BOOL prvCpssDxChFdbManagerDbIsOverLimitCheck_applicationFormat(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC       *fdbManagerPtr,
    IN LIMIT_OPER_ENT                                        oper,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC  *entryMacPtr
);

/**
* @internal prvCpssDxChFdbManagerDbLimitOper function
* @endinternal
*
* @brief  function to update the dynamic UC MAC current counters. (doing ++ or -- on the counters)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] oper                  - options are :
*                                     1. LIMIT_OPER_ADD_NEW_E
*                                     2. LIMIT_OPER_DELETE_OLD_E
*                                     3. LIMIT_OPER_INTERFACE_UPDATE_E:
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                - on success
* @retval GT_BAD_STATE         - DB coherency issue
*/
GT_STATUS prvCpssDxChFdbManagerDbLimitOper(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN LIMIT_OPER_ENT                                           oper
);

/**
* @internal prvCpssDxChFdbManagerDbIsOverLimitCheck_newSpecificInterface function
* @endinternal
*
* @brief  function to check if the entry violate any of the dynamic UC MAC limits.
*       the function is called only for UC mac entries.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] old_dbEntryMacPtr     - (pointer to) the MAC format with old interface (DB entry)
* @param[in] new_interfacePtr      - (pointer to) the new interface.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_TRUE              - the new interface     violate one of the interface limits
* @retval GT_FALSE             - the new interface not violate any of the interface limits
*
*/
GT_BOOL prvCpssDxChFdbManagerDbIsOverLimitCheck_newSpecificInterface(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC  *old_dbEntryMacPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC      *new_interfacePtr
);

/**
* @internal prvCpssDxChFdbManagerDbUpdateManagerParams function
* @endinternal
*
* @brief  The function validate 'API level' 'manager config update' parameters.
*         The function save those updated values to the DB.
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerId          - the FDB Manager id to associate with the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryAttrPtr          - (pointer to) parameters to override entryAttrPtr
*                                       that was given during 'manager create' API.
*                                     NOTE: ignored if NULL
* @param[in] learningPtr           - (pointer to) parameters to override learningPtr
*                                       that was given during 'manager create' API.
*                                     NOTE: ignored if NULL
* @param[in] agingPtr              - (pointer to) parameters to override agingPtr
*                                       that was given during 'manager create' API.
*                                     NOTE: ignored if NULL
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS    prvCpssDxChFdbManagerDbUpdateManagerParams
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC         *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC           *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC              *agingPtr
);

/**
* @internal prvCpssDxChBrgFdbManagerDbEntryGet function
* @endinternal
*
* @brief   Returns DB entry per bank ID
*           
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB manager 
* @param[in] bankId                 - bank ID to get info from calc
* @param[inout] calcInfoPtr         - (pointer to) the calculated hash and lookup results
* @param[out] dbEntryPtrPtr         - (pointer to)(pointer to) the entry in memory pool 
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - wrong parameter
*/
GT_STATUS prvCpssDxChBrgFdbManagerDbEntryGet
(
    IN    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN    GT_U32                                            bankId,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr,
    OUT   PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    **dbEntryPtrPtr
);

/**
* @internal prvCpssDxChBrgFdbManagerBankCalcInfoGet function
* @endinternal
*
* @brief   This function gets DB calc info per bank in hash calculated in prvCpssDxChFdbManagerDbCalcHashAndLookupResult.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB manager
* @param[in] bankId                 - bank ID to get calc info 
* @param[in] bankStep               - bank step to get calc info 
* @param[in] entryPtr               - (pointer to) the current FDB entry need to be rehashed
* @param[in] addParamsPtr           - (pointer to) info given for 'entry add' that
*                                     may use specific bank from the hash.
*                                     NOTE: ignored if NULL
* @param[inout] calcInfoPtr         - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS  prvCpssDxChBrgFdbManagerBankCalcInfoGet
(
    IN    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN    GT_U32                                            bankId,
    IN    GT_U32                                            bankStep,
    IN    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *addParamsPtr,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChBrgFdbManager_db_h */


