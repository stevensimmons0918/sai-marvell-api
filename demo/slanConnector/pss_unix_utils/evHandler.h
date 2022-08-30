// evHandler.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef __evHandler_h__
#define __evHandler_h__

#include <inttypes.h>

/*
 * Typedef: struct evHandlerEventSTC
 *
 * Description:
 *      Event structure
 *
 * Fields:
 *      userData     - custom pointer passed back with evHandlerWait()
 *      events       - bitmap of events (RX/TX) to handle
 */
typedef struct evHandlerEventSTC {
    void        *userData;
    uint32_t    events;
} evHandlerEventSTC;

/*******************************************************************************
* evHandlerInit
*
* DESCRIPTION:
*       Initialize event handler API
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
int evHandlerInit(int maxConn);

/*******************************************************************************
* evHandlerCmd
*
* DESCRIPTION:
*       Add/modify/delete events per file descriptor
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
int evHandlerCmd(int cmd, int fd, uint32_t events, void *userData);

/*******************************************************************************
* evHandlerWait
*
* DESCRIPTION:
*       Add/modify/delete events per file descriptor
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
int evHandlerWait(evHandlerEventSTC *events, int maxEvents);


/* implementation specific */
#ifdef __linux__
   /* Linux: use epoll*() */
#  define EV_HANDLER_IMPLEMENTATION_EPOLL
#else
#  define EV_HANDLER_IMPLEMENTATION_SELECT
#endif



#ifdef EV_HANDLER_IMPLEMENTATION_SELECT
/* select() implementation */

#define EV_HANDLER_EV_CLOSE     0x01
#define EV_HANDLER_EV_READ      0x02
#define EV_HANDLER_EV_WRITE     0x04

#define EV_HANDLER_CTL_ADD  0
#define EV_HANDLER_CTL_MOD  1
#define EV_HANDLER_CTL_DEL  2
#endif /* EV_HANDLER_IMPLEMENTATION_SELECT */



#ifdef EV_HANDLER_IMPLEMENTATION_EPOLL
#include <sys/epoll.h>

#define EV_HANDLER_EV_CLOSE     EPOLLHUP
#define EV_HANDLER_EV_READ      EPOLLIN
#define EV_HANDLER_EV_WRITE     EPOLLOUT

#define EV_HANDLER_CTL_ADD  EPOLL_CTL_ADD
#define EV_HANDLER_CTL_MOD  EPOLL_CTL_MOD
#define EV_HANDLER_CTL_DEL  EPOLL_CTL_DEL

#endif /* EV_HANDLER_IMPLEMENTATION_EPOLL */

#endif
