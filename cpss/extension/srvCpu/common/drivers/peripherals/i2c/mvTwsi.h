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
* @file mvTwsi.h
*
* @brief I2C capabilities declarations.
*
* @version   1
********************************************************************************
*/

#ifndef __INCmvTwsiH
#define __INCmvTwsiH

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "common.h"

/* Codes for controller HW status byte (taken from DataSheet) */
typedef enum SRV_CPU_TWSI_CONTROLLER_STATUS{

  SRV_CPU_TWSI_STATUS_IDLE_E                  = 0xF8, /* Idle */

  SRV_CPU_TWSI_STATUS_MSTR_STA_E              = 0x08, /* A START condition transmitted */
  SRV_CPU_TWSI_STATUS_MSTR_REP_STA_E          = 0x10, /* A repeated START condition transmitted */
  SRV_CPU_TWSI_STATUS_MSTR_TRAN_SLA_ACK_E     = 0x18, /* SLA+W transmitted; ACK received */
  SRV_CPU_TWSI_STATUS_MSTR_TRAN_SLA_NACK_E    = 0x20, /* SLA+W transmitted; NACK received */
  SRV_CPU_TWSI_STATUS_MSTR_TRAN_DATA_ACK_E    = 0x28, /* Data byte/s in I2CDAT/buffer transmitted; ACK received for all bytes*/
  SRV_CPU_TWSI_STATUS_MSTR_TRAN_DATA_NACK_E   = 0x30, /* Data byte/s in I2CDAT/buffertransmitted; NACK received for last byte*/
  SRV_CPU_TWSI_STATUS_MSTR_ARB_LOST_E         = 0x38, /* Arbitration lost in SLA+W or Data bytes */

  SRV_CPU_TWSI_STATUS_MSTR_RECV_SLA_ACK_E     = 0x40, /* SLA+R transmitted; ACK received */
  SRV_CPU_TWSI_STATUS_MSTR_RECV_SLA_NACK_E    = 0x48, /* SLA+R transmitted; NACK received */
  SRV_CPU_TWSI_STATUS_MSTR_RECV_DATA_ACK_E    = 0x50, /* Data byte received; ACK returned */
  SRV_CPU_TWSI_STATUS_MSTR_RECV_DATA_NACK_E   = 0x58, /* Data byte received; NACK returned (possibly end of transmition) */

  SRV_CPU_TWSI_STATUS_SLAV_REC_OWN_SLA_E      = 0x60, /* Own SLA+W received; ACK returned */
  SRV_CPU_TWSI_STATUS_SLAV_REC_ARB_LOST_E     = 0x68, /* Arbitration lost in SLA+R/W as master; Own SLA+W received, ACK returned */
  SRV_CPU_TWSI_STATUS_SLAV_GEN_CAL_E          = 0xD0, /* General Call address (00h) received; ACK returned. */
  SRV_CPU_TWSI_STATUS_SLAV_GEN_CAL_ARB_LOST_E = 0xD8, /* Arbitration lost in SLA = R/W as master; General Call address received; ACK bit returned. */
  SRV_CPU_TWSI_STATUS_SLAV_REC_DATA_ACK_E     = 0x80, /* Previously addressed with own slave address; DATA received; ACK returned */
  SRV_CPU_TWSI_STATUS_SLAV_REC_DATA_NACK_E    = 0x88, /* Previously addressed with own slave address; DATA byte received; NACK returned for last byte */
  SRV_CPU_TWSI_STATUS_SLAV_GEN_DATA_ACK_E     = 0xE0, /* Previously addressed with General Call; Data received; ACK returned */
  SRV_CPU_TWSI_STATUS_SLAV_GEN_DATA_NACK_E    = 0xE8, /* Previously addressed with General Call; Data received; NACK returned for last byte */
  SRV_CPU_TWSI_STATUS_SLAV_STO_REP_STA_E      = 0xA0, /* A STOP/repeated START received while still addressed as Slave Receiver */

  SRV_CPU_TWSI_STATUS_SLAV_TRA_OWN_SLA_E      = 0xA8, /* Own SLA+R received; ACK returned */
  SRV_CPU_TWSI_STATUS_SLAV_TRA_ARB_LOST_E     = 0xB0, /* Arbitration lost in SLA+R/W as master; Own SLA+W received, ACK returned */
  SRV_CPU_TWSI_STATUS_SLAV_TRA_DATA_ACK_E     = 0xB8, /* Data bytes transmitted; ACK received */
  SRV_CPU_TWSI_STATUS_SLAV_TRA_DATA_NACK_E    = 0xC0, /* Data bytes transmitted; NACK received for last byte */
  SRV_CPU_TWSI_STATUS_SLAV_REC_DATA_ACK_NA_E  = 0xC8, /* Data bytes transmitted; ACK received (AA = 0) */

  SRV_CPU_TWSI_STATUS_BUS_ERROR_SDA_E         = 0x70,  /* Bus error SDA stuck low */
  SRV_CPU_TWSI_STATUS_BUS_ERROR_SCL_E         = 0x78,  /* Bus error SCL stuck low */
  SRV_CPU_TWSI_STATUS_INVALID_COUNT_VALUE_E   = 0xFC,  /* Invalid value has been loaded to I2CCOUNT register */
  SRV_CPU_TWSI_STATUS_BUS_ERROR_ILLEGAL_E     = 0x00   /* Bus error due to illegal start/stop */

 }SRV_CPU_TWSI_CONTROLLER_STATUS;

/* Internal registers offsets */
typedef enum SRV_CPU_TWSI_REGISTERS{
  SRV_CPU_TWSI_REG_SLV_ADDR_E = 0x00, /* slave (own) address */
  SRV_CPU_TWSI_REG_DATA_E     = 0x04, /* data */
  SRV_CPU_TWSI_REG_CTRL_E     = 0x08, /* control */
  SRV_CPU_TWSI_REG_STATUS_E   = 0x0C, /* status [Read only]*/
  SRV_CPU_TWSI_REG_BAUD_E     = 0x0C, /* BAUD rate */
  SRV_CPU_TWSI_REG_RESET_E    = 0x1C  /* soft reset */
}SRV_CPU_TWSI_REGISTERS;

/* Control register bits */
typedef enum SRV_CPU_TWSI_CONTROL_BIT{
  SRV_CPU_TWSI_CTRL_BIT_IE_E    = 0x80, /* Interrupt enable */
  SRV_CPU_TWSI_CTRL_BIT_SLAVE_E = 0x40, /* Enable slave */
  SRV_CPU_TWSI_CTRL_BIT_STA_E   = 0x20, /* Set START bit */
  SRV_CPU_TWSI_CTRL_BIT_STO_E   = 0x10, /* Set STOP bit */
  SRV_CPU_TWSI_CTRL_BIT_IF_E    = 0x08, /* Interrupt flag */
  SRV_CPU_TWSI_CTRL_BIT_ACK_E   = 0x04  /* Ack */
}SRV_CPU_TWSI_CONTROL_BIT;

/* Interrupt machine states */
typedef enum SRV_CPU_TWSI_CONTROLLER_STATE{
  SRV_CPU_TWSI_STATE_IDLE_E           , /* Idle */
  SRV_CPU_TWSI_STATE_STA_E            , /* A START condition should have been transmitted */
  SRV_CPU_TWSI_STATE_TRAN_E           , /* In master transmitter mode, slave address + bytes transmitted */
  SRV_CPU_TWSI_STATE_REC_E            , /* In master receiver mode, slave address transmitted and bytes received */
  SRV_CPU_TWSI_STATE_OFFS_E           , /* In master receiver mode, transmitting offset */
  SRV_CPU_TWSI_STATE_DONE_E             /* Master transaction ended */
 }SRV_CPU_TWSI_CONTROLLER_STATE;

/* The TWSI interface supports both 7-bit and 10-bit addressing. */
/* This enumerator describes addressing type.                    */
typedef enum SRV_CPU_TWSI_ADDR_TYPE
{
  ADDR7_BIT,                            /* 7 bit address  */
  ADDR10_BIT                            /* 10 bit address */
}SRV_CPU_TWSI_ADDR_TYPE;

/* This structure describes TWSI address. */
typedef struct _mvTwsiAddr
{
  MV_U32            address;            /* address      */
  SRV_CPU_TWSI_ADDR_TYPE type;               /* Address type */
}SRV_CPU_TWSI_ADDR;

/* This structure describes a TWSI slave. */
typedef struct SRV_CPU_TWSI_SLAVE_STC
{
  SRV_CPU_TWSI_ADDR slaveAddr;
  MV_BOOL      validOffset;             /* whether the slave has offset (i.e. Eeprom  etc.) */
  MV_U32       offset;                  /* offset in the slave. */
  MV_U32       offset_length;           /* # of bytes that compose the offset */
}SRV_CPU_TWSI_SLAVE_STC;

typedef MV_U8 (*SET_I2C_CHANNEL_FUNC_PTR)
(
    MV_U8                   channel
);

/* Definitions */
#define SRV_CPU_TWSI_EXTERNAL_BUFFER_SIZE 128
#define SRV_CPU_TWSI_BASE_ADDR(interface)   INTER_REGS_BASE + 0x11000 + (interface*0x100)

#define SRV_CPU_TWSI_TRANSMIT     0x00
#define SRV_CPU_TWSI_RECEIVE      0x01


/* Prototypes */
/***********************************************************************
 * registerSetI2cChannelFunc()                                         *
 *    register I2C set channel function pointer                        *
 ***********************************************************************/
MV_VOID registerSetI2cChannelFunc( SET_I2C_CHANNEL_FUNC_PTR funcPtr );

/***********************************************************************
 * srvCpuTwsiInit()                                                    *
 *    Initialize controller and global registers                        *
 ***********************************************************************/
MV_U32 srvCpuTwsiInit(MV_U32 frequency, MV_U32 tclk, MV_U8 interface);

/**
* @internal
*           srvCpuTwsiMasterTransceive function
* @endinternal
*
* @brief   Transmit / receive BUFFER on I2C bus to / from SLAVE_ADDRESS
*
* @param MV_U8* BUFFER:              - pre allocated buffer with length bytes
* @param MV_U32 LENGTH:              - How many bytes to read
* @param SRV_CPU_TWSI_SLAVE SLAVE    - slave structure pre filled with address & offset
* @param MV_U32 TX_RX                - 1 for transmit, 0 for receive.
* @param MV_U8 INTERFACE             - I2C interface     
*
* @retval 0                          - on success
* @retval 1                          - if not ready
*/
MV_U32 srvCpuTwsiMasterTransceive( MV_U8 *buffer,
                                  MV_U32 length,
                                  SRV_CPU_TWSI_SLAVE_STC *slave,
                                  MV_U32 tx_rx,
                                  MV_U8 interface);

/**
* @internal
*           srvCpuI2cRead function
* @endinternal
*
* @brief   Receive BUFFER on I2C bus from SLAVE_ADDRESS
*          The same read interface, but implement with locks semaphore to avoid simultaneous contact to i2c   
*          this wrapper fills the slave structure and using semaphore lock to prevent simulations access to i2c
*
* @param MV_U8* channel  - I2C channel 
*                          (while channel == 0xFF - i2c set channel will be ignored)
* @param MV_U32 address  - i2c address (expected 7 Bit address)
* @param MV_U32 offset   - offset in slave
* @param MV_U32 width    - number of bytes that compose the offset ('0' - for no offset)
* @param MV_U8* buffer   - pre allocated buffer with length bytes
* @param MV_U32 length   - How many bytes to read
*
* @retval 0                          - on success
* @retval 1                          - if not ready
*/
MV_STATUS srvCpuI2cRead(MV_U8 channel, MV_U32 address, MV_U32 offset, MV_U8 width, MV_U8 *buffer, MV_U32 length);

/**
* @internal
*           srvCpuI2cWrite function
* @endinternal
*
* @brief   Transmit BUFFER on I2C bus to SLAVE_ADDRESS 
*          The same write interface, but implement with locks semaphore to avoid simultaneous contact to i2c
*          this wrapper fills the slave structure and using semaphore lock to prevent simulations access to i2c
*
* @param MV_U8* channel  - I2C channel 
*                          (while channel == 0xFF - i2c set channel will be ignored)
* @param MV_U32 address  - i2c address
* @param MV_U32 offset   - offset in slave
* @param MV_U32 width    - number of bytes that compose the offset ('0' - for no offset)
* @param MV_U8* buffer   - pre allocated buffer with length bytes
* @param MV_U32 length   - How many bytes to write
*
* @retval 0                          - on success
* @retval 1                          - if not ready
*/
MV_STATUS srvCpuI2cWrite(MV_U8 channel, MV_U32 address, MV_U32 offset, MV_U8 width, MV_U8 *buffer, MV_U32 length);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCmvTwsiH */
