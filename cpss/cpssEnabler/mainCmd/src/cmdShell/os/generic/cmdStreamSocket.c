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
* @file cmdStreamSocket.c
*
* @brief This file contains common functions for mainCmd socket streams
*
* @version   6
********************************************************************************
*/
#include <cmdShell/os/cmdStreamImpl.h>
#include <extUtils/IOStream/IOStreamTCP.h>
#include <extUtils/luaCLI/luaCLIEngine.h>
#include <extUtils/luaCLI/luaCLIEngineCLI.h>
#include <cmdShell/FS/cmdFS.h>
#include <stdio.h>
#include <signal.h>

# include "../../lua/cmdLua_os_mainCmd.inc"
#ifdef ENV_POSIX
# include "../../lua/cmdLua_os_posix.inc"
#endif

#define cmdOsShell NULL
/*********** generic socket stream implementation *****************************/


/*********** Socket listener implementation **********************************/

#ifndef WIN32
/**
* @internal cmdStreamSocketInit function
* @endinternal
*
* @brief   Initialize TCP/IP socket engine
*/
GT_STATUS cmdStreamSocketInit(void)
{
    signal(SIGPIPE, SIG_IGN);
    return GT_OK;
}

/**
* @internal cmdStreamSocketFinish function
* @endinternal
*
* @brief   Close socket engine
*/
GT_STATUS cmdStreamSocketFinish(void)
{
    return GT_OK;
}

#endif

/**
* @internal osSocketTcpCreateListen function
* @endinternal
*
* @brief   Create TCP socket, bind it and start listening for incoming connections
*
* @param[in] ip                       -  address to listen on
*                                      NULL means listen on all interfaces
* @param[in] port                     - tcp  for incoming connections
*
* @param[out] socketPtr                - Store socket descriptior
*                                       GT_OK
*                                       GT_FAIL
*/
GT_STATUS osSocketTcpCreateListen
(
    IN  char *ip,
    IN  int port,
    OUT GT_SOCKET_FD *socketPtr
)
{
    GT_SOCKET_FD    socket;
    GT_VOID*        bindAddr;
    GT_SIZE_T       bindAddrLen;

    if (!socketPtr)
        return GT_BAD_PARAM;

    /* create master server socket and start listening on specified ip/port */
    socket = cmdOsSocketTcpCreate(GT_SOCKET_DEFAULT_SIZE);
    if (socket < 0)
        return GT_FAIL;

    if (cmdOsSocketCreateAddr(ip, port, &bindAddr, &bindAddrLen) != GT_OK)
    {
        cmdOsSocketTcpDestroy(socket);
        return GT_FAIL;
    }

    if (cmdOsSocketBind(socket, bindAddr, bindAddrLen) != GT_OK)
    {
        cmdOsSocketDestroyAddr(bindAddr);
        cmdOsSocketTcpDestroy(socket);
        return GT_FAIL;
    }

    cmdOsSocketDestroyAddr(bindAddr);


    if (cmdOsSocketListen(socket, 32) != GT_OK)
    {
        cmdOsSocketTcpDestroy(socket);
        return GT_FAIL;
    }

    *socketPtr = socket;
    return GT_OK;
}


GT_STATUS osSocketUdsCreateListen
(
    OUT GT_SOCKET_FD *socketPtr
)
{
    GT_SOCKET_FD    socket;

    if (!socketPtr)
        return GT_BAD_PARAM;

    socket = cmdOsSocketUdsCreate();
    if (socket < 0)
        return GT_FAIL;

    if (cmdOsSocketListen(socket, 32) != GT_OK)
    {
        return GT_FAIL;
    }

    *socketPtr = socket;
    return GT_OK;

}

/**
* @struct SERVICE_PARAMS_STC
* @endinternal
*
* @brief   Service parameters structure
*
*   This structure is used as service description
*   to newly incoming connection handled by one single task
*
*/
typedef struct SERVICE_PARAMS_STC {
    char                       *serviceName;
    char                       *ip;
    int                         port;
    GT_BOOL                     multipleInstances;
    GT_BOOL                     isConsole;
    GT_BOOL                     running;
    GT_BOOL                     onHold;
    eventLoopHandlerPTR         handler;
    GT_SOCKET_FD                masterSocket;
    IOStreamPTR                 stream;
    CPSS_LUACLI_INSTANCE_STC    luaCLI;
} SERVICE_PARAMS_STC;

#define SOCKET_TIMEOUT  100
#define SERVICES_MAX    10
SERVICE_PARAMS_STC  services[SERVICES_MAX];
int                 srvIndex    = 0;
static int          numServices = 0;
static GT_SOCKET_FD max_fd      = -1;
static GT_VOID     *rdSet;

#ifdef CMD_LUA_CLI
GT_STATUS luaGlobalGetIntVal(
  IN  const char *key,
  OUT GT_U32     *valuePtr
);
#endif

/**
* @internal cmdStreamSocketServiceCreate function
* @endinternal
*
* @brief   Create socket listener service
*
* @param[in] serviceName              - service name
* @param[in] ip                       -  address to listen on
*                                      NULL means listen on all interfaces
* @param[in] port                     tcp  for incoming connections
* @param[in] handler                  - pointer to  function
* @param[in] multipleInstances        - Allow more than one handler at time
* @param[in] isConsole                - application flag
*                                       GT_OK
*                                       GT_FAIL
*/
GT_STATUS cmdStreamSocketServiceCreate(
    IN  char *serviceName,
    IN  char *ip,
    IN  GT_U16 port,
    IN  eventLoopHandlerPTR handler,
    IN  GT_BOOL multipleInstances,
    IN  GT_BOOL isConsole
)
{
    int i;
    SERVICE_PARAMS_STC *s;

    if (numServices == SERVICES_MAX)
        return GT_NO_RESOURCE;

    for (i = 0; i < SERVICES_MAX; i++)
    {
        if ( services[i].serviceName == NULL )
            continue;
        if (cmdOsStrCmp(serviceName, services[i].serviceName) == 0)
            return GT_ALREADY_EXIST;
    }

    /* search for an empty service */
    for (i = 0; i < SERVICES_MAX; i++)
    {
        if ( services[i].serviceName == NULL )
            break;
    }
    s = &(services[i]);
    cmdOsMemSet(s, 0, sizeof(*s));

    s->serviceName = (char*)cmdOsMalloc(cmdOsStrlen(serviceName)+1);
    cmdOsStrCpy(s->serviceName, serviceName);
    if (ip)
    {
        s->ip = (char*)cmdOsMalloc(cmdOsStrlen(ip)+1);
        cmdOsStrCpy(s->ip, ip);
    }
    s->port = port;
#ifdef CMD_LUA_CLI
    {
        /* override port with _p_${serviceName} from luaGlobal, if exists */
        char varname[80];
        GT_U32 portNum;

        cmdOsSprintf(varname, "_p_%s", s->serviceName);
        if (luaGlobalGetIntVal(varname, &portNum) == GT_OK)
        {
            s->port = (int)portNum;
        }
    }
#endif
    s->handler = handler;
    s->multipleInstances = multipleInstances;
    s->isConsole = isConsole;
    s->running = GT_FALSE;

    cmdOsMemSet(&(s->luaCLI), 0, sizeof(CPSS_LUACLI_INSTANCE_STC));

    numServices++;
    return GT_OK;
}

/**
* @internal luaCLI_tcpInstanceCreate function
* @endinternal
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS luaCLI_tcpInstanceCreate
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    /* add telnet protocol layer */
    IOStreamPTR telnetStream;
    telnetStream = IOStreamCreateTelnet(services[srvIndex].stream);
    if (telnetStream == NULL)
    {
        services[srvIndex].stream->destroy(services[srvIndex].stream);
        return GT_FAIL;
    }
    /* It is a place to add
     * login/password check here over telnetStream
     */

    services[srvIndex].stream = telnetStream;

#ifdef CMD_LUA_CLI
    rc = luaCLIEngineEventInit(services[srvIndex].stream, &cmdFS,
        (LUA_CLI_GETFUNCBYNAME_T)osShellGetFunctionByName, cmdOsShell,
        &services[srvIndex].luaCLI.L, &services[srvIndex].luaCLI.line);
#endif
    if ( rc != GT_OK )
        telnetStream->destroy(telnetStream);

    return rc;
}

/*******************************************************************************
* cmdSocketInstanceHandler
*
* DESCRIPTION:
*       Socket instance service Handler
*       handle one command over CLI
*
* INPUTS:
*       IOStreamPTR stream
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cmdSocketInstanceHandler
(
    IOStreamPTR stream
)
{
    GT_STATUS rc = GT_OK;


#ifdef CMD_LUA_CLI
    if ( services[srvIndex].onHold == GT_FALSE ) {
        rc = luaCLIEngine_Event(stream, cmdOsShell, &services[srvIndex].luaCLI, 0);

        if ( rc != GT_OK && luaRestartRequest == 1){
            luaRestartRequest = 0;
            rc = luaCLIEngineEventInit(stream, &cmdFS,
                (LUA_CLI_GETFUNCBYNAME_T)osShellGetFunctionByName, cmdOsShell,
                &services[srvIndex].luaCLI.L, &services[srvIndex].luaCLI.line);

            if ( rc != GT_OK )
                stream->destroy(stream);
        }
        else if (rc == GT_NO_RESOURCE)
        {
            stream->destroy(stream);
        }
    }
#else
    GT_UNUSED_PARAM(stream);
#endif
    return rc;
}


/*******************************************************************************
* cmdStreamSocketHandler
*
* DESCRIPTION:
*       Socket service Handler
*       Create new telnet connection
*
* INPUTS:
*       GT_VOID
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS cmdStreamSocketHandler
(
    IN char *socketType
)
{
    GT_SOCKET_FD socket = -1;
    char serviceName[20] = {0};

    if ( srvIndex >= SERVICES_MAX )
        return GT_FAIL;

    /* socket descriptor */
    socket = cmdOsSocketAccept(services[srvIndex].masterSocket, NULL, NULL);
    if (socket < 0)
        return GT_FAIL;

    for ( srvIndex = 0 ; srvIndex < SERVICES_MAX ; srvIndex++)
    {
        if (( services[srvIndex].serviceName == NULL ) || (services[srvIndex].onHold == GT_TRUE))
            break;
    }

    if (srvIndex >= SERVICES_MAX)
    {
        /* DB is full */
        return GT_FAIL;
    }

    if ( services[srvIndex].onHold == GT_TRUE) {
        /*In case of reconnection (without using CLIexit command) don't need to create the
          service, add existing one*/
            cmdOsSelectAddFdToSet(rdSet, socket);
            services[srvIndex].luaCLI.len=0;
    }
    else{

        if ((cmdOsStrCmp(socketType, "luaCLI_uds") == 0))
            cmdOsSprintf(serviceName, "uds_%d", socket);
        else
            cmdOsSprintf(serviceName, "telnet_%d", socket);

        if ( cmdStreamSocketServiceCreate(
                serviceName,
                NULL, 0/*port*/,
                cmdSocketInstanceHandler,
                GT_TRUE/*multipleInstances*/,
                GT_TRUE/*isConsole*/) != GT_OK )
            goto destroySocketLabel;

        cmdOsSelectAddFdToSet(rdSet, socket);
        if ( socket > max_fd)
            max_fd = socket;

    }

    services[srvIndex].masterSocket = socket;
    services[srvIndex].running = GT_TRUE;
    services[srvIndex].onHold = GT_FALSE;

    if ((services[srvIndex].stream = IOStreamCreateSocket(socket)) == NULL)
            goto destroySocketLabel;

    if (services[srvIndex].multipleInstances == GT_FALSE)
            services[srvIndex].stream->isConsole = services[srvIndex].isConsole;

    if ( luaCLI_tcpInstanceCreate() != GT_OK )
            goto destroySocketLabel;

    return GT_OK;

destroySocketLabel:
    cmdOsSocketTcpDestroy(socket);
    return GT_FAIL;

}


/*******************************************************************************
* cmdStreamSocketServiceListenerTask
*
* DESCRIPTION:
*       Socket service listener task
*
* INPUTS:
*       GT_VOID
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_VOID __TASKCONV cmdStreamSocketServiceListenerTask
(
    GT_VOID
)
{
    GT_SOCKET_FD fd;
    GT_STATUS rc;

    rdSet = cmdOsSelectCreateSet();

    if(rdSet!=NULL)
    {
        while (GT_TRUE)
        {
            /* Use select to check if service need to be handled */
            cmdOsSelectZeroSet(rdSet);

            /* add child sockets to set */
            for ( srvIndex = 0 ; srvIndex < SERVICES_MAX ; srvIndex++)
            {
                if ( services[srvIndex].running == GT_FALSE )
                    continue;

                /* socket descriptor */
                fd = services[srvIndex].masterSocket;
                cmdOsSelectAddFdToSet(rdSet, fd);
                if ( fd > max_fd)
                    max_fd = fd;
            }

            /* wait for an activity on one of the sockets. timeout is NULL - so wait indefinitely */
            if (cmdOsSelect(max_fd + 1, rdSet, NULL, NULL, GT_INFINITE) <= 0)
                continue;

            for ( srvIndex = 0 ; srvIndex < SERVICES_MAX ; srvIndex++)
            {
                if ( services[srvIndex].serviceName == NULL )
                    continue;
                /* socket descriptor */
                fd = services[srvIndex].masterSocket;

                /* If something happened on the master socket, then it's an incoming connection */
                if ( cmdOsSelectIsFdSet(rdSet, fd) == GT_TRUE ){

                    /* Telnet connection from master socket */
                    if ((cmdOsStrCmp(services[srvIndex].serviceName, MASTER_SOCKET_SERVICE_NAME) == 0) ||
                        (cmdOsStrCmp(services[srvIndex].serviceName, "luaCLI_uds") == 0))
                        cmdStreamSocketHandler(services[srvIndex].serviceName);
                    else if (services[srvIndex].stream != NULL) {
                        rc = services[srvIndex].handler(services[srvIndex].stream);
                        /* rc != GT_OK means the connection is probably closed */
                        if( rc != GT_OK ){
                            /* In case the connection is closed but session needs to be saved, keep session alive on hold */
                            services[srvIndex].onHold = GT_TRUE;
                            services[srvIndex].running = GT_FALSE;
                            cmdOsSocketTcpDestroy(fd);
                            cmdOsSelectClearFdFromSet(rdSet, fd);
                            if (rc != GT_NO_RESOURCE){
                                services[srvIndex].stream->destroy(services[srvIndex].stream);
                                if (services[srvIndex].ip)
                                   cmdOsFree(services[srvIndex].ip);
                                if (services[srvIndex].serviceName)
                                   cmdOsFree(services[srvIndex].serviceName);
                                cmdOsMemSet(&services[srvIndex], 0, sizeof(SERVICE_PARAMS_STC));
                                numServices--;
                            }
                        }
                    }
                }
            }
            if (numServices == 0)
                break;

        }
    cmdOsSelectEraseSet(rdSet);
   }
}


/**
* @internal cmdStreamSocketServiceStart function
* @endinternal
*
* @brief   Start socket service
*
* @param[in] serviceName
*/
GT_STATUS cmdStreamSocketServiceStart(
    IN  const char *serviceName
)
{
#define SERVICE_FIND() \
    int i; \
    SERVICE_PARAMS_STC *s; \
    for (i = 0; i < SERVICES_MAX; i++) \
    { \
        if (cmdOsStrCmp(serviceName, services[i].serviceName) == 0) \
        { \
            break; \
        } \
    } \
    if (i == SERVICES_MAX) \
    { \
        return GT_NO_SUCH; \
    } \
    s = &(services[i]);

    SERVICE_FIND();
    if (s->running == GT_TRUE)
    {
        return GT_OK;
    }

    if (cmdOsStrCmp(serviceName, "luaCLI_uds") == 0)
    {
        if ( osSocketUdsCreateListen(&(s->masterSocket)) != GT_OK )
        {
            return GT_FAIL;
        }
    }
    else if ( osSocketTcpCreateListen(s->ip, s->port, &(s->masterSocket)) != GT_OK )
    {
        return GT_FAIL;
    }

    s->running = GT_TRUE;
    return GT_OK;
}

/**
* @internal cmdStreamSocketServiceStop function
* @endinternal
*
* @brief   Stop socket service
*
* @param[in] serviceName
*/
GT_STATUS cmdStreamSocketServiceStop(
    IN  const char *serviceName
)
{
    SERVICE_FIND();
    if (s->running == GT_FALSE)
    {
        return GT_OK;
    }

    s->running = GT_FALSE;
    cmdOsSocketTcpDestroy(s->masterSocket); /* listen() may be interrupted */
    s->masterSocket = -1;
    return GT_OK;
}

/**
* @internal cmdStreamSocketServiceIsRunning function
* @endinternal
*
* @brief   Return service running status
*
* @param[in] serviceName
*/
GT_BOOL cmdStreamSocketServiceIsRunning(
    IN  const char *serviceName
)
{
    int i;
    for (i = 0; i < SERVICES_MAX; i++)
    {
        if (services[i].serviceName == NULL)
            continue;
        if (cmdOsStrCmp(serviceName, services[i].serviceName) == 0)
            return services[i].running;
    }
    return GT_FALSE;
}

/**
* @internal cmdStreamSocketServiceGetPort function
* @endinternal
*
* @brief   Get service port number
*
* @param[in] serviceName
*
* @param[out] portNumberPtr
*/
GT_STATUS cmdStreamSocketServiceGetPort(
    IN  const char *serviceName,
    OUT GT_U16     *portNumberPtr
)
{
    SERVICE_FIND();

    if (portNumberPtr == NULL)
        return GT_BAD_PARAM;
    *portNumberPtr = (GT_U16)(s->port);
    return GT_OK;
}

/**
* @internal cmdStreamSocketServiceSetPort function
* @endinternal
*
* @brief   Get service port number
*
* @param[in] serviceName
* @param[in] portNumber
*/
GT_STATUS cmdStreamSocketServiceSetPort(
    IN  const char *serviceName,
    IN  GT_U16     portNumber
)
{
    SERVICE_FIND();
    s->port = portNumber;
    return GT_OK;
}

