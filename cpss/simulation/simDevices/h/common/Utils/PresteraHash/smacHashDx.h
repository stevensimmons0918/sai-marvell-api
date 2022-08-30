/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* macHashDx.h
*
* DESCRIPTION:
*       Hash calculate for MAC address table implementation for Salsa.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 8 $
*
*******************************************************************************/
#ifndef __smacHashDxh
#define __smacHashDxh

#include <os/simTypes.h>
#include <asicSimulation/SKernel/smain/smain.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
    SNET_CHEETAH_FDB_ENTRY_MAC_E = 0x0,
    SNET_CHEETAH_FDB_ENTRY_IPV4_IPMC_BRIDGING_E = 0x1,/*IPv4 Multicast address entry (IGMP snooping);*/
    SNET_CHEETAH_FDB_ENTRY_IPV6_IPMC_BRIDGING_E = 0x2,/*IPv6 Multicast address entry (MLD snooping);*/

    /* SIP5 new types */
    SNET_CHEETAH_FDB_ENTRY_IPV4_UC_ROUTING_E    = 0x3,/*Unicast routing IPv4 address entry.*/
    SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_KEY_E= 0x4,/*Unicast routing IPv6 address lookup-key entry.*/
    SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_DATA_E=0x5,/*Unicast routing IPv6 address data entry.*/
    SNET_CHEETAH_FDB_ENTRY_FCOE_UC_ROUTING_E    = 0x6,/*Unicast routing FCOE address entry.*/

    /* Sip6.30 new  */
    SNET_CHEETAH_FDB_ENTRY_HSU_PRP_DDE_E        = 0x7


} SNET_CHEETAH_FDB_ENTRY_ENT;

typedef enum {
    SNET_CHEETAH_FDB_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E,/* use 16 upper bit 64..80 as 0 */
    SNET_CHEETAH_FDB_CRC_HASH_UPPER_BITS_MODE_FID_E,     /* use 16 upper bit 64..80 as FID */
    SNET_CHEETAH_FDB_CRC_HASH_UPPER_BITS_MODE_MAC_E      /* use 16 upper bit 64..80 as MAC LSBits */
}SNET_CHEETAH_FDB_CRC_HASH_UPPER_BITS_MODE_ENT;

typedef enum{
    CRC_INIT_VALUE_MODE_FORCE_ALL_ONE___64K_ALL_ZER0__E, /* not sip5 */
    CRC_INIT_VALUE_MODE_FORCE_ALL_ONE_E,                 /* sip5_15  */
    CRC_INIT_VALUE_MODE_FORCE_ALL_ZERO_E                 /* sip5,sip5_10 , sip5_20...*/
}CRC_INIT_VALUE_MODES_ENT;

typedef struct{
    SNET_CHEETAH_FDB_ENTRY_ENT     entryType;

    /*common to most entry types, but not relevant for UC routing */
    GT_U16  origFid;/* the fid may be modified due to fid16BitHashEn , but this value should hold original fid  */
    GT_U16  fid;/*vlanId - common to most entry types */
    GT_BIT  fdbLookupKeyMode;   /* indicates FDB lookup mode: single tag FDB lookup or double tag FDB lookup (sip5_10)*/
    GT_U32  vid1; /* vid1 - used for double tag FDB Lookup (sip5_10)*/
    GT_BIT  fid16BitHashEn;/* indication if FID used 12 or 16 bits*/
    CRC_INIT_VALUE_MODES_ENT  crcInitValueMode;

    union{
        struct{
            GT_U8       macAddr[6];/*source / destination mac address */
            SNET_CHEETAH_FDB_CRC_HASH_UPPER_BITS_MODE_ENT crcHashUpperBitsMode;/* 16 MSbits mode for of DATA into the hash function */
        }macInfo;  /*SNET_CHEETAH_FDB_ENTRY_MAC_E , SNET_CHEETAH_FDB_ENTRY_HSU_PRP_DDE_E */

        struct{
            GT_U32      sip;/*source IP*/
            GT_U32      dip;/*destination IP*/
        }ipmcBridge;/*SNET_CHEETAH_FDB_ENTRY_IPV4_IPMC_BRIDGING_E , SNET_CHEETAH_FDB_ENTRY_IPV6_IPMC_BRIDGING_E */

        struct{
            GT_U32  vrfId;   /*vrf Id*/
            GT_U32  dip[4];  /*destination IP, for ipv4 and fcoe dip[0] only used*/
        }ucRouting; /* SNET_CHEETAH_FDB_ENTRY_IPV4_UC_ROUTING_E
                       SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_KEY_E
                       SNET_CHEETAH_FDB_ENTRY_FCOE_UC_ROUTING_E */
    }info;
}SNET_CHEETAH_FDB_ENTRY_HASH_INFO_STC;

/* macro to get bit from the array of words */
#define _D(bit) (D[(bit)>>5] >> ((bit)&0x1f))

GT_STATUS salsaMacHashCalc
(
    IN  GT_ETHERADDR    *addr,
    IN  GT_U16          vid,
    IN  GT_U8           vlanMode,
    OUT GT_U32          *hash
);

GT_STATUS salsa2MacHashCalc
(
    IN  GT_ETHERADDR    *addr,
    IN  GT_U16          vid,
    IN  GT_U8           vlanMode,
    OUT GT_U32          *hash
);

/**
* @internal cheetahMacHashCalc function
* @endinternal
*
* @brief   This function prepares MAC+VID data for calculating 12-bit hash
*
* @param[in] devObjPtr                - (pointer to) device object
* @param[in] hashType                 - 1 - CRC , or 0- XOR
* @param[in] vlanLookupMode           - 0 - without VLAN,
*                                      1 - vlan + mac - relevant only for MAC based HASH
* @param[in] fdbEntryType             - 0 - MAC, 1 - IPv4 IPM, 2 - IPv6 IPM
* @param[in] macAddrPtr               - MAC address
* @param[in] fid                      - Forwarding id
* @param[in] sourceIpAddr             - source IP
* @param[in] destIpAddr               - destination IP
* @param[in] numBitsToUse             - number of bits used to address FDB mac entry in FDB table,
*                                      depends on FDB size (11 - 8K entries, 12 - 16K entries,
*                                      13 - 32K entries)
*                                       hash value
*
* @note for CRC used polinom X^12+X^11+X^3+X^2+X+1 i.e 0xF01.
*
*/
GT_U32 cheetahMacHashCalc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  hashType,
    IN GT_U32  vlanLookupMode,
    IN GT_U32  fdbEntryType,
    IN GT_U8   *macAddrPtr,
    IN GT_U32  fid,
    IN GT_U32  sourceIpAddr,
    IN GT_U32  destIpAddr,
    IN GT_U32  numBitsToUse
);

/**
* @internal cheetahMacHashCalcByStc function
* @endinternal
*
* @brief   This function prepares MAC+VID data for calculating (info by structure).
*
* @param[in] hashType                 - 1 - CRC , or 0- XOR
* @param[in] vlanLookupMode           - 0 - without VLAN,
*                                      1 - vlan + mac - relevant only for MAC based HASH
* @param[in] entryInfoPtr             - (pointer to) entry hash info
* @param[in] numBitsToUse             - number of bits used to address FDB mac entry in FDB table,
*                                      depends on FDB size (11 - 8K entries, 12 - 16K entries,
*                                      13 - 32K entries)
*                                       hash value
*
* @note for CRC used polinom X^12+X^11+X^3+X^2+X+1 i.e 0xF01.
*
*/
GT_U32 cheetahMacHashCalcByStc
(
    IN GT_U32  hashType,
    IN GT_U32  vlanLookupMode,
    IN SNET_CHEETAH_FDB_ENTRY_HASH_INFO_STC *entryInfoPtr,
    IN GT_U32  numBitsToUse
);


/**
* @internal sip5MacHashCalcMultiHash function
* @endinternal
*
* @brief   This function calculate <numBitsToUse> hash , for multi hash results
*         according to numOfHashToCalc.
* @param[in] vlanLookupMode           - 0 - without VLAN,
*                                      1 - vlan + mac - relevant only for MAC based HASH
*                                      fdbEntryType   - 0 - MAC, 1 - IPv4 IPM, 2 - IPv6 IPM
*                                      fid16BitHashEn - indication that 16 bits of FID are used
* @param[in] entryInfoPtr             - (pointer to) entry hash info
* @param[in] numBitsToUse             - number of bits used to address FDB mac entry in FDB table,
*                                      depends on FDB size (11 - 8K entries, 12 - 16K entries,
*                                      13 - 32K entries)
* @param[in] numOfHashToCalc          - the number of hash functions to generate
*                                      (also it is the number of elements in calculatedHashArr[])
*
* @param[out] calculatedHashArr[]      - array of calculated hash by the different functions
*                                       None
*/
void sip5MacHashCalcMultiHash
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32  hashType,
    IN GT_U32  vlanLookupMode,
    IN SNET_CHEETAH_FDB_ENTRY_HASH_INFO_STC *entryInfoPtr,
    IN GT_U32  numBitsToUse,
    IN GT_U32  numOfHashToCalc,
    OUT GT_U32 calculatedHashArr[]
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smacHashDxh */



