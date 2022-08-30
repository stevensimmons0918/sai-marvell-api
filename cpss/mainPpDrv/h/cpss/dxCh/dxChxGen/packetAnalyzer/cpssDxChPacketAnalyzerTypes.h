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
* @file cpssDxChPacketAnalyzerTypes.h
*
* @brief This file contains types for Packet Analyzer object.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChPacketAnalyzerTypesh
#define __cpssDxChPacketAnalyzerTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/* max number of bytes for PA name */
#define CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS                   128

/* max number of words for field length */
#define CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS                  64

/* max number of string length in iDebug DB */
#define CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS                       CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS

/* max number of User defined stages */
#define CPSS_DXCH_PACKET_ANALYZER_MAX_UDS_NUM_CNS                       20

/**
* @enum CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT
*
*  @brief packet analyzer fields type enumeration
*
*/

typedef enum{

    /**@brief Port & Device */

    /** local dev src is trunk  */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_IS_TRUNK_E,

    /** local dev src port */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_PORT_E,

     /** local dev src trunk id */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_TRUNK_ID_E,

    /** local dev src eport */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_EPORT_E,

    /** local dev trg phy port */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E,

    /** Orig Is Trunk */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_IS_TRUNK_E,

    /** Orig Src_Dev */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_DEV_E,

    /** Orig Src Phy Is Trunk */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_PHY_IS_TRUNK_E,

    /** Orig Src Phy Port or Trunk ID */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_PHY_PORT_OR_TRUNK_ID_E,

     /** Orig Src Trunk ID */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_TRUNK_ID_E,

    /** trg eport */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_TRG_EPORT_E,

    /** trg phy port */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_TRG_PHY_PORT_E,

    /**@brief L2 */

    /** mac sa */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E,

    /** mac da */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E,

     /** orig vid */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_VID_E,

     /** evidx */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_EVIDX_E,

    /** evlan */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E,

    /** ether type */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ETHER_TYPE_E,

    /**@brief L3 */

    /** ipv4_sip */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E,

    /** ipv4 dip */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E,

    /** ipv6 sip */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E,

    /** ipv6 dip */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_DIP_E,

    /** ipx protocol */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_IPX_PROTOCOL_E,

    /** dscp */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_DSCP_E,

    /**@brief MetaData */

    /** Byte Count */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,

    /** is ip */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E,

    /** is ipv4 */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV4_E,

    /** is ipv6 */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV6_E,

    /** ip legal */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_IP_LEGAL_E,

     /** ipm */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_IPM_E,

     /** l4 valid */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_L4_VALID_E,

    /** mac to me */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_TO_ME_E,

    /** packet cmd */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,

    /** Queue Port */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_PORT_E,

    /** Queue Priority */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_PRIORITY_E,

    /** use vidx */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_USE_VIDX_E,

    /** egress filter drop */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_FILTER_DROP_E,

    /** egress filter en */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_FILTER_EN_E,

    /** egress filter registered */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_FILTER_REGISTERED_E,

    /** cpu code */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_CPU_CODE_E,

    /** marvell tagged */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_MARVELL_TAGGED_E,

    /** marvell tagged extended */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_MARVELL_TAGGED_EXTENDED_E,

    /** bypass bridge */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_BYPASS_BRIDGE_E,

    /** bypass ingress pipe */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_BYPASS_INGRESS_PIPE_E,

    /** tunnel start */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_TUNNEL_START_E,

    /** tunnel terminated */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_TUNNEL_TERMINATED_E,

    /** egress packet cmd */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_PACKET_CMD_E,

    /** packet_trace **/
    CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_TRACE_E,

    /** outgoing_mtag_cmd **/
    CPSS_DXCH_PACKET_ANALYZER_FIELD_OUTGOING_MTAG_CMD_E,

    /** queue_offset **/
    CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_OFFSET_E,

    /** User Defined Fields */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_1_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_2_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_3_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_4_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_5_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_6_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_7_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_8_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_9_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_10_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_11_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_12_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_13_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_14_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_15_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_16_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_17_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_18_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_19_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_20_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_21_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_22_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_23_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_24_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_25_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_26_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_27_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_28_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_29_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_30_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_31_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_32_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_33_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_34_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_35_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_36_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_37_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_38_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_39_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_40_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_41_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_42_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_43_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_44_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_45_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_46_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_47_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_48_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_49_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_50_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_51_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_52_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_53_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_54_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_55_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_56_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_57_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_58_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_59_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_60_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_61_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_62_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_63_E,

    CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E

} CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT;

/**
* @enum CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT
*
*  @brief packet analyzer field assignment mode
*/
typedef enum{

    /** @brief all input fields must be assigned in each input stage
     *         required in the parameter above.
     */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E,

    /** @brief each input stage might includes partial of the fields
     *         based on hardware support.
     */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E

} CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT;

/**
* @enum CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_ENT
*
*  @brief packet analyzer sampling mode
*/
typedef enum{

    /** @brief Sample until the first match occurs, copy
     *  the data into the interface and stop sampling
     */
    CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E,

    /** @brief Continue sampling and copying data into
     *  interface
     */
    CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E

} CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_ENT;

/**
* @struct CPSS_DXCH_PACKET_ANALYZER_COUNTER_INTERRUPT_THRESH_STC
 *
 *  @brief Match counter supports interrupt setting if its
 *  value reaches configured value. The interrupt threshold
 *  configuration is built from 3 configurations, totalling in 9
 *  bits. 4 bits of counter LSB, 4 bits of counter MSB and 1 bit
 *  which is duplicated to all other bits.
*/
typedef struct{

    /** @brief Counter Interrupt Thresh LSB; 4 bits */
    GT_U32                                lsb;

    /** @brief Counter Interrupt Thresh Middle; 1 bits */
    GT_U32                                middle;

    /** @brief Counter Interrupt Thresh MSB; 4 bits */
    GT_U32                                msb;
}CPSS_DXCH_PACKET_ANALYZER_COUNTER_INTERRUPT_THRESH_STC;

/**
 * @struct CPSS_DXCH_PACKET_ANALYZER_ACTION_STC
 *
 *  @brief packet analyzer action configurations
 *
*/
typedef struct{

    /* enable/disable sampling */
    GT_BOOL                                                     samplingEnable;

    /* sampling mode */
    CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_ENT                 samplingMode;

    /* enable/disable match counter */
    GT_BOOL                                                     matchCounterEnable; /* TBD place holder; ignore */

    /* match counter interrupt threshold */
    CPSS_DXCH_PACKET_ANALYZER_COUNTER_INTERRUPT_THRESH_STC      matchCounterIntThresh;

    /* enable/disable sampling interrupt */
    GT_BOOL                                                     interruptSamplingEnable;

    /* enable/disable match counter interrupt */
    GT_BOOL                                                     interruptMatchCounterEnable;

    /* enable/disbale an inverse match */
    GT_BOOL                                                     inverseEnable;

}CPSS_DXCH_PACKET_ANALYZER_ACTION_STC;

/**
* @enum CPSS_DXCH_PACKET_ANALYZER_TYPE_ENT
*
*  @brief packet analyzer unit type for stage representation
*/
typedef enum{

    /** I_DEBUG */
    CPSS_DXCH_PACKET_ANALYZER_TYPE_I_DEBUG_ENT,

    /** TTI  */
    CPSS_DXCH_PACKET_ANALYZER_TYPE_TTI_ENT,

    /** IPCL  */
    CPSS_DXCH_PACKET_ANALYZER_TYPE_IPCL_KEY_ENT,

    /** EPCL  */
    CPSS_DXCH_PACKET_ANALYZER_TYPE_EPCL_KEY_ENT

} CPSS_DXCH_PACKET_ANALYZER_TYPE_ENT;

/**
* @enum CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT
*
*  @brief packet analyzer ingress and egress lookup stages
*/
typedef enum{

    /** Pre TTI0,1 ingress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,

    /** Pre PCL0,1,2 ingress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E,

    /** Pre Bridge ingress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,

    /** Pre Router ingress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E,

    /** Pre OAM ingress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E,

    /** Pre Policer ingress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,

    /** Pre Mll ingress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E,

    /** Pre e-Queue ingress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E,

    /** Pre filter egress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,

    /** Pre Transmit Queue egress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,

    /** Pre Header Alteration egress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,

    /** Pre OAM egress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,

    /** Pre PCL egress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,

    /** Pre Policer egress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,

    /** Pre Timestamp egress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,

    /** Pre MAC egress stage */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E,

    /** Pre programmable Header Alteration egress stage
     * (Falcon,Ac5x and Ac5p only)*/
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E,

    /** Pre replication egress stage (Falcon,Ac5x and Ac5p only) */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E,

    /** User Defined Stages */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_1_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_2_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_3_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_4_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_5_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_6_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_7_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_8_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_9_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_10_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_11_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_12_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_13_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_14_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_15_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_16_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_17_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_18_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_19_E,
    /*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E + CPSS_DXCH_PACKET_ANALYZER_MAX_UDS_NUM_CNS,*/

    /* these should be last; don't change order */
    /** ALL_INGRESS_STAGES */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E,

    /** ALL_EGRESS_STAGES */
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_E,

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E,

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E

} CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT;

/**
 * @struct CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC
 *
 *  @brief packet analyzer key attributes
 *
*/
typedef struct{

    /* key name */
    GT_CHAR             keyNameArr[CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS];

}CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC;

/**
 * @struct CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC
 *
 *  @brief packet analyzer user define stage attributes
 *
*/
typedef struct{

    /* key name */
    GT_CHAR             udsNameArr[CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS];

}CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC;

/**
 * @struct CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC
 *
 *  @brief packet analyzer user define field attributes
 *
*/
typedef struct{

    /* key name */
    GT_CHAR             udfNameArr[CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS];

}CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC;

/**
 * @struct CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC
 *
 *  @brief packet analyzer user define field name
 *
*/
typedef struct{

    /* key name */
    GT_CHAR             fieldNameArr[CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS];

}CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC;

/**
 * @struct CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC
 *
 *  @brief packet analyzer group attributes
 *
*/
typedef struct{

    /* group name */
    GT_CHAR             groupNameArr[CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS];

}CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC;

/**
 * @struct CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC
 *
 *  @brief packet analyzer rule attributes
 *
*/
typedef struct{

    /* rule name */
    GT_CHAR             ruleNameArr[CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS];

}CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC;

/**
 * @struct CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC
 *
 *  @brief packet analyzer field and its content (Data and mask)
 *
*/
typedef struct{

    /* field name */
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         fieldName;

    /* field data */
    GT_U32                                      data[CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS];

    /* field msk */
    GT_U32                                      msk[CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS];

}CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC;

/**
 * @struct CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC
 *
 *  @brief stage interface attributes
 *
*/
typedef struct{

    /* instance ID */
    GT_CHAR                                             instanceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];

    /* interface ID */
    GT_CHAR                                             interfaceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];

    /* interface index in iDebug 0..7 */
    GT_U32                                              interfaceIndex;

}CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC;


/**
* @enum CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_TYPE_ENT
*
*  @brief packet analyzer search type.
*/
typedef enum{

    /** @brief the serach is performed by device/tile/pipe/dp */
    CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_TYPE_CORE_E,

    /** @brief the serach is performed by device/physical port*/

   CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_TYPE_PORT_E,

} CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_TYPE_ENT;


/**
 * @struct CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC
 *
 *  @brief packet analyzer search attribute.
 *
*/
typedef struct{

    /* device id */
    GT_U32                                              device;

    /* search type*/
    CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_TYPE_ENT type;

    /* tile  id */
    GT_U32                                              tile;

    /* pipe id */
    GT_U32                                              pipe;

    /* data path */
    GT_U32                                              dp;

    /* port */
    GT_PHYSICAL_PORT_NUM                                portNum;

}CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC;


/**
 * @struct CPSS_DXCH_PACKET_ANALYZER_INTERNAL_FIELD_VALUE_STC
 *
 *  @brief internal field name and its content (Data and mask)
 *
*/
typedef struct{

    /* field name */
    GT_CHAR                                     fieldName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];

    /* field data */
    GT_U32                                      data[CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS];

    /* field mask */
    GT_U32                                      msk[CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS];

}CPSS_DXCH_PACKET_ANALYZER_INTERNAL_FIELD_VALUE_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPacketAnalyzerTypesh */

