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
* @file prvTgfEventDeviceGenerate.h
* @version   1
********************************************************************************
*/

#ifndef __prvTgfEventDeviceGenerate
#define __prvTgfEventDeviceGenerate

#include <utf/utfMain.h>

/* Size of array of failed events */
#define PRV_TGF_FAILED_EVENTS_ARRAY_SIZE 100
/* List of events disabled by default */
#define PRV_TGF_DISABLED_BY_DEFAULT_EVENTS {                              \
                                      CPSS_PP_MISC_TWSI_TIME_OUT_E        \
                                     ,CPSS_PP_MISC_TWSI_STATUS_E          \
                                     ,CPSS_PP_TQ_PORT_DESC_FULL_E         \
                                     ,CPSS_PP_MAC_UPDATE_FROM_CPU_DONE_E  \
                                     ,CPSS_PP_MAC_NA_SELF_LEARNED_E       \
                                     ,CPSS_PP_MAC_NA_FROM_CPU_DROPPED_E   \
                                     ,CPSS_PP_MAC_AGED_OUT_E              \
                                     ,CPSS_PP_XSMI_WRITE_DONE_E           \
                                            }

/* List of unsupported events */
#define PRV_TGF_UNSUPPORTED_EVENTS {  CPSS_PP_PORT_802_3_AP_E             \
                                     ,CPSS_PP_CRITICAL_HW_ERROR_E         \
                                     ,CPSS_PP_RX_BUFFER_QUEUE0_E          \
                                     ,CPSS_PP_RX_BUFFER_QUEUE1_E          \
                                     ,CPSS_PP_RX_BUFFER_QUEUE2_E          \
                                     ,CPSS_PP_RX_BUFFER_QUEUE3_E          \
                                     ,CPSS_PP_RX_BUFFER_QUEUE4_E          \
                                     ,CPSS_PP_RX_BUFFER_QUEUE5_E          \
                                     ,CPSS_PP_RX_BUFFER_QUEUE6_E          \
                                     ,CPSS_PP_RX_BUFFER_QUEUE7_E          \
                                     ,CPSS_PP_RX_ERR_QUEUE0_E             \
                                     ,CPSS_PP_RX_ERR_QUEUE1_E             \
                                     ,CPSS_PP_RX_ERR_QUEUE2_E             \
                                     ,CPSS_PP_RX_ERR_QUEUE3_E             \
                                     ,CPSS_PP_RX_ERR_QUEUE4_E             \
                                     ,CPSS_PP_RX_ERR_QUEUE5_E             \
                                     ,CPSS_PP_RX_ERR_QUEUE6_E             \
                                     ,CPSS_PP_RX_ERR_QUEUE7_E             \
                                     ,CPSS_PP_RX_CNTR_OVERFLOW_E          \
                                     ,CPSS_PP_TX_BUFFER_QUEUE_E           \
                                     ,CPSS_PP_TX_ERR_QUEUE_E              \
                                     ,CPSS_PP_TX_END_E                    \
                                     ,CPSS_PP_BM_INVALID_ADDRESS_E        \
                                    }

/* List of unsupported events for SIP6 and above devices */
#define PRV_TGF_SIP6_UNSUPPORTED_EVENTS {  CPSS_PP_PORT_802_3_AP_E                  \
                                          ,CPSS_PP_RX_BUFFER_QUEUE0_E               \
                                          ,CPSS_PP_RX_BUFFER_QUEUE1_E               \
                                          ,CPSS_PP_RX_BUFFER_QUEUE2_E               \
                                          ,CPSS_PP_RX_BUFFER_QUEUE3_E               \
                                          ,CPSS_PP_RX_BUFFER_QUEUE4_E               \
                                          ,CPSS_PP_RX_BUFFER_QUEUE5_E               \
                                          ,CPSS_PP_RX_BUFFER_QUEUE6_E               \
                                          ,CPSS_PP_RX_BUFFER_QUEUE7_E               \
                                          ,CPSS_PP_RX_ERR_QUEUE0_E                  \
                                          ,CPSS_PP_RX_ERR_QUEUE1_E                  \
                                          ,CPSS_PP_RX_ERR_QUEUE2_E                  \
                                          ,CPSS_PP_RX_ERR_QUEUE3_E                  \
                                          ,CPSS_PP_RX_ERR_QUEUE4_E                  \
                                          ,CPSS_PP_RX_ERR_QUEUE5_E                  \
                                          ,CPSS_PP_RX_ERR_QUEUE6_E                  \
                                          ,CPSS_PP_RX_ERR_QUEUE7_E                  \
                                          ,CPSS_PP_RX_CNTR_OVERFLOW_E               \
                                          ,CPSS_PP_TX_BUFFER_QUEUE_E                \
                                          ,CPSS_PP_TX_ERR_QUEUE_E                   \
                                          ,CPSS_PP_TX_END_E                         \
                                          ,CPSS_PP_BM_INVALID_ADDRESS_E             \
                                          ,CPSS_PP_MACSEC_SA_EXPIRED_E              \
                                          ,CPSS_PP_MACSEC_SA_PN_FULL_E              \
                                          ,CPSS_PP_MACSEC_EGRESS_SEQ_NUM_ROLLOVER_E \
                                          ,CPSS_PP_MACSEC_STATISTICS_SUMMARY_E      \
                                          ,CPSS_PP_MACSEC_TRANSFORM_ENGINE_ERR_E    \
                                         }

/* PRV_PRINT_EVENT_CTX types is used for printing failed events*/
typedef struct
{
    CPSS_UNI_EV_CAUSE_ENT event; /* event to print */
    GT_STATUS rc;                /* return code from
                                 cpssEventDeviceGenerate function*/
    GT_U32 counter;              /* event counter value */
    GT_U32 evExtData;
} PRV_PRINT_EVENT_CTX;

GT_VOID prvTgfGenEvent(GT_VOID);
GT_VOID prvTgfPhyToMacPortMapping(GT_VOID);
GT_VOID prvTgfEventPortLinkStatusUnbindBind(GT_VOID);
GT_VOID prvTgfInterruptCoalescingWireSpeedTrafficGenerate(GT_BOOL coalescingEnable);
GT_VOID prvTgfInterruptCoalescingWireSpeedConfigurationRestore(GT_VOID);
GT_VOID prvTgfInterruptCoalescingConfig(GT_BOOL config, GT_BOOL linkChangeOverride);
GT_VOID prvTgfInterruptCoalescingForLinkChangeEvent(GT_BOOL linkChangeOverride);

#endif /* __prvTgfEventDeviceGenerate */

