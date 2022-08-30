// slanConnector.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>


#include "slanLib.h"
#include "evHandler.h"

/************ Defines  ********************************************************/
#define SLAN_CONNECTIONS_MAX_DEF 128

/************* Typedefs *******************************************************/

typedef enum {
    CONN_STATE_NULL = 0,
    CONN_STATE_CONNECTED,
    CONN_STATE_IDENTIFIED
} CONN_STATE_ENUM;
/*
 * Typedef: struct slanConnectionSTC
 *
 * Description:
 *      This structure represent a single SLAN connection
 *
 * Fields:
 *      state        - current connection state
 *      fd           - connection file descriptor
 *      name         - SLAN name for this connection
 *      buffer       - input buffer
 *      bufferLength - length of data currenly stored in buffer
 *      outputBuffer - output buffer
 *      outputLength - length of data currenly stored in outputBuffer
 *      events       - bitmap of events (RX/TX) to handle
 *      pair         - pointer to second side
 */
typedef struct slanConnectionSTC {
    CONN_STATE_ENUM                 state;
    int                             fd;
    char                            name[SLAN_NAME_LEN];
    char                            clientName[SLAN_NAME_LEN];
    char                            buffer[SLAN_INPUT_BUFFER_SIZE];
    int                             bufferLength;
    char                            outputBuffer[SLAN_INPUT_BUFFER_SIZE];
    int                             outputLength;
    uint32_t                        events;
    struct slanConnectionSTC        *pair;
} slanConnectionSTC;


/************* Global data ****************************************************/
int connMax = SLAN_CONNECTIONS_MAX_DEF;
slanConnectionSTC* conns = NULL;
int activeConnections = 0;
int listenerFd;

/*******************************************************************************
* startListen
*
* DESCRIPTION:
*       Start listening for incoming connections:
*       create master socket, bind, then listen
*
* INPUTS:
*       nodename    - address to bind to (NULL == any addr)
*       service     - service to bind to
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Non zero if success
*       Zero if error
*
* COMMENTS:
*
*******************************************************************************/
int startListen(const char *nodename, const char *service)
{
	struct addrinfo	hints;
	struct addrinfo	*ai;
	int	er;
	int	on = 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;/* AF_UNSPEC */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	er = getaddrinfo(nodename, service, &hints, &ai);
	if (er) {
		fprintf(stderr, "can't getaddrinfo(): %s\n", gai_strerror(er));
        return 0;
	}

	listenerFd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (listenerFd < 0) {
		perror("socket()");
		freeaddrinfo(ai);
		return 0;
	}

    setsockopt(listenerFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	if (bind(listenerFd, ai->ai_addr,ai->ai_addrlen) < 0) {
		perror("bind()");
		freeaddrinfo(ai);
		close(listenerFd);
        return 0;
	}
	freeaddrinfo(ai);
	if (listen(listenerFd, 32) < 0) {
		perror("listen()");
		close(listenerFd);
        return 0;
	}
    return 1;
}

#define EV_CMD(_cmd,_conn) \
    evHandlerCmd(_cmd, (_conn)->fd, (_conn)->events, (_conn))

/*******************************************************************************
* acceptConnection
*
* DESCRIPTION:
*       Accept incoming connection.
*       This function called when select() flags listenerFd ready for read
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Non zero if success
*       Zero if error
*
* COMMENTS:
*
*******************************************************************************/
int acceptConnection(void)
{
    int fd;
    int k;

	fd = accept(listenerFd, NULL, NULL);
	if(fd < 0) {
		perror("can't accept()");
		return 0;
	}
    for (k = 0; k < connMax; k++)
        if (conns[k].state == CONN_STATE_NULL)
            break;
    if (k >= connMax)
    {
        fprintf(stderr, "Too many connections already");
        close(fd);
        return 0;
    }
    memset(conns+k,0,sizeof(*conns));
    conns[k].fd = fd;
    conns[k].state = CONN_STATE_CONNECTED;
    conns[k].events = EV_HANDLER_EV_READ;
    EV_CMD(EV_HANDLER_CTL_ADD, &(conns[k]));
    activeConnections++;
    return 1;
}

void connClose(slanConnectionSTC *conn);
/*******************************************************************************
* connWrite
*
* DESCRIPTION:
*       write to connection (non-blocking)
*       suspend sender reads when not complete
*
* INPUTS:
*       sender      - sender connection, can be NULL
*       conn        - Connection to close
*       buff        - buffer to write
*       len         - data to write
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Non zero if success
*       Zero if error
*
* COMMENTS:
*
*******************************************************************************/
int connWrite(slanConnectionSTC *sender, slanConnectionSTC *conn, char *buff, int len)
{
    int k;
    k = send(conn->fd, buff, len, MSG_DONTWAIT | MSG_NOSIGNAL);
    if (k < 0 && errno != EINTR)
    {
        connClose(conn);
        return -1;
    }
    if (k == len)
        return 0;
    memcpy(conn->outputBuffer, buff+k, len - k);
    conn->outputLength = len - k;
    conn->events |= EV_HANDLER_EV_WRITE;
    EV_CMD(EV_HANDLER_CTL_MOD, conn);
    if (sender) /* suspend sender rx */
    {
        sender->events &= ~EV_HANDLER_EV_READ;
        EV_CMD(EV_HANDLER_CTL_MOD, sender);
    }
    return 1;
}


/*******************************************************************************
* connClose
*
* DESCRIPTION:
*       Close connection
*       LinkDown message sent to pair
*
* INPUTS:
*       conn        - Connection to close
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Non zero if success
*       Zero if error
*
* COMMENTS:
*
*******************************************************************************/
void connClose(slanConnectionSTC *conn)
{
    slanConnectionSTC *pair = conn->pair;
    if (pair)
    {
        /* send LINKDOWN event */
        char hdr[SLAN_PKT_HDR_SIZE];

        SLAN_LIB_PACK_HEADER(hdr, SLAN_PKT_HDR_SIZE, SLAN_LIB_EV_LINKDOWN);
        pair->pair = NULL;
        connWrite(NULL, pair, hdr, SLAN_PKT_HDR_SIZE);
    }
    evHandlerCmd(EV_HANDLER_CTL_DEL, conn->fd, 0, NULL);
    close(conn->fd);
    conn->state = CONN_STATE_NULL;
    activeConnections--;
}

/*******************************************************************************
* performReadOperation
*
* DESCRIPTION:
*       This function called when select() flags connection ready for read
*
* INPUTS:
*       conn        - connection
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Non zero if success
*       Zero if error
*
* COMMENTS:
*
*******************************************************************************/
int performReadOperation(slanConnectionSTC *conn)
{
    int r;

    r = recv(conn->fd,
            conn->buffer+conn->bufferLength,
            sizeof(conn->buffer)-conn->bufferLength,
            MSG_DONTWAIT);
    if (r < 0)
    {
        /* handle error */
        perror("recv()");
        /* handle errors */
        return 0;
    }
    if (r == 0)
    {
        connClose(conn);
        return 0;
    }
    conn->bufferLength += r;
    r = 0;
#define BYTES_LEFT  (conn->bufferLength - r)
#define CURR_PTR    (conn->buffer + r)
    while (BYTES_LEFT >= SLAN_PKT_HDR_SIZE)
    {
        int dataSize;
        SLAN_LIB_EVENT_TYPE evType;
        char hdr[SLAN_PKT_HDR_SIZE];
        char *dataPtr;
        int k;

        
        SLAN_LIB_UNPACK_HEADER(CURR_PTR, dataSize, evType);
        if (dataSize < SLAN_PKT_HDR_SIZE || dataSize > SLAN_INPUT_BUFFER_SIZE)
        {
            /* protocol error */
            connClose(conn);
            return 0;
        }
        if (dataSize > BYTES_LEFT)
            break;

        /* got packet */
        dataPtr = CURR_PTR;
        r += dataSize;

        if (conn->state == CONN_STATE_IDENTIFIED)
        {
            if (conn->pair)
            {
                k = connWrite(conn, conn->pair, dataPtr, dataSize);
                if (k == 0)
                {
                    /* success */
                    continue;
                }
                if (k == 1)
                {
                    /* incomplete write */
                    break;
                }
            }
            else
            {
                /* discard */
            }
            continue;
        }

        /* conn->state == CONN_STATE_CONNECTED */
        if (evType != SLAN_LIB_EV_HANDSHAKE)
        {
            /* discard */
            continue;
        }

        /* SLAN_LIB_EV_HANDSHAKE packet received */
        strncpy(conn->name, dataPtr + SLAN_PKT_HDR_SIZE, SLAN_NAME_LEN-1);
        conn->name[SLAN_NAME_LEN-1] = 0;
        /* copy slanClientName */
        conn->clientName[0] = 0;
        k = strlen(dataPtr + SLAN_PKT_HDR_SIZE);
        if (SLAN_PKT_HDR_SIZE + k + 1 < dataSize)
        {
            strncpy(conn->clientName, dataPtr + SLAN_PKT_HDR_SIZE + k + 1, SLAN_NAME_LEN-1);
            conn->clientName[SLAN_NAME_LEN-1] = 0;
        }

        /* identified, search pair */
        for (k = 0; k < connMax; k++)
        {
            if (conns[k].state != CONN_STATE_IDENTIFIED)
                continue;
            if (strcasecmp(conns[k].name, conn->name) != 0)
                continue;
            if (conns[k].pair)
            {
                /* there are already connection present for this name */
                dataSize = sprintf(conn->buffer + SLAN_PKT_HDR_SIZE,
                        "The SLAN '%s' already conencted", conn->name);
                dataSize += 1 + SLAN_PKT_HDR_SIZE;
                SLAN_LIB_PACK_HEADER(conn->buffer, dataSize, SLAN_LIB_EV_HANDSHAKE_FAILED);
                connWrite(NULL, conn, conn->buffer, dataSize);
                connClose(conn);
                return 0;
            }
            break;
        }
        if (k < connMax)
        {
            /* connection established */
            conns[k].pair = conn;
            conn->pair = conns+k;
            SLAN_LIB_PACK_HEADER(hdr, SLAN_PKT_HDR_SIZE, SLAN_LIB_EV_LINKUP);
            k = connWrite(conn, conn->pair, hdr, SLAN_PKT_HDR_SIZE);
            if (k != 0)
                break;
        }
        /* identified */
        /* ack handshake */
        SLAN_LIB_PACK_HEADER(hdr, SLAN_PKT_HDR_SIZE, SLAN_LIB_EV_HANDSHAKE_SUCCESS);
        connWrite(NULL,conn, hdr, SLAN_PKT_HDR_SIZE);
        if (conn->pair)
        {
            SLAN_LIB_PACK_HEADER(hdr, SLAN_PKT_HDR_SIZE, SLAN_LIB_EV_LINKUP);
            connWrite(NULL,conn, hdr, SLAN_PKT_HDR_SIZE);
        }
        conn->state = CONN_STATE_IDENTIFIED;
    }
    if (BYTES_LEFT)
        memcpy(conn->buffer, CURR_PTR, BYTES_LEFT);
    conn->bufferLength = BYTES_LEFT;
#undef BYTES_LEFT
#undef CURR_PTR
    return 1;
}

/*******************************************************************************
* performWriteOperation
*
* DESCRIPTION:
*       This function called when socket is ready for write
*       When output buffer flushed unlocks pair reads
*
* INPUTS:
*       conn        - connection
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Non zero if success
*       Zero if error
*
* COMMENTS:
*
*******************************************************************************/
int performWriteOperation(slanConnectionSTC *conn)
{
    int k;
    k = send(conn->fd, conn->outputBuffer, conn->outputLength, MSG_DONTWAIT | MSG_NOSIGNAL);
    if (k < 0 && errno != EINTR)
    {
        connClose(conn);
        return 0;
    }
    if (k == 0)
        return 1;
    if (k < conn->outputLength)
    {
        memcpy(conn->outputBuffer, conn->outputBuffer+k, conn->outputLength-k);
        conn->outputLength -= k;
        return 1;
    }
    conn->outputLength = 0;
    conn->events &= ~EV_HANDLER_EV_WRITE;
    EV_CMD(EV_HANDLER_CTL_MOD, conn);
    if (conn->pair) /* restore pair rx */
    {
        conn->pair->events |= EV_HANDLER_EV_READ;
        EV_CMD(EV_HANDLER_CTL_MOD, conn->pair);
        performReadOperation(conn->pair);
    }
    return 1;
}

/*******************************************************************************
* usage
*
* DESCRIPTION:
*       Print command line usage
*
* INPUTS:
*       argv0
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
void usage(char* argv0)
{
    printf("Usage: %s [-D] [-c conns] [-h]\n", argv0);
    printf("\t-h        This message\n");
    printf("\t-D        Daemonize\n");
    printf("\t-c conns  Set maximum number of connections. Default is %d\n",
            SLAN_CONNECTIONS_MAX_DEF);
}
/*******************************************************************************
* main
*
* DESCRIPTION:
*       main()
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
int main(int argc, char **argv)
{
    int daemonize = 0;
    int c;

    while ((c = getopt(argc, argv, "Dc:h?")) != EOF)
    {
        switch (c)
        {
            case 'D':
                daemonize = 1;
                break;
            case 'c':
                connMax = atoi(optarg);
                if (connMax < 2)
                    connMax = SLAN_CONNECTIONS_MAX_DEF;
                break;
            case 'h':
            case '?':
                usage(argv[0]);
                return 0;
            default:
                fprintf(stderr, "Command line error\n");
                usage(argv[0]);
                return 1;
        }
    }

    conns = (slanConnectionSTC*)calloc(connMax, sizeof(*conns));

    if (!startListen(SLAN_CONNECTOR_ADDRESS, SLAN_CONNECTOR_PORT))
        return 1;

    if (daemonize)
    {
        /* check if any #include required for FreeBSD here */
        if (daemon(0,0) < 0)
        {
            perror("Can't daemonize");
            /* continue in foreground mode */
        }
    }
    evHandlerInit(connMax+1);
    evHandlerCmd(EV_HANDLER_CTL_ADD, listenerFd, EV_HANDLER_EV_READ, NULL);

    /* mailLoop */
    while (1)
    {
        int i, numEvents;
        evHandlerEventSTC ev[20];

        numEvents = evHandlerWait(ev, 20);
        for (i = 0; i < numEvents; i++)
        {
            if (ev[i].userData == NULL) /* listener */
            {
                acceptConnection();
                continue;
            }
            if (ev[i].events & EV_HANDLER_EV_CLOSE)
            {
                connClose(ev[i].userData);
                continue;
            }
            if (ev[i].events & EV_HANDLER_EV_WRITE)
                performWriteOperation(ev[i].userData);

            if (ev[i].events & EV_HANDLER_EV_READ)
                performReadOperation(ev[i].userData);
        }
    }
    return 0;
}
