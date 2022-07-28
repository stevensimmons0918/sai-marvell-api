/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* cpssHalProfile.h
*
* DESCRIPTION:
*       initialize system
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __INCLUDE_MRVL_HAL_PROFILE_API_H
#define __INCLUDE_MRVL_HAL_PROFILE_API_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cpss/common/phy/cpssGenPhySmi.h"
#include "cpss/common/port/cpssPortCtrl.h"

#define PROFILE_SPEED_UNITS_CNS 100 /* units of 100MB */

typedef enum
{
    PROFILE_TYPE_INVALID_E=0,
    PROFILE_TYPE_PORT_MAP_E,
    PROFILE_TYPE_LANE_SWAP_E,
    PROFILE_TYPE_SIM_INIFILE_E,
    PROFILE_TYPE_CPU_PORT_MAP_E,
    PROFILE_TYPE_CASCADE_PORT_MAP_E,
    PROFILE_TYPE_HW_SKU_E,
    /* MUST BE LAST */
    PROFILE_TYPE_MAX_E,
    PROFILE_TYPE_LAST_E = 0xFF
} PROFILE_TYPE_ENT;

typedef struct
{
    unsigned int maxPhyPorts;
    unsigned int maxVlans; /* Max eVlans */
    unsigned int maxLAgrps;
    unsigned int maxMCgrps;
    unsigned int maxMCeVidxgrps;
    unsigned int maxSTPgrps;
    unsigned int maxLAGMbrPerGrp;
} PROFILE_TYPE_HW_SKU_STC;

typedef struct
{
    //shared mem resource 1
    uint32_t maxHashTable;
    //shared mem resource 2
    uint32_t maxRouteLPM;
    //shared mem resource 3
    uint32_t maxExactMatch;
    uint32_t maxPBR;
    // shared by NH+ECMP
    uint32_t maxNH;
    uint32_t maxNhEcmpGrps;
    uint32_t maxNhEcmpMbrPerGrp;
    //number of neighbours, this would be
    //minimum of (ARP entries, hosts that can fit into FDB)
    uint32_t maxNeighbours;
    //table shared ARP+TS+NAT
    //shared mem resource 4
    uint32_t maxARP;
    //shared mem resource 5
    uint32_t maxTunStart; //max entries in ARP-TS table
    uint32_t maxNATTable;
    uint32_t maxVRF;
    //Default30B rules, table shared IACL+EACL+TTI
    uint32_t maxIACL;
    uint32_t maxEACL;
    uint32_t maxTunTerm;
    //ePorts
    uint32_t maxIVIF;
    uint32_t maxEVIF;
    uint32_t maxMirrorSessions;
    //TODO need to include FDB based mac2me?
    uint32_t maxMac2Me;
    // MLL Piar table shared b/w L2 and IP MLL
    uint32_t maxMllPair;
    /*TODO other tables*/
    //policer entries
    //counter IDs
    // copp policers
} HW_TABLE_PROFILE_STC;

typedef enum
{
    PROFILE_INTERFACE_MODE_1000BASE_X_E = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,    /* 1000BASE_X_E 1G, */
    PROFILE_INTERFACE_MODE_QSGMII_E = CPSS_PORT_INTERFACE_MODE_QSGMII_E,   /* QSGMII 1G, */
    PROFILE_INTERFACE_MODE_USX_OUSGMII_E = CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E,   /* USX_OUSGMII 1G, */
    PROFILE_INTERFACE_MODE_USX_20G_OXGMII_E = CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E,   /* USX_20G_OXGMII 2.5G, */
    PROFILE_INTERFACE_MODE_KR_E         = CPSS_PORT_INTERFACE_MODE_KR_E,   /* KR_E 10G, 12G, 20G, 40G */
    PROFILE_INTERFACE_MODE_SR_LR_E      = CPSS_PORT_INTERFACE_MODE_SR_LR_E,   /* SR_LR_E 10G, 12G, 20G, 40G, */
    PROFILE_INTERFACE_MODE_SR_LR2_E     = CPSS_PORT_INTERFACE_MODE_SR_LR2_E,
    PROFILE_INTERFACE_MODE_SR_LR4_E     = CPSS_PORT_INTERFACE_MODE_SR_LR4_E,
    PROFILE_INTERFACE_MODE_SR_LR8_E     = CPSS_PORT_INTERFACE_MODE_SR_LR8_E,
    PROFILE_INTERFACE_MODE_KR2_E        = CPSS_PORT_INTERFACE_MODE_KR2_E,   /* KR2_E 25G */
    PROFILE_INTERFACE_MODE_KR4_E        = CPSS_PORT_INTERFACE_MODE_KR4_E,   /* KR4_E 100G */
    PROFILE_INTERFACE_MODE_CR2_E        = CPSS_PORT_INTERFACE_MODE_CR2_E,   /* CR2_E 50G */
    PROFILE_INTERFACE_MODE_CR4_E        = CPSS_PORT_INTERFACE_MODE_CR4_E,   /* CR4_E 100G */
    PROFILE_INTERFACE_MODE_KR8_E = CPSS_PORT_INTERFACE_MODE_KR8_E,
    PROFILE_INTERFACE_MODE_CR8_E = CPSS_PORT_INTERFACE_MODE_CR8_E,
    PROFILE_INTERFACE_MODE_SGMII_E = CPSS_PORT_INTERFACE_MODE_SGMII_E
} PROFILE_INTERFACE_MODE_ENT;
typedef struct
{
    unsigned int                existsPhy;
    unsigned int                phyAddr;
    CPSS_PHY_SMI_INTERFACE_ENT  smiIf;
    CPSS_PHY_XSMI_INTERFACE_ENT xSmiIf;
} SMI_PHY_STC;


typedef struct
{
    unsigned int                portNum;
    unsigned int                macNum;
    unsigned int                txQNum;
    PROFILE_INTERFACE_MODE_ENT  interfaceMode;
    /*interfaceSpeed : in units of PROFILE_SPEED_UNITS_CNS  */
    unsigned int                interfaceSpeed;
    SMI_PHY_STC                 phyInfo;
    unsigned int                frontPanelPortIdx;
    unsigned int                isRxTxParamValid;
    CPSS_PORT_SERDES_RX_CONFIG_UNT rxParam[8]; /* isRxTxParamValid & 1 */
    CPSS_PORT_SERDES_TX_CONFIG_UNT txParam[8]; /* isRxTxParamValid & 2 */
} PORT_MAP_STC;

typedef struct
{
    unsigned int    laneNum;
    unsigned int    invertTx;
    unsigned int    invertRx;
} SERDES_LANE_POLARITY_STC;

typedef union
{
    PORT_MAP_STC                portMap; /* PROFILE_TYPE_PORT_MAP_E */
    SERDES_LANE_POLARITY_STC    serdes_polarity; /* PROFILE_TYPE_LANE_SWAP_E */
    char                        sim_inifile[64]; /*PROFILE_TYPE_SIM_INIFILE_E*/
    PROFILE_TYPE_HW_SKU_STC*    hwTableSizes;
    /* last */
    unsigned int                no_param;
} PROFILE_VALUE;

typedef struct
{
    PROFILE_TYPE_ENT    profileType;
    PROFILE_VALUE       profileValue;
} PROFILE_STC;

typedef enum
{
    LED_PROFILE_DEFAULT_E=0,
    LED_PROFILE_64BIT_AC3X,
    LED_PROFILE_97BIT_AC3X,
    LED_PROFILE_LAST_E = 0xFF
} LED_PROFILE_TYPE_E;

#define LED_PROFILE_97BIT_AC3X_STR  "ac3x97bits"
#define LED_PROFILE_65BIT_AC3X_STR  "ac3x65bits"

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__INCLUDE_MRVL_HAL_PROFILE_API_H*/
