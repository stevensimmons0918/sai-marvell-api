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
* @file common/h/cpss/common/port/cpssPortCtrl.h
*
* @brief CPSS definitions for port configurations.
*
* @version   32
********************************************************************************
*/

#ifndef __cpssPortCtrl_h
#define __cpssPortCtrl_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>

/* size of DFE values array */
#define CPSS_PORT_DFE_VALUES_ARRAY_SIZE_CNS    6

#define CPSS_PORT_DFE_AVAGO_VALUES_ARRAY_SIZE_CNS    13

/* size of array of interfaces advertised by port during AP process */
#define CPSS_PORT_AP_IF_ARRAY_SIZE_CNS 10

/* adaptive CTLE in CPSS*/
/*max serdes lanes*/
#define CPSS_IPC_MAX_LANES_NUM 4
/*adaptive ctle DB size, for debug on host side*/
#define CPSS_MAX_ADAPT_CTLE_DB_SIZE 8

/* size of PAM4 eyes array*/
#define CPSS_PAM4_EYES_ARRAY_SIZE_CNS 6

/* check if USX port mode has 2/4/8 MACs per 1 SerDes */
#define CPSS_PORT_MULTIPLE_USX_PORT_MODE_CHECK_MAC(_ifMode)     \
    (((_ifMode) == CPSS_PORT_INTERFACE_MODE_QSGMII_E)         ||  \
     ((_ifMode) == CPSS_PORT_INTERFACE_MODE_USX_5G_DXGMII_E)  ||  \
     ((_ifMode) == CPSS_PORT_INTERFACE_MODE_USX_10G_DXGMII_E) ||  \
     ((_ifMode) == CPSS_PORT_INTERFACE_MODE_USX_20G_DXGMII_E) ||  \
     ((_ifMode) == CPSS_PORT_INTERFACE_MODE_USX_QUSGMII_E)    ||  \
     ((_ifMode) == CPSS_PORT_INTERFACE_MODE_USX_10G_QXGMII_E) ||  \
     ((_ifMode) == CPSS_PORT_INTERFACE_MODE_USX_20G_QXGMII_E) ||  \
     ((_ifMode) == CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E)    ||  \
     ((_ifMode) == CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E))

/* QSGMII and USX_QUSGMII should be treated the same at PM level */
#define CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(_ifMode)     \
    (((_ifMode) == CPSS_PORT_INTERFACE_MODE_QSGMII_E)         ||  \
     ((_ifMode) == CPSS_PORT_INTERFACE_MODE_USX_QUSGMII_E))

/**
* @enum CPSS_PORT_DUPLEX_ENT
 *
 * @brief Enumeration of port duplex modes
*/
typedef enum{

    /** full duplex mode */
    CPSS_PORT_FULL_DUPLEX_E,

    /** half duplex mode */
    CPSS_PORT_HALF_DUPLEX_E

} CPSS_PORT_DUPLEX_ENT;

/**
* @enum CPSS_PORT_INTERFACE_MODE_ENT
 *
 * @brief Enumeration of port interface modes
*/
typedef enum
{
    CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E,   /* 0 */
    CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E,    /* 1 */
    CPSS_PORT_INTERFACE_MODE_MII_E,             /* 2 */
    CPSS_PORT_INTERFACE_MODE_SGMII_E,           /* 3 */ /* CPSS_PORT_SPEED_1000_E , CPSS_PORT_SPEED_2500_E   */
    CPSS_PORT_INTERFACE_MODE_XGMII_E,           /* 4 */ /* CPSS_PORT_SPEED_10000_E, CPSS_PORT_SPEED_12000_E, CPSS_PORT_SPEED_16000_E, CPSS_PORT_SPEED_20000_E, */
    CPSS_PORT_INTERFACE_MODE_MGMII_E,           /* 5 */
    CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,      /* 6 */ /* CPSS_PORT_SPEED_1000_E, */
    CPSS_PORT_INTERFACE_MODE_GMII_E,            /* 7 */
    CPSS_PORT_INTERFACE_MODE_MII_PHY_E,         /* 8 */
    CPSS_PORT_INTERFACE_MODE_QX_E,              /* 9 */  /* CPSS_PORT_SPEED_2500_E,  CPSS_PORT_SPEED_5000_E,  */
    CPSS_PORT_INTERFACE_MODE_HX_E,              /* 10 */ /* CPSS_PORT_SPEED_5000_E,  CPSS_PORT_SPEED_10000_E, */
    CPSS_PORT_INTERFACE_MODE_RXAUI_E,           /* 11 */ /* CPSS_PORT_SPEED_10000_E  */
    CPSS_PORT_INTERFACE_MODE_100BASE_FX_E,      /* 12 */
    CPSS_PORT_INTERFACE_MODE_QSGMII_E,          /* 13 */ /* CPSS_PORT_SPEED_1000_E, */
    CPSS_PORT_INTERFACE_MODE_XLG_E,             /* 14 */
    CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E,     /* 15 */
    CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E =
                                        CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E,

    CPSS_PORT_INTERFACE_MODE_KR_E,              /* 16 */ /* CPSS_PORT_SPEED_10000_E, CPSS_PORT_SPEED_12000_E, CPSS_PORT_SPEED_20000_E, CPSS_PORT_SPEED_40000_E, CPSS_PORT_SPEED_100G_E, */
    CPSS_PORT_INTERFACE_MODE_HGL_E,             /* 17 */ /* CPSS_PORT_SPEED_15000_E, CPSS_PORT_SPEED_16000_E, CPSS_PORT_SPEED_40000_E */
    CPSS_PORT_INTERFACE_MODE_CHGL_12_E,         /* 18 */ /* CPSS_PORT_SPEED_100G_E , */
    CPSS_PORT_INTERFACE_MODE_ILKN12_E,          /* 19 */
    CPSS_PORT_INTERFACE_MODE_SR_LR_E,           /* 20 */ /* CPSS_PORT_SPEED_5000_E, CPSS_PORT_SPEED_10000_E, CPSS_PORT_SPEED_12000_E, CPSS_PORT_SPEED_20000_E, CPSS_PORT_SPEED_40000_E */
    CPSS_PORT_INTERFACE_MODE_ILKN16_E,          /* 21 */
    CPSS_PORT_INTERFACE_MODE_ILKN24_E,          /* 22 */
    CPSS_PORT_INTERFACE_MODE_ILKN4_E,           /* 23 */ /* CPSS_PORT_SPEED_12000_E, CPSS_PORT_SPEED_20000_E, */
    CPSS_PORT_INTERFACE_MODE_ILKN8_E,           /* 24 */ /* CPSS_PORT_SPEED_20000_E, CPSS_PORT_SPEED_40000_E, */

    CPSS_PORT_INTERFACE_MODE_XHGS_E,            /* 25 */ /* CPSS_PORT_SPEED_11800_E, CPSS_PORT_SPEED_23600_E, CPSS_PORT_SPEED_47200_E, */
    CPSS_PORT_INTERFACE_MODE_XHGS_SR_E,         /* 26 */ /* CPSS_PORT_SPEED_11800_E, CPSS_PORT_SPEED_47200_E, */
    CPSS_PORT_INTERFACE_MODE_KR2_E,             /* 27 */
    CPSS_PORT_INTERFACE_MODE_KR4_E,             /* 28 */
    CPSS_PORT_INTERFACE_MODE_SR_LR2_E,          /* 29 */ /* CPSS_PORT_SPEED_50000_E*/
    CPSS_PORT_INTERFACE_MODE_SR_LR4_E,              /* 30 */ /* CPSS_PORT_SPEED_100G_E  */
    CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E, /* 31 */ /* Multi-Link Gearbox speeds: 40G, 10G, 40G, 10G */

    CPSS_PORT_INTERFACE_MODE_KR_C_E,            /* 32 */ /*CONSORTIUM - CPSS_PORT_SPEED_25000_E*/
    CPSS_PORT_INTERFACE_MODE_CR_C_E,            /* 33 */ /*CONSORTIUM - CPSS_PORT_SPEED_25000_E*/
    CPSS_PORT_INTERFACE_MODE_KR2_C_E,           /* 34 */ /*CONSORTIUM - CPSS_PORT_SPEED_50000_E*/
    CPSS_PORT_INTERFACE_MODE_CR2_C_E,           /* 35 */ /*CONSORTIUM - CPSS_PORT_SPEED_50000_E*/

    CPSS_PORT_INTERFACE_MODE_CR_E,              /* 36 */
    CPSS_PORT_INTERFACE_MODE_CR2_E,             /* 37 */
    CPSS_PORT_INTERFACE_MODE_CR4_E,             /* 38 */
    CPSS_PORT_INTERFACE_MODE_KR_S_E,            /* 39 */
    CPSS_PORT_INTERFACE_MODE_CR_S_E,            /* 40 */

    CPSS_PORT_INTERFACE_MODE_KR8_E,             /* 41 */
    CPSS_PORT_INTERFACE_MODE_CR8_E,             /* 42 */
    CPSS_PORT_INTERFACE_MODE_SR_LR8_E,          /* 43 */

    /* USX 1 Lane */
    CPSS_PORT_INTERFACE_MODE_USX_2_5G_SXGMII_E, /* 44 */ /* 1 Port -> 10M/100M/1G/2.5G/5G       with SD speed 2.578125G */
    CPSS_PORT_INTERFACE_MODE_USX_5G_SXGMII_E,   /* 45 */ /* 1 Port -> 10M/100M/1G/2.5G/5G       with SD speed 5.15625G */
    CPSS_PORT_INTERFACE_MODE_USX_10G_SXGMII_E,  /* 46 */ /* 1 Port -> 10M/100M/1G/2.5G/5G/10G   with SD speed 10.3125G */

    /* USX 2 Lanes */
    CPSS_PORT_INTERFACE_MODE_USX_5G_DXGMII_E,   /* 47 */ /* 2 Ports -> 10M/100M/1G/2.5G         with SD speed 5.15625G */
    CPSS_PORT_INTERFACE_MODE_USX_10G_DXGMII_E,  /* 48 */ /* 2 Ports -> 10M/100M/1G/2.5G/5G      with SD speed 10.3125G */
    CPSS_PORT_INTERFACE_MODE_USX_20G_DXGMII_E,  /* 49 */ /* 2 Ports -> 10M/100M/1G/2.5G/5G/10G  with SD speed 20.625G */

    /* USX 4 Lanes */
    CPSS_PORT_INTERFACE_MODE_USX_QUSGMII_E,     /* 50 */ /* Like QSGMII but with PCH - maximum of 4 ports, CPSS_PORT_SPEED_1000_E */
    CPSS_PORT_INTERFACE_MODE_USX_10G_QXGMII_E,  /* 51 */ /* 4 Ports -> 10M/100M/1G/2.5G         with SD speed 10.3125G */
    CPSS_PORT_INTERFACE_MODE_USX_20G_QXGMII_E,  /* 52 */ /* 4 Ports -> 10M/100M/1G/2.5G/5G      with SD speed 20.625G */

    /* USX 8 Lanes */
    CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E,     /* 53 */ /* Maximum of 8 ports with PCH, CPSS_PORT_SPEED_1000_E, SD speed 10G */
    CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E,  /* 54 */ /* 8 Ports -> 10M/100M/1G/2.5G         with SD speed 20.625G */

    CPSS_PORT_INTERFACE_MODE_2500BASE_X_E,      /* 55 */ /* CPSS_PORT_SPEED_2500_E, */
    CPSS_PORT_INTERFACE_MODE_REMOTE_E,              /* 56 */ /* used for remote ports configuration.(For SIP6 devices only) */

    CPSS_PORT_INTERFACE_MODE_NA_E,              /* 57 */

    CPSS_PORT_INTERFACE_MODE_NA_HCD_E = 0xFF    /* 255 */ /* Used to indicate we are waiting for HCD resolution in AP */

}CPSS_PORT_INTERFACE_MODE_ENT;

/**
* @enum MV_HWS_RX_TERMINATION
* @endinternal
*
* @brief   RX termination mode
*
*/
typedef enum
{
    CPSS_SERDES_TERMINATION_GND,
    CPSS_SERDES_TERMINATION_VDD,
    CPSS_SERDES_TERMINATION_FLOATING

}CPSS_SERDES_TERMINATION;

/**
* @enum CPSS_PORT_MAC_TYPE_ENT
 *
 * @brief Port MAC Type enumeration
*/
typedef enum{

    /** port not exists in the device. */
    CPSS_PORT_MAC_TYPE_NOT_EXISTS_E,

    /** 10/100 Mbps - using GE MAC Unit */
    CPSS_PORT_MAC_TYPE_FE_E,

    /** 10/100/1000 Mbps - using GE MAC Unit */
    CPSS_PORT_MAC_TYPE_GE_E,

    /** using XG MAC Unit */
    CPSS_PORT_MAC_TYPE_XG_E,

    /** using XLG MAC Unit */
    CPSS_PORT_MAC_TYPE_XLG_E,

    /** using HGL MAC Unit */
    CPSS_PORT_MAC_TYPE_HGL_E,

    /** using CG MAC Unit (100Gbps) */
    CPSS_PORT_MAC_TYPE_CG_E,

    /** using Interlaken MAC Unit */
    CPSS_PORT_MAC_TYPE_ILKN_E,

    /** using MTI MAC 64 Unit(1G to 100G) */
    CPSS_PORT_MAC_TYPE_MTI_100_E,

    /** using MTI MAC 400 Unit(200G to 400G) */
    CPSS_PORT_MAC_TYPE_MTI_400_E,

    /** using MTI MAC CPU Unit(10G/25G) */
    CPSS_PORT_MAC_TYPE_MTI_CPU_E,

    CPSS_PORT_MAC_TYPE_MTI_USX_E,

    /** @brief not applicable mode, for
     *  validity checks and loops
     */
    CPSS_PORT_MAC_TYPE_NOT_APPLICABLE_E,

    CPSS_PORT_MAC_TYPE_LAST_E = CPSS_PORT_MAC_TYPE_NOT_APPLICABLE_E

} CPSS_PORT_MAC_TYPE_ENT;

/**
* @enum CPSS_PORT_SPEED_ENT
 *
 * @brief Enumeration of port speeds
*/
typedef enum
{
    CPSS_PORT_SPEED_10_E,       /* 0 */
    CPSS_PORT_SPEED_100_E,      /* 1 */
    CPSS_PORT_SPEED_1000_E,     /* 2 */
    CPSS_PORT_SPEED_10000_E,    /* 3 */
    CPSS_PORT_SPEED_12000_E,    /* 4 */
    CPSS_PORT_SPEED_2500_E,     /* 5 */
    CPSS_PORT_SPEED_5000_E,     /* 6 */
    CPSS_PORT_SPEED_13600_E,    /* 7 */
    CPSS_PORT_SPEED_20000_E,    /* 8 */
    CPSS_PORT_SPEED_40000_E,    /* 9 */
    CPSS_PORT_SPEED_16000_E,    /* 10 */
    CPSS_PORT_SPEED_15000_E,    /* 11 */
    CPSS_PORT_SPEED_75000_E,    /* 12 */
    CPSS_PORT_SPEED_100G_E,     /* 13 */
    CPSS_PORT_SPEED_50000_E,    /* 14 */
    CPSS_PORT_SPEED_140G_E,     /* 15 */

    CPSS_PORT_SPEED_11800_E,    /* 16 */ /*used in combination with CPSS_PORT_INTERFACE_MODE_XHGS_E or CPSS_PORT_INTERFACE_MODE_XHGS_SR_E */
    CPSS_PORT_SPEED_47200_E,    /* 17  */ /*used in combination with CPSS_PORT_INTERFACE_MODE_XHGS_E or CPSS_PORT_INTERFACE_MODE_XHGS_SR_E */
    CPSS_PORT_SPEED_22000_E,    /* 18  */ /*used in combination with CPSS_PORT_INTERFACE_MODE_XHGS_SR_E */
    CPSS_PORT_SPEED_23600_E,    /* 19  */ /*used in combination with CPSS_PORT_INTERFACE_MODE_XHGS_E or CPSS_PORT_INTERFACE_MODE_XHGS_SR_E */
    CPSS_PORT_SPEED_12500_E,    /* 20  */ /* used in combination with CPSS_PORT_INTERFACE_MODE_KR_E */
    CPSS_PORT_SPEED_25000_E,    /* 21  */ /* used in combination with CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_INTERFACE_MODE_KR2_E */
    CPSS_PORT_SPEED_107G_E,     /* 22 */  /* used in combination with CPSS_PORT_INTERFACE_MODE_KR4_E */
    CPSS_PORT_SPEED_29090_E,    /* 23 */  /* used in combination with CPSS_PORT_INTERFACE_MODE_SR_LR_E */
    CPSS_PORT_SPEED_200G_E,     /* 24 */  /* Falcon new speed */
    CPSS_PORT_SPEED_400G_E,     /* 25 */  /* Falcon new speed */
    CPSS_PORT_SPEED_102G_E,     /* 26 */  /* used in combination with CPSS_PORT_INTERFACE_MODE_KR4_E */
    CPSS_PORT_SPEED_52500_E,    /* 27 */  /* used in combination with CPSS_PORT_INTERFACE_MODE_KR2_E */
    CPSS_PORT_SPEED_26700_E,    /* 28 */  /* used in combination with CPSS_PORT_INTERFACE_MODE_KR_E */
    CPSS_PORT_SPEED_106G_E,     /* 29 */  /* used in combination with CPSS_PORT_INTERFACE_MODE_KR4_E */
    CPSS_PORT_SPEED_42000_E,    /* 30 */  /* used in combination with CPSS_PORT_INTERFACE_MODE_KR4_E */
    CPSS_PORT_SPEED_53000_E,    /* 31 */  /* used in combination with CPSS_PORT_INTERFACE_MODE_KR2_E */
    CPSS_PORT_SPEED_424G_E,     /* 32 */  /* used in combination with CPSS_PORT_INTERFACE_MODE_KR8_E */
    CPSS_PORT_SPEED_212G_E,     /* 33 */  /* used in combination with CPSS_PORT_INTERFACE_MODE_KR4_E */
    CPSS_PORT_SPEED_REMOTE_E,   /* 34 */     /* used for remote port configuration (For SIP6 devices only)*/
    CPSS_PORT_SPEED_NA_E,       /* 35 */
    CPSS_PORT_SPEED_NA_HCD_E = 0xFF   /* 255 */  /* Used to indicate we are waiting for HCD resolution in AP */

}CPSS_PORT_SPEED_ENT;

#define CPSS_PORT_SPEED_SPECIAL_MODE(_speed) ((_speed == CPSS_PORT_SPEED_42000_E) || (_speed == CPSS_PORT_SPEED_47200_E) || (_speed == CPSS_PORT_SPEED_53000_E) || (_speed == CPSS_PORT_SPEED_106G_E) || \
                                    (_speed == CPSS_PORT_SPEED_102G_E) || (_speed == CPSS_PORT_SPEED_424G_E))

/**
* @struct CPSS_PORT_ATTRIBUTES_STC
 *
 * @brief port attributes
*/
typedef struct{

    /** Is link up */
    GT_BOOL portLinkUp;

    /** Port speed */
    CPSS_PORT_SPEED_ENT portSpeed;

    /** Duplex mode */
    CPSS_PORT_DUPLEX_ENT portDuplexity;

} CPSS_PORT_ATTRIBUTES_STC;


/**
* @enum CPSS_PORT_XGMII_MODE_ENT
 *
 * @brief Enumeration of XGMII Transmit Inter-Packet Gap (IPG) modes
 * that can be used in the Prestera system.
*/
typedef enum{

    /** @brief LAN Mode Maintains a Deficit Idle Count (DIC), which is used to decide
     *  whether to add or delete idle characters and maintain an average IPG of
     *  96 bits
     */
    CPSS_PORT_XGMII_LAN_E,

    /** IPG is stretched to adopt to OC192 speed. */
    CPSS_PORT_XGMII_WAN_E,

    /** @brief Adds up to three idle symbols to a Base IPG that ranges from 64 bits
     *  to 120 bits in steps of 32bits, to align the start symbol to Lane 0
     */
    CPSS_PORT_XGMII_FIXED_E

} CPSS_PORT_XGMII_MODE_ENT;


/**
* @struct CPSS_PORT_MAC_STATUS_STC
 *
 * @brief Structure for configuring Protocol based classification
*/
typedef struct{

    /** whether port receives pause. */
    GT_BOOL isPortRxPause;

    /** whether port transmit pause. */
    GT_BOOL isPortTxPause;

    /** whether is in Back pressure. */
    GT_BOOL isPortBackPres;

    /** whether port lacking rx buffers. */
    GT_BOOL isPortBufFull;

    /** whether port sync test failed. */
    GT_BOOL isPortSyncFail;

    /** whether the port in High Error rate. */
    /** this parameter is not relevant and always FALSE and we keep it for Backward compatibility only.*/
    GT_BOOL isPortHiErrorRate;

    /** whether port Auto Neg done. */
    GT_BOOL isPortAnDone;

    /** whether port has fatal error. */
    /**true if "Local Fault" or "Remote Fault" is on.*/
    GT_BOOL isPortFatalError;

} CPSS_PORT_MAC_STATUS_STC;

/**
* @enum CPSS_PORT_DIRECTION_ENT
 *
 * @brief Enumeration of port direction.
*/
typedef enum{

    /** port's Rx direction */
    CPSS_PORT_DIRECTION_RX_E,

    /** port's Tx direction */
    CPSS_PORT_DIRECTION_TX_E,

    /** port's Rx and Tx directions */
    CPSS_PORT_DIRECTION_BOTH_E

} CPSS_PORT_DIRECTION_ENT;

/**
* @enum CPSS_PORT_RX_FC_PROFILE_SET_ENT
 *
 * @brief Enumeration of Flow control profile set
*/
typedef enum{

    /** Flow Control profile 1 */
    CPSS_PORT_RX_FC_PROFILE_1_E =0,

    /** Flow Control profile 2 */
    CPSS_PORT_RX_FC_PROFILE_2_E,

    /** Flow Control profile 3 */
    CPSS_PORT_RX_FC_PROFILE_3_E,

    /** Flow Control profile 4 */
    CPSS_PORT_RX_FC_PROFILE_4_E,

    /** Flow Control profile 5 (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman) */
    CPSS_PORT_RX_FC_PROFILE_5_E,

    /** Flow Control profile 6 (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman) */
    CPSS_PORT_RX_FC_PROFILE_6_E,

    /** Flow Control profile 7 (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman) */
    CPSS_PORT_RX_FC_PROFILE_7_E,

    /** Flow Control profile 8 (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman) */
    CPSS_PORT_RX_FC_PROFILE_8_E

} CPSS_PORT_RX_FC_PROFILE_SET_ENT;

/**
* @enum CPSS_PORT_XG_FIXED_IPG_ENT
 *
 * @brief Enumeration of The IPG base used to calculate the IPG when XG
 * port is set to fixed mode.
*/
typedef enum{

    /** Fixed IPG uses 8 bytes as the base IPG. */
    CPSS_PORT_XG_FIXED_IPG_8_BYTES_E,

    /** Fixed IPG uses 12 bytes as the base IPG. */
    CPSS_PORT_XG_FIXED_IPG_12_BYTES_E

} CPSS_PORT_XG_FIXED_IPG_ENT;

/**
* @enum CPSS_PORT_FLOW_CONTROL_ENT
 *
 * @brief Enumeration of Port FC Direction enablers.
*/
typedef enum{

    /** Both disabled */
    CPSS_PORT_FLOW_CONTROL_DISABLE_E = GT_FALSE,

    /** Both enabled */
    CPSS_PORT_FLOW_CONTROL_RX_TX_E = GT_TRUE,

    /** Only Rx FC enabled */
    CPSS_PORT_FLOW_CONTROL_RX_ONLY_E,

    /** Only Tx FC enabled */
    CPSS_PORT_FLOW_CONTROL_TX_ONLY_E

} CPSS_PORT_FLOW_CONTROL_ENT;

/**
* @enum CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT
 *
 * @brief Enumeration of Port Periodic FC type enablers.
*/
typedef enum{

    /** Both disabled */
    CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = GT_FALSE,

    /** Both enabled */
    CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_XOFF_E = GT_TRUE,

    /** Only XON FC enabled */
    CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E,

    /** Only XOFF FC enabled */
    CPSS_PORT_PERIODIC_FLOW_CONTROL_XOFF_ONLY_E

} CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT;


/**
* @struct CPSS_PORT_MODE_SPEED_STC
 *
 * @brief pair of port interface mode and speed
 * possible configuration
*/
typedef struct{

    /** port interface mode */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;

    /** port speed */
    CPSS_PORT_SPEED_ENT speed;

} CPSS_PORT_MODE_SPEED_STC;

/**
* @enum CPSS_PORT_PCS_RESET_MODE_ENT
 *
 * @brief Enumeration of port PCS reset direction.
*/
typedef enum{

    /** reset/unreset PCS Rx only */
    CPSS_PORT_PCS_RESET_MODE_RX_E,

    /** reset/unreset PCS Tx only */
    CPSS_PORT_PCS_RESET_MODE_TX_E,

    /** reset/unreset PCS in all modes */
    CPSS_PORT_PCS_RESET_MODE_ALL_E

} CPSS_PORT_PCS_RESET_MODE_ENT;

/**
* @enum CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT
 *
 * @brief Enumeration of serdes training optimisation algorithms that could
 * run on port's serdes
*/
typedef enum{

    /** HW optimization only */
    CPSS_PORT_SERDES_TRAINING_OPTIMISATION_NONE_E       = 0,

    /** @brief DFE SW optimization on
     *  top of HW optimization
     */
    CPSS_PORT_SERDES_TRAINING_OPTIMISATION_DFE_E        = 0x1,

    /** @brief FFE SW optimization on
     *  top of HW optimization
     */
    CPSS_PORT_SERDES_TRAINING_OPTIMISATION_FFE_E        = 0x2,

    /** @brief Align90 SW
     *  optimization on top of HW optimization
     */
    CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALIGN90_E    = 0x4,

    /** @brief All SW optimization on top
     *  of HW optimization
     */
    CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALL_E        = 0xF

} CPSS_PORT_SERDES_TRAINING_OPTIMISATION_ALGORITHM_ENT;

/**
* @enum CPSS_PORT_REF_CLOCK_SOURCE_ENT
 *
 * @brief Enumeration for port reference clock source parameter
*/
typedef enum{

    /** primary reference clock source */
    CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E,

    /** primary reference clock source */
    CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E,

    /** last (illegal) value of enum */
    CPSS_PORT_REF_CLOCK_SOURCE_LAST_E

} CPSS_PORT_REF_CLOCK_SOURCE_ENT;

/**
* @enum CPSS_PORT_SERDES_SPEED_ENT
 *
 * @brief Defines SERDES speed.
*/
typedef enum{

    /** Serdes speed is 1.25G, used by 1000Base_X , SGMII */
    CPSS_PORT_SERDES_SPEED_1_25_E,

    /** Serdes speed is 3.125G, used by XAUI, 2500Base_X/2.5G SGMII */
    CPSS_PORT_SERDES_SPEED_3_125_E,

    /** Serdes speed is 3.75G */
    CPSS_PORT_SERDES_SPEED_3_75_E,

    /** Serdes speed is 6.25G, used by DXAUI and RXAUI */
    CPSS_PORT_SERDES_SPEED_6_25_E,

    /** Serdes speed is 5G */
    CPSS_PORT_SERDES_SPEED_5_E,

    /** Serdes speed is 4.25G */
    CPSS_PORT_SERDES_SPEED_4_25_E,

    /** Serdes speed is 2.5G */
    CPSS_PORT_SERDES_SPEED_2_5_E,

    /** Serdes speed is 5.156G (x8 serdes = 40G data speed) */
    CPSS_PORT_SERDES_SPEED_5_156_E,

    /** used by KR, SR_LR, 40G KR4, 40G SR_LR, 20G_KR */
    CPSS_PORT_SERDES_SPEED_10_3125_E,

    /** used by HGL 16G */
    CPSS_PORT_SERDES_SPEED_3_333_E,

    /** for 12.1G */
    CPSS_PORT_SERDES_SPEED_12_5_E,

    /** legacy */
    CPSS_PORT_SERDES_SPEED_7_5_E,

    /** legacy */
    CPSS_PORT_SERDES_SPEED_11_25_E,

    /** for 22G SR_LR */
    CPSS_PORT_SERDES_SPEED_11_5625_E,

    /** legacy */
    CPSS_PORT_SERDES_SPEED_10_9375_E,

    /** for 11.8G, 23.6G, 47.2G */
    CPSS_PORT_SERDES_SPEED_12_1875_E,

    /** for 5.45G in xCat3 */
    CPSS_PORT_SERDES_SPEED_5_625_E,

    /** for 12.5G, 25G and 50G port speed */
    CPSS_PORT_SERDES_SPEED_12_8906_E,

    /** for 40G R2 */
    CPSS_PORT_SERDES_SPEED_20_625_E,

    /** for 25G, 50G, 100G KR4 and SR_LR4 */
    CPSS_PORT_SERDES_SPEED_25_78125_E,

    /** for EDSA compensation mode 107G */
    CPSS_PORT_SERDES_SPEED_27_5_E,

    /** for EDSA compensation mode 109G */
    CPSS_PORT_SERDES_SPEED_28_05_E,

    /** for port mode 102GBase-KR4 */
    CPSS_PORT_SERDES_SPEED_26_25_E,

    /** for EDSA compensation mode 52_5G */
    CPSS_PORT_SERDES_SPEED_27_1_E,

    /** Last member, the speed is not determind */
    CPSS_PORT_SERDES_SPEED_NA_E

} CPSS_PORT_SERDES_SPEED_ENT;

/**
* @enum CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT
 *
 * @brief Enumeration of port's serdes auto tuning modes.
*/
typedef enum{

    /** serdes auto-tuning (rx/tx) succeeded */
    CPSS_PORT_SERDES_AUTO_TUNE_PASS_E,

    /** serdes auto-tuning (rx/tx) failed */
    CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E,

    /** @brief serdes auto-tuning (rx/tx)
     *  still in process
     */
    CPSS_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E

} CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT;

/**
 * @enum CPSS_PORT_SERDES_TYPE_ENT
 *
 * @brief Enumeration of port's serdes types.
*/
typedef enum{

    /** COMPHY_H - BobCat2, Lion2, xCat3 */
    CPSS_PORT_SERDES_COMPHY_H_E,

    /**  BobK, PIPE, Aldrin, AC3X, Aldrin2, BC3, Falcon */
    CPSS_PORT_SERDES_AVAGO_E,

    /**  AC5 */
    CPSS_PORT_SERDES_COMPHY_C12G_E,

    /**  AC5X, AC5P */
    CPSS_PORT_SERDES_COMPHY_C28G_E,

    /**  AC5P */
    CPSS_PORT_SERDES_COMPHY_C112G_E,

    /**  Harrier */
    CPSS_PORT_SERDES_COMPHY_C56G_E,

    CPSS_PORT_SERDES_TYPE_LAST_E

} CPSS_PORT_SERDES_TYPE_ENT;

/**
* @struct CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC
 *
 * @brief Port COMPHY_H SERDES TX configuration parameters.
*/
typedef struct{

    /** @brief Tx Driver output Amplitude/Attenuator: APPLICABLE RANGES: [0...31]
     *  In ComPhyH Serdes for xCat3; Lion2; Bobcat2 devices: Amplitude
     */
    GT_U32 txAmp;

    /** @brief Transmitter Amplitude Adjust: (GT_TRUE
     *  APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; AC5; Lion2; Bobcat2 devices
     */
    GT_BOOL txAmpAdjEn;

    /** @brief Controls the Emphasis Amplitude for Gen0 bit rates
     *  for Lion2 device [0...15]
     */
    GT_32 emph0;

    /** @brief Controls the emphasis amplitude for Gen1 bit rates
     *  for Lion2 device [0...15]
     */
    GT_32 emph1;

    /** @brief Transmitter Amplitude Shift
    *   (APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2)
    *   (APPLICABLE RANGES: GT_TRUE - enable, GT_FALSE - disable)
    */
    GT_BOOL  txAmpShft;

    /** @brief Pre
     *  APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; AC5; Lion2; Bobcat2 devices
     */
    GT_BOOL txEmphEn;

    /** @brief Pre
     *  APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; AC5; Lion2; Bobcat2 devices
     */
    GT_BOOL txEmphEn1;

    /** @brief Tx Driver output amplitude
     *  txAmpAdj- Transmitter Amplitude Adjust (APPLICABLE DEVICES:
     *  BC2, Lion2 ) (APPLICABLE RANGES: 0..15)
     */
    GT_U32 txAmpAdj;

    /** @brief slewCtrlEn
     *  APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; AC5; Bobcat2 devices
     */
    GT_BOOL slewCtrlEn;

    /** @brief slewRate
     *  APPLICABLE DEVICES only for ComPhyH Serdes: xCat3; AC5; Bobcat2 devices
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 slewRate;

} CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC;

/**
 * @struct CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC;
 *
 * @brief Comphy C12G Tx tune parameters
*/
typedef struct
{
    GT_U32 pre;
    GT_U32 peak;
    GT_U32 post;
}CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC;

/**
 * @struct CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC;
 *
 * @brief Comphy C28G Tx tune parameters
*/
typedef struct
{
    GT_U32 pre;
    GT_U32 peak;
    GT_U32 post;
}CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC;

/**
 * @struct CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC
 *
 * @brief Comphy C56G Tx tune parameters
*/
typedef struct
{
    GT_32   pre2;
    GT_32   pre;
    GT_32   main;
    GT_32   post;
    GT_BOOL usr;
}CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC;

/**
 * @struct CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC
 *
 * @brief Comphy C112G Tx tune parameters
*/
typedef struct
{
    GT_32 pre2;
    GT_32 pre;
    GT_32 main;
    GT_32 post;
}CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC;

/**
* @struct CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC
*
* @brief Port AVAGO SERDES TX configuration parameters
*/
typedef struct
{
    /** @brief Serdes Tx Attenuator
    *   (APPLICABLE DEVICES: Caelum and above)
    *   (APPLICABLE RANGES: 0...31)
    */
    GT_U32   atten;

    /** @brief Serdes Post-Cursor
    *   (APPLICABLE DEVICES: Caelum, Aldrin, AC3X, Aldrin2(for
    *                        Serdes 24-71), Pipe(for Serdes 0-11)
    *   (APPLICABLE RANGES: 31...31)
    *   APPLICABLE DEVICES: Bobcat3, Aldrin2(for Serdes 0-23),
    *                       Pipe(for ports 12-15)
    *   (APPLICABLE RANGES: 0...31)
    *   APPLICABLE DEVICE: Falcon
    *   (APPLICABLE RANGES: even values -18...18)
    */
    GT_32    post;

    /** @brief Serdes Pre-Cursor
    *   (APPLICABLE DEVICES: Caelum, Aldrin, AC3X, Aldrin2(for
    *                        Serdes 24-71), Pipe(for ports 0-11)
    *   (APPLICABLE RANGES: -31...31)
    *   (APPLICABLE DEVICES: Bobcat3, Aldrin2(for Serdes 0-23),
    *                        Pipe(for ports 12-15)
    *   (APPLICABLE RANGES: 0...31)
    *   (APPLICABLE DEVICE: Falcon
    *   (APPLICABLE RANGES: even values -10...10)
    */
    GT_32    pre;

    /** @brief Serdes Pre2-Cursor
    *   APPLICABLE DEVICE: Falcon
    *   (APPLICABLE RANGES -15...15)
    */
    GT_32    pre2;

    /** @brief Serdes Pre3-Cursor
    *   APPLICABLE DEVICE: Falcon
    *   (APPLICABLE RANGES [-1, 0, 1)
    */
    GT_32    pre3;

}CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC;

/**
* @union CPSS_PORT_SERDES_TX_CONFIG_UNT
 *
*  @brief includes a union for the different serdeses.
*/
typedef union {

    CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC comphy;
    CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC   comphy_C12G;
    CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC   comphy_C28G;
    CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC   comphy_C56G;
    CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC  comphy_C112G;
    CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC  avago;
}CPSS_PORT_SERDES_TX_CONFIG_UNT;

/**
* @struct CPSS_PORT_SERDES_TX_CONFIG_STC
 *
 * @brief Port SERDES TX configuration parameters.
*/
typedef struct{

    CPSS_PORT_SERDES_TYPE_ENT type;
    CPSS_PORT_SERDES_TX_CONFIG_UNT txTune;

} CPSS_PORT_SERDES_TX_CONFIG_STC;

/**
* @struct CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC
 *
 * @brief Port COMPHY SERDES RX configuration parameters.
*/
typedef struct{
    /** @brief dcGain controls the gain according to the previous
     *  received bit (one tap) and compensates for interconnect ISI
     *  and ILD (refer to the DFE_F1 field)
     *  (APPLICABLE DEVICES: xCat3; AC5)
     *  (APPLICABLE RANGES: 0..31)
     */
    GT_U32 dcGain;

    /** @brief bandwidth controls the BW according to the
     *  previous received bit (one tap) and compensates for
     *  interconnect ISI and ILD (refer to the DFE_F1 field)
     *  (APPLICABLE DEVICES: xCat3; AC5)
     *  (APPLICABLE RANGES: 0..31)
     */
    GT_U32 bandWidth;

    /** @brief A digital filter controls the gain according to the previous
     *  received bit (one tap) and compensates for interconnect ISI
     *  and ILD (refer to the DFE_F1 field)
     *  (APPLICABLE DEVICES: xCat3; AC5)
     *  (APPLICABLE RANGES: 0..31)
     */
    GT_U32 dfe;

    /** @brief mainly controls the low frequency gain (refer to the
    *   FFE_res_sel field)
    *   (APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2)
    *   (APPLICABLE RANGES: 0..7)
    */
    GT_U32  ffeR;

    /** @brief mainly controls the high frequency gain (refer to the
    *   FFE_cap_sel field)
    *   (APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2)
    *   (APPLICABLE RANGES: 0..15)
    */
    GT_U32  ffeC;

    /** @brief sampler (refer to the cal_os_ph_rd field)
     *  (APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2)
     */
    GT_U32  sampler;

    /** @brief Threshold that trips the Squelch detector peak
    *   differential amplitude (refer to the SQ_THRESH field)
    *   (APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2)
    *  (APPLICABLE RANGES: 0..15)
    */
    GT_U32  sqlch;

    /** @brief Align 90 Calibration Phase Offset(This is the external
     *  value used in place of the autocalibration value for
     *  rxclkalign90). (APPLICABLE RANGES: 0..127)
     */
    GT_U32 align90;

    /** @brief FFE signal swing control (APPLICABLE DEVICES: xCat3; AC5)
    *  (APPLICABLE RANGES: 0..3)
    */
    GT_U32 ffeS;

    /** @brief Adapted DFE Coefficient
    *   This field indicates the DFE auto-calibration and auto-trained results
    *   (APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2)
    */
    GT_32  dfeValsArray[CPSS_PORT_DFE_VALUES_ARRAY_SIZE_CNS];

}CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC;

/**
 * @struct CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_ST
 *
 * @brief This struct include RX data
*/
typedef struct
{
    /* Basic (CTLE) */
    GT_U32 resSel;
    GT_U32 resShift;
    GT_U32 capSel;

    GT_U8 ffeSettingForce;
    GT_U8 adaptedResSel;
    GT_U8 adaptedCapSel;

    /* Advanced (CDR) */
    GT_U32 selmufi;
    GT_U32 selmuff;
    GT_U32 selmupi;
    GT_U32 selmupf;

    GT_32 squelch;

    GT_U32 align90;
    GT_U32 sampler;
    GT_U32 slewRateCtrl0;
    GT_U32 slewRateCtrl1;
    GT_32  dfe[12];

    GT_U32 EO;

} CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC;

/** @struct CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC
 *
 * @brief This struct include RX data
*/
typedef struct
{
    /* Basic (CTLE) */
    GT_U32 dataRate;
    GT_U32 res1Sel;
    GT_U32 res2Sel;
    GT_U32 cap1Sel;
    GT_U32 cap2Sel;

    /* Advanced (CDR) */
    GT_U32 selmufi;
    GT_U32 selmuff;
    GT_U32 selmupi;
    GT_U32 selmupf;

    /* Advanced (Thresholds) */
    GT_U32 midpointLargeThresKLane;
    GT_U32 midpointSmallThresKLane;
    GT_U32 midpointLargeThresCLane;
    GT_U32 midpointSmallThresCLane;

    /* Advanced (DFE) */
    GT_U32 dfeResF0aHighThresInitLane;
    GT_U32 dfeResF0aHighThresEndLane;

    GT_32  squelch;

    GT_U32 align90;
    GT_U32 sampler;
    GT_U32 slewRateCtrl0;
    GT_U32 slewRateCtrl1;
    GT_32  dfe[25];

    GT_U32  EO;

} CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC;

/**
 * @struct CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC
 *
 * @brief This struct include RX data
*/
typedef struct
{
    /* Basic (CTLE) */
    GT_U32 cur1Sel;
    GT_U32 rl1Sel;
    GT_U32 rl1Extra;
    GT_U32 res1Sel;
    GT_U32 cap1Sel;
    GT_U32 enMidfreq;
    GT_U32 cs1Mid;
    GT_U32 rs1Mid;
    GT_U32 cur2Sel;
    GT_U32 rl2Sel;
    GT_U32 rl2TuneG;
    GT_U32 res2Sel;
    GT_U32 cap2Sel;

    /* Advanced (CDR) */
    GT_U32 selmufi;
    GT_U32 selmuff;
    GT_U32 selmupi;
    GT_U32 selmupf;

    GT_32  squelch;
    GT_32  dfe[26];

} CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC;

/**
 * @struct CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC
 *
 * @brief This struct include RX data
*/
typedef struct
{
    /* Basic (CTLE) */
    /* 1st stage GM Main */
    GT_U32 current1Sel;
    GT_U32 rl1Sel;
    GT_U32 rl1Extra;
    GT_U32 res1Sel;
    GT_U32 cap1Sel;
    GT_U32 cl1Ctrl;
    GT_U32 enMidFreq;
    GT_U32 cs1Mid;
    GT_U32 rs1Mid;
    /* 1st stage TIA */
    GT_U32 rfCtrl;
    GT_U32 rl1TiaSel;
    GT_U32 rl1TiaExtra;
    GT_U32 hpfRSel1st;
    GT_U32 current1TiaSel;
    /* 2nd Stage */
    GT_U32 rl2Tune;
    GT_U32 rl2Sel;
    GT_U32 rs2Sel;
    GT_U32 current2Sel;
    GT_U32 cap2Sel;
    GT_U32 hpfRsel2nd;

    /* Advanced (CDR) */
    GT_U32 selmufi;
    GT_U32 selmuff;
    GT_U32 selmupi;
    GT_U32 selmupf;

    GT_U32  squelch;

    GT_U32 align90AnaReg;
    GT_32  align90;
    GT_U32 sampler;
    GT_U32 slewRateCtrl0;
    GT_U32 slewRateCtrl1;
    GT_32  dfe[40];

    GT_U32  EO;

} CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC;

/**
* @struct CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC
 *
 * @brief Port AVAGO SERDES RX configuration parameters.
*/
typedef struct{



    /** @brief Threshold that trips the Squelch detector peak
    *   differential amplitude (refer to the SQ_THRESH field)
    *  In Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Pipe devices: mV [68...308]
    */
    GT_U32  sqlch;

    /** (APPLICABLE RANGES: 0..1)(APPLICABLE DEVICES, Aldrin, AC3X: Caelum, Pipe) */
    GT_U32 DC;

    /** (APPLICABLE RANGES: 0..1)(APPLICABLE DEVICES, Aldrin, AC3X: Caelum, Pipe) */
    GT_U32 LF;

    /** (APPLICABLE RANGES: 0..1)(APPLICABLE DEVICES, Aldrin, AC3X: Caelum, Pipe) */
    GT_U32 HF;

    /** (APPLICABLE RANGES: 0..1)(APPLICABLE DEVICES, Aldrin, AC3X: Caelum, Pipe) */
    GT_U32 BW;

    /** (APPLICABLE RANGES: 0..1)(APPLICABLE DEVICES, Aldrin, AC3X: Caelum, Pipe) */
    GT_U32 EO;

    /** @brief DFE values
    *   (APPLICABLE DEVICES: Caelum, Bobcat3, Aldrin, AC3X, Aldrin2;
    *   Pipe; Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: -15...15)
    */
    GT_32  DFE[CPSS_PORT_DFE_AVAGO_VALUES_ARRAY_SIZE_CNS];

    /** @brief CTLE gainshape1
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: 0...3)
    */
    GT_U32  gainshape1;

    /** @brief CTLE gainshape2
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: 0...3)
    */
    GT_U32  gainshape2;

    /** @brief CTLE Enable/Disable Short channel
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: GT_TRUE - enable, GT_FALSE - disable)
    */
    GT_U32  shortChannelEn;

    /** @brief DFE Gain Tap strength
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: 0...255)
    */
    GT_U32  dfeGAIN;

    /** @brief DFE Gain Tap2 strength
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: 0...255)
    */
    GT_U32  dfeGAIN2;

    /** @brief DFE BFLF
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: 0...5)
    */
    GT_32  BFLF;

    /** @brief DFE BFHF
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: 0...8)
    */
    GT_32  BFHF;

    /** @brief CTLE minLf
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: 0...15)
    */
    GT_U32  minLf;

    /** @brief CTLE maxLf
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: 0...15)
    */
    GT_U32  maxLf;

    /** @brief CTLE minHf
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: 0...15)
    */
    GT_U32  minHf;

    /** @brief CTLE maxHf
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: 0...15)
    */
    GT_U32  maxHf;

    /** @brief FFE minPre1
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: 0...15)
    */
    GT_32   minPre1;

    /** @brief FFE maxPre1
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: 0...15)
    */
    GT_32   maxPre1;

    /** @brief FFE minPre2
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: -10...10)
    */
    GT_32   minPre2;

    /** @brief FFE maxPre2
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: -10...10)
    */
    GT_32   maxPre2;

    /** @brief FFE minPost
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: -15...15)
    */
    GT_32   minPost;

    /** @brief FFE minPost
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: -15...15)
    */
    GT_32   maxPost;

    /** @brief FFE pre1
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: 0...15)
    */
    GT_32 pre1;

    /** @brief FFE pre2
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: -15...15)
    */
    GT_32 pre2;

    /** @brief FFE post1
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    *   (APPLICABLE RANGES: -15...15)
    */
    GT_32 post1;

    /** @brief PAM4 values
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman;
    *   indices 0-2 : even (lower, middle, upper)
    *   indices 3-5 : odd  (lower, middle, upper)
    */
    GT_32  pam4EyesArr[CPSS_PAM4_EYES_ARRAY_SIZE_CNS];

    /** @brief termination
    *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman;
    *   (APPLICABLE RANGES: 0...2)
    */
    CPSS_SERDES_TERMINATION  termination;
    /** @brief coldEnvelope
     *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32                    coldEnvelope;
    /** @brief hotEnvelope
     *   (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32                    hotEnvelope;
}CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC;

/**
* @union CPSS_PORT_SERDES_RX_CONFIG_UNT
 *
*  @brief includes a union for the different serdeses.
*/
typedef union {

    CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC       comphy;
    CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC   comphy_C12G;
    CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC       comphy_C28G;
    CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC         comphy_C56G;
    CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC        comphy_C112G;
    CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC        avago;

}CPSS_PORT_SERDES_RX_CONFIG_UNT;

/**
* @struct CPSS_PORT_SERDES_RX_CONFIG_STC
 *
 * @brief Port SERDES RX configuration parameters.
*/
typedef struct{

    CPSS_PORT_SERDES_TYPE_ENT type;
    CPSS_PORT_SERDES_RX_CONFIG_UNT rxTune;

} CPSS_PORT_SERDES_RX_CONFIG_STC;

/**
* @struct *CPSS_PORT_SERDES_TUNE_STC_PTR
*
* @brief Port SERDES lanes fine tuning values
*/
typedef struct{

    /** @brief type -
     *  Type of Serdes
     */
    CPSS_PORT_SERDES_TYPE_ENT type;
    /** @brief txTune -
     *  TX parameters
     */
    CPSS_PORT_SERDES_TX_CONFIG_UNT txTune;
    /** @brief rxTune -
     *  RX parameters
     */
    CPSS_PORT_SERDES_RX_CONFIG_UNT rxTune;

} CPSS_PORT_SERDES_TUNE_STC, *CPSS_PORT_SERDES_TUNE_STC_PTR;

/**
* @enum CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT
 *
 * @brief Enumeration of port's serdes auto tuning modes.
*/
typedef enum{

    /** @brief TX training
     *  configure state
     */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E,

    /** @brief start TX training
     *  itself
     */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E,

    /** @brief get TX training
     *  result
     */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STATUS_E,

    /** run RX training */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E,

    /** stop TX and RX training */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_TRAINING_STOP_E,

    /** @brief start adaptive RX training
     *  (APPLICABLE DEVICES: BobK, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman, Pipe)
     */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_START_E,

    /** @brief stop adaptive RX training
     *  (APPLICABLE DEVICES: BobK, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman, Pipe)
     */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ADAPTIVE_RX_TRAINING_STOP_E,

    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENHANCE_E,

    /** @brief wait for peer TxAmp change than launch training
     *  (APPLICABLE DEVICES: BobK, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman, Pipe) currently this enamurator is
     *  applicable only as a parameter to CPSS_PORT_MANAGER_GENERIC_PARAMS_STC structure
     */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_EDGE_DETECT_E,

    /** @brief run RX training and return
     *  only when rx training finished (APPLICABLE DEVICES: BobK, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman, Pipe)
     */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_WAIT_FOR_FINISH_E,

    /** @brief run iCal-VSR mode, Bypass DFE TAP tuning (APPLICABLE
     *         DEVICES: BobK, Aldrin, AC3X, Bobcat3; Aldrin2;
     *         Falcon, Pipe)
     */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_VSR_E,
        /** @brief Bypass RX training (APPLICABLE
     *         DEVICES: BobK, Aldrin, AC3X, Bobcat3; Aldrin2;
     *         Falcon, Pipe)
     */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_FIXED_CTLE_E,

    /** stop TX training */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STOP_E,

    /** stop RX training */
    CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_STOP_E,

    CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E

} CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT;

/**
* @enum CPSS_PORT_FEC_MODE_ENT
 *
 * @brief Enumeration of port's Forward Error Correction modes.
*/
typedef enum{

    /** FC-FEC enabled on port */
    CPSS_PORT_FEC_MODE_ENABLED_E,

    /** FEC disabled on port */
    CPSS_PORT_FEC_MODE_DISABLED_E,

    /** @brief Reed-Solomon (528,514) FEC mode
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_PORT_RS_FEC_MODE_ENABLED_E,

    /** @brief Used only for AP advertisment.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_PORT_BOTH_FEC_MODE_ENABLED_E,

    /** @brief Reed-Solomon (544,514) FEC mode
     *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E,

    CPSS_PORT_FEC_MODE_LAST_E

} CPSS_PORT_FEC_MODE_ENT;

/**
* @enum CPSS_PORT_AP_FLOW_CONTROL_ENT
 *
 * @brief Enumerator of AP Port FC Direction enablers.
*/
typedef enum{

    /** flow control in both directions */
    CPSS_PORT_AP_FLOW_CONTROL_SYMMETRIC_E,

    /** in one direction */
    CPSS_PORT_AP_FLOW_CONTROL_ASYMMETRIC_E

} CPSS_PORT_AP_FLOW_CONTROL_ENT;

/**
* @struct CPSS_PORT_AP_PARAMS_STC
 *
 * @brief Structure for configuring AP special parameters and advertisment
 * options on port
*/
typedef struct{

    /** FC pause (true/false) */
    GT_BOOL fcPause;

    /** FC assymetric direction (Annex 28B) */
    CPSS_PORT_AP_FLOW_CONTROL_ENT fcAsmDir;

    /** @brief FEC ability (true/false)
     *  (APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X)
     */
    GT_BOOL fecSupported;

    /** @brief Request link partner to enable FEC (true/false)
     *  (APPLICABLE DEVICES: Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_BOOL fecRequired;

    /** @brief GT_TRUE
     *  GT_FALSE - choose one of sides to be ceed according to protocol
     */
    GT_BOOL noneceDisable;

    /** number of serdes lane of port where to run AP */
    GT_U32 laneNum;

    CPSS_PORT_MODE_SPEED_STC modesAdvertiseArr [CPSS_PORT_AP_IF_ARRAY_SIZE_CNS];

    CPSS_PORT_FEC_MODE_ENT fecAbilityArr [CPSS_PORT_AP_IF_ARRAY_SIZE_CNS];

    CPSS_PORT_FEC_MODE_ENT fecRequestedArr [CPSS_PORT_AP_IF_ARRAY_SIZE_CNS];

} CPSS_PORT_AP_PARAMS_STC;

/**
* @struct CPSS_PORT_AP_STATUS_STC
 *
 * @brief Structure for AP resolution result
*/
typedef struct{

    /** AP resolved port number (lane swap result) */
    GT_U32 postApPortNum;

    /** port (interface mode;speed) */
    CPSS_PORT_MODE_SPEED_STC portMode;

    /** @brief indicating AP succeeded to find highest common denominator
     *  with partner
     */
    GT_BOOL hcdFound;

    /** FEC enabled */
    GT_BOOL fecEnabled;

    /** enable RX flow control pause frames */
    GT_BOOL fcRxPauseEn;

    /** enable TX flow control pause frames */
    GT_BOOL fcTxPauseEn;

    /** FEC type fc or RS */
    CPSS_PORT_FEC_MODE_ENT fecType;

} CPSS_PORT_AP_STATUS_STC;

/**
* @struct CPSS_PORT_AP_SERDES_TX_OFFSETS_STC
 *
 * @brief Structure for serdes TX offset parameters
 * to take place during AP port init after resolution
 * found and prior to running TRx training
*/
typedef struct{

    /** TxAmp parameter offset (Range: */
    GT_8 txAmpOffset;

    /** TxEmph0 parameter offset (Range: */
    GT_8 txEmph0Offset;

    /** TxEmph1 parameter offset (Range: */
    GT_8 txEmph1Offset;
#if 0
    /** TxType parameter offset (init - GT_FALSE; preset - GT_TRUE) */
    /* Currently not work*/
    GT_BOOL txTypePresetOffset;
#endif
    /** Request from peer side to start in init and not Preset
     *  (Disable Preset request */
    GT_BOOL reqInit;

    /** interface mode associated with the offset values */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;

    /** speed associated with the offset values */
    CPSS_PORT_SPEED_ENT speed;

} CPSS_PORT_AP_SERDES_TX_OFFSETS_STC;

#define CPSS_PORT_AP_SERDES_RX_SQLCH_CNS           0x1
#define CPSS_PORT_AP_SERDES_RX_LF_CNS              0x2
#define CPSS_PORT_AP_SERDES_RX_HF_CNS              0x4
#define CPSS_PORT_AP_SERDES_RX_DCGAIN_CNS          0x8
#define CPSS_PORT_AP_SERDES_RX_BANDWIDTH_CNS       0x10
#define CPSS_PORT_AP_SERDES_ETL_MIN_DELAY_CNS      0x40
#define CPSS_PORT_AP_SERDES_ETL_MAX_DELAY_CNS      0x80
#define CPSS_PORT_AP_SERDES_ETL_ENABLE_CNS         0x100

/**
* @struct CPSS_PORT_AP_SERDES_RX_CONFIG_STC
 *
 * @brief Structure for serdes RX parameters
 * to take place during AP port init TRx training
*/
typedef struct{

    /** 310) */
    GT_U16 squelch;

    /** Frequency  (rang: 0 */
    GT_U8 lowFrequency;

    /** Frequency (rang: 0 */
    GT_U8 highFrequency;

    /** Gain value    (rang: 0 */
    GT_U8 dcGain;

    /** width   (rang: 0 */
    GT_U8 bandWidth;

    /** @brief 31)
     *  etlMinDelay  ETL max delay    (rang: 0-31)
     */
    GT_U8 etlMinDelay;

    GT_U8 etlMaxDelay;

    /** Etl enable(override defaults) */
    GT_BOOL etlEnable;

    /** @brief field bitmap
     *  speed   - serdes speed associated with the config values
     */
    GT_U16 fieldOverrideBmp;

    CPSS_PORT_SERDES_SPEED_ENT serdesSpeed;

} CPSS_PORT_AP_SERDES_RX_CONFIG_STC;

/**
* @struct CPSS_PORT_MAC_TO_SERDES_STC
 *
*  @brief Structure for serdes muxing mapping
*/
typedef struct
{
    GT_U8 serdesLanes[MV_HWS_MAX_LANES_NUM_PER_PORT];
}  CPSS_PORT_MAC_TO_SERDES_STC;

/**
* @struct CPSS_DXCH_RSFEC_COUNTERS_STC
 *
 * @brief RSFEC counters .
*/
typedef struct {
    /** @brief correctedFecCodeword  */
    GT_U64   correctedFecCodeword;
    /** @brief uncorrectedFecCodeword. */
    GT_U64   uncorrectedFecCodeword;
    /** @brief correctFecCodeword. */
    GT_U64   correctFecCodeword;
    /** @brief Symbol errors counter */
    GT_U32   symbolError[4];
    /** @brief total_cw_received */
    GT_U64   total_cw_received;
} CPSS_RSFEC_COUNTERS_STC;
/**
* @struct CPSS_DXCH_FCFEC_COUNTERS_STC
 *
 * @brief FCFEC counters .
*/
typedef struct {
    /** @brief numReceivedBlocks  */
    GT_U32   numReceivedBlocks;
    /** @brief numReceivedBlocksNoError. */
    GT_U32   numReceivedBlocksNoError;
    /** @brief numReceivedBlocksCorrectedError */
    GT_U32   numReceivedBlocksCorrectedError;
    /** @brief numReceivedBlocksUncorrectedError  */
    GT_U32   numReceivedBlocksUncorrectedError;
    /** @brief numReceivedCorrectedErrorBits. */
    GT_U32   numReceivedCorrectedErrorBits;
    /** @brief numReceivedUncorrectedErrorBits */
    GT_U32   numReceivedUncorrectedErrorBits;
    /** @brief  BlocksCorrectedError for every lane array   */
    GT_U32   blocksCorrectedError[4];
    /** @brief  BlocksUncorrectedError for every lane array   */
    GT_U32   blocksUncorrectedError[4];
} CPSS_FCFEC_COUNTERS_STC;


/*
 * ANP counters information:
 *    anRestartCounter  - Counts the number of times AN had restart, after got to resolution.
 *                        That is, link_fail_inhibit timer expired without link, or AN completed, and then link failed.
 *    dspLockFailCounter- Counts the number of times Channel SM waits for dsp_lock and gets timeout.
 *    linkFailCounter   - Counts the number of times Channel SM waits for link and gets timeout.
 *    txTrainDuration   - Holds the duration (clock cycles) of the last tx_train.
 *    txTrainFailCounter- Counts the number of times tx_train performed and completed with fail status.
 *    txTrainTimeoutCounter - Counts the number of times tx_train performed and got timeout.
 *    txTrainOkCounter  - Counts the number of times tx_train performed and completed without fail.
 *    rxInitOk  - Counts the number of times rx_init performed and finished with rx_init_done.
 *    rxInitTimeOut  - Counts the number of times rx_init performed and finished with timeout.
 *    rxTrainFailed  - Counts the number of times rx_train performed and completed with fail status.
 *    rxTrainOk  - Counts the number of times rx_train performed and completed without fail.
 *    rxTrainTimeOut  - Counts the number of times rx_train performed and got timeout.

 * ANP SM information:
 *    chSmState  - Channel state machine.
 *    txSmState  - Tx state machine.
 *    rxSmState  - Rx state machine.
*/
/**
* @struct CPSS_ANP_INFO_STC
 *
 * @brief ANP counters .
*/

typedef struct
{
    /** @brief anRestartCounter  */
    GT_U32 anRestartCounter;
    /** @brief dspLockFailCounter  */
    GT_U32 dspLockFailCounter;
    /** @brief linkFailCounter  */
    GT_U32 linkFailCounter;
    /** @brief txTrainDuration  */
    GT_U32 txTrainDuration;
    /** @brief txTrainFailCounter  */
    GT_U32 txTrainFailCounter;
    /** @brief txTrainTimeoutCounter  */
    GT_U32 txTrainTimeoutCounter;
    /** @brief txTrainOkCounter  */
    GT_U32 txTrainOkCounter;
    /** @brief rxInitOk  */
    GT_U32 rxInitOk;
    /** @brief rxInitTimeOut  */
    GT_U32 rxInitTimeOut;
    /** @brief rxTrainFailed  */
    GT_U32 rxTrainFailed;
    /** @brief rxTrainOk  */
    GT_U32 rxTrainOk;
    /** @brief rxTrainTimeOut  */
    GT_U32 rxTrainTimeOut;

    /** @brief chSmState  */
    GT_U32 chSmState;
    /** @brief txSmState  */
    GT_U32 txSmState;
    /** @brief rxSmState  */
    GT_U32 rxSmState;

}CPSS_ANP_INFO_STC;

/**
* @internal genRegisterSet function
* @endinternal
*
* @brief   Writes the unmasked bits of a register.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP device number to write to.
* @param[in] portGroup                - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] address                  - Register  to write to.
* @param[in] data                     - Data to be written to register.
* @param[in] mask                     - Mask for selecting the written bits.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be written are the masked bit of 'mask'.
*
*/
GT_STATUS genRegisterSet
(
    IN GT_U8 devNum,
    IN GT_U32 portGroup,
    IN GT_U32 address,
    IN GT_U32 data,
    IN GT_U32 mask
);

/**
* @internal genRegisterGet function
* @endinternal
*
* @brief   Reads the unmasked bits of a register.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP device number to read from.
* @param[in] portGroup                - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] address                  - Register  to read from.
* @param[in] mask                     - Mask for selecting the read bits.
*
* @param[out] data                     - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be read are the masked bit of 'mask'.
*
*/
GT_STATUS genRegisterGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroup,
    IN  GT_U32  address,
    OUT GT_U32  *data,
    IN  GT_U32  mask
);

/**
* @enum CPSS_PORT_SERDES_LOOPBACK_MODE_ENT
 *
 * @brief Enumeration of port's serdes loopback modes.
*/
typedef enum{

    /** no any loopback on serdes */
    CPSS_PORT_SERDES_LOOPBACK_DISABLE_E,

    /** analog TX to RX loopback */
    CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E,

    /** digital TX to RX loopback */
    CPSS_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E,

    /** digital RX to TX loopback */
    CPSS_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E,

    /** enum type upper boarder */
    CPSS_PORT_SERDES_LOOPBACK_MAX_E

} CPSS_PORT_SERDES_LOOPBACK_MODE_ENT;

/**
* @enum CPSS_PORT_PCS_LOOPBACK_MODE_ENT
 *
 * @brief Enumeration of port's PCS loopback modes.
*/
typedef enum{

    /** no any loopback on PCS */
    CPSS_PORT_PCS_LOOPBACK_DISABLE_E,

    /** TX to RX loopback */
    CPSS_PORT_PCS_LOOPBACK_TX2RX_E,

    /** RX to TX loopback */
    CPSS_PORT_PCS_LOOPBACK_RX2TX_E

} CPSS_PORT_PCS_LOOPBACK_MODE_ENT;

/**
* @enum CPSS_PORT_PA_BW_MODE_ENT
 *
 * @brief Enumeration of the Pizza Arbiter Bandwidth Port modes.
*/
typedef enum{

    /** Regular Mode - the bandwidth is defined by port's speed. It is the default mode. */
    CPSS_PORT_PA_BW_MODE_REGULAR_E,

    /** Extra Mode1 ports have 147% bandwidth of port's speed (to provide 102G x 147% = 149.94 -> 150 slices) */
    CPSS_PORT_PA_BW_EXT_MODE_1_E,

    /** Extra Mode2 ports have 108% bandwidth of port's speed. (to provide 102G x 108% = 110.16 -> 110 slices) */
    CPSS_PORT_PA_BW_EXT_MODE_2_E,

    /** Extra Mode3 ports have 83% bandwidth of port's speed. (to provide 102G x 83% = 84.66 -> 85 slices) */
    CPSS_PORT_PA_BW_EXT_MODE_3_E,

    /** Extra Mode4 ports have 1% bandwidth of port's speed. (to provide 102G x 1% = 1.02 -> 1 slices) */
    CPSS_PORT_PA_BW_EXT_MODE_4_E,

    /** Extra Mode5 reserved for future use, Not Available now.*/
    CPSS_PORT_PA_BW_EXT_MODE_5_E,

    /** Extra Mode6 reserved for future use, Not Available now.*/
    CPSS_PORT_PA_BW_EXT_MODE_6_E,

    /** Extra Mode7 reserved for future use, Not Available now.*/
    CPSS_PORT_PA_BW_EXT_MODE_7_E,

    /** Extra Mode8 reserved for future use, Not Available now.*/
    CPSS_PORT_PA_BW_EXT_MODE_8_E,

    /** used as amount of enum elements, should be last */
    CPSS_PORT_PA_BW_MODE_LAST_E

} CPSS_PORT_PA_BW_MODE_ENT;


/**
* @internal prvCpssGenPortGroupSerdesWriteRegBitMask function
* @endinternal
*
* @brief   Implement write access to SERDES external/internal registers.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group number
* @param[in] regType                  - internal/external
* @param[in] serdesNum                - seredes number to access
* @param[in] regAddr                  - serdes register address (offset) to access
* @param[in] data                     -  to write
* @param[in] mask                     -  write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvCpssGenPortGroupSerdesWriteRegBitMask
(
    IN  GT_U8  devNum,
    IN  GT_U8  portGroup,
    IN  GT_U8  regType,
    IN  GT_U32 serdesNum,
    IN  GT_U32 regAddr,
    IN  GT_U32 data,
    IN  GT_U32 mask
);

/**
* @internal prvCpssGenPortGroupSerdesReadRegBitMask function
* @endinternal
*
* @brief   Implement read access from SERDES external/internal registers.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group number
* @param[in] regType                  - internal/external
* @param[in] serdesNum                - seredes number to access
* @param[in] regAddr                  - serdes register address (offset) to access
* @param[in] mask                     - read mask
*
* @param[out] dataPtr                  - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssGenPortGroupSerdesReadRegBitMask
(
    IN  GT_U8  devNum,
    IN  GT_U8  portGroup,
    IN  GT_U8  regType,
    IN  GT_U32 serdesNum,
    IN  GT_U32 regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32 mask
);



/**
* @internal prvCpssCommonPortIfModeToHwsTranslate function
* @endinternal
*
* @brief   Translate port interface mode and speed from CPSS enum to hwService enum
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum               - device number
* @param[in] cpssIfMode               - i/f mode in CPSS format
* @param[in] cpssSpeed                - port speed in CPSS format
*
* @param[out] hwsIfModePtr             - port i/f mode in format of hwServices library
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - i/f mode/speed pair not supported
*/
GT_STATUS prvCpssCommonPortIfModeToHwsTranslate
(  IN   GT_U8 devNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    cpssIfMode,
    IN  CPSS_PORT_SPEED_ENT             cpssSpeed,
    OUT MV_HWS_PORT_STANDARD            *hwsIfModePtr
);

/**
* @internal prvCpssCommonPortTraceDelay function
* @endinternal
*
* @brief   common to DX and PX : Make delay and trace it
*
* @param[in] devNum                   - PP device number
* @param[in] portGroupId              - ports group number
* @param[in] millisec                 -  the delay in millisec
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS prvCpssCommonPortTraceDelay
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_U32      millisec
);

/**
* @internal prvCpssCommonPortSpeedEnumToMbPerSecConvert function
* @endinternal
*
* @brief   Convert Port speed enum value to amount megabits per second
*
* @param[in] cpssSpeed                - port speed in CPSS format
*
* @retval Speed in megabits per second, 0 - for wrong values
*/
GT_U32 prvCpssCommonPortSpeedEnumToMbPerSecConvert
(
    IN  CPSS_PORT_SPEED_ENT             cpssSpeed
);

/**
* @internal prvCpssCommonPortModeEnumToSerdesUsageUnitsConvert function
* @endinternal
*
* @brief   Convert Port mode enum value to amount of serdes usage units
* @note    Serdes usage unit is 1/256 of one SerDes line - defined to support USX modes
*
* @param[in] portMode                - port mode in CPSS format
*
* @retval Amount of Serdes usage units occupated by the given mode
*/
GT_U32 prvCpssCommonPortModeEnumToSerdesUsageUnitsConvert
(
    IN  CPSS_PORT_INTERFACE_MODE_ENT      portMode
);

/**
* @internal prvCpssCommonPortUsxModeMaxSpeedGet function
* @endinternal
*
* @brief   Get maximal speed supported by USX mode
*
* @param[in] portMode                - port mode in CPSS format
*
* @retval Maximal speed in CPSS enum, CPSS_PORT_SPEED_NA_E for not USX modes.
*/
CPSS_PORT_SPEED_ENT prvCpssCommonPortUsxModeMaxSpeedGet
(
    IN  CPSS_PORT_INTERFACE_MODE_ENT      portMode
);

/**
* @internal prvCpssCommonPortSpeedEnumToHwsSpeedConvert function
* @endinternal
*
* @brief   Convert Port speed enum value to hws speed enum
*
* @param[in] cpssSpeed                - port speed in CPSS format
*
* @retval hws Speed enum, 0 - for wrong values
*/
MV_HWS_PORT_SPEED_ENT prvCpssCommonPortSpeedEnumToHwsSpeedConvert
(
    IN  CPSS_PORT_SPEED_ENT             cpssSpeed
);

/**
* @internal prvCpssCommonPortInterfaceSpeedGet function
* @endinternal
*
* @brief   Check if given pair ifMode and speed supported by given port on
*         given device
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*                                      portNum   - port MAC number (not CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @param[out] supportedPtr             ? GT_TRUE ? (ifMode; speed) supported
*                                      GT_FALSE ? (ifMode; speed) not supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if one of input parameters wrong
* @retval GT_BAD_PTR               - if supportedPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssCommonPortInterfaceSpeedGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portMacNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT GT_BOOL                         *supportedPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPortCtrl_h */


