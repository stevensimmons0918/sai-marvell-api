/******************************************************************************
*              Copyright (c) Marvell International Ltd. and its affiliates
*
* This software file (the \"File\") is owned and distributed by Marvell
* International Ltd. and/or its affiliates (\"Marvell\") under the following
* alternative licensing terms.
* If you received this File from Marvell, you may opt to use, redistribute
* and/or modify this File under the following licensing terms.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*  -   Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*  -   Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*  -    Neither the name of Marvell nor the names of its contributors may be
*       used to endorse or promote products derived from this software without
*       specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************
* wrHwsSerdes.c
*
* DESCRIPTION:
*       Implementation of the Galtis Wrapper functions
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 27 $
*
*******************************************************************************/

/* Common for all HWS Galtis wrappers */
#include <galtisAgent/wrapCpss/systemLab/wrHwsCommon.h>

/* General H Files */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <stdlib.h>

/* Feature specific includes */
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH/mvComPhyEomIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nm/mvComPhyH28nmIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/gw16/mvGw16If.h>
#include <cpss/common/labServices/port/gop/port/mac/d2dMac/mvHwsD2dMacIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/d2dPcs/mvHwsD2dPcsIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralCpll.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsDiagnostic.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h>


GT_STATUS mvHwsComHRev2DfeCheck
(
    GT_U8 devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum
);

typedef enum
{
  SERDES_DISABLE   = 0,
  SERDES_ENABLE    = 1,
  SERDES_TX_CONFIG = 2

}MV_HWS_SERDES_AUTO_TUNE_MODE;

GT_U32  eomUpperMatrix[EOM_VOLT_RANGE * EOM_PHASE_RANGE];
GT_U32  eomLowerMatrix[EOM_VOLT_RANGE * EOM_PHASE_RANGE];

#ifdef CHX_FAMILY
extern GT_STATUS ravenInitSystem(GT_U8 startDevNum, GT_U8 endDevNum, GT_U32 gopRev);
#endif
/*******************************************************************************
* wrHwsSerdesAutoTuneStartSetEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesAutoTuneStartSetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc = GT_OK; /* Return code  */
    GT_U8       devNum;
    GT_U32      serdesNum;
    GT_U8       rxTr;
    GT_U8       txTr;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum    = (GT_U8)inArgs[0];
    serdesNum = (GT_U32)inFields[0];
    rxTr = (GT_U8)inFields[1];
    txTr = (GT_U8)inFields[2];

    if ((rxTr != SERDES_DISABLE) && (txTr != SERDES_DISABLE))
    {
        cpssOsPrintf("\nRxTraining and TxTraining cannot be run at the same time!!\n");
        galtisOutput(outArgs, GT_FAIL, "%f");
        return CMD_OK;
    }

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);
    /* Enable or Disable Rx mode */
    rc = mvHwsSerdesRxAutoTuneStart(globalDevNum, devNum, serdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, serdesNum), rxTr);
    if (rc != GT_OK)
    {
        cpssOsPrintf("\nexecute mvHwsSerdesRxAutoTuneStart fail\n");
        galtisOutput(outArgs, rc, "%f");
        return CMD_OK;
    }

    /* Enable or Disable Tx mode */
    if ((txTr == SERDES_ENABLE) || (txTr == SERDES_DISABLE))
    {
        rc = mvHwsSerdesTxAutoTuneStart(globalDevNum, devNum, serdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, serdesNum), txTr);
    }

    /* Config Tx mode */
    if (txTr == SERDES_TX_CONFIG)
    {
        rc = mvHwsSerdesAutoTuneCfg(globalDevNum, devNum, serdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, serdesNum), GT_FALSE, GT_TRUE);
    }

    if (rc != GT_OK)
    {
        (txTr == SERDES_TX_CONFIG) ? cpssOsPrintf("\nexecute mvHwsSerdesAutoTuneCfg fail\n")
                                   : cpssOsPrintf("\nexecute mvHwsSerdesTxAutoTuneStart fail\n");
        galtisOutput(outArgs, rc, "%f");
        return CMD_OK;
    }

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesLoopbackCfgSetEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesLoopbackCfgSetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      serdesNum;
    MV_HWS_SERDES_LB     lb;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    serdesNum = (GT_U32)inFields[0];

    switch(inFields[1])
    {
    case 0:
      lb = SERDES_LP_DISABLE;
      break;
    case 1:
      lb = SERDES_LP_AN_TX_RX;
      break;
    case 2:
      lb = SERDES_LP_DIG_TX_RX;
      break;
    case 3:
      lb = SERDES_LP_DIG_RX_TX;
      break;
    default:
      lb = SERDES_LP_DISABLE;
      break;
    }

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum)

    rc = mvHwsSerdesLoopback(devNum, globalPortGroup, serdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum), lb);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesManualRxCfgSetEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesManualRxCfgSetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      serdesNum;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT  rxConfig;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    serdesNum = (GT_U32)inFields[0];
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);
    rxConfig.rxComphyH.portTuningMode = (MV_HWS_PORT_MAN_TUNE_MODE)inFields[1];
    rxConfig.rxComphyH.sqlch = (GT_U32)inFields[2];
    rxConfig.rxComphyH.ffeRes = (GT_U32)inFields[3];
    rxConfig.rxComphyH.ffeCap = (GT_U32)inFields[4];
    rxConfig.rxComphyH.dfeEn = (GT_BOOL)inFields[5];
    rxConfig.rxComphyH.alig = (GT_U32)inFields[6];

    rc = mvHwsSerdesManualRxConfig(globalDevNum, devNum, serdesNum,(MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, serdesNum), &rxConfig);
    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static GT_U32 SerdesAutoTuneResultsSerdesNum;

/*******************************************************************************
* wrHwsSerdesAutoTuneResultsGetFirstEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesAutoTuneResultsGetFirstEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      startSerdesNum;
    GT_U32      endSerdesNum;
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT results;
    int i;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    SerdesAutoTuneResultsSerdesNum = (GT_U32)inArgs[1];
    startSerdesNum = (GT_U32)inArgs[1];
    endSerdesNum = (GT_U32)inArgs[2];
    if (endSerdesNum <  startSerdesNum)
    {
        endSerdesNum = startSerdesNum;
    }
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,SerdesAutoTuneResultsSerdesNum);
    rc = mvHwsSerdesAutoTuneResult(globalDevNum, devNum,SerdesAutoTuneResultsSerdesNum,(MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, SerdesAutoTuneResultsSerdesNum),&results);

    if ((HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneResultsSerdesNum) == AVAGO) || (HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneResultsSerdesNum) == AVAGO_16NM))
    {
        inFields[5] = (GT_32)results.avagoResults.avagoStc.atten;
        inFields[6] = 0;
        inFields[7] = 0;
        inFields[8] = (GT_32)results.avagoResults.avagoStc.post;
        inFields[9] = (GT_32)results.avagoResults.avagoStc.pre;
        inFields[10] = 0;
        inFields[11] = 0;
        for (i = 0; i < 6; i++)
        {
            inFields[12+i] = abs(results.avagoResults.DFE[i]); /*DFE_F[i]*/
        }
    }
    else
    {
        inFields[1] = results.comphyHResults.ffeR;
        inFields[2] = results.comphyHResults.ffeC;
        inFields[3] = results.comphyHResults.sampler;
        inFields[4] = results.comphyHResults.sqleuch;
        inFields[5] = results.comphyHResults.txAmp;
        inFields[6] = results.comphyHResults.txAmpAdj;
        inFields[7] = results.comphyHResults.txAmpShft;
        inFields[8] = (GT_32)results.comphyHResults.txEmph0; /*TX_Emph0*/
        inFields[9] = (GT_32)results.comphyHResults.txEmph1; /*TX_Emph1*/
        inFields[10] = (GT_32)results.comphyHResults.txEmph0En; /*TX_Emph0_En*/
        inFields[11] = (GT_32)results.comphyHResults.txEmph1En; /*TX_Emph1_En*/
        for (i = 0; i < 6; i++)
        {
            inFields[12+i] = abs(results.comphyHResults.dfeVals[i]); /*DFE_F[i]*/
        }
    }

    if ((HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneResultsSerdesNum) == AVAGO) || (HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneResultsSerdesNum) == AVAGO_16NM))
    {
        cpssOsPrintf("\nTX Atten is %d", results.avagoResults.avagoStc.atten);
        cpssOsPrintf("\nTX Post is %d",  results.avagoResults.avagoStc.pre);
        cpssOsPrintf("\nTX Pre is %d",   results.avagoResults.avagoStc.post);
    }
    else {
        cpssOsPrintf("\nFFE R is %d",       results.comphyHResults.ffeR);
        cpssOsPrintf("\nFFE C is %d",       results.comphyHResults.ffeC);
        cpssOsPrintf("\nSampler is %d",     results.comphyHResults.sampler);
        cpssOsPrintf("\nSqleuch is %d",     results.comphyHResults.sqleuch);
        cpssOsPrintf("\nTX Amp is %d",      results.comphyHResults.txAmp);
        cpssOsPrintf("\nTX Amp Adj is %d",  results.comphyHResults.txAmpAdj);
        cpssOsPrintf("\nTX Amp Shft is %d", results.comphyHResults.txAmpShft);
        cpssOsPrintf("\nTX Emph0 is %d",    results.comphyHResults.txEmph0);
        cpssOsPrintf("\nTX Emph1 is %d",    results.comphyHResults.txEmph1);
        cpssOsPrintf("\nTX Emph0 En is %d", results.comphyHResults.txEmph0En);
        cpssOsPrintf("\nTX Emph1 En is %d", results.comphyHResults.txEmph1En);
    }
    for (i = 0; i < 6; i++)
    {
        cpssOsPrintf("\nDFE%d is %d", i, results.comphyHResults.dfeVals[i]);
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", SerdesAutoTuneResultsSerdesNum, inFields[1], inFields[2], inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15], inFields[16], inFields[17]);

    galtisOutput(outArgs, rc, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesAutoTuneResultsGetNextEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesAutoTuneResultsGetNextEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      endSerdesNum;
    GT_U32      serdesNum;
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT results;
    GT_U32      i;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    endSerdesNum = (GT_U32)inArgs[2];
    serdesNum = SerdesAutoTuneResultsSerdesNum;

    /* get next index */
    serdesNum++;
    if (serdesNum >    endSerdesNum)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = serdesNum;
    SerdesAutoTuneResultsSerdesNum = serdesNum;
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,SerdesAutoTuneResultsSerdesNum);
    rc = mvHwsSerdesAutoTuneResult(globalDevNum, devNum,serdesNum,(MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, serdesNum),&results);

    if ((HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO) || (HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO_16NM))
    {
        inFields[5] = (GT_32)results.avagoResults.avagoStc.atten;
        inFields[6] = 0;
        inFields[7] = 0;
        inFields[8] = (GT_32)results.avagoResults.avagoStc.post;
        inFields[9] = (GT_32)results.avagoResults.avagoStc.pre;
        inFields[10] = 0;
        inFields[11] = 0;
    }
    else
    {
        inFields[1] =  results.comphyHResults.ffeR;
        inFields[2] =  results.comphyHResults.ffeC;
        inFields[3] =  results.comphyHResults.sampler;
        inFields[4] =  results.comphyHResults.sqleuch;
        inFields[5] =  results.comphyHResults.txAmp;
        inFields[6] =  results.comphyHResults.txAmpAdj;
        inFields[7] =  results.comphyHResults.txAmpShft;
        inFields[8] =  (GT_32)results.comphyHResults.txEmph0; /*TX_Emph0*/
        inFields[9] =  (GT_32)results.comphyHResults.txEmph1; /*TX_Emph1*/
        inFields[10] = (GT_32)results.comphyHResults.txEmph0En; /*TX_Emph0_En*/
        inFields[11] = (GT_32)results.comphyHResults.txEmph1En; /*TX_Emph1_En*/
    }


    for (i = 0; i < 6; i++)
    {
        inFields[12+i] = abs(results.comphyHResults.dfeVals[i]); /*DFE_F[i]*/
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15], inFields[16], inFields[17]);
    galtisOutput(outArgs, rc, "%f");
    return CMD_OK;

}

/*******************************************************************************
* wrHwsSerdesManualTxCfgSetEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesManualTxCfgSetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT  manTxTuneStc;   /* TX configuration parameters */

    /* Parameters List */
    GT_U32  DevNum    = (GT_U32)inArgs[0];
    GT_U32  SerdesNum = (GT_U32)inFields[0];

    GT_UNUSED_PARAM(numFields);

    hwsOsMemSetFuncPtr(&manTxTuneStc, 0, sizeof(manTxTuneStc));

    if (HWS_DEV_GOP_REV(DevNum) <= GOP_28NM_REV2)
    {
        manTxTuneStc.txComphyH.txAmp     = (GT_U32)inFields[1];
        manTxTuneStc.txComphyH.txAmpAdj  = (GT_U32)inFields[2];
        manTxTuneStc.txComphyH.txAmpShft = (GT_U32)inFields[3];
        manTxTuneStc.txComphyH.txEmph0   = (GT_U32)inFields[4];
        manTxTuneStc.txComphyH.txEmph1   = (GT_U32)inFields[5];
    }

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(DevNum,SerdesNum);

    res = mvHwsSerdesManualTxConfig(globalDevNum, DevNum, SerdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, SerdesNum), &manTxTuneStc);

    cpssOsPrintf("res is 0x%x\n", res);

    if (res != GT_OK)
    {
        galtisOutput(outArgs, GT_FAIL, "%f");
        return CMD_OK;
    }
    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvagoSerdesManualTxCfgSet
*
* DESCRIPTION:
*
* INPUTS:
*       devId - Device Id
*       serdesNum
*       atten - Attenuator [0...31]
*       post  - Post-Cursor:
*               for BobK; Aldrin; Aldrin2(for Serdes 24-71); Pipe(for Serdes 0-11) devices: [-31...31]
*               for Bobcat3; Aldrin2(for Serdes 0-23); Pipe(for Serdes 12-15) device: [0...31]
*               for Raven device: [even values: -18...18]
*       pre   - Pre-Cursor:
*               for BobK; Aldrin; Aldrin2(for Serdes 24-71); Pipe(for Serdes 0-11) devices: [-31...31]
*               for Bobcat3; Aldrin2(for Serdes 0-23); Pipe(for Serdes 12-15) devices: [0...31]
*               for Raven device: [even values: [-10...10]
*       pre2  - Pre2-cursor: for Raven device:  [-15...15]
*       pre3  - Pre3-cursor: for Raven device:  [-1, 0, 1]
*
*               Note: For Raven device: (Pre3 + Pre2 + Pre + Atten + Post <= 26)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsAvagoSerdesManualTxCfgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT  txConfig;

    /* Parameters List */
    GT_U32  devNum    = (GT_U32)inArgs[0];
    GT_U32  serdesNum = (GT_U32)inArgs[1];

    GT_UNUSED_PARAM(numFields);

    hwsOsMemSetFuncPtr(&txConfig, 0, sizeof(txConfig));

    /* Fields List */
    txConfig.txAvago.atten = (GT_U32)inFields[0];
    txConfig.txAvago.post  = (GT_U32)inFields[1];
    txConfig.txAvago.pre   = (GT_U32)inFields[2];
    txConfig.txAvago.pre2  = (GT_U32)inFields[3];
    txConfig.txAvago.pre3  = (GT_U32)inFields[4];

    if ((HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO) &&
       ((txConfig.txAvago.pre2 != 0) ||
        (txConfig.txAvago.pre3 != 0)))
    {
        cpssOsPrintf("For Avago 28nm: Pre2 and Pre3 are not supported!!\n");
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }

    /* (Pre3 + Pre2 + Pre1 + Atten + Post <= 26) */
    if (txConfig.txAvago.atten +
        txConfig.txAvago.post  +
        txConfig.txAvago.pre   +
        txConfig.txAvago.pre2  +
        txConfig.txAvago.pre3 > 26)
    {
        cpssOsPrintf("Pre3 + Pre2 + Pre + Atten + Post > 26)\n");
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    res = mvHwsAvagoSerdesManualTxConfig(devNum, globalPortGroup, serdesNum, &txConfig);

    galtisOutput(outArgs, res, "");
    return CMD_OK;
}

/******************************************************************************
* wrHwsAvagoSerdesManualTxCfgGet
*
* DESCRIPTION:
*       Avago SerDes Tx values get.
*
* INPUTS:
*       devNum  - physical device number
*       serdesNum - physical port number
*
* OUTPUTS:
*       atten - Attenuator [0...31]
*       post  - Post-Cursor:
*               for BobK; Aldrin; Aldrin2(for Serdes 24-71); Pipe(for Serdes 0-11) devices: [-31...31]
*               for Bobcat3; Aldrin2(for Serdes 0-23); Pipe(for Serdes 12-15) device: [0...31]
*               for Raven device: [even values: -18...18]
*       pre   - Pre-Cursor:
*               for BobK; Aldrin; Aldrin2(for Serdes 24-71); Pipe(for Serdes 0-11) devices: [-31...31]
*               for Bobcat3; Aldrin2(for Serdes 0-23); Pipe(for Serdes 12-15) devices: [0...31]
*               for Raven device: [even values: [-10...10]
*       pre2  - Pre2-cursor: for Raven device:  [-15...15]
*       pre3  - Pre3-cursor: for Raven device:  [-1, 0, 1]
*
* RETURNS:
*     GT_OK        - on success.
*     GT_BAD_PARAM - on bad parameters
*     GT_FAIL      - on error
*
******************************************************************************/
static CMD_STATUS wrHwsAvagoSerdesManualTxCfgGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS rc;
    GT_U8     devNum;
    GT_U32    serdesNum;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT    tuneValues;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    serdesNum = (GT_U32)inArgs[1];

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    rc = mvHwsAvagoSerdesManualTxConfigGet(devNum, globalPortGroup, serdesNum, &tuneValues);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "mvHwsAvagoSerdesManualCtleConfigGet");
        return CMD_OK;
    }

    inFields[0] = tuneValues.txAvago.atten;
    inFields[1] = tuneValues.txAvago.post;
    inFields[2] = tuneValues.txAvago.pre;
    inFields[3] = tuneValues.txAvago.pre2;
    inFields[4] = tuneValues.txAvago.pre3;
    fieldOutput("%d%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3], inFields[4]);

    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

/*******************************************************************************
*
* @brief   Set the Low Power mode from Avago Serdes
*
* @param[in] devNum    - system device number
* @param[in] serdesNum - physical serdes number
* @param[in] mode      - enable/disable low power mode
*
* @retval 0 - on success
* @retval 1 - on error
*/
static CMD_STATUS wrHwsAvagoSerdesLowPowerModeEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   res;
    GT_BOOL     mode;

    /* Parameters List */
    GT_U32  devNum    = (GT_U32)inArgs[0];
    GT_U32  serdesNum = (GT_U32)inArgs[1];

    GT_UNUSED_PARAM(numFields);

    /* Fields List */
    mode = (GT_BOOL)inFields[0];
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);
#ifndef ASIC_SIMULATION
    res = mvHwsAvagoSerdesLowPowerModeEnable(devNum, globalPortGroup, serdesNum, mode);
#else
    devNum = devNum;
    serdesNum = serdesNum;
    mode = mode;
    res = GT_OK;
#endif

    galtisOutput(outArgs, res, "");
    return CMD_OK;
}

/*******************************************************************************
*
* @brief   Get the Low Power mode from Avago Serdes
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - physical serdes number
*
* @param[out] mode - enable/disable low power mode
*
* @retval 0 - on success
* @retval 1 - on error
*/
static CMD_STATUS wrHwsAvagoSerdesLowPowerModeEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8       devNum;
    GT_U32      serdesNum;
    GT_BOOL     mode = 0;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    serdesNum = (GT_U32)inArgs[1];
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);
#ifndef ASIC_SIMULATION
    rc = mvHwsAvagoSerdesLowPowerModeEnableGet(devNum, globalPortGroup, serdesNum, &mode);
#else
    devNum = devNum;
    serdesNum = serdesNum;
    mode = mode;
    rc = GT_OK;
#endif
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "mvHwsAvagoSerdesLowPowerModeEnableGet");
        return CMD_OK;
    }

    inFields[0] = mode;
    fieldOutput("%d", inFields[0]);

    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesPowerCtrlSetEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesPowerCtrlSetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               rc;         /* Return code  */
    GT_U8                   devNum;
    GT_U32                  serdesNum;
    MV_HWS_SERDES_SPEED     baudRate;
    MV_HWS_REF_CLOCK        refClock;
    MV_HWS_REF_CLOCK_SRC    refClockSource;
    MV_HWS_SERDES_MEDIA     media;
    MV_HWS_SERDES_BUS_WIDTH_ENT  mode;
    MV_HWS_SERDES_CONFIG_STC    serdesConfig;
    MV_HWS_SERDES_ENCODING_TYPE serdesEncoding;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    serdesNum = (GT_U32)inFields[0];

    refClock = (MV_HWS_REF_CLOCK)inFields[2];
    refClockSource = (MV_HWS_REF_CLOCK_SRC)inFields[3];

    switch(inFields[4])
    {
    case 0:
      baudRate = _1_25G; /*_1_25G;*/
      break;
    case 1:
      baudRate = _3_125G; /*_3_125G;*/
      break;
    case 2:
      baudRate = _3_75G; /*_3_75G;*/
      break;
    case 3:
      baudRate = _4_25G; /*_4_25G;*/
      break;
    case 4:
      baudRate = _5G; /*_5G;*/
      break;
    case 5:
      baudRate = _6_25G; /*_6_25G;*/
      break;
    case 6:
      baudRate = _7_5G; /*_7_5G;*/
      break;
    case 7:
      baudRate = _10_3125G; /*_10_3125G;*/
      break;
    case 8:
      baudRate = _12_5G; /*_12_5G;*/
      break;
    case 9:
      baudRate = _3_33G; /*_3_33G;*/
      break;
    case 10:
      baudRate = _11_25G; /*_11_25G;*/
      break;
    case 11:
      baudRate = _11_5625G; /*_11_5625G;*/
      break;
    case 12:
      baudRate = _10_9375G; /*_10_9375G;*/
      break;
    case 13:
      baudRate = _12_1875G; /*_12_1875G;*/
      break;
    case 14:
      baudRate = _5_625G; /*_5_625G;*/
      break;
    case 15:
      baudRate = _5_15625G; /*_5_15625G;*/
      break;
    case 16:
      baudRate = _12_8906G;
      break;
    case 17:
      baudRate = _25_78125G;
      break;
    case 18:
      baudRate = _27_5G; /* for 106G */
      break;
    case 19:
      baudRate = _28_05G;
      break;
    case 20:
      baudRate = _26_5625G;/* for NRZ mode at 26.5625G, for PAM4 mode at 53.125G */
      break;
    case 21:
      baudRate = _28_28125G;/* for NRZ mode at 28.28125G, for PAM4 mode at 56.5625G */
      break;
    case 22:
      baudRate = _26_5625G_PAM4; /* for NRZ mode at 26.5625G, for PAM4 mode at 53.125G */
      break;
    case 23:
      baudRate = _28_125G_PAM4;/* for NRZ mode at 28.125G, for PAM4 mode at 56.25G */
      break;

    default:
      baudRate = _10_3125G; /*_10_3125G;*/
      break;
    }

    switch(inFields[5])
    {
    case 0:
      mode = (MV_HWS_SERDES_BUS_WIDTH_ENT)1; /*_10BIT_OFF;*/
      break;
    case 1:
      mode = (MV_HWS_SERDES_BUS_WIDTH_ENT)0; /*_10BIT_ON;*/
      break;
    default:
      mode = (MV_HWS_SERDES_BUS_WIDTH_ENT)2; /*_10BIT_NA;*/
      break;
    }
    media = (MV_HWS_SERDES_MEDIA)inFields[6];

    if((baudRate == _25_78125G)|| (baudRate ==_27_5G) || (baudRate == _28_05G) ||
        (baudRate == _26_5625G)|| (baudRate == _28_28125G) || (baudRate == _26_5625G_PAM4) || (baudRate == _28_125G_PAM4) || (baudRate == _27_1875_PAM4))
    {
        mode = _40BIT_ON;
    }
     else if(baudRate == _12_8906G)
    {
        mode = _20BIT_ON;
    }

    serdesConfig.baudRate = baudRate;
    serdesConfig.media = media;
    serdesConfig.busWidth = mode;
    serdesConfig.refClock = refClock;
    serdesConfig.refClockSource = refClockSource;
    serdesConfig.serdesType = (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, serdesNum);
    switch (serdesConfig.baudRate) {
        case _26_5625G_PAM4:      /* for NRZ mode at 26.5625G, for PAM4 mode at 53.125G */
        case _28_125G_PAM4:       /* for NRZ mode at 28.125G, for PAM4 mode at 56.25G */
            serdesEncoding = SERDES_ENCODING_PAM4;
            break;
        default:
            serdesEncoding = SERDES_ENCODING_NA;
            break;

    }
    serdesConfig.encoding = serdesEncoding;

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    rc = mvHwsSerdesPowerCtrl(globalDevNum, devNum, serdesNum,(GT_BOOL)inFields[1],&serdesConfig);

    if ((HWS_DEV_SILICON_TYPE(devNum) >= BobK) && ((GT_BOOL)inFields[1] == GT_TRUE))
    {
        /* Enable the Tx signal, the signal was disabled during Serdes init */
        rc = mvHwsSerdesTxEnable(globalDevNum, devNum, serdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, serdesNum), GT_TRUE);
        if (rc != GT_OK)
        {
            galtisOutput(outArgs, CMD_AGENT_ERROR, "");
            return CMD_OK;
        }
    }

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}


/*******************************************************************************
* wrHwsSerdesAvagoPowerCtrlSet
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesAvagoPowerCtrlSetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      serdesNum;
    GT_BOOL     powerUp;
    MV_HWS_SERDES_CONFIG_STC    serdesConfig;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];

    serdesNum = (GT_U32)inFields[0];
    powerUp   = (GT_BOOL)inFields[1];
    serdesConfig.refClock = (MV_HWS_REF_CLOCK)inFields[2];
    serdesConfig.refClockSource = (MV_HWS_REF_CLOCK_SRC)inFields[3];
    serdesConfig.baudRate   = (MV_HWS_SERDES_SPEED)inFields[4];
    serdesConfig.busWidth   = (MV_HWS_SERDES_BUS_WIDTH_ENT)inFields[5];
    serdesConfig.media      = (MV_HWS_SERDES_MEDIA)inFields[6];
    serdesConfig.encoding   = (MV_HWS_SERDES_ENCODING_TYPE)inFields[7];
    serdesConfig.serdesType = (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum);

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    rc = mvHwsAvagoSerdesPowerCtrl(devNum, globalPortGroup, serdesNum, powerUp, &serdesConfig);
    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static CMD_STATUS wrHwsSerdesTxEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      portGroup;
    GT_U32      serdesNum;
    GT_BOOL     txEnable;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    serdesNum = (GT_U32)inArgs[2];
    txEnable = (GT_BOOL)inArgs[3];
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);
    rc = mvHwsSerdesTxEnable(devNum, portGroup, serdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum), txEnable);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static CMD_STATUS wrHwsAvago16nmSerdesPmroMetricGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U8       chipIndex;
    GT_U16      metric;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    chipIndex = devNum;
    /* TODO - update galtis DB and wrapper with chip index */
    rc = mvHwsAvago16nmSerdesPmroMetricGet(0, chipIndex, &metric);

    galtisOutput(outArgs, rc, "%d", metric);

    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesDfeCfgSetEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesDfeCfgSetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      serdesNum;
    MV_HWS_DFE_MODE   dfeMode;
    GT_32      dfeCfg[6];
    int        i;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    serdesNum = (GT_U32)inFields[0];
    dfeMode = (MV_HWS_DFE_MODE)inFields[1];
    dfeCfg[0] = (GT_U32)inFields[2];
    for ( i = 1; i < 6; i++)
    {
      if (inFields[i*2+1] == 1)
      {
        dfeCfg[i] = (GT_U32)inFields[i*2+2] - (GT_U32)inFields[i*2+2]*2;
      }
      else
      {
        dfeCfg[i] = (GT_U32)inFields[i*2+2];
      }
    }
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);
    rc = mvHwsSerdesDfeConfig(globalDevNum, devNum,serdesNum,(MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, serdesNum),dfeMode,dfeCfg);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static GT_U32    wrSerdesDfeCur;

/*******************************************************************************
* wrHwsSerdesDfeCfgGetFirstEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesDfeCfgGetFirstEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      startSerdesNum;
    GT_U32      endSerdesNum;
    GT_U32      serdesNum;
    GT_BOOL    dfeLock;
    GT_32      dfeCfg[6];
    GT_32      f0Cfg[3];
    GT_32      savCfg[5];
    int         i;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    startSerdesNum = (GT_U32)inArgs[1];
    endSerdesNum = (GT_U32)inArgs[2];
    if (endSerdesNum <  startSerdesNum)
    {
        endSerdesNum = startSerdesNum;
    }
    serdesNum = startSerdesNum;
    /* for first index from range */
    wrSerdesDfeCur = startSerdesNum;

    inFields[0] = startSerdesNum;
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);
    rc = mvHwsSerdesDfeStatusExt(globalDevNum, devNum, serdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, serdesNum), &dfeLock, dfeCfg, f0Cfg, savCfg);
    if (rc != GT_OK)
    {
          galtisOutput(outArgs, CMD_AGENT_ERROR, "");
          return CMD_OK;
    }
    inFields[1] = 0;
    inFields[2] = dfeCfg[0];
    for (i = 1;  i < 6; i++)
    {
      if (dfeCfg[i] < 0)
      {
        inFields[1+(2*i)] = 1;
      }
      else
      {
        inFields[1+(2*i)] = 0;
      }
      inFields[2+(2*i)] = abs(dfeCfg[i]);
    }
    inFields[13] = (dfeLock == GT_TRUE) ? 1 : 0;

    /* f0 fields */
    for (i = 0;  i < 3; i++)
    {
        inFields[14 + i] = f0Cfg[i];
    }

    /* sav fields */
    for (i = 0;  i < 5; i++)
    {
        inFields[17 + i] = savCfg[i];
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1], inFields[2], inFields[3], inFields[4], inFields[5], inFields[6], inFields[7],
                inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15],
                inFields[16], inFields[17], inFields[18], inFields[19], inFields[20], inFields[21]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesDfeCfgGetNextEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesDfeCfgGetNextEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      endSerdesNum;
    GT_U32      serdesNum;
    GT_BOOL    dfeLock;
    GT_32      dfeCfg[6];
    GT_32      f0Cfg[3];
    GT_32      savCfg[5];
    int        i;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    endSerdesNum = (GT_U32)inArgs[2];
    serdesNum = wrSerdesDfeCur;

      /* get next index */
      serdesNum++;
      if (serdesNum >    endSerdesNum)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

      inFields[0] = serdesNum;
      wrSerdesDfeCur = serdesNum;
      CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);
    rc = mvHwsSerdesDfeStatusExt(globalDevNum, devNum, serdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, serdesNum), &dfeLock, dfeCfg, f0Cfg, savCfg);
    if (rc != GT_OK)
    {
          galtisOutput(outArgs, CMD_AGENT_ERROR, "");
          return CMD_OK;
    }
    inFields[1] = 0;
    inFields[2] = dfeCfg[0];
    for (i = 1;  i < 6; i++)
    {
      if (dfeCfg[i] < 0)
      {
        inFields[1+(2*i)] = 1;
      }
      else
      {
        inFields[1+(2*i)] = 0;
      }
      inFields[2+(2*i)] = abs(dfeCfg[i]);
    }
    inFields[13] = (dfeLock == GT_TRUE) ? 1 : 0;

    /* f0 fields */
    for (i = 0;  i < 3; i++)
    {
        inFields[14 + i] = f0Cfg[i];
    }

    /* sav fields */
    for (i = 0;  i < 5; i++)
    {
        inFields[17 + i] = savCfg[i];
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1], inFields[2], inFields[3], inFields[4], inFields[5], inFields[6], inFields[7],
                inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15],
                inFields[16], inFields[17], inFields[18], inFields[19], inFields[20], inFields[21]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesResetCfgSetEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesResetCfgSetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      serdesNum;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    serdesNum = (GT_U32)inFields[0];
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);
    rc = mvHwsSerdesReset(devNum, globalPortGroup, serdesNum,
                          (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum),
                          (GT_BOOL)inFields[1],(GT_BOOL)inFields[2],(GT_BOOL)inFields[3]);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static GT_U32 SerdesAutoTuneStatusSerdesNum;

/*******************************************************************************
* wrHwsSerdesAutoTuneStatusGetFirstEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesAutoTuneStatusGetFirstEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    MV_HWS_AUTO_TUNE_STATUS rxStatus;
    MV_HWS_AUTO_TUNE_STATUS txStatus;
    MV_HWS_ALIGN90_PARAMS   serdesParams;
    MV_HWS_AUTO_TUNE_STATUS status;
    GT_BOOL allLanesPass;
    GT_U32 loopNum;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    SerdesAutoTuneStatusSerdesNum = (GT_U32)inArgs[1];

    rxStatus = txStatus = TUNE_NOT_COMPLITED;

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,SerdesAutoTuneStatusSerdesNum);

    rc = mvHwsSerdesAutoTuneStatus(globalDevNum, devNum,SerdesAutoTuneStatusSerdesNum,
                                   (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, SerdesAutoTuneStatusSerdesNum),&rxStatus,&txStatus);

    inFields[1] = rxStatus;
    inFields[2] = txStatus;

    /* start Align90 */
    rc = mvHwsSerdesFixAlign90Start(globalDevNum, devNum,SerdesAutoTuneStatusSerdesNum,
                    (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, SerdesAutoTuneStatusSerdesNum), &serdesParams);
    if (rc != GT_OK)
        {
                galtisOutput(outArgs, rc, "%f");
        return CMD_OK;
        }

    loopNum = 0;
    do
    {
        allLanesPass = GT_TRUE;
        hwsOsExactDelayPtr(globalDevNum, devNum, 20);

        /* check status on related serdes */
        mvHwsSerdesFixAlign90Status(globalDevNum, devNum, SerdesAutoTuneStatusSerdesNum,
                        (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, SerdesAutoTuneStatusSerdesNum), &status);
        if (status != TUNE_PASS)
        {
            allLanesPass = GT_FALSE;
        }
        loopNum++;

    } while ((!allLanesPass) && (loopNum < 10));

        if(!allLanesPass)
        {
                cpssOsPrintf("allLanesPass is FALSE\n");
        }

    /* stop Align90 on each related serdes */
    rc = mvHwsSerdesFixAlign90Stop(globalDevNum, devNum, SerdesAutoTuneStatusSerdesNum,
                        (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, SerdesAutoTuneStatusSerdesNum), &serdesParams, ((status == TUNE_PASS) ? GT_TRUE : GT_FALSE));
                                                if (rc != GT_OK)
        {
                cpssOsPrintf("mvHwsSerdesFixAlign90Stop failed\n");
                galtisOutput(outArgs, rc, "%f");
        return CMD_OK;
        }

    rc = mvHwsSerdesRev2DfeCheck(globalDevNum, devNum, SerdesAutoTuneStatusSerdesNum,
                                 (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, SerdesAutoTuneStatusSerdesNum));
        if (rc != GT_OK)
        {
                cpssOsPrintf("mvHwsSerdesRev2DfeCheck failed\n");
                galtisOutput(outArgs, rc, "%f");
        return CMD_OK;
        }

    /* pack and output table fields */
    fieldOutput("%d%d%d",SerdesAutoTuneStatusSerdesNum,inFields[1],inFields[2]);
    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesAutoTuneStatusGetNextEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesAutoTuneStatusGetNextEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      startSerdesNum;
    GT_U32      endSerdesNum;
    GT_U32      serdesNum;
    MV_HWS_AUTO_TUNE_STATUS rxStatus;
    MV_HWS_AUTO_TUNE_STATUS txStatus;
    MV_HWS_ALIGN90_PARAMS   serdesParams;
    MV_HWS_AUTO_TUNE_STATUS status;
    GT_BOOL allLanesPass;
    GT_U32 loopNum;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    startSerdesNum = (GT_U32)inArgs[1];
    endSerdesNum = (GT_U32)inArgs[2];
    if (endSerdesNum <  startSerdesNum)
    {
        endSerdesNum = startSerdesNum;
    }
      serdesNum = SerdesAutoTuneStatusSerdesNum;

      /* get next index */
      serdesNum++;
      if (serdesNum >    endSerdesNum)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

      inFields[0] = serdesNum;
      SerdesAutoTuneStatusSerdesNum = serdesNum;

    rxStatus = txStatus = TUNE_NOT_COMPLITED;
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,SerdesAutoTuneStatusSerdesNum);
    rc = mvHwsSerdesAutoTuneStatus(globalDevNum, devNum,SerdesAutoTuneStatusSerdesNum,
                                   (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, SerdesAutoTuneStatusSerdesNum),&rxStatus,&txStatus);

    inFields[1] = rxStatus;
    inFields[2] = txStatus;

    /* start Align90 */
    rc = mvHwsSerdesFixAlign90Start(globalDevNum, devNum,SerdesAutoTuneStatusSerdesNum,
                    (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, SerdesAutoTuneStatusSerdesNum), &serdesParams);
    if (rc != GT_OK)
        return rc;

    loopNum = 0;
    do
    {
        allLanesPass = GT_TRUE;
        hwsOsExactDelayPtr(globalDevNum, devNum, 20);
        /* check status on related serdes */
        mvHwsSerdesFixAlign90Status(globalDevNum, devNum, SerdesAutoTuneStatusSerdesNum,
                        (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, SerdesAutoTuneStatusSerdesNum), &status);
        if (status != TUNE_PASS)
        {
            allLanesPass = GT_FALSE;
        }
        loopNum++;

    } while ((!allLanesPass) && (loopNum < 10));

    /* stop Align90 on each related serdes */
    mvHwsSerdesFixAlign90Stop(globalDevNum, devNum, SerdesAutoTuneStatusSerdesNum,
                        (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, SerdesAutoTuneStatusSerdesNum), &serdesParams, ((status == TUNE_PASS) ? GT_TRUE : GT_FALSE));
    mvHwsSerdesRev2DfeCheck(globalDevNum, devNum, SerdesAutoTuneStatusSerdesNum,
                            (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, SerdesAutoTuneStatusSerdesNum));

    /* pack and output table fields */
    fieldOutput("%d%d%d",SerdesAutoTuneStatusSerdesNum,inFields[1],inFields[2]);
    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesTestGeneratorCfgSet
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesTestGeneratorCfgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      serdesNum;
    MV_HWS_SERDES_TX_PATTERN txP;
    MV_HWS_SERDES_TEST_GEN_MODE mode;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    serdesNum = (GT_U32)inFields[0];

    if (inArgs[1] != 0)
    {
      galtisOutput(outArgs, GT_BAD_PARAM, "");
      return CMD_OK;
    }
    switch(inFields[1])
    {
    case 0:
      txP = _1T;
      break;
    case 1:
      txP = _2T;
      break;
    case 2:
      txP = _5T;
      break;
    case 3:
      txP = _10T;
      break;
    case 4:
      txP = PRBS7;
      break;
    case 5:
      txP = PRBS9;
      break;
    case 6:
      txP = PRBS15;
      break;
    case 7:
      txP = PRBS23;
      break;
    case 8:
      txP = PRBS31;
      break;
    case 9:
      txP = DFETraining;
      break;
    default:
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }
    switch(inFields[2])
    {
    case 0:
      mode = SERDES_NORMAL;
      break;
    case 1:
      mode = SERDES_TEST;
      break;
    default:
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    rc = mvHwsSerdesTestGen(globalDevNum, devNum, serdesNum, txP, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, serdesNum), mode);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesTestGeneratorCfgSet
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesTestGeneratorCfgSet1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      portGroup;
    GT_U32      serdesNum;
    MV_HWS_SERDES_TX_PATTERN txP;
    MV_HWS_SERDES_TEST_GEN_MODE mode;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    serdesNum = (GT_U32)inFields[0];

    txP = (MV_HWS_SERDES_TX_PATTERN)inFields[1];
    mode = (MV_HWS_SERDES_TEST_GEN_MODE)inFields[2];

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    rc = mvHwsSerdesTestGen(devNum, portGroup, serdesNum, txP, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum), mode);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}


/*******************************************************************************
* wrHwsSerdesTestGeneratorStatusGetFirstEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesTestGeneratorStatusGetFirstEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      serdesNum;
    GT_BOOL     counterAccumulateMode;
    MV_HWS_SERDES_TX_PATTERN txP;
    MV_HWS_SERDES_TEST_GEN_STATUS status;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    /* get counter mode (clear on read or accumulate mode */
    counterAccumulateMode = (GT_BOOL)inArgs[1];

    /* get serdes number */
    serdesNum = (GT_U32)inArgs[2];

    switch(inArgs[3])
    {
    case 0:
      txP = _1T;
      break;
    case 1:
      txP = _2T;
      break;
    case 2:
      txP = _5T;
      break;
    case 3:
      txP = _10T;
      break;
    case 4:
      txP = PRBS7;
      break;
    case 5:
      txP = PRBS9;
      break;
    case 6:
      txP = PRBS15;
      break;
    case 7:
      txP = PRBS23;
      break;
    case 8:
      txP = PRBS31;
      break;
    case 9:
      txP = DFETraining;
      break;
    default:
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    rc = mvHwsSerdesTestGenStatus(globalDevNum, devNum,serdesNum,(MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, serdesNum),
                                  txP, counterAccumulateMode, &status);

    inFields[1] = status.errorsCntr;
    inFields[3] = status.lockStatus;
    inFields[2] = status.txFramesCntr.l[0];

    /* pack and output table fields */
    fieldOutput("%d%d%d%d",serdesNum,inFields[1],inFields[2],inFields[3]);
    galtisOutput(outArgs, rc, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesTestGeneratorStatusGetNextEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesTestGeneratorStatusGetNextEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesTestGeneratorStatusGetFirstEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesTestGeneratorStatus1GetFirstEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      portGroup;
    GT_U32      serdesNum;
    GT_BOOL     counterAccumulateMode;
    MV_HWS_SERDES_TX_PATTERN txP;
    MV_HWS_SERDES_TEST_GEN_STATUS status;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    serdesNum = (GT_U32)inArgs[2];
    txP = (MV_HWS_SERDES_TX_PATTERN)inArgs[3];
    counterAccumulateMode = (GT_BOOL)inArgs[4];

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    rc = mvHwsSerdesTestGenStatus(devNum, portGroup, serdesNum,(MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum),
                                  txP, counterAccumulateMode, &status);

    inFields[1] = status.errorsCntr;
    inFields[3] = status.lockStatus;
    inFields[2] = status.txFramesCntr.l[0];

    /* pack and output table fields */
    fieldOutput("%d%d%d%d",serdesNum,inFields[1],inFields[2],inFields[3]);
    galtisOutput(outArgs, rc, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesTestGeneratorStatusGetNextEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesTestGeneratorStatus1GetNextEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}


static GT_U32 seqCurLine;
/*GT_U8 serdesDevNum = 0;

GT_VOID SetSerdesDevNum(GT_U8 devNum)
{
    serdesDevNum = devNum;
    cpssOsPrintf("Dev Num was set to %d\n", serdesDevNum);
}*/
/*******************************************************************************
* wrHwsSerdesSeqCfgGetFirst
*
* DESCRIPTION:
*
* INPUTS:
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesSeqCfgGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    MV_OP_PARAMS    cfgEl;
    MV_HWS_SERDES_TYPE  serdesType;
    GT_U8           devNum;
    GT_U32          baseAddr, unitIndex;

    GT_UNUSED_PARAM(numFields);

    seqCurLine = 0;
    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, 0));

    rc = hwsSerdesSeqGet(globalDevNum, devNum,(MV_HWS_COM_PHY_H_SUB_SEQ)inArgs[1], 0, &cfgEl);

    inFields[0] = 0;
    inFields[1] = WRITE_OP/*cfgEl.op*/;

    if (serdesType < COM_PHY_28NM)
    {
        switch(inFields[1])
        {
        case WRITE_OP:
          mvUnitInfoGet(globalDevNum, (MV_HWS_UNITS_ID)cfgEl.unitId, &baseAddr, &unitIndex);
          inFields[2] = baseAddr;
          inFields[3] = cfgEl.regOffset;
          inFields[4] = cfgEl.operData;
          inFields[5] = cfgEl.mask;
          inFields[6] = 0;
          inFields[7] = 0;
          break;
        case DELAY_OP:
          inFields[2] = 0;
          inFields[3] = 0;
          inFields[4] = 0;
          inFields[5] = 0;
          inFields[6] = cfgEl.operData;
          inFields[7] = 0;
          break;
        case POLLING_OP:
          mvUnitInfoGet(globalDevNum, (MV_HWS_UNITS_ID)cfgEl.unitId, &baseAddr, &unitIndex);
          inFields[2] = baseAddr;
          inFields[3] = cfgEl.regOffset;
          inFields[4] = cfgEl.operData;
          inFields[5] = cfgEl.mask;
          inFields[6] = 0;
          inFields[7] = 0;
          break;
        }
    }
    else
    {
        switch(inFields[1])
        {
        case WRITE_OP:
            mvUnitInfoGet(globalDevNum, (MV_HWS_UNITS_ID)cfgEl.unitId, &baseAddr, &unitIndex);
            inFields[2] = baseAddr;
            inFields[3] = cfgEl.regOffset;
            inFields[4] = cfgEl.operData;
            inFields[5] = cfgEl.mask;
            inFields[6] = 0;
            inFields[7] = 0;
            break;
        case DELAY_OP:
            inFields[2] = 0;
            inFields[3] = 0;
            inFields[4] = 0;
            inFields[5] = 0;
            inFields[6] = 0;
            inFields[7] = 0;
            break;
        case POLLING_OP:
            mvUnitInfoGet(globalDevNum, (MV_HWS_UNITS_ID)cfgEl.unitId, &baseAddr, &unitIndex);
            inFields[2] = baseAddr;
            inFields[3] = cfgEl.regOffset;
            inFields[4] = cfgEl.operData;
            inFields[5] = cfgEl.mask;
            inFields[6] = 0;
            inFields[7] = 0;
            break;
        }
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d",inFields[0],inFields[1],inFields[2],inFields[3],inFields[4],inFields[5],inFields[6],inFields[7]);
    galtisOutput(outArgs, rc, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesSeqCfgGetNext
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesSeqCfgGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    MV_HWS_SERDES_TYPE serdesType;
    MV_OP_PARAMS    cfgEl;
    GT_U8           devNum;
    GT_U32          baseAddr, unitIndex;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    inFields[0] = ++seqCurLine;

    devNum = (GT_U8)inArgs[0];
    serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, 0));

    rc = hwsSerdesSeqGet(globalDevNum, devNum,(MV_HWS_COM_PHY_H_SUB_SEQ)inArgs[1], seqCurLine, &cfgEl);
    if (rc == GT_NO_MORE)
    {
      galtisOutput(outArgs, GT_OK, "%d", -1);
      return CMD_OK;
    }

    inFields[1] = WRITE_OP/*cfgEl.op*/;

    if (serdesType < COM_PHY_28NM)
    {
        switch(inFields[1])
        {
        case WRITE_OP:
          mvUnitInfoGet(globalDevNum, (MV_HWS_UNITS_ID)cfgEl.unitId, &baseAddr, &unitIndex);
          inFields[2] = baseAddr;
          inFields[3] = cfgEl.regOffset;
          inFields[4] = cfgEl.operData;
          inFields[5] = cfgEl.mask;
          inFields[6] = 0;
          inFields[7] = 0;
          break;
        case DELAY_OP:
          inFields[2] = 0;
          inFields[3] = 0;
          inFields[4] = 0;
          inFields[5] = 0;
          inFields[6] = cfgEl.operData;
          inFields[7] = 0;
          break;
        case POLLING_OP:
          mvUnitInfoGet(globalDevNum, (MV_HWS_UNITS_ID)cfgEl.unitId, &baseAddr, &unitIndex);
          inFields[2] = baseAddr;
          inFields[3] = cfgEl.regOffset;
          inFields[4] = cfgEl.operData;
          inFields[5] = cfgEl.mask;
          inFields[6] = 0;
          inFields[7] = 0;
          break;
        }
    }
    else
    {
        switch(inFields[1])
        {
        case WRITE_OP:
            mvUnitInfoGet(globalDevNum, (MV_HWS_UNITS_ID)cfgEl.unitId, &baseAddr, &unitIndex);
            inFields[2] = baseAddr;
            inFields[3] = cfgEl.regOffset;
            inFields[4] = cfgEl.operData;
            inFields[5] = cfgEl.mask;
            inFields[6] = 0;
            inFields[7] = 0;
            break;
        case DELAY_OP:
            inFields[2] = 0;
            inFields[3] = 0;
            inFields[4] = 0;
            inFields[5] = 0;
            inFields[6] = 0;
            inFields[7] = 0;
            break;
        case POLLING_OP:
            mvUnitInfoGet(globalDevNum, (MV_HWS_UNITS_ID)cfgEl.unitId, &baseAddr, &unitIndex);
            inFields[2] = baseAddr;
            inFields[3] = cfgEl.regOffset;
            inFields[4] = cfgEl.operData;
            inFields[5] = cfgEl.mask;
            inFields[6] = 0;
            inFields[7] = 0;
            break;
        }
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d",inFields[0],inFields[1],inFields[2],inFields[3],inFields[4],inFields[5],inFields[6],inFields[7]);
    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;

}

/*******************************************************************************
* wrHwsSerdesSeqCfgSet
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesSeqCfgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    MV_HWS_SERDES_TYPE serdesType;
    MV_OP_PARAMS         cfgEl;
    GT_U8                devNum;
    MV_HWS_UNITS_ID      unitId;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    serdesType = (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, 0);

    if (serdesType < COM_PHY_28NM)
    {
        switch(inFields[1])
        {
        case WRITE_OP:
          mvUnitInfoGetByAddr(globalDevNum, (GT_U32)inFields[2], &unitId);
          cfgEl.unitId = unitId;
          cfgEl.regOffset = (GT_U32)inFields[3];
          cfgEl.operData = (GT_U32)inFields[4];
          cfgEl.mask = (GT_U32)inFields[5];
/*          cfgEl.op = WRITE_OP;*/
          break;
        case DELAY_OP:
          cfgEl.operData = (GT_U16)inFields[6];
/*          cfgEl.op = DELAY_OP;*/
          break;
        case POLLING_OP:
          mvUnitInfoGetByAddr(globalDevNum, (GT_U32)inFields[2], &unitId);
          cfgEl.unitId = unitId;
          cfgEl.regOffset = (GT_U32)inFields[3];
          cfgEl.operData = (GT_U32)inFields[4];
          cfgEl.mask = (GT_U32)inFields[5];
/*          cfgEl.op = POLLING_OP;*/
          break;
        }
    }
    else
    {
        switch(inFields[1])
        {
        case WRITE_OP:
            mvUnitInfoGetByAddr(globalDevNum, (GT_U32)inFields[2], &unitId);
            cfgEl.unitId = unitId;
            cfgEl.regOffset = (GT_U32)inFields[3];
            cfgEl.operData = (GT_U32)inFields[4];
            cfgEl.mask = (GT_U32)inFields[5];
/*            cfgEl.op = WRITE_OP;*/
            break;
        case DELAY_OP:
/*            cfgEl.op = DELAY_OP;*/
            break;
        case POLLING_OP:
            mvUnitInfoGetByAddr(globalDevNum, (GT_U32)inFields[2], &unitId);
            cfgEl.unitId = unitId;
            cfgEl.regOffset = (GT_U32)inFields[3];
            cfgEl.operData = (GT_U32)inFields[4];
            cfgEl.mask = (GT_U32)inFields[5];
/*            cfgEl.op = POLLING_OP;*/
            break;
        }
    }

    rc = hwsSerdesSeqSet(globalDevNum, devNum,((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
      (MV_HWS_COM_PHY_H_SUB_SEQ)inArgs[1], &cfgEl, (GT_U32)inFields[0]);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;

}

static GT_U32 wrCalibSerdesNum;

/*******************************************************************************
* wrSerdesCalibrationGetFirst
*
* DESCRIPTION:
*
* INPUTS:
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrSerdesCalibrationGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;
    GT_32      i;
    GT_U32  DevNum;
    GT_U32  SerdesNumStart;
    GT_U32  SerdesNumEnd;
    MV_HWS_CALIBRATION_RESULTS  results;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    /* Parameters List */
    DevNum         = (GT_U32)inArgs[0];
    SerdesNumStart = (GT_U32)inArgs[1];
    SerdesNumEnd   = (GT_U32)inArgs[2];


    if (SerdesNumEnd <  SerdesNumStart)
    {
        SerdesNumEnd = SerdesNumStart;
    }

    wrCalibSerdesNum = SerdesNumStart;
    res = mvHwsSerdesCalibrationStatus(globalDevNum, DevNum,  wrCalibSerdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, wrCalibSerdesNum), &results);

    if (res != GT_OK)
    {
        galtisOutput(outArgs, GT_FAIL, "%f");
        return CMD_OK;
    }

    /* Fields List */
    inFields[0] = (GT_32)wrCalibSerdesNum;      /*SerdesNum*/
    inFields[1] = (GT_32)results.calDone;       /*Cal_Done*/
    inFields[2] = (GT_32)results.txImpCal;      /*TxImpCal*/
    inFields[3] = (GT_32)results.rxImpCal;      /*RxImpCal*/
    inFields[4] = (GT_32)results.ProcessCal;    /*ProcessCal*/
    inFields[5] = (GT_32)results.speedPll;      /*SpeedPll*/
    inFields[6] = (GT_32)results.sellvTxClk;    /*SELLV_TxClk*/
    inFields[7] = (GT_32)results.sellvTxData;   /*SELLV_TxData*/
    inFields[8] = (GT_32)results.sellvTxIntp;   /*SELLV_TxIntp*/
    inFields[9] = (GT_32)results.sellvTxDrv;    /*SELLV_TxDrv*/
    inFields[10] = (GT_32)results.sellvTxDig;   /*SELLV_RxDig*/
    inFields[11] = (GT_32)results.sellvRxSample;/*SELLV_RxSampl*/
    inFields[12] = (GT_32)results.ffeDone;      /*FFE_Done*/

    for (i = 0; i < 8; i++)
    {
        inFields[13+i] = results.ffeCal[i];     /*FFE_Cal0[i]*/
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15], inFields[16], inFields[17], inFields[18], inFields[19], inFields[20]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrSerdesCalibrationGetNext
*
* DESCRIPTION:
*
* INPUTS:
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrSerdesCalibrationGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;
    GT_32        i;
    GT_U32  DevNum;
    GT_U32  SerdesNumEnd;
    MV_HWS_CALIBRATION_RESULTS  results;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    /* Parameters List */
    DevNum         = (GT_U32)inArgs[0];
    SerdesNumEnd   = (GT_U32)inArgs[2];

    wrCalibSerdesNum++;

    if (wrCalibSerdesNum > SerdesNumEnd)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    res = mvHwsSerdesCalibrationStatus(globalDevNum, DevNum, wrCalibSerdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(globalDevNum, wrCalibSerdesNum), &results);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, GT_FAIL, "%f");
        return CMD_OK;
    }

    /* Fields List */
    inFields[0] = (GT_32)wrCalibSerdesNum;      /*SerdesNum*/
    inFields[1] = (GT_32)results.calDone;       /*Cal_Done*/
    inFields[2] = (GT_32)results.txImpCal;      /*TxImpCal*/
    inFields[3] = (GT_32)results.rxImpCal;      /*RxImpCal*/
    inFields[4] = (GT_32)results.ProcessCal;    /*ProcessCal*/
    inFields[5] = (GT_32)results.speedPll;      /*SpeedPll*/
    inFields[6] = (GT_32)results.sellvTxClk;    /*SELLV_TxClk*/
    inFields[7] = (GT_32)results.sellvTxData;   /*SELLV_TxData*/
    inFields[8] = (GT_32)results.sellvTxIntp;   /*SELLV_TxIntp*/
    inFields[9] = (GT_32)results.sellvTxDrv;    /*SELLV_TxDrv*/
    inFields[10] = (GT_32)results.sellvTxDig;   /*SELLV_RxDig*/
    inFields[11] = (GT_32)results.sellvRxSample;/*SELLV_RxSampl*/
    inFields[12] = (GT_32)results.ffeDone;      /*FFE_Done*/

    for (i = 0; i < 8; i++)
    {
        inFields[13+i] = results.ffeCal[i];     /*FFE_Cal0[i]*/
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15], inFields[16], inFields[17], inFields[18], inFields[19], inFields[20]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesScanSamplerGetFirst
*
* DESCRIPTION:
*
* INPUTS:
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesScanSamplerGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  retVal;

    /* Parameters List */
    GT_U8   DevNum;
    GT_U8   CoreNum;
    GT_U32  SerdesNum;
    GT_U32  WaitTime;
    GT_U32  SamplerSelect;
    GT_U32  Threshold;
    MV_HWS_SAMPLER_RESULTS samplerResults;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    DevNum        = (GT_U8)inArgs[0];
    CoreNum       = (GT_U8)inArgs[1];
    SerdesNum     = (GT_U32)inArgs[2];
    WaitTime      = (GT_U32)inArgs[3];
    SamplerSelect = (GT_U32)inArgs[4];
    Threshold     = (GT_U32)inArgs[5];

    cpssOsPrintf("ScanSampler: DevNum %d SerdesNum %d, WaitTime %d, SamplerSelect %d Threshold %d\n",
             DevNum, SerdesNum, WaitTime, SamplerSelect, Threshold);

    retVal = mvHwsSerdesScanSampler(DevNum, CoreNum, SerdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(DevNum, SerdesNum),
                                    WaitTime, SamplerSelect, Threshold, &samplerResults);

    if (retVal != GT_OK)
    {
        galtisOutput(outArgs, GT_FAIL, "%f");
        return CMD_OK;
    }

    /* Fields List */
    inFields[0] = samplerResults.bestVal;

    /* pack and output table fields */
    fieldOutput("%d", inFields[0]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesScanSamplerGetNext
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesScanSamplerGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

static GT_U32 SerdesScanSamplerSerdesNum;
#define MAX_SAMPLER_NUM                7

/*******************************************************************************
* wrHwsSerdesScanSamplerExtGetFirst
*
* DESCRIPTION:
*
* INPUTS:
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesScanSamplerExtGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32  i;
    GT_STATUS  retVal;

        /* Parameters List */
    GT_U8   DevNum        = (GT_U8)inArgs[0];
    GT_U32  SerdesStart   = (GT_U32)inArgs[2];
    GT_U32  SerdesEnd     = (GT_U32)inArgs[3];
    GT_U32  WaitTime      = (GT_U32)inArgs[4];
    GT_U32  SamplerStart  = (GT_U32)inArgs[5];
    GT_U32  SamplerEnd    = (GT_U32)inArgs[6];
    GT_U32  Thershold     = (GT_U32)inArgs[7];

    MV_HWS_SAMPLER_RESULTS samplerResults[MAX_SAMPLER_NUM];

    GT_UNUSED_PARAM(numFields);

        /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    cpssOsMemSet(samplerResults, 0, sizeof(MV_HWS_SAMPLER_RESULTS) * MAX_SAMPLER_NUM);

        SerdesScanSamplerSerdesNum = SerdesStart;

    cpssOsPrintf("ScanSampler (2): DevNum %d SerdesStart %d, SerdesEnd %d, WaitTime %d, SamplerStart %d SamplerEnd %d, Thershold %d\n",
             globalDevNum, SerdesStart, SerdesEnd, WaitTime, SamplerStart, SamplerEnd, Thershold);

        if(SamplerStart > SamplerEnd)
        {
                cpssOsPrintf("changing SamplerEnd to SamplerStart (%d)\n", SamplerStart);
                SamplerEnd = SamplerStart;
        }

        for(i = SamplerStart; i < (SamplerEnd + 1); i++)
        {
                cpssOsPrintf("Serdes %d: Scan sampler %d\n", SerdesScanSamplerSerdesNum, i);
                retVal = mvHwsSerdesScanSampler(globalDevNum, DevNum, SerdesScanSamplerSerdesNum,
                                                (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(DevNum, SerdesScanSamplerSerdesNum), WaitTime, i, Thershold, &samplerResults[i]);
                if(retVal != GT_OK)
                {
                        cpssOsPrintf("Serdes %d: Failed to scan sampler %d (res 0x%x)\n", SerdesScanSamplerSerdesNum, i, retVal);
                        galtisOutput(outArgs, retVal, "%f");
                        return CMD_OK;
                }
        }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", SerdesScanSamplerSerdesNum,
                                                                        samplerResults[0].bestVal, samplerResults[0].positiveVal, samplerResults[0].negativeVal,
                                                                        samplerResults[1].bestVal, samplerResults[1].positiveVal, samplerResults[1].negativeVal,
                                                                        samplerResults[2].bestVal, samplerResults[2].positiveVal, samplerResults[2].negativeVal,
                                                                        samplerResults[3].bestVal, samplerResults[3].positiveVal, samplerResults[3].negativeVal,
                                                                        samplerResults[4].bestVal, samplerResults[4].positiveVal, samplerResults[4].negativeVal,
                                                                        samplerResults[5].bestVal, samplerResults[5].positiveVal, samplerResults[5].negativeVal,
                                                                        samplerResults[6].bestVal, samplerResults[6].positiveVal, samplerResults[6].negativeVal);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesScanSamplerExtGetNext
*
* DESCRIPTION:
*
* INPUTS:
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesScanSamplerExtGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32  i;
    GT_STATUS  retVal;

        /* Parameters List */
    GT_U8   DevNum;
    GT_U32  SerdesEnd;
    GT_U32  WaitTime;
    GT_U32  SamplerStart;
    GT_U32  SamplerEnd;
    GT_U32  Thershold;

    MV_HWS_SAMPLER_RESULTS samplerResults[MAX_SAMPLER_NUM];

    GT_UNUSED_PARAM(numFields);

        /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    DevNum        = (GT_U8)inArgs[0];
    SerdesEnd     = (GT_U32)inArgs[3];
    WaitTime      = (GT_U32)inArgs[4];
    SamplerStart  = (GT_U32)inArgs[5];
    SamplerEnd    = (GT_U32)inArgs[6];
    Thershold     = (GT_U32)inArgs[7];

        SerdesScanSamplerSerdesNum++;

        if (SerdesScanSamplerSerdesNum > SerdesEnd)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

        if(SamplerStart > SamplerEnd)
        {
                SamplerEnd = SamplerStart;
        }

        for(i = SamplerStart; i < (SamplerEnd + 1); i++)
        {
                cpssOsPrintf("Serdes (2) %d: Scan sampler %d\n", SerdesScanSamplerSerdesNum, i);
                retVal = mvHwsSerdesScanSampler(globalDevNum, DevNum, SerdesScanSamplerSerdesNum,
                                                (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(DevNum, SerdesScanSamplerSerdesNum), WaitTime, i, Thershold, &samplerResults[i]);
                if(retVal != GT_OK)
                {
                        cpssOsPrintf("Serdes %d: Failed to scan sampler %d (res 0x%x)\n", SerdesScanSamplerSerdesNum, i, retVal);
                        galtisOutput(outArgs, retVal, "%f");
                        return CMD_OK;
                }
        }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", SerdesScanSamplerSerdesNum,
                                                                        samplerResults[0].bestVal, samplerResults[0].positiveVal, samplerResults[0].negativeVal,
                                                                        samplerResults[1].bestVal, samplerResults[1].positiveVal, samplerResults[1].negativeVal,
                                                                        samplerResults[2].bestVal, samplerResults[2].positiveVal, samplerResults[2].negativeVal,
                                                                        samplerResults[3].bestVal, samplerResults[3].positiveVal, samplerResults[3].negativeVal,
                                                                        samplerResults[4].bestVal, samplerResults[4].positiveVal, samplerResults[4].negativeVal,
                                                                        samplerResults[5].bestVal, samplerResults[5].positiveVal, samplerResults[5].negativeVal,
                                                                        samplerResults[6].bestVal, samplerResults[6].positiveVal, samplerResults[6].negativeVal);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesEomGetGetFirst
*
* DESCRIPTION:
*
* INPUTS:
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesEomGetGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MV_HWS_SERDES_EOM_RES   results;
    GT_STATUS  retVal;

    /* Parameters List */
    GT_U8   DevNum;
    GT_U8   CoreNum;
    GT_U32  SerdesNum;
    GT_U32  WaitTime;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    DevNum        = (GT_U8)inArgs[0];
    CoreNum       = (GT_U8)inArgs[1];
    SerdesNum     = (GT_U32)inArgs[2];
    WaitTime      = (GT_U32)inArgs[3];

    cpssOsPrintf("EOM Get: DevNum %d CoreNum %d SerdesNum %d, WaitTime\n",
                                 DevNum, CoreNum, SerdesNum, WaitTime);
    results.upperMatrix = eomUpperMatrix;
    results.lowerMatrix = eomLowerMatrix;
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(DevNum,SerdesNum);
    retVal = mvHwsSerdesEOMGet(DevNum, CoreNum, SerdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(DevNum, SerdesNum), WaitTime, &results);

    if (retVal != GT_OK)
    {
        galtisOutput(outArgs, GT_FAIL, "%f");
        return CMD_OK;
    }

    /* Fields List */
    inFields[0] = results.eomMechanism;
    inFields[1] = results.horizontalEye;
    inFields[2] = results.verticalEye;

    /* pack and output table fields */
    fieldOutput("%d%d%d", inFields[0], inFields[1], inFields[2]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesEomGetGetNext
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesEomGetGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesRegAccessGetFirst
*
* DESCRIPTION:
*
* INPUTS:
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesRegAccessGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32 address;
    GT_U32 unitAddr = 0;
    GT_U32 unitIndex = 0;
    GT_U32 data;
    GT_STATUS res;

    /* Parameters List */
    GT_U8  DevNum;
    GT_U32 SerdesNum;
    GT_U32 RegAddress;
    GT_U32 Mask;

    GT_UNUSED_PARAM(numFields);

        /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    DevNum     = (GT_U8)inArgs[0];
    SerdesNum  = (GT_U8)inArgs[1];
    RegAddress = (GT_U32)inArgs[2];
    Mask       = (GT_U32)inArgs[3];

    if (Mask == 0)
    {
        Mask = 0xFFFFFFFF;
    }

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(DevNum,SerdesNum);

    /* get unit base address and unit index for current device */
    mvUnitExtInfoGet(DevNum, SERDES_UNIT, SerdesNum, &unitAddr, &unitIndex , &SerdesNum);
    if ((unitAddr == 0) || (unitIndex == 0))
    {
        cpssOsPrintf("\nUnable to get the base address and unit index for current device, unitAddr or unitIndex is 0\n");
        galtisOutput(outArgs, GT_FAIL, "%f");
        return CMD_OK;
    }
    address = unitAddr + unitIndex * SerdesNum + RegAddress;

    res = hwsRegisterGetFuncPtr(DevNum, globalPortGroup, address, &data, Mask);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, res, "%f");
        return CMD_OK;
    }

    /* Fields List */
    inFields[0] = data;

    /* pack and output table fields */
    fieldOutput("%d", inFields[0]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesRegAccessGetNext
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesRegAccessGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesRegAccessSet
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesRegAccessSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32 address;
    GT_U32 unitAddr = 0;
    GT_U32 unitIndex = 0;
    GT_STATUS res;

    /* Parameters List */
    GT_U8   DevNum;
    GT_U32  SerdesNum;
    GT_U32  RegAddress;
    GT_U32  Mask;
    GT_U32  Data;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    DevNum     = (GT_U8)inArgs[0];
    SerdesNum  = (GT_U8)inArgs[1];
    RegAddress = (GT_U32)inArgs[2];
    Mask       = (GT_U32)inArgs[3];
    Data       = (GT_U32)inFields[0];

    if (Mask == 0)
    {
        Mask = 0xFFFFFFFF;
    }

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(DevNum,SerdesNum);

    /* get unit base address and unit index for current device */
    mvUnitExtInfoGet(DevNum, SERDES_UNIT, SerdesNum, &unitAddr, &unitIndex , &SerdesNum);
    if ((unitAddr == 0) || (unitIndex == 0))
    {
        cpssOsPrintf("\nUnable to get the base address and unit index for current device, unitAddr or unitIndex is 0\n");
        galtisOutput(outArgs, GT_FAIL, "%f");
        return CMD_OK;
    }
    address = unitAddr + unitIndex * SerdesNum + RegAddress;

    res = hwsRegisterSetFuncPtr(DevNum, globalPortGroup, address, Data, Mask);

    galtisOutput(outArgs, res, "");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvagoSerdesEnhanceTuneSet
*
* DESCRIPTION:
*       Set the ICAL with shifted sampling point to find best sampling point
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesArr - collection of SERDESes to configure
*       numOfSerdes - number of SERDESes to configure
*       min_LF      - Minimum LF value that can be set on Serdes
*       max_LF      - Maximum LF value that can be set on Serdes
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvagoSerdesEnhanceTuneSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;

    /* Parameters List */
    GT_U32  devNum    = (GT_U32)inArgs[0];
    GT_U32  serdesNum = (GT_U32)inArgs[1];

    /* Fields List */
    GT_U8  minLF = (GT_U8)inFields[0];
    GT_U8  maxLF = (GT_U8)inFields[1];

    GT_UNUSED_PARAM(numFields);

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    res = mvHwsAvagoSerdesEnhanceTune(devNum, globalPortGroup, &serdesNum, 1, minLF, maxLF);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, res, "");
        return CMD_OK;
    }

    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvagoSerdesSpicoInterruptSet
*
* DESCRIPTION: Issue the interrupt to the Spico processor
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       interruptCode - Code of interrupt
*       interruptData - Data to write
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvagoSerdesSpicoInterruptSetEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
#ifndef ASIC_SIMULATION
    GT_STATUS  res;

    /* Parameters List */
    GT_U32  devNum        = (GT_U32)inArgs[0];
    GT_U32  serdesNum     = (GT_U32)inArgs[1];
    GT_U32  interruptCode = (GT_U32)inArgs[2];
    GT_U32  interruptData = (GT_U32)inArgs[3];

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    res = mvHwsAvagoSerdesSpicoInterrupt(devNum, globalPortGroup, serdesNum, interruptCode, interruptData, NULL);

    galtisOutput(outArgs, res, "");
#else

    galtisOutput(outArgs, GT_OK, "");
#endif /* ASIC_SIMULATION */

    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvagoSerdesSpicoInterruptGetFirst
*
* DESCRIPTION:
*       Get the interrupt from the Spico processor
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       interruptCode - Code of interrupt
*       interruptData - Data from interrupt
*
* OUTPUTS:
*       interruptResult - return value of Spico interrupt
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvagoSerdesSpicoInterruptGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
#ifndef ASIC_SIMULATION
    GT_STATUS   res;
    int         interruptResult;

    /* Parameters List */
    GT_U32  devNum        = (GT_U32)inArgs[0];
    GT_U32  serdesNum     = (GT_U32)inArgs[1];
    GT_U32  interruptCode = (GT_U32)inArgs[2];
    GT_U32  interruptData = (GT_U32)inArgs[3];

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);
    res = mvHwsAvagoSerdesSpicoInterrupt(devNum, globalPortGroup, serdesNum, interruptCode, interruptData, &interruptResult);

    /* pack and output table fields */
    fieldOutput("%d", interruptResult);

    galtisOutput(outArgs, res, "%f");
#else

    /* set default value for Output field */
    fieldOutput("%d", 0);
    galtisOutput(outArgs, GT_OK, "");
#endif /* ASIC_SIMULATION */

    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvagoSerdesSpicoInterruptGetNext
*
* DESCRIPTION:
*       Get the interrupt from the Spico processor
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       interruptCode - Code of interrupt
*
* OUTPUTS:
*       interruptData - Data from interrupt
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvagoSerdesSpicoInterruptGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvagoSerdesShiftSamplePointSet
*
* DESCRIPTION:
*       Shift Serdes sampling point earlier in time
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       delay     - set the delay (0-0xF)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvagoSerdesShiftSamplePointSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;

    /* Parameters List */
    GT_U32  devNum    = (GT_U32)inArgs[0];
    GT_U32  serdesNum = (GT_U32)inArgs[1];

    /* Fields List */
    GT_U32  delay = (GT_U32)inFields[0];
    GT_UNUSED_PARAM(numFields);

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);
    res = mvHwsAvagoSerdesShiftSamplePoint(devNum, globalPortGroup, serdesNum, delay);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, res, "");
        return CMD_OK;
    }

    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvagoSerdesManualCtleCfgSet
*
* DESCRIPTION:
*       Set the Serdes Manual CTLE config for DFE
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       dcGain    - DC-Gain value
*       lowFrequency  - CTLE Low-Frequency
*       highFrequency - CTLE High-Frequency
*       bandWidth     - CTLE Band-width
*       squelch       - Signal OK threshold
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvagoSerdesManualCtleCfgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    ctleConfig;
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT rxConfig;

    /* Parameters List */
    GT_U32  devNum    = (GT_U32)inArgs[0];
    GT_U32  serdesNum = (GT_U32)inArgs[1];

    GT_UNUSED_PARAM(numFields);

    hwsOsMemSetFuncPtr(& ctleConfig, 0, sizeof(ctleConfig));

    /* Fields List */
    ctleConfig.highFrequency  = (GT_U32)inFields[0];
    ctleConfig.lowFrequency   = (GT_U32)inFields[1];
    ctleConfig.dcGain         = (GT_U32)inFields[2];
    ctleConfig.bandWidth      = (GT_U32)inFields[3];
    ctleConfig.squelch        = (GT_U32)inFields[5];

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    hwsOsMemCopyFuncPtr(&rxConfig.rxAvago, &ctleConfig, sizeof(MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA));
    res = mvHwsAvagoSerdesManualCtleConfig(devNum, globalPortGroup, serdesNum, &rxConfig);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, res, "");
        return CMD_OK;
    }

    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvagoSerdesManualCtleCfg1Set
*
* DESCRIPTION:
*       Set the Serdes Manual CTLE config for DFE
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*         dcGain         - DC-Gain              (rang: 0-255)
*         lowFrequency   - CTLE Low-Frequency   (rang: 0-15)
*         highFrequency  - CTLE High-Frequency  (rang: 0-15)
*         bandWidth      - CTLE Band-width      (rang: 0-15)
*         squelch        - Signal OK threshold  (rang: 0-308)
*         gainshape1     - CTLE gainshape1      (rang: 0-3)
*         gainshape2     - CTLE gainshape2      (rang: 0-3)
*         shortChannelEn - Enable/Disable Short channel
*         dfeGAIN        - DFE Gain Tap strength  (rang: 0-255)
*         dfeGAIN2       - DFE Gain Tap2 strength (rang: 0-255)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvagoSerdesManualCtleCfg1Set
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    ctleConfig;
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT rxConfig;

    /* Parameters List */
    GT_U32  devNum    = (GT_U32)inArgs[0];
    GT_U32  serdesNum = (GT_U32)inArgs[1];

    GT_UNUSED_PARAM(numFields);

    /* Fields List */
    ctleConfig.highFrequency  = (GT_U32)inFields[0];
    ctleConfig.lowFrequency   = (GT_U32)inFields[1];
    ctleConfig.dcGain         = (GT_U32)inFields[2];
    ctleConfig.bandWidth      = (GT_U32)inFields[3];
    ctleConfig.squelch        = (GT_U32)inFields[4];
    ctleConfig.gainshape1     = (GT_U32)inFields[5];
    ctleConfig.gainshape2     = (GT_U32)inFields[6];
    ctleConfig.dfeGAIN        = (GT_U32)inFields[7];
    ctleConfig.dfeGAIN2       = (GT_U32)inFields[8];
    ctleConfig.shortChannelEn = (GT_BOOL)inFields[9];

    if ((HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO) &&
       ((ctleConfig.gainshape1     != 0) ||
        (ctleConfig.gainshape2     != 0) ||
        (ctleConfig.dfeGAIN        != 0) ||
        (ctleConfig.dfeGAIN2       != 0) ||
        (ctleConfig.shortChannelEn != 0)))
    {
        cpssOsPrintf("For Avago 28nm: gainshape1, gainshape2, dfeGAIN, dfeGAIN2, shortChannelEn are not supported!!\n");
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    hwsOsMemCopyFuncPtr(&rxConfig.rxAvago, &ctleConfig, sizeof(MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA));
    res = mvHwsAvagoSerdesManualCtleConfig(devNum, globalPortGroup, serdesNum, &rxConfig);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, res, "");
        return CMD_OK;
    }

    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/******************************************************************************
* wrHwsAvagoSerdesManualCtleCfg1Get
*
* DESCRIPTION:
*       Avago SerDes Rx-CTLE values get.
*
* INPUTS:
*       devNum  - physical device number
*       serdesNum - physical port number
*
* OUTPUTS:
*       dcGain         - DC-Gain              (rang: 0-255)
*       lowFrequency   - CTLE Low-Frequency   (rang: 0-15)
*       highFrequency  - CTLE High-Frequency  (rang: 0-15)
*       bandWidth      - CTLE Band-width      (rang: 0-15)
*       squelch        - Signal OK threshold  (rang: 0-308)
*       gainshape1     - CTLE gainshape1      (rang: 0-3)
*       gainshape2     - CTLE gainshape2      (rang: 0-3)
*       shortChannelEn - Enable/Disable Short channel
*       dfeGAIN        - DFE Gain Tap strength  (rang: 0-255)
*       dfeGAIN2       - DFE Gain Tap2 strength (rang: 0-255)
*
* RETURNS:
*     GT_OK        - on success.
*     GT_BAD_PARAM - on bad parameters
*     GT_FAIL      - on error
*
******************************************************************************/
static CMD_STATUS wrHwsAvagoSerdesManualCtleCfg1Get
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS rc;
    GT_U8     devNum;
    GT_U32    serdesNum;
    GT_U32    eoValue;
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    configParams;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum    = (GT_U8)inArgs[0];
    serdesNum = (GT_U32)inArgs[1];

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    rc = mvHwsAvagoSerdesManualCtleConfigGet(devNum, globalPortGroup, serdesNum, &configParams);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "mvHwsAvagoSerdesManualCtleConfigGet");
        return CMD_OK;
    }
    if(HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO_16NM)
    {
        rc = mvHwsAvago16nmSerdesHalGet(devNum, serdesNum, HWS_SERDES_PAM4_EYE, 0, (GT_32*)&eoValue);
        if (rc != GT_OK)
        {
            galtisOutput(outArgs, rc, "mvHwsAvago16nmSerdesHalGet");
            return CMD_OK;
        }
    }
    else
    {
        rc = mvHwsAvagoSerdesEyeMetricGet(devNum, globalPortGroup, serdesNum, &eoValue);
        if (rc != GT_OK)
        {
            galtisOutput(outArgs, rc, "mvHwsAvagoSerdesEyeMetricGet");
            return CMD_OK;
        }

    }

    inFields[0] = configParams.highFrequency;
    inFields[1] = configParams.lowFrequency;
    inFields[2] = configParams.dcGain;
    inFields[3] = configParams.bandWidth;
    inFields[4] = configParams.squelch;
    inFields[5] = configParams.gainshape1;
    inFields[6] = configParams.gainshape2;
    inFields[7] = configParams.dfeGAIN;
    inFields[8] = configParams.dfeGAIN2;
    inFields[9] = configParams.shortChannelEn;
    inFields[10] = (GT_32)eoValue;
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3], inFields[4],
                inFields[5], inFields[6], inFields[7], inFields[8], inFields[9], inFields[10]);

    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvagoSerdesAutoTuneResultsGetFirst
*
* DESCRIPTION:
*
* INPUTS:
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvagoSerdesAutoTuneResultsGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8       devNum;
    GT_U32      startSerdesNum;
    GT_U32      endSerdesNum;
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT results;
    int i;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = (GT_U8)inArgs[0];
    SerdesAutoTuneResultsSerdesNum = (GT_U32)inArgs[1];
    startSerdesNum = (GT_U32)inArgs[1];
    endSerdesNum = (GT_U32)inArgs[2];
    if (endSerdesNum < startSerdesNum)
    {
        endSerdesNum = startSerdesNum;
    }

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,SerdesAutoTuneResultsSerdesNum);

    rc = mvHwsSerdesAutoTuneResult(devNum, globalPortGroup, SerdesAutoTuneResultsSerdesNum,
                                   (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneResultsSerdesNum), &results);
    if (rc != GT_OK)
    {
        cpssOsPrintf("\nexecute mvHwsSerdesAutoTuneResult fail\n");
        galtisOutput(outArgs, rc, "");
        return CMD_OK;
    }


    if ((HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneResultsSerdesNum) == AVAGO) || (HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneResultsSerdesNum) == AVAGO_16NM))
    {
        for (i = 0; i < 13; i++)
        {
            inFields[1 + i] = (results.avagoResults.DFE[i] < 0) ? (GT_32)(0x80000000 - results.avagoResults.DFE[i]) : results.avagoResults.DFE[i]; /* for negative value */
        }
        inFields[13] = (GT_32)results.avagoResults.HF;
        inFields[14] = (GT_32)results.avagoResults.LF;
        inFields[15] = (GT_32)results.avagoResults.DC;
        inFields[16] = (GT_32)results.avagoResults.BW;
        inFields[17] = 0;
        inFields[18] = (GT_32)results.avagoResults.EO;

        inFields[19] = (GT_32)results.avagoResults.avagoStc.atten;
        inFields[20] = (GT_32)results.avagoResults.avagoStc.post;
        inFields[21] = (GT_32)results.avagoResults.avagoStc.pre;
        for (i = 0; i < 13; i++)
        {
            cpssOsPrintf("\nDFE%d is %d", i, results.avagoResults.DFE[i]);
        }
        cpssOsPrintf("\nHF: %d", results.avagoResults.HF);
        cpssOsPrintf("\nLF: %d", results.avagoResults.LF);
        cpssOsPrintf("\nDC: %d", results.avagoResults.DC);
        cpssOsPrintf("\nBW: %d", results.avagoResults.BW);
        cpssOsPrintf("\nEO: %d", results.avagoResults.EO);
    }
    else
    {
        inFields[19] = (GT_32)results.comphyHResults.txAmp;
        inFields[20] = (GT_32)results.comphyHResults.txEmph0;
        inFields[21] = (GT_32)results.comphyHResults.txEmph1;

        cpssOsPrintf("\nTX Amp/Atten:  %d",   inFields[19]);
        cpssOsPrintf("\nTX Emph0/Post: %d",   inFields[20]);
        cpssOsPrintf("\nTX Emph1/Pre:  %d\n", inFields[21]);


    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", SerdesAutoTuneResultsSerdesNum, inFields[1], inFields[2], inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15], inFields[16], inFields[17], inFields[18], inFields[19], inFields[20], inFields[21]);

    galtisOutput(outArgs, rc, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvagoSerdesAutoTuneResultsGetNext
*
* DESCRIPTION:
*
* INPUTS:
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvagoSerdesAutoTuneResultsGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8       devNum;
    GT_U32      endSerdesNum;
    GT_U32      serdesNum;
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT results;
    GT_U32      i;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    endSerdesNum = (GT_U32)inArgs[2];
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,SerdesAutoTuneResultsSerdesNum);
    serdesNum = SerdesAutoTuneResultsSerdesNum;

    /* get next index */
    serdesNum++;
    if (serdesNum > endSerdesNum)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = serdesNum;
    SerdesAutoTuneResultsSerdesNum = serdesNum;

    rc = mvHwsSerdesAutoTuneResult(devNum, globalPortGroup, serdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum), &results);
    if (rc != GT_OK)
    {
        cpssOsPrintf("\nexecute mvHwsSerdesAutoTuneResult fail\n");
        galtisOutput(outArgs, rc, "");
        return CMD_OK;
    }


    if ((HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO) || (HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO_16NM))
    {
        for (i = 0; i < 13; i++)
        {
            inFields[1 + i] = (results.avagoResults.DFE[i] < 0) ? (GT_32)(0x80000000 - results.avagoResults.DFE[i]) : results.avagoResults.DFE[i]; /* for negative value */
        }
        inFields[13] = (GT_32)results.avagoResults.HF;
        inFields[14] = (GT_32)results.avagoResults.LF;
        inFields[15] = (GT_32)results.avagoResults.DC;
        inFields[16] = (GT_32)results.avagoResults.BW;
        inFields[17] = 0;
        inFields[18] = (GT_32)results.avagoResults.EO;
        inFields[19] = (GT_32)results.avagoResults.avagoStc.atten;
        inFields[20] = (GT_32)results.avagoResults.avagoStc.post;
        inFields[21] = (GT_32)results.avagoResults.avagoStc.pre;
    }
    else
    {
        inFields[19] = (GT_32)results.comphyHResults.txAmp;
        inFields[20] = (GT_32)results.comphyHResults.txEmph0;
        inFields[21] = (GT_32)results.comphyHResults.txEmph1;
    }

    if ((HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO) || (HWS_DEV_SERDES_TYPE(devNum, serdesNum) == AVAGO_16NM))
    {
        for (i = 0; i < 13; i++)
        {
            cpssOsPrintf("\nDFE%d is %d", i, results.avagoResults.DFE[i]);
        }
        cpssOsPrintf("\nHF is %d", results.avagoResults.HF);
        cpssOsPrintf("\nLF is %d", results.avagoResults.LF);
        cpssOsPrintf("\nDC is %d", results.avagoResults.DC);
        cpssOsPrintf("\nBW is %d", results.avagoResults.BW);
        cpssOsPrintf("\nEO is %d", results.avagoResults.EO);

        cpssOsPrintf("\nTX Atten is %d", results.avagoResults.avagoStc.atten);
        cpssOsPrintf("\nTX Post is %d",  results.avagoResults.avagoStc.post);
        cpssOsPrintf("\nTX Pre is %d\n", results.avagoResults.avagoStc.pre);
    }
    else
    {
        cpssOsPrintf("\nTX Amp is %d",      results.comphyHResults.txAmp);
        cpssOsPrintf("\nTX Emph0 is %d",    results.comphyHResults.txEmph0);
        cpssOsPrintf("\nTX Emph1 is %d\n",  results.comphyHResults.txEmph1);
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15], inFields[16], inFields[17], inFields[18], inFields[19], inFields[20], inFields[21]);
    galtisOutput(outArgs, rc, "%f");
    return CMD_OK;
}


/*******************************************************************************
* wrHwsAvago16nmSerdesAutoTuneResultsGetFirst
*
* DESCRIPTION:
*
* INPUTS:
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvago16nmSerdesAutoTuneResultsGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8       devNum;
    GT_U32      portGroup;
    GT_U32      startSerdesNum;
    GT_U32      endSerdesNum;
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT results;
    int i;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    SerdesAutoTuneResultsSerdesNum = (GT_U32)inArgs[2];
    startSerdesNum = (GT_U32)inArgs[2];
    endSerdesNum = (GT_U32)inArgs[3];
    if (endSerdesNum < startSerdesNum)
    {
        endSerdesNum = startSerdesNum;
    }

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,SerdesAutoTuneResultsSerdesNum);
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,startSerdesNum);
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,endSerdesNum);

    rc = mvHwsAvago16nmSerdesAutoTuneResult(devNum, portGroup, SerdesAutoTuneResultsSerdesNum, &results);
    if (rc != GT_OK)
    {
        cpssOsPrintf("\nexecute mvHwsSerdesAutoTuneResult fail\n");
        galtisOutput(outArgs, rc, "");
        return CMD_OK;
    }

    for (i = 0; i < 13; i++)
    {
        inFields[1 + i] = results.avagoResults.DFE[i];
    }

    /* Avago Rx params: 28nm and 16nm */
    inFields[14] = results.avagoResults.HF;
    inFields[15] = results.avagoResults.LF;
    inFields[16] = results.avagoResults.DC;
    inFields[17] = results.avagoResults.BW;
    inFields[18] = (GT_32)results.avagoResults.EO;
    /* Avago Rx params: 16nm */
    inFields[19] = results.avagoResults.gainshape1;
    inFields[20] = results.avagoResults.gainshape2;

    /* AVAGO Rx-FFE params: 16nm */
    inFields[21] = results.avagoResults.rxffe_pre2;
    inFields[22] = results.avagoResults.rxffe_pre1;
    inFields[23] = results.avagoResults.rxffe_post1;
    inFields[24] = results.avagoResults.rxffe_bflf;
    inFields[25] = results.avagoResults.rxffe_bfhf;
    inFields[26] = results.avagoResults.rxffe_datarate;

    /* AVAGO Rx-Vernier params: 16nm */
    inFields[27] = results.avagoResults.vernier_upper_odd_dly;
    inFields[28] = results.avagoResults.vernier_upper_even_dly;
    inFields[29] = results.avagoResults.vernier_middle_odd_dly;
    inFields[30] = results.avagoResults.vernier_middle_even_dly;
    inFields[31] = results.avagoResults.vernier_lower_odd_dly;
    inFields[32] = results.avagoResults.vernier_lower_even_dly;
    inFields[33] = results.avagoResults.vernier_test_odd_dly;
    inFields[34] = results.avagoResults.vernier_test_even_dly;
    inFields[35] = results.avagoResults.vernier_edge_odd_dly;
    inFields[36] = results.avagoResults.vernier_edge_even_dly;
    inFields[37] = results.avagoResults.vernier_tap_dly;

    /* AVAGO Tx params: 28nm and 16nm */
    inFields[38] = results.avagoResults.avagoStc.atten;
    inFields[39] = results.avagoResults.avagoStc.post;
    inFields[40] = results.avagoResults.avagoStc.pre;
    inFields[41] = results.avagoResults.avagoStc.pre2;
    inFields[42] = results.avagoResults.avagoStc.pre3;

    /* AVAGO Rx params: 16nm */
    for (i = 0; i < CPSS_PAM4_EYES_ARRAY_SIZE_CNS; i++)
    {
        inFields[i + 43] = results.avagoResults.pam4EyesArr[i];
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                SerdesAutoTuneResultsSerdesNum,
                inFields[1], inFields[2], inFields[3], inFields[4],
                inFields[5], inFields[6], inFields[7], inFields[8], inFields[9],
                inFields[10], inFields[11], inFields[12], inFields[13], inFields[14],
                inFields[15], inFields[16], inFields[17], inFields[18], inFields[19],
                inFields[20], inFields[21], inFields[22], inFields[23], inFields[24],
                inFields[25], inFields[26], inFields[27], inFields[28], inFields[29],
                inFields[30], inFields[31], inFields[32], inFields[33], inFields[34],
                inFields[35], inFields[36], inFields[37], inFields[38], inFields[39],
                inFields[40], inFields[41], inFields[42], inFields[43], inFields[44],
                inFields[45], inFields[46], inFields[47], inFields[48]);

    galtisOutput(outArgs, rc, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvago16nmSerdesAutoTuneResultsGetNext
*
* DESCRIPTION:
*
* INPUTS:
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvago16nmSerdesAutoTuneResultsGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8       devNum;
    GT_U32      portGroup;
    GT_U32      endSerdesNum;
    GT_U32      serdesNum;
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT results;
    GT_U32      i;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    endSerdesNum = (GT_U32)inArgs[3];
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,endSerdesNum);
    serdesNum = SerdesAutoTuneResultsSerdesNum;

    /* get next index */
    serdesNum++;
    if (serdesNum > endSerdesNum)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = serdesNum;
    SerdesAutoTuneResultsSerdesNum = serdesNum;
    rc = mvHwsAvago16nmSerdesAutoTuneResult(devNum, portGroup, serdesNum, &results);
    if (rc != GT_OK)
    {
        cpssOsPrintf("\nexecute mvHwsSerdesAutoTuneResult fail\n");
        galtisOutput(outArgs, rc, "");
        return CMD_OK;
    }

    for (i = 0; i < 13; i++)
    {
        inFields[1 + i] = results.avagoResults.DFE[i];
    }

    /* Avago Rx params: 28nm and 16nm */
    inFields[14] = results.avagoResults.HF;
    inFields[15] = results.avagoResults.LF;
    inFields[16] = results.avagoResults.DC;
    inFields[17] = results.avagoResults.BW;
    inFields[18] = (GT_32)results.avagoResults.EO;
    /* Avago Rx params: 16nm */
    inFields[19] = results.avagoResults.gainshape1;
    inFields[20] = results.avagoResults.gainshape2;

    /* AVAGO Rx-FFE params: 16nm */
    inFields[21] = results.avagoResults.rxffe_pre2;
    inFields[22] = results.avagoResults.rxffe_pre1;
    inFields[23] = results.avagoResults.rxffe_post1;
    inFields[24] = results.avagoResults.rxffe_bflf;
    inFields[25] = results.avagoResults.rxffe_bfhf;
    inFields[26] = results.avagoResults.rxffe_datarate;

    /* AVAGO Rx-Vernier params: 16nm */
    inFields[27] = results.avagoResults.vernier_upper_odd_dly;
    inFields[28] = results.avagoResults.vernier_upper_even_dly;
    inFields[29] = results.avagoResults.vernier_middle_odd_dly;
    inFields[30] = results.avagoResults.vernier_middle_even_dly;
    inFields[31] = results.avagoResults.vernier_lower_odd_dly;
    inFields[32] = results.avagoResults.vernier_lower_even_dly;
    inFields[33] = results.avagoResults.vernier_test_odd_dly;
    inFields[34] = results.avagoResults.vernier_test_even_dly;
    inFields[35] = results.avagoResults.vernier_edge_odd_dly;
    inFields[36] = results.avagoResults.vernier_edge_even_dly;
    inFields[37] = results.avagoResults.vernier_tap_dly;

    /* AVAGO Tx params: 28nm and 16nm */
    inFields[38] = results.avagoResults.avagoStc.atten;
    inFields[39] = results.avagoResults.avagoStc.post;
    inFields[40] = results.avagoResults.avagoStc.pre;
    inFields[41] = results.avagoResults.avagoStc.pre2;
    inFields[42] = results.avagoResults.avagoStc.pre3;

    /* AVAGO Rx params: 16nm */
    for (i = 0; i < CPSS_PAM4_EYES_ARRAY_SIZE_CNS; i++)
    {
        /* fill indices 43-48 of inFields */
        inFields[i + 43] = results.avagoResults.pam4EyesArr[i];
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                SerdesAutoTuneResultsSerdesNum,
                inFields[1], inFields[2], inFields[3], inFields[4],
                inFields[5], inFields[6], inFields[7], inFields[8], inFields[9],
                inFields[10], inFields[11], inFields[12], inFields[13], inFields[14],
                inFields[15], inFields[16], inFields[17], inFields[18], inFields[19],
                inFields[20], inFields[21], inFields[22], inFields[23], inFields[24],
                inFields[25], inFields[26], inFields[27], inFields[28], inFields[29],
                inFields[30], inFields[31], inFields[32], inFields[33], inFields[34],
                inFields[35], inFields[36], inFields[37], inFields[38], inFields[39],
                inFields[40], inFields[41], inFields[42], inFields[43], inFields[44],
                inFields[45], inFields[46], inFields[47], inFields[48]);

    galtisOutput(outArgs, rc, "%f");
    return CMD_OK;
}

/**
* @internal wrHwsAvago16nmSerdesHalGet function
* @endinternal
*
* @brief   Retrieves the indicated HAL register value
*
* @param[in] devNum    - system device number
* @param[in] serdesNum - serdes number
* @param[in] halType   - HAL types
* @param[in] regSelect - HAL register selector
*
* @param[out] halRegValue - HAL register value
*
* @retval 0 - on success
* @retval 1 - on error
*/
CMD_STATUS wrHwsAvago16nmSerdesHalGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8   devNum    = (GT_U8)inArgs[0];
    GT_U32  serdesNum = (GT_U32)inArgs[1];
    MV_HWS_AVAGO_SERDES_HAL_TYPES   halType = inArgs[2];
    GT_U32      regSelect = (GT_U32)inArgs[3];
    GT_32       halRegValue;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    rc = mvHwsAvago16nmSerdesHalGet(devNum, serdesNum, halType, regSelect, &halRegValue);

    galtisOutput(outArgs, rc, "%d", halRegValue);

    return CMD_OK;
}

/**
* @internal wrHwsAvago16nmSerdesHalSet function
* @endinternal
*
* @brief   Sets the indicated HAL register value
*
* @param[in] devNum    - system device number
* @param[in] serdesNum - serdes number
* @param[in] halType   - HAL types
* @param[in] regSelect - HAL register selector
* @param[in] registerValue - Value to write into the register
*
* @param[out] halRegister - HAL register value
*
* @retval 0 - on success
* @retval 1 - on error
*/
CMD_STATUS wrHwsAvago16nmSerdesHalSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8   devNum    = (GT_U8)inArgs[0];
    GT_U32  serdesNum = (GT_U32)inArgs[1];
    MV_HWS_AVAGO_SERDES_HAL_TYPES   halType = inArgs[2];
    GT_U32  regSelect = (GT_U32)inArgs[3];
    GT_32   registerValue = (GT_32)inArgs[4];

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT)
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);
    rc = mvHwsAvago16nmSerdesHalSet(devNum, serdesNum, halType, regSelect, registerValue);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "");
        return CMD_OK;
    }
#endif
    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/**
* @internal wrHwsAvago16nmSerdesHalFunc function
* @endinternal
*
* @brief   Invokes the indicated HAL function
*
* @param[in] devNum      - system device number
* @param[in] serdesNum   - serdes number
* @param[in] halFuncType - Type of HAL function
*
* @retval 0 - on success
* @retval 1 - on error
*/
CMD_STATUS wrHwsAvago16nmSerdesHalFunc
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8   devNum    = (GT_U8)inArgs[0];
    GT_U32  serdesNum = (GT_U32)inArgs[1];
    MV_HWS_AVAGO_SERDES_HAL_FUNC_TYPES  halFuncType = (GT_U32)inArgs[2];

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);
    rc = mvHwsAvago16nmSerdesHalFunc(devNum, serdesNum, halFuncType);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "");
        return CMD_OK;
    }

    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvagoSerdesClockGlobalControlSet
*
* DESCRIPTION:
*       Configure CPLL
*
* INPUTS:
*       devNum    - system device number
*       serdesNum - SERDES number
*       inputClkSource - Input clock source
*       inputClkFreq   - input frequency
        outputClkFreq  - output frequency
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvagoSerdesClockGlobalControlSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;

    /* Parameters List */
    GT_U32  devNum    = (GT_U32)inArgs[0];
    GT_U32  serdesNum = (GT_U32)inArgs[1];

    /* Fields List */
    MV_HWS_INPUT_SRC_CLOCK          inputClkSource = (GT_U32)inFields[0];
    MV_HWS_CPLL_INPUT_FREQUENCY     inputClkFreq   = (GT_U32)inFields[1];
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outputClkFreq  = (GT_U32)inFields[2];

    GT_UNUSED_PARAM(numFields);

    res = mvHwsSerdesClockGlobalControl(devNum, globalPortGroup, serdesNum, inputClkSource, inputClkFreq, outputClkFreq);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, res, "");
        return CMD_OK;
    }

    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvagoSerdesVoltageTemperatureGetGetFirst
*
* DESCRIPTION:
*       Get the voltage and temperature from sbus master
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       voltage     - voltage (in mV) of sbus master
*       temperature - temperature (in Celsius) of sbus master
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvagoSerdesVoltageTemperatureGetGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;

    /* Parameters List */
    GT_U32  devNum    = (GT_U32)inArgs[0];
#ifndef ASIC_SIMULATION
    GT_U32  serdesNum = (GT_U32)inArgs[1];
#endif
    GT_U32  voltage;
    GT_32   temperature=0;
    unsigned int chipIndex;

    GT_UNUSED_PARAM(numFields);

#ifndef ASIC_SIMULATION
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);
    chipIndex = mvHwsAvagoGetAaplChipIndex(devNum,serdesNum);
#else
    chipIndex = devNum;
#endif

    res = mvHwsAvagoSerdesVoltageGet(devNum, globalPortGroup, chipIndex, &voltage);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, res, "");
        return CMD_OK;
    }

#ifndef ASIC_SIMULATION
    res = mvHwsAvagoSerdesTemperatureGet(devNum, globalPortGroup, chipIndex, &temperature);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, res, "");
        return CMD_OK;
    }
#endif /* ASIC_SIMULATION */

    /* Fields List */
    inFields[0] = (GT_32)voltage;
    inFields[1] = (GT_32)temperature;

    /* pack and output table fields */
    fieldOutput("%d%d", inFields[0], inFields[1]);

    galtisOutput(outArgs, GT_OK, "%f");

    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvagoSerdesVoltageTemperatureGetGetNext
*
* DESCRIPTION:
*       Get the voltage and temperature from sbus master
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       voltage     - voltage (in mV) of sbus master
*       temperature - temperature (in Celsius) of sbus master
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvagoSerdesVoltageTemperatureGetGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    galtisOutput(outArgs, GT_OK, "%d", -1);

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvagoSerdesErrorInjectSet
*
* DESCRIPTION:
*       Injects errors into the RX or TX data
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       numOfBits - Number of error bits to inject (max=65535)
*       serdesDirection - Rx or Tx
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvagoSerdesErrorInjectSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;

    /* Parameters List */
    GT_U32  devNum    = (GT_U32)inArgs[0];
    GT_U32  serdesNum = (GT_U32)inArgs[1];

    /* Fields List */
    GT_U32  numOfBits = (GT_U32)inFields[0];
    GT_U32  direction = (MV_HWS_SERDES_DIRECTION)inFields[1];

    GT_UNUSED_PARAM(numFields);

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    res = mvHwsAvagoSerdesErrorInject(devNum, globalPortGroup, serdesNum, numOfBits, direction);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, res, "");
        return CMD_OK;
    }

    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsAvagoSerdesDfeCfgSet
*
* DESCRIPTION:
*       Per SERDES configure the DFE parameters.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - physical serdes number
*       dfeMode   - Enable/disable/freeze/Force
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
CMD_STATUS wrHwsAvagoSerdesDfeCfgSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;
    MV_HWS_AUTO_TUNE_STATUS rxStatus = TUNE_NOT_COMPLITED;
    GT_U32 i;

    /* Parameters List */
    GT_U32  devNum    = (GT_U32)inArgs[0];
    GT_U32  serdesNum = (GT_U32)inArgs[1];

    /* Fields List */
    GT_U32  DFE_Mode = (GT_U32)inFields[0];

    GT_UNUSED_PARAM(numFields);

    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);

    res = mvHwsAvagoSerdesDfeConfig(devNum, globalPortGroup, serdesNum, (MV_HWS_DFE_MODE)DFE_Mode, NULL);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, res, "");
        return CMD_OK;
    }

    if (HWS_DEV_SERDES_TYPE(devNum, serdesNum) != AVAGO_16NM)
    {
        for (i = 0; i < MV_AVAGO_TRAINING_TIMEOUT; i++)
        {
            /* Delay in 1ms */
            hwsOsExactDelayPtr(devNum, globalPortGroup, 1);

            /* run the Rx-Training status to enable the TAP1 if it passed */
            CHECK_STATUS(mvHwsAvagoSerdesAutoTuneStatusShort(devNum, globalPortGroup, serdesNum, &rxStatus, NULL));
            if (rxStatus == TUNE_PASS)
                break;
        }
    }

    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesSpeedGet
*
* DESCRIPTION:
*       SERDES speed get function
*
* INPUTS:
*       devNum    - system device number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK              - on success.
*       CMD_AGENT_ERROR     - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsSerdesSpeedGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      serdesNum;
    MV_HWS_SERDES_SPEED   rate;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    serdesNum = (GT_U32)inArgs[1];
    CONVERT_RAVENDEVNUM_TO_FALCONDEVNUM(devNum,serdesNum);
    rc = mvHwsSerdesSpeedGet(devNum, globalPortGroup, serdesNum, HWS_DEV_SERDES_TYPE(devNum, serdesNum), &rate);
    galtisOutput(outArgs, rc, "%d", rate);
    return CMD_OK;
}
#ifdef CHX_FAMILY
/*******************************************************************************
* wrHwsD2dPmaLaneTestGenStatusGetFirstEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsD2dPmaLaneTestGenStatusGetFirstEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      d2dNum;
    GT_U32      d2dLaneNum;
    GT_BOOL     counterAccumulateMode;
    MV_HWS_SERDES_TX_PATTERN txP;
    MV_HWS_SERDES_TEST_GEN_STATUS status;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    d2dNum = (GT_U32)inArgs[1];
    d2dLaneNum = (GT_U32)inArgs[2];
    txP = (MV_HWS_SERDES_TX_PATTERN)inArgs[3];
    counterAccumulateMode = (GT_BOOL)inArgs[4];

    rc = mvHwsD2dPmaLaneTestGenStatus(devNum, d2dNum, d2dLaneNum, txP, counterAccumulateMode, &status);

    inFields[1] = status.errorsCntr;
    inFields[3] = status.lockStatus;
    inFields[2] = status.txFramesCntr.l[0];

    /* pack and output table fields */
    fieldOutput("%d%d%d%d",d2dLaneNum,inFields[1],inFields[2],inFields[3]);
    galtisOutput(outArgs, rc, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsSerdesTestGeneratorStatusGetNextEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsD2dPmaLaneTestGenStatusGetNextEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    galtisOutput(outArgs, GT_OK, "%d", -1);

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    return CMD_OK;
}


/*******************************************************************************
* wrHwsD2dPmaLaneTestGen
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsGw16TileTestGen
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      portGroup;
    GT_U32      serdesNum;
    MV_HWS_SERDES_TX_PATTERN txP;
    MV_HWS_SERDES_TEST_GEN_MODE mode;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    serdesNum = (GT_U32)inFields[0];

    txP = (MV_HWS_SERDES_TX_PATTERN)inFields[1];
    mode = (MV_HWS_SERDES_TEST_GEN_MODE)inFields[2];

    rc = mvHwsGw16TileTestGen(devNum, portGroup, serdesNum, txP, mode);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}


/*******************************************************************************
* wrHwsGw16TileTestGenStatusGetFirstEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsGw16TileTestGenStatusGetFirstEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      portGroup;
    GT_U32      serdesNum;
    GT_BOOL     counterAccumulateMode;
    MV_HWS_SERDES_TX_PATTERN txP;
    MV_HWS_SERDES_TEST_GEN_STATUS status;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    serdesNum = (GT_U32)inArgs[2];
    txP = (MV_HWS_SERDES_TX_PATTERN)inArgs[3];
    counterAccumulateMode = (GT_BOOL)inArgs[4];

    rc = mvHwsGw16TileTestGenStatus(devNum, portGroup, serdesNum, txP, counterAccumulateMode, &status);

    inFields[1] = status.errorsCntr;
    inFields[3] = status.lockStatus;
    inFields[2] = status.txFramesCntr.l[0];

    /* pack and output table fields */
    fieldOutput("%d%d%d%d",serdesNum,inFields[1],inFields[2],inFields[3]);
    galtisOutput(outArgs, rc, "%f");
    return CMD_OK;
}

/*******************************************************************************
* wrHwsGw16TileTestGenStatusGetNextEntry
*
* DESCRIPTION:
*
* INPUTS:
*       devId           - Device Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       CMD_OK            - on success.
*       CMD_AGENT_ERROR   - on failure.
*       CMD_FIELD_UNDERFLOW - not enough field arguments.
*       CMD_FIELD_OVERFLOW  - too many field arguments.
*       CMD_SYNTAX_ERROR    - on fail
*
* COMMENTS:
*       None
*
*******************************************************************************/
static CMD_STATUS wrHwsGw16TileTestGenStatusGetNextEntry
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    galtisOutput(outArgs, GT_OK, "%d", -1);

    return CMD_OK;
}
#endif

static CMD_STATUS wrHwsRegisterSetFuncPtr
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      portGroup;
    GT_U32      regAddr;
    GT_U32      regValue;
    GT_U32      mask;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    regAddr = (GT_U32)inArgs[2];
    regValue = (GT_U32)inArgs[3];
    mask = (GT_U32)inArgs[4];

    if(mask == 0)
    {
        mask = 0xFFFFFFFF;
    }

    rc = hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, regValue, mask);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static CMD_STATUS wrHwsRegisterGetFuncPtr
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      portGroup;
    GT_U32      regAddr;
    GT_U32      regValue;
    GT_U32      mask;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    regAddr = (GT_U32)inArgs[2];
    mask = (GT_U32)inArgs[3];

    if(mask == 0)
    {
        mask = 0xFFFFFFFF;
    }

    rc = hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &regValue, mask);

    galtisOutput(outArgs, rc, "%d", regValue);

    return CMD_OK;
}

static CMD_STATUS wrHwsDfxRegisterSetFuncPtr
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      regAddr;
    GT_U32      regValue;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    regAddr = (GT_U32)inArgs[1];
    regValue = (GT_U32)inArgs[2];

    rc = hwsServerRegSetFuncPtr(devNum, regAddr, regValue);

    galtisOutput(outArgs, rc, "%d", regValue);

    return CMD_OK;
}


static CMD_STATUS wrHwsDfxRegisterGetFuncPtr
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      regAddr;
    GT_U32      regValue;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    regAddr = (GT_U32)inArgs[1];

    rc = hwsServerRegGetFuncPtr(devNum, regAddr, &regValue);

    galtisOutput(outArgs, rc, "%d", regValue);

    return CMD_OK;
}

#ifndef PX_FAMILY
static CMD_STATUS wrRavenInitSystem
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8 startDevNum=0;
    GT_U8 endDevNum=5;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
#ifndef CPSS_APP_PLATFORM_REFERENCE
    rc = ravenInitSystem(startDevNum, endDevNum, 0);
#else
    (void)startDevNum;
    (void)endDevNum;
    return CMD_OK;
#endif

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}
#endif
#ifdef CHX_FAMILY
static CMD_STATUS wrHwsD2dPmaLaneTestGeneneratorConfig
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   rc;         /* Return code  */
    GT_U8                       devNum;
    GT_U32                      d2dNum;
    GT_U32                      d2dLaneNum;
    MV_HWS_D2D_PRBS_CONFIG_STC  config;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    d2dNum = (GT_U32)inArgs[1];
    d2dLaneNum = (GT_U32)inFields[0];

    config.serdesPattern = (MV_HWS_SERDES_TX_PATTERN)inFields[1];
    config.mode = (MV_HWS_SERDES_TEST_GEN_MODE)inFields[2];
    config.direction = (MV_HWS_D2D_DIRECTION_MODE_ENT)inFields[3];
    config.fecMode = (MV_HWS_PORT_FEC_MODE)inFields[4];
    config.userDefPatternEnabled = (GT_BOOL)inFields[5];
    config.userDefPattern[0] = (GT_U32)inFields[6];
    config.userDefPattern[1] = (GT_U32)inFields[7];
    config.userDefPattern[2] = (GT_U32)inFields[8];
    config.userDefPattern[3] = (GT_U32)inFields[9];

    rc = mvHwsD2dPmaLaneTestGeneneratorConfig(devNum, d2dNum, d2dLaneNum, &config);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static CMD_STATUS wrHwsD2dPmaLaneTestCheckerConfig
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   rc;         /* Return code  */
    GT_U8                       devNum;
    GT_U32                      d2dNum;
    GT_U32                      d2dLaneNum;
    MV_HWS_D2D_PRBS_CONFIG_STC  config;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    d2dNum = (GT_U32)inArgs[1];
    d2dLaneNum = (GT_U32)inFields[0];

    config.serdesPattern = (MV_HWS_SERDES_TX_PATTERN)inFields[1];
    config.mode = (MV_HWS_SERDES_TEST_GEN_MODE)inFields[2];
    config.direction = (MV_HWS_D2D_DIRECTION_MODE_ENT)inFields[3];
    config.fecMode = (MV_HWS_PORT_FEC_MODE)inFields[4];
    config.userDefPatternEnabled = (GT_BOOL)inFields[5];
    config.userDefPattern[0] = (GT_U32)inFields[6];
    config.userDefPattern[1] = (GT_U32)inFields[7];
    config.userDefPattern[2] = (GT_U32)inFields[8];
    config.userDefPattern[3] = (GT_U32)inFields[9];

    rc = mvHwsD2dPmaLaneTestCheckerConfig(devNum, d2dNum, d2dLaneNum, &config);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static CMD_STATUS wrHwsD2dPmaLaneTestLoopbackSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       rc;             /* return code  */
    GT_U8                           devNum;
    GT_U32                          d2dNum;         /* D2D number */
    MV_HWS_PORT_LB_TYPE             lbType;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    d2dNum = (GT_U32)inArgs[1];
    lbType = (MV_HWS_PORT_LB_TYPE)inArgs[2];

    rc = mvHwsD2dPmaLaneTestLoopbackSet(devNum, d2dNum, lbType);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static CMD_STATUS wrHwsD2dPmaLaneTestGenInjectErrorEnable
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       rc;             /* return code  */
    GT_U8                           devNum;
    GT_U32                          d2dNum;         /* D2D number */
    GT_U32                          d2dLaneNum;     /* lane index 0..3 */
    MV_HWS_SERDES_DIRECTION         direction;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    d2dNum = (GT_U32)inArgs[1];
    d2dLaneNum = (GT_U32)inArgs[2];
    direction = (MV_HWS_SERDES_DIRECTION)inArgs[3];

    rc = mvHwsD2dPmaLaneTestGenInjectErrorEnable(devNum, d2dNum, d2dLaneNum, direction);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static CMD_STATUS wrHwsD2dMacFrameTableConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       rc;             /* return code  */
    GT_U8                           devNum;
    GT_U32                          d2dNum;
    GT_U32                          frameIndex;     /* lane index 0..31 */
    MV_HWS_D2D_FRAME_TABLE_CONFIG_STC config;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    d2dNum = (GT_U32)inArgs[1];
    frameIndex = (GT_U32)inFields[0];
    config.frameRepeatCnt = (GT_U32)inFields[1];
    config.idleWord = (GT_U32)inFields[2];
    config.frameCommand = (GT_U32)inFields[3];
    config.frameLengthMax = (GT_U32)inFields[4];
    config.frameLengthMin = (GT_U32)inFields[5];
    config.framePayload = (GT_U32)inFields[6];
    config.frameHeader[0] = (GT_U32) inFields[7];
    config.frameHeader[1] = (GT_U32) inFields[8];
    config.frameHeader[2] = (GT_U32) inFields[9];
    config.frameHeader[3] = (GT_U32) inFields[10];
    config.frameHeader[4] = (GT_U32) inFields[11];
    config.frameHeader[5] = (GT_U32) inFields[12];
    config.frameHeader[6] = (GT_U32) inFields[13];

    rc = hwsD2dMacFrameTableConfigSet(devNum, d2dNum, frameIndex, &config);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static CMD_STATUS wrHwsD2dMacFrameGenConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       rc;             /* return code  */
    GT_U8                           devNum;
    GT_U32                          d2dNum;
    GT_BOOL                         enable;
    GT_U32                          channelNum;     /* lane index 0..16 */
    MV_HWS_D2D_FRAME_GEN_CONFIG_STC config;

    GT_UNUSED_PARAM(numFields);


    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    d2dNum = (GT_U32)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    channelNum = (GT_U32)inFields[0];
    config.txFrameRepeatCount = (GT_U32)inFields[1];
    config.txFrameTableStartPointer = (GT_U32)inFields[2];
    config.txFrameTableCount = (GT_U32)inFields[3];
    config.txPayloadType = (GT_U32)inFields[4];

    rc = hwsD2dMacFrameGenConfigSet(devNum, d2dNum, enable, channelNum, &config);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static CMD_STATUS wrHwsD2dMacFrameCheckerConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       rc;             /* return code  */
    GT_U8                           devNum;
    GT_U32                          d2dNum;
    GT_BOOL                         enable;
    GT_U32                          channelNum;     /* lane index 0..16 */
    MV_HWS_D2D_FRAME_CHECKER_CONFIG_STC config;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    d2dNum = (GT_U32)inArgs[1];
    enable = (GT_BOOL)inArgs[2];
    channelNum = (GT_U32)inFields[0];
    config.rxPayloadType = (GT_U32)inFields[1];
    config.rxBadCntMask = (GT_U32)inFields[2];

    rc = hwsD2dMacFrameCheckerConfigSet(devNum, d2dNum, enable, channelNum, &config);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}


static CMD_STATUS wrHwsD2dMacFrameGenStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      d2dNum;
    GT_U32      channelNum;
    GT_U32      frameGoodCnt;
    GT_U32      frameBadCnt;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    d2dNum = (GT_U32)inArgs[1];
    channelNum = (GT_U32)inArgs[2];

    rc = hwsD2dMacFrameGenStatusGet(devNum, d2dNum, channelNum, &frameGoodCnt, &frameBadCnt);

    galtisOutput(outArgs, rc, "%d%d", frameGoodCnt, frameBadCnt);

    return CMD_OK;
}
#endif

CMD_STATUS wrHwsAvsVoltageSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;

    /* Parameters List */
    GT_U8  devNum    = (GT_U8)inArgs[0];
    MV_HWS_UNIT_NUM  chipIndex = (MV_HWS_UNIT_NUM)inArgs[1];
    GT_U32  vcore = (GT_U32)inArgs[2];

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    res = mvHwsAvsVoltageSet(devNum, chipIndex ,vcore);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, res, "");
        return CMD_OK;
    }

    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

static CMD_STATUS wrHwsAvsVoltageGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      regValue=0;
    MV_HWS_UNIT_NUM  chipIndex;

    GT_UNUSED_PARAM(numFields);

   /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    chipIndex = (MV_HWS_UNIT_NUM)inArgs[1];

    rc = mvHwsAvsVoltageGet(devNum, chipIndex ,&regValue);

    galtisOutput(outArgs, rc, "%d", regValue);

    return CMD_OK;
}


static CMD_STATUS wrHwsDroStatisticsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum;
    GT_U32      droAvg,minDro,maxDro;
    MV_HWS_UNIT_NUM  chipIndex;

    GT_UNUSED_PARAM(numFields);

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    devNum = (GT_U8)inArgs[0];
    chipIndex = (MV_HWS_UNIT_NUM)inArgs[1];

    rc = mvHwsDroStatisticsGet(devNum, chipIndex ,&droAvg,&maxDro,&minDro);

    inFields[0] = droAvg;
    inFields[1] = maxDro;
    inFields[2] = minDro;

    /* pack and output table fields */
    fieldOutput("%d%d%d",inFields[0],inFields[1],inFields[2]);
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

/*******************************************************************************
* wrHwsGetNext
*
* DESCRIPTION:
*
*   This function is called for ending table printing.
*
* APPLICABLE DEVICES:  All
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK  - on success
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static CMD_STATUS wrHwsGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);

    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

/*******************************************************************************
*
* dbCommands - Galtis database galtis commands
*
*******************************************************************************/
static CMD_COMMAND dbCommands[] = {
{"HwsSerdesAutoTuneStartSet", &wrHwsSerdesAutoTuneStartSetEntry, 1,  3},
{"HwsSerdesLoopbackCfgSet", &wrHwsSerdesLoopbackCfgSetEntry, 1,  2},
{"HwsSerdesManualRxCfgSet", &wrHwsSerdesManualRxCfgSetEntry, 1,  6},
{"HwsSerdesAutoTuneResultsGetFirst", &wrHwsSerdesAutoTuneResultsGetFirstEntry, 3,  0},
{"HwsSerdesAutoTuneResultsGetNext", &wrHwsSerdesAutoTuneResultsGetNextEntry, 3,  0},
{"HwsSerdesManualTxCfgSet", &wrHwsSerdesManualTxCfgSetEntry, 1,  5},
{"HwsAvagoSerdesManualTxCfgSet", &wrHwsAvagoSerdesManualTxCfgSet, 2, 5},
{"HwsAvagoSerdesManualTxCfgGetFirst", &wrHwsAvagoSerdesManualTxCfgGet, 2, 0},
{"HwsAvagoSerdesManualTxCfgGetNext", &wrHwsGetNext, 2, 0},
{"HwsSerdesPowerCtrlSet", &wrHwsSerdesPowerCtrlSetEntry, 1,  7},
{"HwsAvagoSerdesPowerCtrlSet", &wrHwsSerdesAvagoPowerCtrlSetEntry, 1,  8},
{"HwsSerdesAutoTuneStatusGetFirst", &wrHwsSerdesAutoTuneStatusGetFirstEntry, 3,  0},
{"HwsSerdesAutoTuneStatusGetNext", &wrHwsSerdesAutoTuneStatusGetNextEntry, 3,  0},
{"HwsSerdesDfeCfgSet", &wrHwsSerdesDfeCfgSetEntry, 3,  22},
{"HwsSerdesDfeCfgGetFirst", &wrHwsSerdesDfeCfgGetFirstEntry, 3,  0},
{"HwsSerdesDfeCfgGetNext", &wrHwsSerdesDfeCfgGetNextEntry, 3,  0},
{"HwsSerdesResetCfgSet", &wrHwsSerdesResetCfgSetEntry, 1,  4},
{"HwsSerdesCalibartionGetFirst", &wrSerdesCalibrationGetFirst, 3, 0},
{"HwsSerdesCalibartionGetNext", &wrSerdesCalibrationGetNext, 3, 0},
{"HwsSerdesTxEnable", &wrHwsSerdesTxEnableSet, 4, 0},
{"HwsAvago16nmSerdesPmroMetricGet", &wrHwsAvago16nmSerdesPmroMetricGet, 1, 0},
{"HwsSerdesSpeedGet", &wrHwsSerdesSpeedGet, 2, 0},
{"HwsSerdesTestGeneratorCfgSet", &wrHwsSerdesTestGeneratorCfgSet, 2,  3},
{"HwsSerdesTestGeneratorCfg1Set", &wrHwsSerdesTestGeneratorCfgSet1, 2,  3},
{"HwsSerdesTestGeneratorStatusGetFirst", &wrHwsSerdesTestGeneratorStatusGetFirstEntry, 4,  0},
{"HwsSerdesTestGeneratorStatus1GetFirst", &wrHwsSerdesTestGeneratorStatus1GetFirstEntry, 5,  0},
{"HwsSerdesTestGeneratorStatusGetNext", &wrHwsSerdesTestGeneratorStatusGetNextEntry, 4,  0},
{"HwsSerdesTestGeneratorStatus1GetNext", &wrHwsSerdesTestGeneratorStatus1GetNextEntry, 5,  0},

{"HwsSerdesSeqCfgGetFirst", &wrHwsSerdesSeqCfgGetFirst, 2,  0},
{"HwsSerdesSeqCfgGetNext", &wrHwsSerdesSeqCfgGetNext, 2,  0},
{"HwsSerdesSeqCfgSet", &wrHwsSerdesSeqCfgSet, 2,  8},

{"HwsSerdesScanSamplerGetFirst", &wrHwsSerdesScanSamplerGetFirst, 6, 0},
{"HwsSerdesScanSamplerGetNext", &wrHwsSerdesScanSamplerGetNext, 6, 0},

{"HwsSerdesScanSamplerExtGetFirst", &wrHwsSerdesScanSamplerExtGetFirst, 8, 0},
{"HwsSerdesScanSamplerExtGetNext", &wrHwsSerdesScanSamplerExtGetNext, 8, 0},

{"HwsSerdesEomGetGetFirst", &wrHwsSerdesEomGetGetFirst, 4, 0},
{"HwsSerdesEomGetGetNext", &wrHwsSerdesEomGetGetNext, 4, 0},

{"HwsSerdesRegAccessGetFirst", &wrHwsSerdesRegAccessGetFirst, 4,  0},
{"HwsSerdesRegAccessGetNext", &wrHwsSerdesRegAccessGetNext, 4,  0},
{"HwsSerdesRegAccessSet", &wrHwsSerdesRegAccessSet, 4,  1},

{"HwsAvagoSerdesEnhanceTuneSet", &wrHwsAvagoSerdesEnhanceTuneSet, 2, 2},
{"HwsAvagoSerdesSpicoInterruptSet", &wrHwsAvagoSerdesSpicoInterruptSetEntry, 4, 1},
{"HwsAvagoSerdesSpicoInterruptGetFirst", &wrHwsAvagoSerdesSpicoInterruptGetFirst, 4, 0},
{"HwsAvagoSerdesSpicoInterruptGetNext", &wrHwsAvagoSerdesSpicoInterruptGetNext, 4, 0},
{"HwsAvagoSerdesShiftSamplePointSet", &wrHwsAvagoSerdesShiftSamplePointSet, 2, 1},
{"HwsAvagoSerdesManualCtleCfgSet", &wrHwsAvagoSerdesManualCtleCfgSet, 2, 6},
{"HwsAvagoSerdesManualCtleCfg1Set", &wrHwsAvagoSerdesManualCtleCfg1Set, 2, 10},
{"HwsAvagoSerdesManualCtleCfg1GetFirst", &wrHwsAvagoSerdesManualCtleCfg1Get, 2, 0},
{"HwsAvagoSerdesManualCtleCfg1GetNext", &wrHwsGetNext, 2, 0},
{"HwsAvagoSerdesAutoTuneResultsGetFirst", &wrHwsAvagoSerdesAutoTuneResultsGetFirst, 3, 0},
{"HwsAvagoSerdesAutoTuneResultsGetNext", &wrHwsAvagoSerdesAutoTuneResultsGetNext, 3, 0},
{"HwsAvago16nmSerdesAutoTuneResultsGetFirst", &wrHwsAvago16nmSerdesAutoTuneResultsGetFirst, 4, 0},
{"HwsAvago16nmSerdesAutoTuneResultsGetNext", &wrHwsAvago16nmSerdesAutoTuneResultsGetNext, 4, 0},
{"HwsAvagoSerdesClockGlobalControlSet", &wrHwsAvagoSerdesClockGlobalControlSet, 2, 3},
{"HwsAvagoSerdesVoltageTemperatureGetGetFirst", &wrHwsAvagoSerdesVoltageTemperatureGetGetFirst, 2, 0},
{"HwsAvagoSerdesVoltageTemperatureGetGetNext", &wrHwsAvagoSerdesVoltageTemperatureGetGetNext, 2, 0},
{"HwsAvagoSerdesErrorInjectSet", &wrHwsAvagoSerdesErrorInjectSet, 2, 2},
{"HwsAvsVoltageSet", &wrHwsAvsVoltageSet, 3, 0},
{"HwsAvsVoltageGet", &wrHwsAvsVoltageGet, 2, 0},
{"HwsAvagoSerdesDfeCfgSet", &wrHwsAvagoSerdesDfeCfgSet, 2, 1},
{"HwsAvagoSerdesLowPowerModeEnableSet", &wrHwsAvagoSerdesLowPowerModeEnableSet, 2, 1},
{"HwsAvagoSerdesLowPowerModeEnableGetFirst", &wrHwsAvagoSerdesLowPowerModeEnableGet, 2, 0},
{"HwsAvagoSerdesLowPowerModeEnableGetNext", &wrHwsGetNext, 2, 0},
#ifdef CHX_FAMILY
{"HwsD2dPmaLaneTestGenStatusGetFirst", &wrHwsD2dPmaLaneTestGenStatusGetFirstEntry, 5, 0},
{"HwsD2dPmaLaneTestGenStatusGetNext", &wrHwsD2dPmaLaneTestGenStatusGetNextEntry, 5, 0},
#endif
{"HwsRegisterSetFunc", &wrHwsRegisterSetFuncPtr, 5, 0},
{"HwsRegisterGetFunc", &wrHwsRegisterGetFuncPtr, 4, 0},
{"HwsDfxRegisterSetFunc", &wrHwsDfxRegisterSetFuncPtr, 3, 0},
{"HwsDfxRegisterGetFunc", &wrHwsDfxRegisterGetFuncPtr, 2, 0},
#ifndef PX_FAMILY
{"ravenInitSystem", &wrRavenInitSystem, 0, 0},
#endif
#ifdef CHX_FAMILY
{"HwsGw16TileTestGenSet", &wrHwsGw16TileTestGen, 2,3},
{"HwsGw16TileTestGenStatusGetFirst", &wrHwsGw16TileTestGenStatusGetFirstEntry, 5, 0},
{"HwsGw16TileTestGenStatusGetNext", &wrHwsGw16TileTestGenStatusGetNextEntry, 5, 0},

{"HwsD2dPmaLaneTestGeneneratorConfigSet", &wrHwsD2dPmaLaneTestGeneneratorConfig, 2,10},
{"HwsD2dPmaLaneTestCheckerConfigSet", &wrHwsD2dPmaLaneTestCheckerConfig, 2,10},
{"HwsD2dPmaLaneTestLoopbackSet", &wrHwsD2dPmaLaneTestLoopbackSet, 4, 0},
{"HwsD2dPmaLaneTestGenInjectErrorEnable", &wrHwsD2dPmaLaneTestGenInjectErrorEnable, 4, 0},

{"HwsD2dMacFrameTableConfigSet", &wrHwsD2dMacFrameTableConfigSet, 2,14},
{"HwsD2dMacFrameGenConfigSet", &wrHwsD2dMacFrameGenConfigSet, 3, 5},
{"HwsD2dMacFrameCheckerConfigSet", &wrHwsD2dMacFrameCheckerConfigSet, 3, 3},
{"HwsD2dMacFrameGenStatusGet", &wrHwsD2dMacFrameGenStatusGet, 3, 0},
#endif
{"HwsAvago16nmSerdesHalGet",  &wrHwsAvago16nmSerdesHalGet, 4, 0},
{"HwsAvago16nmSerdesHalSet",  &wrHwsAvago16nmSerdesHalSet, 5, 0},
{"HwsAvago16nmSerdesHalFunc", &wrHwsAvago16nmSerdesHalFunc, 3, 0},
{"HwsDroStatisticsGetFirst", &wrHwsDroStatisticsGet, 2, 0},
{"HwsDroStatisticsGetNext", &wrHwsGetNext, 2, 0}
};


#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/*******************************************************************************
* cmdLibInitHwsSerdesInit
*
* DESCRIPTION:
*     Library database initialization function.
*
* INPUTS:
*     none
*
* OUTPUTS:
*     none
*
* RETURNS:
*     GT_OK   - on success.
*     GT_FAIL - on failure.
*
* COMMENTS:
*     none
*
*******************************************************************************/
GT_STATUS cmdLibInitHwsSerdesInit()
{
    return cmdInitLibrary(dbCommands, numCommands);
}

