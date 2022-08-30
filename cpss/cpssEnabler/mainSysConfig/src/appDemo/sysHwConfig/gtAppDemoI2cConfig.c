/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtAppDemoI2cConfig.c
*
* @brief Host I2C device driver module, for nokm mode
*
* @version   1
********************************************************************************
*/

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <gtExtDrv/drivers/gtI2cDrv.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

static CPSS_HW_DRIVER_STC *i2cDrv;

/* enable/disable print i2c functions error */
static GT_BOOL printErrorEnable = GT_TRUE;

GT_STATUS appDemoHostI2cInit(void)
{
     i2cDrv = extDrvI2cCreateDrv(NULL);
     if( NULL == i2cDrv)
         return GT_NO_RESOURCE;

     return GT_OK;
}


/**
* @internal appDemoHostI2cDetect function
* @endinternal
*
* @brief   This routine detects all I2C slaves on a given bus, by writing '0' to
*          the the slave at offset 0.
* Note that calling this func, will modify slaves which have configurable
* register at offset 0 - such as MUXes.
*
* @param[in] bus_id        - the I2C bus id.
*
* @retval GT_OK
*/

GT_STATUS appDemoHostI2cDetect(
    IN GT_U8    bus_id
)
{
#ifndef ASIC_SIMULATION
    GT_STATUS   rc;
    GT_U32      addrSpace;
    GT_U32      i, data = 0;

    if (NULL == i2cDrv) {
        cpssOsPrintf("I2C driver not initialised!\n");
        return GT_NOT_INITIALIZED;
    }

    cpssOsPrintf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    cpssOsPrintf("00:          ");
    for (i=3; i<0x7F; i++) {
        addrSpace = (i & 0xFF) | (bus_id << 8);
        rc = i2cDrv->writeMask(i2cDrv, addrSpace, 0, &data, 1, 0xFFFFFFFF);
        if (rc)
            cpssOsPrintf("-- ");
        else
            cpssOsPrintf("%2x ",i);

        if (!((i+1)%16)) {
            cpssOsPrintf("\n%x: ",i+1);
        }
    }
    cpssOsPrintf("\n");
#else
    cpssOsPrintf("Sim - no i2c devices\n");
#endif

    return GT_OK;
}

/**
 * @internal appDemoHostI2cRead functionh
 * @endinternal
 *
 * @param bus_id
 * @param slave_address
 * @param offset_type
 * @param offset
 * @param size
 * @param buffer
 *
 * @return GT_STATUS
 */
GT_STATUS appDemoHostI2cRead(
    IN GT_U8    bus_id,
    IN GT_U8    slave_address,
    IN EXT_DRV_I2C_OFFSET_TYPE_ENT   offset_type,
    IN GT_U32   offset,
    IN GT_U32   size,
    GT_U8 * buffer
)
{
    GT_STATUS rc = GT_OK;
#ifndef ASIC_SIMULATION
    GT_U32  addrSpace;

    if (NULL == buffer) {
        cpssOsPrintf("Output buffer isn't allocated (%p)\n", buffer);
        return GT_FAIL;
    }

    if (NULL == i2cDrv) {
        cpssOsPrintf("I2C driver not initialised!\n");
        return GT_NOT_INITIALIZED;
    }

    if (size > 64) {
        cpssOsPrintf("size (%d) > 64\n", size);
        return GT_FAIL;
    }



    addrSpace = slave_address + (bus_id << 8) + (offset_type << 16);

    rc = i2cDrv->read(i2cDrv, addrSpace, offset, (GT_U32 *)buffer, size);

    if (GT_OK != rc && printErrorEnable) {
        cpssOsPrintf("appDemoHostI2cRead: bus_id 0x%x, slave_address 0x%x, offset_type 0x%x, offset 0x%x, size 0x%x\n",
                 bus_id, slave_address, offset_type, offset, size);
        cpssOsPrintf("i2c driver read returns with rc %x\n", rc);
    }
#else
    cpssOsPrintf("Sim - no i2c devices\n");
#endif

    return rc;
}

/**
 * @internal appDemoHostI2cDump function
 * @endinternal
 *
 * @param bus_id
 * @param slave_address
 * @param offset_type
 * @param offset
 * @param size
 *
 * @return GT_STATUS
 */
GT_STATUS appDemoHostI2cDump(
    IN GT_U8    bus_id,
    IN GT_U8    slave_address,
    IN EXT_DRV_I2C_OFFSET_TYPE_ENT   offset_type,
    IN GT_U32   offset,
    IN GT_U32   size
)
{
    GT_STATUS rc = GT_OK;
#ifndef ASIC_SIMULATION
    GT_U32  addrSpace;
    GT_U8  buffer[64];
    GT_U32 i;

    if (NULL == i2cDrv) {
        cpssOsPrintf("I2C driver not initialised!\n");
        return GT_NOT_INITIALIZED;
    }

    if (size > 64) {
        cpssOsPrintf("size (%d) > 64\n", size);
        return GT_FAIL;
    }

    cpssOsPrintf("appDemoHostI2cRead: bus_id 0x%x, slave_address 0x%x, offset_type 0x%x, offset 0x%x, size 0x%x\n",
                 bus_id, slave_address, offset_type, offset, size);

    addrSpace = slave_address + (bus_id << 8) + (offset_type << 16);

    rc = i2cDrv->read(i2cDrv, addrSpace, offset, (GT_U32 *)buffer, size);

    if (GT_OK == rc) {
        for ( i = 0; i < size ; i++)
            cpssOsPrintf("0x%x: 0x%x\n", i, buffer[i]);
    } else
        cpssOsPrintf("i2c driver read returns with rc %x\n", rc);
#else
    cpssOsPrintf("Sim - no i2c devices\n");
#endif

    return rc;
}

/**
 *
 * @internal appDemoHostI2cWrite function
 * @endinternal
 *
 * @param bus_id
 * @param slave_address
 * @param offset_type
 * @param offset
 * @param nof_args
 * @param arg1
 * @param arg2
 * @param arg3
 * @param arg4
 *
 * @return GT_STATUS
 */
GT_STATUS appDemoHostI2cWrite(
    IN GT_U8    bus_id,
    IN GT_U8    slave_address,
    IN EXT_DRV_I2C_OFFSET_TYPE_ENT   offset_type,
    IN GT_U32   offset,
    IN GT_U8   nof_args,
    IN GT_U8   arg1,
    IN GT_U8   arg2,
    IN GT_U8   arg3,
    IN GT_U8   arg4
)
{
    GT_STATUS rc = GT_OK;
#ifndef ASIC_SIMULATION
    GT_U32  addrSpace;
    GT_U8   buffer[4];

    if (NULL == i2cDrv) {
        cpssOsPrintf("I2C driver not initialised!\n");
        return GT_NOT_INITIALIZED;
    }

    switch (nof_args) {
    case 4:
        buffer[3] = arg4;
        GT_ATTR_FALLTHROUGH;
    case 3:
        buffer[2] = arg3;
        GT_ATTR_FALLTHROUGH;
    case 2:
        buffer[1] = arg2;
        GT_ATTR_FALLTHROUGH;
    case 1:
        buffer[0] = arg1;
    }

    addrSpace = slave_address + (bus_id << 8) + (offset_type << 16);

    rc = i2cDrv->writeMask(i2cDrv, addrSpace, offset, (GT_U32 *)buffer, nof_args, 0);

    if (GT_OK != rc  && printErrorEnable) {
        cpssOsPrintf("appDemoHostI2cWrite: bus_id 0x%x, slave_address 0x%x, offset_type 0x%x, offset 0x%x\nnof_args %d, arg1 0x%x,arg3 0x%x,arg3 0x%x,arg4 0x%x\n",
                   bus_id, slave_address, offset_type, offset, nof_args, arg1,arg2,arg3,arg4);
        cpssOsPrintf("i2c driver write returns with rc %x\n", rc);
    }
#else
    cpssOsPrintf("Sim - no i2c devices\n");
#endif

    return rc;
}

/**
 *
 * @internal appDemoHostI2cPrintErrorEnableSet function
 * @endinternal
 *
 * @param param[in] en - GT_TRUE - enable printing I2C functions errors
 *                       GT_FALSE - disable printing I2C functions errors
 *
 * @return GT_STATUS
 */
GT_STATUS appDemoHostI2cPrintErrorEnableSet(
    IN GT_BOOL en
)
{
    printErrorEnable = en;
    return GT_OK;
}

