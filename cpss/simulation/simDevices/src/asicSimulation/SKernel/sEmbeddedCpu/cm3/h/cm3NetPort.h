#ifndef _CM3_NET_PORT_H
#define _CM3_NET_PORT_H

#define LB_MASK 0x0F


typedef enum
{
    MANUAL_CTLE=0,
    FIXED_CTLE,
    CONFIDENCE_CTLE

}MI_CTLE_MODES;

typedef enum
{
    LB_MODE_NOLB = 0x0,
    LB_MODE_SD_TXRX,
    LB_MODE_SD_RXTX,
    LB_MODE_MAC_TXRX,
    PRBS_TEST = 0x10, /* uses 2nd nibble */

}MI_PORT_DIAG_MODES;

/**
* @enum MV_HWS_UNIT
 *
 * @brief Defines different port loop back levels.
*/
typedef enum{

    HWS_MAC,

    HWS_PCS,

    HWS_PMA,

    HWS_MPF

} CM3_SIM__HWS_UNIT;


/*
* @enum MV_HWS_PORT_LB_TYPE
 *
 * @brief Defines different port loop back types.
 * DISABLE_LB - disable loop back
 * RX_2_TX_LB - configure port to send back all received packets
 * TX_2_RX_LB - configure port to receive back all sent packets
*/
typedef enum{

    DISABLE_LB,

    RX_2_TX_LB,

    TX_2_RX_LB,

    TX_2_RX_DIGITAL_LB

} CM3_SIM__HWS_PORT_LB_TYPE;



typedef struct {
    char * paramName;
    GT_U8 level;
    GT_U8 type;

} MI_LOOPBACK_MODES;


/*copy from mvHwsPortInitIf.h*/

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
/* 46 */ _48GBaseR,              /* Serdes speed: 12.1875G;    Data speed: 4*11.8G */
/* 47 */ _12GBase_SR,            /* Serdes speed: 12.1875G;    Data speed: 1*11.8G */
/* 48 */ _48GBase_SR,            /* Serdes speed: 12.1875G;    Data speed: 4*11.8G */
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

    NON_SUP_MODE,
    LAST_PORT_MODE = NON_SUP_MODE

}CM3_SIM_HWS_PORT_STANDARD;

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

}CM3_SIM_MV_HWS_PORT_FEC_MODE;




typedef struct {
    GT_BOOL active;/*the speed is set*/
    GT_U8 devNum;
    GT_U8 ppPortGroup;
    GT_U8 ppPortNum;

    CM3_SIM_HWS_PORT_STANDARD portMode;
    CM3_SIM_HWS_PORT_STANDARD reqPortMode;

    GT_BOOL txLanePolarity;
    GT_BOOL rxLanePolarity;

    MI_PORT_DIAG_MODES diagMode;
    MI_PORT_DIAG_MODES reqDiagMode;

    /* AP params */
    GT_BOOL isAp;
    GT_BOOL fec_sup;
    GT_BOOL fec_req;
    GT_U32  lane;

} MV_NET_PORT;

GT_STATUS mvNetPortInit();
MV_NET_PORT *mvNetPortArrayGetPortByDevPPort( GT_U8 devNum, GT_U8 portNum );
GT_STATUS simCm3PollingInit( GT_VOID );
GT_STATUS simCm3LinkUp( GT_VOID );



size_t mvNetPortArrayGetSize( void );

GT_STATUS cm3SimInterfaceModeFromString(char *modeName,
                            CM3_SIM_HWS_PORT_STANDARD *mode);

GT_STATUS simCm3BootChannelGetPortStatus(MV_NET_PORT *port, GT_BOOL *enablePtr);

GT_STATUS cm3SimNetPortSetMode
(
    MV_NET_PORT *port,
    CM3_SIM_HWS_PORT_STANDARD mode
    /*MV_HWS_PORT_FEC_MODE fecMode*/
);



GT_STATUS cm3SimNetPortLbSet
(
    GT_U32  ppPortNum,
    GT_BOOL loopback
);

#endif /* MV_NET_PORT_H */
