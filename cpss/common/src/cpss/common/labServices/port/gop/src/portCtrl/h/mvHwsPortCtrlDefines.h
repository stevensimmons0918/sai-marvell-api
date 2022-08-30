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
* @file mvHwsPortCtrlDefines.h
*
* @brief Port Control Definitions
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPortCtrlDefines_H
#define __mvHwsPortCtrlDefines_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3))

#define AN_PORT_SM /*(M7)*/
#define AP_PORT_SM /*(M4)(M5)*/

#elif (defined(RAVEN_DEV_SUPPORT)) || ( defined (FALCON_DEV_SUPPORT) && defined (MICRO_INIT) )
#define AN_PORT_SM /*(M7)*/
#else
#define AP_PORT_SM /*(M4)(M5)*/
#endif

#ifndef ASIC_SIMULATION

/*#define MV_PORT_CTRL_DEBUG*/
#ifdef MV_PORT_CTRL_DEBUG
#define mvPcPrintf(str...) osPrintf(str)
#else
#define mvPcPrintf(str...)
#endif

#endif /* ndef ASIC_SIMULATION */



/* Global definitions */
/* ================== */
/* For CM3: SHMEM_SIZE = 0x800; LOAD_ADDR = 0; SHMEM_REGS = (SHMEM_BASE + SHMEM_SIZE - 4); SHMEM_BASE = SRAM size (128KB BOBK) - SHMEM_SIZE;*/
#if !defined(CHX_FAMILY) && !defined(PX_FAMILY)
  #ifdef MICRO_INIT
  #ifdef CM3
    #define LOAD_ADDR       0
    #if (defined ALDRIN2_DEV_SUPPORT) || (defined FALCON_DEV_SUPPORT)
        #define SHMEM_BASE         (0x5f800)
    #else
        #ifndef BOBK_DEV_SUPPORT
          #define SHMEM_BASE         (0x2f800)
        #else
          #define SHMEM_BASE         (0x1f800)
        #endif
    #endif
  /* type=2==CM3_ALDRIN, SRAM_BASE=0==0, SRAM_SIZE=1==192K */
 #else /* !CM3 */
  #if !defined(SHMEM_BASE)
   #define SHMEM_BASE      (0x4007F800)
  #endif
  /* type=0==MSYS, SRAM_BASE=1==0x40000000, SRAM_SIZE=2==512K */
 #endif /* !ALDRIN_DEV_SUPPORT */
 #if !defined(SHMEM_SIZE)
  #define SHMEM_SIZE      (0x800)
 #endif
 #define SHMEM_REGS      (SHMEM_BASE + SHMEM_SIZE - 4)
 #define HWS_IPC_LINK_ID (IPC_CM3_FREERTOS_LINK_ID)
#else /* MICRO_INIT */
 #ifdef BC2_DEV_SUPPORT
  #ifndef LOAD_ADDR
  #define LOAD_ADDR       (0xFFF80000)
  #endif
  #if !(defined(SHMEM_BASE) && defined(SHMEM_SIZE))
  #define SHMEM_BASE      (0xFFF40000)
  #define SHMEM_SIZE      (0x40000)
  #endif
  #define SHMEM_REGS      (0xfffffffc)
  #define HWS_IPC_LINK_ID (IPC_SCPU_FREERTOS_LINK_ID)
  /* type=0==MSYS, SRAM_BASE=2==END, SRAM_SIZE=3==2048K */
 #elif defined(BOBK_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)
  #if !(defined(SHMEM_BASE) && defined(SHMEM_SIZE))
  #define LOAD_ADDR       0
  #define SHMEM_BASE      (0x1F800)
  #define SHMEM_SIZE      (0x800)
  #endif
  #define SHMEM_REGS      (SHMEM_BASE + SHMEM_SIZE - 4)
  #define HWS_IPC_LINK_ID (IPC_CM3_FREERTOS_LINK_ID)
 #elif defined(ALDRIN2_DEV_SUPPORT)
  #if !(defined(SHMEM_BASE) && defined(SHMEM_SIZE))
  #define LOAD_ADDR       0
  #define SHMEM_BASE      (0x5F800)
  #define SHMEM_SIZE      (0x800)
  #endif
  #define SHMEM_REGS      (SHMEM_BASE + SHMEM_SIZE - 4)
 #elif defined(RAVEN_DEV_SUPPORT)
  #define LOAD_ADDR       (0x0)
  #if !(defined(SHMEM_BASE) && defined(SHMEM_SIZE))
  #define SHMEM_BASE      (0x27800)
  #define SHMEM_SIZE      (0x800)
  #endif
  #define SHMEM_REGS      0x27ffc
  /* type=2==CM3_RAVEN, SRAM_BASE=0==0, SRAM_SIZE=1==160K */
 #elif defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)
  #define LOAD_ADDR       (0x0)
  #if !(defined(SHMEM_BASE) && defined(SHMEM_SIZE))
  #define SHMEM_BASE      (0x2F800)
  #define SHMEM_SIZE      (0x800)
  #endif
  #define SHMEM_REGS      0x2fffc
  /* type=2==CM3_ALDRIN, SRAM_BASE=0==0, SRAM_SIZE=1==192K */
#else
   #pragma message "Global Defintion - Load Address, Shared Memory Address / Size not defined"
  #endif
#endif /* MICRO_INIT */
 #define SHMEM_VIRTREGS_BASE ((SHMEM_BASE) + 0x100)
#endif /* !defined(CHX_FAMILY) && !defined(PX_FAMILY) */


/* BobK Device Definition */
/* ====================== */
#ifdef REG_PORT_TASK
/* Max number of supported ports */
#define MV_PORT_CTRL_MAX_PORT_NUM        (72)
#endif
/* Max number of Executing processes */
#define MV_PORT_CTRL_MAX_PROCESS_NUM     (2)

/* Max number of pending messages per port */
#define MV_PORT_CTRL_PORT_PEND_MSG_NUM   (3)

#define MV_PORT_CTRL_NUM_OF_QUEUE_MSGS   (6)

/* Max number of messages enter execution by the Supervisor */
#define MV_PORT_CTRL_SPV_MSG_EXEC_THRESHOLD (MV_PORT_CTRL_NUM_OF_QUEUE_MSGS)

/* 4KByes space of log (both AP and Hws) */

/* Max number of AP log message entries */
/* 416 entry's x 8 bytes = 3328Bytes */
#define MV_PORT_CTRL_LOG_SIZE       (416)
#define MV_PORT_CTRL_LOG_EXTRA_STATUS_SIZE       (18) /*num of ports(raven) + 1 global time stamp + 1 version number*/
#define MV_PORT_CTRL_AP_VERSION       (0x043300)    /*version 4.3.3 043300*/

/* Max number of FW Hws log message entries */
/* 12 entry's x 64 bytes = 768Bytes*/
#define MV_FW_HWS_LOG_SIZE          (12)

/* Port Control port number definition - starting 0 */
#define MV_PORT_CTRL_NUM(port)           (0x1 << port)

/* Port Control port number is currently not executing */
#define MV_PORT_CTRL_PORT_NOT_EXECUTING  (0xFF)

/* Port Control training duration */
#define PORT_CTRL_TRAINING_DURATION      (5)
#define PORT_CTRL_TRAINING_INTERVAL      (100)

#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
#define PORT_CTRL_ETL_INTERVAL      (200)
#endif


/* Port Control system config duration */
#define PORT_CTRL_SYSTEM_CONFIG_DURATION (10)

/* Port Control system config duration.
1000BaseX and _10GBase_KX4 sequence is shorter (not include training) and therefore we check ACK from host for pizza allocation earlier.
This threshold ensure we are waiting enough time for the ACK.*/
#define PORT_CTRL_SYSTEM_CONFIG_DURATION_1000BASEX_10GBase_KX4 (50)

#define PORT_CTRL_TIMER_DEFAULT          (0)

/* Port Control Avago GUI flag */
#define MV_PORT_CTRL_AVAGO_GUI_ENABLED   (0x1)
#define MV_PORT_CTRL_AVAGO_GUI_MASK      (0x1)

/* Port Control port type definition */
typedef enum
{
    REG_PORT = 0,
    AP_PORT  = 1

}MV_PORT_CTRL_TYPE;

typedef enum
{
    INIT_PORT  = 0,
    RESET_PORT = 1

}MV_PORT_CTRL_ACTION;


/* Supervisor module (M1) definition */
/* ================================= */

/* Supervisor module (M1) state definition */
typedef enum
{
    SPV_HIGH_MSG_PROCESS_STATE = 0,
    SPV_LOW_MSG_PROCESS_STATE  = 1,
    SPV_DELAY_STATE            = 2,
    SPV_MAX_STATE              = 3

}MV_SUPERVISOR_STATE;

typedef enum
{
    SPV_HIGH_MSG_EVENT = 0,
    SPV_LOW_MSG_EVENT  = 1,
    SPV_DELAY_EVENT    = 2,
    SPV_MAX_EVENT      = 3

}MV_SUPERVISOR_EVENT;


/* Port Management module (M2) definition */
/* ====================================== */

/* Port Management module (M2) state definition */
typedef enum
{
    PORT_MNG_MSG_PROCESS_STATE = 0,
    PORT_MNG_DELAY_STATE       = 1,
    PORT_MNG_MAX_STATE         = 2

}MV_PORT_MNG_STATE;

typedef enum
{
    PORT_MNG_MSG_PROCESS_EVENT = 0,
    PORT_MNG_DELAY_EVENT       = 1,
    PORT_MNG_MAX_EVENT         = 2

}MV_PORT_MNG_EVENT;

/* Port State Machine module (M3) definition */
/* ========================================= */

/* Port State Machine module (M3) state definition */
typedef enum
{
    PORT_SM_IDLE_STATE            = 0,
    PORT_SM_SERDES_CONFIG_STATE   = 1,
    PORT_SM_SERDES_TRAINING_STATE = 2,
    PORT_SM_MAC_PCS_CONFIG_STATE  = 3,
    PORT_SM_ACTIVE_STATE          = 4,
    PORT_SM_MAX_STATE             = 5

}MV_PORT_SM_STATE;

/* Port State Machine module (M3) status definition */
typedef enum
{
    PORT_SM_NOT_RUNNING                          = 0,
    PORT_SM_START_EXECUTE                        = 1,
    PORT_SM_SERDES_CONFIG_IN_PROGRESS            = 2,
    PORT_SM_SERDES_CONFIG_SUCCESS                = 3,
    PORT_SM_SERDES_CONFIG_FAILURE                = 4,
    PORT_SM_SERDES_TRAINING_IN_PROGRESS          = 5,
    PORT_SM_SERDES_TRAINING_SUCCESS              = 6,
    PORT_SM_SERDES_TRAINING_FAILURE              = 7,
    PORT_SM_SERDES_TRAINING_CONFIG_FAILURE       = 8,
    PORT_SM_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK = 9,
    PORT_SM_MAC_PCS_CONFIG_IN_PROGRESS           = 10,
    PORT_SM_MAC_PCS_CONFIG_SUCCESS               = 11,
    PORT_SM_MAC_PCS_CONFIG_FAILURE               = 12,
    PORT_SM_DELETE_IN_PROGRESS                   = 13,
    PORT_SM_DELETE_SUCCESS                       = 14,
    PORT_SM_DELETE_FAILURE                       = 15,
    PORT_SM_FEC_CONFIG_IN_PROGRESS               = 16,
    PORT_SM_FEC_CONFIG_SUCCESS                   = 17,
    PORT_SM_FEC_CONFIG_FAILURE                   = 18,
    PORT_SM_FC_STATE_SET_IN_PROGRESS             = 19,
    PORT_SM_FC_STATE_SET_SUCCESS                 = 20,
    PORT_SM_FC_STATE_SET_FAILURE                 = 21,
    PORT_SM_SERDES_TRAINING_NOT_READY            = 22,
    PORT_SM_SERDES_POLARITY_SET_IN_PROGRESS      = 23,
    PORT_SM_SERDES_POLARITY_SET_SUCCESS          = 24,
    PORT_SM_SERDES_POLARITY_SET_FAILURE          = 25,
    PORT_SM_SERDES_TRAINING_END_SYS_ACK_RCV      = 26,
    PORT_SM_PORT_ENABLE_FAIL                     = 27,
    PORT_SM_SERDES_ADAPTIVE_TRAINING_START_FAIL  = 28,
    PORT_SM_SERDES_ENHANCE_TUNE_LITE_FAIL        = 29,
    PORT_SM_SERDES_ENHANCE_TUNE_LITE_IN_PROGRESS = 30,
    PORT_SM_SERDES_ENHANCE_TUNE_LITE_SUCCESS     = 31,
    PORT_SM_SERDES_CTLE_ETL_CONFIG_FAIL          = 32,
    PORT_SM_SERDES_ADAPTIVE_RX_TRAINING_SUCCESS  = 33,



    PORT_SM_MAX_STATUS                           = 34


}MV_PORT_SM_STATUS;

/* Port State Machine module (M3) system config ("PIzza Arbiter") definition */
typedef enum
{
    PORT_SM_SERDES_SYSTEM_ENABLE  = 0x01,
    PORT_SM_SERDES_SYSTEM_DISABLE = 0x00

}MV_PORT_SM_SYS_CONFIG;

/* Port State Machine module (M3) system state ("PIzza Arbiter") definition */
typedef enum
{
    PORT_SM_SERDES_SYSTEM_VALID     = 0x01,
    PORT_SM_SERDES_SYSTEM_NOT_VALID = 0x00

}MV_PORT_SM_SYS_STATE;

/* AP Port Management module (M4) definition */
/* ========================================= */

#ifdef AP_PORT_SM
/* AP Port Management module (M4) state definition */
typedef enum
{
    AP_PORT_MNG_ACTIVE_PORT_STATE = 1,
    AP_PORT_MNG_MSG_PROCESS_STATE = 2,
    AP_PORT_MNG_PORT_DELAY_STATE  = 3,
    AP_PORT_MNG_MAX_STATE         = 4

}MV_AP_PORT_MNG_STATE;

typedef enum
{
    AP_PORT_MNG_ACTIVE_PORT_EVENT = 0,
    AP_PORT_MNG_MSG_PROCESS_EVENT = 1,
    AP_PORT_MNG_DELAY_EVENT       = 2,
    AP_PORT_MNG_MAX_EVENT         = 3

}MV_AP_PORT_MNG_EVENT;

/* AP Port Management module (M4) status definition */
typedef enum
{
    AP_PORT_MNG_POWER_UP_START = 1,
    AP_PORT_MNG_POWER_UP_STOP  = 2

}MV_AP_PORT_MNG_STATUS;

/* AP Port State Machine module (M5) definition */
/* ============================================ */

/* AP Port Detect module (M5) state definition */
typedef enum
{
    AP_PORT_DETECT_ACTIVE_PORT_STATE = 1,
    AP_PORT_DETECT_MSG_PROCESS_STATE = 2,
    AP_PORT_DETECT_DELAY_STATE       = 3,
    AP_PORT_DETECT_MAX_STATE         = 4

}MV_AP_PORT_DET_STATE;

/* AP Port Detect module (M5) event definition */
typedef enum
{
    AP_PORT_DETECT_ACTIVE_PORT_EVENT = 0,
    AP_PORT_DETECT_MSG_PROCESS_EVENT = 1,
    AP_PORT_DETECT_DELAY_EVENT       = 2,
    AP_PORT_DETECT_MAX_EVENT         = 3

}MV_AP_PORT_DET_EVENT;

/* AP Port State Machine module (M5) state definition */
typedef enum
{
    AP_PORT_SM_IDLE_STATE       = 0,
    AP_PORT_SM_INIT_STATE       = 1,
    AP_PORT_SM_TX_DISABLE_STATE = 2,
    AP_PORT_SM_RESOLUTION_STATE = 3,
    AP_PORT_SM_ACTIVE_STATE     = 4,
    AP_PORT_SM_DELETE_STATE     = 5,
    AP_PORT_SM_DISABLE_STATE    = 6,

    /*Non AP ports adaptive CTLE STATES*/
    NON_AP_PORT_SM_ADAPTIVE_CTLE_STATE = 7,

    AP_PORT_SM_MAX_STATE        = 8
}MV_AP_PORT_SM_STATE;

/* AP Port State Machine module (M5) status definition */
typedef enum
{
    AP_PORT_START_EXECUTE                  = 0,
    AP_PORT_SERDES_INIT_FAILURE            = 1,
    AP_PORT_INIT_SUCCESS                   = 2,
    AP_PORT_TX_DISABLE_IN_PROGRESS         = 3,
    AP_PORT_TX_DISABLE_FAILURE             = 4,
    AP_PORT_TX_DISABLE_SUCCESS             = 5,
    AP_PORT_RESOLUTION_IN_PROGRESS         = 6,
    AP_PORT_PD_RESOLUTION_IN_PROGRESS      = 7,
    AP_PORT_PD_RESOLUTION_FAILURE          = 8,
    AP_PORT_AP_RESOLUTION_IN_PROGRESS      = 9,
    AP_PORT_AP_NO_RESOLUTION               = 10,
    AP_PORT_AP_RESOLUTION_TIMER_FAILURE    = 11,
    AP_PORT_AP_RESOLUTION_MAX_LOOP_FAILURE = 12,
    AP_PORT_RESOLUTION_SUCCESS             = 13,
    AP_PORT_LINK_CHECK_START               = 14,
    AP_PORT_LINK_CHECK_VALIDATE            = 15,
    AP_PORT_LINK_CHECK_FAILURE             = 16,
    AP_PORT_LINK_CHECK_SUCCESS             = 17,
    AP_PORT_DELETE_IN_PROGRESS             = 18,
    AP_PORT_DELETE_SUCCESS                 = 19,
    AP_PORT_UNKNOWN_HCD                    = 20,
    AP_PORT_AP_RESOLUTION_CHK_CONSORTIUM   = 21,
    AP_PORT_AP_RESOLUTION_CHK_ACK2         = 22,
    AP_PORT_AP_RESOLUTION_NO_MORE_PAGES    = 23,
    AP_PORT_MSG_SEND_FAILURE               = 24,

    /*Non AP ports adaptive CTLE STATUS*/
    NON_AP_ADAPTIVE_CTLE_INIT              = 25,
    NON_AP_ADAPTIVE_CTLE_RUNNING           = 26,

    AP_PORT_ENABLE_WAIT                    = 27,

    AP_PORT_MAX_STATUS                     = 28
}MV_AP_PORT_SM_STATUS;

/* AP Port State Machine module (M5) Resolution result definition */
/*typedef enum
{
    AP_PORT_RESOLUTION_NOT_RESOLVED = 1,
    AP_PORT_RESOLUTION_RESOLVED     = 2

}MV_AP_PORT_RESOLUTION_RESULT;*/

/* AP Port State Machine module (M5) Resolution type definition */
/*typedef enum
{
    AP_PORT_RESOLUTION_1000_BASE_KX = 0,
    AP_PORT_RESOLUTION_10G_BASE_KX4 = 1,
    AP_PORT_RESOLUTION_10G_BASE_R   = 2,
    AP_PORT_RESOLUTION_40G_BASE_R   = 3

}MV_AP_PORT_RESOLUTION_TYPE;*/

/* AP Port State Machine module (M5) FEC definition */
/*typedef enum
{
    AP_PORT_RESOLUTION_FEC_DIS = 0,
    AP_PORT_RESOLUTION_FEC_EN  = 1

}MV_AP_PORT_RESOLUTION_FEC;*/

/* AP Port State Machine module (M5) Signal Detect definition */
/*typedef enum
{
    AP_PORT_RESOLUTION_NO_SIGNAL_DETECT = 0,
    AP_PORT_RESOLUTION_SIGNAL_DETECT    = 1

}MV_AP_PORT_RESOLUTION_SIGNAL_DETECT;*/

/* AP Port State Machine module (M5) CDR Lock definition */
/*typedef enum
{
    AP_PORT_RESOLUTION_NO_CDR_LOCK = 0,
    AP_PORT_RESOLUTION_CDR_LOCK    = 1

}MV_AP_PORT_RESOLUTION_CDR_LOCK;*/

/* AP Port State Machine module (M5) PCS Lock definition */
/*typedef enum
{
    AP_PORT_RESOLUTION_NO_PCS_LOCK = 0,
    AP_PORT_RESOLUTION_PCS_LOCK    = 1

}MV_AP_PORT_RESOLUTION_PCS_LOCK;*/

/* AP Port State Machine module (M5) Rx Flow Control definition */
/*typedef enum
{
    AP_PORT_RESOLUTION_RX_FLOW_CTRL_DIS = 0,
    AP_PORT_RESOLUTION_RX_FLOW_CTRL_EN  = 1

}MV_AP_PORT_RESOLUTION_RX_FLOW_CTRL;*/

/* AP Port State Machine module (M5) Tx Flow Control definition */
/*typedef enum
{
    AP_PORT_RESOLUTION_TX_FLOW_CTRL_DIS = 0,
    AP_PORT_RESOLUTION_TX_FLOW_CTRL_EN  = 1

}MV_AP_PORT_RESOLUTION_TX_FLOW_CTRL;*/

/* AP Port State Machine module (M5) Introp definition */

/* Port Control AP Tx Dis duration */
#define PORT_CTRL_AP_TX_DIS_DURATION        (60)

/* Port Control AP Ability Detect duration */
#define PORT_CTRL_AP_ABILITY_DURATION       (5)
#define PORT_CTRL_AP_ABILITY_MAX_COUNT      (800 /*20*/)
#define PORT_CTRL_AP_ABILITY_FAIL_MAX_COUNT (10)

/* Port Control Link Check duration */
#define PORT_CTRL_AP_LINK_CHECK_MODE        (0)
#define PORT_CTRL_AP_LINK_CHECK_DURATION    (10)
#define PORT_CTRL_AP_LINK_CHECK_MAX_COUNT   (50)

#endif /*AP_PORT_SM*/


#define PORT_CTRL_PD_LINK_CHECK_MODE        (1)
#define PORT_CTRL_PD_LINK_CHECK_DURATION    (25)
#define PORT_CTRL_PD_LINK_CHECK_MAX_COUNT   (10)
#define PORT_CTRL_PD_LINK_CHECK_MAX_COUNT_M7 (20) /* in new aapl we need more time to reach link up */

#ifdef AP_GENERAL_TASK
/* Port General module (M6) definition */
/* =================================== */
typedef enum
{
    PORT_GEN_MSG_PROCESS_STATE = 0,
    PORT_GEN_DELAY_STATE       = 1,
    PORT_GEN_MAX_STATE         = 2

}MV_PORT_GENERAL_STATE;

typedef enum
{
    PORT_GEN_MSG_EVENT   = 0,
    PORT_GEN_DELAY_EVENT = 1,
    PORT_GEN_MAX_EVENT   = 2

}MV_PORT_GENERAL_EVENT;

#define MV_GEN_LINK_DOWN (0x0)
#define MV_GEN_LINK_UP   (0x1)
#define MV_GEN_TUNE_FAIL (0x0)
#define MV_GEN_TUNE_PASS (0x1)
#endif



#ifdef AN_PORT_SM


/* AN Port Detect module (M7) state definition */
typedef enum
{
    AN_PORT_DETECT_ACTIVE_PORT_STATE = 1,
    AN_PORT_DETECT_MSG_PROCESS_STATE = 2,
    AN_PORT_DETECT_DELAY_STATE       = 3,
    AN_PORT_DETECT_MAX_STATE         = 4

}MV_AN_PORT_DET_STATE;

/* AN Port Detect module (M7) event definition */
typedef enum
{
    AN_PORT_DETECT_ACTIVE_PORT_EVENT = 0,
    AN_PORT_DETECT_MSG_PROCESS_EVENT = 1,
    AN_PORT_DETECT_DELAY_EVENT       = 2,
    AN_PORT_DETECT_MAX_EVENT         = 3

}MV_AN_PORT_DET_EVENT;

/* AN Port State Machine module (M7) state definition */
typedef enum
{
    AN_PORT_SM_IDLE_STATE            = 0,
    AN_PORT_SM_AN_STATE              = 1,
    AN_PORT_SM_SERDES_CONFIG_STATE   = 2,
    AN_PORT_SM_LINK_STATUS_STATE     = 3,
    AN_PORT_SM_DELETE_STATE          = 4,
    AN_PORT_SM_DISABLE_STATE         = 5,

    AN_PORT_SM_MAX_STATE             = 6
}MV_AN_PORT_SM_STATE;


/* AN Port State Machine module (M7) status definition */
typedef enum
{
    AN_PORT_START_EXECUTE                        = 0,
    AN_PORT_SERDES_INIT_FAILURE                  = 1,
    AN_PORT_INIT_SUCCESS                         = 2,
    AN_PORT_SET_AN_PARAMETERS                    = 3,
    AN_PORT_RESOLUTION_IN_PROGRESS               = 4,
    AN_PORT_RESOLUTION_SUCCESS                   = 5,
    AN_PORT_RESOLUTION_FAILURE                   = 6,
    AN_PORT_SERDES_CONFIGURATION_IN_PROGRESS     = 7,
    AN_PORT_SERDES_CONFIGURATION_SUCCESS         = 8,
    AN_PORT_SERDES_CONFIGURATION_FAILURE         = 9,
    AN_PORT_SERDES_TRAINING_IN_PROGRESS          = 10,
    AN_PORT_SERDES_TRAINING_NOT_READY            = 11,
    AN_PORT_SERDES_TRAINING_SUCCESS              = 12,
    AN_PORT_SERDES_TRAINING_FAILURE              = 13,
    AN_PORT_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK = 14,
    AN_PORT_SERDES_TRAINING_END_SYS_ACK_RCV      = 15,
    AN_PORT_MAC_PCS_CONFIG_IN_PROGRESS           = 16,
    AN_PORT_MAC_PCS_CONFIG_SUCCESS               = 17,
    AN_PORT_MAC_PCS_CONFIG_FAILURE               = 18,
    AN_PORT_FEC_CONFIG_IN_PROGRESS               = 19,
    AN_PORT_FEC_CONFIG_SUCCESS                   = 20,
    AN_PORT_FEC_CONFIG_FAILURE                   = 21,
    AN_PORT_PORT_ENABLE_FAIL                     = 22,
    AN_PORT_FC_STATE_SET_SUCCESS                 = 23,
    AN_PORT_FC_STATE_SET_FAILURE                 = 24,
    AN_PORT_AN_ASSERT_LINK_IN_PROGRESS           = 25,
    AN_PORT_AN_ASSERT_LINK_SUCCESS               = 26,
    AN_PORT_AN_ASSERT_LINK_FAILURE               = 27,
    AN_PORT_AN_COMPLETE_IN_PROGRESS              = 28,
    AN_PORT_AN_COMPLETE_SUCCESS                  = 29,
    AN_PORT_AN_COMPLETE_FAILURE                  = 30,
    AN_PORT_LINK_CHECK_SUCCESS                   = 31,
    AN_PORT_LINK_CHECK_FAILURE                   = 32,
    AN_PORT_DISABLE_IN_PROGRESS                  = 33,
    AN_PORT_DISABLE_SUCCESS                      = 34,
    AN_PORT_DELETE_IN_PROGRESS                   = 35,
    AN_PORT_DELETE_SUCCESS                       = 36,
    AN_PORT_DELETE_FAILURE                       = 37,
    AN_PORT_SERDES_POWER_UP                      = 38,
    AN_PORT_AN_RESOLUTION_CHK_CONSORTIUM         = 39,
    AN_PORT_AN_RESOLUTION_CHK_ACK2               = 40,
    AN_PORT_AN_RESOLUTION_NO_MORE_PAGES          = 41,
    AN_PORT_ENABLE_WAIT                          = 42,
    AN_PORT_SERDES_TRAINING_IN_PROGRESS_WAIT     = 43,

    AN_PORT_MAX_STATUS                           = 44
}MV_AN_PORT_SM_STATUS;



/**
* @enum MV_AN_SERDES_STATUS
* @endinternal
*
* @brief   AN_SERDES_AN_READ_HCD              - hcd value
*          AN_SERDES_AN_READ_FEC_ENABLE       - enable FC fec
*          AN_SERDES_AN_BASE_PAGE_RX          - base page
*          AN_SERDES_AN_NEXT_PAGE_RX          - next page
*          AN_SERDES_AN_LP_AN_ABLE            - LB enable
*          AN_SERDES_AN_COMPLETE              - complete all the Auto Negotiation,training, port init (serdes, mac, pcs)
*          AN_SERDES_AN_GOOD                  - resulotion success, HCD found
*          AN_SERDES_AN_READ_RSFEC_ENABLE     - enable RS fec
*          AN_SERDES_AN_READ_LINK_CONTROL     - link control status
*          AN_SERDES_AN_READ_TRAINING_RESULTS - training status
*          AN_SERDES_AN_ARB_STATE             - ARB status
*          AN_SERDES_AN_DATA_OUT              - AN_SERDES_AN_READ_HCD + AN_SERDES_AN_READ_FEC_ENABLE + AN_SERDES_AN_READ_RSFEC_ENABLE
*                                              to read the register 0x707 only once.
*/
typedef enum
{
    AN_SERDES_AN_READ_HCD                            =  0,
    AN_SERDES_AN_READ_FEC_ENABLE                     =  1,
    AN_SERDES_AN_BASE_PAGE_RX                        =  2,
    AN_SERDES_AN_NEXT_PAGE_RX                        =  3,
    AN_SERDES_AN_LP_AN_ABLE                          =  4,
    AN_SERDES_AN_COMPLETE                            =  5,
    AN_SERDES_AN_GOOD                                =  6,
    AN_SERDES_AN_READ_RSFEC_ENABLE                   =  7,
    AN_SERDES_AN_READ_LINK_CONTROL                   =  8,
    AN_SERDES_AN_READ_TRAINING_RESULTS               =  9,
    AN_SERDES_AN_ARB_STATE                           = 10,
    AN_SERDES_AN_DATA_OUT                            = 11,

    AN_SERDES_MAX_STATUS                             = 12
}MV_AN_SERDES_STATUS;


/* Port Control AP Ability Detect duration */
#define PORT_CTRL_AN_ABILITY_DURATION       (5)
#define PORT_CTRL_AN_ABILITY_MAX_COUNT      (800 /*20*/)
#define PORT_CTRL_AN_ABILITY_FAIL_MAX_COUNT (10)

/* Port Control Link Check duration */
#define PORT_CTRL_AN_LINK_CHECK_MODE        (0)
#define PORT_CTRL_AN_LINK_CHECK_DURATION    (5/*10*/)
#define PORT_CTRL_AN_LINK_CHECK_MAX_COUNT   (130/*100/ *50*/)
/*Port Control Link Check duration for PAM4*/
#define PORT_CTRL_AN_PAM4_LINK_CHECK_MAX_COUNT   (1000 /*310*/)
#endif/*AN_PORT_SM*/

/* Global OS definitions */
/* ===================== */
#define MV_PROCESS_MSG_TX_DELAY    (0) /* msec */
#define MV_PROCESS_MSG_RX_DELAY    (1) /* msec */
#define MV_PROCESS_MSG_RX_NO_DELAY (0)

#define MV_GENERAL_STACK           (512) /* words - 2K bytes */
#define MV_GENERAL_PRIORITY        (2)   /* 2 - Medium priority */
#define MV_GENERAL_MSG_NUM         (MV_PORT_CTRL_NUM_OF_QUEUE_MSGS)
#define MV_GENERAL_MSG_SIZE        (sizeof (MV_HWS_IPC_CTRL_MSG_STRUCT))

#ifdef AC5_DEV_SUPPORT
#define MV_PORT_STACK              (768) /* words - 3K bytes */
#else
#define MV_PORT_STACK              (512) /* words - 2K bytes */
#endif
#define MV_PORT_PRIORITY           (2)   /* 2 - Medium priority */
#define MV_PORT_MSG_NUM            (MV_PORT_CTRL_NUM_OF_QUEUE_MSGS)
#define MV_PORT_MSG_SIZE           (sizeof (MV_HWS_IPC_CTRL_MSG_STRUCT))

#define MV_AP_STACK                (768) /* words - 3K bytes */
#define MV_AP_PRIORITY             (3)   /* 3 - High priority */
#define MV_AP_MSG_NUM              (MV_PORT_CTRL_NUM_OF_QUEUE_MSGS)
#define MV_AP_MSG_SIZE             (sizeof (MV_HWS_IPC_CTRL_MSG_STRUCT))

#define MV_SUPERVISOR_STACK        (512) /* words - 2K bytes */
#define MV_SUPERVISOR_PRIORITY     (4)   /* 4- Very high priority */

#define MV_PORT_MAINTANANCE_STACK       (400) /* words - 2K bytes */
#define MV_PORT_MAINTANANCE_PRIORITY    (1)   /* 3 - Low priority */

#ifdef FREE_RTOS_HWS_ENHANCED_PRINT_MODE

#define MV_DEBUG_STACK             (128) /* words - 512 bytes */
#define MV_DEBUG_PRIORITY          (1)   /* 1- Low priority */

#endif /* FREE_RTOS_HWS_ENHANCED_PRINT_MODE */

/* Global Port Ctrl Debug definitions */
/* ================================== */
#define PORT_CTRL_DBG_REG_DUMP_DISABLE (0)
#define PORT_CTRL_DBG_REG_INT_NONE     (0x0)
#define PORT_CTRL_DBG_REG_INT_REDUCE   (0x1)
#define PORT_CTRL_DBG_REG_INT_FULL     (0x2)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsPortCtrlDefines_H */


