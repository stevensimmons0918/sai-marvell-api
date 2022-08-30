/*******************************************************************************
*              (c), Copyright 2020, Marvell International Ltd.                 *
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
* @file main.c
*
* @brief Main file of standalone simulation
*
* @version   1
********************************************************************************
*/

#include<gtOs/gtOsMem.h>
#include<gtOs/gtOsIo.h>
#include <gtStack/gtOsSocket.h>

#include <os/simTypesBind.h>
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#include <os/simOsBindOwn.h>
#undef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES

#include <asicSimulation/SInit/sinit.h>
#include <asicSimulation/wmApi.h>
#include <unistd.h>
#include <errno.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GT_VOID  simulationDistributedAsicSet(void);

GT_STATUS osTimerWkAfter(    IN GT_U32 mils);

GT_STATUS osWrapperOpen(void * osSpec);

/* commandLine should belong to this module to make mainOs module workable
 * without CPSS Enabler into another third-party client. */
char  commandLine[1280] ;

/*For debug .This function starty simulation log with from cpu.*/
GT_STATUS wm__sslw(void);



char ** cmdArgv;
char ** cmdEnv;
/* commandLine should belong to simulation (simulation can be used by another client withot this var) */
extern char commandLine[1280];

#define BIND_FUNC(funcName)        \
                        simOsBindInfo.funcName = funcName

#define BIND_LEVEL_FUNC(level,funcName) \
            simOsBindInfo.level.funcName = funcName

SIM_OS_FUNC_BIND_STC simOsBindInfo;

static void simulationBindOsFuncs(void)
{
        /* reset all fields of simOsBindInfo */
        osMemSet(&simOsBindInfo,0,sizeof(simOsBindInfo));

        /* let the simulation OS bind it's own functions to the simulation */
        simOsFuncBindOwnSimOs();

        simOsBindInfo.osWrapperOpen = osWrapperOpen;

        BIND_LEVEL_FUNC(sockets,osSocketTcpCreate);
        BIND_LEVEL_FUNC(sockets,osSocketUdpCreate);
        BIND_LEVEL_FUNC(sockets,osSocketTcpDestroy);
        BIND_LEVEL_FUNC(sockets,osSocketUdpDestroy);
        BIND_LEVEL_FUNC(sockets,osSocketCreateAddr);
        BIND_LEVEL_FUNC(sockets,osSocketDestroyAddr);
        BIND_LEVEL_FUNC(sockets,osSocketBind);
        BIND_LEVEL_FUNC(sockets,osSocketListen);
        BIND_LEVEL_FUNC(sockets,osSocketAccept);
        BIND_LEVEL_FUNC(sockets,osSocketConnect);
        BIND_LEVEL_FUNC(sockets,osSocketSetNonBlock);
        BIND_LEVEL_FUNC(sockets,osSocketSetBlock);
        BIND_LEVEL_FUNC(sockets,osSocketSend);
        BIND_LEVEL_FUNC(sockets,osSocketSendTo);
        BIND_LEVEL_FUNC(sockets,osSocketRecv);
        BIND_LEVEL_FUNC(sockets,osSocketRecvFrom);
        BIND_LEVEL_FUNC(sockets,osSocketSetSocketNoLinger);
        BIND_LEVEL_FUNC(sockets,osSocketExtractIpAddrFromSocketAddr);
        BIND_LEVEL_FUNC(sockets,osSocketGetSocketAddrSize);
        BIND_LEVEL_FUNC(sockets,osSocketShutDown);

        BIND_LEVEL_FUNC(sockets,osSelectCreateSet);
        BIND_LEVEL_FUNC(sockets,osSelectEraseSet);
        BIND_LEVEL_FUNC(sockets,osSelectZeroSet);
        BIND_LEVEL_FUNC(sockets,osSelectAddFdToSet);
        BIND_LEVEL_FUNC(sockets,osSelectClearFdFromSet);
        BIND_LEVEL_FUNC(sockets,osSelectIsFdSet);
        BIND_LEVEL_FUNC(sockets,osSelectCopySet);
        BIND_LEVEL_FUNC(sockets,osSelect);
        BIND_LEVEL_FUNC(sockets,osSocketGetSocketFdSetSize);

        /* this needed for binding the OS of simulation with our OS functions */
        simOsFuncBind(&simOsBindInfo);

        return;
}


void simulationStart (void)
{

    /* Must be called before use any of OS functions. */
    osWrapperOpen(NULL);

    osPrintf("Simulation init start....\n");

    simulationBindOsFuncs();

    /* call simulation init */
    simulationLibInit();
    /*debug*/
    /*wm__sslw();*/

    osPrintf("\nSimulation init done\n");

    while(1)
    {
        osTimerWkAfter(1000);
    }

}




int main(int argc, const char * argv[])
{
   int i,len=0;

   cmdArgv = calloc(argc+1, sizeof(char*));
    if (cmdArgv == NULL)
    {
        fprintf(stderr, "calloc(%d, %d) failed: %s\n",
                argc+1, (int)sizeof(char*), strerror(errno));
        return 1;
    }
    commandLine[0] = 0;
    for (i=0; i<argc; i++)
    {
        len += strlen(argv[i]);
        cmdArgv[i] = (char *)argv[i];
        if ((unsigned)len < sizeof(commandLine)-2) /*for ' ' and '\0' */
        {
            strcat(commandLine,argv[i]);
            if (i<argc-1)
            {
                strcat(commandLine," ");
                len++;
            }
        }

    }
    cmdArgv[argc] = 0;

    simulationDistributedAsicSet();

    simulationStart();

   return 0;
}
