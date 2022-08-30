// xpsXpImports.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsXpImports.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Interface Manager
 */

#ifndef _xpsXpImports_h_
#define _xpsXpImports_h_

#include "xpTypes.h"
#include "xpEnums.h"
#include "xpsEnums.h"
#include "openXpsInterface.h"
#include <signal.h>
#ifdef __cplusplus
extern "C" {
#endif
//TODO RPR
#define MAX_SERDES_PER_PORT 8
#define XP_ANA_BANK_MODE_NONE    (0)       /* Invalid */
#define XP_ANA_BANK_MODE_A       (1)       /* counter mode A */
#define XP_ANA_BANK_MODE_B       (2)       /* counter mode B */
#define XP_ANA_BANK_MODE_C       (3)       /* counter mode C */
#define XP_ANA_BANK_MODE_D       (4)       /* counter mode D */
#define XP_ANA_BANK_MODE_P       (8)       /* police without billing counter */
#define XP_ANA_BANK_MODE_PC      (9)       /* police with billing counter */
#define XP_ANA_BANK_MODE_S       (12)      /* sampling */
#define MAX_GENEVE_SUPPORTED_OPTIONS     3 ///< MAX_GENEVE_SUPPORTED_OPTIONS
#define XP_MAX_FILE_NAME_LEN      (256)
typedef enum xp_nl_hostif_trap_channel
{
    XP_NL_HOSTIF_TRAP_FD,     /* Receive packets via file desriptor */
    XP_NL_HOSTIF_TRAP_CB,     /* Receive packets via callback       */
    XP_NL_HOSTIF_TRAP_NETDEV, /* Receive packets via OS net device  */
    XP_NL_HOSTIF_TRAP_CUSTOM_RANGE_BASE = 0x10000000
} xp_nl_hostif_trap_channel_t;


/**
 * \public
 * \brief This struct represents an FDB entry.
 */
typedef struct xpFdbTblEntry_t
{
    macAddr_t macAddr;          ///< macAddr
    uint32_t bdId;          ///< bdId
    uint8_t pktCmd;         ///< pktCmd
    uint8_t macDAIsControl;     ///< macDAIsControl
    uint8_t routerMAC;          ///< routerMAC
    uint8_t countMode;          ///< countMode
    uint8_t ecmpSize;           ///< ecmpSize
    xpL2EncapType_e encapType;      ///< encapType
    xpL2Encapdata_t l2EncapData;    ///< l2EncapData
    uint8_t isTunnelSpoke;      ///< isTunnelSpoke
    uint8_t isStaticMac;        ///< isStaticMac
    xpVif_t vif;            ///< vif
} xpFdbTblEntry_t;


/**
 * \typedef xpTnlIvifFieldList_t
 * \public
 * \brief This type (enum) defines the fields in the entry structure of
 *           TnlIvif Table.
 *
 */
typedef enum
{
    TNL_IVIF_SET_INGRESS_VIF,
    TNL_IVIF_SET_PKT_CMD,
    TNL_IVIF_P2MP_BUDNODE,
    TNL_IVIF_INGRESS_VIF,
    TNL_IVIF_RSVD,
    TUNNELIVIF_KEY_IPADDRESS,
    TNL_IVIF_FIELD_NUM
} xpTnlIvifFieldList_t;

/**
 * \public
 * \brief Structure representing QinQVEB key format for TnlIvif Table entry.
 *
 */
typedef struct xpTunnelIvifQinQVEBKey_t
{
    uint32_t keyReserved0:4;        ///< keyReserved0
    uint32_t keyIngressVif : 16;    ///< keyIngressVif
    uint32_t keySTag : 12;          ///< keySTag
    uint32_t keyCTag : 12;          ///< keyCTag
    uint32_t keyReserved1 : 20;     ///< keyReserved1
    uint32_t keyReserved2 : 32;     ///< keyReserved2
    uint32_t keyReserved3 : 32;     ///< keyReserved3
} xpTunnelIvifQinQVEBKey_t;

/* Shaper configuration modes */
typedef enum xpShaperType
{
    XP_PORT_SHAPER_TYPE,
    XP_QUEUE_SHAPER_TYPE,
    XP_QUEUE_FAST_SHAPER_TYPE
} xpShaperType;


/**
 * \brief To get serdes from port.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 to 127 and 176.
 * \param [in] macConfigMode enum specifying mac configuration mode
 * \param [out] portSerdesArrIds serdes IDs in array
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpLinkManagerGetSerdesIdsForPort(xpDevice_t devId, uint32_t portNum,
                                           xpMacConfigMode macConfigMode, int16_t *portSerdesArrIds);
XP_STATUS xpLinkManagerPlatformGetSerdesLaneSwapInfo(xpDevice_t devId,
                                                     uint32_t portNum, uint8_t channelNum, uint8_t *channelSwapNum);

/**
 * \brief This method initializes the SAL object.
 * \param [in] devType
 * \return int
 */
int xpSalInit(XP_DEV_TYPE_T devType);


/**
 * \public
 * \brief
 *
 * \return xpSal *
 */
xpSalType_t xpGetSalType();

/**
 *
 * \param salType
 */
void xpSetSalType(xpSalType_t salType);

/**
 * \brief get device type, for HW it will return based on HW and
 *  for WM it will return devType specified in SAL init
 * \param [in] devId
 * \param [in] devType
 * \return XP_STATUS
 */
XP_STATUS xpSalGetDeviceType(xpDevice_t devId, XP_DEV_TYPE_T *devType);

/**
 * \brief To get mcpu port number.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [out] mcpuPortNum MCPU port number
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpGlobalSwitchControlGetMcpuPortNumber(xpDevice_t devId,
                                                 uint32_t* mcpuPortNum);

/**

 * \brief Get various statistics about Port in xpStatistics structure directly
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0-127 and 176.
 * \param [in] fromStatNum Start collecting statistics from this number. Valid values are 0-31
 * \param [in] toStatNum Collect statistics till this number. Valid values are 0-31
 * \param [out] stat A structure which holds various statistics of a port. Statistics of counters which
 *                   falls in range of fromStatNum to toStatNum will only be updated.
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpLinkManagerCounterStatsDirectGet(xpDevice_t devId, uint32_t portNum,
                                             uint8_t fromStatNum, uint8_t toStatNum, xp_Statistics *stat);

/**
 * \brief xpSalThreadCreateName
 *
 * \param [in] thread Thread Id of thread
 * \param [in] attr  Thread attributes
 * \param [in] start_routine thread handler
 * \param [in] arg   Thread arguments
 *
 * \return Status
 */
XP_STATUS xpSalThreadCreateName(void *thread, const char *name,
                                const void *attr, void *(*start_routine)(void *), void *arg);

/**
 * \brief xpSalThreadDelete
 *
 * \param [in] thread Thread Id of thread
 *
 * \return Status
 */
XP_STATUS xpSalThreadDelete(void *thread);

XP_STATUS xpIpcRecvDebugMessage(xpDevice_t devId, char *data, uint16_t *size);

#define xpSalThreadCreate(t, r, h, a)   xpSalThreadCreateName(t, #h, r, h, a)

#ifdef __cplusplus
}
#endif

#endif // _xpsXpImports_h_


