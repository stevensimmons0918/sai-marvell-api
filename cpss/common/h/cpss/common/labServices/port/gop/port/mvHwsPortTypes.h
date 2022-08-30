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
* @file mvHwsPortTypes.h
*
* @brief Ln2CpuPortunit registers and tables classes declaration.
*
* @version   15
********************************************************************************
*/

#ifndef __mvHwsPortTypes_H
#define __mvHwsPortTypes_H

#ifndef MV_HWS_FREE_RTOS
#include <cpss/generic/cpssHwInfo.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#endif

/* General H Files */
/**
* @enum MV_HWS_PORT_MAC_TYPE
 *
 * @brief Defines the different port MAC types.
*/
typedef enum
{
/*0*/   MAC_NA,
/*1*/   GEMAC_X,
/*2*/   GEMAC_SG,
/*3*/   XGMAC,
/*4*/   XLGMAC,
/*5*/   HGLMAC,
/*6*/   CGMAC,
/*7*/   INTLKN_MAC,
/*8*/   TCAM_MAC,

/*9 */  GEMAC_NET_X,
/*10*/  GEMAC_NET_SG,

/*11*/  QSGMII_MAC,

/*12*/  D2D_MAC,

/*13*/  MTI_MAC_100,        /*  MAC for 1G to 100G ports.
                                APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman */
/*14*/  MTI_MAC_400,        /*  MAC for 200G/400G ports.
                                APPLICABLE DEVICES: Falcon; AC5P */
/*15*/  MTI_CPU_MAC,        /*  reduced/CPU MAC for 10G/25G port (port 17 in Raven).
                                APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman */
/*16*/  MTI_USX_MAC,        /*  USX MAC for 10M/100M/1G/2.5G/5G/10G/25G port
                                APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman */
/*17*/  MTI_MAC_100_BR,     /*  MAC with preemption feature for 1G to 100G ports
                                APPLICABLE DEVICES: AC5P */

    LAST_MAC

}MV_HWS_PORT_MAC_TYPE;

/**
* @enum MV_HWS_PORT_PCS_TYPE
*
* @brief Defines the different port PCS types.
*/
typedef enum
{
    PCS_NA,
    GPCS,
    XPCS,
    MMPCS,
    HGLPCS,
    CGPCS,
    INTLKN_PCS,
    TCAM_PCS,

    GPCS_NET,

    QSGMII_PCS,

    D2D_PCS,

    MTI_PCS_50,         /*  PCS for 10G to 50G odd ports.
                            APPLICABLE DEVICES: Falcon; AC5P, AC5X, Harrier, Ironman  */
    MTI_PCS_100,        /*  PCS for 10G to 100G even ports.
                            APPLICABLE DEVICES: Falcon; AC5P, AC5X, Harrier, Ironman  */
    MTI_PCS_200,        /*  PCS for 200G even ports.
                            APPLICABLE DEVICES: Falcon; AC5P */
    MTI_PCS_400,        /*  PCS for 400G port.
                            APPLICABLE DEVICES: Falcon; AC5P, AC5X, Harrier, Ironman */
    MTI_PCS_LOW_SPEED,  /*  Low speed PCS for 1G, 2.5G ports.
                            APPLICABLE DEVICES: Falcon; AC5P, AC5X, Harrier, Ironman */
    MTI_CPU_PCS,        /*  Reduced/CPU 10G/25G port.
                            APPLICABLE DEVICES: Falcon; AC5P, AC5X, Harrier, Ironman */
    MTI_USX_PCS,        /*  PCS for USX port.
                            APPLICABLE DEVICES: AC5P, AC5X, Harrier, Ironman */
    MTI_USX_PCS_LOW_SPEED,/* Low speed PCS for USX ports.
                            APPLICABLE DEVICES: AC5P, AC5X, Harrier, Ironman */
    MTI_PCS_25,         /*  PCS for 10G to 25G odd ports.
                            APPLICABLE DEVICES: AC5X; Harrier; Ironman */

    MTI_CPU_SGPCS,      /*  Reduced/CPU  for 1G to 2.5G ports.
                            APPLICABLE DEVICES: AC5X; Harrier; Ironman, AC5P */

    LAST_PCS

}MV_HWS_PORT_PCS_TYPE;

/**
* @enum MV_HWS_SERDES_SPEED
*
* @brief Defines the different values of serdes speed.
*/
typedef enum
{
    SPEED_NA,
    _1_25G,
    _3_125G,
    _3_33G,
    _3_75G,
    _4_25G,
    _5G,
    _6_25G,
    _7_5G,
    _10_3125G,          /* for 10G, 20G, 40G */
    _11_25G,
    _11_5625G,
    _12_5G,
    _10_9375G,          /* for 42G */
    _12_1875G,          /* for 11.8G, 23.6G, 47.2G */
    _5_625G,
    _5_15625G,          /* for 5G */
    _12_8906G,          /* for 12.5G, 25G, 50G */
    _20_625G,           /* for 40G R2 */
    _25_78125G,         /* for 25G, 50G, 100G */
    _27_5G,             /* for 106G */
    _28_05G,
    _26_5625G,          /* for NRZ mode at 26.5625G,  200R8  */
    _28_28125G,         /* for NRZ mode at 28.28125G, for PAM4 mode at 56.5625G */
    _26_5625G_PAM4,     /* for PAM4 mode at 53.125G  */
    _28_125G_PAM4,      /* for NRZ mode at 28.125G, for PAM4 mode at 56.25G */
    _26_25G,            /* for 102G */
    _27_1G,             /* for 52.5G */
    _10_3125G_SR_LR,    /*for 10G optics , 40G optics */
    _25_78125G_SR_LR,   /* for 25G optics , 50G optics , 100G optics */
    _26_5625G_PAM4_SR_LR, /* Optics -  for PAM4 mode at 53.125G  */
    _27_34375G,         /* for 53G, 106G  */
    _10G,               /* for USX mode OUSGMII */
    _2_578125,          /* for USX mode _2.5G_SXGMII */
    _1_25G_SR_LR,
    _26_5625G_SR_LR,    /* Optics - for NRZ mode at 26.5625G,  200R8  */
    _27_1875_PAM4,      /* for PAM4 mode at 54.375G */
    LAST_MV_HWS_SERDES_SPEED

}MV_HWS_SERDES_SPEED;

/**
* @enum MV_HWS_SERDES_MEDIA
*
* @brief Defines the serdes media.
*/
typedef enum{

    XAUI_MEDIA,

    RXAUI_MEDIA

} MV_HWS_SERDES_MEDIA;

/**
* @enum MV_HWS_PORT_FEC_MODE
*
* @brief Defines the FEC status.
*/
typedef enum
{
    FEC_NA         = 0,

    FEC_OFF        = (1 << 0),  /* FEC disable */
    FC_FEC         = (1 << 1),  /* Fire Code FEC */
    RS_FEC         = (1 << 2),  /* Reed Solomon FEC (528, 514) */
    RS_FEC_544_514 = (1 << 3),  /* Reed Solomon FEC (544, 514) */

}MV_HWS_PORT_FEC_MODE;

/**
* @enum MV_HWS_SERDES_BUS_WIDTH_ENT
*
* @brief Defines the serdes 10Bit status.
*/
typedef enum
{
    _10BIT_ON,
    _10BIT_OFF,
    _10BIT_NA,
    _20BIT_ON,
    _40BIT_ON,
    _80BIT_ON

}MV_HWS_SERDES_BUS_WIDTH_ENT;

/**
* @enum MV_HWS_SERDES_TYPE
*
* @brief Defines the different SERDES types.
*/
typedef enum
{
    COM_PHY_H,
    COM_PHY_H_REV2,
    COM_PHY_28NM,
    AVAGO,
    AVAGO_16NM,
    D2D_PHY_GW16,        /* GW16 500G SERDES */
    COM_PHY_C12GP41P2V,  /* Used in: AC5 */
    COM_PHY_C28GP4X2,    /* Used in: Fabrico */
    COM_PHY_C28GP4X1,    /* Used in: AC5X */
    COM_PHY_C28GP4X4,    /* Used in: AC5X */
    COM_PHY_C112GX4,     /* Used in: AC5X, AC5P */
    COM_PHY_N5XC56GP5X4, /* Used in: Harrier */
    SERDES_LAST

}MV_HWS_SERDES_TYPE;

/**
* @enum MV_HWS_REF_CLOCK
*
* @brief Defines ref clock.
*/
typedef enum
{
    _156dot25Mhz,
    _78Mhz,
    _25Mhz,
    _125Mhz,
    _312_5Mhz,
    _164_24Mhz
}MV_HWS_REF_CLOCK; /* need to be synchronized with MV_HWS_REF_CLOCK_SUP_VAL enum */

/**
* @enum MV_HWS_REF_CLOCK_SRC
*
* @brief Defines ref clock src.
*/
typedef enum
{
    PRIMARY,
    SECONDARY

}MV_HWS_REF_CLOCK_SRC;

/**
* @enum MV_HWS_SERDES_ENCODING_TYPE
*
* @brief Defines the different SERDES encoding types.
*/
typedef enum{

    SERDES_ENCODING_NA,

    SERDES_ENCODING_NRZ,

    SERDES_ENCODING_PAM4,

    SERDES_ENCODING_LAST

} MV_HWS_SERDES_ENCODING_TYPE;


/**
* @enum MV_HWS_PORT_SPEED_ENT
 *
 * @brief Enumeration of port speeds
*/
typedef enum
{
    MV_HWS_PORT_SPEED_NA_E    = 0,
    MV_HWS_PORT_SPEED_10M_E   = 10,
    MV_HWS_PORT_SPEED_100M_E  = 100,
    MV_HWS_PORT_SPEED_1G_E    = 1000,
    MV_HWS_PORT_SPEED_2_5G_E  = 2500,
    MV_HWS_PORT_SPEED_5G_E    = 5000,
    MV_HWS_PORT_SPEED_10G_E   = 10000,
    MV_HWS_PORT_SPEED_20G_E   = 20000,
    MV_HWS_PORT_SPEED_25G_E   = 25000,
    MV_HWS_PORT_SPEED_40G_E   = 40000,
    MV_HWS_PORT_SPEED_50G_E   = 50000,
    MV_HWS_PORT_SPEED_100G_E  = 100000,
    MV_HWS_PORT_SPEED_200G_E  = 200000,
    MV_HWS_PORT_SPEED_400G_E  = 400000

}MV_HWS_PORT_SPEED_ENT;

/**
* @enum MV_HWS_SERDES_DUMP_TYPE_ENT
*
* @brief Defines the serdes dump types.
*/
typedef enum{

    SERDES_DUMP_INTERNAL_REGISTERS,
    SERDES_DUMP_EXTERNAL_REGISTERS,
    SERDES_DUMP_PINS,
    SERDES_DUMP_SELECTED_FIELDS_0

} MV_HWS_SERDES_DUMP_TYPE_ENT;

/**
* @struct MV_HWS_SERDES_CONFIG_STC
*
* @brief Defines structure for SERDES power up procedure.
*/
typedef struct
{
    MV_HWS_SERDES_TYPE              serdesType;
    MV_HWS_SERDES_SPEED             baudRate;
    MV_HWS_REF_CLOCK                refClock;
    MV_HWS_REF_CLOCK_SRC            refClockSource;
    MV_HWS_SERDES_MEDIA             media;
    MV_HWS_SERDES_BUS_WIDTH_ENT     busWidth;
    MV_HWS_SERDES_ENCODING_TYPE     encoding;
    GT_BOOL                         opticalMode;
} MV_HWS_SERDES_CONFIG_STC;

/**
* @struct MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC
 *
 * @brief Port SERDES datapath configuration, includes:
 *  Polarity inversion, Gray coding, Pre-coding, Swizzle coding.
*/
typedef struct
{
    GT_BOOL polarityInvert;
    GT_BOOL grayEnable;
    GT_BOOL precodeEnable;
    GT_BOOL swizzleEnable;
} MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC;


#ifndef BIT_MASK
/* Return the mask including "numOfBits" bits.          */
#define BIT_MASK(numOfBits) (~(0xFFFFFFFF << (numOfBits)))

/* Calculate the field mask for a given offset & length */
/* e.g.: BIT_MASK(8,2) = 0xFFFFFCFF                     */
#define FIELD_MASK_NOT(offset,len)                      \
        (~(BIT_MASK((len)) << (offset)))

/* Sets the field located at the specified offset & length in data.     */
#define U32_SET_FIELD(data,offset,length,val)           \
   (data) = (((data) & FIELD_MASK_NOT((offset),(length))) | ((val) <<(offset)))

#endif
#endif /* __mvPortModeElements_H */


