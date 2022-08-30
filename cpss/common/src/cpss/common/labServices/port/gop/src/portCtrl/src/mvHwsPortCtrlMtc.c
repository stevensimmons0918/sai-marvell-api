

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
* @file mvHwsPortCtrlMtc.c
*
* @brief Port Control Maintanance Task for Raven
*
* @version   1
********************************************************************************
*/
#include <mvHwsPortCtrlInc.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <mvHwsServiceCpuInt.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <mvHwsPortCtrlCommonEng.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <mvHwsPortCtrlLog.h>


/* Default values for AP Device and Port Group */
#ifdef MV_SERDES_AVAGO_IP
#include "cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h"
#endif /* MV_SERDES_AVAGO_IP */
#include "mvHwsPortCtrlMtc.h"

#if defined( MV_PORT_MTC_TASK) || defined(MV_PORT_MTC_WO_TASK)

/* Globals */
#define MV_PORT_CTRL_MAX_MTC_PORT_NUM 16
#define MV_PORT_CTRL_MTC_BROADCAST    0

#ifdef MV_PORT_MTC_TASK
extern GT_U32 mvPortCtrlAnStopDebugFlag;

/**
* @internal mvHwsPortMaintananceRoutine function
* @endinternal
*
* @brief execute the Routine (ACTIVE -> IDLE->)
*
*/
void mvHwsPortMtcRoutine(void* pvParameters)
{
    /* prevent acess of L1 properties before host is finished to load L1 FW's */
    mvPortCtrlMtcLock();
    for( ;; )
    {
        if(!mvPortCtrlAnStopDebugFlag)
        {
            /* Active */
            mvPortCtrlPortMaintananceActive();
        }
        /* delay */
        mvPortCtrlProcessDelay(5000);
    }
}
#endif

/**
* @internal mvPortCtrlPortMaintananceActive function
* @endinternal
*
* @brief   Port Maintanance task state execution
*         - Scan all ports and call port execution function
*/
void mvPortCtrlPortMaintananceActive(void)
{
    GT_32 temperature = 0;
    GT_U8 portIndex;
    static GT_32 lastTemperature = 0;

#ifdef MV_PORT_MTC_WO_TASK
    static GT_U32 lastMiliSecondsStamp=0;
    GT_U32 miliSeconds=0;

    miliSeconds = mvPortCtrlCurrentTs();
    if (miliSeconds > lastMiliSecondsStamp +5000){
        /* Active */
        miliSeconds = lastMiliSecondsStamp;
    }
    else
    {
        return;
    }
#endif

    if ( GT_OK == (mvHwsPortTemperatureGet(0, 0, 0, &temperature)))
    {
        if (ABS(lastTemperature -temperature) > 2   )
        {
            lastTemperature = temperature;
#if MV_PORT_CTRL_MTC_BROADCAST
            mvPortMaintananceBroadcastTemperature(temperature);
#else
            for (portIndex = 0; portIndex < MV_PORT_CTRL_MAX_MTC_PORT_NUM; portIndex++)
            {
                /* Currently the only operation of port maintanance task is to transmit temperature to the SD's */
                mvPortMaintananceTransmitTemperature(portIndex, temperature);
            }
#endif
        }
    }
    else {
        /* use to dump errors for debug purposes */
        mvPcPrintf("mvPortCtrlPortMaintananceActive failed\n");
    }
}

/**
* @internal mvPortMaintananceTransmitTemperature function
* @endinternal
*
* @brief   check if adaptive is running and transmit temperature
*
* @param[in] portIndex                 - number of physical port
*
* @retval GT_OK                    - on success/delete
* */
GT_STATUS mvPortMaintananceTransmitTemperature
(
    IN GT_U8 portIndex,
    IN GT_32 temperature
)
{
    /* SerdesNum is not actually equals to the portIndex, Having said that we don't care about this because of
       the serdes muxing so this is valid */
    GT_U8 serdesNum = portIndex;
    GT_STATUS rc;
    /* inside the function check if adaptive is running on the serdes, if yes trasnmit temperature to SerDes */
    rc = mvHwsAvagoSerdesTemperatureTransmitSet(0,0,serdesNum,temperature);
    if (rc != GT_OK) {
        /* using CM3 print for debug, since it's a CM3 task do not return error */
        mvPcPrintf("mvHwsAvagoSerdesTemperatureTransmitSet serdesNum %d\n",serdesNum);
    }
    return GT_OK;
}
#if MV_PORT_CTRL_MTC_BROADCAST
/**
* @internal mvPortMaintananceBroadcastTemperature function
* @endinternal
*
* @brief   broadcast temperature
*
* @param[in] temperature                 - temperature to
*       broadcast
*
* @retval GT_OK                    - on success/delete
* */
GT_STATUS mvPortMaintananceBroadcastTemperature
(
    IN GT_32 temperature
)
{
    GT_STATUS rc;
    rc = mvHwsAvagoSpicoInterrupt(0, 0, 0, MV_HWS_AVAGO_SPICO_BRDCST_ADDR, 0x2C, 0x2800, NULL);
    if (rc != GT_OK)
    {
        mvPcPrintf("Failed rc=%d. mvHwsAvagoSpicoInterrupt sbusAddr:0x%x, interruptCode:0x%x ,interruptData:0x%x\n",
                   rc, MV_HWS_AVAGO_SPICO_BRDCST_ADDR, 0x2C, 0x2800);
    }
    rc = mvHwsAvagoSpicoInterrupt(0, 0, 0, MV_HWS_AVAGO_SPICO_BRDCST_ADDR, 0x6C, temperature, NULL);
    if (rc != GT_OK) {
        mvPcPrintf("Failed rc=%d. mvHwsAvagoSpicoInterrupt sbusAddr:0x%x, interruptCode:0x%x ,interruptData:0x%x\n",
                   rc, MV_HWS_AVAGO_SPICO_BRDCST_ADDR, 0x6C, 0);
    }

    return GT_OK;
}
#endif /* MV_PORT_CTRL_MTC_BROADCAST */
#endif /* MV_PORT_MTC_TASK */


