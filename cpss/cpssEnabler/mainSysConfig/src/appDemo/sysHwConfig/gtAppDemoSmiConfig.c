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
* @file gtAppDemoSmiConfig.c
*
* @brief Prestera Devices smi initialization & detection module.
*
* @version   8
********************************************************************************
*/
#include <appDemo/sysHwConfig/gtAppDemoSmiConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <gtExtDrv/drivers/gtSmiHwCtrl.h>
#include <gtExtDrv/drivers/gtGenDrv.h>

#include <cpssCommon/cpssPresteraDefs.h> /* for MARVELL_VENDOR_ID */

#ifndef PRESTERA_NO_HW
#include <appDemo/sysHwConfig/gtAppDemoPciConfig.h>
#endif

/*******************************************************************************
* internal definitions
*******************************************************************************/
#ifdef PRESTERA_DEBUG
#define APP_INIT_DEBUG
#endif

#ifdef APP_INIT_DEBUG
#define DBG_INFO(x)     osPrintf x
#else
#define DBG_INFO(x)
#endif

#define GT_NA ((GT_U32)~0)
#define DEV_ID(gtDevice) ((gtDevice)>>16)
/*******************************************************************************
* Internal usage environment parameters
*******************************************************************************/
static CPSS_PP_DEVICE_TYPE deviceArr[] =
{
    /* hooper devices */
    CPSS_LION2_HOOPER_PORT_GROUPS_0123_CNS,
    /* PIPE devices */
    PRV_CPSS_PIPE_ALL_DEVICES,

    GT_NA/* must be last */
};

#ifndef PRESTERA_NO_HW
static GT_U32   first_time_SMI_access_sleep_time = 100;
/* debug function to allow to set the time to sleep between call to
   appDemoSoCMDCFrequencySet(mode);
   and first time to access the SMI device */
void set_first_time_SMI_access_sleep_time(IN GT_U32 newTime)
{
    first_time_SMI_access_sleep_time = newTime;
}

/*******************************************************************************
* gtPresteraGetSmiDev
*
* DESCRIPTION:
*       This routine searches for Prestera Devices Over the SMI.
*
* INPUTS:
*       first - whether to bring the first device, if GT_FALSE return the next
*               device.
*
* OUTPUTS:
*       smiInfo <- the next device SMI info.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - otherwise.
*       GT_NO_MORE - no more prestera devices.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gtPresteraGetSmiDev
(
    IN  GT_BOOL     first,
    OUT GT_SMI_INFO *smiInfo
)
{
    static GT_U32   deviceIdx = 0;      /* device id index      */
    GT_U32          i;                  /* device instance      */
    GT_U16          vendorId;
    GT_U16          devId;
    GT_U16          devNum;

    /* check parameters */
    if(smiInfo == NULL)
    {
        return GT_BAD_PARAM;
    }
    /* check whether first call */
    if(first == GT_TRUE)
    {
        APP_DEMO_SOC_MDC_FREQUENCY_MODE_ENT mode = appDemoSoCMDCFrequency_getFromDb();
        /* Hooper devices connected to 0-3 */
        deviceIdx = 0x0;

        /* on HW must call next function to allow SMI accessing */
        extDrvSoCInit();

        if(mode <= APP_DEMO_SOC_MDC_FREQUENCY_MODE_ACCELERATED_E) /* check if value was saved ! */
        {
            osPrintf("appDemoSoCMDCFrequencySet : mode[%d] \n",mode);
            /* can't call appDemoSoCMDCFrequencySet() before extDrvSoCInit() */
            appDemoSoCMDCFrequencySet(mode);
        }

        /* the first accessing to the SMI after calling extDrvSoCInit(...)
           should be only after some 'sleep' to get better results !!! */
        osTimerWkAfter(first_time_SMI_access_sleep_time);
    }


    /* call the BSP SMI facility to get the next Prestera device */
    for (i = deviceIdx; i < 32; i++)
    {
        if(extDrvSmiDevVendorIdGet(&vendorId,
                                   &devId,
                                   i) != GT_OK)
        {
            continue;
        }

        /* a device has been found, search in the device list */
        /* if it exists add it to the device list             */
        for (devNum = 0; deviceArr[devNum] != GT_NA ; devNum++)
        {
            if (MARVELL_VENDOR_ID/*0x11ab*/ == vendorId &&
                DEV_ID(deviceArr[devNum]) == devId)
            {
                DBG_INFO(("found the SMI device 0x%04x\n", devId));

                smiInfo->smiDevVendorId.devId       = devId;
                smiInfo->smiDevVendorId.vendorId    = vendorId;
                smiInfo->smiIdSel                   = i;

                deviceIdx = i + 1;

                return GT_OK;
            }
        }
    }
    return GT_NO_MORE;
}

#else


/**
* @internal gtPresteraGetSmiDev function
* @endinternal
*
* @brief   This routine search for Prestera Devices Over the SMI.
*
* @param[in] first                    - whether to bring the  device, if GT_FALSE return the next
*                                      device.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NO_MORE               - no more prestera devices.
*/
GT_STATUS gtPresteraGetSmiDev
(
    IN  GT_BOOL     first,
    OUT GT_SMI_INFO *smiInfo
)
{
    static GT_U32 instance;

    /* check parameters */
    if(smiInfo == NULL)
    {
        return GT_BAD_PARAM;
    }
    if(first == GT_TRUE)
    {
        instance = 0;
    }
    if(i >= 2)
    {
        return GT_NO_MORE;
    }
    i++;
    smiInfo->smiDevVendorId.devId    = DEV_ID(deviceArr[0]);
    smiInfo->smiDevVendorId.vendorId = MARVELL_VENDOR_ID;

    return GT_OK;
}

#endif



