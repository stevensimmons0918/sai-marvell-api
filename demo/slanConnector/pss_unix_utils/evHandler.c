// evHandler.c

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
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <inttypes.h>

#include "evHandler.h"

#ifdef EV_HANDLER_IMPLEMENTATION_SELECT
typedef struct fdEventsSTC {
    int         active;
    int         fd;
    uint32_t    events;
    void        *userData;
} fdEventsSTC;

static fd_set rfds, wfds; 
static fdEventsSTC *events = NULL;
static int connMax = 0;
static int activeConnections = 0;
static int maxFd = -1;

/*******************************************************************************
* evHandlerInit
*
* DESCRIPTION:
*       Initialize event handler API
*       select() implementation
*
* INPUTS:
*       macConn     - maximum file descriptors to handle
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Zero if success
*       < 0 if error
*
* COMMENTS:
*
*******************************************************************************/
int evHandlerInit(int _connMax)
{
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    connMax = _connMax;
    events = (fdEventsSTC*)calloc(connMax, sizeof(*events));
    return 0;
}

/*******************************************************************************
* evHandlerCmd
*
* DESCRIPTION:
*       Add/modify/delete events per file descriptor
*       select() implementation
*
* INPUTS:
*       cmd      - EV_HANDLER_CTL_{ADD,MOD,DEL} to add/modify/delete fd events
*       fd       - file descriptor
*       events   - bitmap of events (EV_HANDLER_EV_{CLOSE,READ,WRITE}) to handle
*       userData - custom pointer passed back with evHandlerWait()
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Zero if success
*       < 0 if error
*
* COMMENTS:
*
*******************************************************************************/
int evHandlerCmd(int cmd, int fd, uint32_t events, void *userData)
{
    int k;
    if (cmd == EV_HANDLER_CTL_ADD)
    {
        for (k = 0; k < connMax; k++)
        {
            if (!events[k].active)
                break;
        }
        if (k >= connMax)
            return -1;
        events[k].active = 1;
        events[k].fd = fd;
        if (fd > maxFd)
            maxFd = fd;
        activeConnections++;
    }
    else
    {
        for (k = 0; k < connMax; k++)
        {
            if (!events[k].active)
                continue;
            if (events[k].fd == fd)
                break;
        }
        if (k >= connMax)
            return -1;
    }
    if (cmd == EV_HANDLER_CTL_DEL)
    {
        FD_CLR(fd, &rfds);
        FD_CLR(fd, &wfds);
        events[k].active = 0;
        if (fd == maxFd)
        {
            maxFd = -1;
            for (k = 0; k < connMax; k++)
            {
                if (!events[k].active)
                    continue;
                if (events[k].fd >maxFd)
                    maxFd = events[k].fd;
            }
        }
        activeConnections--;
        return 0;
    }
    /* ADD | MOD */
    events[k].events = events;
    events[k].userData = userData;
    if (events[k].events & EV_HANDLER_EV_READ)
        FD_SET(fd, &rfds);
    else
        FD_CLR(fd, &rfds);
    if (events[k].events & EV_HANDLER_EV_WRITE)
        FD_SET(fd, &wfds);
    else
        FD_CLR(fd, &wfds);

    return 0;
}
/*******************************************************************************
* evHandlerWait
*
* DESCRIPTION:
*       Add/modify/delete events per file descriptor
*       select() implementation
*
* INPUTS:
*       maxEvents - max number of events to wait
*
* OUTPUTS:
*       events    - array of events
*
* RETURNS:
*       actual number of events, <= maxEvents
*       < 0 if error
*
* COMMENTS:
*
*******************************************************************************/
int evHandlerWait(evHandlerEventSTC *revents, int maxEvents)
{
    int k, a;
    int ret = 0;
    fd_set rfds_cur, wfds_cur;
    rfds_cur = rfds;
    wfds_cur = wfds;
    k = select(maxFd+1, &rfds_cur, &wfds_cur, (fd_set*)NULL, NULL);
    if (k < 0)
    {
        if (errno != EINTR)
            perror("select()");
        return 0;
    }
    if (k == 0)
        return 0;

    for (k = 0,a = 0; k < connMax && a < activeConnections; k++)
    {
        if (!events[k].active)
            continue;
        a++;
        revents[ret].events = 0;
        revents[ret].userData = events[k].userData;

        if (FD_ISSET(events[k].fd, &rfds_cur))
            revents[ret].events |= EV_HANDLER_EV_READ;
        if (FD_ISSET(events[k].fd, &wfds_cur))
            revents[ret].events |= EV_HANDLER_EV_WRITE;
        /* TODO check for close */
        if (revents[ret].events)
        {
            ret++;
            if (ret >= maxEvents)
                return ret;
        }
    }
    return ret;
}
#endif /* EV_HANDLER_IMPLEMENTATION_SELECT */



#ifdef EV_HANDLER_IMPLEMENTATION_EPOLL
static int epollfd;
#include <alloca.h>

/*******************************************************************************
* evHandlerInit
*
* DESCRIPTION:
*       Initialize event handler API
*       epoll() implementation
*
* INPUTS:
*       macConn     - maximum file descriptors to handle
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Zero if success
*       < 0 if error
*
* COMMENTS:
*
*******************************************************************************/
int evHandlerInit(int maxConn)
{
    epollfd = epoll_create(maxConn);
    return 0;
}

/*******************************************************************************
* evHandlerCmd
*
* DESCRIPTION:
*       Add/modify/delete events per file descriptor
*       epoll() implementation
*
* INPUTS:
*       cmd      - EV_HANDLER_CTL_{ADD,MOD,DEL} to add/modify/delete fd events
*       fd       - file descriptor
*       events   - bitmap of events (EV_HANDLER_EV_{CLOSE,READ,WRITE}) to handle
*       userData - custom pointer passed back with evHandlerWait()
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Zero if success
*       < 0 if error
*
* COMMENTS:
*
*******************************************************************************/
int evHandlerCmd(int cmd, int fd, uint32_t events, void *userData)
{
    struct epoll_event evc;
    evc.events = events;
    evc.data.ptr = userData;
    return epoll_ctl(epollfd, cmd, fd, &evc);
}

/*******************************************************************************
* evHandlerWait
*
* DESCRIPTION:
*       Add/modify/delete events per file descriptor
*       epoll() implementation
*
* INPUTS:
*       maxEvents - max number of events to wait
*
* OUTPUTS:
*       events    - array of events
*
* RETURNS:
*       actual number of events, <= maxEvents
*       < 0 if error
*
* COMMENTS:
*
*******************************************************************************/
int evHandlerWait(evHandlerEventSTC *events, int maxEvents)
{
    struct epoll_event *eva;
    int nev, k;

    eva = (struct epoll_event*)alloca(sizeof(struct epoll_event)*maxEvents);

    nev = epoll_wait(epollfd, eva, maxEvents, -1);
    for (k = 0; k < nev; k++)
    {
        events[k].events = eva[k].events;
        events[k].userData = eva[k].data.ptr;
    }
    return nev;
}
#endif /* EV_HANDLER_IMPLEMENTATION_EPOLL */
