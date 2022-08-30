/*
 * wm_asic_sim_defs.h
 *
 *  Created on: Dec 12, 2016
 *      Author: doronf
 */

#ifndef WM_ASIC_SIM_DEFS_H
#define WM_ASIC_SIM_DEFS_H
#include "ppa_fw_base_types.h"

#ifdef ASIC_SIMULATION
    void ppaFwSpWrite(uint32_t addr,uint32_t data);
    void ppaFwSpShortWrite(uint32_t addr,uint16_t data);
    void ppaFwSpByteWrite(uint32_t addr,uint8_t data);
    uint32_t ppaFwSpRead(uint32_t addr);
    uint16_t ppaFwSpShortRead(uint32_t addr);
    uint8_t ppaFwSpByteRead(uint32_t addr);
    struct nonQcn_desc* pipeMemCast_nonQcn_desc(uint32_t address);
    struct Qcn_desc* pipeMemCast_Qcn_desc(uint32_t address);
    struct thr1_e2u_cfg* pipeMemCast_thr1_e2u_cfg(uint32_t address);
    struct thr2_u2e_in_hdr * pipeMemCast_thr2_u2e_in_hdr(uint32_t address);
    struct thr2_u2e_cfg * pipeMemCast_thr2_u2e_cfg (uint32_t address);
    struct thr3_u2c_in_hdr * pipeMemCast_thr3_u2c_in_hdr(uint32_t address);
    struct thr3_u2c_cfg * pipeMemCast_thr3_u2c_cfg (uint32_t address);
    struct thr4_et2u_cfg * pipeMemCast_thr4_et2u_cfg(uint32_t address);
    struct thr5_eu2u_cfg * pipeMemCast_thr5_eu2u_cfg(uint32_t address);
    struct thr6_u2e_in_hdr * pipeMemCast_thr6_u2e_in_hdr(uint32_t address);
    struct thr6_u2e_cfg * pipeMemCast_thr6_u2e_cfg (uint32_t address);
    struct thr7_mrr2e_in_hdr * pipeMemCast_thr7_mrr2e_in_hdr(uint32_t address);
    struct thr7_mrr2e_cfg * pipeMemCast_thr7_mrr2e_cfg(uint32_t address);
    struct thr8_e_v2u_cfg * pipeMemCast_thr8_e_v2u_cfg(uint32_t address);
    struct thr9_e2u_cfg * pipeMemCast_thr9_e2u_cfg (uint32_t address);
    struct thr10_c_v2u_cfg * pipeMemCast_thr10_c_v2u_cfg(uint32_t address);
    struct thr10_c_v2u_in_hdr * pipeMemCast_thr10_c_v2u_in_hdr(uint32_t address);
    struct thr11_c2u_cfg * pipeMemCast_thr11_c2u_cfg(uint32_t address);
    struct thr11_c2u_in_hdr * pipeMemCast_thr11_c2u_in_hdr(uint32_t address);
    struct thr12_u_uc2c_cfg * pipeMemCast_thr12_u_uc2c_cfg(uint32_t address);
    struct thr12_u_uc2c_in_hdr * pipeMemCast_thr12_u_uc2c_in_hdr(uint32_t address);
    struct thr13_u_mc2c_cfg * pipeMemCast_thr13_u_mc2c_cfg(uint32_t address);
    struct thr14_u_mr2c_cfg * pipeMemCast_thr14_u_mr2c_cfg(uint32_t address);
    struct thr14_u_mr2c_in_hdr * pipeMemCast_thr14_u_mr2c_in_hdr(uint32_t address);
    struct thr15_qcn_cfg * pipeMemCast_thr15_qcn_cfg(uint32_t address);
    struct thr16_u2e_in_hdr * pipeMemCast_thr16_u2e_in_hdr(uint32_t address);
    struct thr16_u2e_cfg * pipeMemCast_thr16_u2e_cfg (uint32_t address);
    struct thr17_u2ipl_in_hdr * pipeMemCast_thr17_u2ipl_in_hdr(uint32_t address);
    struct thr18_ipl2ipl_cfg * pipeMemCast_thr18_ipl2ipl_cfg(uint32_t address);
    struct thr19_e2u_untagged_cfg * pipeMemCast_thr19_e2u_untagged_cfg(uint32_t address);
    struct thr20_u2e_m4_cfg * pipeMemCast_thr20_u2e_m4_cfg(uint32_t address);
    struct thr20_u2e_m4_in_hdr * pipeMemCast_thr20_u2e_m4_in_hdr(uint32_t address);
    struct thr21_u2e_m8_cfg * pipeMemCast_thr21_u2e_m8_cfg(uint32_t address);
    struct thr21_u2e_m8_in_hdr * pipeMemCast_thr21_u2e_m8_in_hdr(uint32_t address);
    struct thr23_evb_e2u_cfg * pipeMemCast_thr23_evb_e2u_cfg(uint32_t address);
    struct thr23_evb_e2u_in_hdr * pipeMemCast_thr23_evb_e2u_in_hdr(uint32_t address);
    struct thr24_evb_u2e_cfg * pipeMemCast_thr24_evb_u2e_cfg(uint32_t address);
    struct thr24_evb_u2e_in_hdr * pipeMemCast_thr24_evb_u2e_in_hdr(uint32_t address);
    struct thr25_evb_qcn_cfg * pipeMemCast_thr25_evb_qcn_cfg(uint32_t address);
    struct thr25_evb_qcn_in_hdr * pipeMemCast_thr25_evb_qcn_in_hdr(uint32_t address);
    struct thr26_pre_da_u2e_cfg * pipeMemCast_thr26_pre_da_u2e_cfg(uint32_t address);
    struct thr27_pre_da_e2u_cfg * pipeMemCast_thr27_pre_da_e2u_cfg(uint32_t address);
    struct thr45_VariableCyclesLengthWithAcclCmd_cfg * pipeMemCast_thr45_VariableCyclesLengthWithAcclCmd_cfg(uint32_t address);
    struct thr46_RemoveAddBytes_cfg * pipeMemCast_thr46_RemoveAddBytes_cfg(uint32_t address);
    struct thr47_Add20Bytes_cfg * pipeMemCast_thr47_Add20Bytes_cfg(uint32_t address);
    struct thr48_Remove20Bytes_cfg * pipeMemCast_thr48_Remove20Bytes_cfg(uint32_t address);
    struct thr49_VariableCyclesLength_cfg * pipeMemCast_thr49_VariableCyclesLength_cfg(uint32_t address);
    struct ptp_header* pipeMemCast_ptp_header(uint32_t address);
    struct IPv4_Header* pipeMemCast_IPv4_Header(uint32_t address);
    struct IPv6_Header* pipeMemCast_IPv6_Header(uint32_t address);
    void pipeMemApply_nonQcn_desc(uint32_t address);
    void pipeMemApply_Qcn_desc(uint32_t address);
    struct pipe_fw_version * pipeMemCast_pipe_fw_version();
#endif /*!ASIC_SIMULATION*/



#endif /* WM_ASIC_SIM_DEFS_H */
