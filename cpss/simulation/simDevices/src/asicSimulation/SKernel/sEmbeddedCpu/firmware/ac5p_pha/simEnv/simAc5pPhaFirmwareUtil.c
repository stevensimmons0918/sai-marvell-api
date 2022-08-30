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
* @file simAc5pPhaFirmwareUtil.c
*
* @brief Ac5p PHA processing (programmable header modifications)
*
*   APPLICABLE DEVICES:      AC5P; AC5X.
*
* @version   1
********************************************************************************
*/

#ifdef _VISUALC
    #pragma warning(disable: 4214) /* nonstandard extension used : bit field types other than int */
#endif

#include <asicSimulation/SKernel/smem/smem.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SKernel/suserframes/snetLion.h>
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/ppa_fw_defs.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/Default/ppa_fw_accelerator_commands.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/ac5p_pha/wm_asic_sim_defs.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/simSip6Pha.h"

/* Tables and functions generated automatically by fw tool per pha fw image ID  */
extern ACCEL_INFO_STC accelInfoArrAc5pDefault[];
extern THREAD_TYPE phaThreadsTypeAc5pDefault[];
void Ac5pDefault_load_accelerator_commands();
void Ac5pDefault_get_fw_version (uint32_t imageId);

extern ACCEL_INFO_STC accelInfoArrAc5pImage01[];
extern THREAD_TYPE phaThreadsTypeAc5pImage01[];
void Ac5pImage01_load_accelerator_commands();
void Ac5pImage01_get_fw_version (uint32_t imageId);


/*******************************************************************************************
   Constants and macro definitions
********************************************************************************************/

typedef enum {

    /* Default PHA firmware image ID */
    AC5P_PHA_FW_IMAGE_ID_DEFAULT,
    /* PHA firmware image ID 01 */
    AC5P_PHA_FW_IMAGE_ID_01,

    /* Not in use */
    AC5P_PHA_FW_IMAGE_ID_LAST
} AC5P_PHA_FW_IMAGE_ID_ENT;

/* Number of bytes in PHA for packet header */
#define AC5P_PACKET_HEADER_MAX_COPY_SIZE   (128+32)

/* Get fw accelerator commands maximum space size (1600 bytes) */
#define AC5P_FW_ACCEL_CMDS_SPACE_SIZE    (1600)
#define AC5P_ACCEL_CMDS_TRIG_BASE_ADDR___WORDS_SPACE_SIZE  (AC5P_FW_ACCEL_CMDS_SPACE_SIZE/4)

/* Number of WORDS in PHA for descriptor */
#define AC5P_DESC_NUM_WORDS              16  /*64 bytes*/

/* Number of WORDS in PHA for configurations */
#define AC5P_CONFIG_NUM_WORDS           (8+1)  /* 9 words*/

/* Place holder for future threads */
#define AC5P_PLACE_HOLDER_CNS (SMAIN_NOT_VALID_CNS - 1)

/* Number of supported fw versions per image */
#define AC5P_FW_VERSIONS_NUM_PER_IMAGE  3

/* Maximum number of fw versions for all supported images in Ac5p device */
#define AC5P_FW_VERSIONS_TOTAL_NUM  (AC5P_FW_VERSIONS_NUM_PER_IMAGE*AC5P_PHA_FW_IMAGE_ID_LAST)



/* Threads ID definitions */
enum PPA_FW_THREAD_ID_ENT {
    PPA_FW_THREAD_ID_0,
    PPA_FW_THREAD_ID_1,
    PPA_FW_THREAD_ID_2,
    PPA_FW_THREAD_ID_3,
    PPA_FW_THREAD_ID_4,
    PPA_FW_THREAD_ID_5,
    PPA_FW_THREAD_ID_6,
    PPA_FW_THREAD_ID_7,
    PPA_FW_THREAD_ID_8,
    PPA_FW_THREAD_ID_9,
    PPA_FW_THREAD_ID_10,
    PPA_FW_THREAD_ID_11,
    PPA_FW_THREAD_ID_12,
    PPA_FW_THREAD_ID_13,
    PPA_FW_THREAD_ID_14,
    PPA_FW_THREAD_ID_15,
    PPA_FW_THREAD_ID_16,
    PPA_FW_THREAD_ID_17,
    PPA_FW_THREAD_ID_18,
    PPA_FW_THREAD_ID_19,
    PPA_FW_THREAD_ID_20,
    PPA_FW_THREAD_ID_21,
    PPA_FW_THREAD_ID_22,
    PPA_FW_THREAD_ID_23,
    PPA_FW_THREAD_ID_24,
    PPA_FW_THREAD_ID_25,
    PPA_FW_THREAD_ID_26,
    PPA_FW_THREAD_ID_27,
    PPA_FW_THREAD_ID_28,
    PPA_FW_THREAD_ID_29,
    PPA_FW_THREAD_ID_30,
    PPA_FW_THREAD_ID_31,
    PPA_FW_THREAD_ID_32,
    PPA_FW_THREAD_ID_33,
    PPA_FW_THREAD_ID_34,
    PPA_FW_THREAD_ID_35,
    PPA_FW_THREAD_ID_36,
    PPA_FW_THREAD_ID_37,
    PPA_FW_THREAD_ID_38,
    PPA_FW_THREAD_ID_39,
    PPA_FW_THREAD_ID_40,
    PPA_FW_THREAD_ID_41,
    PPA_FW_THREAD_ID_42,
    PPA_FW_THREAD_ID_43,
    PPA_FW_THREAD_ID_44,
    PPA_FW_THREAD_ID_45,
    PPA_FW_THREAD_ID_46,
    PPA_FW_THREAD_ID_47,
    PPA_FW_THREAD_ID_48,
    PPA_FW_THREAD_ID_49,
    PPA_FW_THREAD_ID_50,
    PPA_FW_THREAD_ID_51,
    PPA_FW_THREAD_ID_52,
    PPA_FW_THREAD_ID_53,
    PPA_FW_THREAD_ID_54,
    PPA_FW_THREAD_ID_55,
    PPA_FW_THREAD_ID_56,
    PPA_FW_THREAD_ID_57,
    PPA_FW_THREAD_ID_58,
    PPA_FW_THREAD_ID_59,
    PPA_FW_THREAD_ID_60,
    PPA_FW_THREAD_ID_61,
    PPA_FW_THREAD_ID_62,
    PPA_FW_THREAD_ID_63,
    PPA_FW_THREAD_ID_64,
    PPA_FW_THREAD_ID_65,
    PPA_FW_THREAD_ID_66,
    PPA_FW_THREAD_ID_67,
    PPA_FW_THREAD_ID_68,
    PPA_FW_THREAD_ID_69,
    PPA_FW_THREAD_ID_70,
    PPA_FW_THREAD_ID_71,
    PPA_FW_THREAD_ID_72,
    PPA_FW_THREAD_ID_73,
    PPA_FW_THREAD_ID_74,
    PPA_FW_THREAD_ID_75,
    PPA_FW_THREAD_ID_76,
    PPA_FW_THREAD_ID_77,
    PPA_FW_THREAD_ID_78,
    PPA_FW_THREAD_ID_79,
    PPA_FW_THREAD_ID_NA
};

#define STATE_NAME_AND_TYPE_MAC(thread) \
    #thread/*name*/ , thread

/* define for 'not implemented' cases */
#define ac5pMemCast_DEFAULT_IMPLEMENTATION(structCast)                          \
    struct structCast  *ac5pMemCast_##structCast(uint32_t address)              \
    {                                                                           \
        return  (struct structCast  *)pha_findMem(address);                     \
    }                                                                           \
                                                                                \
    void ac5pMemApply_##structCast(uint32_t address)                            \
    {                                                                           \
        /* function not needed ! do nothing*/                                   \
        return;                                                                 \
    }

ac5pMemCast_DEFAULT_IMPLEMENTATION(ppa_in_desc                 );
ac5pMemCast_DEFAULT_IMPLEMENTATION(SRv6_Header                 );
ac5pMemCast_DEFAULT_IMPLEMENTATION(IPv4_Header                 );
ac5pMemCast_DEFAULT_IMPLEMENTATION(IPv6_Header                 );
ac5pMemCast_DEFAULT_IMPLEMENTATION(ac5p_fw_version             );
ac5pMemCast_DEFAULT_IMPLEMENTATION(thr12_MPLS_NO_EL_in_hdr     );
ac5pMemCast_DEFAULT_IMPLEMENTATION(thr13_MPLS_ONE_EL_in_hdr    );
ac5pMemCast_DEFAULT_IMPLEMENTATION(thr14_MPLS_TWO_EL_in_hdr    );
ac5pMemCast_DEFAULT_IMPLEMENTATION(thr15_MPLS_THREE_EL_in_hdr  );
ac5pMemCast_DEFAULT_IMPLEMENTATION(thr11_vxlan_gpb_cfg         );
ac5pMemCast_DEFAULT_IMPLEMENTATION(thr11_vxlan_gpb_in_hdr_ipv4 );
ac5pMemCast_DEFAULT_IMPLEMENTATION(thr11_vxlan_gpb_in_hdr_ipv6 );
ac5pMemCast_DEFAULT_IMPLEMENTATION(thr48_srv6_best_effort_in_hdr );
ac5pMemCast_DEFAULT_IMPLEMENTATION(thr49_srv6_source_node_1_container_in_hdr );
ac5pMemCast_DEFAULT_IMPLEMENTATION(thr53_SRV6_End_Node_GSID_COC32_in_hdr );
ac5pMemCast_DEFAULT_IMPLEMENTATION(thr53_SRV6_End_Node_GSID_COC32_cfg );
ac5pMemCast_DEFAULT_IMPLEMENTATION(thr53_SRV6_End_Node_GSID_COC32_out_hdr );
ac5pMemCast_DEFAULT_IMPLEMENTATION(thr59_sls_test_in_hdr );


/*******************************************************************************************
   AC5P PHA threads information table
   Includes: instruction pointer , threadId , threadType , threadDescription
********************************************************************************************/

static PHA_THREAD_INFO ac5p_phaThreadsInfo[] = {
    /* 0*/ {AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_0  , invalidFirmwareThread , "PHA_THREAD_DO_NOTHING: do no modifications " }
    /* 1*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_1  , invalidFirmwareThread , "PHA_THREAD_SRV6_END_NODE: Covers the IPv6 Segment Routing (SR)" }
    /* 2*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_2  , invalidFirmwareThread , "PHA_THREAD_SRV6_SOURCE_NODE_1_SEGMENT: Handle source node IPv6 SR with 1 segment packet" }
    /* 3*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_3  , invalidFirmwareThread , "PHA_THREAD_SRV6_SOURCE_NODE_FIRST_PASS_2_3_SEGMENTS: Handle first pass source node IPv6 SR with 2 or 3 segments packet" }
    /* 4*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_4  , invalidFirmwareThread , "PHA_THREAD_SRV6_SOURCE_NODE_SECOND_PASS_3_SEGMENTS: Handle second pass source node IPv6 SR with 3 segments packet" }
    /* 5*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_5  , invalidFirmwareThread , "PHA_THREAD_SRV6_SOURCE_NODE_SECOND_PASS_2_SEGMENTS: Handle second pass source node IPv6 SR with 2 segments packet" }
    /* 6*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_6  , invalidFirmwareThread , "PHA_THREAD_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4: ERSPAN over IPv4 tunnel for ePort-based CC while Egress port of the ERSPAN packet is line card port" }
    /* 7*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_7  , invalidFirmwareThread , "PHA_THREAD_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6: ERSPAN over IPv6 tunnel for ePort-based CC while Egress port of the ERSPAN packet is line card port" }
    /* 8*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_8  , invalidFirmwareThread , "PHA_THREAD_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4: ERSPAN over IPv4 tunnel for ePort-based CC while Egress port of the ERSPAN packet is Falcon direct port" }
    /* 9*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_9  , invalidFirmwareThread , "PHA_THREAD_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6: ERSPAN over IPv6 tunnel for ePort-based CC while Egress port of the ERSPAN packet is Falcon direct port" }
    /*10*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_10 , invalidFirmwareThread , "PHA_THREAD_CC_ERSPAN_TYPE_II_SrcDev: Handle SrcDevMirroring for ingress/egress" }
    /*11*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_11 , invalidFirmwareThread , "PHA_THREAD_VXLAN_GPB_SourceGroupPolicyID: Handle VXLAN-GPB packets when Source Group Policy ID is carried in Desc<Copy Reserved> field" }
    /*12*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_12 , invalidFirmwareThread , "PHA_THREAD_MPLS_SR_NO_EL: Handle MPLS Segment Routing with no Entropy Label" }
    /*13*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_13 , invalidFirmwareThread , "PHA_THREAD_MPLS_SR_ONE_EL: Handle MPLS Segment Routing with one Entropy Label" }
    /*14*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_14 , invalidFirmwareThread , "PHA_THREAD_MPLS_SR_TWO_EL: Handle MPLS Segment Routing with two Entropy Label" }
    /*15*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_15 , invalidFirmwareThread , "PHA_THREAD_MPLS_SR_three_EL: Handle MPLS Segment Routing with three Entropy Label" }
    /*16*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_16 , invalidFirmwareThread , "PHA_THREAD_SGT_NET_ADD_MSB: Handles Network Port. Add MSB SGT (Security Group Tag)" }
    /*17*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_17 , invalidFirmwareThread , "PHA_THREAD_SGT_NET_FIX: Handles Network Port with Fix SGT (Security Group Tag)" }
    /*18*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_18 , invalidFirmwareThread , "PHA_THREAD_SGT_NET_REMOVE: Handles Network Port. Remove SGT (Security Group Tag)" }
    /*19*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_19 , invalidFirmwareThread , "PHA_THREAD_SGT_EDSA_FIX: Handles eDSA SGT packets while SGT remains fix" }
    /*20*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_20 , invalidFirmwareThread , "PHA_THREAD_SGT_EDSA_REMOVE: Handles eDSA SGT packets while SGT is removed" }
    /*21*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_21 , invalidFirmwareThread , "PHA_THREAD_SGT_GBP_FIX_IPV4: Handles VXLAN GBP IPv4 packets" }
    /*22*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_22 , invalidFirmwareThread , "PHA_THREAD_SGT_GBP_FIX_IPV6: Handle VXLAN GBP IPv6 packets" }
    /*23*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_23 , invalidFirmwareThread , "PHA_THREAD_SGT_GBP_REMOVE_IPv4: Handles IPv4 VXLAN-GBP SGT packets while SGT is removed " }
    /*24*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_24 , invalidFirmwareThread , "PHA_THREAD_SGT_GBP_REMOVE_IPV6: Handles IPv6 VXLAN-GBP SGT packets while SGT is removed" }
    /*25*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_25 , invalidFirmwareThread , "PHA_THREAD_PTP_PHY_1_STEP: Handles 1 step timestamping for PTPv2 in Marvell Switch-PHY system" }
    /*26*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_26 , invalidFirmwareThread , "PHA_THREAD_SRV6_PENULTIMATE_END_NODE: Handle the penultimate End Node" }
    /*27*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_27 , invalidFirmwareThread , "PHA_THREAD_INT_INGRESS_SWITCH_IPV6: Handle INT Ingress with IPv6 packet" }
    /*28*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_28 , invalidFirmwareThread , "PHA_THREAD_INT_INGRESS_SWITCH_IPV4: Handle INT Ingress with IPv4 packet" }
    /*29*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_29 , invalidFirmwareThread , "PHA_THREAD_INT_TRANSIT_SWITCH_IPV6: Handle INT transit with IPv6 packet" }
    /*30*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_30 , invalidFirmwareThread , "PHA_THREAD_INT_TRANSIT_SWITCH_IPV4: Handle INT transit with IPv4 packet" }
    /*31*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_31 , invalidFirmwareThread , "PHA_THREAD_IOAM_INGRESS_SWITCH_IPV4: Handle IOAM Ingress with IPv4 packet" }
    /*32*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_32 , invalidFirmwareThread , "PHA_THREAD_IOAM_INGRESS_SWITCH_IPV6: Handle IOAM Ingress with IPv6 packet" }
    /*33*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_33 , invalidFirmwareThread , "PHA_THREAD_IOAM_TRANSIT_SWITCH_IPV4: Handle IOAM Ingress with IPv4 packet" }
    /*34*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_34 , invalidFirmwareThread , "PHA_THREAD_IOAM_TRANSIT_SWITCH_IPV6: Handle IOAM Ingress with IPv6 packet" }
    /*35*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_35 , invalidFirmwareThread , "PHA_THREAD_IOAM_EGRESS_SWITCH_IPV6: Handle IOAM Egress with IPv6 packet" }
    /*36*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_36 , invalidFirmwareThread , "PHA_THREAD_INT_IOAM_Mirroring: Handle INT/IOAM Mirrored packets" }
    /*37*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_37 , invalidFirmwareThread , "PHA_THREAD_TUNNEL_Termination: Handle INT/IOAM Tunnel Terminated packets" }
    /*38*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_38 , invalidFirmwareThread , "PHA_THREAD_UNIFIED_SR: Handle Unified Segment Routing for IPv6 Packets" }
    /*39*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_39 , invalidFirmwareThread , "PHA_THREAD_CLASSIFIER_NSH_OVER_ETHERNET: Handle Classifier of NSH over Ethernet packets" }
    /*40*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_40 , invalidFirmwareThread , "PHA_THREAD_CLASSIFIER_NSH_OVER_VXLAN_GPE: Handle Classifier of NSH over VXLAN-GPE packets" }
    /*41*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_41 , invalidFirmwareThread , "PHA_THREAD_SFF_NSH_VXLAN_GPE_TO_ETHERNET: Handle SFF NSH of VXLAN-GPE to Ethernet packets" }
    /*42*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_42 , invalidFirmwareThread , "PHA_THREAD_SFF_NSH_ETHERNET_TO_VXLAN_GPE: Handle SFF NSH of Ethernet to VXLAN-GPE packets" }
    /*43*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_43 , invalidFirmwareThread , "PHA_THREAD_EGRESS_MIRRORING_WITH_METADATA: verification thread. Include MD associated with the original egress packet"}
    /*44*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_44 , invalidFirmwareThread , "PHA_THREAD_VARIABLE_CYCLES_LENGTH_WITH_ACCL_CMD: Debug thread which gives the user to control thread's duration"}
    /*45*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_45 , invalidFirmwareThread , "PHA_THREAD_REMOVE_ADD_BYTES: debug thread to add or remove bytes. Option to not modify the packet at all"}
    /*46*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_46 , invalidFirmwareThread , "PHA_THREAD_SFLOW_IPv4: mirroring traffic via sFlow over IPv4 udp packets" }
    /*47*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_47 , invalidFirmwareThread , "PHA_THREAD_SFLOW_IPv6: mirroring traffic via sFlow over IPv6 udp packets" }
    /*48*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_48 , invalidFirmwareThread , "PHA_THREAD_SRV6_Best_Effort: Handle SRv6 of Best Effort type tunneling" }
    /*49*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_49 , invalidFirmwareThread , "PHA_THREAD_SRV6_Source_Node_1_CONTAINER: Source node single pass processing with 1 SRH Container" }
    /*50*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_50 , invalidFirmwareThread , "PHA_THREAD_SRV6_Source_Node_First_Pass_1_CONTAINER: Handle source node of first pass processing in which 1 container is being added" }
    /*51*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_51 , invalidFirmwareThread , "PHA_THREAD_SRV6_Source_Node_Second_Pass_3_CONTAINER: Handles source node of second pass processing in which SRH and 2 containers are added" }
    /*52*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_52 , invalidFirmwareThread , "PHA_THREAD_SRV6_Source_Node_Second_Pass_2_CONTAINER: Handles source node of second pass processing in which SRH and 1 container are added" }
    /*53*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_53 , invalidFirmwareThread , "PHA_THREAD_SRV6_End_Node_GSID_COC32: covers G-SID CoC32 end node processing" }
    /*54*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_54 , invalidFirmwareThread , "TBD"}
    /*55*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_55 , invalidFirmwareThread , "TBD"}
    /*56*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_56 , invalidFirmwareThread , "TBD"}
    /*57*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_57 , invalidFirmwareThread , "TBD"}
    /*58*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_58 , invalidFirmwareThread , "TBD"}
    /*59*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_59 , invalidFirmwareThread , "PHA_SLS_Test: Debug thread aimed to check PHA engin in SLS test" }
    /*60*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_60 , invalidFirmwareThread , "PHA_THREAD_DropAllTraffic: set descriptor fields to indicate packet is dropped"}
    /*61*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_61 , invalidFirmwareThread , "PHA_THREAD_save_target_port_info: save target port information in Desc<copy_reserved> field" }
    /*62*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_62 , invalidFirmwareThread , "PHA_THREAD_enhanced_sFlow_fill_remain_IPv4: handles enhanced sFlow mirroring packets over IPv4 udp tunnel in second pass after loopback" }
    /*63*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_63 , invalidFirmwareThread , "PHA_THREAD_enhanced_sFlow_fill_remain_IPv6: handles enhanced sFlow mirroring packets over IPv6 udp tunnel in second pass after loopback" }
    /*64*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_64 , invalidFirmwareThread , "TBD" }
    /*65*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_65 , invalidFirmwareThread , "TBD" }
    /*66*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_66 , invalidFirmwareThread , "PHA_THREAD_enhanced_sFlow: handles enhanced sFlow mirroring packets in first pass before loopback" }
    /*67*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_67 , invalidFirmwareThread , "TBD" }
    /*68*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_68 , invalidFirmwareThread , "TBD" }
    /*69*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_69 , invalidFirmwareThread , "TBD" }
    /*70*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_70 , invalidFirmwareThread , "TBD" }
    /*71*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_71 , invalidFirmwareThread , "TBD" }
    /*72*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_72 , invalidFirmwareThread , "TBD" }
    /*73*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_73 , invalidFirmwareThread , "TBD" }
    /*74*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_74 , invalidFirmwareThread , "TBD" }
    /*75*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_75 , invalidFirmwareThread , "TBD" }
    /*76*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_76 , invalidFirmwareThread , "TBD" }
    /*77*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_77 , invalidFirmwareThread , "TBD" }
    /*78*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_78 , invalidFirmwareThread , "TBD" }
    /*79*/,{AC5P_PLACE_HOLDER_CNS , PPA_FW_THREAD_ID_79 , invalidFirmwareThread , "TBD" }

    /* must be last */
    /*NA*/,{SMAIN_NOT_VALID_CNS , PPA_FW_THREAD_ID_NA , invalidFirmwareThread , "TBD"}
};


/*******************************************************************************************
   Table to hold pointers to Ac5p PHA fw threads types tables
   Each entry points to specific table which represents specific fw version
********************************************************************************************/
static  THREAD_TYPE *phaThreadsTypeAc5pPtr[AC5P_FW_VERSIONS_TOTAL_NUM] = {
    /* 0*/ phaThreadsTypeAc5pDefault,         /* For Default image & most up to date fw version */
    /* 1*/ phaThreadsTypeAc5pDefault,         /* For Default image & previous fw version */
    /* 2*/ phaThreadsTypeAc5pDefault,         /* For Default image & oldest fw version */
    /* 3*/ phaThreadsTypeAc5pImage01,         /* For Image01 & most up to date fw version */
    /* 4*/ phaThreadsTypeAc5pImage01,         /* For Image01 & previous fw version */
    /* 5*/ phaThreadsTypeAc5pImage01,         /* For Image01 & oldest fw version */
    };

/*******************************************************************************************
   Table to hold pointers to Ac5p PHA fw accelerator commands tables
   Each entry points to specific table which represents specific fw version
********************************************************************************************/
static  ACCEL_INFO_STC *accelInfoArrAc5pPtr[AC5P_FW_VERSIONS_TOTAL_NUM] = {
    /* 0*/ accelInfoArrAc5pDefault,           /* For Default image & most up to date fw version */
    /* 1*/ accelInfoArrAc5pDefault,           /* For Default image & previous fw version */
    /* 2*/ accelInfoArrAc5pDefault,           /* For Default image & oldest fw version */
    /* 3*/ accelInfoArrAc5pImage01,           /* For Image01 & most up to date fw version */
    /* 4*/ accelInfoArrAc5pImage01,           /* For Image01 & previous fw version */
    /* 5*/ accelInfoArrAc5pImage01,           /* For Image01 & oldest fw version */
    };

/*******************************************************************************************
   Table to hold functions' pointers
   Each entry holds load_accelerator_commands() function compatible to specific fw version
********************************************************************************************/
static  void (*load_accelerator_commands[AC5P_FW_VERSIONS_TOTAL_NUM])() = {
    /* 0*/ Ac5pDefault_load_accelerator_commands,
    /* 1*/ Ac5pDefault_load_accelerator_commands,
    /* 2*/ Ac5pDefault_load_accelerator_commands,
    /* 3*/ Ac5pImage01_load_accelerator_commands,
    /* 4*/ Ac5pImage01_load_accelerator_commands,
    /* 5*/ Ac5pImage01_load_accelerator_commands,
    };

/*******************************************************************************************
   Table to hold functions' pointers
   Each entry holds get_fw_version() function compatible to specific fw version
********************************************************************************************/
static  void (*get_fw_version[AC5P_FW_VERSIONS_TOTAL_NUM])(GT_U32 fwImageId) = {
    /* 0*/ Ac5pDefault_get_fw_version,         /* For Default image & most up to date fw version */
    /* 1*/ Ac5pDefault_get_fw_version,         /* For Default image & previous fw version */
    /* 2*/ Ac5pDefault_get_fw_version,         /* For Default image & oldest fw version */
    /* 3*/ Ac5pImage01_get_fw_version,         /* For Image01 & most up to date fw version */
    /* 4*/ Ac5pImage01_get_fw_version,         /* For Image01 & previous fw version */
    /* 5*/ Ac5pImage01_get_fw_version,         /* For Image01 & oldest fw version */
    };


/**
* @internal simulationAc5pFirmwareThreadsInfoSet function
* @endinternal
*
* @brief  CPSS call in order to update PHA fw simulation pending fw image ID
*         For PHA threads information table per thread ID:
*         - set thread PC address
*         - set thread type
*         For PHA accelerator information table
*         - set pointer to table compatible to fw image ID
*         Load accelerator commands into PHA SP memory
*         Set fw version
*/
GT_STATUS simulationAc5pFirmwareThreadsInfoSet
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32   threadId,
    IN GT_U32   instruction_pointer,
    IN GT_U32   phaFwImageId,
    IN GT_U32   phaFwVersionId,
    IN GT_BOOL  firstTime
)
{
    GT_U32 ii;
    THREAD_TYPE *phaThreadsTypePtr;
    GT_U32 index;

    current_fw_devObjPtr = devObjPtr;/* needed for operations in
        FalconDefault_load_accelerator_commands(),Ac5pDefault_get_fw_version() */

    /* Verify fw image ID and fw version ID are valid */
    if ((phaFwImageId >= AC5P_PHA_FW_IMAGE_ID_LAST) || (phaFwVersionId>AC5P_FW_VERSIONS_NUM_PER_IMAGE))
    {
        return GT_NOT_SUPPORTED;
    }

    /* Get index to fw tables according to fw image ID and fw version ID */
    index = phaFwImageId*3 + phaFwVersionId;

    /* Set pointer to the compatible fw threads type table */
    phaThreadsTypePtr = phaThreadsTypeAc5pPtr[index];

    /* Since this function is called for each valid entry in phaFwThreadsInformation[] do the below configurations only once */
    if (firstTime)
    {
        /* Set pointer to the compatible fw acclerator information table */
        devObjPtr->PHA_FW_support.pha_acceleratorInfoPtr = accelInfoArrAc5pPtr[index];

        /* The code is written as 'single instance' so we must lock it from other simulation tasks !!! */
        SCIB_SEM_TAKE;
        /* Load accelerator commands data into SP memory */
        (*load_accelerator_commands[index])();

        /* Get falcon fw version and save it into fixed address in SP memory */
        (*get_fw_version[index])(phaFwImageId);
        SCIB_SEM_SIGNAL;

        /* Need to re-init table in case fw image was already loaded before and there are leftovers */
        for(ii = 0 ;ac5p_phaThreadsInfo[ii].instruction_pointer !=  SMAIN_NOT_VALID_CNS;ii++)
        {
            ac5p_phaThreadsInfo[ii].instruction_pointer = AC5P_PLACE_HOLDER_CNS;
            ac5p_phaThreadsInfo[ii].threadType = invalidFirmwareThread;
        }
    }

    /* According to instruction_pointer look for the threadId */
    for(ii = 0 ;ac5p_phaThreadsInfo[ii].instruction_pointer !=  SMAIN_NOT_VALID_CNS;ii++)
    {
        if((GT_U32)ac5p_phaThreadsInfo[ii].threadId == threadId)
        {
            /* Set thread PC address */
            ac5p_phaThreadsInfo[ii].instruction_pointer = instruction_pointer;
            /* Set thread type */
            ac5p_phaThreadsInfo[ii].threadType = phaThreadsTypePtr[ii];
            return GT_OK;
        }
    }

    return GT_NOT_FOUND;
}


GT_U32   simPhaDebugQueueLimit;
GT_U32   simPhaDebugQueueLen;

/**
* @internal ac5p_convertSimulationDescToPHaDesc function
* @endinternal
*
* @brief   convert Simulation Descriptor To PHA Descriptor.
*         converted descriptor is written to PPN 'desc memory'
*/
static void ac5p_convertSimulationDescToPHaDesc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32       egressPort
)
{
    struct ppa_in_desc* ppa_in_desc_Ptr;

    #define TBD 0
    #define OUT_PARAM 0

    ppa_in_desc_Ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);

    ppa_in_desc_Ptr->phal2ppa.reserved_alignment                    = TBD;
    ppa_in_desc_Ptr->phal2ppa.truncated                             = descrPtr->truncated;
    ppa_in_desc_Ptr->phal2ppa.is_trill                              = descrPtr->isTrillEtherType;
    ppa_in_desc_Ptr->phal2ppa.l4_valid                              = descrPtr->l4Valid;
    ppa_in_desc_Ptr->phal2ppa.two_byte_header_added                 = TBD;
    ppa_in_desc_Ptr->phal2ppa.egress_timestamp_tagged               = descrPtr->timestampTagged[SMAIN_DIRECTION_EGRESS_E] & 0x3;
    ppa_in_desc_Ptr->phal2ppa.ip_legal                              = !descrPtr->ipHeaderError;
    ppa_in_desc_Ptr->phal2ppa.latency_monitoring_en                 = descrPtr->lmuEn;
    ppa_in_desc_Ptr->phal2ppa.mark_ecn                              = descrPtr->markEcn;
    ppa_in_desc_Ptr->phal2ppa.oam_rdi                               = TBD;
    ppa_in_desc_Ptr->phal2ppa.force_new_dsa_tag                     = descrPtr->forceNewDsaToCpu;
    ppa_in_desc_Ptr->phal2ppa.egress_header_size                    = TBD;
    ppa_in_desc_Ptr->phal2ppa.oam_opcode                            = TBD;
    ppa_in_desc_Ptr->phal2ppa.oam_processing_en                     = TBD;
    ppa_in_desc_Ptr->phal2ppa.queue_dp                              = descrPtr->queue_dp;
    ppa_in_desc_Ptr->phal2ppa.egress_outer_is_tagged                = TBD;
    ppa_in_desc_Ptr->phal2ppa.qcn_generated_by_local_dev            = TBD;
    ppa_in_desc_Ptr->phal2ppa.lm_counter_insert_en                  = descrPtr->oamInfo.lmCounterInsertEnable;
    ppa_in_desc_Ptr->phal2ppa.local_dev_packet_source               = TBD;
    ppa_in_desc_Ptr->phal2ppa.oam_tx_period                         = TBD;
    ppa_in_desc_Ptr->phal2ppa.egress_inner_packet_type              = TBD;
    ppa_in_desc_Ptr->phal2ppa.egress_outer_packet_type              = TBD;
    ppa_in_desc_Ptr->phal2ppa.ptp_packet_format                     = TBD;
    ppa_in_desc_Ptr->phal2ppa.sst_id                                = descrPtr->sstId;
    ppa_in_desc_Ptr->phal2ppa.cfi1                                  = descrPtr->cfidei1;
    ppa_in_desc_Ptr->phal2ppa.cfi0                                  = descrPtr->cfidei;
    ppa_in_desc_Ptr->phal2ppa.egress_tag_state                      = descrPtr->egressPhysicalPortInfo.egressVlanTagMode;
    ppa_in_desc_Ptr->phal2ppa.mac_timestamping_en                   = TBD;
    ppa_in_desc_Ptr->phal2ppa.local_dev_trg_phy_port                = egressPort;
    ppa_in_desc_Ptr->phal2ppa.tc                                    = descrPtr->tc;
    ppa_in_desc_Ptr->phal2ppa.dec_ttl                               = descrPtr->decTtl;
    ppa_in_desc_Ptr->phal2ppa.tunnel_terminated                     = descrPtr->tunnelTerminated;
    ppa_in_desc_Ptr->phal2ppa.trigger_qcn                           = TBD;
    ppa_in_desc_Ptr->phal2ppa.qos_mapped_up                         = TBD;
    ppa_in_desc_Ptr->phal2ppa.latency_profile                       = descrPtr->lmuProfile;
    ppa_in_desc_Ptr->phal2ppa.copy_reserved                         = descrPtr->copyReserved;
    ppa_in_desc_Ptr->phal2ppa.do_nat                                = TBD;
    ppa_in_desc_Ptr->phal2ppa.ptp_timestamp_queue_entry_id          = TBD;
    ppa_in_desc_Ptr->phal2ppa.ptp_dispatching_en                    = TBD;
    ppa_in_desc_Ptr->phal2ppa.egress_byte_count                     = descrPtr->egressByteCount;
    ppa_in_desc_Ptr->phal2ppa.do_route_ha                           = descrPtr->doRouterHa;
    ppa_in_desc_Ptr->phal2ppa.egress_tunnel_start                   = descrPtr->tunnelStart;
    ppa_in_desc_Ptr->phal2ppa.pha_metadata5                         = descrPtr->pha.pha_metadata_ext[1];
    ppa_in_desc_Ptr->phal2ppa.pha_metadata4                         = descrPtr->pha.pha_metadata_ext[0];
    ppa_in_desc_Ptr->phal2ppa.pha_metadata3                         = (descrPtr->pha.pha_metadata[0] >> 24) & 0xFF ;
    ppa_in_desc_Ptr->phal2ppa.pha_metadata2                         = (descrPtr->pha.pha_metadata[0] >> 16) & 0xFF ;
    ppa_in_desc_Ptr->phal2ppa.pha_metadata1                         = (descrPtr->pha.pha_metadata[0] >> 8) & 0xFF ;
    ppa_in_desc_Ptr->phal2ppa.pha_metadata0                         =  descrPtr->pha.pha_metadata[0] & 0xFF ;
    ppa_in_desc_Ptr->phal2ppa.queue_length                          = simPhaDebugQueueLen;
    ppa_in_desc_Ptr->phal2ppa.packet_hash                           = descrPtr->pktHash;
    ppa_in_desc_Ptr->phal2ppa.egress_outer_l3_offset                = TBD;
    ppa_in_desc_Ptr->phal2ppa.queue_limit                           = simPhaDebugQueueLimit;
    ppa_in_desc_Ptr->phal2ppa.ptp_tai_select                        = TBD;
    ppa_in_desc_Ptr->phal2ppa.ptp_timestamp_queue_select            = TBD;
    ppa_in_desc_Ptr->phal2ppa.ptp_action                            = TBD;
    ppa_in_desc_Ptr->phal2ppa.ptp_egress_tai_sel                    = TBD;
    ppa_in_desc_Ptr->phal2ppa.egress_checksum_mode                  = TBD;
    ppa_in_desc_Ptr->phal2ppa.timestamp_mask_profile                = TBD;
    ppa_in_desc_Ptr->phal2ppa.udp_checksum_update_en                = TBD;
    ppa_in_desc_Ptr->phal2ppa.udp_checksum_offset                   = TBD;
    ppa_in_desc_Ptr->phal2ppa.timestamp_offset                      = TBD;
    ppa_in_desc_Ptr->phal2ppa.timestamp_tagged                      = TBD;
    ppa_in_desc_Ptr->phal2ppa.is_trg_phy_port_valid                 = descrPtr->eArchExtInfo.isTrgPhyPortValid;
    ppa_in_desc_Ptr->phal2ppa.dp                                    = descrPtr->dp;
    ppa_in_desc_Ptr->phal2ppa.sr_end_node                           = TBD;
    ppa_in_desc_Ptr->phal2ppa.egress_filter_registered              = TBD;
    ppa_in_desc_Ptr->phal2ppa.routed                                = descrPtr->routed;
    ppa_in_desc_Ptr->phal2ppa.ptp_cf_wraparound_check_en            = TBD;
    ppa_in_desc_Ptr->phal2ppa.ptp_trigger_type                      = TBD;
    ppa_in_desc_Ptr->phal2ppa.analyzer_index                        = descrPtr->analyzerIndex;
    ppa_in_desc_Ptr->phal2ppa.egress_inner_l3_offset                = descrPtr->haToEpclInfo.l3StartOffsetPtr - descrPtr->haToEpclInfo.macDaSaPtr;
    ppa_in_desc_Ptr->phal2ppa.ptp_offset                            = descrPtr->ptpOffset;
    ppa_in_desc_Ptr->phal2ppa.egress_inner_header_offset            = descrPtr->haToEpclInfo.macDaSaPtr - devObjPtr->egressBuffer;
    ppa_in_desc_Ptr->phal2ppa.l4_offset                             = TBD;
    ppa_in_desc_Ptr->phal2ppa.tables_read_error                     = descrPtr->tables_read_error;
    ppa_in_desc_Ptr->phal2ppa.l2_echo                               = descrPtr->VntL2Echo;
    ppa_in_desc_Ptr->phal2ppa.flow_id                               = descrPtr->flowId;
    ppa_in_desc_Ptr->phal2ppa.egress_dscp                           = TBD;
    ppa_in_desc_Ptr->phal2ppa.cpu_code                              = descrPtr->cpuCode;
    ppa_in_desc_Ptr->phal2ppa.sr_eh_offset                          = TBD;
    ppa_in_desc_Ptr->phal2ppa.ttl                                   = descrPtr->ttl;
    ppa_in_desc_Ptr->phal2ppa.trg_dev                               = descrPtr->trgDev;
    ppa_in_desc_Ptr->phal2ppa.trg_phy_port                          = TBD;
    ppa_in_desc_Ptr->phal2ppa.outgoing_mtag_cmd                     = descrPtr->outGoingMtagCmd;
    ppa_in_desc_Ptr->phal2ppa.timestamp_en                          = TBD;
    ppa_in_desc_Ptr->phal2ppa.is_ptp                                = TBD;
    ppa_in_desc_Ptr->phal2ppa.orig_is_trunk                         = descrPtr->origIsTrunk;
    ppa_in_desc_Ptr->phal2ppa.use_vidx                              = descrPtr->useVidx;
    ppa_in_desc_Ptr->phal2ppa.egress_packet_cmd                     = descrPtr->packetCmd;
    ppa_in_desc_Ptr->phal2ppa.orig_src_eport                        = descrPtr->origSrcEPortOrTrnk;
    ppa_in_desc_Ptr->phal2ppa.qos_profile                           = TBD;
    if (descrPtr->oamInfo.lmCounterInsertEnable)
    {
        ppa_in_desc_Ptr->phal2ppa.timestamp                         = descrPtr->oamInfo.lmCounter;
    }
    else
    {
        ppa_in_desc_Ptr->phal2ppa.timestamp                         = descrPtr->packetTimestamp;
    }

    ppa_in_desc_Ptr->ppa_internal_desc.accessible_header_start        = TBD;
    ppa_in_desc_Ptr->ppa_internal_desc.accessible_header_length       = TBD;
    ppa_in_desc_Ptr->ppa_internal_desc.bypass_buffer_id               = TBD;
    ppa_in_desc_Ptr->ppa_internal_desc.fw_bc_modification             = TBD;
    ppa_in_desc_Ptr->ppa_internal_desc.header_remainder_size          = TBD;
    ppa_in_desc_Ptr->ppa_internal_desc.extension_increase             = TBD;
    ppa_in_desc_Ptr->ppa_internal_desc.pha_cnc_client_index_override  = descrPtr->pha.pha_cncClientIndexOverride;
    ppa_in_desc_Ptr->ppa_internal_desc.pha_cnc_client_trigger         = descrPtr->pha.pha_cncClientTrigger;
    ppa_in_desc_Ptr->ppa_internal_desc.prep_table_read_error          = TBD;
    ppa_in_desc_Ptr->ppa_internal_desc.fw_packet_command              = TBD;
    ppa_in_desc_Ptr->ppa_internal_desc.fw_drop_code                   = TBD;
    ppa_in_desc_Ptr->ppa_internal_desc.skip_count                     = descrPtr->pha.pha_SkipCounter;
    ppa_in_desc_Ptr->ppa_internal_desc.align_to_32b                   = TBD;

    return;
}


/**
* @internal ac5p_convertPHaDescToSimulationDesc function
* @endinternal
*
* @brief   convert the 'PHA descriptor' (from memory) back to fields in simulation 'descrPtr'
*/
static void ac5p_convertPHaDescToSimulationDesc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    struct ppa_in_desc* ppa_in_desc_Ptr;
    GT_U32  dummyGetValue;
    #ifdef TBD
        #undef TBD
    #endif

    #define TBD     dummyGetValue

    ppa_in_desc_Ptr = AC5P_MEM_CAST(AC5P_DESC_REGs_lo,ppa_in_desc);

    TBD                                      = ppa_in_desc_Ptr->phal2ppa.reserved_alignment                 ;
    descrPtr->truncated                      = ppa_in_desc_Ptr->phal2ppa.truncated                          ;
    descrPtr->isTrillEtherType               = ppa_in_desc_Ptr->phal2ppa.is_trill                           ;
    descrPtr->l4Valid                        = ppa_in_desc_Ptr->phal2ppa.l4_valid                           ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.two_byte_header_added              ;
    descrPtr->timestampTagged[SMAIN_DIRECTION_EGRESS_E] = ppa_in_desc_Ptr->phal2ppa.egress_timestamp_tagged ;
    descrPtr->ipHeaderError                  = ! ppa_in_desc_Ptr->phal2ppa.ip_legal                         ;
    descrPtr->lmuEn                          = ppa_in_desc_Ptr->phal2ppa.latency_monitoring_en              ;
    descrPtr->markEcn                        = ppa_in_desc_Ptr->phal2ppa.mark_ecn                           ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.oam_rdi                            ;
    descrPtr->forceNewDsaToCpu               = ppa_in_desc_Ptr->phal2ppa.force_new_dsa_tag                  ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.egress_header_size                 ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.oam_opcode                         ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.oam_processing_en                  ;
    descrPtr->queue_dp                       = ppa_in_desc_Ptr->phal2ppa.queue_dp                           ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.egress_outer_is_tagged             ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.qcn_generated_by_local_dev         ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.lm_counter_insert_en               ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.local_dev_packet_source            ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.oam_tx_period                      ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.egress_inner_packet_type           ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.egress_outer_packet_type           ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.ptp_packet_format                  ;
    descrPtr->sstId                          = ppa_in_desc_Ptr->phal2ppa.sst_id                             ;
    descrPtr->cfidei1                        = ppa_in_desc_Ptr->phal2ppa.cfi1                               ;
    descrPtr->cfidei                         = ppa_in_desc_Ptr->phal2ppa.cfi0                               ;
    descrPtr->egressTagged                   = ppa_in_desc_Ptr->phal2ppa.egress_tag_state                   ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.mac_timestamping_en                ;
    /*egressPort                               = ppa_in_desc_Ptr->phal2ppa.local_dev_trg_phy_port             ;*/
    descrPtr->tc                             = ppa_in_desc_Ptr->phal2ppa.tc                                 ;
    descrPtr->decTtl                         = ppa_in_desc_Ptr->phal2ppa.dec_ttl                            ;
    descrPtr->tunnelTerminated               = ppa_in_desc_Ptr->phal2ppa.tunnel_terminated                  ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.trigger_qcn                        ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.qos_mapped_up                      ;
    descrPtr->lmuProfile                     = ppa_in_desc_Ptr->phal2ppa.latency_profile                    ;
    descrPtr->copyReserved                   = ppa_in_desc_Ptr->phal2ppa.copy_reserved                      ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.do_nat                             ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.ptp_timestamp_queue_entry_id       ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.ptp_dispatching_en                 ;
    descrPtr->egressByteCount                = ppa_in_desc_Ptr->phal2ppa.egress_byte_count                  ;
    descrPtr->doRouterHa                     = ppa_in_desc_Ptr->phal2ppa.do_route_ha                        ;
    descrPtr->tunnelStart                    = ppa_in_desc_Ptr->phal2ppa.egress_tunnel_start                ;
    descrPtr->pha.pha_metadata_ext[1]        = ppa_in_desc_Ptr->phal2ppa.pha_metadata5                      ;
    descrPtr->pha.pha_metadata_ext[0]        = ppa_in_desc_Ptr->phal2ppa.pha_metadata4                      ;
    descrPtr->pha.pha_metadata[0]            = (ppa_in_desc_Ptr->phal2ppa.pha_metadata3 << 24 | \
                                                ppa_in_desc_Ptr->phal2ppa.pha_metadata2 << 16 | \
                                                ppa_in_desc_Ptr->phal2ppa.pha_metadata1 << 8  | \
                                                ppa_in_desc_Ptr->phal2ppa.pha_metadata0)                    ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.queue_length                       ;
    descrPtr->pktHash                        = ppa_in_desc_Ptr->phal2ppa.packet_hash                        ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.egress_outer_l3_offset             ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.queue_limit                        ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.ptp_tai_select                     ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.ptp_timestamp_queue_select         ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.ptp_action                         ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.ptp_egress_tai_sel                 ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.egress_checksum_mode               ;
    descrPtr->timestampMaskProfile           = ppa_in_desc_Ptr->phal2ppa.timestamp_mask_profile             ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.udp_checksum_update_en             ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.udp_checksum_offset                ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.timestamp_offset                   ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.timestamp_tagged                   ;
    descrPtr->eArchExtInfo.isTrgPhyPortValid = ppa_in_desc_Ptr->phal2ppa.is_trg_phy_port_valid              ;
    descrPtr->dp                             = ppa_in_desc_Ptr->phal2ppa.dp                                 ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.sr_end_node                        ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.egress_filter_registered           ;
    descrPtr->routed                         = ppa_in_desc_Ptr->phal2ppa.routed                             ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.ptp_cf_wraparound_check_en         ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.ptp_trigger_type                   ;
    descrPtr->analyzerIndex                  = ppa_in_desc_Ptr->phal2ppa.analyzer_index                     ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.egress_inner_l3_offset             ;
    descrPtr->ptpOffset                      = ppa_in_desc_Ptr->phal2ppa.ptp_offset                         ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.egress_inner_header_offset         ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.l4_offset                          ;
    descrPtr->tables_read_error              = ppa_in_desc_Ptr->phal2ppa.tables_read_error                  ;
    descrPtr->VntL2Echo                      = ppa_in_desc_Ptr->phal2ppa.l2_echo                            ;
    descrPtr->flowId                         = ppa_in_desc_Ptr->phal2ppa.flow_id                            ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.egress_dscp                        ;
    descrPtr->cpuCode                        = ppa_in_desc_Ptr->phal2ppa.cpu_code                           ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.sr_eh_offset                       ;
    descrPtr->ttl                            = ppa_in_desc_Ptr->phal2ppa.ttl                                ;
    descrPtr->trgDev                         = ppa_in_desc_Ptr->phal2ppa.trg_dev                            ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.trg_phy_port                       ;
    descrPtr->outGoingMtagCmd                = ppa_in_desc_Ptr->phal2ppa.outgoing_mtag_cmd                  ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.timestamp_en                       ;
    TBD                                      = ppa_in_desc_Ptr->phal2ppa.is_ptp                             ;
    descrPtr->origIsTrunk                    = ppa_in_desc_Ptr->phal2ppa.orig_is_trunk                      ;
    descrPtr->useVidx                        = ppa_in_desc_Ptr->phal2ppa.use_vidx                           ;
    descrPtr->packetCmd                      = ppa_in_desc_Ptr->phal2ppa.egress_packet_cmd                  ;
    descrPtr->origSrcEPortOrTrnk             = ppa_in_desc_Ptr->phal2ppa.orig_src_eport                     ;
    descrPtr->packetTimestamp                = ppa_in_desc_Ptr->phal2ppa.timestamp                          ;

    TBD                                      = ppa_in_desc_Ptr->ppa_internal_desc.accessible_header_start   ;
    TBD                                      = ppa_in_desc_Ptr->ppa_internal_desc.accessible_header_length  ;
    TBD                                      = ppa_in_desc_Ptr->ppa_internal_desc.bypass_buffer_id          ;
    descrPtr->pha.pha_fw_bc_modification     = ppa_in_desc_Ptr->ppa_internal_desc.fw_bc_modification        ;
    TBD                                      = ppa_in_desc_Ptr->ppa_internal_desc.header_remainder_size     ;
    TBD                                      = ppa_in_desc_Ptr->ppa_internal_desc.extension_increase        ;
    descrPtr->pha.pha_cncClientIndexOverride  = ppa_in_desc_Ptr->ppa_internal_desc.pha_cnc_client_index_override ;
    descrPtr->pha.pha_cncClientTrigger           = ppa_in_desc_Ptr->ppa_internal_desc.pha_cnc_client_trigger    ;
    TBD                                      = ppa_in_desc_Ptr->ppa_internal_desc.prep_table_read_error     ;
    /*TBD                                      = ppa_in_desc_Ptr->ppa_internal_desc.fw_packet_command         ;*/
    /*TBD                                      = ppa_in_desc_Ptr->ppa_internal_desc.fw_drop_code              ;*/
    descrPtr->pha.pha_SkipCounter            = ppa_in_desc_Ptr->ppa_internal_desc.skip_count                ;
    TBD                                      = ppa_in_desc_Ptr->ppa_internal_desc.align_to_32b              ;

    if(dummyGetValue == 5)
    {
        dummyGetValue = 6;
    }

    snetChtEgressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                      descrPtr->packetCmd,
                                      ppa_in_desc_Ptr->ppa_internal_desc.fw_packet_command,
                                      descrPtr->cpuCode,
                                      ppa_in_desc_Ptr->ppa_internal_desc.fw_drop_code
                                      );

    if (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
    {
        descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;
    }

    return;
}



/***********************/
/* memory for FW usage */
/***********************/
static MEMORY_RANGE ac5p_fw_memoryMap[]=
{    {0x00000000 , 2048  , "scratchpad",NULL}
    ,{AC5P_CFG_REGs_lo , 4*AC5P_CONFIG_NUM_WORDS , "configurations",NULL}
    ,{AC5P_DESC_REGs_lo , 4*AC5P_DESC_NUM_WORDS , "packet descriptor",NULL}
    ,{AC5P_PKT_REGs_lo , AC5P_PACKET_HEADER_MAX_COPY_SIZE , "packet header",NULL}

    /**************************************************************************
     * PPN I/O Registers addresses
     **************************************************************************/
    ,{PPN_IO_REG_STATUS_ADDR , PPN_IO_REG_TOD_1_WORD_2_ADDR-PPN_IO_REG_STATUS_ADDR , "PPN I/O Registers addresses",NULL}

    /*address in PPA*/ /* /Cider/EBU/PIPE/PIPE {Current}/Switching core/<PHA_IP> PHA/<PPA> PPA/PPA/Shared DMEM */
    ,{DRAM_BASE_ADDR /*0x00010000*/ , 8192 , "DRAM_BASE_ADDR",NULL}
    /*seems like address in PPN[1]*/
    ,{PKT_SWAP_INST_ADDR /*0x04000*/, 4 , "PKT_SWAP_INST_ADDR",NULL} /* where in Cider ??? */
    /*seems like address in PPN[1]*/
    ,{PKT_LOAD_PC_INST_ADDR /*0x04004*/, 4 , "PKT_LOAD_PC_INST_ADDR",NULL} /* where in Cider ??? */
    /*seems like address in PPN[1]*/
    ,{ACCEL_CMDS_TRIG_BASE_ADDR /*0x01010*/, 4*AC5P_ACCEL_CMDS_TRIG_BASE_ADDR___WORDS_SPACE_SIZE , "ACCEL_CMDS_TRIG_BASE_ADDR",NULL} /* where in Cider ??? */

    /*must be last*/
    ,{0,0,0,NULL}/*must be last*/
};

/******************************/
/* active memory for FW usage */
/******************************/
static ACTIVE_MEMORY_RANGE ac5p_fw_activeMemoryMap[]=
{
     {ACCEL_CMDS_TRIG_BASE_ADDR , AC5P_ACCEL_CMDS_TRIG_BASE_ADDR___WORDS_SPACE_SIZE , 0x4  , "ACCEL_CMDS_TRIG_BASE_ADDR",pha_fw_activeMem_write_ACCEL_CMDS_TRIG_BASE_ADDR/*write*/,NULL/*read*/}

    ,{AC5P_FW_VERSION_ADDR,       2 /* word0-2 */ , 0x4  , "VERSION,DATE registers", pha_fw_activeMem_write_toPpnMem/*write*/,NULL/*read*/}

    ,{AC5P_FW_IMAGE_ID_ADDR,      1               , 0x0  , "Firmware image ID register", pha_fw_activeMem_write_toPpnMem/*write*/,NULL/*read*/}

    ,{PPN_IO_REG_PKT_HDR_OFST_ADDR, 1,                0x0  , "PKT offset registers", pha_fw_activeMem_write_toPpnMem/*write*/, NULL}
     /* TOD mapped registers into TOD of the Eagle */
    ,{PPN_IO_REG_TOD_0_WORD_0_ADDR, 3 /* word0-2 */ , 0x4  , "TOD_0 registers", NULL/*write*/,pha_fw_activeMem_read_TOD}
    ,{PPN_IO_REG_TOD_1_WORD_0_ADDR, 3 /* word0-2 */ , 0x4  , "TOD_1 registers", NULL/*write*/,pha_fw_activeMem_read_TOD}

    /*address in PPA*//* /Cider/EBU/PIPE/PIPE {Current}/Switching core/<PHA_IP> PHA/<PPA> PPA/PPA/Shared DMEM */
    ,{DRAM_BASE_ADDR /*0x00010000*/ , 8192/4 , 4 , "DRAM_BASE_ADDR",fw_activeMem_write_DRAM_BASE_ADDR/*write*/,fw_activeMem_read_DRAM_BASE_ADDR/*read*/}

    /*must be last*/
    ,{0,0,0,NULL,NULL,NULL}/*must be last*/
};


/**
* @internal simulationAc5pFirmwareInit function
* @endinternal
*
* @brief   Init Hawk FW
*/
void simulationAc5pFirmwareInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{

    devObjPtr->PHA_FW_support.pha_acceleratorInfoPtr         = NULL;/* Set by cpssDxChPhaInit API */
    devObjPtr->PHA_FW_support.pha_memoryMap                  = ac5p_fw_memoryMap;
    devObjPtr->PHA_FW_support.pha_activeMemoryMap            = ac5p_fw_activeMemoryMap;
    devObjPtr->PHA_FW_support.addr_ACCEL_CMDS_TRIG_BASE_ADDR = ACCEL_CMDS_TRIG_BASE_ADDR;
    devObjPtr->PHA_FW_support.addr_ACCEL_CMDS_CFG_BASE_ADDR  = ACCEL_CMDS_CFG_BASE_ADDR;
    devObjPtr->PHA_FW_support.addr_PKT_REGs_lo               = AC5P_PKT_REGs_lo;
    devObjPtr->PHA_FW_support.addr_DESC_REGs_lo              = AC5P_DESC_REGs_lo;
    devObjPtr->PHA_FW_support.addr_CFG_REGs_lo               = AC5P_CFG_REGs_lo;
    devObjPtr->PHA_FW_support.phaFwApplyChangesInDescriptor  = NULL;/*not needed*/
    devObjPtr->PHA_FW_support.phaThreadsInfoPtr              = ac5p_phaThreadsInfo;
    devObjPtr->PHA_FW_support.convertSimulationDescToPHaDesc = ac5p_convertSimulationDescToPHaDesc;
    devObjPtr->PHA_FW_support.convertPHaDescToSimulationDesc = ac5p_convertPHaDescToSimulationDesc;

    /************************************************/
    /* the code is written as 'single instance' !!! */
    /* so we must lock it from other simulation tasks*/
    /************************************************/
    SCIB_SEM_TAKE;

    current_fw_devObjPtr = devObjPtr;
    ac5p_init();


    /* do sanity test for little endian memory */
    testEndianCasting(devObjPtr);

    SCIB_SEM_SIGNAL;
}
