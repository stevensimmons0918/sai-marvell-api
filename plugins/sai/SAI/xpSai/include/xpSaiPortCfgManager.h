// xpSaiPortCfgManager.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiPortCfgManager_h_
#define _xpSaiPortCfgManager_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * XDK XPS layer provides the APIs that mostly operates from HW registers
 * perspective while upper layer SW (SAI Adapter) expects management of
 * individual port attributes like "speed", "duplex", "fec", "autoneg", etc.
 * Currently XDK does not provide possibility of "per-attribute" management,
 * but operates with a set of attributes.
 * To satisfy OCP SAI requirements for "per-attribute" management of port
 * attributes the port configuration and state manager is introduced.
 * xpSaiPortCfgManager implements XPS specific configuration related activities
 * like serdes tuning or AN+LT state machine and reads link related information
 * from HW. Also it provides convenient interface and guarantees acceptable
 * response time.
 *
 *                  |-----------------------------------------------|
 *                  |                                               |
 *                  |            Adapter Port Settings              |
 *                  |     (speed, duplex, admin state, autoneg      |
 *                  |      abilities, loopback mode, PVID, etc.)    |
 *                  |                                               |
 *                  |-----------------------------------------------|
 *                           |                             |
 * |----------------------------------------|  |------------------------------|
 * |    XPS Interface + Port properties     |  |  Port (link) Config Manager  |
 * |     (PVID, BD, Policer, CoS, etc.)     |  |---------------|--------------|
 * |----------------------------------------|  |  fixed speed  |              |
 *                                             |---------------|  autoneg+lt  |
 *                                             |    loopback   |              |
 *                                             |---------------|--------------|
 *                                                             |
 *                                             |------------------------------|
 *                                             |           MAC Layer          |
 *                                             |------------------------------|
 *                                                             |
 *                                             |------------------------------|
 *                                             |         Serdes Layer         |
 *                                             |------------------------------|
 *
 * Adapter port settings work with properties like speed/FEC/autoneg/loopback
 * without really knowing of underlying HW.
 * On Marvell chip family port speed/FEC/autoneg/loopback configuration works on
 * top of MAC and Serdes layers and should be treated with a respect to underlying
 * HW.
 *
 * ** Configuration parameters application order **
 *
 * Port configuration parameters of speed/FEC/autoneg/loopback share the same
 * MAC and serdes layer and changing of one of this parameters should be applied
 * with a respect to others.
 *
 * From speed/FEC/autoneg/loopback port configuration parameters the configuration
 * manager checks for "loopback mode" in prior to others.
 *
 * If "loopback mode" is configured the port is configured to fixed speed+FEC
 * and "loopback mode" is applied.
 *
 * If "loopback mode" is disabled and autoneg is "ON" the port auto-negotiation
 * process is active. Enabling of autoneg is considered as another speed mode,
 * where speed and FEC values are selected based on the negotiation with peer
 * instead of fixed value.
 *
 * If If "loopback mode" is disabled and autoneg is "OFF" the port is configured to
 * fixed speed+FEC.
 *
 * Example #1:
 *  - User have configured 1X break-out mode and set speed to 40G, FEC to NONE.
 *    After enabling the port should come up with configured speed and FEC.
 *    After some time user configures autoneg to "ON" with 40G/100G and FC/RS
 *    FEC in local advertisements. Starting from now the previously
 *    configured "speed" and "FEC" are not taken into account, but autoneg is used.
 *    If user will configure autoneg to "OFF", then port will be re-configured
 *    to previously set "speed" and "FEC" settings (40G + no FEC).
 *
 * Example #2:
 *  - User have configured 1X break-out mode and set speed to 40G, FEC to NONE.
 *    After enabling the port should come up with configured speed and FEC.
 *    After some time user configures autoneg to "ON" and enabled loopback mode
 *    "MAC". In this case we cannot perform autoneg with loopback mode "MAC" and
 *    port will be configured to fixed speed 40G and FEC NONE and loopback mode
 *    "MAC" will be enabled. After user will disable loopback mode the auto-
 *    negotiation process will be started on the port.
 *
 *
 * ** Port beak-out operation **
 *
 * Port can operate with one of next break-out modes:
 * 1. XPSAI_PORT_BREAKOUT_MODE_1X -- Port use 1 serdes lane (hw lane)
 * 2. XPSAI_PORT_BREAKOUT_MODE_2X -- Port use 2 serdes lanes (hw lanes)
 * 3. XPSAI_PORT_BREAKOUT_MODE_4X -- Port use 4 serdes lanes (hw lanes)
 * 4. XPSAI_PORT_BREAKOUT_MODE_8X -- Port use 8 serdes lanes (hw lanes)
 *
 * Internally "isActive" and "breakOutMember" port config fields are use to
 * track port break-out membership. "hwLaneList" field contains the list of HW
 * lanes that port can use during break-out configuration. Some ports have
 * "hwLaneList" containing 8 numbers, some 4 numbers, some 2 numbers and all others 1 number in
 * the list. Example:
 *  - Port #0 has "hwLaneList" containing values "0,1,2,3,4,5,6,7"
 *  - Port #4 has "hwLaneList" containing values "4,5,6,7"
 *  - Port #2 and #6 has "hwLaneList" containing values "2,3" and "6,7" correspondingly
 *  - Port #1, #3, #5, #7 has "hwLaneList" containg values "1", "3", "5" and "7" correspondingly
 *
 * "breakOutMember" field is used to track port break-out membership. Example:
 *  - Port #0 is configured in XPSAI_PORT_BREAKOUT_MODE_4X mode, so "breakOutMember"
 *    attribute for ports #0, #1, #2 and #3 are set to "0" specifying that HW lanes
 *    are currently participating in 1X break-out mode configured on port #0
 *
 * "isActive" field is used to track if port is currently configured into any
 * break-out mode. Example:
 *  - Port #0 is configured in XPSAI_PORT_BREAKOUT_MODE_4X mode, so "isActive" field
 *    if set to "1" for port #0 and to "0" for all other break-out members #1, #2 and #3
 *
 * Port break-out mode transition is possible only via special service
 * "XPSAI_PORT_BREAKOUT_MODE_NONE" mode. Example:
 *  - Port #0 is configured in XPSAI_PORT_BREAKOUT_MODE_4X mode, but the target
 *    is to have 2 ports with XPSAI_PORT_BREAKOUT_MODE_2X. To achieve this the
 *    next transitions should be made:
 *      1. Set port #0 break-out mode to XPSAI_PORT_BREAKOUT_MODE_NONE
 *      2. Set port #0 break-out mode to XPSAI_PORT_BREAKOUT_MODE_2X
 *      3. Set port #1 break-out mode to XPSAI_PORT_BREAKOUT_MODE_2X
 *
 *
 * ** Port configuration internals **
 *
 * The adapter level speed/FEC/autoneg/loopback properties configuration leads
 * to various mix of MAC and Serdes configurations. Autoneg 40G/100G Example:
 * 1. Autoneg process is reset on MAC layer
 * 2. MAC and Serdes layers for all 4 hw lanes are configured to predefined
 *    fixed configuration for AN
 * 3. After AN page exchange has been complete the link training process starts
 *    by setting port serdes signal override to "XP_SERDES_OK_FALSE_TO_PCS"
 * 4. The link training process sets MAC PCS loopback for a port and configure
 *    all serdes lanes associated with a port.
 * 5. Link training loop polls for link training completion status and when
 *    link training is complete removes MAC PCS loopback configuration from the
 *    port and sets port serdes signal override to "XP_SERDES_OK_LIVE"
 *
 * The above description takes some period of time and can fail at particular
 * step, so if we want to switch autoneg off we need to reset current MAC
 * and Serdes layers for a given port with a respect to current break-out mode
 * and reconfigure port to fixed speed settings.
 *
 * Internal API "xpSaiApplyPortInfo()" is actually taking care of configuration
 * reset of MAC and Serdes layers associated with a given port.
 *
 */

#define XPSAI_PORT_BREAKOUT_MODE_NONE           0
#define XPSAI_PORT_BREAKOUT_MODE_1X             1
#define XPSAI_PORT_BREAKOUT_MODE_2X             2
#define XPSAI_PORT_BREAKOUT_MODE_4X             3
#define XPSAI_PORT_BREAKOUT_MODE_8X             4

#define XPSAI_PORT_HW_LANE_CNT_MAX              8
#define XPSAI_PORT_HW_LANE_INVALID              255

#define XPSAI_PORT_DISABLE                      0
#define XPSAI_PORT_ENABLE                       1

#define XPSAI_PORT_SPEED_INVALID                0
#define XPSAI_PORT_SPEED_10MB                   10
#define XPSAI_PORT_SPEED_100MB                  100
#define XPSAI_PORT_SPEED_1000MB                 1000
#define XPSAI_PORT_SPEED_2500MB                 2500
#define XPSAI_PORT_SPEED_10G                    10000
#define XPSAI_PORT_SPEED_25G                    25000
#define XPSAI_PORT_SPEED_40G                    40000
#define XPSAI_PORT_SPEED_50G                    50000
#define XPSAI_PORT_SPEED_100G                   100000
#define XPSAI_PORT_SPEED_200G                   200000
#define XPSAI_PORT_SPEED_400G                   400000

#define XPSAI_PORT_DUPLEX_HALF                  0
#define XPSAI_PORT_DUPLEX_FULL                  1

#define XPSAI_PORT_FEC_NONE                     0
#define XPSAI_PORT_FEC_FC                       1
#define XPSAI_PORT_FEC_RS                       2
#define XPSAI_PORT_FEC_RS_544_514               3

#define XPSAI_PORT_AUTONEG_DISABLE              0
#define XPSAI_PORT_AUTONEG_ENABLE               1

#define XPSAI_PORT_ADVERT_SPEED_NONE            0x00000000
#define XPSAI_PORT_ADVERT_SPEED_10MB            0x00000001
#define XPSAI_PORT_ADVERT_SPEED_100MB           0x00000002
#define XPSAI_PORT_ADVERT_SPEED_1000MB          0x00000004
#define XPSAI_PORT_ADVERT_SPEED_10G             0x00000008
#define XPSAI_PORT_ADVERT_SPEED_25G             0x00000010
#define XPSAI_PORT_ADVERT_SPEED_40G             0x00000020
#define XPSAI_PORT_ADVERT_SPEED_50G             0x00000040
#define XPSAI_PORT_ADVERT_SPEED_100G            0x00000080
#define XPSAI_PORT_ADVERT_SPEED_200G            0x00000100  //doubt
#define XPSAI_PORT_ADVERT_SPEED_400G            0x00000200  //doubt

#define XPSAI_PORT_ADVERT_FEC_NONE              0x00000000
#define XPSAI_PORT_ADVERT_FEC_FC                0x00000001
#define XPSAI_PORT_ADVERT_FEC_RS                0x00000002
#define XPSAI_PORT_ADVERT_FEC_RS_544_514        0x00000003

#define XPSAI_PORT_ADVERT_FD_SPEED_1X_ALLOWED   (XPSAI_PORT_ADVERT_SPEED_10G | \
                                                 XPSAI_PORT_ADVERT_SPEED_25G | \
                                                 XPSAI_PORT_ADVERT_SPEED_50G)
#define XPSAI_PORT_ADVERT_HD_SPEED_1X_ALLOWED   (XPSAI_PORT_ADVERT_SPEED_NONE)
#define XPSAI_PORT_ADVERT_FEC_1X_ALLOWED        (XPSAI_PORT_ADVERT_FEC_FC | \
                                                 XPSAI_PORT_ADVERT_FEC_RS | \
                                                 XPSAI_PORT_ADVERT_FEC_RS_544_514)

#define XPSAI_PORT_ADVERT_FD_SPEED_2X_ALLOWED   (XPSAI_PORT_ADVERT_SPEED_10G | \
                                                 XPSAI_PORT_ADVERT_SPEED_25G | \
                                                 XPSAI_PORT_ADVERT_SPEED_50G | \
                                                 XPSAI_PORT_ADVERT_SPEED_100G)
#define XPSAI_PORT_ADVERT_HD_SPEED_2X_ALLOWED   (XPSAI_PORT_ADVERT_SPEED_NONE)
#define XPSAI_PORT_ADVERT_FEC_2X_ALLOWED        (XPSAI_PORT_ADVERT_FEC_FC | \
                                                 XPSAI_PORT_ADVERT_FEC_RS | \
                                                 XPSAI_PORT_ADVERT_FEC_RS_544_514)

#define XPSAI_PORT_ADVERT_FD_SPEED_4X_ALLOWED   (XPSAI_PORT_ADVERT_SPEED_10G | \
                                                 XPSAI_PORT_ADVERT_SPEED_25G | \
                                                 XPSAI_PORT_ADVERT_SPEED_50G | \
                                                 XPSAI_PORT_ADVERT_SPEED_40G | \
                                                 XPSAI_PORT_ADVERT_SPEED_100G| \
                                                 XPSAI_PORT_ADVERT_SPEED_200G)
#define XPSAI_PORT_ADVERT_HD_SPEED_4X_ALLOWED   (XPSAI_PORT_ADVERT_SPEED_NONE)
#define XPSAI_PORT_ADVERT_FEC_4X_ALLOWED        (XPSAI_PORT_ADVERT_FEC_FC | \
                                                 XPSAI_PORT_ADVERT_FEC_RS | \
                                                 XPSAI_PORT_ADVERT_FEC_RS_544_514)

#define XPSAI_PORT_ADVERT_FD_SPEED_8X_ALLOWED   (XPSAI_PORT_ADVERT_SPEED_10G | \
                                                 XPSAI_PORT_ADVERT_SPEED_25G | \
                                                 XPSAI_PORT_ADVERT_SPEED_50G | \
                                                 XPSAI_PORT_ADVERT_SPEED_40G | \
                                                 XPSAI_PORT_ADVERT_SPEED_100G| \
                                                 XPSAI_PORT_ADVERT_SPEED_200G|\
                                                 XPSAI_PORT_ADVERT_SPEED_400G)
#define XPSAI_PORT_ADVERT_HD_SPEED_8X_ALLOWED   (XPSAI_PORT_ADVERT_SPEED_NONE)
#define XPSAI_PORT_ADVERT_FEC_8X_ALLOWED        (XPSAI_PORT_ADVERT_FEC_FC | \
                                                 XPSAI_PORT_ADVERT_FEC_RS | \
                                                 XPSAI_PORT_ADVERT_FEC_RS_544_514)


#define XPSAI_PORT_LOOPBACK_MODE_NONE           0x00000000
#define XPSAI_PORT_LOOPBACK_MODE_MAC            0x00000001
#define XPSAI_PORT_LOOPBACK_MODE_PHY            0x00000002
#define XPSAI_PORT_LOOPBACK_MODE_RMT            0x00000004

#define XPSAI_PORT_LINK_STATUS_DOWN             0
#define XPSAI_PORT_LINK_STATUS_UP               1

#define XPSAI_PORT_ATTR_BREAKOUT_MASK           0x00000001
#define XPSAI_PORT_ATTR_HW_LANES_MASK           0x00000002
#define XPSAI_PORT_ATTR_ENABLE_MASK             0x00000004
#define XPSAI_PORT_ATTR_SPEED_MASK              0x00000008
#define XPSAI_PORT_ATTR_DUPLEX_MASK             0x00000010
#define XPSAI_PORT_ATTR_FEC_MASK                0x00000020
#define XPSAI_PORT_ATTR_AUTONEG_MASK            0x00000040
#define XPSAI_PORT_ATTR_LOCAL_ADVERT_FD_MASK    0x00000080
#define XPSAI_PORT_ATTR_LOCAL_ADVERT_HD_MASK    0x00000100
#define XPSAI_PORT_ATTR_LOCAL_ADVERT_FEC_MASK   0x00000200
#define XPSAI_PORT_ATTR_LOOPBACK_MASK           0x00000400
#define XPSAI_PORT_ATTR_LINK_STATUS_MASK        0x00000800
#define XPSAI_PORT_ATTR_REMOTE_ADVERT_FD_MASK   0x00001000
#define XPSAI_PORT_ATTR_REMOTE_ADVERT_HD_MASK   0x00002000
#define XPSAI_PORT_ATTR_REMOTE_ADVERT_FEC_MASK  0x00004000
#define XPSAI_PORT_ATTR_INTF_TYPE_MASK          0x00008000

#define XPSAI_PORT_ATTR_NONE_MASK               0x00000000

#define XPSAI_PORT_ATTR_ALL_RO_MASK             (XPSAI_PORT_ATTR_HW_LANES_MASK | XPSAI_PORT_ATTR_LINK_STATUS_MASK | \
                                                 XPSAI_PORT_ATTR_REMOTE_ADVERT_FD_MASK | XPSAI_PORT_ATTR_REMOTE_ADVERT_HD_MASK | \
                                                 XPSAI_PORT_ATTR_REMOTE_ADVERT_FEC_MASK)

typedef struct xpSaiPortMgrInfo_s
{
    uint8_t isActive;                               ///< Indicates if port currently can be accessed for configuration and
    ///< state management. Used to manage port break-in/break-out.
    ///< If main port is currently in break-in mode (e.g., 1x40G),
    ///< the non-main ports that are connected to same PTG will have
    ///< "isActive" set to "0" and will not be available for configuration and
    ///< state management. During main port break-out (4x mode)
    ///< the "isActive" flag for non-main ports will be set to "1".
    int32_t serdesRecheckDelay;                     ///< SerDes re-tune delay in case if media is connected, but link is down
    int32_t serdesTuneWait;                         ///< SerDes tune timeout in milliseconds. Non-zero value means SerDes tuning
    ///< is in progress. Should be set to "0" to restart tune procedure.
    uint32_t serdesFineTuneWait;                    ///< SerDes Fine tune timeout in milliseconds.
    uint8_t breakOutMember;                         ///< In break-in mode (e.g., 1x40G), the non-main ports will have
    ///< the "breakOutMember" set to the main port number. The main port will point
    ///< to itself. During main port break-out (4x mode) the "breakOutMember"
    ///< for non-main ports will point to themselves.
    uint8_t breakOutMode;                           ///< Port break-out mode. Is only valid for main port of the PTG.
    uint32_t hwLaneList[XPSAI_PORT_HW_LANE_CNT_MAX]; ///< HW lanes associated with the port. The port with 1X/2X/4X break-out
    ///< capabilities will contain the list of all 4 ports associated with PTG.
    ///< The port with 2X/4X break-out capabilities will contain the list of 2 ports.
    ///< The port with 4X capability will have only "hwLaneList[0]" filled with
    ///< current port HW lane number.
    uint8_t enable;                                 ///< Port admin state.
    uint32_t speed;                                 ///< Port speed.
    uint32_t intfType;                              ///< Port interface type
    uint8_t duplex;                                 ///< Port duplex mode.
    uint8_t fec;                                    ///< Port FEC mode.
    uint8_t autoneg;                                ///< Port auto-negotiation mode.
    uint32_t localAdvertFdAbility;                  ///< List of port local auto-negotiation full duplex advertised speeds.
    uint32_t localAdvertHdAbility;                  ///< List of port local auto-negotiation half duplex advertised speeds.
    uint32_t localAdvertFec;                        ///< List of port local auto-negotiation advertised FEC modes.
    uint8_t loopbackMode;                           ///< Port loopback mode.
    uint8_t linkStatus;                             ///< Port link status.
    uint32_t remoteAdvertFdAbility;                 ///< List of port remote partner auto-negotiation full duplex advertised speeds.
    uint32_t remoteAdvertHdAbility;                 ///< List of port remote partner auto-negotiation half duplex advertised speeds.
    uint32_t remoteAdvertFec;                       ///< List of port remote partner auto-negotiation advertised FEC modes.
    uint32_t removePortSpeed;                       ///< Removed the port with speed
    uint8_t countersWALinkState;                    ///< WA for counters junk value.
    uint32_t operSpeed;                             ///< Port operational speed
} xpSaiPortMgrInfo_t;


XP_STATUS xpSaiPortCfgMgrInit(xpsDevice_t devId);
uint8_t xpSaiIsPortActive(uint32_t portNum);
void xpSaiPortInfoInit(xpSaiPortMgrInfo_t *info);
XP_STATUS xpSaiPortCfgSet(xpsDevice_t devId, uint32_t portNum,
                          const xpSaiPortMgrInfo_t *info, uint32_t actionMask);
XP_STATUS xpSaiPortCfgGet(xpsDevice_t devId, uint32_t portNum,
                          xpSaiPortMgrInfo_t *info);
void xpSaiPortCfgMgrStateUpdate(xpsDevice_t devId, uint32_t pollCycleTimeMs);
void xpSaiPortCfgMgrStatusUpdate(xpsDevice_t devId, uint32_t portNum,
                                 int linkStatus);
uint32_t xpSaiPortGetOperSpeed(xpsDevice_t devId, uint32_t portNum);


#ifdef __cplusplus
}
#endif

#endif //_xpSaiPortCfgManager_h_
