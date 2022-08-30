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
* @file smemFalcon.h
*
* @brief Falcon memory mapping implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __smemFalconh
#define __smemFalconh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemAldrin2.h>

#define ADD_INSTANCE_OF_UNIT_IN_TILE_TO_STR(unitNameStr , instance,tile)    \
    unitNameStr "_INSTANCE_" #instance "_TILE_" #tile

#define ADD_INSTANCE_OF_TILE_TO_STR(unitNameStr , tile)    \
    unitNameStr "_TILE_" #tile

#define FALCON_PORTS_PER_DP 8

#define XSB_ADRESS_SPACE (1<<14)

#define FALCON_XSB_UNIT_MEMORY_MAC(offset)   \
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (offset+0x00000000, offset+0x000000CC)}\
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (offset+0x000000D0, 4)}\
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (offset+0x000000E0, 4)}\
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_NUM_BYTES_MAC (offset+0x000000F0, 4)}\
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (offset+0x00000100, offset+0x00000138)}\
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (offset+0x000001FC, offset+0x000001FC)}\

#define FALCON_UVALUE_XSB_MEMORY_MAC(m_offset,u_value) FALCON_XSB_UNIT_MEMORY_MAC(m_offset+(u_value<<9))


#define FALCON_MVALUE_XSB_MEMORY_MAC(_cluster_base_offset,m_value) \
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x0)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x1)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x2)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x3)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x4)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x5)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x6)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x7)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x8)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x9)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0xa)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0xb)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0xc)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0xd)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0xe)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0xf)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x10)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x11)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x12)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x13)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x14)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x15)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x16)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x17)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x18)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x19)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x1a)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x1b)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x1c)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x1d)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x1e)\
    FALCON_UVALUE_XSB_MEMORY_MAC(_cluster_base_offset+(m_value<<15),0x1f)

#define FALCON_XSB_CLUSTER_MEMORY_MAC(_base_offset) \
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x0)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x1)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x2)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x3)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x4)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x5)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x6)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x7)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x8)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x9)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0xa)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0xb)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0xc)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0xd)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0xe)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0xf)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x10)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x11)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x12)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x13)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x14)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x15)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x16)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x17)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x18)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x19)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x1a)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x1b)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x1c)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x1d)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x1e)\
            FALCON_MVALUE_XSB_MEMORY_MAC(_base_offset,0x1f)

/* PEX ATU (Address Translation Unit) registers */
#define ATU_REGISTERS_OFFSET_IN_BAR0  0x1200

/**
* @internal smemFalconInit function
* @endinternal
*
* @brief   Init memory module for a Falcon device.
*
* @param[in] deviceObj                - pointer to device object.
*/
void smemFalconInit
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemFalconInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemFalconInit2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemFalconInterruptTreeInit function
* @endinternal
*
* @brief   Init the interrupts tree for the Falcon device
*
* @param[in] devObjPtr                - pointer to device object.
*/
GT_VOID smemFalconInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal snetFalconLinkStateNotify function
* @endinternal
*
* @brief   Notify devices database that link state changed
*/
GT_VOID snetFalconLinkStateNotify
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32 port,
    IN GT_U32 linkState
);

/**
* @internal snetFalconPortStatisticCounterUpdate function
* @endinternal
*
* @brief  Updates the offset from the start of the statistic memory for
*         specific counter type and port
*
* @param[in] devObjPtr                - (pointer to) device object
* @param[in] macPort                  - MAC port number
* @param[in] counterType              - statistic counter type
* @param[in] incValue                 - counter increment value
*/
GT_VOID snetFalconPortStatisticCounterUpdate
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN GT_U32                           macPort,
    IN SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_ENT counterType,
    IN GT_U32                           incValue
);

/* the fields of the (FDB) AU messages format in Sip6 */
typedef enum {
    /* fields from 'MAC NA moved' */
     SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MESSAGE_ID
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MSG_TYPE
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_VALID
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SKIP
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_AGE
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_CHAIN_TOO_LONG
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_OFFSET
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_UP0
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_DEV_ID
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_RESERVED
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_FID

    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_IS_TRUNK
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_EPORT_NUM
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_TRUNK_NUM

    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_10_7
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_0
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_4_1
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_6_5

    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_11_7
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_6_3
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_2_1
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_0

    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1_11_7
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1_6_1
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1_0

    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SP_UNKNOWN

    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_IS_MOVED
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_IS_TRUNK
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_EPORT
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_TRUNK_NUM
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_DEVICE
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_SRC_ID

    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_IS_TRUNK
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_EPORT_NUM
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_TRUNK_NUM

    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_USER_DEFINED_10_7
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_USER_DEFINED_0
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_USER_DEFINED_4_1
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_USER_DEFINED_6_5

    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_SOURCE_ID_11_7
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_SOURCE_ID_6_3
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_SOURCE_ID_2_1
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_SOURCE_ID_0

    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_ORIG_VID1_11_7
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_ORIG_VID1_6_1
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_ORIG_VID1_0

    /******************************/
    /* extra fields from 'MAC NA' */
    /******************************/

    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_VIDX
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_INDEX_8_0
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_INDEX_20_9
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_ENTRY_FOUND
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_DIP
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SIP_26_0
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SIP_30_27
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SIP_31
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SA_ACCESS_LEVEL
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_DA_ACCESS_LEVEL

    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SEARCH_TYPE
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MULTIPLE
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_DA_ROUTE
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_IS_STATIC
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_DA_CMD
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SA_CMD

    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_FDB_LOOKUP_KEY_MODE
    ,SMEM_SIP6_30_FDB_AU_MSG_TABLE_FIELDS_MAC_MC_IP_EPG_STREAM_ID
    ,SMEM_SIP6_30_FDB_AU_MSG_TABLE_FIELDS_UC_EPG_STREAM_ID

    /****************************************************************/
    /* extra fields from 'IPv4 of FCOE or IPv6-Data Update Message' */
    /****************************************************************/
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MAC_ADDR_INDEX_20_9
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_VRF_ID
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV4_DIP
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_127_106
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MTU_INDEX
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_USE_VIDX
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EVIDX
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IS_TRUNK
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TARGET_DEVICE
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EPORT_NUM
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TRUNK_NUM
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ECMP_POINTER
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_PTR
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT
    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE


    /********************************/
    /* ipv6 key routing fields      */
    /********************************/
    ,SMEM_SIP6_FDB_AU_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_31_0
    ,SMEM_SIP6_FDB_AU_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_63_32
    ,SMEM_SIP6_FDB_AU_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_95_64
    ,SMEM_SIP6_FDB_AU_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_105_96

    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS___LAST_VALUE___E
}SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_ENT;

/* macro to shorten the calling code of (FDB) 'AU MSG' fields - for GET field */
#define SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(_devObjPtr,_memPtr,fieldName)       \
    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(_devObjPtr,_memPtr,fieldName)

/* macro to shorten the calling code of (FDB) 'AU MSG' fields - for SET field */
#define SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(_devObjPtr,_memPtr,fieldName,_value)       \
    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(_devObjPtr,_memPtr,fieldName,_value)

/* macro to shorten the calling code of (FDB) 'AU MSG' field - MAC ADDR - for GET field */
#define SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_MAC_ADDR_GET(_devObjPtr,_memPtr,_valueArr)       \
    snetFieldFromEntry_Any_Get(_devObjPtr,                           \
        _memPtr,                           \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_AU_MSG_E].formatNamePtr, \
        0xFFFFFFFF,/* no read index for AU message */                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_AU_MSG_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_AU_MSG_E].fieldsNamePtr,       \
        SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR,_valueArr)

/* macro to shorten the calling code of (FDB) 'AU MSG' field - MAC ADDR - for SET field */
#define SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_MAC_ADDR_SET(_devObjPtr,_memPtr,_valueArr)       \
    snetFieldFromEntry_Any_Set(_devObjPtr,                           \
        _memPtr,                           \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_AU_MSG_E].formatNamePtr, \
        0xFFFFFFFF,/* no read index for AU message */                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_AU_MSG_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_AU_MSG_E].fieldsNamePtr,       \
        SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR,_valueArr)

/**
* @enum SMEM_SIP6_FDB_FDB_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of FDB table in SIP6 devices.(FDB unit)
 * NOTE: the FDB entry reduced to 115 bits from 140 in SIP5 devices.
*/
typedef enum {
     SMEM_SIP6_FDB_FDB_TABLE_FIELDS_VALID
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SKIP
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_AGE
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_FID
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_MAC_ADDR
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DEV_ID
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DIP
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SIP_26_0
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_11_7
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_6_1
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_0
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_7
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_6_3
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_10_7
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_0
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_4_1
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_6_5
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SIP_30_27
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_VIDX
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_IS_TRUNK
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_TRUNK_NUM
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_EPORT_NUM
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SIP_31
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_MULTIPLE
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_IS_STATIC
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DA_CMD
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SA_CMD
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DA_ROUTE
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE
    ,SMEM_SIP6_30_FDB_FDB_TABLE_FIELDS_EPG_STREAM_ID


    /**********************************/
    /* ipv4/6 and fcoe routing fields */
    /**********************************/
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_VRF_ID
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV4_DIP
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_127_106
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MTU_INDEX
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_USE_VIDX
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_EVIDX
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_IS_TRUNK
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_EPORT
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_TRUNK_ID
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_DEV
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ECMP_POINTER
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ARP_PTR
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_POINTER_TYPE
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE

    /********************************/
    /* ipv6 key routing fields      */
    /********************************/
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_31_0
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_63_32
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_95_64
    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_105_96

    /************************/
    /* DDE fields (SIP 6.30)*/
    /************************/
    /* NOTE: sharing the same lower bits of :
         SMEM_SIP6_FDB_FDB_TABLE_FIELDS_VALID               0..0
        ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SKIP                1..1
        ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_AGE                 2..2
        ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE      3..5
    */
    ,SMEM_SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_MAC_ADDRESS         /*6..53*/
    ,SMEM_SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_SEQUENCE_NUMBER     /*54..69*/
    ,SMEM_SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TARGET_PORT_BITMAP  /*70..79*/
    ,SMEM_SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TARGET_COUNTERS     /*80..99*/
    ,SMEM_SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TARGET_SOURCE_HPORT /*100..103*/
    ,SMEM_SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TARGET_INSTANCE     /*104..105*/
    ,SMEM_SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TIMESTAMP           /*106..127*/

    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E

}SMEM_SIP6_FDB_FDB_TABLE_FIELDS_ENT;

/* macro to shorten the calling code of (FDB) 'FDB' table fields - for GET field */
#define SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)       \
    SNET_TABLE_ENTRY_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName,SKERNEL_TABLE_FORMAT_FDB_E)

/* macro to shorten the calling code of (FDB) 'FDB' table fields - for SET field */
#define SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value)       \
    SNET_TABLE_ENTRY_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value,SKERNEL_TABLE_FORMAT_FDB_E)

/* macro to shorten the calling code of (FDB) 'FDB' table field - MAC ADDR - for GET field */
#define SMEM_SIP6_FDB_FDB_ENTRY_FIELD_MAC_ADDR_GET(_devObjPtr,_memPtr,_index,_valueArr)       \
    snetFieldFromEntry_Any_Get(_devObjPtr,                           \
        _memPtr,                                                        \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_E].formatNamePtr,                              \
        _index,/* the index */                                          \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_E].fieldsNamePtr, \
        SMEM_SIP6_FDB_FDB_TABLE_FIELDS_MAC_ADDR,_valueArr)

/* macro to shorten the calling code of (FDB) 'FDB' table field - MAC ADDR - for SET field */
#define SMEM_SIP6_FDB_FDB_ENTRY_FIELD_MAC_ADDR_SET(_devObjPtr,_memPtr,_index,_valueArr)       \
    snetFieldFromEntry_Any_Set(_devObjPtr,                           \
        _memPtr,                                                        \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_E].formatNamePtr,                              \
        _index,/* the index */                                          \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_E].fieldsNamePtr, \
        SMEM_SIP6_FDB_FDB_TABLE_FIELDS_MAC_ADDR,_valueArr)


/* macro to shorten the calling code of 'FDB' table DDE format field - MAC ADDR - for GET field */
#define SMEM_SIP6_FDB_DDE_ENTRY_FIELD_MAC_ADDR_GET(_devObjPtr,_memPtr,_index,_valueArr) \
    snetFieldFromEntry_Any_Get(_devObjPtr,                                     \
        _memPtr,                                                               \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_E].formatNamePtr, \
        _index,/* the index */                                                 \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_E].fieldsNamePtr, \
        SMEM_SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_MAC_ADDRESS,_valueArr)

/* macro to shorten the calling code of 'FDB' table DDE format field - MAC ADDR - for SET field */
#define SMEM_SIP6_FDB_DDE_ENTRY_FIELD_MAC_ADDR_SET(_devObjPtr,_memPtr,_index,_valueArr) \
    snetFieldFromEntry_Any_Set(_devObjPtr,                                     \
        _memPtr,                                                               \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_E].formatNamePtr, \
        _index,/* the index */                                                 \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_FDB_E].fieldsNamePtr, \
        SMEM_SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_MAC_ADDRESS,_valueArr)

/**
* @enum SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of EXACT_MATCH table in SIP6 devices.
*/
typedef enum {
     SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_VALID
    ,SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE

    /*EM ACTION*/
    ,SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_AGE
    ,SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_SIZE
    ,SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_LOOKUP_NUMBER
    ,SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_31_0
    ,SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_39_32
    ,SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_31_0
    ,SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_63_32
    ,SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_67_64

    /*EM KEY*/
    ,SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_31_0
    ,SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_63_32
    ,SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_95_64
    ,SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_111_96

    ,SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS___LAST_VALUE___E

}SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_ENT;

/* macro to shorten the calling code of (EM) 'Exact match' table fields - for GET field */
#define SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)       \
    SNET_TABLE_ENTRY_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName,SKERNEL_TABLE_FORMAT_EXACT_MATCH_ENTRY_E)

/* macro to shorten the calling code of (EM) 'Exact match' table fields - for SET field */
#define SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value)       \
    SNET_TABLE_ENTRY_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value,SKERNEL_TABLE_FORMAT_EXACT_MATCH_ENTRY_E)

/* macro to shorten the calling code of (HA) tunnel START table field - Generic TS entry Data1 bytes - for GET field */
#define SMEM_SIP6_HA_TUNNEL_START_ENTRY_FIELD_DATA1_GET(_devObjPtr,_memPtr,_index,_valueArr)       \
    snetFieldFromEntry_Any_Get(_devObjPtr,                           \
        _memPtr,                                                     \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TUNNEL_START_E].formatNamePtr,                      \
        _index,/* the index */                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TUNNEL_START_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TUNNEL_START_E].fieldsNamePtr, \
        SMEM_SIP6_HA_TUNNEL_START_TABLE_FIELDS_DATA1,_valueArr)

/* macro to shorten the calling code of (HA) tunnel START table field - Generic TS entry Data2 bytes - for GET field */
#define SMEM_SIP6_HA_TUNNEL_START_ENTRY_FIELD_DATA2_GET(_devObjPtr,_memPtr,_index,_valueArr)       \
    snetFieldFromEntry_Any_Get(_devObjPtr,                           \
        _memPtr,                                                     \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TUNNEL_START_E].formatNamePtr,                      \
        _index,/* the index */                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TUNNEL_START_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TUNNEL_START_E].fieldsNamePtr, \
        SMEM_SIP6_HA_TUNNEL_START_TABLE_FIELDS_DATA2,_valueArr)

/**
* @internal smemFalconPexAndBar0DeviceUnitAlloc function
* @endinternal
*
* @brief   allocate 'PEX config space' and 'BAR0' -- if not allocated already
* @param[in] devObjPtr                - pointer to device object.
*
*/
void smemFalconPexAndBar0DeviceUnitAlloc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemFalconSpecificDeviceUnitAlloc_SIP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemFalconSpecificDeviceUnitAlloc_SIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal hawk_tmp_use_smemFalconSpecificDeviceUnitAlloc_DP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DP units -- (non sip units)
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void hawk_tmp_use_smemFalconSpecificDeviceUnitAlloc_DP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemFalconTxqQfcCounterTable1Update function
* @endinternal
*
* @brief   function to update the number of descriptors in the txq of a txdma port.
*
* @param[in] devObjPtr      - pointer to device object.
* @param[in] globalDmaPort  - global dma number
* @param[in] increment      - GT_TRUE - increment , GT_FALSE - decrement

*/
void smemFalconTxqQfcCounterTable1Update(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 globalDmaPort,
    IN GT_BOOL  increment
);

/**
* @internal smemSip6DfxServerOwnDevNum function
* @endinternal
*
* @brief   function for sip6 devices to set/get the DFX <ownDev>.
*
* @param[in] devObjPtr     - pointer to device object.
* @param[in] setOrGet      - GT_TRUE-set , GT_FALSE-get
* @param[in/out] valuePtr  - on 'set' use '*valuePtr' , on 'get' return value in '*valuePtr'

*/
void smemSip6DfxServerOwnDevNum
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_BOOL                 setOrGet,
    IN /* or */ OUT GT_U32    *valuePtr
);

int  sim_sip6_units_cellCompare
(
    const GT_VOID * cell1Ptr,
    const GT_VOID * cell2Ptr
);


/* next can be used to state that MIB support 'preemption MAC' (EMAC,PMAC) for :
    smemFalconActiveWriteMtiStatisticControl,
    smemFalconActiveReadMtiStatisticCounters
*/
#define SMEM_ACTIVE_MTI_MIB_SUPPORT_PREEMPTION_MAC  1

ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemFalconActiveReadLatencyProfileStatTable);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemFalconActiveReadMtiStatisticCounters);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemFalconActiveWriteLatencyOverThreshIntrCauseReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemFalconActiveWriteMtiStatisticControl);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemFalconActiveWriteCpuPortInterruptsMaskReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemFalconActiveWritePcsControl1);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemFalconActiveWritePortInterruptsMaskReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemFalconActiveWriteRxDmaDebugClearAllCounters);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemFalconActiveWriteToTxDmaEnableDebugCounters);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemFalconActiveWriteTxqQfcCounterTable1IndirectReadAddress);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemFalconActiveWriteConfiProcessorGlobalConfiguration);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemFalconActiveWriteConfiProcessorIpcMemory);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemFalconh */


