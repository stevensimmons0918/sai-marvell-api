// slanEthPortFreeBsd.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/bpf.h>
#include <pthread.h>

#include "slanLib.h"

/************ Defines  ********************************************************/

/************* Global data ****************************************************/
int nicFd;
char*    buff = NULL;
u_int    buffer_size;

SLAN_ID slan;
int slanIsUp = 0;

/************* Functions ******************************************************/

/*******************************************************************************
* bsd_bpf_open
*
* DESCRIPTION:
*       open first free BPF device.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       < 0 if error
*       BPF File descriptor
*
*******************************************************************************/
int bsd_bpf_open(void)
{
    int fd = -1;
    char device[sizeof "/dev/bpf000"];
    static int start_bpf = 1; /* /dev/bpf0 is reserved by the IT group */
    /*
     *  Go through all the minors and find one that isn't in use.
     */
    for (; start_bpf < 1000; start_bpf++)
    {
        sprintf(device, "/dev/bpf%d", start_bpf);

        fd = open(device, O_RDWR);
        if (fd == -1 && errno == EBUSY)
        {
            /*
             *  Device is busy.
             */
            continue;
        }
        else
        {
            /*
             *  Either we've got a valid file descriptor, or errno is not
             *  EBUSY meaning we've probably run out of devices.
             */
            break;
        }
    }

    if (fd == -1)
    {
        fprintf(stderr, "%s(): open(): (%s): %s\n",
                __func__, device, strerror(errno));
    }

    return (fd);
}


/*******************************************************************************
* nicOpen
*
* DESCRIPTION:
*       open NIC
*
* INPUTS:
*       ifname  - interface name
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Zero if error
*       Non zero if success
*
*******************************************************************************/
int nicOpen(char *ifname)
{
    struct ifreq ifr;
    struct bpf_version bv;
#if 1
    u_int  immediate_mode = 1;
#endif
    u_int  v;

#if defined(BIOCGHDRCMPLT) && defined(BIOCSHDRCMPLT) && !(__APPLE__)
    u_int spoof_eth_src = 1;
#endif

    nicFd = bsd_bpf_open();
    if (nicFd < 0)
    {
        return 0;
    }

    /*
     *  Get bpf version.
     */
    if (ioctl(nicFd, BIOCVERSION, (caddr_t)&bv) < 0)
    {
        fprintf(stderr, "%s(): BIOCVERSION: %s\n",
                __func__, strerror(errno));
        close(nicFd);
        return 0;
    }

    if (bv.bv_major != BPF_MAJOR_VERSION || bv.bv_minor < BPF_MINOR_VERSION)
    {
        fprintf(stderr, "%s(): kernel bpf filter out of date\n", __func__);
        close(nicFd);
        return 0;
    }

    /*
    *  Attach network interface to bpf device.
    */
    memset((void *)&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);
    ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';

    if (ioctl(nicFd, BIOCSETIF, (caddr_t)&ifr) == -1)
    {
        fprintf(stderr, "%s(): BIOCSETIF: (%s): %s\n",
                __func__, ifname, strerror(errno));
        close(nicFd);
        return 0;
    }

#if 1
    /*  activate immediate mode (therefore, buf_len is initially set to "1") */
    if( ioctl( nicFd, BIOCIMMEDIATE, &immediate_mode ) == -1 )
    {
        fprintf(stderr, "%s(): BIOCIMMEDIATE: (%s): %s\n",
                __func__, ifname, strerror(errno));
        close(nicFd);
        return 0;
    }
#endif
    /* request buffer length */

    if (ioctl(nicFd, BIOCGBLEN, (caddr_t)&buffer_size) < 0)
    {
        fprintf(stderr, "%s(): BIOCGBLEN: (%s): %s\n",
                __func__, ifname, strerror(errno));
        close(nicFd);
        return 0;
    }

    /*
     *  Get the data link-layer type.
     */
    if (ioctl(nicFd, BIOCGDLT, (caddr_t)&v) == -1)
    {
        fprintf(stderr, "%s(): BIOCGDLT: %s\n",
                __func__, strerror(errno));
        close(nicFd);
        return 0;
    }

    /*
     *  NetBSD and FreeBSD BPF have an ioctl for enabling/disabling
     *  automatic filling of the link level source address.
     */
#if defined(BIOCGHDRCMPLT) && defined(BIOCSHDRCMPLT) && !(__APPLE__)
    if (ioctl(nicFd, BIOCSHDRCMPLT, &spoof_eth_src) == -1)
    {
        fprintf(stderr, "%s(): BIOCSHDRCMPLT: %s\n",
                __func__, strerror(errno));
        close(nicFd);
        return 0;
    }
#endif

    /* set promiscuous mode, okay if it fails */
    if (ioctl(nicFd, BIOCPROMISC, NULL) < 0)
    {
        fprintf(stderr, "BIOCPROMISC: %s", strerror(errno));
        close(nicFd);
        return 0;
    }

    /* Don't reflect trasmitted packets */
    v = 0;
    if (ioctl(nicFd, BIOCSSEESENT, &v) < 0)
    {
        fprintf(stderr, "BIOCSSEESENT: %s", strerror(errno));
        close(nicFd);
        return 0;
    }

    /* allocate buffer */
    if ((buff = (char*)malloc(buffer_size)) == NULL)
    {
        fprintf(stderr, "Can't allocate input buffer for bpf reads: %s",
                strerror(errno));
        close(nicFd);
        return 0;
    }

    return 1;
}

/*******************************************************************************
* nicDoRx
*
* DESCRIPTION:
*       perform Rx on NIC
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
*       This function called when input is ready on nicFd
*
*******************************************************************************/
int nicDoRx(void)
{
    int read_bytes;
    unsigned char * frame;
    unsigned char * bpf_packetraw;
    unsigned char * bpf_eob;
    struct bpf_hdr * bpf_packet;

    /* handle partial read */
    read_bytes = read(nicFd, buff, buffer_size);
    if (read_bytes < 0)
    {
        perror("read");
        if (errno != EINTR)
            return 0;
    }

    bpf_eob = buff + read_bytes;
    bpf_packetraw = buff;
    while ( bpf_packetraw < bpf_eob )
    {
        /* Format the data */
        bpf_packet = (struct bpf_hdr *) bpf_packetraw; 
        /* Here is the actual Ethenet frame */
        frame = bpf_packetraw + bpf_packet->bh_hdrlen;
        /* For next time, BPF may return more then one frame in  the buffer */
        bpf_packetraw += BPF_WORDALIGN( bpf_packet->bh_hdrlen + bpf_packet->bh_caplen );
        if (bpf_packet->bh_caplen < bpf_packet->bh_datalen)
        {
            /* Not the whole fram is inside */
            /* I think it can be rejected, Ethrenet is not assume to be reliable */
            perror("Partial frame recieved");
            return 1;
        }
        if (bpf_packet->bh_datalen < 0)
        {
            fprintf(stderr, "file: %s(%d),  bad frame: %s\n", __FILE__, __LINE__, strerror(errno));
            return 1;
        }

#if 0
        fprintf(stderr, "got packet. len=%d\n", bpf_packet->bh_datalen);
        {
            int k;
            for (k = 0; k < bpf_packet->bh_datalen; k++)
            {
                fprintf(stderr, " %02x", (unsigned char)frame[k]);
                if (k % 16 == 15)
                    fprintf(stderr, "\n");
            }
            if (k % 16 != 15)
                fprintf(stderr, "\n");
        }
#endif
        if (slanIsUp)
            slanLibTransmit(slan, frame, bpf_packet->bh_datalen);                
    }
    return 1;
}

/*******************************************************************************
* nicTx
*
* DESCRIPTION:
*       Transmit packet to NIC
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
int nicTx(const char *pktData, int pktLen)
{
    write(nicFd, pktData, pktLen);
    return 1;
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
#if 0
    printf("Got event for slan %d, userData=%p, evType=%d pktLen=%d\n",
            slanId, userData, eventType, pktLen);
#endif

    switch (eventType)
    {
        case SLAN_LIB_EV_CLOSED:
            fprintf(stderr,"Connector has closed connection\n");
            exit(0);
        case SLAN_LIB_EV_LINKUP:
            slanIsUp = 1;
            /* up interface */
            return;
        case SLAN_LIB_EV_LINKDOWN:
            slanIsUp = 0;
            /* down interface */
            return;
        case SLAN_LIB_EV_PACKET:
            nicTx(pktData, pktLen);
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
    printf("Usage: %s [-D] [-h] [-n slan_name] interface\n", argv0);
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
    if (optind >= argc)
    {
        usage(argv[0]);
        return 0;
    }

    if (slanName == NULL)
        slanName = argv[optind];

    if (!nicOpen(argv[optind]))
        return 1;

    slanLibInit(1,"ether");

    if (daemonize)
    {
        if (daemon(0,0) < 0)
        {
            perror("Can't daemonize");
        }
    }

    pthread_create(&rxthreadId, NULL, rx_thread, NULL);

    if (slanLibBind(slanName, ev_handler_func, NULL, &slan) != 0)
        return 1;

    while (1)
    {
        fd_set  rfds;

        FD_ZERO(&rfds);
        FD_SET(nicFd, &rfds);

        if (select(nicFd+1, &rfds, NULL, NULL, NULL) < 0)
			continue;

        if (FD_ISSET(nicFd, &rfds))
        {
            if (!nicDoRx())
                break;
        }
    }
    return 0;
}
