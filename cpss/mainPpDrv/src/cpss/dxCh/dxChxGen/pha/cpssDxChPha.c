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
* @file cpssDxChPha.c
*
* @brief CPSS declarations relate to PHA (programmable header alteration) in the
*   egress processing , that allows enhanced key technologies such as:
*   Telemetry, NSH metadata, ERSPAN, MPLS, SRv6, VXLAN, SGT, PTP, Unified SR and
*   any new tunnel/shim header that may emerge.
*
*   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
*
*   NOTEs:
*   1. GT_NOT_INITIALIZED will be return for any 'PHA' lib APIs if called before
*       cpssDxChPhaInit(...)
*       (exclude cpssDxChPhaInit(...) itself)
*   2. GT_NOT_INITIALIZED will be return for EPCL APIs trying to enable PHA processing
*       if called before cpssDxChPhaInit(...)
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/pha/private/prvCpssDxChPhaLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/dxCh/dxChxGen/pha/private/prvCpssDxChPha.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* Maximum number of accelerator commands */
#define FW_ACCEL_CMDS_MAX_NUM_CNS    400

/* Header files to hold fw images information */
#include <cpss/dxCh/dxChxGen/pha/private/prvCpssDxChPpaFwImageInfo_FalconDefault.h>
#include <cpss/dxCh/dxChxGen/pha/private/prvCpssDxChPpaFwImageInfo_FalconImage01.h>
#include <cpss/dxCh/dxChxGen/pha/private/prvCpssDxChPpaFwImageInfo_FalconImage02.h>
#include <cpss/dxCh/dxChxGen/pha/private/prvCpssDxChPpaFwImageInfo_FalconImage02Previous.h>
#include <cpss/dxCh/dxChxGen/pha/private/prvCpssDxChPpaFwImageInfo_FalconImage02Oldest.h>
#include <cpss/dxCh/dxChxGen/pha/private/prvCpssDxChPpaFwImageInfo_Ac5pDefault.h>
#include <cpss/dxCh/dxChxGen/pha/private/prvCpssDxChPpaFwImageInfo_Ac5pImage01.h>


/* ALL addresses of instruction pointers need to be with prefix 0x00400000 */
/* the PHA table need to hold only lower 16 bits (the prefix is added internally by the HW) */
#define FW_INSTRUCTION_DOMAIN_ADDR_CNS      0x00400000
#define FW_INSTRUCTION_DOMAIN_ADDR_MAX_CNS  0x0040FFFF
/* max number of analyzer indexes supported by ERSPAN */
#define PRV_CPSS_DXCH_PHA_THREAD_ANALYZER_INDEX_CNS                  7
/* required number of PPATHREADsConfWords */
#define PRV_CPSS_DXCH_PPA_THREAD_CONF_WORDS_CNS                      16
/* required number of L2 PPATHREADsConfWords */
#define PRV_CPSS_DXCH_PPA_THREAD_L2_CONF_WORDS_CNS                    4
/* number of words in PHA shared memory needed for IP header template per analyzer index */
#define PRV_CPSS_DXCH_PHA_THREAD_ANALYZER_INDEX_TEMPLATE_WORD_CNS 0x10
/* Maximum number of fw versions of all supported images */
#define FW_VERSIONS_MAX_NUM_CNS  (CPSS_DXCH_PHA_FW_IMAGE_ID_LAST_E*CPSS_DXCH_PHA_FW_NUM_OF_SUPPORTED_VERSIONS_PER_IMAGE_CNS)

typedef    GT_U32   FW_THREAD_ID;



/**
 * @struct PHA_FW_IMAGE_INFO_TABLES_STC
 *
 * @brief Holds pointers and related data to specific
 *        fw tables which includes fw version information
*/
typedef struct {

    /** Pointer to table which holds fw image code */
    GT_U32    *fwImemPtr;

    /** Number of entries in table of fw image code */
    GT_U32    fwImemSize;

    /** Pointer to table which holds fw threads' addresses */
    GT_U32    *fwThreadAddrPtr;

    /** Pointer to table which holds fw accelerator commands */
    GT_U32    *fwAcclCmdPtr;

    /** Pointer to structure which holds fw version information */
    PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC   *fwImageInfoPtr;

    /** Firmware version index within fw image */
    GT_U32    fwVersIndex;
}PHA_FW_IMAGE_INFO_TABLES_STC;

/**
 * @struct PHA_FW_THREAD_STATIC_INFO_STC
 *
 * @brief Information per PHA FW threadId
*/
typedef struct {

    /** PC address of firmware thread */
    GT_U32               firmwareInstructionPointer;

    /** firmware thread's latency (proccessing cycles) */
    GT_U32               latency;

    /** Packet start point in PPN buffer */
    HEADER_WINDOW_ANCHOR headerWindowAnchor;

    /** Maximum packet expansion in PPN buffer */
    HEADER_WINDOW_SIZE   headerWindowSize;
}PHA_FW_THREAD_STATIC_INFO_STC;

/*
    Table to hold PHA firmware threads with initial data while each entry represents different thread.
    During PHA initialization this data will be copied to table per device and threads addresses
    will be set pending PHA firmware image.
*/
static const PHA_FW_THREAD_STATIC_INFO_STC  phaFwThreadsInitInformation[PRV_CPSS_DXCH_PHA_MAX_THREADS_CNS] =

{                  /* Thread address */               /* latency */ /* headerWindowAnchore */ /* headerWindowSize */
    {PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      18     ,    Outer_L2_Start       ,     EXPAND_32B  }      /*  0 THR_DoNothing                                    */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      56     ,    Outer_L3_Start       ,     EXPAND_32B  }      /*  1 THR_SRv6_End_Node                                */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      31     ,    Outer_L3_Start       ,     EXPAND_32B  }      /*  2 THR_SRv6_Source_Node_1_segment                   */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      29     ,    Outer_L2_Start       ,     EXPAND_32B  }      /*  3 THR_SRv6_Source_Node_First_Pass_2_3_segments     */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      60     ,    Outer_L2_Start       ,     EXPAND_48B  }      /*  4 THR_SRv6_Source_Node_Second_Pass_3_segments      */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      60     ,    Outer_L2_Start       ,     EXPAND_48B  }      /*  5 THR_SRv6_Source_Node_Second_Pass_2_segments      */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     183     ,    Outer_L2_Start       ,     EXPAND_64B  }      /*  6 THR_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4      */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     219     ,    Outer_L2_Start       ,     EXPAND_64B  }      /*  7 THR_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6      */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     170     ,    Outer_L2_Start       ,     EXPAND_64B  }      /*  8 THR_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4  */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     203     ,    Outer_L2_Start       ,     EXPAND_64B  }      /*  9 THR_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6  */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      30     ,    Outer_L2_Start       ,     EXPAND_64B  }      /* 10 THR_Cc_Erspan_TypeII_SrcDevMirroring             */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      43     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 11 THR_VXLAN_GPB_SourceGroupPolicyID                */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      26     ,    Outer_L3_Start       ,     EXPAND_48B  }      /* 12 THR_MPLS_SR_NO_EL                                */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      54     ,    Outer_L3_Start       ,     EXPAND_48B  }      /* 13 THR_MPLS_SR_ONE_EL                               */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      73     ,    Outer_L3_Start       ,     EXPAND_48B  }      /* 14 THR_MPLS_SR_TWO_EL                               */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      96     ,    Outer_L3_Start       ,     EXPAND_48B  }      /* 15 THR_MPLS_SR_THREE_EL                             */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      53     ,    Outer_L2_Start       ,     EXPAND_32B  }      /* 16 THR_SGT_NetAddMSB                                */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      40     ,    Outer_L2_Start       ,     EXPAND_32B  }      /* 17 THR_SGT_NetFix                                   */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      49     ,    Outer_L2_Start       ,     EXPAND_32B  }      /* 18 THR_SGT_NetRemove                                */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      20     ,    Outer_L2_Start       ,     EXPAND_32B  }      /* 19 THR_SGT_eDSAFix                                  */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      41     ,    Outer_L2_Start       ,     EXPAND_32B  }      /* 20 THR_SGT_eDSARemove                               */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      22     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 21 THR_SGT_GBPFixIPv4                               */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      23     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 22 THR_SGT_GBPFixIPv6                               */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      37     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 23 THR_SGT_GBPRemoveIPv4                            */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      34     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 24 THR_SGT_GBPRemoveIPv6                            */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      27     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 25 THR_PTP_Phy_1_Step                               */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      71     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 26 THR_SRv6_Penultimate_End_Node                    */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     143     ,    Outer_L3_Start       ,     EXPAND_64B  }      /* 27 THR_INT_Ingress_Switch_IPv6                      */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     150     ,    Outer_L3_Start       ,     EXPAND_64B  }      /* 28 THR_INT_Ingress_Switch_IPv4                      */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     170     ,    Outer_L3_Start       ,     EXPAND_64B  }      /* 29 THR_INT_Transit_Switch_IPv6                      */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     183     ,    Outer_L3_Start       ,     EXPAND_64B  }      /* 30 THR_INT_Transit_Switch_IPv4                      */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     206     ,    Outer_L3_Start       ,     EXPAND_64B  }      /* 31 THR_IOAM_Ingress_Switch_IPv4                     */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     196     ,    Outer_L3_Start       ,     EXPAND_64B  }      /* 32 THR_IOAM_Ingress_Switch_IPv6                     */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     242     ,    Outer_L3_Start       ,     EXPAND_64B  }      /* 33 THR_IOAM_Transit_Switch_IPv4                     */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     222     ,    Outer_L3_Start       ,     EXPAND_64B  }      /* 34 THR_IOAM_Transit_Switch_IPv6                     */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     303     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 35 THR_IOAM_Egress_Switch_IPv6                      */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      68     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 36 THR_Current_Hop_INT_IOAM_Mirroring               */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      47     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 37 THR_Tunnel_Terminated_Data_Traffic               */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      37     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 38 THR_Unified_SR                                   */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     254     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 39 THR_Classifier_NSH_over_Ethernet                 */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     256     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 40 THR_Classifier_NSH_over_VXLAN_GPE                */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      20     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 41 THR_SFF_NSH_VXLAN_GPE_to_Ethernet                */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      20     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 42 THR_SFF_NSH_Ethernet_to_VXLAN_GPE                */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      28     ,    Outer_L3_Start       ,     EXPAND_64B  }      /* 43 THR_EgressMirroringWithMetadata                  */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      52     ,    Outer_L2_Start       ,     EXPAND_32B  }      /* 44 THR_VariableCyclesLengthWithAcclCmd              */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      67     ,    Outer_L2_Start       ,     EXPAND_64B  }      /* 45 THR_RemoveAddBytes                               */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      94     ,    Outer_L3_Start       ,     EXPAND_64B  }      /* 46 THR_SFLOW_IPv4                                   */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      90     ,    Outer_L3_Start       ,     EXPAND_64B  }      /* 47 THR_SFLOW_IPv6                                   */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      40     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 48 THR_SRV6_Best_Effort                             */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      51     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 49 THR_SRV6_Source_Node_1_CONTAINER                 */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      27     ,    Outer_L2_Start       ,     EXPAND_32B  }      /* 50 THR_SRV6_Source_Node_First_Pass_1_CONTAINER      */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      85     ,    Outer_L2_Start       ,     EXPAND_48B  }      /* 51 THR_SRV6_Source_Node_Second_Pass_3_CONTAINER     */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      85     ,    Outer_L2_Start       ,     EXPAND_48B  }      /* 52 THR_SRV6_Source_Node_Second_Pass_2_CONTAINER     */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      55     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 53 THR_SRV6_End_Node_GSID_COC32                     */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      29     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 54 THR_IPv4_TTL_Increment                           */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      25     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 55 THR_IPv6_HopLimit_Increment                      */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      28     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 56 THR_Clear_Outgoing_Mtag_Cmd                      */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     147     ,    Outer_L3_Start       ,     EXPAND_64B  }      /* 57 THR_SFLOW_V5_IPv4                                */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     143     ,    Outer_L3_Start       ,     EXPAND_64B  }      /* 58 THR_SFLOW_V5_IPv6                                */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      56     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 59 THR_SLS_Test                                     */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      28     ,    Outer_L2_Start       ,     EXPAND_32B  }      /* 60 THR_DropAllTraffic                               */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      22     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 61 THR_save_target_port_info                        */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      34     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 62 THR62_enhanced_sFlow_fill_remain_IPv4            */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      30     ,    Outer_L3_Start       ,     EXPAND_32B  }      /* 63 THR63_enhanced_sFlow_fill_remain_IPv6            */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     132     ,    Outer_L2_Start       ,     EXPAND_64B  }      /* 64 THR_Erspan_TypeII_SameDevMirroring_Ipv4          */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,     168     ,    Outer_L2_Start       ,     EXPAND_64B  }      /* 65 THR_Erspan_TypeII_SameDevMirroring_Ipv6          */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,      76     ,    Outer_L2_Start       ,     EXPAND_64B  }      /* 66 THR66_enhanced_sFlow                             */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,       0     ,           0             ,         0       }      /* 67 TBD                                              */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,       0     ,           0             ,         0       }      /* 68 TBD                                              */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,       0     ,           0             ,         0       }      /* 69 TBD                                              */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,       0     ,           0             ,         0       }      /* 70 TBD                                              */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,       0     ,           0             ,         0       }      /* 71 TBD                                              */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,       0     ,           0             ,         0       }      /* 72 TBD                                              */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,       0     ,           0             ,         0       }      /* 73 TBD                                              */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,       0     ,           0             ,         0       }      /* 74 TBD                                              */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,       0     ,           0             ,         0       }      /* 75 TBD                                              */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,       0     ,           0             ,         0       }      /* 76 TBD                                              */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,       0     ,           0             ,         0       }      /* 77 TBD                                              */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,       0     ,           0             ,         0       }      /* 78 TBD                                              */
   ,{PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS ,       0     ,           0             ,         0       }      /* 79 TBD                                              */
} ;



#define FW_CASE_LIKE_AC5P                                               \
    /*'case ' by the caller*/CPSS_PP_FAMILY_DXCH_AC5P_E:                \
              case CPSS_PP_FAMILY_DXCH_AC5X_E:                          \
              case CPSS_PP_FAMILY_DXCH_HARRIER_E/* ' :' by the caller' */


extern GT_STATUS cpssSimFalconFirmwareThreadsInfoSet(
    IN GT_U32 cpssDevNum,
    IN GT_U32 threadId,
    IN GT_U32 firmwareInstructionPointer,
    IN CPSS_DXCH_PHA_FW_IMAGE_ID_ENT firmwareImageId,
    IN GT_U32   firmwareVersionId,
    IN GT_BOOL  firstTime
);
extern GT_STATUS cpssSimAc5pFirmwareThreadsInfoSet(
    IN GT_U32 cpssDevNum,
    IN GT_U32 threadId,
    IN GT_U32 firmwareInstructionPointer,
    IN CPSS_DXCH_PHA_FW_IMAGE_ID_ENT firmwareImageId,
    IN GT_U32   firmwareVersionId,
    IN GT_BOOL  firstTime
);

#define PHA_UNIT_CHECK(_devNum)                                       \
    if(0 == PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.phaInfo.numOfPpg)   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,       \
            "The device not supports the PHA unit") /* ; in caller */



/**
* @internal firmwareAacInit function
* @endinternal
*
* @brief   This function sets the Atomic Access Controller (AAC) related parameters to the HW.
*          - Enable AAC
*          - Set AAC address mode to direct
*          - Multicast mode
*            - for multi tiles device set to multicast (support transactions to multi tiles and control pipes)
*            - for single tile device disable multicast
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum             - physical device number
*
* @retval GT_OK                 - on success
* @retval GT_FAIL               - on other error.
* @retval GT_HW_ERROR           - on hardware error
*/
static GT_STATUS firmwareAacInit
(
    IN GT_U8    devNum
)
{
    GT_STATUS    rc;       /* holds return code status  */
    GT_U32       regAddr;  /* holds HW register address */
    GT_U32       regVal;   /* holds HW register value   */


    /* Get AAC global control register address */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACControl;
    /* Enable AAC */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, 1);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get AAC Engine <<%n>> Control register for channel PHA */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineControl[PRV_CPSS_DXCH_AAC_CHANNEL_DLB_AND_PHA_E];
    /* Read current value */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &regVal);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Set AAC address mode to direct (continues addresses which will be incremented automatic by 4 bytes) */
    U32_SET_FIELD_MAC(regVal, 0, 1, 0);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* Enable AAC multicast to support writes/reads to/from all tiles */
        U32_SET_FIELD_MAC(regVal, 2, 1, 1);
    }
    else
    {
        /* Disable AAC multicast */
        U32_SET_FIELD_MAC(regVal, 2, 1, 0);
    }
    /* Write the above in AAC Engine <<%n>> Control register */
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, regVal);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* Get AAC Engine <<%n>> MC External Offset register for channel PHA */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineMCExternalOffset[PRV_CPSS_DXCH_AAC_CHANNEL_DLB_AND_PHA_E];
        /* Set tile's offset */
        rc = prvCpssHwPpWriteRegister(devNum, regAddr, FALCON_TILE_OFFSET_CNS);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Get AAC Engine <<%n>> MC Internal Offset register for channel PHA */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineMCInternalOffset[PRV_CPSS_DXCH_AAC_CHANNEL_DLB_AND_PHA_E];
        /* Set control pipe offset per tile */
        rc = prvCpssHwPpWriteRegister(devNum, regAddr, FALCON_CONTROL_PIPE_OFFSET_CNS);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Set AAC multicast external loop number of iterations as number of tiles */
        U32_SET_FIELD_MAC(regVal, 0, 10, PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);

        /* Set AAC multicast internal loop number of iterations as number of control pipes per tile */
        U32_SET_FIELD_MAC(regVal, 10, 10, PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile);

        /* Get AAC Engine <<%n>> MC Control register address */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineMCControl[PRV_CPSS_DXCH_AAC_CHANNEL_DLB_AND_PHA_E];
        /* Write the above configuration in AAC Engine <<%n>> MC Control register */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 20, regVal);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal firmwareAacWriteSet function
* @endinternal
*
* @brief   Copy firmware data using AAC HW method. Write it in 32 words block.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum             - physical device number
* @param[in] srcDataArr[]       - array of words of firmware data
* @param[in] dstRegAddr         - destination register address
* @param[in] totalNumberOfWords - number of words to copy
*
* @retval GT_OK                 - on success
* @retval GT_FAIL               - on other error.
* @retval GT_HW_ERROR           - on hardware error
*/
static GT_STATUS firmwareAacWriteSet
(
    IN GT_U8           devNum,
    IN const GT_U32    srcDataArr[],
    IN GT_U32          dstRegAddr,
    IN GT_U32          totalNumberOfWords
)
{
    GT_STATUS    rc;                   /* holds return code status                             */
    GT_U32       regAddr;              /* holds register address                               */
    GT_U32       regAddrAacEnginData;  /* holds AAC engine data register address               */
    GT_32        residualCommands;     /* holds number of left over words (less than 32 words) */
    GT_U32       i;                    /* index for outer loop                                 */
    GT_U32       j;                    /* index for inner loop                                 */

    /* Clear index */
    i = 0;

    /* Get AAC engine data register address for AAC PHA Channel */
    regAddrAacEnginData = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineData[PRV_CPSS_DXCH_AAC_CHANNEL_DLB_AND_PHA_E];

    /* Configure the AAC to copy 32 words per iteration as long as we don't cross the maximum number */
    while ( (i+32) <= totalNumberOfWords)
    {
        /* Check that previous AAC access done by busy-wait polling before trigger next access. */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEnginesStatus;
        rc = prvCpssPortGroupBusyWait(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, PRV_CPSS_DXCH_AAC_CHANNEL_DLB_AND_PHA_E, GT_FALSE/*busyWait*/);
        if(rc != GT_OK)
        {
            return rc;
        }
        /* AAC channel is free to be used therefore prepare the next 32 words */
        for (j=0;j<32;j++)
        {
            /* Write each word to AAC engine data register */
            rc = prvCpssDrvHwPpWriteRegister(devNum, regAddrAacEnginData, srcDataArr[i+j]);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        /* Get address register for AAC PHA Channel - AAC Trigger */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineAddress[PRV_CPSS_DXCH_AAC_CHANNEL_DLB_AND_PHA_E];

        /* Write the instruction memory address which is the address that the 32 words will be written to
           - address will be incremented automatically by 4 bytes
           - writing the adderss is also used as the trgger to the AAC to start the copy
         */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, dstRegAddr + i*4);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Update index for next 32 words */
        i +=32;
    }

    /* Handle left over commands. Should cover cases where less than 32 words are left to copy */
    residualCommands = totalNumberOfWords-i;
    if (residualCommands > 0)
    {
        /* Check that previous AAC access done by busy-wait polling before trigger next access. */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEnginesStatus;
        rc = prvCpssPortGroupBusyWait(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr,PRV_CPSS_DXCH_AAC_CHANNEL_DLB_AND_PHA_E, GT_FALSE/*busyWait*/);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* AAC channel is free to be used therefore prepare the next left over words */
        for (j=0;j<(GT_U32)residualCommands;j++)
        {
            /* Write each word to AAC engine data register */
            rc = prvCpssDrvHwPpWriteRegister(devNum, regAddrAacEnginData, srcDataArr[i+j]);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        /* Get address register for AAC PHA Channel - AAC Trigger */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineAddress[PRV_CPSS_DXCH_AAC_CHANNEL_DLB_AND_PHA_E];

        /* Write the instruction memory address which is the address that the left over words will be written to
           - address will be incremented automatically by 4 bytes
           - writing the adderss is also used as the trgger to the AAC to start the copy
         */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, dstRegAddr + i*4);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal firmwareImageInfoTablesGet function
* @endinternal
*
* @brief  Gather all firmware image information tables for all supported devices and image types.
*         Get pointers to specific firmware tables per firmware version and image ID.
*         These tables include the following:
*         - firmware code
*         - firmware thread's addresses
*         - firmware accelerators commands
*         - firmware version information
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in]  devNum                - the device number
* @param[in]  fwImageId             - firmware image ID
* @param[in]  fwVerNumber           - firmware version number
* @param[out] fwImageInfoTablesPtr  - pointer to fw image information tables
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid firmware version number
* @retval GT_BAD_PTR               - fwImageInfoTablesPtr or fwImageInfoArr[index] is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS firmwareImageInfoTablesGet
(
    IN    GT_U8                        devNum,
    IN    GT_U32                       fwImageId,
    IN    GT_U32                       fwVerNumber,
    OUT   PHA_FW_IMAGE_INFO_TABLES_STC *fwImageInfoTablesPtr
)
{
    GT_STATUS rc = GT_OK;                              /* return code status */
    GT_U32 fwImemSizeArr[FW_VERSIONS_MAX_NUM_CNS];     /* array of firmware code size per version */
    GT_U32 *fwImemArr[FW_VERSIONS_MAX_NUM_CNS];        /* array of pointers to firmware code table per fw version */
    GT_U32 *fwThreadAddArr[FW_VERSIONS_MAX_NUM_CNS];   /* array of pointers to firmware thread's addresses table per fw version */
    GT_U32 *fwAcclCmdArr[FW_VERSIONS_MAX_NUM_CNS];     /* array of pointers to firmware accelerator commands table per fw version */
    GT_U32 index = 0;                                  /* holds index to the requested entry in the above tables */
    GT_U32 i;                                          /* outer loop index for firmware image ID */
    GT_U32 j;                                          /* inner loop index for firmware version number */
    GT_BOOL found = GT_FALSE;                          /* flag to indicate if the requested firmware version was found */
    PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC  *fwImageInfoArr[FW_VERSIONS_MAX_NUM_CNS]; /* array of pointers to firmware image information structure per firmware version */

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(fwImageInfoTablesPtr);

    /* Clear all arrays */
    cpssOsMemSet(fwImemSizeArr,0,sizeof(fwImemSizeArr));
    cpssOsMemSet(fwImemArr,0,sizeof(GT_U32*)*FW_VERSIONS_MAX_NUM_CNS);
    cpssOsMemSet(fwThreadAddArr,0,sizeof(GT_U32*)*FW_VERSIONS_MAX_NUM_CNS);
    cpssOsMemSet(fwAcclCmdArr,0,sizeof(GT_U32*)*FW_VERSIONS_MAX_NUM_CNS);
    cpssOsMemSet(fwImageInfoArr,0,sizeof(PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)*FW_VERSIONS_MAX_NUM_CNS);

    /* Gather all possible fw versions */
    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:

            /** Get pointers to fw image version information structures */
            /* For Default image */
            fwImageInfoArr[0] = (PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)&fwImageVersionFalconDefault;
            fwImageInfoArr[1] = (PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)&fwImageVersionFalconDefault;
            fwImageInfoArr[2] = (PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)&fwImageVersionFalconDefault;
            /* For Image01 */
            fwImageInfoArr[3] = (PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)&fwImageVersionFalconImage01;
            fwImageInfoArr[4] = (PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)&fwImageVersionFalconImage01;
            fwImageInfoArr[5] = (PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)&fwImageVersionFalconImage01;
            /* For Image02 */
            fwImageInfoArr[6] = (PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)&fwImageVersionFalconImage02;
            fwImageInfoArr[7] = (PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)&fwImageVersionFalconImage02Previous;
            fwImageInfoArr[8] = (PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)&fwImageVersionFalconImage02Oldest;

            /** Get pointers to fw code tables */
            /* For Default image */
            fwImemArr[0] = (GT_U32*)prvCpssDxChPpaFwImemFalconDefault;
            fwImemSizeArr[0] = sizeof(prvCpssDxChPpaFwImemFalconDefault)/sizeof(prvCpssDxChPpaFwImemFalconDefault[0]);
            fwImemArr[1] = (GT_U32*)prvCpssDxChPpaFwImemFalconDefault;
            fwImemSizeArr[1] = sizeof(prvCpssDxChPpaFwImemFalconDefault)/sizeof(prvCpssDxChPpaFwImemFalconDefault[0]);
            fwImemArr[2] = (GT_U32*)prvCpssDxChPpaFwImemFalconDefault;
            fwImemSizeArr[2] = sizeof(prvCpssDxChPpaFwImemFalconDefault)/sizeof(prvCpssDxChPpaFwImemFalconDefault[0]);
            /* For Image01 */
            fwImemArr[3] = (GT_U32*)prvCpssDxChPpaFwImemFalconImage01;
            fwImemSizeArr[3] = sizeof(prvCpssDxChPpaFwImemFalconImage01)/sizeof(prvCpssDxChPpaFwImemFalconImage01[0]);
            fwImemArr[4] = (GT_U32*)prvCpssDxChPpaFwImemFalconImage01;
            fwImemSizeArr[4] = sizeof(prvCpssDxChPpaFwImemFalconImage01)/sizeof(prvCpssDxChPpaFwImemFalconImage01[0]);
            fwImemArr[5] = (GT_U32*)prvCpssDxChPpaFwImemFalconImage01;
            fwImemSizeArr[5] = sizeof(prvCpssDxChPpaFwImemFalconImage01)/sizeof(prvCpssDxChPpaFwImemFalconImage01[0]);
            /* For Image02 */
            fwImemArr[6] = (GT_U32*)prvCpssDxChPpaFwImemFalconImage02;
            fwImemSizeArr[6] = sizeof(prvCpssDxChPpaFwImemFalconImage02)/sizeof(prvCpssDxChPpaFwImemFalconImage02[0]);
            fwImemArr[7] = (GT_U32*)prvCpssDxChPpaFwImemFalconImage02Previous;
            fwImemSizeArr[7] = sizeof(prvCpssDxChPpaFwImemFalconImage02Previous)/sizeof(prvCpssDxChPpaFwImemFalconImage02Previous[0]);
            fwImemArr[8] = (GT_U32*)prvCpssDxChPpaFwImemFalconImage02Oldest;
            fwImemSizeArr[8] = sizeof(prvCpssDxChPpaFwImemFalconImage02Oldest)/sizeof(prvCpssDxChPpaFwImemFalconImage02Oldest[0]);

            /** Get pointers to threads' fw addresses tables */
            /* For Default image */
            fwThreadAddArr[0] = (GT_U32*)phaFwThreadsPcAddressFalconDefault;
            fwThreadAddArr[1] = (GT_U32*)phaFwThreadsPcAddressFalconDefault;
            fwThreadAddArr[2] = (GT_U32*)phaFwThreadsPcAddressFalconDefault;
            /* For Image01 */
            fwThreadAddArr[3] = (GT_U32*)phaFwThreadsPcAddressFalconImage01;
            fwThreadAddArr[4] = (GT_U32*)phaFwThreadsPcAddressFalconImage01;
            fwThreadAddArr[5] = (GT_U32*)phaFwThreadsPcAddressFalconImage01;
            /* For Image02 */
            fwThreadAddArr[6] = (GT_U32*)phaFwThreadsPcAddressFalconImage02;
            fwThreadAddArr[7] = (GT_U32*)phaFwThreadsPcAddressFalconImage02Previous;
            fwThreadAddArr[8] = (GT_U32*)phaFwThreadsPcAddressFalconImage02Oldest;

            /** Get pointers to fw accelerator commands tables */
            /* For Default image */
            fwAcclCmdArr[0] = (GT_U32*)phaFwAccelCmdsFalconDefault;
            fwAcclCmdArr[1] = (GT_U32*)phaFwAccelCmdsFalconDefault;
            fwAcclCmdArr[2] = (GT_U32*)phaFwAccelCmdsFalconDefault;
            /* For Image01 */
            fwAcclCmdArr[3] = (GT_U32*)phaFwAccelCmdsFalconImage01;
            fwAcclCmdArr[4] = (GT_U32*)phaFwAccelCmdsFalconImage01;
            fwAcclCmdArr[5] = (GT_U32*)phaFwAccelCmdsFalconImage01;
            /* For Image02 */
            fwAcclCmdArr[6] = (GT_U32*)phaFwAccelCmdsFalconImage02;
            fwAcclCmdArr[7] = (GT_U32*)phaFwAccelCmdsFalconImage02Previous;
            fwAcclCmdArr[8] = (GT_U32*)phaFwAccelCmdsFalconImage02Oldest;
            break;

        case FW_CASE_LIKE_AC5P:

            /** Get pointers to fw image version information structures */
            /* For Default image */
            fwImageInfoArr[0] = (PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)&fwImageVersionAc5pDefault;
            fwImageInfoArr[1] = (PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)&fwImageVersionAc5pDefault;
            fwImageInfoArr[2] = (PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)&fwImageVersionAc5pDefault;
            /* For Image01 */
            fwImageInfoArr[3] = (PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)&fwImageVersionAc5pImage01;
            fwImageInfoArr[4] = (PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)&fwImageVersionAc5pImage01;
            fwImageInfoArr[5] = (PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC*)&fwImageVersionAc5pImage01;

            /** Get pointers to fw code tables */
            /* For Default image */
            fwImemArr[0] = (GT_U32*)prvCpssDxChPpaFwImemAc5pDefault;
            fwImemSizeArr[0] = sizeof(prvCpssDxChPpaFwImemAc5pDefault)/sizeof(prvCpssDxChPpaFwImemAc5pDefault[0]);
            fwImemArr[1] = (GT_U32*)prvCpssDxChPpaFwImemAc5pDefault;
            fwImemSizeArr[1] = sizeof(prvCpssDxChPpaFwImemAc5pDefault)/sizeof(prvCpssDxChPpaFwImemAc5pDefault[0]);
            fwImemArr[2] = (GT_U32*)prvCpssDxChPpaFwImemAc5pDefault;
            fwImemSizeArr[2] = sizeof(prvCpssDxChPpaFwImemAc5pDefault)/sizeof(prvCpssDxChPpaFwImemAc5pDefault[0]);
            /* For Image01 */
            fwImemArr[3] = (GT_U32*)prvCpssDxChPpaFwImemAc5pImage01;
            fwImemSizeArr[3] = sizeof(prvCpssDxChPpaFwImemAc5pImage01)/sizeof(prvCpssDxChPpaFwImemAc5pImage01[0]);
            fwImemArr[4] = (GT_U32*)prvCpssDxChPpaFwImemAc5pImage01;
            fwImemSizeArr[4] = sizeof(prvCpssDxChPpaFwImemAc5pImage01)/sizeof(prvCpssDxChPpaFwImemAc5pImage01[0]);
            fwImemArr[5] = (GT_U32*)prvCpssDxChPpaFwImemAc5pImage01;
            fwImemSizeArr[5] = sizeof(prvCpssDxChPpaFwImemAc5pImage01)/sizeof(prvCpssDxChPpaFwImemAc5pImage01[0]);

            /** Get pointers to threads' fw addresses tables */
            /* For Default image */
            fwThreadAddArr[0] = (GT_U32*)phaFwThreadsPcAddressAc5pDefault;
            fwThreadAddArr[1] = (GT_U32*)phaFwThreadsPcAddressAc5pDefault;
            fwThreadAddArr[2] = (GT_U32*)phaFwThreadsPcAddressAc5pDefault;
            /* For Image01 */
            fwThreadAddArr[3] = (GT_U32*)phaFwThreadsPcAddressAc5pImage01;
            fwThreadAddArr[4] = (GT_U32*)phaFwThreadsPcAddressAc5pImage01;
            fwThreadAddArr[5] = (GT_U32*)phaFwThreadsPcAddressAc5pImage01;

            /** Get pointers to fw accelerator commands tables */
            /* For Default image */
            fwAcclCmdArr[0] = (GT_U32*)phaFwAccelCmdsAc5pDefault;
            fwAcclCmdArr[1] = (GT_U32*)phaFwAccelCmdsAc5pDefault;
            fwAcclCmdArr[2] = (GT_U32*)phaFwAccelCmdsAc5pDefault;
            /* For Image01 */
            fwAcclCmdArr[3] = (GT_U32*)phaFwAccelCmdsAc5pImage01;
            fwAcclCmdArr[4] = (GT_U32*)phaFwAccelCmdsAc5pImage01;
            fwAcclCmdArr[5] = (GT_U32*)phaFwAccelCmdsAc5pImage01;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "not supported device family[%d]", PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    /** Find the index which points to the requested fw version */
    /* Outer loop to scan PHA fw image ID */
    for(j=0;j<CPSS_DXCH_PHA_FW_IMAGE_ID_LAST_E;j++)
    {
        /* Check if this is the fw image ID that is requested */
        if(j == fwImageId)
        {
            /* Inner loop to scan fw versions per image ID */
            for(i=0;i<CPSS_DXCH_PHA_FW_NUM_OF_SUPPORTED_VERSIONS_PER_IMAGE_CNS;i++)
            {
                /* Set index to table */
                index = j*CPSS_DXCH_PHA_FW_NUM_OF_SUPPORTED_VERSIONS_PER_IMAGE_CNS + i;

                /* Verify pointer is not NULL */
                if (fwImageInfoArr[index] == NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, "fwImageInfoArr[%d] contains NULL pointer", index);
                }

                /* Check if the requested fw version exists */
                if (fwImageInfoArr[index]->imageVersion == fwVerNumber)
                {
                    /* Indicate fw version was found */
                    found = GT_TRUE;
                    /* Break the inner loop */
                    break;
                }
            }

            /* Check if fw version was found for this image */
            if(found == GT_TRUE)
            {
                /* Version was found therefore no need to search anymore. Break the outer loop */
                break;
            }
        }
    }

    /* Failure indication in case fw version was not found */
    if(found == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid fw version. Does not exist [%x]", fwVerNumber);
    }

    /* Update output parameters */
    fwImageInfoTablesPtr->fwImemPtr       = fwImemArr[index];
    fwImageInfoTablesPtr->fwImemSize      = fwImemSizeArr[index];
    fwImageInfoTablesPtr->fwThreadAddrPtr = fwThreadAddArr[index];
    fwImageInfoTablesPtr->fwAcclCmdPtr    = fwAcclCmdArr[index];
    fwImageInfoTablesPtr->fwImageInfoPtr  = fwImageInfoArr[index];
    fwImageInfoTablesPtr->fwVersIndex     = i;

    return rc;
}

/**
* @internal firmwareVersionsArrayGet function
* @endinternal
*
* @brief  Get all fw versions for all supported devices and image types.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in]    devNum             - the device number
* @param[out] fwVerArr             - (array of) fw versions of all supported devices and images
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - fwVerArr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS firmwareVersionsArrayGet
(
    IN    GT_U8   devNum,
    OUT  GT_U32   fwVerArr[]
)
{
    GT_STATUS rc = GT_OK;     /* return code status */

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(fwVerArr);

    /* Gather all possible fw versions */
    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:

            /* Get fw versions for Default image */
            fwVerArr[0] = fwImageVersionFalconDefault.imageVersion;
            fwVerArr[1] = fwImageVersionFalconDefault.imageVersion;
            fwVerArr[2] = fwImageVersionFalconDefault.imageVersion;

            /* Get fw versions for Image01 image */
            fwVerArr[3] = fwImageVersionFalconImage01.imageVersion;
            fwVerArr[4] = fwImageVersionFalconImage01.imageVersion;
            fwVerArr[5] = fwImageVersionFalconImage01.imageVersion;

            /* Get fw versions for Image02 image */
            fwVerArr[6] = fwImageVersionFalconImage02.imageVersion;
            fwVerArr[7] = fwImageVersionFalconImage02Previous.imageVersion;
            fwVerArr[8] = fwImageVersionFalconImage02Oldest.imageVersion;
            break;

        case FW_CASE_LIKE_AC5P:

            /* Get fw versions for Default image */
            fwVerArr[0] = fwImageVersionAc5pDefault.imageVersion;
            fwVerArr[1] = fwImageVersionAc5pDefault.imageVersion;
            fwVerArr[2] = fwImageVersionAc5pDefault.imageVersion;

            /* Get fw versions for Image01 image */
            fwVerArr[3] = fwImageVersionAc5pImage01.imageVersion;
            fwVerArr[4] = fwImageVersionAc5pImage01.imageVersion;
            fwVerArr[5] = fwImageVersionAc5pImage01.imageVersion;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "not supported device family[%d]", PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

    return rc;
}

/**
* @internal ASIC_SIMULATION_firmwareInit function
* @endinternal
*
* @brief   ASIC_SIMULATION : init the ASIC_SIMULATION for FW addresses.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - on unknown threadId
*/
GT_STATUS ASIC_SIMULATION_firmwareInit
(
    IN GT_U8    devNum,
    IN GT_U32   phaFwVersionId
)
{
    GT_U32  threadId;
    GT_STATUS   rc;
    CPSS_DXCH_PHA_FW_IMAGE_ID_ENT phaFwImageId = PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.phaFwImageId;
    GT_BOOL  firstTime = GT_TRUE;

    /* Update threads addresses in simulation taken from CPSS PHA threads information table */
    for(threadId = 0 ; threadId < PRV_CPSS_DXCH_PHA_MAX_THREADS_CNS; threadId++)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[threadId].firmwareInstructionPointer == PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS)
        {
            continue;
        }

        switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_FALCON_E:

                rc = cpssSimFalconFirmwareThreadsInfoSet(devNum,threadId,
                                                         PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[threadId].firmwareInstructionPointer,
                                                         phaFwImageId,
                                                         phaFwVersionId,
                                                         firstTime);
                firstTime = GT_FALSE;
                break;

            case FW_CASE_LIKE_AC5P:

                rc = cpssSimAc5pFirmwareThreadsInfoSet(devNum,threadId,
                                                       PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[threadId].firmwareInstructionPointer,
                                                       phaFwImageId,
                                                       phaFwVersionId,
                                                       firstTime);
                firstTime = GT_FALSE;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "not supported device family[%d]", PRV_CPSS_PP_MAC(devNum)->devFamily);
        }
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ASIC_SIMULATION failed to bind threadId[%d] with InstructionPointer[0x%x]",
                threadId, PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[threadId].firmwareInstructionPointer);
        }
    }

    return GT_OK;
}


/**
* @internal firmwareThreadsSkipCounterSet function
* @endinternal
*
* @brief   Calculate skip counter value according to below formula.
*          skipCounter = ceil ( ( threadLatency / ( (ppaClkMhz / cpClkMhz) * numOfPpns ) ) -1 )
*          Since division result is not an integer value need to do fixed point arithmetic calculations
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - the device number
*
* @retval GT_OK                    - on success
*/
static GT_STATUS firmwareThreadsSkipCounterSet
(
    IN GT_U8    devNum
)
{
    GT_U32 numOfPpns;               /* Total number of ppns */
    GT_U32 skipCounter;             /* The final output result of Skip counter */
    GT_U32 skipCounterFullValQ7;    /* Skip counter value in Q7 representation include 7 bits for fraction part */
    GT_U32 skipFactorQ16;           /* Skip counter factor ((ppaClkMhz/cpClkMhz)*numOfPpns)) in Q16 representation */
    GT_U32 skipCounterIntegerPart;  /* Integer part of skip counter value */
    GT_U32 skipCounterFractionPart; /* Fraction part of skip counter value */
    GT_U32 cpClkMhz;                /* CP clock in MHZ units */
    GT_U32 ppaClkMhz;               /* PPA clock in MHZ units */
    GT_U32 threadLatency;           /* Thread's core proccessing cycles number */
    GT_U32 i;                       /* Are used to scan all threads */

    /* Get device CP clock (in mhz) */
    cpClkMhz = PRV_CPSS_PP_MAC(devNum)->coreClock;

    /* Get PPA clock (in mhz) */
    ppaClkMhz = 1700;

    /* Get total number of PPNs. (number of PPGs * number of PPNs per group) */
    numOfPpns = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg * PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpn);

    /* Calculate skip counter for all threads */
    for(i=0;i<PRV_CPSS_DXCH_PHA_MAX_THREADS_CNS;i++)
    {
        /* Get thread's latency (proccessing cycles) */
        threadLatency = phaFwThreadsInitInformation[i].latency;

        /* Calculate (ppaClkMhz/cpClkMhz) * numOfPpns
           Since division result most probably is not an integer need to do fixed point arithmetic calculations
           For that we will represent the numbers as follows:
           - ppaClkMhz in Q16 (16 bits represent the fractional part)
           - cpClkMhz as Q0
           - numOfPpns as Q0
           Therefore the division result should be represented as Q16
           Multiply it by numOfPpns should provide the skip counter factor represented as Q16 with integer and fraction parts
         */
        skipFactorQ16 = ( (ppaClkMhz << 16) / cpClkMhz ) * numOfPpns;

        /* Next step is to divide thread latency with skip factor:  threadLatency / skipFactor
           Since we need to divide threadLatency with fixed point value we need to represent it as fixed point number as well.
           Therefore represent threadLatency as Q7 since we assume the value shouldn't be bigger than 9 bits (max of 512 cycles).
           Shift skipFactor by 9 to convert it from Q16 to Q7.
           For the division calculation change the numerator from Q7 to Q14 therefore shift it by total of 14 bits
           the result (skipCounterFullVal) should be represented as Q7 and should hold integer and fractional parts.
        */
        skipCounterFullValQ7 = ( threadLatency << 14 ) / (skipFactorQ16 >> 9);

        /* Take the integer part. Since skipCounterFullVal is represented in Q7 get rid of the 7 LS bits */
        skipCounterIntegerPart = skipCounterFullValQ7 >> 7 ;

        /* According to PHA designer need to reduce the number by 1. For 0 values skip counter should be set to 0. */
        if(skipCounterIntegerPart == 0)
        {
            skipCounter = 0;
        }
        else
        {
            /* Reduce by 1 */
            skipCounter = skipCounterIntegerPart - 1;

            /* Take the fraction part. Since skipCounterFullValQ7 is represented in Q7 need to extract the 7 LS bits. */
            skipCounterFractionPart = skipCounterFullValQ7 & 0x7F ;

            /* According to PHA designer need to check the fraction part and round it up if bigger than 0.5 (0x40 in Q7) */
            if(skipCounterFractionPart > 0x40)
            {
                skipCounter +=1;
            }
        }
        /* Set skip counter value in pha information table per device */
        PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[i].skipCounter = skipCounter;
    }

    return GT_OK;
}


/**
* @internal firmwareThreadTypeToThreadId function
* @endinternal
*
* @brief   convert firmware thread type to firmware thread ID
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] threadType - type of the firmware thread that should handle the egress packet.
*
* @param[out] firmwareThreadIdPtr      - (pointer to) the firmware Thread Id (0..255)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid threadType
*/
static GT_STATUS firmwareThreadTypeToThreadId(
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    threadType,
    OUT GT_U32   *firmwareThreadIdPtr
)
{
    /* Get the compatible fw thread ID */
    switch(threadType)
    {
        case CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E:
            *firmwareThreadIdPtr = THR_ID_0;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_E:
            *firmwareThreadIdPtr = THR_ID_1;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_SEGMENT_E:
            *firmwareThreadIdPtr = THR_ID_2;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_SEGMENTS_E:
            *firmwareThreadIdPtr = THR_ID_3;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_SEGMENTS_E:
            *firmwareThreadIdPtr = THR_ID_4;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_SEGMENTS_E:
            *firmwareThreadIdPtr = THR_ID_5;
            break;
        /* Centralized Chassis ERSPAN mirror without loopback port */
        /* Ingress/Egress */
        case CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV4_E:
            *firmwareThreadIdPtr = THR_ID_6;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV6_E:
            *firmwareThreadIdPtr = THR_ID_7;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV4_E:
            *firmwareThreadIdPtr = THR_ID_8;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV6_E:
            *firmwareThreadIdPtr = THR_ID_9;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_SRC_DEV_E:
            *firmwareThreadIdPtr = THR_ID_10;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_E:
            *firmwareThreadIdPtr = THR_ID_11;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_NO_EL_E:
            *firmwareThreadIdPtr = THR_ID_12;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_ONE_EL_E:
            *firmwareThreadIdPtr = THR_ID_13;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_TWO_EL_E:
            *firmwareThreadIdPtr = THR_ID_14;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_THREE_EL_E:
            *firmwareThreadIdPtr = THR_ID_15;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_ADD_MSB_E:
            *firmwareThreadIdPtr = THR_ID_16;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_FIX_E:
            *firmwareThreadIdPtr = THR_ID_17;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_REMOVE_E:
            *firmwareThreadIdPtr = THR_ID_18;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_FIX_E:
            *firmwareThreadIdPtr = THR_ID_19;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_REMOVE_E:
            *firmwareThreadIdPtr = THR_ID_20;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV4_E:
            *firmwareThreadIdPtr = THR_ID_21;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV6_E:
            *firmwareThreadIdPtr = THR_ID_22;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV4_E:
            *firmwareThreadIdPtr = THR_ID_23;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV6_E:
            *firmwareThreadIdPtr = THR_ID_24;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_PTP_PHY_1_STEP_E:
            *firmwareThreadIdPtr = THR_ID_25;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_PENULTIMATE_END_NODE_E:
            *firmwareThreadIdPtr = THR_ID_26;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E:
            *firmwareThreadIdPtr = THR_ID_31;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E:
            *firmwareThreadIdPtr = THR_ID_32;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E:
            *firmwareThreadIdPtr = THR_ID_33;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E:
            *firmwareThreadIdPtr = THR_ID_34;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_EGRESS_SWITCH_IPV6_E:
            *firmwareThreadIdPtr = THR_ID_35;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_MIRRORING_E:
            *firmwareThreadIdPtr = THR_ID_36;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_EGRESS_SWITCH_E:
            *firmwareThreadIdPtr = THR_ID_37;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_UNIFIED_SR_E:
            *firmwareThreadIdPtr = THR_ID_38;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_ETHERNET_E:
            *firmwareThreadIdPtr = THR_ID_39;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_E:
            *firmwareThreadIdPtr = THR_ID_40;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_VXLAN_GPE_TO_ETHERNET_E:
            *firmwareThreadIdPtr = THR_ID_41;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_ETHERNET_TO_VXLAN_GPE_E:
            *firmwareThreadIdPtr = THR_ID_42;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_EGRESS_MIRRORING_METADATA_E:
            *firmwareThreadIdPtr = THR_ID_43;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_BEST_EFFORT_E:
            *firmwareThreadIdPtr = THR_ID_48;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E:
            *firmwareThreadIdPtr = THR_ID_49;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E:
            *firmwareThreadIdPtr = THR_ID_50;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E:
            *firmwareThreadIdPtr = THR_ID_51;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E:
            *firmwareThreadIdPtr = THR_ID_52;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_COC32_GSID_E:
            *firmwareThreadIdPtr = THR_ID_53;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IPV4_TTL_INCREMENT_E:
            *firmwareThreadIdPtr = THR_ID_54;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IPV6_HOP_LIMIT_INCREMENT_E:
            *firmwareThreadIdPtr = THR_ID_55;
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E:
            *firmwareThreadIdPtr = THR_ID_56;
            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV4_E:
            *firmwareThreadIdPtr = THR_ID_57;
            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV6_E:
            *firmwareThreadIdPtr = THR_ID_58;
            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_SLS_E:
            *firmwareThreadIdPtr = THR_ID_59;
            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_DROP_ALL_TRAFFIC_E:
            *firmwareThreadIdPtr = THR_ID_60;
            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_II_SAME_DEV_IPV4_E:
            *firmwareThreadIdPtr = THR_ID_64;
            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_II_SAME_DEV_IPV6_E:
            *firmwareThreadIdPtr = THR_ID_65;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(threadType);
    }

    return GT_OK;
}

/**
* @internal firmwareAddrValidityCheck function
* @endinternal
*
* @brief   Check Firmware addresses.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - the device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on threadId that hold bad address.
*/
static GT_STATUS firmwareAddrValidityCheck
(
    IN GT_U8    devNum
)
{
    GT_U32  threadId;
    GT_U32  addr;

    for(threadId = 0 ; threadId < PRV_CPSS_DXCH_PHA_MAX_THREADS_CNS; threadId++)
    {
        /* Get PHA firmware thread address */
        addr = PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[threadId].firmwareInstructionPointer;

        if(addr == PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS)
        {
            continue;
        }

        if((addr & 0xFFFF0000) != FW_INSTRUCTION_DOMAIN_ADDR_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "firmware address for thread [%d] must be in [0x%x..0x%x]\n"
                "but got[0x%x] --> need to check PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[]",
                threadId ,
                FW_INSTRUCTION_DOMAIN_ADDR_CNS,
                FW_INSTRUCTION_DOMAIN_ADDR_MAX_CNS,
                addr
                );
        }
    }

    return GT_OK;
}


/**
* @internal firmwareImemDownload function
* @endinternal
*
* @brief  Download PHA firmware code into instruction memory of PHA unit in the device.
*         Need to download to IMEM of ALL PPGs (4 PPGs)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   -  the device number
* @param[in] firmwareDataArr[]        - array of words of firmware data.
* @param[in] firmwareDataNumWords     - number of words in firmwareDataArr[].
*                                      must be multiple of 4.
*                                      meaning (firmwareDataNumWords % 4) must be 0
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad firmwareDataNumWords
*/
static GT_STATUS firmwareImemDownload
(
    IN GT_U8            devNum,
    IN const GT_U32     firmwareDataArr[],
    IN GT_U32           firmwareDataNumWords
)
{
    GT_STATUS   rc;
    GT_U32  ppg;
    GT_U32  regAddr;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {

        if(firmwareDataNumWords % 4)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "firmware data number of words must me multiple of 4 but got [%d] words \n",
                firmwareDataNumWords);
        }

        if(firmwareDataNumWords > _4K)/* size of IMEM in words */
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "firmware data number of words is [%d] but must NOT be more than [%d] words \n",
                firmwareDataNumWords,
                _4K);
        }

        /* Initialize AAC for firmware usage */
        rc = firmwareAacInit(devNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        for(ppg = 0 ; ppg < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg; ppg++)
        {
            /* Get Instruction memory address per PPG */
            regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPG_IMEM_base_addr;

            /* Per PPG copy firmware code into Instruction memory using AAC HW */
            rc = firmwareAacWriteSet(devNum,firmwareDataArr,regAddr,firmwareDataNumWords);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /* set field <ppa_imem_hold_off> to :
            0x0 = No HoldOff; IMEM responds with data
        */
        regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).PPA.ppa_regs.PPACtrl;
        rc = prvCpssHwPpSetRegField(devNum,regAddr,0,1,
            0);/*0x0 = No HoldOff; IMEM responds with data*/
        if (rc != GT_OK)
        {
            return rc;
        }

    }

    return GT_OK;
}


/**
* @internal firmwareDmemDownload function
* @endinternal
*
* @brief  Download PHA firmware data such as fw version information and accelerator commands
*         into PHA fw Scratchpad memory in the device.
*         Do it for all PPGs and all ppns per ppg.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] firmwareAccelCmdsArr[]   - array of fw accelerator commands.
* @param[in] fwVersionPtr             - (pointer to) fw version information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS firmwareDmemDownload
(
    IN GT_U8                                          devNum,
    IN const GT_U32                                   firmwareAccelCmdsArr[],
    IN PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC    *fwVersionPtr
)
{
    GT_STATUS   rc;                                       /* Return Code status */
    GT_U32      ppg;                                      /* Holds number of packet processor groups */
    GT_U32      ppn;                                      /* Holds number of packet processor nodes */
    GT_U32      regAddr;                                  /* Holds HW register address */
    GT_U32      hwValue;                                  /* Holds field value from HW register */
    GT_U32      fwVersionOfst = 0x4;                      /* Offset to fw version location in data memory */
    GT_U32      fwImageIdOfst = 0x8;                      /* Offset to fw image ID location in data memory */
    GT_U32      fwAccelCmdsOfst = 0x40;                   /* Offset to fw accelerator commands location in data memory */
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;/* Holds information regarding system recovery process */

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Do it if there is no active system recovery process */
    if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        /* Read field <ppa_imem_hold_off> from PPA ctrl register */
        regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).PPA.ppa_regs.PPACtrl;
        rc = prvCpssHwPpGetRegField(devNum,regAddr,0,1,&hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Skip the load if it is the first time (out of reset) since it is done by firmware init code */
        /* Check ppaCtrl.ppa_imem_hold_off bit. If it is still '1' then this is the first time */
        if (hwValue == 0)
        {
            /* Initialize AAC for firmware usage */
            rc = firmwareAacInit(devNum);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* Do it for all PPGs and PPNs */
            for(ppg = 0 ; ppg < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpg; ppg++) /* per PPG */
            {
                for(ppn = 0 ; ppn < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpn; ppn++) /* per PPN (PER PPG) */
                {
                    /* Get address of firmware version register which is the first address of data memory */
                    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPN[ppn].ppn_regs.fwDeviceName;

                    /* Update firmware version */
                    rc = prvCpssHwPpWriteRegister(devNum,regAddr+fwVersionOfst,fwVersionPtr->imageVersion);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    /* Update firmware image ID */
                    rc = prvCpssHwPpWriteRegister(devNum,regAddr+fwImageIdOfst,fwVersionPtr->imageId);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    /* Now point to start of acclerator commands domain */
                    regAddr +=fwAccelCmdsOfst;

                    /* Per PPN copy firmware accelerator commands into SP memory using AAC HW */
                    rc = firmwareAacWriteSet(devNum,firmwareAccelCmdsArr,regAddr,FW_ACCEL_CMDS_MAX_NUM_CNS);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
    }

    return GT_OK;
}


/**
* @internal firmwareInit function
* @endinternal
*
* @brief   init the PHA related firmware configurations
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   -  the device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - firmware address check fail. One of firmware threads holds bad address
*/
static GT_STATUS firmwareInit
(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc;
    GT_U32  regAddr;
    GT_U32  value;
    GT_U32  phaThreadId;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC    commonInfo;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType;
    CPSS_DXCH_PHA_THREAD_INFO_UNT    extInfo;
    GT_U32  *fwImemDataArrStart;
    GT_U32  fmImemDataArrSize;
    GT_U32  *fwThreadsPcAddressArrStart;
    GT_U32  *fwAccelCmdsArrStart;
    GT_U32  i;
    GT_U32  fwVersionId = 0 /* Most up to date fw version */;


    /* Get PHA firmware tables compatible to desired firmware image */
    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:

            if (PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.phaFwImageId == CPSS_DXCH_PHA_FW_IMAGE_ID_DEFAULT_E)
            {   /* Falcon default PHA firmware image */
                fwImemDataArrStart          = (GT_U32*)prvCpssDxChPpaFwImemFalconDefault;
                fmImemDataArrSize           = PRV_CPSS_DXCH_PPA_FW_IMAGE_SIZE_FALCON_DEFAULT;
                fwThreadsPcAddressArrStart  = (GT_U32*)phaFwThreadsPcAddressFalconDefault;
                PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.fwVersionInfo = fwImageVersionFalconDefault;
                fwAccelCmdsArrStart         = (GT_U32*)phaFwAccelCmdsFalconDefault;
            }
            else if (PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.phaFwImageId == CPSS_DXCH_PHA_FW_IMAGE_ID_01_E)
            {   /* Falcon PHA firmware image 01 */
                fwImemDataArrStart          = (GT_U32*)prvCpssDxChPpaFwImemFalconImage01;
                fmImemDataArrSize           = PRV_CPSS_DXCH_PPA_FW_IMAGE_SIZE_FALCON_IMAGE01;
                fwThreadsPcAddressArrStart  = (GT_U32*)phaFwThreadsPcAddressFalconImage01;
                PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.fwVersionInfo = fwImageVersionFalconImage01;
                fwAccelCmdsArrStart         = (GT_U32*)phaFwAccelCmdsFalconImage01;
            }
            else if (PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.phaFwImageId == CPSS_DXCH_PHA_FW_IMAGE_ID_02_E)
            {   /* Falcon PHA firmware image 02 */
                fwImemDataArrStart          = (GT_U32*)prvCpssDxChPpaFwImemFalconImage02;
                fmImemDataArrSize           = PRV_CPSS_DXCH_PPA_FW_IMAGE_SIZE_FALCON_IMAGE02;
                fwThreadsPcAddressArrStart  = (GT_U32*)phaFwThreadsPcAddressFalconImage02;
                PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.fwVersionInfo = fwImageVersionFalconImage02;
                fwAccelCmdsArrStart         = (GT_U32*)phaFwAccelCmdsFalconImage02;
            }
            else
            {
                /* Wrong PHA firmware image ID */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"PHA fw image ID[%d] is not supported", PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.phaFwImageId);
            }
            break;

        case FW_CASE_LIKE_AC5P:

            if (PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.phaFwImageId == CPSS_DXCH_PHA_FW_IMAGE_ID_DEFAULT_E)
            {   /* Ac5p default PHA firmware image */
                fwImemDataArrStart          = (GT_U32*)prvCpssDxChPpaFwImemAc5pDefault;
                fmImemDataArrSize           = PRV_CPSS_DXCH_PPA_FW_IMAGE_SIZE_AC5P_DEFAULT;
                fwThreadsPcAddressArrStart  = (GT_U32*)phaFwThreadsPcAddressAc5pDefault;
                PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.fwVersionInfo = fwImageVersionAc5pDefault;
                fwAccelCmdsArrStart         = (GT_U32*)phaFwAccelCmdsAc5pDefault;
            }
            else if (PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.phaFwImageId == CPSS_DXCH_PHA_FW_IMAGE_ID_01_E)
            {   /* Ac5p PHA firmware image 01 */
                fwImemDataArrStart          = (GT_U32*)prvCpssDxChPpaFwImemAc5pImage01;
                fmImemDataArrSize           = PRV_CPSS_DXCH_PPA_FW_IMAGE_SIZE_AC5P_IMAGE01;
                fwThreadsPcAddressArrStart  = (GT_U32*)phaFwThreadsPcAddressAc5pImage01;
                PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.fwVersionInfo = fwImageVersionAc5pImage01;
                fwAccelCmdsArrStart         = (GT_U32*)phaFwAccelCmdsAc5pImage01;
            }
            else
            {   /* Wrong PHA firmware image ID */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"PHA fw image ID[%d] is not supported", PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.phaFwImageId);
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "not supported device family[%d]", PRV_CPSS_PP_MAC(devNum)->devFamily);
    }

     /* Download firmware data such as accelerators and version info into data memory of PHA unit in the device */
     rc = firmwareDmemDownload(devNum, fwAccelCmdsArrStart, &(PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.fwVersionInfo));
     if(rc != GT_OK){
         return rc;
     }

     /* Download firmware code into instruction memory of PHA unit in the device */
     rc = firmwareImemDownload(devNum, fwImemDataArrStart, fmImemDataArrSize);
     if(rc != GT_OK){
         return rc;
     }

     /* Set firmware threads addresses */
     for (i=0;i<PRV_CPSS_DXCH_PHA_MAX_THREADS_CNS;i++)
     {
        PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[i].firmwareInstructionPointer = fwThreadsPcAddressArrStart[i];
     }

     /* Get address of 'Do nothing' thread */
     value = PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[0].firmwareInstructionPointer & (~FW_INSTRUCTION_DOMAIN_ADDR_CNS);

    /* Check firmware addresses */
    rc = firmwareAddrValidityCheck(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_PP_MAC(devNum)->isWmDevice)
    {
        /* Bind firmware addresses to simulation */
        rc = ASIC_SIMULATION_firmwareInit(devNum,fwVersionId);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* Set Null Processing Instruction Pointer with 'do nothing' thread 'instruction pointer' */
    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.PPAThreadConfigs.nullProcessingInstructionPointer;
    rc = prvCpssHwPpSetRegField(devNum,regAddr,0,16,value);
    if(rc != GT_OK)
    {
        return rc;
    }

    /****************************************************************/
    /* initialize all the PHA table entries to bind to 'do nothing' */
    /****************************************************************/
    {
        extType = CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E;
        extInfo.notNeeded = 0;/* dummy ... not used ... don't care */
        cpssOsMemSet(&commonInfo,0,sizeof(commonInfo));

        /* must set valid 'drop code' otherwise function will fail! */
        commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;


        for(phaThreadId  = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;
            phaThreadId <= PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC;
            phaThreadId++)
        {
            rc = cpssDxChPhaThreadIdEntrySet(
                devNum,phaThreadId,&commonInfo,extType,&extInfo);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChPhaFwVersionPrint function
* @endinternal
*
* @brief   debug function to print the PHA related firmware name,date and version
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   -  the device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS   prvCpssDxChPhaFwVersionPrint
(
    IN GT_U8    devNum
)
{
    GT_STATUS   rc;
    GT_U32  regAddr;
    GT_U32  value1,value2, value3;
    GT_U32  ppg,ppn;
    char    projName[5];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    ppg = 0;
    ppn = 0;

    /* Get fw device name */
    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPN[ppn].ppn_regs.fwDeviceName;
    rc = prvCpssHwPpReadRegister(devNum,regAddr,&value1);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get fw version name */
    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPN[ppn].ppn_regs.fwVersion;
    rc = prvCpssHwPpReadRegister(devNum,regAddr,&value2);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get fw image ID */
    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPN[ppn].ppn_regs.fwImageId;
    rc = prvCpssHwPpReadRegister(devNum,regAddr,&value3);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(value1 == 0x000badad || value2 == 0x000badad || value3 == 0x000badad ||
       value1 == 0          || value2 == 0 )
    {
        /*****************************************/
        /* 0x00badadd -->                        */
        /* the GM not supports those registers ! */
        /* and the FW is not loaded in it        */
        /*****************************************/
        cpssOsPrintf("The PHA FW for dev[%d] : was NOT loaded (value1=[0x%8.8x] , value2=[0x%8.8x] , value3=[0x%8.8x])!!! \n",
            devNum,value1,value2,value3);

        /* do not fail the INIT */
        return GT_OK;
    }

    /* Prepare fw device name for print as a string */
    projName[0] = (char)(value1 >> 24);
    projName[1] = (char)(value1 >> 16);
    projName[2] = (char)(value1 >>  8);
    projName[3] = (char)(value1 >>  0);
    projName[4] = 0;


    cpssOsPrintf("The PHA FW for dev[%d]:[%s] , date[20%02x/%02x] version[%x] debug[%x] fw_image[id:%d,name:%s]\n"
        , devNum
        , projName
        ,(value2 >> 24) & 0xFF/* year     */
        ,(value2 >> 16) & 0xFF/* month    */
        ,(value2 >>  8) & 0xFF/* version  */
        ,(value2 >>  0) & 0xFF/* debug    */
        ,(value3 >> 24) & 0xFF/* image ID */
        , PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.fwVersionInfo.imageName
        );

    return GT_OK;
}


/**
* @internal firmwareInstructionPointerGet function
* @endinternal
*
* @brief   convert operationType to firmwareInstructionPointer and firmwareThreadId
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   -  the device number
* @param[in] operationType            -  the operation type
*
* @param[out] firmwareThreadInfoPtr      - (pointer to) the firmware Thread info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on wrong parameter
*/
static GT_STATUS firmwareInstructionPointerGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PHA_THREAD_TYPE_ENT         operationType,
    OUT PRV_CPSS_DXCH_PHA_FW_THREAD_INFO_STC  *firmwareThreadInfoPtr
)
{
    FW_THREAD_ID      firmwareThreadId = 0;
    GT_STATUS         rc;

    /* Check thread is valid (exist in current fw image) */
    rc = prvCpssDxChPhaThreadValidityCheck(devNum, operationType, &firmwareThreadId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Points to thread's entry */
    *firmwareThreadInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[firmwareThreadId];

    /* Convert the 'firmwareThreadId' to the actual firmware address in memory */
    firmwareThreadInfoPtr->firmwareInstructionPointer = (PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[firmwareThreadId].firmwareInstructionPointer) & (~FW_INSTRUCTION_DOMAIN_ADDR_CNS);
    return rc;
}


/**
* @internal firmwareInstructionPointerConvert function
* @endinternal
*
* @brief   convert firmwareInstructionPointer to firmwareThreadId
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                     - the device number
* @param[in] firmwareInstructionPointer - the firmware Instruction Pointer.
*
* @param[out] firmwareThreadIdPtr       - (pointer to) the firmware Thread Id (0..79)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on wrong parameter
*/
static GT_STATUS firmwareInstructionPointerConvert(
    IN  GT_U8          devNum,
    IN  GT_U32         firmwareInstructionPointer,
    OUT FW_THREAD_ID   *firmwareThreadIdPtr
)
{
    FW_THREAD_ID  firmwareThreadId;

    firmwareInstructionPointer |= FW_INSTRUCTION_DOMAIN_ADDR_CNS;

    /* Scan all entries in PHA thread information table, look for the desired PC address and get the compatible thread ID */
    for(firmwareThreadId = 0 ; firmwareThreadId < PRV_CPSS_DXCH_PHA_MAX_THREADS_CNS; firmwareThreadId++)
    {
        if(firmwareInstructionPointer == PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[firmwareThreadId].firmwareInstructionPointer)
        {
            *firmwareThreadIdPtr = firmwareThreadId;
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
        "firmwareInstructionPointer[0x%x] was not found as valid address for known firmware thread",
        firmwareInstructionPointer);
}


/**
* @internal prvCpssDxChPhaClockEnable function
* @endinternal
*
* @brief   enable/disable the clock to PPA in the PHA unit in the device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum   - device number.
* @param[in] enable   - Enable/Disable the clock to PPA in the PHA unit in the device.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPhaClockEnable
(
    IN  GT_U8   devNum,
    IN GT_BOOL  enable
)
{
    GT_U32  regAddr;
    GT_U32  value;

    if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "PHA : Only for sip6 and above");
    }

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.generalRegs.PHACtrl;
    /* <Disable PPA Clock> */
    value = 1 - BOOL2BIT_MAC(enable);

    return prvCpssHwPpSetRegField(devNum,regAddr,2,1,value);
}


/**
* @internal prvCpssDxChPhaThreadValidityCheck function
* @endinternal
*
* @brief   Check if the required firmware thread is valid (exists in current fw image)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - the device number
* @param[in] threadType            - type of the firmware thread that should handle the egress packet.
*
* @param[out] firmwareThreadIdPtr  - (pointer to) the firmware Thread Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid threadType
* @retval GT_BAD_STATE             - return thread ID is not in valid range
* @retval GT_NOT_SUPPORTED         - thread does not exist in fw image
*/
GT_STATUS prvCpssDxChPhaThreadValidityCheck
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_PHA_THREAD_TYPE_ENT   threadType,
    OUT GT_U32                          *firmwareThreadIdPtr
)
{
    FW_THREAD_ID  firmwareThreadId=0;
    GT_STATUS     rc;

    /* Get the compatible fw thread ID */
    rc = firmwareThreadTypeToThreadId(threadType, &firmwareThreadId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Check that firmware thread ID is in valid range */
    if(firmwareThreadId >= PRV_CPSS_DXCH_PHA_MAX_THREADS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "firmwareThreadId[%d] expected to be less than[%d] ", firmwareThreadId, PRV_CPSS_DXCH_PHA_MAX_THREADS_CNS);
    }

    /* Check firmware thread is in firmware image */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[firmwareThreadId].firmwareInstructionPointer == PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "PC address of firmwareThreadId[%d] is not valid. Thread does not exist in fw image.",firmwareThreadId);
    }

    /* Set output parameter with firmwareThreadId */
    *firmwareThreadIdPtr = firmwareThreadId;

    return rc;
}


/**
* @internal firmwareDropThreadPhaEntriesTableSet function
* @endinternal
*
* @brief  Set PHA entries tables with Drop thread's parameters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS firmwareDropThreadPhaEntriesTableSet
(
    IN GT_U8     devNum
)
{
    GT_STATUS                                 rc;                      /* return code status                            */
    PRV_CPSS_DXCH_PHA_FW_THREAD_INFO_STC      firmwareThreadInfo;      /* the firmware thread info                      */
    GT_U32                                    PPAThreadsConf1Word = 0; /* table 1: 23 bits                              */
    GT_U32                                    PPAThreadsConf2Words[5]; /* table 2: 149 bits                             */
    GT_U32                                    hwSkipCounter;           /* hw value for <skipCounter>                    */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT    dsaCpuCode;              /* DSA code                                      */
    GT_U32                                    phaTableIndex;           /* loop index iterator to scan PHA entries table */
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC      commonInfo;              /* Firmware thread's common information          */


    /* Check validity of input parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    /* Drop thread's common information */
    commonInfo.stallDropCode    = CPSS_NET_PHA_FW_DROP_E;
    commonInfo.busyStallMode    = CPSS_DXCH_PHA_BUSY_STALL_MODE_PUSH_BACK_E;
    commonInfo.statisticalProcessingFactor = 0;

    /* Convert SW cpu code to HW cpu code */
    rc = prvCpssDxChNetIfCpuToDsaCode(commonInfo.stallDropCode,&dsaCpuCode);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get Drop thread related information */
    rc = firmwareInstructionPointerGet(devNum, CPSS_DXCH_PHA_THREAD_TYPE_DROP_ALL_TRAFFIC_E, &firmwareThreadInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get skip counter value of Drop thread */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.packetOrderChangeEnable == GT_TRUE)
    {
        hwSkipCounter = firmwareThreadInfo.skipCounter;
    }
    else
    {
        hwSkipCounter = 0;
    }

    /* Do last Drop thread parameters check */
    CPSS_DATA_CHECK_MAX_MAC(firmwareThreadInfo.firmwareInstructionPointer,BIT_16);
    CPSS_DATA_CHECK_MAX_MAC(hwSkipCounter,BIT_5);
    CPSS_DATA_CHECK_MAX_MAC(dsaCpuCode,BIT_8);
    CPSS_DATA_CHECK_MAX_MAC(firmwareThreadInfo.headerWindowAnchor,BIT_2);
    CPSS_DATA_CHECK_MAX_MAC(firmwareThreadInfo.headerWindowSize,BIT_2);

    /* Clear local Conf2 table. Clear 16B template to zero as Drop thread template is not in use */
    cpssOsMemSet(PPAThreadsConf2Words,0,sizeof(PPAThreadsConf2Words));

    /* Set Drop thread address and skip counter into local Conf2 table */
    PPAThreadsConf2Words[4] = firmwareThreadInfo.firmwareInstructionPointer | hwSkipCounter << 16;

    /* Prepare Conf1 table parameters */
    U32_SET_FIELD_MAC(PPAThreadsConf1Word, 0,8,commonInfo.statisticalProcessingFactor);
    U32_SET_FIELD_MAC(PPAThreadsConf1Word, 9,2,commonInfo.busyStallMode);
    U32_SET_FIELD_MAC(PPAThreadsConf1Word,11,8,dsaCpuCode);
    U32_SET_FIELD_MAC(PPAThreadsConf1Word,19,2,firmwareThreadInfo.headerWindowAnchor);
    U32_SET_FIELD_MAC(PPAThreadsConf1Word,21,2,firmwareThreadInfo.headerWindowSize);

    /* Write Drop threads parameters into PHA entries HW tables
       Go over all tables entries except the first entry which is kept for do nothing thread */
    for(phaTableIndex  = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;
        phaTableIndex <= PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC;
        phaTableIndex++)
    {
        /**********************************************************************************************************
         * The combination of continues PHA traffic while configuring PHA parameters into 2 separate tables       *
         * can lead to execution of fw thread with incompatible windows size/anchor. It can cause unexpected      *
         * behavior of this fw thread. Therefore, when setting Drop thread over entry that was already            *
         * configured with other thread need first to set Drop thread's address and only then window anchor/size. *
         * Since Drop thread does not use the packet (only set descriptor), incompatible windows size/anchor      *
         * values won't matter so it should be o.k. However when setting any other thread over Drop thread,       *
         * should first set the thread's window anchor/size and only then set the thread's address so in this     *
         * case fw thread will run with compatible windows size/anchor values.                                    *
         **********************************************************************************************************/
        /* First, Set the entry of Conf2 table with:
           - thread's address
           - thread's skip counter
           - thread's template
         */
        rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_2_E,
                                        phaTableIndex,
                                        &PPAThreadsConf2Words[0]);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Last, Set the entry of Conf1 table with:
           - thread's header Window Anchor
           - thread's header Window Size
           - thread's statistical Processing Factor
           - thread's hw_busy Stall Mode
           - thread's dsa Cpu Code
         */
        rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_1_E,
                                        phaTableIndex,
                                        &PPAThreadsConf1Word);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Save the thread type (per threadId) in DB */
        PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadTypeArr[phaTableIndex] = CPSS_DXCH_PHA_THREAD_TYPE_DROP_ALL_TRAFFIC_E;
    }

    return rc;
}


/**
* @internal internal_cpssDxChPhaInit function
* @endinternal
*
* @brief   Init the PHA unit in the device.
*
*   NOTEs:
*   1. GT_NOT_INITIALIZED will be return for any 'PHA' lib APIs if called before
*       cpssDxChPhaInit(...)
*       (exclude cpssDxChPhaInit(...) itself)
*   2. GT_NOT_INITIALIZED will be return for EPCL APIs trying to enable PHA processing
*       if called before cpssDxChPhaInit(...)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] packetOrderChangeEnable  - Enable/Disable the option for packet order
*                                      change between heavily processed flows and lightly processed flows
*                                      GT_TRUE  - packet order is not maintained
*                                      GT_FALSE  - packet order is maintained
* @param[in] phaFwImageId             - PHA firmware image ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_ALREADY_EXIST         - the library was already initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - firmware address check fail. One of firmware threads holds bad address
*/
static GT_STATUS internal_cpssDxChPhaInit
(
    IN  GT_U8                           devNum,
    IN GT_BOOL                          packetOrderChangeEnable,
    IN CPSS_DXCH_PHA_FW_IMAGE_ID_ENT    phaFwImageId
)
{
    GT_STATUS   rc;
    GT_U32 i;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);

    if(0 == PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.phaInfo.numOfPpn)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "The device not supports the 'PHA unit'");
    }
    /*
        1. Set < ppaClockEnable > enable
        2. Load the FW to ALL PPG (same FW in all PPGs)
        3. Bind the WM simulation with 'instruction pointer' to 'processing thread'
        4. Use the 'recycleEnable'to know if to set the <Skip Counter> with value 0
            (recycleEnable == GT_FALSE) or with 'real value' (that CPSS got from FW team)
        5. Not allow to set in any API the 'thread-id' != 0 if the 'pha init ' was not called
        6. Set Null Processing Instruction Pointer with 'do nothing' thread 'instruction pointer'
    */

    /* save the value */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.packetOrderChangeEnable = packetOrderChangeEnable;
    /* state that the library was initialized */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.phaInitDone = GT_TRUE;
    /* save PHA firmware image ID */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.phaFwImageId = phaFwImageId;
    /* Copy threads initial data to pha information table per device */
    for(i=0; i<PRV_CPSS_DXCH_PHA_MAX_THREADS_CNS; i++)
    {
       PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[i].firmwareInstructionPointer = phaFwThreadsInitInformation[i].firmwareInstructionPointer;
       PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[i].headerWindowAnchor = phaFwThreadsInitInformation[i].headerWindowAnchor;
       PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[i].headerWindowSize = phaFwThreadsInitInformation[i].headerWindowSize;
    }

    /* Set skip counter value for each thread per device */
    rc = firmwareThreadsSkipCounterSet(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* allow the PPA sub unit in the PHA to get clock (as it was disabled during 'phase 1' init) */
    rc = prvCpssDxChPhaClockEnable(devNum,GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* init the FW related */
    rc = firmwareInit(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPhaInit function
* @endinternal
*
* @brief   Init the PHA unit in the device.
*
*   NOTEs:
*   1. GT_NOT_INITIALIZED will be return for any 'PHA' lib APIs if called before
*       cpssDxChPhaInit(...)
*       (exclude cpssDxChPhaInit(...) itself)
*   2. GT_NOT_INITIALIZED will be return for EPCL APIs trying to enable PHA processing
*       if called before cpssDxChPhaInit(...)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] packetOrderChangeEnable  - Enable/Disable the option for packet order
*                                      change between heavily processed flows and lightly processed flows
*                                      GT_TRUE  - packet order is not maintained
*                                      GT_FALSE  - packet order is maintained
* @param[in] phaFwImageId             - PHA firmware image ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_ALREADY_EXIST         - the library was already initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - firmware address check fail. One of firmware threads holds bad address
*/
GT_STATUS cpssDxChPhaInit
(
    IN  GT_U8                           devNum,
    IN GT_BOOL                          packetOrderChangeEnable,
    IN CPSS_DXCH_PHA_FW_IMAGE_ID_ENT    phaFwImageId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetOrderChangeEnable, phaFwImageId));

    rc = internal_cpssDxChPhaInit(devNum, packetOrderChangeEnable, phaFwImageId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetOrderChangeEnable, phaFwImageId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaFwImageIdGet function
* @endinternal
*
* @brief  Get PHA firmware image ID
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                - device number.
* @param[out] phaFwImageIdPtr      - (pointer to) the PHA firmware image ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaFwImageIdGet
(
    IN GT_U8                           devNum,
    OUT CPSS_DXCH_PHA_FW_IMAGE_ID_ENT  *phaFwImageIdPtr
)
{
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);

    /* Verify pointers are not null */
    CPSS_NULL_PTR_CHECK_MAC(phaFwImageIdPtr);

    /* Check that the PHA LIB was initialized */
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    /* Get PHA fw image ID value */
    *phaFwImageIdPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.phaFwImageId;

    return rc;
}

/**
* @internal cpssDxChPhaFwImageIdGet function
* @endinternal
*
* @brief  Get PHA firmware image ID
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                - device number.
* @param[out] phaFwImageIdPtr      - (pointer to) the PHA firmware image ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaFwImageIdGet
(
    IN GT_U8                           devNum,
    OUT CPSS_DXCH_PHA_FW_IMAGE_ID_ENT  *phaFwImageIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaFwImageIdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, phaFwImageIdPtr));

    rc = internal_cpssDxChPhaFwImageIdGet(devNum, phaFwImageIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, phaFwImageIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal buildIoamTemplateForIngressSwitch function
* @endinternal
*
* @brief   check that IOAM For Ingress Switch parameters are valid.
*         haEntry : build 4 words of IOAM header from the info
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] ioamIngressSwitchPtr  - (pointer to) the IOAM header info.
*
* @param[out] dataPtr[4]           - the 4 words that hold the needed IOAM header format
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS buildIoamTemplateForIngressSwitch
(
    IN  CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC *ioamIngressSwitchPtr,
    OUT GT_U32                 dataPtr[/*4*/]
)
{
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->IOAM_Trace_Type , BIT_16 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Maximum_Length  ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Flags           ,  BIT_8 );

    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Hop_Lim         ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->node_id         , BIT_24 );

    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Type1           ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->IOAM_HDR_len1   ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Reserved1       ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Next_Protocol1  ,  BIT_8 );

    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Type2           ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->IOAM_HDR_len2   ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Reserved2       ,  BIT_8 );
    CPSS_DATA_CHECK_MAX_MAC(ioamIngressSwitchPtr->Next_Protocol2  ,  BIT_8 );

    dataPtr[0] =  ioamIngressSwitchPtr->IOAM_Trace_Type  << 16 |
                  ioamIngressSwitchPtr->Maximum_Length   <<  8 |
                  ioamIngressSwitchPtr->Flags;

    dataPtr[1] =  ioamIngressSwitchPtr->Hop_Lim          << 24 |
                  ioamIngressSwitchPtr->node_id;

    dataPtr[2] =  ioamIngressSwitchPtr->Type1           << 24 |
                  ioamIngressSwitchPtr->IOAM_HDR_len1   << 16 |
                  ioamIngressSwitchPtr->Reserved1       <<  8 |
                  ioamIngressSwitchPtr->Next_Protocol1;

    dataPtr[3] =  ioamIngressSwitchPtr->Type2           << 24 |
                  ioamIngressSwitchPtr->IOAM_HDR_len2   << 16 |
                  ioamIngressSwitchPtr->Reserved2       <<  8 |
                  ioamIngressSwitchPtr->Next_Protocol2;

    return GT_OK;
}

/**
* @internal parseIoamTemplateForIngressSwitch function
* @endinternal
*
* @brief   parse 2 words of ETAG from haEntry to IOAM For Ingress Switch parameters
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] dataPtr[4]           - the 4 words that hold the needed IOAM header format
*
* @param[out] ioamIngressSwitchPtr  - (pointer to) the IOAM header info.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS parseIoamTemplateForIngressSwitch(
    IN GT_U32   dataPtr[/*4*/],
    OUT CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC *ioamIngressSwitchPtr
)
{
    ioamIngressSwitchPtr->IOAM_Trace_Type = U32_GET_FIELD_MAC(dataPtr[0],16,16);
    ioamIngressSwitchPtr->Maximum_Length  = U32_GET_FIELD_MAC(dataPtr[0], 8, 8);
    ioamIngressSwitchPtr->Flags           = U32_GET_FIELD_MAC(dataPtr[0], 0, 8);

    ioamIngressSwitchPtr->Hop_Lim         = U32_GET_FIELD_MAC(dataPtr[1],24, 8);
    ioamIngressSwitchPtr->node_id         = U32_GET_FIELD_MAC(dataPtr[1], 0,24);

    ioamIngressSwitchPtr->Type1           = U32_GET_FIELD_MAC(dataPtr[2],24, 8);
    ioamIngressSwitchPtr->IOAM_HDR_len1   = U32_GET_FIELD_MAC(dataPtr[2],16, 8);
    ioamIngressSwitchPtr->Reserved1       = U32_GET_FIELD_MAC(dataPtr[2], 8, 8);
    ioamIngressSwitchPtr->Next_Protocol1  = U32_GET_FIELD_MAC(dataPtr[2], 0, 8);

    ioamIngressSwitchPtr->Type2           = U32_GET_FIELD_MAC(dataPtr[3],24, 8);
    ioamIngressSwitchPtr->IOAM_HDR_len2   = U32_GET_FIELD_MAC(dataPtr[3],16, 8);
    ioamIngressSwitchPtr->Reserved2       = U32_GET_FIELD_MAC(dataPtr[3], 8, 8);
    ioamIngressSwitchPtr->Next_Protocol2  = U32_GET_FIELD_MAC(dataPtr[3], 0, 8);

    return GT_OK;
}

/**
* @internal parseMplsIpv6Template function
* @endinternal
*
* @brief   parse 4 word of MPLS header to Unified SR IPv6 header.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] dataPtr[4]             - the 4 words that hold the needed IOAM header format
*
* @param[out] ioamIngressSwitchPtr  - (pointer to) the IOAM header info.
*
* @retval GT_OK                     - on success
*/
static GT_STATUS parseMplsIpv6Template
(
    IN  GT_U32   dataPtr[/*4*/],
    OUT CPSS_DXCH_PHA_THREAD_INFO_TYPE_UNIFIED_SR_STC *unifiedSRIpv6Ptr
)
{
    unifiedSRIpv6Ptr->srcAddr.u32Ip[0] = dataPtr[0];
    unifiedSRIpv6Ptr->srcAddr.u32Ip[1] = dataPtr[1];
    unifiedSRIpv6Ptr->srcAddr.u32Ip[2] = dataPtr[2];
    unifiedSRIpv6Ptr->srcAddr.u32Ip[3] = dataPtr[3];

    return GT_OK;
}

/**
* @internal parseClassifierTemplateForNshOverVxlanGpe function
* @endinternal
*
* @brief   parse 1 word of Classifier NSH over vxlan-gpe parameters
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman;
*
* @param[in] dataPtr[1]                    - the 1 word that holds the needed template format
* @param[out] classifierNshOverVxlanGpePtr - (pointer to) Classifier NSH over vxlan-gpe header info
*
* @retval GT_OK - on success
*/
static GT_STATUS parseClassifierTemplateForNshOverVxlanGpe
(
    IN  GT_U32                 dataPtr[/*1*/],
    OUT CPSS_DXCH_PHA_THREAD_INFO_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC *classifierNshOverVxlanGpePtr
)
{
    /* Get Source Node ID (12 MS bits)*/
    classifierNshOverVxlanGpePtr->source_node_id = U32_GET_FIELD_MAC(dataPtr[0],20,12);

    return GT_OK;
}

/**
* @internal buildClassifierTemplateForNshOverVxlanGpe function
* @endinternal
*
* @brief  check that Classifier NSH over VXLAN-GPE parameters are valid.
*         haEntry : build 1 word of Classifier NSH over vxlan-gpe header from the info
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] classifierNshOverVxlanGpePtr - (pointer to) Classifier NSH over vxlan-gpe header info
* @param[out] dataPtr[1]                  - the 1 word that holds the needed template format
*
* @retval GT_OK           - on success
* @retval GT_OUT_OF_RANGE - on out of range parameter
*/
static GT_STATUS buildClassifierTemplateForNshOverVxlanGpe
(
    IN  CPSS_DXCH_PHA_THREAD_INFO_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC *classifierNshOverVxlanGpePtr,
    OUT GT_U32    dataPtr[/*1*/]
)
{
   /* Check parameter is valid */
   CPSS_DATA_CHECK_MAX_MAC(classifierNshOverVxlanGpePtr->source_node_id, BIT_12);
   /* Save parameter as defined in architect's document */
    dataPtr[0] = classifierNshOverVxlanGpePtr->source_node_id <<20 ;

    return GT_OK;
}

/**
* @internal buildIoamTemplateForTransitSwitch function
* @endinternal
*
* @brief   check that IOAM For Transit Switch parameters are valid.
*         haEntry : build 1 word of IOAM header from the info
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] ioamTransitSwitchPtr  - (pointer to) the IOAM header info.
*
* @param[out] dataPtr[1]           - the 1 word that hold the needed IOAM header format
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS buildIoamTemplateForTransitSwitch
(
    IN  CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC *ioamTransitSwitchPtr,
    OUT GT_U32                 dataPtr[/*1*/]
)
{

    CPSS_DATA_CHECK_MAX_MAC(ioamTransitSwitchPtr->node_id         , BIT_24 );

    dataPtr[0] =  /* 8 bits of 'Hop_Lim' are 'ignored' and 'ttl' is used by the device */
                  ioamTransitSwitchPtr->node_id;

    return GT_OK;
}

/**
* @internal parseIoamTemplateForTransitSwitch function
* @endinternal
*
* @brief   parse 2 words of ETAG from haEntry to IOAM For Transit Switch parameters
*         haEntry : build 1 word of IOAM header from the info
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] dataPtr[1]           - the 1 word that hold the needed IOAM header format
*
* @param[out] ioamTransitSwitchPtr  - (pointer to) the IOAM header info.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS parseIoamTemplateForTransitSwitch
(
    IN  GT_U32                 dataPtr[/*1*/],
    OUT CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC *ioamTransitSwitchPtr
)
{
    /* 8 bits of 'Hop_Lim' are 'ignored' and 'ttl' is used by the device */
    ioamTransitSwitchPtr->node_id         = U32_GET_FIELD_MAC(dataPtr[0], 0,24);

    return GT_OK;
}

/**
* @internal buildIoamTemplateForEgressSwitch function
* @endinternal
*
* @brief   check that IOAM For Egress Switch parameters are
*         valid. haEntry : build 1 word of IOAM header from the info
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] ioamEgressSwitchPtr  - (pointer to) the IOAM header info.
*
* @param[out] dataPtr[1]           - the 1 word that hold the needed IOAM header format
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS buildIoamTemplateForEgressSwitch
(
    IN  CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_STC *ioamEgressSwitchPtr,
    OUT GT_U32                 dataPtr[/*1*/]
)
{

    CPSS_DATA_CHECK_MAX_MAC(ioamEgressSwitchPtr->node_id         , BIT_24 );

    dataPtr[0] =  /* 8 bits of 'Hop_Lim' are 'ignored' and 'ttl' is used by the device */
                  ioamEgressSwitchPtr->node_id;

    return GT_OK;
}

/**
* @internal parseIoamTemplateForEgressSwitch function
* @endinternal
*
* @brief  parse 1 word of PHA template for IOAM Egress Switch
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] dataPtr[1]           - the 1 word that hold the needed IOAM header format
*
* @param[out] ioamEgressSwitchPtr  - (pointer to) the IOAM header info.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS parseIoamTemplateForEgressSwitch
(
    IN  GT_U32                 dataPtr[/*1*/],
    OUT CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_STC *ioamEgressSwitchPtr
)
{
    /* 8 bits of 'Hop_Lim' are 'ignored' and 'ttl' is used by the device */
    ioamEgressSwitchPtr->node_id         = U32_GET_FIELD_MAC(dataPtr[0], 0,24);

    return GT_OK;
}

/**
* @internal buildMplsIpv6Template function
* @endinternal
*
* @brief  check that the mpls IPv6 parameters are valid.
*         build 4 words of mpls header from the info
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] unifiedSRIpv6Ptr  - (pointer to) the MPLS SR IPv6 info.
*
* @param[out] dataPtr[4]           - the 4 words that hold the needed MPLS header format
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*/
static GT_STATUS buildMplsIpv6Template
(
    IN  CPSS_DXCH_PHA_THREAD_INFO_TYPE_UNIFIED_SR_STC *unifiedSRIpv6Ptr,
    OUT GT_U32                 dataPtr[/*4*/]
)
{
    /* TODO - Paramaters validation */
    dataPtr[0] =  unifiedSRIpv6Ptr->srcAddr.u32Ip[0];
    dataPtr[1] =  unifiedSRIpv6Ptr->srcAddr.u32Ip[1];
    dataPtr[2] =  unifiedSRIpv6Ptr->srcAddr.u32Ip[2];
    dataPtr[3] =  unifiedSRIpv6Ptr->srcAddr.u32Ip[3];

    return GT_OK;
}

/**
* @internal buildSrv6SrcNodeTemplate function
* @endinternal
*
* @brief  build 4 words of SRv6 Source Node header from the info
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] srv6SrcNodePtr  - (pointer to) the SR IPv6 Source Node info (IPv6 src address).
*
* @param[out] dataPtr[4]     - the 4 words that hold the needed SRv6 source node header format
*
* @retval GT_OK              - on success
*/
static GT_STATUS buildSrv6SrcNodeTemplate
(
    IN  CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC *srv6SrcNodePtr,
    OUT GT_U32                 dataPtr[/*4*/]
)
{
    GT_U32 ii;

    for (ii = 0; ii < 4; ii++)
    {
        dataPtr[ii] =   (srv6SrcNodePtr->srcAddr.arIP[(ii * 4)]     << 24) |
                        (srv6SrcNodePtr->srcAddr.arIP[(ii * 4) + 1] << 16) |
                        (srv6SrcNodePtr->srcAddr.arIP[(ii * 4) + 2] << 8)  |
                        (srv6SrcNodePtr->srcAddr.arIP[(ii * 4) + 3]);
    }

    return GT_OK;
}

/**
* @internal parseSrv6SrcNodeTemplate function
* @endinternal
*
* @brief   parse 4 words of PHA template for SRv6 Source Node
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] dataPtr[4]           - the 4 words that hold the needed SRv6 Source Node header format
*
* @param[out] srv6SrcNodePtr      - (pointer to) the SRv6 Source Node header info (IPv6 src address).
*
* @retval GT_OK                   - on success
*/
static GT_STATUS parseSrv6SrcNodeTemplate
(
    IN  GT_U32   dataPtr[/*4*/],
    OUT CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC *srv6SrcNodePtr
)
{
    GT_U32 ii;

    for (ii = 0; ii < 4; ii++)
    {
        srv6SrcNodePtr->srcAddr.arIP[(ii * 4)] =     (GT_U8)((dataPtr[ii] >> 24) & 0xFF);
        srv6SrcNodePtr->srcAddr.arIP[(ii * 4) + 1] = (GT_U8)((dataPtr[ii] >> 16) & 0xFF);
        srv6SrcNodePtr->srcAddr.arIP[(ii * 4) + 2] = (GT_U8)((dataPtr[ii] >> 8) & 0xFF);
        srv6SrcNodePtr->srcAddr.arIP[(ii * 4) + 3] = (GT_U8)(dataPtr[ii] & 0xFF);
    }

    return GT_OK;
}

/**
* @internal buildSgtNetworkTemplate function
* @endinternal
*
* @brief  build 4 words of SGT network thread template from the info
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] sgtNetworkPtr   - (pointer to) the sgt network info.
*
* @param[out] dataPtr[4]     - the 4 words that hold the needed SGT TAG header format
*
* @retval GT_OK              - on success
*/
static GT_STATUS buildSgtNetworkTemplate
(
    IN  CPSS_DXCH_PHA_THREAD_INFO_TYPE_SGT_NETWORK_STC *sgtNetworkPtr,
    OUT GT_U32                                          dataPtr[/*4*/]
)
{
    /* Check parameter is valid */
    CPSS_DATA_CHECK_MAX_MAC(sgtNetworkPtr->length, BIT_12);
    CPSS_DATA_CHECK_MAX_MAC(sgtNetworkPtr->optionType, BIT_12);

    dataPtr[0] = (sgtNetworkPtr->etherType  << 16) |
                 (sgtNetworkPtr->version    << 8 ) |
                 (sgtNetworkPtr->length     >> 4 );

    dataPtr[1] = 0;
    dataPtr[1] = (sgtNetworkPtr->length     << 28) |
                 (sgtNetworkPtr->optionType << 16);
    dataPtr[2] = 0;           /* Reserved2 */
    dataPtr[3] = 0;           /* Reserved3 */
    return GT_OK;
}

/**
* @internal parseSgtNetworkTemplate function
* @endinternal
*
* @brief  parse 4 words of SGT network thread template from the info
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] dataPtr[4]       - the 4 words that hold the needed SGT TAG header format
*
* @param[out] sgtNetworkPtr   - (pointer to) SGT Network info.
*
* @retval GT_OK               - on success
*/
static GT_STATUS parseSgtNetworkTemplate
(
    IN  GT_U32                                          dataPtr[/*4*/],
    OUT CPSS_DXCH_PHA_THREAD_INFO_TYPE_SGT_NETWORK_STC *sgtNetworkPtr
)
{
    sgtNetworkPtr->etherType    = U32_GET_FIELD_MAC(dataPtr[0],  16, 16);
    sgtNetworkPtr->version      = U32_GET_FIELD_MAC(dataPtr[0],   8,  8);
    sgtNetworkPtr->length       = (U32_GET_FIELD_MAC(dataPtr[0],  0,  8) << 4) |
                                   U32_GET_FIELD_MAC(dataPtr[1], 28,  4);
    sgtNetworkPtr->optionType   = U32_GET_FIELD_MAC(dataPtr[1],  16, 12);
    return GT_OK;
}

/**
* @internal parseSflowV5Template function
* @endinternal
*
* @brief  build sflow agent ip address's thread template from the user info
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] dataPtr[0]     -  holds the ip address of the sflow agent header format
*            dataPtr[1]     -  holds the enterprise data format
*
* @param[out] sflowV5MirrorPtr   - (pointer to) the sflow v5 info.
*
* @retval GT_OK              - on success
*/
static GT_STATUS parseSflowV5Template
(
    IN   GT_U32 dataPtr[/*4*/],
    OUT  CPSS_DXCH_PHA_THREAD_INFO_TYPE_SFLOW_V5_STC  *sflowV5MirrorPtr
)
{

    /*Word2: Agent IPv4 Address is Set to template[31:0]*/
    sflowV5MirrorPtr->sflowAgentIpAddr.arIP[0] = (GT_U8)(dataPtr[0] >> 24) & 0xFF;
    sflowV5MirrorPtr->sflowAgentIpAddr.arIP[1] = (GT_U8)(dataPtr[0] >> 16) & 0xFF;
    sflowV5MirrorPtr->sflowAgentIpAddr.arIP[2] = (GT_U8)(dataPtr[0] >> 8)  & 0xFF;
    sflowV5MirrorPtr->sflowAgentIpAddr.arIP[3] = (GT_U8)(dataPtr[0] & 0xFF);

    /*Word7: Data Format is Set to template[63:32]*/
    sflowV5MirrorPtr->sflowDataFormat = dataPtr[1];
    return GT_OK;
}
/**
* @internal buildSflowV5Template function
* @endinternal
*
* @brief  build sflow agent ip address's thread template from the user info
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] sflowV5MirrorPtr - (pointer to) the sflow v5 info.
*
* @param[out] dataPtr[0]     -  holds the ip address of the sflow agent header format
*             dataPtr[1]     -  holds the enterprise data format
*
* @retval GT_OK              - on success
*/
static GT_STATUS buildSflowV5Template
(
    IN  CPSS_DXCH_PHA_THREAD_INFO_TYPE_SFLOW_V5_STC  *sflowV5MirrorPtr,
    OUT GT_U32 dataPtr[/*4*/]
)
{

    dataPtr[0] =    (sflowV5MirrorPtr->sflowAgentIpAddr.arIP[0] << 24) |
                    (sflowV5MirrorPtr->sflowAgentIpAddr.arIP[1] << 16) |
                    (sflowV5MirrorPtr->sflowAgentIpAddr.arIP[2] << 8)  |
                    (sflowV5MirrorPtr->sflowAgentIpAddr.arIP[3]);
    dataPtr[1] = sflowV5MirrorPtr->sflowDataFormat;
    return GT_OK;
}
/**
* @internal buildVxlanGbpSourceGroupPolicyIdTemplate function
* @endinternal
*
* @brief  build 1 word of VXLAN GBP Source Group Policy ID thread template from the info
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                           - device number.
* @param[in] vxlanGbpSourceGroupPolicyIdPtr   - (pointer to) the VXLAN GBP Source Group Policy ID info.
*
* @param[out] dataPtr[1]     - the 1 word that hold the needed VXLAN GBP Group Policy ID header format
*
* @retval GT_OK              - on success
*/
static GT_STATUS buildVxlanGbpSourceGroupPolicyIdTemplate
(
    IN GT_U8                                                                devNum,
    IN  CPSS_DXCH_PHA_THREAD_INFO_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_STC *vxlanGbpSourceGroupPolicyIdPtr,
    OUT GT_U32                                                              dataPtr[/*1*/]
)
{
    /* Check parameter is valid, allowed HW range is from bits 19:2 */
    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_DATA_CHECK_MAX_MAC(vxlanGbpSourceGroupPolicyIdPtr->copyReservedLsb + 1, (BIT_4 + BIT_2));
        CPSS_DATA_CHECK_MAX_MAC(vxlanGbpSourceGroupPolicyIdPtr->copyReservedMsb + 1, (BIT_4 + BIT_2));
    }
    else
    {
        CPSS_DATA_CHECK_MAX_MAC(vxlanGbpSourceGroupPolicyIdPtr->copyReservedLsb + 2, (BIT_4 + BIT_2));
        CPSS_DATA_CHECK_MAX_MAC(vxlanGbpSourceGroupPolicyIdPtr->copyReservedMsb + 2, (BIT_4 + BIT_2));
    }

    if((vxlanGbpSourceGroupPolicyIdPtr->copyReservedLsb) > (vxlanGbpSourceGroupPolicyIdPtr->copyReservedMsb))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "CopyReserved First Bit cannot be greater than Last Bit\n");
    }

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        dataPtr[0] = ((vxlanGbpSourceGroupPolicyIdPtr->copyReservedLsb + 1) |
                     ((vxlanGbpSourceGroupPolicyIdPtr->copyReservedMsb + 1) << 5));
    }
    else
    {
        dataPtr[0] = ((vxlanGbpSourceGroupPolicyIdPtr->copyReservedLsb + 2) |
                     ((vxlanGbpSourceGroupPolicyIdPtr->copyReservedMsb + 2) << 5));
    }

    return GT_OK;
}

/**
* @internal parseVxlanGbpSourceGroupPolicyIdTemplate function
* @endinternal
*
* @brief  parse 1 word of VXLAN GBP Source Group Policy ID thread template from the info
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                            - device number.
* @param[in] dataPtr[1]                        - the 1 word that hold the needed VXLAN GBP Group Policy ID header format
*
* @param[out] vxlanGbpSourceGroupPolicyIdPtr   - (pointer to) VXLAN GBP Source Group Policy ID info.
*
* @retval GT_OK               - on success
*/
static GT_STATUS parseVxlanGbpSourceGroupPolicyIdTemplate
(
    IN GT_U8                                                                devNum,
    IN  GT_U32                                                              dataPtr[/*1*/],
    OUT CPSS_DXCH_PHA_THREAD_INFO_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_STC *vxlanGbpSourceGroupPolicyIdPtr
)
{
    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        vxlanGbpSourceGroupPolicyIdPtr->copyReservedLsb     = (U32_GET_FIELD_MAC(dataPtr[0],  0, 5) - 1);
        vxlanGbpSourceGroupPolicyIdPtr->copyReservedMsb     = (U32_GET_FIELD_MAC(dataPtr[0],  5, 5) - 1);
    }
    else
    {
        vxlanGbpSourceGroupPolicyIdPtr->copyReservedLsb     = (U32_GET_FIELD_MAC(dataPtr[0],  0, 5) - 2);
        vxlanGbpSourceGroupPolicyIdPtr->copyReservedMsb     = (U32_GET_FIELD_MAC(dataPtr[0],  5, 5) - 2);
    }

    return GT_OK;
}

/**
* @internal buildSrv6Coc32GsidSrhTemplate function
* @endinternal
*
* @brief  build 1 word of SRH Coc32 G-SID common prefix thread template from the info
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] srv6Coc32GsidCommonPrefixPtr   - (pointer to) the common prefix in SRv6 CoC32 G-SID SRH info.
*
* @param[out] dataPtr[1]                    - the 1 word that hold the needed common prefix in SRv6 CoC32 G-SID SRH header format
*
* @retval GT_OK              - on success
*/
static GT_STATUS buildSrv6Coc32GsidSrhTemplate
(
    IN  CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_COC32_GSID_STC                  *srv6Coc32GsidCommonPrefixPtr,
    OUT GT_U32                                                              dataPtr[/*1*/]
)
{
    dataPtr[0] = srv6Coc32GsidCommonPrefixPtr->dipCommonPrefixLength;

    return GT_OK;
}

/**
* @internal parseSrv6Coc32GsidSrhTemplate function
* @endinternal
*
* @brief  parse 1 word of SRH Coc32 G-SID common prefix thread template from the info
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] dataPtr[1]                    - the 1 word that hold the needed common prefix in SRv6 CoC32 G-SID SRH header format
*
* @param[out] srv6Coc32GsidCommonPrefixPtr - (pointer to) the common prefix in SRv6 CoC32 G-SID SRH info.
*
* @retval GT_OK               - on success
*/
static GT_STATUS parseSrv6Coc32GsidSrhTemplate
(
    IN  GT_U32                                                              dataPtr[/*1*/],
    OUT CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_COC32_GSID_STC                  *srv6Coc32GsidCommonPrefixPtr
)
{
    srv6Coc32GsidCommonPrefixPtr->dipCommonPrefixLength = (U32_GET_FIELD_MAC(dataPtr[0],  0, 8));

    return GT_OK;
}

/**
* @internal buildErspanL2Template function
* @endinternal
*
* @brief  build 4 words of erspan L2 header from the info
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] erspanPtr       - (pointer to) the erspan info.
*
* @param[out] dataPtr[4]     - the 4 words that hold the needed ERSPAN tunnel L2 header format
*
* @retval GT_OK              - on success
* @retval GT_BAD_PARAM       - on wrong vlan id
* @retval GT_OUT_OF_RANGE    - on out of range UP/CFI/VID
*/
static GT_STATUS buildErspanL2Template
(
    IN  CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC *erspanPtr,
    OUT GT_U32                                           dataPtr[/*4*/]
)
{
    /* Check parameter is valid */
    CPSS_DATA_CHECK_MAX_MAC(erspanPtr->l2Info.up,   BIT_3);
    CPSS_DATA_CHECK_MAX_MAC(erspanPtr->l2Info.cfi,  BIT_1);
    CPSS_DATA_CHECK_MAX_MAC(erspanPtr->l2Info.vid,  BIT_12);

    /* DMAC and SMAC */
    dataPtr[0] =   (erspanPtr->l2Info.macDa.arEther[0] << 24) |
                   (erspanPtr->l2Info.macDa.arEther[1] << 16) |
                   (erspanPtr->l2Info.macDa.arEther[2] << 8) |
                    erspanPtr->l2Info.macDa.arEther[3];
    dataPtr[1] =   (erspanPtr->l2Info.macDa.arEther[4] << 24) |
                   (erspanPtr->l2Info.macDa.arEther[5] << 16) |
                   (erspanPtr->l2Info.macSa.arEther[0] << 8) |
                    erspanPtr->l2Info.macSa.arEther[1];
    dataPtr[2] =   (erspanPtr->l2Info.macSa.arEther[2] << 24) |
                   (erspanPtr->l2Info.macSa.arEther[3] << 16) |
                   (erspanPtr->l2Info.macSa.arEther[4] << 8) |
                    erspanPtr->l2Info.macSa.arEther[5];

    dataPtr[3] = (erspanPtr->l2Info.tpid << 16) |
                 (erspanPtr->l2Info.up   << 13) |
                 (erspanPtr->l2Info.cfi  << 12) |
                  erspanPtr->l2Info.vid;

    return GT_OK;
}

/**
* @internal parseErspanL2Template function
* @endinternal
*
* @brief   parse 4 words of PHA L2 header template for ERSPAN
*
* @note   APPLICABLE DEVICES:      Falcon
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] dataPtr[4]           - the 4 words that hold the needed ERSPAN L2 header format
*
* @param[out] erspanPtr           - (pointer to) the ERSPAN L2 header template.
*
* @retval GT_OK                   - on success
*/
static GT_STATUS parseErspanL2Template
(
    IN  GT_U32                                           dataPtr[/*4*/],
    OUT CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC *erspanPtr
)
{
    erspanPtr->l2Info.macDa.arEther[0] = (dataPtr[0] >> 24) & 0xff;
    erspanPtr->l2Info.macDa.arEther[1] = (dataPtr[0] >> 16) & 0xff;
    erspanPtr->l2Info.macDa.arEther[2] = (dataPtr[0] >> 8) & 0xff;
    erspanPtr->l2Info.macDa.arEther[3] = (dataPtr[0] >> 0) & 0xff;
    erspanPtr->l2Info.macDa.arEther[4] = (dataPtr[1] >> 24) & 0xff;
    erspanPtr->l2Info.macDa.arEther[5] = (dataPtr[1] >> 16) & 0xff;

    erspanPtr->l2Info.macSa.arEther[0] = (dataPtr[1] >> 8) & 0xff;
    erspanPtr->l2Info.macSa.arEther[1] = (dataPtr[1] >> 0) & 0xff;
    erspanPtr->l2Info.macSa.arEther[2] = (dataPtr[2] >> 24) & 0xff;
    erspanPtr->l2Info.macSa.arEther[3] = (dataPtr[2] >> 16) & 0xff;
    erspanPtr->l2Info.macSa.arEther[4] = (dataPtr[2] >> 8) & 0xff;
    erspanPtr->l2Info.macSa.arEther[5] = (dataPtr[2] >> 0) & 0xff;

    erspanPtr->l2Info.tpid = (dataPtr[3] >> 16) & 0xffff;
    erspanPtr->l2Info.up   = (dataPtr[3] >> 13) & 0x7;
    erspanPtr->l2Info.cfi  = (dataPtr[3] >> 12) & 0x1;
    erspanPtr->l2Info.vid  = (dataPtr[3] >>  0) & 0xfff;

    return GT_OK;
}

/**
* @internal internal_cpssDxChPhaThreadIdEntrySet function
* @endinternal
*
* @brief   Set the entry in the PHA Thread-Id table.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                - device number.
* @param[in] phaThreadId           - the thread-Id.
*                                    (APPLICABLE RANGE: 1..255)
* @param[in] commonInfoPtr         - (pointer to) the common information needed for this threadId.
* @param[in] extType               - the type of operation that this entry need to do.
*                                    NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[in] extInfoPtr            - (pointer to) union of operation information related to the 'operationType'
*                                    NOTE: this is the template that is needed by the firmware thread
*                                    that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device or extType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaThreadIdEntrySet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PHA_FW_THREAD_INFO_STC                                firmwareThreadInfo;     /* the firmware thread info */
    GT_U32                                                              PPAThreadsConf1Word = 0;/* table 1 :  23 bits */
    GT_U32                                                              PPAThreadsConf2Words[5];/* table 2 : 149 bits */
    GT_U32                                                              hw_busyStallMode;       /*hw value for <busyStallMode> */
    GT_U32                                                              hw_skipCounter;         /*hw value for <skipCounter>   */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT                              dsaCpuCode;             /* DSA code */
    GT_U32                                                              templateArr[4];         /* 4 words of Template */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC              *ioamIngressSwitchPtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC              *ioamTransitSwitchPtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_STC               *ioamEgressSwitchPtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_UNIFIED_SR_STC                       *unifiedSRIpv6Ptr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC    *classifierNshOverVxlanGpePtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC                    *srv6SrcNodePtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_SGT_NETWORK_STC                      *sgtNetworkPtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_STC *vxlanGbpSourceGroupPolicyIdPtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_COC32_GSID_STC                  *srv6Coc32GsidCommonPrefixPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);
    CPSS_NULL_PTR_CHECK_MAC(commonInfoPtr);
    CPSS_NULL_PTR_CHECK_MAC(extInfoPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    /**************************************/
    /* start to :                         */
    /* check validity of INPUT parameters */
    /* and convert to HW values           */
    /**************************************/


    /* check the index to the table */
    PRV_CPSS_DXCH_PHA_THREAD_ID_CHECK_MAC(devNum,phaThreadId);

    /* convert SW cpu code to HW cpu code */
    rc = prvCpssDxChNetIfCpuToDsaCode(commonInfoPtr->stallDropCode,&dsaCpuCode);
    if(rc != GT_OK)
    {
        return rc;
    }

    CPSS_DATA_CHECK_MAX_MAC(commonInfoPtr->statisticalProcessingFactor,BIT_8);

    switch(commonInfoPtr->busyStallMode)
    {
        case CPSS_DXCH_PHA_BUSY_STALL_MODE_PUSH_BACK_E:
            hw_busyStallMode = 0;
            break;
        case CPSS_DXCH_PHA_BUSY_STALL_MODE_DROP_E     :
            hw_busyStallMode = 1;
            break;
        case CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E:
            hw_busyStallMode = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(commonInfoPtr->busyStallMode);
    }

    cpssOsMemSet(PPAThreadsConf2Words,0,sizeof(PPAThreadsConf2Words));

    switch(extType)
    {
        /** the threadId is not used */
        case CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_MIRRORING_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_EGRESS_SWITCH_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_NO_EL_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_ONE_EL_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_TWO_EL_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_THREE_EL_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_ETHERNET_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_VXLAN_GPE_TO_ETHERNET_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_ETHERNET_TO_VXLAN_GPE_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_PENULTIMATE_END_NODE_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_SEGMENTS_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_SEGMENTS_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_PTP_PHY_1_STEP_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_EGRESS_MIRRORING_METADATA_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_REMOVE_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_FIX_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_REMOVE_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV6_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV6_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV6_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV6_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_SRC_DEV_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_BEST_EFFORT_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_IPV4_TTL_INCREMENT_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_IPV6_HOP_LIMIT_INCREMENT_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SLS_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_DROP_ALL_TRAFFIC_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_II_SAME_DEV_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_II_SAME_DEV_IPV6_E:
            PPAThreadsConf2Words[3] = extInfoPtr->notNeeded;
            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E:
            if(extType == CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E)
            {
                ioamIngressSwitchPtr = &extInfoPtr->ioamIngressSwitchIpv4;
            }
            else
            {
                ioamIngressSwitchPtr = &extInfoPtr->ioamIngressSwitchIpv6;
            }
            /* check and build specific info */
            rc = buildIoamTemplateForIngressSwitch(ioamIngressSwitchPtr, &templateArr[0]);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* swap the words for the firmware */
            PPAThreadsConf2Words[3] = templateArr[0];
            PPAThreadsConf2Words[2] = templateArr[1];
            PPAThreadsConf2Words[1] = templateArr[2];
            PPAThreadsConf2Words[0] = templateArr[3];
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E:
            if(extType == CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E)
            {
                ioamTransitSwitchPtr = &extInfoPtr->ioamTransitSwitchIpv4;
            }
            else
            {
                ioamTransitSwitchPtr = &extInfoPtr->ioamTransitSwitchIpv6;
            }
            /* check and build specific info */
            rc = buildIoamTemplateForTransitSwitch(ioamTransitSwitchPtr, &templateArr[0]);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* swap the words for the firmware */
            PPAThreadsConf2Words[3] = templateArr[0];
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_EGRESS_SWITCH_IPV6_E:
            ioamEgressSwitchPtr = &extInfoPtr->ioamEgressSwitchIpv6;

            /* check and build specific info */
            rc = buildIoamTemplateForEgressSwitch(ioamEgressSwitchPtr, &templateArr[0]);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* swap the words for the firmware */
            PPAThreadsConf2Words[3] = templateArr[0];
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_UNIFIED_SR_E:
            unifiedSRIpv6Ptr = &extInfoPtr->unifiedSRIpv6;

            /* check and build specific info */
            rc = buildMplsIpv6Template(unifiedSRIpv6Ptr, &templateArr[0]);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* swap the words for the firmware */
            PPAThreadsConf2Words[3] = templateArr[0];
            PPAThreadsConf2Words[2] = templateArr[1];
            PPAThreadsConf2Words[1] = templateArr[2];
            PPAThreadsConf2Words[0] = templateArr[3];
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_E:
            classifierNshOverVxlanGpePtr = &extInfoPtr->classifierNshOverVxlanGpe;

            /* check and build specific info */
            rc = buildClassifierTemplateForNshOverVxlanGpe(classifierNshOverVxlanGpePtr, &templateArr[0]);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* swap the words for the firmware */
            PPAThreadsConf2Words[3] = templateArr[0];
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_SEGMENT_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_SEGMENTS_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E:
            srv6SrcNodePtr = &extInfoPtr->srv6SrcNode;

            /* check and build specific info */
            rc = buildSrv6SrcNodeTemplate(srv6SrcNodePtr, &templateArr[0]);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* swap the words for the firmware */
            PPAThreadsConf2Words[3] = templateArr[0];
            PPAThreadsConf2Words[2] = templateArr[1];
            PPAThreadsConf2Words[1] = templateArr[2];
            PPAThreadsConf2Words[0] = templateArr[3];
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_ADD_MSB_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_FIX_E:
            sgtNetworkPtr = &extInfoPtr->sgtNetwork;

            /* check and build specific info */
            rc = buildSgtNetworkTemplate(sgtNetworkPtr, &templateArr[0]);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* swap the words for the firmware */
            PPAThreadsConf2Words[3] = templateArr[0];
            PPAThreadsConf2Words[2] = templateArr[1];
            PPAThreadsConf2Words[1] = templateArr[2];
            PPAThreadsConf2Words[0] = templateArr[3];
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_E:
            vxlanGbpSourceGroupPolicyIdPtr = &extInfoPtr->vxlanGbpSourceGroupPolicyId;

            /* check and build specific info */
            rc = buildVxlanGbpSourceGroupPolicyIdTemplate(devNum, vxlanGbpSourceGroupPolicyIdPtr, &templateArr[0]);
            if(rc != GT_OK)
            {
                return rc;
            }

            PPAThreadsConf2Words[0] = templateArr[0];
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_COC32_GSID_E:
            srv6Coc32GsidCommonPrefixPtr = &extInfoPtr->srv6Coc32GsidCommonPrefix;

            /* check and build specific info */
            rc = buildSrv6Coc32GsidSrhTemplate(srv6Coc32GsidCommonPrefixPtr, &templateArr[0]);
            if(rc != GT_OK)
            {
                return rc;
            }

            PPAThreadsConf2Words[0] = templateArr[0];
            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV6_E:
            rc = buildSflowV5Template(&extInfoPtr->sflowV5Mirror, &templateArr[0]);
            if(rc != GT_OK)
            {
                return rc;
            }
            PPAThreadsConf2Words[0] = templateArr[0];
            PPAThreadsConf2Words[1] = templateArr[1];
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(extType);
    }

    /* get info related to the thread type */
    rc = firmwareInstructionPointerGet(devNum, extType, &firmwareThreadInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.packetOrderChangeEnable == GT_TRUE)
    {
        hw_skipCounter = firmwareThreadInfo.skipCounter;
    }
    else
    {
        hw_skipCounter = 0;
    }

    /* do last parameters check */
    CPSS_DATA_CHECK_MAX_MAC(firmwareThreadInfo.firmwareInstructionPointer,BIT_16);
    CPSS_DATA_CHECK_MAX_MAC(hw_skipCounter,BIT_5);
    CPSS_DATA_CHECK_MAX_MAC(hw_busyStallMode,BIT_2);
    CPSS_DATA_CHECK_MAX_MAC(dsaCpuCode,BIT_8);
    CPSS_DATA_CHECK_MAX_MAC(firmwareThreadInfo.headerWindowAnchor,BIT_2);
    CPSS_DATA_CHECK_MAX_MAC(firmwareThreadInfo.headerWindowSize,BIT_2);

    PPAThreadsConf2Words[4] = firmwareThreadInfo.firmwareInstructionPointer |
                              hw_skipCounter << 16;

    /* Prepare Conf1 table parameters */
    U32_SET_FIELD_MAC(PPAThreadsConf1Word, 0,8,commonInfoPtr->statisticalProcessingFactor);
    U32_SET_FIELD_MAC(PPAThreadsConf1Word, 9,2,hw_busyStallMode);
    U32_SET_FIELD_MAC(PPAThreadsConf1Word,11,8,dsaCpuCode);
    U32_SET_FIELD_MAC(PPAThreadsConf1Word,19,2,firmwareThreadInfo.headerWindowAnchor);
    U32_SET_FIELD_MAC(PPAThreadsConf1Word,21,2,firmwareThreadInfo.headerWindowSize);

    /**************************************/
    /* ended to :                         */
    /* check validity of INPUT parameters */
    /* and convert to HW values           */
    /**************************************/

    /**************************************/
    /* start to :                         */
    /* write to the 2 HW tables           */
    /**************************************/

    /* First, Set the entry of Conf1 table with:
       - thread's header Window Anchor
       - thread's header Window Size
       - thread's statistical Processing Factor
       - thread's hw_busy Stall Mode
       - thread's dsa Cpu Code
    */
    rc = prvCpssDxChWriteTableEntry(devNum,CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_1_E,phaThreadId,&PPAThreadsConf1Word);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Last, Set the entry of Conf2 table with:
       - thread's address
       - thread's skip counter
       - thread's template
    */
    rc = prvCpssDxChWriteTableEntry(devNum,CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_2_E,phaThreadId,&PPAThreadsConf2Words[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* save the thread type (per threadId) in DB , used by 'Get' API when a specific
        'instruction pointer' is represented by more than single 'SW thread type'.
    */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadTypeArr[phaThreadId] = extType;

    return rc;
}

/**
* @internal cpssDxChPhaThreadIdEntrySet function
* @endinternal
*
* @brief   Set the entry in the PHA Thread-Id table.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number.
* @param[in] phaThreadId              - the thread-Id.
*                                      (APPLICABLE RANGE: 1..255)
* @param[in] commonInfoPtr            - (pointer to) the common information needed for this threadId.
* @param[in] extType                  - the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[in] extInfoPtr               - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device or extType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaThreadIdEntrySet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaThreadIdEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, phaThreadId , commonInfoPtr , extType , extInfoPtr));

    rc = internal_cpssDxChPhaThreadIdEntrySet(devNum, phaThreadId , commonInfoPtr , extType , extInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, phaThreadId , commonInfoPtr , extType , extInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaThreadIdEntryGet function
* @endinternal
*
* @brief   Get the entry in the PHA Thread-Id table.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                - device number.
* @param[in] phaThreadId           - the thread-Id.
*                                    (APPLICABLE RANGE: 1..255)
* @param[out] commonInfoPtr        - (pointer to) the common information needed for this threadId.
* @param[out] extTypePtr           - (pointer to) the type of operation that this entry need to do.
*                                    NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[out] extInfoPtr           - (pointer to) union of operation information related to the 'operationType'
*                                    NOTE: this is the template that is needed by the firmware thread
*                                    that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device or extType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaThreadIdEntryGet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    OUT CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    OUT CPSS_DXCH_PHA_THREAD_TYPE_ENT    *extTypePtr,
    OUT CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
)
{
    GT_STATUS                                                        rc;
    FW_THREAD_ID                                                     firmwareThreadId;          /* the firmware thread Id */
    GT_U32                                                           firmwareInstructionPointer;/* the firmware Instruction Pointer */
    GT_U32                                                           PPAThreadsConf1Word;       /* table 1 :  23 bits */
    GT_U32                                                           PPAThreadsConf2Words[5];   /* table 2 : 149 bits */
    GT_U32                                                           hw_busyStallMode;          /* hw value for <busyStallMode> */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT                           dsaCpuCode;                /* DSA code */
    CPSS_DXCH_PHA_THREAD_TYPE_ENT                                    extType;
    GT_U32                                                           templateArr[4];            /* 4 words of Template */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC           *ioamIngressSwitchPtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC           *ioamTransitSwitchPtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_STC            *ioamEgressSwitchPtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_UNIFIED_SR_STC                    *unifiedSRIpv6Ptr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC *classifierNshOverVxlanGpePtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC                 *srv6SrcNodePtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_SGT_NETWORK_STC                   *sgtNetworkPtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_STC *vxlanGbpSourceGroupPolicyIdPtr;
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_COC32_GSID_STC               *srv6Coc32GsidCommonPrefixPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);
    CPSS_NULL_PTR_CHECK_MAC(commonInfoPtr);
    CPSS_NULL_PTR_CHECK_MAC(extTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(extInfoPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    /* check the index to the table */
    PRV_CPSS_DXCH_PHA_THREAD_ID_CHECK_MAC(devNum,phaThreadId);

    /* reset the 'union' to allow application to compare 'set' values with 'get' values */
    cpssOsMemSet(extInfoPtr,0,sizeof(*extInfoPtr));


    /* get the entry from table '2' */
    rc = prvCpssDxChReadTableEntry(devNum,CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_2_E,
        phaThreadId,&PPAThreadsConf2Words[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get the entry from table '1' */
    rc = prvCpssDxChReadTableEntry(devNum,CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_1_E,
        phaThreadId,&PPAThreadsConf1Word);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get the 'instruction pointer' */
    firmwareInstructionPointer = U32_GET_FIELD_MAC(PPAThreadsConf2Words[4],0,16);

    /* convert the 'instruction pointer' to 'firmware ThreadId' */
    rc = firmwareInstructionPointerConvert(devNum, firmwareInstructionPointer, &firmwareThreadId);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(firmwareThreadId >= PRV_CPSS_DXCH_PHA_MAX_THREADS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "firmwareThreadId must be less than [%d] but got[%d]", PRV_CPSS_DXCH_PHA_MAX_THREADS_CNS, firmwareThreadId);
    }

    /**************************************/
    /* start to :                         */
    /* convert HW values to SW values     */
    /**************************************/

    commonInfoPtr->statisticalProcessingFactor = U32_GET_FIELD_MAC(PPAThreadsConf1Word, 0,8);

    hw_busyStallMode = U32_GET_FIELD_MAC(PPAThreadsConf1Word, 9,2);
    switch(hw_busyStallMode)
    {
        case 0:
            commonInfoPtr->busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_PUSH_BACK_E;
            break;
        case 1:
            commonInfoPtr->busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_DROP_E;
            break;
        case 2:
            commonInfoPtr->busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
            break;
        case 3:
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "unexpected value[%d] from HW for hw_busyStallMode",
                hw_busyStallMode);
    }

    dsaCpuCode = U32_GET_FIELD_MAC(PPAThreadsConf1Word,11,8);

    rc = prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode,&commonInfoPtr->stallDropCode);
    if(rc != GT_OK)
    {
        return rc;
    }

    extType = PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadTypeArr[phaThreadId];
    *extTypePtr = extType;

    switch(extType)
    {
        case CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_MIRRORING_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_EGRESS_SWITCH_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_NO_EL_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_ONE_EL_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_TWO_EL_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_THREE_EL_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_ETHERNET_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_VXLAN_GPE_TO_ETHERNET_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_ETHERNET_TO_VXLAN_GPE_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_PENULTIMATE_END_NODE_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_SEGMENTS_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_SEGMENTS_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_PTP_PHY_1_STEP_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_EGRESS_MIRRORING_METADATA_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_REMOVE_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_FIX_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_REMOVE_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV6_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV6_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV6_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV6_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_SRC_DEV_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_BEST_EFFORT_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_IPV4_TTL_INCREMENT_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_IPV6_HOP_LIMIT_INCREMENT_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SLS_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_DROP_ALL_TRAFFIC_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_II_SAME_DEV_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_II_SAME_DEV_IPV6_E:
            extInfoPtr->notNeeded = PPAThreadsConf2Words[3];
            break;

            /** the threadId used , with info according to type */
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E:
            if(extType == CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E)
            {
                ioamIngressSwitchPtr = &extInfoPtr->ioamIngressSwitchIpv4;
            }
            else
            {
                ioamIngressSwitchPtr = &extInfoPtr->ioamIngressSwitchIpv6;
            }

            /* swap back the words */
            templateArr[0] = PPAThreadsConf2Words[3];
            templateArr[1] = PPAThreadsConf2Words[2];
            templateArr[2] = PPAThreadsConf2Words[1];
            templateArr[3] = PPAThreadsConf2Words[0];

            /* parse HW info to SW info */
            rc = parseIoamTemplateForIngressSwitch(&templateArr[0],ioamIngressSwitchPtr);
            if(rc != GT_OK)
            {
                return rc;
            }

            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E:
            if(extType == CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E)
            {
                ioamTransitSwitchPtr = &extInfoPtr->ioamTransitSwitchIpv4;
            }
            else
            {
                ioamTransitSwitchPtr = &extInfoPtr->ioamTransitSwitchIpv6;
            }

            /* swap back the words */
            templateArr[0] = PPAThreadsConf2Words[3];

            /* parse HW info to SW info */
            rc = parseIoamTemplateForTransitSwitch(&templateArr[0],ioamTransitSwitchPtr);
            if(rc != GT_OK)
            {
                return rc;
            }

            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_IOAM_EGRESS_SWITCH_IPV6_E:
            ioamEgressSwitchPtr = &extInfoPtr->ioamEgressSwitchIpv6;

            /* swap back the words */
            templateArr[0] = PPAThreadsConf2Words[3];

            /* parse HW info to SW info */
            rc = parseIoamTemplateForEgressSwitch(&templateArr[0],ioamEgressSwitchPtr);
            if(rc != GT_OK)
            {
                return rc;
            }

            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_UNIFIED_SR_E:
            unifiedSRIpv6Ptr = &extInfoPtr->unifiedSRIpv6;

            /* swap back the words */
            templateArr[0] = PPAThreadsConf2Words[3];
            templateArr[1] = PPAThreadsConf2Words[2];
            templateArr[2] = PPAThreadsConf2Words[1];
            templateArr[3] = PPAThreadsConf2Words[0];

            /* check and build specific info */
            rc = parseMplsIpv6Template(&templateArr[0], unifiedSRIpv6Ptr);
            if(rc != GT_OK)
            {
                return rc;
            }
            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_E:
            classifierNshOverVxlanGpePtr = &extInfoPtr->classifierNshOverVxlanGpe;

            /* swap back the words */
            templateArr[0] = PPAThreadsConf2Words[3];

            /* parse HW info to SW info */
            rc = parseClassifierTemplateForNshOverVxlanGpe(&templateArr[0], classifierNshOverVxlanGpePtr);
            if(rc != GT_OK)
            {
                return rc;
            }
            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_SEGMENT_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_SEGMENTS_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E:
            srv6SrcNodePtr = &extInfoPtr->srv6SrcNode;

            /* swap back the words */
            templateArr[0] = PPAThreadsConf2Words[3];
            templateArr[1] = PPAThreadsConf2Words[2];
            templateArr[2] = PPAThreadsConf2Words[1];
            templateArr[3] = PPAThreadsConf2Words[0];

            /* parse HW info to SW info */
            rc = parseSrv6SrcNodeTemplate(&templateArr[0], srv6SrcNodePtr);
            if(rc != GT_OK)
            {
                return rc;
            }
            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_ADD_MSB_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_FIX_E:
            sgtNetworkPtr = &extInfoPtr->sgtNetwork;

            /* swap back the words */
            templateArr[0] = PPAThreadsConf2Words[3];
            templateArr[1] = PPAThreadsConf2Words[2];
            templateArr[2] = PPAThreadsConf2Words[1];
            templateArr[3] = PPAThreadsConf2Words[0];

            /* parse HW info to SW info */
            rc = parseSgtNetworkTemplate(&templateArr[0], sgtNetworkPtr);
            if(rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_E:
            vxlanGbpSourceGroupPolicyIdPtr = &extInfoPtr->vxlanGbpSourceGroupPolicyId;

            /* parse HW info to SW info */
            rc = parseVxlanGbpSourceGroupPolicyIdTemplate(devNum, &PPAThreadsConf2Words[0], vxlanGbpSourceGroupPolicyIdPtr);
            if(rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_COC32_GSID_E:
            srv6Coc32GsidCommonPrefixPtr = &extInfoPtr->srv6Coc32GsidCommonPrefix;

            /* parse HW info to SW info */
            rc = parseSrv6Coc32GsidSrhTemplate(&PPAThreadsConf2Words[0], srv6Coc32GsidCommonPrefixPtr);
            if(rc != GT_OK)
            {
                return rc;
            }
            break;

        case CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV4_E:
        case CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV6_E:
            rc = parseSflowV5Template(&PPAThreadsConf2Words[0], &extInfoPtr->sflowV5Mirror);
            if(rc != GT_OK)
            {
                return rc;
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(extType);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPhaThreadIdEntryGet function
* @endinternal
*
* @brief   Get the entry in the PHA Thread-Id table.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number.
* @param[in] phaThreadId              - the thread-Id.
*                                      (APPLICABLE RANGE: 1..255)
* @param[out] commonInfoPtr            - (pointer to) the common information needed for this threadId.
* @param[out] extTypePtr               - (pointer to) the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[out] extInfoPtr               - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device or extType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaThreadIdEntryGet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    OUT CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    OUT CPSS_DXCH_PHA_THREAD_TYPE_ENT    *extTypePtr,
    OUT CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaThreadIdEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, phaThreadId , commonInfoPtr , extTypePtr , extInfoPtr));

    rc = internal_cpssDxChPhaThreadIdEntryGet(devNum, phaThreadId , commonInfoPtr , extTypePtr , extInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, phaThreadId , commonInfoPtr , extTypePtr , extInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaPortThreadIdSet function
* @endinternal
*
* @brief   Per target port ,set the associated thread-Id.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the target port number
* @param[in] enable                   - enable/disable the use of threadId for the target port.
* @param[in] phaThreadId              - the associated thread-Id
*                                       NOTE: relevant only when enable = GT_TRUE
*                                      (APPLICABLE RANGE: 1..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port or phaThreadId
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPhaPortThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable,
    IN GT_U32       phaThreadId
)
{
    GT_U32  hw_phaThreadId;/* hw value for <phaThreadId> */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    if(enable == GT_TRUE)
    {
        /* check the thread-Id value */
        PRV_CPSS_DXCH_PHA_THREAD_ID_CHECK_MAC(devNum,phaThreadId);
        hw_phaThreadId = phaThreadId;
    }
    else
    {
        hw_phaThreadId = 0;
    }

    return prvCpssDxChWriteTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP6_HA_EPORT_TABLE_1_FIELDS_PHA_THREAD_NUMBER_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   hw_phaThreadId);
}

/**
* @internal cpssDxChPhaPortThreadIdSet function
* @endinternal
*
* @brief   Per target port ,set the associated thread-Id.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the target port number
* @param[in] enable                   - enable/disable the use of threadId for the target port.
* @param[in] phaThreadId              - the associated thread-Id
*                                       NOTE: relevant only when enable = GT_TRUE
*                                      (APPLICABLE RANGE: 1..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port or phaThreadId
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaPortThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable,
    IN GT_U32       phaThreadId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaPortThreadIdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , enable , phaThreadId));

    rc = internal_cpssDxChPhaPortThreadIdSet(devNum, portNum , enable , phaThreadId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , enable , phaThreadId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPhaPortThreadIdGet function
* @endinternal
*
* @brief   Per target port ,get the associated thread-Id.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the target port number
* @param[in] enablePtr                - (pointer to)enable/disable the use of threadId for the target port.
* @param[in] phaThreadIdPtr           - (pointer to)the associated thread-Id
*                                       NOTE: relevant only when enable = GT_TRUE
*                                      (APPLICABLE RANGE: 1..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPhaPortThreadIdGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr,
    OUT GT_U32      *phaThreadIdPtr
)
{
    GT_STATUS   rc;
    GT_U32  hw_phaThreadId;/* hw value for <phaThreadId> */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(phaThreadIdPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    rc = prvCpssDxChReadTableEntryField(devNum,
                                   CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                                   portNum,
                                   PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                   SIP6_HA_EPORT_TABLE_1_FIELDS_PHA_THREAD_NUMBER_E, /* field name */
                                   PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                   &hw_phaThreadId);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(hw_phaThreadId == 0)
    {
        *enablePtr = GT_FALSE;
        *phaThreadIdPtr = 0;/* 'dont care' */
    }
    else
    {
        *enablePtr = GT_TRUE;
        *phaThreadIdPtr = hw_phaThreadId;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPhaPortThreadIdGet function
* @endinternal
*
* @brief   Per target port ,get the associated thread-Id.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the target port number
* @param[in] enablePtr                - (pointer to)enable/disable the use of threadId for the target port.
* @param[in] phaThreadIdPtr           - (pointer to)the associated thread-Id
*                                       NOTE: relevant only when enable = GT_TRUE
*                                      (APPLICABLE RANGE: 1..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaPortThreadIdGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr,
    OUT GT_U32      *phaThreadIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaPortThreadIdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , enablePtr , phaThreadIdPtr));

    rc = internal_cpssDxChPhaPortThreadIdGet(devNum, portNum , enablePtr , phaThreadIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , enablePtr , phaThreadIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPhaSourcePortEntrySet function
* @endinternal
*
* @brief   Set per source physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - source physical port number.
* @param[in] infoType              - the type of source port info.
* @param[in] portInfoPtr           - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaSourcePortEntrySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_U32  hwValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(portInfoPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    switch(infoType)
    {
        case CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNUSED_E:
            hwValue = portInfoPtr->rawFormat;/* not used */
            break;
        case CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E:
            hwValue = portInfoPtr->erspanSameDevMirror.erspanIndex;
            if (hwValue > 0xFFFF )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                        "Erspan Index must be in range 0..65535 and not [%d] \n",
                        hwValue);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(infoType);
    }

    /* save the port type (per port) in DB ,  used by 'Get' API to
        know how to parse the HW info..
    */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.sourcePortEntryTypeArr[portNum] = infoType;

    /* write the info to the table */
    return prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP6_TABLE_PHA_SOURCE_PHYSICAL_PORT_E,
        portNum,
        &hwValue);
}

/**
* @internal cpssDxChPhaSourcePortEntrySet function
* @endinternal
*
* @brief   Set per source physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - source physical port number.
* @param[in] infoType              - the type of source port info.
* @param[in] portInfoPtr           - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSourcePortEntrySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaSourcePortEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , infoType , portInfoPtr));

    rc = internal_cpssDxChPhaSourcePortEntrySet(devNum, portNum , infoType , portInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , infoType , portInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaSourcePortEntryGet function
* @endinternal
*
* @brief   Get per source physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - source physical port number.
* @param[out] infoTypePtr          - (pointer to) the type of source port info.
* @param[out] portInfoPtr          - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaSourcePortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  hwValue; /* value read from HW */
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT infoType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(infoTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(portInfoPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    /* read the info from the table */
    rc =  prvCpssDxChReadTableEntry(devNum,
        CPSS_DXCH_SIP6_TABLE_PHA_SOURCE_PHYSICAL_PORT_E,
        portNum,
        &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get the entry format from the DB */
    infoType = PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.sourcePortEntryTypeArr[portNum];
    *infoTypePtr = infoType;

    switch(infoType)
    {
        case CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNUSED_E:
            portInfoPtr->rawFormat = hwValue;/*not used*/
            break;
        case CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E:
            portInfoPtr->erspanSameDevMirror.erspanIndex = hwValue;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "unexpected value[%d] from DB for 'source port type'",
                infoType);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPhaSourcePortEntryGet function
* @endinternal
*
* @brief   Get per source physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - source physical port number.
* @param[out] infoTypePtr          - (pointer to) the type of source port info.
* @param[out] portInfoPtr          - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSourcePortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaSourcePortEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , infoTypePtr , portInfoPtr));

    rc = internal_cpssDxChPhaSourcePortEntryGet(devNum, portNum , infoTypePtr , portInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , infoTypePtr , portInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPhaTargetPortEntrySet function
* @endinternal
*
* @brief   Set per target physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - target physical port number.
* @param[in] infoType              - the type of target port info.
* @param[in] portInfoPtr           - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaTargetPortEntrySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_U32  hwValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(portInfoPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    switch(infoType)
    {
        case CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNUSED_E:
            hwValue = portInfoPtr->rawFormat;/* not used */
            break;
        case CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E:
            hwValue = portInfoPtr->erspanSameDevMirror.erspanIndex;
            if (hwValue > 0xFFFF )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                        "Erspan Index must be in range 0..65535 and not [%d] \n",
                        hwValue);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(infoType);
    }

    /* save the port type (per port) in DB ,  used by 'Get' API to
        know how to parse the HW info..
    */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.targetPortEntryTypeArr[portNum] = infoType;

    /* write the info to the table */
    return prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP6_TABLE_PHA_TARGET_PHYSICAL_PORT_E,
        portNum,
        &hwValue);
}

/**
* @internal cpssDxChPhaTargetPortEntrySet function
* @endinternal
*
* @brief   Set per target physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - target physical port number.
* @param[in] infoType              - the type of target port info.
* @param[in] portInfoPtr           - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaTargetPortEntrySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaTargetPortEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , infoType , portInfoPtr));

    rc = internal_cpssDxChPhaTargetPortEntrySet(devNum, portNum , infoType , portInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , infoType , portInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaTargetPortEntryGet function
* @endinternal
*
* @brief   Get per target physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - target physical port number.
* @param[out] infoTypePtr          - the type of target port info.
* @param[out] portInfoPtr          - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaTargetPortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  hwValue; /* value read from HW */
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT infoType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(infoTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(portInfoPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    /* read the info from the table */
    rc =  prvCpssDxChReadTableEntry(devNum,
        CPSS_DXCH_SIP6_TABLE_PHA_TARGET_PHYSICAL_PORT_E,
        portNum,
        &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get the entry format from the DB */
    infoType = PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.targetPortEntryTypeArr[portNum];
    *infoTypePtr = infoType;

    switch(infoType)
    {
        case CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNUSED_E:
            portInfoPtr->rawFormat = hwValue;/*not used */
            break;
        case CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E:
            portInfoPtr->erspanSameDevMirror.erspanIndex = hwValue;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "unexpected value[%d] from DB for 'target port type'",
                infoType);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPhaTargetPortEntryGet function
* @endinternal
*
* @brief   Get per target physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - target physical port number.
* @param[out] infoTypePtr          - the type of target port info.
* @param[out] portInfoPtr          - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaTargetPortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaTargetPortEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , infoTypePtr , portInfoPtr));

    rc = internal_cpssDxChPhaTargetPortEntryGet(devNum, portNum , infoTypePtr , portInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , infoTypePtr , portInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPhaHeaderModificationViolationInfoSet function
* @endinternal
*
* @brief   Set the packet Command and the Drop code for case of header modification
*           size violations of header pointers consistency checks
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum          - device number.
* @param[in] dropCode        - Drop code to attach to packets causing header modification
*                              size violations of header pointers consistency checks.
* @param[in] packetCommand   - Packet command to attach to packets violating header
*                              size checks or header pointer consistency checks
*                              Valid values : CPSS_PACKET_CMD_FORWARD_E /
*                                             CPSS_PACKET_CMD_DROP_HARD_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or dropCode or packetCommand
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaHeaderModificationViolationInfoSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    dropCode,
    IN  CPSS_PACKET_CMD_ENT         packetCommand
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;  /* DSA code */
    GT_U32  regAddr;
    GT_U32  hwValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    /* set bit 0 in the register */
    switch(packetCommand)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            hwValue = 0x0;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            hwValue = 0x1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(packetCommand);
    }


    /* convert SW cpu code to HW cpu code */
    rc = prvCpssDxChNetIfCpuToDsaCode(dropCode,&dsaCpuCode);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* set bits 1..8 in the register */
    hwValue |= dsaCpuCode << 1;

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.
        errorsAndInterrupts.headerModificationViolationConfig;

    return prvCpssHwPpSetRegField(devNum,regAddr,0,9,hwValue);
}

/**
* @internal cpssDxChPhaHeaderModificationViolationInfoSet function
* @endinternal
*
* @brief   Set the packet Command and the Drop code for case of header modification
*           size violations of header pointers consistency checks
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum          - device number.
* @param[in] dropCode        - Drop code to attach to packets causing header modification
*                              size violations of header pointers consistency checks.
* @param[in] packetCommand   - Packet command to attach to packets violating header
*                              size checks or header pointer consistency checks
*                              Valid values : CPSS_PACKET_CMD_FORWARD_E /
*                                             CPSS_PACKET_CMD_DROP_HARD_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or dropCode or packetCommand
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaHeaderModificationViolationInfoSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    dropCode,
    IN  CPSS_PACKET_CMD_ENT         packetCommand
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaHeaderModificationViolationInfoSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dropCode , packetCommand));

    rc = internal_cpssDxChPhaHeaderModificationViolationInfoSet(devNum, dropCode , packetCommand);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dropCode , packetCommand));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaHeaderModificationViolationInfoGet function
* @endinternal
*
* @brief   Get the packet Command and the Drop code for case of header modification
*           size violations of header pointers consistency checks
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum          - device number.
* @param[out] dropCodePtr    - (pointer to) Drop code to attach to packets causing header modification
*                              size violations of header pointers consistency checks.
* @param[out] packetCommandPtr - (pointer to) Packet command to attach to packets violating header
*                              size checks or header pointer consistency checks
*                              Valid values : CPSS_PACKET_CMD_FORWARD_E /
*                                             CPSS_PACKET_CMD_DROP_HARD_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaHeaderModificationViolationInfoGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *dropCodePtr,
    OUT CPSS_PACKET_CMD_ENT         *packetCommandPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;  /* DSA code */
    GT_U32  regAddr;
    GT_U32  hwValue; /* value read from the HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);

    CPSS_NULL_PTR_CHECK_MAC(dropCodePtr);
    CPSS_NULL_PTR_CHECK_MAC(packetCommandPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.
        errorsAndInterrupts.headerModificationViolationConfig;

    rc = prvCpssHwPpGetRegField(devNum,regAddr,0,9,&hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get bit 0 from the register */
    switch(hwValue & 0x1)
    {
        default:
        case 0x0:
            *packetCommandPtr = CPSS_PACKET_CMD_FORWARD_E;
            break;
        case 0x1:
            *packetCommandPtr = CPSS_PACKET_CMD_DROP_HARD_E;
            break;
    }

    /* get bits 1..8 from the register */
    dsaCpuCode = hwValue >> 1;

    /* convert HW cpu code to SW cpu code */
    rc = prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode,dropCodePtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPhaHeaderModificationViolationInfoGet function
* @endinternal
*
* @brief   Get the packet Command and the Drop code for case of header modification
*           size violations of header pointers consistency checks
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum          - device number.
* @param[out] dropCodePtr    - (pointer to) Drop code to attach to packets causing header modification
*                              size violations of header pointers consistency checks.
* @param[out] packetCommandPtr - (pointer to) Packet command to attach to packets violating header
*                              size checks or header pointer consistency checks
*                              Valid values : CPSS_PACKET_CMD_FORWARD_E /
*                                             CPSS_PACKET_CMD_DROP_HARD_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaHeaderModificationViolationInfoGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *dropCodePtr,
    OUT CPSS_PACKET_CMD_ENT         *packetCommandPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaHeaderModificationViolationInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dropCodePtr , packetCommandPtr));

    rc = internal_cpssDxChPhaHeaderModificationViolationInfoGet(devNum, dropCodePtr , packetCommandPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dropCodePtr , packetCommandPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaHeaderModificationViolationCapturedGet function
* @endinternal
*
* @brief  Get the thread ID of the first header size violating packet , and the violation type.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum          - device number.
* @param[out] capturedThreadIdPtr   - (pointer to) The thread Id of the first header size violating packet
* @param[out] violationTypePtr      - (pointer to) Type describes which exact header size violation occurred
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NO_MORE               - when no more modification violations to get info about
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaHeaderModificationViolationCapturedGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                     *capturedThreadIdPtr,
    OUT CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT  *violationTypePtr
)
{
    GT_STATUS   rc;
    GT_U32  portGroupId; /*the port group Id - support multi-port-groups device */
    GT_U32  portGroupsBmp; /* port groups to query - support multi-port-groups device */
    GT_U32  regAddr;
    GT_U32  hwValue; /* value read from the HW */
    GT_BOOL didCoverOnAllInstances = GT_FALSE;
    GT_U32  first_portGroupId, next_portGroupId;
    GT_U32  hw_ThreadId;
    GT_U32  hw_reason;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);

    CPSS_NULL_PTR_CHECK_MAC(capturedThreadIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(violationTypePtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    first_portGroupId =
        PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.phaViolationCapturedNextPortGroupToServe;

    if(0 == PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        portGroupId = first_portGroupId;
        didCoverOnAllInstances = GT_TRUE;/* single instance */
    }
    else
    {
        tryInfoOnNextPortGroup_lbl:
        portGroupId =
            PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.phaViolationCapturedNextPortGroupToServe;

        /* start with bmp of all active port groups */
        portGroupsBmp = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp;


        /* look for port group to query */
        while(0 == (portGroupsBmp & (1 << portGroupId)))
        {
            if(portGroupId > PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.lastActivePortGroup)
            {
                portGroupId = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.firstActivePortGroup;
                break;
            }

            portGroupId++;
        }

        next_portGroupId = portGroupId + 1;
        /* calc the 'next valid port group' ... to be saved at <phaViolationCapturedNextPortGroupToServe> */
        {
            /* look for port group to query */
            while(0 == (portGroupsBmp & (1 << next_portGroupId)))
            {
                if(next_portGroupId > PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.lastActivePortGroup)
                {
                    next_portGroupId = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.firstActivePortGroup;
                    break;
                }

                next_portGroupId++;
            }
        }

        /* save the info about the next port group to be served */
        PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.phaViolationCapturedNextPortGroupToServe =
            next_portGroupId;

        if(first_portGroupId == next_portGroupId)
        {
            /* next one to serve is the one that we started with */
            didCoverOnAllInstances = GT_TRUE;/* indicate that no more instances to check */
        }

    }

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.
        errorsAndInterrupts.headerModificationViolationAttributes;

    rc = prvCpssHwPpPortGroupGetRegField(devNum,portGroupId,regAddr,0,10,&hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    hw_ThreadId =  hwValue >> 2;/* bits 2..9 */
    hw_reason   =  hwValue & 0x3;/*bits 0..1 */

    if(hw_ThreadId == 0) /* indication that there is no error in this instance */
    {
        if(didCoverOnAllInstances == GT_FALSE)
        {
            /* support multi instances of the PHA unit (one per port group) */
            goto tryInfoOnNextPortGroup_lbl;
        }

        *capturedThreadIdPtr = 0;  /* dummy */
        *violationTypePtr    = 0;  /* dummy */

        return /* not error for the ERROR LOG */ GT_NO_MORE;
    }

    *capturedThreadIdPtr = hw_ThreadId;

    switch(hw_reason)
    {
        case 0:
            *violationTypePtr = CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_FW_INC_OVERSIZE_E;
            break;
        case 1:
            *violationTypePtr = CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_HEADER_OVERSIZE_E;
            break;
        case 2:
            *violationTypePtr = CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_FW_DEC_VIOLATION_E;
            break;
        case 3:
        default:
            *violationTypePtr = CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_HEADER_UNDERSIZE_E;
            break;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPhaHeaderModificationViolationCapturedGet function
* @endinternal
*
* @brief  Get the thread ID of the first header size violating packet , and the violation type.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum          - device number.
* @param[out] capturedThreadIdPtr   - (pointer to) The thread Id of the first header size violating packet
* @param[out] violationTypePtr      - (pointer to) Type describes which exact header size violation occurred
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NO_MORE               - when no more modification violations to get info about
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaHeaderModificationViolationCapturedGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                     *capturedThreadIdPtr,
    OUT CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT  *violationTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaHeaderModificationViolationCapturedGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, capturedThreadIdPtr , violationTypePtr));

    rc = internal_cpssDxChPhaHeaderModificationViolationCapturedGet(devNum, capturedThreadIdPtr , violationTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, capturedThreadIdPtr , violationTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPhaStatisticalProcessingCounterThreadIdSet function
* @endinternal
*
* @brief   set the PHA thread-Id to count it's statistical processing cancellations.
*           When zero, all PHA threads are counted (i.e. non-zero).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] phaThreadId              - the associated thread-Id
*                                      (APPLICABLE RANGE: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or phaThreadId
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPhaStatisticalProcessingCounterThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_U32       phaThreadId
)
{
    GT_U32  regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    if(phaThreadId == 0)
    {
        /* this is the only API that we supports '0' as input from the
           application about phaThreadId */
    }
    else
    {
        /* validate the phaThreadId */
        PRV_CPSS_DXCH_PHA_THREAD_ID_CHECK_MAC(devNum,phaThreadId);
    }

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.
        debugCounters.statisticalProcessingDebugCounterConfig;

    return prvCpssHwPpSetRegField(devNum,regAddr,0,8,phaThreadId);
}

/**
* @internal cpssDxChPhaStatisticalProcessingCounterThreadIdSet function
* @endinternal
*
* @brief   set the PHA thread-Id to count it's statistical processing cancellations.
*           When zero, all PHA threads are counted (i.e. non-zero).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] phaThreadId              - the associated thread-Id
*                                      (APPLICABLE RANGE: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or phaThreadId
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaStatisticalProcessingCounterThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_U32       phaThreadId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaStatisticalProcessingCounterThreadIdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, phaThreadId));

    rc = internal_cpssDxChPhaStatisticalProcessingCounterThreadIdSet(devNum, phaThreadId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, phaThreadId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaStatisticalProcessingCounterThreadIdGet function
* @endinternal
*
* @brief   get the PHA thread-Id to count it's statistical processing cancellations.
*           When zero, all PHA threads are counted (i.e. non-zero).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[out] phaThreadIdPtr           - (pointer to) the associated thread-Id
*                                      (APPLICABLE RANGE: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPhaStatisticalProcessingCounterThreadIdGet
(
    IN  GT_U8        devNum,
    OUT GT_U32       *phaThreadIdPtr
)
{
    GT_U32  regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);

    CPSS_NULL_PTR_CHECK_MAC(phaThreadIdPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.
        debugCounters.statisticalProcessingDebugCounterConfig;

    return prvCpssHwPpGetRegField(devNum,regAddr,0,8,phaThreadIdPtr);
}

/**
* @internal cpssDxChPhaStatisticalProcessingCounterThreadIdGet function
* @endinternal
*
* @brief   get the PHA thread-Id to count it's statistical processing cancellations.
*           When zero, all PHA threads are counted (i.e. non-zero).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[out] phaThreadIdPtr           - (pointer to) the associated thread-Id
*                                      (APPLICABLE RANGE: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaStatisticalProcessingCounterThreadIdGet
(
    IN  GT_U8        devNum,
    OUT GT_U32       *phaThreadIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaStatisticalProcessingCounterThreadIdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, phaThreadIdPtr));

    rc = internal_cpssDxChPhaStatisticalProcessingCounterThreadIdGet(devNum, phaThreadIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, phaThreadIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaStatisticalProcessingCounterGet function
* @endinternal
*
* @brief   Get the PHA counter value of statistical processing cancellations.
*           Note: the counter is cleared after read (by HW).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[out] counterPtr              - (pointer)the counter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPhaStatisticalProcessingCounterGet
(
    IN GT_U8        devNum,
    OUT GT_U64       *counterPtr
)
{
    GT_U32  regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    PHA_UNIT_CHECK(devNum);

    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.
        debugCounters.statisticalProcessingDebugCounter;

    return prvCpssPortGroupsCounterSummary(devNum,regAddr,0,20,NULL,counterPtr);
}

/**
* @internal cpssDxChPhaStatisticalProcessingCounterGet function
* @endinternal
*
* @brief   Get the PHA counter value of statistical processing cancellations.
*           Note: the counter is cleared after read (by HW).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[out] counterPtr              - (pointer)the counter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaStatisticalProcessingCounterGet
(
    IN GT_U8        devNum,
    OUT GT_U64       *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaStatisticalProcessingCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, counterPtr));

    rc = internal_cpssDxChPhaStatisticalProcessingCounterGet(devNum, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal parseErspanIpv6Template function
* @endinternal
*
* @brief  parse 16 words of ERSPAN l2 header and ipv6 thread template from the info
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] dataPtr[16]         - the 16 words that hold the needed ERSPAN l2 header and ipv6 header format
*
* @param[out] erspanIpv6Ptr      - (pointer to) ERSPAN l2 header and ipv6 header info.
*
* @retval GT_OK               - on success
*/
static GT_STATUS parseErspanIpv6Template
(
    IN  GT_U32                                                dataPtr[/*16*/],
    OUT CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC      *erspanIpv6Ptr
)
{
    erspanIpv6Ptr->ipInfo.ipv6.tc               = U32_GET_FIELD_MAC(dataPtr[0],  20, 8);
    erspanIpv6Ptr->ipInfo.ipv6.flowLabel        = U32_GET_FIELD_MAC(dataPtr[0],  0, 20);

    erspanIpv6Ptr->ipInfo.ipv6.hopLimit         = U32_GET_FIELD_MAC(dataPtr[1],  0,   8);

    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[0]  = U32_GET_FIELD_MAC(dataPtr[2],  24,  8);
    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[1]  = U32_GET_FIELD_MAC(dataPtr[2],  16,  8);
    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[2]  = U32_GET_FIELD_MAC(dataPtr[2],   8,  8);
    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[3]  = U32_GET_FIELD_MAC(dataPtr[2],   0,  8);

    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[4]  = U32_GET_FIELD_MAC(dataPtr[3],  24,  8);
    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[5]  = U32_GET_FIELD_MAC(dataPtr[3],  16,  8);
    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[6]  = U32_GET_FIELD_MAC(dataPtr[3],   8,  8);
    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[7]  = U32_GET_FIELD_MAC(dataPtr[3],   0,  8);

    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[8]  = U32_GET_FIELD_MAC(dataPtr[4],  24,  8);
    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[9]  = U32_GET_FIELD_MAC(dataPtr[4],  16,  8);
    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[10] = U32_GET_FIELD_MAC(dataPtr[4],   8,  8);
    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[11] = U32_GET_FIELD_MAC(dataPtr[4],   0,  8);

    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[12] = U32_GET_FIELD_MAC(dataPtr[5],  24,  8);
    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[13] = U32_GET_FIELD_MAC(dataPtr[5],  16,  8);
    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[14] = U32_GET_FIELD_MAC(dataPtr[5],   8,  8);
    erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[15] = U32_GET_FIELD_MAC(dataPtr[5],   0,  8);

    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[0]  = U32_GET_FIELD_MAC(dataPtr[6],  24,  8);
    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[1]  = U32_GET_FIELD_MAC(dataPtr[6],  16,  8);
    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[2]  = U32_GET_FIELD_MAC(dataPtr[6],   8,  8);
    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[3]  = U32_GET_FIELD_MAC(dataPtr[6],   0,  8);

    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[4]  = U32_GET_FIELD_MAC(dataPtr[7],  24,  8);
    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[5]  = U32_GET_FIELD_MAC(dataPtr[7],  16,  8);
    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[6]  = U32_GET_FIELD_MAC(dataPtr[7],   8,  8);
    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[7]  = U32_GET_FIELD_MAC(dataPtr[7],   0,  8);

    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[8]  = U32_GET_FIELD_MAC(dataPtr[8],  24,  8);
    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[9]  = U32_GET_FIELD_MAC(dataPtr[8],  16,  8);
    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[10] = U32_GET_FIELD_MAC(dataPtr[8],   8,  8);
    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[11] = U32_GET_FIELD_MAC(dataPtr[8],   0,  8);

    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[12] = U32_GET_FIELD_MAC(dataPtr[9],  24,  8);
    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[13] = U32_GET_FIELD_MAC(dataPtr[9],  16,  8);
    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[14] = U32_GET_FIELD_MAC(dataPtr[9],   8,  8);
    erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[15] = U32_GET_FIELD_MAC(dataPtr[9],   0,  8);

    erspanIpv6Ptr->ingressSessionId             = U32_GET_FIELD_MAC(dataPtr[10],  10, 10);
    erspanIpv6Ptr->egressSessionId              = U32_GET_FIELD_MAC(dataPtr[10],   0, 10);

    parseErspanL2Template(&dataPtr[11], erspanIpv6Ptr);

    return GT_OK;
}

/**
* @internal parseErspanIpv4Template function
* @endinternal
*
* @brief  parse 16 words of ERSPAN l2 header abd ipv4 thread template from the info
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] dataPtr[16]         - the 16 words that hold the needed ERSPAN l2 header and ipv4 header format
*
* @param[out] erspanIpv4Ptr      - (pointer to) ERSPAN l2 header and ipv4 header info.
*
* @retval GT_OK               - on success
*/
static GT_STATUS parseErspanIpv4Template
(
    IN  GT_U32                                                dataPtr[/*12*/],
    OUT CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC      *erspanIpv4Ptr
)
{

    erspanIpv4Ptr->ipInfo.ipv4.dscp             = U32_GET_FIELD_MAC(dataPtr[0],  18, 6);

    erspanIpv4Ptr->ipInfo.ipv4.flags            = U32_GET_FIELD_MAC(dataPtr[1],  13,  3);

    erspanIpv4Ptr->ipInfo.ipv4.ttl              = U32_GET_FIELD_MAC(dataPtr[2],  24,  8);

    erspanIpv4Ptr->ipInfo.ipv4.sipAddr.arIP[0]  = U32_GET_FIELD_MAC(dataPtr[3],  24,  8);
    erspanIpv4Ptr->ipInfo.ipv4.sipAddr.arIP[1]  = U32_GET_FIELD_MAC(dataPtr[3],  16,  8);
    erspanIpv4Ptr->ipInfo.ipv4.sipAddr.arIP[2]  = U32_GET_FIELD_MAC(dataPtr[3],   8,  8);
    erspanIpv4Ptr->ipInfo.ipv4.sipAddr.arIP[3]  = U32_GET_FIELD_MAC(dataPtr[3],   0,  8);

    erspanIpv4Ptr->ipInfo.ipv4.dipAddr.arIP[0]  = U32_GET_FIELD_MAC(dataPtr[4],  24,  8);
    erspanIpv4Ptr->ipInfo.ipv4.dipAddr.arIP[1]  = U32_GET_FIELD_MAC(dataPtr[4],  16,  8);
    erspanIpv4Ptr->ipInfo.ipv4.dipAddr.arIP[2]  = U32_GET_FIELD_MAC(dataPtr[4],   8,  8);
    erspanIpv4Ptr->ipInfo.ipv4.dipAddr.arIP[3]  = U32_GET_FIELD_MAC(dataPtr[4],   0,  8);

    erspanIpv4Ptr->ingressSessionId             = U32_GET_FIELD_MAC(dataPtr[5],  10, 10);
    erspanIpv4Ptr->egressSessionId              = U32_GET_FIELD_MAC(dataPtr[5],   0, 10);

    parseErspanL2Template(&dataPtr[6], erspanIpv4Ptr);

    return GT_OK;
}

/**
* @internal buildErspanIpv6Template function
* @endinternal
*
* @brief  build 16 words of erspan l2 header and ipv6 header from the info
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] erspanPtr       - (pointer to) the erspan l2 header and ipv6 header info.
*
* @param[out] dataPtr[16]    - the 16 words that hold the needed ERSPAN l2 header and ipv6 header format
*
* @retval GT_OK              - on success
* @retval GT_OUT_OF_RANGE    - on out of range Flow Label
*/
static GT_STATUS buildErspanIpv6Template
(
    IN  CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC      *erspanIpv6Ptr,
    OUT GT_U32                                                dataPtr[/*16*/]
)
{
    GT_STATUS rc = GT_OK;

    cpssOsMemSet(&dataPtr[0], 0, sizeof(GT_U32)*PRV_CPSS_DXCH_PPA_THREAD_CONF_WORDS_CNS);

    /* Check parameter is valid */
    CPSS_DATA_CHECK_MAX_MAC(erspanIpv6Ptr->ipInfo.ipv6.flowLabel, BIT_20);

    dataPtr[0] = (0x6 << 28)/* ipv6 version */ |
                 (erspanIpv6Ptr->ipInfo.ipv6.tc << 20) |
                  erspanIpv6Ptr->ipInfo.ipv6.flowLabel;

    /* dataPtr[1] bits [31:16] = 0: ipv6 payload length written by FW thread */
    dataPtr[1] =  (0x2F << 8)/* ipv6 next header */ |
                  erspanIpv6Ptr->ipInfo.ipv6.hopLimit;

    dataPtr[2] = (erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[0] << 24) |
                 (erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[1] << 16) |
                 (erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[2] << 8) |
                  erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[3];

    dataPtr[3] = (erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[4] << 24) |
                 (erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[5] << 16) |
                 (erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[6] << 8) |
                  erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[7];

    dataPtr[4] = (erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[8] << 24) |
                 (erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[9] << 16) |
                 (erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[10] << 8) |
                  erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[11];

    dataPtr[5] = (erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[12] << 24) |
                 (erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[13] << 16) |
                 (erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[14] << 8) |
                  erspanIpv6Ptr->ipInfo.ipv6.sipAddr.arIP[15];

    dataPtr[6] = (erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[0] << 24) |
                 (erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[1] << 16) |
                 (erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[2] << 8) |
                  erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[3];

    dataPtr[7] = (erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[4] << 24) |
                 (erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[5] << 16) |
                 (erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[6] << 8) |
                  erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[7];

    dataPtr[8] = (erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[8] << 24) |
                 (erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[9] << 16) |
                 (erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[10] << 8) |
                  erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[11];

    dataPtr[9] = (erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[12] << 24) |
                 (erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[13] << 16) |
                 (erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[14] << 8) |
                  erspanIpv6Ptr->ipInfo.ipv6.dipAddr.arIP[15];

    /* ERSPAN ingress session id bits [19:10], egress session id bits [9:0] */
    dataPtr[10] = (erspanIpv6Ptr->ingressSessionId << 10) |
                  (erspanIpv6Ptr->egressSessionId);

    /* L2 parameters:  dataPtr[11:13] = | mac da | mac sa |
                       dataPtr[14]    = | tpid | up | cfi | vid | */
    rc = buildErspanL2Template(erspanIpv6Ptr, &dataPtr[11]);

    return rc;
}

/**
* @internal buildErspanIpv4Template function
* @endinternal
*
* @brief  build 16 words of erspan l2 header and ipv4 header from the info
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] erspanPtr       - (pointer to) the erspan l2 header and ipv4 header info.
*
* @param[out] dataPtr[16]     - the 16 words that hold the needed ERSPAN l2 header and ipv4 header format
*
* @retval GT_OK              - on success
* @retval GT_BAD_PARAM       - on wrong flags input
* @retval GT_OUT_OF_RANGE    - on out of range DSCP/Flags
*/
static GT_STATUS buildErspanIpv4Template
(
    IN  CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC      *erspanIpv4Ptr,
    OUT GT_U32                                                dataPtr[/*16*/]
)
{
    GT_STATUS rc = GT_OK;

    cpssOsMemSet(&dataPtr[0], 0, sizeof(GT_U32)*PRV_CPSS_DXCH_PPA_THREAD_CONF_WORDS_CNS);

    /* Check parameter is valid */
    CPSS_DATA_CHECK_MAX_MAC(erspanIpv4Ptr->ipInfo.ipv4.dscp,  BIT_6);
    CPSS_DATA_CHECK_MAX_MAC(erspanIpv4Ptr->ipInfo.ipv4.flags, BIT_3);

    /* dataPtr[0] bits [17:16] = 0: ipv4 ecn
                  bits [15:0]  = 0: ipv4 total length written by FW thread */
    dataPtr[0] = (0x4 << 28) /* ipv4 version */|
                 (0x5 << 24) /* ipv4 header length */|
                 (erspanIpv4Ptr->ipInfo.ipv4.dscp << 18);

    /* dataPtr[1] bits [31:16] = 0: ipv4 identification
                  bits [12:0]  = 0: ipv4 fragment offset */
    dataPtr[1] =  (erspanIpv4Ptr->ipInfo.ipv4.flags << 13);

    /* dataPtr[2] bits [15:0]  = 0: ipv4 header checksum written by FW thread */
    dataPtr[2] = (erspanIpv4Ptr->ipInfo.ipv4.ttl << 24) |
                 (0x2F << 16);

    dataPtr[3] = (erspanIpv4Ptr->ipInfo.ipv4.sipAddr.arIP[0] << 24) |
                 (erspanIpv4Ptr->ipInfo.ipv4.sipAddr.arIP[1] << 16) |
                 (erspanIpv4Ptr->ipInfo.ipv4.sipAddr.arIP[2] << 8) |
                  erspanIpv4Ptr->ipInfo.ipv4.sipAddr.arIP[3];

    dataPtr[4] = (erspanIpv4Ptr->ipInfo.ipv4.dipAddr.arIP[0] << 24) |
                 (erspanIpv4Ptr->ipInfo.ipv4.dipAddr.arIP[1] << 16) |
                 (erspanIpv4Ptr->ipInfo.ipv4.dipAddr.arIP[2] << 8) |
                  erspanIpv4Ptr->ipInfo.ipv4.dipAddr.arIP[3];

    /* ERSPAN ingress session id bits [19:10], egress session id bits [9:0] */
    dataPtr[5]  = (erspanIpv4Ptr->ingressSessionId << 10) |
                  (erspanIpv4Ptr->egressSessionId);


    /* L2 parameters:  dataPtr[6:8] = | mac da | mac sa |
                       dataPtr[9]   = | tpid | up | cfi | vid | */
    rc = buildErspanL2Template(erspanIpv4Ptr, &dataPtr[6]);

    return rc;
}

/**
* @internal internal_cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet function
* @endinternal
*
* @brief   Set the ERSPAN entry in the PHA shared memory table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] analyzerIndex         - analyzer index.
*                                   (APPLICABLE RANGE: 0..6)
* @param[in] entryPtr              - (pointer to) thread template in PHA shared memory
*                                    NOTE: this is the template that is needed by the firmware thread
*                                    for IP header and Session ID for ERSPAN packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong analyzerIndex or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet
(
    IN GT_U8                                               devNum,
    IN GT_U32                                              analyzerIndex,
    IN CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC    *entryPtr
)
{
    GT_STATUS                                                           rc = GT_OK;
    GT_U32                                                              PPAThreadsConfWords[PRV_CPSS_DXCH_PPA_THREAD_CONF_WORDS_CNS];/*shared memory table: 16 words */
    GT_U32                                                              entryIndex;
    CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC                    *erspanPtr;
    GT_U32                                                              iter;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PHA_UNIT_CHECK(devNum);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    if(analyzerIndex >= PRV_CPSS_DXCH_PHA_THREAD_ANALYZER_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "analyzerIndex must be in range 0-6 and not [%d] \n",
            analyzerIndex);
    }

    erspanPtr = entryPtr;
    CPSS_DATA_CHECK_MAX_MAC(erspanPtr->ingressSessionId, BIT_10);
    CPSS_DATA_CHECK_MAX_MAC(erspanPtr->egressSessionId,  BIT_10);

    /**************************************/
    /* start to :                         */
    /* check validity of INPUT parameters */
    /* and convert to HW values           */
    /**************************************/

    cpssOsMemSet(PPAThreadsConfWords, 0, sizeof(PPAThreadsConfWords));

    if(entryPtr->protocol == CPSS_IP_PROTOCOL_IPV4_E)
    {
        /* check and build specific info */
        rc = buildErspanIpv4Template(erspanPtr, &PPAThreadsConfWords[0]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else if(entryPtr->protocol == CPSS_IP_PROTOCOL_IPV6_E)
    {
        /* check and build specific info */
        rc = buildErspanIpv6Template(erspanPtr, &PPAThreadsConfWords[0]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "IP protocol stack [%d] not valid \n",
            entryPtr->protocol);
    }

    entryIndex = analyzerIndex * PRV_CPSS_DXCH_PHA_THREAD_ANALYZER_INDEX_TEMPLATE_WORD_CNS;

    /* set the entry in shared memory table indexed by the analyzer location */
    for(iter = 0; iter < PRV_CPSS_DXCH_PPA_THREAD_CONF_WORDS_CNS; iter++)
    {
        rc = prvCpssDxChWriteTableEntry(devNum,CPSS_DXCH_SIP6_TABLE_PHA_SHARED_DMEM_E,
            (entryIndex+iter),&PPAThreadsConfWords[iter]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet function
* @endinternal
*
* @brief   Set the ERSPAN entry in the PHA shared memory table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerIndex            - analyzer index.
*                                      (APPLICABLE RANGE: 0..6)
* @param[in] entryPtr                 - (pointer to) thread template in PHA shared memory
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      for IP header and Session ID for ERSPAN packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong analyzerIndex or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet
(
    IN GT_U8                                               devNum,
    IN GT_U32                                              analyzerIndex,
    IN CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC    *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, analyzerIndex , entryPtr));

    rc = internal_cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(devNum, analyzerIndex , entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, analyzerIndex , entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet function
* @endinternal
*
* @brief   Get the ERSPAN entry from the PHA shared memory table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] analyzerIndex         - analyzer index.
*                                   (APPLICABLE RANGE: 0..6)
* @param[out] entryPtr             - (pointer to) thread template in PHA shared memory
*                                    NOTE: this is the template that is needed by the firmware thread
*                                    for IP header and Session ID for ERSPAN packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong analyzerIndex or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet
(
    IN  GT_U8                                               devNum,
    IN  GT_U32                                              analyzerIndex,
    OUT CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC    *entryPtr
)
{
    GT_STATUS                                                        rc = GT_OK;
    GT_U32                                                           PPAThreadsConfWords[PRV_CPSS_DXCH_PPA_THREAD_CONF_WORDS_CNS];   /* shared memory table: 16 words */
    GT_U32                                                           entryIndex;
    CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC                 *erspanPtr;
    GT_U32                                                           iter;
    GT_U32                                                           protocol;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PHA_UNIT_CHECK(devNum);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    if(analyzerIndex >= PRV_CPSS_DXCH_PHA_THREAD_ANALYZER_INDEX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "analyzerIndex must be in range 0-6 and not [%d] \n",
            analyzerIndex);
    }

    /* reset the 'union' to allow application to compare 'set' values with 'get' values */
    cpssOsMemSet(PPAThreadsConfWords,0,sizeof(PRV_CPSS_DXCH_PPA_THREAD_CONF_WORDS_CNS));
    entryIndex = analyzerIndex * PRV_CPSS_DXCH_PHA_THREAD_ANALYZER_INDEX_TEMPLATE_WORD_CNS;

    /* get the entry from shared memory table */
    for(iter = 0; iter < PRV_CPSS_DXCH_PPA_THREAD_CONF_WORDS_CNS; iter++)
    {
        rc = prvCpssDxChReadTableEntry(devNum,CPSS_DXCH_SIP6_TABLE_PHA_SHARED_DMEM_E,
            (entryIndex+iter),&PPAThreadsConfWords[iter]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    erspanPtr = entryPtr;

    protocol = U32_GET_FIELD_MAC(PPAThreadsConfWords[0], 28, 4);

    if(protocol == 0x4 /*CPSS_IP_PROTOCOL_IPV4_E*/)
    {
        /* parse HW info to SW info */
        rc = parseErspanIpv4Template(&PPAThreadsConfWords[0], erspanPtr);
        erspanPtr->protocol = CPSS_IP_PROTOCOL_IPV4_E;
    }
    else /*CPSS_IP_PROTOCOL_IPV6_E*/
    {
        /* parse HW info to SW info */
        rc = parseErspanIpv6Template(&PPAThreadsConfWords[0], erspanPtr);
        erspanPtr->protocol = CPSS_IP_PROTOCOL_IPV6_E;
    }

    return rc;
}

/**
* @internal cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet function
* @endinternal
*
* @brief   Get the ERSPAN entry from the PHA shared memory table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                    - device number.
* @param[in] analyzerIndex             - analyzer index.
*                                       (APPLICABLE RANGE: 0..6)
* @param[out] entryPtr                 - (pointer to) thread template in PHA shared memory
*                                       NOTE: this is the template that is needed by the firmware thread
*                                       for IP header and Session ID for ERSPAN packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong analyzerIndex or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet
(
    IN  GT_U8                                               devNum,
    IN  GT_U32                                              analyzerIndex,
    OUT CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC    *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, analyzerIndex , entryPtr));

    rc = internal_cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet(devNum, analyzerIndex , entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, analyzerIndex , entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaSharedMemoryErspanGlobalConfigSet function
* @endinternal
*
* @brief   Set the ERSPAN device id and VoQ in the PHA shared memory table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] erspanDevId              - ERSPAN device id.
*                                      (APPLICABLE RANGE: 0..7)
* @param[in] isVoQ                    - ERSPAN egress LC port working with/without VoQ.
*                                      GT_TRUE  - LC port working with VoQ
*                                      GT_FALSE - LC port working without VoQ
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChPhaSharedMemoryErspanGlobalConfigSet
(
    IN GT_U8                                               devNum,
    IN GT_U16                                              erspanDevId,
    IN GT_BOOL                                             isVoQ
)
{
    GT_STATUS  rc;
    GT_U32     entryIndex; /* index in the SHARED_DMEM table */
    GT_U32     entryVal = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PHA_UNIT_CHECK(devNum);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

    if(erspanDevId >= BIT_3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "ERSPAN Device id must be in range 0-7 and not [%d] \n",
            erspanDevId);
    }

    /* ERSPAN Device ID is written into PHA shared memory after all the analyzer indexes IP header template.
       There are total of 7 analyzer indexes supported and each index occupies 64B in the shared memory.
     */
    entryIndex = PRV_CPSS_DXCH_PHA_THREAD_ANALYZER_INDEX_CNS * PRV_CPSS_DXCH_PHA_THREAD_ANALYZER_INDEX_TEMPLATE_WORD_CNS;

    /* Locate erspanDevId & isVoQ fields in 32 bits word as expected by firmware */
    entryVal = erspanDevId << 16;
    entryVal |= (isVoQ == GT_TRUE) ? 0x100 : 0x0;

    /* set the device id into the shared memory table */
    rc = prvCpssDxChWriteTableEntry(devNum,CPSS_DXCH_SIP6_TABLE_PHA_SHARED_DMEM_E,
        entryIndex,&entryVal);

    return rc;

}

/**
* @internal cpssDxChPhaSharedMemoryErspanGlobalConfigSet function
* @endinternal
*
* @brief   Set the ERSPAN device id and VoQ in the PHA shared memory table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] erspanDevId              - ERSPAN device id.
*                                      (APPLICABLE RANGE: 0..7)
* @param[in] isVoQ                    - ERSPAN egress LC port working with/without VoQ.
*                                      GT_TRUE  - LC port working with VoQ
*                                      GT_FALSE - LC port working without VoQ
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSharedMemoryErspanGlobalConfigSet
(
    IN GT_U8                                               devNum,
    IN GT_U16                                              erspanDevId,
    IN GT_BOOL                                             isVoQ
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaSharedMemoryErspanGlobalConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, erspanDevId, isVoQ));

    rc = internal_cpssDxChPhaSharedMemoryErspanGlobalConfigSet(devNum, erspanDevId, isVoQ);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, erspanDevId, isVoQ));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaSharedMemoryErspanGlobalConfigGet function
* @endinternal
*
* @brief   Get the ERSPAN device id and VoQ from the PHA shared memory table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[out] erspanDevIdPtr          - (pointer to) ERSPAN device id.
* @param[out] isVoQPtr                - (pointer to) ERSPAN egress LC port working with/without VoQ.
*                                      GT_TRUE  - LC port working with VoQ
*                                      GT_FALSE - LC port working without VoQ
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChPhaSharedMemoryErspanGlobalConfigGet
(
    IN  GT_U8                                               devNum,
    OUT GT_U16                                              *erspanDevIdPtr,
    OUT GT_BOOL                                             *isVoQPtr
)
{
    GT_STATUS  rc;
    GT_U32     entryIndex; /* index in the SHARED_DMEM table */
    GT_U32     entryVal;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PHA_UNIT_CHECK(devNum);

    /**************************************/
    /* check that the LIB was initialized */
    /**************************************/
    PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(erspanDevIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(isVoQPtr);

    /* ERSPAN Device ID is written into PHA shared memory after all the analyzer indexes IP header template.
       There are total of 7 analyzer indexes supported and each index occupies 64B in the shared memory.
     */
    entryIndex = PRV_CPSS_DXCH_PHA_THREAD_ANALYZER_INDEX_CNS * PRV_CPSS_DXCH_PHA_THREAD_ANALYZER_INDEX_TEMPLATE_WORD_CNS;

    /* Get the device id from the shared memory table */
    rc = prvCpssDxChReadTableEntry(devNum,CPSS_DXCH_SIP6_TABLE_PHA_SHARED_DMEM_E,
        entryIndex,&entryVal);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Extract ERSPAN device ID & VoQ indication */
    *erspanDevIdPtr = (entryVal >> 16) & 0x7;
    *isVoQPtr = (((entryVal >> 8) & 0xFF) == 0x1) ? GT_TRUE : GT_FALSE;

    return rc;

}

/**
* @internal cpssDxChPhaSharedMemoryErspanGlobalConfigGet function
* @endinternal
*
* @brief   Get the ERSPAN device id and VoQ from the PHA shared memory table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[out] erspanDevIdPtr          - (pointer to) ERSPAN device id.
* @param[out] isVoQPtr                - (pointer to) ERSPAN egress LC port working with/without VoQ.
*                                      GT_TRUE  - LC port working with VoQ
*                                      GT_FALSE - LC port working without VoQ
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSharedMemoryErspanGlobalConfigGet
(
    IN  GT_U8                                               devNum,
    OUT GT_U16                                              *erspanDevIdPtr,
    OUT GT_BOOL                                             *isVoQPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaSharedMemoryErspanGlobalConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, erspanDevIdPtr, isVoQPtr));

    rc = internal_cpssDxChPhaSharedMemoryErspanGlobalConfigGet(devNum, erspanDevIdPtr, isVoQPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, erspanDevIdPtr, isVoQPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaErrorsConfigSet function
* @endinternal
*
* @brief   Set PHA error code configurations
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;
*                                  AC3X; Bobcat3; Aldrin2; Ironman
*
* @param[in] devNum                   - device number
* @param[in] errorCodePtr             - (pointer to) PHA error code configuration
*
* @retval GT_OK                       - on success
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on wrong device or errorCode
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaErrorsConfigSet
(
    IN  GT_U8                                  devNum,
    IN  CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC    *errorCodePtr
)
{
    GT_STATUS                               rc;
    GT_U32                                  regAddr; /* Register address */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  dsaCpuCode; /* DSA CPU code */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                        | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(errorCodePtr);

    /* Convert dropCode to dsaCpuCode */
    rc = prvCpssDxChNetIfCpuToDsaCode(errorCodePtr->tablesReadErrorDropCode, &dsaCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get register address and configure tables read error drop code */
    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.errorsAndInterrupts.tablesReadErrorConfig;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 1, 8, (GT_U32)dsaCpuCode);

    /* Convert dropCode to dsaCpuCode */
    rc = prvCpssDxChNetIfCpuToDsaCode(errorCodePtr->ppaClockDownErrorDropCode, &dsaCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get register address and configure PPA clock down error drop code */
    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.errorsAndInterrupts.PPAClockDownErrorConfig;
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 8, (GT_U32)dsaCpuCode);
}
/**
* @internal cpssDxChPhaErrorsConfigSet function
* @endinternal
*
* @brief   Set PHA error code configurations
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;
*                                  AC3X; Bobcat3; Aldrin2; Ironman
*
* @param[in] devNum                   - device number
* @param[in] errorCodePtr             - (pointer to) PHA error code configuration
*
* @retval GT_OK                       - on success
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on wrong device or errorCode
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*/
GT_STATUS cpssDxChPhaErrorsConfigSet
(
    IN  GT_U8                                  devNum,
    IN  CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC    *errorCodePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaErrorsConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, errorCodePtr));

    rc = internal_cpssDxChPhaErrorsConfigSet(devNum, errorCodePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, errorCodePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaErrorsConfigGet function
* @endinternal
*
* @brief   Get PHA error code configurations
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;
*                                  AC3X; Bobcat3; Aldrin2; Ironman
*
* @param[in] devNum                   - device number
*
* @param[out] errorCodePtr            - (pointer to) PHA error code configuration
*
* @retval GT_OK                       - on success
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on wrong device or errorCode
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaErrorsConfigGet
(
    IN   GT_U8                                  devNum,
    OUT  CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC    *errorCodePtr
)
{

    GT_STATUS   rc;
    GT_U32      dsaCpuCode;     /* DSA CPU Code */
    GT_U32      regAddr;        /* Register address */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                        | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(errorCodePtr);

    /* Get register address and  table read error drop code value*/
    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.errorsAndInterrupts.tablesReadErrorConfig;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 1, 8, &dsaCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Convert dsaCpuCode to dropCode */
    rc = prvCpssDxChNetIfDsaToCpuCode((PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)dsaCpuCode, &errorCodePtr->tablesReadErrorDropCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get register address and PPA clock down error drop code */
    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).pha_regs.errorsAndInterrupts.PPAClockDownErrorConfig;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 8, &dsaCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Convert dsaCpuCode to dropCode */
    return prvCpssDxChNetIfDsaToCpuCode((PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)dsaCpuCode, &errorCodePtr->ppaClockDownErrorDropCode);
}

/**
* @internal cpssDxChPhaErrorsConfigGet function
* @endinternal
*
* @brief   Get PHA error code configurations
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;
*                                  AC3X; Bobcat3; Aldrin2; Ironman
*
* @param[in] devNum                   - device number
*
* @param[out] errorCodePtr            - (pointer to) PHA error code configuration
*
* @retval GT_OK                       - on success
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on wrong device or errorCode
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*/
GT_STATUS cpssDxChPhaErrorsConfigGet
(
    IN   GT_U8                                  devNum,
    OUT  CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC    *errorCodePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaErrorsConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, errorCodePtr));

    rc = internal_cpssDxChPhaErrorsConfigGet(devNum, errorCodePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, errorCodePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChPhaFwImageInfoGet function
* @endinternal
*
* @brief  Get information related to PHA firmware image version
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in]   devNum           - device number
* @param[out] fwImageInfoPtr    - (pointer to) firmware image information
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong device
* @retval GT_BAD_PTR                - on NULL pointer
* @retval GT_NOT_SUPPORTED          - firmware image ID that was read from PHA memory is not supported
* @retval GT_FAIL                   - on hardware error
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_NOT_INITIALIZED        - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*/
static GT_STATUS internal_cpssDxChPhaFwImageInfoGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_DXCH_PHA_FW_IMAGE_INFO_STC  *fwImageInfoPtr
)
{
    GT_STATUS rc = GT_OK;     /* return code intialized to GT_OK */
    GT_U32  regAddr;          /* holds firmware sp memory address */
    GT_U32  fwImageId;        /* holds firmware image ID */
    GT_U32  fwVersion;        /* holds firmware version number */
    GT_U32  ppg,ppn;          /* holds PPG & PPN numbers from which firmware version is read */
    GT_U32  i;                /* index iterator */
    GT_U32  imageEntryStart;  /* index to table per image ID */
    GT_U32  fwVersionsArray[FW_VERSIONS_MAX_NUM_CNS]; /* table to hold all supported firmware versions */


    /* Check device number is valid and PHA supported */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_IRONMAN_E);
    PHA_UNIT_CHECK(devNum);

    /* Verify pointer is not null */
    CPSS_NULL_PTR_CHECK_MAC(fwImageInfoPtr);

    /* Clear fwVersionsArray[] array */
    cpssOsMemSet(fwVersionsArray,0,sizeof(fwVersionsArray));

    /* Use PPG 0 and PPN 0 to read the firmware version from */
    ppg = 0;
    ppn = 0;

    /* Get current firmware image ID by reading it from PPN SP memory */
    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPN[ppn].ppn_regs.fwImageId;
    rc = prvCpssHwPpReadRegister(devNum,regAddr,&fwImageId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get current firmware version number by reading it from PPN SP memory */
    regAddr = PRV_DXCH_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPN[ppn].ppn_regs.fwVersion;
    rc = prvCpssHwPpReadRegister(devNum,regAddr,&fwVersion);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Update output parameter with firmware image ID (placed in MSB) */
    fwImageInfoPtr->fwImageId = (fwImageId>>24) & 0xFF;

    /* Verify firmware image ID that was read from PHA SP memory is supported by CPSS */
    if (fwImageInfoPtr->fwImageId >= CPSS_DXCH_PHA_FW_IMAGE_ID_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
        "firmware image ID [%d] that was read from PHA SP memory is not supported by CPSS", fwImageInfoPtr->fwImageId);
    }

    /* Update output parameter with firmware version number */
    fwImageInfoPtr->fwVersion = fwVersion;

    /* Get all possible firmware versions */
    rc = firmwareVersionsArrayGet(devNum,fwVersionsArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Updated output with the supported firmware versions pending Image ID */
    for (i=0;i<CPSS_DXCH_PHA_FW_NUM_OF_SUPPORTED_VERSIONS_PER_IMAGE_CNS;i++)
    {
        /* Set index to table per image ID */
        imageEntryStart = fwImageInfoPtr->fwImageId*CPSS_DXCH_PHA_FW_NUM_OF_SUPPORTED_VERSIONS_PER_IMAGE_CNS;
        fwImageInfoPtr->fwVersionsArr[i] = fwVersionsArray[imageEntryStart + i];
    }

    return rc;
}

/**
* @internal cpssDxChPhaFwImageInfoGet function
* @endinternal
*
* @brief  Get information related to PHA firmware image version
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in]   devNum           - device number
* @param[out] fwImageInfoPtr    - (pointer to) firmware image information
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong device
* @retval GT_BAD_PTR                - on NULL pointer
* @retval GT_NOT_SUPPORTED          - firmware image ID that was read from PHA memory is not supported
* @retval GT_FAIL                   - on hardware error
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_NOT_INITIALIZED        - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*/
GT_STATUS cpssDxChPhaFwImageInfoGet
(
    IN     GT_U8                         devNum,
    OUT CPSS_DXCH_PHA_FW_IMAGE_INFO_STC  *fwImageInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaFwImageInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, fwImageInfoPtr));

    rc = internal_cpssDxChPhaFwImageInfoGet(devNum, fwImageInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, fwImageInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPhaFwImageUpgrade function
* @endinternal
*
* @brief  Upgrade PHA firmware by switching current firmware version with new one
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in]  devNum                   - device number
* @param[in]  packetOrderChangeEnable  - Enable/Disable the option for packet order
*                                        change between heavily processed flows and lightly processed flows
*                                        GT_TRUE  - packet order is not maintained
*                                        GT_FALSE - packet order is maintained
* @param[out] fwImageInfoPtr           - (pointer to) firmware information of the upgraded firmware that is requested to be loaded
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong device or invalid fw version
* @retval GT_BAD_PTR                - on NULL pointer
* @retval GT_FAIL                   - on hardware error
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_NOT_INITIALIZED        - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_BAD_STATE              - firmware address check fail. One of firmware threads holds bad address
*/
static GT_STATUS internal_cpssDxChPhaFwImageUpgrade
(
    IN  GT_U8                            devNum,
    IN  GT_BOOL                          packetOrderChangeEnable,
    IN  CPSS_DXCH_PHA_FW_IMAGE_INFO_STC  *fwImageInfoPtr
)
{
    GT_STATUS                            rc = GT_OK;         /* return code status */
    PHA_FW_IMAGE_INFO_TABLES_STC         fwImageInfoTables;  /* holds firmware tables information of the required firmware version */
    GT_U32                               threadId;           /* holds thread ID of thread's type */
    GT_U32                               i;                  /* loop index iterator to scan all supported firmware threads */


    /*****************************************************************************
     * API's checkers                                                            *
     *****************************************************************************/
    /* Check device number is valid and PHA supported */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_IRONMAN_E);
    PHA_UNIT_CHECK(devNum);

    /* Verify pointer is not null */
    CPSS_NULL_PTR_CHECK_MAC(fwImageInfoPtr);

    /* Clear fwImageInfoTables structure */
    cpssOsMemSet(&fwImageInfoTables,0, sizeof(fwImageInfoTables));

    /* Verify requested firmware version exists and valid.
       If so get firmware tables that hold new firmware information */
    rc = firmwareImageInfoTablesGet(devNum,
                                    fwImageInfoPtr->fwImageId,
                                    fwImageInfoPtr->fwVersion,
                                    &fwImageInfoTables);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*****************************************************************************
     * Update PHA firmware global DB per device                                  *
     *****************************************************************************/
    /* Set threads' initial data:
       - update thread's window anchor
       - update thread's window size
       - invalidate thread's address */
    for(i=0; i<PRV_CPSS_DXCH_PHA_MAX_THREADS_CNS; i++)
    {
       PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[i].firmwareInstructionPointer = phaFwThreadsInitInformation[i].firmwareInstructionPointer;
       PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[i].headerWindowAnchor = phaFwThreadsInitInformation[i].headerWindowAnchor;
       PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[i].headerWindowSize = phaFwThreadsInitInformation[i].headerWindowSize;
    }

    /* Set skip counter value for each thread per device */
    rc = firmwareThreadsSkipCounterSet(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Update PHA global DB with valid address of "Do nothing" thread */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[0].firmwareInstructionPointer = fwImageInfoTables.fwThreadAddrPtr[0];

    /* State that the library was initialized */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.phaInitDone = GT_TRUE;

    /* Update Enable/disable skip counter flag */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.packetOrderChangeEnable = packetOrderChangeEnable;

    /* Update firmware global DB with firmware image ID */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.phaFwImageId = fwImageInfoPtr->fwImageId;

    /* Allow the PPA sub unit in the PHA to get clock in case it was disabled during init */
    rc = prvCpssDxChPhaClockEnable(devNum,GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*****************************************************************************
     * Initialize all the PHA table entries to bind to 'Drop all traffic' thread *
     *****************************************************************************/

    /* Get "drop all traffic" thread ID for PHA table in global DB */
    rc = firmwareThreadTypeToThreadId(CPSS_DXCH_PHA_THREAD_TYPE_DROP_ALL_TRAFFIC_E, &threadId);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* Update PHA global DB with valid address of "drop all traffic" thread */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[threadId].firmwareInstructionPointer = fwImageInfoTables.fwThreadAddrPtr[threadId];

    /* Set PHA entries table with "drop all traffic" thread */
    rc = firmwareDropThreadPhaEntriesTableSet(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Sleep for 1 msec to drain any existing packets in PPNs */
    cpssOsTimerWkAfter(1);

    /*****************************************************************************
     * Load new PHA firmware                                                     *
    *****************************************************************************/
    /* Download new firmware data such as accelerators and version info into SP memory of PHA unit in the device */
    rc = firmwareDmemDownload(devNum, fwImageInfoTables.fwAcclCmdPtr, fwImageInfoTables.fwImageInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Update firmware global DB with new firmware image version information */
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.fwVersionInfo.imageVersion = fwImageInfoTables.fwImageInfoPtr->imageVersion;
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.fwVersionInfo.imageId      = fwImageInfoTables.fwImageInfoPtr->imageId;
    PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.fwVersionInfo.imageName    = fwImageInfoTables.fwImageInfoPtr->imageName;

    /* Download new firmware code into instruction memory of PHA unit in the device */
    rc = firmwareImemDownload(devNum, fwImageInfoTables.fwImemPtr, fwImageInfoTables.fwImemSize);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Set new firmware threads addresses in PHA global DB */
    for (i=0;i<PRV_CPSS_DXCH_PHA_MAX_THREADS_CNS;i++)
    {
       PRV_CPSS_DXCH_PP_MAC(devNum)->phaInfo.threadInfoArr[i].firmwareInstructionPointer = fwImageInfoTables.fwThreadAddrPtr[i];
    }

    /* Verify all new firmware addresses are valid */
    rc = firmwareAddrValidityCheck(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* For WM simulation */
    if(PRV_CPSS_PP_MAC(devNum)->isWmDevice)
    {
        /* Bind new firmware addresses to simulation */
        rc = ASIC_SIMULATION_firmwareInit(devNum, fwImageInfoTables.fwVersIndex);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal cpssDxChPhaFwImageUpgrade function
* @endinternal
*
* @brief  Upgrade PHA fw by switching current fw version with new one
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in]  devNum                   - device number
* @param[in]  packetOrderChangeEnable  - Enable/Disable the option for packet order
*                                        change between heavily processed flows and lightly processed flows
*                                        GT_TRUE  - packet order is not maintained
*                                        GT_FALSE - packet order is maintained
* @param[out] fwImageInfoPtr           - (pointer to) the fw information of the upgraded fw that is requested to be loaded
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong device or invalid fw version
* @retval GT_BAD_PTR                - on NULL pointer
* @retval GT_FAIL                   - on hardware error
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_NOT_INITIALIZED        - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_BAD_STATE              - firmware address check fail. One of firmware threads holds bad address
*/
GT_STATUS cpssDxChPhaFwImageUpgrade
(
    IN  GT_U8                            devNum,
    IN  GT_BOOL                          packetOrderChangeEnable,
    IN  CPSS_DXCH_PHA_FW_IMAGE_INFO_STC  *fwImageInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPhaFwImageUpgrade);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetOrderChangeEnable, fwImageInfoPtr));

    rc = internal_cpssDxChPhaFwImageUpgrade(devNum, packetOrderChangeEnable, fwImageInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetOrderChangeEnable, fwImageInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
