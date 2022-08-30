/*
 * ppa_fw_image_info.h
 *
 * PPA fw image info
 *
 */

#ifndef __ppa_fw_image_info_h
#define __ppa_fw_image_info_h

#define ENABLE_THR0_DoNothing
#define ENABLE_THR1_SRv6_End_Node
#define ENABLE_THR2_SRv6_Source_Node_1_segment
#define ENABLE_THR3_SRv6_Source_Node_First_Pass_2_3_segments
#define ENABLE_THR4_SRv6_Source_Node_Second_Pass_3_segments
#define ENABLE_THR5_SRv6_Source_Node_Second_Pass_2_segments
#define ENABLE_THR11_VXLAN_GPB_SourceGroupPolicyID
#define ENABLE_THR12_MPLS_SR_NO_EL
#define ENABLE_THR13_MPLS_SR_ONE_EL
#define ENABLE_THR14_MPLS_SR_TWO_EL
#define ENABLE_THR15_MPLS_SR_THREE_EL
#define ENABLE_THR16_SGT_NetAddMSB
#define ENABLE_THR17_SGT_NetFix
#define ENABLE_THR18_SGT_NetRemove
#define ENABLE_THR19_SGT_eDSAFix
#define ENABLE_THR20_SGT_eDSARemove
#define ENABLE_THR21_SGT_GBPFixIPv4
#define ENABLE_THR22_SGT_GBPFixIPv6
#define ENABLE_THR23_SGT_GBPRemoveIPv4
#define ENABLE_THR24_SGT_GBPRemoveIPv6
#define ENABLE_THR25_PTP_Phy_1_Step
#define ENABLE_THR46_SFLOW_IPv4
#define ENABLE_THR47_SFLOW_IPv6
#define ENABLE_THR48_SRV6_Best_Effort
#define ENABLE_THR49_SRV6_Source_Node_1_CONTAINER
#define ENABLE_THR50_SRV6_Source_Node_First_Pass_1_CONTAINER
#define ENABLE_THR51_SRV6_Source_Node_Second_Pass_3_CONTAINER
#define ENABLE_THR52_SRV6_Source_Node_Second_Pass_2_CONTAINER
#define ENABLE_THR53_SRV6_End_Node_GSID_COC32
#define ENABLE_THR60_DropAllTraffic
#define ENABLE_THR61_save_target_port_info
#define ENABLE_THR62_enhanced_sFlow_fill_remain_IPv4
#define ENABLE_THR63_enhanced_sFlow_fill_remain_IPv6
#define ENABLE_THR66_enhanced_sFlow

#define TARGET_DEVICE Ac5p
#define FW_IMAGE_NAME Image01
#define FW_IMAGE_ID   1

#define VER_YEAR      (0x21) /* Year */
#define VER_MONTH     (0x10) /* Month */
#define VER_IN_MONTH  (0x00) /* Number of version within a month */
#define VER_DEBUG     (0x00) /* Used for private or debug versions, should be zero for official version */

#endif /* __ppa_fw_image_info_h */ 
