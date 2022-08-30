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
* @file mvHwsPortInitIf.h
*
* @brief
*
* @version   18
********************************************************************************
*/

#ifndef __mvHwServicesPortIf_H
#define __mvHwServicesPortIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/labServices/port/gop/common/os/hwsEnv.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>


#ifndef CM3
#define MV_HWS_MAX_LANES_NUM_PER_PORT   10
#else
#if defined RAVEN_DEV_SUPPORT || defined FALCON_DEV_SUPPORT
#define MV_HWS_MAX_LANES_NUM_PER_PORT   8
#else
#define MV_HWS_MAX_LANES_NUM_PER_PORT   4
#endif
#endif

#define MV_HWS_AC5P_GOP_PORT_NUM_CNS    26
#define MV_HWS_AC5P_GOP_PORT_CPU_CNS    105

#define MV_HWS_AC5X_GOP_PORT_CPU_0_CNS  48
#define MV_HWS_AC5X_GOP_PORT_CPU_1_CNS  49

#define MV_HWS_HARRIER_GOP_PORT_NUM_CNS    16
#define MV_HWS_HARRIER_GOP_PORT_CPU_CNS    0xFFFFFFFF /* no such 'cpu port' */

/**
* @enum MV_HWS_PORT_STANDARD
 *
 * @brief Defines the different port standard metrics.
*/
typedef enum
{
/* 0  */ _100Base_FX,
/* 1  */ SGMII,
/* 2  */ _1000Base_X,
/* 3  */ SGMII2_5,
/* 4  */ QSGMII,
/* 5  */ _10GBase_KX4,
/* 6  */ _10GBase_KX2,   /* 10GBase-DHX */
/* 7  */ _10GBase_KR,
/* 8  */ _20GBase_KR2,
/* 9  */ _40GBase_KR,
/* 10 */ _100GBase_KR10,
/* 11 */ HGL,
/* 12 */ RHGL,
/* 13 */ CHGL,   /* CHGL_LR10 */
/* 14 */ RXAUI,
/* 15 */ _20GBase_KX4,   /* HS-XAUI */
/* 16 */ _10GBase_SR_LR,
/* 17 */ _20GBase_SR_LR2,
/* 18 */ _40GBase_SR_LR,
/* 19 */ _12_1GBase_KR,          /* Serdes speed: 12.5G;    Data speed: 12.1G */
/* 20 */ XLHGL_KR4,              /* 48G */
/* 21 */ HGL16G,
/* 22 */ HGS,
/* 23 */ HGS4,
/* 24 */ _100GBase_SR10,
/* 25 */ CHGL_LR12,
/* 26 */ TCAM,
/* 27 */ INTLKN_12Lanes_6_25G,
/* 28 */ INTLKN_16Lanes_6_25G,
/* 29 */ INTLKN_24Lanes_6_25G,
/* 30 */ INTLKN_12Lanes_10_3125G,
/* 31 */ INTLKN_16Lanes_10_3125G,
/* 32 */ INTLKN_12Lanes_12_5G,
/* 33 */ INTLKN_16Lanes_12_5G,
/* 34 */ INTLKN_16Lanes_3_125G,
/* 35 */ INTLKN_24Lanes_3_125G,
/* 36 */ CHGL11_LR12,

/* 37 */ INTLKN_4Lanes_3_125G,
/* 38 */ INTLKN_8Lanes_3_125G,
/* 39 */ INTLKN_4Lanes_6_25G,
/* 40 */ INTLKN_8Lanes_6_25G,

/* 41 */ _2_5GBase_QX,
/* 42 */ _5GBase_DQX,
/* 43 */ _5GBase_HX,
/* 44 */ _12GBaseR,              /* Serdes speed: 12.1875G;    Data speed: 1*11.8G */
/* 45 */ _5_625GBaseR,           /* Serdes speed:   5.625G;    Data speed: 1*5.45G*/
/* 46 */ _48GBaseR4,             /* Serdes speed: 12.1875G;    Data speed: 4*11.8G */
/* 47 */ _12GBase_SR,            /* Serdes speed: 12.1875G;    Data speed: 1*11.8G */
/* 48 */ _48GBase_SR4,           /* Serdes speed: 12.1875G;    Data speed: 4*11.8G */
/* 49 */ _5GBaseR,               /* Serdes speed: 5.15625G;    Data speed: 1*5G */
/* 50 */ _22GBase_SR,            /* Serdes speed: 11.5625G;    Data speed: 2*11.2G */
/* 51 */ _24GBase_KR2,           /* Serdes speed: 12.1875G;    Data speed: 2*11.8G */
/* 52 */ _12_5GBase_KR,          /* Serdes speed: 12.8906G;    Data speed: 1*12.5G */
/* 53 */ _25GBase_KR2,           /* Serdes speed: 12.8906G;    Data speed: 2*12.5G */
/* 54 */ _50GBase_KR4,           /* Serdes speed: 12.8906G;    Data speed: 4*12.5G */

/* 55 */ _25GBase_KR,            /* Serdes speed: 25.78125G;   Data speed: 1*25G */
/* 56 */ _50GBase_KR2,           /* Serdes speed: 25.78125G;   Data speed: 2*25G */
/* 57 */ _100GBase_KR4,          /* Serdes speed: 25.78125G;   Data speed: 4*25G */
/* 58 */ _25GBase_SR,            /* Serdes speed: 25.78125G;   Data speed: 1*25G */
/* 59 */ _50GBase_SR2,           /* Serdes speed: 25.78125G;   Data speed: 2*25G */
/* 60 */ _100GBase_SR4,          /* Serdes speed: 25.78125G;   Data speed: 4*25G */
/* 61 */ _100GBase_MLG,          /* Serdes speed: 25.78125G;   Data speed: 4*25G */
/* 62 */ _107GBase_KR4,          /* Serdes speed: 27.5G;       Data speed: 4*26.66G (106.6G)*/
/* 63 */ _25GBase_KR_C,          /* Consortium - Serdes speed: 25.78125G;   Data speed: 1*25G */
/* 64 */ _50GBase_KR2_C,         /* Consortium - Serdes speed: 25.78125G;   Data speed: 2*25G */
/* 65 */ _40GBase_KR2,           /* Serdes speed: 20.625G;     Data speed: 2*20G */
/* 66 */ _29_09GBase_SR4,        /* Serdes speed: 7.5G;        Data speed: 4*7.272G */

/* 67 */ _40GBase_CR4,
/* 68 */ _25GBase_CR,
/* 69 */ _50GBase_CR2,
/* 70 */ _100GBase_CR4,
/* 71 */ _25GBase_KR_S,
/* 72 */ _25GBase_CR_S,
/* 73 */ _25GBase_CR_C,
/* 74 */ _50GBase_CR2_C,

/* 75 */ _50GBase_KR,              /* Serdes speed: _26_5625G;   Data speed: 1*51.51G */
/* 76 */ _100GBase_KR2,            /* Serdes speed: _26_5625G;   Data speed: 2*51.51G */
/* 77 */ _200GBase_KR4,            /* Serdes speed: _26_5625G;   Data speed: 4*51.51G */
/* 78 */ _200GBase_KR8,            /* Serdes speed:  25.78125G;  Data speed: 8*25G */
/* 79 */ _400GBase_KR8,            /* Serdes speed: _26_5625G;   Data speed: 8*51.51G */

/* 80 */ _102GBase_KR4,            /* Serdes speed: _26.25G;     Data speed: 4*25G (102G)*/
/* 81 */ _52_5GBase_KR2,           /* Serdes speed: _27.1G;      Data speed: 2*26.28G (52.5G) */
/* 82 */ _40GBase_KR4,
/* 83 */ _26_7GBase_KR,            /* Serdes speed: 27.5G;       Data speed: 1*26.66G (26.66G) */
/* 84 */ _40GBase_SR_LR4,
/* 85 */ _50GBase_CR,
/* 86 */ _100GBase_CR2,
/* 87 */ _200GBase_CR4,
/* 88 */ _200GBase_CR8,
/* 89 */ _400GBase_CR8,
/* 90 */ _200GBase_SR_LR4,
/* 91 */ _200GBase_SR_LR8,
/* 92 */ _400GBase_SR_LR8,
/* 93 */ _50GBase_SR_LR,
/* 94 */ _100GBase_SR_LR2,
/* 95 */ _20GBase_KR,
/* 96 */ _106GBase_KR4,

    /* 1 MAC USX modes*/
/* 97 */ _2_5G_SXGMII,          /*Serdes speed: 2.578125G;Data Speed: 1 MAC  (10M..2.5G) */
/* 98 */ _5G_SXGMII,            /*Serdes speed: 5.15625G; Data Speed: 1 MAC  (10M..5G) */
/* 99 */ _10G_SXGMII,           /*Serdes speed: 10.3125G; Data Speed: 1 MAC  (10M..10G) */

    /* 2 MACs USX modes*/
/* 100*/ _5G_DXGMII,            /*Serdes speed: 5.15625G; Data Speed: 2 MACs (10M..2.5G) */
/* 101*/ _10G_DXGMII,           /*Serdes speed: 10.3125G; Data Speed: 2 MACs (10M..5G) */
/* 102*/ _20G_DXGMII,           /*Serdes speed: 20.625G;  Data Speed: 2 MACs (10M..10G) */

    /* 4 MACs USX modes*/
/* 103*/ _5G_QUSGMII,           /*Serdes speed: 5G;       Data Speed: 4 MACs (10M..1G) */
/* 104*/ _10G_QXGMII,           /*Serdes speed: 10.3125G; Data Speed: 4 MACs (10M..2.5G) */
/* 105*/ _20G_QXGMII,           /*Serdes speed: 20.625G;  Data Speed: 4 MACs (10M..5G) */

    /* 8 MACs USX modes*/
/* 106*/ _10G_OUSGMII,          /*Serdes speed: 10G;      Data Speed: 8 MACs (10M..1G) */
/* 107*/ _20G_OXGMII,           /*Serdes speed: 20.625G;  Data Speed: 8 MACs (10M..2.5G) */

/* 108*/ _42GBase_KR4,          /*serdes speed: 10.9375G  Data Speed: 4 MAC  */
/* 109*/ _53GBase_KR2,          /*serdes speed: 27.34375G*/
/* 110*/ _424GBase_KR8,         /*serdes speed: _28_125G_PAM4;   Data speed: 8*54.54G */
/* 111*/ _2500Base_X,           /**/
/* 112*/ _212GBase_KR4,         /*Serdes speed: _28_125G_PAM4;   Data speed: 4*54.54G */
/* 113*/ _106GBase_KR2,         /*Serdes speed: _28_125G_PAM4;   Data speed: 2*54.54G */
/* 114*/ _102GBase_KR2,         /*Serdes speed: _27_1875_PAM4;   Data speed: 2*52.72G */

    NON_SUP_MODE,
    LAST_PORT_MODE = NON_SUP_MODE

}MV_HWS_PORT_STANDARD;

/* Macro for two lanes speeds/port modes - to ease the use. */
#define HWS_TWO_LANES_MODE_CHECK(_mode) ((_mode == _50GBase_KR2) || (_mode == _50GBase_KR2_C) || (_mode == _40GBase_KR2) || \
                                         (_mode == _50GBase_CR2) || (_mode == _50GBase_CR2_C) || (_mode == _50GBase_SR2) || \
                                         (_mode == _52_5GBase_KR2) || (_mode == _53GBase_KR2))

#define HWS_10G_MODE_CHECK(_mode) ((_mode == _10GBase_KR) || (_mode == _10GBase_SR_LR))
/* since MAC/PCS of 20GBase_KR operates in configuration of 25G - include 20G R1 in this macro */
#define HWS_25G_MODE_CHECK(_mode) ((_mode == _25GBase_KR_S) || (_mode == _25GBase_KR) || (_mode == _25GBase_KR_C) || \
                                   (_mode == _25GBase_CR_C) || (_mode == _25GBase_CR_S) || (_mode == _25GBase_CR) || \
                                   (_mode == _25GBase_SR) || (_mode == _20GBase_KR))

#define HWS_40G_R4_MODE_CHECK(_mode) ((_mode == _40GBase_KR4) || (_mode == _40GBase_CR4) || \
                                      (_mode == _40GBase_SR_LR4) || (_mode == _42GBase_KR4) || (_mode == _48GBaseR4) || (_mode == _48GBase_SR4))

#define HWS_100G_R4_MODE_CHECK(_mode) ((_mode == _100GBase_KR4) || (_mode == _100GBase_CR4) || (_mode == _100GBase_SR4) || \
                                       (_mode == _106GBase_KR4) || (_mode == _107GBase_KR4) || (_mode == _102GBase_KR4))

#define HWS_100G_R2_MODE_CHECK(_mode) ((_mode == _100GBase_KR2) || (_mode == _102GBase_KR2) || (_mode == _106GBase_KR2) || (_mode == _100GBase_CR2) || (_mode == _100GBase_SR_LR2))

#define HWS_PAM4_MODE_CHECK(_mode) ((_mode == _50GBase_KR) || (_mode == _40GBase_KR) || (_mode == _100GBase_KR2) || \
                                    (_mode == _200GBase_KR4) || (_mode == _400GBase_KR8) || (_mode == _50GBase_CR) || \
                                    (_mode == _100GBase_CR2) || (_mode == _200GBase_CR4) || (_mode == _400GBase_CR8) || \
                                    (_mode == _200GBase_SR_LR4) ||(_mode == _400GBase_SR_LR8) || (_mode == _50GBase_SR_LR) || \
                                    (_mode == _100GBase_SR_LR2) || (_mode == _424GBase_KR8) || (_mode == _212GBase_KR4) || \
                                    (_mode == _106GBase_KR2) || (_mode == _102GBase_KR2))

#define HWS_25G_SERDES_MODE_CHECK(_mode) \
                                    (HWS_25G_MODE_CHECK(_mode)|| HWS_TWO_LANES_MODE_CHECK(_mode) || HWS_100G_R4_MODE_CHECK(_mode))


#if 0
/* new HAWK modes: check if port use USX MAC*/
#define HWS_USX_MODE_CHECK(_devNum, _portNum, _mode) \
                                    ((((HWS_DEV_SILICON_TYPE(_devNum) == AC5P) || ((HWS_DEV_SILICON_TYPE(_devNum) == AC5X) && (_portNum < MV_HWS_AC5X_GOP_PORT_CPU_0_CNS))) || \
                                    ((_mode == _10G_SXGMII) || (_mode == _5G_SXGMII) || (_mode == _10G_DXGMII)|| \
                                    (_mode == _5G_DXGMII) || (_mode == _20G_QXGMII)  || (_mode == _20G_DXGMII)|| \
                                    (_mode == _2_5G_SXGMII) || (_mode == _10G_QXGMII)  || (_mode == _20G_OXGMII)|| \
                                    (_mode == _5G_QUSGMII) || (_mode == _10G_OUSGMII) || ((_mode == QSGMII) && HWS_DEV_SIP_6_10_CHECK_MAC(_devNum) ))) && \
                                    (HWS_DEV_SILICON_TYPE(_devNum) != Harrier) && (HWS_DEV_SILICON_TYPE(_devNum) != AC5))
#endif

#define HWS_USX_S_MODE_CHECK(_mode) ((_mode == _5G_SXGMII) || (_mode == _10G_SXGMII) || (_mode == _2_5G_SXGMII))

#define HWS_USX_D_MODE_CHECK(_mode) ((_mode == _5G_DXGMII) || (_mode == _10G_DXGMII) || (_mode == _20G_DXGMII))

#define HWS_USX_Q_MODE_CHECK(_mode) ((_mode == _5G_QUSGMII) || (_mode == _10G_QXGMII) || (_mode == _20G_QXGMII) || (_mode == QSGMII))

/* HAWK Octal modes: use only one SERDES*/
#define HWS_USX_O_MODE_CHECK(_mode) ((_mode == _10G_OUSGMII) || (_mode == _20G_OXGMII))

#define HWS_USX_PORT_MODE_FAMILY_CHECK(_mode) (HWS_USX_O_MODE_CHECK(portMode) || HWS_USX_Q_MODE_CHECK(portMode) || HWS_USX_D_MODE_CHECK(portMode) || HWS_USX_S_MODE_CHECK(portMode))

#define HWS_USX_MULTI_LANE_MODE_CHECK(_mode) (HWS_USX_O_MODE_CHECK(_mode) || HWS_USX_Q_MODE_CHECK(_mode) || HWS_USX_D_MODE_CHECK(_mode))

#define HWS_USX_SLOW_MODE_CHECK(_mode) ((_mode == SGMII) || (_mode == QSGMII) || (_mode == _5G_QUSGMII)|| \
                                        (_mode == _10G_OUSGMII) || (_mode == _1000Base_X) || (_mode == SGMII2_5)|| (_mode == _2500Base_X))

#define PRV_CPSS_OPTICAL_PORT_MODE_MAC(_portMode) \
        (((_portMode == _10GBase_SR_LR) || (_portMode == _40GBase_SR_LR4) || (_portMode == _40GBase_SR_LR)) || \
         ((_portMode == _25GBase_SR) || (_portMode == _100GBase_SR4)  || (_portMode == _50GBase_SR2)) || \
         ((_portMode == _200GBase_SR_LR4) || (_portMode == _400GBase_SR_LR8) || (_portMode == _50GBase_SR_LR) || (_portMode == _100GBase_SR_LR2)))

#define HWS_AP_MODE_CHECK(_mode) ((_mode == _1000Base_X)   || (_mode == _10GBase_KR)   || \
                                  (_mode == _25GBase_KR_S) || (_mode == _25GBase_KR)   || (_mode == _25GBase_KR_C) || (_mode == _25GBase_CR_C) || (_mode == _25GBase_CR_S) || (_mode == _25GBase_CR) || \
                                  (_mode == _40GBase_KR4)  || (_mode == _40GBase_CR4)  || (_mode == _40GBase_KR2)  ||\
                                  (_mode == _50GBase_KR2_C)|| (_mode == _50GBase_CR2_C)|| (_mode == _50GBase_KR)   || (_mode == _50GBase_CR)   || \
                                  (_mode == _100GBase_KR4) || (_mode == _100GBase_CR4) || (_mode == _100GBase_KR2) || (_mode == _100GBase_CR2) || \
                                  (_mode == _200GBase_KR4) || (_mode == _200GBase_CR4) || (_mode == _200GBase_KR8) || (_mode == _200GBase_CR8) ||\
                                  (_mode == _400GBase_KR8) || (_mode == _400GBase_CR8))


#define HWS_10G_MODE_CASE \
             _10GBase_KR: \
        case _10GBase_SR_LR

/* since MAC/PCS of 20GBase_KR operates in configuration of 25G - include 20G R1 in this macro */
#define HWS_25G_MODE_CASE       \
             _25GBase_KR_S:     \
        case _25GBase_KR:       \
        case _25GBase_KR_C:     \
        case _25GBase_CR_C:     \
        case _25GBase_CR_S:     \
        case _25GBase_CR:       \
        case _25GBase_SR:       \
        case _20GBase_KR

#define HWS_40G_R4_MODE_CASE    \
             _40GBase_KR4:      \
        case _40GBase_CR4:      \
        case _40GBase_SR_LR4:   \
        case _42GBase_KR4:      \
        case _48GBaseR4:        \
        case _48GBase_SR4

#define HWS_40G_R2_MODE_CASE   \
             _40GBase_KR2

#define HWS_50G_R2_MODE_CASE   \
             _50GBase_KR2:      \
        case _50GBase_CR2:      \
        case _50GBase_KR2_C:    \
        case _50GBase_CR2_C:    \
        case _53GBase_KR2:      \
        case _52_5GBase_KR2:    \
        case _50GBase_SR2

#define HWS_100G_R4_MODE_CASE   \
             _100GBase_KR4:     \
        case _100GBase_CR4:     \
        case _100GBase_SR4:     \
        case _106GBase_KR4:     \
        case _107GBase_KR4:     \
        case _102GBase_KR4

#define HWS_200G_R8_MODE_CASE   \
             _200GBase_CR8:     \
        case _200GBase_SR_LR8:  \
        case _200GBase_KR8

#define HWS_50G_R1_MODE_CASE      \
             _50GBase_KR:       \
        case _50GBase_SR_LR:    \
        case _50GBase_CR        \

#define HWS_40G_R1_MODE_CASE      \
             _40GBase_SR_LR:    \
        case _40GBase_KR

#define HWS_50G_PAM4_MODE_CASE      \
             _50GBase_KR:       \
        case _40GBase_KR:       \
        case _50GBase_SR_LR:    \
        case _50GBase_CR        \

#define HWS_100G_PAM4_MODE_CASE      \
             _100GBase_KR2:     \
        case _100GBase_CR2:     \
        case _102GBase_KR2:     \
        case _100GBase_SR_LR2:  \
        case _106GBase_KR2

#define HWS_200G_PAM4_MODE_CASE      \
             _200GBase_KR4:     \
        case _200GBase_CR4:     \
        case _200GBase_SR_LR4:  \
        case _212GBase_KR4

#define HWS_400G_PAM4_MODE_CASE      \
             _400GBase_KR8:     \
        case _400GBase_CR8:     \
        case _400GBase_SR_LR8:  \
        case _424GBase_KR8

#define HWS_PAM4_MODE_CASE      \
             HWS_50G_PAM4_MODE_CASE:      \
        case HWS_100G_PAM4_MODE_CASE:     \
        case HWS_200G_PAM4_MODE_CASE:     \
        case HWS_400G_PAM4_MODE_CASE


#define HWS_D_USX_MODE_CASE     \
             _20G_DXGMII:       \
        case _10G_DXGMII:       \
        case _5G_DXGMII

#define HWS_Q_USX_MODE_CASE     \
             _10G_QXGMII:       \
        case _20G_QXGMII:       \
        case _20G_OXGMII

#define HWS_SX_MODE_CASE    \
             _2_5G_SXGMII:  \
        case _5G_SXGMII:    \
        case _10G_SXGMII

/**
* @enum MV_HWS_PORT_MII_TYPE_E
 *
 * @brief Defines the different port standard metrics.
*/
typedef enum
{
 /*0*/  _GMII_E,
 /*1*/  _5GMII_E,
 /*2*/  _10GMII_E,
 /*3*/  _25GMII_E,
 /*4*/  _40GMII_E,
 /*5*/  _50GMII_E,
 /*6*/  _100GMII_E,
 /*7*/  _200GMII_E,
 /*8*/  _400GMII_E,
 /*9*/  LAST_MII_TYPE_E

}MV_HWS_PORT_MII_TYPE_E;

#define MV_HWS_PORT_MII_TYPE_GET(_mode, _type)  \
    switch(_mode) {                             \
        case SGMII:                             \
        case SGMII2_5:                          \
        case _1000Base_X:                       \
        case _2500Base_X:                       \
        case QSGMII:                            \
        case _5G_QUSGMII:                       \
        case _10G_OUSGMII:                      \
            _type = _GMII_E;                    \
            break;                              \
        case _5GBaseR:                          \
            _type = _5GMII_E;                   \
            break;                              \
        case _10GBase_KR:                       \
        case _10GBase_SR_LR:                    \
        case _2_5G_SXGMII:                      \
        case _5G_SXGMII:                        \
        case _10G_SXGMII:                       \
        case _10G_QXGMII:                       \
        case _20G_QXGMII:                       \
        case _20G_OXGMII:                       \
        case _5G_DXGMII:                        \
        case _10G_DXGMII:                       \
        case _20G_DXGMII:                       \
            _type = _10GMII_E;                  \
            break;                              \
        case _25GBase_KR:                       \
        case _25GBase_CR:                       \
        case _25GBase_KR_C:                     \
        case _25GBase_CR_C:                     \
        case _25GBase_KR_S:                     \
        case _25GBase_CR_S:                     \
        case _25GBase_SR:                       \
            _type = _25GMII_E;                  \
            break;                              \
        case _40GBase_SR_LR4:                   \
        case _40GBase_KR4:                      \
        case _40GBase_CR4:                      \
        case _40GBase_KR2:                      \
        case _40GBase_KR:                       \
        case _40GBase_SR_LR:                    \
            _type = _40GMII_E;                  \
            break;                              \
        case _50GBase_KR:                       \
        case _50GBase_CR:                       \
        case _50GBase_SR_LR:                    \
        case _50GBase_KR2:                      \
        case _50GBase_CR2:                      \
        case _50GBase_KR2_C:                    \
        case _50GBase_CR2_C:                    \
        case _50GBase_SR2:                      \
            _type = _50GMII_E;                  \
            break;                              \
        case _100GBase_KR2:                     \
        case _102GBase_KR2:                     \
        case _100GBase_CR2:                     \
        case _106GBase_KR2:                     \
        case _100GBase_SR_LR2:                  \
        case _100GBase_KR4:                     \
        case _100GBase_CR4:                     \
        case _107GBase_KR4:                     \
        case _100GBase_SR4:                     \
            _type = _100GMII_E;                 \
            break;                              \
        case _200GBase_CR4:                     \
        case _200GBase_CR8:                     \
        case _200GBase_SR_LR4:                  \
        case _200GBase_SR_LR8:                  \
        case _200GBase_KR4:                     \
        case _212GBase_KR4:                     \
        case _200GBase_KR8:                     \
            _type = _200GMII_E;                 \
            break;                              \
        case _400GBase_CR8:                     \
        case _400GBase_KR8:                     \
        case _400GBase_SR_LR8:                  \
        case _424GBase_KR8:                     \
            _type = _400GMII_E;                 \
            break;                              \
        default:                                \
            _type = LAST_MII_TYPE_E;            \
}

/**
* @enum MV_HWS_REF_CLOCK_SOURCE
 *
 * @brief Defines the supported reference clock source.
*/
typedef enum{

    PRIMARY_LINE_SRC,

    SECONDARY_LINE_SRC

} MV_HWS_REF_CLOCK_SOURCE;

/**
* @enum MV_HWS_REF_CLOCK_SUP_VAL
 *
 * @brief Defines the supported reference clock.
*/
typedef enum
{
  MHz_156,
  MHz_78,
  MHz_25,
  MHz_125,
  MHz_312,
  MHz_164
}MV_HWS_REF_CLOCK_SUP_VAL; /* need to be synchronized with MV_HWS_REF_CLOCK enum */

/**
* @enum MV_HWS_PORT_ACTION
 *
 * @brief Defines different actions during port delete.
*/
typedef enum{

    PORT_POWER_DOWN,

    PORT_RESET,

    PORT_PARTIAL_POWER_DOWN

} MV_HWS_PORT_ACTION;

/**
* @enum MV_HWS_PORT_INIT_FLAVOR
 *
 * @brief Defines different suites of port init process:
 * PORT_AP_INIT -   run AP on current port
 * PORT_REGULAR_INIT - regular port create flow (no SERDES training)
 * PORT_RX_TRAIN_INIT - create port and run RX training on all SERDESes
 * PORT_TRX_TRAIN_INIT - create port and run TRX training on all SERDESes
*/
typedef enum{

    PORT_AP_INIT,

    PORT_REGULAR_INIT,

    PORT_RX_TRAIN_INIT,

    PORT_TRX_TRAIN_INIT

} MV_HWS_PORT_INIT_FLAVOR;

/**
* @enum MV_HWS_RESET
 *
 * @brief Defines reset types to set valid sequence
*/
typedef enum{

    RESET,

    UNRESET,

    POWER_DOWN,

    PARTIAL_POWER_DOWN          /* used to perform partial power down sequence:
                                   return to default values only per-port register,
                                   while common configurations (per 2/4/8 ports) still
                                   should serve other ports in group */

} MV_HWS_RESET;

/**
* @enum MV_HWS_CPLL_NUMBER
 *
 * @brief Defines CPLL number
*/
typedef enum
{
    CPLL0,
    CPLL1,
    CPLL2,
    MV_HWS_MAX_CPLL_NUMBER

}MV_HWS_CPLL_NUMBER;

/**
* @enum MV_HWS_CPLL_INPUT_FREQUENCY
 *
 * @brief Defines CPLL input freq
*/
typedef enum
{
    MV_HWS_25MHzIN,
    MV_HWS_156MHz_IN,
    MV_HWS_MAX_INPUT_FREQUENCY

}MV_HWS_CPLL_INPUT_FREQUENCY;

/**
* @enum MV_HWS_CPLL_OUTPUT_FREQUENCY
 *
 * @brief Defines CPLL output freq
*/
typedef enum
{
    MV_HWS_156MHz_OUT,          /* 156.25   Mhz */
    MV_HWS_78MHz_OUT,           /* 78.125   Mhz */
    MV_HWS_200MHz_OUT,          /* 200      Mhz */
    MV_HWS_312MHz_OUT,          /* 312.5    Mhz */
    MV_HWS_161MHz_OUT,          /* 160.9697 Mhz */
    MV_HWS_164MHz_OUT,          /* 164.2424 Mhz */
    MV_HWS_MAX_OUTPUT_FREQUENCY

}MV_HWS_CPLL_OUTPUT_FREQUENCY;

/**
* @enum MV_HWS_INPUT_SRC_CLOCK
 *
 * @brief Defines Source clock
*/
typedef enum
{
    MV_HWS_ANA_GRP0,
    MV_HWS_ANA_GRP1,
    MV_HWS_CPLL,
    MV_HWS_RCVR_CLK_IN,
    MV_HWS_MAX_INPUT_SRC_CLOCK

}MV_HWS_INPUT_SRC_CLOCK;

/**
* @struct MV_HWS_PORT_SERDES_TO_MAC_MUX
*
* @brief Defines structure for SERDES Muxing.
*/
typedef struct
{
    GT_BOOL enableSerdesMuxing;
    GT_U8   serdesLanes[MV_HWS_MAX_LANES_NUM_PER_PORT];
}MV_HWS_PORT_SERDES_TO_MAC_MUX;

/**
* @struct MV_HWS_PORT_INIT_INPUT_PARAMS
*
* @brief Defines structure for port init parameters.
*/
typedef struct
{
    GT_BOOL                  lbPort;
    MV_HWS_REF_CLOCK_SUP_VAL refClock;
    MV_HWS_REF_CLOCK_SOURCE  refClockSource;
    MV_HWS_PORT_FEC_MODE     portFecMode;
    MV_HWS_PORT_SPEED_ENT    portSpeed;
    GT_BOOL                  isPreemptionEnabled;
    GT_BOOL                  apEnabled;
    GT_BOOL                  autoNegEnabled;
}MV_HWS_PORT_INIT_INPUT_PARAMS;

/**
* @struct MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS
*
* @brief Defines structure for port attributes parameters.
*/
typedef struct
{
    GT_BOOL                  preemptionEnable;
    MV_HWS_PORT_SPEED_ENT    portSpeed;
}MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS;


/* is port 200G? */
#define HWS_IS_PORT_MODE_200G(portMode)     ((portMode == _200GBase_KR4)|| (portMode == _212GBase_KR4) || (portMode == _200GBase_KR8)  || \
                                             (portMode == _200GBase_CR4)|| (portMode == _200GBase_CR8)  || \
                                             (portMode == _200GBase_SR_LR4) || (portMode == _200GBase_SR_LR8))

/* is port 400G ?*/
#define HWS_IS_PORT_MODE_400G(portMode)     ((portMode == _400GBase_KR8)   || \
                                             (portMode == _400GBase_CR8)   || \
                                             (portMode == _400GBase_SR_LR8)|| \
                                             (portMode == _424GBase_KR8))
/* is port segmented ? speed 200/400 */
#define HWS_IS_PORT_MULTI_SEGMENT(portMode)     (HWS_IS_PORT_MODE_200G(portMode) || \
                                                 HWS_IS_PORT_MODE_400G(portMode))


#define HWS_FALCON_3_2_RAVEN_NUM_CNS  4
#define HWS_FALCON_6_4_RAVEN_NUM_CNS  8
#define HWS_FALCON_12_8_RAVEN_NUM_CNS  16

/**
* @internal mvHwsUsxModeCheck function
* @endinternal
*
* @brief  Checks if port is USX.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_BOOL mvHwsUsxModeCheck
(
    IN GT_U8                           devNum,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode
);

/**
* @internal mvHwsIsReducedPort function
* @endinternal
*
* @brief  Checks if port is CPU/reduced for Falcon.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_BOOL mvHwsMtipIsReducedPort
(
    GT_U8   devNum,
    GT_U32  phyPortNum
);

GT_U32 mvHwsPortToD2DIndexConvert
(
    GT_U8   devNum,
    GT_U32  phyPortNum
);

GT_U32 mvHwsPortToD2dChannelConvert
(
    GT_U8   devNum,
    GT_U32  phyPortNum
);

GT_BOOL multiLaneConfigSupport(GT_BOOL enable);

/**
* @internal mvHwsPortInit function
* @endinternal
*
* @brief   Init physical port. Configures the port mode and all it's elements
*         accordingly.
*         Does not verify that the selected mode/port number is valid at the
*         core level.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portInitInParamPtr       - port init parameters,
*                                       this function using:
*                                       lbPort - if true, init port without serdes activity
*                                       refClock - Reference clock frequency
*                                       refClock - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortInit
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
);

/**
* @internal mvHwsApPortInit function
* @endinternal
*
* @brief   Init physical port for 802.3AP protocol. Configures the port mode and all it's elements
*         accordingly.
*         Does not verify that the selected mode/port number is valid at the
*         core level.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portInitInParamPtr       - port parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApPortInit
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
);

#ifdef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsRavenApPortInitPhase function
* @endinternal
*
* @brief  Only for RAVEN (cm3) Init physical port for 802.3AP
*         protocol is done in 2 stages. This function is doing
*         the second phase.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portInitInParamPtr       - port parameters
* @param[in] phase                    - port power up phase
*       line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsRavenApPortInitPhase
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr,
    GT_U8   phase,
    MV_HWS_SERDES_CONFIG_STC        *serdesConfigPtr
);
#endif

/**
* @internal mvHwsPortFlavorInit function
* @endinternal
*
* @brief   Init physical port. Configures the port mode and all it's elements
*         accordingly to specified flavor.
*         Does not verify that the selected mode/port number is valid at the
*         core level.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portInitInParamPtr       - port init parameters,
*                                       this function using:
*                                       lbPort - if true, init port without serdes activity
*                                       refClock - Reference clock frequency
*                                       refClock - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFlavorInit
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
);

/**
* @internal mvHwsPortReset function
* @endinternal
*
* @brief   Clears the port mode and release all its resources according to selected.
*         Does not verify that the selected mode/port number is valid at the core
*         level and actual terminated mode.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] action                   - Power down or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ACTION      action
);

/**
* @internal mvHwsPortValidate function
* @endinternal
*
* @brief   Validate port API's input parameters
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortValidate
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsPortLoopbackValidate function
* @endinternal
*
* @brief   Validate loopback port input parameters.
*         In MMPCS mode: the ref_clk comes from ExtPLL, thus the Serdes can be in power-down.
*         In all other PCS modes: there is no ExtPLL, thus the ref_clk comes is taken
*         from Serdes, so the Serdes should be in power-up.
* @param[in] portPcsType              - port pcs type
* @param[in] lbPort                   - if true, init port without serdes activity
* @param[in] serdesInit               - if true, init port serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLoopbackValidate
(
    GT_U32  portPcsType,
    GT_BOOL lbPort,
    GT_BOOL *serdesInit
);

/**
* @internal mvHwsPortSerdesPowerUp function
* @endinternal
*
* @brief   power up the serdes lanes
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
* @param[in] curLanesList             - active Serdes lanes list according to configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSerdesPowerUp
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_REF_CLOCK_SUP_VAL    refClock,
    MV_HWS_REF_CLOCK_SOURCE refClockSource,
    GT_U32 *curLanesList
);

/**
* @internal mvHwsPortApSerdesPowerUp function
* @endinternal
*
* @brief   power up the serdes lanes for 802.3AP protocol.
*         assumes: parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
* @param[in] curLanesList             - active Serdes lanes list according to configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSerdesPowerUp
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD      portMode,
    MV_HWS_REF_CLOCK_SUP_VAL  refClock,
    MV_HWS_REF_CLOCK_SOURCE   refClockSource,
    GT_U32                    *curLanesList
);

/**
* @internal mvHwsPortModeSquelchCfg function
* @endinternal
*
* @brief   reconfigure default squelch threshold value only for KR (CR) modes
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] curLanesList             - active Serdes lanes list according to configuration
* @param[in] txAmp                    - Reference clock frequency
* @param[in] emph0                    - Reference clock source line
* @param[in] emph1                    - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortModeSquelchCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32 *curLanesList
);

/**
* @internal mvHwsPortModeCfg function
* @endinternal
*
* @brief   configures MAC and PCS components
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] attributesPtr            - port attributes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortModeCfg
(
    IN GT_U8                               devNum,
    IN GT_U32                              portGroup,
    IN GT_U32                              phyPortNum,
    IN MV_HWS_PORT_STANDARD                portMode,
    IN MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
);

/**
* @internal mvHwsPortStartCfg function
* @endinternal
*
* @brief   Unreset MAC and PCS components
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortStartCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal mvHwsPortStopCfg function
* @endinternal
*
* @brief   Reset MAC and PCS components
*         Port power down on each related serdes
*         assumes .parameters validation in the calling function
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] action                   - port power down or reset
* @param[in] curLanesList             - active Serdes lanes list according to configuration
*                                      reset pcs        - reset pcs option
*                                      reset mac        - reset mac option
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortStopCfg
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_PORT_ACTION action,
    GT_U32 *curLanesList,
    MV_HWS_RESET reset_pcs,
    MV_HWS_RESET reset_mac
);

/**
* @internal mvHwsPortAccessCheck function
* @endinternal
*
* @brief   Validate port access
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[out] accessEnabledPtr        - (pointer to) port access enabled/disabled
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAccessCheck
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *accessEnabledPtr
);

#ifdef __cplusplus
}
#endif

#endif /* mvHwServicesPortIf_H */



