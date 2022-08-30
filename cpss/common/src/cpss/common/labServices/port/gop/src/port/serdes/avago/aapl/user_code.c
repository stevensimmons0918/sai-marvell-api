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
 ** @brief Function registration documentation
 **/
#include "cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h"
#ifdef CPSS_BLOB
/* disable */
#define ASIC_SIMULATION
#endif
#ifndef ASIC_SIMULATION
#include <sys/ioctl.h>
#endif
#include <fcntl.h>
#include <errno.h>

#include "aapl.h"

/** @brief  Registers user supplied functions.
 ** @details Provides a single function from which user function registration
 **          can be performed.  Used only by AAPL CLI functions to provide
 **          a single location for user configuration without needing
 **          to change every CLI main function.
 **/
void aapl_register_user_supplied_functions(Aapl_t *aapl)
{
    (void) aapl;
}

#if defined(CHX_FAMILY) || defined(PX_FAMILY) || defined(WIN32)
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#ifndef ASIC_SIMULATION
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#endif
#else
#include <gtOs/gtGenTypes.h>
#include <mvSiliconIf.h>
#include <gtOs/gtOsTimer.h>
#define hwsOsTimerWkFuncPtr osTimerWkAfter
#endif

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#define I2C_SLAVE       0x0703

static int avagoI2cFd = -1;
static unsigned int i2cAvagoSlaveId;

/**
* @internal mvHwsAvagoInitI2cDriver function
* @endinternal
*
* @brief   Init the I2c interface
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
*/
GT_STATUS mvHwsAvagoInitI2cDriver
(
    unsigned char devNum
)
{
#ifdef ASIC_SIMULATION
    avagoI2cFd = 0;
    (void)devNum;
#endif

    if (avagoI2cFd >= 0)
        return GT_OK;

#ifndef ASIC_SIMULATION
    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwInfo[0].busType == CPSS_HW_INFO_BUS_TYPE_MBUS_E)
    {
        /* Open i2c-0 file for MSYS handling */
        avagoI2cFd = open("/dev/i2c-0", O_RDWR);
    }
    else
    {
        /* Open i2c-1 file for ARMADA_XP handling */
        avagoI2cFd = open("/dev/i2c-1", O_RDWR);
    }
#endif /* ASIC_SIMULATION */

    if (avagoI2cFd < 0)
    {
        fprintf(stderr, "failed to open /dev/i2c-*: %s\n", strerror(errno));
        return GT_FAIL;
    }

    i2cAvagoSlaveId = 0xFFFFFFFF;

    return GT_OK;
}

/*******************************************************************************/
GT_STATUS mvHwsAvagoSetSlaveId
(
    GT_U32 devSlvId
)
{
#ifdef ASIC_SIMULATION
    devSlvId = i2cAvagoSlaveId;
#endif

    if(devSlvId == i2cAvagoSlaveId)
    {
        return GT_OK;
    }

#ifndef ASIC_SIMULATION
    if (ioctl(avagoI2cFd, I2C_SLAVE, devSlvId) < 0)
    {
        fprintf(stderr, "Error: Could not set slave address: %s\n", strerror(errno));
        return GT_FAIL;
    }
#endif

    i2cAvagoSlaveId = devSlvId;
    hwsOsTimerWkFuncPtr(10); /* wait till OS DB is updated */

    return GT_OK;
}

/* NOTE: Typically only one of the following is required: */
/*   user_supplied_mdio_function(), */
/*   user_supplied_sbus_function(), or */
/*   user_supplied_i2c_read/write_function() */
/*  */
/* The user can also optionaly supply the  */
/*   user_supplied_serdes_interrupt_function() if the device under test */
/*   is an ASIC and the SerDes' core interrupt interface is available  */
/*   to send interrupts to. */
/*  */
/* The unused functions can be left unchanged. */
/* See the define for SBUS_MODE_DEF in aapl.h where the default is set. */

#if AAPL_ALLOW_USER_SUPPLIED_MDIO

/** @brief  Initializes user supplied mdio access. */
/** @return On success, returns 0. */
/** @return On error, decrements aapl->return_code and returns -1. */
int user_supplied_mdio_open_function(Aapl_t *aapl)
{
    int rc = 0;
    aapl_log_printf(aapl, AVAGO_DEBUG8, __func__, __LINE__, "open status = %d\n", rc);
    return rc;
}

/** @brief  Terminates user supplied mdio access. */
/** @return On success, returns 0. */
/** @return On error, decrements aapl->return_code and returns -1. */
int user_supplied_mdio_close_function(Aapl_t *aapl)
{
    int rc = 0;
    aapl_log_printf(aapl, AVAGO_DEBUG8, __func__, __LINE__, "close status = %d\n", rc);
    return rc;
}

unsigned int user_supplied_mdio_function(
    Aapl_t *aapl,
    Avago_mdio_cmd_t mdio_cmd,
    unsigned int port_addr,
    unsigned int dev_addr,
    unsigned int reg_addr,
    unsigned int data)
{
    BOOL command_succeeded = FALSE;

    aapl_log_printf(aapl, AVAGO_DEBUG6, __func__, __LINE__,
        "mdio_cmd=%d, port_addr=%d, dev_addr=%d, reg_addr=%d, data=0x%x\n",
        mdio_cmd, port_addr, dev_addr, reg_addr, data);

    /*////////////////////////////////////////////////////////////////// */
    /*////////////////////////////////////////////////////////////////// */

    if( ! command_succeeded )
    {
        aapl_fail(aapl, __func__, __LINE__, "MDIO %d %d %d 0x%x FAILED\n", mdio_cmd, port_addr, dev_addr, data);
    }
    return data;
}
#endif

#if AAPL_ALLOW_USER_SUPPLIED_SBUS

/** @brief  Initializes user supplied sbus access. */
/** @return On success, returns 0. */
/** @return On error, decrements aapl->return_code and returns -1. */
int user_supplied_sbus_open_function(Aapl_t *aapl)
{
    int rc = 0;
    aapl_log_printf(aapl, AVAGO_DEBUG8, __func__, __LINE__, "open status = %d\n", rc);
    return rc;
}

/** @brief  Terminates user supplied sbus access. */
/** @return On success, returns 0. */
/** @return On error, decrements aapl->return_code and returns -1. */
int user_supplied_sbus_close_function(Aapl_t *aapl)
{
    int rc = 0;
    aapl_log_printf(aapl, AVAGO_DEBUG8, __func__, __LINE__, "close status = %d\n", rc);
    return rc;
}
#endif

#if AAPL_ALLOW_USER_SUPPLIED_I2C

/** @brief  Initializes user supplied i2c access. */
/** @return On success, returns 0. */
/** @return On error, decrements aapl->return_code and returns -1. */
int user_supplied_i2c_open_function(Aapl_t *aapl)
{
    int rc = 0;
    aapl_log_printf(aapl, AVAGO_DEBUG8, __func__, __LINE__, "open status = %d\n", rc);
    return rc;
}

/** @brief  Terminates user supplied i2c access. */
/** @return On success, returns 0. */
/** @return On error, decrements aapl->return_code and returns -1. */
int user_supplied_i2c_close_function(Aapl_t *aapl)
{
    int rc = 0;
    aapl_log_printf(aapl, AVAGO_DEBUG8, __func__, __LINE__, "close status = %d\n", rc);
    return rc;
}

unsigned int user_supplied_i2c_write_function(
    Aapl_t *aapl,
    unsigned char device_addr,
    unsigned char length,
    unsigned char *buffer)
{
    unsigned int bytes_written = 0;     /* Number of bytes read from the device */

    aapl_log_printf(aapl, AVAGO_DEBUG6, __func__, __LINE__ ,
         "i2c w %02x 0x%08x\n", device_addr, buffer[0]);

     if(GT_OK != mvHwsAvagoSetSlaveId(device_addr))
      return 0;
      aapl_log_printf(aapl, AVAGO_DEBUG6, __func__, __LINE__ ,
         "i2c w1 %02x %02x %02x %02x 0x%08x \n", device_addr, length, buffer[0],buffer[1],buffer[2]);

    /*  Write 'length' number of bytes to the device. */
    /* The function should return the number of bytes actually writen to the devivce */
#ifndef ASIC_SIMULATION
    if( (bytes_written = write(avagoI2cFd,buffer,length)) != length )
    {
        aapl_fail(aapl, __func__, __LINE__, "i2c w %04x %02x FAILED\n", device_addr, buffer[0]);
    }
#endif

    return bytes_written;
}

unsigned int user_supplied_i2c_read_function(
    Aapl_t *aapl,
    unsigned char device_addr,
    unsigned char length,
    unsigned char *buffer)
{
    unsigned int bytes_read = 0;     /* Number of bytes read from the device */

    aapl_log_printf(aapl, AVAGO_DEBUG6, __func__, __LINE__ ,
         "i2c r %02x 0x%08x\n", device_addr, buffer[0]);

    /*  Read 'length' number of bytes from the device. */
    /* Modify the variable "buffer" with the results. */
    /* The function should return the number of bytes actually read (>0==success, 0==fail) */
    if(GT_OK != mvHwsAvagoSetSlaveId(device_addr))
       return 0;
#ifndef ASIC_SIMULATION
    if ((bytes_read = read (avagoI2cFd,buffer,length)) != length)
         aapl_fail(aapl, __func__, __LINE__, "i2c r %04x %02x FAILED\n", device_addr, buffer[0]);
#endif

   return bytes_read;
}
#endif


#if AAPL_ENABLE_USER_SERDES_INT

/** @brief   Execute a SerDes SPICO interrupt using the SerDes' core interrupt */
/**   ports. */
/** @details If AVAGO_ENABLE_SERDES_INT_CORE_PORT is enabled, the user must */
/**   supply a function that executes a SerDes SPICO interrupt using the  */
/**   SerDes' core ports: */
/**      i_core_interrupt_code[15:0] */
/**      i_core_interrupt_data[15:0] */
/**      i_core_interrupt */
/** The resulting data from o_core_interrupt_data_out[15:0] should */
/**   be returned once the interrupt is complete. */
/** */
/** NOTE: This function should wait until the interrupt is complete. */
/**   See the SerDes spec on how to properly execute interrupts and wait for */
/**   them to complete. */
/** @return Returns the result of the interrupt command. */
/**         On error, aapl_fail should be called before returning. */

unsigned int user_supplied_serdes_interrupt_function(
    Aapl_t *aapl,       /**< [in] The Avago APL structure pointer. */
    uint addr,          /**< [in] The device's SBus address. */
    int int_code,       /**< [in] The interrupt number. */
    int int_data)       /**< [in] The interrupt data. */
{
    unsigned int result = 0;
    BOOL command_succeeded = FALSE;
    Avago_addr_t addr_struct;

    avago_addr_to_struct(addr,&addr_struct);

    aapl_log_printf(aapl, AVAGO_DEBUG5, __func__, __LINE__,
        "addr=%s, int_code=%02x, int_data=%02x\n",
        aapl_addr_to_str(addr), int_code, int_data);

    /*////////////////////////////////////////////////////////////////// */
    /*////////////////////////////////////////////////////////////////// */

    if( ! command_succeeded )
    {
        aapl_fail(aapl, __func__, __LINE__, "addr=%s, int_num=%02x, param=%02x\n", aapl_addr_to_str(addr), int_code, int_data);
    }
    return result;
}

#endif


