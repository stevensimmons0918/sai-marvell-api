/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* cpssHalInitialize.c
*
* DESCRIPTION:
*       initialize system
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/



#include <cpssHalDev.h>
#include <cpssHalProfile.h>
/* PP data - should contain entry for each supported PP*/
mrvSupportedDevinfo mrvSupportedDevData[] =
{
    /* Aldrin */

    {
        CPSS_98DX8308_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98DX8312_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98DX8315_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98DX8316_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98DX8324_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98DX8332_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98DXZ832_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98DX8332_Z0_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98DXH831_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98DXH832_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98DXH833_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },



    /* Armstrong */
    {
        CPSS_98EX5410_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98EX5420_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98EX5422_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98EX5424_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98EX5421_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98EX5430_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98EX5431_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    /* Bobcat 3*/
    {
        CPSS_98CX8420_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98CX8410_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    /* Bobk */
    /*cetus*/
    {
        CPSS_98DX4235_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_BOBCAT2_REV_A0_40G_NOT_THROUGH_TM_IS_PA_30G_E, CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    /*cygnus*/
    {
        CPSS_98DX4203_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        {  CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },

    /* TODO:
     * Confirm the ref clock and serdes clock for Aldrin 2
     * Any Workaround list
     */
    /* Aldrin 2 */
    {
        CPSS_98EX5520_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98DX8410_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98DX8548_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98DX8448_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },

    /* AC3X devices - B2B board */
    {
        CPSS_98DX3255_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98DX3258_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98DX3265_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    /*FALCON */
    {
        CPSS_98CX8520_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98CX8540_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98CX8580_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98CX8512_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98CX8514_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98EX5614_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    {
        CPSS_98EX5610_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    /* AC5X devices - RD board */
    {
        CPSS_98DX3550_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    /* AC5X devices - WM RD board */
    {
        CPSS_98DX3510M_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    /* AC5X devices - WM RD board */
    {
        CPSS_98DX3500M_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
    /* AC5X devices - RD SR2 board */
    {
        CPSS_98DX3550M_CNS,
        GT_TRUE,
        GT_FALSE,
        CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E,
        { CPSS_DXCH_IMPLEMENT_WA_LAST_E }
    },
};

const int mrvSupportedDevDataSize =(sizeof(mrvSupportedDevData)/sizeof(
                                        mrvSupportedDevinfo));



/*number of DQ ports in each of the 4 DQ units in TXQ */
#define ALDRIN2_NUM_PORTS_PER_DQ_IN_TXQ_CNS     25
/*macro to convert local port and data path index to TXQ port */
#define ALDRIN2_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC(localPort , dpIndex) \
    (localPort) + ((dpIndex) * ALDRIN2_NUM_PORTS_PER_DQ_IN_TXQ_CNS)

#define ALDRIN2_DQ_INDEX_FROM_GLOBAL_PORT(globalMacPort)               \
    (((globalMacPort) < 24) ? ((globalMacPort) / 12) : /*0/1*/           \
    ((globalMacPort) < 72) ? (2 + ((globalMacPort)-24)/24) : /*2/3*/  \
    /*globalMacPort == 72*/ 0)

#define ALDRIN2_DQ_LOCAL_PORT_FROM_GLOBAL_PORT(globalMacPort)          \
    (((globalMacPort) < 24) ? ((globalMacPort) % 12) : /*0..11*/         \
    ((globalMacPort) < 72) ? ((globalMacPort) % 24) : /*0..23*/         \
    /*globalMacPort == 72*/ 24)

/* build TXQ_port from global mac port */
#define ALDRIN2_TXQ_PORT(globalMacPort)                            \
    ALDRIN2_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC(                      \
        ALDRIN2_DQ_LOCAL_PORT_FROM_GLOBAL_PORT(globalMacPort),     \
        ALDRIN2_DQ_INDEX_FROM_GLOBAL_PORT(globalMacPort))

/* build pair of mac port and it's TXQ port*/
#define ALDRIN2_MAC_AND_TXQ_PORT_MAC(macPort)  \
    (macPort), ALDRIN2_TXQ_PORT(macPort)

#define _10G_SPEED (10000/PROFILE_SPEED_UNITS_CNS)
#define _1G_SPEED  ( 1000/PROFILE_SPEED_UNITS_CNS)
#define INT_FOR_10G   PROFILE_INTERFACE_MODE_KR_E

/*=== AC3X ===*/

/* NOTE: For M0 WM, We instantiate ALDRIN dev instead of AC3X.
         Hence the scale num will be different in WM.
   For now, below Struct is common for both AC3x 3255 and 3265 variants.
   Add new strcut, if scale num varies. */
PROFILE_TYPE_HW_SKU_STC hw_sku_profiles_ald[]=
{
    /*0*/  {
        .maxPhyPorts = 64,
        .maxVlans    = _8K,   // TBD: Max eVlan for AC3X is 4.5K and ALD is 8K. Fix it.
        .maxLAgrps   = (_1K-1),
        .maxMCgrps   = (_4K-1),//4095 is reserved by HW.
        .maxMCeVidxgrps   = (_16K-1),//HW supported range 4096 to max.
        .maxSTPgrps  = _1K,
        .maxLAGMbrPerGrp = 8
    }
};
HW_TABLE_PROFILE_STC ac3x_3255_hw_table_sizes_profile[]=
{
    {
        .maxHashTable    = _16K,
        .maxRouteLPM     = 16*1024,
        .maxExactMatch   = 0,
        .maxPBR          = 0,
        .maxARP          = 4*1024,
        .maxNH           = 4*1024,
        .maxTunStart     = (8*4)+(8*8),
        .maxNATTable     = 0,
        .maxVRF          = _4K,
        .maxVRF          = 4*1024,
        .maxIACL         = 6*1024,
        .maxEACL         = 0,
        .maxTunTerm      = 0,
        .maxIVIF         = 0,
        .maxEVIF         = 0,
        .maxMirrorSessions=7,
        .maxMac2Me       = 128,
        .maxNeighbours   = _4K-(8*4)-(8*8),
        .maxNhEcmpMbrPerGrp = 64,
        .maxNhEcmpGrps = ((4*1024)/64),
        .maxMllPair      = _4K
    },
    {}
};

HW_TABLE_PROFILE_STC ac3x_3265_hw_table_sizes_profile[]=
{
    {
        .maxHashTable    = _32K,
        .maxRouteLPM     = 16*1024,
        .maxExactMatch   = 0,
        .maxPBR          = 0,
        .maxARP          = 16*1024,
        .maxNH           = 8*1024,
        .maxTunStart     = (8*4)+(8*8),
        .maxNATTable     = 0,
        .maxVRF          = _4K,
        .maxVRF          = 4*1024,
        .maxIACL         = 6*1024,
        .maxEACL         = 0,
        .maxTunTerm      = 0,
        .maxIVIF         = 0,
        .maxEVIF         = 0,
        .maxMirrorSessions=7,
        .maxMac2Me       = 128,
        .maxNeighbours   = ((16*1024) - (8*4)-(8*8)),
        .maxNhEcmpMbrPerGrp = 64,
        .maxNhEcmpGrps = ((8*1024)/64),
        .maxMllPair      = _8K
    },
    {}
};

/*=== AC5X ===*/
PROFILE_TYPE_HW_SKU_STC hw_sku_profiles_ac5x[]=
{
    /*0*/  {
        .maxPhyPorts = 128,
        .maxVlans    = _6K,   // TBD: Max eVlan for AC3X is 4.5K and ALD is 8K. Fix it.
        .maxLAgrps   = (_4K-1),
        .maxMCgrps   = (_12K-1),//12K-1 is reserved by HW. VIDX
        .maxSTPgrps  = _1K,
        .maxLAGMbrPerGrp = 8
    }
};

HW_TABLE_PROFILE_STC ac5x_hw_table_sizes_profile[]=
{
    {
        .maxHashTable    = _32K,
        .maxRouteLPM     = _16K, //32K ipv4, 16K ipv6
        .maxExactMatch   = 0,
        .maxPBR          = 0,
        .maxARP          = _16K,
        .maxNH           = _8K,
        .maxTunStart     = (8*4)+(8*8),
        .maxNATTable     = 0,
        .maxVRF          = _4K,
        .maxIACL         = 6*1024,
        .maxEACL         = 0,
        .maxTunTerm      = 0,
        .maxIVIF         = 0,
        .maxEVIF         = 0,
        .maxMirrorSessions=7,
        .maxMac2Me       = 128,
        .maxNeighbours   = ((16*1024) - (8*4)-(8*8)),
        .maxNhEcmpMbrPerGrp = 64,
        .maxNhEcmpGrps = ((4*1024)/64),
        .maxMllPair      = _8K
    },
    {}
};

/*=== ALDRIN2-XL ===*/
PROFILE_TYPE_HW_SKU_STC hw_sku_profiles_aldrin2_xl[]=
{
    {
        .maxPhyPorts = 64,
        .maxVlans    = _8K,
        .maxLAgrps   = (_4K-1),
        .maxMCgrps   = (_4K-1), //4095 is reserved by HW.
        .maxMCeVidxgrps   = (_16K-1),//HW supported range 4096 to max.
        .maxSTPgrps  = _1K,
        .maxLAGMbrPerGrp = 8
    }
};

/*=== ALDRIN2-Fujitsu_Large ===*/
PROFILE_TYPE_HW_SKU_STC hw_sku_profiles_aldrin2_fl[]=
{
    {
        .maxPhyPorts = 48,
        .maxVlans    = _8K,
        .maxLAgrps   = (_4K-1),
        .maxMCgrps   = (_4K-1), //4095 is reserved by HW.
        .maxMCeVidxgrps   = (_16K-1),//HW supported range 4096 to max.
        .maxSTPgrps  = _1K,
        .maxLAGMbrPerGrp = 8
    }
};

HW_TABLE_PROFILE_STC aldrin2_xl_hw_table_sizes_profile[]=
{
    {
        .maxHashTable    = _32K,
        .maxRouteLPM     = 16*1024,
        .maxExactMatch   = 0,
        .maxPBR          = 0,
        .maxARP          = 32*1024,
        .maxNH           = 8*1024,
        .maxTunStart     = (8*4)+(8*8),
        .maxNATTable     = 0,
        .maxVRF          = _4K,
        .maxVRF          = 4*1024,
        .maxIACL         = 3*1024,
        .maxEACL         = 0,
        .maxTunTerm      = 0,
        .maxIVIF         = 0,
        .maxEVIF         = 0,
        .maxMirrorSessions=7,
        .maxMac2Me       = 128,
        .maxNeighbours   = ((32*1024) -(8*4)-(8*8)),
        .maxNhEcmpMbrPerGrp = 64,
        .maxNhEcmpGrps = ((8*1024)/64),
        .maxMllPair      = _16K
    },
};

/*=== FALCON ===*/
/* Below are HW supported Numbers,
 * except maxLAGMbrPerGrp(hwNum is 4K) which we have fixed to 8.
 * NOTE: Global maxLagMemberCnt is 16K. Hence, for prof 0, it will be
 * set to ((16K/8) = _2K) in cpssHalInit.c
 * (maxLAgrps x maxLAGMbrPerGrp) must be less than Device Lag member Cnt (16K).
 */
PROFILE_TYPE_HW_SKU_STC hw_sku_profiles_falcon[]=
{
    /*0*/  {
        .maxPhyPorts = 64,
        .maxVlans    = _8K,
        .maxLAgrps   = (_4K-1),
        .maxMCgrps   = (_4K-1), //4095 is reserved by HW.
        .maxMCeVidxgrps   = (_16K-1),//HW supported range 4096 to max.
        .maxSTPgrps  = _4K,
        .maxLAGMbrPerGrp = 8
    },
    /*1*/  {
        .maxPhyPorts = 128,
        .maxVlans    = _8K,
        .maxLAgrps   = (_2K-1),
        .maxMCgrps   = _2K,
        .maxMCeVidxgrps   = (_16K-1),//HW supported range 4096 to max.
        .maxSTPgrps  = _2K,
        .maxLAGMbrPerGrp = 8
    },
    /*2*/  {
        .maxPhyPorts = 256,
        .maxVlans    = _4K,
        .maxLAgrps   = (_1K-1),
        .maxMCgrps   = _1K,
        .maxMCeVidxgrps   = (_16K-1),//HW supported range 4096 to max.
        .maxSTPgrps  = _1K,
        .maxLAGMbrPerGrp = 8
    },
    /*3*/  {
        .maxPhyPorts = 512,
        .maxVlans    = _2K,
        .maxLAgrps   = 511,
        .maxMCgrps   = _1K,
        .maxMCeVidxgrps   = (_16K-1),//HW supported range 4096 to max.
        .maxSTPgrps  = _1K,
        .maxLAGMbrPerGrp = 8
    }
};
HW_TABLE_PROFILE_STC falcon_hw_table_sizes_profile[]=
{
    {
        .maxHashTable    = 128*1024,
        .maxRouteLPM     = 216*1024,
        .maxExactMatch   = 32*1024,
        .maxPBR          = 0,
        .maxARP          = (96*1024),
        .maxNH           = 24*1024,
        .maxTunTerm      = 0,
        .maxTunStart     = (4*1024),
        .maxNATTable     = 0,
        .maxVRF          = _4K,
        .maxVRF          = 4*1024,
        .maxIACL         = 6*1024,
        .maxEACL         = 0,
        .maxIVIF         = 0,
        .maxEVIF         = 0,
        .maxMirrorSessions=7,
        .maxMac2Me       = 128,
        .maxNeighbours   = ((96*1024)-(4*1024)), // Exclude TS v4 and v6 and NAT
        .maxNhEcmpMbrPerGrp = 64,
        .maxNhEcmpGrps = ((24*1024)/64),
        .maxMllPair      = _16K
    },
    {}
};

PROFILE_TYPE_HW_SKU_STC hw_sku_profiles_cygnus[]=
{
    /*0*/  {64, _8K, _4K, _4K, _4K}
};

// *INDENT-OFF*

PROFILE_STC aldrin2Profile[] =
#include "aldrin2Profile.h"

PROFILE_STC defaultProfile[] =
#include "defaultProfile.h"

PROFILE_STC DB_CPSS_aldrinProfile[] =
#include "DB_CPSS_aldrinProfile.h"

PROFILE_STC aldrinProfile[] =
#include "aldrinProfile.h"

PROFILE_STC ac3x_b2b_profile0[] =
#include "ac3x_b2b_profile0.h"

PROFILE_STC ac3x_b2b_profile1[] =
#include "ac3x_b2b_profile1.h"

PROFILE_STC ac3x_b2b_tg48mp_profile0[] =
#include "ac3x_b2b_tg48mp_profile0.h"

PROFILE_STC ac3x_b2b_tg48mp_profile1[] =
#include "ac3x_b2b_tg48mp_profile1.h"

PROFILE_STC ac5x_rd_board_profile[] =
#include "ac5x_rd_board_profile.h"

PROFILE_STC ac5x_48x1G6x10G_port_profile[] =
#include "ac5x_48x1G6x10G_port_profile.h"

PROFILE_STC falcon_3_2_profile[] =
#include "falcon_3_2_profile.h"

PROFILE_STC falcon_6_4_profile[] =
#include "falcon_6_4_profile.h"

PROFILE_STC falcon_32x100_8x400_port_profile[] =
#include "falcon_32x100_8x400_port_profile.h"

PROFILE_STC falcon_32x25_port_profile[] =
#include "falcon_32x25_port_profile.h"

PROFILE_STC falcon_32x400_port_profile[] =
#include "falcon_32x400_port_profile.h"

PROFILE_STC falcon_32x25_6_4_port_profile[] =
#include "falcon_32x25_6_4_port_profile.h"

PROFILE_STC falcon_32_port_profile[] =
#include "falcon_32_port_profile.h"

PROFILE_STC falcon_64x25_port_profile[] =
#include "falcon_64x25_port_profile.h"

PROFILE_STC falcon_16x100_8x400_port_profile[] =
#include "falcon_16x100_8x400_port_profile.h"

PROFILE_STC falcon_24x25_8x100_6_4_port_profile[] =
#include "falcon_24x25_8x100_6_4_port_profile.h"

PROFILE_STC falcon_24x25_4x200_6_4_port_profile[] =
#include "falcon_24x25_4x200_6_4_port_profile.h"

PROFILE_STC falcon_48x10_8x100_port_profile[]=
#include "falcon_48x10_8x100_port_profile.h"

PROFILE_STC falcon_48x25_8x100_port_profile[]=
#include "falcon_48x25_8x100_port_profile.h"

PROFILE_STC falcon_128x50_port_profile[] =
#include "falcon_128x50_port_profile.h"

PROFILE_STC falcon_128x10_port_profile[] =
#include "falcon_128x10_port_profile.h"

PROFILE_STC falcon_64x100_port_profile[] =
#include "falcon_64x100_port_profile.h"

PROFILE_STC falcon_32x100_R4_6_4_port_profile_ixia[] =
#include "falcon_32x100_R4_6_4_port_profile_ixia.h"

PROFILE_STC falcon_64x100_R4_port_profile[] =
#include "falcon_64x100_R4_port_profile.h"

PROFILE_STC falcon_64x100_ixia_port_profile[] =
#include "falcon_64x100_R4_port_profile.h"

PROFILE_STC falcon_16x400_port_profile[] =
#include "falcon_16x400_port_profile.h"

PROFILE_STC falcon_16x25_port_profile[] =
#include "falcon_16x25_port_profile.h"

PROFILE_STC falcon_256_port_profile[] =
#include "falcon_256_port_profile.h"

PROFILE_STC falcon_belly2belly_profile[] =
#include "falcon_belly2belly_profile.h"

PROFILE_STC falcon_32x25_8_ixia[] =
#include "falcon_32x25_8_ixia.h"

PROFILE_STC falcon_24x25_8x100_12_8_port_profile[] =
#include "falcon_24x25_8x100_12_8_port_profile.h"

PROFILE_STC falcon_24x25_8x200_port_profile[] =
#include "falcon_24x25_8x200_port_profile.h"

PROFILE_STC falcon_24x100_4x400_port_profile[] =
#include "falcon_24x100_4x400_port_profile.h"

PROFILE_STC falcon_48x100_8x400_port_profile[] =
#include "falcon_48x100_8x400_profile.h"

PROFILE_STC falcon_32x100_16x400_port_profile[] =
#include "falcon_32x100_16x400_profile.h"

PROFILE_STC falcon_96x100_8x400_port_profile[] =
#include "falcon_96x100_8x400_profile.h"

PROFILE_STC falcon_128x100_port_profile[] =
#include "falcon_128x100_profile.h"

PROFILE_STC falcon_16x25_4_ixia[] =
#include "falcon_16x25_4_ixia.h"

PROFILE_STC falcon_128x25_port_profile[] =
#include "falcon_128x25_port_profile.h"

PROFILE_STC falcon_64x25_64x10_port_profile[] =
#include "falcon_64x25_64x10_port_profile.h"

PROFILE_STC falcon_32x25_8x100_port_profile[] =
#include "falcon_32x25_8x100_port_profile.h"

PROFILE_STC falcon_2T_80x25_port_profile[] =
#include "falcon_2T_80x25_port_profile.h"

PROFILE_STC falcon_2T_48x25_8x100_port_profile[] =
#include "falcon_2T_48x25_8x100_port_profile.h"

PROFILE_STC falcon_2T_48x10_8x100_port_profile[] =
#include "falcon_2T_48x10_8x100_port_profile.h"

PROFILE_STC cygnus[] =
#include "cygnus.h"

PROFILE_STC aldrin2_xl_port_profile[] =
#include "aldrin2_xl_port_profile.h"

PROFILE_STC aldrin2_xl_fujitsu_large_profile[] =
#include "aldrin2_xl_fujitsu_large_profile.h"

PROFILE_STC aldrin2_xl_fujitsu_large_eval_profile[] =
#include "aldrin2_xl_fujitsu_large_eval_profile.h"

PROFILE_STC ac3x_fujitsu_small_profile[] =
#include "ac3x_fujitsu_small_profile.h"

PROFILE_STC ac3x_fujitsu_small_25_profile[] =
#include "ac3x_fujitsu_small_25_profile.h"

PROFILE_STC ac3x_fujitsu_small_ila_profile[] =
#include "ac3x_fujitsu_small_ila_profile.h"
