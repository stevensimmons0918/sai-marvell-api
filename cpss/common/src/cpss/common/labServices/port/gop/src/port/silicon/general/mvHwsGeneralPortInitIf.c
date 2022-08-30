/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
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
* @file mvHwsGeneralPortInitIf.c
*
* @brief
*
* @version   55
********************************************************************************
*/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralPortIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#ifdef RAVEN_DEV_SUPPORT
#include <cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#endif

#ifndef ASIC_SIMULATION
extern GT_STATUS mvHwsAvagoSerdesSpicoInterrupt
(
    IN GT_U8   devNum,
    IN GT_U32    portGroup,
    IN GT_U32    serdesNum,
    IN GT_U32    interruptCode,
    IN GT_U32    interruptData,
    OUT GT_32    *result
);
#endif /* ASIC_SIMULATION */

#define POSITIVE_NEGATIVE_VALUE(_val) \
    (((_val & 0x10))?                                  \
     ( (GT_8)255 - (GT_8)(((_val & 0xF) *2) - 1) ) :    \
     (_val)*2)

#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
/**
* @internal mvHwsGeneralPortInit function
* @endinternal
*
* @brief   Init physical port. Configures the port mode and all it's elements
*         accordingly.
*         Does not verify that the selected mode/port number is valid at the
*         core level.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lbPort                   - if true, init port without serdes activity
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeneralPortInit
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD      portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_BOOL serdesInit = GT_TRUE;
#ifndef MV_HWS_REDUCED_BUILD
    GT_U32 tmpData = 0;
    GT_U32 data = 0;
#endif /* MV_HWS_REDUCED_BUILD */
    GT_BOOL                   lbPort;
    MV_HWS_REF_CLOCK_SUP_VAL  refClock;
    MV_HWS_REF_CLOCK_SOURCE   refClockSource;
    if(NULL == portInitInParamPtr)
    {
       return GT_BAD_PTR;
    }
    lbPort = portInitInParamPtr->lbPort;
    refClock = portInitInParamPtr->refClock;
    refClockSource = portInitInParamPtr->refClockSource;
    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
       return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsPortLoopbackValidate((GT_U32)curPortParams.portPcsType, lbPort, &serdesInit));

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    if (serdesInit == GT_TRUE)
    {
#ifndef MV_HWS_REDUCED_BUILD
        /* Turning BobK OOB port 90 with mac 62 to network port */
        if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (curPortParams.portMacNumber==62))
        {
            /* Setting serdes RX If selector to 1 */
            CHECK_STATUS(mvHwsSerdesTxIfSelect(devNum, 0, MV_HWS_SERDES_NUM(curLanesList[0]), HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[0])), 1 << 3));

            /* Change Serdes 20 MUX to mac 62 by configurting bit[12] of a DFX server register to 0 */
            CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, 0xf829c, &tmpData));
            data  = tmpData & 0xFFFFEFFF;
            CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, 0xf829c, data));
        }
#endif /* MV_HWS_REDUCED_BUILD */
        if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin)
            || (HWS_DEV_SILICON_TYPE(devNum) == Pipe) || (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) ||
            (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2))
        {
            hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasLanesNum = curPortParams.numOfActLanes;
            hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasMacNum  = curPortParams.portMacNumber;
        }

        /* power up the serdes */
        CHECK_STATUS(mvHwsPortSerdesPowerUp(devNum, portGroup, phyPortNum, portMode, refClock, refClockSource, &(curLanesList[0])));

        /* set P Sync Bypass */
        CHECK_STATUS(mvHwsPortPsyncBypassCfg(devNum, portGroup, phyPortNum, portMode));
        /* Configures the DP/Core Clock Selector on port */
        CHECK_STATUS(mvHwsClockSelectorConfig(devNum, portGroup, phyPortNum, portMode));
    }

    /* Configure MAC/PCS */
    CHECK_STATUS(mvHwsPortModeCfg(devNum, portGroup, phyPortNum, portMode, NULL));

    /* Implement WA for 1G/SGMII2_5 portModes: when on the same GOP there is RXAUI and 1G/SGMII2_5 port,
       corrupted packets receive on RXAUI port */
    if ((curPortParams.portMacType == GEMAC_SG) || (curPortParams.portMacType == GEMAC_X))
    {
        /* set speed MAC mode in 10G */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, curPortParams.portMacNumber, MSM_PORT_MAC_CONTROL_REGISTER3, (1 << 13), (7 << 13)));

        /* set DMA MAC mode in 1G */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, curPortParams.portMacNumber, MSM_PORT_MAC_CONTROL_REGISTER4, (1 << 12), (1 << 12)));
    }

    /* in lbPort mode, only for MMPCS configure PCS with External PLL */
    if ((lbPort == GT_TRUE) && (curPortParams.portPcsType == MMPCS))
    {
        CHECK_STATUS(mvHwsPcsExtPllCfg(devNum, portGroup, curPortParams.portPcsNumber, curPortParams.portPcsType));
    }

    /* Un-Reset the port */
    CHECK_STATUS(mvHwsPortStartCfg(devNum, portGroup, phyPortNum, portMode));

    if (serdesInit == GT_TRUE)
    {
        /* run SERDES digital reset / unreset */
        switch (portMode)
        {
        case SGMII2_5:
        case _2500Base_X:
            CHECK_STATUS(mvHwsSerdesDigitalRfResetToggleSet(devNum, portGroup, phyPortNum, portMode, 10));
            break;

        default:
            break;
        }
    }

    return GT_OK;
}

#endif /* #if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT) */

/*******************************************************************************
* mvHwsGeneralApPortInit
*
* DESCRIPTION:
*       Init port for 802.3AP protocol.
*
* INPUTS:
*       devNum      - system device number
*       portGroup   - port group (core) number
*       phyPortNum  - physical port number
*       portMode    - port standard metric
*       lbPort      - if true, init port without serdes activity
*       refClock    - Reference clock frequency
*       refClockSrc - Reference clock source line
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
#ifndef RAVEN_DEV_SUPPORT
extern MV_HWS_TX_TUNE_PARAMS hwsAvagoSerdesTxTunePresetParams[2];
extern MV_HWS_TX_TUNE_PARAMS hwsAvagoSerdesTxTuneInitParams[2];
#else
extern MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA hwsAvagoSerdesTxTune16nmPresetParams[3][AN_PRESETS_NUM];
extern MV_HWS_AVAGO_TXRX_OVERRIDE_CONFIG_DATA mvHwsAvagoSerdesManualConfigDb[3][MV_HWS_IPC_MAX_PORT_NUM];

extern MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA hwsAvagoSerdesPresetParamsManualConfigDb[3][MV_HWS_IPC_MAX_PORT_NUM];

GT_BOOL                         cfgHalEnable = GT_FALSE;
MV_HWS_AVAGO_SERDES_HAL_TYPES   cfgHalType;
GT_U32                          cfgHalRegSelect;
GT_32                           cfgHalRegisterValue;

#endif
#if defined(MV_HWS_FREE_RTOS)
extern GT_U16 mvHwsAvagoSerdesManualTxOffsetDb[3][MV_HWS_IPC_MAX_PORT_NUM];
#endif

#ifdef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsRavenApPortInitPhase function
* @endinternal
*
* @brief  Only for RAVEN (cm3) Init physical port for 802.3AP
*         protocol is done in 2 stages. This function is doing
*         the second phase.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portInitInParamPtr       - port parameters
* @param[in] phase                    - port power up phase
*       line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsRavenApPortInitPhase
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr,
    GT_U8   phase,
    MV_HWS_SERDES_CONFIG_STC        *serdesConfigPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_BOOL updateTxParam = GT_FALSE;
    GT_U32   sdSpeed=0;

    GT_U32   Atten=0;
    GT_U32   PostCursor=0;
    GT_U32   PreCursor=0;
    GT_U32   Pre2Cursor = 0;
    GT_32    temperature;
    GT_U32  i;
    GT_U8   presetIndex;
    GT_U32  presetOffset;
    /*MV_HWS_SERDES_CONFIG_STC        serdesConfig;*/

    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
       return GT_BAD_PTR;
    }

    /* apply SerDes optical values */
    MV_HWS_CONVERT_TO_OPTICAL_BAUD_RATE_MAC(serdesConfigPtr->opticalMode, serdesConfigPtr->baudRate);

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    CHECK_STATUS( mvHwsAvagoSerdesArrayPowerCtrlImplPhase(devNum, portGroup, curPortParams.numOfActLanes, &(curLanesList[0]), serdesConfigPtr, phase));

    /* in raven we split the serdes power up to several stages*/
#ifndef ASIC_SIMULATION
    if (phase == MV_HWS_AVAGO_SERDES_POWER_LAST_PHASE)
    {
        /* supported speeds are currently 10G and 25G */
        switch (curPortParams.serdesSpeed)
        {
            case _10_3125G:
            case _10_9375G:
            case _12_1875G:
                updateTxParam = GT_TRUE;
                sdSpeed = 0;
                break;
            case _20_625G:
            case _25_78125G:
            case _27_34375G:
                updateTxParam = GT_TRUE;
                sdSpeed = 1;
                break;
            case _26_5625G_PAM4:
            case _27_1875_PAM4:
            case _28_125G_PAM4:
                updateTxParam = GT_TRUE;
                sdSpeed = 2 ;
                break;
            default:
                updateTxParam = GT_FALSE;
        }

        /* Get Tj temperature for later envelop settings */
        CHECK_STATUS(mvHwsPortTemperatureGet(devNum, portGroup,0,&temperature));

        for (i = 0; i < curPortParams.numOfActLanes; i++)
        {

            for(presetIndex = 0 ; presetIndex < AN_PRESETS_NUM ; presetIndex++)
            {
                updateTxParam=updateTxParam;
#if 0
                if ((updateTxParam) &&
                    (mvHwsAvagoSerdesManualConfigDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])].valid & MV_HWS_SERDES_TXRX_TUNE_PARAMS_DB_TX_VALID))
                {
                    Atten      = (0xA << 12) | mvHwsAvagoSerdesManualConfigDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])].txData.atten;
                    PostCursor = (0xB << 12) | mvHwsAvagoSerdesManualConfigDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])].txData.post;
                    PreCursor  = (0x9 << 12) | mvHwsAvagoSerdesManualConfigDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])].txData.pre;
                    Pre2Cursor = (0xC << 12) | mvHwsAvagoSerdesManualConfigDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])].txData.pre2;
                }
                else
#endif
                {
                    Atten      = (0xA << 12) | hwsAvagoSerdesTxTune16nmPresetParams[sdSpeed][presetIndex].atten;
                    PostCursor = (0xB << 12) | hwsAvagoSerdesTxTune16nmPresetParams[sdSpeed][presetIndex].post;
                    PreCursor  = (0x9 << 12) | hwsAvagoSerdesTxTune16nmPresetParams[sdSpeed][presetIndex].pre;
                    Pre2Cursor = (0xC << 12) | hwsAvagoSerdesTxTune16nmPresetParams[sdSpeed][presetIndex].pre2;
                }
                if (updateTxParam)
                {
#ifdef RAVEN_DEV_SUPPORT
                    if ( presetIndex == 0 ) {
                        if ( hwsAvagoSerdesPresetParamsManualConfigDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])].atten != TX_NA ) {
                            Atten      = (0xA << 12) | hwsAvagoSerdesPresetParamsManualConfigDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])].atten;

                        }
                        if ( hwsAvagoSerdesPresetParamsManualConfigDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])].post != TX_NA) {
                            PostCursor = (0xB << 12) | hwsAvagoSerdesPresetParamsManualConfigDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])].post;

                        }
                        if ( hwsAvagoSerdesPresetParamsManualConfigDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])].pre != TX_NA) {
                            PreCursor  = (0x9 << 12) | hwsAvagoSerdesPresetParamsManualConfigDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])].pre;

                        }
                    }
#endif
#if 0

                    if (HWS_DEV_SERDES_TYPE(devNum) == AVAGO_16NM)
                    {
                        if ((Atten & 0xFF) != TX_NA)
                        {
                            Atten = 0xFF &(hwsAvagoSerdesTxTune16nmPresetParams[sdSpeed][presetIndex].atten +
                                           POSITIVE_NEGATIVE_VALUE(mvHwsAvagoSerdesManualTxOffsetDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])] & 0x1F));

                            Atten |= (0xA << 12);
                        }
                        if((PostCursor & 0xFF) != TX_NA)
                        {
                            PostCursor = 0xFF &(hwsAvagoSerdesTxTune16nmPresetParams[sdSpeed][presetIndex].post +
                                                POSITIVE_NEGATIVE_VALUE((mvHwsAvagoSerdesManualTxOffsetDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])] >> 5) & 0x1F));
                            PostCursor |= (0xB << 12);
                        }
                        if((PreCursor & 0xFF) != TX_NA)
                        {
                            PreCursor = 0xFF & (hwsAvagoSerdesTxTune16nmPresetParams[sdSpeed][presetIndex].pre +
                                                POSITIVE_NEGATIVE_VALUE((mvHwsAvagoSerdesManualTxOffsetDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])] >> 10) & 0x1F));

                            PreCursor |= (0x9 << 12);
                        }
                        if((Pre2Cursor & 0xFF) != TX_NA)
                        {
                            Pre2Cursor = 0xF & (hwsAvagoSerdesTxTune16nmPresetParams[sdSpeed][presetIndex].pre2);
                            Pre2Cursor |= (0xC << 12);
                        }
                    }
#endif
                }
                switch(presetIndex)
                {
                    case 0:
                        presetOffset = 0x800; /* Preset1[CL136] and Preset[CL72] */
                        break;
                    case 1:
                        if(serdesConfigPtr->encoding == SERDES_ENCODING_PAM4)
                        {
                            presetOffset = 0x400; /* Preset2[CL136] */
                        }
                        else
                        {
                            presetOffset = 0x0;  /* Initialize[CL72] */
                        }
                        break;
                    case 2:
                        presetOffset = 0xC00;   /* Preset3[CL136] */
                        break;
                    default:
                    return GT_BAD_PARAM;
                }
                if((Atten & 0xFF) != TX_NA){
                    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                                0x3D, Atten|presetOffset, NULL));

                }
                if((PostCursor & 0xFF) != TX_NA){
                    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                                0x3D, PostCursor|presetOffset, NULL));
                }
                if((PreCursor & 0xFF) != TX_NA){
                    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                                0x3D, PreCursor|presetOffset, NULL));
                }

                if((Pre2Cursor & 0xFF) != TX_NA){
                    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                                0x3D, Pre2Cursor|presetOffset, NULL));
                }

            }

            /* Make sure SD temperature is updated from tsen */
            if(temperature < MV_HWS_AVAGO_MIN_TEMEPRATURE)
            {
                temperature = MV_HWS_AVAGO_MIN_TEMEPRATURE;
            }
            else if(temperature > MV_HWS_AVAGO_MAX_TEMPERATURE)
            {
                temperature = MV_HWS_AVAGO_MAX_TEMPERATURE;
            }
            CHECK_STATUS(mvHwsAvago16nmSerdesHalSet(devNum,MV_HWS_SERDES_NUM(curLanesList[i]),HWS_SERDES_TEMPERATURE,0,temperature));

            /*eid disable*/
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]), 0x20, 0, NULL));


            /* Trigger the config state HAL if needed */
            if(cfgHalEnable)
            {
                mvHwsAvago16nmSerdesHalSet(devNum,MV_HWS_SERDES_NUM(curLanesList[i]),(MV_HWS_AVAGO_SERDES_HAL_TYPES)cfgHalType,cfgHalRegSelect,cfgHalRegisterValue);
            }
        }
    }


#endif /* ASIC_SIMULATION */
    return GT_OK;
}
#endif

GT_STATUS mvHwsGeneralApPortInit
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_BOOL serdesInit = GT_TRUE;
    GT_BOOL mask;
#ifndef RAVEN_DEV_SUPPORT
#if defined(MV_HWS_FREE_RTOS)
    GT_BOOL updateTxParam = GT_FALSE;
    GT_BOOL reqInit      = GT_FALSE;
#endif
#ifndef ASIC_SIMULATION
    GT_U32   sdSpeed=0;
    GT_U32   Atten=0;
    GT_U32   PostCursor=0;
    GT_U32   PreCursor=0;
    GT_U32  i;
#endif /* ASIC_SIMULATION */
#endif
    GT_BOOL                   lbPort;
    MV_HWS_REF_CLOCK_SUP_VAL  refClock;
    MV_HWS_REF_CLOCK_SOURCE   refClockSource;
    if(NULL == portInitInParamPtr)
    {
       return GT_BAD_PTR;
    }
    lbPort = portInitInParamPtr->lbPort;
    refClock = portInitInParamPtr->refClock;
    refClockSource = portInitInParamPtr->refClockSource;
    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
       return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsPortLoopbackValidate((GT_U32)curPortParams.portPcsType, lbPort, &serdesInit));

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    if (serdesInit == GT_TRUE)
    {
        if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin)
            || (HWS_DEV_SILICON_TYPE(devNum) == Pipe) || (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) ||
            (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2))
        {
            hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasLanesNum = curPortParams.numOfActLanes;
            hwsDeviceSpecInfo[devNum].avagoSerdesInfo.ctleBiasMacNum  = curPortParams.portMacNumber;
        }
        /* power up the serdes */
        CHECK_STATUS(mvHwsPortSerdesPowerUp(devNum, portGroup, phyPortNum, portMode, refClock, refClockSource, &(curLanesList[0])));
/* in raven we split the serdes power up to several stages*/
#ifndef RAVEN_DEV_SUPPORT
#ifndef ASIC_SIMULATION
        if((AVAGO == (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, curLanesList[0]))) || (AVAGO_16NM == (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(devNum, curLanesList[0]))))
        {
#if defined(MV_HWS_FREE_RTOS)
            /* supported speeds are currently 10G and 25G */
            switch (curPortParams.serdesSpeed)
            {
                case _10_3125G:
                case _10_9375G:
                case _12_1875G:
                updateTxParam = GT_TRUE;
                sdSpeed = 0;
                break;
                case _25_78125G:
                updateTxParam = GT_TRUE;
                sdSpeed = 1;
                break;
#ifndef ALDRIN_DEV_SUPPORT
                case _26_5625G_PAM4:
                case _27_1875_PAM4:
                if (AVAGO_16NM == HWS_DEV_SERDES_TYPE(devNum, curLanesList[0]))
                {
                    updateTxParam = GT_TRUE;
                    sdSpeed = 2 ;
                }
                else
                {
                    updateTxParam = GT_FALSE;
                }
                break;
#endif
            default:
                updateTxParam = GT_FALSE;
            }
#endif

            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                if (HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])) == AVAGO)
                {
                    Atten      = (0xA << 12) | hwsAvagoSerdesTxTuneInitParams[sdSpeed].txAmp;
                    PostCursor = (0xB << 12) | hwsAvagoSerdesTxTuneInitParams[sdSpeed].txEmph0;
                    PreCursor  = (0x9 << 12) | hwsAvagoSerdesTxTuneInitParams[sdSpeed].txEmph1;
                }

#if defined(MV_HWS_FREE_RTOS)
                if (updateTxParam)
                {
                    if((mvHwsAvagoSerdesManualTxOffsetDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])] >> 15) & 0x1)
                        reqInit = GT_TRUE;

                    if (HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])) == AVAGO)
                    {
                        Atten = 0xFF &(hwsAvagoSerdesTxTuneInitParams[sdSpeed].txAmp +
                        POSITIVE_NEGATIVE_VALUE(mvHwsAvagoSerdesManualTxOffsetDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])] &
                                                0x1F));
                        Atten |= (0xA << 12);

                        PostCursor = 0xFF &(hwsAvagoSerdesTxTuneInitParams[sdSpeed].txEmph0 +
                                                 POSITIVE_NEGATIVE_VALUE((mvHwsAvagoSerdesManualTxOffsetDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])] >> 5) &
                                                                         0x1F));
                        PostCursor |= (0xB << 12);

                        PreCursor = 0xFF &(hwsAvagoSerdesTxTuneInitParams[sdSpeed].txEmph1 +
                                                POSITIVE_NEGATIVE_VALUE((mvHwsAvagoSerdesManualTxOffsetDb[sdSpeed][MV_HWS_SERDES_NUM(curLanesList[i])] >> 10) &
                                                                        0x1F));
                        PreCursor |= (0x9 << 12);

                    }
                }
#endif
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                            0x3D, Atten, NULL));

                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                            0x3D, PostCursor, NULL));

                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                            0x3D, PreCursor, NULL));
#if defined(MV_HWS_FREE_RTOS)
                if (reqInit==GT_TRUE)
                {
                        /* Disable PRESET request */
                        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                                    0x835e, 0x1, NULL));
                }
                else
                {
                        /* enable PRESET request */
                        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                                                    0x835e, 0x0, NULL));
                }
#endif
            }
        }
#endif /* ASIC_SIMULATION */
#endif /* RAVEN_DEV_SUPPORT*/
   }
    mask = GT_FALSE;
#ifndef RAVEN_DEV_SUPPORT
    /* set P Sync Bypass - must for 1000baseX, otherwise will see bad octets in traffic */
    CHECK_STATUS(mvHwsPortPsyncBypassCfg(devNum, portGroup, phyPortNum, portMode));

    mask = (portMode == _1000Base_X) ? GT_TRUE : GT_FALSE;
    /* mask/unmask signal detect */
#endif
    CHECK_STATUS(mvHwsGeneralPortSignalDetectMask(devNum, portGroup, phyPortNum, portMode, mask));

    return GT_OK;
}

/**
* @internal mvHwsGeneralPortReset function
* @endinternal
*
* @brief   Clears the port mode and release all its resources according to selected.
*         Does not verify that the selected mode/port number is valid at the core
*         level and actual terminated mode.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] action                   - Power down or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeneralPortReset
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ACTION  action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
       return GT_BAD_PTR;
    }
    if (HWS_DEV_SILICON_INDEX(devNum) == Falcon) {
        hwsOsPrintf("TODO Falcon mvHwsGeneralPortReset: dev %d port %d mode %d action %d\n", devNum, phyPortNum, portMode, action);
        return GT_OK;
    }

#ifndef RAVEN_DEV_SUPPORT
    /* mark port as under delete */
    switch (portMode)
    {
        case SGMII2_5:
        case _2500Base_X:
            /* clear P Sync Bypass */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, curPortParams.portMacNumber,
                                            GIG_PORT_MAC_CONTROL_REGISTER4, 0, (1 << 6)));
            break;
        default:
            break;
    }
#endif
    CHECK_STATUS(mvHwsPortStopCfg(devNum, portGroup, phyPortNum, portMode, action, &(curLanesList[0]), POWER_DOWN, POWER_DOWN));

/* REMOVING THIS BACKDORE MASKING. MASKING SHOULD BE DONE IN THE ORDINARY WAY (XPCS LANE INTERRUPT)*/
#if 0
    /* mask signal detect */
    CHECK_STATUS(mvHwsGeneralPortSignalDetectMask(devNum, portGroup, phyPortNum, portMode, GT_TRUE));
#endif

    /* PORT_RESET on each related serdes */
    for (i = 0; (PORT_RESET == action) && (i < curPortParams.numOfActLanes); i++) {
        CHECK_STATUS(mvHwsSerdesReset(devNum, portGroup, MV_HWS_SERDES_NUM(curLanesList[i]),
                                      HWS_DEV_SERDES_TYPE(devNum, MV_HWS_SERDES_NUM(curLanesList[i])), GT_TRUE, GT_TRUE, GT_TRUE));
    }
    return GT_OK;
}

