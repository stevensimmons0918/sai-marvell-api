/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    *   Neither the name of Marvell nor the names of its contributors may be
        used to endorse or promote products derived from this software without
        specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
/**
********************************************************************************
* @file watchdog.c
*
* @brief Watchdog mechanism implementation.
*
* @version   1
********************************************************************************
*/

#include <stdio.h>
#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <printf.h>
#include <mv_services.h>

#include "watchdog.h"

#ifdef SHM_UART
  #define WATCHDOG_POINTER_FLAG_ADDR (SHM_UART_BASE +2*SHM_UART_SIZE)
#endif

extern MV_U32 AC5_REG_BASE_ADDRESS;

/*
 * TIMER_RATE_HZ is hard coded here to be 12500000
 * which is the value in AC5 board
*/
#define TIMER_RATE_HZ                12500000
#define TIMER_NUMBER_OF_TICKS        100
#define WATCHDOG_ENABLE              (AC5_REG_BASE_ADDRESS + 0x54)
#define TIMER_INTERRUPT_CAUSE        (AC5_REG_BASE_ADDRESS + 0x144)
#define TIMER1_BASE                  (AC5_REG_BASE_ADDRESS + 0x400)
#define TIMER1_RATIO_REG             (AC5_REG_BASE_ADDRESS + 0x350)
#define TIMER1_MAX_VAL_LSB_REG       (TIMER1_BASE)
#define TIMER1_MAX_VAL_MSB_REG       (TIMER1_BASE + 0x04)
#define TIMER1_CNT_VAL_LSB_REG       (TIMER1_BASE + 0x08)
#define TIMER1_CNT_VAL_MSB_REG       (TIMER1_BASE + 0x0c)
#define TIMER1_START_REG             (TIMER1_BASE + 0x10)
#define TIMER1_CNRTL_REG             (TIMER1_BASE + 0x14)

static MV_U32 configWdTaskPeriodTimeMs;
static MV_U32 isStopRequested;

/**
* @internal srvCpuWatchdogInit function
* @endinternal
*
* @brief  init Watchdog and timer values
*
* @param MV_U32 timerExpirationMs - watchdog timer expiration in milliseconds
*
* @retval none
*
*/
void srvCpuWatchdogInit(MV_U32 timerExpirationMs)
{
    MV_U32 configRatioValue = timerExpirationMs * ((TIMER_RATE_HZ/1000)/TIMER_NUMBER_OF_TICKS); /* divide by 1000 in order to get value in millisecond */
#ifdef SHM_UART
    MV_U32 *wdExpired = (MV_U32*)WATCHDOG_POINTER_FLAG_ADDR;
    *wdExpired = MV_FALSE;
#endif

    /* Checking if watchdog asserted */
    if( srvCpuRegRead(TIMER_INTERRUPT_CAUSE) == 0x10 )
    {
#ifdef SHM_UART
        *wdExpired = MV_TRUE;
#endif
        printf("Watchdog interrupt is asserted!\n");
    }

    srvCpuRegWrite( WATCHDOG_ENABLE, 0xffff0001); /* enable Watchdog */
    srvCpuRegWrite(TIMER1_CNRTL_REG, 4); /* stop timer */
    srvCpuRegWrite(TIMER1_CNT_VAL_LSB_REG, 0x0); /* clear cnt_val LSB */
    srvCpuRegWrite(TIMER1_CNT_VAL_MSB_REG, 0x0); /* clear cnt_val MSB */
    srvCpuRegWrite(TIMER1_RATIO_REG, configRatioValue);
    srvCpuRegWrite(TIMER1_MAX_VAL_LSB_REG, TIMER_NUMBER_OF_TICKS);
    srvCpuRegWrite(TIMER1_MAX_VAL_MSB_REG, 0x0);
}

/**
* @internal srvCpuWatchdogStart function
* @endinternal
*
* @brief  Start Watchdog by start running the timer
*
* @param none
*
* @retval none
*
*/
void srvCpuWatchdogStart()
{
    srvCpuRegWrite(TIMER1_CNRTL_REG, 8); /* reset timer */
    srvCpuRegWrite(TIMER1_START_REG, 1); /* start timer */
    isStopRequested = MV_FALSE;
}

/**
* @internal srvCpuWatchdogStop function
* @endinternal
*
* @brief  Disable Watchdog by stopping the timer
*
* @param none
*
* @retval none
*
*/
void srvCpuWatchdogStop()
{
    srvCpuRegWrite(TIMER1_CNRTL_REG, 8); /* reset timer */
    isStopRequested = MV_TRUE;
}

/**
* @internal watchdogCreateTask function
* @endinternal
*
* @brief  create watchdog task
*
* @param MV_U32 wdTaskPeriodTimeMs -  delay in the task in milliseconds,
*                                     effects on the period of time to clear the watchdog timer before timer expired
*
* @retval none
*
*/
void watchdogCreateTask(MV_U32 wdTaskPeriodTimeMs)
{
    configWdTaskPeriodTimeMs = wdTaskPeriodTimeMs;
    /* Create the Watchdog task */
    xTaskCreate( prvWatchDogTask, (signed portCHAR *)"WD",
                configMINIMAL_STACK_SIZE, NULL,
                tskIDLE_PRIORITY + 2, NULL );
}

void prvWatchDogTask(void *pvParameters)
{
    for ( ;; )
    {
        vTaskDelay(configWdTaskPeriodTimeMs);
        if(!isStopRequested)
        {
            srvCpuRegWrite(TIMER1_CNRTL_REG, 8); /* reset timer */
            srvCpuRegWrite(TIMER1_START_REG, 1); /* start timer */
        }
    }
}


