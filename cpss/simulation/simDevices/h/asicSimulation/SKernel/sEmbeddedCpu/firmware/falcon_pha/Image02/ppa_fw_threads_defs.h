/*
 *
 * ppa_fw_threads_defs.h
 *
 * PPA fw threads definitions
 *
 */
#ifndef __ppa_fw_threads_defs_h
#define __ppa_fw_threads_defs_h


#ifdef ASIC_SIMULATION
    #ifndef CPU_BE
        /* get the definitions dedicated for little endian CPU */
        #include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/ppa_fw_common_strc_defs_little_endian.h"
        #include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/Image02/ppa_fw_threads_strc_defs_little_endian.h"
    #else /*!CPU_BE*/
        /* remove next line if actually running ASIC_SIMULATION in 'big endian CPU' ... not likely ! */
        #error  "ASIC_SIMULATION : 'CPU_BE' defined ???"
    #endif /*!CPU_BE*/
#else /*!ASIC_SIMULATION*/
    #include "ppa_fw_common_strc_defs.h"
    #include "ppa_fw_threads_strc_defs.h"
#endif /*!ASIC_SIMULATION*/


/**************************************************************************
 * Function prototypes
 **************************************************************************/
void PPA_FW(THR0_DoNothing)();
void PPA_FW(THR1_SRv6_End_Node)();
void PPA_FW(THR2_SRv6_Source_Node_1_segment)();
void PPA_FW(THR3_SRv6_Source_Node_First_Pass_2_3_segments)();
void PPA_FW(THR4_SRv6_Source_Node_Second_Pass_3_segments)();
void PPA_FW(THR5_SRv6_Source_Node_Second_Pass_2_segments)();
void PPA_FW(THR6_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv4)();
void PPA_FW(THR7_Cc_Erspan_TypeII_TrgDevLcMirroring_Ipv6)();
void PPA_FW(THR8_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv4)();
void PPA_FW(THR9_Cc_Erspan_TypeII_TrgDevDirectMirroring_Ipv6)();
void PPA_FW(THR10_Cc_Erspan_TypeII_SrcDevMirroring)();
void PPA_FW(THR11_VXLAN_GPB_SourceGroupPolicyID)();
void PPA_FW(THR12_MPLS_SR_NO_EL)();
void PPA_FW(THR13_MPLS_SR_ONE_EL)();
void PPA_FW(THR14_MPLS_SR_TWO_EL)();
void PPA_FW(THR15_MPLS_SR_THREE_EL)();
void PPA_FW(THR53_SRV6_End_Node_GSID_COC32)();
void PPA_FW(THR54_IPv4_TTL_Increment)();
void PPA_FW(THR55_IPv6_HopLimit_Increment)();
void PPA_FW(THR56_Clear_Outgoing_Mtag_Cmd)();
void PPA_FW(THR57_SFLOW_V5_IPv4)();
void PPA_FW(THR58_SFLOW_V5_IPv6)();
void PPA_FW(THR60_DropAllTraffic)();
void PPA_FW(THR64_Erspan_TypeII_SameDevMirroring_Ipv4)();
void PPA_FW(THR65_Erspan_TypeII_SameDevMirroring_Ipv6)();

#if RECOVER_FROM_EXCEPTION
INLINE void falconRecoveFromException();
#endif
/* Required for profile measurements */
void       _exit();

#endif /* __ppa_fw_threads_defs_h */
