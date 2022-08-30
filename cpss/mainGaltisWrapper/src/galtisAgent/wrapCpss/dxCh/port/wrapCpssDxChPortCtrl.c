/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wrapPhyCpssDxCh.c
*
* DESCRIPTION:
*       Wrapper functions for Phy cpss.dxCh functions
*
* FILE REVISION NUMBER:
*       $Revision: 103 $
*
*******************************************************************************/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/**
* @internal wrCpssDxChPortSerdesManualTxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes TX in HW.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssDxChPortSerdesTuningSet.
*
*/
CMD_STATUS wrCpssDxChPortSerdesManualTxConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U8                   portNum;
    GT_U32                  laneNum;
    CPSS_PORT_SERDES_TX_CONFIG_STC  serdesCfg;

    /* check for valid arguments */
    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_U8)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        serdesCfg.type = CPSS_PORT_SERDES_AVAGO_E;
        serdesCfg.txTune.avago.post = inArgs[4];
        serdesCfg.txTune.avago.pre = inArgs[5];
    }
    else
    {
        serdesCfg.type = CPSS_PORT_SERDES_COMPHY_H_E;
        serdesCfg.txTune.comphy.txAmp = (GT_U32)inArgs[3];
        serdesCfg.txTune.comphy.txAmpAdjEn = (GT_BOOL)inArgs[4];
        serdesCfg.txTune.comphy.emph0 = (GT_32)inArgs[5];
        serdesCfg.txTune.comphy.emph1 = (GT_32)inArgs[6];
        serdesCfg.txTune.comphy.txAmpShft = (GT_BOOL)inArgs[7];
    }
    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortSerdesManualTxConfigSet(devNum, portNum, laneNum,
                                                 &serdesCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesManualRxConfigSet function
* @endinternal
*
* @brief   Configure specific parameters of serdes RX in HW.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*
* @note Pay attention: every serdes power up restore CPSS default configurations,
*       so this API should be egaged by application after every serdes power up
*       or use cpssDxChPortSerdesTuningSet.
*
*/
CMD_STATUS wrCpssDxChPortSerdesManualRxConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U8                   portNum;
    GT_U32                  laneNum;
    CPSS_PORT_SERDES_RX_CONFIG_STC  serdesCfg;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_U8)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        serdesCfg.type = CPSS_PORT_SERDES_AVAGO_E;
        serdesCfg.rxTune.avago.sqlch = (GT_U32)inArgs[3];
        serdesCfg.rxTune.avago.LF = (GT_U32)inArgs[4];
        serdesCfg.rxTune.avago.HF = (GT_U32)inArgs[5];
        serdesCfg.rxTune.avago.DC = (GT_U32)inArgs[6];
        serdesCfg.rxTune.avago.BW = (GT_U32)inArgs[7];
    }
    else
    {
        serdesCfg.type = CPSS_PORT_SERDES_COMPHY_H_E;
        serdesCfg.rxTune.comphy.sqlch = (GT_U32)inArgs[3];
        serdesCfg.rxTune.comphy.ffeR = (GT_U32)inArgs[4];
        serdesCfg.rxTune.comphy.ffeC = (GT_U32)inArgs[5];
        serdesCfg.rxTune.comphy.align90 = (GT_U32)inArgs[7];
    }
    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortSerdesManualRxConfigSet(devNum, portNum, laneNum,
                                                 &serdesCfg);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesManualRxConfigGet function
* @endinternal
*
* @brief   Gets and shows configured specific parameters of serdes RX in HW.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*/
CMD_STATUS wrCpssDxChPortSerdesManualRxConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U8                   portNum;
    GT_U32                  laneNum;
    CPSS_PORT_SERDES_RX_CONFIG_STC  serdesRxCfg;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_U8)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    result = cpssDxChPortSerdesManualRxConfigGet( devNum, portNum, laneNum, &serdesRxCfg);

    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        galtisOutput(outArgs, result, "%d%d%d%d%d%d",
            serdesRxCfg.rxTune.avago.sqlch, serdesRxCfg.rxTune.avago.LF, serdesRxCfg.rxTune.avago.HF,
                     serdesRxCfg.rxTune.avago.DC, serdesRxCfg.rxTune.avago.BW);
    } else
    {
        galtisOutput(outArgs, result, "%d%d%d%d%d%d",
            serdesRxCfg.rxTune.comphy.sqlch, serdesRxCfg.rxTune.comphy.ffeR, serdesRxCfg.rxTune.comphy.ffeC,
                     serdesRxCfg.rxTune.comphy.dcGain, serdesRxCfg.rxTune.comphy.bandWidth);

    }
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Gets and shows configured specific parameters of serdes TX in HW.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if serdes was not intialized
* @retval GT_OUT_OF_RANGE          - parameter out of range
*/
CMD_STATUS wrCpssDxChPortSerdesManualTxConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_U8                   portNum;
    GT_U32                  laneNum;
    CPSS_PORT_SERDES_TX_CONFIG_STC  serdesTxCfg;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_U8)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    result = cpssDxChPortSerdesManualTxConfigGet( devNum, portNum, laneNum, &serdesTxCfg);
    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        galtisOutput(outArgs, result, "%d%d%d",serdesTxCfg.txTune.avago.atten, serdesTxCfg.txTune.avago.post, serdesTxCfg.txTune.avago.pre);
    }
    else {
        galtisOutput(outArgs, result, "%d%d%d",serdesTxCfg.txTune.comphy.txAmp, serdesTxCfg.txTune.comphy.emph0, serdesTxCfg.txTune.comphy.emph1);
    }
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortFecModeSet function
* @endinternal
*
* @brief   Configure Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortFecModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U8 portNum;
    CPSS_DXCH_PORT_FEC_MODE_ENT mode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portNum = (GT_U8)inArgs[1];
    mode = (CPSS_DXCH_PORT_FEC_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortFecModeSet(devNum, portNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortFecModeGet function
* @endinternal
*
* @brief   Read current Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortFecModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result = GT_OK;
    GT_U8                       devNum;
    GT_U8                       portNum;
    CPSS_DXCH_PORT_FEC_MODE_ENT mode = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_U8)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortFecModeGet(devNum, portNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPcsLoopbackModeSet function
* @endinternal
*
* @brief   Configure loopback of specific type on PCS.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortPcsLoopbackModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U8       portNum;
    CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT    mode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_U8)inArgs[1];
    mode = (CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortPcsLoopbackModeSet(devNum, portNum, mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPcsLoopbackModeGet function
* @endinternal
*
* @brief   Configure loopback of specific type on PCS.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortPcsLoopbackModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U8       portNum;
    CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT mode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_U8)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortPcsLoopbackModeGet(devNum, portNum, &mode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", mode);

    return CMD_OK;
}


/**
* @internal wrCpssDxChPortExtendedModeEnableSet function
* @endinternal
*
* @brief   Define which GE and XG MAC ports 9 and 11 of every mini-GOP will use -
*         from local mini-GOP or extended from other mini-GOP
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For now if application interested to implement GE and XG modes of ports 9
*       and 11 of every mini-GOP over extended MAC's, it can call this function
*       at init stage once for port 9 and once for port 11 and enough
*
*/
CMD_STATUS wrCpssDxChPortExtendedModeEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U8       portNum;
    GT_BOOL     enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_U8)inArgs[1];
    enable = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortExtendedModeEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortExtendedModeEnableGet function
* @endinternal
*
* @brief   Read which GE and XG MAC ports 9 and 11 of every mini-GOP will use -
*         from local mini-GOP or extended from other mini-GOP
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - enablePtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortExtendedModeEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U8       portNum;
    GT_BOOL     enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_U8)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortExtendedModeEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesLaneTuningSet function
* @endinternal
*
* @brief   SerDes lane fine tuning values set.
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set tuning parameters for lane in SW DB, then
*       cpssDxChPortSerdesPowerStatusSet or cpssDxChPortModeSpeedSet will write
*       them to HW.
*
*/
CMD_STATUS wrCpssDxChPortSerdesLaneTuningSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS rc;
    GT_U8     devNum;
    GT_U32    portGroupNum;
    GT_U32    laneNum;
    CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency;
    CPSS_PORT_SERDES_TUNE_STC tuneValues;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portGroupNum    = (GT_U32)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];
    serdesFrequency = (CPSS_DXCH_PORT_SERDES_SPEED_ENT)inArgs[3];

    cpssOsMemSet(&tuneValues, 0, sizeof(tuneValues));

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        tuneValues.type = CPSS_PORT_SERDES_AVAGO_E;
        tuneValues.rxTune.avago.BW        = (GT_U32)inFields[0];
        tuneValues.rxTune.avago.DC        = (GT_U32)inFields[1];
        tuneValues.rxTune.avago.HF        = (GT_U32)inFields[2];
        tuneValues.rxTune.avago.LF        = (GT_U32)inFields[4];
        tuneValues.rxTune.avago.sqlch     = (GT_U32)inFields[5];
        tuneValues.txTune.avago.atten     = (GT_U32)inFields[6];
        tuneValues.txTune.avago.pre       = inFields[7];
        tuneValues.txTune.avago.post      = inFields[8];
    }
    else
    {
        tuneValues.type = CPSS_PORT_SERDES_COMPHY_H_E;
        tuneValues.rxTune.comphy.dfe      = (GT_U32)inFields[0];
        tuneValues.rxTune.comphy.ffeC     = (GT_U32)inFields[1];
        tuneValues.rxTune.comphy.ffeR     = (GT_U32)inFields[2];
        tuneValues.rxTune.comphy.ffeS     = (GT_U32)inFields[3];
        tuneValues.rxTune.comphy.sampler  = (GT_U32)inFields[4];
        tuneValues.rxTune.comphy.sqlch    = (GT_U32)inFields[5];
        tuneValues.txTune.comphy.txAmp    = (GT_U32)inFields[6];
        tuneValues.txTune.comphy.txAmpAdj = (GT_U32)inFields[7];
        tuneValues.txTune.comphy.emph0    = (GT_U32)inFields[8];
        tuneValues.txTune.comphy.txEmphEn = (GT_BOOL)inFields[9];

        if(PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(devNum))
        {
            GT_U32  i;

            tuneValues.txTune.comphy.emph1 = (GT_U32)inFields[10];
            tuneValues.rxTune.comphy.align90 = (GT_U32)inFields[11];
            tuneValues.txTune.comphy.txEmphEn1 = (GT_BOOL)inFields[12];
            tuneValues.txTune.comphy.txAmpShft = (GT_BOOL)inFields[13];

            for(i = 0; i < CPSS_PORT_DFE_VALUES_ARRAY_SIZE_CNS; i++)
            {
                tuneValues.rxTune.comphy.dfeValsArray[i] = (GT_U32)inFields[14+i];
            }
        }
    }

    rc = cpssDxChPortSerdesLaneTuningSet(devNum,
                                         portGroupNum,
                                         laneNum,
                                         serdesFrequency,
                                         &tuneValues);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesLaneTuningGet function
* @endinternal
*
* @brief   Get SerDes lane fine tuning values.
*
* @note   APPLICABLE DEVICES:      xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get tuning parameters for lane from SW DB.
*
*/
CMD_STATUS wrCpssDxChPortSerdesLaneTuningGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS rc;
    GT_U8     devNum;
    GT_U32    portGroupNum;
    GT_U32    laneNum;
    CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency;
    CPSS_PORT_SERDES_TUNE_STC tuneValues;
    GT_U32  i;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portGroupNum    = (GT_U32)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];
    serdesFrequency = (CPSS_DXCH_PORT_SERDES_SPEED_ENT)inArgs[3];

    rc = cpssDxChPortSerdesLaneTuningGet(devNum, portGroupNum, laneNum,
                                         serdesFrequency,
                                         &tuneValues);
    if (rc != GT_OK)
    {
        galtisOutput(outArgs, rc, "cpssDxChPortSerdesTuningGet");
        return CMD_OK;
    }

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        inFields[0] = tuneValues.rxTune.avago.BW ;
        inFields[1] = tuneValues.rxTune.avago.DC;
        inFields[2] = tuneValues.rxTune.avago.HF;
        inFields[3] = 0;
        inFields[4] = tuneValues.rxTune.avago.LF;
        inFields[5] = tuneValues.rxTune.avago.sqlch;
        inFields[6] = tuneValues.txTune.avago.atten;
        inFields[7] = tuneValues.txTune.avago.pre;
        inFields[8] = tuneValues.txTune.avago.post;
        fieldOutput("%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                        inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8]);
    }
    else
    {
        inFields[0] = tuneValues.rxTune.comphy.dfe;
        inFields[1] = tuneValues.rxTune.comphy.ffeC;
        inFields[2] = tuneValues.rxTune.comphy.ffeR;
        inFields[3] = tuneValues.rxTune.comphy.ffeS;
        inFields[4] = tuneValues.rxTune.comphy.sampler;
        inFields[5] = tuneValues.rxTune.comphy.sqlch;
        inFields[6] = tuneValues.txTune.comphy.txAmp;
        inFields[7] = tuneValues.txTune.comphy.txAmpAdj;
        inFields[8] = tuneValues.txTune.comphy.emph0;
        inFields[9] = tuneValues.txTune.comphy.txEmphEn;

        if(PRV_CPSS_DXCH_LION2_B0_AND_ABOVE_CHECK_MAC(devNum))
        {
            inFields[10] = tuneValues.txTune.comphy.emph1;
            inFields[11] = tuneValues.rxTune.comphy.align90;
            inFields[12] = tuneValues.txTune.comphy.txEmphEn1;
            inFields[13] = tuneValues.txTune.comphy.txAmpShft;

            for(i = 0; i < CPSS_PORT_DFE_VALUES_ARRAY_SIZE_CNS; i++)
            {
                inFields[14+i] = tuneValues.rxTune.comphy.dfeValsArray[i];
            }
        }
        else
        {
            for(i = 10; i < 14+CPSS_PORT_DFE_VALUES_ARRAY_SIZE_CNS; i++)
            {
                inFields[i] = 0;
            }
        }

        /* pack and output table fields */
        fieldOutput("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", inFields[0], inFields[1], inFields[2],
                        inFields[3], inFields[4], inFields[5], inFields[6], inFields[7], inFields[8],
                        inFields[9], inFields[10], inFields[11], inFields[12], inFields[13], inFields[14], inFields[15],
                        inFields[16], inFields[17], inFields[18], inFields[19]);
    }
    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX data, TX data or both
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat3; xCat2; Bobcat2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_INIT_ERROR            - on Hws initialization failre
*
* @note Debug function.
*
*/
CMD_STATUS wrCpssDxChPortSerdesErrorInject
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8    devNum;
    GT_U32   portGroup;
    GT_U32   serdesNum;
    GT_U32   numOfBits;
    CPSS_PORT_DIRECTION_ENT  direction;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portGroup = (GT_U32)inArgs[1];
    serdesNum = (GT_U32)inArgs[2];
    numOfBits = (GT_U32)inArgs[3];
    direction = (CPSS_PORT_DIRECTION_ENT)inArgs[4];

    rc = cpssDxChPortSerdesErrorInject(devNum, portGroup, serdesNum, numOfBits,direction);

    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesLanePolaritySet function
* @endinternal
*
* @brief   Set the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2;
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note - If serdes initialized: Set the Tx/Rx polarity parameters for lane
*       in both HW and SW DB
*       - If serdes was not initialized: Set the Tx/Rx polarity parameters
*       for lane only in SW DB
*
*/
CMD_STATUS wrCpssDxChPortSerdesLanePolaritySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8       devNum;
    GT_U32      portGroupNum;
    GT_U32      laneNum;
    GT_BOOL     invertTx;
    GT_BOOL     invertRx;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portGroupNum    = (GT_U32)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];
    invertTx        = (GT_BOOL)inArgs[3];
    invertRx        = (GT_BOOL)inArgs[4];

    rc = cpssDxChPortSerdesLanePolaritySet(devNum, portGroupNum, laneNum, invertTx, invertRx);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesLanePolarityGet function
* @endinternal
*
* @brief   Get the SerDes lane polarity values.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; xCat3; Bobcat2;
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_OUT_OF_RANGE          - one of tune values is out of range
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - HW does not support the requested operation
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Get the Tx/Rx polarity parameters for lane from HW if serdes initialized
*       or from SW DB if serdes was not initialized
*
*/
CMD_STATUS wrCpssDxChPortSerdesLanePolarityGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8       devNum;
    GT_U32      portGroupNum;
    GT_U32      laneNum;
    GT_BOOL     invertTx;
    GT_BOOL     invertRx;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portGroupNum    = (GT_U32)inArgs[1];
    laneNum         = (GT_U32)inArgs[2];

    rc = cpssDxChPortSerdesLanePolarityGet(devNum, portGroupNum, laneNum, &invertTx, &invertRx);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "%d %d", invertTx, invertRx);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPcsResetSet function
* @endinternal
*
* @brief   Set/unset the PCS reset for given mode on port.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Reset only PCS unit used by port for currently configured interface.
*
*/
CMD_STATUS wrCpssDxChPortPcsResetSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   rc;
    GT_U8                          devNum;
    GT_PHYSICAL_PORT_NUM           portNum;
    CPSS_PORT_PCS_RESET_MODE_ENT   mode;
    GT_BOOL                        state;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[0];
    portNum    = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    mode       = (CPSS_PORT_PCS_RESET_MODE_ENT)inArgs[2];
    state      = (GT_BOOL)inArgs[3];

    rc = cpssDxChPortPcsResetSet(devNum, portNum, mode, state);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, rc, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortRefClockSourceOverrideEnableSet function
* @endinternal
*
* @brief   Enables/disables reference clock source override for port create.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, refClockSource
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortRefClockSourceOverrideEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         overrideEnable;
    CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSource;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    overrideEnable = (GT_BOOL)inArgs[2];
    refClockSource = (CPSS_PORT_REF_CLOCK_SOURCE_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortRefClockSourceOverrideEnableSet(devNum, portNum, overrideEnable, refClockSource);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;

}

/**
* @internal wrCpssDxChPortRefClockSourceOverrideEnableGet function
* @endinternal
*
* @brief   Gets status of reference clock source override for port create.
*
* @note   APPLICABLE DEVICES:      xCat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortRefClockSourceOverrideEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         overrideEnable;
    CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSource;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortRefClockSourceOverrideEnableGet(devNum, portNum, &overrideEnable, &refClockSource);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", overrideEnable, refClockSource);

    return CMD_OK;

}


/**
* @internal wrCpssDxChGetNext function
* @endinternal
*
* @brief   This function is called for ending table printing.
*
* @note   APPLICABLE DEVICES:      All DxCh devices
*
* @retval GT_OK                    - on success
*/
static CMD_STATUS wrCpssDxChGetNext

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


/**
* @internal wrCpssDxChPortResourceTmBandwidthSet function
* @endinternal
*
* @brief   Define system bandwith through TM port. Following this configuration
*         TM port resources will be allocated.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note:
*       - API should not be called under traffic
*       - zero bandwidth will release resources of TM for non TM ports
*
*/
CMD_STATUS wrCpssDxChPortResourceTmBandwidthSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      tmBandwidthMbps;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    tmBandwidthMbps = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPortResourceTmBandwidthSet(devNum, tmBandwidthMbps);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssDxChPortResourceTmBandwidthGet function
* @endinternal
*
* @brief   Get system bandwith through TM port. Following this configuration
*         TM port resources will be allocated.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortResourceTmBandwidthGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_U32      tmBandwidthMbps;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum  = (GT_U8)inArgs[0];
    tmBandwidthMbps = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChPortResourceTmBandwidthGet(devNum, &tmBandwidthMbps);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", tmBandwidthMbps);

    return CMD_OK;

}

/**
* @internal wrCpssDxChPortMacPcsStatusGet function
* @endinternal
*
* @brief   port mac pcs status collector.
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] portStatus             - port status:
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is
*         NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChPortMacPcsStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_DXCH_PORT_STATUS_STC  portStatus;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    result = cpssDxChPortMacPcsStatusGet( devNum, portNum, &portStatus);

    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",portStatus.rsfecAligned, portStatus.hiBer, portStatus.alignDone , portStatus.lpcsAnDone
                 ,portStatus.lpcsRxSync ,portStatus.lpcsLinkStatus ,portStatus.hiSer ,portStatus.degradeSer ,portStatus.alignLock ,
                 portStatus.linkStatus ,portStatus.linkOk ,portStatus.phyLos ,portStatus.rxRemFault ,portStatus.rxLocFault ,portStatus.fault ,
                 portStatus.pcsReceiveLink ,portStatus.devicePresent ,portStatus.transmitFault ,portStatus.receiveFault ,portStatus.berCounter ,portStatus.errorBlockCounter);
    return CMD_OK;
}

/**
* @internal wrCpssDxChRsFecCounterGet function
* @endinternal
*
* @brief   Return number of codewords corrected by RS-FEC.
*
* @note   APPLICABLE DEVICES:       Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond;
*         DxCh2DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
*
* @param[out] corecctedErrorPtr          - number of corrected
*                                        errors
* @param[out] uncorrectedErrorPtr        - number of uncorrected
*                                        errors
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
CMD_STATUS wrCpssDxChRsFecCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_RSFEC_COUNTERS_STC rsfecCounters;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    result = cpssDxChRsFecCounterGet(devNum, portNum, &rsfecCounters);

    galtisOutput(outArgs, result, "%d%d",rsfecCounters.correctedFecCodeword, rsfecCounters.uncorrectedFecCodeword);
    return CMD_OK;
}

/**
* @internal wrCpssDxChFcFecCounterGet function
* @endinternal
*
* @brief   Return number of codewords corrected and uncorrected
*          by FC-FEC.
*
* @note   APPLICABLE DEVICES:       Aldrin; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond;
*         DxCh2DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2;
*         Caelum; AC3X; Falcon.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
*
* @param[out] corecctedErrorPtr          - number of corrected
*                                        errors
* @param[out] uncorrectedErrorPtr        - number of uncorrected
*                                        errors
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
CMD_STATUS wrCpssDxChFcFecCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_FCFEC_COUNTERS_STC    fcfecCounters;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    result = cpssDxChFcFecCounterGet(devNum, portNum, &fcfecCounters);

    galtisOutput(outArgs, result, "%d%d",fcfecCounters.numReceivedBlocksCorrectedError, fcfecCounters.numReceivedBlocksUncorrectedError);
    return CMD_OK;
}

/**
* @internal wrCpssDxChRsFecStatusGet function
* @endinternal
*
* @brief   RSFEC status collector.
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2;
*         DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] rsfecStatusPtr          - RSFEC counters/status:
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - RS_FEC_544_514 not support
* @retval GT_BAD_PARAM             - wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is
*         NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
CMD_STATUS wrCpssDxChRsFecStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS               result;
    GT_U8                   devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_DXCH_RSFEC_STATUS_STC  portStatus;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum         = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    result = cpssDxChRsFecStatusGet(devNum, portNum, &portStatus);

    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",portStatus.fecAlignStatus,portStatus.ampsLock,portStatus.highSer
                                    , portStatus.symbolError[0]
                                    , portStatus.symbolError[1]
                                    , portStatus.symbolError[2]
                                    , portStatus.symbolError[3]
                                    , portStatus.symbolError[4]
                                    , portStatus.symbolError[5]
                                    , portStatus.symbolError[6]
                                    , portStatus.symbolError[7]
                                    , portStatus.symbolError[8]
                                    , portStatus.symbolError[9]
                                    , portStatus.symbolError[10]
                                    , portStatus.symbolError[11]
                                    , portStatus.symbolError[12]
                                    , portStatus.symbolError[13]
                                    , portStatus.symbolError[14]
                                    , portStatus.symbolError[15]);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortSerdesActiveLanesListGet function
* @endinternal
*
* @brief   Return number of active serdeses and array of active
*          serdeses numbers that port uses
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
*
* @param[out] activeLanePtr          - number of active lanes
*                                        and list.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
CMD_STATUS wrCpssDxChPortSerdesActiveLanesListGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS rc;
    GT_U8     devNum;
    GT_U32    portNum;

    CPSS_DXCH_ACTIVE_LANES_STC activeLane;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum          = (GT_U8)inArgs[0];
    portNum    = (GT_U32)inArgs[1];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    rc = cpssDxChPortSerdesActiveLanesListGet(devNum, portNum,&activeLane);

        inFields[0] = activeLane.numActiveLanes;
        inFields[1] = activeLane.activeLaneList[0];
        inFields[2] = activeLane.activeLaneList[1];
        inFields[3] = activeLane.activeLaneList[2];
        inFields[4] = activeLane.activeLaneList[3];
        inFields[5] = activeLane.activeLaneList[4];
        inFields[6] = activeLane.activeLaneList[5];
        inFields[7] = activeLane.activeLaneList[6];
        inFields[8] = activeLane.activeLaneList[7];

        /* pack and output table fields */
        fieldOutput("%d%d%d%d%d%d%d%d%d%d%d",devNum,portNum, inFields[0], inFields[1], inFields[2],
                        inFields[3], inFields[4], inFields[5], inFields[6], inFields[7] ,inFields[8]);

    galtisOutput(outArgs, rc, "%f");

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChPortSerdesLaneTuningEntrySet",
        &wrCpssDxChPortSerdesLaneTuningSet,
        4, 20},

    {"cpssDxChPortSerdesLaneTuningEntryGetFirst",
        &wrCpssDxChPortSerdesLaneTuningGet,
        4, 0},

    {"cpssDxChPortSerdesLaneTuningEntryGetNext",
        &wrCpssDxChGetNext,
        4, 0},

    {"cpssDxChPortSerdesLaneTuningEntry1Set",
        &wrCpssDxChPortSerdesLaneTuningSet,
        4, 9},

    {"cpssDxChPortSerdesLaneTuningEntry1GetFirst",
        &wrCpssDxChPortSerdesLaneTuningGet,
        4, 0},

    {"cpssDxChPortSerdesLaneTuningEntry1GetNext",
        &wrCpssDxChGetNext,
        4, 0},

    {"cpssDxChPortFecModeSet",
        &wrCpssDxChPortFecModeSet,
        3, 0},

    {"cpssDxChPortFecModeGet",
        &wrCpssDxChPortFecModeGet,
        2, 0},

    {"cpssDxChPortSerdesManualTxConfigSet",
         &wrCpssDxChPortSerdesManualTxConfigSet,
         8, 0},

    {"cpssDxChPortSerdesManualTxConfigSet1",
         &wrCpssDxChPortSerdesManualTxConfigSet,
         6, 0},

    {"cpssDxChPortSerdesManualRxConfigSet",
         &wrCpssDxChPortSerdesManualRxConfigSet,
         8, 0},

    {"cpssDxChPortSerdesManualRxConfigSet1",
         &wrCpssDxChPortSerdesManualRxConfigSet,
         9, 0},

    {"cpssDxChPortSerdesManualRxConfigGet1",
         &wrCpssDxChPortSerdesManualRxConfigGet,
         3, 0},

    {"cpssDxChPortSerdesManualTxConfigGet1",
         &wrCpssDxChPortSerdesManualTxConfigGet,
         3, 0},

    {"cpssDxChPortPcsLoopbackModeSet",
        &wrCpssDxChPortPcsLoopbackModeSet,
        3, 0},

    {"cpssDxChPortPcsLoopbackModeGet",
        &wrCpssDxChPortPcsLoopbackModeGet,
        2, 0},

    {"cpssDxChPortExtendedModeEnableSet",
        &wrCpssDxChPortExtendedModeEnableSet,
        3, 0},

    {"cpssDxChPortExtendedModeEnableGet",
        &wrCpssDxChPortExtendedModeEnableGet,
        2, 0},

    {"cpssDxChPortRefClockSourceOverrideEnableSet",
        &wrCpssDxChPortRefClockSourceOverrideEnableSet,
        4, 0},

    {"cpssDxChPortRefClockSourceOverrideEnableGet",
        &wrCpssDxChPortRefClockSourceOverrideEnableGet,
        2, 0},

    {"cpssDxChPortResourceTmBandwidthSet",
        &wrCpssDxChPortResourceTmBandwidthSet,
        2, 0},

    {"cpssDxChPortResourceTmBandwidthGet",
        &wrCpssDxChPortResourceTmBandwidthGet,
        1, 0},

    {"cpssDxChPortSerdesLanePolaritySet",
        &wrCpssDxChPortSerdesLanePolaritySet,
        5, 0},

    {"cpssDxChPortSerdesLanePolarityGet",
        &wrCpssDxChPortSerdesLanePolarityGet,
        3, 0},
    {"cpssDxChPortSerdesErrorInject",
        &wrCpssDxChPortSerdesErrorInject,
        5, 0},

    {"cpssDxChPortPcsResetSet",
        &wrCpssDxChPortPcsResetSet,
        4, 0},

    { "cpssDxChPortMacPcsStatusGet",
        &wrCpssDxChPortMacPcsStatusGet,
        2, 0 },

    { "cpssDxChRsFecCounterGet",
        &wrCpssDxChRsFecCounterGet,
        2, 0 },

    { "cpssDxChFcFecCounterGet",
        &wrCpssDxChFcFecCounterGet,
        2, 0 },
    { "cpssDxChRsFecStatusGet",
        &wrCpssDxChRsFecStatusGet,
        2, 0 },

    {"cpssDxChPortSerdesActiveLanesListGetFirst",
        &wrCpssDxChPortSerdesActiveLanesListGet,
        2, 0},

    {"cpssDxChPortSerdesActiveLanesListGetNext",
        &wrCpssDxChGetNext,
        2, 0},

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChPortCtrl function
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
GT_STATUS cmdLibInitCpssDxChPortCtrl
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

