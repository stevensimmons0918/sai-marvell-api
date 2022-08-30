/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file wrapCpssDxChHwInitLedCtrl.c
* @version   11
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>

static CMD_STATUS cmdLedStreamPortGroupConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN  GT_32 version
);

static CMD_STATUS cmdLedStreamPortGroupConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN  GT_32 version
);

static CMD_STATUS cmdLedStreamPortGroupClassManipulationSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN  GT_32 version
);

static CMD_STATUS cmdLedStreamPortGroupClassManipulationGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN  GT_U32 version
);

/**
* @internal wrCpssDxChLedStreamClassAndGroupConfig function
* @endinternal
*
* @brief   This routine configures the control, the class manipulation and the
*         group configuration registers for both led interfaces in Dx.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - ledClassNum value or ledGroupNum value is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. See the Led Serial Interface in the device's data-sheet for full
*       detail of the leds configuration, classes and groups.
*
*/
static CMD_STATUS wrCpssDxChLedStreamClassAndGroupConfig
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 ledControl;
    CPSS_DXCH_LED_CLASS_CONF_STC ledClassConf[1];
    CPSS_DXCH_LED_GROUP_CONF_STC ledGroupConf[1];

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum                 = (GT_U8)inArgs[0];
    ledControl             = (GT_U32)inArgs[1];
    ledClassConf[0].classNum  = (GT_U32)inArgs[2];
    ledClassConf[0].classConf = (GT_U32)inArgs[3];
    ledGroupConf[0].groupNum  = (GT_U32)inArgs[5];
    ledGroupConf[0].groupConf = (GT_U32)inArgs[6];

    /* call cpss api function */
    result = cpssDxChLedStreamClassAndGroupConfig(devNum, ledControl,
                                                  ledClassConf, 1, ledGroupConf, 1);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamHyperGStackTxQStatusEnableSet function
* @endinternal
*
* @brief   Enables the display to HyperGStack Ports Transmit Queue Status via LED
*         Stream.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChLedStreamHyperGStackTxQStatusEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    enable = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChLedStreamHyperGStackTxQStatusEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamHyperGStackTxQStatusEnableGet function
* @endinternal
*
* @brief   Gets HyperGStack Ports Transmit Queue Status via LED Stream.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChLedStreamHyperGStackTxQStatusEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_BOOL enable;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChLedStreamHyperGStackTxQStatusEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamPortGroupConfigSet function
* @endinternal
*
* @brief   This routine configures the LED stream.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANG           - ledStart or ledEnd out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamPortGroupConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return cmdLedStreamPortGroupConfigSet(inArgs,inFields,numFields,outArgs,0);
}

/**
* @internal wrCpssDxChLedStreamPortGroupConfigSet_1 function
* @endinternal
*
* @brief   This routine configures the LED stream.
*
* @note   APPLICABLE DEVICES:      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANG           - ledStart or ledEnd out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamPortGroupConfigSet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return cmdLedStreamPortGroupConfigSet(inArgs,inFields,numFields,outArgs,1);
}

/**
* @internal wrCpssDxChLedStreamPortGroupConfigGet function
* @endinternal
*
* @brief   This routine gets the LED stream configuration.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamPortGroupConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return cmdLedStreamPortGroupConfigGet(inArgs,inFields,numFields,outArgs, 0);
}

/**
* @internal wrCpssDxChLedStreamPortGroupConfigGet_1 function
* @endinternal
*
* @brief   This routine gets the LED stream configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamPortGroupConfigGet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return cmdLedStreamPortGroupConfigGet(inArgs,inFields,numFields,outArgs, 1);
}

/**
* @internal wrCpssDxChLedStreamCpuOrPort27ModeSet function
* @endinternal
*
* @brief   This routine configures the whether LED stream interface 0 contains CPU
*         or port 27 indications.
*
* @note   APPLICABLE DEVICES:      DxCh3 and DxChXcat
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamCpuOrPort27ModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_LED_CPU_OR_PORT27_ENT   indicatedPort;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    indicatedPort = (CPSS_DXCH_LED_CPU_OR_PORT27_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssDxChLedStreamCpuOrPort27ModeSet(devNum, indicatedPort);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamCpuOrPort27ModeGet function
* @endinternal
*
* @brief   This routine gets the whether LED stream interface 0 contains CPU
*         or port 27 indications.
*
* @note   APPLICABLE DEVICES:      DxCh3 and DxChXcat
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamCpuOrPort27ModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    CPSS_DXCH_LED_CPU_OR_PORT27_ENT   indicatedPort;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChLedStreamCpuOrPort27ModeGet(devNum, &indicatedPort);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", indicatedPort);

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamClassManipulationSet function
* @endinternal
*
* @brief   This routine configures a LED stream class manipulation.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamClassManipulationSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32                            ledInterfaceNum;
    CPSS_DXCH_LED_PORT_TYPE_ENT       portType;
    GT_U32                            classNum;
    CPSS_LED_CLASS_MANIPULATION_STC   classParams;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ledInterfaceNum = (GT_U32)inArgs[1];
    portType = (CPSS_DXCH_LED_PORT_TYPE_ENT)inArgs[2];
    classNum = (GT_U32)inArgs[3];
    classParams.invertEnable = (GT_BOOL)inArgs[4];
    classParams.blinkEnable = (GT_BOOL)inArgs[5];
    classParams.blinkSelect = (CPSS_LED_BLINK_SELECT_ENT)inArgs[6];
    classParams.forceEnable = (GT_BOOL)inArgs[7];
    classParams.forceData = (GT_U32)inArgs[8];

    /* call cpss api function */
    result = cpssDxChLedStreamClassManipulationSet(devNum, ledInterfaceNum,
                                            portType, classNum, &classParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamClassManipulationGet function
* @endinternal
*
* @brief   This routine gets a LED stream class manipulation configuration.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamClassManipulationGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32                            ledInterfaceNum;
    CPSS_DXCH_LED_PORT_TYPE_ENT       portType;
    GT_U32                            classNum;
    CPSS_LED_CLASS_MANIPULATION_STC   classParams;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ledInterfaceNum = (GT_U32)inArgs[1];
    portType = (CPSS_DXCH_LED_PORT_TYPE_ENT)inArgs[2];
    classNum = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChLedStreamClassManipulationGet(devNum, ledInterfaceNum,
                                            portType, classNum, &classParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d", classParams.invertEnable,
                                                classParams.blinkEnable,
                                                classParams.blinkSelect,
                                                classParams.forceEnable,
                                                classParams.forceData);
    return CMD_OK;
}


/**
* @internal wrCpssDxChLedStreamGroupConfigSet function
* @endinternal
*
* @brief   This routine configures a LED stream group.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - on out of range class number
*                                       (allowable class range o..15)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamGroupConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32                      ledInterfaceNum;
    CPSS_DXCH_LED_PORT_TYPE_ENT portType;
    GT_U32                      groupNum;
    CPSS_LED_GROUP_CONF_STC     groupParams;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ledInterfaceNum = (GT_U32)inArgs[1];
    portType = (CPSS_DXCH_LED_PORT_TYPE_ENT)inArgs[2];
    groupNum = (GT_U32)inArgs[3];
    groupParams.classA = (GT_U32)inArgs[4];
    groupParams.classB = (GT_U32)inArgs[5];
    groupParams.classC = (GT_U32)inArgs[6];
    groupParams.classD = (GT_U32)inArgs[7];

    /* call cpss api function */
    result = cpssDxChLedStreamGroupConfigSet(devNum, ledInterfaceNum,
                                            portType, groupNum, &groupParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamGroupConfigGet function
* @endinternal
*
* @brief   This routine gets a LED stream group configuration.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamGroupConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32                      ledInterfaceNum;
    CPSS_DXCH_LED_PORT_TYPE_ENT portType;
    GT_U32                      groupNum;
    CPSS_LED_GROUP_CONF_STC     groupParams;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ledInterfaceNum = (GT_U32)inArgs[1];
    portType = (CPSS_DXCH_LED_PORT_TYPE_ENT)inArgs[2];
    groupNum = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssDxChLedStreamGroupConfigSet(devNum, ledInterfaceNum,
                                             portType, groupNum, &groupParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d", groupParams.classA,
                                              groupParams.classB,
                                              groupParams.classC,
                                              groupParams.classD);

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamConfigSet function
* @endinternal
*
* @brief   This routine configures the LED stream.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANG           - ledStart or ledEnd out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32              ledInterfaceNum;
    CPSS_LED_CONF_STC   ledConf;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ledInterfaceNum = (GT_U32)inArgs[1];
    ledConf.ledOrganize = (CPSS_LED_ORDER_MODE_ENT)inArgs[2];;
    ledConf.disableOnLinkDown = (GT_BOOL)inArgs[3];
    ledConf.blink0DutyCycle = (CPSS_LED_BLINK_DUTY_CYCLE_ENT)inArgs[4];
    ledConf.blink0Duration = (CPSS_LED_BLINK_DURATION_ENT)inArgs[5];
    ledConf.blink1DutyCycle = (CPSS_LED_BLINK_DUTY_CYCLE_ENT)inArgs[6];
    ledConf.blink1Duration = (CPSS_LED_BLINK_DURATION_ENT)inArgs[7];
    ledConf.pulseStretch = (CPSS_LED_PULSE_STRETCH_ENT)inArgs[8];
    ledConf.ledStart = (GT_U32)inArgs[9];
    ledConf.ledEnd = (GT_U32)inArgs[10];
    ledConf.clkInvert = (GT_BOOL)inArgs[11];
    ledConf.class5select = (CPSS_LED_CLASS_5_SELECT_ENT)inArgs[12];
    ledConf.class13select = (CPSS_LED_CLASS_13_SELECT_ENT)inArgs[13];

    /* call cpss api function */
    result = cpssDxChLedStreamConfigSet(devNum, ledInterfaceNum, &ledConf);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssDxChLedStreamConfigGet function
* @endinternal
*
* @brief   This routine gets the LED stream configuration.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32              ledInterfaceNum;
    CPSS_LED_CONF_STC   ledConf;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ledInterfaceNum = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChLedStreamConfigGet(devNum, ledInterfaceNum, &ledConf);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d%d", ledConf.ledOrganize,
                                                       ledConf.disableOnLinkDown,
                                                       ledConf.blink0DutyCycle,
                                                       ledConf.blink0Duration,
                                                       ledConf.blink1DutyCycle,
                                                       ledConf.blink1Duration,
                                                       ledConf.pulseStretch,
                                                       ledConf.ledStart,
                                                       ledConf.ledEnd,
                                                       ledConf.clkInvert,
                                                       ledConf.class5select,
                                                       ledConf.class13select);
    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamPortGroupClassManipulationSet function
* @endinternal
*
* @brief   This routine configures a LED stream class manipulation.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamPortGroupClassManipulationSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return cmdLedStreamPortGroupClassManipulationSet(inArgs,inFields,numFields,outArgs,0);
}

/**
* @internal wrCpssDxChLedStreamPortGroupClassManipulationSet_1 function
* @endinternal
*
* @brief   This routine configures a LED stream class manipulation.
*
* @note   APPLICABLE DEVICES:      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamPortGroupClassManipulationSet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return cmdLedStreamPortGroupClassManipulationSet(inArgs,inFields,numFields,outArgs,1);
}

/**
* @internal wrCpssDxChLedStreamPortGroupClassManipulationGet function
* @endinternal
*
* @brief   This routine gets a LED stream class manipulation configuration.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamPortGroupClassManipulationGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return cmdLedStreamPortGroupClassManipulationGet(inArgs,inFields,numFields,outArgs,0);
}

/**
* @internal wrCpssDxChLedStreamPortGroupClassManipulationGet_1 function
* @endinternal
*
* @brief   This routine gets a LED stream class manipulation configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamPortGroupClassManipulationGet_1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    return cmdLedStreamPortGroupClassManipulationGet(inArgs,inFields,numFields,outArgs,1);
}

/**
* @internal wrCpssDxChLedStreamPortGroupGroupConfigSet function
* @endinternal
*
* @brief   This routine configures a LED stream group.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - on out of range class number
*                                       (allowable class range o..15)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamPortGroupGroupConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_GROUPS_BMP  portGroupsBmp;
    GT_U32              ledInterfaceNum;
    CPSS_DXCH_LED_PORT_TYPE_ENT portType;
    GT_U32                      groupNum;
    CPSS_LED_GROUP_CONF_STC     ledGroupParams;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (GT_PORT_GROUPS_BMP)inArgs[1];
    ledInterfaceNum = (GT_U32)inArgs[2];
    portType = (CPSS_DXCH_LED_PORT_TYPE_ENT)inArgs[3];
    groupNum = (GT_U32)inArgs[4];
    ledGroupParams.classA = (GT_U32)inArgs[5];
    ledGroupParams.classB = (GT_U32)inArgs[6];
    ledGroupParams.classC = (GT_U32)inArgs[7];
    ledGroupParams.classD = (GT_U32)inArgs[8];

    /* call cpss api function */
    result = cpssDxChLedStreamPortGroupGroupConfigSet(devNum, portGroupsBmp, ledInterfaceNum,
                                                      portType, groupNum, &ledGroupParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamPortGroupGroupConfigGet function
* @endinternal
*
* @brief   This routine gets a LED stream group configuration.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamPortGroupGroupConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_GROUPS_BMP  portGroupsBmp;
    GT_U32              ledInterfaceNum;
    CPSS_DXCH_LED_PORT_TYPE_ENT portType;
    GT_U32                      groupNum;
    CPSS_LED_GROUP_CONF_STC     ledGroupParams;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (GT_PORT_GROUPS_BMP)inArgs[1];
    ledInterfaceNum = (GT_U32)inArgs[2];
    portType = (CPSS_DXCH_LED_PORT_TYPE_ENT)inArgs[3];
    groupNum = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChLedStreamPortGroupGroupConfigGet(devNum, portGroupsBmp, ledInterfaceNum,
                                                      portType, groupNum, &ledGroupParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d",
                 ledGroupParams.classA,
                 ledGroupParams.classB,
                 ledGroupParams.classC,
                 ledGroupParams.classD);

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamClassIndicationSet function
* @endinternal
*
* @brief   This routine configures classes 9-11 indication (debug) for flex ports.
*
* @note   APPLICABLE DEVICES:      DxChXcat only.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamClassIndicationSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 ledInterfaceNum;
    GT_U32 classNum;
    CPSS_DXCH_LED_INDICATION_ENT   indication;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ledInterfaceNum = (GT_U32)inArgs[1];
    classNum = (GT_U32)inArgs[2];
    indication = (CPSS_DXCH_LED_INDICATION_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssDxChLedStreamClassIndicationSet(devNum, ledInterfaceNum, classNum, indication);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamClassIndicationGet function
* @endinternal
*
* @brief   This routine gets classes 9-11 indication (debug) for flex ports.
*
* @note   APPLICABLE DEVICES:      DxChXcat only.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamClassIndicationGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 ledInterfaceNum;
    GT_U32 classNum;
    CPSS_DXCH_LED_INDICATION_ENT   indication;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ledInterfaceNum = (GT_U32)inArgs[1];
    classNum = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChLedStreamClassIndicationGet(devNum, ledInterfaceNum, classNum, &indication);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", indication);

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamDirectModeEnableSet function
* @endinternal
*
* @brief   This routine enables\disables LED Direct working mode.
*
* @note   APPLICABLE DEVICES:      DxChXcat only.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note In Direct mode LED pin indicates a different Stack port status.
*       The external pins are mapped as follows:
*       LEDDATA1 � indicates stack port 24
*       LEDSTB1 � indicates stack port 25
*       LEDDATA0 � indicates stack port 26
*       LEDSTB0 � indicates stack port 27
*       The indication for each port is driven from the LED group0
*       configurations.
*
*/
static CMD_STATUS wrCpssDxChLedStreamDirectModeEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 ledInterfaceNum;
    GT_BOOL enable;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ledInterfaceNum = (GT_U32)inArgs[1];
    enable = inArgs[2];

    /* call cpss api function */
    result = cpssDxChLedStreamDirectModeEnableSet(devNum, ledInterfaceNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamDirectModeEnableGet function
* @endinternal
*
* @brief   This routine gets LED pin indication direct mode enabling status.
*
* @note   APPLICABLE DEVICES:      DxChXcat only.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note In Direct mode LED pin indicates a different Stack port status.
*       The external pins are mapped as follows:
*       LEDDATA1 � indicates stack port 24
*       LEDSTB1 � indicates stack port 25
*       LEDDATA0 � indicates stack port 26
*       LEDSTB0 � indicates stack port 27
*       The indication for each port is driven from the LED group0
*       configurations.
*
*/
static CMD_STATUS wrCpssDxChLedStreamDirectModeEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_U32 ledInterfaceNum;
    GT_BOOL enable;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    ledInterfaceNum = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChLedStreamDirectModeEnableGet(devNum, ledInterfaceNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamPortPositionSet function
* @endinternal
*
* @brief   Sets port position in LED stream.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamPortPositionSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  position;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    position = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;

}


/**
* @internal wrCpssDxChLedStreamPortPositionGet function
* @endinternal
*
* @brief   Gets port position in LED stream.
*
* @note   APPLICABLE DEVICES:      xCat3; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS wrCpssDxChLedStreamPortPositionGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  position;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* call cpss api function */
    result = cpssDxChLedStreamPortPositionGet(devNum, portNum, &position);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", position);

    return CMD_OK;

}

/*******************************************************************************
* wrcpssDxChLedStreamPortClassIndicationSet
*
* DESCRIPTION:
*       This routine set the polarity of the selected indications
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2, Falcon.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* INPUTS:
*       devNum          - physical device number
*       portNum         - physical port number.
*       classNum        - class number.
*                         (APPLICABLE RANGES: 0..5)
*       indication      - value for the port class indication
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PARAM             - one of the parameters value is wrong
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_FAIL                  - otherwise
*       GT_NOT_INITIALIZED       - port's LED position is not set yet by cpssDxChLedStreamPortPositionSet
*
* COMMENTS:
*
*******************************************************************************/
static CMD_STATUS wrCpssDxChLedStreamPortClassIndicationSet
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
    GT_U32                          classNum;
    CPSS_DXCH_LED_INDICATION_ENT    indication;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    classNum = (GT_U32)inArgs[2];
    indication = (CPSS_DXCH_LED_INDICATION_ENT)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssDxChLedStreamPortClassIndicationSet(devNum, portNum, classNum, indication);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/*******************************************************************************
* wrcpssDxChLedStreamPortClassIndicationGet
*
* DESCRIPTION:
*       This routine gets the port class indication
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2, Falcon.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* INPUTS:
*       devNum          - physical device number
*       portNum         - physical port number.
*       classNum        - class number.
*                         (APPLICABLE RANGES: 0..5)
*
*
* OUTPUTS:
*       indication      - (pointer to) port class indication.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_BAD_PARAM             - one of the parameters value is wrong
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_FAIL                  - otherwise
*       GT_NOT_INITIALIZED       - port's LED position is not set yet by cpssDxChLedStreamPortPositionSet
*
* COMMENTS:
*
*******************************************************************************/
static CMD_STATUS wrCpssDxChLedStreamPortClassIndicationGet
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
    GT_U32                          classNum;
    CPSS_DXCH_LED_INDICATION_ENT    indication;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    classNum = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssDxChLedStreamPortClassIndicationGet(devNum, portNum, classNum, &indication);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", indication);

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamPortClassPolarityInvertEnableSet function
* @endinternal
*
* @brief   This routine set the polarity of the selected indications
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_INITIALIZED       - port's LED position is not set yet by cpssDxChLedStreamPortPositionSet
*/
static CMD_STATUS wrCpssDxChLedStreamPortClassPolarityInvertEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32      classNum;
    GT_BOOL     invertEnable;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    classNum = (GT_U32)inArgs[2];
    invertEnable = (GT_BOOL)inArgs[3];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssDxChLedStreamPortClassPolarityInvertEnableSet(devNum, portNum, classNum, invertEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedStreamPortClassPolarityInvertEnableGet function
* @endinternal
*
* @brief   This routine set the polarity of the selected indications
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_INITIALIZED       - port's LED position is not set yet by cpssDxChLedStreamPortPositionSet
*/
static CMD_STATUS wrCpssDxChLedStreamPortClassPolarityInvertEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32      classNum;
    GT_BOOL     invertEnable;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    classNum = (GT_U32)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssDxChLedStreamPortClassPolarityInvertEnableGet(devNum, portNum, classNum, &invertEnable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", invertEnable);

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedPhyControlGlobalSet function
* @endinternal
*
* @brief   Set global (per-PHY) LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
static CMD_STATUS wrCpssDxChLedPhyControlGlobalSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC  ledGlobalConfig;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    ledGlobalConfig.directMode = (GT_BOOL)inArgs[2];
    ledGlobalConfig.skipColumns = (GT_U32)inArgs[3];
    ledGlobalConfig.specialLed1Bitmap = (GT_U32)inArgs[4];
    ledGlobalConfig.specialLed2Bitmap = (GT_U32)inArgs[5];
    ledGlobalConfig.specialLed3Bitmap = (GT_U32)inArgs[6];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssDxChLedPhyControlGlobalSet(devNum, portNum, &ledGlobalConfig);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedPhyControlGlobalGet function
* @endinternal
*
* @brief   Get global (per-PHY) LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
static CMD_STATUS wrCpssDxChLedPhyControlGlobalGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC  ledGlobalConfig;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssDxChLedPhyControlGlobalGet(devNum, portNum, &ledGlobalConfig);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d",
                    ledGlobalConfig.directMode,
                    ledGlobalConfig.skipColumns,
                    ledGlobalConfig.specialLed1Bitmap,
                    ledGlobalConfig.specialLed2Bitmap,
                    ledGlobalConfig.specialLed3Bitmap);

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedPhyControlPerPortSet function
* @endinternal
*
* @brief   Set per-port LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
static CMD_STATUS wrCpssDxChLedPhyControlPerPortSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_DXCH_LED_PHY_PORT_CONF_STC  ledPerPortConfig;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];
    ledPerPortConfig.led0Select = (CPSS_DXCH_LED_PHY_SELECT_ENT) inArgs[2];
    ledPerPortConfig.led1Select = (CPSS_DXCH_LED_PHY_SELECT_ENT) inArgs[3];
    ledPerPortConfig.pulseStretch = (GT_U32) inArgs[4];
    ledPerPortConfig.blinkRate = (GT_U32) inArgs[5];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssDxChLedPhyControlPerPortSet(devNum, portNum, &ledPerPortConfig);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChLedPhyControlPerPortGet function
* @endinternal
*
* @brief   Get per-port LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; xCat3; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
static CMD_STATUS wrCpssDxChLedPhyControlPerPortGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   result;
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    CPSS_DXCH_LED_PHY_PORT_CONF_STC  ledPerPortConfig;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portNum = (GT_PHYSICAL_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);
    /* call cpss api function */
    result = cpssDxChLedPhyControlPerPortGet(devNum, portNum, &ledPerPortConfig);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d",
                    ledPerPortConfig.led0Select,
                    ledPerPortConfig.led1Select,
                    ledPerPortConfig.pulseStretch,
                    ledPerPortConfig.blinkRate);

    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChLedStreamClassAndGroupConfig",
         &wrCpssDxChLedStreamClassAndGroupConfig,
         8, 0},
    {"cpssDxChLedStreamHyperGStackTxQStatusEnableSet",
         &wrCpssDxChLedStreamHyperGStackTxQStatusEnableSet,
         2, 0},
    {"cpssDxChLedStreamHyperGStackTxQStatusEnableGet",
         &wrCpssDxChLedStreamHyperGStackTxQStatusEnableGet,
         1, 0},

    {"cpssDxChLedStreamPortGroupConfigSet",
         &wrCpssDxChLedStreamPortGroupConfigSet,
         15, 0},

    {"cpssDxChLedStreamPortGroupConfigGet",
         &wrCpssDxChLedStreamPortGroupConfigGet,
         3, 0},

    {"cpssDxChLedStreamPortGroupConfigSet_1",
         &wrCpssDxChLedStreamPortGroupConfigSet_1,
         17, 0},

    {"cpssDxChLedStreamPortGroupConfigGet_1",
         &wrCpssDxChLedStreamPortGroupConfigGet_1,
         3, 0},


    {"cpssDxChLedStreamCpuOrPort27ModeSet",
         &wrCpssDxChLedStreamCpuOrPort27ModeSet,
         2, 0},

    {"cpssDxChLedStreamCpuOrPort27ModeGet",
         &wrCpssDxChLedStreamCpuOrPort27ModeGet,
         1, 0},

    {"cpssDxChLedStreamClassManipulationSet",
         &wrCpssDxChLedStreamClassManipulationSet,
         9, 0},

    {"cpssDxChLedStreamClassManipulationGet",
         &wrCpssDxChLedStreamClassManipulationGet,
         4, 0},

    {"cpssDxChLedStreamGroupConfigSet",
         &wrCpssDxChLedStreamGroupConfigSet,
         8, 0},

    {"cpssDxChLedStreamGroupConfigGet",
         &wrCpssDxChLedStreamGroupConfigGet,
         4, 0},

    {"cpssDxChLedStreamConfigSet",
         &wrCpssDxChLedStreamConfigSet,
         14, 0},

    {"cpssDxChLedStreamConfigGet",
         &wrCpssDxChLedStreamConfigGet,
         2, 0},

    {"cpssDxChLedStreamPortGroupClassManipulationSet",
         &wrCpssDxChLedStreamPortGroupClassManipulationSet,
         10, 0},

    {"cpssDxChLedStreamPortGroupClassManipulationGet",
         &wrCpssDxChLedStreamPortGroupClassManipulationGet,
         5, 0},

    {"cpssDxChLedStreamPortGroupClassManipulationSet_1",
         &wrCpssDxChLedStreamPortGroupClassManipulationSet_1,
         12, 0},

    {"cpssDxChLedStreamPortGroupClassManipulationGet_1",
         &wrCpssDxChLedStreamPortGroupClassManipulationGet_1,
         5, 0},

    {"cpssDxChLedStreamPortGroupGroupConfigSet",
         &wrCpssDxChLedStreamPortGroupGroupConfigSet,
         9, 0},

    {"cpssDxChLedStreamPortGroupGroupConfigGet",
         &wrCpssDxChLedStreamPortGroupGroupConfigGet,
         5, 0},

    {"cpssDxChLedStreamClassIndicationSet",
         &wrCpssDxChLedStreamClassIndicationSet,
         4, 0},

    {"cpssDxChLedStreamClassIndicationGet",
         &wrCpssDxChLedStreamClassIndicationGet,
         3, 0},

    {"cpssDxChLedStreamDirectModeEnableSet",
         &wrCpssDxChLedStreamDirectModeEnableSet,
         3, 0},

    {"cpssDxChLedStreamDirectModeEnableGet",
         &wrCpssDxChLedStreamDirectModeEnableGet,
         2, 0},

    {"cpssDxChLedStreamPortPositionSet",
        &wrCpssDxChLedStreamPortPositionSet,
        3, 0},

    {"cpssDxChLedStreamPortPositionGet",
        &wrCpssDxChLedStreamPortPositionGet,
        2, 0},

    {"cpssDxChLedStreamPortClassPolarityInvertEnableSet",
        &wrCpssDxChLedStreamPortClassPolarityInvertEnableSet,
        4, 0},

    {"cpssDxChLedStreamPortClassPolarityInvertEnableGet",
        &wrCpssDxChLedStreamPortClassPolarityInvertEnableGet,
        3, 0},

    {"cpssDxChLedStreamPortClassIndicationSet",
        &wrCpssDxChLedStreamPortClassIndicationSet,
        4, 0},

    {"cpssDxChLedStreamPortClassIndicationGet",
        &wrCpssDxChLedStreamPortClassIndicationGet,
        3, 0},

    {"cpssDxChLedPhyControlGlobalSet",
        &wrCpssDxChLedPhyControlGlobalSet,
        7, 0},
    {"cpssDxChLedPhyControlGlobalGet",
        &wrCpssDxChLedPhyControlGlobalGet,
        2, 0},
    {"cpssDxChLedPhyControlPerPortSet",
        &wrCpssDxChLedPhyControlPerPortSet,
        6, 0},
    {"cpssDxChLedPhyControlPerPortGet",
        &wrCpssDxChLedPhyControlPerPortGet,
        2, 0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChHwInitLedCtrl function
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
GT_STATUS cmdLibInitCpssDxChHwInitLedCtrl
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

/**
* @internal cmdLedStreamPortGroupConfigSet function
* @endinternal
*
* @brief   This routine configures the LED stream.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANG           - ledStart or ledEnd out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS cmdLedStreamPortGroupConfigSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN  GT_32 version
)
{
    GT_STATUS              result;
    GT_U8               devNum;
    GT_PORT_GROUPS_BMP  portGroupsBmp;
    GT_U32              ledInterfaceNum;
    CPSS_LED_CONF_STC   ledStreamConf;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (GT_PORT_GROUPS_BMP)inArgs[1];
    ledInterfaceNum = (GT_U32)inArgs[2];
    ledStreamConf.ledOrganize = (CPSS_LED_ORDER_MODE_ENT)inArgs[3];
    ledStreamConf.disableOnLinkDown = (GT_BOOL)inArgs[4];
    ledStreamConf.blink0DutyCycle = (CPSS_LED_BLINK_DUTY_CYCLE_ENT)inArgs[5];
    ledStreamConf.blink0Duration  = (CPSS_LED_BLINK_DURATION_ENT)inArgs[6];
    ledStreamConf.blink1DutyCycle = (CPSS_LED_BLINK_DUTY_CYCLE_ENT)inArgs[7];
    ledStreamConf.blink1Duration  = (CPSS_LED_BLINK_DURATION_ENT)inArgs[8];
    ledStreamConf.pulseStretch  = (CPSS_LED_PULSE_STRETCH_ENT)inArgs[9];
    ledStreamConf.ledStart  = (GT_U32)inArgs[10];
    ledStreamConf.ledEnd    = (GT_U32)inArgs[11];

    ledStreamConf.clkInvert = (GT_BOOL)inArgs[12];
    ledStreamConf.class5select  = (CPSS_LED_CLASS_5_SELECT_ENT)inArgs[13];
    ledStreamConf.class13select = (CPSS_LED_CLASS_13_SELECT_ENT)inArgs[14];

    if (version == 1)
    {
        /* Bobcat2, Caelum, Bobcat3 */
        ledStreamConf.invertEnable = (GT_BOOL)inArgs[15];
        ledStreamConf.ledClockFrequency = (CPSS_LED_CLOCK_OUT_FREQUENCY_ENT)inArgs[16];
    }

    /* call cpss api function */
    result = cpssDxChLedStreamPortGroupConfigSet(devNum, portGroupsBmp, ledInterfaceNum, &ledStreamConf);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal cmdLedStreamPortGroupConfigGet function
* @endinternal
*
* @brief   This routine gets the LED stream configuration.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS cmdLedStreamPortGroupConfigGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN  GT_32 version
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_GROUPS_BMP  portGroupsBmp;
    GT_U32              ledInterfaceNum;
    CPSS_LED_CONF_STC   ledStreamConf;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (GT_PORT_GROUPS_BMP)inArgs[1];
    ledInterfaceNum = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssDxChLedStreamPortGroupConfigGet(devNum, portGroupsBmp, ledInterfaceNum, &ledStreamConf);

    if (version == 0)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d%d",
                     ledStreamConf.ledOrganize,
                     ledStreamConf.disableOnLinkDown,
                     ledStreamConf.blink0DutyCycle,
                     ledStreamConf.blink0Duration,
                     ledStreamConf.blink1DutyCycle,
                     ledStreamConf.blink1Duration,
                     ledStreamConf.pulseStretch,
                     ledStreamConf.ledStart,
                     ledStreamConf.ledEnd,
                     ledStreamConf.clkInvert,
                     ledStreamConf.class5select,
                     ledStreamConf.class13select);
    }
    else
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                     ledStreamConf.ledOrganize,
                     ledStreamConf.disableOnLinkDown,
                     ledStreamConf.blink0DutyCycle,
                     ledStreamConf.blink0Duration,
                     ledStreamConf.blink1DutyCycle,
                     ledStreamConf.blink1Duration,
                     ledStreamConf.pulseStretch,
                     ledStreamConf.ledStart,
                     ledStreamConf.ledEnd,
                     ledStreamConf.clkInvert,
                     ledStreamConf.class5select,
                     ledStreamConf.class13select,
                     ledStreamConf.invertEnable,
                     ledStreamConf.ledClockFrequency);
    }

    return CMD_OK;
}

/**
* @internal cmdLedStreamPortGroupClassManipulationSet function
* @endinternal
*
* @brief   This routine configures a LED stream class manipulation.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS cmdLedStreamPortGroupClassManipulationSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN  GT_32 version
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_GROUPS_BMP  portGroupsBmp;
    GT_U32              ledInterfaceNum;
    CPSS_DXCH_LED_PORT_TYPE_ENT     portType;
    GT_U32                          classNum;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassParams;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (GT_PORT_GROUPS_BMP)inArgs[1];
    ledInterfaceNum = (GT_U32)inArgs[2];
    portType = (CPSS_DXCH_LED_PORT_TYPE_ENT)inArgs[3];
    classNum = (GT_U32)inArgs[4];
    ledClassParams.invertEnable = (GT_BOOL)inArgs[5];
    ledClassParams.blinkEnable  = (GT_BOOL)inArgs[6];
    ledClassParams.blinkSelect  = inArgs[7];
    ledClassParams.forceEnable  = (GT_BOOL)inArgs[8];
    ledClassParams.forceData    = (GT_U32)inArgs[9];
    if (version == 1)
    {
        ledClassParams.pulseStretchEnable = (GT_BOOL)inArgs[10];
        ledClassParams.disableOnLinkDown = (GT_BOOL)inArgs[11];
    }
    /* call cpss api function */
    result = cpssDxChLedStreamPortGroupClassManipulationSet(devNum, portGroupsBmp, ledInterfaceNum,
                                                            portType, classNum, &ledClassParams);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal cmdLedStreamPortGroupClassManipulationGet function
* @endinternal
*
* @brief   This routine gets a LED stream class manipulation configuration.
*
* @note   APPLICABLE DEVICES:      All DxCh Devices
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static CMD_STATUS cmdLedStreamPortGroupClassManipulationGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER],
    IN  GT_U32 version
)
{
    GT_STATUS result;
    GT_U8 devNum;
    GT_PORT_GROUPS_BMP  portGroupsBmp;
    GT_U32              ledInterfaceNum;
    CPSS_DXCH_LED_PORT_TYPE_ENT     portType;
    GT_U32                          classNum;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassParams;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portGroupsBmp = (GT_PORT_GROUPS_BMP)inArgs[1];
    ledInterfaceNum = (GT_U32)inArgs[2];
    portType = (CPSS_DXCH_LED_PORT_TYPE_ENT)inArgs[3];
    classNum = (GT_U32)inArgs[4];

    /* call cpss api function */
    result = cpssDxChLedStreamPortGroupClassManipulationGet(devNum, portGroupsBmp, ledInterfaceNum,
                                                            portType, classNum, &ledClassParams);

    if (version == 0)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, result, "%d%d%d%d%d",
                     ledClassParams.invertEnable,
                     ledClassParams.blinkEnable,
                     ledClassParams.blinkSelect,
                     ledClassParams.forceEnable,
                     ledClassParams.forceData);
    }
    else
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, result, "%d%d%d%d%d%d%d",
                     ledClassParams.invertEnable,
                     ledClassParams.blinkEnable,
                     ledClassParams.blinkSelect,
                     ledClassParams.forceEnable,
                     ledClassParams.forceData,
                     ledClassParams.pulseStretchEnable,
                     ledClassParams.disableOnLinkDown);
    }

    return CMD_OK;
}


