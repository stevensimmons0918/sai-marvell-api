/*
 *  ppa_fw_ppn2mg.h
 *
 *  PPA fw ppn to MG IF definitions
 *
 */

#ifndef PPA_FW_PPN2MG_H
#define PPA_FW_PPN2MG_H

#include "ppa_fw_base_types.h"
/* ===========================
   Constants & Macros
   =========================== */
#define PPN2MG_EVENTS_BUF_ADDRESS   (0x30)
#define PPN2MG_RESET_VAL            (0x88888888)
#define PPN_ERR_BUF_NUM_OF_ENTRIES  (10)
#define STOP_PPN                    (1)
#define DONT_STOP_PPN               (0)

/* Read memory actions for MG to apply */
#define RD_SP                    (1)
#define RD_SRAM                  (1)
#define RD_REG_FILE              (1)
#define RD_REG_IO                (1)

/********************** Internal PPN EVENTS ID  ***********************/
/* PPN read requests for MG   */
#define PPN_RD_SP_REQUEST_ID                    (0x3300)
#define PPN_RD_SRAM_REQUEST_ID                  (0x3301)
#define PPN_RD_REG_FILE_REQUEST_ID              (0x3302)
#define PPN_RD_REG_IO_REQUEST_ID                (0x3303)
#define PPN_RD_ALL_REQUEST_ID                   (0x3304)
#define PPN_RD_REQUEST_RESERVED_ID              (0x3305)
/* PPN warning indications for MG   */
#define PPN_EXCEPTION_RECOVER_EVENT_ID          (0x3310)
#define PTP_INVALID_UDP_CS_OFFSET_EVENT_ID      (0x3311)
#define PTP_INVALID_ETHERNET_TYPE_EVENT_ID      (0x3312)
#define ECN_INVALID_IP_VERSION_EVENT_ID         (0x3313)
#define PPN_WARNING_EVENT_RESERVED_ID           (0x3314)
/* PPN critical error indications for MG   */
#define PPN_EXCEPTION_CRASH_EVENT_ID            (0x3320)
#define PPN_STACK_OVERFLOW_EVENT_ID             (0x3321)
#define PPN_ERROR_EVENT_RESERVED_ID             (0x3322)


/* Macro to build PPN2MG event opcode:                            Bits:  |     19      |       18      |     17    |   16    | 15   ---     1 0 |
   Concatenate PPN even id with read flags                               |   RD_REG_IO |  RD_REG_FILE  |  RD_SRAM  |  RD_SP  |   PPN_EVENT_ID   |         */
#define BUILD_PPN2MG_MSG(rd_reg_io,rd_reg_file,rd_sram,rd_sp,event_id)   (rd_reg_io<<19|rd_reg_file<<18|rd_sram<<17|rd_sp<<16|event_id<<0)
/* User PPN to MG EVENTS opcodes */
#define PPN2MG_RD_SP_REQUEST                     BUILD_PPN2MG_MSG(NULL        , NULL        , NULL        , RD_SP    , PPN_RD_SP_REQUEST_ID)
#define PPN2MG_RD_SRAM_REQUEST                   BUILD_PPN2MG_MSG(NULL        , NULL        , RD_SRAM     , NULL     , PPN_RD_SRAM_REQUEST_ID)
#define PPN2MG_RD_REGISTER_FILE_REQUEST          BUILD_PPN2MG_MSG(NULL        , RD_REG_FILE , NULL        , NULL     , PPN_RD_REG_FILE_REQUEST_ID)
#define PPN2MG_RD_REGISTER_IO_REQUEST            BUILD_PPN2MG_MSG(RD_REG_IO   , NULL        , NULL        , NULL     , PPN_RD_REG_IO_REQUEST_ID)
#define PPN2MG_RD_ALL_REQUEST                    BUILD_PPN2MG_MSG(RD_REG_IO   , RD_REG_FILE , RD_SRAM     , RD_SP    , PPN_RD_ALL_REQUEST_ID)
#define PPN2MG_PPN_EXCEPTION_RECOVER_EVENT       BUILD_PPN2MG_MSG(NULL        , NULL        , NULL        , NULL     , PPN_EXCEPTION_RECOVER_EVENT_ID)      /* Enable RD_SP flag to get error parameters */
#define PPN2MG_PTP_INVALID_UDP_CS_OFFSET_EVENT   BUILD_PPN2MG_MSG(NULL        , NULL        , NULL        , NULL     , PTP_INVALID_UDP_CS_OFFSET_EVENT_ID)  /* Enable RD_SP flag to get error parameters */
#define PPN2MG_PTP_INVALID_ETHERNET_TYPE_EVENT   BUILD_PPN2MG_MSG(NULL        , NULL        , NULL        , NULL     , PTP_INVALID_ETHERNET_TYPE_EVENT_ID)  /* Enable RD_SP flag to get error parameters */
#define PPN2MG_ECN_INVALID_IP_VERSION_EVENT      BUILD_PPN2MG_MSG(NULL        , NULL        , NULL        , NULL     , ECN_INVALID_IP_VERSION_EVENT_ID)     /* Enable RD_SP flag to get error parameters */
#define PPN2MG_PPN_EXCEPTION_CRASH_EVENT         BUILD_PPN2MG_MSG(RD_REG_IO   , RD_REG_FILE , RD_SRAM     , RD_SP    , PPN_EXCEPTION_CRASH_EVENT_ID)
#define PPN2MG_PPN_STACK_OVERFLOW_EVENT          BUILD_PPN2MG_MSG(RD_REG_IO   , RD_REG_FILE , RD_REG_FILE , RD_SP    , PPN_STACK_OVERFLOW_EVENT_ID)


/* ===========================
   Externs
   =========================== */
extern uint32_t* ppn2mg_events_buf_ptr;
extern uint32_t ppn2mg_events_buf_indx;
extern uint32_t ppn_err_params_buf[PPN_ERR_BUF_NUM_OF_ENTRIES];
extern uint32_t* ppn_err_params_buf_ptr;


/* ===========================
   Function prorotypes
   =========================== */
void ppn2mg_powerup();
void ppn2mg_rd_request(uint32_t event_id, uint32_t stop_ppn);
void ppn2mg_failure_event(uint32_t event_id, uint32_t* err_params_ptr ,uint32_t num_of_params, uint32_t stop_ppn);
void ppn2mg_exception_crash_event();

#endif
