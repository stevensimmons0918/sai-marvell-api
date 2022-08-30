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
* @file cpssDxChHsrPrp.h
*
* @brief CPSS DxCh HSR and PRP configuration API
*   NOTE: some of the function or fields are only for HSR or for PRP.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChHsrPrph
#define __cpssDxChHsrPrph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>

/** List of Acronyms used in the API names
    PRP        Parallel Redundancy Protocol
    HSR        High-availability Seamless Redundancy
    RCT        Redundancy Control Trailer
    LSDU       Link service data unit (data after the vlan tag and next ethertype)
               this is 'payload'
    SNG        Sequence Number Generation
    IRF        Individual Recovery Function
    SRF        Sequence Recovery Function
    DDE        Duplicate Discard Entry (FDB entry type)
    DDL        Duplicate Discard Lookup
*/


/**
* @define CPSS_DXCH_HSR_PRP_H_PORTS_NUM_CNS
 *
 * @brief the number of HPorts supported by the device
*/
#define CPSS_DXCH_HSR_PRP_H_PORTS_NUM_CNS   10

/**
* @enum CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT
 *
 * @brief HSR/PRP exception types
*/
typedef enum{

    /** @brief : exception type :
        The HSR size mismatch */
    CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E,

    /** @brief : exception type :
        The Proxy Node Table is full and
        the packet is not tagged with HSR tag or
        doesn't has PRP trailer  */
    CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_UNTAGGED_E,

    /** @brief : exception type :
        Proxy Node Table is full and
        the packet is tagged with HSR tag or
        has PRP trailer */
    CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_TAGGED_E,

    /** @brief : exception type :
        The target ports bitmap is zero
    */
    CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ZERO_BMP_E

} CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT;

/**
* @enum CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT
 *
 * @brief HSR/PRP ports bmp types
*/
typedef enum{
    /** @brief : ports bmp type :
        filter #1 */
    CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_1_E,
    /** @brief : ports bmp type :
        filter #2 */
    CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_2_E,
    /** @brief : ports bmp type :
        filter #3 */
    CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_3_E,
    /** @brief : ports bmp type :
        ignore DDL decision */
    CPSS_DXCH_HSR_PRP_PORTS_BMP_IGNORE_DDL_DECISION_E,
    /** @brief : ports bmp type :
        ignore source duplication discard */
    CPSS_DXCH_HSR_PRP_PORTS_BMP_IGNORE_SOURCE_DUPLICATION_DISCARD_E,
    /** @brief : ports bmp type :
        source is me cleaning */
    CPSS_DXCH_HSR_PRP_PORTS_BMP_SOURCE_IS_ME_CLEANING_E
} CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT;


/**
* @enum CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT
 *
 * @brief HSR/PRP global counters types
*/
typedef enum{
    /** @brief : global counter type :
        PNT : number of ready entries
        NOTE: Read only , but not clear after read */
    CPSS_DXCH_HSR_PRP_COUNTER_TYPE_PNT_NUM_OF_READY_ENTRIES_E,
    /** @brief : global counter type :
        NOTE: Read only and clear after read */
    CPSS_DXCH_HSR_PRP_COUNTER_OVERRIDEN_NON_EXPIRED_ENTRIES_E

} CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT;

/**
* @enum CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_ENT
 *
 * @brief HSR/PRP PNT lookup modes
*/
typedef enum{
    /** @brief : PNT lookup mode :
        skip */
    CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_SKIP_E,
    /** @brief : PNT lookup mode :
        Source and Learn :
        Do Source MAC address Lookup.
        If the entry doesn't exist Learn it. */
    CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_SRC_AND_LEARN_E,
    /** @brief : PNT lookup mode :
        Destination and Source Lookup:
        Do Destination address lookup and then
        Do source address lookup */
    CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_DST_AND_SRC_E,
    /** @brief : PNT lookup mode :
        Source Lookup:
        Do only source lookup
    */
    CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_SRC_E

} CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_ENT;


/**
* @enum CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_ENT
 *
 * @brief HSR/PRP FDB DDL modes
*/
typedef enum{
    /** @brief : PNT FDB DDL mode :
        Skip : Both lookups are disabled.
    */
    CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_SKIP_E,
    /** @brief : PNT FDB DDL mode :
        DDL : Duplicate Discard Lookup (DDL) is enabled.
    */
    CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_DDL_E,
    /** @brief : PNT FDB DDL mode :
        FDB : FDB Lookup is enabled.
    */
    CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_FDB_E

} CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_ENT;

/**
* @enum CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_ENT
 *
 * @brief HSR/PRP LRE types
 *
*/
typedef enum{
    /** @brief : LRE type :
        LRE type 'A'
    */
    CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_A_E,
    /** @brief : LRE type :
        LRE type 'B'
    */
    CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_B_E

} CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_ENT;

/**
* @struct CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC
 *
 * @brief HSR/PRP HPORT entry fields
 *
*/
typedef struct{

    /** @brief PNT lookup mode.
     */
    CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_ENT    pntLookupMode;
    /** @brief FDB/DDL lookup mode.
     */
    CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_ENT       fdbDdlMode;
    /** @brief enable/disable HSR tag or RCT trailer to learn in the PNT.
        If enabled then packets incoming with HSR tag or RCT trailer are also learned
        in the Proxy Node table (PNT).
        NOTE: Relevant when pntLookupMode is 'SRC_AND_LEARN'

        GT_FALSE - disable the learn in PNT for 'tagged'
        GT_TRUE  - enable  the learn in PNT for 'tagged'
     */
    GT_BOOL        pntLearnTaggedPackets;
    /** @brief accept/deny Duplicated packets from the same ingress port
        If enabled the Duplicated packets from the same ingress port would not be dropped
        GT_FALSE - deny   Duplicated packets (and such are     dropped)
        GT_TRUE  - accept Duplicated packets (and such are NOT dropped)

        NOTE:
            for PRP use GT_TRUE  (accept)
            for HRS use GT_FALSE (deny)
     */
    GT_BOOL        acceptSamePort;
    /** @brief LRE ring type for counter .
        select type 'A' or 'B'
        NOTE:
        1. Relevant when the port is Ring Port.
        2. relevant to the counter only.
            (no limit to have LRE 0x0..0xF , not only 0xA,0xB)
     */
    CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_ENT      lreRingPortType;
    /** @brief The LRE instance that the HPort belongs to
        APPLICABLE RANGE: 0..3
     */
    GT_U32         lreInstance;
    /** @brief Bitmap of the HSR/PRP ports that describes the Link Redundancy Entity (LRE)
        forwarding rules before applying the Proxy Node and Duplicate Discard Lookups.
        If a bit in the Bitmap is set, it means that packets are forwarded from the
        source port (the index to this entry) to the port associated with that bit.
        APPLICABLE RANGE: combination of bits 0..9. (0..1023)
     */
    CPSS_PORTS_SHORT_BMP_STC    defaultForwardingBmp;
    /** @brief When pntLookupMode is 'DST_AND_SRC' it is the bitmap of the HSR/PRP
        ports that describes the ports that should be filtered when PNT Source
        Lookup has a match.
        If a bit in the Bitmap is set, it means that packets forwarded from the
        source port (the index to this entry) should be filtered in the port
        associated with that bit.
        APPLICABLE RANGE: combination of bits 0..9. (0..1023)
     */
    CPSS_PORTS_SHORT_BMP_STC    srcIsMeBmp;
    /** @brief An optional configuration to filter out the source port in addition
        to the target ports in case the same packet would be received again from
        other port.
        This configuration is a bitmap of the HSR/PRP ports.
        If a bit in the Bitmap is set, it means not to send this packet again to
        the port associated with this bit.
        APPLICABLE RANGE: combination of bits 0..9. (0..1023)
     */
    CPSS_PORTS_SHORT_BMP_STC    srcDuplicateDiscardBmp;
    /** @brief When pntLookupMode is 'DST_AND_SRC' it is the bitmap of the HSR/PRP
        ports that describes the target ports when PNT Destination Lookup has a match.
        If a bit in the Bitmap is set, it means that packets are forwarded from
        the source port (the index to this entry) to the port associated with that bit.
        APPLICABLE RANGE: combination of bits 0..9. (0..1023)
     */
    CPSS_PORTS_SHORT_BMP_STC    destToMeForwardingBmp;

} CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC;


/**
* @enum CPSS_DXCH_HSR_PRP_PNT_OPER_ENT
 *
 * @brief HSR/PRP types of operations to apply on entry in the PNT
 *      NOTE: 1. relevant only to 'Set' API (and not to 'Get')
 *            2. for 'Aging' entries , see API : cpssDxChHsrPrpPntAgingApply(...)
 *
*/
typedef enum{

    /** @brief : operation type to apply on entry in the PNT :
        the function fail if  the entry is valid.
        set the entry ready for the HW to learn into it.
        the caller need set the 'StreamId'
    */
    CPSS_DXCH_HSR_PRP_PNT_OPER_SET_READY_E ,
    /** @brief : operation type to apply on entry in the PNT :
        Invalidate the entry.
    */
    CPSS_DXCH_HSR_PRP_PNT_OPER_SET_INVALID_E,
    /** @brief : operation type to apply on entry in the PNT :
        Validate the entry , with all it's fields.
        the function first invalidate the entry.
    */
    CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E

} CPSS_DXCH_HSR_PRP_PNT_OPER_ENT;

/**
* @enum CPSS_DXCH_HSR_PRP_PNT_STATE_ENT
 *
 * @brief HSR/PRP types of states of entry in the PNT
 *      NOTE: relevant only to 'Get' API (and not to 'Set')
 *
*/
typedef enum{

    /** @brief : type of states of entry in the PNT :
        The entry is ready for HW to use the 'StreamId' and the entry.
    */
    CPSS_DXCH_HSR_PRP_PNT_STATE_READY_E,
    /** @brief : type of states of entry in the PNT :
        The entry is valid and HW using it.
    */
    CPSS_DXCH_HSR_PRP_PNT_STATE_VALID_E,
    /** @brief : type of states of entry in the PNT :
        The entry is not valid and HW not using it.
    */
    CPSS_DXCH_HSR_PRP_PNT_STATE_NOT_VALID_E

} CPSS_DXCH_HSR_PRP_PNT_STATE_ENT;

/**
* @struct CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC
 *
 * @brief HSR/PRP PNT entry fields (not include valid bit and ready bit)
 *
 *  NOTE: the valid bit and ready bit are set/get by
 *      CPSS_DXCH_HSR_PRP_PNT_OPER_ENT , CPSS_DXCH_HSR_PRP_PNT_STATE_ENT
*/
typedef struct{

    /** @brief The stream ID.
        This field set on operations : CPSS_DXCH_HSR_PRP_PNT_OPER_SET_READY_E , CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E
        NOTE: value 0 should NOT be used.
        APPLICABLE RANGE: 0..(2K-1)
     */
    GT_U32         streamId;
    /** @brief The macAddr.
        This field set on operation : CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E
     */
    GT_ETHERADDR   macAddr;
    /** @brief The ageBit.
        This field set on operations : CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E
        the bit is set to GT_TRUE each time the entry is accessed by the HW.
        GT_FALSE - the entry is aged.
        GT_TRUE  - the entry is fresh.
     */
    GT_BOOL        ageBit;
    /** @brief is untagged : is mac for SAN.
        GT_FALSE - source MAC address is NOT associated with a Singly Attached Node (SAN).
        GT_TRUE  - source MAC address is     associated with a Singly Attached Node (SAN).
     */
    GT_BOOL        untagged;
    /** @brief The LRE instance
        APPLICABLE RANGE: 0..3
     */
    GT_U32         lreInstance;
    /** @brief the destination bitmap of the HSR/PRP ports .
        APPLICABLE RANGE: combination of bits 0..9. (0..1023)
     */
    CPSS_PORTS_SHORT_BMP_STC destBmp;

} CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC;

/**
* @struct CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC
 *
 * @brief HSR/PRP LRE Instance entry fields (without the counters)
 *
*/
typedef struct{

    /** @brief The number of ports in the LRE instance.
        APPLICABLE RANGE: 0..10
     */
    GT_U32         numOfPorts;
    /** @brief The HPort with the lowest port number in this instance.
        APPLICABLE RANGE: 0..9
        Validity check :(numOfPorts + lowestHPort) <= 10
     */
    GT_U32         lowestHPort;
    /** @brief The base eVIDX number Used only for mapping Bitmap to eVIDX.
        APPLICABLE RANGE: eVidx range
        Validity check : eVidxMappingBase < (numOf_eVidx - (2^ numOfPorts))
     */
    GT_U32         eVidxMappingBase;

} CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC;



/**
* @enum CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT
 *
 * @brief HSR/PRP types of FDB partition with the DDE.
 *
*/
typedef enum{

    /** @brief : type of FDB partition with the DDE :
        There are no DDEs in the FDB.
        All the entries in the FDB are only for MAC/UC-IP-route/IPMC-bridging.
    */
    CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E,
    /** @brief : type of FDB partition with the DDE :
        upper 1/2 of the FDB entries for DDE usage.
        lower 1/2 of the FDB entries for for MAC/UC-IP-route/IPMC-bridging.
    */
    CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_1_2_E,
    /** @brief : type of FDB partition with the DDE :
        upper 3/4 of the FDB entries for DDE usage.
        lower 1/4 of the FDB entries for for MAC/UC-IP-route/IPMC-bridging.
    */
    CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_3_4_E,
    /** @brief : type of FDB partition with the DDE :
        upper 7/8 of the FDB entries for DDE usage.
        lower 1/8 of the FDB entries for for MAC/UC-IP-route/IPMC-bridging.
    */
    CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_7_8_E,
    /** @brief : type of FDB partition with the DDE :
        All the FDB entries for DDEs.
        None for MAC/UC-IP-route/IPMC-bridging.
    */
    CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E

} CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT;


/**
* @enum CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT
 *
 * @brief HSR/PRP modes of DDE timer granularity.
 *
*/
typedef enum{

    /** @brief : mode of DDE timer granularity :
        10 micro-second granularity.
    */
    CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_10_MICRO_SEC_E,
    /** @brief : mode of DDE timer granularity :
        20 micro-second granularity.
    */
    CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_20_MICRO_SEC_E,
    /** @brief : mode of DDE timer granularity :
        40 micro-second granularity.
    */
    CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_40_MICRO_SEC_E,
    /** @brief : mode of DDE timer granularity :
        80 micro-second granularity.
    */
    CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_80_MICRO_SEC_E

} CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT;

/**
* @enum CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT
 *
 * @brief HSR/PRP types of DDE time fields.
 *
*/
typedef enum{

    /** @brief : type of DDE time fields :
        Duplicate Discard Entry that stays in the FDB table over this time is
        considered as expired entry when the Duplicate Discard Lookup has a
        match with this entry.
    */
    CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_SAME_KEY_EXPIRATION_E,
    /** @brief : type of DDE time fields :
        Duplicate Discard Entry that stays in the FDB table over this time is
        considered as expired entry when learning a new entry.
    */
    CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_EXPIRATION_E,
    /** @brief : type of DDE time fields :
         wraparound timer.
         NOTE: this is 'Read Only' , so not supported by the 'Set' API
    */
    CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_TIMER_E

} CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT;


/**
* @enum CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT
 *
 * @brief HSR/PRP types of PRP trailer actions.
 *
*/
typedef enum{

    /** @brief : type of PRP trailer action :
         do nothing.
    */
    CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_NONE_E,
    /** @brief : type of PRP trailer action :
         add PRP trailer.
    */
    CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ADD_PRP_E,
    /** @brief : type of PRP trailer action :
         remove PRP trailer.
    */
    CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_REMOVE_PRP_E

} CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT;

/**
* @enum CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT
 *
 * @brief HSR/PRP types of PRP padding modes.
 *
*/
typedef enum{

    /** @brief : type of PRP padding mode :
         Pad untagged VLAN packets to 60 bytes.
         Pad VLAN   tagged packets to 64 bytes.
    */
    CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_UNTAGGED_TO_60_OR_VLAN_TAGGED_TO_64_E,
    /** @brief : type of PRP padding mode :
         Pad packets to 60 bytes.
    */
    CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_TO_60_E,
    /** @brief : type of PRP padding mode :
         Pad packets to 64 bytes.
    */
    CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_TO_64_E

} CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT;




/**
* @struct CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC
 *
 * @brief DDE table entry hash key info.
 *      used by the device for Duplicate Discard Lookup (DDL)
 *      The index can be calculated by cpssDxChHsrPrpDdeHashCalc(...)
 *
*/
typedef struct{

    /** @brief The mac address of the entry.
     */
    GT_ETHERADDR     macAddr;
    /** @brief The sequence number.
        APPLICABLE RANGE: 0..(64K-1)

        NOTE: the seqNum in the HW is carried in next descriptor fields <up1,cfi1,vid1>
    */
    GT_U32           seqNum;
    /** @brief The LRE instance that the HPort belongs to
        APPLICABLE RANGE: 0..3
     */
    GT_U32           lreInstance;

} CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC;

/**
* @struct CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC
 *
 * @brief DDE table entry info.
 *      used by the device for Duplicate Discard Lookup (DDL)
 *
*/
typedef struct{

    /** @brief The info about the hash key.
     */
    CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC   ddeKey;
    /** @brief the destination bitmap of the HSR/PRP ports .
        APPLICABLE RANGE: combination of bits 0..9. (0..1023)
     */
    CPSS_PORTS_SHORT_BMP_STC  destBmp;
    /** @brief The source HPort index.
        APPLICABLE RANGE: 0..9
    */
    GT_U32           srcHPort;
    /** @brief The time (in micro-seconds) when the entry was created.
        APPLICABLE RANGE: 0..max
        NOTE: max depend on CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT
        NOTE: value 0xFFFFFFFF on the 'Write' API means : use current HW timer.
              meaning that the API will read the current timer value from the HW
              and will set it as the time of the entry.
    */
    GT_U32           timeInMicroSec;
    /** @brief The counters of the target HPorts that traffic send to.
        each HPort (0..9) hold counter in this array.
        APPLICABLE RANGE: 0..3 (per counter in the array)
        NOTE: each counter in not 'wraparound' (will stuck at '3')
    */
    GT_U8            destHportCounters[CPSS_DXCH_HSR_PRP_H_PORTS_NUM_CNS];
    /** @brief The ageBit.
        GT_FALSE - the entry is aged.
        GT_TRUE  - the entry is fresh.
     */
    GT_BOOL         ageBit;

} CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC;


/**
* @internal cpssDxChHsrPrpExceptionCommandSet function
* @endinternal
*
* @brief  Set the packet command to specific exception type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
* @param[in] command        - the packet command.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type/command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpExceptionCommandSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
    IN CPSS_PACKET_CMD_ENT                      command
);

/**
* @internal cpssDxChHsrPrpExceptionCommandGet function
* @endinternal
*
* @brief  Get the packet command for specific exception type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
*
* @param[out] commandPtr    - (pointer to) the packet command.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpExceptionCommandGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
    OUT CPSS_PACKET_CMD_ENT                     *commandPtr
);


/**
* @internal cpssDxChHsrPrpExceptionCpuCodeSet function
* @endinternal
*
* @brief  Set the cpu code to specific exception type.
*         Note : Relevant when the packet command for the exception is not 'FORWARD'
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
* @param[in] cpuCode        - the cpu code.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type/cpuCode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpExceptionCpuCodeSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
    IN  CPSS_NET_RX_CPU_CODE_ENT                 cpuCode
);

/**
* @internal cpssDxChHsrPrpExceptionCpuCodeGet function
* @endinternal
*
* @brief  Get the cpu code for specific exception type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
*
* @param[out] cpuCodePtr    - (pointer to) The cpu code.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpExceptionCpuCodeGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
    OUT CPSS_NET_RX_CPU_CODE_ENT                 *cpuCodePtr
);


/**
* @internal cpssDxChHsrPrpPortPrpEnableSet function
* @endinternal
*
* @brief  Enable/Disable the PRP on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: 'Default eport' range.
* @param[in] enable         - Indication to Enable/Disable the PRP.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpEnableSet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  GT_BOOL                  enable
);

/**
* @internal cpssDxChHsrPrpPortPrpEnableGet function
* @endinternal
*
* @brief  Get the Enable/Disable of PRP on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: 'Default eport' range.
*
* @param[out] enablePtr     - (pointer to) Indication to Enable/Disable the PRP.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    OUT GT_BOOL                  *enablePtr
);


/**
* @internal cpssDxChHsrPrpPortPrpLanIdSet function
* @endinternal
*
* @brief  Set the PRP LAN ID on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                             For Ingress direction (and 'Both directions') :
*                               APPLICABLE RANGE: 'Default eport' range.
*                             For Egress direction (and 'Both directions') :
*                               APPLICABLE RANGE: eport range.
* @param[in] direction      - select direction (ingress/egress/Both)
* @param[in] lanId          - The LAN ID.
*                             APPLICABLE RANGE: 0..15.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum/direction/lanId
* @retval GT_OUT_OF_RANGE          - out of range lanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpLanIdSet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  CPSS_DIRECTION_ENT       direction,
    IN  GT_U32                   lanId
);

/**
* @internal cpssDxChHsrPrpPortPrpLanIdGet function
* @endinternal
*
* @brief  Get the PRP LAN ID of specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                             For Ingress direction :
*                               APPLICABLE RANGE: 'Default eport' range.
*                             For Egress direction :
*                               APPLICABLE RANGE: eport range.
* @param[in] direction      - select direction (ingress/egress/Both)
*
* @param[out] lanIdPtr      - (pointer to) The LAN ID.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum/direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpLanIdGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  CPSS_DIRECTION_ENT       direction,
    OUT GT_U32                   *lanIdPtr
);



/**
* @internal cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet function
* @endinternal
*
* @brief  Enable/Disable the PRP treat wrong LanId as RCT exists on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: 'Default eport' range.
* @param[in] enable         - Indication to treat wrong LanId as RCT exists.
*                             GT_TRUE  - Do    treat wrong LanId as RCT exists.
*                             GT_FALSE - Don't treat wrong LanId as RCT exists.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  GT_BOOL                  enable
);

/**
* @internal cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet function
* @endinternal
*
* @brief  Get Enable/Disable for the PRP treat wrong LanId as RCT exists on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: 'Default eport' range.
*
* @param[out] enablePtr     - (pointer to) Indication to treat wrong LanId as RCT exists.
*                             GT_TRUE  - Do    treat wrong LanId as RCT exists.
*                             GT_FALSE - Don't treat wrong LanId as RCT exists.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    OUT GT_BOOL                  *enablePtr
);


/**
* @internal cpssDxChHsrPrpPortsBmpSet function
* @endinternal
*
* @brief  Set ports bitmap for specific bitmap type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] bmpType        - The bitmap type.
* @param[in] bmp            - The ports bitmap.
*                             APPLICABLE RANGE: combination of bits 0..9. (0..1023)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/bmpType
* @retval GT_OUT_OF_RANGE          - wrong bmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortsBmpSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT  bmpType,
    IN  CPSS_PORTS_SHORT_BMP_STC            bmp
);

/**
* @internal cpssDxChHsrPrpPortsBmpGet function
* @endinternal
*
* @brief  Get ports bitmap for specific bitmap type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] bmpType        - The bitmap type.
*
* @param[out] bmpPtr        - (pointer to)The ports bitmap.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/bmpType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortsBmpGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT  bmpType,
    OUT CPSS_PORTS_SHORT_BMP_STC            *bmpPtr
);


/**
* @internal cpssDxChHsrPrpHPortSrcEPortBaseSet function
* @endinternal
*
* @brief  Set source eport base for HPorts.
*         Defines the range of source ePorts that are mapped to HSR/PRP port.
*         The HSR/PRP port number is the 4 lsb of the source ePort number
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] baseEport      - The eport base.
*                             APPLICABLE RANGE: 0..65520 . limitation : in steps of 16 (0,16,32,..65520)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
*                                    on baseEport in range but violate the limitation.
* @retval GT_OUT_OF_RANGE          - wrong baseEport
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpHPortSrcEPortBaseSet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         baseEport
);

/**
* @internal cpssDxChHsrPrpHPortSrcEPortBaseGet function
* @endinternal
*
* @brief  Get source eport base for HPorts.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] baseEportPtr  - (pointer to)The eport base.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpHPortSrcEPortBaseGet
(
    IN  GT_U8               devNum,
    OUT GT_PORT_NUM         *baseEportPtr
);


/**
* @internal cpssDxChHsrPrpCounterGet function
* @endinternal
*
* @brief  Get specific global Counter.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] counterType    - The counter type.
*
* @param[out] counterPtr    - (pointer to)The counter.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/counterType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpCounterGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT  counterType,
    OUT GT_U64                              *counterPtr
);

/**
* @internal cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet function
* @endinternal
*
* @brief  Set threshold for number of ready entries in PNT.
*         The event CPSS_PP_HSR_PRP_PNT_NUM_OF_READY_ENTRIES_E is triggered when
*         <PNT Number of Ready Entries> is equal or below this value.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] threshold      - The threshold.
*                             APPLICABLE RANGE: 0..1023
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - out of range threshold
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet
(
    IN  GT_U8               devNum,
    IN  GT_U32              threshold
);

/**
* @internal cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet function
* @endinternal
*
* @brief  Get threshold for number of ready entries in PNT.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] thresholdPtr  - (pointer to) The threshold.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet
(
    IN  GT_U8               devNum,
    OUT GT_U32              *thresholdPtr
);

/**
* @internal cpssDxChHsrPrpHPortEntrySet function
* @endinternal
*
* @brief  Set the HPort entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] hPort          - The HPort index.
*                             APPLICABLE RANGE: 0..9
* @param[in] infoPtr        - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or hPort or other wrong parameter
* @retval GT_OUT_OF_RANGE          - out of range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpHPortEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          hPort,
    IN  CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC    *infoPtr
);

/**
* @internal cpssDxChHsrPrpHPortEntryGet function
* @endinternal
*
* @brief  Get the HPort entry info and counters.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] hPort          - The HPort index.
*                             APPLICABLE RANGE: 0..9
*
* @param[out] infoPtr        - (pointer to) The entry info.
*                               NOTE: if NULL ignored. allow to get only counter(s).
* @param[out] firstCounterPtr - (pointer to) The 'first' counter.
*                               NOTE: if NULL ignored. allow to skip the counter.
*                               The counter is Read only and clear after read
* @param[out] secondCounterPtr - (pointer to) The 'second' counter.
*                               NOTE: if NULL ignored. allow to skip the counter.
*                               The counter is Read only and clear after read
* @param[out] thirdCounterPtr - (pointer to) The 'third' counter.
*                               NOTE: if NULL ignored. allow to skip the counter.
*                               The counter is Read only and clear after read
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or hPort or other wrong parameter
* @retval GT_BAD_PTR               - if ALL the pointers are NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpHPortEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          hPort,
    OUT CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC    *infoPtr,
    OUT GT_U64                          *firstCounterPtr,
    OUT GT_U64                          *secondCounterPtr,
    OUT GT_U64                          *thirdCounterPtr
);

/**
* @internal cpssDxChHsrPrpPntEntrySet function
* @endinternal
*
* @brief  Set the PNT entry info.
*       NOTE : for 'Aging' entries , see API : cpssDxChHsrPrpPntAgingApply(...)
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index in the table.
*                             APPLICABLE RANGE: 0..127
* @param[in] infoPtr        - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index or other wrong parameter
* @retval GT_OUT_OF_RANGE          - out of range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_ALLOWED           - the operation not allowed.
*                                    relevant when operation == 'SET_READY' but entry is not invalid.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPntEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_HSR_PRP_PNT_OPER_ENT       operation,
    IN  CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC      *infoPtr
);

/**
* @internal cpssDxChHsrPrpPntEntryGet function
* @endinternal
*
* @brief  Get the PNT entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index in the table.
*                             APPLICABLE RANGE: 0..127
*
* @param[out] statePtr      - (pointer to) The state of the entry.
* @param[out] infoPtr       - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPntEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_DXCH_HSR_PRP_PNT_STATE_ENT      *statePtr,
    OUT CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC      *infoPtr
);

/**
* @internal cpssDxChHsrPrpPntAgingApply function
* @endinternal
*
* @brief  Age a range of entries in the PNT.
*       for each entry the operation is :
*        Ask to age the entry :
*        If the entry is not valid the operation is ignored
*        if the entry is fresh     (aged bit == 1) the entry will be aged (aged bit = 0).
*        if the entry already aged (aged bit == 0) the entry will be invalid (aged out). and counted.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The start index in the table.
*                             APPLICABLE RANGE: 0..127
* @param[in] numOfEntries   - The number of entries in the table to apply
*                             aging on , starting from 'index' (with wraparound support)
*                             NOTE: value 0xFFFFFFFF means 'Full Table' , starting from 'index'
*                               (not implicitly from index 0)
*                             APPLICABLE RANGE: 1..128 , 0xFFFFFFFF
*
* @param[out] numOfEntriesDeletedPtr   - (pointer to) The number of entries that
*                               deleted (invalidated) due to the aging.
*                               NOTE: if NULL ignored.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index or numOfEntries
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPntAgingApply
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  GT_U32                          numOfEntries,
    OUT GT_U32                          *numOfEntriesDeletedPtr
);

/**
* @internal cpssDxChHsrPrpLreInstanceEntrySet function
* @endinternal
*
* @brief  Set the LRE instance entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index (instance) in the table.
*                             APPLICABLE RANGE: 0..3
* @param[in] infoPtr        - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_OUT_OF_RANGE          - out of range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpLreInstanceEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC *infoPtr
);


/**
* @internal cpssDxChHsrPrpLreInstanceEntryGet function
* @endinternal
*
* @brief  Get the LRE instance entry info and counters.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index (instance) in the table.
*                             APPLICABLE RANGE: 0..3
*
* @param[out] infoPtr       - (pointer to) The entry info.
*                               NOTE: if NULL ignored. allow to get only counter(s).
* @param[out] lreADiscardCounterPtr - (pointer to)The LRE 'A' discard counter.
*                               NOTE: if NULL ignored. allow to skip the counter.
*                               The counter is Read only and clear after read
* @param[out] lreBDiscardCounterPtr - (pointer to)The LRE 'B' discard counter.
*                               NOTE: if NULL ignored. allow to skip the counter.
*                               The counter is Read only and clear after read
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - if ALL the pointers are NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpLreInstanceEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    OUT CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC *infoPtr,
    OUT GT_U64                              *lreADiscardCounterPtr,
    OUT GT_U64                              *lreBDiscardCounterPtr
);


/**
* @internal cpssDxChHsrPrpFdbPartitionModeSet function
* @endinternal
*
* @brief  Set the FDB partition with the DDE.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] mode           - The partition mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpFdbPartitionModeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT mode
);


/**
* @internal cpssDxChHsrPrpFdbPartitionModeGet function
* @endinternal
*
* @brief  Get the FDB partition with the DDE.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] modePtr       - (pointer to) The partition mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpFdbPartitionModeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT *modePtr
);

/**
* @internal cpssDxChHsrPrpDdeTimerGranularityModeSet function
* @endinternal
*
* @brief  Set the DDE timer granularity mode.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] mode           - The granularity mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeTimerGranularityModeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT mode
);

/**
* @internal cpssDxChHsrPrpDdeTimerGranularityModeGet function
* @endinternal
*
* @brief  Get the DDE timer granularity mode.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] modePtr       - (pointer to)The granularity mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeTimerGranularityModeGet
(
    IN  GT_U8                                       devNum,
    OUT CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT *modePtr
);

/**
* @internal cpssDxChHsrPrpDdeTimeFieldSet function
* @endinternal
*
* @brief  Set time for a specific DDE time field.
*
*       NOTEs:
*       1. The time is in micro-seconds.
*       2. The time range and granularity depends on 'DdeTimerGranularity' set by :
*          cpssDxChHsrPrpDdeTimerGranularityModeSet(...)
*       3. the function round the time to the nearest value supported by the granularity.
*          the actual value can be retrieved by the 'Get' function.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] field          - The time field.
* @param[in] timeInMicroSec - The time in micro-seconds.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or field
* @retval GT_OUT_OF_RANGE          - out of range timeInMicroSec
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeTimeFieldSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT field,
    IN  GT_U32                          timeInMicroSec
);

/**
* @internal cpssDxChHsrPrpDdeTimeFieldGet function
* @endinternal
*
* @brief  Get time for a specific DDE time field.
*
*       NOTEs:
*       1. The time is in micro-seconds.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] field          - The time field.
*
* @param[out] timeInMicroSecPtr - (pointer to)The time in micro-seconds.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or field
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeTimeFieldGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT field,
    OUT GT_U32                          *timeInMicroSecPtr
);

/**
* @internal cpssDxChHsrPrpPortPrpTrailerActionSet function
* @endinternal
*
* @brief  Set the PRP trailer action on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
* @param[in] action         - The trailer action.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum/action
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpTrailerActionSet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT action
);

/**
* @internal cpssDxChHsrPrpPortPrpTrailerActionGet function
* @endinternal
*
* @brief  Get the PRP trailer action on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
*
* @param[out] actionPtr     - (pointer to)The trailer action.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpTrailerActionGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT *actionPtr
);

/**
* @internal cpssDxChHsrPrpPortPrpPaddingModeSet function
* @endinternal
*
* @brief  Set the PRP padding mode on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
* @param[in] mode           - The padding mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum/mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpPaddingModeSet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT   mode
);

/**
* @internal cpssDxChHsrPrpPortPrpPaddingModeGet function
* @endinternal
*
* @brief  Get the PRP padding mode on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
*
* @param[out] modePtr       - (pointer to)The padding mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpPaddingModeGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT   *modePtr
);

/**
* @internal cpssDxChHsrPrpDdeEntryInvalidate function
* @endinternal
*
* @brief   Invalidate an entry in DDE table in specified index.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] index                 - The index of the entry.
*                                    APPLICABLE RANGE: 0..max
*                                    NOTE: 1. 'max' index depends on the mode selected
*                                             by CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT
*                                          2. The index can be calculated by cpssDxChHsrPrpDdeHashCalc(...)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,index
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeEntryInvalidate
(
    IN  GT_U8         devNum,
    IN  GT_U32        index
);


/**
* @internal cpssDxChHsrPrpDdeEntryWrite function
* @endinternal
*
* @brief   Write an entry in DDE table in specified index.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] index                 - The index of the entry.
*                                    APPLICABLE RANGE: 0..max
*                                    NOTE: 1. 'max' index depends on the mode selected
*                                             by CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT
*                                          2. The index can be calculated by cpssDxChHsrPrpDdeHashCalc(...)
* @param[in] infoPtr               - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,index
* @retval GT_OUT_OF_RANGE          - on out of range parameters in entryPtr :
*                                    ddeKey.seqNum , ddeKey.lreInstance , destBmp  , srcHPort  ,
*                                    timeInMicroSec , destHportCounters[]
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeEntryWrite
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       index,
    IN  CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC   *entryPtr
);

/**
* @internal cpssDxChHsrPrpDdeEntryRead function
* @endinternal
*
* @brief   Read an entry from DDE table in specified index.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] index                 - The index of the entry.
*                                    APPLICABLE RANGE: 0..max
*                                    NOTE: 1. 'max' index depends on the mode selected
*                                             by CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT
*                                          2. The index can be calculated by cpssDxChHsrPrpDdeHashCalc(...)
*
* @param[out] infoPtr              - (pointer to) The entry info.
* @param[out] isValidPtr           - (pointer to) Is the entry valid.
*                                    if the entry is not valid (valid bit unset or not 'DDE' type) ,
*                                    the values are not parsed into infoPtr.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeEntryRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       index,
    OUT CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC  *entryPtr,
    OUT GT_BOOL                     *isValidPtr
);

/**
* @internal cpssDxChHsrPrpDdeAgingApply function
* @endinternal
*
* @brief  Age a range of entries in the DDE table.
*       for each entry the operation is :
*        Ask to age the entry :
*        If the entry is not valid the operation is ignored
*        if the entry is fresh     (aged bit == 1) the entry will be aged (aged bit = 0).
*        if the entry already aged (aged bit == 0) the entry will be invalid (aged out). and counted.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index of the entry.
*                             APPLICABLE RANGE: 0..max
*                             NOTE: 1. 'max' index depends on the mode selected
*                                      by CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT
*                                   2. The index can be calculated by cpssDxChHsrPrpDdeHashCalc(...)
* @param[in] numOfEntries   - The number of entries in the table to apply
*                             aging on , starting from 'index' (with wraparound support)
*                             NOTE: value 0xFFFFFFFF means 'Full Table' , starting from 'index'
*                               (not implicitly from index 0)
*                             APPLICABLE RANGE: 1..max , 0xFFFFFFFF
*
* @param[out] numOfEntriesDeletedPtr   - (pointer to) The number of entries that
*                               deleted (invalidated) due to the aging.
*                               NOTE: if NULL ignored.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index or numOfEntries
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeAgingApply
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  GT_U32                          numOfEntries,
    OUT GT_U32                          *numOfEntriesDeletedPtr
);

/**
* @internal cpssDxChHsrPrpDdeHashCalc function
* @endinternal
*
* @brief   This function calculates the hash index for the DDE key (into the DDE table).
*         NOTE: the function do not access the HW , and do only SW calculations.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] entryKeyPtr           - (pointer to) The key of the entry.
* @param[in] startBank             - the first bank for hash result.
*                                   (APPLICABLE RANGES: 0..15)
* @param[in] numOfBanks            - number of banks for the hash result.
*                                   this value indicates the number of elements that will be
*                                   retrieved by hashArr[]
*                                   restriction of (numOfBanks + startBank) <= 16 .
*                                   (APPLICABLE RANGES: 1..16)
*
* @param[out] hashArr[]            - (array of) the hash results. index in this array is 'bank Id'
*                                   (index 0 will hold value relate to bank startBank).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or parameters in entryKeyPtr :
*                                    seqNum , lreInstance.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeHashCalc
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC   *entryKeyPtr,
    IN  GT_U32                          startBank,
    IN  GT_U32                          numOfBanks,
    OUT GT_U32                          hashArr[] /*maxArraySize=16*/
);

/**
* @internal cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet function
* @endinternal
*
* @brief  Set the PRP LSDU offset on specific target physical port.
*         Offset to the LSDU as defined by the PRP standard for untagged VLAN packets
*         that are also without Timestamp tag.
*         The device automatically takes into account the VLAN tag size and
*         Timestamp tag in the LSDU size calculation.
*         The device doesn't automatically take into account the eDSA tag size
*         and the pushed tag size. In case the packets are sent with eDSA tag or
*         pushed tag this parameter that is per target port, should be set to
*         14 + the size of the tag.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
* @param[in] lsduOffset     - The PRP lsdu Offset.
*                               APPLICABLE RANGE: 0..63
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - out of range lsduOffset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   lsduOffset
);

/**
* @internal cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet function
* @endinternal
*
* @brief  Get the PRP LSDU offset on specific target physical port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
*
* @param[out] lsduOffsetPtr - (pointer to) The PRP lsdu Offset.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_U32                   *lsduOffsetPtr
);


/**
* @internal cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet function
* @endinternal
*
* @brief  Set the HSR LSDU offset on specific source physical port.
*         offset to the LSDU as defined by HSR standard for untagged VLAN packets
*         that are also without Timestamp tag.
*         The device automatically takes into account the VLAN tag size and Timestamp tag
*         in the LSDU size calculation.
*         The device doesn't automatically take into account the eDSA tag size
*         and the pushed tag size.
*         in case the packet are received with eDSA tag or pushed tag over the
*         source port, this parameter that is per source port should be set to
*         14+the size of the tag.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                             APPLICABLE RANGE: physical port range.
* @param[in] lsduOffset     - The HSR LSDU Offset.
*                             APPLICABLE RANGE: 0..63
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - out of range lsduOffset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   lsduOffset
);

/**
* @internal cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet function
* @endinternal
*
* @brief  Get the HSR LSDU offset on specific source physical port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                             APPLICABLE RANGE: physical port range.
*
* @param[out] lsduOffsetPtr - (pointer to) The HSR LSDU Offset.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_U32                   *lsduOffsetPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChHsrPrph */

