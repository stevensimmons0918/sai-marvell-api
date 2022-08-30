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
* @file mvHwsGeMacDb.c
*
* @brief
*
* @version   21
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/geMac/mvHwsGeMacDb.h>

static const MV_OP_PARAMS unresetSeqParams[] = {
    {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER2,            0x0,      (1 << 6)}
};

static const MV_OP_PARAMS resetSeqParams[] = {
    {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER2,           (1 << 6),  (1 << 6)}
};

#if defined(BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)
static const MV_OP_PARAMS xModeSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,           0x0,      (7 << 13)},
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER0,          (1 << 1),  (1 << 1)},
    {GEMAC_UNIT,  GIG_Port_Auto_Negotiation_Configuration,  0x9260,    0xFFF6,},
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,           0x0,      (1 << 1)}
};
#else
static const MV_OP_PARAMS xModeSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,           0x0,      (7 << 13)},
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER0,          (1 << 1),  (1 << 1)},
    {GEMAC_UNIT,  GIG_Port_Auto_Negotiation_Configuration,  0x9260,    0xFFF6,},
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,           0x0,      (1 << 1)},
    {GEMAC_UNIT,  MSM_PORT_MAC_CONTROL_REGISTER4,          (1 << 12), (1 << 12)},
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER2,           0x0,      (1)      }  /* set InBandAutoNegMode to 1000X */
};
#endif

#ifndef MV_HWS_REDUCED_BUILD
static const MV_OP_PARAMS xModeUpdateSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,           0x0,      (7 << 13)},
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER0,          (1 << 1),  (1 << 1)},
    {GEMAC_UNIT,  GIG_Port_Auto_Negotiation_Configuration,  0x9260,    0xFFF6,},
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,           0x0,      (1 << 1)}
};
#endif

static const MV_OP_PARAMS sgModeSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3,           0x0,      (7 << 13)},
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER0,           0x0,      (1 << 1)},
    {GEMAC_UNIT,  GIG_Port_Auto_Negotiation_Configuration,  0xB0E4,    0xFFF6},  /* Disable the AN-FlowControl, bit #11=0 */
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,           0x0,      (1 << 1)},
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER2,           0x1,      (1)     }  /* set InBandAutoNegMode to SGMII */
};

static const MV_OP_PARAMS lbNormalSeqParams[] = {
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,           0x0,      (1 << 5)}
};

static const MV_OP_PARAMS lbTx2RxSeqParams[] = {
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,          (1 << 5),  (1 << 5)}
};

#if defined(BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)
static const MV_OP_PARAMS netxModeSeqParams[] = {
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER0,          (1 << 1),  (1 << 1)},
    {GEMAC_UNIT,  GIG_Port_Auto_Negotiation_Configuration,  0x9260,    0xFFF6},
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,           0x0,      (1 << 1)}
};
#else
static const MV_OP_PARAMS netxModeSeqParams[] = {
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER0,          (1 << 1),  (1 << 1)},
    {GEMAC_UNIT,  GIG_Port_Auto_Negotiation_Configuration,  0x9260,    0xFFF6},
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,           0x0,      (1 << 1)},
    {GEMAC_UNIT,  MSM_PORT_MAC_CONTROL_REGISTER4,          (1 << 12), (1 << 12)},
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER2,           0x0,      (1)      }  /* set InBandAutoNegMode to 1000X */
};
#endif

#ifndef MV_HWS_REDUCED_BUILD
static const MV_OP_PARAMS netxModeUpdateSeqParams[] = {
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER0,          (1 << 1),  (1 << 1)},
    {GEMAC_UNIT,  GIG_Port_Auto_Negotiation_Configuration,  0x9260,    0xFFF6},
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,           0x0,      (1 << 1)}
};
#endif

static const MV_OP_PARAMS netsgModeSeqParams[] = {
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER0,           0x0,      (1 << 1)},
    {GEMAC_UNIT,  GIG_Port_Auto_Negotiation_Configuration,  0xB0E4,    0xFFF6},  /* Disable the AN-FlowControl, bit #11=0 */
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER1,           0x0,      (1 << 1)},
    {GEMAC_UNIT,  GIG_PORT_MAC_CONTROL_REGISTER2,           0x1,      (1)     } /* set InBandAutoNegMode to SGMII */
};

/* For BC2 and AC3 only for XLG ports RX is configured via MAC - TX is controlled by FCA unit */
static const MV_OP_PARAMS fcDisableSeqParams[] = {
    {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER4          ,  (0 << 3),  (3 << 3)},
    {MMPCS_UNIT , 0x600 /* FCA Control register offset */,  (1 << 1),  (1 << 1)} /* enable bypass FCA */
};
static const MV_OP_PARAMS fcBothSeqParams[] = {
    {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER4 , (3 << 3),  (3 << 3)},
    {MMPCS_UNIT , 0x600                         , (0 << 1),  (1 << 1)} /* disable bypass FCA */
};
static const MV_OP_PARAMS fcRxOnlySeqParams[] = {
    {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER4 , (1 << 3),  (3 << 3)},
    {MMPCS_UNIT , 0x600                         , (1 << 1),  (1 << 1)} /* enable bypass FCA */
};
static const MV_OP_PARAMS fcTxOnlySeqParams[] = {
    {GEMAC_UNIT, GIG_PORT_MAC_CONTROL_REGISTER4 , (2 << 3),  (3 << 3)},
    {MMPCS_UNIT , 0x600                         , (0 << 1),  (1 << 1)} /* disable bypass FCA */
};

/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsGeMacSeqDb[] =
{
    {unresetSeqParams,   MV_SEQ_SIZE(unresetSeqParams)},    /* GEMAC_UNRESET_SEQ     */
    {resetSeqParams,     MV_SEQ_SIZE(resetSeqParams)},      /* GEMAC_RESET_SEQ       */
    {xModeSeqParams,     MV_SEQ_SIZE(xModeSeqParams)},      /* GEMAC_X_MODE_SEQ      */
    {sgModeSeqParams,    MV_SEQ_SIZE(sgModeSeqParams)},     /* GEMAC_SG_MODE_SEQ     */
    {lbNormalSeqParams,  MV_SEQ_SIZE(lbNormalSeqParams)},   /* GEMAC_LPBK_NORMAL_SEQ */
    {NULL,               MV_SEQ_SIZE(0)},                   /* GEMAC_LPBK_RX2TX_SEQ  */
    {lbTx2RxSeqParams,   MV_SEQ_SIZE(lbTx2RxSeqParams)},    /* GEMAC_LPBK_TX2RX_SEQ  */
    {netxModeSeqParams,  MV_SEQ_SIZE(netxModeSeqParams)},   /* GEMAC_NET_X_MODE_SEQ  */
    {netsgModeSeqParams, MV_SEQ_SIZE(netsgModeSeqParams)},  /* GEMAC_NET_SG_MODE_SEQ */
    {fcDisableSeqParams, MV_SEQ_SIZE(fcDisableSeqParams)},  /* GEMAC_FC_DISABLE_SEQ */
    {fcBothSeqParams,    MV_SEQ_SIZE(fcBothSeqParams)},     /* GEMAC_FC_BOTH_SEQ */
    {fcRxOnlySeqParams,  MV_SEQ_SIZE(fcRxOnlySeqParams)},   /* GEMAC_FC_RX_ONLY_SEQ */
    {fcTxOnlySeqParams,  MV_SEQ_SIZE(fcTxOnlySeqParams)}    /* GEMAC_FC_TX_ONLY_SEQ */
#ifndef MV_HWS_REDUCED_BUILD
    ,{xModeUpdateSeqParams,     MV_SEQ_SIZE(xModeUpdateSeqParams)}     /* GEMAC_X_MODE_UPDATE_SEQ      */
    ,{netxModeUpdateSeqParams,  MV_SEQ_SIZE(netxModeUpdateSeqParams)}   /* GEMAC_NET_X_MODE_UPDATE_SEQ  */
#endif
};

#ifndef ALDRIN_DEV_SUPPORT
GT_STATUS hwsGeMacSeqInit(GT_U8 devNum)
{
    devNum = devNum;
#if 0
    /* GE MAC Sequences update */
    if (HWS_DEV_GOP_REV(devNum) < GOP_28NM_REV2)
    {
        hwsGeMacSeqDb[GEMAC_X_MODE_SEQ].cfgSeqSize = 4;
        hwsGeMacSeqDb[GEMAC_NET_X_MODE_SEQ].cfgSeqSize = 3;
    }
#endif
    return GT_OK;
}

GT_STATUS hwsGeMacSgSeqInit(GT_U8 devNum)
{
    return hwsGeMacSeqInit(devNum);
}
#endif
GT_STATUS hwsGeMacSeqGet(MV_HWS_GE_MAC_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
    if ((seqType >= MV_MAC_LAST_SEQ) ||
        (hwsGeMacSeqDb[seqType].cfgSeq == NULL))
    {
        return GT_NO_SUCH;
    }

    *seqLine = hwsGeMacSeqDb[seqType].cfgSeq[lineNum];
    return GT_OK;
}


