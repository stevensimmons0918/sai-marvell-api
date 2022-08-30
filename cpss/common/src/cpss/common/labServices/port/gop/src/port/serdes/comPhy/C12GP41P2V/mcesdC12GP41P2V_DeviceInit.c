/*******************************************************************************
Copyright (C) 2018, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
COMPHY_12G_PIPE4_1P2V
********************************************************************/
#include "../mcesdTop.h"
#include "../mcesdApiTypes.h"
#include "mcesdC12GP41P2V_Defs.h"
#include "mcesdC12GP41P2V_RegRW.h"
#include "mcesdC12GP41P2V_HwCntl.h"
#include "mcesdC12GP41P2V_API.h"
#include "mcesdC12GP41P2V_DeviceInit.h"

#ifdef C12GP41P2V

/* Forward internal function prototypes used only in this module */
static MCESD_STATUS INT_C12GP41P2V_AdditionalInit1(IN MCESD_DEV_PTR devPtr);
static MCESD_STATUS INT_C12GP41P2V_AdditionalInit2(IN MCESD_DEV_PTR devPtr);
static MCESD_STATUS INT_C12GP41P2V_AdditionalInit3(IN MCESD_DEV_PTR devPtr);
static MCESD_STATUS INT_C12GP41P2V_AdditionalInit4(IN MCESD_DEV_PTR devPtr);

MCESD_STATUS API_C12GP41P2V_PowerOnSeq
(
    IN MCESD_DEV_PTR devPtr,
    IN S_C12GP41P2V_PowerOn powerOn
)
{
    MCESD_BOOL powerLane0 = powerOn.powerLaneMask & 0x1;
    /* Set SERDES Mode */
    MCESD_ATTEMPT(API_C12GP41P2V_SetPhyMode(devPtr, C12GP41P2V_PHYMODE_SERDES));

    /* Disable TX Output */
    MCESD_ATTEMPT(API_C12GP41P2V_SetTxOutputEnable(devPtr, MCESD_FALSE));

    /* Power up current and voltage reference */
    MCESD_ATTEMPT(API_C12GP41P2V_SetPowerIvRef(devPtr, MCESD_TRUE));

    /* Make sure PHY PLLs, Transmitter and Receiver power is off */
    /*MCESD_ATTEMPT(API_C12GP41P2V_PowerOffLane(devPtr));*/

    /* Configure Reference Frequency and reference clock source group */
    MCESD_ATTEMPT(API_C12GP41P2V_SetRefFreq(devPtr, powerOn.refFreq, powerOn.refClkSel));

    /* Additional Steps */
    if (powerLane0)
    {
        MCESD_ATTEMPT(INT_C12GP41P2V_AdditionalInit1(devPtr));
        MCESD_ATTEMPT(INT_C12GP41P2V_AdditionalInit2(devPtr));
        MCESD_ATTEMPT(INT_C12GP41P2V_AdditionalInit3(devPtr));
        MCESD_ATTEMPT(INT_C12GP41P2V_AdditionalInit4(devPtr));
    }

    /* Configure TX and RX bitrates */
    if (powerLane0)
        MCESD_ATTEMPT(API_C12GP41P2V_SetTxRxBitRate(devPtr, powerOn.speed));

    /* Set Data Bus Width for all lanes */
    if (powerLane0)
        MCESD_ATTEMPT(API_C12GP41P2V_SetDataBusWidth(devPtr, powerOn.dataBusWidth));
#if 0
    /* Enable DFE for speeds above 6 Gbps */
    if (powerLane0)
        MCESD_ATTEMPT(API_C12GP41P2V_SetDfeEnable(devPtr, (powerOn.speed > 10) ? MCESD_TRUE : MCESD_FALSE));
#endif
    /* Power on PHY PLLs, Transmitter and Receiver */
    if (powerLane0)
    {
        if (powerOn.initTx)
            MCESD_ATTEMPT(API_C12GP41P2V_SetPowerTx(devPtr, MCESD_TRUE));
        if (powerOn.initRx)
            MCESD_ATTEMPT(API_C12GP41P2V_SetPowerRx(devPtr, MCESD_TRUE));
        MCESD_ATTEMPT(API_C12GP41P2V_SetPowerPLL(devPtr, MCESD_TRUE));
    }

    /* Enable TX Output for all lanes */
    if ((powerOn.txOutputEn) && (powerLane0))
        MCESD_ATTEMPT(API_C12GP41P2V_SetTxOutputEnable(devPtr, MCESD_TRUE));
#if 0
    /* Toggle RX_INIT */
    if (powerOn.initRx && powerOn.powerLane0)
        MCESD_ATTEMPT(API_C12GP41P2V_RxInit(devPtr));
#endif
    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_PowerOffLane
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_StopTraining(devPtr, C12GP41P2V_TRAINING_RX));
    MCESD_ATTEMPT(API_C12GP41P2V_SetPowerPLL(devPtr, MCESD_FALSE));
    MCESD_ATTEMPT(API_C12GP41P2V_SetPowerTx(devPtr, MCESD_FALSE));
    MCESD_ATTEMPT(API_C12GP41P2V_SetPowerRx(devPtr, MCESD_FALSE));

    return MCESD_OK;
}

MCESD_STATUS INT_C12GP41P2V_AdditionalInit1
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD linkTrainMode       = F_C12GP41P2VR2P0_LINK_TRAINMODE;
    MCESD_FIELD detBypass           = F_C12GP41P2VR2P0_DET_BYPASS;
    MCESD_FIELD txTrainStartSqEn    = F_C12GP41P2VR2P0_START_SQ_EN;
    MCESD_FIELD txPresetIndex       = F_C12GP41P2VR2P0_TXPRESET_INDEX;
    MCESD_FIELD trainPatNum         = F_C12GP41P2VR2P0_TRAIN_PAT_NUM;
    MCESD_FIELD txTrainPatTwoZero   = F_C12GP41P2VR2P0_PAT_TWO_ZERO;
    MCESD_FIELD ethernetMode        = F_C12GP41P2VR2P0_ETHERNET_MODE;
    MCESD_FIELD txAmpDefault1       = F_C12GP41P2VR2P0_AMP_DEFAULT1;
    MCESD_FIELD txEmph1Default1     = F_C12GP41P2VR2P0_EMPH1_DEFAULT1;
    MCESD_FIELD txEmph0Default1     = F_C12GP41P2VR2P0_EMPH0_DEFAULT1;
    MCESD_FIELD txAmpDefault2       = F_C12GP41P2VR2P0_AMP_DEFAULT2;
    MCESD_FIELD txEmph1Default2     = F_C12GP41P2VR2P0_EMPH1_DEFAULT2;
    MCESD_FIELD txEmph0Default2     = F_C12GP41P2VR2P0_EMPH0_DEFAULT2;
    MCESD_FIELD txTrainPatSel       = F_C12GP41P2VR2P0_TRAIN_PAT_SEL;
    MCESD_FIELD txTrainPatMode      = F_C12GP41P2VR2P0_TRAIN_PAT_MODE;
    MCESD_FIELD localTxPresetIndex  = F_C12GP41P2VR2P0_LOCAL_INDEX;
    MCESD_FIELD rxCtleVicmSmplrSel  = F_C12GP41P2VR2P0_VICM_SMPLR_SEL;
    MCESD_FIELD rxCtleVicm1Ctrl     = F_C12GP41P2VR2P0_CTLE_VICM1;
    MCESD_FIELD rxCtleVicm2Ctrl     = F_C12GP41P2VR2P0_CTLE_VICM2;
    MCESD_FIELD rxVicmSmplrCtrl     = F_C12GP41P2VR2P0_VICM_SMPLR;
    MCESD_FIELD rxVicmDfeCtrl       = F_C12GP41P2VR2P0_VICM_DFE_CTRL;
    MCESD_FIELD vrefSamplerVcmSel   = F_C12GP41P2VR2P0_SAMPLER_VCM;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &linkTrainMode, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &detBypass, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txTrainStartSqEn, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txPresetIndex, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &trainPatNum, 0x88));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txTrainPatTwoZero, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ethernetMode, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txAmpDefault1, 0x10));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmph1Default1, 0x3));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmph0Default1, 0x3));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txAmpDefault2, 0x16));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmph1Default2, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txEmph0Default2, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txTrainPatSel, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txTrainPatMode, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &localTxPresetIndex, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleVicmSmplrSel, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleVicm1Ctrl, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxCtleVicm2Ctrl, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxVicmSmplrCtrl, 0x2));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxVicmDfeCtrl, 0x2));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &vrefSamplerVcmSel, 0x3));

    return MCESD_OK;
}

MCESD_STATUS INT_C12GP41P2V_AdditionalInit2
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD rxMaxDfeAdapt0      = F_C12GP41P2VR2P0_RX_MAX_DFE_0;
    MCESD_FIELD rxMaxDfeAdapt1      = F_C12GP41P2VR2P0_RX_MAX_DFE_1;
    MCESD_FIELD cdrMaxDfeAdapt0     = F_C12GP41P2VR2P0_CDR_MAX_DFE_0;
    MCESD_FIELD cdrMaxDfeAdapt1     = F_C12GP41P2VR2P0_CDR_MAX_DFE_1;
    MCESD_FIELD txMaxDfeAdapt       = F_C12GP41P2VR2P0_TX_MAX_DFE;
    MCESD_FIELD sumfBoostTargetK    = F_C12GP41P2VR2P0_SUMF_TARGET_K;
    MCESD_FIELD sumfBoostTargetC    = F_C12GP41P2VR2P0_SUMF_TARGET_C;
    MCESD_FIELD eoBased             = F_C12GP41P2VR2P0_EO_BASED;
    MCESD_FIELD dfeResF0aHighThres  = F_C12GP41P2VR2P0_DFE_RES_HIGH;
    MCESD_FIELD dfeResF0aLowThres   = F_C12GP41P2VR2P0_DFE_RES_LOW;
    MCESD_FIELD ffeResSel10         = F_C12GP41P2VR2P0_FFE_RES_SEL10;
    MCESD_FIELD ffeResSel11         = F_C12GP41P2VR2P0_FFE_RES_SEL11;
    MCESD_FIELD ffeResSel12         = F_C12GP41P2VR2P0_FFE_RES_SEL12;
    MCESD_FIELD ffeResSel13         = F_C12GP41P2VR2P0_FFE_RES_SEL13;
    MCESD_FIELD ffeResSel14         = F_C12GP41P2VR2P0_FFE_RES_SEL14;
    MCESD_FIELD ffeResSel15         = F_C12GP41P2VR2P0_FFE_RES_SEL15;
    MCESD_FIELD ffeResSel16         = F_C12GP41P2VR2P0_FFE_RES_SEL16;
    MCESD_FIELD ffeResSel17         = F_C12GP41P2VR2P0_FFE_RES_SEL17;
    MCESD_FIELD ffeResSel20         = F_C12GP41P2VR2P0_FFE_RES_SEL20;
    MCESD_FIELD ffeResSel21         = F_C12GP41P2VR2P0_FFE_RES_SEL21;
    MCESD_FIELD ffeResSel22         = F_C12GP41P2VR2P0_FFE_RES_SEL22;
    MCESD_FIELD ffeResSel23         = F_C12GP41P2VR2P0_FFE_RES_SEL23;
    MCESD_FIELD ffeResSel24         = F_C12GP41P2VR2P0_FFE_RES_SEL24;
    MCESD_FIELD ffeResSel25         = F_C12GP41P2VR2P0_FFE_RES_SEL25;
    MCESD_FIELD ffeResSel26         = F_C12GP41P2VR2P0_FFE_RES_SEL26;
    MCESD_FIELD ffeResSel27         = F_C12GP41P2VR2P0_FFE_RES_SEL27;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxMaxDfeAdapt0, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &rxMaxDfeAdapt1, 0x3));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &cdrMaxDfeAdapt0, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &cdrMaxDfeAdapt1, 0x4));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txMaxDfeAdapt, 0x3));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &sumfBoostTargetK, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &sumfBoostTargetC, 0x3F0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &eoBased, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeResF0aHighThres, 0x5));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &dfeResF0aLowThres, 0x2));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel10, 0x9));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel11, 0x8));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel12, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel13, 0x6));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel14, 0x5));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel15, 0x4));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel16, 0x3));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel17, 0x2));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel20, 0x4));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel21, 0x4));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel22, 0x4));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel23, 0x4));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel24, 0x4));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel25, 0x4));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel26, 0x4));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel27, 0x4));

    return MCESD_OK;
}

MCESD_STATUS INT_C12GP41P2V_AdditionalInit3
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD ffeCapSel10         = F_C12GP41P2VR2P0_FFE_CAP_SEL10;
    MCESD_FIELD ffeCapSel11         = F_C12GP41P2VR2P0_FFE_CAP_SEL11;
    MCESD_FIELD ffeCapSel12         = F_C12GP41P2VR2P0_FFE_CAP_SEL12;
    MCESD_FIELD ffeCapSel13         = F_C12GP41P2VR2P0_FFE_CAP_SEL13;
    MCESD_FIELD ffeCapSel14         = F_C12GP41P2VR2P0_FFE_CAP_SEL14;
    MCESD_FIELD ffeCapSel15         = F_C12GP41P2VR2P0_FFE_CAP_SEL15;
    MCESD_FIELD ffeCapSel16         = F_C12GP41P2VR2P0_FFE_CAP_SEL16;
    MCESD_FIELD ffeCapSel17         = F_C12GP41P2VR2P0_FFE_CAP_SEL17;
    MCESD_FIELD ffeCapSel18         = F_C12GP41P2VR2P0_FFE_CAP_SEL18;
    MCESD_FIELD ffeCapSel19         = F_C12GP41P2VR2P0_FFE_CAP_SEL19;
    MCESD_FIELD ffeCapSel1A         = F_C12GP41P2VR2P0_FFE_CAP_SEL1A;
    MCESD_FIELD ffeCapSel1B         = F_C12GP41P2VR2P0_FFE_CAP_SEL1B;
    MCESD_FIELD ffeCapSel1C         = F_C12GP41P2VR2P0_FFE_CAP_SEL1C;
    MCESD_FIELD ffeCapSel1D         = F_C12GP41P2VR2P0_FFE_CAP_SEL1D;
    MCESD_FIELD ffeCapSel1E         = F_C12GP41P2VR2P0_FFE_CAP_SEL1E;
    MCESD_FIELD ffeCapSel1F         = F_C12GP41P2VR2P0_FFE_CAP_SEL1F;
    MCESD_FIELD ffeCapSel20         = F_C12GP41P2VR2P0_FFE_CAP_SEL20;
    MCESD_FIELD ffeCapSel21         = F_C12GP41P2VR2P0_FFE_CAP_SEL21;
    MCESD_FIELD ffeCapSel22         = F_C12GP41P2VR2P0_FFE_CAP_SEL22;
    MCESD_FIELD ffeCapSel23         = F_C12GP41P2VR2P0_FFE_CAP_SEL23;
    MCESD_FIELD ffeCapSel24         = F_C12GP41P2VR2P0_FFE_CAP_SEL24;
    MCESD_FIELD ffeCapSel25         = F_C12GP41P2VR2P0_FFE_CAP_SEL25;
    MCESD_FIELD ffeCapSel26         = F_C12GP41P2VR2P0_FFE_CAP_SEL26;
    MCESD_FIELD ffeCapSel27         = F_C12GP41P2VR2P0_FFE_CAP_SEL27;
    MCESD_FIELD ffeCapSel28         = F_C12GP41P2VR2P0_FFE_CAP_SEL28;
    MCESD_FIELD ffeCapSel29         = F_C12GP41P2VR2P0_FFE_CAP_SEL29;
    MCESD_FIELD ffeCapSel2A         = F_C12GP41P2VR2P0_FFE_CAP_SEL2A;
    MCESD_FIELD ffeCapSel2B         = F_C12GP41P2VR2P0_FFE_CAP_SEL2B;
    MCESD_FIELD ffeCapSel2C         = F_C12GP41P2VR2P0_FFE_CAP_SEL2C;
    MCESD_FIELD ffeCapSel2D         = F_C12GP41P2VR2P0_FFE_CAP_SEL2D;
    MCESD_FIELD ffeCapSel2E         = F_C12GP41P2VR2P0_FFE_CAP_SEL2E;
    MCESD_FIELD ffeCapSel2F         = F_C12GP41P2VR2P0_FFE_CAP_SEL2F;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel10, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel11, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel12, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel13, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel14, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel15, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel16, 0x2));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel17, 0x3));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel18, 0x5));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel19, 0x7));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel1A, 0xA));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel1B, 0xF));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel1C, 0xF));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel1D, 0xF));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel1E, 0xF));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel1F, 0xF));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel20, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel21, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel22, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel23, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel24, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel25, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel26, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel27, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel28, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel29, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel2A, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel2B, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel2C, 0x2));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel2D, 0x5));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel2E, 0x9));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeCapSel2F, 0xF));

    return MCESD_OK;
}

MCESD_STATUS INT_C12GP41P2V_AdditionalInit4
(
    IN MCESD_DEV_PTR devPtr
)
{
    MCESD_FIELD ffeResSel30         = F_C12GP41P2VR2P0_FFE_RES_SEL30;
    MCESD_FIELD ffeResSel31         = F_C12GP41P2VR2P0_FFE_RES_SEL31;
    MCESD_FIELD ffeResSel32         = F_C12GP41P2VR2P0_FFE_RES_SEL32;
    MCESD_FIELD ffeResSel33         = F_C12GP41P2VR2P0_FFE_RES_SEL33;
    MCESD_FIELD ffeResSel34         = F_C12GP41P2VR2P0_FFE_RES_SEL34;
    MCESD_FIELD ffeResSel35         = F_C12GP41P2VR2P0_FFE_RES_SEL35;
    MCESD_FIELD ffeResSel36         = F_C12GP41P2VR2P0_FFE_RES_SEL36;
    MCESD_FIELD ffeResSel37         = F_C12GP41P2VR2P0_FFE_RES_SEL37;
    MCESD_FIELD ffeResSel38         = F_C12GP41P2VR2P0_FFE_RES_SEL38;
    MCESD_FIELD ffeResSel39         = F_C12GP41P2VR2P0_FFE_RES_SEL39;
    MCESD_FIELD ffeResSel3A         = F_C12GP41P2VR2P0_FFE_RES_SEL3A;
    MCESD_FIELD ffeResSel3B         = F_C12GP41P2VR2P0_FFE_RES_SEL3B;
    MCESD_FIELD ffeResSel3C         = F_C12GP41P2VR2P0_FFE_RES_SEL3C;
    MCESD_FIELD ffeResSel3D         = F_C12GP41P2VR2P0_FFE_RES_SEL3D;
    MCESD_FIELD ffeResSel3E         = F_C12GP41P2VR2P0_FFE_RES_SEL3E;
    MCESD_FIELD ffeResSel3F         = F_C12GP41P2VR2P0_FFE_RES_SEL3F;
    MCESD_FIELD g1RxTrainMode       = F_C12GP41P2VR2P0_G1_RX_TRAIN;
    MCESD_FIELD vthTximpcal         = F_C12GP41P2VR2P0_VTH_TXIMPCAL;
    MCESD_FIELD train32bitAutoEn    = F_C12GP41P2VR2P0_TRAIN_32BIT_EN;
    MCESD_FIELD selBitsSerdesTrain  = F_C12GP41P2VR2P0_SEL_BITS_TRAIN;
    MCESD_FIELD txAdaptG1En         = F_C12GP41P2VR2P0_TXADAPT_G1_EN;
    MCESD_FIELD txAdaptGn1En        = F_C12GP41P2VR2P0_TXADAPT_GN1_EN;
    MCESD_FIELD txAdaptG0En         = F_C12GP41P2VR2P0_TXADAPT_G0_EN;
    S_C12GP41P2V_TRAINING_TIMEOUT trainingTimeout;
    trainingTimeout.enable = MCESD_FALSE;
    trainingTimeout.timeout = 0x3FF;

    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel30, 0x4));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel31, 0x3));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel32, 0x2));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel33, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel34, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel35, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel36, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel37, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel38, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel39, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel3A, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel3B, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel3C, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel3D, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel3E, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &ffeResSel3F, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &g1RxTrainMode, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &vthTximpcal, 0x2));

    MCESD_ATTEMPT(API_C12GP41P2V_SetTrainingTimeout(devPtr, C12GP41P2V_TRAINING_RX, &trainingTimeout));
    MCESD_ATTEMPT(API_C12GP41P2V_SetTrainingTimeout(devPtr, C12GP41P2V_TRAINING_TRX, &trainingTimeout));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &train32bitAutoEn, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &selBitsSerdesTrain, 0x6));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txAdaptG1En, 0x0));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txAdaptGn1En, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_WriteField(devPtr, &txAdaptG0En, 0x1));
    MCESD_ATTEMPT(API_C12GP41P2V_SetSlewRateEnable(devPtr, C12GP41P2V_SR_DISABLE));
    MCESD_ATTEMPT(API_C12GP41P2V_SetSlewRateParam(devPtr, C12GP41P2V_SR_CTRL0, 0));
    MCESD_ATTEMPT(API_C12GP41P2V_SetSlewRateParam(devPtr, C12GP41P2V_SR_CTRL1, 0));
    return MCESD_OK;
}

#endif /* C12GP41P2V */