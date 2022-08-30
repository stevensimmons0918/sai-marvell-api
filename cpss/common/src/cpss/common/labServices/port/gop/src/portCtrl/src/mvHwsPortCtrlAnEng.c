
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
* mvHwsPortCtrlAn.c
*
* DESCRIPTION:
*       Port Control AP Detection Engine
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <mvHwsPortCtrlInc.h>
#include <mvHwsServiceCpuInt.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <mvHwsPortCtrlCommonEng.h>
#include <mvHwsPortCtrlAn.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <mvHwsPortCtrlLog.h>


/* Default values for AP Device and Port Group */
#ifdef MV_SERDES_AVAGO_IP
#include "cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h"
#endif /* MV_SERDES_AVAGO_IP */

#include "mvHwsPortCtrlDoorbellEvents.h"
#include "mvHwsPortCtrlAn.h"

#define LINK_CONTROL_ENABLE 0x2
/*
#define DEBUG_AN
*/
extern MV_HWS_DFE_MODE  mvPortCtrlLinkUpDfeMode;
extern GT_U32 mvHws40GAnResolutionBitShift;
extern GT_U8 mvHwsMiMode;


GT_U32 anPortGroup = 0;
GT_U8  anDevNum = 0;
GT_BOOL autoKrAndEnableAvagoTimers = GT_FALSE;
GT_U16 printCounter[MV_PORT_CTRL_MAX_AP_PORT_NUM] = {0};
GT_U16 status2[MV_PORT_CTRL_MAX_AP_PORT_NUM] = {0};
GT_U16 dfeStatus[MV_PORT_CTRL_MAX_AP_PORT_NUM] = {0};
GT_U8  stage11d[MV_PORT_CTRL_MAX_AP_PORT_NUM] = {0};
GT_U16 extraTime[MV_PORT_CTRL_MAX_AP_PORT_NUM] = {0};
GT_U8  calDeltaCntr[MV_PORT_CTRL_MAX_AP_PORT_NUM] = {0xFF};
GT_U32 timestampTime2[MV_PORT_CTRL_MAX_AP_PORT_NUM] = {0};

/* HF for each temperature*/
typedef struct {
    GT_U8   upToTemperature;
    GT_U8   hfThreshold;
}MV_HWS_TEMP_HF_STC;

/* last value temperature must be 0xFF*/
MV_HWS_TEMP_HF_STC hfThresholdPerTemperature[] = {{4,4},{19,5},{28,6},{37,7},{48,8},{60,9},{0xFF,10}};

static GT_STATUS mvAnConvertAdvertisement
(
    GT_U8 portIndex,
    MV_HWA_AP_PORT_MODE *apPortMode,
    GT_U32 *fecType,
    MV_HWS_PORT_FEC_MODE *portFecType
);

static void mvPortCtrlAnPortFail
(
    char                           *errorMsg,
    GT_U32                          portIndex,
    GT_U8                           status
);
#ifdef CONSORTIUM_SUPPORT
static GT_STATUS mvAnConsortiumPortResolution
(
    GT_U8 portIndex,
    GT_BOOL *resolutionDone
);
#endif
/**
* @internal mvAnResetStatus function
* @endinternal
*
* @brief  reset status
*
* @param[in] portIndex             - number of physical port
*
* */
static void mvAnResetStatus
(
    IN  GT_U8      portIndex
)
{
    MV_HWS_AN_SM_INFO *anSm = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);

    anSm->hcdStatus   = 0;
    anSm->status      = 0;
    anSm->portMode    = NON_SUP_MODE;
}

/**
* @internal mvAnResetTimer function
* @endinternal
*
* @brief  reset timer
*
* @param[in] portIndex             - number of physical port
* @param[in] isAnTimers            - An timers or link timers
* */
static void mvAnResetTimer
(
    IN  GT_U8      portIndex
)
{
    MV_HWS_AN_SM_TIMER *anTimer = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].timer);

    anTimer->abilityThreshold = 0;
    anTimer->abilityCount     = 0;
    anTimer->linkThreshold    = 0;
    anTimer->linkCount        = 0;

}

/**
* @internal mvAnResetStats function
* @endinternal

* @brief   reset statistic
*
* @param[in] portIndex                 - number of physical port
*
* */
void mvAnResetStats
(
    IN  GT_U8      portIndex
)
{
    MV_HWS_AN_SM_STATS *anStats = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].stats);

    anStats->linkFailCnt       = 0;
    anStats->linkSuccessCnt    = 0;
}

/**
* @internal mvAnResetStatsTimestamp function
* @endinternal

* @brief   reset statistic timestamp
*
* @param[in] portIndex                 - number of physical port
*
* */
static void mvAnResetStatsTimestamp
(
    IN  GT_U8      portIndex
)
{
    MV_HWS_AN_SM_STATS *anStats = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].stats);

    anStats->timestampTime     = 0;
    anStats->hcdResoultionTime = 0;
    anStats->linkUpTime        = 0;
}


/**
* @internal mvHwsAvagoAnConvertHcdToApPortMode function
* @endinternal
*
* @brief   convert raven HCD to port ap mode
*
* @param[in] resulotionResult     - the HCD result
*
* @param[out] apPortModePtr       - the AP port mode
*
* */
static GT_STATUS mvHwsAvagoAnConvertHcdToApPortMode
(
    IN  GT_U32               resulotionResult,
    IN  MV_HWS_AN_SM_INFO    *anSm,
    OUT MV_HWA_AP_PORT_MODE  *apPortModePtr
)
{
 /* Note:
        Port_200GBase_KR8 - Not Supported Yet,
        Port_400GBase_KR8,- Not Supported Yet
  */

    switch (resulotionResult)
    {
        case 0x00:
            *apPortModePtr = Port_1000Base_KX;
            break ;
        case 0x01:
            *apPortModePtr = Port_10GBase_KX4; /* not supported*/
            break ;
        case 0x02:
            *apPortModePtr = Port_10GBase_R;
            break ;
        case 0x03:
            *apPortModePtr = Port_40GBase_R;
            break;
        case 0x04:
            *apPortModePtr = Port_40GBASE_CR4;
            break ;
        case 0x05:
            *apPortModePtr = Port_100GBASE_CR10; /* not supported*/
            break ;
        case 0x06:
            *apPortModePtr = Port_100GBASE_KP4; /* not supported*/
            break ;
        case 0x08:
            *apPortModePtr = Port_100GBASE_KR4;
            break ;
        case 0x09:
            *apPortModePtr = Port_100GBASE_CR4;
            break ;
        case 0x0a:
            if (AP_CTRL_25GBase_KR1S_GET(anSm->capability))
                *apPortModePtr = Port_25GBASE_KR_S;
            else
                *apPortModePtr = Port_25GBASE_CR_S;
            break ;
        case 0x0b:
            if (AP_CTRL_25GBase_KR1_GET(anSm->capability))
                *apPortModePtr = Port_25GBASE_KR;
            else
                *apPortModePtr = Port_25GBASE_CR;
            break ;
        case 0x0e:
            if (AP_CTRL_50GBase_KR1_GET(anSm->capability))
                *apPortModePtr = Port_50GBase_KR;
            else
                *apPortModePtr = Port_50GBase_CR;
            break ;
        case 0x10:
            if (AP_CTRL_100GBase_KR2_GET(anSm->capability))
                *apPortModePtr = Port_100GBase_KR2;
            else
                *apPortModePtr = Port_100GBase_CR2;
            break ;
        case 0x11:
            if (AP_CTRL_200GBase_KR4_GET(anSm->capability))
                *apPortModePtr = Port_200GBase_KR4;
            else
                *apPortModePtr = Port_200GBase_CR4;
            break ;

       default:
            return GT_FAIL;
    }
    return GT_OK;
}

 /**
* @internal mvAnPortReset function
* @endinternal
*
* @brief   execute port reset
*
* @param[in] portIndex           - index of port in local AP DB
* */
static GT_STATUS mvAnPortReset
(
    IN  GT_U8      portIndex
)
{
    MV_HWS_AN_SM_INFO *anSm = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_PORT_STANDARD portMode = anSm->portMode;
    GT_U32 regData, regMask, localPortIndex, i;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_STATUS rc;
    GT_BOOL macEnable;

    /* Un-Reset mtip */
    /* mvHwsMtipExtReset(anDevNum, anSm->portNum, portMode, GT_FALSE);*/

    if (GT_OK != hwsPortModeParamsGetToBuffer(anDevNum,anPortGroup, (GT_U32)anSm->portNum, portMode, &curPortParams))
    {
        /*anSm->status = AN_PORT_DELETE_FAILURE;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_DELETE_FAILURE);

        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, (GT_U32)anSm->portNum));
        return GT_FAIL;
    }
    /* bit [7:0] v = m_RAL.mtip_ext_units_RegFile.Global_Reset_Control.gc_sd_tx_reset_.get();*/
    /* for UNIT_MTI_EXT we check only port 0 or 8 */
    if (!mvHwsMtipIsReducedPort(anDevNum, anSm->portNum))
    {
        regData = 0;
        localPortIndex = anSm->portNum %8;
        regMask = 0;
        /* on each related serdes */
        for (i = 0; i < curPortParams.numOfActLanes; i++)
        {
            /* set the rx[0-7] and the tx[8-15] bits */
            regMask |= ((1<<(localPortIndex+i) )| (1<<(8+localPortIndex+i)));
        }
        genUnitRegisterSet(anDevNum, 0, MTI_EXT_UNIT, anSm->portNum, MTIP_EXT_GLOBAL_RESET_CONTROL, regData, regMask);
    }
    else
    {
        regData = 0;
        regMask = 0x3 ;
        genUnitRegisterSet(anDevNum, 0, MTI_CPU_EXT_UNIT, anSm->portNum,
                                     MTIP_CPU_EXT_PORT_RESET, regData, regMask);
    }

   /*protect mac clock enable*/
    mvHwsHWAccessLock(anDevNum, MV_SEMA_PM_CM3);

    /* read mac clock */
    rc = mvHwsMtipExtMacClockEnableGet(anDevNum,anSm->portNum, portMode, &macEnable);
    if (( rc == GT_OK ) && (macEnable == GT_FALSE)) {
        rc = GT_FAIL;
        /*mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(anSm->portNum, 7, 77, 77 ));*/
    }
    else
    {
        /* Execute HWS Port Reset */
        rc = mvHwsPortReset(anDevNum,
                            anPortGroup,
                            (GT_U32)anSm->portNum,
                            (MV_HWS_PORT_STANDARD)portMode,
                            PORT_POWER_DOWN);
    }
    mvHwsHWAccessUnlock(anDevNum, MV_SEMA_PM_CM3);

    if (rc != GT_OK ) {

    /*anSm->status = AN_PORT_DELETE_FAILURE;*/
        AN_PORT_SM_SET_STATE(anSm->portNum, anSm, AN_PORT_SM_DELETE_STATE);

        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, (GT_U32)anSm->portNum));
        return GT_OK;
    }
    return GT_OK;
}


/**
* @internal mvAnPortDelete function
* @endinternal
*
* @brief   execute port reset
*
* @param[in] portIndex           - index of port in local AP DB
*
* @param[in] rest                 - indicate if port disable operation (or delete)
*
* */
static GT_STATUS mvAnPortDelete
(
    IN  GT_U8      portIndex,
    IN  GT_BOOL    disable
)
{
    MV_HWS_AN_SM_INFO    *anSm = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_AN_SM_STATS   *anStats  = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].stats);
    GT_BOOL portEnable = GT_FALSE;
    MV_HWA_AP_PORT_MODE  apPortMode;
    MV_HWS_PORT_STANDARD portMode;
    GT_STATUS rc = GT_OK;
    GT_BOOL macEnable = GT_FALSE;

    apPortMode = AP_ST_HCD_TYPE_GET(anSm->hcdStatus);
    portMode = mvHwsApConvertPortMode(apPortMode);

    if(portMode == NON_SUP_MODE)
    {
        return GT_NOT_SUPPORTED;
    }

    if (AN_PORT_SERDES_TRAINING_IN_PROGRESS == anSm->status)
    {
        /* Execute HWS Training check */
        if (mvHwsPortAutoTuneSet(anDevNum, /* devNum */
                                 anPortGroup, /* portGroup */
                                 (GT_U32)anSm->portNum,
                                 (MV_HWS_PORT_STANDARD)portMode,
                                 TRxTuneStop,
                                 NULL) != GT_OK)
        {
            /*anSm->status = AN_PORT_SERDES_TRAINING_FAILURE;*/
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_TRAINING_FAILURE);

            mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, (GT_U32)anSm->portNum));
            /* still don't exit - try to delete port */
        }

        /* let training finish */
        hwsOsTimerWkFuncPtr(10);
    }

    /*anSm->status = AN_PORT_DELETE_IN_PROGRESS;*/
    AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_DELETE_IN_PROGRESS);
    mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, (GT_U32)anSm->portNum));

    if ( mvHwsMiMode )
    {
        rc = mvAnPortReset(portIndex);
        if(rc == GT_OK)
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_DELETE_SUCCESS);
        return rc;
    }

    if(disable)
    {
        rc = mvAnPortReset(portIndex);
    }
    else
    {
       /*protect mac clock enable*/
        mvHwsHWAccessLock(anDevNum, MV_SEMA_PM_CM3);

        if (anSm->portMode != NON_SUP_MODE) {
            /* read mac clock */
            rc = mvHwsMtipExtMacClockEnableGet(anDevNum,anSm->portNum, portMode, &macEnable);
            if (( rc == GT_OK ) && (macEnable == GT_FALSE)) {
                portEnable = GT_FALSE;
               /* mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(anSm->portNum, 7, 77, 79 ));*/
            }
            else
            {

                mvHwsPortEnableGet(0, 0, anSm->portNum, portMode, &portEnable);
            }
        }
        mvHwsHWAccessUnlock(anDevNum, MV_SEMA_PM_CM3);
        if (portEnable)
        {
            /*anSm->status = AN_PORT_ENABLE_WAIT;*/
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_ENABLE_WAIT);
            anStats->timestampTime = mvPortCtrlCurrentTs();
            /* send doorbell - to stop the port */
            doorbellIntrAddEvent(AP_DOORBELL_EVENT_PORT_AP_DISABLE(anSm->portNum));
        }
        else
        {
            rc = mvAnPortReset(portIndex);
            if(rc == GT_OK)
                AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_DELETE_SUCCESS);
        }
    }
    return rc;
}

/**
* @internal mvPortCtrlAnPortFail function
* @endinternal
*
* @brief   Execute fail sequence for different failure cases
*          doing port configuration and training.
*
* @param[in] errorMsg                 - string describing failure
* @param[in] portIndex                - index of port in local AP DB
* @param[in] status                   - new  of port treatment
*                                       None.
*/
static void mvPortCtrlAnPortFail
(
    IN  char        *errorMsg,
    IN  GT_U32       portIndex,
    IN  GT_U8        status
)
{
    MV_HWS_AN_SM_INFO  *anSm    = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_AN_SM_STATS *anStats       = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].stats);

    mvPcPrintf(errorMsg, anSm->portNum);
    AN_PRINT_MAC(("mvPortCtrlAnPortFail: port:%d %s\n",portIndex,errorMsg));

    /*anSm->status = status;*/
    AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, status);
    mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

    if ((AN_CTRL_RX_TRAIN_GET(anSm->options)) && (anSm->state  < AN_PORT_SM_LINK_STATUS_STATE))
    {
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_CONFIGURATION_IN_PROGRESS);
        AN_PORT_SM_SET_STATE(anSm->portNum, anSm, AN_PORT_SM_SERDES_CONFIG_STATE);
        mvAnResetStatsTimestamp(anSm->portNum);
        timestampTime2[anSm->portNum] = mvPortCtrlCurrentTs();
       /* mvPortCtrlThresholdSet(anIntrop->apLinkDuration, &(anTimer->linkThreshold));*/
        anStats->timestampTime = PORT_CTRL_TIMER_DEFAULT/*mvPortCtrlCurrentTs()*/;

    }
    else
    {
        mvAnPortDelete(portIndex,GT_FALSE);

        /* AP state & status update */
        /*anSm->state = AN_PORT_SM_DELETE_STATE;*/
        AN_PORT_SM_SET_STATE(anSm->portNum, anSm, AN_PORT_SM_DELETE_STATE);
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
    }

    return;
}

/**
* @internal mvAnPortRemoteFaultEnable function
* @endinternal

* @brief  enable/disable remote fault
*
* @param[in] portIndex                 - number of physical port
* @param[in] enableRmf                 - enable/disable
*
* */
GT_STATUS  mvAnPortRemoteFaultEnable(
    IN  GT_U32   portIndex,
    IN  GT_BOOL  enableRmf
)
{
    MV_HWS_AN_SM_INFO  *anSm    = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_PORT_STANDARD            portMode = anSm->portMode;
    GT_U32  regAddr;

    if ((!mvHwsMtipIsReducedPort(anDevNum, anSm->portNum)) && ( !HWS_USX_SLOW_MODE_CHECK(portMode) ))
    {
        switch ( portMode )
        {
        case HWS_200G_PAM4_MODE_CASE:
        case HWS_400G_PAM4_MODE_CASE:
            regAddr = MTIP_EXT_SEG_PORT_CONTROL + 0x14 * ((anSm->portNum%8)/4);
            break;
        default:
            regAddr = MTIP_EXT_PORT_CONTROL + 0x18 * (anSm->portNum%8);
            break;
        }
      /*  mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(anSm->portNum, 0, portMode, enableRmf ));*/
        genUnitRegisterSet(anDevNum, 0, MTI_EXT_UNIT, anSm->portNum, regAddr, enableRmf, 0x1);
    }
    return GT_OK;
}

/**
* @internal mvAnSerdesStatus function
* @endinternal
*
* @brief   Reads various status of AN process
*
*
* @param[in] portIndex           - index of port in local AP DB
* @param[in] mvAnSerdesStatus    - the status to read
* @param[out] ret                - return value of ARB SM
*/
static GT_STATUS mvAnSerdesStatus
(
    IN  GT_U8                 serdesNum,
    IN  MV_AN_SERDES_STATUS   status,
    OUT GT_U32                *ret
)
{
    GT_32 resulotionResult;

    *ret = 0;

    switch(status)
    {
        /*HCD value*/
        case AN_SERDES_AN_READ_HCD:
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0707, 0x01, &resulotionResult));
            *ret =  (resulotionResult & 0x01f);
            break;
        case AN_SERDES_AN_READ_FEC_ENABLE:
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0707, 0x01, &resulotionResult));
            *ret = (resulotionResult & 0x0100)>> 8;
            break;
        case AN_SERDES_AN_READ_RSFEC_ENABLE:
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0707, 0x01, &resulotionResult));
            *ret = (resulotionResult & 0x0080)>> 7;
            break;
        case AN_SERDES_AN_BASE_PAGE_RX:
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0707, 0x01, &resulotionResult));
            *ret = (resulotionResult & 0x0200)>> 9;
            break;
        case AN_SERDES_AN_NEXT_PAGE_RX:
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x4069, 0x01, &resulotionResult));
            *ret = (resulotionResult & 0x0002) >> 1;
            break;
        case AN_SERDES_AN_LP_AN_ABLE:
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0707, 0x01, &resulotionResult));
            *ret = (resulotionResult & 0x1000) >> 12;
            break;
            /* o_core_status[19]*/
        case AN_SERDES_AN_COMPLETE:
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x4069, 0x01, &resulotionResult));
            *ret = (resulotionResult & 0x8) >> 3;
            break;
            /*o_core_status[20]*/
       case AN_SERDES_AN_GOOD:
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x4069, 0x01, &resulotionResult));
            *ret = (resulotionResult & 0x10)>> 4;
            break;
            /*link control: o_core_status[21:22]*/
        case AN_SERDES_AN_READ_LINK_CONTROL:
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x4069, 0x01, &resulotionResult));
            *ret = (resulotionResult & 0x60)>> 5 ;
            break;
        case AN_SERDES_AN_READ_TRAINING_RESULTS:
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x4027, 0x01, &resulotionResult));
            *ret = (resulotionResult & 0xffff);
            break;
        case AN_SERDES_AN_ARB_STATE:
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0807, 0x01, &resulotionResult));
            *ret = (resulotionResult & 0x0f);
            break;
        case AN_SERDES_AN_DATA_OUT:
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0707, 0x01, &resulotionResult));
            *ret = resulotionResult;
            break;


       default:
            return GT_FAIL;
    }
    /*mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(serdesNum, 1, status, (*ret) ));*/

    return GT_OK;
}



/**
* @internal mvAnPortAutoNegOrderCapability function
* @endinternal
*
* @brief   take the port advertsment list, return capability
*          vector in IEEE format.
*
* @param[in]  currCapability        - current order currCapability
* @param[out] orederdCapabilityPtr  - the correct order of capability
*
*/
static void mvAnPortAutoNegOrderCapability
(
    IN  GT_U32      currCapability,
    OUT GT_16      *orederdCapabilityPtr
)
{
    /*return capability vector in IEEE format . the vector will advertise by the DME*/
    *orederdCapabilityPtr =
        (AP_CTRL_1000Base_KX_GET(currCapability)  << AN_ADV_1G_KX_SHIFT   ) |
        (AP_CTRL_10GBase_KX4_GET(currCapability)  << AN_ADV_10G_KX4_SHIFT ) |
        (AP_CTRL_10GBase_KR_GET(currCapability)   << AN_ADV_10G_KR_SHIFT  ) |
        (AP_CTRL_40GBase_KR4_GET(currCapability)  << AN_ADV_40G_KR4_SHIFT ) |
        (AP_CTRL_40GBase_CR4_GET(currCapability)  << AN_ADV_40G_CR4_SHIFT ) |
        (AP_CTRL_100GBase_KR4_GET(currCapability) << AN_ADV_100G_KR4_SHIFT) |
        (AP_CTRL_100GBase_CR4_GET(currCapability) << AN_ADV_100G_CR4_SHIFT) |
        (AP_CTRL_25GBase_KR1S_GET(currCapability) << AN_ADV_25G_S_SHIFT   ) |
        (AP_CTRL_25GBase_CR1S_GET(currCapability) << AN_ADV_25G_S_SHIFT   ) |
        (AP_CTRL_25GBase_KR1_GET(currCapability)  << AN_ADV_25G_SHIFT     ) |
        (AP_CTRL_25GBase_CR1_GET(currCapability)  << AN_ADV_25G_SHIFT     ) |
        (AP_CTRL_50GBase_KR1_GET(currCapability)  << AN_ADV_50G_KR1_SHIFT ) |
        (AP_CTRL_50GBase_CR1_GET(currCapability)  << AN_ADV_50G_KR1_SHIFT ) |
        (AP_CTRL_100GBase_KR2_GET(currCapability) << AN_ADV_100G_KR2_SHIFT) |
        (AP_CTRL_100GBase_CR2_GET(currCapability) << AN_ADV_100G_KR2_SHIFT) |
        (AP_CTRL_200GBase_KR4_GET(currCapability) << AN_ADV_200G_KR4_SHIFT) |
        (AP_CTRL_200GBase_CR4_GET(currCapability) << AN_ADV_200G_KR4_SHIFT) ;
        /*(AP_CTRL_200GBase_KR8_GET(currCapability) << AN_ADV_200G_KR8_SHIFT) |
        (AP_CTRL_400GBase_KR8_GET(currCapability) << AN_ADV_400G_KR8_SHIFT) |
        (AP_CTRL_400GBase_CR8_GET(currCapability) << AN_ADV_400G_KR8_SHIFT);*/

}

/**
* @internal mvAnPortAutoNegOrderFec function
* @endinternal
*
* @brief   return FEC bits to advertise according to IEEE
*
* @param[in] portIndex           - index of port in local AP DB
* @param[out] fecRequest         - fecRequest in the right order
*       according to IEEE
*/
static void mvAnPortAutoNegOrderFec
(
    IN  GT_U8      portIndex,
    IN  GT_16     *fecRequest
)
{

    MV_HWS_AN_SM_INFO  *anSm    = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    GT_U32 f0,f1,f2,f3;

    /*f0 -  fecAbility bit 46
      f1 - 10G FEC bit 47
      f2 - 25G and above RS fec bit 44
      f3 - 25G and above FC fec bit 45
     */
    f0 = AP_CTRL_FEC_ABIL_GET(anSm->options);   /* BIT 4 in options*/
    f1 = AP_CTRL_FEC_REQ_GET(anSm->options);    /* BIT 5 in options*/
    f2 = AP_CTRL_FEC_RS_REQ_GET(anSm->options); /* BIT 7 in options*/
    f3 = AP_CTRL_FEC_FC_REQ_GET(anSm->options); /* BIT 6 in options*/

    *fecRequest = (((f0 & 0x1) << 14) |  ((f1 & 0x1) << 15) |
                   ((f3 & 0x1) << 13) | ((f2 & 0x1) << 12));

}

/**
* @internal mvAnPortAutoNegAnConfig function
* @endinternal
*
* @brief   AN Port configure AN parameters, create DME page and
*          start AN
*
* @param[in] portIndex           - index of port in local AP DB
*/
static GT_STATUS mvAnPortAutoNegAnConfig
(
    IN  GT_U8      portIndex
)
{
    /* AN parametrs*/
    GT_16 cap1, cap2, fecRequest, npEnable, pause, asmDir, nonceUserPattern, npContinuousLoad;
    GT_16 anClk,noncePatternSel;
    GT_16 disableLinkInhibitTimer, ignoreNonceMatch;

    MV_HWS_AN_SM_INFO  *anSm    = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);

    /* AN inputs*/
    GT_U32 anInput = 0x0001;
    GT_16 an29Params = 0x0000;
    GT_16 an129Params = 0x0000;
    GT_16 an229Params = 0x0000;
    GT_16 orederdCapability = 0x0000;
    GT_U32 serdesNum = AP_CTRL_LANE_GET(anSm->ifNum);
    GT_32 temperature;

    mvAnPortAutoNegOrderCapability(anSm->capability,&orederdCapability);
    /* Get auto neg parameters*/
    cap1 = (orederdCapability & 0x7ff) << 5;
    cap2 = (orederdCapability & 0xf800) >> 11;

    /*np = (serdesAnConfigArray[devNum1][serdesNum1].np_enable & 0x1) << 15; tbd*/
    pause  = (AP_CTRL_FC_PAUSE_GET(anSm->options)& 0x1) << 10;
    asmDir = (AP_CTRL_FC_ASM_GET(anSm->options)& 0x1) << 11;

    nonceUserPattern =  0;/* should be with mask 0x1f but it is 0 for now..*/

    mvAnPortAutoNegOrderFec(portIndex,&fecRequest);

    /* Base page bit definition:
      # bit[15:0] configured through Int 0x0029:
          D[4:0] Selector field
          D[9:5] contains the Echoed Nonce field
          D[11:10] contains flow control capability
          D[15:13] contains the RF, Ack, and NP bits

      # bit[16:31] configured through Int 0x0129:
          D[20:16] contains the Transmitted Nonce field
          D[31:21] contains the Technology Ability field -part 1

      # bit[47:32] configured through Int 0x0229:
          D[43:32] contains the Technology Ability field -part 2
          D[47:44] contains FEC capability
    */
    npContinuousLoad        = 1; /* next page continuous load feature useful when a device has no
                                    Next Page data to transmit and the device does not plan to respond
                                    to the Next Page data received from its link partner.
                                    (only effective when the local device has no Next Page data to transmit)
                                    1 - auto load null page
                                    0 - cancel the feature */
    npEnable         =  0 << 15;
#ifdef CONSORTIUM_SUPPORT
    if (AP_CTRL_ADV_CONSORTIUM_GET(anSm->capability))
    {
        AN_PRINT_MAC(("set consortium portIndex:%d advertise capability:0x%x\n",portIndex, anSm->capability));
        npEnable         =  1 << 15;
        npContinuousLoad        = 0;
    }
#endif
    /* Load base page using Serdes Interrupt 0x29 */
    an29Params  =  (0x0001 | npEnable | pause | asmDir);
    an129Params = (nonceUserPattern | cap1);
    an229Params = (cap2 | fecRequest);

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0029, an29Params, NULL));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0129, an129Params, NULL));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0229, an229Params, NULL));

#if 0
    if (AN_CTRL_AUTO_KR_ENABLE_GET(anSm->options) == GT_TRUE)
    {
        /* Configure Auto KR */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x907, 0x1, NULL));
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0xA07, 0x12, NULL));/* pmd config*/
    }
#endif
    anClk                   = 0 /*1: 3.125, 0: 1.25*/;
    disableLinkInhibitTimer = ((AN_CTRL_DISABLE_LINK_TIMER_GET(anSm->options) == GT_TRUE) ? 1 : 0);
    ignoreNonceMatch        = AP_CTRL_LB_EN_GET(anSm->options);
    noncePatternSel         = 0;

    /* auto load device's next pages to null */
    if(npContinuousLoad == 1)
    {
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum,anPortGroup, serdesNum, 0x507, 0x2, NULL));
    }

    /* Set user settings from the config struct and enable Auto Negotiation */
    anInput |= (anClk                    & 0x1) <<  1;
    anInput |= (disableLinkInhibitTimer  & 0x1) <<  2;
    anInput |= (ignoreNonceMatch         & 0x1) <<  3;
    anInput |= (noncePatternSel          & 0x3) <<  4;

    /* Get Tj temperature for envelop settings in the SerDes config state.
       To save time, the function reads the temperature from the hardware only
       if it has exceeded 5 seconds from the last time it was read
       thus we do it now to save time during the AN */
    CHECK_STATUS(mvHwsPortTemperatureGet(anDevNum, anPortGroup,0,&temperature));

    /*Start AN*/
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum,anPortGroup, serdesNum, 0x07, anInput, NULL));

    return GT_OK;

}


/**
* @internal mvAnPortAutoNeg function
* @endinternal
*
* @brief  execute Auto Negotiation. Configure the DME page and all the relevant avago SM registers
*         start Auto Negotiation, pollink o_core_status[20] for HCD result
*         extract all the relevant information after resulotion
*
*         linkThreshold = the time from HCD to full link
*         training threshold = the time for TRX training
*
*
* @param[in] portIndex             - number of physical port
*
* @retval GT_OK                    - on success/delete
* */
GT_STATUS mvAnPortAutoNeg
(
    IN  GT_U8      portIndex
)
{
    MV_HWS_AN_SM_INFO  *anSm          = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_AN_SM_TIMER *anTimer       = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].timer);
    MV_HWS_AN_SM_STATS *anStats       = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].stats);

    GT_U32               linkControlResulotionResult = 0x00000000;
    GT_U32               AnGoodResult = 0x00000000;
    GT_U32               dataOutResulotionResult = 0x00000000;
    MV_HWA_AP_PORT_MODE  apPortMode = MODE_NOT_SUPPORTED;
    MV_HWS_AP_DETECT_ITEROP *anIntrop = &(mvHwsPortCtrlAnPortDetect.introp);
    GT_U8                serdesNum  = AP_CTRL_LANE_GET(anSm->ifNum);
    GT_U32               hcdStatus;
    GT_U32               fcFecEnable;
    GT_U32               rsFecEnable;
    GT_U32               fecType = AP_ST_HCD_FEC_RES_NONE;
    GT_STATUS            rc;
    MV_HWS_PORT_FEC_MODE portFecType = FEC_NA;
#ifdef CONSORTIUM_SUPPORT
    GT_BOOL              resolutionDone = GT_FALSE;
    GT_U8                priority = 0;
    GT_U16               priorityConsortium = 0;
    GT_BOOL              hcdTypeIsConsortium = GT_FALSE;
#endif
    MV_HWS_PORT_INIT_PARAMS *portModesCurr;
    GT_U32                       profile;

    if ((AP_CTRL_SPECIAL_SPEED_GET(anSm->options)&& AP_CTRL_SPECIAL_SPEED_MODE_GET(anSm->capability)) || (AN_CTRL_SKIP_RES_GET(anSm->options)))/*106G, 42G, 53G,47G */
    {
        extraTime[anSm->portNum] = 0;
        /* AN state & status update */
        /*anSm->state =  AN_PORT_SM_SERDES_CONFIG_STATE;*/
        AN_PORT_SM_SET_STATE(anSm->portNum, anSm, AN_PORT_SM_SERDES_CONFIG_STATE);
        /*anSm->status = AN_PORT_RESOLUTION_SUCCESS;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_RESOLUTION_SUCCESS);
        if((AP_CTRL_SPECIAL_SPEED_MODE_GET(anSm->capability)) && (AP_CTRL_SPECIAL_SPEED_GET(anSm->options)))
        {
            if (AP_CTRL_100GBase_KR4_GET(anSm->capability))     /* 106G */
            {
                apPortMode = Port_100GBASE_KR4;
                anSm->portMode = _100GBase_KR4;
            }
            else if (AP_CTRL_40GBase_KR4_GET(anSm->capability)) /* 42G/48G */
            {
                apPortMode = Port_40GBase_R;
                anSm->portMode = _40GBase_KR4;
            }
            else if (AP_CTRL_50GBase_KR2_CONSORTIUM_GET(anSm->capability)) /* 53G */
            {
                apPortMode = Port_50GBASE_KR2_C;
                anSm->portMode = _50GBase_KR2_C;
            }
            else if (AP_CTRL_100GBase_KR2_GET(anSm->capability))
            {
                apPortMode = Port_100GBase_KR2;
                anSm->portMode = _100GBase_KR2;
            }
            else return GT_FAIL;
        }
        else
        {
            if (mvAnConvertAdvertisement(portIndex,&apPortMode,&fecType,&portFecType) != GT_OK)
                return GT_FAIL;
        }

        AP_ST_HCD_TYPE_SET(anSm->hcdStatus, apPortMode);
        AP_ST_HCD_IS_OPTICAL_MODE_SET(anSm->hcdStatus,AN_CTRL_IS_OPTICAL_MODE_GET(anSm->options));
        /* AP Port mode parameters update */
        if(mvHwsApSetPortParameters(anSm->portNum, apPortMode) != GT_OK)
        {
            mvPcPrintf("Error, AP , Port-%d Params set Failed\n", anSm->portNum);
            return GT_FAIL;
        }

        portModesCurr = hwsPortModeParamsGet(anDevNum, anPortGroup, anSm->portNum, anSm->portMode,NULL);

        if ((AP_CTRL_SPECIAL_SPEED_MODE_GET(anSm->capability)) && (AP_CTRL_SPECIAL_SPEED_GET(anSm->options)))
        {
            if(portModesCurr != NULL && AP_CTRL_100GBase_KR4_GET(anSm->capability))/* 106G */
            {
                portModesCurr->serdesSpeed = _27_34375G;
                fecType = AP_ST_HCD_FEC_RES_RS;
                portFecType = RS_FEC;
            }
            else if (portModesCurr != NULL && AP_CTRL_40GBase_KR4_GET(anSm->capability))/* 42G */
            {
                if( AP_CTRL_SPECIAL_SPEED_GET(anSm->options) == 1)
                {
                    portModesCurr->serdesSpeed = _10_9375G;
                }
                else if( AP_CTRL_SPECIAL_SPEED_GET(anSm->options) == 2)
                {
                    portModesCurr->serdesSpeed = _12_1875G;
                }
                else
                {
                    return GT_FAIL;
                }

                if (AP_CTRL_FEC_REQ_GET(anSm->options))
                {
                    fecType = AP_ST_HCD_FEC_RES_FC;
                    portFecType = FC_FEC;
                }
                else
                {
                    fecType = AP_ST_HCD_FEC_RES_NONE;
                    portFecType = FEC_OFF;
                }
            }
            else if (portModesCurr != NULL && AP_CTRL_50GBase_KR2_CONSORTIUM_GET(anSm->capability))/* 53G */
            {
                portModesCurr->serdesSpeed = _27_34375G;
                fecType = AP_CTRL_FEC_ADVANCED_REQ_GET(anSm->options);

                if(fecType == AP_ST_HCD_FEC_RES_RS)
                {
                    portFecType = RS_FEC;
                }
                else
                {
                    portFecType = FEC_OFF;
                }
            }
            else if (portModesCurr != NULL && AP_CTRL_100GBase_KR2_GET(anSm->capability)) /* 102G_R2 */
            {
                portModesCurr->serdesSpeed = _27_1875_PAM4;
                fecType = AP_ST_HCD_FEC_RES_RS;
                portFecType = RS_FEC_544_514;
            }
        }
        else
        {
            if (AP_CTRL_SPECIAL_SPEED_GET(anSm->options) && apPortMode == Port_400GBase_KR8)/*424*/
            {
                portModesCurr->serdesSpeed = _28_125G_PAM4;
            }
        }

        AP_ST_HCD_FEC_RES_SET(anSm->hcdStatus, fecType)

        if ((anSm->portMode != _1000Base_X) && (anSm->portMode != _10GBase_KX4))
        {
                hwsPortModeParamsSetFec(anDevNum, anPortGroup,(GT_U32)anSm->portNum, anSm->portMode, portFecType); /* update the elements data base since curPortParams is only a buffer*/
        }
        mvHwsHWAccessLock(anDevNum, MV_SEMA_PM_CM3);
        mvHwsMtipExtFecClockEnable(anDevNum, anSm->portNum, anSm->portMode, portFecType, GT_TRUE);
        mvHwsHWAccessUnlock(anDevNum, MV_SEMA_PM_CM3);

        mvHwsMtipExtFecTypeSet(anDevNum, anPortGroup, anSm->portNum, anSm->portMode, portFecType);


         AP_ST_HCD_FOUND_SET(anSm->hcdStatus,1);
         mvHwsAvagoSerdesManualInterconnectDBGet(anDevNum, anPortGroup, anSm->portNum, MV_HWS_SERDES_NUM(portModesCurr->activeLanesList[0]), &profile);

         /* using auto profile - need to overwrite default rxtx parameters */
         if ((HWS_PAM4_MODE_CHECK(anSm->portMode))||( profile != 2 ))
         {
            /* Send the HCD to the host for Pizza Configuration*/
            doorbellIntrAddEvent(AP_DOORBELL_EVENT_PORT_802_3_AP(anSm->portNum));
            mvAnPortRemoteFaultEnable(portIndex,GT_TRUE);
         }

        /* start link check timer for serdes configuration threshold.
           Set this timer also in auto KR (when the timers should be disable)
           because we assert link status before we checked that the training
           done and disable the internal link inhebit timer by doing it */
         mvAnResetStatsTimestamp(anSm->portNum);
         timestampTime2[anSm->portNum] = mvPortCtrlCurrentTs();
         mvPortCtrlThresholdSet(anIntrop->apLinkDuration, &(anTimer->linkThreshold));
         anStats->timestampTime = PORT_CTRL_TIMER_DEFAULT/*mvPortCtrlCurrentTs()*/;

        return GT_OK;
    }

    if (anSm->status == AN_PORT_RESOLUTION_IN_PROGRESS       ||
        anSm->status == AN_PORT_AN_RESOLUTION_CHK_CONSORTIUM ||
        anSm->status == AN_PORT_AN_RESOLUTION_CHK_ACK2       ||
        anSm->status == AN_PORT_AN_RESOLUTION_NO_MORE_PAGES)
    {
        /* Check AN threshold */
        if (!(mvPortCtrlThresholdCheck(anTimer->abilityThreshold)))
        {
            return GT_OK;
        }
        /* check if resulotion has been found:
        1. check 0_core_status[22:21] : link control*/
        if ((mvAnSerdesStatus(serdesNum,AN_SERDES_AN_READ_LINK_CONTROL,&linkControlResulotionResult)) != GT_OK)
        {
           /* AN_PRINT_MAC_ERR(("mvAnSerdesStatus: port:%d failed to read AN avago SM status %d\n",portIndex, AN_SERDES_AN_READ_LINK_CONTROL));*/
        }
#ifdef CONSORTIUM_SUPPORT
        if (AP_CTRL_ADV_CONSORTIUM_GET(anSm->capability))
        {
            CHECK_STATUS(mvAnConsortiumPortResolution(portIndex,&resolutionDone));
            if ((anSm->status == AN_PORT_AN_RESOLUTION_NO_MORE_PAGES) && (resolutionDone == GT_TRUE))
            {
                linkControlResulotionResult = LINK_CONTROL_ENABLE;
            }
        }
#endif
        if (linkControlResulotionResult == LINK_CONTROL_ENABLE)
        {
            /* 2. check o_core_status 20: link good check*/
            if ((mvAnSerdesStatus(serdesNum,AN_SERDES_AN_GOOD,&AnGoodResult)) != GT_OK)
            {
                /*AN_PRINT_MAC_ERR(("mvAnSerdesStatus: port:%d failed to read AN avago SM status %d\n",portIndex, AN_SERDES_AN_GOOD));*/
            }
            else if (AnGoodResult == 0x1)/* HCD found*/
            {
                AP_ST_HCD_FOUND_SET(anSm->hcdStatus,AnGoodResult);

#ifdef DEBUG_AN
                osPrintf("mvAnPortAutoNeg : HCD FOUND! 0x%8.8x \n",resulotionResult);
#endif
                /* AN state & status update */
                /*anSm->state =  AN_PORT_SM_SERDES_CONFIG_STATE;*/
                AN_PORT_SM_SET_STATE(anSm->portNum, anSm, AN_PORT_SM_SERDES_CONFIG_STATE);
                /*anSm->status = AN_PORT_RESOLUTION_SUCCESS;*/
                AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_RESOLUTION_SUCCESS);

                mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
                /* extract all the relevant parameters for port configuration and training*/
                /* read the HCD*/
                if((mvAnSerdesStatus(serdesNum,AN_SERDES_AN_DATA_OUT,&dataOutResulotionResult)) != GT_OK)
                {
                     /*AN_PRINT_MAC_ERR(("mvAnSerdesStatus: port:%d failed to read AN avago SM status %d\n",portIndex, AN_SERDES_AN_READ_HCD));*/
                }
                else
                {
                    /* When using both consortium and advertise ap speeds, the state machine will use priority to chose which method to use.
                       The highest priority will be the method*/
                    hcdStatus = (dataOutResulotionResult & 0x01f);
#ifdef CONSORTIUM_SUPPORT
                    switch (hcdStatus)
                    {
                        case 0x11: /*200GBASE-KR4/CR4*/
                            priority = 200;
                            break;
                        case 0x10: /*100GBASE-KR2/CR2*/
                        case 0x9: /*100GBASE-CR4*/
                        case 0x8: /*100GBASE-KR4*/
                        case 0x6: /*100GBASE-KP4*/
                            priority = 100;
                            break;
                        case 0xe: /*50GBASE-KR/CR*/
                            priority = 50;
                            break;
                        case 0xb: /*25GBASE-KRCR*/
                        case 0xa: /*25GBASE-KRCR-S*/
                            priority = 25;
                            break;
                        case 0x2: /*10GBASE-KR*/
                        case 0x1: /*10GBASE-KX4*/
                            priority = 10;
                            break;
                        default:
                            priority = 0;
                    }

                    if (AP_CTRL_ADV_CONSORTIUM_GET(anSm->capability))
                    {
                        apPortMode = AP_ST_HCD_TYPE_GET(anSm->hcdStatus);
                        switch (apPortMode)
                        {
                            case Port_400GBase_KR8:
                            case Port_400GBase_CR8:
                                priorityConsortium = 400;
                                break;
                            case Port_25GBASE_KR_C:
                            case Port_25GBASE_CR_C:
                                priorityConsortium = 25;
                                break;
                            case Port_50GBASE_KR2_C:
                            case Port_50GBASE_CR2_C:
                                priorityConsortium = 50;
                                break;
                            case Port_40GBase_KR2:
                                priorityConsortium = 40;
                                break;
                            default:
                                priorityConsortium = 0;
                        }
                        if (priority < priorityConsortium)
                        {
                            hcdTypeIsConsortium  = GT_TRUE;
                        }
                    }

                    if (!(hcdTypeIsConsortium))
                    {
#endif /*#ifdef CONSORTIUM_SUPPORT*/
                        CHECK_STATUS(mvHwsAvagoAnConvertHcdToApPortMode(hcdStatus, anSm, &apPortMode));
                        AP_ST_HCD_TYPE_SET(anSm->hcdStatus, apPortMode)
#ifdef CONSORTIUM_SUPPORT
                    }
                    else /*consortium*/
                    {
                        apPortMode = AP_ST_HCD_TYPE_GET(anSm->hcdStatus);
                    }
#endif
                    anSm->portMode = mvHwsApConvertPortMode(apPortMode);

                    /* AP Port mode parameters update */
                    if(mvHwsApSetPortParameters(anSm->portNum, apPortMode) != GT_OK)
                    {
                        mvPcPrintf("Error, AP , Port-%d Params set Failed\n", anSm->portNum);
                        return GT_FAIL;
                    }

                    if (AP_CTRL_SPECIAL_SPEED_GET(anSm->options) && apPortMode == Port_400GBase_KR8)
                    {
                        portModesCurr = hwsPortModeParamsGet(anDevNum, anPortGroup, anSm->portNum, anSm->portMode,NULL);
                        if(portModesCurr != NULL)
                            portModesCurr->serdesSpeed = _28_125G_PAM4;
                    }
#ifdef CONSORTIUM_SUPPORT
                    if (!(hcdTypeIsConsortium))
                    {
#endif
                        /* Read FC FEC enable*/
                        fcFecEnable = (dataOutResulotionResult & 0x0100)>> 8;

                        /* Read RS FEC enable*/
                        rsFecEnable = (dataOutResulotionResult & 0x0080)>> 7;
                        if (HWS_PAM4_MODE_CHECK(anSm->portMode)|| (_200GBase_CR8 == anSm->portMode) ||
                            (_200GBase_KR8 == anSm->portMode) || (HWS_100G_R4_MODE_CHECK(anSm->portMode)))
                        {
                            rsFecEnable = 1;
                        }

                        if (fcFecEnable && rsFecEnable)
                        {
                            /* start AN again */
                            mvPortCtrlAnPortFail("Error, Port AN mvAnPortAutoNeg, Port-%d AN Failed,"
                                                 " both RS fec and FC fec enable after resulotion.\n",
                                                 portIndex,  AN_PORT_RESOLUTION_FAILURE);
                            return GT_OK;
                        }
                        else if (fcFecEnable)
                        {
                            fecType = AP_ST_HCD_FEC_RES_FC;
                            portFecType = FC_FEC;
                        }
                        else if (rsFecEnable)
                        {
                            fecType = AP_ST_HCD_FEC_RES_RS;
                            if (HWS_PAM4_MODE_CHECK(anSm->portMode)|| (_200GBase_CR8 == anSm->portMode) || (_200GBase_KR8 == anSm->portMode) )
                                portFecType = RS_FEC_544_514;
                            else
                                portFecType = RS_FEC;
                        }
                        else
                        {
                            fecType = AP_ST_HCD_FEC_RES_NONE;
                            portFecType = FEC_OFF;
                        }

                        AP_ST_HCD_FEC_RES_SET(anSm->hcdStatus, fecType)
#ifdef CONSORTIUM_SUPPORT
                    }
                    else /* consortium*/
                    {
                        fecType = AP_ST_HCD_FEC_RES_GET(anSm->hcdStatus);

                        if (fecType == AP_ST_HCD_FEC_RES_RS)
                        {
                            if ((apPortMode == Port_400GBase_KR8) ||
                                (apPortMode == Port_400GBase_CR8))
                            {
                                portFecType = RS_FEC_544_514;
                            }
                            else
                            {
                                portFecType = RS_FEC;
                            }
                        }
                        else if (fecType == AP_ST_HCD_FEC_RES_FC)
                        {
                            portFecType = FC_FEC;
                        }
                        else
                        {
                            if ((apPortMode == Port_400GBase_KR8) ||
                                (apPortMode == Port_400GBase_CR8))
                            {
                                portFecType = RS_FEC_544_514;
                                AP_ST_HCD_FEC_RES_SET(anSm->hcdStatus, AP_ST_HCD_FEC_RES_RS)
                            }
                            else
                            {
                                portFecType = FEC_OFF;
                            }
                        }
                    }
#endif
                    if ((anSm->portMode != _1000Base_X) && (anSm->portMode != _10GBase_KX4))
                    {
                        hwsPortModeParamsSetFec(anDevNum, anPortGroup,(GT_U32)anSm->portNum, anSm->portMode, portFecType); /* update the elements data base since curPortParams is only a buffer*/
                    }

                    /* mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(anSm->portNum,0, portFecType, anSm->portMode ));*/
                    /* mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG_SERDES(anSm->portNum,LOG_DEBUG_SERDES_RESOLUTION, (portFecType<<8 | anSm->portMode)));*/

                    mvHwsHWAccessLock(anDevNum, MV_SEMA_PM_CM3);
                    mvHwsMtipExtFecClockEnable(anDevNum, anSm->portNum, anSm->portMode, portFecType, GT_TRUE);
                    mvHwsHWAccessUnlock(anDevNum, MV_SEMA_PM_CM3);
                    mvHwsMtipExtFecTypeSet(anDevNum, anPortGroup, anSm->portNum, anSm->portMode, portFecType);

                    /* start link check timer for serdes configuration threshold.
                       Set this timer also in auto KR (when the timers should be disable)
                       because we assert link status before we checked that the training
                       done and disable the internal link inhebit timer by doing it */
                    mvPortCtrlThresholdSet(anIntrop->apLinkDuration, &(anTimer->linkThreshold));
                    /* Send the HCD to the host for Pizza Configuration*/
                    doorbellIntrAddEvent(AP_DOORBELL_EVENT_PORT_802_3_AP(anSm->portNum));
                    mvAnPortRemoteFaultEnable(portIndex,GT_TRUE);

                            /* AP Timing measurment */
                    anStats->hcdResoultionTime = mvPortCtrlCurrentTs() - anStats->timestampTime;
                    anStats->timestampTime = mvPortCtrlCurrentTs();

                    return GT_OK;
                }
            }
        }
        /* resulotion still in progress*/
        anTimer->abilityCount++;

        /* Validate AN max interval, in case crossed start AN again */
        if(anTimer->abilityCount >= anIntrop->abilityMaxInterval)
        {
            /* AN state & status update */
            /*anSm->status = AN_PORT_RESOLUTION_FAILURE;*/
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_RESOLUTION_FAILURE);

            AN_PRINT_MAC_ERR(("Resolution portIndex:%d failed status:0x%x\n",portIndex, anSm->status));
            mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

            mvPortCtrlAnPortFail("Error, Port AN mvAnPortAutoNeg, Port-%d AN Failed - resulotion timeout.\n",
                                  portIndex,  AN_PORT_RESOLUTION_FAILURE);

            /* start AN again */
            /*anSm->status = AN_PORT_START_EXECUTE;*/
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_START_EXECUTE);

            /* set random timer - each port will start AP in a different time*/
            mvPortCtrlThresholdSet(((portIndex%8) * 100), &(anTimer->abilityThreshold));
            return GT_OK;
        }
        /* Ability timer restart */
        mvPortCtrlThresholdSet(anIntrop->abilityDuration, &(anTimer->abilityThreshold));
        return GT_OK;
    }

    if (anSm->status == AN_PORT_INIT_SUCCESS)
    {
#ifdef RAVEN_DEV_SUPPORT
        /*anSm->status = AN_PORT_SERDES_POWER_UP;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_POWER_UP);
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
        rc = mvApSerdesPowerUp(serdesNum, GT_TRUE, anSm->portNum, MV_HWS_AVAGO_SERDES_POWER_LAST_PHASE);
        if (rc != GT_OK)
        {
            /* if Serdes power-up failed, power it down, exit and next iteration
               the Serdes will be powered-up again
            */
            /*anSm->status = AN_PORT_SERDES_INIT_FAILURE;*/
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_INIT_FAILURE);
            mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
            mvApSerdesPowerUp(serdesNum, GT_FALSE, anSm->portNum, MV_HWS_AVAGO_SERDES_POWER_FIRST_PHASE);
            /* return to the AN_PORT_START_EXECUTE to try serdes power up again next time */
            /*anSm->status = AN_PORT_START_EXECUTE;*/
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_START_EXECUTE);

            /* set random timer - each port will start AP in a different time*/
            mvPortCtrlThresholdSet(((portIndex%8) * 100), &(anTimer->abilityThreshold));
            return GT_OK;
        }
#endif
        /*anSm->status = AN_PORT_SET_AN_PARAMETERS;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SET_AN_PARAMETERS);

        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
        /* AP register dump */
        mvPortCtrlDbgAllRegsDump(portIndex, serdesNum, AN_PORT_SM_AN_STATE);
        CHECK_STATUS(mvAnPortAutoNegAnConfig(portIndex));
        /*anSm->status = AN_PORT_RESOLUTION_IN_PROGRESS;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_RESOLUTION_IN_PROGRESS);

        mvPortCtrlThresholdSet(anIntrop->abilityDuration, &(anTimer->abilityThreshold)); /* 5 ms threshold*/
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

    }

    /* configure AN parameters.
       configure DME parameters.
       start AN*/
    if (anSm->status == AN_PORT_START_EXECUTE)
    {
        /* Check AN threshold */
        if ((anTimer->abilityThreshold != PORT_CTRL_TIMER_DEFAULT) && (!mvPortCtrlThresholdCheck(anTimer->abilityThreshold)))
        {
            return GT_OK;
        }
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
        /* reset timestamps */
        mvAnResetStatsTimestamp(portIndex);
        anStats->timestampTime = mvPortCtrlCurrentTs();

        mvAnResetStatus(portIndex);
        /* reset AN timers*/
        mvAnResetTimer(portIndex);
        AN_PRINT_MAC(("Calling mvApSerdesPowerUp FALSE portIndex:%d serdes %d\n",anSm->portNum, serdesNum));
        mvApSerdesPowerUp(serdesNum, GT_FALSE, anSm->portNum, MV_HWS_AVAGO_SERDES_POWER_FIRST_PHASE);

        AN_CTRL_AUTO_KR_ENABLE_SET(anSm->options, autoKrAndEnableAvagoTimers);
        AN_CTRL_DISABLE_LINK_TIMER_SET(anSm->options, (!autoKrAndEnableAvagoTimers));

        /* Lane power up in 1.25*/
        /*Wait for SerDes calibration to complete*/
        /*Wait for SerDes o_tx_rdy == 1 and o_rx_rdy == 1*/
        /*AN_PRINT_MAC(("Calling mvApSerdesPowerUp TRUE portIndex:%d serdes %d\n",anSm->portNum, serdesNum));*/
        rc = mvApSerdesPowerUp(serdesNum, GT_TRUE, anSm->portNum, MV_HWS_AVAGO_SERDES_POWER_FIRST_PHASE);
        AN_PRINT_MAC(("Calling mvApSerdesPowerUp portIndex:%d serdes %d status:0x%x\n",anSm->portNum, serdesNum, rc));
        if ( rc != GT_OK)
        {
            /* if Serdes power-up failed, power it down, exit and next iteration
               the Serdes will be powered-up again
            */
            /*anSm->status = AN_PORT_SERDES_INIT_FAILURE;*/
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_INIT_FAILURE);

            mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
            mvApSerdesPowerUp(serdesNum, GT_FALSE, anSm->portNum, MV_HWS_AVAGO_SERDES_POWER_FIRST_PHASE);
            /* return to the AN_PORT_START_EXECUTE to try serdes power up again next time */
            /*anSm->status = AN_PORT_START_EXECUTE;*/
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_START_EXECUTE);

            /* set random timer - each port will start AP in a different time*/
            mvPortCtrlThresholdSet(((portIndex%8) * 100), &(anTimer->abilityThreshold));
            return GT_OK;
        }

        /* AP state & status update */
        /*anSm->status = AN_PORT_INIT_SUCCESS;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_INIT_SUCCESS);
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
    }


    return GT_OK;
}



/**
* @internal mvPortCtrlAnPortFcSet function
* @endinternal
*
* @brief   Execute Flow Control relosultion configuration in
*          mvPortCtrlApPortPendExec Disable until get the
*          relevant information from avago
*
* @param[in] portIndex           - index of port in local AP DB
* @param[in] portMode            - HWS port mode
**/
static void mvPortCtrlAnPortFcSet
(
    IN  GT_U32                          portIndex,
    IN  MV_HWS_PORT_STANDARD            portMode
)
{
#if 0
    MV_HWS_PORT_FLOW_CONTROL_ENT    fcState;
   /* GT_U32                          fcRxPauseEn, fcTxPauseEn;*/
    MV_HWS_AN_SM_INFO              *anSm      = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);

    mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, PORT_SM_FC_STATE_SET_IN_PROGRESS, anSm->portNum));
    fcRxPauseEn = AP_ST_HCD_FC_RX_RES_GET(anSm->hcdStatus);
    fcTxPauseEn = AP_ST_HCD_FC_TX_RES_GET(anSm->hcdStatus);

    if (fcRxPauseEn && fcTxPauseEn)
    {
        fcState = MV_HWS_PORT_FLOW_CONTROL_RX_TX_E;
    }
    else if (fcRxPauseEn)
    {
        fcState = MV_HWS_PORT_FLOW_CONTROL_RX_ONLY_E;
    }
    else if (fcTxPauseEn)
    {
        fcState = MV_HWS_PORT_FLOW_CONTROL_TX_ONLY_E;
    }
    else
    {
        fcState = MV_HWS_PORT_FLOW_CONTROL_DISABLE_E;
    }

    fcState = MV_HWS_PORT_FLOW_CONTROL_DISABLE_E;

    if (mvHwsPortFlowControlStateSet(anDevNum, /* devNum */
                                     anPortGroup, /* portGroup */
                                     (GT_U32)anSm->portNum,
                                     portMode,
                                     fcState) != GT_OK)
    {
        mvPortCtrlAnPortFail("Error, Port AN, Port-%d FC state set Failed\n",
                             portIndex, AN_PORT_FC_STATE_SET_FAILURE);
        return;
    }

    mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, AN_PORT_FC_STATE_SET_SUCCESS, anSm->portNum));
#endif

    return;
}

/**
* @internal mvAnPortSerdesConfigPcsFecMac function
* @endinternal
*
* @brief   configure PCS and MAC
*
* @param[in] portIndex           - index of port in local AP DB
*
*/
static void mvAnPortSerdesConfigPcsFecMac
(
    IN  GT_U8      portIndex
)
{

    MV_HWS_AN_SM_INFO               *anSm  = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    MV_HWS_PORT_STANDARD            portMode = anSm->portMode;
    GT_STATUS                       rc;
    GT_U32 regData, regMask, localPortIndex, i;
    GT_BOOL macEnable;

    if (GT_OK != hwsPortModeParamsGetToBuffer(anDevNum,anPortGroup, (GT_U32)anSm->portNum, portMode, &curPortParams))
    {
        mvPortCtrlAnPortFail("Error, Port AN, Port-%d hwsPortModeParamsGet return NULL\n",
                             portIndex, AN_PORT_SERDES_CONFIGURATION_FAILURE);
        return;
    }
    mvHwsMtipExtMacResetRelease(anDevNum, anSm->portNum, portMode, GT_TRUE);

    rc = mvApPolarityCfg((GT_U32)anSm->portNum, portMode, anSm->polarityVector);
    if (rc != GT_OK)
    {
        mvPortCtrlAnPortFail("Error, Port AN, Port-%d Set polarity Failed\n",
                             portIndex, AN_PORT_SERDES_CONFIGURATION_FAILURE);
        return;
    }

    /*anSm->status = AN_PORT_MAC_PCS_CONFIG_IN_PROGRESS;*/
    AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_MAC_PCS_CONFIG_IN_PROGRESS);
    mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

    /*protect mac clock enable*/
    mvHwsHWAccessLock(anDevNum, MV_SEMA_PM_CM3);
    /* read mac clock */
    rc = mvHwsMtipExtMacClockEnableGet(anDevNum,anSm->portNum, portMode, &macEnable);
    if (( rc == GT_OK ) && (macEnable == GT_FALSE)) {
        rc = GT_FAIL;
        /*mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(anSm->portNum, 7, 77, 78 ));*/
        /*mvHwsHWAccessUnlock(anDevNum, MV_SEMA_PM_CM3);
        mvPortCtrlAnPortFail("Error, Port AN, Port-%d mac clk enable Failed\n",
                             portIndex,  AN_PORT_MAC_PCS_CONFIG_FAILURE);
        return;*/
        mvHwsMtipExtPcsClockEnable(anDevNum,  anSm->portNum, portMode, GT_TRUE);
        mvHwsMtipExtMacClockEnable(anDevNum,  anSm->portNum, portMode, GT_TRUE);

    }

    /* Configure MAC/PCS */
    if (mvHwsPortModeCfg(anDevNum,anPortGroup,anSm->portNum, portMode, NULL) != GT_OK)
    {
        mvHwsHWAccessUnlock(anDevNum, MV_SEMA_PM_CM3);
        mvPortCtrlAnPortFail("Error, Port AN, Port-%d mvHwsPortModeCfg Failed\n",
                             portIndex, AN_PORT_MAC_PCS_CONFIG_FAILURE);
                        return;
    }
    mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
    /* Un-Reset the port */
    if (mvHwsPortStartCfg(anDevNum,anPortGroup, anSm->portNum, portMode) != GT_OK)
    {
        mvHwsHWAccessUnlock(anDevNum, MV_SEMA_PM_CM3);
        mvPortCtrlAnPortFail("Error, Port AN, Port-%d mvHwsPortStartCfg Failed\n",
                             portIndex,  AN_PORT_MAC_PCS_CONFIG_FAILURE);
        return;
    }

    mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
    /* Un-Reset mtip */
    if (!mvHwsMtipIsReducedPort(anDevNum, anSm->portNum))
    {

        /* bit [7:0] v = m_RAL.mtip_ext_units_RegFile.Global_Reset_Control.gc_sd_tx_reset_.get();*/
        /* for UNIT_MTI_EXT we check only port 0 or 8 */
        regData = 0;
        localPortIndex = anSm->portNum %8;
        regMask = 0;
        /* on each related serdes */
        for (i = 0; i < curPortParams.numOfActLanes; i++)
        {
            /* set the rx[0-7] and the tx[8-15] bits */
            regMask |= ((1<<(localPortIndex+i) )| (1<<(8+localPortIndex+i)));
        }
        regData =  regMask;
        genUnitRegisterSet(anDevNum, 0, MTI_EXT_UNIT, anSm->portNum,
                                    MTIP_EXT_GLOBAL_RESET_CONTROL, regData, regMask);
    }
    else
    {
        regData = 0x3;
        regMask = 0x3;
        genUnitRegisterSet(anDevNum, 0, MTI_CPU_EXT_UNIT, anSm->portNum,
                                     MTIP_CPU_EXT_PORT_RESET, regData, regMask);
    }
    mvHwsHWAccessUnlock(anDevNum, MV_SEMA_PM_CM3);


    mvPortCtrlAnPortFcSet(portIndex, portMode);

    /*anSm->status = AN_PORT_MAC_PCS_CONFIG_SUCCESS;*/
    AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_MAC_PCS_CONFIG_SUCCESS);

    mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

}

/**
* @internal mvAnPortSerdesCompleteAn function
* @endinternal
*
* @brief   complete AN process - read the avago SM status and
*          check the link control and training status check the
*          PCS lionk status
*
* @param[in] portIndex           - index of port in local AP DB
*
*/
static void mvAnPortSerdesCompleteAn
(
    GT_U8      portIndex
)
{
    GT_U32                          oCoreStat0to15Result = 0;
    GT_U32                          linkControlResult    = 0;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    MV_HWS_AN_SM_INFO               *anSm    = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_PORT_STANDARD            portMode;
    GT_U16                          serdesIndex;
    GT_BOOL                         pcsLinkStatus;
    GT_STATUS                       rc;
    /*MV_HWS_AN_SM_STATS              *anStats  = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].stats);*/

    portMode = anSm->portMode;

    /*check training status*/
    /*1. Check o_core_status [15:0] for all ports serdeses*/
    if (GT_OK != hwsPortModeParamsGetToBuffer(anDevNum, anPortGroup, (GT_U32)anSm->portNum, portMode, &curPortParams))
    {
        mvPortCtrlAnPortFail("Error, Port AN, Port-%d mvAnPortSerdesCompleteAn Failed\n",
                             portIndex, AN_PORT_SERDES_CONFIGURATION_FAILURE);
        return;
    }

    if (!AN_CTRL_RX_TRAIN_GET(anSm->options))
    {
        if ((portMode != _1000Base_X) && (portMode != _10GBase_KX4))
        {
            for (serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++)
            {
                if ((mvAnSerdesStatus(curPortParams.activeLanesList[serdesIndex], AN_SERDES_AN_READ_TRAINING_RESULTS, &oCoreStat0to15Result)) != GT_OK)
                {
                    AN_PRINT_MAC_ERR(("mvAnSerdesStatus: serdes:%d failed to read AN avago SM status %d\n",curPortParams.activeLanesList[serdesIndex], AN_SERDES_AN_READ_TRAINING_RESULTS));
                }
                /* training failed, o_core_status[0] = 1*/
                if ((oCoreStat0to15Result & 0x1) == 0x1)
                {
                    /* training failed*/
                    /*anSm->status = AN_PORT_SERDES_TRAINING_FAILURE;*/
                    AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_TRAINING_FAILURE);

                    mvPortCtrlAnPortFail("Error, Port AN mvAnPortSerdesCompleteAn, Serdes-%d training Failed\n",
                                    curPortParams.activeLanesList[serdesIndex],  AN_PORT_SERDES_TRAINING_FAILURE);
                }
            }
        }

        /* successful KR training completion , all port serdeses have o_core_status[0:15] = 0x34*/
        /*2. check 0_core_status[22:21]: link control*/
        for (serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++)
        {
            if((mvAnSerdesStatus(curPortParams.activeLanesList[serdesIndex],AN_SERDES_AN_READ_LINK_CONTROL,&linkControlResult)) != GT_OK)
            {
                /*AN_PRINT_MAC_ERR(("mvAnSerdesStatus: serdes:%d failed to read AN avago SM status %d\n",curPortParams.activeLanesList[serdesIndex], AN_SERDES_AN_READ_LINK_CONTROL));*/
            }
            if (linkControlResult != LINK_CONTROL_ENABLE)
            {
                /*anSm->status = AN_PORT_AN_COMPLETE_FAILURE;*/
                AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_AN_COMPLETE_FAILURE);

                mvPortCtrlAnPortFail("Error, Port AN, Serdes-%d mvAnPortSerdesCompleteAn Failed\n",
                                 curPortParams.activeLanesList[serdesIndex], AN_PORT_SERDES_CONFIGURATION_FAILURE);
                return;
            }
        }
    }
    /* PCS link status from*/
    rc = prvPortCtrlApEngLinkUpCheck(anSm->portNum,
                                     anSm->portMode,
                                     AP_ST_HCD_FEC_RES_GET(anSm->hcdStatus),
                                     &pcsLinkStatus);

    if (rc != GT_OK)
    {
        /*anSm->status = AN_PORT_AN_COMPLETE_FAILURE;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_AN_COMPLETE_FAILURE);

        mvPortCtrlAnPortFail("Error, Port AN, Port-%d mvAnPortSerdesCompleteAn can't read PCS status \n",
                             portIndex, AN_PORT_SERDES_CONFIGURATION_FAILURE);
        return;
    }
    if (pcsLinkStatus == GT_TRUE)
    {
        /*anSm->status = AN_PORT_AN_COMPLETE_SUCCESS;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_AN_COMPLETE_SUCCESS);
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
    }
    else
    {
#if 0
        if ( HWS_PAM4_MODE_CHECK(portMode))
        {
            /*check timer above 3.3 sec or above 6.5 sec send ical*/
            if ((((mvPortCtrlCurrentTs() - anStats->timestampTime) > 4200) && (status2[anSm->portNum] == 1)) ||
                (((mvPortCtrlCurrentTs() - anStats->timestampTime) > 7000) && (status2[anSm->portNum] == 2)))
            {
                /* send ical*/
                for (serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++)
                {
                    mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), 0xA, 0x01, NULL);
                }
                mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG_SERDES(anSm->portNum, LOG_DEBUG_SERDES_PCAL, status2[anSm->portNum]));
                status2[anSm->portNum]++ ;
            }
        }

        /* Need some time after training in order to get link in PCS - so no error state for a while.
           If there is some actual error with link - 500ms timer will fail */

        /*anSm->status = AN_PORT_AN_COMPLETE_FAILURE;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_AN_COMPLETE_FAILURE);

        mvPortCtrlAnPortFail("Error, Port AN, Port-%d mvAnPortSerdesCompleteAn Failed\n",
                             portIndex, AN_PORT_SERDES_CONFIGURATION_FAILURE);
#endif
        return;
    }

    if (AN_CTRL_AUTO_KR_ENABLE_GET(anSm->options) == GT_FALSE)
    {
        /*anSm->status = AN_PORT_AN_ASSERT_LINK_IN_PROGRESS;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_AN_ASSERT_LINK_IN_PROGRESS);

        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
    }

    return;
}

/**
* @internal mvAnPortSerdesConfigAssertLinkStatus function
* @endinternal
*
* @brief  assert link status after trainig done and all the
*         status are ok. in case of auto KR we assert link
*         status before training to invalidate the avago SM
*         timers.
*
* @param[in] portIndex           - index of port in local AP DB
* */
static GT_STATUS mvAnPortSerdesConfigAssertLinkStatus
(
    IN  GT_U8      portIndex
)
{

    MV_HWS_AN_SM_INFO *anSm      = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    GT_U32             hcd       = AP_ST_HCD_TYPE_GET(anSm->hcdStatus);
    GT_U32             serdesNum = AP_CTRL_LANE_GET(anSm->ifNum);
    MV_HWS_AN_SM_STATS *anStats = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].stats);

    if (anSm->status == AN_PORT_AN_COMPLETE_IN_PROGRESS)
    {
        mvAnPortSerdesCompleteAn(portIndex);
    }
    if (anSm->status == AN_PORT_AN_ASSERT_LINK_IN_PROGRESS)
    {
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x107, (0x8000 | hcd), NULL));
        /*anSm->status = AN_PORT_AN_ASSERT_LINK_SUCCESS;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_AN_ASSERT_LINK_SUCCESS);
        anSm->state  = AN_PORT_SM_LINK_STATUS_STATE;
        if (anStats->linkUpTime == 0)
        {
            anStats->linkUpTime = mvPortCtrlCurrentTs() - anStats->timestampTime;
        }
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
    }

    return GT_OK;
}

static GT_BOOL  mvAnPortSerdesQaulifier
(
    IN  GT_U32      serdesNum
)
{
    /* Read DFEtap2 */
    GT_32 dfeTap2 = 0, level3e;
    mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x2c, 0xb02, &dfeTap2);
    /*mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(serdesNum,4, ((dfeTap2>>8)&0xFF), (dfeTap2&0xFF)));*/
    /*mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG_SERDES(serdesNum, LOG_DEBUG_SERDES_TAP2, (dfeTap2&0xFFFF)));*/

    /* read level3e*/
    mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x2c, 0x1f06, &level3e);
    /*mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(serdesNum,5, ((level3e>>8)&0xFF), (level3e&0xFF)));
    mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG_SERDES(serdesNum, LOG_DEBUG_SERDES_EO, (level3e&0xFFFF)));*/

    if ((dfeTap2 > 27) || (dfeTap2 <-27) || (level3e < 30))
    {
        return GT_FALSE;
    }
    else
    {
        return GT_TRUE;
    }
}


/**
* @internal mvAnPortSerdesConfigCheckSystemConfig function
* @endinternal
*
* @brief  check system configuration.
*
* @param[in] portIndex           - index of port in local AP DB
* */
static void mvAnPortSerdesConfigCheckSystemConfig
(
    IN  GT_U8      portIndex
)
{
    MV_HWS_AN_SM_INFO  *anSm    = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_AN_SM_TIMER *anTimer  = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].timer);
    MV_HWS_PORT_STANDARD           portMode;
/*    GT_U32 delay;*/

    portMode = anSm->portMode;

#if 0
    if ((portMode == _1000Base_X) || (portMode == _10GBase_KX4))
    {
        delay = PORT_CTRL_SYSTEM_CONFIG_DURATION_1000BASEX_10GBase_KX4;
    }
    else
    {
        delay = PORT_CTRL_SYSTEM_CONFIG_DURATION;
    }
#endif
    /* Check System Config */
    if (anSm->status == AN_PORT_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK)
    {
        if ( mvHwsMiMode )
        {
            /*anSm->status = AN_PORT_SERDES_TRAINING_END_SYS_ACK_RCV;*/
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_TRAINING_END_SYS_ACK_RCV);

            mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
        } else {
            if ((anTimer->sysCfStatus == PORT_SM_SERDES_SYSTEM_VALID) &&
                 ((portMode == anTimer->sysCfMode) || ((AP_CTRL_SPECIAL_SPEED_GET(anSm->options)|| AN_CTRL_RX_TRAIN_GET(anSm->options)) &&  (anTimer->sysCfMode != NON_SUP_MODE))  ||
                  (portMode == _25GBase_KR_S && anTimer->sysCfMode ==_25GBase_KR) ||
                  (portMode == _25GBase_CR_S && anTimer->sysCfMode ==_25GBase_CR)))

            {
                /*anSm->status = AN_PORT_SERDES_TRAINING_END_SYS_ACK_RCV;*/
                AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_TRAINING_END_SYS_ACK_RCV);

                mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
            }
            else
            {
                return;
            }
        }
    }
    /*anSm->status = AN_PORT_AN_COMPLETE_IN_PROGRESS;*/
    AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_AN_COMPLETE_IN_PROGRESS);

    mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
    if ((portMode != _1000Base_X) && (portMode != _10GBase_KX4))
    {
        /*skip one loop*/
        mvAnPortSerdesConfigAssertLinkStatus(portIndex);
    }
    return;
}
#if 0
/**
* @internal mvAnPortSerdesConfigAutoKr function
* @endinternal
*
* @brief  execute auto KR on port, the port init and training
*         are automaticly.
*
* @param[in] portIndex           - index of port in local AP DB
* */
static GT_STATUS mvAnPortSerdesConfigAutoKr
(
    IN  GT_U8      portIndex
)
{
    MV_HWS_AN_SM_INFO  *anSm    = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    GT_STATUS rc;
    GT_U32 hcd = AP_ST_HCD_TYPE_GET(anSm->hcdStatus);
    GT_U32 serdesNum = AP_CTRL_LANE_GET(anSm->ifNum);

    if (anSm->status == AN_PORT_RESOLUTION_SUCCESS)
    {
        mvAnPortSerdesConfigPcsFecMac(portIndex);
        /* We do assert link status here Although the training in progress
           because we want to disable the link fail inhabit timer
           so in case of failure we will do the restart and not the avago SM
           because we need to power up the serdes back to 1.25G.
           we use marvell timers to heck for timeout
         */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x107, (0x1<<hcd), NULL));

        /*anSm->status = AN_PORT_AN_ASSERT_LINK_SUCCESS;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_AN_ASSERT_LINK_SUCCESS);

        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
        /*anSm->status = AN_PORT_AN_COMPLETE_IN_PROGRESS;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_AN_COMPLETE_IN_PROGRESS);

        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
    }

    if (anSm->status == AN_PORT_AN_COMPLETE_IN_PROGRESS)
    {
        rc = mvAnPortSerdesConfigAssertLinkStatus(portIndex);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (anSm->status != AN_PORT_AN_COMPLETE_SUCCESS)
    {
        return GT_OK;
    }
    /* training done*/
    /*wait to pizza ack*/
    /*anSm->status = AN_PORT_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK;*/
    AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK);

    mvAnPortSerdesConfigCheckSystemConfig(portIndex);

    /* start polling on o_core _status[19] : AN complete*/
    anSm->state  = AN_PORT_SM_LINK_STATUS_STATE;
    mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

    return GT_OK;
}
#endif
/**
* @internal mvAnPortLinkFailure function
* @endinternal
*
* @brief   AP Port Link failure execution sequence
*
* @param[in] portIndex           - index of port in local AP DB
*/
static GT_STATUS mvAnPortLinkFailure
(
    IN  GT_U8      portIndex
)
{
    MV_HWS_AN_SM_INFO  *anSm    =  &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_AN_SM_STATS *anStats =  &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].stats);
    MV_HWS_AN_SM_TIMER *anTimer  = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].timer);
    /* This point means 1 of 2:
     1) during port power up, Max link check was exceeded so the port should
        be shut down and the AP process will start over.
     2) link was up but now it's down so AP process should start over (after
        the port will be shut down).
    */

    AN_PRINT_MAC(("mvAnPortLinkFailure portIndex:%d\n",portIndex));

    /* Link Interrupt status reset */
    mvHwsServCpuIntrStatusReset(portIndex);

    /* AP statistics update */
    anStats->linkFailCnt++;
    anStats->linkUpTime  = 0;
    if ((AN_CTRL_RX_TRAIN_GET(anSm->options)) && (anSm->state  < AN_PORT_SM_LINK_STATUS_STATE))
    {
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, AN_PORT_AN_ASSERT_LINK_FAILURE, anSm->portNum));
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_CONFIGURATION_IN_PROGRESS);
        AN_PORT_SM_SET_STATE(anSm->portNum, anSm, AN_PORT_SM_SERDES_CONFIG_STATE);
        mvAnResetStatsTimestamp(anSm->portNum);
        timestampTime2[anSm->portNum] = mvPortCtrlCurrentTs();
        mvPortCtrlThresholdSet(10, &(anTimer->linkThreshold));
        anStats->timestampTime = PORT_CTRL_TIMER_DEFAULT/*mvPortCtrlCurrentTs()*/;


    }
    else
    {
        /* Delete the port */
        if (mvAnPortDelete(portIndex, GT_FALSE) != GT_OK)
        {
            AN_PRINT_MAC_ERR(("mvAnPortLinkFailure: Port %d - Delete AP delete task failed\n",anSm->portNum));
            mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, AN_PORT_PORT_ENABLE_FAIL, anSm->portNum));
           /* return GT_OK;*/
        }
        /* AP state & status update */
        /*anSm->state = AN_PORT_SM_DELETE_STATE;*/
        AN_PORT_SM_SET_STATE(anSm->portNum, anSm, AN_PORT_SM_DELETE_STATE);
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
    }



    return GT_OK;
}

/**
* @internal mvAnPortSerdesConfigTrainingNotReady function
* @endinternal
*
* @brief   get training ready
*
* @param[in] portIndex           - index of port in local AP DB
* */
static GT_STATUS mvAnPortSerdesConfigTrainingNotReady
(
    IN  GT_U8      portIndex
)
{
    MV_HWS_AN_SM_INFO           *anSm     = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_AN_SM_TIMER          *anTimer  = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].timer);
    MV_HWS_AP_DETECT_ITEROP     *anIntrop = &(mvHwsPortCtrlAnPortDetect.introp);
    MV_HWS_AN_SM_STATS          *anStats  = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].stats);
    MV_HWS_AUTO_TUNE_STATUS_RES tuneRes;
    MV_HWS_PORT_STANDARD        portMode;
    MV_HWS_PORT_AUTO_TUNE_MODE trainMode = TRxTuneStart;

    portMode = anSm->portMode;

    tuneRes.txTune = TUNE_FAIL;


    if (AN_CTRL_RX_TRAIN_GET(anSm->options))
    {
        trainMode = RxTrainingOnly;
       /* tuneRes.txTune = TUNE_READY;*/
    }
    /* Execute HWS Training start */
    if (mvHwsPortAutoTuneSet(anDevNum,
                             anPortGroup,
                             (GT_U32)anSm->portNum,
                             portMode,
                             trainMode/*TRxTuneStart*/,
                             &tuneRes) != GT_OK)
    {
        tuneRes.txTune = TUNE_FAIL;
    }

   if (tuneRes.txTune == TUNE_NOT_READY)
   {
       if (printCounter[anSm->portNum] < 4)
       {
           mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
           printCounter[anSm->portNum]++;
       }

   }
   else if ((tuneRes.txTune == TUNE_PASS) || (tuneRes.txTune == TUNE_READY))
   {
       printCounter[anSm->portNum] = 0;
       /*anSm->status = AN_PORT_SERDES_TRAINING_IN_PROGRESS;*/
       AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_TRAINING_IN_PROGRESS);

       mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

       /* for 42G/53G/106G we need to start timer only after we have signal */
       if ((AN_CTRL_SKIP_RES_GET(anSm->options)) || (AP_CTRL_SPECIAL_SPEED_GET(anSm->options) && (AP_CTRL_SPECIAL_SPEED_MODE_GET(anSm->capability))))
       {
           /* start link check timer for serdes configuration threshold.
              Set this timer also in auto KR (when the timers should be disable)
              because we assert link status before we checked that the training
              done and disable the internal link inhebit timer by doing it */
           mvPortCtrlThresholdSet(anIntrop->apLinkDuration, &(anTimer->linkThreshold));
           anStats->timestampTime = mvPortCtrlCurrentTs();
       }
       if (AN_CTRL_RX_TRAIN_GET(anSm->options))
       {
                anStats->hcdResoultionTime = mvPortCtrlCurrentTs() - (GT_U32)timestampTime2[anSm->portNum];
       }
   }
   else
   {
       mvPortCtrlAnPortFail("Error, Port AN mvAnPortSerdesConfig, Port-%d training starting Failed\n",
                            portIndex,  AN_PORT_SERDES_TRAINING_FAILURE);

   }
   return GT_OK;
}

/**
* @internal mvAnPortSerdesConfigTrainingInProgress function
* @endinternal
*
* @brief   execute training on AN port.
*         Execute port Init
*         Execute SERDES Configuration Execute MAC/PCS
*         Configuration Execute training Training configuration
*         Training start Training timeout timer
*
*         linkThreshold = the time from HCD to full link
*         training threshold = the time for TRX training
*
* @param[in] portIndex           - index of port in local AP DB
* */
static void  mvAnPortSerdesConfigTrainingInProgress
(
    IN  GT_U8      portIndex
)
{
    MV_HWS_AN_SM_INFO       *anSm     = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_AN_SM_STATS      *anStats  = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].stats);
    MV_HWS_AP_DETECT_ITEROP *anIntrop = &(mvHwsPortCtrlAnPortDetect.introp);
    MV_HWS_AN_SM_TIMER      *anTimer  = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].timer);
    GT_U32                         tuneResult = TUNE_NOT_COMPLITED;
    MV_HWS_AUTO_TUNE_STATUS_RES    tuneRes;
    MV_HWS_PORT_STANDARD           portMode;
    GT_U32                         i, oCoreStat0to15Result, serdesIndex;
    MV_HWS_PORT_INIT_PARAMS        curPortParams;
    GT_BOOL                        printOnce = GT_TRUE, pcsLinkStatus;
   /* GT_32 oCoreStat0to15Result2 = 0, dfeStatusRead=0;*/
    MV_HWS_AUTO_TUNE_STATUS     rxStatus;
    GT_32                       calStatus, hf;
    GT_STATUS                   rc;
    GT_U32                      profile, cnt;
    GT_32                       tj;
    MV_HWS_PORT_AUTO_TUNE_MODE  trainMode = TRxTuneStatusNonBlocking;
    GT_BOOL                     signalDet;
    portMode = anSm->portMode;
    if (GT_OK != hwsPortModeParamsGetToBuffer(anDevNum,anPortGroup, (GT_U32)anSm->portNum, portMode, &curPortParams))
    {
        mvPortCtrlAnPortFail("Error, Port AN, Port-%d hwsPortModeParamsGet return NULL\n",
                             portIndex, AN_PORT_DELETE_FAILURE);
        return;
    }

    /* read o_core_status for debug */
    if ( anSm->status == AN_PORT_SERDES_TRAINING_IN_PROGRESS)
    {
        cnt = 0;
        for (serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++)
        {
            if ((mvAnSerdesStatus(curPortParams.activeLanesList[serdesIndex], AN_SERDES_AN_READ_TRAINING_RESULTS, &oCoreStat0to15Result)) != GT_OK)
            {
                 /*AN_PRINT_MAC_ERR(("mvAnSerdesStatus: serdes:%d failed to read AN avago SM status %d\n",curPortParams.activeLanesList[serdesIndex], AN_SERDES_AN_READ_TRAINING_RESULTS));*/
            }
            if (printCounter[anSm->portNum + serdesIndex] != oCoreStat0to15Result)
            {
                /* print log for the first time */
                if (printOnce == GT_TRUE)
                {
                    mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
                    printOnce = GT_FALSE;
                }
                if (!AN_CTRL_RX_TRAIN_GET(anSm->options))
                {
                /*mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(curPortParams.activeLanesList[serdesIndex],1, ((oCoreStat0to15Result>>8)&0xFF), (oCoreStat0to15Result&0xFF)));*/
                /*mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG_SERDES(curPortParams.activeLanesList[serdesIndex],LOG_DEBUG_SERDES_OCORE, (oCoreStat0to15Result&0xFFFF)));*/
                }
                printCounter[anSm->portNum + serdesIndex] = oCoreStat0to15Result;
            }
            /* for 42G/53G/106G we need to start timer only after we have signal */


           /* mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), 0x4069, 0x01, &oCoreStat0to15Result2);
            if (status2[anSm->portNum + serdesIndex] != oCoreStat0to15Result2)
            {
                 mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(curPortParams.activeLanesList[serdesIndex],2, ((oCoreStat0to15Result2>>8)&0xFF), (oCoreStat0to15Result2&0xFF)));
                 mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG_SERDES(curPortParams.activeLanesList[serdesIndex],LOG_DEBUG_SERDES_OCORE2, (oCoreStat0to15Result2&0xFFFF)));
                status2[anSm->portNum + serdesIndex] = oCoreStat0to15Result2;
            }*/

            /*Read DFE status int 0x126 data 0xB00*/
           /* mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), 0x126, 0xb00, &dfeStatusRead);
            if (dfeStatus[anSm->portNum + serdesIndex] != dfeStatusRead)
            {
                 mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(curPortParams.activeLanesList[serdesIndex],3, ((dfeStatusRead>>8)&0xFF), (dfeStatusRead&0xFF)));
                 mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG_SERDES(curPortParams.activeLanesList[serdesIndex],LOG_DEBUG_SERDES_DFE, (dfeStatusRead&0xFFFF)));
                dfeStatus[anSm->portNum + serdesIndex] = dfeStatusRead;
            }*/

            if (((AN_CTRL_SKIP_RES_GET(anSm->options)) || (AP_CTRL_SPECIAL_SPEED_GET(anSm->options) && (AP_CTRL_SPECIAL_SPEED_MODE_GET(anSm->capability)))) &&
                ((anStats->timestampTime == 0) && (oCoreStat0to15Result & 0xcc )))
            {
                /*lane finish training*/
                if ( (oCoreStat0to15Result & 0x6) == 0x4 )
                {
                    /* check if signal is down - reset*/
                   rc = mvHwsSerdesSignalDetectGet(anDevNum, anPortGroup,curPortParams.activeLanesList[serdesIndex],
                                                HWS_DEV_SERDES_TYPE(anDevNum, curPortParams.activeLanesList[serdesIndex]), &signalDet);
                   if ( (rc != GT_OK) || (signalDet == GT_FALSE) )
                   {
                       mvPortCtrlAnPortFail("Error, Port-%d TRX training Failed\n",
                                            portIndex, AN_PORT_SERDES_TRAINING_FAILURE);
                       return;
                   }
                }
                /* start link check timer for serdes configuration threshold.
                   Set this timer also in auto KR (when the timers should be disable)
                   because we assert link status before we checked that the training
                   done and disable the internal link inhebit timer by doing it */
                cnt++;
                if ( cnt == curPortParams.numOfActLanes)
                {
                    mvPortCtrlThresholdSet(anIntrop->apLinkDuration, &(anTimer->linkThreshold));
                    anStats->timestampTime = mvPortCtrlCurrentTs();
                    mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(anSm->portNum,2, 7,7));
                }
            }
        }

        if (AN_CTRL_RX_TRAIN_GET(anSm->options))
        {
            trainMode = RxStatusNonBlocking;
            tuneRes.txTune = TUNE_PASS;
        }

        /* Execute HWS Training check */
        if (mvHwsPortAutoTuneSet(anDevNum,
                                 anPortGroup,
                                 (GT_U32)anSm->portNum,
                                 portMode,
                                 trainMode/*TRxTuneStatusNonBlocking*/,
                                 &tuneRes) == GT_OK)
        /* Execute HWS Training check */
        {
            if (tuneRes.txTune == TUNE_PASS)
            {
                /* Training success */
                tuneResult = TUNE_PASS;
                AN_PRINT_MAC(("\nvAnPortSerdesConfigTrainingInProgress portIndex:%d  TUNE_PASS***\n",portIndex));
            }
            else if (tuneRes.txTune == TUNE_FAIL)
            {
                /* Training failure */
                tuneResult = TUNE_FAIL;
                AN_PRINT_MAC(("mvAnPortSerdesConfigTrainingInProgress portIndex:%d  TUNE_FAIL***\n",portIndex));
            }
        }
    }
    else
    {
        tuneResult = TUNE_PASS;
    }

    /* TRX training still in progress*/
    if (tuneResult == TUNE_NOT_COMPLITED)
    {
        if (( HWS_PAM4_MODE_CHECK(portMode)) && ( anSm->status == AN_PORT_SERDES_TRAINING_IN_PROGRESS)&&
            (!AN_CTRL_IS_OPTICAL_MODE_GET(anSm->options))){
             for ( serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++ )
             {
                 mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), 0x2C, 0x11B, &calStatus);
                 if (ABS(calStatus & 0x7FFF) > 4) {
                      if (calDeltaCntr[anSm->portNum] !=0xFF && calDeltaCntr[anSm->portNum] > 0) {
                          calDeltaCntr[anSm->portNum]--;
                          mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG_SERDES(curPortParams.activeLanesList[serdesIndex],LOG_DEBUG_SERDES_OPERATION, (ABS(calStatus)&0xffff) ));
                          mvPortCtrlAnPortFail("Error, Port-%d TRX training Failed\n",
                                               portIndex, AN_PORT_SERDES_TRAINING_FAILURE);
                          return;
                      }
                 }
             }
        }
        return;
    }

    /* clear counters*/
    for (serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++)
    {
        printCounter[anSm->portNum + serdesIndex] = 0;
    }
    /* stop trx training*/
    if (tuneResult == TUNE_FAIL)
    {
        mvHwsPortAutoTuneSet(anDevNum,
                             anPortGroup,
                             (GT_U32)anSm->portNum,
                             portMode,
                             TRxTuneStop,
                             NULL);

        /* Training failure */
        /* let training finish */
        hwsOsTimerWkFuncPtr(20);

        mvPortCtrlAnPortFail("Error, Port-%d TRX training Failed\n",
                             portIndex, AN_PORT_SERDES_TRAINING_FAILURE);
        return;
    }
    /* TRX-Training success */
    else if (tuneResult == TUNE_PASS)
    {
        mvHwsAvagoSerdesManualInterconnectDBGet(anDevNum, anPortGroup, anSm->portNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[0]), &profile);

        /* using auto profile - need to overwrite default rxtx parameters */
        if (/*( portMode == _100GBase_KR4) && (AP_CTRL_SPECIAL_SPEED_MODE_GET(anSm->capability)) */
            AP_CTRL_AUTO_PROFILE_SUPPORT(anSm->capability,anSm->options, portMode)&& (profile == 2 ))
        {
        /*if (( profileIsKnown & (1<<anSm->portNum)) == 0 ) {*/
            /* read tj and set hf threshold */
            mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[0]), 0x2C, 0x2800, &tj);
            /*hfThreshold = (7*tj+415)/75;*/
            /* read HF */
            mvHwsAvago16nmSerdesHalGet(anDevNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[0]), HWS_SERDES_RXEQ_CTLE, 0, (GT_32*)&hf);
            for (i = 0; hfThresholdPerTemperature[i].upToTemperature != 0xff; i++)
            {
                if ( tj <= hfThresholdPerTemperature[i].upToTemperature ) {
                    break;
                }
            }
            profile = (hf > hfThresholdPerTemperature[i].hfThreshold)? 0: 1;


           /* mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(anSm->portNum, 7, hf, profile ));*/


            mvAnPortLinkFailure(portIndex);
            /* set profile */
            for (serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++)
            {
                mvHwsAvagoSerdesManualInterconnectDBSet(anDevNum, anPortGroup, anSm->portNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), profile);
                /* for WA we use register AVAGO_SD_METAL_FIX bits 12-15 to save HF value*/
                hwsSerdesRegSetFuncPtr(anDevNum, anPortGroup, EXTERNAL_REG, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), AVAGO_SD_METAL_FIX, ((hf&0xf) << 12), (0xf << 12));
            }
            return;
        }

        if (((AN_CTRL_SKIP_RES_GET(anSm->options)) || (AP_CTRL_SPECIAL_SPEED_GET(anSm->options) && (AP_CTRL_SPECIAL_SPEED_MODE_GET(anSm->capability)))) &&
            (anStats->timestampTime == 0))
        {
            anStats->timestampTime = mvPortCtrlCurrentTs();
            /*mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(anSm->portNum,2, 8,8));*/
        }


        if (( HWS_PAM4_MODE_CHECK(portMode)) && ( anSm->status == AN_PORT_SERDES_TRAINING_IN_PROGRESS)){
            for ( serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++ )
            {
                mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), 0x2C, 0x11B, &calStatus);
                if (ABS(calStatus & 0x7FFF) > 4) {
                     if (calDeltaCntr[anSm->portNum] !=0xFF && calDeltaCntr[anSm->portNum] > 0) {
                         calDeltaCntr[anSm->portNum]--;
                         mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG_SERDES(curPortParams.activeLanesList[serdesIndex],LOG_DEBUG_SERDES_OPERATION, (calStatus&0x7FFF)));
                         mvPortCtrlAnPortFail("Error, Port-%d TRX training Failed\n",
                                              portIndex, AN_PORT_SERDES_TRAINING_FAILURE);
                         return;
                     }
                }
            }

            for ( serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++ )
            {
                mvHwsAvagoSerdesCalcLevel3(anDevNum, anPortGroup, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]));
            }
        }

        /*
        1.  Wait for KR training to finish.
        2.  only in 106 start pcal and wait untill it finish.
        3.  check qaulifier If abs(dfe2) > 27 or EO < 30 then Training = FAIL else ok */
        if (((HWS_25G_SERDES_MODE_CHECK(portMode)) /*&& (profile != 1)*/) ||
            ((( portMode == _100GBase_KR4) || ( portMode == _50GBase_KR2_C)) && (AP_CTRL_SPECIAL_SPEED_GET(anSm->options))))
        {
            /* PCS link status from*/
            rc = prvPortCtrlApEngLinkUpCheck(anSm->portNum,
                                             portMode,
                                             AP_ST_HCD_FEC_RES_GET(anSm->hcdStatus),
                                             &pcsLinkStatus);

            if ((rc == GT_OK) && (pcsLinkStatus == GT_TRUE))
            {
                if (anStats->linkUpTime == 0)
                 {
                     anStats->linkUpTime = mvPortCtrlCurrentTs() - anStats->timestampTime;
                     mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, AN_PORT_AN_ASSERT_LINK_IN_PROGRESS, anSm->portNum));
                 }
            }

            if ( anSm->status == AN_PORT_SERDES_TRAINING_IN_PROGRESS)
            {
                extraTime[anSm->portNum] = 2000;
                for (serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++)
                {
                    /*start pcal*/
                    mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), 0xA, 0x02, NULL);
                }
                /*anSm->status = AN_PORT_SERDES_TRAINING_IN_PROGRESS_WAIT;*/
                AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_TRAINING_IN_PROGRESS_WAIT);
                /*mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(curPortParams.activeLanesList[0],6, 0xA, 2));
                mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG_SERDES(anSm->portNum, LOG_DEBUG_SERDES_PCAL, stage11d[anSm->portNum]));*/
                mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
                return;
            }
            else if(anSm->status == AN_PORT_SERDES_TRAINING_IN_PROGRESS_WAIT)
            {
                for (serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++)
                {
                    /*wait for pcal to end*/
                    mvHwsAvagoSerdesAutoTuneStatusShort(anDevNum, anPortGroup, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), &rxStatus,NULL);
                    if (rxStatus != TUNE_PASS)
                    {
                        if (rxStatus == TUNE_FAIL)
                        {
                            mvAnPortLinkFailure(portIndex);
                            return;
                        }
                        return;
                    }
                }

                for (serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++)
                {
                    if ( (stage11d[anSm->portNum] < 8 ) && (mvAnPortSerdesQaulifier(MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex])) == GT_FALSE) )
                    {
                        /* qualifier failed, update stage11d for next round*/
                        stage11d[anSm->portNum]++;
                        mvAnPortLinkFailure(portIndex);
                        return;
                    }
                }
                stage11d[anSm->portNum] = 0;
            }
        }
        /*anSm->status = AN_PORT_SERDES_TRAINING_SUCCESS;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_TRAINING_SUCCESS);

        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

        /*anSm->status = AN_PORT_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK);
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

         /* Check System Config */
        mvAnPortSerdesConfigCheckSystemConfig(portIndex);

        /* operate SerDes in low power mode */
        for (i = 0; i < curPortParams.numOfActLanes; i++)
        {
            mvHwsAvagoSerdesLowPowerModeEnable(anDevNum, anPortGroup, curPortParams.activeLanesList[i], GT_TRUE);
        }

    }
    return;
}


/**
* @internal mvAnPortSerdesConfigPcsMacFecStartTraining function
* @endinternal
*
* @brief  execute training on AN port. Execute port Init Execute
*         SERDES Configuration Execute MAC/PCS Configuration
*         Execute training Training configuration Training start
*
*         linkThreshold = the time from HCD to full link
*
* @param[in] portIndex           - index of port in local AP DB
* */
static void mvAnPortSerdesConfigPcsMacFecStartTraining
(
    IN  GT_U8 portIndex
)
{

    MV_HWS_AN_SM_INFO       *anSm     = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_AN_SM_STATS      *anStats  = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].stats);
    MV_HWS_PORT_STANDARD portMode = anSm->portMode;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    MV_HWS_AUTO_TUNE_STATUS_RES     tuneRes;
    GT_STATUS                       rc        = GT_OK;
    GT_U8                           trainingStatus;
    MV_HWS_PORT_INIT_INPUT_PARAMS   portInitInParam;
    GT_U32                          oCoreStat0to15Result, serdesIndex;
    GT_32                           dfeCommon;
    GT_U32                          result = 0, data[2] = {0}, profile;

    if (GT_OK != hwsPortModeParamsGetToBuffer(anDevNum, anPortGroup, (GT_U32)anSm->portNum, portMode, &curPortParams))
    {
        mvPortCtrlAnPortFail("Error, Port AN, Port-%d hwsPortModeParamsGet return NULL\n",
                             portIndex, AN_PORT_SERDES_CONFIGURATION_FAILURE);
        return;
    }

    if (anSm->status == AN_PORT_SERDES_CONFIGURATION_SUCCESS)
    {

        /* config PCS, FEC, MAC*/
        trainingStatus = anSm->status;
        mvAnPortSerdesConfigPcsFecMac(portIndex);
        /*anSm->status = trainingStatus;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, trainingStatus);

        for (serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++)
        {
            printCounter[anSm->portNum+serdesIndex] = 0;
            status2[anSm->portNum+serdesIndex]=0;
            dfeStatus[anSm->portNum+serdesIndex]=0;


        }

        if (mvHwsPortTxEnable(anDevNum, anPortGroup, (GT_U32)anSm->portNum, portMode, GT_TRUE) != GT_OK)
        {
             mvPortCtrlAnPortFail("Error, Port AN, Port-%d mvHwsPortTxEnable Failed\n",
                                  portIndex, AN_PORT_SERDES_CONFIGURATION_FAILURE);
             return;
        }
#if 0
        /* set avago trining timeout to 3.2 sec */
        if (HWS_PAM4_MODE_CHECK(portMode)) {
            for ( serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++ )
            {
                mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), 0x3D, 0x2004, NULL);

                mvHwsAvago16nmSerdesHalSet(anDevNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), HWS_SERDES_KR_TRAINING_CONFIG, 0xB, 0x1DCD);
                mvHwsAvago16nmSerdesHalSet(anDevNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), HWS_SERDES_KR_TRAINING_CONFIG, 0xA, 0x6500);
            }
        }
#endif
        if ((portMode != _1000Base_X) && (portMode != _10GBase_KX4))
        {
            AN_PRINT_MAC(("mvAnPortSerdesConfigPcsMacFecStartTraining: call mvHwsPortAutoTuneSet  port:%d \n",portIndex));

            if ((AN_CTRL_PRECODING_GET(anSm->options)) && (HWS_PAM4_MODE_CHECK(portMode)))
            {
                /* write to serdes to send precoding */
                if (mvHwsPortOperation(anDevNum, anPortGroup, (GT_U32)anSm->portNum, portMode,
                                   MV_HWS_PORT_SERDES_OPERATION_PRECODING_E, data, &result)!= GT_OK)
                {
                    mvPortCtrlAnPortFail("Error, Port AN mvAnPortSerdesConfigPcsMacFecStartTraining, Port-%d training config Failed\n",
                                         portIndex, AN_PORT_SERDES_TRAINING_FAILURE);
                    return;
                }
            }
            /* Execute HWS Training config, done nothing */
            if (mvHwsPortAutoTuneSet(anDevNum,
                                     anPortGroup,
                                     (GT_U32)anSm->portNum,
                                     portMode,
                                     TRxTuneCfg,
                                     0) != GT_OK)
            {
                mvPortCtrlAnPortFail("Error, Port AN mvAnPortSerdesConfigPcsMacFecStartTraining, Port-%d training config Failed\n",
                                     portIndex, AN_PORT_SERDES_TRAINING_FAILURE);
                return;
            }

            tuneRes.txTune = TUNE_NOT_COMPLITED;

            for (serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++)
            {
                if ((mvAnSerdesStatus(curPortParams.activeLanesList[serdesIndex], AN_SERDES_AN_READ_TRAINING_RESULTS, &oCoreStat0to15Result)) != GT_OK)
                {
                    if (((AN_CTRL_SKIP_RES_GET(anSm->options)) || (AP_CTRL_SPECIAL_SPEED_GET(anSm->options) && (AP_CTRL_SPECIAL_SPEED_MODE_GET(anSm->capability)))) &&
                         ((anStats->timestampTime == 0) && (oCoreStat0to15Result & 0xc8 )))
                     {
                         /* start link check timer for serdes configuration threshold.
                            Set this timer also in auto KR (when the timers should be disable)
                            because we assert link status before we checked that the training
                            done and disable the internal link inhebit timer by doing it */
                         anStats->timestampTime = mvPortCtrlCurrentTs();
                     }
                    /*AN_PRINT_MAC_ERR(("mvAnSerdesStatus: serdes:%d failed to read AN avago SM status %d\n",curPortParams.activeLanesList[serdesIndex], AN_SERDES_AN_READ_TRAINING_RESULTS));*/
                }
                /*mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(curPortParams.activeLanesList[serdesIndex],0, ((oCoreStat0to15Result>>8)&0xFF), (oCoreStat0to15Result&0xFF)));*/
                /*mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG_SERDES(curPortParams.activeLanesList[serdesIndex],LOG_DEBUG_SERDES_OCORE, (oCoreStat0to15Result&0xFFFF)));*/
            }
            {
                MV_HWS_PORT_AUTO_TUNE_MODE trainMode = TRxTuneStart;
                if (AN_CTRL_RX_TRAIN_GET(anSm->options))
                {
                    trainMode = RxTrainingOnly;
                }

                if (tuneRes.txTune != TUNE_NOT_READY)
                {
                    /* Execute HWS Training start */
                    if (mvHwsPortAutoTuneSet(anDevNum,
                                             anPortGroup,
                                             (GT_U32)anSm->portNum,
                                             portMode,
                                             trainMode/*TRxTuneStart*/,
                                             &tuneRes) != GT_OK)
                    {
                        tuneRes.txTune = TUNE_FAIL;
                    }
                }
            }
            if (tuneRes.txTune == TUNE_NOT_READY)
            {
                /*anSm->status = AN_PORT_SERDES_TRAINING_NOT_READY;*/
                AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_TRAINING_NOT_READY);
                mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
            }
            else if (!((tuneRes.txTune == TUNE_PASS) || (tuneRes.txTune == TUNE_READY)))
            {
                mvPortCtrlAnPortFail("Error, Port AN, Port-%d training starting Failed\n",
                                     portIndex, AN_PORT_SERDES_TRAINING_FAILURE);
                return;
            }
            else
            {
                if ((AN_CTRL_RX_TRAIN_GET(anSm->options)) &&(anStats->timestampTime == 0))
                {
                             anStats->timestampTime = mvPortCtrlCurrentTs();
                             anStats->hcdResoultionTime = mvPortCtrlCurrentTs()- (GT_U32)timestampTime2[anSm->portNum] ;
                             mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(anSm->portNum,2, 6, 6));
                }
                /*anSm->status = AN_PORT_SERDES_TRAINING_IN_PROGRESS;*/
                AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_TRAINING_IN_PROGRESS);
                mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
                AN_PRINT_MAC(("mvAnPortSerdesConfigTrainingInProgress portIndex:%d  \n",anSm->portNum));

            }

        }

        if ((portMode == _1000Base_X) || (portMode == _10GBase_KX4))
        {
            /*anSm->status = AN_PORT_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK;*/
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK);
            mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
            /* Check System Config */
            mvAnPortSerdesConfigCheckSystemConfig(portIndex);

        }
        return;
    }

    if (anSm->status == AN_PORT_SERDES_CONFIGURATION_IN_PROGRESS)
    {
#ifdef RAVEN_DEV_SUPPORT
        MV_HWS_SERDES_CONFIG_STC        serdesConfig = {0};
        serdesConfig.opticalMode = AN_CTRL_IS_OPTICAL_MODE_GET(anSm->options);
        serdesConfig.baudRate       = curPortParams.serdesSpeed;
        serdesConfig.media          = curPortParams.serdesMediaType;
        serdesConfig.busWidth       = curPortParams.serdes10BitStatus;
        serdesConfig.serdesType     = (MV_HWS_SERDES_TYPE)(HWS_DEV_SERDES_TYPE(anDevNum, MV_HWS_SERDES_NUM(curLanesList[0])));
        if ( HWS_PAM4_MODE_CHECK(portMode)) {
            serdesConfig.encoding = SERDES_ENCODING_PAM4;
        }
        else
        {
            serdesConfig.encoding = SERDES_ENCODING_NA;
        }
        rc = mvHwsRavenApPortInitPhase(anDevNum,
                             anPortGroup,
                             (GT_U32)anSm->portNum,
                             portMode,
                             &portInitInParam,
                             MV_HWS_AVAGO_SERDES_POWER_LAST_PHASE,
                             &serdesConfig);

        if (rc != GT_OK)
        {
            mvPortCtrlAnPortFail("Error, Port AN, Port-%d Init Failed\n",
                                 portIndex, AN_PORT_SERDES_CONFIGURATION_FAILURE);
            return;
        }
#endif
        /* if qualifier failed last round we try to update int 0x11d (range 150,200,250,300)*/
        if (HWS_25G_SERDES_MODE_CHECK(portMode))
        {
            if (stage11d[anSm->portNum] > 0){
                for (serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++)
                {
                    mvHwsAvago16nmSerdesHalSet(anDevNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), HWS_SERDES_GLOBAL_TUNE_PARAMS,0x1D,(180 - stage11d[anSm->portNum]*10) );
                }
            }
        }
        if ( HWS_PAM4_MODE_CHECK(portMode)) {
            if ( calDeltaCntr[anSm->portNum] == 0xFF){
                mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[0]), 0x409e, 0, &dfeCommon);
                calDeltaCntr[anSm->portNum] = ((dfeCommon& 0X1C)>>2);


            }
            else
            {
                dfeCommon = (calDeltaCntr[anSm->portNum]<<2)+ 0x220;
                for ( serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++ )
                {
                    mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), 0x809e, dfeCommon, NULL);
                }
            }
            if ( (AN_CTRL_RX_TRAIN_GET(anSm->options)) && (!AN_CTRL_IS_OPTICAL_MODE_GET(anSm->options)))
            {
                for ( serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++ )
                {
                    /* vernier delay*/
                    mvHwsAvago16nmSerdesHalSetIfValid(anDevNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), HWS_SERDES_RXCLK_SELECT,  0x5, 0 , NA_8BIT );
                    mvHwsAvago16nmSerdesHalSetIfValid(anDevNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), HWS_SERDES_RXCLK_SELECT,  0x6, 0 , NA_8BIT );
                    mvHwsAvago16nmSerdesHalSetIfValid(anDevNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), HWS_SERDES_RXCLK_SELECT,  0x7, 0 , NA_8BIT );
                    mvHwsAvago16nmSerdesHalSetIfValid(anDevNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), HWS_SERDES_RXCLK_SELECT,  0x8, 0 , NA_8BIT );
                    mvHwsAvago16nmSerdesHalSetIfValid(anDevNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), HWS_SERDES_RXCLK_SELECT,  0x9, 0 , NA_8BIT );
                    mvHwsAvago16nmSerdesHalSetIfValid(anDevNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), HWS_SERDES_RXCLK_SELECT,  0xa, 0 , NA_8BIT );
                    mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), 0xEC, 0x3, NULL);
                }
            }
        }

        mvHwsAvagoSerdesManualInterconnectDBGet(anDevNum, anPortGroup, anSm->portNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[0]),&profile);

        /* using auto profile - need to overwrite default rxtx parameters */
        if ((AP_CTRL_AUTO_PROFILE_SUPPORT(anSm->capability,anSm->options, portMode) && (profile == 2 )) ||
            ((HWS_PAM4_MODE_CHECK(anSm->portMode)) && ((profile != 1 )||(AN_CTRL_RX_TRAIN_GET(anSm->options)))))
        {
            GT_32 tj,val;
            /* read tj and set hf threshold */
            mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[0]), 0x2C, 0x2800, &tj);
            if ((HWS_PAM4_MODE_CHECK(anSm->portMode)) && (AN_CTRL_RX_TRAIN_GET(anSm->options)))
            {
                val = (2*tj/50+11)&0xf;
                val = (val>15)?15:val;
            }
            else if ( (HWS_PAM4_MODE_CHECK(anSm->portMode)) && (profile != 1) )
            {
                val = (5*tj/70+2)&0xf;
                val = (val>10)?10:val;
            }
            else
            {
                val = (5*tj/80+6)&0xf;
                val = (val>11)?11:val;
            }
            mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(anSm->portNum,7, 55, 55));
            for (serdesIndex = 0; serdesIndex < curPortParams.numOfActLanes; serdesIndex++)
             {
                 mvHwsAvago16nmSerdesHalSetIfValid(anDevNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), HWS_SERDES_RXEQ_CTLE, 0x0,  val, NA_8BIT);
                 mvHwsAvago16nmSerdesHalSetIfValid(anDevNum, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), HWS_SERDES_RXEQ_CTLE, 0x3,  val, NA_8BIT);

                 mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, MV_HWS_SERDES_NUM(curPortParams.activeLanesList[serdesIndex]), 0xEC, 0x15, NULL);

             }
        }
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_CONFIGURATION_SUCCESS);
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
        return;
    }

    if (anSm->status == AN_PORT_RESOLUTION_SUCCESS)
    {
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

        /* SERDES Tx Disable - disable signal on lane where managed negotiatio */
        mvHwsSerdesTxEnable(anDevNum, anPortGroup, AP_CTRL_LANE_GET(anSm->ifNum), HWS_DEV_SERDES_TYPE(0/*devNum*/, AP_CTRL_LANE_GET(anSm->ifNum)), GT_FALSE);
        osMemSet(&portInitInParam, 0,sizeof(portInitInParam));
        portInitInParam.refClock = MHz_156;
        portInitInParam.refClockSource = PRIMARY_LINE_SRC;
        portInitInParam.lbPort = GT_FALSE/* action*/;

        rc = mvHwsApPortInit(anDevNum,
                             anPortGroup,
                             (GT_U32)anSm->portNum,
                             portMode,
                             &portInitInParam);

        if (rc != GT_OK)
        {
            mvPortCtrlAnPortFail("Error, Port AN, Port-%d Init Failed\n",
                                 portIndex, AN_PORT_SERDES_CONFIGURATION_FAILURE);
            return;
        }
        /*anSm->status = AN_PORT_SERDES_CONFIGURATION_IN_PROGRESS;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_SERDES_CONFIGURATION_IN_PROGRESS);
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

    }

    /* training in progress*/
    return;
}


/**
* @internal mvAnPortSerdesConfig function
* @endinternal
*
* @brief   execute training on AN port.
*         Execute port Init
*         Execute SERDES Configuration Execute MAC/PCS
*         Configuration Execute training Training configuration
*         Training start Training timeout timer
*
*         linkThreshold = the time from HCD to full link
*         training threshold = the time for TRX training
*
* @param[in] portIndex             - number of physical port
*
* @retval GT_OK                    - on success/delete
* */
GT_STATUS mvAnPortSerdesConfig
(
    IN  GT_U8      portIndex
)
{

    MV_HWS_AN_SM_INFO  *anSm    = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_AN_SM_TIMER *anTimer  = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].timer);
    MV_HWS_AN_SM_STATS *anStats = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].stats);
    MV_HWS_AP_DETECT_ITEROP *anIntrop = &(mvHwsPortCtrlAnPortDetect.introp);
    GT_U16               linkCheckDuration;
    GT_U16               linkCheckInterval;
    MV_HWS_PORT_STANDARD            portMode;
    portMode = anSm->portMode;
    /* if auto KR = 1 , the Serdes power up the port to the HCD and start the training
       internaly with internals timers.
       if not, we need to configure the port to the HCD and do the training.

    For auto KR enable the flow is:
         1. polling on o_core_status[0:15] = 0x34 whitch indicate that the training done and port is OK.
         2. check link_control == 0x2 ( Enable)
         3. configure PCS, FEC, MAC.
         4. validate port's PIZZA and enable traffic.
         5. assert link status.
         6. wait for o_core_status[19] = 1.

     For auto KR disable or multi lane the flow is:
         1. configure the serdes and start training
         2. configure PCS, FEC, MAC, training finished
         3. validate port's PIZZA and enable traffic.
         4. polling on o_core_status[0:15] = 0x34 wich indicate that the port is OK.
         5. check link_control == 0x2 ( Enable)
         6. check pcs link status
         7. assert link status.
         8. wait for o_core_status[19] = 1.
     */

    /* Check link on MAC according to HCD resolution*/
    switch(portMode)
    {
        case _1000Base_X:
        case _10GBase_KX4:
            linkCheckDuration = anIntrop->pdLinkDuration;
            linkCheckInterval = anIntrop->pdLinkMaxInterval;
            break;

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
            linkCheckDuration = anIntrop->apLinkDuration;
            linkCheckInterval = anIntrop->anPam4LinkMaxInterval;
            break;

        default:
            linkCheckDuration = anIntrop->apLinkDuration;
            linkCheckInterval = anIntrop->apLinkMaxInterval;
    }

    /* Check link threshold */
    if ((anTimer->linkThreshold != PORT_CTRL_TIMER_DEFAULT) && (!mvPortCtrlThresholdCheck(anTimer->linkThreshold)))
    {
        return GT_OK;
    }

    if (AP_CTRL_SPECIAL_SPEED_GET(anSm->options) && (AP_CTRL_100GBase_KR4_GET(anSm->capability) || AP_CTRL_50GBase_KR2_CONSORTIUM_GET(anSm->capability)))
    {
        /* for 106 we give 1sec for link up time*/
        linkCheckInterval = linkCheckInterval+70;
    }
    else if((anSm->status == AN_PORT_AN_COMPLETE_IN_PROGRESS) || (anSm->status == AN_PORT_AN_ASSERT_LINK_IN_PROGRESS)
            || (anSm->status == AN_PORT_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK) )
    {
        if ((AN_CTRL_RX_TRAIN_GET(anSm->options)) && (HWS_PAM4_MODE_CHECK(anSm->portMode)))
        {
            linkCheckInterval = 1100;
        }
        else
        /* for 100 we give 650msec for training and 1sec for link up time*/
        linkCheckInterval =  linkCheckInterval+70;
    }

    anTimer->linkCount++;
    if ((anStats->timestampTime != PORT_CTRL_TIMER_DEFAULT) &&
        ((mvPortCtrlCurrentTs() - anStats->timestampTime) > (GT_U32)(extraTime[anSm->portNum] + (linkCheckInterval*linkCheckDuration))))
    {
        anTimer->linkCount = 0;
        /* link timer reset */
        mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(anTimer->linkThreshold));

        /* Link check failure, start AN again */
        /* Link check failure */
        mvAnPortLinkFailure(portIndex);
        return GT_OK;
    }

#if 0
    if (AN_CTRL_AUTO_KR_ENABLE_GET(anSm->options))
    {
        AN_PRINT_MAC(("mvAnPortSerdesConfig: portIndex:%d starta auto KR training\n",portIndex));
        CHECK_STATUS(mvAnPortSerdesConfigAutoKr(portIndex));
        return GT_OK;
    }
    else
#endif
    {

        /* auto KR disabled, the flow control:
            AN_PORT_RESOLUTION_SUCCESS ->
            (opt: AN_PORT_SERDES_TRAINING_NOT_READY ->)
            AN_PORT_SERDES_TRAINING_IN_PROGRESS ->
            AN_PORT_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK->
            AN_PORT_AN_COMPLETE_IN_PROGRESS*/

        switch (anSm->status)
        {
            case AN_PORT_RESOLUTION_SUCCESS:
            case AN_PORT_SERDES_CONFIGURATION_IN_PROGRESS:
            case AN_PORT_SERDES_CONFIGURATION_SUCCESS:
                mvAnPortSerdesConfigPcsMacFecStartTraining(portIndex);
                break;
            case AN_PORT_SERDES_TRAINING_NOT_READY:
                CHECK_STATUS(mvAnPortSerdesConfigTrainingNotReady(portIndex));
                break;
            case AN_PORT_SERDES_TRAINING_IN_PROGRESS:
            case AN_PORT_SERDES_TRAINING_IN_PROGRESS_WAIT:
                mvAnPortSerdesConfigTrainingInProgress(portIndex);
                break;
            case AN_PORT_SERDES_TRAINING_END_WAIT_FOR_SYS_ACK:
                mvAnPortSerdesConfigCheckSystemConfig(portIndex);
                break;
            case AN_PORT_AN_COMPLETE_IN_PROGRESS:
                CHECK_STATUS(mvAnPortSerdesConfigAssertLinkStatus(portIndex));
                break;
            default:
                return GT_OK;
        }
    }

    /* Link check timer restart */
    mvPortCtrlThresholdSet(linkCheckDuration, &(anTimer->linkThreshold));

    return GT_OK;
}


/**
* @internal mvAnPortDeleteMsg function
* @endinternal
*
* @brief   AN Port Delete Msg execution sequence
*
* @param[in] portIndex                 - number of physical port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  -on failure
* */
GT_STATUS mvAnPortDeleteMsg
(
    IN  GT_U8      portIndex
)
{
    MV_HWS_AN_SM_INFO *anSm = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_AN_SM_TIMER *anTimer  = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].timer);
    /* Validate AP port state & status */
    if (anSm->state != AN_PORT_SM_IDLE_STATE)
    {
        if ((anSm->state > AN_PORT_SM_AN_STATE) &&
            (anSm->status != AN_PORT_DELETE_IN_PROGRESS))
        {
            doorbellIntrAddEvent(AP_DOORBELL_EVENT_PORT_STATUS_CHANGE(anSm->portNum));

            /* AP Port Delete */
            if(mvAnPortDelete(portIndex,GT_TRUE) != GT_OK)
            {
                AN_PRINT_MAC_ERR(("Port %d - Delete AN delete task failed\n",anSm->portNum));
                return GT_FAIL;
            }

            /* AP state & status update */
            /*anSm->state = AN_PORT_SM_DISABLE_STATE;*/
            AN_PORT_SM_SET_STATE(anSm->portNum, anSm, AN_PORT_SM_DISABLE_STATE);
            mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

            /*other reset operation will be done after AP training task delete*/
            return GT_OK;

        }
        else
        {

            /*we get to this point if resulotion was not found till the point user ask to disable AP*/

            /* AP state & status update */
            /*anSm->state = AN_PORT_SM_DISABLE_STATE;*/
            AN_PORT_SM_SET_STATE(anSm->portNum, anSm, AN_PORT_SM_DISABLE_STATE);
            /*anSm->status = AN_PORT_DISABLE_IN_PROGRESS;*/
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_DISABLE_IN_PROGRESS);
            mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

            /* AP Port Delete - eliminate the option of one active lane after port deletion
               (SerDes power-down optimization flow) */
            mvApSerdesPowerUp(AP_CTRL_LANE_GET(anSm->ifNum), GT_FALSE, anSm->portNum, MV_HWS_AVAGO_SERDES_POWER_FIRST_PHASE);

            /* AP Reset All */
            mvAnResetStatus(portIndex);
            /*reset all timers*/
            mvAnResetTimer(portIndex);
            mvAnResetStats(portIndex);
            mvAnResetStatsTimestamp(portIndex);

            /*Reset Pizza ack parameters*/
            anTimer->sysCfStatus = PORT_SM_SERDES_SYSTEM_NOT_VALID;
            anTimer->sysCfMode = NON_SUP_MODE;
            /*anSm->status = AN_PORT_DELETE_SUCCESS;*/
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_DELETE_SUCCESS);
            mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

            /* AN state & status update */
            /*anSm->state = AN_PORT_SM_IDLE_STATE;*/
            AN_PORT_SM_SET_STATE(anSm->portNum, anSm, AN_PORT_SM_IDLE_STATE);

        }
    }

    return GT_OK;
}


/**
* @internal mvAnPortDeleteValidate function
* @endinternal
*
* @brief   AN delete port execution sequence
*
* @param[in] portIndex                 - number of physical port
*
* @retval GT_OK                    - on success/delete
* */
GT_STATUS mvAnPortDeleteValidate
(
    IN  GT_U8      portIndex
)
{
    MV_HWS_AN_SM_INFO    *anSm = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_AN_SM_STATS       *anStats = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].stats);
    MV_HWS_AN_SM_TIMER *anTimer  = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].timer);

    /* AP state & status update */
    if(anSm->state == AN_PORT_SM_DELETE_STATE)
    {

        if(anSm->status == AN_PORT_ENABLE_WAIT)
        {
            if (mvPortCtrlCurrentTs() - anStats->timestampTime > 1000){
                anStats->timestampTime = mvPortCtrlCurrentTs();
                /*mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(anSm->portNum, 7, 77, 80 ));*/
                doorbellIntrAddEvent(AP_DOORBELL_EVENT_PORT_AP_DISABLE(anSm->portNum));
            }
            return GT_OK;
        }


        if(anSm->status == AN_PORT_DELETE_IN_PROGRESS)
        {
            if(mvAnPortReset(portIndex) == GT_OK)
            {
                /*anSm->status = AN_PORT_DELETE_SUCCESS;*/
                AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_DELETE_SUCCESS);
                mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
            }
        }

        /* AP Timing reset */
        mvAnResetStatsTimestamp(portIndex);
        /* AP Timing measurment */
        anStats->timestampTime = mvPortCtrlCurrentTs();

        /* AP state and status update */
        /*anSm->state = AN_PORT_SM_AN_STATE;*/
        AN_PORT_SM_SET_STATE(anSm->portNum, anSm, AN_PORT_SM_AN_STATE);
        /*anSm->status = AN_PORT_START_EXECUTE;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_START_EXECUTE);

        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

        /* set random timer when deleting port, each port will start AP in a different time*/
        mvPortCtrlThresholdSet(((portIndex%8) * 100), &(anTimer->abilityThreshold));
    }
    else if(anSm->state == AN_PORT_SM_DISABLE_STATE)
    {
        /* AP Reset All */
        mvAnResetStatus(portIndex);
        /* reset all timers*/
        mvAnResetTimer(portIndex);
        mvAnResetStats(portIndex);
        mvAnResetStatsTimestamp(portIndex);

        /*Reset Pizza ack parameters*/
        anTimer->sysCfStatus = PORT_SM_SERDES_SYSTEM_NOT_VALID;
        anTimer->sysCfMode = NON_SUP_MODE;

        /*anSm->status = AN_PORT_DELETE_SUCCESS;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_DELETE_SUCCESS);
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

        /* AP state & status update */
        anSm->state  = AN_PORT_SM_IDLE_STATE;
    }

    return GT_OK;
}


/**
* @internal mvAnPortLinkUp function
* @endinternal
*
* @brief   Link up check. check for port AN complete by polling
*          o_core_status[19] and after that validate port link
*          up staus.
*
* @param[in] portIndex                 - number of physical port
*
* @retval GT_OK                    - on success/delete
* */
GT_STATUS mvAnPortLinkUp
(
    IN  GT_U8      portIndex
)
{

    MV_HWS_AN_SM_INFO  *anSm    = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    MV_HWS_AN_SM_STATS *anStats = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].stats);
    MV_HWS_AN_SM_TIMER *anTimer  = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].timer);
    MV_HWS_AP_DETECT_ITEROP *anIntrop = &(mvHwsPortCtrlAnPortDetect.introp);

    GT_U32               anCompleteResult = 0;
    GT_BOOL              linkUp;
    GT_STATUS            rc;
    GT_U16               linkCheckDuration;
    GT_U16               linkCheckInterval;
    GT_U16               linkCheckIdleDuration;
    MV_HWS_PORT_STANDARD portMode;
    GT_U8 serdesNum = AP_CTRL_LANE_GET(anSm->ifNum);

    portMode = anSm->portMode;
    /* Check link threshold */
    if (!(mvPortCtrlThresholdCheck(anTimer->linkThreshold)))
    {
        return GT_OK;
    }

    switch(portMode)
    {
        case _1000Base_X:
        case _10GBase_KX4:
            linkCheckDuration = anIntrop->pdLinkDuration;
            linkCheckInterval = anIntrop->pdLinkMaxInterval;
            linkCheckIdleDuration = anIntrop->pdLinkDuration*2;
            break;

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
            linkCheckDuration = anIntrop->apLinkDuration;
            linkCheckInterval = anIntrop->anPam4LinkMaxInterval;
            linkCheckIdleDuration = anIntrop->apLinkDuration*2;
            break;
        case _100GBase_KR4:
        case _100GBase_CR4:
            linkCheckDuration = anIntrop->apLinkDuration;
            linkCheckInterval = anIntrop->apLinkMaxInterval*2;
            linkCheckIdleDuration = anIntrop->apLinkDuration*2;
            break;
        default:
            linkCheckDuration = anIntrop->apLinkDuration;
            linkCheckInterval = anIntrop->apLinkMaxInterval;
            linkCheckIdleDuration = anIntrop->apLinkDuration*2;
    }

    if ((AN_CTRL_RX_TRAIN_GET(anSm->options)) && (HWS_PAM4_MODE_CHECK(anSm->portMode)))
    {
        linkCheckInterval = 1100;
    }
    if (anSm->status == AN_PORT_AN_ASSERT_LINK_SUCCESS)
    {
#if 0 /* TODO we don't get AN complete - we skip it for now*/
        /*training Done, but we still waiting to o_core_status[19] for AN complete*/
        if ((mvAnSerdesStatus(serdesNum, AN_SERDES_AN_COMPLETE,&anCompleteResult)) != GT_OK)
        {
            AN_PRINT_MAC(("mvAnSerdesStatus: port:%d failed to read AN avago SM status %d\n",portIndex, AN_SERDES_AN_COMPLETE));
        }
#else
        anCompleteResult = 1;
#endif

        if ((anCompleteResult & 0x1) == 0x1)
        {
            /*anSm->status = AN_PORT_AN_COMPLETE_SUCCESS;*/
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_AN_COMPLETE_SUCCESS);

            mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
            calDeltaCntr[anSm->portNum]=0xFF;

        }
        else
        {
            anTimer->linkCount++;

            if ((mvPortCtrlCurrentTs() - anStats->timestampTime) < (GT_U32)(extraTime[anSm->portNum] + ((linkCheckInterval+70)*linkCheckDuration)))
            {
                /* link timer restart */
                osPrintf("mvAnPortSerdesConfigTrainingInProgress: set threshold again\n");
                mvPortCtrlThresholdSet(linkCheckDuration, &(anTimer->linkThreshold));
                return GT_OK;
            }
            else /*timeout*/
            {
                osPrintf("mvAnPortLinkUp: assert link done but o_cor_status[19]is [0x%8.8x], timeout\n",anCompleteResult);
                /* Clear link timer */
                anTimer->linkCount = 0;
                /* link timer reset */
                mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(anTimer->linkThreshold));
                /* Link check failure, start AN again */
                mvAnPortLinkFailure(portIndex);
                return GT_OK;
            }
        }
    }

    if ((anSm->status == AN_PORT_AN_COMPLETE_SUCCESS )||(anSm->status == AN_PORT_LINK_CHECK_SUCCESS))
    {
        rc = prvPortCtrlApEngLinkUpCheck(anSm->portNum, portMode,
                                    AP_ST_HCD_FEC_RES_GET(anSm->hcdStatus), &linkUp);
        if (rc != GT_OK)
        {
            /* AP state & status update */
            /*anSm->status = AN_PORT_LINK_CHECK_FAILURE;*/
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_LINK_CHECK_FAILURE);
            mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));
            return rc;
        }
        /* Port is Active & Link_UP detected */
        if (linkUp)
        {

            /* Link up process - only at first occurance */
            if (anSm->status != AN_PORT_LINK_CHECK_SUCCESS)
            {
                /* AP state & status update */
                /*anSm->status = AN_PORT_LINK_CHECK_SUCCESS;*/
                AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_LINK_CHECK_SUCCESS);
                mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

                /* AP HCD update */
                AP_ST_HCD_LINK_SET(anSm->hcdStatus, 1);

                /* Link Interrupt update */
                AP_ST_LINK_INT_TRIG_SET(anSm->hcdStatus, 1);

                /* AP Timing measurment */
                if (anStats->linkUpTime == 0)
                {
                    anStats->linkUpTime = mvPortCtrlCurrentTs() - anStats->timestampTime;
                }
                anStats->timestampTime = mvPortCtrlCurrentTs();

                /* AP statistics update */
                anStats->linkSuccessCnt++;

                /*Doorbell interrupt*/
                doorbellIntrAddEvent(AP_DOORBELL_EVENT_PORT_STATUS_CHANGE(anSm->portNum));
                mvAnPortRemoteFaultEnable(portIndex,GT_FALSE);
                /* AP register dump */
                mvPortCtrlDbgCtrlRegsDump(portIndex, serdesNum, AN_PORT_SM_LINK_STATUS_STATE);
            }
            if (mvPortCtrlCurrentTs() - anStats->timestampTime > 500)
            {
                anStats->timestampTime = mvPortCtrlCurrentTs();
                if (_1000Base_X != portMode)
                {
                    mvHwsPortAvagoDfeCfgSet(anDevNum,anPortGroup,anSm->portNum,portMode,mvPortCtrlLinkUpDfeMode);
                }
            }

            /* Link check timer restart */
            mvPortCtrlThresholdSet(linkCheckIdleDuration, &(anTimer->linkThreshold));
            anTimer->linkCount = 0;
        }
        /* Port is Active NO Link_UP detected */
        else
        {
            /* Port is Active and No link detected,
            ** The previous status was link success, therefore it means link down
            ** is detected first time
            ** In this case execute Link failure
            */
            if (anSm->status == AN_PORT_LINK_CHECK_SUCCESS)
            {
                /* AP state & status update */
                /*anSm->status = AN_PORT_LINK_CHECK_FAILURE;*/
                AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_LINK_CHECK_FAILURE);
                mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, anSm->portNum));

                /* Link failure */
                mvAnPortLinkFailure(portIndex);
                /*Doorbell interrupt*/
                doorbellIntrAddEvent(AP_DOORBELL_EVENT_PORT_STATUS_CHANGE(anSm->portNum));
                return GT_OK;
            }
            /* Port is Active and No link detected,
            ** The previous status was NOT link success
            ** It means that the port is in the process of bring up
            ** this case execute Link validate
            */
            anStats->linkUpTime = 0;
            anTimer->linkCount++;
            if ((mvPortCtrlCurrentTs() - anStats->timestampTime) > (GT_U32)(extraTime[anSm->portNum] + (linkCheckInterval*linkCheckDuration)))
            {
                anTimer->linkCount = 0;
                /* Training timer reset */
                mvPortCtrlThresholdSet(PORT_CTRL_TIMER_DEFAULT, &(anTimer->linkThreshold));
                /* Link check failure, start AN again */
                /* Link check failure */
                mvAnPortLinkFailure(portIndex);
            }
            else
            {
                /* Link check timer restart */
                mvPortCtrlThresholdSet(linkCheckDuration, &(anTimer->linkThreshold));
            }
        }
    }
    return GT_OK;
}
#ifdef CONSORTIUM_SUPPORT
/**
* @internal mvAnConsortiumAdvertiseNextPage0Set
* @endinternal
*
* @brief set the data for next page (for np0 data)
*
* @param[in] serdesNum              - serdes number
* @param[in] msg                    - message
* @param[in] ack2                   - acknowledge 2
* @param[in] mp                     - acknowledge or data page
* @param[in] ack                    - acknowledge
* @param[in] np                     - np
* @param[in] nullPage               - set null page or next page
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
*/
static GT_STATUS mvAnConsortiumAdvertiseNextPage0Set(GT_U32 serdesNum, GT_U32 msg, GT_U32 ack2, GT_U32 mp, GT_U32 ack, GT_U32 np,GT_BOOL nullPage)
{
    /* in consortium, the first 16 bits are page 0, mp is the 14 bit, ack is the 15 bit, and np is the 16 bit
       mp - 1 when it's acknowledge page (page without data and only use to set the protocol)
            0 when it's data page that include params to set speed and mode
       ack - other opition in this protocol for acknowledge, we don't using it (we always set it to 0) but still keeping the place to change it
       np  - when it's acknowledge page (mp = 1), np is 0 to send null, and 1 to send ack. when mp is 0, send 0 */

    GT_U32 data = ((msg & AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK) |
                   ((ack2 << AP_INT_REG_802_3_NEXT_PG_TX_REG_ACK2_SHIFT) & AP_INT_REG_802_3_NEXT_PG_TX_REG_ACK2_MASK)|
                   ((mp << AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_SHIFT) & AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK)|
                   ((ack << AP_INT_REG_802_3_NEXT_PG_TX_REG_ACK_SHIFT) & AP_INT_REG_802_3_NEXT_PG_TX_REG_ACK_MASK)|
                   ((np << AP_INT_REG_802_3_NEXT_PG_TX_REG_NP_SHIFT) & AP_INT_REG_802_3_NEXT_PG_TX_REG_NP_MASK));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0329, data, NULL));

    if (nullPage == GT_TRUE)
    {
         CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x507, 0x2, NULL));
    }
    else /*data page*/
    {
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x507, 0x1, NULL));
    }

    return GT_OK;
}

/**
* @internal mvAnConsortiumAcknowledgeNp
* @endinternal
*
* @brief sending acknowledge next page
*
* @param[in] serdesNum              - serdes number
* @param[in] ack2                   - acknowledge 2
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
*/
static GT_STATUS mvAnConsortiumAcknowledgeNp(GT_U32 serdesNum, GT_U8 ack2)
{
    /* on Acknowledge page, page 1 set to be 0x353, and page 2 set to be 0x4df. this show ack in the protocol */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0429, AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_OUI_TAGGED_FORM_CNS, NULL));
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0529, AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_OUI_TAGGED_FORM_CNS, NULL));
    return mvAnConsortiumAdvertiseNextPage0Set(serdesNum, AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_FORM_CNS, ack2, 1, 0, 1,GT_FALSE);
}

/**
* @internal mvAnConsortiumNullPageTransmitter
* @endinternal
*
* @brief sending null page
*
* @param[in] serdesNum              - serdes number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
*/
static GT_STATUS mvAnConsortiumNullPageTransmitter(GT_U32 serdesNum)
{
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0429, 0, NULL));
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0529, 0, NULL));
    return mvAnConsortiumAdvertiseNextPage0Set(serdesNum, AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_FORM_NULL_CNS, 0, 1, 0, 0,GT_TRUE);
}

/**
* @internal mvAnConsortiumNextPageReceiver
* @endinternal
*
* @brief reading the next page from the peer
*
* @param[in] serdesNum              - serdes number
*
* @param[out] nextPage0                   - next page 0
* @param[out] nextPage1                   - next page 1
* @param[out] nextPage2                   - next page 2
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
*/
static GT_STATUS mvAnConsortiumNextPageReceiver(GT_U32 serdesNum, GT_32 *nextPage0, GT_32 *nextPage1, GT_32 *nextPage2)
{
    *nextPage0 = 0;
    *nextPage1 = 0;
    *nextPage2 = 0;

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0729, 0x0, nextPage0));
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0729, 0x1, nextPage1));
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0729, 0x2, nextPage2));

    return GT_OK;
}

/**
* @internal mvAnConsortiumResolutionSet
* @endinternal
*
* @brief set the data we recevie from peer.
*
* @param[in] portIndex             - port index
* @param[in] np0                   - next page 0
* @param[in] np1                   - next page 1
* @param[in] np2                   - next page 2
*
* @param[out] resolutionDone       - boolean to check resolution status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
*/
static GT_STATUS mvAnConsortiumResolutionSet(GT_U8 portIndex, GT_U16 np0, GT_U16 np1, GT_U16 np2, GT_BOOL *resolutionDone)
{
    MV_HWS_AN_SM_INFO *anSm = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);

    AN_PRINT_MAC(("Consortium partner resolution portIndex:%d  capability 0x%x\n", portIndex,anSm->capability));

    /* Set the speed. In case of multi speed advertisement, need to set the highest speed*/
    if ((AP_CTRL_400GBase_KR8_GET(anSm->capability) == 1) &&
        ((np2 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_400G_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_400G_MASK)){
        /* 400G is selected */
        AP_ST_HCD_TYPE_SET(anSm->hcdStatus, Port_400GBase_KR8); /* Set FC Tx */
        /*AN_PRINT_MAC(("Consortium partner resolution portIndex:%d  Port_400GBASE_KR8_CONSORTIUM\n", portIndex));*/
    }else if ((AP_CTRL_400GBase_CR8_GET(anSm->capability) == 1) &&
        ((np2 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_400G_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_400G_MASK)){
        /* 400G is selected */
        AP_ST_HCD_TYPE_SET(anSm->hcdStatus, Port_400GBase_CR8); /* Set FC Tx */
        /*AN_PRINT_MAC(("Consortium partner resolution portIndex:%d  Port_400GBASE_CR8_CONSORTIUM\n", portIndex));*/
    }else if ((AP_CTRL_50GBase_KR2_CONSORTIUM_GET(anSm->capability) == 1) &&
        ((np1 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_KR2_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_KR2_MASK)){
        /* 50G is selected */
        AP_ST_HCD_TYPE_SET(anSm->hcdStatus, Port_50GBASE_KR2_C); /* Set FC Tx */
        /*AN_PRINT_MAC(("Consortium partner resolution portIndex:%d  Port_50GBASE_KR2_CONSORTIUM\n", portIndex));*/
    }else if ((AP_CTRL_50GBase_CR2_CONSORTIUM_GET(anSm->capability) == 1) &&
        ((np1 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_CR2_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_CR2_MASK)){
        /* 50G is selected */
        AP_ST_HCD_TYPE_SET(anSm->hcdStatus, Port_50GBASE_CR2_C); /* Set FC Tx */
        /*AN_PRINT_MAC(("Consortium partner resolution portIndex:%d  Port_50GBASE_CR2_CONSORTIUM\n", portIndex));*/
    }else if ((AP_CTRL_40GBase_KR2_GET(anSm->capability) == 1) &&
        ((np2 & (AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_40G_MASK << mvHws40GAnResolutionBitShift)) == (AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_40G_MASK <<mvHws40GAnResolutionBitShift))){
        /* 40G is selected */
        AP_ST_HCD_TYPE_SET(anSm->hcdStatus, Port_40GBase_KR2); /* Set FC Tx */
        /*AN_PRINT_MAC(("Consortium partner resolution portIndex:%d  Port_40GBASE_KR2_CONSORTIUM\n", portIndex));*/
    }else if ((AP_CTRL_25GBase_KR_CONSORTIUM_GET(anSm->capability) == 1) &&
        ((np1 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_KR1_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_KR1_MASK)){
        /* 25G is selected */
        AP_ST_HCD_TYPE_SET(anSm->hcdStatus, Port_25GBASE_KR_C); /* Set FC Tx */
        /*AN_PRINT_MAC(("Consortium partner resolution portIndex:%d  Port_25GBASE_KR1_CONSORTIUM\n", portIndex));*/
    }else if ((AP_CTRL_25GBase_CR_CONSORTIUM_GET(anSm->capability) == 1) &&
        ((np1 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_CR1_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_CR1_MASK)){
        /* 25G is selected */
        AP_ST_HCD_TYPE_SET(anSm->hcdStatus, Port_25GBASE_CR_C); /* Set FC Tx */
        /*AN_PRINT_MAC(("Consortium partner resolution portIndex:%d  Port_25GBASE_CR1_CONSORTIUM\n", portIndex));*/
    }else {/* there is no match in consortium */
        *resolutionDone = GT_TRUE;
        return GT_OK;
    }
    AP_ST_HCD_CONSORTIUM_RES_SET(anSm->hcdStatus, 1);

    AN_PRINT_MAC(("Consortium partner resolution portIndex:%d  options:0x%x\n", portIndex,anSm->options));
    /* set the fec mode */
    if (((AP_CTRL_RS_FEC_REQ_CONSORTIUM_GET(anSm->options) == 1) ||
         ((np2 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_RS_REQ_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_RS_REQ_MASK))&&
        (AP_CTRL_RS_FEC_ABIL_CONSORTIUM_GET(anSm->options) == 1) &&
        ((np2 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_RS_ABIL_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_RS_ABIL_MASK)){
        /* RS FEC is selected */
        AP_ST_HCD_FEC_RES_SET(anSm->hcdStatus,AP_ST_HCD_FEC_RES_RS);
        /*AN_PRINT_MAC(("Consortium partner resolution portIndex:%d  AP_ST_HCD_FEC_RES_RS\n", portIndex));*/
    } else if (((AP_CTRL_FC_FEC_REQ_CONSORTIUM_GET(anSm->options) == 1) ||
                ((np2 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_FC_REQ_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_FC_REQ_MASK))&&
               (AP_CTRL_FC_FEC_ABIL_CONSORTIUM_GET(anSm->options) == 1) &&
               ((np2 & AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_FC_ABIL_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_FC_ABIL_MASK)){
        /* BASE-R FEC is selected */
        AP_ST_HCD_FEC_RES_SET(anSm->hcdStatus, AP_ST_HCD_FEC_RES_FC);
        /*AN_PRINT_MAC(("Consortium partner resolution portIndex:%d  AP_ST_HCD_FEC_RES_BASE_R\n", portIndex));*/
    } /*else {
        AN_PRINT_MAC(("Consortium partner resolution portIndex:%d  NO FEC\n", portIndex));
    }*/

    *resolutionDone = GT_TRUE;
    return GT_OK;
}

/**
* @internal mvAnConsortiumAdvertiseNextPage2Set
* @endinternal
*
* @brief set the data for next page (for np2 data)
*
* @param[in] serdesNum             - serdes number
* @param[in] modeVector            - vector that hold the data
* @param[in] fecAbility            - fec ability
* @param[in] fecEnable             - fec enable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
*/
static GT_STATUS mvAnConsortiumAdvertiseNextPage2Set(GT_U32 serdesNum, GT_U32 modeVector, GT_U8 fecAbility, GT_U8 fecEnable)
{
GT_U32 data = (fecEnable << 2) + fecAbility;
    data = data << AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_SHIFT;
    /* add 400G bit to the data if configure */
    data = data + ((AP_CTRL_400GBase_CR8_GET(modeVector) << AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_400G_SHIFT)|
                   (AP_CTRL_400GBase_KR8_GET(modeVector) << AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_400G_SHIFT)|
                   (AP_CTRL_40GBase_KR2_GET(modeVector) << mvHws40GAnResolutionBitShift));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0529,
                  ((data) & ((AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_FEC_MASK) | (AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_400G_MASK) | (AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_40G_MASK << mvHws40GAnResolutionBitShift))), NULL));

    return GT_OK;
}

/**
* @internal mvAnConsortiumAdvertiseNextPage1Set
* @endinternal
*
* @brief set the data for next page (for np1 data)
*
* @param[in] serdesNum             - serdes number
* @param[in] modeVector            - vector that hold the data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
*/
static GT_STATUS mvAnConsortiumAdvertiseNextPage1Set(GT_U32 serdesNum, GT_U32 modeVector)
{

    GT_U32 data = ((AP_CTRL_ADV_50G_CONSORTIUM_GET(modeVector) << AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_SHIFT)|
                   (AP_CTRL_ADV_25G_CONSORTIUM_GET(modeVector) << AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_SHIFT));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(anDevNum, anPortGroup, serdesNum, 0x0429,
                     (data & (AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_25G_MASK | AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_50G_MASK)), NULL));
    return GT_OK;
}

/** mvAnHcdIsPartnerSupportConsortium
*******************************************************************************/
static GT_BOOL mvAnHcdIsPartnerSupportConsortium(GT_U16 np0, GT_U16 np1, GT_U16 np2)
{

    if ((np2 == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_OUI_TAGGED_FORM_CNS) &&
        (np1 == AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_OUI_TAGGED_FORM_CNS)  &&
        ((np0 & (AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK | AP_INT_REG_802_3_NEXT_PG_TX_REG_NP_MASK | AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK)) ==
            (AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_FORM_CNS | AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK | AP_INT_REG_802_3_NEXT_PG_TX_REG_NP_MASK))){
        return GT_TRUE;
    } else {
        return GT_FALSE;
    }
}

/**
* @internal mvAnConsortiumPortResolution
* @endinternal
*
* @brief Port resolution for consortium speeds.
*
* @param[in] portIndex             - port index
*
* @param[out] resolutionDone       - boolean to check resolution status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
*/
static GT_STATUS mvAnConsortiumPortResolution(GT_U8 portIndex, GT_BOOL *resolutionDone)
{
    MV_HWS_AN_SM_INFO  *anSm = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    GT_U32             serdesNum = AP_CTRL_LANE_GET(anSm->ifNum);
    /*GT_U8              ack2;*/
    /*in consortium, each page is divide to 3 pages that are part from big page*/
    GT_32              nextPage0 = 0,
                       nextPage1 = 0,
                       nextPage2 = 0;
    GT_U32             statusNP = 0;
    GT_BOOL supported;
     static GT_U32 consortiumSupported = 0;

    if (AP_CTRL_ADV_CONSORTIUM_GET(anSm->capability) == 0)
    {
        /* we don't support consortium - need to send NULL next page*/
        mvAnConsortiumNullPageTransmitter(serdesNum);
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, portIndex));
        *resolutionDone = GT_TRUE;
        return GT_OK;
    }

    switch (anSm->status)
    {
    case AN_PORT_RESOLUTION_IN_PROGRESS:

        /* clear consortium bit */
        consortiumSupported &= ~(0x1 << (portIndex));
        /* start of resolution*/
        mvAnConsortiumNextPageReceiver(serdesNum, &nextPage0, &nextPage1, &nextPage2);
        supported = mvAnHcdIsPartnerSupportConsortium(nextPage0, nextPage1, nextPage2);
        if (supported)
            consortiumSupported |= (0x1 << (portIndex));
        else
            consortiumSupported &= ~(0x1 << (portIndex));

        AN_PRINT_MAC(("ResolutionWaitNP RESOLUTION_IN_PROGRESS portIndex:%d  next page:0x%x, 0x%x, 0x%x\n",portIndex, nextPage0, nextPage1, nextPage2));
        /* prepare next page and send it (ack2 = 0) */
        mvAnConsortiumAcknowledgeNp(serdesNum,0);

        /*anSm->status = AN_PORT_AN_RESOLUTION_CHK_CONSORTIUM;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_AN_RESOLUTION_CHK_CONSORTIUM);
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, portIndex));
        break;

    case AN_PORT_AN_RESOLUTION_CHK_CONSORTIUM:
        statusNP = 0;
        /*wait until next page is received*/
        mvAnSerdesStatus(serdesNum, AN_SERDES_AN_NEXT_PAGE_RX, &statusNP);
        if (statusNP != 0x1)
        {
            break;
        }

        /* read the next page that has been received */
         mvAnConsortiumNextPageReceiver(serdesNum, &nextPage0, &nextPage1, &nextPage2);
        if ((consortiumSupported & (0x1 << (portIndex))) == 0)
        {
            supported = mvAnHcdIsPartnerSupportConsortium(nextPage0, nextPage1, nextPage2);
            if (supported)
                consortiumSupported |= (0x1 << (portIndex));
            else
                consortiumSupported &= ~(0x1 << (portIndex));
        }
        else
        {
            if (((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_UNFORM_CNS) &&
                ((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK) == 0)){
                /* read pertner fec & speed*/
                mvAnConsortiumResolutionSet(portIndex, nextPage0, nextPage1, nextPage2, resolutionDone);
            }
        }
        AN_PRINT_MAC(("ResolutionWaitNP CHK_CONSORTIUM portIndex:%d partner next page:0x%x, 0x%x, 0x%x \n",portIndex, nextPage0, nextPage1, nextPage2));
#if 0
        if ((consortiumSupported & (0x1 << (portIndex))) == 0) {
            /* partner don't support consortium - need to send NULL next page*/
            mvAnConsortiumNullPageTransmitter(serdesNum);
            AN_PRINT_MAC(("ResolutionWaitNP portIndex:%d  send null next page\n",portIndex));
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_AN_RESOLUTION_NO_MORE_PAGES);
            break;
        }

        /* check if  the parthner is advertising consortium*/
        /* page 2 must be 0x4df, and page 1 must be 0x353. this show it's ack page
           page 0 masking the peer mp,ack and np to see if the peer support consortium and set ack2 = 1, or not and set ack2 = 0*/
        if ((nextPage2 == AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_OUI_TAGGED_FORM_CNS) &&
            (nextPage1 == AP_INT_REG_802_3_NEXT_PG_TX_CODE_0_15_OUI_TAGGED_FORM_CNS)  &&
            ((nextPage0 & (AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK |
                           AP_INT_REG_802_3_NEXT_PG_TX_REG_NP_MASK |
                           AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK)) ==
                          (AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_FORM_CNS |
                           AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK |
                           AP_INT_REG_802_3_NEXT_PG_TX_REG_NP_MASK)))
        {
            ack2 = 1; /* yes*/
        }
        else
        {
            ack2 = 0; /* no */
        }
        /* prepare next page and send it */
        mvAnConsortiumAcknowledgeNp(serdesNum, ack2);

        /*anSm->status = AN_PORT_AN_RESOLUTION_CHK_ACK2;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_AN_RESOLUTION_CHK_ACK2);
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, portIndex));
        break;

    case AN_PORT_AN_RESOLUTION_CHK_ACK2:
        statusNP = 0;
        /*wait until next page is received*/
        mvAnSerdesStatus(serdesNum, AN_SERDES_AN_NEXT_PAGE_RX, &statusNP);
        if (statusNP != 0x1)
        {
            break;
        }

        /* read the next page that has been received */
        mvAnConsortiumNextPageReceiver(serdesNum, &nextPage0, &nextPage1, &nextPage2);

        if (nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_ACK2_MASK) /* check partner ack2 is set */
#endif

        {
            /* send partner all ports abilities fec/speed */
            mvAnConsortiumAdvertiseNextPage2Set(serdesNum,
                               AP_CTRL_ADV_ALL_GET(anSm->capability),
                              (GT_U8)AP_CTRL_FEC_ABIL_CONSORTIUM_GET(anSm->options),
                              (GT_U8)AP_CTRL_FEC_REQ_CONSORTIUM_GET(anSm->options));

            mvAnConsortiumAdvertiseNextPage1Set(serdesNum, AP_CTRL_ADV_ALL_GET(anSm->capability));

            mvAnConsortiumAdvertiseNextPage0Set(serdesNum, AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_UNFORM_CNS, 1 /*ack2*/, 0 /*mp*/, 0 /*ack*/, 0 /*np*/,GT_FALSE);

            /* check partner abilities */
            if (((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_UNFORM_CNS) &&
                ((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK) == 0))
            {
                /* read pertner fec & speed*/
                mvAnConsortiumResolutionSet(portIndex, nextPage0, nextPage1, nextPage2,resolutionDone);
            }
        }
#if 0
        else /* parthner ack2 is not set, send null page*/
        {
            mvAnConsortiumNullPageTransmitter(serdesNum);
        }
#endif
        /*anSm->status = AN_PORT_AN_RESOLUTION_NO_MORE_PAGES;*/
        AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_AN_RESOLUTION_NO_MORE_PAGES);
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG(anSm->state, anSm->status, portIndex));
        break;

        case AN_PORT_AN_RESOLUTION_NO_MORE_PAGES:
        /*wait until next page is received*/
        statusNP = 0;
        mvAnSerdesStatus(serdesNum, AN_SERDES_AN_NEXT_PAGE_RX, &statusNP);
        if (statusNP != 0x1)
        {
            break;
        }

        /* read the next page that has been received */
        mvAnConsortiumNextPageReceiver(serdesNum, &nextPage0, &nextPage1, &nextPage2);

        /* check partner abilities */
        if (((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_MASK) == AP_INT_REG_802_3_NEXT_PG_TX_REG_OUI_TAGGED_UNFORM_CNS) &&
            ((nextPage0 & AP_INT_REG_802_3_NEXT_PG_TX_REG_MP_MASK) == 0)){
            /* read pertner fec & speed*/
            mvAnConsortiumResolutionSet(portIndex, nextPage0, nextPage1, nextPage2, resolutionDone);
        }

        /* done, send null to close this section */
        mvAnConsortiumNullPageTransmitter(serdesNum);

        return GT_OK;
    }

    return GT_OK;
}
#endif /*#ifdef CONSORTIUM_SUPPORT*/
static GT_STATUS mvAnConvertAdvertisement(GT_U8 portIndex, MV_HWA_AP_PORT_MODE *apPortMode, GT_U32 *fecType,  MV_HWS_PORT_FEC_MODE *portFecType)
{
    MV_HWS_AN_SM_INFO  *anSm          = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    GT_U8 rsFecEnable = 0, fcFecEnable = 0;

    if(AP_CTRL_1000Base_KX_GET(anSm->capability))
    {
        anSm->portMode = _1000Base_X;
        *apPortMode = Port_1000Base_KX;
    }
    else if(AP_CTRL_10GBase_KX4_GET(anSm->capability))
    {
        anSm->portMode = _10GBase_KX4;
        *apPortMode = Port_10GBase_KX4;
    }
    else if(AP_CTRL_10GBase_KR_GET(anSm->capability))
    {
        anSm->portMode = _10GBase_KR;
        *apPortMode = Port_10GBase_R;
    }
    else if(AP_CTRL_40GBase_KR4_GET(anSm->capability))
    {
        anSm->portMode = _40GBase_KR4;
        *apPortMode = Port_40GBase_R;
    }
    else if(AP_CTRL_40GBase_CR4_GET(anSm->capability))
    {
        anSm->portMode = _40GBase_CR4;
        *apPortMode = Port_40GBASE_CR4;
    }
    else if(AP_CTRL_100GBase_KR4_GET(anSm->capability))
    {
        anSm->portMode = _100GBase_KR4;
        *apPortMode = Port_100GBASE_KR4;
    }
    else if(AP_CTRL_100GBase_CR4_GET(anSm->capability))
    {
        anSm->portMode = _100GBase_CR4;
        *apPortMode = Port_100GBASE_CR4;
    }
    else if(AP_CTRL_25GBase_KR1S_GET(anSm->capability))
    {
        anSm->portMode = _25GBase_KR_S;
        *apPortMode = Port_25GBASE_KR_S;
    }
    else if(AP_CTRL_25GBase_CR1S_GET(anSm->capability))
    {
        anSm->portMode = _25GBase_CR_S;
        *apPortMode = Port_25GBASE_CR_S;
    }
    else if(AP_CTRL_25GBase_KR1_GET(anSm->capability))
    {
        anSm->portMode = _25GBase_KR;
        *apPortMode = Port_25GBASE_KR;
    }
    else if(AP_CTRL_25GBase_CR1_GET(anSm->capability))
    {
        anSm->portMode = _25GBase_CR;
        *apPortMode = Port_25GBASE_CR;
    }
    else if(AP_CTRL_50GBase_KR1_GET(anSm->capability))
    {
        anSm->portMode = _50GBase_KR;
        *apPortMode = Port_50GBase_KR;
    }
    else if(AP_CTRL_50GBase_CR1_GET(anSm->capability))
    {
        anSm->portMode = _50GBase_CR;
        *apPortMode = Port_50GBase_CR;
    }
    else if(AP_CTRL_100GBase_KR2_GET(anSm->capability))
    {
        anSm->portMode = _100GBase_KR2;
        *apPortMode = Port_100GBase_KR2;
    }
    else if(AP_CTRL_100GBase_CR2_GET(anSm->capability))
    {
        anSm->portMode = _100GBase_CR2;
        *apPortMode = Port_100GBase_CR2;
    }
    else if(AP_CTRL_200GBase_KR4_GET(anSm->capability))
    {
        anSm->portMode = _200GBase_KR4;
        *apPortMode = Port_200GBase_KR4;
    }
    else if(AP_CTRL_200GBase_CR4_GET(anSm->capability))
    {
        anSm->portMode = _200GBase_CR4;
        *apPortMode = Port_200GBase_CR4;
    }
    else if(AP_CTRL_200GBase_KR8_GET(anSm->capability))
    {
        anSm->portMode = _200GBase_KR8;
        *apPortMode = Port_200GBase_KR8;
    }
    else if(AP_CTRL_200GBase_CR8_GET(anSm->capability))
    {
        anSm->portMode = _200GBase_CR8;
        *apPortMode = Port_200GBase_CR8;
    }
    else if(AP_CTRL_400GBase_KR8_GET(anSm->capability))
    {
        anSm->portMode = _400GBase_KR8;
        *apPortMode = Port_400GBase_KR8;
    }
    else if(AP_CTRL_400GBase_CR8_GET(anSm->capability))
    {
        anSm->portMode = _400GBase_CR8;
        *apPortMode = Port_400GBase_CR8;
    }
    else if(AP_CTRL_40GBase_KR2_GET(anSm->capability))
    {
        anSm->portMode = _40GBase_KR2;
        *apPortMode = Port_40GBase_KR2;
    }
    else if(AP_CTRL_25GBase_KR_CONSORTIUM_GET(anSm->capability))
    {
        anSm->portMode = _25GBase_KR_C;
        *apPortMode = Port_25GBASE_KR_C;
    }
    else if(AP_CTRL_25GBase_CR_CONSORTIUM_GET(anSm->capability))
    {
        anSm->portMode = _25GBase_CR_C;
        *apPortMode = Port_25GBASE_CR_C;
    }
    else if(AP_CTRL_50GBase_KR2_CONSORTIUM_GET(anSm->capability))
    {
        anSm->portMode = _50GBase_KR2_C;
        *apPortMode = Port_50GBASE_KR2_C;
    }
    else if(AP_CTRL_50GBase_CR2_CONSORTIUM_GET(anSm->capability))
    {
        anSm->portMode = _50GBase_CR2_C;
        *apPortMode = Port_50GBASE_CR2_C;
    }
    else
    {
        return GT_FAIL;
    }

       /*convert Fec param*/
    if (HWS_PAM4_MODE_CHECK(anSm->portMode)|| (_200GBase_CR8 == anSm->portMode) ||
        (_200GBase_KR8 == anSm->portMode) ||
        (HWS_100G_R4_MODE_CHECK(anSm->portMode) && (!AN_CTRL_RX_TRAIN_GET(anSm->options))))
    {
        rsFecEnable = 1;
    }
    else if ((anSm->portMode == _1000Base_X) || (anSm->portMode == _10GBase_KX4) || (anSm->portMode == _10GBase_KR) ||
        (anSm->portMode == _40GBase_KR4) || (anSm->portMode == _40GBase_CR4))
    {
        if (AP_CTRL_FEC_REQ_GET(anSm->options))
        {
            fcFecEnable = 1;
        }
    }
    else if (AP_CTRL_ADV_CONSORTIUM_GET(anSm->capability))
    {
        if(AP_CTRL_RS_FEC_REQ_CONSORTIUM_GET(anSm->options))
        {
            rsFecEnable = 1;
        }
        if (AP_CTRL_FC_FEC_REQ_CONSORTIUM_GET(anSm->options))
        {
            fcFecEnable = 1;
        }
    }
    else
    {
        if(AP_CTRL_FEC_RS_REQ_GET(anSm->options))
        {
            rsFecEnable = 1;
        }
        if (AP_CTRL_FEC_FC_REQ_GET(anSm->options))
        {
            fcFecEnable = 1;
        }
    }

    if (fcFecEnable && rsFecEnable)
    {
        return GT_FAIL;
    }
    else if (fcFecEnable)
    {
        *fecType = AP_ST_HCD_FEC_RES_FC;
        *portFecType = FC_FEC;
    }
    else if (rsFecEnable)
    {
        *fecType = AP_ST_HCD_FEC_RES_RS;
        if (HWS_PAM4_MODE_CHECK(anSm->portMode)|| (_200GBase_CR8 == anSm->portMode) || (_200GBase_KR8 == anSm->portMode) )
        {
            *portFecType = RS_FEC_544_514;
        }
        else
        {
            *portFecType = RS_FEC;
        }
    }
    else
    {
        *fecType = AP_ST_HCD_FEC_RES_NONE;
        *portFecType = FEC_OFF;
    }

    return GT_OK;

}
