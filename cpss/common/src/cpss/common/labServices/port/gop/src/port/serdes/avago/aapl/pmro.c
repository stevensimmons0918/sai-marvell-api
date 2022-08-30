/* AAPL CORE Revision: 2.7.3
 *
 * Copyright (c) 2014-2018 Avago Technologies. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** Doxygen File Header
 ** @file
 ** @brief PMRO device specific functions.
 **/

#define AAPL_ENABLE_INTERNAL_FUNCTIONS
#include "aapl.h"

static uint avago_sbus_rd_until_valid(Aapl_t *aapl, uint addr, int reg)
{
    int loop;
    uint total = 0;
    for( loop = 0; loop < 5; loop++ )
    {
        if( loop >= 3 )
            ms_sleep(1);
        if( (total = avago_sbus_rd(aapl, addr, reg)) != 0 )
            break;
    }
    if( loop > 0 )
        aapl_log_printf(aapl, AVAGO_DEBUG1, __func__, __LINE__, "loop = %d\n", loop);
    return total;
}

/** @brief   Measures process performance of the chip.
 ** @details The returned value is dependent on the reference clock frequency.
 **          As a result, values obtained with different reference clock
 **          frequencies cannot be meaningfully compared.
 ** @return  Returns a number indicating propagation delay through
 **          a set of gates and interconnects.
 **          A higher number indicates faster process performance.
 **/
uint avago_pmro_get_metric(
    Aapl_t *aapl,        /**< [in] Pointer to AAPL structure */
    uint addr)           /**< [in] SBus address of SerDes */
{
    uint total;
    if( !aapl_check_ip_type(aapl, addr, __func__, __LINE__, TRUE, 2, AVAGO_PMRO, AVAGO_PMRO2) )
        return 0;
    if( aapl->capabilities & AACS_SERVER_NO_CRC )
        return 0;

    avago_sbus_reset(aapl, addr, 0);
    avago_sbus_wr(aapl, addr, 0, 1);
    total = avago_sbus_rd_until_valid(aapl, addr, 6);
    if( total == 0 )
    {
        aapl_log_printf(aapl, AVAGO_WARNING, __func__, __LINE__, "Working around non-implemented soft SBus reset.\n");
        avago_sbus_wr(aapl, addr, 0, 0x00000000);
        avago_sbus_wr(aapl, addr, 1, 0x0000ffff);
        avago_sbus_wr(aapl, addr, 2, 0x0000ffff);
        avago_sbus_wr(aapl, addr, 3, 0x0000ffff);
        avago_sbus_wr(aapl, addr, 4, 0x0000ffff);
        avago_sbus_wr(aapl, addr, 5, 0x00fff000);
        avago_sbus_wr(aapl, addr, 6, 0x00000000);
        avago_sbus_wr(aapl, addr, 0, 1);
        total = avago_sbus_rd_until_valid(aapl, addr, 6);
    }
    return total;
}

