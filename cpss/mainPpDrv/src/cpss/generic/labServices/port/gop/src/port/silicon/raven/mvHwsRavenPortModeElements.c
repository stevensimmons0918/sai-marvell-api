/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*  mvHwsRavenPortModeElements.c
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>


/************************* definition *****************************************************/

/************************* definition *****************************************************/
#define RAVEN_X2_SUP_MODES     hwsRavenPort0SupModes,hwsRavenPort1SupModes
#define RAVEN_X4_SUP_MODES     RAVEN_X2_SUP_MODES,RAVEN_X2_SUP_MODES
#define RAVEN_X8_SUP_MODES     RAVEN_X4_SUP_MODES,hwsRavenPort4SupModes,hwsRavenPort1SupModes,RAVEN_X2_SUP_MODES
#define RAVEN_X16_SUP_MODES    RAVEN_X8_SUP_MODES,RAVEN_X8_SUP_MODES

/************************* Globals ********************************************************/
/* Last line of each SupModes array MUST be NON_SUP_MODE */
const MV_HWS_PORT_INIT_PARAMS hwsRavenPort0SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type    PCS Num  FEC             Speed           FirstLane  LanesList         LanesNum  Media         10Bit     FecSupported */
    { _10GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,    0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _50GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_CR,   MTI_MAC_100,   0,     MTI_PCS_100,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_SR_LR,MTI_MAC_100,   0,     MTI_PCS_100,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_KR2, MTI_MAC_100,   0,     MTI_PCS_100,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_CR2, MTI_MAC_100,   0,     MTI_PCS_100,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_SR_LR2, MTI_MAC_100,0,     MTI_PCS_100,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_KR4, MTI_MAC_400,   0,     MTI_PCS_400,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_CR4, MTI_MAC_400,   0,     MTI_PCS_400,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_SR_LR4,MTI_MAC_400, 0,     MTI_PCS_400,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_KR8, MTI_MAC_400,   0,     MTI_PCS_400,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _40BIT_ON, RS_FEC_544_514},
    { _200GBase_CR8, MTI_MAC_400,   0,     MTI_PCS_400,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _40BIT_ON, RS_FEC_544_514},
    { _200GBase_SR_LR8, MTI_MAC_400,0,     MTI_PCS_400,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _40BIT_ON, RS_FEC_544_514},
    { _400GBase_KR8, MTI_MAC_400,   0,     MTI_PCS_400,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _400GBase_CR8, MTI_MAC_400,   0,     MTI_PCS_400,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _400GBase_SR_LR8, MTI_MAC_400,0,     MTI_PCS_400,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,    NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};
const MV_HWS_PORT_INIT_PARAMS hwsRavenPort1SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type    PCS Num  FEC             Speed           FirstLane  LanesList         LanesNum  Media         10Bit     FecSupported */
    { _10GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_50,     0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _50GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_50,     0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_CR,   MTI_MAC_100,   0,     MTI_PCS_50,     0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_SR_LR,MTI_MAC_100,   0,     MTI_PCS_50,     0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,    NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};
const MV_HWS_PORT_INIT_PARAMS hwsRavenPort4SupModes[] = {
     /* port mode,    MAC Type    Mac Num   PCS Type    PCS Num  FEC             Speed           FirstLane  LanesList        LanesNum  Media         10Bit     FecSupported */
    { _10GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,    0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _50GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_CR,   MTI_MAC_100,   0,     MTI_PCS_100,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_KR2, MTI_MAC_100,   0,     MTI_PCS_100,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_KR4, MTI_MAC_400,   0,     MTI_PCS_200,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_CR2, MTI_MAC_100,   0,     MTI_PCS_100,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_CR4, MTI_MAC_400,   0,     MTI_PCS_200,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_SR_LR4,MTI_MAC_400, 0,     MTI_PCS_200,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_SR_LR,MTI_MAC_100,   0,     MTI_PCS_100,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_SR_LR2,MTI_MAC_100, 0,     MTI_PCS_100,    0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,    NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};


const MV_HWS_PORT_INIT_PARAMS hwsRavenCpu0PortSupModes[] = {
    /* port mode,    MAC Type      Mac Num   PCS Type       PCS Num  FEC             Speed       FirstLane  LanesList        LanesNum  Media         10Bit     FecSupported */
    { _10GBase_KR,   MTI_CPU_MAC,   512,     MTI_CPU_PCS,    512,    FEC_OFF,   _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_CPU_MAC,   512,     MTI_CPU_PCS,    512,    FEC_OFF,   _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { NON_SUP_MODE,  MAC_NA,        NA_NUM,  PCS_NA,         NA_NUM, FEC_OFF,   SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS hwsRavenCpu1PortSupModes[] = {
    /* port mode,    MAC Type      Mac Num   PCS Type       PCS Num  FEC             Speed        FirstLane  LanesList        LanesNum  Media         10Bit     FecSupported */
    { _10GBase_KR,   MTI_CPU_MAC,   513,     MTI_CPU_PCS,    513,    FEC_OFF,        _10_3125G,   NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_CPU_MAC,   513,     MTI_CPU_PCS,    512,    FEC_OFF,        _25_78125G,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { NON_SUP_MODE,  MAC_NA,        NA_NUM,  PCS_NA,         NA_NUM, FEC_OFF,        SPEED_NA,    NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};


/* map the right supported modes table for each port */
const MV_HWS_PORT_INIT_PARAMS *hwsPortsRavenParamsSupModesMap[] =
{
    RAVEN_X16_SUP_MODES
};

/************************* functions ******************************************************/

