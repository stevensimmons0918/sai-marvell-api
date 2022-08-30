// slanEthPortLinux.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

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
#include <sys/uio.h>
#include <linux/sockios.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "slanLib.h"
#include "linux_mii.h"

/************ Defines  ********************************************************/
#define NIC_MTU_SIZE     1600

/************* Global data ****************************************************/
int nicFd;                  /* socket FD that is bind to the NIC                     */
struct ifreq saved_ifr;     /* NIC old settings to be reconfigured                   */
int nicIfIndex;      /* NIC if-Index                                          */
char    buff[SLAN_INPUT_BUFFER_SIZE];

SLAN_ID slan;
int slanIsUp = 0;
int linkState = -1;

/************* Functions ******************************************************/

/*******************************************************************************
* restore_ifr
*
* DESCRIPTION:
*       restore interface flags at exit
*       called using atexit()
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
static void restore_ifr(void)
{
    ioctl(nicFd, SIOCSIFFLAGS, &saved_ifr);
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
* COMMENTS:
*
*******************************************************************************/
int nicOpen(const char* ifname)
{ 
    struct sockaddr_ll     sa;
    struct ifreq           ifr;

    /* create packet socket for each NIC */
    /* The code reference is: http://www.senie.com/dan/technology/sock_packet.html */

    /* we create only one socket per nic */
    nicFd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (nicFd < 0)
    {
        fprintf(stderr, "file: %s(%d), socket failed (err: %s)\n",
                 __FILE__, __LINE__, strerror(errno));
        return 0;
    }

    memset((void *)&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

    if (ioctl(nicFd, SIOCGIFINDEX, &ifr) != 0)
    {
        fprintf(stderr, "file: %s(%d), ioctl failed (err: %s)\n", 
                 __FILE__, __LINE__, strerror(errno));
        return 0;
    }
    
    /* Bind to the interface name */
    memset((void *)&sa, 0, sizeof(sa));

    nicIfIndex = ifr.ifr_ifindex;
    
    sa.sll_family   = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_ALL);
    sa.sll_ifindex  = ifr.ifr_ifindex;

    if (bind(nicFd, (struct sockaddr *)&sa, sizeof(sa)))
    {
        fprintf(stderr, "file: %s(%d), bind failed (err: %s)\n", 
                 __FILE__, __LINE__, strerror(errno));
        return 0;
    }

    memset((void *)&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

    if (ioctl(nicFd, SIOCGIFFLAGS, &ifr) < 0 )
    {
        fprintf(stderr,"file: %s(%d), ioctl failed (err: %s)\n", 
                 __FILE__, __LINE__, strerror(errno));
        return 0;
    }

    saved_ifr = ifr;

    if (atexit(restore_ifr) != 0)
    {
        fprintf(stderr, "file: %s(%d), atexit failed\n", __FILE__, __LINE__);
        return 0;
    }

    ifr.ifr_flags |= IFF_PROMISC | IFF_UP;
    if (ioctl(nicFd, SIOCSIFFLAGS, &ifr) < 0 )
    {
        fprintf(stderr, "file: %s(%d), ioctl failed (err: %s)\n",
                 __FILE__, __LINE__, strerror(errno));
        return 0;
    }

    return 1;
}

/********* CRC32 for ethernet packets **********************************/
static uint32_t crc32Table[256] =
{
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2, 
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c, 
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59, 
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106, 
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433, 
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84, 
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e, 
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28, 
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777, 
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};
/*******************************************************************************
* calcCrc32
*
* DESCRIPTION:
*       Calculate CRC 32 bit for input data.
*
* INPUTS:
*       buffer  - pointer to the buffer.
*       byteNum - number of bytes in the buffer.
*
* OUTPUTS:
*       crcPtr     - pointer to buffer for crc
*
* RETURNS:
*       none
*
* COMMENTS:
*   For calculation a new CRC the value of CRC should be 0xffffffff.
*
*******************************************************************************/
static void calcCrc32
(
    const char *buffer,
    int        byteNum,
    char       *crcPtr
)
{
    uint32_t crc;
    int      i;           /* Loop counter                      */
    uint8_t  newEntry;    /* The new entry in the lookup table */

    /* For calculation a new CRC the value of CRC should be 0xffffffff. */
    crc = 0xffffffff;

    for ( i = 0;  i < byteNum;  i++ )
    {
        newEntry = (uint8_t)( ( crc ^ (*((uint8_t*)buffer)) ) & 0xff );
        crc = crc32Table[newEntry] ^ (crc >> 8);
        buffer++;
    }

    crc = crc ^ 0XFFFFFFFF;

    crcPtr[0] = (crc & 0x000000FF);
    crcPtr[1] = (crc & 0x0000FF00) >> 8;
    crcPtr[2] = (crc & 0x00FF0000) >> 16;
    crcPtr[3] = (crc & 0xFF000000) >> 24;
}

/*******************************************************************************
* mii_read
*
* DESCRIPTION:
*       Read MII register
*
* INPUTS:
*       reg     - register number
*
* OUTPUTS:
*       None
*
* RETURNS:
*       value
*
* COMMENTS:
*
*******************************************************************************/
int mii_read(int reg)
{
    struct ifreq ifr;
    struct mii_data *mii;
    ifr = saved_ifr;
    mii = (struct mii_data *)&ifr.ifr_data;
    mii->reg_num = reg;
    if (ioctl(nicFd, SIOCGMIIREG, &ifr) < 0)
    {
        return -1;
    }
    return mii->val_out;
}

/*******************************************************************************
* readLinkState
*
* DESCRIPTION:
*       Read link up/down state
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0 - link down
*       1 - link up
*       -1 - if error
*
* COMMENTS:
*
*******************************************************************************/
int readLinkState(void)
{
    int i, mii_val[32];
    unsigned bmcr, bmsr, advert, lkpar;

    /* Some bits in the BMSR are latched, but we can't rely on being
     *        the only reader, so only the current values are meaningful */
    mii_read(MII_BMSR);
    for (i = 0; i < MII_BASIC_MAX; i++)
    {
        mii_val[i] = mii_read(i);
    }
    if (mii_val[MII_BMCR] == 0xffff)
    {
        /* No MII transceiver present */
        return -1;
    }
    bmcr = mii_val[MII_BMCR]; bmsr = mii_val[MII_BMSR];
    advert = mii_val[MII_ANAR]; lkpar = mii_val[MII_ANLPAR];
    if (bmcr & MII_BMCR_AN_ENA)
    {
        if (bmsr & MII_BMSR_AN_COMPLETE)
        {
            if (advert & lkpar)
            {
                return (lkpar & MII_AN_ACK) ? 1: 0;
            }
        }
    }
    else
    {
        /* no autoneg */
        return 1;
    }
    return 0;
}

/*******************************************************************************
* linkState_thread
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
void* linkState_thread(void* unused)
{
    int s;
    while (1)
    {
        s = readLinkState();
        if (s != linkState)
        {
            if (s >= 0)
            {
                if (slanIsUp)
                {
                    slanLibSetLinkStatus(slan,
                          (s > 0 ) ? SLAN_LIB_LINK_UP : SLAN_LIB_LINK_DOWN);
                }
            }
            linkState = s;
        }
        sleep(2);
    }
    return NULL;
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
*
*******************************************************************************/
int nicDoRx(void)
{
    int rcv;
    socklen_t fromlen;
    struct sockaddr_ll  from;

    /* read data from socket */
    fromlen = sizeof(from);
    rcv = recvfrom(nicFd, buff, NIC_MTU_SIZE,
                   0,  (struct sockaddr*)&from, &fromlen);

    if (rcv == -1)
    {
        fprintf(stderr, "file: %s(%d), recvfrom failed %s\n", __FILE__, __LINE__, strerror(errno));
        return 0;
    }
    if (from.sll_ifindex != nicIfIndex)
    {
        /* Not our device ignore */
        fprintf(stderr, "file: %s(%d), incorrect ifIndex (Rx %d, dev: %d)\n",
                __FILE__, __LINE__, from.sll_ifindex, nicIfIndex);
        return 1;
    }
    if (slanIsUp)
    {
        /*
         * Ethernet CRC dropped by Linux kernel
         * Add CRC to received packet to fix packet
         */
        calcCrc32(buff, rcv, buff+rcv);
        rcv += 4;

        slanLibTransmit(slan, buff, rcv);
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
    struct sockaddr_ll from;
    int                fromlen;

    memset((void *)&from, '\0', sizeof(from));

    from.sll_family = AF_PACKET;
    from.sll_protocol = htons(ETH_P_ALL);
    from.sll_ifindex = nicIfIndex;
    
    fromlen = sizeof(from);

    sendto(nicFd, pktData, pktLen, 0, (struct sockaddr *)&from, fromlen);
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
            if (linkState == 0)
                linkState = 1;
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
    pthread_t rxthreadId, lsthreadId;
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
        /* check if any #include required for FreeBSD here */
        if (daemon(0,0) < 0)
        {
            perror("Can't daemonize");
            /* continue in foreground mode */
        }
    }

    pthread_create(&rxthreadId, NULL, rx_thread, NULL);
    pthread_create(&lsthreadId, NULL, linkState_thread, NULL);

    if (slanLibBind(slanName, ev_handler_func, (void*)0xa, &slan) != 0)
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
