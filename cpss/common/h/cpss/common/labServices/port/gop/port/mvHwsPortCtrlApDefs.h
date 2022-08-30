/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsPortCtrlApDefs.h
*
* @brief AP Global definitions
*
* @version   17
********************************************************************************
*/

#ifndef __mvHwServicesPortCtrlApDefs_H
#define __mvHwServicesPortCtrlApDefs_H

#ifdef __cplusplus
extern "C" {
#endif

#define AP_PRINT_MAC(x) /* \
    osPrintf("%d: ",mvPortCtrlCurrentTs());\
    osPrintf x*/
#define AP_PRINT_MAC2(x) /*\
    osPrintf("%d: ",mvPortCtrlCurrentTs());\
    osPrintf x*/

#define AP_PORT_SM_SET_STATUS(port, dataStc ,newStatus)              \
    dataStc->status = newStatus;         /*                             \
    AP_PRINT_MAC2(("psm: P %d ST %d se %d l:%d\n",port, newStatus, dataStc->state, __LINE__))*/


#define AP_PORT_SM_SET_STATE(port, dataStc, newState)               \
    dataStc->state = newState;                           /*           \
    AP_PRINT_MAC2(("psm: P %d st %d SE %d l:%d\n",port, dataStc->status, newState, __LINE__))*/

#define AN_PORT_SM_SET_STATUS(port, dataStc ,newStatus)              \
    dataStc->status = newStatus;                                     \
    mvPortCtrlLogAddStatus(port,((dataStc->state<<8)|dataStc->status));
    /*AP_PRINT_MAC2(("psm: P %d ST %d se %d l:%d\n",port, newStatus, dataStc->state, __LINE__))*/


#define AN_PORT_SM_SET_STATE(port, dataStc, newState)               \
    dataStc->state = newState;                                      \
    mvPortCtrlLogAddStatus(port,((dataStc->state<<8)|dataStc->status));
    /*AP_PRINT_MAC2(("psm: P %d st %d SE %d l:%d\n",port, dataStc->status, newState, __LINE__))*/

/* tbd undef */
/*#define LINK_BINDING_PRINT*/
#ifdef LINK_BINDING_PRINT
  #define LKB_PRINT(x, ...)  osPrintf("" x, ##__VA_ARGS__)
#else
  #define LKB_PRINT(x, ...)
#endif


/* Port Control AP Port State machine module (M4) table */
/* Port Control AP Port State machine module (M5) table */

#if defined (RAVEN_DEV_SUPPORT) || ( defined (FALCON_DEV_SUPPORT) && defined (MICRO_INIT) )
#define AN_PORT_SM /*(M7)*/
#else
#define AP_PORT_SM /*(M4)(M5)*/
#endif

/*******************************************************************************
*                         AP Global Definition                                 *
* ============================================================================ *
*******************************************************************************/

/* AP Port Numbering Definition */
#ifdef BOBK_DEV_SUPPORT

#define MV_PORT_CTRL_MAX_AP_PORT_NUM  (16)
#define MV_PORT_CTRL_AP_PORT_NUM_BASE (56)

#elif defined ALDRIN_DEV_SUPPORT

#define MV_PORT_CTRL_MAX_AP_PORT_NUM  (32)
#define MV_PORT_CTRL_AP_PORT_NUM_BASE (0)

#elif defined BC3_DEV_SUPPORT

#define MV_PORT_CTRL_MAX_AP_PORT_NUM  (74)
#define MV_PORT_CTRL_AP_PORT_NUM_BASE (0)

#elif defined PIPE_DEV_SUPPORT

#define MV_PORT_CTRL_MAX_AP_PORT_NUM  (16)
#define MV_PORT_CTRL_AP_PORT_NUM_BASE (0)

#elif defined ALDRIN2_DEV_SUPPORT

#define MV_PORT_CTRL_MAX_AP_PORT_NUM  (73)
#define MV_PORT_CTRL_AP_PORT_NUM_BASE (0)

#elif defined RAVEN_DEV_SUPPORT

#define MV_PORT_CTRL_MAX_AP_PORT_NUM  (17)
#define MV_PORT_CTRL_AP_PORT_NUM_BASE (0)

#elif defined AC5_DEV_SUPPORT

#define MV_PORT_CTRL_MAX_AP_PORT_NUM  (6)
#define MV_PORT_CTRL_AP_PORT_NUM_BASE (24)

#else /*BC2 or AC3 or AC5*/

#define MV_PORT_CTRL_MAX_AP_PORT_NUM  (24)
#define MV_PORT_CTRL_AP_PORT_NUM_BASE (48)

#define MV_PORT_CTRL_BC2_MAX_AP_PORT_NUM  (24)
#define MV_PORT_CTRL_BC2_AP_PORT_NUM_BASE (48)

#define MV_PORT_CTRL_AC3_MAX_AP_PORT_NUM  (6)
#define MV_PORT_CTRL_AC3_AP_PORT_NUM_BASE (24)

#endif

#define MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET(phyPort, portNumBase) \
                                            (phyPort - portNumBase)
#define MV_HWS_PORT_CTRL_AP_PHYSICAL_PORT_GET(phyPort, portNumBase) \
                                             (phyPort + portNumBase)

/* Port Control modules definition */
typedef enum
{
    M0_DEBUG           = 0,
    M1_SUPERVISOR      = 1,
    M2_PORT_MNG        = 2,
    M3_PORT_SM         = 3,
    M4_AP_PORT_MNG     = 4,
    M5_AP_PORT_DET     = 5,
    M5_AP_PORT_DET_EXT = 6,
#ifdef AP_GENERAL_TASK
    M6_GENERAL         = 7,
    MAX_MODULE         = 8
#else
    M7_AN_PORT_SM      = 7,
    M8_MTC_PORT_SM     = 8,
    M9_AN_PORT_SM2     = 9,
    MAX_MODULE         = 10

#endif

}MV_PORT_CTRL_MODULE;


typedef enum
{
    PORT_CTRL_AP_REG_CFG_0                                   = 1,
    PORT_CTRL_AP_REG_CFG_1                                   = 2,
    PORT_CTRL_AP_REG_ST_0                                    = 3,
    PORT_CTRL_AP_INT_REG_802_3_AP_CTRL                       = 4,
    PORT_CTRL_AP_INT_REG_802_3_AP_ST                         = 5,
    PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_1                  = 6,
    PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_2                  = 7,
    PORT_CTRL_AP_INT_REG_802_3_AP_ADV_REG_3                  = 8,
    PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_1         = 9,
    PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_2         = 10,
    PORT_CTRL_AP_INT_REG_802_3_LP_BASE_ABILITY_REG_3         = 11,
    PORT_CTRL_AP_INT_REG_ANEG_CTRL_0                         = 12,
    PORT_CTRL_AP_INT_REG_ANEG_CTRL_1                         = 13,
    PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_REG                = 14,
    PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15          = 15,
    PORT_CTRL_AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31         = 16,
    PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_REG        = 17,
    PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_0_15  = 18,
    PORT_CTRL_AP_INT_REG_802_3_LP_NEXT_PG_ABILITY_CODE_16_31 = 19,
    PORT_CTRL_AP_INT_REG_802_3_BP_ETH_ST_REG                 = 20,
    PORT_CTRL_AP_INT_REG_802_3_LP_ADV_REG                    = 21,
    PORT_CTRL_AP_REG_ST_1                                    = 22,
    PORT_CTRL_AP_REG_CFG_2                                   = 23,
    PORT_CTRL_AP_REG_O_CORE                                  = 24,
    PORT_CTRL_MAX_AP_REGS

}MV_PORT_CTRL_AP_REGS;


#define AP_CTRL_GET(param, mask, shift) (((param) >> (shift)) & (mask))
#define AP_CTRL_SET(param, mask, shift, val) \
{ \
    param &= (~((mask) << (shift))); \
    param |= (((val) & (mask)) << (shift)); \
}

/*
** AP Configuration
** ================
*/

/*
** Interface Number
** [00:07] Lane Number
** [08:15] PCS Number
** [16:23] MAC Number
** [24:31] Queue Id
*/
#define AP_CTRL_LANE_MASK                  (0xFF)
#define AP_CTRL_LANE_SHIFT                 (0)
#define AP_CTRL_LANE_GET(ifNum)            AP_CTRL_GET(ifNum, AP_CTRL_LANE_MASK, AP_CTRL_LANE_SHIFT)
#define AP_CTRL_LANE_SET(ifNum, val)       AP_CTRL_SET(ifNum, AP_CTRL_LANE_MASK, AP_CTRL_LANE_SHIFT, val)

#define AP_CTRL_PCS_MASK                   (0xFF)
#define AP_CTRL_PCS_SHIFT                  (8)
#define AP_CTRL_PCS_GET(ifNum)             AP_CTRL_GET(ifNum, AP_CTRL_PCS_MASK, AP_CTRL_PCS_SHIFT)
#define AP_CTRL_PCS_SET(ifNum, val)        AP_CTRL_SET(ifNum, AP_CTRL_PCS_MASK, AP_CTRL_PCS_SHIFT, val)

#define AP_CTRL_MAC_MASK                   (0xFF)
#define AP_CTRL_MAC_SHIFT                  (16)
#define AP_CTRL_MAC_GET(ifNum)             AP_CTRL_GET(ifNum, AP_CTRL_MAC_MASK, AP_CTRL_MAC_SHIFT)
#define AP_CTRL_MAC_SET(ifNum, val)        AP_CTRL_SET(ifNum, AP_CTRL_MAC_MASK, AP_CTRL_MAC_SHIFT, val)

#define AP_CTRL_QUEUEID_MASK                   (0xFF)
#define AP_CTRL_QUEUEID_SHIFT                  (24)
#define AP_CTRL_QUEUEID_GET(ifNum)             AP_CTRL_GET(ifNum, AP_CTRL_QUEUEID_MASK, AP_CTRL_QUEUEID_SHIFT)
#define AP_CTRL_QUEUEID_SET(ifNum, val)        AP_CTRL_SET(ifNum, AP_CTRL_QUEUEID_MASK, AP_CTRL_QUEUEID_SHIFT, val)

/*
** Capability
** [00:00] Advertisement 40GBase KR4
** [01:01] Advertisement 10GBase KR
** [02:02] Advertisement 10GBase KX4
** [03:03] Advertisement 1000Base KX
** [04:04] Advertisement 20GBase KR2
** [05:05] Advertisement 10GBase KX2
** [06:06] Advertisement 100GBase KR4
** [07:07] Advertisement 100GBase CR4
** [08:08] Advertisement 25GBASE-KR-S                           0x00100
** [09:09] Advertisement 25GBASE-KR                             0x00200
** [10:10] Advertisement consortium 25GBase KR1                 0x00400
** [11:11] Advertisement consortium 25GBase CR1                 0x00800
** [12:12] Advertisement consortium 50GBase KR2                 0x01000
** [13:13] Advertisement consortium 50GBase CR2                 0x02000
** [14:14] Advertisement 40Base CR4                             0x04000
** [15:15] Advertisement 25GBASE-CR-S                           0x08000
** [16:16] Advertisement 25GBASE-CR                             0x10000
** [17:17] Advertisement 50GBASE-KR                             0x20000
** [18:18] Advertisement 100GBase_KR2                           0x40000
** [19:19] Advertisement 200GBase_KR4                           0x80000
** [20:20] Advertisement 200GBase_KR8                           0x100000
** [21:21] Advertisement 400GBase_KR8                           0x200000
** [22:22] Advertisement 50GBASE-CR                             0x400000
** [23:23] Advertisement 100GBase_CR2                           0x800000
** [24:24] Advertisement 200GBase_CR4                           0x1000000
** [25:25] Advertisement 200GBase_CR8                           0x2000000
** [26:26] Advertisement 400GBase_CR8                           0x4000000
** [27:31] Reserved
*/
#define AP_CTRL_ADV_MASK                   (0x1)
#define AP_CTRL_40GBase_KR4_SHIFT          (0)
#define AP_CTRL_40GBase_KR4_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_40GBase_KR4_SHIFT)
#define AP_CTRL_40GBase_KR4_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_40GBase_KR4_SHIFT, val)

#define AP_CTRL_10GBase_KR_SHIFT           (1)
#define AP_CTRL_10GBase_KR_GET(adv)        AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_10GBase_KR_SHIFT)
#define AP_CTRL_10GBase_KR_SET(adv, val)   AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_10GBase_KR_SHIFT, val)

#define AP_CTRL_10GBase_KX4_SHIFT          (2)
#define AP_CTRL_10GBase_KX4_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_10GBase_KX4_SHIFT)
#define AP_CTRL_10GBase_KX4_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_10GBase_KX4_SHIFT, val)

#define AP_CTRL_1000Base_KX_SHIFT          (3)
#define AP_CTRL_1000Base_KX_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_1000Base_KX_SHIFT)
#define AP_CTRL_1000Base_KX_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_1000Base_KX_SHIFT, val)

#define AP_CTRL_20GBase_KR2_SHIFT          (4)
#define AP_CTRL_20GBase_KR2_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_20GBase_KR2_SHIFT)
#define AP_CTRL_20GBase_KR2_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_20GBase_KR2_SHIFT, val)

#define AP_CTRL_10GBase_KX2_SHIFT          (5)
#define AP_CTRL_10GBase_KX2_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_10GBase_KX2_SHIFT)
#define AP_CTRL_10GBase_KX2_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_10GBase_KX2_SHIFT, val)

#define AP_CTRL_100GBase_KR4_SHIFT         (6)
#define AP_CTRL_100GBase_KR4_GET(adv)      AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_100GBase_KR4_SHIFT)
#define AP_CTRL_100GBase_KR4_SET(adv, val) AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_100GBase_KR4_SHIFT, val)

#define AP_CTRL_100GBase_CR4_SHIFT         (7)
#define AP_CTRL_100GBase_CR4_GET(adv)      AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_100GBase_CR4_SHIFT)
#define AP_CTRL_100GBase_CR4_SET(adv, val) AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_100GBase_CR4_SHIFT, val)

#define AP_CTRL_ADV_100G_R4_ALL_MASK           (0x3)
#define AP_CTRL_ADV_100G_R4_ALL_GET(adv)   AP_CTRL_GET(adv, AP_CTRL_ADV_100G_R4_ALL_MASK, AP_CTRL_100GBase_KR4_SHIFT)

#define AP_CTRL_25GBase_KR1S_SHIFT         (8)
#define AP_CTRL_25GBase_KR1S_GET(adv)      AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_KR1S_SHIFT)
#define AP_CTRL_25GBase_KR1S_SET(adv, val) AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_KR1S_SHIFT, val)

#define AP_CTRL_25GBase_KR1_SHIFT          (9)
#define AP_CTRL_25GBase_KR1_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_KR1_SHIFT)
#define AP_CTRL_25GBase_KR1_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_KR1_SHIFT, val)

#define AP_CTRL_25GBase_KR_CONSORTIUM_SHIFT          (10)
#define AP_CTRL_25GBase_KR_CONSORTIUM_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_KR_CONSORTIUM_SHIFT)
#define AP_CTRL_25GBase_KR_CONSORTIUM_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_KR_CONSORTIUM_SHIFT, val)

#define AP_CTRL_25GBase_CR_CONSORTIUM_SHIFT          (11)
#define AP_CTRL_25GBase_CR_CONSORTIUM_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_CR_CONSORTIUM_SHIFT)
#define AP_CTRL_25GBase_CR_CONSORTIUM_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_CR_CONSORTIUM_SHIFT, val)

#define AP_CTRL_50GBase_KR2_CONSORTIUM_SHIFT          (12)
#define AP_CTRL_50GBase_KR2_CONSORTIUM_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_50GBase_KR2_CONSORTIUM_SHIFT)
#define AP_CTRL_50GBase_KR2_CONSORTIUM_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_50GBase_KR2_CONSORTIUM_SHIFT, val)

#define AP_CTRL_50GBase_CR2_CONSORTIUM_SHIFT          (13)
#define AP_CTRL_50GBase_CR2_CONSORTIUM_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_50GBase_CR2_CONSORTIUM_SHIFT)
#define AP_CTRL_50GBase_CR2_CONSORTIUM_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_50GBase_CR2_CONSORTIUM_SHIFT, val)

#define AP_CTRL_40GBase_CR4_SHIFT          (14)
#define AP_CTRL_40GBase_CR4_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_40GBase_CR4_SHIFT)
#define AP_CTRL_40GBase_CR4_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_40GBase_CR4_SHIFT, val)

#define AP_CTRL_ADV_40G_R4_ALL_MASK       (0x4001)
#define AP_CTRL_ADV_40G_R4_ALL_GET(adv)   AP_CTRL_GET(adv, AP_CTRL_ADV_100G_R4_ALL_MASK, AP_CTRL_40GBase_KR4_SHIFT)

#define AP_CTRL_25GBase_CR1S_SHIFT         (15)
#define AP_CTRL_25GBase_CR1S_GET(adv)      AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_CR1S_SHIFT)
#define AP_CTRL_25GBase_CR1S_SET(adv, val) AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_CR1S_SHIFT, val)

#define AP_CTRL_25GBase_CR1_SHIFT          (16)
#define AP_CTRL_25GBase_CR1_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_CR1_SHIFT)
#define AP_CTRL_25GBase_CR1_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_25GBase_CR1_SHIFT, val)

/* PAM4 modes*/
#define AP_CTRL_ADV_PAM4_MASK              (0x3FF)
#define AP_CTRL_PAM4_SHIFT                 (17)
#define AP_CTRL_PAM4_GET(adv)              AP_CTRL_GET(adv, AP_CTRL_ADV_PAM4_MASK, AP_CTRL_PAM4_SHIFT)

#define AP_CTRL_50GBase_KR1_SHIFT          (17)
#define AP_CTRL_50GBase_KR1_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_50GBase_KR1_SHIFT)
#define AP_CTRL_50GBase_KR1_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_50GBase_KR1_SHIFT, val)

#define AP_CTRL_100GBase_KR2_SHIFT          (18)
#define AP_CTRL_100GBase_KR2_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_100GBase_KR2_SHIFT)
#define AP_CTRL_100GBase_KR2_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_100GBase_KR2_SHIFT, val)

#define AP_CTRL_200GBase_KR4_SHIFT         (19)
#define AP_CTRL_200GBase_KR4_GET(adv)      AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_200GBase_KR4_SHIFT)
#define AP_CTRL_200GBase_KR4_SET(adv, val) AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_200GBase_KR4_SHIFT, val)

#define AP_CTRL_200GBase_KR8_SHIFT         (20)
#define AP_CTRL_200GBase_KR8_GET(adv)      AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_200GBase_KR8_SHIFT)
#define AP_CTRL_200GBase_KR8_SET(adv, val) AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_200GBase_KR8_SHIFT, val)

#define AP_CTRL_400GBase_KR8_SHIFT         (21)
#define AP_CTRL_400GBase_KR8_GET(adv)      AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_400GBase_KR8_SHIFT)
#define AP_CTRL_400GBase_KR8_SET(adv, val) AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_400GBase_KR8_SHIFT, val)

#define AP_CTRL_50GBase_CR1_SHIFT          (22)
#define AP_CTRL_50GBase_CR1_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_50GBase_CR1_SHIFT)
#define AP_CTRL_50GBase_CR1_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_50GBase_CR1_SHIFT, val)

#define AP_CTRL_100GBase_CR2_SHIFT          (23)
#define AP_CTRL_100GBase_CR2_GET(adv)       AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_100GBase_CR2_SHIFT)
#define AP_CTRL_100GBase_CR2_SET(adv, val)  AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_100GBase_CR2_SHIFT, val)

#define AP_CTRL_200GBase_CR4_SHIFT         (24)
#define AP_CTRL_200GBase_CR4_GET(adv)      AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_200GBase_CR4_SHIFT)
#define AP_CTRL_200GBase_CR4_SET(adv, val) AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_200GBase_CR4_SHIFT, val)

#define AP_CTRL_200GBase_CR8_SHIFT         (25)
#define AP_CTRL_200GBase_CR8_GET(adv)      AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_200GBase_CR8_SHIFT)
#define AP_CTRL_200GBase_CR8_SET(adv, val) AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_200GBase_CR8_SHIFT, val)

#define AP_CTRL_400GBase_CR8_SHIFT         (26)
#define AP_CTRL_400GBase_CR8_GET(adv)      AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_400GBase_CR8_SHIFT)
#define AP_CTRL_400GBase_CR8_SET(adv, val) AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_400GBase_CR8_SHIFT, val)

#define AP_CTRL_40GBase_KR2_SHIFT         (27)
#define AP_CTRL_40GBase_KR2_GET(adv)      AP_CTRL_GET(adv, AP_CTRL_ADV_MASK, AP_CTRL_40GBase_KR2_SHIFT)
#define AP_CTRL_40GBase_KR2_SET(adv, val) AP_CTRL_SET(adv, AP_CTRL_ADV_MASK, AP_CTRL_40GBase_KR2_SHIFT, val)

#define AP_CTRL_ADV_CONSORTIUM_MASK                   (0x3080F)
#define AP_CTRL_ADV_CONSORTIUM_SHIFT                  (10)
#define AP_CTRL_ADV_CONSORTIUM_GET(adv)               AP_CTRL_GET(adv, AP_CTRL_ADV_CONSORTIUM_MASK, AP_CTRL_ADV_CONSORTIUM_SHIFT)
#define AP_CTRL_ADV_25G_CONSORTIUM_MASK               (0x3)
#define AP_CTRL_ADV_25G_CONSORTIUM_SHIFT              (10)
#define AP_CTRL_ADV_25G_CONSORTIUM_GET(adv)           AP_CTRL_GET(adv, AP_CTRL_ADV_25G_CONSORTIUM_MASK, AP_CTRL_ADV_25G_CONSORTIUM_SHIFT)
#define AP_CTRL_ADV_25G_ALL_MASK                      (0x18F)
#define AP_CTRL_ADV_25G_ALL_SHIFT                     (8)
#define AP_CTRL_ADV_25G_ALL_GET(adv)                  AP_CTRL_GET(adv, AP_CTRL_ADV_25G_ALL_MASK, AP_CTRL_ADV_25G_ALL_SHIFT)

#define AP_CTRL_ADV_50G_CONSORTIUM_MASK               (0x3)
#define AP_CTRL_ADV_50G_CONSORTIUM_SHIFT              (12)
#define AP_CTRL_ADV_50G_CONSORTIUM_GET(adv)           AP_CTRL_GET(adv, AP_CTRL_ADV_50G_CONSORTIUM_MASK, AP_CTRL_ADV_50G_CONSORTIUM_SHIFT)

#define AP_CTRL_ADV_ALL_MASK               (0xFFFFFFF)
#define AP_CTRL_ADV_ALL_SHIFT              (0)
#define AP_CTRL_ADV_ALL_GET(adv)           AP_CTRL_GET(adv, AP_CTRL_ADV_ALL_MASK, AP_CTRL_ADV_ALL_SHIFT)
#define AP_CTRL_ADV_ALL_SET(adv, val)      AP_CTRL_SET(adv, AP_CTRL_ADV_ALL_MASK, AP_CTRL_ADV_ALL_SHIFT, val)

/*
** Options
** [00:00] Flow Control Pause Enable
** [01:01] Flow Control Asm Direction
** [02:02] Remote Flow Control Pause Enable
** [03:03] Remote Flow Control Asm Direction
** [04:04] FEC Suppress En
** [05:05] FEC Request
** [06:06] BASE-R FEC requested
** [07:07] RS-FEC requested
** [08:08] disableLinkInhibitTimer - AN only
** [09:09] loopback Enable
** [10:10]consortium (RS-FEC) ability (F1)
** [11:11]consortium (BASE-R FEC) ability (F2)
** [12:12]consortium (RS-FEC) request (F3)
** [13:13]consortium (BASE-R FEC) request (F4)
** [14:14] CTLE Bias value
** [15:15] autoKrEnable            - AN only
** [16:16] 20G_R1 enable
** [17:17] precoding enable        - AN only
** [18:18] skip resolution         - AN only
** */
#define AP_CTRL_FC_PAUSE_MASK              (0x1)
#define AP_CTRL_FC_PAUSE_SHIFT             (0)
#define AP_CTRL_FC_PAUSE_GET(opt)          AP_CTRL_GET(opt, AP_CTRL_FC_PAUSE_MASK, AP_CTRL_FC_PAUSE_SHIFT)
#define AP_CTRL_FC_PAUSE_SET(opt, val)     AP_CTRL_SET(opt, AP_CTRL_FC_PAUSE_MASK, AP_CTRL_FC_PAUSE_SHIFT, val)

#define AP_CTRL_FC_ASM_MASK                (0x1)
#define AP_CTRL_FC_ASM_SHIFT               (1)
#define AP_CTRL_FC_ASM_GET(opt)            AP_CTRL_GET(opt, AP_CTRL_FC_ASM_MASK, AP_CTRL_FC_ASM_SHIFT)
#define AP_CTRL_FC_ASM_SET(opt, val)       AP_CTRL_SET(opt, AP_CTRL_FC_ASM_MASK, AP_CTRL_FC_ASM_SHIFT, val)

#define AP_CTRL_REM_FC_PAUSE_MASK          (0x1)
#define AP_CTRL_REM_FC_PAUSE_SHIFT         (2)
#define AP_CTRL_REM_FC_PAUSE_GET(opt)      AP_CTRL_GET(opt, AP_CTRL_REM_FC_PAUSE_MASK, AP_CTRL_REM_FC_PAUSE_SHIFT)
#define AP_CTRL_REM_FC_PAUSE_SET(opt, val) AP_CTRL_SET(opt, AP_CTRL_REM_FC_PAUSE_MASK, AP_CTRL_REM_FC_PAUSE_SHIFT, val)

#define AP_CTRL_REM_FC_ASM_MASK            (0x1)
#define AP_CTRL_REM_FC_ASM_SHIFT           (3)
#define AP_CTRL_REM_FC_ASM_GET(opt)        AP_CTRL_GET(opt, AP_CTRL_REM_FC_ASM_MASK, AP_CTRL_REM_FC_ASM_SHIFT)
#define AP_CTRL_REM_FC_ASM_SET(opt, val)   AP_CTRL_SET(opt, AP_CTRL_REM_FC_ASM_MASK, AP_CTRL_REM_FC_ASM_SHIFT, val)

#define AP_CTRL_FEC_ABIL_MASK              (0x1)
#define AP_CTRL_FEC_ABIL_SHIFT             (4)
#define AP_CTRL_FEC_ABIL_GET(opt)          AP_CTRL_GET(opt, AP_CTRL_FEC_ABIL_MASK, AP_CTRL_FEC_ABIL_SHIFT)
#define AP_CTRL_FEC_ABIL_SET(opt, val)     AP_CTRL_SET(opt, AP_CTRL_FEC_ABIL_MASK, AP_CTRL_FEC_ABIL_SHIFT, val)

#define AP_CTRL_FEC_REQ_MASK               (0x1)
#define AP_CTRL_FEC_REQ_SHIFT              (5)
#define AP_CTRL_FEC_REQ_GET(opt)           AP_CTRL_GET(opt, AP_CTRL_FEC_REQ_MASK, AP_CTRL_FEC_REQ_SHIFT)
#define AP_CTRL_FEC_REQ_SET(opt, val)      AP_CTRL_SET(opt, AP_CTRL_FEC_REQ_MASK, AP_CTRL_FEC_REQ_SHIFT, val)

#define AP_CTRL_FEC_FC_REQ_MASK            (0x1)
#define AP_CTRL_FEC_FC_REQ_SHIFT           (6)
#define AP_CTRL_FEC_FC_REQ_GET(opt)        AP_CTRL_GET(opt, AP_CTRL_FEC_FC_REQ_MASK, AP_CTRL_FEC_FC_REQ_SHIFT)
#define AP_CTRL_FEC_FC_REQ_SET(opt, val)   AP_CTRL_SET(opt, AP_CTRL_FEC_FC_REQ_MASK, AP_CTRL_FEC_FC_REQ_SHIFT, val)

#define AP_CTRL_FEC_RS_REQ_MASK            (0x1)
#define AP_CTRL_FEC_RS_REQ_SHIFT           (7)
#define AP_CTRL_FEC_RS_REQ_GET(opt)        AP_CTRL_GET(opt, AP_CTRL_FEC_RS_REQ_MASK, AP_CTRL_FEC_RS_REQ_SHIFT)
#define AP_CTRL_FEC_RS_REQ_SET(opt, val)   AP_CTRL_SET(opt, AP_CTRL_FEC_RS_REQ_MASK, AP_CTRL_FEC_RS_REQ_SHIFT, val)

#define AP_CTRL_FEC_ADVANCED_REQ_MASK           (0x3)
#define AP_CTRL_FEC_ADVANCED_REQ_SHIFT          (6)
#define AP_CTRL_FEC_ADVANCED_REQ_GET(opt)       AP_CTRL_GET(opt, AP_CTRL_FEC_ADVANCED_REQ_MASK, AP_CTRL_FEC_ADVANCED_REQ_SHIFT)
#define AP_CTRL_FEC_ADVANCED_REQ_SET(opt, val)  AP_CTRL_SET(opt, AP_CTRL_FEC_ADVANCED_REQ_MASK, AP_CTRL_FEC_ADVANCED_REQ_SHIFT, val)

#define AP_CTRL_LB_EN_MASK                 (0x1)
#define AP_CTRL_LB_EN_SHIFT                (9)
#define AP_CTRL_LB_EN_GET(opt)             AP_CTRL_GET(opt, AP_CTRL_LB_EN_MASK, AP_CTRL_LB_EN_SHIFT)
#define AP_CTRL_LB_EN_SET(opt, val)        AP_CTRL_SET(opt, AP_CTRL_LB_EN_MASK, AP_CTRL_LB_EN_SHIFT, val)

#define AP_CTRL_RS_FEC_ABIL_CONSORTIUM_MASK          (0x1)
#define AP_CTRL_RS_FEC_ABIL_CONSORTIUM_SHIFT         (10)
#define AP_CTRL_RS_FEC_ABIL_CONSORTIUM_GET(opt)      AP_CTRL_GET(opt, AP_CTRL_RS_FEC_ABIL_CONSORTIUM_MASK, AP_CTRL_RS_FEC_ABIL_CONSORTIUM_SHIFT)
#define AP_CTRL_RS_FEC_ABIL_CONSORTIUM_SET(opt, val) AP_CTRL_SET(opt, AP_CTRL_RS_FEC_ABIL_CONSORTIUM_MASK, AP_CTRL_RS_FEC_ABIL_CONSORTIUM_SHIFT, val)

#define AP_CTRL_FC_FEC_ABIL_CONSORTIUM_MASK          (0x1)
#define AP_CTRL_FC_FEC_ABIL_CONSORTIUM_SHIFT         (11)
#define AP_CTRL_FC_FEC_ABIL_CONSORTIUM_GET(opt)      AP_CTRL_GET(opt, AP_CTRL_FC_FEC_ABIL_CONSORTIUM_MASK, AP_CTRL_FC_FEC_ABIL_CONSORTIUM_SHIFT)
#define AP_CTRL_FC_FEC_ABIL_CONSORTIUM_SET(opt, val) AP_CTRL_SET(opt, AP_CTRL_FC_FEC_ABIL_CONSORTIUM_MASK, AP_CTRL_FC_FEC_ABIL_CONSORTIUM_SHIFT, val)

#define AP_CTRL_FEC_ABIL_CONSORTIUM_MASK              (0x3)
#define AP_CTRL_FEC_ABIL_CONSORTIUM_SHIFT             (10)
#define AP_CTRL_FEC_ABIL_CONSORTIUM_GET(opt)          AP_CTRL_GET(opt, AP_CTRL_FEC_ABIL_CONSORTIUM_MASK, AP_CTRL_FEC_ABIL_CONSORTIUM_SHIFT)
#define AP_CTRL_FEC_ABIL_CONSORTIUM_SET(opt, val)     AP_CTRL_SET(opt, AP_CTRL_FEC_ABIL_CONSORTIUM_MASK, AP_CTRL_FEC_ABIL_CONSORTIUM_SHIFT, val)

#define AP_CTRL_RS_FEC_REQ_CONSORTIUM_MASK           (0x1)
#define AP_CTRL_RS_FEC_REQ_CONSORTIUM_SHIFT          (12)
#define AP_CTRL_RS_FEC_REQ_CONSORTIUM_GET(opt)       AP_CTRL_GET(opt, AP_CTRL_RS_FEC_REQ_CONSORTIUM_MASK, AP_CTRL_RS_FEC_REQ_CONSORTIUM_SHIFT)
#define AP_CTRL_RS_FEC_REQ_CONSORTIUM_SET(opt, val)  AP_CTRL_SET(opt, AP_CTRL_RS_FEC_REQ_CONSORTIUM_MASK, AP_CTRL_RS_FEC_REQ_CONSORTIUM_SHIFT, val)

#define AP_CTRL_FC_FEC_REQ_CONSORTIUM_MASK           (0x1)
#define AP_CTRL_FC_FEC_REQ_CONSORTIUM_SHIFT          (13)
#define AP_CTRL_FC_FEC_REQ_CONSORTIUM_GET(opt)       AP_CTRL_GET(opt, AP_CTRL_FC_FEC_REQ_CONSORTIUM_MASK, AP_CTRL_FC_FEC_REQ_CONSORTIUM_SHIFT)
#define AP_CTRL_FC_FEC_REQ_CONSORTIUM_SET(opt, val)  AP_CTRL_SET(opt, AP_CTRL_FC_FEC_REQ_CONSORTIUM_MASK, AP_CTRL_FC_FEC_REQ_CONSORTIUM_SHIFT, val)

#define AP_CTRL_FEC_REQ_CONSORTIUM_MASK              (0x3)
#define AP_CTRL_FEC_REQ_CONSORTIUM_SHIFT             (12)
#define AP_CTRL_FEC_REQ_CONSORTIUM_GET(opt)          AP_CTRL_GET(opt, AP_CTRL_FEC_REQ_CONSORTIUM_MASK, AP_CTRL_FEC_REQ_CONSORTIUM_SHIFT)
#define AP_CTRL_FEC_REQ_CONSORTIUM_SET(opt, val)     AP_CTRL_SET(opt, AP_CTRL_FEC_REQ_CONSORTIUM_MASK, AP_CTRL_FEC_REQ_CONSORTIUM_SHIFT, val)

#define AP_CTRL_CTLE_BIAS_VAL_MASK                   (0x1)
#define AP_CTRL_CTLE_BIAS_VAL_SHIFT                  (14)
#define AP_CTRL_CTLE_BIAS_VAL_GET(opt)               AP_CTRL_GET(opt, AP_CTRL_CTLE_BIAS_VAL_MASK, AP_CTRL_CTLE_BIAS_VAL_SHIFT)
#define AP_CTRL_CTLE_BIAS_VAL_SET(opt, val)          AP_CTRL_SET(opt, AP_CTRL_CTLE_BIAS_VAL_MASK, AP_CTRL_CTLE_BIAS_VAL_SHIFT, val)

#define AN_CTRL_AUTO_KR_ENABLE_MASK                  (0x1)
#define AN_CTRL_AUTO_KR_ENABLE_SHIFT                 (15)
#define AN_CTRL_AUTO_KR_ENABLE_GET(opt)              AP_CTRL_GET(opt, AN_CTRL_AUTO_KR_ENABLE_MASK, AN_CTRL_AUTO_KR_ENABLE_SHIFT)
#define AN_CTRL_AUTO_KR_ENABLE_SET(opt, val)         AP_CTRL_SET(opt, AN_CTRL_AUTO_KR_ENABLE_MASK, AN_CTRL_AUTO_KR_ENABLE_SHIFT, val)

#define AN_CTRL_DISABLE_LINK_TIMER_MASK              (0x1)
#define AN_CTRL_DISABLE_LINK_TIMER_SHIFT             (8)
#define AN_CTRL_DISABLE_LINK_TIMER_GET(opt)          AP_CTRL_GET(opt, AN_CTRL_DISABLE_LINK_TIMER_MASK, AN_CTRL_DISABLE_LINK_TIMER_SHIFT)
#define AN_CTRL_DISABLE_LINK_TIMER_SET(opt, val)     AP_CTRL_SET(opt, AN_CTRL_DISABLE_LINK_TIMER_MASK, AN_CTRL_DISABLE_LINK_TIMER_SHIFT, val)

#define AP_CTRL_20G_R1_MASK                         (0x3)
#define AP_CTRL_20G_R1_SHIFT                        (16)
#define AP_CTRL_20G_R1_GET(opt)                     AP_CTRL_GET(opt, AP_CTRL_20G_R1_MASK, AP_CTRL_20G_R1_SHIFT)
#define AP_CTRL_20G_R1_SET(opt, val)                AP_CTRL_SET(opt, AP_CTRL_20G_R1_MASK, AP_CTRL_20G_R1_SHIFT, val)
#define AP_CTRL_SPECIAL_SPEED_GET(opt)              AP_CTRL_GET(opt, AP_CTRL_20G_R1_MASK, AP_CTRL_20G_R1_SHIFT)
#define AP_CTRL_SPECIAL_SPEED_SET(opt, val)         AP_CTRL_SET(opt, AP_CTRL_20G_R1_MASK, AP_CTRL_20G_R1_SHIFT, val)

#define AN_CTRL_PRECODING_MASK                      (0x1)
#define AN_CTRL_PRECODING_SHIFT                     (18)
#define AN_CTRL_PRECODING_GET(opt)                  AP_CTRL_GET(opt, AN_CTRL_PRECODING_MASK, AN_CTRL_PRECODING_SHIFT)
#define AN_CTRL_PRECODING_SET(opt, val)             AP_CTRL_SET(opt, AN_CTRL_PRECODING_MASK, AN_CTRL_PRECODING_SHIFT, val)

#define AN_CTRL_SKIP_RES_MASK                    (0x1)
#define AN_CTRL_SKIP_RES_SHIFT                   (19)
#define AN_CTRL_SKIP_RES_GET(opt)                AP_CTRL_GET(opt, AN_CTRL_SKIP_RES_MASK, AN_CTRL_SKIP_RES_SHIFT)
#define AN_CTRL_SKIP_RES_SET(opt, val)           AP_CTRL_SET(opt, AN_CTRL_SKIP_RES_MASK, AN_CTRL_SKIP_RES_SHIFT, val)

#define AN_CTRL_RX_TRAIN_MASK                    (0x1)
#define AN_CTRL_RX_TRAIN_SHIFT                   (20)
#define AN_CTRL_RX_TRAIN_GET(opt)                AP_CTRL_GET(opt, AN_CTRL_RX_TRAIN_MASK, AN_CTRL_RX_TRAIN_SHIFT)
#define AN_CTRL_RX_TRAIN_SET(opt, val)           AP_CTRL_SET(opt, AN_CTRL_RX_TRAIN_MASK, AN_CTRL_RX_TRAIN_SHIFT, val)

#define AN_CTRL_IS_OPTICAL_MODE_MASK                    (0x1)
#define AN_CTRL_IS_OPTICAL_MODE_SHIFT                   (21)
#define AN_CTRL_IS_OPTICAL_MODE_GET(opt)                AP_CTRL_GET(opt, AN_CTRL_IS_OPTICAL_MODE_MASK, AN_CTRL_IS_OPTICAL_MODE_SHIFT)
#define AN_CTRL_IS_OPTICAL_MODE_SET(opt, val)           AP_CTRL_SET(opt, AN_CTRL_IS_OPTICAL_MODE_MASK, AN_CTRL_IS_OPTICAL_MODE_SHIFT, val)

/*Special speeds : 42G, 53G, 106G*/
#define AP_CTRL_SPECIAL_SPEED_MODE_GET(adv)\
      (AP_CTRL_40GBase_KR4_GET(adv) || \
       AP_CTRL_100GBase_KR2_GET(adv) || \
       AP_CTRL_50GBase_KR2_CONSORTIUM_GET(adv) || \
       AP_CTRL_100GBase_KR4_GET(adv))

#define AP_CTRL_AUTO_PROFILE_SUPPORT(adv, opt,portMode)\
        (((AP_CTRL_SPECIAL_SPEED_GET(opt) && AP_CTRL_100GBase_KR4_GET(adv)) ||\
          (HWS_25G_SERDES_MODE_CHECK(portMode))) && \
         (!AN_CTRL_RX_TRAIN_GET(opt)))
/*
** AP Status Info
** ==============
**/

/*
** HCD Status
** [03:03] Found
** [04:04] ARBSmError
** [05:05] OPTICAL_MODE
** [06:06] Flow Control Rx Result
** [07:07] Flow Control Tx Result
** [08:19] Local HCD Type -------------------------------->        Port_1000Base_KX,                0
**                                                                 Port_10GBase_KX4,                1
**                                                                 Port_10GBase_R,                  2
**                                                                 Port_25GBASE_KR_S,               3
**                                                                 Port_25GBASE_KR,                 4
**                                                                 Port_40GBase_R,                  5
**                                                                 Port_40GBASE_CR4,                6
**                                                                 Port_100GBASE_CR10,              7
**                                                                 Port_100GBASE_KP4,               8
**                                                                 Port_100GBASE_KR4,               9
**                                                                 Port_100GBASE_CR4,               10
**                                                                 Port_25GBASE_KR1_CONSORTIUM,     11
**                                                                 Port_25GBASE_CR1_CONSORTIUM,     12
**                                                                 Port_50GBASE_KR2_CONSORTIUM,     13
**                                                                 Port_50GBASE_CR2_CONSORTIUM,     14
**                                                                 Port_25GBASE_CR_S,               15
**                                                                 Port_25GBASE_CR,                 16
**                                                                 Port_20GBASE_KR,                 17
**                                                                 Port_50GBase_KR,                 18
**                                                                 Port_50GBase_CR,                 19
**                                                                 Port_100GBase_KR2,               20
**                                                                 Port_100GBase_CR2,               21
**                                                                 Port_200GBase_KR4,               22
**                                                                 Port_200GBase_CR4,               23
**                                                                 Port_200GBase_KR8,               24
**                                                                 Port_200GBase_CR8,               25
**                                                                 Port_400GBase_KR8,               26
**                                                                 Port_400GBase_CR8,               27
** [20:20] Consortium result
** [22:23] FEC type --------------> 0 - NO FEC
**                                  1 - BASE-R FEC
**                                  2 - RS-FEC
** [29:29] HCD Interrupt Trigger
** [30:30] Link Interrupt Trigger
** [31:31] Link
**
*/
#define AP_ST_HCD_FOUND_MASK                 (0x1)
#define AP_ST_HCD_FOUND_SHIFT                (3)
#define AP_ST_HCD_FOUND_GET(hcd)             AP_CTRL_GET(hcd, AP_ST_HCD_FOUND_MASK, AP_ST_HCD_FOUND_SHIFT)
#define AP_ST_HCD_FOUND_SET(hcd, val)        AP_CTRL_SET(hcd, AP_ST_HCD_FOUND_MASK, AP_ST_HCD_FOUND_SHIFT, val)

#define AP_ST_AP_ERR_MASK                    (0x1)
#define AP_ST_AP_ERR_SHIFT                   (4)
#define AP_ST_AP_ERR_GET(hcd)                AP_CTRL_GET(hcd, AP_ST_AP_ERR_MASK, AP_ST_AP_ERR_SHIFT)
#define AP_ST_AP_ERR_SET(hcd, val)           AP_CTRL_SET(hcd, AP_ST_AP_ERR_MASK, AP_ST_AP_ERR_SHIFT, val)

#define AP_ST_HCD_IS_OPTICAL_MODE_MASK             (0x1)
#define AP_ST_HCD_IS_OPTICAL_MODE_SHIFT            (5)
#define AP_ST_HCD_IS_OPTICAL_MODE_GET(hcd)         AP_CTRL_GET(hcd, AP_ST_HCD_IS_OPTICAL_MODE_MASK, AP_ST_HCD_IS_OPTICAL_MODE_SHIFT)
#define AP_ST_HCD_IS_OPTICAL_MODE_SET(hcd, val)    AP_CTRL_SET(hcd, AP_ST_HCD_IS_OPTICAL_MODE_MASK, AP_ST_HCD_IS_OPTICAL_MODE_SHIFT, val)

#define AP_ST_HCD_FC_RX_RES_MASK             (0x1)
#define AP_ST_HCD_FC_RX_RES_SHIFT            (6)
#define AP_ST_HCD_FC_RX_RES_GET(hcd)         AP_CTRL_GET(hcd, AP_ST_HCD_FC_RX_RES_MASK, AP_ST_HCD_FC_RX_RES_SHIFT)
#define AP_ST_HCD_FC_RX_RES_SET(hcd, val)    AP_CTRL_SET(hcd, AP_ST_HCD_FC_RX_RES_MASK, AP_ST_HCD_FC_RX_RES_SHIFT, val)

#define AP_ST_HCD_FC_TX_RES_MASK             (0x1)
#define AP_ST_HCD_FC_TX_RES_SHIFT            (7)
#define AP_ST_HCD_FC_TX_RES_GET(hcd)         AP_CTRL_GET(hcd, AP_ST_HCD_FC_TX_RES_MASK, AP_ST_HCD_FC_TX_RES_SHIFT)
#define AP_ST_HCD_FC_TX_RES_SET(hcd, val)    AP_CTRL_SET(hcd, AP_ST_HCD_FC_TX_RES_MASK, AP_ST_HCD_FC_TX_RES_SHIFT, val)

#define AP_ST_HCD_TYPE_MASK                  (0xFFF)
#define AP_ST_HCD_TYPE_SHIFT                 (8)
#define AP_ST_HCD_TYPE_GET(hcd)              AP_CTRL_GET(hcd, AP_ST_HCD_TYPE_MASK, AP_ST_HCD_TYPE_SHIFT)
#define AP_ST_HCD_TYPE_SET(hcd, val)         AP_CTRL_SET(hcd, AP_ST_HCD_TYPE_MASK, AP_ST_HCD_TYPE_SHIFT, val)

#define AP_ST_HCD_CONSORTIUM_RES_MASK          (0x1)
#define AP_ST_HCD_CONSORTIUM_RES_SHIFT         (20)
#define AP_ST_HCD_CONSORTIUM_RES_GET(hcd)      AP_CTRL_GET(hcd, AP_ST_HCD_CONSORTIUM_RES_MASK, AP_ST_HCD_CONSORTIUM_RES_SHIFT)
#define AP_ST_HCD_CONSORTIUM_RES_SET(hcd, val) AP_CTRL_SET(hcd, AP_ST_HCD_CONSORTIUM_RES_MASK, AP_ST_HCD_CONSORTIUM_RES_SHIFT, val)

#define AP_ST_HCD_FEC_RES_NONE               (0x0)
#define AP_ST_HCD_FEC_RES_FC                 (0x1)
#define AP_ST_HCD_FEC_RES_RS                 (0x2)
#define AP_ST_HCD_FEC_RES_BOTH               (0x3)
#define AP_ST_HCD_FEC_RES_MASK               (0x3)
#define AP_ST_HCD_FEC_RES_SHIFT              (22)
#define AP_ST_HCD_FEC_RES_GET(hcd)           AP_CTRL_GET(hcd, AP_ST_HCD_FEC_RES_MASK, AP_ST_HCD_FEC_RES_SHIFT)
#define AP_ST_HCD_FEC_RES_SET(hcd, val)      AP_CTRL_SET(hcd, AP_ST_HCD_FEC_RES_MASK, AP_ST_HCD_FEC_RES_SHIFT, val)

#define AP_ST_HCD_INT_TRIG_MASK              (0x1)
#define AP_ST_HCD_INT_TRIG_SHIFT             (29)
#define AP_ST_HCD_INT_TRIG_GET(hcd)          AP_CTRL_GET(hcd, AP_ST_HCD_INT_TRIG_MASK, AP_ST_HCD_INT_TRIG_SHIFT)
#define AP_ST_HCD_INT_TRIG_SET(hcd, val)     AP_CTRL_SET(hcd, AP_ST_HCD_INT_TRIG_MASK, AP_ST_HCD_INT_TRIG_SHIFT, val)

#define AP_ST_LINK_INT_TRIG_MASK             (0x1)
#define AP_ST_LINK_INT_TRIG_SHIFT            (30)
#define AP_ST_LINK_INT_TRIG_GET(hcd)         AP_CTRL_GET(hcd, AP_ST_LINK_INT_TRIG_MASK, AP_ST_LINK_INT_TRIG_SHIFT)
#define AP_ST_LINK_INT_TRIG_SET(hcd, val)    AP_CTRL_SET(hcd, AP_ST_LINK_INT_TRIG_MASK, AP_ST_LINK_INT_TRIG_SHIFT, val)

#define AP_ST_HCD_LINK_MASK                  (0x1)
#define AP_ST_HCD_LINK_SHIFT                 (31)
#define AP_ST_HCD_LINK_GET(hcd)              AP_CTRL_GET(hcd, AP_ST_HCD_LINK_MASK, AP_ST_HCD_LINK_SHIFT)
#define AP_ST_HCD_LINK_SET(hcd, val)         AP_CTRL_SET(hcd, AP_ST_HCD_LINK_MASK, AP_ST_HCD_LINK_SHIFT, val)

/*
** ARBSmStatus
** [00:00] ST_AN_ENABLE
** [01:01] ST_TX_DISABLE
** [02:02] ST_LINK_STAT_CK
** [03:03] ST_PARALLEL_FLT
** [04:04] ST_ABILITY_DET
** [05:05] ST_ACK_DETECT
** [06:06] ST_COMPLETE_ACK
** [07:07] ST_NP_WAIT
** [08:08] ST_AN_GOOD_CK
** [09:09] ST_AN_GOOD
** [10:10] ST_SERDES_WAIT
*/
#define AP_ST_ARB_FSM_MASK                   (0x7FF)
#define AP_ST_ARB_FSM_SHIFT                  (0)
#define AP_ST_ARB_FSM_GET(arb)               AP_CTRL_GET(arb, AP_ST_ARB_FSM_MASK, AP_ST_ARB_FSM_SHIFT)
#define AP_ST_ARB_FSM_SET(arb, val)          AP_CTRL_SET(arb, AP_ST_ARB_FSM_MASK, AP_ST_ARB_FSM_SHIFT, val)

/*
** AP Status
** [00:00] Signal Detect
** [01:01] CDR Lock
** [02:02] PCS Lock
** [03:07] Reserved
*/
/*#define AP_ST_SD_MASK                        (0x1)
#define AP_ST_SD_SHIFT                       (0)
#define AP_ST_SD_GET(status)                 AP_CTRL_GET(arb, AP_ST_SD_MASK, AP_ST_SD_SHIFT)
#define AP_ST_SD_SET(status, val)            AP_CTRL_SET(arb, AP_ST_SD_MASK, AP_ST_SD_SHIFT, val)

#define AP_ST_CDR_LOCK_MASK                  (0x1)
#define AP_ST_CDR_LOCK_SHIFT                 (1)
#define AP_ST_CDR_LOCK_GET(status)           AP_CTRL_GET(arb, AP_ST_CDR_LOCK_MASK, AP_ST_CDR_LOCK_SHIFT)
#define AP_ST_CDR_LOCK_SET(status, val)      AP_CTRL_SET(arb, AP_ST_CDR_LOCK_MASK, AP_ST_CDR_LOCK_SHIFT, val)

#define AP_ST_PCS_LOCK_MASK                  (0x1)
#define AP_ST_PCS_LOCK_SHIFT                 (2)
#define AP_ST_PCS_LOCK_GET(status)           AP_CTRL_GET(arb, AP_ST_PCS_LOCK_MASK, AP_ST_PCS_LOCK_SHIFT)
#define AP_ST_PCS_LOCK_SET(status, val)      AP_CTRL_SET(arb, AP_ST_PCS_LOCK_MASK, AP_ST_PCS_LOCK_SHIFT, val)
*/


/**
* @enum MV_AP_DETECT_ITEROP
* @endinternal
*
* @brief   AP and AN introp
*
*/
typedef enum
{
    AP_PORT_INTROP_TX_DIS               = 0x01,
    AP_PORT_INTROP_ABILITY_DUR          = 0x02,
    AP_PORT_INTROP_ABILITY_MAX_INT      = 0x04,
    AP_PORT_INTROP_ABILITY_MAX_FAIL_INT = 0x08,
    AP_PORT_INTROP_AP_LINK_DUR          = 0x10,
    AP_PORT_INTROP_AP_LINK_MAX_INT      = 0x20,
    PD_PORT_INTROP_AP_LINK_DUR          = 0x40,
    PD_PORT_INTROP_AP_LINK_MAX_INT      = 0x80,
    AN_PORT_INTROP_AP_TRAINING_MAX_INT  = 0x100,
    AN_PAM4_PORT_INTROP_AP_LINK_DUR     = 0x200,
    AN_PAM4_PORT_INTROP_AP_LINK_MAX_INT = 0x400,
    AN_PAM4_PORT_INTROP_AP_TRAINING_MAX_INT = 0x800

}MV_AP_DETECT_ITEROP;

#ifdef __cplusplus
}
#endif

#endif /* __mvHwServicesPortCtrlApDefs_H */



