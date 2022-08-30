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
* @file cmdMain.c
*
* @brief command shell entry point implementation
*
*
* @version   31
********************************************************************************
*/

/***** Include files ***************************************************/

#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/common/cmdExtServices.h>

#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/shell/cmdMain.h>
#include <cmdShell/shell/cmdEngine.h>
#include <cmdShell/shell/cmdConsole.h>
#include <cmdShell/shell/cmdSystem.h>
#include <cmdShell/shell/cmdParser.h>
#include <cmdShell/shell/userInitCmdDb.h>
#include <cmdShell/os/cmdStreamImpl.h>
#include <cmdShell/FS/cmdFS.h>
#include <cmdShell/os/cmdExecute.h>
#include <extUtils/luaCLI/luaCLIEngineCLI.h>

#define cmdOsShell NULL


#define MAX_NUM_UNITS 128
#define MSG_EXCHANGING_BASE_SOCKET_PORT 6000


#if (defined LINUX && defined ASIC_SIMULATION)
    #include <string.h>
#endif

/***** Variables *******************************************************/
static GT_32 globalInit = 0;

static GT_U32 prvCmdTelnetPort = 12345;

extern IOStreamPTR defaultStdio;

/* Unit to IP Mapping linked list */
/*STRUCT_LL_HEAD *cmdLlHead;*/

/* cpssInitSystem has been run */
extern GT_BOOL systemInitialized;

/* for task thread safe */
GT_SEM parserMutex = 0;
GT_SEM serverMutex = 0;

GT_U32 taskSerial = 0;

/* Stdio sem -  only one stream can have the stdio */
GT_MUTEX  gStdioMtx;

/***** Public Functions ************************************************/

GT_VOID cmdTelnetPortSet
(
    GT_U32 port
)
{
    prvCmdTelnetPort = port;
}

GT_STATUS tcpKeepAliveTask(
        IOStreamPTR stream
)
{
    char buf[100];
    int k;
    while (stream->connected(stream))
    {
        if ((k = stream->read(stream, buf, sizeof(buf))) > 0)
            stream->write(stream, buf, k);
    }
    stream->destroy(stream);
    return GT_OK;
}

GT_STATUS cmdEventLoop
(
    IOStreamPTR IOStream
);

GT_STATUS tcpServerInstance(
        IOStreamPTR stream
)
{
    /* start keepalive here */
    cmdEventLoop(stream);
    stream->destroy(stream);
    return GT_OK;
}

#ifdef CMD_LUA_CLI
IOStreamPTR IOStreamCreateStdout(void);

GT_STATUS luaCLI_LoadConfigurationFile(
  IN  IOStreamPTR IOStream,
  OUT GT_VOID_PTR *luaInstancePtrPtr
);

#endif /* CMD_LUA_CLI */
#ifdef  CMD_FS
GT_STATUS cmdTFTPDstart(void);
#endif
#ifdef _WIN32
IOStreamPTR cmdStreamCreateWin32Console(void);
#endif

#ifdef CMD_INCLUDE_TCPIP
static const char* tcpServices[] = {
    MASTER_SOCKET_SERVICE_NAME,
    "cmdShell",
    NULL
};
#endif

/**
* @internal cmdInit function
* @endinternal
*
* @brief   commander initialization (entry point); this function spawns
*         configured streaming threads and immediately returns to caller.
* @param[in] devNum                   - The device number of the serial port.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - failed to allocate memory for socket
* @retval GT_FAIL                  - on error
*
* @note For a new pemanent connection (like serial one) add the following:
*       IOStreamPTR stream;
*       ...
*       stream = cmdStreamCreate....();
*       cmdStreamStartEventLoopHandler(...);
*       To accept tcp connections on second port:
*       cmdStreamSocketServiceCreate(...);
*       cmdStreamSocketServiceStart(...);
*
*/
GT_STATUS cmdInit
(
    IN  GT_U32  devNum
)
{
#ifdef CMD_INCLUDE_SERIAL
    GT_TASK taskSerial = 0;
#endif

    GT_U32 taskId;

#ifndef PSS_PRODUCT
    /* initialize external services (can't call directly to mainOs functions)*/
    if(GT_OK != cmdInitExtServices())
    {
        return GT_FAIL;
    }
#endif


#if (defined PSS_PRODUCT) || (defined CPSS_PRODUCT)
    if (osWrapperOpen(NULL) != GT_OK)
    {   /* os wrapper failed to initialize */
        cmdOsPrintf("osWrapper initialization failure!\n");
        return GT_FAIL;
    }
#endif

    if (cmdStreamSocketInit() != GT_OK)
    {   /* os wrapper failed to initialize */
        cmdOsPrintf("cmdStreamSocketInit() initialization failure!\n");
        return GT_FAIL;
    }
#ifdef CMD_INCLUDE_SERIAL
    if (cmdStreamSerialInit() != GT_OK)
    {   /* os wrapper failed to initialize */
        cmdOsPrintf("cmdStreamSocketInit() initialization failure!\n");
        return GT_FAIL;
    }
#endif

    /* was before - osMutexCreate("parser", &parserMutex) */
    if (cmdParseInit(NULL) == GT_FAIL ||
        cmdOsMutexCreate("server", &serverMutex) != GT_OK)
    {
        cmdOsPrintf("failed to create mutex descriptors!\n");
        return GT_FAIL;
    }

    /* check if we already running */
    if (globalInit++)
        return GT_FAIL;
#ifdef IMPL_GALTIS
    /* initialize commander database */
    if (cmdCpssInitDatabase() != GT_OK)
    {
        cmdOsPrintf("failed to initialize command database!\n");
        return GT_FAIL;
    }
#endif
    /* add test app level commands to commander database */
    if (cmdUserInitDatabase() != GT_OK)
    {
        cmdOsPrintf("failed to initialize command database!\n");
        return GT_FAIL;
    }
    if (cmdEngineInit() != GT_OK)
    {
            cmdOsPrintf("failed to initialize command engine!\n");
       return GT_FAIL;
    }

    /* Allocate stdio semaphore */
    if (cmdOsMutexCreate("cmdStdioMtx", &gStdioMtx) != GT_OK)
    {
        cmdOsPrintf("failed to initialize stdio semaphore!\n");
        return GT_FAIL;
    }


#ifdef CMD_INCLUDE_SERIAL
    defaultStdio = cmdStreamCreateSerial(devNum);
    if (defaultStdio != NULL)
    {
        cmdTakeGlobalStdio(defaultStdio);
        cmdReleaseGlobalStdio();
    }
#else
    defaultStdio = cmdStreamCreateNULL();
#endif /* CMD_INCLUDE_SERIAL */
#ifdef  _WIN32
    if (devNum == 0)
    {
        /* if appDemo started with -serial 0 use console */
        if (defaultStdio != NULL)
        {
            defaultStdio->destroy(defaultStdio);
        }
        defaultStdio = cmdStreamCreateWin32Console();
        cmdTakeGlobalStdio(defaultStdio);
        cmdReleaseGlobalStdio();
    }
#endif

    if (defaultStdio == NULL)
    {
#ifdef CMD_LUA_CLI
        defaultStdio = IOStreamCreateStdout();
#else
        defaultStdio = IOStreamCreateNULL();
#endif
    }
    cmdStreamRedirectStdout(defaultStdio);

#if 0
#ifdef _WIN32
    {
        extern GT_U32 simulationInitReady;

        while(simulationInitReady == 0)
        {
            /* wait for simulation to be in operational state */
            cmdOsTimerWkAfter(500);
        }
    }
#endif /*_WIN32*/
#endif


#ifdef CMD_INCLUDE_TCPIP
#ifdef  CMD_LUA_CLI
#ifndef DISABLE_CLI_SERVICE
    cmdStreamSocketServiceCreate(
            MASTER_SOCKET_SERVICE_NAME,
            NULL, prvCmdTelnetPort/*port*/,
            cmdSocketInstanceHandler,
            GT_TRUE/*multipleInstances*/,
            GT_TRUE/*isConsole*/);
    /* initialize LUA CLI */
    if (cmdStreamSocketServiceStart(MASTER_SOCKET_SERVICE_NAME) != GT_OK)
    {
        cmdOsPrintf("commander: failed to create LUA CLI listening task\n");
    }
#endif /*DISABLE_CLI_SERVICE*/
#ifndef _WIN32
    cmdStreamSocketServiceCreate(
            "luaCLI_uds",
            NULL, 0, /*port is not used for UDS socket*/
            cmdSocketInstanceHandler,
            GT_TRUE/*multipleInstances*/,
            GT_TRUE/*isConsole*/);
    /* initialize LUA CLI with uds support*/
    if (cmdStreamSocketServiceStart("luaCLI_uds") != GT_OK)
    {
        cmdOsPrintf("commander: failed to create LUA CLI UDS listening task\n");
    }
#endif /*_WIN32*/
#endif
#ifdef  CMD_FS
    cmdFSinit();
    cmdTFTPDstart();

#endif
#ifndef DISABLE_SHELL_SERVICE
    cmdStreamSocketServiceCreate(
            "cmdShell",
            NULL, CMD_SERVER_PORT,
            tcpServerInstance,
            GT_TRUE/*multipleInstances*/,
            GT_TRUE/*isConsole*/);
    if (cmdStreamSocketServiceStart("cmdShell") != GT_OK)
    {
        cmdOsPrintf("commander: failed to create tcpip server thread\n");
    }
#endif /*DISABLE_SHELL_SERVICE*/
#ifndef DISABLE_KEEPALIVE_SERVICE
    cmdStreamSocketServiceCreate(
            "TCPKEEPALIVE",
            NULL, CMD_KA_SERVER_PORT,
            tcpKeepAliveTask,
            GT_TRUE/*multipleInstances*/,
            GT_FALSE/*isConsole*/);
    if (cmdStreamSocketServiceStart("TCPKEEPALIVE") != GT_OK)
    {
        cmdOsPrintf("commander: failed to create tcpip keepalive task thread\n");
    }
#endif /*DISABLE_KEEPALIVE_SERVICE*/
#endif /* CMD_INCLUDE_TCPIP */

    /* Create one single task to handle all services */
    if (cmdOsTaskCreate(
                "CPSSGenServer",
                STREAM_THREAD_PRIO+1,   /* thread priority          */
                65536,                  /* use default stack size   */
                (unsigned (__TASKCONV *)(void*))
                    cmdStreamSocketServiceListenerTask,
                NULL,
                &(taskId)) != GT_OK)
        {
            cmdOsPrintf("commander: failed to create CPSSGenServer task thread\n");
        }

    {
        char services[80] = {0};
#ifdef CMD_INCLUDE_TCPIP
        int i;
        for (i = 0; tcpServices[i]; i++)
        {
            if (cmdStreamSocketServiceIsRunning(tcpServices[i]) == GT_TRUE)
            {
                cmdOsStrCat(services, " ");
                cmdOsStrCat(services, tcpServices[i]);
            }
        }
#endif
        if (taskSerial)
        {
            cmdOsStrCat(services, " SERIAL");
        }
        if (!services[0])
        {
            cmdOsStrCpy(services, " NONE");
        }
        cmdOsPrintf("commander: threads are running:%s\n\n", services);
    }

#if 0
    /* initialize remote ip detection thread */
    if (cmdRemoteIpDetectionServerCreate() != GT_OK)
    {
        cmdOsPrintf("commander: fail to create remoteip detection server thread\n");
    }
#endif

#ifdef CMD_LUA_CLI
    luaCLI_LoadConfigurationFile(
#ifdef CMD_INCLUDE_SERIAL
            defaultStdio
#else
            NULL
#endif
            , &(defaultStdio->customPtr));

#endif

#ifdef CMD_INCLUDE_SERIAL
    defaultStdio->flags |= IO_STREAM_FLAG_C_FIRSTSHELL;
    /* initialize serial server thread */
    if (cmdStreamStartEventLoopHandler(
                "SERIAL",
                cmdEventLoop,
                defaultStdio,
                GT_TRUE/*isConsole*/,
                &taskSerial) != GT_OK)
    {
        cmdOsPrintf("commander: failed to spawn serial thread\n");
        taskSerial = 0;
    }
#endif /* CMD_INCLUDE_SERIAL */


    return GT_OK;
}


