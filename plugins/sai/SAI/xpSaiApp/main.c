// main.cpp

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <stdlib.h>
#include "xpSaiApp.h"
#include "xpSaiAppSaiUtils.h"
#include "xpSaiUtil.h"
#include "xpAppUtil.h"
#include "xpPyInc.h"
#include "xpsMac.h"
#include "xpsSerdes.h"
#include "xpSaiDev.h"
#include "xpSai.h"

#ifndef MAC
#endif

XP_SAI_LOG_REGISTER_API(SAI_API_UNSPECIFIED);

#define SAIAPP_PROFILE_ID 0
#define SAI_NPU_WM "wm"
#define SAI_NPU_HW "_npu0"
#define XP_PLATFORM_CFG_FOLDER "platform"  ///< Folder with config files
#define XP_SCPU_FIRMWARE_PATH   "/home/xp/xp-tools/"     ///< Directory where firmware bins present
#define XP_SCPU_FIRMWARE_FILE_XP80A0    "xp80Firmware.bin"
#define XP_SCPU_FIRMWARE_FILE_XP80B0    "xp80FirmwareB0.bin"
#define XP_SCPU_FIRMWARE_FILE_XP70A0    "xp70FirmwareA0.bin"
#define XP_SCPU_FIRMWARE_FILE_XP70A1    "xp70FirmwareA1.bin"
#define XP_Z80_FIRMWARE_FILE_XP70A1     "xp70Tv80AutoNegStateMachine.ihx.mem"

sai_switch_api_t* xpSaiSwitchApi;
sai_vlan_api_t* xpSaiVlanApi;
sai_bridge_api_t* xpSaiBridgeApi;
sai_mirror_api_t* xpSaiMirrorApi;
sai_stp_api_t* xpSaiStpApi;
sai_lag_api_t* xpSaiLagApi;
sai_router_interface_api_t* xpSaiRouterInterfaceApi;
sai_route_api_t* xpSaiRouteApi;
sai_next_hop_api_t* xpSaiNextHopApi;
sai_fdb_api_t* xpSaiFdbApi;
sai_acl_api_t* xpSaiAclApi;
sai_virtual_router_api_t* xpSaiVrfApi;
sai_neighbor_api_t* xpSaiNeighborApi;
sai_next_hop_api_t* xpSaiNexthopApi;
sai_next_hop_group_api_t* xpSaiNextHopGroupApi;
sai_port_api_t* xpSaiPortApi;
sai_queue_api_t* xpSaiQueueApi;
sai_scheduler_api_t* xpSaiSchedulerApi;
sai_wred_api_t* xpSaiWredApi;
sai_qos_map_api_t* xpSaiQosMapApi;
sai_policer_api_t* xpSaiPolicerApi;
sai_hostif_api_t* xpSaiHostInterfaceApi;
static sai_object_id_t swObjId = 0;

static int fd_stdin = 0;
static int fd_stdout = 0;
static int fd_stderr = 0;

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ACL_DB       5
uint8_t acl_db[MAX_ACL_DB] = {1, 1, 1, 1, 1};
void xpSaiAppDaemonize(int* fdw, int* fdr, int* fdrw)
{
    int result = 0;

    PRINT_LOG("Calling %s\n", __FUNCTION__);

    if (fdr == NULL || fdw == NULL || fdrw == NULL)
    {
        printf("Error:: XP_ERR_NULL_POINTER\n");
        return;
    }

    //Make multiuser xpShell to work with a daemons
    if (xpAppConf.daemonMode != 0)
    {
        pid_t pid, sid;
        const char* env = getenv("XP_ROOT");

        if (env == NULL)
        {
            env = getenv(XP_SAI_CONFIG_PATH);
        }

        //Fork the Parent Process
        pid = fork();
        if (pid < 0)
        {
            exit(EXIT_FAILURE);
        }
        //Close the Parent Process
        if (pid > 0)
        {
            exit(EXIT_SUCCESS);
        }
        umask(0);
        //Create a new Signature Id for our child
        //a daemon should not receive signals from a terminal
        sid = setsid();
        if (sid < 0)
        {
            exit(EXIT_FAILURE);
        }

        if (env != NULL)
        {
            result = chdir(env);
            if (result == -1)
            {
                PRINT_LOG("Error calling chdir: errno %s. Possible wrong "
                          "XP_ROOT variable which set to %s.\n",
                          strerror(errno), env);
            }
        }

        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        // Establish new open descriptors for stdin, stdout and stder
        // to avoid error if app has code that relies on them
        fd_stdin = open("/dev/null", O_RDONLY);
        if (fd_stdin == -1)
        {
            exit(EXIT_FAILURE);
        }
        fd_stdout = open("/dev/null", O_WRONLY);
        if (fd_stdout == -1)
        {
            exit(EXIT_FAILURE);
        }
        fd_stderr = open("/dev/null", O_RDWR);
        if (fd_stderr == -1)
        {
            exit(EXIT_FAILURE);
        }
    }
}

XP_STATUS xpSaiAppPopulateData(xpDevice_t devId, void* arg)
{
    xpSaiAppPopulateTables(devId, arg);
    xpAppSchedPolicyInit(devId);

    if (arg)
    {
        xpSaiWmIpcParam_t *param = (xpSaiWmIpcParam_t *)arg;
        swObjId = param->xpSaiSwitchObjId;
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiAppSendPackets(xpDevice_t devId)
{
    XP_STATUS status = XP_NO_ERR;

    if ((!XP_CONFIG.userCli) && (XP_CONFIG.noConfig))
    {
        /* Normal process termination and the value of status
           & 0377 is returned to the parent */
        exit(3);
    }

    /* Send packet to XP if requested by user */
    if (0 != strcmp(XP_CONFIG.pcapConfigFile, ""))
    {
        if ((status = xpAppSendPacketToXp(devId, swObjId)) != XP_NO_ERR)
        {
            printf("xpAppSendPacketToXp failed. RC: %u\n", status);
            return XP_ERR_INIT_FAILED;
        }
    }

    return status;
}

// User suppose to implement his own action for mac learning/aging events
void sai_fdb_event_cb(uint32_t count, sai_fdb_event_notification_data_t *data)
{
    XP_SAI_LOG_NOTICE("############# SAI USER SPECIFIC CALL: FDB event ###################\n");
    if (data->event_type == SAI_FDB_EVENT_AGED)
    {
        XP_SAI_LOG_NOTICE("############# SAI USER SPECIFIC CALL: FDB Aging Event ###################\n");
    }
    else if (data->event_type == SAI_FDB_EVENT_LEARNED)
    {
        XP_SAI_LOG_NOTICE("############# SAI USER SPECIFIC CALL: FDB Learning Event ###################\n");
    }
}

// User suppose to implement his own action for the link up/down events
void sai_port_state_cb(uint32_t count,
                       sai_port_oper_status_notification_t *data)
{
    printf("\nSAI LINK EVENT: Port:[%" PRIu64 "] %u State:[%d] %s\n",
           (uint64_t)data->port_id, (uint32_t)data->port_id, data->port_state,
           (data->port_state==SAI_PORT_OPER_STATUS_UP)?"UP":"DOWN");
}

static void sai_acl_db_file_path(char *filePath)
{
    char *xpRootPath = getenv("XP_ROOT");

    if (xpRootPath == NULL)
    {
        xpRootPath = getenv(XP_SAI_CONFIG_PATH);
    }

    if ((xpRootPath != NULL) && (strlen(xpRootPath) > 1))
    {
        if (xpRootPath[strlen(xpRootPath) - 1] != '/')
        {
            sprintf(filePath, "%s%s", xpRootPath,
                    "testSuites/pipeline-profiles/xpDefault/apptest/");
        }
        else
        {
            sprintf(filePath, "%s%s", xpRootPath,
                    "testSuites/pipeline-profiles/xpDefault/apptest/");
        }

    }
    else
    {
        //Assuming customer is running from xdk path
        sprintf(filePath, "%s", "./testSuites/pipeline-profiles/xpDefault/apptest/");
    }
}

void sai_read_acl_db_alloc(void)
{
    char    line[XP_MAX_CONFIG_LINE_LEN];
    char    *aclToken = NULL;
    uint8_t index  = 0;
    char    filePath[512];
    FILE*   aclConfigFile = NULL;

    sai_acl_db_file_path(filePath);
    if (strlen(filePath) > (512 - strlen("acl_db_alloc.cfg")))
    {
        printf("Length of filepath out of bound\n");
        return;
    }

    snprintf(filePath+strlen(filePath), sizeof(filePath)-strlen(filePath), "%s",
             "acl_db_alloc.cfg");
    aclConfigFile = fopen(filePath, "r");

    if (aclConfigFile == NULL)
    {
        printf("Unable to open configuration file \" %s \". ", "acl_db_alloc.cfg");
        return;
    }
    else
    {
        while (!feof(aclConfigFile))  // While not at the end of the file:
        {
            if (fgets(line, XP_MAX_CONFIG_LINE_LEN, aclConfigFile) != NULL)
            {
                aclToken = strtok(line, " ,");
                if (aclToken != NULL)
                {
                    index = atoi(aclToken);
                }
                if (index >= MAX_ACL_DB)
                {
                    printf("ACL DB index out of range. Index = %d\n", index);
                }
                aclToken = strtok(NULL, " ,");
                if (aclToken != NULL)
                {
                    acl_db[index] = atoi(aclToken);
                }

                while (aclToken != NULL)
                {
                    aclToken = strtok(NULL, " ,");
                    if (aclToken != NULL)
                    {
                        index = atoi(aclToken);
                    }
                    if (index >= MAX_ACL_DB)
                    {
                        printf("ACL DB index out of range. Index = %d\n", index);
                    }
                    aclToken = strtok(NULL, " ,");
                    if (aclToken != NULL)
                    {
                        acl_db[index] = atoi(aclToken);
                    }

                }
            }
        }
        fclose(aclConfigFile);
    }
}

int appMain(int argc, char *argv[])
{
    uint8_t         logModIndex = 0;
    xpsDevice_t     devId       = 0;
    const char**    moduleNames = NULL;
    int fdStdIn=-1, fdStdout=-1, fdStdErr =-1;
    sai_status_t      saiStatus   = SAI_STATUS_SUCCESS;

    if (strlen(argv[0]) >= sizeof(XP_PROGNAME))
    {
        printf("Invalid program name input: %s. Length cannot be more than %zu\n",
               argv[0], sizeof(XP_PROGNAME));
        return XP_ERR_INVALID_INPUT;
    }
    memcpy(XP_PROGNAME, argv[0],
           strlen(argv[0]));                 // Save name of executable.
    /* load additional configuration */
    getEnvValues();                               // Process environment variables first,
    getOptValues(argc, argv,
                 moduleNames);        // then process command line options => parsing for both client and wm

    //check daemon mode
    xpSaiAppDaemonize(&fdStdIn, &fdStdout, &fdStdErr);

    initConfigEntryList();                        // initialize entry data structure of configFile
    readConfigFile(XP_CONFIG.configFileName);     // then read configuration file.

    // Set logger value of different modules
    while (strcmp(xpAppConf.dbgLogInfo[logModIndex].ModName, "") != 0)
    {
        xpsSdkSetLoggerOptions(xpAppConf.dbgLogInfo[logModIndex].xpLogModuleId,
                               xpAppConf.dbgLogInfo[logModIndex].ModName);
        logModIndex++;
    }

    // connect logger to console
    xpsSdkLogConsole();

    // global sdk initialization : 1st thing to do
    //TODO: Assuming that in case of the use Sai we will call this
    //init from the sai wrapper
    sai_service_method_table_t* xpSaiAppServiceMethodTable =
        (sai_service_method_table_t*)
        malloc(sizeof(sai_service_method_table_t));
    if (xpSaiAppServiceMethodTable == NULL)
    {
        printf("Failed to allocate memory for sai service mothod table.");
        return XP_ERR_NULL_POINTER;
    }

    if (!XP_CONFIG.withHw)
    {
        xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "interruptPort",
                                         XP_CONFIG.interruptPort);
        xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "devType",
                                         XP_CONFIG.devType);
    }

    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "initType",
                                     XP_CONFIG.initType);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "performanceMode",
                                     XP_CONFIG.performanceMode);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "mode", XP_CONFIG.mode);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "diag", XP_CONFIG.diag);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "userCli",
                                     XP_CONFIG.userCli);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "defaultVlan",
                                     XP_SAI_APP_DEFAULT_VLAN);
    xpSaiAppSaiProfileKeyValueAdd(SAIAPP_PROFILE_ID, "hwCfgPath",
                                  (char *)XP_PLATFORM_CFG_FOLDER);

    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "pktBufSize",
                                     XP_CONFIG.pktBufSize);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "txDescDepth",
                                     XP_CONFIG.txDescDepth);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "numOfTxQueue",
                                     XP_CONFIG.numOfTxQueue);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "rxDescDepth",
                                     XP_CONFIG.rxDescDepth);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "numOfRxQueue",
                                     XP_CONFIG.numOfRxQueue);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "dmaCompletionBoundary",
                                     XP_CONFIG.completionBoundary);

    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "SAI_WARM_BOOT_READ_FILE",
                                     0);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "SAI_WARM_BOOT_WRITE_FILE",
                                     0);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "SAI_NUM_UNICAST_QUEUES",
                                     16);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "SAI_NUM_MULTICAST_QUEUES",
                                     8);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "SAI_NUM_QUEUES", 24);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "SAI_NUM_CPU_QUEUES", 128);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, SAI_KEY_NUM_ECMP_MEMBERS,
                                     32);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, SAI_KEY_NUM_ECMP_GROUPS,
                                     96);
    xpSaiAppSaiProfileKeyValueAdd(SAIAPP_PROFILE_ID, "SAI_ACL_TYPE_READ_FILE",
                                  (char *)"saiAcl.cfg");
    sai_read_acl_db_alloc();
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "SAI_ING_ACL1_NUM_BLOCKS",
                                     acl_db[0]);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "SAI_ING_ACL2_NUM_BLOCKS",
                                     acl_db[1]);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "SAI_ING_ACL3_NUM_BLOCKS",
                                     acl_db[2]);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "SAI_ING_ACL4_NUM_BLOCKS",
                                     acl_db[3]);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "SAI_ING_ACL5_NUM_BLOCKS",
                                     acl_db[4]);
    xpSaiAppSaiProfileKeyValueAdd(SAIAPP_PROFILE_ID, "SAI_XP_IBUF_CONFIG_READ_FILE",
                                  (char *)"saiIbuf.cfg");
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "rxConfigMode",
                                     XP_CONFIG.rxConfigMode);
    if (XP_CONFIG.initType == INIT_WARM)
    {
        xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "SAI_WARM_BOOT", 1);
    }
    else if (XP_CONFIG.initType == INIT_COLD)
    {
        xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "SAI_WARM_BOOT", 0);
    }
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "logDest",
                                     XP_SAI_LOG_CONSOLE);
    xpSaiAppSaiProfileKeyValueAdd(SAIAPP_PROFILE_ID, "l3_counter_index_base",
                                  (char *)"0");

    xpSaiAppSaiProfileKeyValueAdd(SAIAPP_PROFILE_ID, "lpm_fdb_em_profile",
                                  (char *)"MID_L3_MID_L2_NO_EM");
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "max_routes_ipv4",
                                     (uint32_t) 1024);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "max_routes_ipv6",
                                     (uint32_t) 256);
    xpSaiAppSaiProfileKeyIntValueAdd(SAIAPP_PROFILE_ID, "pbrMaxEntries",
                                     (uint32_t) 512);
    xpSaiAppSaiProfileKeyValueAdd(SAIAPP_PROFILE_ID, "portConfig",
                                  (char *)"portsProfile.cfg");

    xpSaiAppServiceMethodTable->profile_get_value      = xpSaiAppSaiProfileGetValue;
    xpSaiAppServiceMethodTable->profile_get_next_value =
        xpSaiAppSaiProfileGetNextValue;

    //TODO: decide the flags
    sai_api_initialize(0 /*flags*/, xpSaiAppServiceMethodTable);

    sai_api_query(SAI_API_SWITCH, (void**) &xpSaiSwitchApi);
    sai_api_query(SAI_API_VLAN, (void**) &xpSaiVlanApi);
    sai_api_query(SAI_API_BRIDGE, (void**) &xpSaiBridgeApi);
    sai_api_query(SAI_API_STP, (void**) &xpSaiStpApi);
    sai_api_query(SAI_API_LAG, (void**) &xpSaiLagApi);
    sai_api_query(SAI_API_ROUTER_INTERFACE, (void**) &xpSaiRouterInterfaceApi);
    sai_api_query(SAI_API_ROUTE, (void**) &xpSaiRouteApi);
    sai_api_query(SAI_API_NEXT_HOP, (void**) &xpSaiNextHopApi);
    sai_api_query(SAI_API_ACL, (void**) &xpSaiAclApi);
    sai_api_query(SAI_API_FDB, (void**) &xpSaiFdbApi);
    sai_api_query(SAI_API_VIRTUAL_ROUTER, (void**) &xpSaiVrfApi);
    sai_api_query(SAI_API_NEIGHBOR, (void**) &xpSaiNeighborApi);
    sai_api_query(SAI_API_NEXT_HOP, (void**) &xpSaiNexthopApi);
    sai_api_query(SAI_API_NEXT_HOP_GROUP, (void**) &xpSaiNextHopGroupApi);
    sai_api_query(SAI_API_PORT, (void**) &xpSaiPortApi);
    sai_api_query(SAI_API_QUEUE, (void**) &xpSaiQueueApi);
    sai_api_query(SAI_API_SCHEDULER, (void**) &xpSaiSchedulerApi);
    sai_api_query(SAI_API_WRED, (void**) &xpSaiWredApi);
    sai_api_query(SAI_API_QOS_MAP, (void**) &xpSaiQosMapApi);
    sai_api_query(SAI_API_POLICER, (void**) &xpSaiPolicerApi);
    sai_api_query(SAI_API_MIRROR, (void**) &xpSaiMirrorApi);

    uint8_t count = 0, tmpCount = 0;
    sai_attribute_t switchAttr[6];
    sai_object_id_t switch_id;

    switchAttr[count].id = SAI_SWITCH_ATTR_INIT_SWITCH;
    switchAttr[count].value.booldata = true; /* this value has to be controlled */

    switchAttr[++count].id = SAI_SWITCH_ATTR_SWITCH_PROFILE_ID;
    switchAttr[count].value.u32 = SAIAPP_PROFILE_ID;

    switchAttr[++count].id = SAI_SWITCH_ATTR_SWITCH_HARDWARE_INFO;
    tmpCount = count;

    switch (XP_CONFIG.devType)
    {
        case ALD:
            XP_CONFIG.withHw = "wm";
            break;
        case ALDB2B:
            XP_CONFIG.withHw = "et6448m";
            break;
        case TG48M_P:
            XP_CONFIG.withHw = "TG48MP";
            break;
        case FC32x10016x400G:
            XP_CONFIG.withHw = "FC32x10016x400G";
            break;
        case FC64x100GR4IXIA:
            XP_CONFIG.withHw = "FC64x100GR4IXIA";
            break;
        case FC32x100GR4IXIA:
            XP_CONFIG.withHw = "FC32x100GR4IXIA";
            break;
        case FALCON32:
            XP_CONFIG.withHw = "FALCON32";
            break;
        case FALCON32x400G:
            XP_CONFIG.withHw = "FALCON32x400G";
            break;
        case FALCONEBOF:
            XP_CONFIG.withHw = "FALCONEBOF";
            break;
        case FC24x258x100G64:
            XP_CONFIG.withHw = "FC24x258x100G64";
            break;
        case FC24x25G8x200G:
            XP_CONFIG.withHw = "FC24x25G8x200G";
            break;
        case FC24x25G8x100G:
            XP_CONFIG.withHw = "FC24x25G8x100G";
            break;
        case FC24x254x200G64:
            XP_CONFIG.withHw = "FC24x254x200G64";
            break;
        case FC24x100G4x400G:
            XP_CONFIG.withHw = "FC24x100G4x400G";
            break;
        case FC16x100G8x400G:
            XP_CONFIG.withHw = "FC16x100G8x400G";
            break;
        case FALCON64x100G:
            XP_CONFIG.withHw = "FALCON64x100G";
            break;
        case FALCON64x100GR4:
            XP_CONFIG.withHw = "FALCON64x100GR4";
            break;
        case FC32x100G8x400G:
            XP_CONFIG.withHw = "FC32x100G8x400G";
            break;
        case FC48x100G8x400G:
            XP_CONFIG.withHw = "FC48x100G8x400G";
            break;
        case FC96x100G8x400G:
            XP_CONFIG.withHw = "FC96x100G8x400G";
            break;
        case FALCON128x10G:
            XP_CONFIG.withHw = "FALCON128x10G";
            break;
        case FALCON128x25G:
            XP_CONFIG.withHw = "FALCON128x25G";
            break;
        case FALCON128x50G:
            XP_CONFIG.withHw = "FALCON128x50G";
            break;
        case FALCON128x100G:
            XP_CONFIG.withHw = "FALCON128x100G";
            break;
        case FALCON128:
            XP_CONFIG.withHw = "FALCON128";
            break;
        case FALCON32x25G64:
            XP_CONFIG.withHw = "FALCON32x25G64";
            break;
        case FALCON32X25G:
            XP_CONFIG.withHw = "FALCON32X25G";
            break;
        case FALCON64x25G:
            XP_CONFIG.withHw = "FALCON64x25G";
            break;
        case FALCON16x400G:
            XP_CONFIG.withHw = "FALCON16x400G";
            break;
        case FALCON16X25G:
            XP_CONFIG.withHw = "FALCON16X25G";
            break;
        case FC32x25GIXIA:
            XP_CONFIG.withHw = "FC32x25GIXIA";
            break;
        case FC16x25GIXIA:
            XP_CONFIG.withHw = "FC16x25GIXIA";
            break;
        case FC48x10G8x100G:
            XP_CONFIG.withHw = "FC48x10G8x100G";
            break;
        case FC48x25G8x100G:
            XP_CONFIG.withHw = "FC48x25G8x100G";
            break;
        case FC64x25G64x10G:
            XP_CONFIG.withHw = "FC64x25G64x10G";
            break;
        case FC32x25G8x100G:
            XP_CONFIG.withHw = "FC32x25G8x100G";
            break;
        case F2T80x25G:
            XP_CONFIG.withHw = "F2T80x25G";
            break;
        case F2T80x25GIXIA:
            XP_CONFIG.withHw = "F2T80x25GIXIA";
            break;
        case F2T48x10G8x100G:
            XP_CONFIG.withHw = "F2T48x10G8x100G";
            break;
        case F2T48x25G8x100G:
            XP_CONFIG.withHw = "F2T48x25G8x100G";
            break;
        case CYGNUS:
            XP_CONFIG.withHw = "CYGNUS";
            break;
        case ALDRIN2XL:
            XP_CONFIG.withHw = "ALDRIN2XL";
            break;
        case AC5XRD:
            XP_CONFIG.withHw = "AC5XRD";
            break;
        case AC5X48x1G6x10G:
            XP_CONFIG.withHw = "AC5X48x1G6x10G";
            break;
        default:
            XP_CONFIG.withHw = "et6448m";
    }
    switchAttr[count].value.s8list.list = (sai_int8_t*)malloc(sizeof(sai_int8_t) *
                                                              (strlen(XP_CONFIG.withHw)+1));
    memset(switchAttr[count].value.s8list.list, 0, strlen(XP_CONFIG.withHw) + 1);
    strcpy((char*)switchAttr[count].value.s8list.list, XP_CONFIG.withHw);
    printf("hardware platform string is : %s\n",
           switchAttr[count].value.s8list.list);

    switchAttr[++count].id = SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY;
    switchAttr[count].value.ptr = (sai_pointer_t) sai_port_state_cb;

    switchAttr[++count].id = SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY;
    switchAttr[count].value.ptr = (sai_pointer_t) sai_fdb_event_cb;

    /*
     * SAI_SWITCH_ATTR_FDB_AGING_TIME attribute is not mandatory in create_switch; Optional for user application.
     * 1) Set Attribute value: non-zero; Aging is Enable
     * 2) Set Attribute value: zero; Aging is Disable
     * 3) Not-used SAI_SWITCH_ATTR_FDB_AGING_TIME attribute in application; Aging is Disable by default
    */

    switchAttr[++count].id = SAI_SWITCH_ATTR_FDB_AGING_TIME;
    switchAttr[count].value.u32 = 0; // FDB AGING value in seconds

    if (xpSaiSwitchApi->create_switch(&switch_id, count + 1,
                                      switchAttr) != SAI_STATUS_SUCCESS)
    {
        printf("SAI adapted for device ID %d has not been initialized in proper manner.\n",
               devId);
        if (fdStdIn != -1)
        {
            close(fdStdIn);
        }
        if (fdStdout != -1)
        {
            close(fdStdout);
        }
        if (fdStdErr != -1)
        {
            close(fdStdErr);
        }
        free(switchAttr[tmpCount].value.s8list.list);
        return 1;
    }
#if 0 //SAI WARMBOOT OID retrieval debug code
    if (xpAppConf.initType == INIT_WARM)
    {
        xpSaiAppPrintObjectAttributes();
    }
#endif

    if (XP_CONFIG.withHw)
    {
        if (xpAppConf.createdemoports)
        {
            sai_api_query(SAI_API_HOSTIF, (void**) &xpSaiHostInterfaceApi);
            sai_attribute_t attr, portList, hifAttr[3];
            sai_object_id_t hifId = 0, portId = 0;
            uint16_t totalPortsNum = 0;
            char intfName[SAI_HOSTIF_NAME_SIZE] = {0};
            uint8_t i = 0;

            // Get number of ports on the switch
            attr.id = SAI_SWITCH_ATTR_PORT_NUMBER;
            attr.value.u32 = 0;
            saiStatus = xpSaiSwitchApi->get_switch_attribute(switch_id, 1, &attr);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                printf("ERROR: Fail to call xpSaiSwitchApi->get_switch_attribute: %d\n",
                       saiStatus);
                return saiStatus;
            }
            totalPortsNum = attr.value.u32;

            // Get the port list
            portList.id = SAI_SWITCH_ATTR_PORT_LIST;
            portList.value.objlist.count = totalPortsNum;
            portList.value.objlist.list = (sai_object_id_t*)malloc(sizeof(
                                                                       sai_object_id_t)*totalPortsNum);
            if (portList.value.objlist.list == NULL)
            {
                printf("ERROR: Failed to allocate memory to get total inited ports\n");
                return XP_ERR_MEM_ALLOC_ERROR;
            }
            memset(portList.value.objlist.list, 0, sizeof(sai_object_id_t)*totalPortsNum);

            saiStatus = xpSaiSwitchApi->get_switch_attribute(switch_id, 1, &portList);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                printf("ERROR: Fail to call xpSaiSwitchApi->get_switch_attribute: %d\n",
                       saiStatus);
                free(portList.value.objlist.list);
                return saiStatus;
            }

            // Create netdev interfaces based on SKU
            for (i = 0; i < portList.value.objlist.count; i++)
            {
                portId = portList.value.objlist.list[i];
                uint32_t portNum = (uint32_t)xpSaiObjIdValueGet(portId);
                sprintf(intfName, "sw%dp%d", devId, portNum);  // Name of netdev

                count = 0;
                hifAttr[count].id = SAI_HOSTIF_ATTR_TYPE;
                hifAttr[count++].value.s32 = SAI_HOSTIF_TYPE_NETDEV;
                hifAttr[count].id = SAI_HOSTIF_ATTR_NAME;
                strncpy((char *)hifAttr[count++].value.chardata, intfName,
                        SAI_HOSTIF_NAME_SIZE);
                hifAttr[count].id = SAI_HOSTIF_ATTR_OBJ_ID;
                hifAttr[count++].value.oid = portId;

                saiStatus = xpSaiHostInterfaceApi->create_hostif(&hifId, switch_id, count,
                                                                 hifAttr);
                if (saiStatus != SAI_STATUS_SUCCESS)
                {
                    printf("ERROR: SAI Host interface is not created for port : %d\n", portNum);
                    free(portList.value.objlist.list);
                    return saiStatus;
                }
                memset(intfName, 0x0, sizeof(intfName));
                printf("SAI Host interface is created with intf id : %ld and with name: %s\n",
                       (long)hifId, hifAttr[1].value.chardata);
            }
            free(portList.value.objlist.list);
        }
        xpSaiAppPopulateData(devId, NULL);
        xpSaiAppSendPackets(devId);
    }

    if ((XP_CONFIG.userCli & XP_CLI_USER) && (!(XP_CONFIG.userCli & XP_CLI_CONFIG)))
    {
        xpPyInit();
        xpPyInvoke(0, XP_SHELL_APP, NULL);
    }

    if (XP_CONFIG.userCli & XP_CLI_MULTISHELL)
    {
        sai_attribute_t switchAttrShell;
        switchAttrShell.id = SAI_SWITCH_ATTR_SWITCH_SHELL_ENABLE;
        switchAttrShell.value.booldata = true;

        saiStatus = xpSaiSwitchApi->set_switch_attribute(switch_id, &switchAttrShell);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            printf("MULTISHELL ENABLE  for device ID %d is failed with saiStatus %d .\n",
                   devId, saiStatus);
            if (fdStdIn != -1)
            {
                close(fdStdIn);
            }
            if (fdStdout != -1)
            {
                close(fdStdout);
            }
            if (fdStdErr != -1)
            {
                close(fdStdErr);
            }
            free(switchAttr[tmpCount].value.s8list.list);
            return saiStatus;
        }
    }


    printf("SAI xpSaiAppInitDevice(devId = 0x%x) done.\n", devId);
    printf("xpSaiAppInit done.\n");

    /* Register handler to receive events from driver
     * This method will fall via SAL and for current case (i.e. linux)
     * it will register our PID with driver to send us(this process) events
     * and also setup a signal handler
     */

    while (1)
    {
        usleep(2000000);

        if (xpsGetExitStatus())
        {
            xpAppCleanup(devId);
            break;
        }
    }
    printf("Exit.\n");
    if (fdStdIn != -1)
    {
        close(fdStdIn);
    }
    if (fdStdout != -1)
    {
        close(fdStdout);
    }
    if (fdStdErr != -1)
    {
        close(fdStdErr);
    }
    free(switchAttr[tmpCount].value.s8list.list);
    return 0;
}

int main(int argc, char *argv[])
{
    return (appMain(argc, argv));
}

#ifdef __cplusplus
}
#endif

