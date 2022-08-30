// xpAppUtil.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpAppUtil_h_
#define _xpAppUtil_h_

#include "xpTypes.h"
#include "xpsLink.h"

//Tree managment utility (standard linux library)
#include <search.h>
//To support directory related operations (Posix library)
#include <dirent.h>
#include <semaphore.h>

#ifdef MAC
#include <libkern/OSByteOrder.h>
#define htole32(x) OSSwapHostToLittleInt32(x)
#endif

//Port Tune and Auto Negotiation thread Interval in Micro Second
#define PORT_TUNE_AND_AN_THREAD_INTERVAL 1000       ///< PORT_TUNE_AND_AN_THREAD_INTERVAL 

//MACROS defined by the customer code
#define MAX_SYS_DEVICES (XP_MAX_DEVICES)
#define MAX_SYS_PORTS   136
#define NUM_OF_TUNNELS  256
#define MAX_STAT_BUF_SIZE   80
#define MAX_MULTICAST_ENTRIES  100
#define XP_PCI_CLASS "/sys/class/xppci"
#define XP_PCI_DEV_PREFIX "xppcidev"

#define MAX_FILENAME_LEN    256
#define MAX_PATH_LENGTH     256
#define MAX_RX_QUEUES 64
#define XP_SEM_NAME_LEN     15
#define MAX_RX_QUEUES       64
#define MAX_IPC_MSG_LENGTH 1024
#define MAX_PATH_NAME_LENGTH 512
#define RATE_LIMIT_COUNT    500
#define NUM_IPC_DESC  77
#define IPC_MSG_POLLING_DELAY 100000
#define SCPU_IPC_LOG_FILE_NAME "/scpuIpc.log"

/* Mutual Exclusion Related Definitions - For Multithreaded Environment*/
#ifdef MAC
// This is to work around Darwin library incompatibilities
#define xpSemCreate(semPtr, count)          XP_NO_ERR
#define xpSemDestroy(semPtr)
#else
#define xpSemCreate(semPtr, count)          sem_init(semPtr, 0, count)
#define xpSemDestroy(semPtr)                sem_destroy(semPtr)
#endif // MAC

#define xpSem_t                             sem_t
#define xpSemWait(semPtr)                   sem_wait(semPtr)
#define xpSemPost(semPtr)                   sem_post(semPtr)
#define xpSemTryWait(semPtr)                sem_trywait(semPtr)
/* User has to specify number of packets needed to be received in a call */
#define XP_NUM_OF_PKTS_TO_RECV  1

// Link polling thread sleep time in microsecond
#define LINK_POLL_THREAD_INTERVAL 15000000
//global definition for allocated mpls vif database's "mplsTnlVifRoot"
extern void *mplsTnlVifRoot[XP_MAX_DEVICES];

//compare function for mpls tunnel vif database
int compareMplsTnlVifMap(const void *l, const void *r);

typedef struct allocatedMplsTnlVifMap
{
    int key;           ///< key
    uint32_t tnlId;    ///< tnlId
} allocatedMplsTnlVifMap;

/*Port Tune and Auto Negotiation thread information*/
typedef struct _xpLinkPortTuneAndANThreadInfo
{
    uint8_t enableStatus; ///< Enable status. 1/0 for enable/disable.
    unsigned long long sleepInterval; ///<Thread sleep time in microsecond.
} xpPortTuneAndANThreadInfo;

/* Rx Packet Thread config */
typedef struct xpRecvThreadCfg
{
    uint32_t numPktsToRecv; ///< Number of pkts to receive, set 0 to infinite loop
} xpRecvThreadCfg;
//get allocated tnlvif from mplsIdx
void* getAllocatedMplsTnlVif(xpDevice_t devId, uint32_t mplsIdx);

void setBit(uint32_t devId, uint8_t *devBitMap);

void clearBit(uint32_t devId, uint8_t *devBitMap);

int32_t isBitSet(uint32_t devId, uint8_t *devBitMap);

void* xpAppReceiveTrapPacket(void *arg);
XP_STATUS xpAppPacketDriverCallbackConfigurations(xpDevice_t deviceId);

/**
 * \brief  Checks all bits of array are cleard or not
 *
 * \param [in] devId
 * \param [in] devBitMap
 *
 * \return 0 if all bits are cleared and -1 otherwise
 */
int32_t isAllBitClear(uint32_t devId, uint8_t *devBitMap);

XP_STATUS xpAppConfigDefault(xpDevice_t devId, void* arg);
XP_STATUS xpAppPopulateData(xpDevice_t devId, void* arg);
XP_STATUS xpAppShellInit(xpDevice_t devId, void* arg);
XP_STATUS xpAppDevDefaultConfigSet();
XP_STATUS xpAppNewDevicesInit(xpInitType_t initType);
XP_STATUS xpAppMainLoop(xpInitType_t initType);
XP_STATUS xpAppSetDeviceValid(xpDevice_t devId);
XP_STATUS xpAppSchedPolicyInit(xpDevice_t devId);
XP_STATUS xpAppGetExecutableFileDir(const char *executableFileName,
                                    char *executableFileDir, unsigned long int maxLen);
XP_STATUS xpAppSafeMemoryFree(void **pptr);
XP_STATUS xpAppGetValidDeviceIds(xpDevice_t* devIds, uint32_t* n);
XP_STATUS xpGetPktDataAndPktLength(int fd, char **readData, uint32_t *pktLen);
void twalkFreeTreeNode(const void *nodep, const VISIT which, const int depth);
XP_STATUS xpAppTDestroy(void **root);
/**
 * \brief xpAppCreatePortTuneAndANThread
 *
 * \param [in] deviceId
 *
 * \return XP_STATUS
 */
XP_STATUS xpAppPortTuneAndANThreadCreate(xpDevice_t deviceId);
/**
 * \brief xpAppIntThreadCreate
 *
 * \param [in] deviceId
 *
 * \return XP_STATUS
 */
XP_STATUS xpAppIntThreadCreate(xpDevice_t deviceId);

/**
 * \brief xpAppSetPortTuneAndANThreadSleepInterval
 *
 * \param [in] deviceId
 * \param [in] value
 *
 * \return XP_STATUS
 */
XP_STATUS xpAppPortTuneAndANThreadSleepIntervalSet(xpDevice_t deviceId,
                                                   uint32_t value);

/**
 * \brief xpAppGetPortTuneAndANThreadSleepInterval
 *
 * \param [in] deviceId
 * \param [out] value
 *
 * \return XP_STATUS
 */
XP_STATUS xpAppPortTuneAndANThreadSleepIntervalGet(xpDevice_t deviceId,
                                                   uint32_t *value);

/**
 * \brief xpAppPortTuneAndANThreadHndlr
 *
 * \param [in] arg
 *
 * \return XP_STATUS
 */
void* xpAppPortTuneAndANThreadHndlr(void *arg);

/**
 * \brief xpAppFdbL2LearningThreadCreate
 *
 * \param [in] deviceId Device Id. Valid values are 0-63
 *
 * \return XP_STATUS
 */

XP_STATUS xpAppFdbL2LearningThreadCreate(xpDevice_t deviceId);

/**
 * \brief xpAppFdbLearnigThreadHndlr
 *
 * \param [in] arg FDB thread handler input argument
 *
 * \return XP_STATUS
 */

void* xpAppFdbLearnigThreadHndlr(void *arg);

/*
 * \brief xpAppGetExitStatus
 *
 *
 * \return exitStatus > 1 if exit is set
 */
int xpAppGetExitStatus(void);

/**
 * \brief xpAppSetExitStatus
 *
 *
 * \return void
 */

void xpAppSetExitStatus(void);
#endif // _xpAppUtil_h_
