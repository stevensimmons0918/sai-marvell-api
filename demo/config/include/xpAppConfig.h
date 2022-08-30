// xpAppConfig.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef __XPAPPCONFIG_H__
#define __XPAPPCONFIG_H__
#include "xpTypes.h"
#include "xpConfigFile.h"
#ifdef __cplusplus
#endif

#define XP_DEFAULT_CONFIG_FILE      ""                          ///< Default Config File
#define XP_CONFIG                   (xpAppConf)                 ///< Config
#define XP_PROGNAME                 (xpAppConf.programName)     ///< Progname
#define XP_DEBUG                    false                       ///< Debug
#ifdef _POSIX_HOST_NAME_MAX
#define XP_HOSTNAME_LEN             _POSIX_HOST_NAME_MAX        ///< Hostname Len
#elif defined(HOST_NAME_MAX)
#define XP_HOSTNAME_LEN             HOST_NAME_MAX               ///< Hostname Len
#else
#define XP_HOSTNAME_LEN             255                         ///< Hostname Len
#endif
/**
 * \brief debugLogInfo
 *
 */
typedef struct _debugLogInfo
{
    uint8_t xpLogModuleId;      ///< xpLogModuleId
    char ModName[10];           ///< ModName
} debugLogInfo;

#define XP_CLI_NONE             (0)        ///<  Xp Cli None
#define XP_CLI_USER           (0x1)        ///<  Xp Cli User
#define XP_CLI_DEBUG          (0x2)        ///<  Xp Cli Debug
#define XP_CLI_CONFIG         (0x4)        ///<  Xp Cli Config
#define XP_CLI_MULTISHELL     (0x8)        ///<  Xp Cli Multishell

/**
 * \brief xpAppConfig
 *
 */
typedef struct _xpAppConfig
{
    uint32_t          userCli;                ///< CLI Mode
    bool              debugCli;               ///< Debug CLI Mode
    bool              diag;                   ///< Diag
    char*             withHw;                 ///< With Specified Target Board
    char              configFileName[100];    ///< Configuration File.
    char
    profileXmlFile[100];    ///< XML file Containing List of Profile Files.
    char              pcapConfigFile[100];    ///< PCAP File Name .
    uint8_t           queueNum;               ///< Queue Number
    uint8_t           rxConfigMode;           ///< Packet Driver Rx Config Mode
    char              programName[250];       ///< Name of the Executable.
    debugLogInfo      dbgLogInfo[32];         ///< Debug-info for Logger.
    int32_t           interruptPort;          ///< The Port for Interrupts
    uint32_t
    unitNum;                ///< Box on which this Code is being run
    char              schedPolicy[4];         ///< Scheduling Policy
    xpInitType_t      initType;               ///< Init Type
    xpSkuMode_e       mode;                   ///< Mode
    char              shellConfigFile[100];   ///< XPSHELL Configuration File.
    uint32_t          performanceMode;        ///< Performance Mode
    bool              noConfig;               ///< No Config done
    char              logfile[200];           ///< Log File Name
    XP_DEV_TYPE_T     devType;                ///< Device Type
    xpPacketInterface
    packetInterface;        ///< Packet Interface DMA/ETHER or NETDEV
    uint8_t
    numScope;               ///< Number of scope in Multiscope Environment.(default 0 indicates scope feature is disabled)
    uint8_t           disableLog;             ///< no logging in xpsApp output
    bool              daemonMode;             ///< Run as daemon
    uint8_t           portTuneStatus;         ///< Port auto-tune status.
    uint16_t          pktBufSize;             ///< packet buffer size
    uint16_t          txDescDepth;            ///< Tx Descriptor Depth
    uint16_t          rxDescDepth;            ///< Rx Descriptor Depth
    uint8_t           numOfTxQueue;           ///< Number of Tx Queue
    uint8_t           numOfRxQueue;           ///< Number of Rx Queue
    uint32_t          fetchRetryCount;        ///< Max retries to fetch desc
    bool              enablePrefetch;         ///< Enable Prefetch mode
    bool              enableDescRing;         ///< Enable Descriptor ring mode
    uint32_t          numOfPktsRecv;          ///< Pkts to be recevied in thread
    uint8_t
    completionBoundary;     ///< DMA Completion Boundary (Zero - to use the default one)
    uint8_t           portInitAllowed;        ///< link port need to be init
    uint32_t          initialPortVifVal;      ///< Initial Port Vif Value
    bool
    createdemoports;        /// create some netdev ports, used with switchdev development
    bool
    intEnable;              ///< Inband Network Telementry enable/disable
    bool              learnThreadStatus;      ///< FDB learning thread status
} xpAppConfig;

/**
 * \brief structure for Input Control Packet Entry
 *
 */
typedef struct _xpInputControlPktEntry
{
    char        pcapName[256];          ///< PCAP File Name
    char        queueNum[4];            ///< Queue Number
    char        nextEngine[256];        ///< Next Engine
    uint8_t     egressPort;             ///< Egress Port
    uint8_t     ingressPort;            ///< Ingress Port
    uint32_t    setBd;                  ///< Set BD
    uint32_t    bridgeDomain;           ///< Bridge Domain
    uint8_t     macSAMissCmd;           ///< MAC Source address Miss command
    uint8_t     acceptedFrameType;      ///< Accepted Frame Type
} xpInputControlPktEntry;

extern xpAppConfig xpAppConf;           ///< xpAppConf

/* print the logs only if enabled */
#define PRINT_LOG(format,args...)        \
                  if (xpAppConf.disableLog == 0) {      \
                     printf(format, ## args);    \
                  }

void setRxConfigMode(char *arg);
void getEnvValues(void);

/**
 * \brief Print help
 *
 */

void printHelp(void);
void readPcapConfigFile(char *);
/**
 * \brief getOptValue
 *
 * \param [in] argc
 * \param [in] argv
 * \param [in] moduleNames
 *
 */
void getOptValues(int argc, char* argv[], const char** moduleNames);

/**
 * \brief readPcapConfigFile
 *
 * \param [in] PCAP Config File to read
 *
 */
void readPcapConfigFile(char* PcapConfigFile);

/**
 * @fn printSkuToArgMap
 * @brief To print SKU vs argument value mapping.
 *
 */
void printSkuToArgMap();
#endif // __XPAPPCONFIG_H__
