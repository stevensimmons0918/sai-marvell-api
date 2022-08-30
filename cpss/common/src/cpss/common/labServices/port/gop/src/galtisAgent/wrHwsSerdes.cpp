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
* wrLn2LMS.c
*
* DESCRIPTION:
*       Implementation of the Galtis Wrapper functions
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 22 $
*
*******************************************************************************/

/* General H Files */
#include <common/os/gtOs.h>

#include <common/system/system.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <common/macros/gtCommonFuncs.h>

#include <galtisAgent/wrapUtil/galtisValues.h>
#include <galtisAgent/wrapUtil/cmdUtils.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH/mvComPhyEomIf.h>



extern "C" {
GT_STATUS cmdLibInitHwsSerdesInit
(
    GT_VOID
);
GT_STATUS mvHwsComHRev2DfeCheck
(
    GT_U8 devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum
);

}

typedef enum
{
  SerdesDisable  = 0,
  SerdesEnable   = 1,
  SerdesTxConfig = 2,

}MV_SERDES_AUTO_TUNE_MODE;


GT_U32  eomUpperMatrix[EOM_VOLT_RANGE * EOM_PHASE_RANGE];
GT_U32  eomLowerMatrix[EOM_VOLT_RANGE * EOM_PHASE_RANGE];

/**
* @internal wrHwsSerdesAutoTuneStartSetEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesAutoTuneStartSetEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      serdesNum = inFields[0];
    GT_BOOL     rxTr;
    GT_BOOL     txTr;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    rxTr = (GT_BOOL)inFields[1];
    txTr = (GT_BOOL)(inFields[2] != SerdesDisable);

    if (inFields[2] == SerdesEnable)
    {
        rc = mvHwsSerdesTxAutoTuneStart(devNum, devNum, serdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum), GT_TRUE);
    }
    else
    {
        rc = mvHwsSerdesAutoTuneCfg(devNum, devNum, serdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum), rxTr, txTr);
    }

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/**
* @internal wrHwsSerdesLoopbackCfgSetEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesLoopbackCfgSetEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];

    GT_U32      serdesNum = inFields[0];
    MV_HWS_SERDES_LB     lb;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
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

    rc = mvHwsSerdesLoopback(devNum, devNum, serdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum), lb);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/**
* @internal wrHwsSerdesManualRxCfgSetEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesManualRxCfgSetEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      serdesNum = inFields[0];

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    rc = mvHwsSerdesManualRxConfig(devNum,devNum,serdesNum,(MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum), (MV_HWS_PORT_MAN_TUNE_MODE)inFields[1], (GT_U32)inFields[2],(GT_U32)inFields[3],(GT_U32)inFields[4],(GT_BOOL)inFields[5], (GT_U32)inFields[6]);
    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static GT_U32 SerdesAutoTuneResultsSerdesNum;

/**
* @internal wrHwsSerdesAutoTuneResultsGetFirstEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesAutoTuneResultsGetFirstEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      startSerdesNum;
    GT_U32      endSerdesNum;
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT results;
    int i;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    SerdesAutoTuneResultsSerdesNum = inArgs[1];
      startSerdesNum = inArgs[1];
      endSerdesNum = inArgs[2];
      if (endSerdesNum <  startSerdesNum)
    {
        endSerdesNum = startSerdesNum;
    }

    rc = mvHwsSerdesAutoTuneResult(devNum,devNum,SerdesAutoTuneResultsSerdesNum,
                                   (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneResultsSerdesNum),&results);

    if ((HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneResultsSerdesNum) == AVAGO) || (HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneResultsSerdesNum) == AVAGO_16NM))
    {
        inFields[4]  = results.avagoResults.sqleuch;
        inFields[5]  = (GT_32)results.avagoResults.avagoStc.atten;
        inFields[6]  = 0;
        inFields[7]  = 0;
        inFields[8]  = (GT_32)results.avagoResults.avagoStc.post;
        inFields[9]  = (GT_32)results.avagoResults.avagoStc.pre;
        inFields[10] = 0;
        inFields[11] = 0;
    }
    else
    {
        inFields[1]  = results.comphyHResults.ffeR;
        inFields[2]  = results.comphyHResults.ffeC;
        inFields[3]  = results.comphyHResults.sampler;
        inFields[4]  = results.comphyHResults.sqleuch;
        inFields[5]  = results.comphyHResults.txAmp;
        inFields[6]  = results.comphyHResults.txAmpAdj;
        inFields[7]  = results.comphyHResults.txAmpShft;
        inFields[8]  = (GT_32)results.comphyHResults.txEmph0; /*TX_Emph0*/
        inFields[9]  = (GT_32)results.comphyHResults.txEmph1; /*TX_Emph1*/
        inFields[10] = (GT_32)results.comphyHResults.txEmph0En; /*TX_Emph0_En*/
        inFields[11] = (GT_32)results.comphyHResults.txEmph1En; /*TX_Emph1_En*/
        for (i = 0; i < 6; i++)
        {
            inFields[12+i] = abs(results.comphyHResults.dfeVals[i]); //DFE_F[i]
        }
    }


    if ( (HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneResultsSerdesNum) == AVAGO) || (HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneResultsSerdesNum) == AVAGO_16NM) )
    {
        cpssOsPrintf("\nTX Atten is %d",    results.avagoResults.avagoStc.atten);
        cpssOsPrintf("\nTX Post is %d",     results.avagoResults.avagoStc.post);
        cpssOsPrintf("\nTX Pre is %d\n",    results.avagoResults.avagoStc.pre);
    }
    else
    {
        hwsOsPrintf("\nFFE R is %d",        results.comphyHResults.ffeR);
        hwsOsPrintf("\nFFE C is %d",        results.comphyHResults.ffeC);
        hwsOsPrintf("\nSampler is %d",      results.comphyHResults.sampler);
        hwsOsPrintf("\nSqleuch is %d",      results.comphyHResults.sqleuch);

        hwsOsPrintf("\nTX Amp is %d",       results.comphyHResults.txAmp);
        hwsOsPrintf("\nTX Amp Adj is %d",   results.comphyHResults.txAmpAdj);
        hwsOsPrintf("\nTX Amp Shft is %d",  results.comphyHResults.txAmpShft);
        hwsOsPrintf("\nTX Emph0 is %d",     results.comphyHResults.txEmph0);
        hwsOsPrintf("\nTX Emph1 is %d",     results.comphyHResults.txEmph1);
        hwsOsPrintf("\nTX Emph0 En is %d",  results.comphyHResults.txEmph0En);
        hwsOsPrintf("\nTX Emph1 En is %d",  results.comphyHResults.txEmph1En);
        for (i = 0; i < 6; i++)
        {
            hwsOsPrintf("\nDFE%d is %d", i, results.comphyHResults.dfeVals[i]);
        }

    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", SerdesAutoTuneResultsSerdesNum, inFields[1], inFields[2], inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15], inFields[16], inFields[17]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/**
* @internal wrHwsSerdesAutoTuneResultsGetNextEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesAutoTuneResultsGetNextEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      startSerdesNum;
    GT_U32      endSerdesNum;
    GT_U32      serdesNum;
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT results;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
      startSerdesNum = inArgs[1];
      endSerdesNum = inArgs[2];
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

    rc = mvHwsSerdesAutoTuneResult(devNum,devNum,serdesNum,(MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum),&results);

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

        for (int i = 0; i < 6; i++)
        {
            inFields[12+i] = abs(results.comphyHResults.dfeVals[i]); //DFE_F[i]
        }

    }


    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15], inFields[16], inFields[17]);
    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;

}

/**
* @internal wrHwsSerdesManualTxCfgSetEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesManualTxCfgSetEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;

    /* Parameters List */
    GT_U32  DevNum = (GT_U32)inArgs[0];

    /* Fields List */
    GT_U32  SerdesNum   = (GT_U32)inFields[0];
    GT_U32  TX_Amp      = (GT_U32)inFields[1];
    GT_U32  TX_Amp_Adj  = (GT_U32)inFields[2];
    GT_U32  TX_Amp_Shft = (GT_U32)inFields[3];
    GT_U32  Emph0       = (GT_U32)inFields[4];
    GT_U32  Emph1       = (GT_U32)inFields[5];

    res = mvHwsSerdesManualTxConfig(DevNum, DevNum, SerdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(DevNum, SerdesNum),
                                    TX_Amp, (GT_BOOL)TX_Amp_Adj, Emph0, Emph1, (GT_BOOL)TX_Amp_Shft, GT_TRUE);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, GT_FAIL, "%f");
        return CMD_OK;
    }
    return CMD_OK;
}

/**
* @internal wrHwsSerdesPowerCtrlSetEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesPowerCtrlSetEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      serdesNum = inFields[0];
    MV_HWS_SERDES_SPEED     baudRate;
    MV_HWS_REF_CLOCK        refClock;
    MV_HWS_REF_CLOCK_SRC    refClockSource;
    MV_HWS_SERDES_MEDIA     media;
    MV_HWS_SERDES_BUS_WIDTH_ENT  mode;
    GT_U32 serdesArr[1];

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    refClock = _156dot25Mhz;
    refClockSource = (MV_HWS_REF_CLOCK_SRC)inFields[3];

    switch (inFields[4])
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
    default:
        galtisOutput(outArgs, GT_NOT_SUPPORTED, "");
        return CMD_OK;
    }

    switch(inFields[5])
    {
    case 0:
      mode = (MV_HWS_SERDES_BUS_WIDTH_ENT)1; //_10BIT_OFF;
      break;
    case 1:
      mode = (MV_HWS_SERDES_BUS_WIDTH_ENT)0; //_10BIT_ON;
      break;
    default:
      mode = (MV_HWS_SERDES_BUS_WIDTH_ENT)2; //_10BIT_NA;
      break;
    }
    media = (MV_HWS_SERDES_MEDIA)inFields[6];

    serdesArr[0] = serdesNum;
    rc = mvHwsSerdesArrayPowerCtrl(devNum,(GT_U32)devNum,(MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum),
      1, &serdesArr[0], (GT_BOOL)inFields[1],baudRate,refClock,refClockSource,media,mode);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/**
* @internal wrHwsSerdesDfeCfgSetEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesDfeCfgSetEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      serdesNum = inFields[0];
    MV_HWS_DFE_MODE   dfeMode;
    GT_32      dfeCfg[6];
    int        i;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    dfeMode = (MV_HWS_DFE_MODE)inFields[1];
    dfeCfg[0] = inFields[2];
    for ( i = 1; i < 6; i++)
    {
      if (inFields[i*2+1] == 1)
      {
        dfeCfg[i] = inFields[i*2+2] - inFields[i*2+2]*2;
      }
      else
      {
        dfeCfg[i] = inFields[i*2+2];
      }
    }
    rc = mvHwsSerdesDfeConfig(devNum,devNum,serdesNum,(MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum),dfeMode,dfeCfg);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static GT_U32    wrSerdesDfeCur;

/**
* @internal wrHwsSerdesDfeCfgGetFirstEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesDfeCfgGetFirstEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      startSerdesNum;
    GT_U32      endSerdesNum;
    GT_U32      serdesNum;
    GT_BOOL    dfeLock;
    GT_32      dfeCfg[6];
    GT_32      f0Cfg[3];
    GT_32      savCfg[5];
    int         i;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

      startSerdesNum = inArgs[1];
      endSerdesNum = inArgs[2];
      if (endSerdesNum <  startSerdesNum)
    {
        endSerdesNum = startSerdesNum;
    }
    serdesNum = startSerdesNum;
    /* for first index from range */
    wrSerdesDfeCur = startSerdesNum;

    inFields[0] = startSerdesNum;

    rc = mvHwsSerdesDfeStatusExt(devNum,devNum,serdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum), &dfeLock, dfeCfg, f0Cfg, savCfg);
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

/**
* @internal wrHwsSerdesDfeCfgGetNextEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesDfeCfgGetNextEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      startSerdesNum;
    GT_U32      endSerdesNum;
    GT_U32      serdesNum;
    GT_BOOL    dfeLock;
    GT_32      dfeCfg[6];
    GT_32      f0Cfg[3];
    GT_32      savCfg[5];
    int        i;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

      startSerdesNum = inArgs[1];
      endSerdesNum = inArgs[2];
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

    rc = mvHwsSerdesDfeStatusExt(devNum,devNum,serdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum), &dfeLock, dfeCfg, f0Cfg, savCfg);
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

/**
* @internal wrHwsSerdesResetCfgSetEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesResetCfgSetEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      serdesNum = inFields[0];

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    rc = mvHwsSerdesReset(devNum,devNum,serdesNum,(MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum),(GT_BOOL)inFields[1],(GT_BOOL)inFields[2],(GT_BOOL)inFields[3]);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

static GT_U32 SerdesAutoTuneStatusSerdesNum;

/**
* @internal wrHwsSerdesAutoTuneStatusGetFirstEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesAutoTuneStatusGetFirstEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    MV_HWS_AUTO_TUNE_STATUS rxStatus;
    MV_HWS_AUTO_TUNE_STATUS txStatus;
    MV_HWS_ALIGN90_PARAMS   serdesParams;
    MV_HWS_AUTO_TUNE_STATUS status;
    GT_BOOL allLanesPass;
    GT_U32 loopNum;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    SerdesAutoTuneStatusSerdesNum = inArgs[1];

    rxStatus = txStatus = TUNE_NOT_COMPLITED;
    rc = mvHwsSerdesAutoTuneStatus(devNum,devNum,SerdesAutoTuneStatusSerdesNum,
                                   (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneStatusSerdesNum),&rxStatus,&txStatus);

    inFields[1] = rxStatus;
    inFields[2] = txStatus;

    if ((rxStatus == TUNE_PASS) || (txStatus == TUNE_PASS))
    {
        /* start Align90 */
        CHECK_CMD_STATUS(mvHwsSerdesFixAlign90Start(devNum,devNum,SerdesAutoTuneStatusSerdesNum,
                        (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneStatusSerdesNum), &serdesParams));
        loopNum = 0;
        do
        {
            allLanesPass = GT_TRUE;
            hwsOsExactDelayPtr(devNum, devNum, 20);
            /* check status on related serdes */
            mvHwsSerdesFixAlign90Status(devNum, devNum, SerdesAutoTuneStatusSerdesNum,
                            (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneStatusSerdesNum), &status);
            if (status != TUNE_PASS)
            {
                allLanesPass = GT_FALSE;
            }
            loopNum++;

        } while ((!allLanesPass) && (loopNum < 10));

        /* stop Align90 on each related serdes */
        mvHwsSerdesFixAlign90Stop(devNum, devNum, SerdesAutoTuneStatusSerdesNum,
                            (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneStatusSerdesNum), &serdesParams, ((status == TUNE_PASS) ? GT_TRUE : GT_FALSE));
        mvHwsSerdesRev2DfeCheck(devNum, devNum, SerdesAutoTuneStatusSerdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneStatusSerdesNum));
    }
    /* pack and output table fields */
    fieldOutput("%d%d%d",SerdesAutoTuneStatusSerdesNum,inFields[1],inFields[2]);
    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/**
* @internal wrHwsSerdesAutoTuneStatusGetNextEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesAutoTuneStatusGetNextEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      startSerdesNum;
    GT_U32      endSerdesNum;
    GT_U32      serdesNum;
    MV_HWS_AUTO_TUNE_STATUS rxStatus;
    MV_HWS_AUTO_TUNE_STATUS txStatus;
    MV_HWS_ALIGN90_PARAMS   serdesParams;
    MV_HWS_AUTO_TUNE_STATUS status;
    GT_BOOL allLanesPass;
    GT_U32 loopNum;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    startSerdesNum = inArgs[1];
    endSerdesNum = inArgs[2];
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
    rc = mvHwsSerdesAutoTuneStatus(devNum,devNum,SerdesAutoTuneStatusSerdesNum,
                                   (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneStatusSerdesNum),&rxStatus,&txStatus);

    inFields[1] = rxStatus;
    inFields[2] = txStatus;

    if ((rxStatus == TUNE_PASS) || (txStatus == TUNE_PASS))
    {
        /* start Align90 */
        CHECK_CMD_STATUS(mvHwsSerdesFixAlign90Start(devNum,devNum,SerdesAutoTuneStatusSerdesNum,
                        (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneStatusSerdesNum), &serdesParams));
        loopNum = 0;
        do
        {
            allLanesPass = GT_TRUE;
            hwsOsExactDelayPtr(devNum, devNum, 20);
            /* check status on related serdes */
            mvHwsSerdesFixAlign90Status(devNum, devNum, SerdesAutoTuneStatusSerdesNum,
                            (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneStatusSerdesNum), &status);
            if (status != TUNE_PASS)
            {
                allLanesPass = GT_FALSE;
            }
            loopNum++;

        } while ((!allLanesPass) && (loopNum < 10));

        /* stop Align90 on each related serdes */
        mvHwsSerdesFixAlign90Stop(devNum, devNum, SerdesAutoTuneStatusSerdesNum,
                            (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneStatusSerdesNum), &serdesParams, ((status == TUNE_PASS) ? GT_TRUE : GT_FALSE));
        mvHwsSerdesRev2DfeCheck(devNum, devNum, SerdesAutoTuneStatusSerdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, SerdesAutoTuneStatusSerdesNum));
    }
    /* pack and output table fields */
    fieldOutput("%d%d%d",SerdesAutoTuneStatusSerdesNum,inFields[1],inFields[2]);
    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/**
* @internal wrHwsSerdesTestGeneratorCfgSet function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesTestGeneratorCfgSet
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      serdesNum = inFields[0];
    MV_HWS_SERDES_TX_PATTERN txP;
    MV_HWS_SERDES_TEST_GEN_MODE mode;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
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
    rc = mvHwsSerdesTestGen(devNum, devNum, serdesNum, txP, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum), mode);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;
}

/**
* @internal wrHwsSerdesTestGeneratorStatusGetFirstEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesTestGeneratorStatusGetFirstEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8       devNum = inArgs[0];
    GT_U32      serdesNum;
    MV_HWS_SERDES_TX_PATTERN txP;
    MV_HWS_SERDES_TEST_GEN_STATUS status;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    if (inArgs[1] != 0)
    {
      galtisOutput(outArgs, GT_BAD_PARAM, "");
      return CMD_OK;
    }
      serdesNum = inArgs[2];
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

    rc = mvHwsSerdesTestGenStatus(devNum,devNum,serdesNum,(MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, serdesNum),txP,GT_TRUE,&status);

    inFields[1] = status.errorsCntr;
    inFields[3] = status.lockStatus;
    inFields[2] = status.txFramesCntr.l[0];

    /* pack and output table fields */
    fieldOutput("%d%d%d%d",serdesNum,inFields[1],inFields[2],inFields[3]);
    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/**
* @internal wrHwsSerdesTestGeneratorStatusGetNextEntry function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesTestGeneratorStatusGetNextEntry
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

static GT_U32 seqCurLine;
/*GT_U8 serdesDevNum = 0;

GT_VOID SetSerdesDevNum(GT_U8 devNum)
{
    serdesDevNum = devNum;
    hwsOsPrintf("Dev Num was set to %d\n", serdesDevNum);
}*/
/**
* @internal wrHwsSerdesSeqCfgGetFirst function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesSeqCfgGetFirst
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       rc;         /* Return code  */
    GT_U8           devNum;
    GT_U32          baseAddr, unitIndex;
    MV_POLLING_OP_PARAMS *polParam;
    MV_WRITE_OP_PARAMS   *wrParam;
    MV_DELAY_OP_PARAMS   *delOp;
    MV_OP_PARAMS         *genOp;
    MV_CFG_ELEMENT      cfgEl;
    MV_HWS_SERDES_TYPE  serdesType;

    seqCurLine = 0;
    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = inArgs[0];
    serdesType = (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum);

    rc = hwsSerdesSeqGet(devNum,devNum,(MV_HWS_COM_PHY_H_SUB_SEQ)inArgs[1], &cfgEl, 0);

    inFields[0] = 0;
    inFields[1] = cfgEl.op;

    if (serdesType < COM_PHY_28NM)
    {
        switch(inFields[1])
        {
        case WRITE_OP:
          wrParam = (MV_WRITE_OP_PARAMS *)cfgEl.params;
          mvUnitInfoGet(devNum, (MV_HWS_UNITS_ID)wrParam->indexOffset, &baseAddr, &unitIndex);
          inFields[2] = baseAddr;
          inFields[3] = wrParam->regOffset;
          inFields[4] = wrParam->data;
          inFields[5] = wrParam->mask;
          inFields[6] = 0;
          inFields[7] = 0;
          break;
        case DELAY_OP:
          delOp = (MV_DELAY_OP_PARAMS *)cfgEl.params;
          inFields[2] = 0;
          inFields[3] = 0;
          inFields[4] = 0;
          inFields[5] = 0;
          inFields[6] = delOp->delay;
          inFields[7] = 0;
          break;
        case POLLING_OP:
          polParam = (MV_POLLING_OP_PARAMS *)cfgEl.params;
          mvUnitInfoGet(devNum, (MV_HWS_UNITS_ID)polParam->indexOffset, &baseAddr, &unitIndex);
          inFields[2] = baseAddr;
          inFields[3] = polParam->regOffset;
          inFields[4] = polParam->data;
          inFields[5] = polParam->mask;
          inFields[6] = polParam->waitTime;
          inFields[7] = polParam->numOfLoops;
          break;
        }
    }
    else
    {
        genOp = (MV_OP_PARAMS *)cfgEl.params;
        switch(inFields[1])
        {
        case WRITE_OP:
            mvUnitInfoGet(devNum, (MV_HWS_UNITS_ID)genOp->unitId, &baseAddr, &unitIndex);
            inFields[2] = baseAddr;
            inFields[3] = genOp->regOffset;
            inFields[4] = genOp->operData;
            inFields[5] = genOp->mask;
            inFields[6] = 0;
            inFields[7] = 0;
            break;
        case DELAY_OP:
            inFields[2] = 0;
            inFields[3] = 0;
            inFields[4] = 0;
            inFields[5] = 0;
            inFields[6] = genOp->waitTime;
            inFields[7] = 0;
            break;
        case POLLING_OP:
            mvUnitInfoGet(devNum, (MV_HWS_UNITS_ID)genOp->unitId, &baseAddr, &unitIndex);
            inFields[2] = baseAddr;
            inFields[3] = genOp->regOffset;
            inFields[4] = genOp->operData;
            inFields[5] = genOp->mask;
            inFields[6] = genOp->waitTime;
            inFields[7] = genOp->numOfLoops;
            break;
        }
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d",inFields[0],inFields[1],inFields[2],inFields[3],inFields[4],inFields[5],inFields[6],inFields[7]);
    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/**
* @internal wrHwsSerdesSeqCfgGetNext function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesSeqCfgGetNext
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS       rc;         /* Return code  */
    GT_U8           devNum;
    MV_HWS_SERDES_TYPE serdesType;
    MV_POLLING_OP_PARAMS *polParam;
    MV_WRITE_OP_PARAMS   *wrParam;
    MV_DELAY_OP_PARAMS   *delOp;
    MV_OP_PARAMS         *genOp;
    MV_CFG_ELEMENT cfgEl;
    GT_U32          baseAddr, unitIndex;

    inFields[0] = ++seqCurLine;
    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = inArgs[0];
    serdesType = (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum);

    rc = hwsSerdesSeqGet(devNum,devNum,(MV_HWS_COM_PHY_H_SUB_SEQ)inArgs[1], seqCurLine, &cfgEl);
    if (rc == GT_NO_MORE)
    {
      galtisOutput(outArgs, GT_OK, "%d", -1);
      return CMD_OK;
    }
    inFields[1] = cfgEl.op;

    if (serdesType < COM_PHY_28NM)
    {
        switch(inFields[1])
        {
        case WRITE_OP:
          wrParam = (MV_WRITE_OP_PARAMS *)cfgEl.params;
          mvUnitInfoGet(devNum, (MV_HWS_UNITS_ID)wrParam->indexOffset, &baseAddr, &unitIndex);
          inFields[2] = baseAddr;
          inFields[3] = wrParam->regOffset;
          inFields[4] = wrParam->data;
          inFields[5] = wrParam->mask;
          inFields[6] = 0;
          inFields[7] = 0;
          break;
        case DELAY_OP:
          delOp = (MV_DELAY_OP_PARAMS *)cfgEl.params;
          inFields[2] = 0;
          inFields[3] = 0;
          inFields[4] = 0;
          inFields[5] = 0;
          inFields[6] = delOp->delay;
          inFields[7] = 0;
          break;
        case POLLING_OP:
          polParam = (MV_POLLING_OP_PARAMS *)cfgEl.params;
          mvUnitInfoGet(devNum, (MV_HWS_UNITS_ID)polParam->indexOffset, &baseAddr, &unitIndex);
          inFields[2] = baseAddr;
          inFields[3] = polParam->regOffset;
          inFields[4] = polParam->data;
          inFields[5] = polParam->mask;
          inFields[6] = polParam->waitTime;
          inFields[7] = polParam->numOfLoops;
          break;
        }
    }
    else
    {
        genOp = (MV_OP_PARAMS *)cfgEl.params;
        switch(inFields[1])
        {
        case WRITE_OP:
            mvUnitInfoGet(devNum, (MV_HWS_UNITS_ID)genOp->unitId, &baseAddr, &unitIndex);
            inFields[2] = baseAddr;
            inFields[3] = genOp->regOffset;
            inFields[4] = genOp->operData;
            inFields[5] = genOp->mask;
            inFields[6] = 0;
            inFields[7] = 0;
            break;
        case DELAY_OP:
            inFields[2] = 0;
            inFields[3] = 0;
            inFields[4] = 0;
            inFields[5] = 0;
            inFields[6] = genOp->waitTime;
            inFields[7] = 0;
            break;
        case POLLING_OP:
            mvUnitInfoGet(devNum, (MV_HWS_UNITS_ID)genOp->unitId, &baseAddr, &unitIndex);
            inFields[2] = baseAddr;
            inFields[3] = genOp->regOffset;
            inFields[4] = genOp->operData;
            inFields[5] = genOp->mask;
            inFields[6] = genOp->waitTime;
            inFields[7] = genOp->numOfLoops;
            break;
        }
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d",inFields[0],inFields[1],inFields[2],inFields[3],inFields[4],inFields[5],inFields[6],inFields[7]);
    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;

}

/**
* @internal wrHwsSerdesSeqCfgSet function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesSeqCfgSet
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;         /* Return code  */
    GT_U8                devNum;
    MV_HWS_UNITS_ID      unitId;
    MV_HWS_SERDES_TYPE serdesType;
    MV_POLLING_OP_PARAMS polParam;
    MV_WRITE_OP_PARAMS   wrParam;
    MV_DELAY_OP_PARAMS   delOp;
    MV_OP_PARAMS         genOp;
    MV_CFG_ELEMENT       cfgEl;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }
    devNum = inArgs[0];
    serdesType = (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum);

    if (serdesType < COM_PHY_28NM)
    {
        switch(inFields[1])
        {
        case WRITE_OP:
          mvUnitInfoGetByAddr(devNum, (GT_U32)inFields[2], &unitId);
          wrParam.indexOffset = unitId;
          wrParam.regOffset = inFields[3];
          wrParam.data = inFields[4];
          wrParam.mask = inFields[5];
          cfgEl.params = &wrParam;
          cfgEl.op = WRITE_OP;
          break;
        case DELAY_OP:
          delOp.delay = inFields[6];
          cfgEl.params = &delOp;
          cfgEl.op = DELAY_OP;
          break;
        case POLLING_OP:
          mvUnitInfoGetByAddr(devNum, (GT_U32)inFields[2], &unitId);
          polParam.indexOffset = unitId;
          polParam.regOffset = inFields[3];
          polParam.data = inFields[4];
          polParam.mask = inFields[5];
          polParam.waitTime = inFields[6];
          polParam.numOfLoops = inFields[7];
          cfgEl.params = &polParam;
          cfgEl.op = POLLING_OP;
          break;
        }
    }
    else
    {
        switch(inFields[1])
        {
        case WRITE_OP:
            mvUnitInfoGetByAddr(devNum, (GT_U32)inFields[2], &unitId);
            genOp.unitId = unitId;
            genOp.regOffset = inFields[3];
            genOp.operData = inFields[4];
            genOp.mask = inFields[5];
            cfgEl.op = WRITE_OP;
            break;
        case DELAY_OP:
            genOp.waitTime = inFields[6];
            cfgEl.op = DELAY_OP;
            break;
        case POLLING_OP:
            mvUnitInfoGetByAddr(devNum, (GT_U32)inFields[2], &unitId);
            genOp.unitId = unitId;
            genOp.regOffset = inFields[3];
            genOp.operData = inFields[4];
            genOp.mask = inFields[5];
            genOp.waitTime = inFields[6];
            genOp.numOfLoops = inFields[7];
            cfgEl.op = POLLING_OP;
            break;
        }
        cfgEl.params = &genOp;
    }

    rc = hwsSerdesSeqSet(devNum,devNum,((inFields[0] == 0) ? GT_TRUE : GT_FALSE),
      (MV_HWS_COM_PHY_H_SUB_SEQ)inArgs[1], &cfgEl, inFields[0]);

    galtisOutput(outArgs, rc, "");
    return CMD_OK;

}

static GT_U32 wrCalibSerdesNum;

/**
* @internal wrSerdesCalibrationGetFirst function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrSerdesCalibrationGetFirst
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;
    int        i;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    /* Parameters List */
    GT_U32  DevNum         = (GT_U32)inArgs[0];
    GT_U32  SerdesNumStart = (GT_U32)inArgs[1];
    GT_U32  SerdesNumEnd   = (GT_U32)inArgs[2];

    MV_HWS_CALIBRATION_RESULTS  results;

    if (SerdesNumEnd <  SerdesNumStart)
    {
        SerdesNumEnd = SerdesNumStart;
    }

    wrCalibSerdesNum = SerdesNumStart;
    res = mvHwsSerdesCalibrationStatus(DevNum, DevNum, wrCalibSerdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(DevNum, wrCalibSerdesNum), &results);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, GT_FAIL, "%f");
        return CMD_OK;
    }

    /* Fields List */
    inFields[0] = (GT_32)wrCalibSerdesNum;      //SerdesNum
    inFields[1] = (GT_32)results.calDone;       //Cal_Done
    inFields[2] = (GT_32)results.txImpCal;      //TxImpCal
    inFields[3] = (GT_32)results.rxImpCal;      //RxImpCal
    inFields[4] = (GT_32)results.ProcessCal;    //ProcessCal
    inFields[5] = (GT_32)results.speedPll;      //SpeedPll
    inFields[6] = (GT_32)results.sellvTxClk;    //SELLV_TxClk
    inFields[7] = (GT_32)results.sellvTxData;   //SELLV_TxData
    inFields[8] = (GT_32)results.sellvTxIntp;   //SELLV_TxIntp
    inFields[9] = (GT_32)results.sellvTxDrv;    //SELLV_TxDrv
    inFields[10] = (GT_32)results.sellvTxDig;   //SELLV_RxDig
    inFields[11] = (GT_32)results.sellvRxSample;//SELLV_RxSampl
    inFields[12] = (GT_32)results.ffeDone;      //FFE_Done

    for (i = 0; i < 8; i++)
    {
        inFields[13+i] = results.ffeCal[i];     //FFE_Cal0[i]
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15], inFields[16], inFields[17], inFields[18], inFields[19], inFields[20]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/**
* @internal wrSerdesCalibrationGetNext function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrSerdesCalibrationGetNext
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS  res;
    int        i;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    /* Parameters List */
    GT_U32  DevNum         = (GT_U32)inArgs[0];
    GT_U32  SerdesNumEnd   = (GT_U32)inArgs[2];

    MV_HWS_CALIBRATION_RESULTS  results;
    wrCalibSerdesNum++;

    if (wrCalibSerdesNum > SerdesNumEnd)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    res = mvHwsSerdesCalibrationStatus(DevNum, DevNum, wrCalibSerdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(DevNum, wrCalibSerdesNum), &results);
    if (res != GT_OK)
    {
        galtisOutput(outArgs, GT_FAIL, "%f");
        return CMD_OK;
    }

    /* Fields List */
    inFields[0] = (GT_32)wrCalibSerdesNum;      //SerdesNum
    inFields[1] = (GT_32)results.calDone;       //Cal_Done
    inFields[2] = (GT_32)results.txImpCal;      //TxImpCal
    inFields[3] = (GT_32)results.rxImpCal;      //RxImpCal
    inFields[4] = (GT_32)results.ProcessCal;    //ProcessCal
    inFields[5] = (GT_32)results.speedPll;      //SpeedPll
    inFields[6] = (GT_32)results.sellvTxClk;    //SELLV_TxClk
    inFields[7] = (GT_32)results.sellvTxData;   //SELLV_TxData
    inFields[8] = (GT_32)results.sellvTxIntp;   //SELLV_TxIntp
    inFields[9] = (GT_32)results.sellvTxDrv;    //SELLV_TxDrv
    inFields[10] = (GT_32)results.sellvTxDig;   //SELLV_RxDig
    inFields[11] = (GT_32)results.sellvRxSample;//SELLV_RxSampl
    inFields[12] = (GT_32)results.ffeDone;      //FFE_Done

    for (i = 0; i < 8; i++)
    {
        inFields[13+i] = results.ffeCal[i];     //FFE_Cal0[i]
    }

    /* pack and output table fields */
    fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2], inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8], inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15], inFields[16], inFields[17], inFields[18], inFields[19], inFields[20]);

    galtisOutput(outArgs, GT_OK, "%f");
    return CMD_OK;
}

/**
* @internal wrHwsSerdesScanSamplerGetFirst function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesScanSamplerGetFirst
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MV_HWS_SAMPLER_RESULTS samplerResults;
    GT_STATUS  retVal;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    /* Parameters List */
    GT_U32  DevNum        = (GT_U32)inArgs[0];
    GT_U32  CoreNum       = (GT_U32)inArgs[1];
    GT_U32  SerdesNum     = (GT_U32)inArgs[2];
    GT_U32  WaitTime      = (GT_U32)inArgs[3];
    GT_U32  SamplerSelect = (GT_U32)inArgs[4];
    GT_U32  Thershold     = (GT_U32)inArgs[5];


    hwsOsPrintf("ScanSampler: DevNum %d SerdesNum %d, WaitTime %d, SamplerSelect %d Thershold %d\n",
             DevNum, SerdesNum, WaitTime, SamplerSelect, Thershold);

    retVal = mvHwsSerdesScanSampler(DevNum, DevNum, SerdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(DevNum, SerdesNum),
                                    WaitTime, SamplerSelect, Thershold, &samplerResults);

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

/**
* @internal wrHwsSerdesScanSamplerGetNext function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesScanSamplerGetNext
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    galtisOutput(outArgs, GT_OK, "%d", -1);
    return CMD_OK;
}

static GT_U32 SerdesScanSamplerSerdesNum;
#define MAX_SAMPLER_NUM     7

/**
* @internal wrHwsSerdesScanSamplerExtGetFirst function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesScanSamplerExtGetFirst
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32  i;
    GT_STATUS  retVal;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    /* Parameters List */
    GT_U32  DevNum        = (GT_U32)inArgs[0];
    /*GT_U32  CoreNum       = (GT_U32)inArgs[1];*/
    GT_U32  SerdesStart   = (GT_U32)inArgs[2];
    GT_U32  SerdesEnd     = (GT_U32)inArgs[3];
    GT_U32  WaitTime      = (GT_U32)inArgs[4];
    GT_U32  SamplerStart  = (GT_U32)inArgs[5];
    GT_U32  SamplerEnd    = (GT_U32)inArgs[6];
    GT_U32  Thershold     = (GT_U32)inArgs[7];

    MV_HWS_SAMPLER_RESULTS samplerResults[MAX_SAMPLER_NUM] = {0};
    osMemSet(samplerResults, 0, sizeof(MV_HWS_SAMPLER_RESULTS) * MAX_SAMPLER_NUM);

    SerdesScanSamplerSerdesNum = SerdesStart;

    hwsOsPrintf("ScanSampler (2): DevNum %d SerdesStart %d, SerdesEnd %d, WaitTime %d, SamplerStart %d SamplerEnd %d, Thershold %d\n",
             DevNum, SerdesStart, SerdesEnd, WaitTime, SamplerStart, SamplerEnd, Thershold);

    if(SamplerStart > SamplerEnd)
    {
        hwsOsPrintf("changing SamplerEnd to SamplerStart (%d)\n", SamplerStart);
        SamplerEnd = SamplerStart;
    }

    for(i = SamplerStart; i < (SamplerEnd + 1); i++)
    {
        hwsOsPrintf("Serdes %d: Scan sampler %d\n", SerdesScanSamplerSerdesNum, i);
        retVal = mvHwsSerdesScanSampler(DevNum, DevNum, SerdesScanSamplerSerdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(DevNum, SerdesScanSamplerSerdesNum),
                                        WaitTime, i, Thershold, &samplerResults[i]);
        if(retVal != GT_OK)
        {
            hwsOsPrintf("Serdes %d: Failed to scan sampler %d (res 0x%x)\n", SerdesScanSamplerSerdesNum, i, retVal);
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

/**
* @internal wrHwsSerdesScanSamplerExtGetNext function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesScanSamplerExtGetNext
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32  i;
    GT_STATUS  retVal;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    /* Parameters List */
    GT_U32  DevNum        = (GT_U32)inArgs[0];
    /*GT_U32  CoreNum       = (GT_U32)inArgs[1];*/
    GT_U32  SerdesStart   = (GT_U32)inArgs[2];
    GT_U32  SerdesEnd     = (GT_U32)inArgs[3];
    GT_U32  WaitTime      = (GT_U32)inArgs[4];
    GT_U32  SamplerStart  = (GT_U32)inArgs[5];
    GT_U32  SamplerEnd    = (GT_U32)inArgs[6];
    GT_U32  Thershold     = (GT_U32)inArgs[7];

    MV_HWS_SAMPLER_RESULTS samplerResults[MAX_SAMPLER_NUM] = {0};

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
        hwsOsPrintf("Serdes (2) %d: Scan sampler %d\n", SerdesScanSamplerSerdesNum, i);
        retVal = mvHwsSerdesScanSampler(DevNum, DevNum, SerdesScanSamplerSerdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(DevNum, SerdesScanSamplerSerdesNum),
                                        WaitTime, i, Thershold, &samplerResults[i]);
        if(retVal != GT_OK)
        {
            hwsOsPrintf("Serdes %d: Failed to scan sampler %d (res 0x%x)\n", SerdesScanSamplerSerdesNum, i, retVal);
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

/**
* @internal wrHwsSerdesEomGetGetFirst function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesEomGetGetFirst
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    MV_HWS_SERDES_EOM_RES   results;
    GT_STATUS  retVal;

    /*check for valid arguments */
    if(!inArgs || !outArgs || !inFields)
    {
        return CMD_AGENT_ERROR;
    }

    /* Parameters List */
    GT_U32  DevNum        = (GT_U32)inArgs[0];
    GT_U32  CoreNum       = (GT_U32)inArgs[1];
    GT_U32  SerdesNum     = (GT_U32)inArgs[2];
    GT_U32  WaitTime      = (GT_U32)inArgs[3];

    hwsOsPrintf("EOM Get: DevNum %d CoreNum %d SerdesNum %d, WaitTime\n",
             DevNum, CoreNum, SerdesNum, WaitTime);

    results.upperMatrix = eomUpperMatrix;
    results.lowerMatrix = eomLowerMatrix;

    retVal = mvHwsSerdesEOMGet(DevNum, DevNum, SerdesNum, (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(DevNum, SerdesNum),
                               WaitTime, &results);

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

/**
* @internal wrHwsSerdesEomGetGetNext function
* @endinternal
*
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
* @retval CMD_SYNTAX_ERROR         - on fail
*/
CMD_STATUS wrHwsSerdesEomGetGetNext
(
    IN  GT_32 inArgs[CMD_MAX_ARGS],
    IN  GT_32 inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
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
{"HwsSerdesPowerCtrlSet", &wrHwsSerdesPowerCtrlSetEntry, 1,  7},
{"HwsSerdesAutoTuneStatusGetFirst", &wrHwsSerdesAutoTuneStatusGetFirstEntry, 3,  0},
{"HwsSerdesAutoTuneStatusGetNext", &wrHwsSerdesAutoTuneStatusGetNextEntry, 3,  0},
{"HwsSerdesDfeCfgSet", &wrHwsSerdesDfeCfgSetEntry, 3,  22},
{"HwsSerdesDfeCfgGetFirst", &wrHwsSerdesDfeCfgGetFirstEntry, 3,  0},
{"HwsSerdesDfeCfgGetNext", &wrHwsSerdesDfeCfgGetNextEntry, 3,  0},
{"HwsSerdesResetCfgSet", &wrHwsSerdesResetCfgSetEntry, 1,  4},
{"HwsSerdesCalibartionGetFirst", &wrSerdesCalibrationGetFirst, 3, 0},
{"HwsSerdesCalibartionGetNext", &wrSerdesCalibrationGetNext, 3, 0},

{"HwsSerdesTestGeneratorCfgSet", &wrHwsSerdesTestGeneratorCfgSet, 2,  3},
{"HwsSerdesTestGeneratorStatusGetFirst", &wrHwsSerdesTestGeneratorStatusGetFirstEntry, 4,  0},
{"HwsSerdesTestGeneratorStatusGetNext", &wrHwsSerdesTestGeneratorStatusGetNextEntry, 4,  0},

{"HwsSerdesSeqCfgGetFirst", &wrHwsSerdesSeqCfgGetFirst, 2,  0},
{"HwsSerdesSeqCfgGetNext", &wrHwsSerdesSeqCfgGetNext, 2,  0},
{"HwsSerdesSeqCfgSet", &wrHwsSerdesSeqCfgSet, 2,  8},

{"HwsSerdesScanSamplerGetFirst", &wrHwsSerdesScanSamplerGetFirst, 6, 0},
{"HwsSerdesScanSamplerGetNext", &wrHwsSerdesScanSamplerGetNext, 6, 0},

{"HwsSerdesScanSamplerExtGetFirst", &wrHwsSerdesScanSamplerExtGetFirst, 8, 0},
{"HwsSerdesScanSamplerExtGetNext", &wrHwsSerdesScanSamplerExtGetNext, 8, 0},

{"HwsSerdesEomGetGetFirst", &wrHwsSerdesEomGetGetFirst, 4, 0},
{"HwsSerdesEomGetGetNext", &wrHwsSerdesEomGetGetNext, 4, 0},
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitHwsSerdesInit function
* @endinternal
*
* @brief   Library database initialization function.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
GT_STATUS cmdLibInitHwsSerdesInit()
{
    return cmdInitLibrary(dbCommands, numCommands);
}



