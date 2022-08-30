
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
* mvHwsPortCtrlCommonEng.c
*
* DESCRIPTION:
*       Port Control AP Detection Engine
*       common functions for both AP and AN tasks.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <mvHwsPortCtrlInc.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <mvHwsPortCtrlCommonEng.h>


static GT_U32 apPortGroup = 0;

static GT_U8  apDevNum = 0;

#ifdef ALDRIN_DEV_SUPPORT
extern GT_U8 refClockSourceDb[ALDRIN_PORTS_NUM];
#endif

#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
extern GT_U8 refClockSourceDb[MV_HWS_MAX_PORT_NUM];
#endif

#ifdef PIPE_DEV_SUPPORT
extern GT_U8 refClockSourceDb[PIPE_PORTS_NUM];
#endif

#ifdef AC5_DEV_SUPPORT
extern GT_U8 refClockSourceDb[AC5_PORTS_NUM];
#endif

#ifdef ALDRIN_DEV_SUPPORT
/**
* @internal mvApRefClockUpdate function
* @endinternal
*
* @brief   update reference clock
*
* @param[in] refClockSrcFromDb     - source ref clock
* @param[in] validFromDb           - valid or not
* @param[in] cpllOutFreqFromDb     - cpll out frequency
* @param[out] refClockPtr          - pointer to updated ref clock
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE            - on unknown cpll value
* */
GT_STATUS mvApRefClockUpdate
(
    GT_U8                       refClockSrcFromDb,
    GT_U8                       validFromDb,
    GT_U8                       cpllOutFreqFromDb,
    MV_HWS_REF_CLOCK_SUP_VAL    *refClockPtr
)
{
    MV_HWS_CPLL_OUTPUT_FREQUENCY    cpllOutFreq;
    MV_HWS_REF_CLOCK_SOURCE         refClockSource;


    cpllOutFreq = cpllOutFreqFromDb;
    refClockSource = refClockSrcFromDb;

    if (Aldrin == hwsDeviceSpecInfo[0].devType)
    {
        if((refClockSource == SECONDARY_LINE_SRC) && (validFromDb == GT_TRUE))
        {   /* CPLL as ref clock source */
            switch(cpllOutFreq)
            {
                case MV_HWS_156MHz_OUT:
                    *refClockPtr = MHz_156;
                    break;

                case MV_HWS_78MHz_OUT:
                    *refClockPtr = MHz_78;
                    break;

                default:
                    mvPcPrintf("GT_BAD_STATE\n");
                    return GT_BAD_STATE;
            }
        }
        else if (refClockSource == PRIMARY_LINE_SRC)
        {
            *refClockPtr = MHz_156;
        }
    }

    return GT_OK;
}
#endif

/**
* @internal mvApSerdesPowerUp function
* @endinternal
*
* @brief   update reference clock
*
* @param[in] serdesNum         - serdes number
* @param[in] powerUp           - port number
* @param[in] portNum           - powerUp or powerDown
* @param[in] phase             - powerUp phase
*
* @retval 0                        - on success
* @retval 1                        - on error
* */
GT_STATUS mvApSerdesPowerUp(GT_U8 serdesNum, GT_BOOL powerUp, GT_U8 portNum, GT_U8 phase)
{
    GT_STATUS status;
    MV_HWS_SERDES_CONFIG_STC    serdesConfig;

#ifdef RAVEN_DEV_SUPPORT
    GT_UOPT serdes = serdesNum;
#endif
#ifdef ALDRIN_DEV_SUPPORT
    MV_HWS_REF_CLOCK_SUP_VAL    refClock;
    GT_U8 isValid;
    GT_U8 cpllOutFreq;
    GT_U8 refClockSrc;
    isValid = refClockSourceDb[portNum] & 0x1;
    cpllOutFreq = (refClockSourceDb[portNum] >> 1) & 0x7;
    refClockSrc = (refClockSourceDb[portNum] >> 4) & 0xF;
    serdesConfig.refClockSource = refClockSrc;
#elif defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)
    portNum = portNum; /* avoid warning */
    serdesConfig.refClock = (refClockSourceDb[portNum] >> 5) & 0x7;
    serdesConfig.refClockSource = (refClockSourceDb[portNum] >> 4) & 0x1;
#else
    portNum = portNum; /* avoid warning */
    serdesConfig.refClock = _156dot25Mhz;
    serdesConfig.refClockSource = PRIMARY;
#endif

#ifdef RAVEN_DEV_SUPPORT
    serdesConfig.baudRate = _1_25G;
#else
    serdesConfig.baudRate = _3_125G;
#endif
    serdesConfig.media = XAUI_MEDIA;
    if (powerUp)
    {
        serdesConfig.busWidth = _20BIT_ON;
        serdesConfig.encoding = SERDES_ENCODING_NRZ;
    }
    else
    {
        serdesConfig.busWidth = _10BIT_OFF;
        serdesConfig.encoding = SERDES_ENCODING_NA;
    }

    serdesConfig.serdesType = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(apDevNum, serdesNum));

#ifdef ALDRIN_DEV_SUPPORT
    status = mvApRefClockUpdate(refClockSrc,isValid,cpllOutFreq,&refClock);
    if (status != GT_OK)
    {
        mvPcPrintf("Error, mvApRefClockUpdate failed, status : %d ", status);
        return status;
    }
    serdesConfig.refClock = (MV_HWS_REF_CLOCK)refClock;
#endif

#ifdef RAVEN_DEV_SUPPORT /* In RAVEN (cm3) we call the power up in phases (after the first phase) */
    if ((powerUp == GT_TRUE) && (phase != MV_HWS_AVAGO_SERDES_POWER_FIRST_PHASE))
    {
        status = mvHwsAvagoSerdesArrayPowerCtrlImplPhase(apDevNum, apPortGroup, 1, &serdes, &serdesConfig, phase);
    }
    else
#endif
    {
        status = mvHwsSerdesPowerCtrl(apDevNum,             /* devNum         */
                                      apPortGroup,          /* portGroup      */
                                      serdesNum,            /* serdesNum      */
                                      powerUp,              /* powerUp        */
                                      &serdesConfig);
    }

    return status;
}


/**
* @internal mvHwsApConvertPortMode function
* @endinternal
*
*/
MV_HWS_PORT_STANDARD mvHwsApConvertPortMode(MV_HWA_AP_PORT_MODE apPortMode)
{
    switch (apPortMode)
    {
    case Port_1000Base_KX:   return _1000Base_X;
    case Port_10GBase_KX4:   return _10GBase_KX4;
    case Port_10GBase_R:     return _10GBase_KR;
#ifndef ALDRIN_DEV_SUPPORT
    case Port_25GBASE_KR_S:  return _25GBase_KR_S;
    case Port_25GBASE_KR:    return _25GBase_KR;
#endif
    case Port_40GBase_R:     return _40GBase_KR4;
    case Port_40GBASE_CR4:   return _40GBase_CR4;
#ifndef ALDRIN_DEV_SUPPORT
    case Port_100GBASE_KR4:  return _100GBase_KR4;
    case Port_100GBASE_CR4:  return _100GBase_CR4;
    case Port_25GBASE_KR_C:  return _25GBase_KR_C;
    case Port_25GBASE_CR_C:  return _25GBase_CR_C;
    case Port_50GBASE_KR2_C: return _50GBase_KR2_C;
    case Port_50GBASE_CR2_C: return _50GBase_CR2_C;
    case Port_25GBASE_CR_S:  return _25GBase_CR_S;
    case Port_25GBASE_CR:    return _25GBase_CR;
    case Port_20GBASE_KR:    return _20GBase_KR;
    /* PAM4 modes for Raven*/
    case Port_50GBase_KR:    return _50GBase_KR;
    case Port_50GBase_CR:    return _50GBase_CR;
    case Port_100GBase_KR2:  return _100GBase_KR2;
    case Port_100GBase_CR2:  return _100GBase_CR2;
    case Port_200GBase_KR4:  return _200GBase_KR4;
    case Port_200GBase_CR4:  return _200GBase_CR4;
    case Port_200GBase_KR8:  return _200GBase_KR8;
    case Port_200GBase_CR8:  return _200GBase_CR8;
    case Port_400GBase_KR8:  return _400GBase_KR8;
    case Port_400GBase_CR8:  return _400GBase_CR8;
    case Port_40GBase_KR2:   return _40GBase_KR2;
#endif
    default:                 return NON_SUP_MODE;
    }
}

/**
* @internal mvApPolarityCfg function
* @endinternal
*
* @brief   Configure polarity of serdes lanes of port
*
* @param[in] portNum           - powerUp or powerDown
* @param[in] portMode          - port mode in HWS format
* @param[in] polarityVector    - polarity Tx, Rx
*
*
* @retval GT_OK                    - on success
* @retval otherwise                - error
*/
GT_STATUS mvApPolarityCfg(GT_U8 portNum,  MV_HWS_PORT_STANDARD portMode, GT_U16 polarityVector)
{
    GT_STATUS               status;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U8                   i;
    GT_BOOL                 polarityTx;
    GT_BOOL                 polarityRx;

    if (polarityVector)
    {
        if (GT_OK != hwsPortModeParamsGetToBuffer(0, 0, (GT_U32)portNum, portMode, &curPortParams))
        {
           return GT_BAD_PTR;
        }

        for (i = 0; i < curPortParams.numOfActLanes; i++)
        {
            polarityTx = (polarityVector & (1 << (2 * i + 1)))?GT_TRUE:GT_FALSE;
            polarityRx = (polarityVector & (1 << (2 * i)))?GT_TRUE:GT_FALSE;;
            if (polarityTx || polarityRx)
            {
                status = mvHwsSerdesPolarityConfig(0, 0,
                                                   curPortParams.activeLanesList[i],
                                                   HWS_DEV_SERDES_TYPE(0, curPortParams.activeLanesList[i]),
                                                   polarityTx,
                                                   polarityRx);
                if (status != GT_OK)
                {
                    return status;
                }
            }
        }

    }

    return GT_OK;
}



/**
* @internal prvPortCtrlApEngLinkUpCheck function
* @endinternal
*
* @brief   Check link/sync status, which means in AP terms - connection established
*         succesfully or not
* @param[in] portNum                  - port number
* @param[in] portMode                 - interface found by negotiation
* @param[in] fecMode                  - FEC mode found by negotiation(affects which PCS used)
*
* @param[out] linkUpPtr                - connection status
*                                       None.
*
* @retval GT_OK                    - on success
* @retval otherwise                - error
*/
GT_STATUS prvPortCtrlApEngLinkUpCheck
(
    GT_U8                   portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  fecMode,
    GT_BOOL                 *linkUpPtr
)
{
    GT_STATUS               rc;

    *linkUpPtr = GT_FALSE;
    switch (portMode)
    {
        case _10GBase_KX4:
            rc = mvHwsPcsPortLinkStatusGet(apDevNum, apDevNum, portNum, portMode, linkUpPtr);
            if (rc != GT_OK)
            {
                mvPcPrintf("Error, AP Detect, portNum-%d portMode=%d mvHwsPcsPortLinkStatusGet %d!!!\n", portNum, portMode, rc);
                return rc;
            }
            break;

        case _1000Base_X:
            rc = mvHwsPortLinkStatusGet(apDevNum, apDevNum, portNum, portMode, linkUpPtr);
            if (rc != GT_OK)
            {
                mvPcPrintf("Error, AP Detect, portNum-%d portMode=%d mvHwsPortLinkStatusGet %d!!!\n", portNum, portMode, rc);
                return rc;
            }
            break;

        case _10GBase_KR:
        case _40GBase_KR4:
        case _40GBase_CR4:
#ifndef ALDRIN_DEV_SUPPORT
        case _25GBase_KR:
        case _25GBase_CR:
        case _25GBase_KR_C:
        case _50GBase_KR2_C:
        case _25GBase_CR_C:
        case _50GBase_CR2_C:
        case _25GBase_KR_S:
        case _25GBase_CR_S:
        case _100GBase_KR4:
        case _100GBase_CR4:
        case _50GBase_KR:
        case _50GBase_CR:
        case _100GBase_KR2:
        case _100GBase_CR2:
        case _200GBase_KR4:
        case _200GBase_CR4:
        case _200GBase_KR8:
        case _200GBase_CR8:
        case _400GBase_KR8:
        case _400GBase_CR8:
        case _40GBase_KR2:
#endif
            {
                MV_HWS_PORT_INIT_PARAMS curPortParams;

                if ((GT_OK != hwsPortModeParamsGetToBuffer(apDevNum, apDevNum, portNum, portMode, &curPortParams))|| (0 == curPortParams.numOfActLanes))
                {
                    mvPcPrintf("Error, AP Detect, portNum-%d,portMode=%d,hwsPortModeParamsGet:GT_NOT_SUPPORTED\n",
                                portNum, portMode);
                    return GT_NOT_SUPPORTED;
                }

                rc = mvHwsPcsAlignLockGet(apDevNum, apDevNum, portNum, portMode, linkUpPtr);
                if (rc != GT_OK)
                {
                    mvPcPrintf("Error, AP Detect, portNum-%d,portMode=%d,pcsNum=%d,pcsType=%d,mvHwsPcsAlignLockGet %d!!!\n",
                                portNum, portMode, curPortParams.portPcsNumber, curPortParams.portPcsType, rc);
                    return rc;
                }
            }
            break;

        default:
            mvPcPrintf("Error, AP Detect, portNum-%d Invalid port mode %d!!!\n", portNum, portMode);
            return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

#if !defined(BOBK_DEV_SUPPORT)
/**
* @internal mvPortCtrlApPortGetMaxLanes function
* @endinternal
*
* @brief   get port max lanes accourding to capability
* @param[in] capability             - port capability
*
* */
GT_U32 mvPortCtrlApPortGetMaxLanes(GT_U32 capability)
{
    GT_U32 numOfLanes = 0;
    if (capability & ((1<<AP_CTRL_400GBase_CR8_SHIFT) | (1<<AP_CTRL_400GBase_KR8_SHIFT)|
                      (1<<AP_CTRL_200GBase_CR8_SHIFT)| (1<<AP_CTRL_200GBase_KR8_SHIFT)))
    {
        numOfLanes = 8;
    }
    else if (capability & ((1<<AP_CTRL_200GBase_CR4_SHIFT) | (1<<AP_CTRL_200GBase_KR4_SHIFT)|
                           (1<<AP_CTRL_100GBase_CR4_SHIFT)| (1<<AP_CTRL_100GBase_KR4_SHIFT)|
                           (1<<AP_CTRL_40GBase_CR4_SHIFT)| (1<<AP_CTRL_40GBase_KR4_SHIFT) | (1<<AP_CTRL_10GBase_KX4_SHIFT)))
    {
        numOfLanes = 4;
    }
    else if (capability & ((1<<AP_CTRL_100GBase_CR2_SHIFT) | (1<<AP_CTRL_100GBase_KR2_SHIFT)|
                           (1<<AP_CTRL_50GBase_CR2_CONSORTIUM_SHIFT)| (1<<AP_CTRL_50GBase_KR2_CONSORTIUM_SHIFT)|
                           (1<<AP_CTRL_10GBase_KX2_SHIFT) | (1<<AP_CTRL_20GBase_KR2_SHIFT)))
    {
        numOfLanes = 2;
    }
    else if (capability != 0)
    {
        numOfLanes = 1;
    }

    return numOfLanes;
}
#endif
