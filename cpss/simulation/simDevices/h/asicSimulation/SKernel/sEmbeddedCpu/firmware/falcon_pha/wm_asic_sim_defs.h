/*
 * wm_asic_sim_defs.h
 *
 *  Created on: Dec 12, 2016
 *      Author: doronf
 */

#ifndef __wm_asic_sim_defs_h
#define __wm_asic_sim_defs_h

#ifdef ASIC_SIMULATION
    #include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/ppa_fw_base_types.h"
    #include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/Default/ppa_fw_threads_defs.h"
    extern void ppaFwSpWrite(uint32_t addr,uint32_t data);
    extern void ppaFwSpShortWrite(uint32_t addr,uint16_t data);
    extern void ppaFwSpByteWrite(uint32_t addr,uint8_t data);
    extern uint32_t ppaFwSpRead(uint32_t addr);
    extern uint16_t ppaFwSpShortRead(uint32_t addr);
    extern uint8_t ppaFwSpByteRead(uint32_t addr);
    struct ppa_in_desc*  falconMemCast_ppa_in_desc(uint32_t address);
    struct IPv4_Header* falconMemCast_IPv4_Header(uint32_t address);

    /* Thread#6 */
    struct thr6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_cfg*  falconMemCast_thr6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_cfg(uint32_t address);
    struct thr6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_in_hdr*  falconMemCast_thr6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4_in_hdr(uint32_t address);
    /* Thread#7 */
    struct thr7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_cfg*  falconMemCast_thr7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_cfg(uint32_t address);
    struct thr7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_in_hdr*  falconMemCast_thr7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6_in_hdr(uint32_t address);
    /* Thread#8 */
    struct thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_cfg*  falconMemCast_thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_cfg(uint32_t address);
    struct thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_in_hdr*  falconMemCast_thr8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4_in_hdr(uint32_t address);
    /* Thread#9 */
    struct thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_cfg*  falconMemCast_thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_cfg(uint32_t address);
    struct thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_in_hdr*  falconMemCast_thr9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6_in_hdr(uint32_t address);
    /* Thread#11 */
    struct thr11_vxlan_gpb_cfg*  falconMemCast_thr11_vxlan_gpb_cfg(uint32_t address);
    struct thr11_vxlan_gpb_in_hdr_ipv4*  falconMemCast_thr11_vxlan_gpb_in_hdr_ipv4(uint32_t address);
    struct thr11_vxlan_gpb_in_hdr_ipv6*  falconMemCast_thr11_vxlan_gpb_in_hdr_ipv6(uint32_t address);
    /* Thread#12 */
    struct thr12_MPLS_NO_EL_in_hdr*  falconMemCast_thr12_MPLS_NO_EL_in_hdr(uint32_t address);
    /* Thread#13 */
    struct thr13_MPLS_ONE_EL_in_hdr*  falconMemCast_thr13_MPLS_ONE_EL_in_hdr(uint32_t address);
    /* Thread#14 */
    struct thr14_MPLS_TWO_EL_in_hdr*  falconMemCast_thr14_MPLS_TWO_EL_in_hdr(uint32_t address);
    /* Thread#15 */
    struct thr15_MPLS_THREE_EL_in_hdr*  falconMemCast_thr15_MPLS_THREE_EL_in_hdr(uint32_t address);
    /* Thread#26 */
    struct thr26_SRv6_penultimateEndNode_in_hdr*  falconMemCast_thr26_SRv6_penultimateEndNode_in_hdr(uint32_t address);
    /* Thread#27 */
    struct thr27_INT_Ingress_Switch_IPv6_cfg*  falconMemCast_thr27_INT_Ingress_Switch_IPv6_cfg(uint32_t address);
    struct thr27_INT_Ingress_Switch_IPv6_in_hdr*  falconMemCast_thr27_INT_Ingress_Switch_IPv6_in_hdr(uint32_t address);
    /* Thread#28 */
    struct thr28_INT_Ingress_Switch_IPv4_cfg*  falconMemCast_thr28_INT_Ingress_Switch_IPv4_cfg(uint32_t address);
    struct thr28_INT_Ingress_Switch_IPv4_in_hdr*  falconMemCast_thr28_INT_Ingress_Switch_IPv4_in_hdr(uint32_t address);
    /* Thread#29 */
    struct thr29_INT_Transit_Switch_IPv6_cfg*  falconMemCast_thr29_INT_Transit_Switch_IPv6_cfg(uint32_t address);
    struct thr29_INT_Transit_Switch_IPv6_in_hdr*  falconMemCast_thr29_INT_Transit_Switch_IPv6_in_hdr(uint32_t address);
    /* Thread#30 */
    struct thr30_INT_Transit_Switch_IPv4_cfg*  falconMemCast_thr30_INT_Transit_Switch_IPv4_cfg(uint32_t address);
    struct thr30_INT_Transit_Switch_IPv4_in_hdr*  falconMemCast_thr30_INT_Transit_Switch_IPv4_in_hdr(uint32_t address);
    /* Thread#31 */
    struct thr31_IOAM_Ingress_Switch_IPv4_cfg*  falconMemCast_thr31_IOAM_Ingress_Switch_IPv4_cfg(uint32_t address);
    struct thr31_IOAM_Ingress_Switch_IPv4_in_hdr*  falconMemCast_thr31_IOAM_Ingress_Switch_IPv4_in_hdr(uint32_t address);
    /* Thread#32 */
    struct thr32_IOAM_Ingress_Switch_IPv6_cfg*  falconMemCast_thr32_IOAM_Ingress_Switch_IPv6_cfg(uint32_t address);
    struct thr32_IOAM_Ingress_Switch_IPv6_in_hdr*  falconMemCast_thr32_IOAM_Ingress_Switch_IPv6_in_hdr(uint32_t address);
    /* Thread#33 */
    struct thr33_IOAM_Transit_Switch_IPv4_cfg*  falconMemCast_thr33_IOAM_Transit_Switch_IPv4_cfg(uint32_t address);
    struct thr33_IOAM_Transit_Switch_IPv4_in_hdr*  falconMemCast_thr33_IOAM_Transit_Switch_IPv4_in_hdr(uint32_t address);
    /* Thread#34 */
    struct thr34_IOAM_Transit_Switch_IPv6_cfg*  falconMemCast_thr34_IOAM_Transit_Switch_IPv6_cfg(uint32_t address);
    struct thr34_IOAM_Transit_Switch_IPv6_in_hdr*  falconMemCast_thr34_IOAM_Transit_Switch_IPv6_in_hdr(uint32_t address);
    /* Thread#35 */
    struct thr35_IOAM_Egress_Switch_IPv6_cfg*  falconMemCast_thr35_IOAM_Egress_Switch_IPv6_cfg(uint32_t address);
    /* Thread#38 */
    struct thr38_Unified_SR_in_hdr*  falconMemCast_thr38_Unified_SR_in_hdr(uint32_t address);
    /* Thread#39 */
    struct thr39_Classifier_NSH_over_Ethernet_out_hdr*  falconMemCast_thr39_Classifier_NSH_over_Ethernet_out_hdr(uint32_t address);
    /* Thread#40 */
    struct thr40_Classifier_NSH_over_VXLAN_GPE_out_hdr*  falconMemCast_thr40_Classifier_NSH_over_VXLAN_GPE_out_hdr(uint32_t address);
    /* Thread#44 */
    struct thr44_VariableCyclesLengthWithAcclCmd_cfg * falconMemCast_thr44_VariableCyclesLengthWithAcclCmd_cfg(uint32_t address);
    /* Thread#45 */
    struct thr45_RemoveAddBytes_cfg * falconMemCast_thr45_RemoveAddBytes_cfg(uint32_t address);
    /* Thread#48 */
    struct thr48_srv6_best_effort_in_hdr*  falconMemCast_thr48_srv6_best_effort_in_hdr(uint32_t address);
    /* Thread#49 */
    struct thr49_srv6_source_node_1_container_in_hdr*  falconMemCast_thr49_srv6_source_node_1_container_in_hdr(uint32_t address);
    /* Thread#53 */
    struct thr53_SRV6_End_Node_GSID_COC32_in_hdr*  falconMemCast_thr53_SRV6_End_Node_GSID_COC32_in_hdr(uint32_t address);
    struct thr53_SRV6_End_Node_GSID_COC32_cfg* falconMemCast_thr53_SRV6_End_Node_GSID_COC32_cfg(uint32_t address);
    struct thr53_SRV6_End_Node_GSID_COC32_out_hdr* falconMemCast_thr53_SRV6_End_Node_GSID_COC32_out_hdr(uint32_t address);
    /* Thread#59 */
    struct thr59_sls_test_in_hdr*  falconMemCast_thr59_sls_test_in_hdr(uint32_t address);









#endif /*!ASIC_SIMULATION*/



#endif /*__wm_asic_sim_defs_h*/
