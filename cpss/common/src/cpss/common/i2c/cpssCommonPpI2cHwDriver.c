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
* @file prvI2cHwDriver.c
*
* @brief I2C driver for Marvell PP.
*
* @version   1
********************************************************************************/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/common/i2c/private/prvCpssGenI2cLog.h>

#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/log/prvCpssLog.h>

#ifdef CHX_FAMILY
    #include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegsVer1.h>
#endif
#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include "cpssCommonPpI2cHwDriver.h"

/* Max I2C buses most devices can have, not including Falcon 12.8 */
#define CPSS_I2C_COMMON_DEV_MAX_I2C_BUS_PER_PP_CNS   2

/* Reg offset delta between buses of a PP */
#define I2C_BUS_REG_STEPPING        0x100
#define I2C_BASE_REG                0x11000
#define MAX_I2C_WRITE_COUNT         32


#define MV_TWSI_READ_REG(_offset, _var) \
    {                                   \
        drv->tmp = 0;                   \
        drv->common.parent->read(drv->common.parent, drv->as, drv->regBase + _offset, &drv->tmp, 1); \
        _var = (GT_U8)drv->tmp;          \
    }

#define MV_TWSI_WRITE_REG(_offset, _val)\
    {                                   \
        drv->tmp = (GT_U32)_val;        \
        drv->common.parent->writeMask(drv->common.parent, drv->as, drv->regBase + _offset, &drv->tmp, 1, 0xffffffff); \
    }
    /*cpssOsPrintf("w: 0x%x\n", drv->regBase + _offset); \*/


typedef struct PRV_I2C_HW_DRIVER_STCT {
    CPSS_HW_DRIVER_STC  common;
    CPSS_OS_MUTEX       mtx;
    GT_U32              as;     /* address space in parent */
    GT_U32              regBase;/* I2C control register base */
    MV_TWSI_CONTROLLER_STATE controller_state;
    GT_U32              tmp;    /* used by above macros, which need a tmp var */
} PRV_I2C_HW_DRIVER_STC;


static void mv_twsi_reset(
    IN  PRV_I2C_HW_DRIVER_STC *drv
)
{
    MV_TWSI_WRITE_REG(MV_TWSI_REG_RESET, 0);
}


static GT_STATUS mv_twsi_master_transceive(
    IN PRV_I2C_HW_DRIVER_STC *drv,
    GT_U8 *buffer,
    GT_U32 length,
    GT_U8 slave_address,    /* 7 bits, located at bits 0-6 */
    GT_U32 offset_length,
    GT_U32 slave_offset,
    GT_U32 tx_rx
)
{
    GT_STATUS   mv_twsi_status;
    GT_U8   mv_twsi_buffer[MV_TWSI_EXTERNAL_BUFFER_SIZE];
    GT_U32  mv_twsi_buffer_offset;
    GT_U32  mv_twsi_length;
    GT_U8   mv_twsi_slave_address, i=0;
    GT_U8   temp;

    MV_TWSI_CONTROLLER_STATUS status;
    GT_U32 ctrl;

    cpssOsMemSet(mv_twsi_buffer, 0, MV_TWSI_EXTERNAL_BUFFER_SIZE);
#if 0
    cpssOsPrintf("transceive: len %d sa 0x%x, offset_length %d slave_offset %d\n",
           length, slave_address, offset_length, slave_offset);
#endif

    if (MV_TWSI_STATE_IDLE != drv->controller_state)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    if (length > MV_TWSI_EXTERNAL_BUFFER_SIZE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    slave_address = slave_address << 1;

    cpssOsMutexLock(drv->mtx);

    /* Set global values for ISR, copy buffer, set START signal */
    temp = mv_twsi_slave_address = slave_address | tx_rx;
    drv->controller_state = MV_TWSI_STATE_START;
    mv_twsi_length = length;
    mv_twsi_buffer_offset = 0;

    /* Copy the slave register offset bytes to transmit buffer */
    if (offset_length) {
        /* Copy offset bytes to transmit buffer */
        for (i=0; i<offset_length ;i++)
            mv_twsi_buffer[i] = (GT_U8)(((slave_offset) >> ((offset_length - 1 - i) * 8)) & 0xFF);
    }

    if (MV_TWSI_TRANSMIT == tx_rx) {
        cpssOsMemCpy(mv_twsi_buffer + i, buffer, length);
        if (offset_length) {
            length += offset_length;
            mv_twsi_length = length;
        }
    } else {/* Receiver mode */
        if (offset_length)
            temp = slave_address | MV_TWSI_TRANSMIT;
    }/* Receiver mode */

    /* Load slave address + W/R bit */
    MV_TWSI_WRITE_REG(MV_TWSI_REG_DATA, temp);

    /* Set START signal on bus */
    MV_TWSI_READ_REG(MV_TWSI_REG_CTRL, temp);
    MV_TWSI_WRITE_REG(MV_TWSI_REG_CTRL, temp | MV_TWSI_CTRL_BIT_STA);

    mv_twsi_status = GT_FAIL;

    while (MV_TWSI_STATE_IDLE != drv->controller_state) {
        cpssOsTimerWkAfter(1);

/***********************************************************************
 * Start of state machine                                              *
 **********************************************************************/

    /* Read ctrl & status registers */
    MV_TWSI_READ_REG(MV_TWSI_REG_STATUS, status);
    MV_TWSI_READ_REG(MV_TWSI_REG_CTRL, ctrl);
    ctrl &= ~MV_TWSI_CTRL_BIT_IF;

        switch (drv->controller_state) {

        /*--------------  Master start transmitted  --------------*/
        /* This iteration will only clear the interrupt flag
         * so slave address can be transmitted
         */
        case MV_TWSI_STATE_START:
            /* Start signal transmitted */
            switch (status) {
            case MV_TWSI_STATUS_MSTR_STA:
            case MV_TWSI_STATUS_MSTR_REP_STA:
                /*
                 * Start signal transmitted successfully.
                 * Set next state to master tx or rx.
                 */
                if (mv_twsi_slave_address & MV_TWSI_RECEIVE) {
                    if (offset_length == 0)
                        drv->controller_state = MV_TWSI_STATE_REC;
                    else
                        drv->controller_state = MV_TWSI_STATE_OFFS;
                } else   /* Master transmitter */
                    drv->controller_state = MV_TWSI_STATE_TRAN;
                break;

            default:
                /* Error or Bytes have been transmitted and nacked
                 * - end of transmition
                 */
                mv_twsi_reset(drv);
                drv->controller_state = MV_TWSI_STATE_DONE;
                break;
            }
            break;

        /*--------------  Master transmitter  --------------*/
        case MV_TWSI_STATE_TRAN:

            switch (status) {
            case MV_TWSI_STATUS_MSTR_TRAN_SLA_ACK:
            case MV_TWSI_STATUS_MSTR_TRAN_DATA_ACK:
                /*
                 * Either slave acked it's address or data byte/s
                 * transmitted successfully. Load next byte/s or stop.
                 */
                if (mv_twsi_length <= mv_twsi_buffer_offset) { /* No more bytes - end transmition */
                    ctrl |= MV_TWSI_CTRL_BIT_STO;
                    mv_twsi_status = GT_OK;
                    drv->controller_state = MV_TWSI_STATE_DONE;
                } else {  /* Transmit next byte if any left */
                    /* Load bytes to data for TX */
                    MV_TWSI_WRITE_REG(MV_TWSI_REG_DATA, mv_twsi_buffer[mv_twsi_buffer_offset++]);
                }
                break;

            case MV_TWSI_STATUS_MSTR_TRAN_SLA_NACK:
                /* No Slave acked it's address. */
                ctrl |= MV_TWSI_CTRL_BIT_STO;
                drv->controller_state = MV_TWSI_STATE_DONE;
                break;

            default:
                /* Error or Bytes have been transmitted and nacked
                 * - end of transmition
                 */
                mv_twsi_reset(drv);
                drv->controller_state = MV_TWSI_STATE_DONE;
                break;
            }
            break;

        /*--------------  Transmit offset  --------------*/
        case MV_TWSI_STATE_OFFS:

            switch (status) {
            case MV_TWSI_STATUS_MSTR_TRAN_SLA_ACK:
            case MV_TWSI_STATUS_MSTR_TRAN_DATA_ACK:
                /*
                 * Either slave acked it's address or offset byte/s
                 * transmitted successfully. Load next byte/s or restart.
                 */
                if (offset_length > 0) {
                    offset_length--;
                    /* Load bytes to data for TX */
                    MV_TWSI_WRITE_REG(MV_TWSI_REG_DATA, mv_twsi_buffer[mv_twsi_buffer_offset++]);
                } else {
                    /* Offset has been transmitted - set repeated start, go to receiver mode */
                    mv_twsi_buffer_offset = 0;
                    ctrl |= MV_TWSI_CTRL_BIT_STA;
                    /* Load slave address + W/R bit */
                    MV_TWSI_WRITE_REG(MV_TWSI_REG_DATA, mv_twsi_slave_address);
                    drv->controller_state = MV_TWSI_STATE_START;
                }
                break;
            case MV_TWSI_STATUS_MSTR_TRAN_SLA_NACK:
                /* No Slave acked it's address. */
                ctrl |= MV_TWSI_CTRL_BIT_STO;
                drv->controller_state = MV_TWSI_STATE_DONE;
                break;

            default:
                /* Error or Bytes have been transmitted and nacked
                 * - end of transmition
                 */
                mv_twsi_reset(drv);
                drv->controller_state = MV_TWSI_STATE_DONE;
                break;
            }
            break;


        /*--------------  Master receiver  --------------*/
        case MV_TWSI_STATE_REC:

            switch (status) {
            case MV_TWSI_STATUS_MSTR_RECV_DATA_NACK:
                /* We're done receiving. Read byte/s, set stop bit, return to IDLE */
            case MV_TWSI_STATUS_MSTR_RECV_DATA_ACK:
                /*
                 * Slave sent byte/s, we acked. Read byte/s to external buffer.
                 * Don't ack last byte if no more space in external buffer.
                 */

                MV_TWSI_READ_REG(MV_TWSI_REG_DATA, mv_twsi_buffer[mv_twsi_buffer_offset]);

                if (MV_TWSI_STATUS_MSTR_RECV_DATA_NACK == status) { /* End of communication - copy buffer, stop bit and IDLE */
                    cpssOsMemCpy(buffer, mv_twsi_buffer, mv_twsi_length);
                    ctrl |= MV_TWSI_CTRL_BIT_STO;
                    mv_twsi_status = GT_OK;
                    drv->controller_state = MV_TWSI_STATE_DONE;
                } else /* Continue to receive next byte */
                    if (mv_twsi_length - mv_twsi_buffer_offset == 1)
                    ctrl &= ~MV_TWSI_CTRL_BIT_ACK; /* Don't acknowledge next byte */
                else
                    ctrl |= MV_TWSI_CTRL_BIT_ACK; /* Acknowledge next byte */

                mv_twsi_buffer_offset++;
                break;

            case MV_TWSI_STATUS_MSTR_RECV_SLA_ACK:
                /* Slave acked it's address. Set ack if needed, and receive first byte */
                if (mv_twsi_length > 1)
                    ctrl |= MV_TWSI_CTRL_BIT_ACK; /* Acknowledge next byte */
                break;

            case MV_TWSI_STATUS_MSTR_RECV_SLA_NACK:
                /* Slave nacked it's address. */
                ctrl |= MV_TWSI_CTRL_BIT_STO;
                drv->controller_state = MV_TWSI_STATE_DONE;
                break;

            default:
                /* Error or Bytes have been transmitted and nacked
                 * - end of transmition
                 */
                mv_twsi_reset(drv);
                drv->controller_state = MV_TWSI_STATE_DONE;
                break;
            }

            break;

        /*--------------  Master done  --------------*/
        case MV_TWSI_STATE_DONE:

            MV_TWSI_WRITE_REG(MV_TWSI_REG_CTRL, ctrl);
            drv->controller_state = MV_TWSI_STATE_IDLE;  /* relinquish */
            break;

        default:
            mv_twsi_reset(drv);
            drv->controller_state = MV_TWSI_STATE_DONE;
        }

        /* Clear interrupt */
        MV_TWSI_WRITE_REG(MV_TWSI_REG_CTRL, ctrl);
    }
    /************************/
    /* End of state machine */

    if (MV_TWSI_TRANSMIT == tx_rx)
        cpssOsTimerWkAfter(5);

    cpssOsMutexUnlock(drv->mtx);

    return mv_twsi_status;
}

/* addrSpace fields:
   bits[0:6]   - Slave Address
   bits[8:15]  - N/A (originally Bus ID)
   bits[16:31] - Offset length in bytes [0..2]
*/
static GT_STATUS prvI2cHwDriverRead(
    IN  PRV_I2C_HW_DRIVER_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_U8     slave_address;   /* the target device slave address on I2C bus */
    EXT_DRV_I2C_OFFSET_TYPE_ENT  offset_type;            /* receive mode */
    GT_STATUS  rc;

    slave_address = (addrSpace & 0xFF);
    offset_type = ((addrSpace & 0xFFFF0000) >> 16);

    rc = mv_twsi_master_transceive(drv, (GT_U8 *)dataPtr, count, slave_address, offset_type, regAddr, MV_TWSI_RECEIVE);

    return rc;
}


/* addrSpace fields:
   bits[0:6]   - Slave Address
   bits[8:15]  - N/A (originally Bus ID)
   bits[16:31] - Offset length in bytes [0..2]
*/
GT_STATUS prvI2cHwDriverWriteMask(
    IN  PRV_I2C_HW_DRIVER_STC *drv,
    IN  GT_U32  addrSpace,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask /* Only first 8-bits are considered */
)
{
    GT_U8     slave_address;   /* the target device slave address on I2C bus */
    EXT_DRV_I2C_OFFSET_TYPE_ENT  offset_type;            /* receive mode */
    GT_U8     masked_buf[MAX_I2C_WRITE_COUNT];
    GT_U8     u8_mask;
    GT_U32    i;
    GT_STATUS rc = GT_OK;

    if (count > MAX_I2C_WRITE_COUNT)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    slave_address = (addrSpace & 0xFF);
    offset_type = ((addrSpace & 0xFFFF0000) >> 16);

    if ( (mask & 0xFF) == 0xFF) {
        rc = mv_twsi_master_transceive(drv, (GT_U8 *)dataPtr, count, slave_address, offset_type, regAddr, MV_TWSI_TRANSMIT);
    } else {
        rc = mv_twsi_master_transceive(drv,       masked_buf, count, slave_address, offset_type, regAddr, MV_TWSI_RECEIVE);
        if (rc == GT_OK) {
            u8_mask = mask & 0xFF;
            for (i=0; i<count; i++)
                masked_buf[i] = (masked_buf[i] & ~u8_mask) | (*((GT_U8*)dataPtr + i) & u8_mask);
            rc = mv_twsi_master_transceive(drv, masked_buf, count, slave_address, offset_type, regAddr, MV_TWSI_TRANSMIT);
        }
    }

    return rc;
}


static void prvI2cHwDriverDestroyDrv(
    IN PRV_I2C_HW_DRIVER_STC *drv
)
{
    cpssOsMutexDelete(drv->mtx);
    cpssOsFree(drv);
}


static GT_STATUS prvI2cHwDriverGetDrvData(GT_U8 devNum, GT_U8 bus_id, char* drvPath, GT_U32  *asPtr, GT_U32* ctrlRegAddrPtr)
{
#ifndef FALCON_TILE_OFFSET_CNS
    /* Overcome undefined FALCON_TILE_OFFSET_CNS */
    #define FALCON_TILE_OFFSET_CNS  0
#endif
    GT_U32  baseAddr;
    GT_BOOL err;

    GT_U32 (*baseAddrGetFunc)
    (
        IN GT_U32                   devNum,
        IN PRV_CPSS_DXCH_UNIT_ENT   unitId,
        OUT GT_BOOL                 *errorPtr
    );

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PX_FAMILY_PIPE_E) {
#ifdef PX_FAMILY
        baseAddrGetFunc = prvCpssPxHwUnitBaseAddrGet;
#else
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
#endif
    } else {
#ifdef CHX_FAMILY
        baseAddrGetFunc = prvCpssDxChHwUnitBaseAddrGet;
#else
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
#endif
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E) )
    {
        baseAddr = baseAddrGetFunc(devNum, PRV_CPSS_DXCH_UNIT_CNM_RUNIT_E, &err);
        if (err == GT_TRUE)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        *asPtr = CPSS_HW_DRIVER_AS_MG0_E;

        /* 4 tiles Falcon */
        if ((bus_id>1) && (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles > 2) )
            *ctrlRegAddrPtr = baseAddr + 2*FALCON_TILE_OFFSET_CNS + I2C_BASE_REG + (bus_id-2)*I2C_BUS_REG_STEPPING;
        else
            *ctrlRegAddrPtr = baseAddr + I2C_BASE_REG + bus_id*I2C_BUS_REG_STEPPING;
    }
    else
    {
        *asPtr = CPSS_HW_DRIVER_AS_CNM_E;
        *ctrlRegAddrPtr = I2C_BASE_REG + bus_id*I2C_BUS_REG_STEPPING;
    }
    /* Names the driver as I2C@[address_space]:[CTRL_REG(equivalent to bus_id)], for example: I2C@0:11000*/
    cpssOsSprintf(drvPath, "I2C@%d:0x%x", *asPtr, *ctrlRegAddrPtr);

    return GT_OK;
}


/**
* @internal internal_cpssI2cHwDriverCreateDrv function
* @endinternal
*
* @brief   Initialize PP I2C master driver, or retrives an already created
*          driver for the same [dev, bus_id]
*
* Note:
*   Name is of the form -I2C@[address_space]:[CTRL_REG(equivalent to bus_id)],
*   for example: I2C@0:11000
*
* @param[in] devNum                - device number
* @param[in] bus_id                - I2C bus id
* @param[in] frequency             - requested bus freq
* @param[in] tclk                  - board's tclk (peripherals clock).
*       (APPLICABLE RANGES: 166MHZ-512MHZ)
* @param[out]drvPtr                - pointer to the created (or already
*                                    existing) driver
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum
* @retval GT_OUT_OF_CPU_MEM        - no memory/resource
*/
static GT_STATUS internal_cpssI2cHwDriverCreateDrv(
    IN  GT_U8   devNum,
    IN  GT_U8   bus_id,
    IN  GT_U32  frequency,
    IN  GT_U32  tclk,
    OUT CPSS_HW_DRIVER_STC **drvPtr
)
{
    GT_U32  addr_space, ctrlRegAddr; /* address of I2C base ctrl regs */
    PRV_CPSS_GEN_PP_CONFIG_STC *ppConfig;
    PRV_I2C_HW_DRIVER_STC *drv;
    CPSS_HW_DRIVER_STC *parent, *tmp;
    char drvPath[64] = {0};
    GT_U32  power, n, actualN = 0, m, actualM = 0, freq, minMargin = 0xffffffff;
    GT_32 diff;
    GT_STATUS  rc = GT_OK;

#ifdef CPSS_LOG_ENABLE
    GT_U32 actualFreq  = 0;
#endif

    /* Currently only 4 tiles Falcon */
    if (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles > 2)
    {
        if (bus_id >= CPSS_I2C_MAX_BUS_PER_PP_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else
        if (bus_id >= CPSS_I2C_COMMON_DEV_MAX_I2C_BUS_PER_PP_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if(frequency > 100000)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    ppConfig = PRV_CPSS_PP_MAC(devNum);
    if (ppConfig->i2cMasters[bus_id] != NULL) {
        if (drvPtr)
            *drvPtr = ppConfig->i2cMasters[bus_id];
        return GT_OK; /* already initialized */
    }
    parent = prvCpssDrvHwPpPortGroupGetDrv(devNum, 0);
    if (!parent)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);

    if (GT_OK != prvI2cHwDriverGetDrvData(devNum, bus_id, drvPath, &addr_space, &ctrlRegAddr))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    tmp = cpssHwDriverLookupRelative(parent, drvPath);
    if (tmp != NULL)
    {
        /* driver already exists - set it to handle also given portGroup */
        ppConfig->i2cMasters[bus_id] = tmp;
        tmp->numRefs++;
        if (drvPtr)
            *drvPtr = tmp;
        return GT_OK;
    }

    drv = cpssOsMalloc(sizeof(*drv));
    if (!drv)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)prvI2cHwDriverRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)prvI2cHwDriverWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)prvI2cHwDriverDestroyDrv;
    drv->common.parent = parent;
    drv->common.type = CPSS_HW_DRIVER_TYPE_I2C_E;

    cpssOsMutexCreate(drvPath, &(drv->mtx));
    drv->as = addr_space;
    drv->regBase = ctrlRegAddr;
    drv->controller_state = MV_TWSI_STATE_IDLE;

    /* Calucalte N and M for the TWSI clock baud rate */
    for(n = 0 ; n < 8 ; n++)
      for(m = 0 ; m < 16 ; m++)
      {
        power = 2 << n; /* power = 2^(n+1) */
        freq = tclk/(10*(m+1)*power);
        diff = frequency - freq;
        if (diff < 0) diff = freq - frequency;
        if ((GT_U32)diff < minMargin)
        {
          minMargin   = diff;
#ifdef CPSS_LOG_ENABLE
          actualFreq  = freq;
#endif
          actualN     = n;
          actualM     = m;
        }
      }

    /* Reset controller */
    mv_twsi_reset(drv);

    /* Set the baud rate */
    MV_TWSI_WRITE_REG(MV_TWSI_REG_BAUD, MV_TWSI_BAUD(actualM, actualN));
    /* Enable TWSI */
    MV_TWSI_WRITE_REG(MV_TWSI_REG_CTRL, MV_TWSI_CTRL_BIT_SLAVE);

    rc = cpssHwDriverRegister((CPSS_HW_DRIVER_STC*)drv, drvPath);
    if (rc == GT_OK) {
        ppConfig->i2cMasters[bus_id] = (CPSS_HW_DRIVER_STC*)drv;

        CPSS_LOG_INFORMATION_MAC("actualFreq %d\n", actualFreq);
        if (drvPtr)
            *drvPtr = (CPSS_HW_DRIVER_STC*)drv;
    }
    else
        cpssOsFree(drv);

    return rc;
}


/**
* @internal cpssI2cHwDriverCreateDrv function
* @endinternal
*
* @brief   Initialize PP I2C master driver, or retrives an already created
*          driver for the same [dev, bus_id]
*
* Note:
*   Name is of the form -I2C@[address_space]:[CTRL_REG(equivalent to bus_id)],
*   for example: I2C@0:11000
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number
* @param[in] bus_id                - I2C bus id
* @param[in] frequency             - requested bus freq
* @param[in] tclk                  - board's tclk (peripherals clock).
*       (APPLICABLE RANGES: 166MHZ-512MHZ)
* @param[out]drvPtr                - pointer to the created (or already
*                                    existing) driver
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum
* @retval GT_OUT_OF_CPU_MEM        - no memory/resource
*/
GT_STATUS cpssI2cHwDriverCreateDrv
(
    IN  GT_U8   devNum,
    IN  GT_U8   bus_id,
    IN  GT_U32  frequency,
    IN  GT_U32  tclk,
    OUT CPSS_HW_DRIVER_STC **drvPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssI2cHwDriverCreateDrv);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bus_id, frequency, tclk, drvPtr));

    rc = internal_cpssI2cHwDriverCreateDrv(devNum, bus_id, frequency, tclk, drvPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bus_id, frequency, tclk, drvPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


static CPSS_HW_DRIVER_STC* prvI2cHwDriverGetDrv(GT_U8 devNum, GT_U8 bus_id)
{
    CPSS_HW_DRIVER_STC *parent;
    PRV_CPSS_GEN_PP_CONFIG_STC *ppConfig;
    GT_U32  addr_space, ctrlRegAddr; /* address of I2C base ctrl regs */
    char drvPath[64] = {0};

    parent = prvCpssDrvHwPpPortGroupGetDrv(devNum, 0);
    if (!parent)
        return NULL;

    ppConfig = PRV_CPSS_PP_MAC(devNum);
    if ( ((ppConfig->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) &&
         (bus_id >= CPSS_I2C_MAX_BUS_PER_PP_CNS))  ||
         ((ppConfig->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E) &&
              (bus_id >= CPSS_I2C_COMMON_DEV_MAX_I2C_BUS_PER_PP_CNS)) )
    {
        cpssOsPrintf("Bus %d does not exist\n", bus_id);
        return NULL;
    }

    if (GT_OK != prvI2cHwDriverGetDrvData(devNum, bus_id, drvPath, &addr_space, &ctrlRegAddr))
        return NULL;

    return cpssHwDriverLookupRelative(parent, drvPath);
}


/* For falcon - 2nd eagle's (if exists) buses are numbered 2,3 */
int cpssDiagI2cDetect(GT_U8 devNum, GT_U8 bus_id)
{
    GT_U32  as;
    CPSS_HW_DRIVER_STC *drv;
    GT_U32 data = 0;
    GT_STATUS rc;
    int i;

    cpssOsPrintf("devNum: %d\n", devNum);

    drv = prvI2cHwDriverGetDrv(devNum, bus_id);
    if (!drv)
        return -1;

    cpssOsPrintf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    cpssOsPrintf("00:          ");
    for (i=3; i<0x75; i++) {
        as = (i & 0xFF) | 0;
        rc = drv->writeMask(drv, as, 0, &data, 1, 0xFFFFFFFF);
        if (rc)
            cpssOsPrintf("-- ");
        else
            cpssOsPrintf("%2x ",i);

        if (!((i+1)%16)) {
            cpssOsPrintf("\n%x: ",i+1);
        }
    }
    cpssOsPrintf("\n");

    return 0;
}


/* For falcon - 2nd eagle's (if exists) buses are numbered 2,3 */
int cpssDiagI2cRead(GT_U8 devNum, GT_U8 bus_id, int slaveAddr, int offType, int reg, int len)
{
    GT_U32  as;
    CPSS_HW_DRIVER_STC *drv;

    GT_U32 data[4]= {0};
    GT_STATUS rc;
    int i;

    if ((unsigned)len>sizeof(data)) {
        cpssOsPrintf("length exceeds %d\n", sizeof(data));
        return -1;
    }

    drv = prvI2cHwDriverGetDrv(devNum, bus_id);
    if (!drv)
        return -1;

    as = (slaveAddr & 0xFF) | ((offType & 0x3)<<16);
    rc = drv->read(drv, as, reg, data, len);

    if (rc)
        return rc;

    cpssOsPrintf("data: ");
    for (i=0; i<len; i++)
        cpssOsPrintf("0x%x ", *((char*)&data + i) );
    cpssOsPrintf("\n");

    return 0;
}


/* For falcon - 2nd eagle's (if exists) buses are numbered 2,3 */
int cpssDiagI2cWrite(GT_U8 devNum, GT_U8 bus_id, int slaveAddr, int offType, int reg, GT_U32 mask, int nof_args,
                     GT_U8 arg1, GT_U8 arg2, GT_U8 arg3, GT_U8 arg4
)
{
    GT_U32  as;
    CPSS_HW_DRIVER_STC *drv;
    GT_U8 data[4]= {0};
    GT_STATUS rc;

    drv = prvI2cHwDriverGetDrv(devNum, bus_id);
    if (!drv)
        return -1;

    switch (nof_args) {
    case 4:
        data[3] = arg4;
        GT_ATTR_FALLTHROUGH;
    case 3:
        data[2] = arg3;
        GT_ATTR_FALLTHROUGH;
    case 2:
        data[1] = arg2;
        GT_ATTR_FALLTHROUGH;
    case 1:
        data[0] = arg1;
    }

    as = (slaveAddr & 0xFF) | ((offType & 0x3)<<16);
    rc = drv->writeMask(drv, as, reg, (GT_U32*)data, nof_args, mask);

    return rc;
}

