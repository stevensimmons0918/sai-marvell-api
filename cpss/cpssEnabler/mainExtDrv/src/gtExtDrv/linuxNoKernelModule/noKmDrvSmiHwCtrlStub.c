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
* @file noKmDrvSmiHwCtrlStub.c
*
* @brief Stub implementation for SMI facilities.
*
*/

/*Includes*/

#include <gtExtDrv/drivers/gtSmiDrvCtrl.h>

/**
* @internal hwIfSmiInitDriver function
* @endinternal
*
* @brief   Init the SMI interface
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiInitDriver
(
    GT_VOID
)
{
  return GT_FAIL;
}


/**
* @internal hwIfSmiWriteReg function
* @endinternal
*
* @brief   Writes the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to write to.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiWriteReg
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    (void)devSlvId;
    (void)regAddr;
    (void)value;
    return GT_NOT_SUPPORTED;
}

/**
* @internal hwIfSmiReadReg function
* @endinternal
*
* @brief   Reads the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiReadReg
(
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
)
{
    (void)devSlvId;
    (void)regAddr;
    (void)dataPtr;
    return GT_NOT_SUPPORTED;
}

/**
* @internal hwIfSmiTaskWriteReg function
* @endinternal
*
* @brief   Writes the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to write to.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiTaskWriteReg
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    (void)devSlvId;
    (void)regAddr;
    (void)value;
    return GT_NOT_SUPPORTED;
}

/**
* @internal hwIfSmiTaskReadReg function
* @endinternal
*
* @brief   Reads the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiTaskReadReg
(
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
)
{
    (void)devSlvId;
    (void)regAddr;
    (void)dataPtr;
    return GT_NOT_SUPPORTED;
}

/**
* @internal hwIfSmiInterruptWriteReg function
* @endinternal
*
* @brief   Writes the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to write to.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiInterruptWriteReg
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    (void)devSlvId;
    (void)regAddr;
    (void)value;
    return GT_NOT_SUPPORTED;
}

/**
* @internal hwIfSmiInterruptReadReg function
* @endinternal
*
* @brief   Reads the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*
* @param[out] dataPtr                  - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiInterruptReadReg
(
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
)
{
    (void)devSlvId;
    (void)regAddr;
    (void)dataPtr;
    return GT_NOT_SUPPORTED;
}

/**
* @internal extDrvDirectSmiReadReg function
* @endinternal
*
* @brief   Reads the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvDirectSmiReadReg
(
    IN  GT_U32  devSlvId,
    IN  GT_U32  regAddr,
    OUT GT_U32 *value
)
{
    (void)devSlvId;
    (void)regAddr;
    (void)value;
    return GT_NOT_SUPPORTED;
}

/**
* @internal extDrvDirectSmiWriteReg function
* @endinternal
*
* @brief   Writes the unmasked bits of a register using SMI.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] regAddr                  - Register address to read from.
*                                      dataPtr    - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS extDrvDirectSmiWriteReg
(
    IN GT_U32 devSlvId,
    IN GT_U32 regAddr,
    IN GT_U32 value
)
{
    (void)devSlvId;
    (void)regAddr;
    (void)value;
    return GT_NOT_SUPPORTED;
}

/**
* @internal hwIfSmiTskRegRamRead function
* @endinternal
*
* @brief   Reads a memory map (contiguous memory) using SMI from task context.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] addr                     - Register address to start write the writing.
* @param[in] arrLen                   - The length of dataArr (the number of registers to read)
*
* @param[out] dataArr[]                - An array containing the data to be written.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*
* @note - register address is incremented in 4 byte per register
*
*/
GT_STATUS hwIfSmiTskRegRamRead
(
    IN GT_U32       devSlvId,
    IN GT_U32       addr,
    OUT GT_U32      dataArr[],
    IN GT_U32       arrLen
)
{
    (void)devSlvId;
    (void)addr;
    (void)dataArr;
    (void)arrLen;
    return GT_NOT_SUPPORTED;
}

/**
* @internal hwIfSmiTskRegRamWrite function
* @endinternal
*
* @brief   Writes a memory map (contiguous memory) using SMI from task context.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] addr                     - Register address to start write the reading.
* @param[in] arrLen                   - The size of addrArr/dataArr.
* @param[in] dataArr[]                - An array containing the data to be written.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*
* @note - register address is incremented in 4 byte per register
*
*/
GT_STATUS hwIfSmiTskRegRamWrite
(
    IN GT_U32       devSlvId,
    IN GT_U32       addr,
    OUT GT_U32      dataArr[],
    IN GT_U32       arrLen
)
{
    (void)devSlvId;
    (void)addr;
    (void)dataArr;
    (void)arrLen;
    return GT_NOT_SUPPORTED;
}

/**
* @internal hwIfSmiTskRegVecRead function
* @endinternal
*
* @brief   Reads SMI register vector from task context.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] addrArr[]                - Array of addresses to write to.
* @param[in] arrLen                   - The size of addrArr/dataArr.
*
* @param[out] dataArr[]                - An array containing the data to be written.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiTskRegVecRead
(
    IN GT_U32       devSlvId,
    IN GT_U32       addrArr[],
    OUT GT_U32      dataArr[],
    IN GT_U32       arrLen
)
{
    (void)devSlvId;
    (void)addrArr;
    (void)dataArr;
    (void)arrLen;
    return GT_NOT_SUPPORTED;
}

/**
* @internal hwIfSmiTskRegVecWrite function
* @endinternal
*
* @brief   Writes SMI register vector from task context.
*
* @param[in] devSlvId                 - Slave Device ID
* @param[in] addrArr[]                - Array of addresses to write to.
* @param[in] dataArr[]                - An array containing the data to be written.
* @param[in] arrLen                   - The size of addrArr/dataArr.
*
* @retval GT_OK                    - on success
* @retval GT_ERROR                 - on hardware error
*/
GT_STATUS hwIfSmiTskRegVecWrite
(
    IN GT_U32       devSlvId,
    IN GT_U32       addrArr[],
    IN GT_U32       dataArr[],
    IN GT_U32       arrLen
)
{
    (void)devSlvId;
    (void)addrArr;
    (void)dataArr;
    (void)arrLen;
    return GT_NOT_SUPPORTED;
}

/* PX device Id and revision id register address */
#define DEVICE_ID_REG_ADDR_MAC      0x4C
/* PX vendor id register address */
#define VENDOR_ID_REG_ADDR_MAC      0x50
#include <cpss/generic/cpssHwInfo.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <gtExtDrv/drivers/gtGenDrv.h>

/**
* @internal extDrvSmiDevVendorIdGet function
* @endinternal
*
* @brief   This routine returns vendor Id of the given device.
*
* @param[in] instance                 - The requested device instance.
*
* @param[out] vendorId                 <- The device vendor Id.
* @param[out] devId                    <- The device Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - othersise.
*/
GT_STATUS extDrvSmiDevVendorIdGet
(
    OUT GT_U16  *vendorIdPtr,
    OUT GT_U16  *devIdPtr,
    IN  GT_U32  instance
)
{
    GT_STATUS rc=GT_OK;
    CPSS_HW_INFO_STC   hwInfo,*hwInfoPtr=&hwInfo;
    GT_U32  vendorId,devId;

    if(instance < 2)
    {
        /* cause the board to loose link on :
           eth0: link down

           so skipped
        */
        return GT_FAIL;
    }

    osMemSet(&hwInfo, 0, sizeof(hwInfo));
    rc = cpssHwDriverSlaveSmiConfigure("/smi0", instance/*slaveSmiPhyId*/, hwInfoPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    hwInfoPtr->driver->read(hwInfoPtr->driver,3,DEVICE_ID_REG_ADDR_MAC,&devId,1);
    hwInfoPtr->driver->read(hwInfoPtr->driver,3,VENDOR_ID_REG_ADDR_MAC,&vendorId,1);

    *vendorIdPtr = (GT_U16)vendorId;
    *devIdPtr    = (GT_U16)(devId >> 4);/* bits 4..19 */

    return GT_OK;
}

GT_STATUS  extDrvSmiScan(void)
{
    GT_STATUS   rc;
    GT_U16  vendorId,devId;
    GT_U32  instance;

    for(instance = 0 ; instance < 32 ; instance++)
    {
        rc = extDrvSmiDevVendorIdGet(&vendorId,&devId,instance);
        if(rc != GT_OK)
        {
            osPrintf("instance[%d] FAILED[%d]  \n",instance,rc);
            continue;
        }

        osPrintf("instance[%d]   ",instance);
        osPrintf("vendorId[0x%x] ",vendorId);
        osPrintf("devId[0x%x]     \n",devId);
    }

    return GT_OK;
}


GT_STATUS extDrvSmiTestWriteRead
(
    IN  GT_U32  instance,
    IN  GT_U32  ppRegAddr
)
{
    GT_STATUS rc=GT_OK;
    CPSS_HW_INFO_STC   hwInfo,*hwInfoPtr=&hwInfo;
    GT_U32  read_regValue,write_regValue;
    GT_U32  ii;

    if(instance < 2)
    {
        /* cause the board to loose link on :
           eth0: link down

           so skipped
        */
        return GT_FAIL;
    }

    osMemSet(&hwInfo, 0, sizeof(hwInfo));
    rc = cpssHwDriverSlaveSmiConfigure("/smi0", instance/*slaveSmiPhyId*/, hwInfoPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    for(ii = 0, write_regValue = 0x5aa5F77F ; ii < 5000 ; ii++ , write_regValue += 0x12345673)
    {
        hwInfoPtr->driver->writeMask(hwInfoPtr->driver,3,ppRegAddr,&write_regValue,1,0xffffffff);
        hwInfoPtr->driver->read (hwInfoPtr->driver,3,ppRegAddr,&read_regValue,1);

        if(write_regValue != read_regValue)
        {
            osPrintf("error write [0x%x] but read [0x%x] \n",
                write_regValue ,
                read_regValue);
        }
    }

    return GT_OK;
}

