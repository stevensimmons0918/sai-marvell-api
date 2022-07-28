// xpAppConfig.cpp

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <sys/utsname.h>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "xpDevTypes.h"
#include "xpAppConfig.h"
#include "xpsMac.h"
#ifdef __cplusplus
#endif

bool isDiagsMode;
/**
 * @fn xpAppConf
 * @public
 * @brief struct for xpApp configuration data.
 *
 */
xpAppConfig xpAppConf =
{
    XP_CLI_NONE,
    0,
    0,
    0,
    XP_DEFAULT_CONFIG_FILE,
    "",
    "",
    0,
    POLL,
    "",
    {{0, ""},},
    -1,
    0,              // unitNumber
    "",
    INIT_COLD,
    SKU_MODE_MAX,
    "None",         // xpShell loadable configuration file
    0,
    0,              // no config done
    "None",         //log file
    XP80B0,            // device type
    XP_DMA,            // default packet interface
    0,               // no scope
    0,              // log is enabled by default
    0,              // no daemon mode
    0,              // Disable port auto tune and AN thread
    XP_DEFAULT_PACKET_BUFFER_SIZE,         // packet buffer size
    XP_DEFAULT_TX_DESCRIPTOR_DEPTH,        // Tx descriptor depth
    XP_DEFAULT_RX_DESCRIPTOR_DEPTH,        // Rx descriptor depth
    XP_DEFAULT_TX_QUEUE_COUNT,             // Number of Tx queue
    XP_DEFAULT_RX_QUEUE_COUNT,             // Number of Rx queue
    0,              // Disable Fetch Retry
    0,              // Disable prefetch mode
    0,              // Disable descriptor ring
    0,              // num pkts for receive thread, 0 is infinite
    XP_DEFAULT_DMA_COMPLETION_BOUNDARY,    // DMA Completion Boundary (Zero - to use the default one)
    1,              // allow to init the link ports
    0,              // Initial port Vif Value
    false,          // create demo port
    false,          // int enablee
    false           // FDB learning thread status
};


/**
 * @fn setRxConfigMode
 * @public
 * @brief Get Receive Config Mode for packet driver
 *
 */
void setRxConfigMode(char *arg)
{
    if (!strncmp("POLL", arg, 4))
    {
        xpAppConf.rxConfigMode = POLL;
    }
}
/**
 * @fn getEnvValues
 * @public
 * @brief Processes environment variables.
 *
 */
void getEnvValues(void)
{
    char *value;
    if ((value= getenv("")) != NULL)
    {
        if (strlen(value) >= sizeof(xpAppConf.configFileName))
        {
            printf("Size of env variable %zu should be less than %zu\n", strlen(value),
                   sizeof(xpAppConf.configFileName));
            return;
        }
        strcpy(xpAppConf.configFileName, value);
    }
}


/**
 * @fn readPcapConfigFile
 * @public
 * @brief Processes pcap config file.
 *
 *
 * @param char* pcapConfigFile
 */
void readPcapConfigFile(char *pcapConfigFile)
{
    initEntryList(&configFile.controlPcapList);
    struct stat buffer;
    char line[XP_MAX_CONFIG_LINE_LEN], queueNum[4];
    int errorCode = 0;
    int notSpace = 0;
    int ingressPort;
    int egressPort;
    int setBd, bridgeDomain, acceptedFrameType, macSAMissCmd;
    int lineNum = 1;
    char pcapName[256];
    char nextEngine[256];
    xpInputControlPktEntry newData;

    if (pcapConfigFile == NULL ||
        strcmp(pcapConfigFile, "") == 0)            // If new filename passed in then:
    {
        // Set this->filename to new filename.
        PRINT_LOG("xpAppConfig::readPcapConfigFile(): No file to read.\n");
        return;
    }

    if (stat(pcapConfigFile, &buffer) == -1)
    {
        PRINT_LOG("Unable to find pcap configuration file \" %s \".  Exiting...\n",
                  pcapConfigFile);
        return;
    }
    FILE* config = fopen(pcapConfigFile, "r");
    if (config == NULL)
    {
        PRINT_LOG("Unable to open pcap configuration file \" %s \".  Exiting...\n",
                  pcapConfigFile);
        return;
    }

    while (!feof(config))                    // While not at the end of the file:
    {
        if (fgets(line, XP_MAX_CONFIG_LINE_LEN, config) != NULL)
        {
            notSpace= strspn(line, " \t\n\r");
            if (line[notSpace] != '\0' &&
                line[notSpace] != '#')    // If not blank line or comment:
            {
                errorCode= sscanf(line,
                                  "%255s\t\t%3s\t\t%255s\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d", pcapName, queueNum,
                                  nextEngine, &egressPort, &ingressPort, &setBd, &bridgeDomain, &macSAMissCmd,
                                  &acceptedFrameType);
                if (errorCode < 9)
                {
                    PRINT_LOG("Unable to parse line # %d in pcap configuration file \" %s \":\n",
                              lineNum, pcapConfigFile);
                    PRINT_LOG("%s\n", line);
                    PRINT_LOG("Skipping to next line...\n");
                    ++lineNum;
                    continue;
                }

                if (strcmp(pcapName, "") == 0 ||
                    strlen(pcapName) >
                    255)    // *** IF strlen(pcapName) > 255 THEN STACK IS CORRUPTED ***
                {
                    PRINT_LOG("pcapName is either empty or to large # %d in configuration file \" %s \".\n",
                              lineNum, pcapConfigFile);
                    PRINT_LOG("Skipping to next line...\n");
                    ++lineNum;
                    continue;
                }
                strncpy(newData.pcapName, pcapName, 254);
                newData.pcapName[255]= '\0';

                if (!strcmp(queueNum, "all"))
                {
                    strncpy(newData.queueNum, queueNum, 4);
                }
                else if (atoi(queueNum) >= 0 && atoi(queueNum) <= 63)
                {
                    strncpy(newData.queueNum, queueNum, 4);
                }
                else
                {
                    PRINT_LOG("Invalid value for queueNum. Valid values are 0-63/all\n");
                    PRINT_LOG("%s\n", line);
                    PRINT_LOG("Skipping to next line...\n");
                    continue;
                }

                if (strcmp(nextEngine, "") == 0 ||
                    strlen(nextEngine) >
                    255)     // *** IF strlen(nextEngine) > 255 THEN STACK IS CORRUPTED ***
                {
                    PRINT_LOG("nextEngine is either empty or to large # %d in configuration file \" %s \".\n",
                              lineNum, pcapConfigFile);
                    PRINT_LOG("Skipping to next line...\n");
                    ++lineNum;
                    continue;
                }
                else
                {
                    if (strcmp(nextEngine, "NONE") != 0 && strcmp(nextEngine, "IVIF") != 0 &&
                        strcmp(nextEngine, "URW") != 0)
                    {
                        PRINT_LOG("Invalid value of nextEngine %s \n", nextEngine);
                        PRINT_LOG("Assuming nextEngine : NONE\n");
                        strcpy(nextEngine, "NONE");
                    }
                }
                memcpy(newData.nextEngine, nextEngine, 254);
                newData.pcapName[255]= '\0';
                newData.egressPort = egressPort;
                newData.ingressPort = ingressPort;
                newData.setBd = setBd;
                newData.bridgeDomain = bridgeDomain;
                newData.macSAMissCmd = macSAMissCmd;
                newData.acceptedFrameType = acceptedFrameType;

                addEntry(&configFile.controlPcapList, &newData, sizeof(xpInputControlPktEntry));
            }
        }
        else if ((errorCode= ferror(config)) != 0)
        {
            printf("Error reading line # %d of file \" %s \":  %s \n", lineNum,
                   pcapConfigFile, strerror(errorCode));
            printf("Exiting...\n");
            fclose(config);
            return;
        }
        ++lineNum;
    }

    fclose(config);
    if (numOfEntry(&configFile.controlPcapList) == 0)
    {
        PRINT_LOG("Warning : File \" %s \" is empty.\n", pcapConfigFile);
        return;
    }
}

/**
 * @fn getOptValues
 * @public
 * @brief Processes command line options.
 *
 *
 * @param int argc
 * @param char** argv
 */
void getOptValues(int argc, char* argv[], const char** moduleNames)
{
    int opt;
    int optIndex = 0;
    isDiagsMode = false;
    char *subopts;
    char *value;
    int subopt;
    uint8_t logModIndex = 0;
    static struct option optList[] =
    {
        { "help",           no_argument,       0, 'h' },      // Print xpsApp help
        { "profile",        required_argument, 0, 'P' },      // Name of profile file.
        { "pcapConfigFile", required_argument, 0, 'x' },      // Name of control pcap config file.
        { "configfile",     required_argument, 0, 'c' },      // Provide configuration file
        { "pipeLine",       required_argument, 0, 'q' },      // Pipe line no (1 or 2, default is 1)
        { "tableProfile",   required_argument, 0, 's' },      // Table Profile (use values from enum XP_PROFILE_TYPE as string) (default is XP_FDB_128K_TWO_PIPE_PROFILE)
        { "xpdiags",        no_argument,       0, 'a' },      // xpdiags mode, needed only if user wants to run xp80 diags
        { "boxNumber",      required_argument, 0, 'b' },      // Box on which this code is being run
        { "emuMode",        no_argument,       0, 'B' },      // Emulation mode configuration
        { "userCli",        no_argument,       0, 'u' },      // User CLI mode
        { "shellConfig",    required_argument, 0, 'n' },      // xpShell configuration file
        { "debugCli",       no_argument,       0, 'w' },      // User debug CLI mode
        { "withHw",         required_argument, 0, 't' },      // Without whitemodel
        { "socketPort",     required_argument, 0, 'z' },      // The port on which xpApp will interact with wms
        { "interruptPort",  required_argument, 0, 'y' },      // The unitId with which whitemodel will interact
        { "debug",          required_argument, 0, 'd' },      // Debug mode.
        { "schedPolicy",    required_argument, 0, 'p' },      // Scheduling policy
        { "init type",      required_argument, 0, 'i' },      // System init type
        { "portSpeed",      required_argument, 0, 'm'},       // Set port speed
        { "coreClkFreq",    required_argument, 0, 'f' },      // Core clk frequency (400 or 500(default) or 550 / 600 / 633 / 700 MHz)
        { "packetInterface",    required_argument, 0, 'I' },  // PacketInterface
        { "performanceMode",  required_argument, 0, 'r' },    // Setup the performance mode (0-128H (Default), 1-64H, 2-192H)
        { "noConfig",       no_argument,      0, 'k' },       // No config done
        { "logfile",        required_argument, 0, 'l' },      // Setup the performance mode
        { "dumpState",      required_argument,      0, 'v' }, // State validation through dump comparison(1,2 or 3, default 3)
        { "device type",    required_argument, 0, 'g' },      // System init type
        { "scope",          required_argument, 0, 'j' },      // Multidevice with scope
        { "multiShell",     no_argument,      0, 'U' },       // Multiple xpShell
        { "persistent",     no_argument,      0, 'e' },       // Persistent configuration
        { "daemon Mode",    no_argument,      0, 'D' },       // Run as a daemon
        { "rxConfigMode",     optional_argument,      0, 'M' },         // rx configuration mode
        { "portTuneStatus",     no_argument,      0, 'o' }, //  Port auto-tune status
        { "pktBufferSize",  required_argument, 0, 'S' },  // Packet Buffer size
        { "txDescDepth",      required_argument, 0, 'T' },  // Tx Descriptor Depth
        { "numOfTxQueue",     required_argument, 0, 'Q' },  // Total number of Tx Queue
        { "rxDescDepth",      required_argument, 0, 'R' },  // Rx Descriptor Depth
        { "numOfRxQueue",     required_argument, 0, 'E' },  // Total number of Rx Queue
        { "enablePrefetch",   no_argument,       0, 'F' },  // Enable Prefetch
        { "enableDescRing",   no_argument,       0, 'L' },  // Enable Descriptor Ring
        { "fetchRetryCount",  required_argument, 0, 'Y' },  // Max retry fetch count
        { "numOfPktsRecv",    required_argument, 0, 'N' },  // No of Pkts to receive in rx thread
        { "dmaCompletionBoundary",  required_argument, 0, 'C' },  // DMA Completion Boundary
        { "billingCounters",  no_argument, 0, 'A' },  // Total number of Rx Queue
        { "LoggingThreadSCPIPCMessages",  no_argument, 0, 'X' },  //Start Running Logging Thread for SCPU IPC Messages
        { "disablePortInitAndVolRegs", no_argument, 0, 'O' }, // Do not allow port init and disable mac volatile register
        { "initialPortVifVal",           required_argument, 0, 'V' },         // Initial Port Vif Value
        { "l2LearningStatus",            no_argument, 0, 'Z' },         // Initial l2 learning thread status
        { "createdemoports", no_argument, 0, 'J'}, // Create interfaces for demo use, e.g. in switchdev development
        { "createdemoports", required_argument, 0, 'W'}, // Enable AP
        { 0,                0,                0,  0  },
    };

    while ((opt= getopt_long(argc, argv,
                             "hd:q:P:c:z:aukwUBs:r:b:x:y:p:i:t:f:m:n:l:v:g:I:j:eDoZJS:T:Q:R:E:C:A:OV:M:FLY:N:XK:W:",
                             optList, &optIndex)) != -1)
    {
        switch (opt)
        {
            case 'h':
                printHelp();
                exit(0);
                break;
            case 'r':
                xpAppConf.performanceMode = atoi(optarg);
                break;
            case 'K':
                break;
            case 'd':
                subopts = optarg;
                xpAppConf.createdemoports = false;
                while (*subopts != '\0' && moduleNames)
                {
                    subopt= getsubopt(&subopts, (char* const*)moduleNames, &value);
                    if (subopt == -1)
                    {
                        PRINT_LOG("Unrecognized module name in expression: %s\n", value);
                        exit(0);
                    }
                    else
                    {
                        PRINT_LOG("Module Name: %s, parameter: %s\n", moduleNames[subopt], value);
                        xpAppConf.dbgLogInfo[logModIndex].xpLogModuleId =  subopt;
                        memcpy(xpAppConf.dbgLogInfo[logModIndex].ModName, value, strlen(value));
                        logModIndex++;
                    }
                }
                break;
            case 'a':
                xpAppConf.diag = true;
                isDiagsMode = true;
                break;
            case 'b':
                xpAppConf.unitNum = atoi(optarg);
                break;
            case 'w':
                xpAppConf.debugCli = true;
                xpAppConf.userCli |= XP_CLI_DEBUG;
            /* dont break */
            case 'u':
                xpAppConf.userCli |= XP_CLI_USER;
                break;
            case 'U':
                xpAppConf.userCli |= XP_CLI_MULTISHELL;
                break;
            case 'D':
                xpAppConf.daemonMode = true;
                break;
            case 'k':
                xpAppConf.noConfig = true;
                break;
            case 'S':
                xpAppConf.pktBufSize = atoi(optarg);
                break;
            case 'T':
                xpAppConf.txDescDepth = atoi(optarg);
                break;
            case 'Q':
                xpAppConf.numOfTxQueue = atoi(optarg);
                break;
            case 'R':
                xpAppConf.rxDescDepth = atoi(optarg);
                break;
            case 'E':
                xpAppConf.numOfRxQueue = atoi(optarg);
                break;
            case 'C':
                xpAppConf.completionBoundary = atoi(optarg);
                break;
            case 'F':
                xpAppConf.enablePrefetch = true;
                break;
            case 'L':
                xpAppConf.enableDescRing = true;
                break;
            case 'Y':
                xpAppConf.fetchRetryCount = atoi(optarg);
                break;
            case 'N':
                xpAppConf.numOfPktsRecv = atoi(optarg);
                break;
            case 'x':
                strcpy(xpAppConf.pcapConfigFile, optarg);
                readPcapConfigFile(
                    XP_CONFIG.pcapConfigFile);            // read the pcap configuration file to send control pkts to WM.
                break;
            case 'P':
                strcpy(xpAppConf.profileXmlFile, optarg);
                break;
            case 'c':
                strcpy(xpAppConf.configFileName, optarg);
                break;
            case 'n':
                strcpy(xpAppConf.shellConfigFile, optarg);
                xpAppConf.userCli |= XP_CLI_CONFIG;
                break;

            case 'Z':
                xpAppConf.learnThreadStatus = true;
                break;

            case 'y':
                xpAppConf.interruptPort = atoi(optarg);
                PRINT_LOG("xpAppConf.interruptPort: %d\n", xpAppConf.interruptPort);
                break;
            case 'M':
                if (!strncmp("POLL", optarg, strlen("POLL")))
                {
                    xpAppConf.rxConfigMode = POLL;
                }
                else if (!strncmp("INTR", optarg, strlen("INTR")))
                {
                    xpAppConf.rxConfigMode = INTR;
                }
                break;
            case 'p':
                memcpy(xpAppConf.schedPolicy, optarg, 4);
                break;
            case 'i':
                /* Set device init type for xpApp */
                xpAppConf.initType = INIT_COLD;
                if (!strncmp("INIT_COLD", optarg, strlen("INIT_COLD")))
                {
                    xpAppConf.initType = INIT_COLD;
                }
                if (!strncmp("INIT_WARM", optarg, strlen("INIT_WARM")))
                {
                    xpAppConf.initType = INIT_WARM;
                }
                if (!strncmp("INIT_WARM_SYNC_TO_HW", optarg, strlen("INIT_WARM_SYNC_TO_HW")))
                {
                    xpAppConf.initType = INIT_WARM_SYNC_TO_HW;
                }
                if (!strncmp("INIT_WARM_SYNC_TO_SHADOW", optarg,
                             strlen("INIT_WARM_SYNC_TO_SHADOW")))
                {
                    xpAppConf.initType = INIT_WARM_SYNC_TO_SHADOW;
                }
                break;

            case 'g':
                /* Set device type for xpApp */
                if (!strncasecmp("ALDB2B", optarg, strlen("ALDB2B")))
                {
                    xpAppConf.devType = ALDB2B;
                }
                else if (!strncasecmp("ALD2", optarg, strlen("ALD2")))
                {
                    xpAppConf.devType = ALD2;
                }
                else if (!strncasecmp("ALD", optarg, strlen("ALD")))
                {
                    xpAppConf.devType = ALD;
                }
                else if (!strncasecmp("BC3", optarg, strlen("BC3")))
                {
                    xpAppConf.devType = BC3;
                }
                else if (!strncasecmp("cygnus", optarg, strlen("cygnus")))
                {
                    xpAppConf.devType = CYGNUS;
                }
                else if (!strncasecmp("TG48M_P", optarg, strlen("TG48M_P")))
                {
                    xpAppConf.devType = TG48M_P;
                }
                if (!strncasecmp("ALDRIN2XL", optarg, strlen("ALDRIN2XL")))
                {
                    xpAppConf.devType = ALDRIN2XL;
                }
                else if (!strncasecmp("FALCON64x100GR4", optarg, strlen("FALCON64x100GR4")))
                {
                    xpAppConf.devType = FALCON64x100GR4;
                }
                else if (!strncasecmp("FALCON64x100G", optarg, strlen("FALCON64x100G")))
                {
                    xpAppConf.devType = FALCON64x100G;
                }
                else if (!strncasecmp("FALCON64x25G", optarg, strlen("FALCON64x25G")))
                {
                    xpAppConf.devType = FALCON64x25G;
                }
                else if (!strncasecmp("FALCON32x25G64", optarg, strlen("FALCON32x25G64")))
                {
                    xpAppConf.devType = FALCON32x25G64;
                }
                else if (!strncasecmp("FALCON32x400G", optarg, strlen("FALCON32x400G")))
                {
                    xpAppConf.devType = FALCON32x400G;
                }
                else if (!strncasecmp("FALCONEBOF", optarg, strlen("FALCONEBOF")))
                {
                    xpAppConf.devType = FALCONEBOF;
                }
                else if (!strncasecmp("FC24x25G8x200G", optarg, strlen("FC24x25G8x200G")))
                {
                    xpAppConf.devType = FC24x25G8x200G;
                }
                else if (!strncasecmp("FC24x25G8x100G", optarg, strlen("FC24x25G8x100G")))
                {
                    xpAppConf.devType = FC24x25G8x100G;
                }
                else if (!strncasecmp("FC24x254x200G64", optarg, strlen("FC24x254x200G64")))
                {
                    xpAppConf.devType = FC24x254x200G64;
                }
                else if (!strncasecmp("FC24x258x100G64", optarg, strlen("FC24x258x100G64")))
                {
                    xpAppConf.devType = FC24x258x100G64;
                }
                else if (!strncasecmp("FC24x100G4x400G", optarg, strlen("FC24x100G4x400G")))
                {
                    xpAppConf.devType = FC24x100G4x400G;
                }
                else if (!strncasecmp("FC16x100G8x400G", optarg, strlen("FC16x100G8x400G")))
                {
                    xpAppConf.devType = FC16x100G8x400G;
                }
                else if (!strncasecmp("FC32x100G8x400G", optarg, strlen("FC32x100G8x400G")))
                {
                    xpAppConf.devType = FC32x100G8x400G;
                }
                else if (!strncasecmp("FC32x10016x400G", optarg, strlen("FC32x10016x400G")))
                {
                    xpAppConf.devType = FC32x10016x400G;
                }
                else if (!strncasecmp("FC64x100GR4IXIA", optarg, strlen("FC64x100GR4IXIA")))
                {
                    xpAppConf.devType = FC64x100GR4IXIA;
                }
                else if (!strncasecmp("FC32x100GR4IXIA", optarg, strlen("FC32x100GR4IXIA")))
                {
                    xpAppConf.devType = FC32x100GR4IXIA;
                }
                else if (!strncasecmp("FC48x10G8x100G", optarg, strlen("FC48x10G8x100G")))
                {
                    xpAppConf.devType = FC48x10G8x100G;
                }
                else if (!strncasecmp("FC48x25G8x100G", optarg, strlen("FC48x25G8x100G")))
                {
                    xpAppConf.devType = FC48x25G8x100G;
                }
                else if (!strncasecmp("FC32x25GIXIA", optarg, strlen("FC32x25GIXIA")))
                {
                    xpAppConf.devType = FC32x25GIXIA;
                }
                else if (!strncasecmp("FALCON32X25G", optarg, strlen("FALCON32X25G")))
                {
                    xpAppConf.devType = FALCON32X25G;
                }
                else if (!strncasecmp("FALCON32", optarg, strlen("FALCON32")))
                {
                    xpAppConf.devType = FALCON32;
                }
                else if (!strncasecmp("FALCON16x400G", optarg, strlen("FALCON16x400G")))
                {
                    xpAppConf.devType = FALCON16x400G;
                }
                else if (!strncasecmp("FC16x25GIXIA", optarg, strlen("FC16x25GIXIA")))
                {
                    xpAppConf.devType = FC16x25GIXIA;
                }
                else if (!strncasecmp("FALCON16X25G", optarg, strlen("FALCON16X25G")))
                {
                    xpAppConf.devType = FALCON16X25G;
                }
                else if (!strncasecmp("FC48x100G8x400G", optarg, strlen("FC48x100G8x400G")))
                {
                    xpAppConf.devType = FC48x100G8x400G;
                }
                else if (!strncasecmp("FALCON128x10G", optarg, strlen("FALCON128x10G")))
                {
                    xpAppConf.devType = FALCON128x10G;
                }
                else if (!strncasecmp("FALCON128x25G", optarg, strlen("FALCON128x25G")))
                {
                    xpAppConf.devType = FALCON128x25G;
                }
                else if (!strncasecmp("FALCON128x50G", optarg, strlen("FALCON128x50G")))
                {
                    xpAppConf.devType = FALCON128x50G;
                }
                else if (!strncasecmp("FALCON128x100G", optarg, strlen("FALCON128x100G")))
                {
                    xpAppConf.devType = FALCON128x100G;
                }
                else if (!strncasecmp("FALCON128", optarg, strlen("FALCON128")))
                {
                    xpAppConf.devType = FALCON128;
                }
                else if (!strncasecmp("FALCON128B2B", optarg, strlen("FALCON128B2B")))
                {
                    xpAppConf.devType = FALCON128B2B;
                }
                else if (!strncasecmp("FC96x100G8x400G", optarg, strlen("FC96x100G8x400G")))
                {
                    xpAppConf.devType = FC96x100G8x400G;
                }
                else if (!strncasecmp("FC64x25G64x10G", optarg, strlen("FC64x25G64x10G")))
                {
                    xpAppConf.devType = FC64x25G64x10G;
                }
                else if (!strncasecmp("FC32x25G8x100G", optarg, strlen("FC32x25G8x100G")))
                {
                    xpAppConf.devType = FC32x25G8x100G;
                }
                else if (!strncasecmp("F2T80x25G", optarg, strlen("F2T80x25G")))
                {
                    xpAppConf.devType = F2T80x25G;
                }
                else if (!strncasecmp("F2T80x25GIXIA", optarg, strlen("F2T80x25GIXIA")))
                {
                    xpAppConf.devType = F2T80x25GIXIA;
                }
                else if (!strncasecmp("F2T48x25G8x100G", optarg, strlen("F2T48x25G8x100G")))
                {
                    xpAppConf.devType = F2T48x25G8x100G;
                }
                else if (!strncasecmp("F2T48x10G8x100G", optarg, strlen("F2T48x10G8x100G")))
                {
                    xpAppConf.devType = F2T48x10G8x100G;
                }
                else if (!strncasecmp("AC5XRD", optarg, strlen("AC5XRD")))
                {
                    xpAppConf.devType = AC5XRD;
                }
                else if (!strncasecmp("AC5X48x1G6x10G", optarg, strlen("AC5X48x1G6x10G")))
                {
                    xpAppConf.devType = AC5X48x1G6x10G;
                }
                if (!strncasecmp("ALDRIN2XLFL", optarg, strlen("ALDRIN2XLFL")))
                {
                    xpAppConf.devType = ALDRIN2XLFL;
                }
                if (!strncasecmp("AC3XFS", optarg, strlen("AC3XFS")))
                {
                    xpAppConf.devType = AC3XFS;
                }

                break;

            case 'I':
                if (!strncmp("XP_DMA", optarg, sizeof("XP_DMA")))
                {
                    xpAppConf.packetInterface = XP_DMA;
                }
                else if (!strncmp("XP_ETHER", optarg, sizeof("XP_ETHER")))
                {
                    xpAppConf.packetInterface = XP_ETHER;
                }
                else if (!strncmp("XP_NETDEV_DMA", optarg, sizeof("XP_NETDEV_DMA")))
                {
                    xpAppConf.packetInterface = XP_NETDEV_DMA;
                }
                else
                {
                    printHelp();
                    exit(0);
                }
                break;

            case 'm':
                break;
            case 'l':
                {
                    struct stat file_stats;
                    if ((stat(optarg, &file_stats)) == -1)
                    {
                        PRINT_LOG("Given log-file path does Not exist\n\n");
                        exit(0);
                    }
                    strcpy(xpAppConf.logfile, optarg);
                    break;
                }
            case 'j' :
                {
                    xpAppConf.numScope = atoi(optarg);
                    break;
                }
            case 'o' :
                {
                    xpAppConf.portTuneStatus = 1;
                    break;
                }
            case 'O':
                {
                    xpAppConf.portInitAllowed = 0;
                    break;
                }
            case 'V' :
                {
                    xpAppConf.initialPortVifVal = atoi(optarg);
                    break;
                }
            case 'J' :
                {
                    xpAppConf.createdemoports = true;
                    break;
                }
            case 'W' :
                {

                    char  apPortStr[256] = {0};
                    //uint32_t apPortlist[256] = {0};
                    //int apportIter = 0;
                    uint32_t  a, p=0;
                    strcpy(apPortStr, optarg);
                    while (apPortStr[p]!='\0')
                    {

                        sscanf(apPortStr+p, "%u", &a);
                        //apPortlist[apportIter++] = a;
                        printf("p %x a 0%3x\n", p, a);
                        p += 4;
                        xpsMacSetPortAnEnable(0, a, 1);
                    }
                    break;
                }
            default:
                PRINT_LOG("Unrecognized command line option ( %d ).\n\n", opt);
                printHelp();
                exit(0);

                break;
        }
    }
}

void printSkuToArgMap()
{

    PRINT_LOG("      +----------------------------------------------------------------+\n");
    PRINT_LOG("      | DeviceType |          SKU            |     Argument value      |\n");
    PRINT_LOG("      +--------------------------------------+-------------------------|\n");
}
/**
 * @fn printHelp
 * @public
 * @brief Help function.
 *
 */
void printHelp()
{
    PRINT_LOG("%s  [OPTIONS] \n", XP_PROGNAME);
    PRINT_LOG("Where OPTIONS are:\n");
    PRINT_LOG(" -h | --help\n");
    PRINT_LOG(" -c | --config | --configfile CONFIGURATION_FILE\n");
    PRINT_LOG(" -n | --name of xpShell Configuration file\n");
    PRINT_LOG(" -P | --profile PROFILE_XML_FILE\n");
    PRINT_LOG(" -q | --pipeLine 1|2\n");
    PRINT_LOG(" -x | --pcap config file for control packets\n");
    PRINT_LOG(" -s | --Table Profile XP_DEFAULT_TWO_PIPE_PROFILE(default)(use values from enum XP_PROFILE_TYPE as string)\n");
    PRINT_LOG(" -a | --xpdiags, serdes Tx-Rx polarity swap will be skipped as in this mode ports will be used in internal loopback only, also application will not receive packets automatically. User needs to explicitly call packet driver APIs to receive trapped packets to CPU\n");
    PRINT_LOG(" -b | --Unit Number 0 (Default) \n");
    PRINT_LOG(" -d | --debug DEBUG_OPTIONS\n");
    PRINT_LOG(" -U |   Multiple xpShell CLI\n");
    PRINT_LOG(" -w |   Debug CLI comes at beginning of xpApp \n");
    PRINT_LOG(" -t | --Run with target instead of WM. e.g. -t svb \n");
    PRINT_LOG(" -z | --socket port on which xpApp will be listening for new whitemodel connections\n");
    PRINT_LOG(" -y | --socket port on which xpApp will be listening for interrupts from WM\n");
    PRINT_LOG(" -i | --Device init type. Default value is INIT_COLD\n");
    PRINT_LOG(" -m | --Set mode of speed for all ports\n");
    printSkuToArgMap();
    PRINT_LOG("      By default mode is selected as per the PCIE device Id\n");
    PRINT_LOG(" -f | --Set xp80 core clock frequency value. 400 (400MHz) | 500 (500MHz-Default) | 550 (550MHz)\n");
    PRINT_LOG(" -I | --Set packet driver interface. XP_DMA(Default) | XP_ETHER | XP_NETDEV_DMA\n");
    PRINT_LOG(" -r | -- Bringup the chip DataPath Config for performance Mode - 0:128U (default), 1:64U, 2:192U\n");
    PRINT_LOG(" -k | -- Do not send config-done to white-model\n");
    PRINT_LOG(" -l | --file in which logs are appended\n");
    PRINT_LOG(" -v | -- Dump the shadow in output log   XP_NO_PRINT(dafault) | XP_REG_ONLY | XP_TABLE_ONLY | XP_REG_AND_TABLE\n\n");
    PRINT_LOG(" -e | --persistent store\n");
    PRINT_LOG(" -g | --devType XP80A0, XP80B0, XP70 | valid only for WM platform \n");
    PRINT_LOG(" -j | --scope <number of scope> Scope0, Scope1\n");
    PRINT_LOG(" -B | --bypassSerdes\n");
    PRINT_LOG(" -W | --initTimeBypass - To optimize init time by skipping some aspects during initialization cycle\n");
    PRINT_LOG(" -D | --run as daemon\n");
    PRINT_LOG(" -o | --Enable port auto-tuning feature for all ports | valid only for HW platform\n");
    PRINT_LOG(" -S | --Packet Buffer Size (DMA Configuration)\n");
    PRINT_LOG(" -T | --Tx Descriptor Depth (DMA Configuration)\n");
    PRINT_LOG(" -Q | --Tx Queue Count (DMA Configuration)\n");
    PRINT_LOG(" -R | --Rx Descriptor Depth (DMA Configuration)\n");
    PRINT_LOG(" -E | --Rx Queue Count (DMA Configuration)\n");
    PRINT_LOG(" -C | --Completion Boundary (DMA Configuration) - Value zero means to use the default one\n");
    PRINT_LOG(" -A | --With Billing counters for ACM policing\n");
    PRINT_LOG(" -O | --Disabling link port initialization and mac volatile register. It helps fast xdk initialization when init with WM. This option is for WM\n");
    PRINT_LOG(" -F | --enablePrefetch Enable Prefetch Mode | valid only for HW platform XP70\n");
    PRINT_LOG(" -L | --enableDescRing Enable Descriptor ring | valid only for HW platform XP70\n");
    PRINT_LOG(" -Y | --fetchRetryCount Max retries for desc fetch | valid only for HW platform XP70\n");
    PRINT_LOG(" -N | --numOfPktsRecv no of Pkts rx thread to receive\n");
    PRINT_LOG(" -X | --Create and Start Running Logging Thread to read SCPU IPC Msgs [To redirect IPC messages to file: export scpuLogPath=<PATH> ]\n");
    PRINT_LOG(" -Z | --Enable l2 packet learning thread\n");
    PRINT_LOG(" -J | --Create interfaces for demo use, e.g. in switchdev development\n");
}
