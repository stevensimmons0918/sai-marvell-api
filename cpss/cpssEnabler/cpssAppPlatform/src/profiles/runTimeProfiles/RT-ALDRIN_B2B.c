/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file RT-ALDRIN_B2B.c
*
* @brief Run time profiles for aldrin Pp.
*
* @version   1
********************************************************************************/

#include <profiles/cpssAppPlatformProfile.h>

static CPSS_UNI_EV_CAUSE_ENT genEventsArr[] = {

                           CPSS_PP_MISC_ILLEGAL_ADDR_E,
                           CPSS_PP_PORT_RX_FIFO_OVERRUN_E, CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,
                           CPSS_PP_PORT_TX_FIFO_OVERRUN_E, CPSS_PP_PORT_TX_UNDERRUN_E,
                           CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E, CPSS_PP_EB_NA_FIFO_FULL_E,
                           CPSS_PP_MAC_SFLOW_E, CPSS_PP_MAC_NUM_OF_HOP_EXP_E,
                           CPSS_PP_MAC_NA_LEARNED_E, CPSS_PP_MAC_NA_NOT_LEARNED_E,
                           CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E, CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,
                           CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E, CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E, CPSS_PP_RX_CNTR_OVERFLOW_E,
                           CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E, CPSS_PP_POLICER_DATA_ERR_E,
                           CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E, CPSS_PP_POLICER_IPFIX_ALARM_E,
                           CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E, CPSS_PP_TQ_MISC_E,
                           CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E, CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,
                           CPSS_PP_PCL_LOOKUP_FIFO_FULL_E, CPSS_PP_CNC_WRAPAROUND_BLOCK_E,
                           CPSS_PP_SCT_RATE_LIMITER_E, CPSS_PP_EGRESS_SFLOW_E,
                           CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E, CPSS_PP_TTI_ACCESS_DATA_ERROR_E,
                           CPSS_PP_GTS_GLOBAL_FIFO_FULL_E, CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,
                           CPSS_PP_TX_END_E, CPSS_PP_TX_ERR_QUEUE_E,
                           CPSS_PP_CRITICAL_HW_ERROR_E, CPSS_PP_DATA_INTEGRITY_ERROR_E,
                           CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E, CPSS_PP_PORT_EEE_E,
                           CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E, CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,
                           CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E, CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,
                           CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E, CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,
                           CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E, CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,
                           CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E, CPSS_PP_PTP_TAI_GENERATION_E,
                           CPSS_PP_BM_MISC_E,
                           CPSS_PP_PORT_PTP_MIB_FRAGMENT_E
                         };

static CPSS_UNI_EV_CAUSE_ENT linkChangeEventsArr[] = {
                           CPSS_PP_PORT_LINK_STATUS_CHANGED_E, CPSS_PP_PORT_AN_COMPLETED_E,
                           CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E, CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E,
                           CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E
                         };

static CPSS_UNI_EV_CAUSE_ENT auEventsArr[] = {
                           CPSS_PP_EB_AUQ_ALMOST_FULL_E,
                           CPSS_PP_EB_AUQ_FULL_E, CPSS_PP_EB_AUQ_OVER_E,
                           CPSS_PP_EB_FUQ_PENDING_E, CPSS_PP_EB_FUQ_FULL_E,
                           CPSS_PP_EB_SECURITY_BREACH_UPDATE_E, CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E
                         };


/* Generic Event call back function */
GT_STATUS cpssAppRefGenEventHandle
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
);

/* AU Event call back function */
GT_STATUS cpssAppRefAuEventHandle
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
);

/* Link change Event call back function */
GT_STATUS cpssAppRefLinkChangeEvHandle
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
);

/* Gen Event handler */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  gen_event_handle_aldrin_b2b =
{
    _SM_(eventListPtr     ) genEventsArr,
    _SM_(numOfEvents      ) sizeof(genEventsArr)/sizeof(genEventsArr[0]),
    _SM_(callbackFuncPtr  ) cpssAppRefGenEventHandle,
    _SM_(taskPriority     ) 200
};

/* AU Event handler */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  au_event_handle_aldrin_b2b =
{
    _SM_(eventListPtr     ) auEventsArr,
    _SM_(numOfEvents      ) sizeof(auEventsArr)/sizeof(auEventsArr[0]),
    _SM_(callbackFuncPtr  ) cpssAppRefAuEventHandle,
    _SM_(taskPriority     ) 193
};

/* Link Change Event handler */
CPSS_APP_PLATFORM_EVENT_HANDLE_STC  link_change_event_handle_aldrin_b2b =
{
    _SM_(eventListPtr     ) linkChangeEventsArr,
    _SM_(numOfEvents      ) sizeof(linkChangeEventsArr)/sizeof(linkChangeEventsArr[0]),
    _SM_(callbackFuncPtr  ) cpssAppRefLinkChangeEvHandle,
    _SM_(taskPriority     ) 191
};

/*------------------*
 * Aldrin  B2B      *
 *------------------*/

static CPSS_APP_PLATFORM_PORT_CONFIG_STC portInitlist_Aldrin_b2b_0[] =
{
     { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {17, 40, 1,      APP_INV_PORT_CNS},  CPSS_PORT_INTERFACE_MODE_QSGMII_E, CPSS_PORT_SPEED_1000_E  , CPSS_PORT_FEC_MODE_DISABLED_E}
    , { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {49, 50, 1,      APP_INV_PORT_CNS},  CPSS_PORT_INTERFACE_MODE_SR_LR_E, CPSS_PORT_SPEED_10000_E , CPSS_PORT_FEC_MODE_DISABLED_E}
    , { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {53, 57, 2,      APP_INV_PORT_CNS},  CPSS_PORT_INTERFACE_MODE_KR2_E, CPSS_PORT_SPEED_25000_E   , CPSS_PORT_FEC_MODE_DISABLED_E}
    ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E,     {               APP_INV_PORT_CNS},  CPSS_PORT_INTERFACE_MODE_NA_E,  CPSS_PORT_SPEED_NA_E        , CPSS_PORT_FEC_MODE_DISABLED_E}
};

static CPSS_APP_PLATFORM_PORT_CONFIG_STC portInitlist_Aldrin_b2b_1[] =
{
     { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {1, 16, 1,      APP_INV_PORT_CNS},  CPSS_PORT_INTERFACE_MODE_QSGMII_E, CPSS_PORT_SPEED_1000_E , CPSS_PORT_FEC_MODE_DISABLED_E}
    , { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {41, 48, 1,      APP_INV_PORT_CNS},  CPSS_PORT_INTERFACE_MODE_QSGMII_E, CPSS_PORT_SPEED_1000_E , CPSS_PORT_FEC_MODE_DISABLED_E}
    , { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {51, 52, 1,      APP_INV_PORT_CNS},  CPSS_PORT_INTERFACE_MODE_SR_LR_E, CPSS_PORT_SPEED_10000_E , CPSS_PORT_FEC_MODE_DISABLED_E}
    , { CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E,  {53, 57, 2,      APP_INV_PORT_CNS},  CPSS_PORT_INTERFACE_MODE_KR2_E, CPSS_PORT_SPEED_25000_E , CPSS_PORT_FEC_MODE_DISABLED_E}
    ,{ CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E,     {               APP_INV_PORT_CNS},  CPSS_PORT_INTERFACE_MODE_NA_E,  CPSS_PORT_SPEED_NA_E      , CPSS_PORT_FEC_MODE_DISABLED_E}
};

/*
static CPSS_APP_PLATFORM_QUAD_PHY_CFG_STC  macSec_PTP_disable_PHY_88E1680M_RevA0_Init_Array[] =
*/
static CPSS_APP_PLATFORM_PHY_CFG_DATA_STC  phyInitArray[] =
{
    /* for all/first port, regaddr, regvalue, delay if need in ms */
     { 22, 0x0000 } /* ensure we on page 0 */
    ,{  4, 0x01E1 } /* RW u1 P0-3 R4 H01E1 - restore default of register #4
                       that may be overridden by BC2 B0 during power-up */
    ,{ 16, 0x3370 } /* PHY power up (reg 0x10_0.2=0 and reg 0x0_0.11=0) */
    ,{  0, 0x1140 }
    ,{ 22, 0x0003 } /* Link indication config */
    ,{ 16, 0x1117 }
    ,{ 22, 0x0004 } /* Link indication config */
    ,{ 27, 0x3FA0 }
    ,{ 22, 0x0000 }
    ,{ 22, 0x0012 } /* MACSec and PTP disable */
    ,{ 27, 0x0000 }
    ,{ 22, 0x0000 }
    ,{ 22, 0x00FD } /* QSGMII Tx Amp change */
    ,{  8, 0x0B53 }
    ,{  7, 0x200D }
    ,{ 22, 0x00FF } /* EEE Initialization */
    ,{ 17, 0xB030 }
    ,{ 16, 0x215C }
    ,{ 22, 0x0000 }
    ,{  0, 0x9140 }  /* Soft Reset */
};

#define CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(x) \
        (x), (sizeof(x)/sizeof(x[0]))

/* External Phy Config */
CPSS_APP_PLATFORM_EXT_PHY_PORT_MAP extPhyConfig_b2b_0_aldrin[] =
{
 /* { 0, {0, {2}, 0}, {&CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
   { 0, {0, {2}, 0}, {&CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} }*/
{18,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x00},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{17,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x01},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{20,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x02},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{19,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x03},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{22,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x04},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{21,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x05},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{24,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x06},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{23,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x07},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{38,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x0c},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{37,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x0d},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{40,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x0e},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{39,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x0f},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{34,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x08},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{33,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x09},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{36,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x0a},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{35,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x0b},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{30,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x04},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{29,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x05},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{32,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x06},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{31,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x07},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{26,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x00},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{25,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x01},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{28,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x02},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{27,{CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x03},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} }
};

CPSS_APP_PLATFORM_EXT_PHY_PORT_MAP extPhyConfig_b2b_1_aldrin[] =
{
{46, {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x00},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{45, {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x01},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{48, {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x02},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{47, {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x03},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{42, {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x04},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{41, {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x05},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{44, {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x06},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{43, {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_0_E}, 0x07},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{14, {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x0c},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{13, {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x0d},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{16, {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x0e},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{15, {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x0f},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{10, {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x08},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{9,  {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x09},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{12, {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x0a},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{11, {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x0b},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{6,  {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x04},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{5,  {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x05},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{8,  {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x06},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{7,  {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x07},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{2,  {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x00},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{1,  {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x01},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{4,  {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x02},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} },
{3,  {CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E ,{CPSS_PHY_SMI_INTERFACE_1_E}, 0x03},{CPSS_CAP_PHY_CONFIG_PTR_AND_SIZE(phyInitArray)} }
};

CPSS_APP_PLATFORM_PORT_MANAGER_HANDLE_STC portManager_ports_param_set_aldrin_b2b =
{
    _SM_(taskPriority     ) 500,
    _SM_(portManagerFunc  ) NULL
};

CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC traffic_enable_info_b2b_0_aldrin =
{
    _SM_(devNum           ) 0,
    _SM_(portTypeListPtr  ) portInitlist_Aldrin_b2b_0,
    _SM_(phyMapListPtr    ) extPhyConfig_b2b_0_aldrin,
    _SM_(phyMapListPtrSize) 24
};

CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC traffic_enable_info_b2b_1_aldrin =
{
    _SM_(devNum           ) 1,
    _SM_(portTypeListPtr  ) portInitlist_Aldrin_b2b_1,
    _SM_(phyMapListPtr    ) extPhyConfig_b2b_1_aldrin,
    _SM_(phyMapListPtrSize) 24
};

/*Run Time profile for generic event Handler (without AP events) - aldrin b2b*/
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_ALDRIN_B2B_GEN_EVENT_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &gen_event_handle_aldrin_b2b}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/*Run Time profile for port manager */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_PORT_MANAGER_INFO_ALDRIN_B2B[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_PORT_MANAGER_HANDLE_E,
#ifdef ANSI_PROFILES
      {_SM_(portManagerHandlePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&portManager_ports_param_set_aldrin_b2b}
#else
      {_SM_(portManagerHandlePtr) &portManager_ports_param_set_aldrin_b2b}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

/*Aldrin B2B - Application runtime profile */
CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_EVENTS_B2B_ALDRIN_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &au_event_handle_aldrin_b2b}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
      {_SM_(eventHandlePtr) &link_change_event_handle_aldrin_b2b}
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_TRAFFIC_B2B_0_ALDRIN_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
#ifdef ANSI_PROFILES
      {_SM_(trafficEnablePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&traffic_enable_info_b2b_0_aldrin}
#else
      {_SM_(trafficEnablePtr) &traffic_enable_info_b2b_0_aldrin}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};

CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC RUNTIME_TRAFFIC_B2B_1_ALDRIN_INFO[] =
{
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
#ifdef ANSI_PROFILES
      {_SM_(trafficEnablePtr) (CPSS_APP_PLATFORM_EVENT_HANDLE_STC*)&traffic_enable_info_b2b_1_aldrin}
#else
      {_SM_(trafficEnablePtr) &traffic_enable_info_b2b_1_aldrin}
#endif
    },
    { _SM_(runtimeInfoType) CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E,
      {_SM_(lastPtr) NULL}
    }
};
