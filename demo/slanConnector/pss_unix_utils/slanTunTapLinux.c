// slanTunTapLinux.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/*******************************************************************************
* slanTunTapLinux.c
*
* DESCRIPTION:
*       SLAN driver to TUN/TAP
*       Linux version
*
* BUILD:
*   cc -Wall -pthread -g   -c -o evHandler.o evHandler.c
*   cc -Wall -pthread -g   -c -o slanLib.o slanLib.c
*   cc -Wall -pthread -g   -c -o slanTunTapLinux.o slanTunTapLinux.c
*   cc -pthread -g  slanTunTapLinux.o evHandler.o slanLib.o -o slanTunTapLinux
*
* USING:
*   ./slanConnector -D
*   sudo ./slanTunTapLinux -D -n slan00
*   sudo ​ip link set tap0 up
*   ​sudo ​ip addr add 192.168.0.​60​/24 dev tap0
*   sudo tcpdump -X -v -i tap0
*
*   and then run appDemoSim
*
* FILE REVISION NUMBER:
*       $Revision 1 $
*
*******************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_tun.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "slanLib.h"

/************ Defines  ********************************************************/

/************* Global data ****************************************************/
int     tnFd = -1;

char    buff[SLAN_INPUT_BUFFER_SIZE];

SLAN_ID slan;

int slanIsUp = 0;

/************* Functions ******************************************************/
static int tunTapOpen(void)
{
    struct ifreq ifr;
    if (tnFd >= 0)
        return 0;
    
    /* open the clone device */
    if ( (tnFd = open("/dev/net/tun", O_RDWR)) < 0 )
    {
        perror("tunTapOpen(): Failed to open /dev/net/tun");
        return 0;
    }

    /* preparation of the struct ifr, of type "struct ifreq" */
    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

    /* the kernel will try to allocate the "next" device of the TAP type */

    /* try to create the device */
    if (ioctl(tnFd, TUNSETIFF, (void *) &ifr) < 0)
    {
        perror("tunTapOpen(): Failed to clone TAP device (TUNSETIFF)");
        close(tnFd);
        tnFd = -1;
        return 0;
    }

    /* if the operation was successful, write back the name of the interface  */
    printf("TAP device opened: %s\n", ifr.ifr_name);

    return 1;
}

/*******************************************************************************
* tunTapDoRx
*
* DESCRIPTION:
*       perform Rx from TAP
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Zero if error
*       Non zero if success
*
* COMMENTS:
*
*******************************************************************************/
int tunTapDoRx(void)
{
    int rcv;

    rcv = read(tnFd, buff, sizeof(buff));
    if (rcv < 0)
    {
        perror("read(tnFd) failed");
        return 0;
    }
    if (!rcv)
        return 1;
    if (slanIsUp)
    {
        slanLibTransmit(slan, buff, rcv);
    }
    return 1;
}

/*******************************************************************************
* tunTapTx
*
* DESCRIPTION:
*       Transmit packet to TUN
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Zero if error
*       Non zero if success
*
* COMMENTS:
*
*******************************************************************************/
int tunTapTx(const char *pktData, int pktLen)
{
    return write(tnFd, pktData, pktLen);
}

/*******************************************************************************
* rx_thread
*
* DESCRIPTION:
*       Rx thread for libSlan
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
void* rx_thread(void* unused)
{
    slanLibMainLoop(0);
    return NULL;
}

/*******************************************************************************
* ev_handler_func
*
* DESCRIPTION:
*       SLAN Event process function
*
* INPUTS:
*       slanId      - SLAN id
*       userData    - The data pointer passed to slanLibBind()
*       eventType   - event type
*       pktData     - pointer to packet
*       pktLen      - packet length
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
void ev_handler_func
(
    IN  SLAN_ID             slanId,
    IN  void*               userData,
    IN  SLAN_LIB_EVENT_TYPE eventType,
    IN  char*               pktData,
    IN  int                 pktLen
)
{

    switch (eventType)
    {
        case SLAN_LIB_EV_CLOSED:
            fprintf(stderr,"Connector has closed connection\n");
            exit(0);
        case SLAN_LIB_EV_LINKUP:
            slanIsUp = 1;
            return;
        case SLAN_LIB_EV_LINKDOWN:
            slanIsUp = 0;
            /* down interface */
            return;
        case SLAN_LIB_EV_PACKET:
            tunTapTx(pktData, pktLen);
            return;
        default:
            fprintf(stderr, "Unexpected slan event: %d\n", eventType);
            break;
    }
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
    printf("Usage: %s [-D] [-h] [-n slan_name]\n", argv0);
    printf("\t-h            This message\n");
    printf("\t-D            Daemonize\n");
    printf("\t-n slan_name  Set slan name. Default is interface name\n");
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
    pthread_t rxthreadId;
    int daemonize = 0;
    char *slanName = NULL;
    int c;

    while ((c = getopt(argc, argv, "Dn:h?")) != EOF)
    {
        switch (c)
        {
            case 'D':
                daemonize = 1;
                break;
            case 'n':
                slanName = optarg;
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
    if (slanName == NULL)
    {
        usage(argv[0]);
        return 0;
    }

    if (!tunTapOpen())
        return 1;

    slanLibInit(1,"tuntap");

    if (daemonize)
    {
        /* check if any #include required for FreeBSD here */
        if (daemon(0,0) < 0)
        {
            perror("Can't daemonize");
            /* continue in foreground mode */
        }
    }

    pthread_create(&rxthreadId, NULL, rx_thread, NULL);

    if (slanLibBind(slanName, ev_handler_func, (void*)0xa, &slan) != 0)
        return 1;

    while (1)
    {
        fd_set  rfds;

        FD_ZERO(&rfds);
        FD_SET(tnFd, &rfds);

        if (select(tnFd+1, &rfds, NULL, NULL, NULL) < 0)
            continue;

        if (FD_ISSET(tnFd, &rfds))
        {
            if (!tunTapDoRx())
                break;
        }
    }
    return 0;
}
