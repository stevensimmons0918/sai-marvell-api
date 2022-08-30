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
* @file cpssPxTables.h
*
* @brief PX - API definition for tables access
*
* @version   1
********************************************************************************
*/


#ifndef __cpssPxTables_h
#define __cpssPxTables_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/private/prvCpssMisc.h>
#include <cpss/common/cpssTypes.h>

/*******************************************************************************
* PRV_CPSS_PX_TABLE_SIZE_GET_MAC
*
* DESCRIPTION:
*       get the number of table in accessTableInfoPtr (see PRV_PX_TABLE_INFO_PTR_GET_MAC)
* INPUTS:
*       devNum - device number
*
* OUTPUTS:
*       NONE
*
* RETURNS:
*       the number of table in accessTableInfoPtr (see PRV_PX_TABLE_INFO_PTR_GET_MAC)
*
* COMMENTS:
*
*
*******************************************************************************/
#define PRV_CPSS_PX_TABLE_SIZE_GET_MAC(devNum) \
           (PRV_CPSS_PX_PP_MAC(devNum)->accessTableInfoSize)

/*******************************************************************************
* PRV_PX_TABLE_INFO_PTR_GET_MAC
*
* DESCRIPTION:
*       PX - get the table info
* INPUTS:
*       devNum - device number
*       tableType - the table to get info
* OUTPUTS:
*       NONE
*
* RETURNS:
*       pointer to the table's info
*
* COMMENTS:
*
*
*******************************************************************************/
#define PRV_PX_TABLE_INFO_PTR_GET_MAC(_devNum, _tableType) \
 (&PRV_CPSS_PX_PP_MAC(_devNum)->accessTableInfoPtr[_tableType])

/**
* @enum CPSS_PX_TABLE_ENT
 *
 * @brief names of the tables that requires 'tabels access mechanism'.
 * (for direct and in direct access for read/write)
 * !!!! Values of enum linked with tables DB. Tables DB must be updated
 * in any cases of changes in the enum !!!!!!
*/
typedef enum{

    /** BMA port mapping */
    CPSS_PX_TABLE_BMA_PORT_MAPPING_E

    /** BMA multicast counters */
    ,CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E

    /** @brief txq shaper per port per TC token bucket configuration
     *  CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E : DQ[1]
     */
    ,CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E

    /** @brief txq shaper per port token bucket configuration
     *  CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E : DQ[1]
     */
    ,CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E

    /** @brief Map the ingress port to TxQ port for PFC response
     *  CPSS_PX_TABLE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E : DQ[1]
     */
    ,CPSS_PX_TABLE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E

    /** @brief Tail Drop Maximum Queue Limits */
    ,CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_MAX_QUEUE_LIMITS_E

    /** @brief  Tail Drop Queue Limits DP0 - Enqueue */
    ,CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP0_E

    /** @brief  Tail Drop Queue Limits DP1 - Enqueue */
    ,CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP1_E

    /** @brief Tail Drop Queue Limits DP2 - Enqueue */
    ,CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP2_E

    /** Tail Drop Queue Buffer Limits - Dequeue */
    ,CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_BUF_LIMITS_E

    /** Tail Drop Queue Descriptor Limits - Dequeue */
    ,CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_DESC_LIMITS_E

    /** @brief Tail Drop Counters -
     *  Buffers Queue Maintenance counters
     */
    ,CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E

    /** @brief CN Sample Intervals table */
    ,CPSS_PX_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E

    /** @brief PFC Counters */
    ,CPSS_PX_TABLE_PFC_LLFC_COUNTERS_E

    /** @brief cnc block 0 */
    ,CPSS_PX_TABLE_CNC_0_COUNTERS_E

    /** @brief cnc block 1 */
    ,CPSS_PX_TABLE_CNC_1_COUNTERS_E

    /** @brief PCP destination port map table. */
    ,CPSS_PX_TABLE_PCP_DST_PORT_MAP_TABLE_E

    /** @brief PCP port filtering table. */
    ,CPSS_PX_TABLE_PCP_PORT_FILTERING_TABLE_E

    /** @brief  PHA HA table */
    ,CPSS_PX_TABLE_PHA_HA_TABLE_E

    /** @brief PHA source port data table. */
    ,CPSS_PX_TABLE_PHA_SRC_PORT_DATA_E

    /** @brief PHA target port data table. */
    ,CPSS_PX_TABLE_PHA_TARGET_PORT_DATA_E

    /** @brief PHA PPA shared data memory.
     *  Used by the Upstream port to Extended port (U2E) thread
     *  to determine the egress VLAN tag state per target port.
     *  ------------ MULTI-instance support
     */
    ,CPSS_PX_TABLE_PHA_SHARED_DMEM_E

    ,CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG____DQ_1___E

    ,CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG____DQ_1___E

    ,CPSS_PX_MULTI_INSTANCE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E____DQ_1___E

    /* ** @brief FW image - contains FW code
     */
    ,CPSS_PX_TABLE_PHA_FW_IMAGE_E

    /** @brief last table (must be last enum value)
     *  NOTE:
     */
    ,CPSS_PX_TABLE_LAST_E

    /* ** @brief HW tables that NOT under CPSS control */
    ,CPSS_PX_INTERNAL_TABLE_MANAGEMENT_E = CPSS_PX_TABLE_LAST_E
    ,CPSS_PX_INTERNAL_TABLE_PACKET_DATA_ECC_E
    ,CPSS_PX_INTERNAL_TABLE_PACKET_DATA_PARITY_E
    ,CPSS_PX_INTERNAL_TABLE_PACKET_DATA_CONTROL_ECC_E
    ,CPSS_PX_INTERNAL_TABLE_PACKET_DATA_CONTROL_PARITY_E
    ,CPSS_PX_INTERNAL_TABLE_DESCRIPTORS_CONTROL_E
    ,CPSS_PX_INTERNAL_TABLE_CM3_RAM_E
    ,CPSS_PX_INTERNAL_TABLE_PHA_RAM_E
    ,CPSS_PX_HW_TABLE_LAST_E

} CPSS_PX_TABLE_ENT;


/**
* @enum CPSS_PX_LOGICAL_TABLE_ENT
 *
 * @brief names of the 'logical' tables
*/
typedef enum{
     /* ** @brief PCP destination port map table. */
     CPSS_PX_LOGICAL_TABLE_INGRESS_DST_PORT_MAP_TABLE_E
     /* ** @brief PCP port filtering table. */
    ,CPSS_PX_LOGICAL_TABLE_INGRESS_PORT_FILTERING_TABLE_E
    /* ** @brief egress header alteration table */
    ,CPSS_PX_LOGICAL_TABLE_EGRESS_HEADER_ALTERATION_TABLE_E
    /* ** @brief tail drop queue limits logical table */
    ,CPSS_PX_LOGICAL_TABLE_TAIL_DROP_LIMITS_E
    /* ** @brief buffers manager multicast counters logical table */
    ,CPSS_PX_LOGICAL_TABLE_BUFFERS_MANAGER_MULTICAST_COUNTERS_E
    /* ** @brief CNC block 0 */
    ,CPSS_PX_LOGICAL_TABLE_CNC_0_COUNTERS_E
    /* ** @brief CNC block 1 */
    ,CPSS_PX_LOGICAL_TABLE_CNC_1_COUNTERS_E
    /* ** @brief TXQ CN Sample Intervals table */
    ,CPSS_PX_LOGICAL_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E
    /* ** @brief PHA PPA shared data memory.*/
    ,CPSS_PX_LOGICAL_TABLE_PHA_SHARED_DMEM_E
    /* ** @brief FW instruction memory*/
    ,CPSS_PX_LOGICAL_TABLE_PHA_FW_IMAGE_E
    /* ** @brief not under CPSS control - internal tables */
    ,CPSS_PX_LOGICAL_INTERNAL_TABLE_MANAGEMENT_E
    ,CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_ECC_E
    ,CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_PARITY_E
    ,CPSS_PX_LOGICAL_INTERNAL_TABLE_CM3_RAM_E
    ,CPSS_PX_LOGICAL_INTERNAL_TABLE_PHA_RAM_E
    ,CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_CONTROL_ECC_E
    ,CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_CONTROL_PARITY_E
    ,CPSS_PX_LOGICAL_INTERNAL_TABLE_DESCRIPTORS_CONTROL_E
    /** @brief last table (must be last enum value)
     *  Note: Values of enum linked with tables DB cpssPxLogicalToHwTablesDB.
     *  The DB _MUST BE_ updated in case of changes the enum values (order).
     */
    ,CPSS_PX_LOGICAL_TABLE_LAST_E

} CPSS_PX_LOGICAL_TABLE_ENT;

/**
* @enum CPSS_PX_SHADOW_TYPE_ENT
 *
 * @brief Defines HW tables shadow type
*/
typedef enum{

    /** @brief shadow is not used type (default)
     *  the table not require shadow (mirrored copy)
     */
    CPSS_PX_SHADOW_TYPE_NONE_E,

    /** @brief cpss shadow type
     *  the table hold/require shadow that is kept by the CPSS (in SW)
     */
    CPSS_PX_SHADOW_TYPE_CPSS_E


} CPSS_PX_SHADOW_TYPE_ENT;


/* ** @brief value to state that need support from the shadow for ALL supported tables
   to use CPSS shadow (type CPSS_PX_SHADOW_TYPE_CPSS_E)
*/
#define CPSS_PX_SHADOW_TYPE_ALL_CPSS_CNS 0xFFFFFFFF

/**
* @struct CPSS_PX_LOGICAL_TABLES_SHADOW_STC
 *
 * @brief This struct defines logical table shadow entry
*/
typedef struct{

    /** @brief name of the logical table */
    CPSS_PX_LOGICAL_TABLE_ENT logicalTableName;

    /** @brief  type of the shadow */
    CPSS_PX_SHADOW_TYPE_ENT shadowType;

} CPSS_PX_LOGICAL_TABLES_SHADOW_STC;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxTables_h */

