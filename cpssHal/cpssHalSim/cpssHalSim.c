/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* cpssHalInitialize.c
*
* DESCRIPTION:
*       initialize system
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifdef ASIC_SIMULATION

#include <cpssHalSim.h>
#include <cpssHalSimPrv.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/generic/cpssTypes.h>
#include <cpss/common/init/cpssInit.h>
#include <cpss/extServices/cpssExtServices.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/generic/log/prvCpssLog.h>

#include <gtOs/gtOsGen.h>
#include <gtExtDrv/drivers/gtIntDrv.h>
#include <gtOs/gtEnvDep.h>

//#include <asicSimulation/SInit/sinit.h>

#include <gtExtDrv/drivers/gtIntDrv.h>
#include <gtExtDrv/drivers/gtPciDrv.h>
#include <gtExtDrv/drivers/gtCacheMng.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>
#include <gtExtDrv/drivers/gtDragoniteDrv.h>
#include <gtExtDrv/drivers/gtEthPortCtrl.h>
#include <gtExtDrv/drivers/gtHsuDrv.h>
#include <gtExtDrv/drivers/gtSmiDrvCtrl.h>
#include <gtExtDrv/drivers/gtSmiHwCtrl.h>

#include <gtStack/gtOsSocket.h>

#include <gtOs/gtOsExc.h>
#include <gtOs/gtOsInet.h>
#include <gtOs/gtOsStdLib.h>
#include <gtOs/gtOsMsgQ.h>

#include <gtUtil/gtBmPool.h>

#include <cmdShell/common/cmdExtServices.h>
#include <cpss/generic/events/cpssGenEventUnifyTypes.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define LINUX_SIM

#include <os/simTypesBind.h>

/* define next -- must by before any include of next os H files */
#ifndef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#endif /*EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES*/

#include <os/simOsBindOwn.h>

#include <asicSimulation/SInit/sinit.h>

/* IntLockmutex should belong to simulation (simulation can be used without Enabler)! */
extern GT_MUTEX IntLockmutex;
extern char commandLine[128];

#define BIND_FUNC(funcName)        \
                        simOsBindInfo.funcName = funcName

#define BIND_LEVEL_FUNC(level,funcName) \
            simOsBindInfo.level.funcName = funcName

extern void scibPciRegRead
(
    GT_U32        deviceId,
    GT_U32        memAddr,
    GT_U32        length,
    GT_U32 *      dataPtr
);

extern void scibCoreClockRegisterUpdate
(
    GT_U32                  deviceId,
    GT_U32                  coreClockInMHz,
    GT_U32                  hwFieldValue
);

/*******************************************************************************
* cpssHalInitServicesSimulationBindOsFuncs
*
* DESCRIPTION:
*    Binds simulation OS functions
* INPUTS:
*       None.
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*
*******************************************************************************/
void cpssHalInitServicesSimulationBindOsFuncs(void)
{

    SIM_OS_FUNC_BIND_STC simOsBindInfo;

    /* reset all fields of simOsBindInfo */
    osMemSet(&simOsBindInfo, 0, sizeof(simOsBindInfo));

    /* let the simulation OS bind it's own functions to the simulation */
    simOsFuncBindOwnSimOs();

    BIND_FUNC(osWrapperOpen);

    BIND_LEVEL_FUNC(sockets, osSocketTcpCreate);
    BIND_LEVEL_FUNC(sockets, osSocketUdpCreate);
    BIND_LEVEL_FUNC(sockets, osSocketTcpDestroy);
    BIND_LEVEL_FUNC(sockets, osSocketUdpDestroy);
    BIND_LEVEL_FUNC(sockets, osSocketCreateAddr);
    BIND_LEVEL_FUNC(sockets, osSocketDestroyAddr);
    BIND_LEVEL_FUNC(sockets, osSocketBind);
    BIND_LEVEL_FUNC(sockets, osSocketListen);
    BIND_LEVEL_FUNC(sockets, osSocketAccept);
    BIND_LEVEL_FUNC(sockets, osSocketConnect);
    BIND_LEVEL_FUNC(sockets, osSocketSetNonBlock);
    BIND_LEVEL_FUNC(sockets, osSocketSetBlock);
    BIND_LEVEL_FUNC(sockets, osSocketSend);
    BIND_LEVEL_FUNC(sockets, osSocketSendTo);
    BIND_LEVEL_FUNC(sockets, osSocketRecv);
    BIND_LEVEL_FUNC(sockets, osSocketRecvFrom);
    BIND_LEVEL_FUNC(sockets, osSocketSetSocketNoLinger);
    BIND_LEVEL_FUNC(sockets, osSocketExtractIpAddrFromSocketAddr);
    BIND_LEVEL_FUNC(sockets, osSocketGetSocketAddrSize);
    BIND_LEVEL_FUNC(sockets, osSocketShutDown);

    BIND_LEVEL_FUNC(sockets, osSelectCreateSet);
    BIND_LEVEL_FUNC(sockets, osSelectEraseSet);
    BIND_LEVEL_FUNC(sockets, osSelectZeroSet);
    BIND_LEVEL_FUNC(sockets, osSelectAddFdToSet);
    BIND_LEVEL_FUNC(sockets, osSelectClearFdFromSet);
    BIND_LEVEL_FUNC(sockets, osSelectIsFdSet);
    BIND_LEVEL_FUNC(sockets, osSelectCopySet);
    BIND_LEVEL_FUNC(sockets, osSelect);
    BIND_LEVEL_FUNC(sockets, osSocketGetSocketFdSetSize);

    /* this needed for binding the OS of simulation with our OS functions */
    simOsFuncBind(&simOsBindInfo);

    return;
}


/*******************************************************************************
* cpssHalSimInitApi
*
* DESCRIPTION:
*       Initizeses PP simulation
*
*
* APPLICABLE DEVICES:
*        Bobcat3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on general error
*
* COMMENTS:
*       Before calling cpssHalSimInit commandLine string must be set in the following format:
*           appName -e "simulation ini file". Example: appDemoSim -e bobcat3_A0_pss_wm.ini.
*
*******************************************************************************/
GT_STATUS cpssHalSimInitApi(
    PROFILE_STC * profile
)
{
    char   semName[50];
    int i;
    const char *ini_file = NULL;

    /* check if commandLine is empty */
    if (commandLine[0] == 0)
    {
        i=0;
        while (profile[i].profileType != PROFILE_TYPE_LAST_E)
        {
            if (profile[i].profileType  == PROFILE_TYPE_SIM_INIFILE_E)
            {
                printf("%s\n", profile[i].profileValue.sim_inifile);
                ini_file=profile[i].profileValue.sim_inifile;
                break;
            }
            i++;
        }
        if (ini_file==NULL)
        {
            printf("\nMissing WM ini file in profile\n");
            return GT_BAD_VALUE;
        }
        sprintf(commandLine, "mainMrvlHal_x86 %s", ini_file);
    }
    else
    {
        ini_file = strstr(commandLine, "-e");
        if (ini_file==NULL)
        {
            printf("\nMissing WM ini file, format has to be -i <ini_file.ini>\n");
            return GT_BAD_VALUE;
        }
        ini_file +=2;
    }

    printf("\nWM ini file: %s\n", ini_file);

    /* all input/output be on local console */
    setenv("PSEUDO", "/dev/tty", 1);

    /* Must be called before use any of OS functions. */
    osWrapperOpen(NULL);
    /* create semaphores */
    osSprintf(semName, "IntLockmutex");

    /* create 'reentrant' mutex ! */
    if (osMutexCreate(semName, &IntLockmutex) != 0)
    {
        exit(0);
    }

    cpssHalInitServicesSimulationBindOsFuncs();

#ifndef RTOS_ON_SIM
    /* call simulation init */
    simulationLibInit();
    while (simulationInitReady == 0)
    {
        cpssOsTimerWkAfter(200);
        cpssOsPrintf(".");
    }


#endif /*RTOS_ON_SIM*/

    return GT_OK;
}


/*******************************************************************************
* cpssHalInitServicesSimulationFindPCIDev
*
* DESCRIPTION:
*    Binds simulation OS functions
* INPUTS:
*       None.
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*
*******************************************************************************/
GT_STATUS cpssHalInitServicesSimulationFindPCIDev(
    GT_PCI_DEV_VENDOR_ID *device,
    IN  GT_U32          *instance_ptr,
    INOUT   GT_U32          *deviceIdx_ptr,
    GT_U32          *busNo_ptr,
    GT_U32          *deviceNo_ptr,
    GT_U32          *funcNo_ptr
)
{

    /* optimize the number of times calling the device to get PCI/PEX info */
    GT_U32  regData;
    for (/*continue*/; (*instance_ptr) < 256; (*instance_ptr)++)
    {
        scibPciRegRead(*instance_ptr, 0, 1, &regData);
        if (regData == 0xFFFFFFFF)
        {
            /* the 'instance' is not exists on the PCI/PEX bus */
            continue;
        }

        for (*deviceIdx_ptr = 0; device[(*deviceIdx_ptr)].vendorId ; (*deviceIdx_ptr)++)
        {
            /* check if the device is 'known device' */
            if (regData == (device[*deviceIdx_ptr].vendorId  | (GT_U32)(
                                device[*deviceIdx_ptr].devId << 16)))
            {
                *busNo_ptr = 0;/* not used by asic simulation */
                *deviceNo_ptr = *instance_ptr;
                *funcNo_ptr = 0;/* not used by asic simulation */

                /* found device , update instance for next iteration*/
                (*instance_ptr)++;
                return GT_OK;
            }
        }
    }

    /* get here only when not doing 'goto foundInstanceOnPex_lbl' */
    return GT_NO_MORE;
}

/*******************************************************************************
* cpssHalInitServicesSimulationPCIInfoUpdate
*
* DESCRIPTION:
*    Binds simulation OS functions
* INPUTS:
*       None.
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*
*******************************************************************************/
void cpssHalInitServicesSimulationPCIInfoUpdate(
    INOUT GT_PCI_INFO *pciInfo
)
{


#if defined LINUX_SIM
    /* Correct Base Addresses for PEX devices */
    /* Internal registers BAR is in the 0x18 reg and only 16 MSB are used */
    pciInfo->pciBaseAddr = pciInfo->pciHeaderInfo[6] & 0xFFFF0000;
    /* correct Internal PCI base to compensate CPSS Driver +
            0x70000 / 0xf0000 PEX offset  */
    pciInfo->internalPciBase &= 0xFFFF;

    if (pciInfo->pciBaseAddr != 0)
    {
        pciInfo->resetAndInitControllerBase = pciInfo->pciBaseAddr + _64M;
    }

#endif

    return;
}
#endif
