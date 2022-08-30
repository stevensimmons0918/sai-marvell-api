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
* @file mvHwsIronmanMtip10GPcsLpcsGeneralUnits.h
*
* @brief IronmanL port interface header file
*
* @version   1
********************************************************************************
*/
#ifndef __mvHwsIronmanMtip10GPcsLpcsGeneralUnits_H
#define __mvHwsIronmanMtip10GPcsLpcsGeneralUnits_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /*0*/  IRONMAN_LPCS_GENERAL_GSTATUS_GSYNC_STATUS_E,
  /*1*/  IRONMAN_LPCS_GENERAL_GSTATUS_GAN_DONE_STATUS_E,
  /*2*/  IRONMAN_LPCS_GENERAL_CFG_CLOCK_RATE_CFGCLOCKRATE_E,
  /*3*/  IRONMAN_LPCS_GENERAL_M1_PORTSENABLE_E,
  /*4*/  IRONMAN_LPCS_GENERAL_USXGMII_ENABLE_INDICATION_USXGMII_ENABLE_IND_E,
  /*5*/  IRONMAN_LPCS_GENERAL_GMODE_LPCS_ENABLE_E,
  /*6*/  IRONMAN_LPCS_GENERAL_GMODE_QSGMII_0_ENABLE_E,
  /*7*/  IRONMAN_LPCS_GENERAL_GMODE_QSGMII_1_ENABLE_E,
  /*8*/  IRONMAN_LPCS_GENERAL_GMODE_USGMII8_ENABLE_E,
  /*9*/  IRONMAN_LPCS_GENERAL_GMODE_USGMII_SCRAMBLE_ENABLE_E,
  /*10*/ IRONMAN_LPCS_GENERAL_M0_PORTSENABLE_E,
    IRONMAN_LPCS_GENERAL_UNITS_REGISTER_LAST_E /* should be last */
} MV_HWS_IRONMAN_LPCS_GENERAL_UNITS_E;

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsIronmanMtip10GPcsLpcsGeneralUnits_H */

