/* AAPL CORE Revision: 2.6.2
 *
 * Copyright (c) 2014-2017 Avago Technologies. All rights reserved.
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

/** Doxygen File Header */
/** @file */
/** @brief User-supplied functions. */
#include "include/aapl.h"
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpssCommon/private/prvCpssEmulatorMode.h>
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
#define hwsOsPrintf osPrintf
#endif
/* BobK and Aldrin SBC addresses */
#define SBC_BOBK_UNIT_BASE_ADDRESS              (0x60000000)
#define SBC_UNIT_REG_ADDR(base, reg)            ((base) | (reg))
#define SBC_BOBK_MASTER_BASE_ADDRESS            (0x60040000)

/* Pipe SBC addresses */
#define SBC_PIPE_UNIT_BASE_ADDRESS              (0x06000000)
#define SBC_PIPE_MASTER_BASE_ADDRESS            (0x06040000)

/* Raven SBC addresses */
#define SBC_RAVEN_UNIT_BASE_ADDRESS             (0x00600000)
#define SBC_RAVEN_MASTER_BASE_ADDRESS           (0x00640000)


/* Bobcat3 SBC addresses */
#define SBC_BC3_UNIT0_BASE_ADDRESS              (0x24000000)
#define SBC_BC3_UNIT1_BASE_ADDRESS              (0xA4000000)
#define SBC_BC3_UNIT0_REG_ADDR(reg)             (SBC_BC3_UNIT0_BASE_ADDRESS | reg)
#define SBC_BC3_UNIT1_REG_ADDR(reg)             (SBC_BC3_UNIT1_BASE_ADDRESS | reg)
#define SBC_BC3_MASTER_UNIT0_BASE_ADDRESS       (0x24040000)
#define SBC_BC3_MASTER_UNIT1_BASE_ADDRESS       (0xA4040000)

/* Falcon SBC addresses */
#define SBC_FALCON_MASTER_UNIT0_BASE_ADDRESS    (0x00640000)
#define SBC_FALCON_UNIT0_BASE_ADDRESS           (0x00600000)
#define SBC_FALCON_UNIT0_SBC_STATUS_REG_ADDRESS (0x00600004)
#define SBC_FALCON_UNIT_Q1Q3_BASE_ADDRESS(chipIndex, reg) (((reg) + ((chipIndex/4) * 0x20000000)) | ((chipIndex % 4) * 0x1000000))
#define SBC_FALCON_UNIT_Q2Q4_BASE_ADDRESS(chipIndex, reg) (((reg) + ((chipIndex/4) * 0x20000000)) | ((0x3 - (chipIndex % 4)) * 0x1000000))


/* SBC addresses */
#define SBC_MASTER_SERDES_NUM_SHIFT         (10)  /* Serdes number offset: 0x400 */
#define SBC_MASTER_REG_ADDR_SHIFT           (2)
#define SBC_UNIT_COMMOM_CTRL_REG_ADDR       (0x0)
#define SBC_UNIT_SOFT_RESET                 (0x1)
#define SBC_UNIT_INTERNAL_ROM_ENABLE        (0x2)

#define SBC_DEBUG_PRINTS                    0

EXT void ms_sleep(uint msec);
/*******************************************************************************
* user_supplied_pex_address
*
* DESCRIPTION:
*       Build PEX address from sbus_addr, and reg_addr
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pex register address
*******************************************************************************/
static unsigned int user_supplied_pex_address
(
    unsigned int devNum,
    unsigned int chipIndex,
    unsigned int sbus_addr,
    unsigned int reg_addr,
    unsigned char command
)
{
    unsigned int serdesAddress;
    unsigned int regBaseAddr = 0;

    GT_UNUSED_PARAM(command);

    /* Read Common control register */
    if(HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        regBaseAddr = SBC_PIPE_MASTER_BASE_ADDRESS;
    }
    else if(HWS_DEV_SILICON_TYPE(devNum) == Raven)
    {
        regBaseAddr = SBC_RAVEN_MASTER_BASE_ADDRESS;
    }
    else if((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2))
    {
        if (sbus_addr & 1 << 8)
        {
            /* ring 1*/
            regBaseAddr = SBC_BC3_MASTER_UNIT1_BASE_ADDRESS;
        }
        else
        {
            /* ring 0*/
            regBaseAddr = SBC_BC3_MASTER_UNIT0_BASE_ADDRESS;
        }
    }
    else if(HWS_DEV_SILICON_TYPE(devNum) == Falcon)
    {
        user_supplied_get_raven_sbus_unit_address(chipIndex, SBC_FALCON_MASTER_UNIT0_BASE_ADDRESS, &regBaseAddr);
    }
    else
    {
        regBaseAddr = SBC_BOBK_MASTER_BASE_ADDRESS;
    }
    sbus_addr &= 0xff;

    serdesAddress = ((regBaseAddr)                              |
                     (sbus_addr << SBC_MASTER_SERDES_NUM_SHIFT) |
                     (reg_addr  << SBC_MASTER_REG_ADDR_SHIFT));

    return serdesAddress;
}

static unsigned int user_supplied_status_polling
(
    unsigned int  devNum,
    unsigned int  chipIndex,
    unsigned int  sbus_addr,
    unsigned int  reg_addr,
    unsigned char command,
    unsigned int  sbus_data
)
{
    unsigned int iterCnt;
    unsigned int regStatusAddr;
    GT_U32       data = 0;
    int          rc;

    if(HWS_DEV_SILICON_TYPE(devNum) == Falcon)
    {
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
        if (!cpssDeviceRunCheck_onEmulator())
#endif
        {
            user_supplied_get_raven_sbus_unit_address(chipIndex, SBC_FALCON_UNIT0_SBC_STATUS_REG_ADDRESS, &regStatusAddr);
            if ((command == 1) || (command == 2))
            {
                if(hwsRegisterGetFuncPtr == NULL)
                {
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
                    hwsOsPrintf("user_supplied_pex_address: BAD PTR for GET FUNC\n");
#endif
                    return GT_BAD_PTR;
                }
#if SBC_DEBUG_PRINTS /* Print transaction's parameters */
                hwsOsPrintf("-command[%d]-reg_addr[0x%x]-data[0x%x]-sbus_data[0x%x]\n",command,reg_addr,data,sbus_data);
#else
                GT_UNUSED_PARAM(sbus_data);
#endif
                if((command == 1) && (sbus_addr == AVAGO_SBUS_MASTER_ADDRESS) && (reg_addr == 0x7))
                {   /* FALCON/RAVEN EOM SUPPORT -
                    WA FOR WRONG DIRECTION STALL SIGNAL.
                    For eye gathering - use a different polling method */
                    regStatusAddr = ((regStatusAddr) & 0xfff00000) | 0x14 ;
                    for(iterCnt = 0; iterCnt < 1000000; iterCnt++)
                    {
                        ms_sleep(3);
                        rc = hwsRegisterGetFuncPtr(devNum, 0, regStatusAddr, &data, 0xefff);
                        if(rc != 0)
                        {
                            hwsOsPrintf("user_supplied_pex_address: ERROR read\n");
                            return GT_GET_ERROR;
                        }

                        if((data & 0xff) == 0)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    for(iterCnt = 0; iterCnt < 1000000; iterCnt++)
                    {
                        rc = hwsRegisterGetFuncPtr(devNum, 0, regStatusAddr, &data, 1);
                        if(rc != 0)
                        {
                            hwsOsPrintf("user_supplied_pex_address: ERROR read\n");
                            return GT_GET_ERROR;
                        }
                        if((data & 1) == 0)
                        {
                            break;
                        }
                    }
                }
                if(iterCnt == 1000000)
                {
                    hwsOsPrintf("user_supplied_pex_address: TIMEOUT ERROR - SBUS is busy (chipIndex=%d, regStatusAddr=0x%x)\n", chipIndex, regStatusAddr);
                    return GT_TIMEOUT;
                }
            }
        }
    }
    return GT_OK;
}
/*******************************************************************************
* user_supplied_sbus_function
*
* DESCRIPTION:
*       Execute an sbus command
*
* INPUTS:
*       aapl      - Pointer to Aapl_t structure
*       addr      - SBus slice address
*       reg_addr  - SBus register to read/write
*       command   - 0 = reset, 1 = write, 2 = read
*       sbus_data - Data to write
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Reads, returns read data
*       Writes and reset, returns 0
*******************************************************************************/
unsigned int user_supplied_sbus_function
(
    struct _Aapl_t  *ptr,
    unsigned int    addr,
    unsigned char   reg_addr,
    unsigned char   command,
    unsigned int    *sbus_data
)
{
    Aapl_t *aapl = (Aapl_t *)ptr;
    unsigned int commandAddress;
    unsigned int mask      = 0xFFFFFFFF;
    unsigned int devNum    = aapl->devNum;
    unsigned int portGroup = aapl->portGroup;
    unsigned int chipIndex = aapl->chipIndex;
    unsigned int rc=0;
    GT_U32       data;

    if (command == 1/*Write Command - WRITE_SBUS_DEVICE*/)
    {
        commandAddress = user_supplied_pex_address(devNum, chipIndex, addr, reg_addr, command);

#if SBC_DEBUG_PRINTS
        hwsOsPrintf("-commandAddress[0x%x]\n",commandAddress);
#endif
        if(hwsRegisterSetFuncPtr == NULL)
        {
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
            hwsOsPrintf("user_supplied_sbus_function: BAD PTR for SET FUNC\n");
#endif
            return GT_BAD_PTR;
        }
        hwsRegisterSetFuncPtr(devNum, portGroup, commandAddress, *sbus_data, mask);
    }
    else if (command == 2/*Read Command - READ_SBUS_DEVICE*/)
    {
        commandAddress = user_supplied_pex_address(devNum, chipIndex, addr,  reg_addr, command);

        if(hwsRegisterGetFuncPtr == NULL)
        {
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
            hwsOsPrintf("user_supplied_sbus_function: BAD PTR for GET FUNC\n");
#endif
            return GT_BAD_PTR;
        }
        rc = hwsRegisterGetFuncPtr(devNum, portGroup, commandAddress, &data, mask);
        if (rc!=0)
        {
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
            hwsOsPrintf("REGISTER READ ERROR\n");
#endif
            return rc;
        }
        *sbus_data = (unsigned int)data;
    }
    else if (command == 0/*Soft Reset - RESET_SBUS_DEVICE*/)
    {
        user_supplied_sbus_soft_reset(aapl);
    }
    else if (command == 3/*Hard Reset - CORE_SBUS_RESET*/)
    {
        /* This command is part of the entire chip reset */
        /* The AAPL start execute after chip reset, therefore this command can be removed */
    }
    user_supplied_status_polling(devNum, chipIndex, addr,  reg_addr, command, *sbus_data);
    return (rc==0) ? 1 : 0;
}

/**
* @internal user_supplied_sbus_soft_reset function
* @endinternal
*
* @brief   Execute Software reset
*
* @param[in] aapl                     - Pointer to Aapl_t structure
*                                       None
*/
void user_supplied_sbus_soft_reset
(
    Aapl_t *aapl
)
{
    unsigned int mask      = 0xFFFFFFFF;
    GT_U32       data      = 0;
    unsigned int devNum    = aapl->devNum;
    unsigned int portGroup = aapl->portGroup;
    unsigned int chipIndex = aapl->chipIndex;
    unsigned int regAddr;
    unsigned int ring;

    if((hwsRegisterGetFuncPtr == NULL) || (hwsRegisterSetFuncPtr == NULL))
    {
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
        hwsOsPrintf("user_supplied_sbus_soft_reset: BAD PTR for SET/GET FUNC\n");
#endif
        return;
    }

    for (ring = 0; ring < aapl->sbus_rings; ring++)
    {
        /* Read Common control register */
        if(HWS_DEV_SILICON_TYPE(devNum) == Pipe)
        {
            regAddr = SBC_UNIT_REG_ADDR(SBC_PIPE_UNIT_BASE_ADDRESS, SBC_UNIT_COMMOM_CTRL_REG_ADDR);
        }
        else if(HWS_DEV_SILICON_TYPE(devNum) == Raven)
        {
            regAddr = SBC_UNIT_REG_ADDR(SBC_RAVEN_UNIT_BASE_ADDRESS, SBC_UNIT_COMMOM_CTRL_REG_ADDR);
        }
        else if(HWS_DEV_SILICON_TYPE(devNum) == Falcon)
        {
            user_supplied_get_raven_sbus_unit_address(chipIndex, SBC_FALCON_UNIT0_BASE_ADDRESS, &regAddr);
        }
        else if((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2))
        {
            regAddr = (ring == 0) ? SBC_BC3_UNIT0_REG_ADDR(SBC_UNIT_COMMOM_CTRL_REG_ADDR) : SBC_BC3_UNIT1_REG_ADDR(SBC_UNIT_COMMOM_CTRL_REG_ADDR);
        }

        else
        {
            regAddr = SBC_UNIT_REG_ADDR(SBC_BOBK_UNIT_BASE_ADDRESS, SBC_UNIT_COMMOM_CTRL_REG_ADDR);
        }

        hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &data, mask);

        if (data & SBC_UNIT_SOFT_RESET)
            continue;

        /* Set SBC in reset */
        data &= ~SBC_UNIT_SOFT_RESET;
        /* Clear internal ROM enable - loading ROM from the application */
        data &= ~SBC_UNIT_INTERNAL_ROM_ENABLE;
        hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, data, mask);
        /* Take SBC out of reset */
        data |= SBC_UNIT_SOFT_RESET;
        hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, data, mask);
    }
}


unsigned int user_supplied_get_raven_sbus_unit_address
(
    unsigned int chipIndex,
    unsigned int reg,
    unsigned int *address
)
{
    switch (chipIndex % 16)
    {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x8:
        case 0x9:
        case 0xA:
        case 0xB:
            *address = SBC_FALCON_UNIT_Q1Q3_BASE_ADDRESS((chipIndex % 16), reg);
            break;
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
        case 0xC:
        case 0xD:
        case 0xE:
        case 0xF:
            *address = SBC_FALCON_UNIT_Q2Q4_BASE_ADDRESS((chipIndex % 16), reg);
            break;
        default:
            return GT_BAD_PARAM;
    }
    return GT_OK;
}


