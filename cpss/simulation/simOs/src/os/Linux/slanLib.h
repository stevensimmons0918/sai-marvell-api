/*******************************************************************************
*              (c), Copyright 2008, Marvell International Ltd.                 *
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
* @file slanLib.h
*
* @brief SLAN library for UNIX-like OSes
*
* @version   3
********************************************************************************
*/
#ifndef __slanLib_h__
#define __slanLib_h__

#ifndef IN
#   define IN
#endif
#ifndef OUT
#   define OUT
#endif

/************ Defines  ********************************************************/
#define SLAN_CONNECTOR_ADDRESS "127.0.0.1"
#define SLAN_CONNECTOR_PORT    "34567"


/************ Protocol definitions ********************************************/
/* packet + header */
#define SLAN_INPUT_BUFFER_SIZE  12228
/* match only first 64 bytes of SLAN name */
#define SLAN_NAME_LEN           64

#define SLAN_PKT_HDR_SIZE       4
#define SLAN_LIB_UNPACK_HEADER(buffer,length,type) \
    do { \
        length = ((((unsigned char*)(buffer))[0] << 8) | ((unsigned char*)(buffer))[1]); \
        type = (SLAN_LIB_EVENT_TYPE)((((unsigned char*)(buffer))[2] << 8) | ((unsigned char*)(buffer))[3]); \
    } while (0)
#define SLAN_LIB_PACK_HEADER(buffer,length,type) \
    do { \
        (buffer)[0] = ((length) >> 8) & 0x0ff; \
        (buffer)[1] = (length) & 0x0ff; \
        (buffer)[2] = ((type) >> 8) & 0x0ff; \
        (buffer)[3] = (type) & 0x0ff; \
    } while (0)

/************* Typedefs *******************************************************/
typedef int SLAN_ID;

typedef enum {
    SLAN_LIB_EV_PACKET,
    SLAN_LIB_EV_LINKUP,
    SLAN_LIB_EV_LINKDOWN,
    SLAN_LIB_EV_CLOSED,
    SLAN_LIB_EV_HANDSHAKE,
    SLAN_LIB_EV_HANDSHAKE_SUCCESS,
    SLAN_LIB_EV_HANDSHAKE_FAILED,
} SLAN_LIB_EVENT_TYPE;

typedef enum {
    SLAN_LIB_LINK_UP,
    SLAN_LIB_LINK_DOWN
} SLAN_LIB_LINK_STATUS_TYPE;


/*******************************************************************************
* SLAN_LIB_EVENT_HANDLER_FUNC
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
typedef void (*SLAN_LIB_EVENT_HANDLER_FUNC)
(
    IN  SLAN_ID             slanId,
    IN  void*               userData,
    IN  SLAN_LIB_EVENT_TYPE eventType,
    IN  char*               pktData,
    IN  int                 pktLen
);


/************* Functions ******************************************************/

/**
* @internal slanLibInit function
* @endinternal
*
* @brief   Initialize library
*
* @param[in] slansMax                 - Maximum number of SLANS
* @param[in] clientName               - Client name for debug tricks in slanConnector
*                                      Can be NULL. The values will be
*                                      "asic", "smbsim", "ether", etc
*                                       Zero if success
*                                       Non zero if error occured
*                                       EALREADY
*                                       EINVAL
*                                       ENOMEM
*/
int slanLibInit
(
    IN  int slansMax,
    IN  const char *clientName
);

/**
* @internal slanLibClose function
* @endinternal
*
* @brief Close SLAN lib
*/
int slanLibClose();

/**
* @internal slanLibMainLoop function
* @endinternal
*
* @brief   Mail loop function. slanLibInit() doesn't create thread because
*         thread can require to be created by specific API
*         Must be called after slanLinInit()
*/
int slanLibMainLoop(int timeoutSec);

/**
* @internal slanLibBind function
* @endinternal
*
* @brief   This function connects to SLAN Connector and send slanName identifier.
*         Connection is added to slanLibMainLoop() pool
* @param[in] slanName                 - SLAN name
* @param[in] handler                  - event  function
* @param[in] userData                 - pointer to user data
*
* @param[out] slanId                   - SLAN id
*                                       Zero if success
*                                       Non zero if error occured
*
* @note $SLAN_PREFIX environment variable will be added to slanName
*
*/
int slanLibBind
(
    IN  const char*                     slanName,
    IN  SLAN_LIB_EVENT_HANDLER_FUNC     handler,
    IN  void*                           userData,
    OUT SLAN_ID                         *slanId
);

/**
* @internal slanLibUnbind function
* @endinternal
*
* @brief   Remove connection from slanLibMainLoop() pool.
*         Then connection to SLAN Connector closed
* @param[in] slanId                   - SLAN id
*                                       Zero if success
*                                       Non zero if error occured
*/
int slanLibUnbind
(
    IN  SLAN_ID     slanId
);

/*******************************************************************************
* slanLibGetUserData
*
* DESCRIPTION:
*       Get custom pointer passed to bind function
*
* INPUTS:
*       slanId      - SLAN id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       User data
*       NULL if error occured
*
* COMMENTS:
*
*******************************************************************************/
void* slanLibGetUserData
(
    IN  SLAN_ID     slanId
);

/*******************************************************************************
* slanLibGetSlanName
*
* DESCRIPTION:
*       Get slan name
*
* INPUTS:
*       slanId      - SLAN id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Slan name
*       NULL if error occured
*
* COMMENTS:
*
*******************************************************************************/
const char* slanLibGetSlanName
(
    IN  SLAN_ID     slanId
);

/**
* @internal slanLibTransmit function
* @endinternal
*
* @brief   Transmit packet to SLAN
*
* @param[in] slanId                   - SLAN id
* @param[in] packetData               - pointer to packet data
* @param[in] packetLen                - pointer to packet length
*                                       Number or bytes sent
*                                       < 0 if error occured
*/
int slanLibTransmit
(
    IN  SLAN_ID     slanId,
    IN  const char  *packetData,
    IN  int         packetLen
);

/**
* @internal slanLibSetLinkStatus function
* @endinternal
*
* @brief   Change SLANs link status
*
* @param[in] slanId                   - SLAN id
* @param[in] linkStatus               - new link status
*                                       Number or bytes sent
*                                       < 0 if error occured
*/
int slanLibSetLinkStatus
(
    IN  SLAN_ID                     slanId,
    IN  SLAN_LIB_LINK_STATUS_TYPE   linkStatus
);

#endif /* __slanLib_h__ */

