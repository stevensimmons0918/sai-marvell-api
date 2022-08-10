// xpSaiSwitch.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiSwitch.h"
#include "xpPyInc.h"
#include "xpSaiAcl.h"
#include "xpSaiAclMapper.h"
#include "xpSaiVlan.h"
#include "xpSaiFdb.h"
#include "xpSaiDev.h"
//#include "xpSaiUtil.h"
#include "xpsAging.h"
#include "xpsFdb.h"
#include "xpsSerdes.h"
#include "xpSaiValidationArrays.h"
#include "cpssHalQos.h"
#include "xpsMac.h"
#include "xpsAcl.h"
#include "xpsVxlan.h"
#include "cpssHalUtil.h"


#define NPU_STR         "npu"
#define ATTR_LIST_LEN   64
#define XPSAI_DEFAULT_LINKSCAN_DELAY    1000    //ms

XP_SAI_LOG_REGISTER_API(SAI_API_SWITCH);

//This is added because we don't have switch DB.
uint32_t gEgressAclId;

typedef struct xpSaiInitParams_s
{
    uint32_t         profileId;                    /* profile ID */
    char             switchHwStr[64];              /* string with HW details */
    bool
    switchInit;                   /* init switch (true) or connect (false) */
    xpsDevice_t      xpSaiDevId;                   /* NPU number */
    uint32_t         set_attr_count;               /* number of SET attributes */
    sai_attribute_t  set_attr_list[ATTR_LIST_LEN]; /* list of SET attributes */
} xpSaiInitParams_t;

xpSaiInitStat gSaiInitStat[XP_MAX_DEVICES];
extern xpSaiFdbParams_t gSaiFdbParams;
extern int32_t ingress_action_list_arr[];
extern int32_t egress_action_list_arr[];
extern uint32_t ingress_action_list_count;
extern uint32_t egress_action_list_count;


static XP_STATUS xpSaiDevConfig(xpDevice_t devId, void* arg);
static void xpSaiSwitchUpdateDefaultMaxRouteNum(int, int32_t *, int32_t *);

static sai_switch_api_t* _xpSaiSwitchApi;
/* TBD: Need to have param allocated per device */
static xpSaiWmIpcParam_t xpSaiInitParam_g = { INIT_UNKNOWN, XP80B0, XPSAI_DEFAULT_VLAN_ID, 0, 64};
xpsDbHandle_t xpSaiSwitchStaticDataDbHandle =
    XPS_STATE_INVALID_DB_HANDLE;
sai_switch_notification_t switch_notifications_g[XP_MAX_DEVICES];
static bool switchInitialized = false;
static sai_mac_t preInitMacAddr;

/* synchronization of SAI threads */
bool     thInitDone    = false;
pthread_mutex_t thInitMutex   = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  thInitCondVar = PTHREAD_COND_INITIALIZER;

#define XPSAI_INBAND_MGMT_INTF_NAME "eth0"
#define XPSAI_INBAND_MGMT_PORT_MTU  9122
#define XPSAI_INBAND_MGMT_RATELIMIT 6000
#define XPSAI_PBR_MAX_SIZE          1024

static uint32_t inbandPortNum_g = 0xFFFF;

uint32_t xpSaiGetSwitchInbandMgmtInf()
{
    return inbandPortNum_g;
}

void xpSaiSwitchInitDone()
{
    pthread_mutex_lock(&thInitMutex);

    thInitDone = true;

    pthread_cond_broadcast(&thInitCondVar);

    pthread_mutex_unlock(&thInitMutex);

}

void xpSaiSwitchThreadsWaitForInit()
{
    pthread_mutex_lock(&thInitMutex);

    while (thInitDone == false)
    {
        pthread_cond_wait(&thInitCondVar, &thInitMutex);
    }

    pthread_mutex_unlock(&thInitMutex);

}

XP_DEV_TYPE_T xpSaiSwitchDevTypeGet()
{
    return xpSaiInitParam_g.devType;
}

uint32_t xpSaiPortCycleTimeGet()
{
    return xpSaiInitParam_g.portScanMs;
}

uint64_t xpSaiGetSwitOId()
{
    return xpSaiInitParam_g.xpSaiSwitchObjId;
}

sai_status_t xpSaiSetSwitchAttribute(_In_ sai_object_id_t switchId,
                                     _In_ const sai_attribute_t *attr);

sai_status_t xpSaiGetSwitchAttrPortNumber(uint32_t *portNum);

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \brief Pointer to WM IPC Handler thread Interface thread.
 *
 */
pthread_t saiWmDevIpcHandlerThread = (pthread_t)NULL;
extern void SetSharedProfileIdx(int id);
extern void SetPbrMaxNum(int num);
extern void SetRouteIpv4MaxNum(int num);
extern void SetRouteIpv6MaxNum(int num);
extern void SetL3CounterIdBase(int id, int pos);
extern int IsValidCounterIdBase(int pos, XP_DEV_TYPE_T  devType);
#ifdef __cplusplus
}
#endif


sai_status_t xpSaiAclGetStaticVariables(xpSaiAclEntry_t* ppSaiAclEntryVar)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    retVal = xpSaiAclStaticVariablesGet(ppSaiAclEntryVar);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_ERR("Failed to get SAI ACL static variables\n");
        return retVal;
    }

    return retVal;

}

sai_switch_notification_t* xpSaiGetSwitchNotifications(xpsDevice_t devId)
{
    if (devId >= ARRAY_SIZE(switch_notifications_g))
    {
        return NULL;
    }
    return &switch_notifications_g[devId];
}

static sai_status_t xpSaiSetSwitchAttrShellEnable(sai_attribute_value_t value)
{
    XP_STATUS retVal = XP_NO_ERR;
    if (value.booldata)
    {
        if ((retVal = xpsShellInit()) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("XP SHELL init failed \n");
            return xpsStatus2SaiStatus(retVal);
        }
    }
    else
    {
        if ((retVal = xpsShellDeInit()) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("XP SHELL de-init failed \n");
            return xpsStatus2SaiStatus(retVal);
        }
    }
    if ((retVal = xpsUtilCpssShellEnable(value.booldata)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Shell enable failed \n");
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}


static void xpSaiSwitchConfigFilePath(char *filePath)
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
            sprintf(filePath, "%s%s", xpRootPath, "/customers/");
        }
        else
        {
            sprintf(filePath, "%s%s", xpRootPath, "customers/");
        }

    }
    else
    {
        //Assuming customer is running from xdk path
        sprintf(filePath, "%s", "./customers/");
    }
}

//Func: xpSaiSwitchIbufferSpeedStrConvert
/*
 * Helper function to convert from string to enum in ibuffer config file parse
 */
static XP_SPEED xpSaiSwitchIbufferSpeedStrConvert(char *speed)
{
    if (!(strcmp("1g", speed)))
    {
        return XP_PG_SPEED_1G;
    }
    else if (!(strcmp("10g", speed)))
    {
        return XP_PG_SPEED_10G;
    }
    else if (!(strcmp("25g", speed)))
    {
        return XP_PG_SPEED_25G;
    }
    else if (!(strcmp("40g", speed)))
    {
        return XP_PG_SPEED_40G;
    }
    else if (!(strcmp("50g", speed)))
    {
        return XP_PG_SPEED_50G;
    }
    else if (!(strcmp("100g", speed)))
    {
        return XP_PG_SPEED_100G;
    }
    else if (!(strcmp("200g", speed)))
    {
        return XP_PG_SPEED_200G;
    }
    return XP_PG_SPEED_MISC;
}

//Func: xpSaiSwitchIbufferInit
/*
 * This function parses a config file with the following syntax:
 *
 * speed[1g] = { lossless=<size>, xoff=<size>, xon=<size>, lossy=<size>, lydrop= [<thld0>,<thld1>,<thld2>,<thld3>,<thld4>,<thld5>,<thld6>,<thld7>] };
 * ...
 * speed[100g] = { lossless=<size>, xoff=<size>, xon=<size>, lossy=<size>, lydrop= [<thld0>,<thld1>,<thld2>,<thld3>,<thld4>,<thld5>,<thld6>,<thld7>] };
 *
 * Speeds supported are: 1g, 10g, 25g, 40g, 50g, 100g
 *
 * Not all speeds are mandatory. We will have defaults for any missing speed
 *
 * All sizes are in bytes. Total buffer size changes based on port speed.
 *
 * Each speed definition must be on its own line
 *
 * Note: The lydrop configurations, must be in ascending order, as they are fill level based thresholds
 *       ex: lydrop = [1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500]
 *
 * Essentially if the total lossy utilization is at 2700, all lossy traffic from priorities 0, 1, 2, 3 will be dropped, but
 * traffic from priorities 4, 5, 6, 7 will be accepted
 *
 * If redefining an allocation, all attributes are mandatory
 */
void xpSaiSwitchIbufferInit(xpSaiSwitchProfile_t *profile)
{
    FILE    *ibufCfgFilePtr = NULL;
    char    *line = NULL;
    char    *tok;
    XP_SPEED speed = XP_PG_SPEED_1G;
    uint32_t priority;
    xpSaiSwitchIbufferParseTkns_t pt =
        XPSAI_IB_PRS_TKN_SPEED;   // Used to maintain current state in the parse
    char     *filePath = NULL;
    char     *filePathTemp = NULL;

    line = (char *)xpMalloc(sizeof(char)*XP_MAX_CONFIG_LINE_LEN);
    if (!line)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for line\n");
        return;
    }
    memset(line, 0, sizeof(char)*XP_MAX_CONFIG_LINE_LEN);

    filePath = (char *)xpMalloc(sizeof(char)*512);
    if (!filePath)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for filePath\n");
        xpFree(line);
        return;
    }
    memset(filePath, 0, sizeof(char)*512);
    filePathTemp = (char *)xpMalloc(sizeof(char)*512);
    if (!filePathTemp)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for filePath\n");
        xpFree(line);
        xpFree(filePath);
        return;
    }
    memset(filePathTemp, 0, sizeof(char)*512);
    if (!strlen(profile->ibufConfigFile))
    {
        // No config file set in profile, keep the default configuration
        xpFree(line);
        xpFree(filePath);
        xpFree(filePathTemp);
        return;
    }

    // See if the config file can be opened (ie: check if we need to use defaults)
    xpSaiSwitchConfigFilePath(filePathTemp);
    sprintf(filePath, "%s/%s", filePathTemp, profile->ibufConfigFile);

    if ((ibufCfgFilePtr = fopen(filePath, "r")) == NULL)
    {
        // No config file present, keep the default configuration
        xpFree(line);
        xpFree(filePathTemp);
        xpFree(filePath);
        return;
    }
    xpFree(filePath);
    xpFree(filePathTemp);

    // Start the parse
    while (!feof(ibufCfgFilePtr))
    {
        while (fgets(line, XP_MAX_CONFIG_LINE_LEN, ibufCfgFilePtr) != NULL)
        {
            if (strlen(line) < (XP_MAX_CONFIG_LINE_LEN-1))
            {
                line[strlen(line) - 1] = '\0';
            }
            else
            {
                line[XP_MAX_CONFIG_LINE_LEN - 1] = '\0';
            }

            priority = 0;

            // Tokenize the string
            tok = strtok(line, " ={,[];}");
            while (tok != NULL)
            {
                if (!strcmp("speed", tok))
                {
                    pt = XPSAI_IB_PRS_TKN_SPEED;
                    tok = strtok(NULL, " ={,[];}");
                    continue;
                }
                else if (!strcmp("lossless", tok))
                {
                    pt = XPSAI_IB_PRS_TKN_LOSSLESS;
                    tok = strtok(NULL, " ={,[];}");
                    continue;
                }
                else if (!strcmp("xoff", tok))
                {
                    pt = XPSAI_IB_PRS_TKN_XOFF;
                    tok = strtok(NULL, " ={,[];}");
                    continue;
                }
                else if (!strcmp("xon", tok))
                {
                    pt = XPSAI_IB_PRS_TKN_XON;
                    tok = strtok(NULL, " ={,[];}");
                    continue;
                }
                else if (!strcmp("lossy", tok))
                {
                    pt = XPSAI_IB_PRS_TKN_LOSSY;
                    tok = strtok(NULL, " ={,[];}");
                    continue;
                }
                else if (!strcmp("lydrop", tok))
                {
                    pt = XPSAI_IB_PRS_TKN_LYDROP;
                    tok = strtok(NULL, " ={,[];}");
                    continue;
                }

                switch (pt)
                {
                    case XPSAI_IB_PRS_TKN_SPEED:
                        // The first token must be a speed token
                        speed = xpSaiSwitchIbufferSpeedStrConvert(tok);
                        profile->ibufConfig[speed].speed = speed;
                        break;
                    case XPSAI_IB_PRS_TKN_LOSSLESS:
                        profile->ibufConfig[speed].losslessAlloc = atoi(tok);
                        break;

                    case XPSAI_IB_PRS_TKN_XOFF:
                        profile->ibufConfig[speed].losslessXoff = atoi(tok);
                        break;

                    case XPSAI_IB_PRS_TKN_XON:
                        profile->ibufConfig[speed].losslessXon = atoi(tok);
                        break;

                    case XPSAI_IB_PRS_TKN_LOSSY:
                        profile->ibufConfig[speed].lossyAlloc = atoi(tok);
                        break;

                    case XPSAI_IB_PRS_TKN_LYDROP:
                        if (priority < XPSAI_MAX_PRIORITIES_SUPPORTED)
                        {
                            profile->ibufConfig[speed].lossyDrop[priority++] = atoi(tok);
                        }
                        else
                        {
                            XP_SAI_LOG_ERR("Unsupported priority %d is being configured with a drop fill level of %d bytes",
                                           priority, atoi(tok));
                            fclose(ibufCfgFilePtr);
                            xpFree(line);
                            return;
                        }
                        break;

                    default:
                        XP_SAI_LOG_ERR("Error in parsing the IBuffer Config file. Please check the syntax");
                        fclose(ibufCfgFilePtr);
                        xpFree(line);
                        return;
                }
                tok = strtok(NULL, " ={,[];}");
            }
        }
    }
    xpFree(line);
    fclose(ibufCfgFilePtr);
}

static sai_status_t xpSaiSetSwitchBindPoint(sai_uint32_t tableId)
{
    xpSaiAclTableIdMappingT  *pSaiAclTableAttribute = NULL;
    sai_status_t             saiRetVal = SAI_STATUS_SUCCESS;

    saiRetVal = xpSaiAclTableAttributesGet(tableId, &pSaiAclTableAttribute);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get Table Entry, ret %d\n", saiRetVal);
        return saiRetVal;
    }

    pSaiAclTableAttribute->isSwitchAclEn = true;

    if (pSaiAclTableAttribute->numEntries)
    {
        for (sai_uint16_t count = 0; count < pSaiAclTableAttribute->numEntries; count++)
        {
            saiRetVal = xpSaiAclAddDuplicateAclEntry(tableId,
                                                     pSaiAclTableAttribute->entryId[count]);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not duplicate Entry, ret %d\n", saiRetVal);
                return saiRetVal;
            }

        }
    }

    return SAI_STATUS_SUCCESS;

}

static sai_status_t xpSaiRemoveSwitchBindPoint(sai_uint32_t tableId)
{
    xpSaiAclTableIdMappingT  *pSaiAclTableAttribute = NULL;
    sai_status_t             saiRetVal = SAI_STATUS_SUCCESS;
    sai_uint32_t             entryId;

    saiRetVal = xpSaiAclTableAttributesGet(tableId, &pSaiAclTableAttribute);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get Table Entry, ret %d\n", saiRetVal);
        return saiRetVal;

    }
    pSaiAclTableAttribute->isSwitchAclEn = false;

    if (pSaiAclTableAttribute->numEntries)
    {
        for (sai_uint16_t count = 0; count < pSaiAclTableAttribute->numEntries; count++)
        {
            entryId = pSaiAclTableAttribute->entryId[count];
            saiRetVal = xpSaiAclRemoveDuplicateAclEntry(tableId, entryId);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Could not delete duplicate Entry, ret %d\n", saiRetVal);
                return saiRetVal;
            }


        }

    }

    return SAI_STATUS_SUCCESS;
}
//Func: xpSaiSetSwitchAttrIngressAcl
sai_status_t xpSaiSetSwitchAttrIngressAcl(sai_object_id_t switch_id,
                                          sai_attribute_value_t value)
{
    sai_status_t        saiStatus    = SAI_STATUS_SUCCESS;
    sai_uint32_t        tableId      = 0;
    XP_STATUS           xpStatus     = XP_NO_ERR;
    sai_object_id_t     aclObjId     = value.oid;
    xpSaiSwitchEntry_t* pSwitchEntry = NULL;
    xpSaiSwitchEntry_t  key;

    XP_SAI_LOG_DBG("Calling xpSaiSetSwitchAttrIngressAcl\n");

    if (!XDK_SAI_OBJID_TYPE_CHECK(switch_id, SAI_OBJECT_TYPE_SWITCH))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(switch_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&key, 0, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                  &key, (void**)&pSwitchEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %d!\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (pSwitchEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find switch static variables entry!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (pSwitchEntry->ingressAclObjId == aclObjId)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (aclObjId != SAI_NULL_OBJECT_ID)
    {
        if (!XDK_SAI_OBJID_TYPE_CHECK(aclObjId, SAI_OBJECT_TYPE_ACL_TABLE) &&
            !XDK_SAI_OBJID_TYPE_CHECK(aclObjId, SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
        {
            XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                           xpSaiObjIdTypeGet(aclObjId));
            return SAI_STATUS_INVALID_PARAMETER;
        }

        if (XDK_SAI_OBJID_TYPE_CHECK(aclObjId, SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
        {
            // Binding ACL table group to port
            sai_uint32_t tableIds[256] = {0};
            sai_uint32_t count         = sizeof(tableIds) / sizeof(sai_uint32_t);

            saiStatus = xpSaiGetACLTableGrpTableIdList(aclObjId, &count, tableIds);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiGetACLTableGrpTableIdList failed with error %d \n",
                               saiStatus);
                return saiStatus;
            }

            if (count > 0)
            {
                tableId = tableIds[count
                                   -1];    // For now supporting binding of single table to port
            }

        }
        else
        {
            // Binding ACL table to port

            // Get the acl table id from table object
            tableId = (xpsPort_t)xpSaiObjIdValueGet(aclObjId);
            saiStatus = xpSaiSetSwitchBindPoint(tableId);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiSetSwitchBindPoint failed with error %d \n", saiStatus);
                return saiStatus;
            }
        }

    }
    else
    {

        tableId = (xpsPort_t)xpSaiObjIdValueGet(pSwitchEntry->ingressAclObjId);
        saiStatus = xpSaiRemoveSwitchBindPoint(tableId);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiRemoveSwitchBindPoint failed with error %d \n", saiStatus);
            return saiStatus;
        }

    }

    // Update state database
    pSwitchEntry->ingressAclObjId = aclObjId;

    return saiStatus;
}


//Func: xpSaiGetSwitchAttrIngressAcl

sai_status_t xpSaiGetSwitchAttrIngressAcl(sai_object_id_t switch_id,
                                          sai_attribute_value_t* value)
{
    XP_STATUS           xpStatus     = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSwitchEntry = NULL;
    xpSaiSwitchEntry_t  key;

    XP_SAI_LOG_DBG("Calling xpSaiGetSwitchAttrIngressAcl\n");

    memset(&key, 0, sizeof(key));
    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                  &key, (void**)&pSwitchEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %d!\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (pSwitchEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find switch static variables entry!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    value->oid = pSwitchEntry->ingressAclObjId;

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiSetSwitchAttrEgressAcl

sai_status_t xpSaiSetSwitchAttrEgressAcl(sai_object_id_t switch_id,
                                         sai_attribute_value_t value)
{
    sai_status_t  saiStatus = SAI_STATUS_SUCCESS;
    uint32_t      tableId = 0;

    XP_SAI_LOG_DBG("Calling xpSaiSetSwitchAttrEgressAcl\n");

    if (value.oid != SAI_NULL_OBJECT_ID)
    {
        if (!XDK_SAI_OBJID_TYPE_CHECK(value.oid, SAI_OBJECT_TYPE_ACL_TABLE))
        {
            XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                           xpSaiObjIdTypeGet(value.oid));
            return SAI_STATUS_INVALID_PARAMETER;
        }

        //Table Id will be used as ACL ID
        tableId = (uint32_t)xpSaiObjIdValueGet(value.oid);
        if ((XP_SAI_EACL_ID_SET & tableId) != XP_SAI_EACL_ID_SET)
        {
            XP_SAI_LOG_ERR("Invalid table Id :%d for Port\n", tableId);
            return xpsStatus2SaiStatus(XP_ERR_INVALID_ARG);
        }

        if ((gEgressAclId != 0) && (gEgressAclId != tableId))
        {
            XP_SAI_LOG_DBG("ACL configuration is already present\n");
            return xpsStatus2SaiStatus(XP_ERR_INVALID_ARG);
        }

        //0-127 is used for port so using 128 for switch.
        saiStatus = xpSaiAclEgressAclIdSet(XP_SAI_SWITCH_ACL_ID, tableId,
                                           SAI_ACL_BIND_POINT_TYPE_SWITCH);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiAclEgressAclIdSet failed with error %d\n", saiStatus);
            return saiStatus;
        }
        gEgressAclId = tableId;
    }
    else
    {
        //TODO.

        gEgressAclId = 0;
    }

    return saiStatus;
}

//Func: xpSaiGetSwitchAttrEgressAcl

sai_status_t xpSaiGetSwitchAttrEgressAcl(sai_object_id_t switch_id,
                                         sai_attribute_value_t* value)
{
    sai_status_t    retVal  = SAI_STATUS_SUCCESS;
    xpsDevice_t     devId   = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Calling xpSaiGetSwitchAttrEgressAcl\n");

    if (gEgressAclId == 0)
    {
        value->oid = SAI_NULL_OBJECT_ID;
    }
    else
    {
        retVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ACL_TABLE, devId,
                                  (sai_uint64_t)gEgressAclId, &value->oid);
    }

    return retVal;
}

//Func: xpSaiGetSwitchAttrVxlanDefaultPort

sai_status_t xpSaiGetSwitchAttrVxlanDefaultPort(sai_object_id_t switch_id,
                                                sai_attribute_value_t* value)
{
    sai_status_t        retVal       = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus     = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSwitchEntry = NULL;
    xpSaiSwitchEntry_t  key;

    XP_SAI_LOG_DBG("Calling xpSaiGetSwitchAttrVxlanDefaultPort\n");

    memset(&key, 0, sizeof(key));
    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                  &key, (void**)&pSwitchEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %d!\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (pSwitchEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find switch static variables entry!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    value->u16 = pSwitchEntry->vxlan_udp_dst_port;

    return retVal;
}

//Func: xpSaiSetSwitchAttrVxlanDefaultPort

sai_status_t xpSaiSetSwitchAttrVxlanDefaultPort(sai_object_id_t switch_id,
                                                sai_attribute_value_t value)
{
    sai_status_t        retVal       = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus     = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSwitchEntry = NULL;
    xpSaiSwitchEntry_t  key;

    XP_SAI_LOG_DBG("Calling xpSaiSetSwitchAttrVxlanDefaultPort\n");

    memset(&key, 0, sizeof(key));
    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                  &key, (void**)&pSwitchEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %d!\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (pSwitchEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find switch static variables entry!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    pSwitchEntry->vxlan_udp_dst_port = value.u16;

    return retVal;
}

//Func: xpSaiGetSwitchAttrVxlanDefaultRouterMac

sai_status_t xpSaiGetSwitchAttrVxlanDefaultRouterMac(sai_object_id_t switch_id,
                                                     sai_attribute_value_t* value)
{
    sai_status_t        retVal       = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus     = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSwitchEntry = NULL;
    xpSaiSwitchEntry_t  key;

    XP_SAI_LOG_DBG("Calling xpSaiGetSwitchAttrVxlanDefaultRouterMac\n");

    memset(&key, 0, sizeof(key));
    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                  &key, (void**)&pSwitchEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %d!\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (pSwitchEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find switch static variables entry!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    xpSaiMacCopy(value->mac, pSwitchEntry->vxlan_default_router_mac);

    return retVal;
}

//Func: xpSaiSetSwitchAttrVxlanDefaultRouterMac

sai_status_t xpSaiSetSwitchAttrVxlanDefaultRouterMac(sai_object_id_t switch_id,
                                                     sai_attribute_value_t value)
{
    sai_status_t        retVal       = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus     = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSwitchEntry = NULL;
    xpSaiSwitchEntry_t  key;

    XP_SAI_LOG_DBG("Calling xpSaiSetSwitchAttrVxlanDefaultRouterMac\n");

    memset(&key, 0, sizeof(key));
    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                  &key, (void**)&pSwitchEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %d!\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (pSwitchEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find switch static variables entry!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    xpSaiMacCopy(pSwitchEntry->vxlan_default_router_mac, value.mac);

    return retVal;
}

static sai_status_t xpSaiSwitchProfileGetRef(sai_switch_profile_id_t profile_id,
                                             const char* key, char** pValue)
{
    const char* profileValue = NULL;

    if (NULL == pValue)
    {
        XP_SAI_LOG_ERR("Missing mandatory argument pValue\n");
        return SAI_STATUS_FAILURE;
    }

    profileValue = adapHostServiceMethodTable->profile_get_value(profile_id, key);
    if (NULL == profileValue)
    {
        profileValue = xpSaiProfileKeyValueGet(profile_id, key);
    }

    if (profileValue == NULL)
    {
        XP_SAI_LOG_DBG("Could not find a value for the key %s. \n", key);
        return SAI_STATUS_FAILURE;
    }

    *pValue = (char*)profileValue;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiSwitchProfileCharValueGet(sai_switch_profile_id_t profile_id,
                                            const char* key, uint8_t* pValue)
{
    sai_status_t    retVal       = SAI_STATUS_SUCCESS;
    char*           profileValue = NULL;
    int32_t         value        = 0;

    retVal = xpSaiSwitchProfileGetRef(profile_id, key, &profileValue);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_DBG("Could not find a value for the key %s. \n", key);
        return SAI_STATUS_FAILURE;
    }

    value = atoi(profileValue);

    XP_SAI_LOG_DBG("key %s, value %d. \n", key, value);

    *pValue = value;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiSwitchProfileShortValueGet(sai_switch_profile_id_t profile_id,
                                             const char* key, uint16_t* pValue)
{
    sai_status_t    retVal       = SAI_STATUS_SUCCESS;
    char*           profileValue = NULL;
    int32_t         value        = 0;

    retVal = xpSaiSwitchProfileGetRef(profile_id, key, &profileValue);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_DBG("Could not find a value for the key %s. \n", key);
        return SAI_STATUS_FAILURE;
    }

    value = atoi(profileValue);

    XP_SAI_LOG_DBG("key %s, value %d. \n", key, value);

    *pValue = value;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiSwitchProfileIntValueGet(sai_switch_profile_id_t profile_id,
                                           const char* key, int32_t* pValue)
{
    sai_status_t    retVal       = SAI_STATUS_SUCCESS;
    char*           profileValue = NULL;

    retVal = xpSaiSwitchProfileGetRef(profile_id, key, &profileValue);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_DBG("Could not find a value for the key %s. \n", key);
        return SAI_STATUS_FAILURE;
    }

    *pValue = atoi(profileValue);

    XP_SAI_LOG_DBG("key %s, value %d. \n", key, *pValue);

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiSwitchProfileStrValueGet(sai_switch_profile_id_t profile_id,
                                           const char* key, char* pValue, uint32_t size)
{
    sai_status_t    retVal       = SAI_STATUS_SUCCESS;
    char*           profileValue = NULL;
    char*           elSymbol     = NULL;

    retVal = xpSaiSwitchProfileGetRef(profile_id, key, &profileValue);
    if (SAI_STATUS_SUCCESS != retVal)
    {
        XP_SAI_LOG_DBG("Could not find a value for the key %s. \n", key);
        return SAI_STATUS_FAILURE;
    }

    strncpy(pValue, profileValue, size);
    /* Force null-termination */
    pValue[size - 1] = '\0';

    /*
     * remove trailing \r\n from the string
     */
    elSymbol = strrchr(pValue, '\r');
    if (elSymbol != NULL)
    {
        *elSymbol = '\0';
    }

    elSymbol = strrchr(pValue, '\n');
    if (elSymbol != NULL)
    {
        *elSymbol = '\0';
    }

    XP_SAI_LOG_DBG("key %s, value %s. \n", key, profileValue);

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief SAI Switch State Key Compare function
 *
 * This API is used by the state manager as a method to compare
 * keys
 *
 * \param [in] key1
 * \param [in] key2
 *
 * \return int32_t
 */
static int32_t xpSaiSwitchStaticDbEntryComp(void *key1, void *key2)
{
    return ((uint32_t)(((xpSaiSwitchEntry_t*)key1)->keyStaticDataType)
            - (uint32_t)(((xpSaiSwitchEntry_t*)key2)->keyStaticDataType));
}

static xpsDbHandle_t xpSaiTableEntryCountDbHndl = XPS_STATE_INVALID_DB_HANDLE;

static int32_t xpSaiTableEntryCountCtxKeyComp(void* key1, void* key2)
{
    return ((((xpSaiTableEntryCountDbEntry *) key1)->keyDevId) - (((
                                                                       xpSaiTableEntryCountDbEntry *) key2)->keyDevId));
}

/*This is called in switch init*/
XP_STATUS xpSaiTableEntryCountDbInit()
{
    XP_STATUS retVal = XP_NO_ERR;

    // Create global Switch Table Entry Count DB
    xpSaiTableEntryCountDbHndl = XPSAI_TABLE_ENTRY_COUNT_DB_HNDL;
    if ((retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "SAI ENTRY COUNT DB",
                                     XPS_GLOBAL, &xpSaiTableEntryCountCtxKeyComp,
                                     xpSaiTableEntryCountDbHndl)) != XP_NO_ERR)
    {
        xpSaiTableEntryCountDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create SAI Table Entry Count DB\n");
        return retVal;
    }
    return retVal;
}

sai_status_t xpSaiTableEntryCountDbDeInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiTableEntryCountDbEntry key;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    memset(&key, 0x0, sizeof(xpSaiTableEntryCountDbEntry));
    key.keyDevId = xpSaiDevId;

    // Remove the corresponding state
    if ((retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, xpSaiTableEntryCountDbHndl,
                                     &key, (void**)&entryCountCtxPtr)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to delete data DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    if ((retVal = xpsStateHeapFree(entryCountCtxPtr)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to to free allocated memory\n");
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

XP_STATUS xpSaiInsertTableEntryCountCtxDb(xpsScope_t scopeId,
                                          xpsDevice_t xpSaiDevId, xpSaiTableEntryCountDbEntry **entryCountCtxPtr)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpCfgTblEntryCountStruct tblEntryCount;

    if ((retVal = xpsStateHeapMalloc(sizeof(xpSaiTableEntryCountDbEntry),
                                     (void**)entryCountCtxPtr)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("XPS state Heap Allocation failed \n");
        return retVal;
    }
    if (*entryCountCtxPtr == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    memset(*entryCountCtxPtr, 0, sizeof(xpSaiTableEntryCountDbEntry));
    (*entryCountCtxPtr)->keyDevId = xpSaiDevId;

    memset(&tblEntryCount, 0, sizeof(xpCfgTblEntryCountStruct));
    if ((retVal = xpsCpssCfgTableNumEntriesGet(xpSaiDevId,
                                               &tblEntryCount)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Cpss Table Entries Count Get failed %d\n", retVal);
        xpsStateHeapFree((void*)*entryCountCtxPtr);
        *entryCountCtxPtr = NULL;
        return retVal;
    }

    // These are software concepts and max values are practically what allocator can allocate.
    tblEntryCount.aclTables = XP_SAI_ACL_TBL_MAX_IDS;
    tblEntryCount.aclTableGroups = XP_SAI_ACL_TBL_GRP_MAX_IDS;
    tblEntryCount.aclCounters = XP_SAI_ACL_PER_TBL_COUNTER_MAX_IDS;

    XP_SAI_LOG_INFO("...CPSS Cfg Table Counts...\n");
    XP_SAI_LOG_INFO("ipv4RouteEntries = %d\n", tblEntryCount.ipv4RouteEntries);
    XP_SAI_LOG_INFO("ipv6RouteEntries = %d\n", tblEntryCount.ipv6RouteEntries);
    XP_SAI_LOG_INFO("nextHopEntries = %d\n", tblEntryCount.nextHopEntries);
    XP_SAI_LOG_INFO("nextHopGroupEntries = %d\n",
                    tblEntryCount.nextHopGroupEntries);
    XP_SAI_LOG_INFO("NeighborEntries = %d\n", tblEntryCount.arpEntries);
    XP_SAI_LOG_INFO("fdpEntries = %d\n", tblEntryCount.fdbEntries);
    XP_SAI_LOG_INFO("aclTables = %d\n", tblEntryCount.aclTables);
    XP_SAI_LOG_INFO("aclTableGroups = %d\n", tblEntryCount.aclTableGroups);
    XP_SAI_LOG_INFO("aclEntries = %d\n", tblEntryCount.aclEntries);
    XP_SAI_LOG_INFO("aclCounters = %d\n", tblEntryCount.aclCounters);

    (*entryCountCtxPtr)->max_ipv4RouteEntries = tblEntryCount.ipv4RouteEntries;
    (*entryCountCtxPtr)->max_ipv6RouteEntries = tblEntryCount.ipv6RouteEntries;
    (*entryCountCtxPtr)->max_nextHopEntries = tblEntryCount.nextHopEntries;
    (*entryCountCtxPtr)->max_nextHopGroupEntries =
        tblEntryCount.nextHopGroupEntries;
    // Max next hop group member will be same as next hop entries.
    (*entryCountCtxPtr)->max_nextHopGroupMemberEntries =
        tblEntryCount.nextHopEntries;
    (*entryCountCtxPtr)->max_arpEntries = tblEntryCount.arpEntries;
    (*entryCountCtxPtr)->max_fdbEntries = tblEntryCount.fdbEntries;
    (*entryCountCtxPtr)->max_aclTables = tblEntryCount.aclTables;
    (*entryCountCtxPtr)->max_aclTableGroups = tblEntryCount.aclTableGroups;
    (*entryCountCtxPtr)->max_aclEntries = tblEntryCount.aclEntries;
    (*entryCountCtxPtr)->max_aclCounters = tblEntryCount.aclCounters;

    (*entryCountCtxPtr)->max_lagMemberPerGroup = tblEntryCount.lagMbrCnt;
    (*entryCountCtxPtr)->max_lagGroups = tblEntryCount.lagIds;

    if ((retVal = xpsStateInsertData(scopeId, xpSaiTableEntryCountDbHndl,
                                     (void*)*entryCountCtxPtr)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("XPS state Insert failed %d\n", retVal);
        xpsStateHeapFree((void*)*entryCountCtxPtr);
        *entryCountCtxPtr = NULL;
        return retVal;
    }
    return retVal;
}

sai_status_t xpSaiGetTableEntryCountCtxDb(xpsDevice_t xpSaiDevId,
                                          xpSaiTableEntryCountDbEntry **entryCountCtxPtr)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiTableEntryCountDbEntry keyTableEntryCtx;
    xpsScope_t scopeId = XP_SCOPE_DEFAULT;

    scopeId = xpSaiScopeFromDevGet(xpSaiDevId);

    memset(&keyTableEntryCtx, 0x0, sizeof(xpSaiTableEntryCountDbEntry));
    keyTableEntryCtx.keyDevId = xpSaiDevId;
    if ((retVal = xpsStateSearchData(scopeId, xpSaiTableEntryCountDbHndl,
                                     (xpsDbKey_t)&keyTableEntryCtx, (void**)entryCountCtxPtr)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Search data failed, retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (!(*entryCountCtxPtr))
    {
        XP_SAI_LOG_ERR("Entry does not exist \n");
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }

    return xpsStatus2SaiStatus(retVal);
}

/**
 * \brief SAI Switch State DB initialize function
 *
 * This API is used to initialize switch state manager
 *
 * \param [in] profile
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchDbInit(xpSaiSwitchProfile_t *profile)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar = NULL;

    xpSaiSwitchStaticDataDbHandle = XPSAI_SWITCH_STATIC_DB_HNDL;
    retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "xpSaiSwitchDataDb state",
                                XPS_GLOBAL, &xpSaiSwitchStaticDbEntryComp, xpSaiSwitchStaticDataDbHandle);
    if (retVal != XP_NO_ERR)
    {
        xpSaiSwitchStaticDataDbHandle = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Failed to register SAI Switch DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    if (profile->initType == INIT_COLD)
    {
        // Create a new structure
        if ((retVal = xpsStateHeapMalloc(sizeof(xpSaiSwitchEntry_t),
                                         (void**)&pSaiSwitchStaticVar)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to allocate memory\n");
            return xpsStatus2SaiStatus(retVal);
        }

        // Initialize the allocated entry
        pSaiSwitchStaticVar->keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

        memset(pSaiSwitchStaticVar->switchSrcMacAddress, 0, sizeof(sai_mac_t));
        pSaiSwitchStaticVar->defaultTrapGroup = 0;
        pSaiSwitchStaticVar->defaultHash = SAI_NULL_OBJECT_ID;
        pSaiSwitchStaticVar->defaultVrf = SAI_NULL_OBJECT_ID;
        memset(pSaiSwitchStaticVar->mirrorSessionIds, 0,
               sizeof(pSaiSwitchStaticVar->mirrorSessionIds));
        memset(pSaiSwitchStaticVar->sampleSessionIds, 0,
               sizeof(pSaiSwitchStaticVar->sampleSessionIds));
        pSaiSwitchStaticVar->symmetricHashEnabled = false;
        pSaiSwitchStaticVar->ecmpHashSeed = 0;
        pSaiSwitchStaticVar->lagHashSeed = 0;
        pSaiSwitchStaticVar->ecmpHashAlgorithm = SAI_HASH_ALGORITHM_CRC;
        pSaiSwitchStaticVar->lagHashAlgorithm = SAI_HASH_ALGORITHM_CRC;
        pSaiSwitchStaticVar->restartTypeSupported = SAI_SWITCH_RESTART_TYPE_ANY;
        pSaiSwitchStaticVar->nv_storage_size_warmboot = NV_MEMPOOLSIZE;
        pSaiSwitchStaticVar->min_planned_restart_interval = MIN_INTERVAL_RESTART;
        pSaiSwitchStaticVar->switch_oper_status = SAI_SWITCH_OPER_STATUS_UNKNOWN;
        pSaiSwitchStaticVar->fdbUnicastMissAction = SAI_PACKET_ACTION_FORWARD;
        pSaiSwitchStaticVar->fdbMaxLearnedEntries =
            0; /* Default value (zero) means learning limit is disabled */
        pSaiSwitchStaticVar->fdbCtrlMacEntries    = 0; /* Default value is zero */
        pSaiSwitchStaticVar->bcastCpuFloodEnable  = false; /*Default value is disable*/
        pSaiSwitchStaticVar->mcastCpuFloodEnable  = false; /*Default value is disable*/
        pSaiSwitchStaticVar->numUcastQueues       =
            profile->numUcastQueues;  /* Port default number of unicast queues */
        pSaiSwitchStaticVar->numMcastQueues       =
            profile->numMcastQueues;  /* Port default number of multicast queues */
        pSaiSwitchStaticVar->numQueues            =
            profile->numQueues;       /* Port default number of queues */
        pSaiSwitchStaticVar->numCpuQueues         =
            profile->numCpuQueues;    /* Port default number of cpu queues */
        retVal = xpsL3GetNextHopGroupsMaxNum(&
                                             (pSaiSwitchStaticVar->numNhGrp));        /* number of next hop group */
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not get next hop groups max num!\n");
        }
        memcpy(pSaiSwitchStaticVar->ibufferConfig, profile->ibufConfig,
               sizeof(xpSaiSwitchIbufferCfg_t) *
               XP_PG_SPEED_MISC); /* Default Ibuffer Config, using MISC PG speed to indicate number of supported speeds */

        // Insert the state into the database
        if ((retVal = xpsStateInsertData(XP_SCOPE_DEFAULT,
                                         xpSaiSwitchStaticDataDbHandle, (void*)pSaiSwitchStaticVar)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to insert data into SAI Switch DB\n");
            return xpsStatus2SaiStatus(retVal);
        }

        pSaiSwitchStaticVar->vxlan_udp_dst_port = VXLAN_UDP_DST_PORT;
        memset(pSaiSwitchStaticVar->vxlan_default_router_mac, 0, sizeof(macAddr_t));
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief SAI Switch State DB deinitialize function
 *
 * This API is used to deinitialize switch state manager
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchDbDeInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;
    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    // Remove the corresponding state
    if ((retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT,
                                     xpSaiSwitchStaticDataDbHandle, &key,
                                     (void**)&pSaiSwitchStaticVar)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to delete data from SAI Switch DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    // Free the memory allocated for the corresponding state
    if ((retVal = xpsStateHeapFree(pSaiSwitchStaticVar)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to to free allocated memory\n");
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetSwitchAttrMtuMax(sai_object_id_t switchId,
                                      sai_attribute_value_t* value)
{
    XP_STATUS xpStatus  = XP_NO_ERR;

    value->u32 = XPSAI_SWITCH_MAX_MTU;

    return  xpsStatus2SaiStatus(xpStatus);
}

/**
 * \brief SAI Get Switch source MAC address
 *
 * \param mac - buffer with where MAC address data should be stored
 *
 * This API is used to get switch source MAC address
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetSwitchSrcMacAddress(sai_mac_t mac)
{
    XP_STATUS           status              = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar = NULL;
    xpSaiSwitchEntry_t  key;
    sai_mac_t zero = {0, 0, 0, 0, 0, 0};

    memset(&key, 0, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %d!\n", status);
        return xpsStatus2SaiStatus(status);
    }

    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Could not find switch static variables entry!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    memcpy(mac, pSaiSwitchStaticVar->switchSrcMacAddress, sizeof(sai_mac_t));

    if (0 == memcmp(mac, zero, sizeof(sai_mac_t)))
    {
        XP_SAI_LOG_NOTICE("Setting Pre Init MAC address %x:%x:%x:%x:%x:%x as mac is zero\n",
                          preInitMacAddr[0], preInitMacAddr[1], preInitMacAddr[2], preInitMacAddr[3],
                          preInitMacAddr[4], preInitMacAddr[5]);
        memcpy(mac, preInitMacAddr, sizeof(sai_mac_t));
    }

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief SAI set switch_oper_status
 *
 * \param oper_status - To set the Operational status of the switch
 *
 * This API is used to store the Operational status of the switch
 *
 * SAI_SWITCH_OPER_STATUS_UNKNOWN should be initial value
 * SAI_SWITCH_OPER_STATUS_UP after full initialization in xpSaiInitializeSwitch()
 * SAI_SWITCH_OPER_STATUS_DOWN is set in xpSaiShutdownSwitch()
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSetSwitchOperStatus(sai_int32_t oper_status)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);

    if (pSaiSwitchStaticVar == NULL)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    pSaiSwitchStaticVar->switch_oper_status = oper_status;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief SAI Get switch_oper_status
 *
 * \param value - To Get the Operational status of the switch
 *
 * This API is used to get the Operational status of the switch
 *
 * SAI_SWITCH_OPER_STATUS_UNKNOWN should be initial value
 * SAI_SWITCH_OPER_STATUS_UP after full initialization in xpSaiInitializeSwitch()
 * SAI_SWITCH_OPER_STATUS_DOWN is set in xpSaiShutdownSwitch()
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetSwitchOperStatus(sai_attribute_value_t* value)
{

    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);

    if (pSaiSwitchStaticVar == NULL)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    value->s32 = pSaiSwitchStaticVar->switch_oper_status;

    return xpsStatus2SaiStatus(status);

}

/**
 * \brief SAI Set Switch Default Trap group
 *
 * \param group - Trap group
 *
 * This API is used to set default switch trap group
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchDefaultTrapGroupSet(sai_object_id_t group)
{
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle, &key,
                       (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    pSaiSwitchStaticVar->defaultTrapGroup = group;

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief SAI Get Switch default Trap group
 *
 * \param pGroup - Trap group
 *
 * This API is used to get default switch trap group
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchDefaultTrapGroupGet(sai_object_id_t* pGroup)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve default trap group id from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *pGroup = pSaiSwitchStaticVar->defaultTrapGroup;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief SAI Get Switch default Hash OID
 *
 * \param pHashId - Hash OID
 *
 * This API is used to get default switch Hash OID
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchDefaultHashGet(sai_object_id_t* pHashId)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (pHashId == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve default hash id from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *pHashId = pSaiSwitchStaticVar->defaultHash;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief SAI Set Switch default Hash OID
 *
 * \param pHashId - Hash OID
 *
 * This API is used to set default switch Hash OID into DB
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchDefaultHashSet(sai_object_id_t hashId)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve default hash id from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    pSaiSwitchStaticVar->defaultHash = hashId;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief SAI Get Switch default Bridge OID
 *
 * \param pBridgeId - Bridge OID
 *
 * This API is used to get default switch Bridge OID
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchDefaultBridgeIdGet(sai_object_id_t* pBridgeId)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (pBridgeId == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve default hash id from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *pBridgeId = pSaiSwitchStaticVar->default1QBridgeId;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief SAI Set Switch default Bridge OID
 *
 * \param pBridgeId - Bridge OID
 *
 * This API is used to set default switch Bridge OID into DB
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchDefaultBridgeIdSet(sai_object_id_t bridgeId)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve default hash id from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    pSaiSwitchStaticVar->default1QBridgeId = bridgeId;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief SAI Get Switch default VRF OID
 *
 * \param pVrfId - VRF OID
 *
 * This API is used to get default switch VRF OID
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchDefaultVrfGet(sai_object_id_t* pVrfId)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (pVrfId == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve default hash id from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *pVrfId = pSaiSwitchStaticVar->defaultVrf;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief SAI Set Switch default VRF OID
 *
 * \param vrfId - VRF OID
 *
 * This API is used to set default switch VRF OID into DB
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchDefaultVrfSet(sai_object_id_t vrfId)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve default hash id from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    pSaiSwitchStaticVar->defaultVrf = vrfId;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief SAI Get Switch default Hash Algorithm
 *
 * \param value - Attr value
 *
 * This API is used to get default switch Hash Algorithm
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetSwitchAttrHashAlgo(sai_attribute_value_t *value,
                                        bool isLag)
{
    XP_SAI_LOG_DBG("Calling xpSaiGetSwitchAttrHashAlgo\n");

    return xpSaiSwitchHashAlgorithmGet(&value->s32, isLag);
}

/**
 * \brief Get the amount of ibuffer allocated for this port for
 *        lossless
 *
 *
 * \param devId     - device Id
 * \param speed     - port speed
 * \param allocation - Buffer allocation
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchGetIbufferLosslessAllocation(xpsDevice_t devId,
                                                     XP_SPEED speed, sai_uint32_t *allocation)
{
    XP_STATUS           status               = XP_NO_ERR;
    xpSaiSwitchEntry_t *pSaiSwitchStaticVar  = 0;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (allocation == 0)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (speed >= XP_PG_SPEED_MISC)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**) &pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == 0)
    {
        XP_SAI_LOG_ERR("Failed to retrieve Ibuffer config entries from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *allocation = pSaiSwitchStaticVar->ibufferConfig[speed].losslessAlloc;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Get the xoff threshold for the ibuffer out of the
 *        lossless allocation
 *
 *
 * \param devId     - deviceId
 * \param speed     - port speed
 * \param xoff      - xoff threshold
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchGetIBufferLosslessXoff(xpsDevice_t devId,
                                               XP_SPEED speed, sai_uint32_t *xoff)
{
    XP_STATUS           status               = XP_NO_ERR;
    xpSaiSwitchEntry_t *pSaiSwitchStaticVar  = 0;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (xoff == 0)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (speed >= XP_PG_SPEED_MISC)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**) &pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == 0)
    {
        XP_SAI_LOG_ERR("Failed to retrieve Ibuffer Config from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *xoff = pSaiSwitchStaticVar->ibufferConfig[speed].losslessXoff;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Get the xon threshold for the ibuffer out of the
 *        lossless allocation
 *
 *
 * \param devId     - deviceId
 * \param speed     - port speed
 * \param xon       - xon threshold
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchGetIbufferLosslessXon(xpsDevice_t devId, XP_SPEED speed,
                                              sai_uint32_t *xon)
{
    XP_STATUS           status               = XP_NO_ERR;
    xpSaiSwitchEntry_t *pSaiSwitchStaticVar  = 0;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (xon == 0)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (speed >= XP_PG_SPEED_MISC)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**) &pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == 0)
    {
        XP_SAI_LOG_ERR("Failed to retrieve Ibuffer Config from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *xon = pSaiSwitchStaticVar->ibufferConfig[speed].losslessXon;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Get the amount of ibuffer allocated for this port for
 *        lossy
 *
 * \param devId     - deviceId
 * \param speed     - port speed
 * \param allocation - Buffer allocation
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchGetIBufferLossyAllocation(xpsDevice_t devId,
                                                  XP_SPEED speed, sai_uint32_t *allocation)
{
    XP_STATUS           status               = XP_NO_ERR;
    xpSaiSwitchEntry_t *pSaiSwitchStaticVar  = 0;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (allocation == 0)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**) &pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == 0)
    {
        XP_SAI_LOG_ERR("Failed to retrieve Ibuffer Config from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *allocation = pSaiSwitchStaticVar->ibufferConfig[speed].lossyAlloc;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Get the lossy drop levels for each priority in the
 *        ibuffer
 *
 *
 * \param devId     - deviceId
 * \param speed     - port speed
 * \param priority  - lossy priority of interest
 * \param level     - lossy drop level threshold
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchGetIBufferLossyDropLevel(xpsDevice_t devId,
                                                 XP_SPEED speed, sai_uint32_t priority, sai_uint32_t *level)
{
    XP_STATUS           status               = XP_NO_ERR;
    xpSaiSwitchEntry_t *pSaiSwitchStaticVar  = 0;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (level == 0)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    if (speed >= XP_PG_SPEED_MISC)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**) &pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == 0)
    {
        XP_SAI_LOG_ERR("Failed to retrieve Ibuffer Config from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *level = pSaiSwitchStaticVar->ibufferConfig[speed].lossyDrop[priority];

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Get maximum number of learned FDB entries
 *
 * \param devId         - device id
 * \param maxFdbEntries - placeholder for max FDB entries
 *
 * This API is used to retrieve maximum allowed FDB entries to be learned from SAI database
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchMaxFdbEntriesGet(xpsDevice_t devId,
                                         sai_uint32_t *maxFdbEntries)
{
    XP_STATUS           status               = XP_NO_ERR;
    xpSaiSwitchEntry_t *pSaiSwitchStaticVar  = 0;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (maxFdbEntries == 0)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key,
                                (void**) &pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == 0)
    {
        XP_SAI_LOG_ERR("Failed to retrieve max FDB entries from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *maxFdbEntries = pSaiSwitchStaticVar->fdbMaxLearnedEntries;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Set maximum number of learned FDB entries
 *
 * \param devId         - device id
 * \param maxFdbEntries - max FDB entries
 *
 * This API is used to set maximum allowed FDB entries that can be learned by device
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchMaxFdbEntriesSet(xpsDevice_t devId,
                                         sai_uint32_t maxFdbEntries)
{
    XP_STATUS           status               = XP_NO_ERR;
    xpSaiSwitchEntry_t  *pSaiSwitchStaticVar = 0;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key,
                                (void**) &pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == 0)
    {
        XP_SAI_LOG_ERR("Failed to retrieve max FDB entries from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    pSaiSwitchStaticVar->fdbMaxLearnedEntries = maxFdbEntries;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Get number of Control MAC entries
 *
 * \param devId          - device id
 * \param ctrlMacEntries - placeholder for Control MAC entries
 *
 * This API is used to retrieve current number of Control MAC entries
 * which were added by SAI adapter
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiSwitchCtrlMacEntriesGet(xpsDevice_t devId,
                                       uint32_t *ctrlMacEntries)
{
    XP_STATUS           status               = XP_NO_ERR;
    xpSaiSwitchEntry_t *pSaiSwitchStaticVar  = 0;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (ctrlMacEntries == 0)
    {
        return XP_ERR_NULL_POINTER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key,
                                (void**) &pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == 0)
    {
        XP_SAI_LOG_ERR("Failed to retrieve number of Control MAC entries from DB\n");
        return XP_ERR_KEY_NOT_FOUND;
    }

    *ctrlMacEntries = pSaiSwitchStaticVar->fdbCtrlMacEntries;

    return status;
}

/**
 * \brief Increment Control MAC entries counter
 *
 * \param devId - device id
 *
 * This API is used to increment Control MAC entries counter when SAI adapter
 * adds new entry
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiSwitchNewCtrlMacEntryAdd(xpsDevice_t devId)
{
    XP_STATUS           status               = XP_NO_ERR;
    xpSaiSwitchEntry_t  *pSaiSwitchStaticVar = 0;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key,
                                (void**) &pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == 0)
    {
        XP_SAI_LOG_ERR("Failed to retrieve Control MAC entries from DB\n");
        return XP_ERR_KEY_NOT_FOUND;
    }

    pSaiSwitchStaticVar->fdbCtrlMacEntries++;

    return status;
}

/**
 * \brief Decrement Control MAC entries counter
 *
 * \param devId - device id
 *
 * This API is used to decrement Control MAC entries counter when SAI adapter
 * removes an entry
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiSwitchCtrlMacEntryDelete(xpsDevice_t devId)
{
    XP_STATUS           status               = XP_NO_ERR;
    xpSaiSwitchEntry_t  *pSaiSwitchStaticVar = 0;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key,
                                (void**) &pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == 0)
    {
        XP_SAI_LOG_ERR("Failed to retrieve Control MAC entries from DB\n");
        return XP_ERR_KEY_NOT_FOUND;
    }

    pSaiSwitchStaticVar->fdbCtrlMacEntries--;

    return status;
}

/**
 * \brief Allocates mirror session ID
 *
 * \param sessionId - Session ID
 *
 * This API is used to allocate mirror session ID
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchMirrorSessionIdAllocate(uint32_t* sessionId)
{
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;
    uint32_t i = 0;

    memset(&key, 0x00, sizeof(key));

    if (sessionId == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle, &key,
                       (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve default hash id from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for (i = 0; i < XP_SAI_MAX_MIRROR_SESSIONS_NUM; i++)
    {
        if (!pSaiSwitchStaticVar->mirrorSessionIds[i])
        {
            *sessionId = i;
            pSaiSwitchStaticVar->mirrorSessionIds[i] = true;
            return SAI_STATUS_SUCCESS;
        }
    }

    return SAI_STATUS_INSUFFICIENT_RESOURCES;
}

/**
 * \brief Releases mirror session ID
 *
 * \param sessionId - Session ID
 *
 * This API is used to release mirror session ID
 *
 * \return
 */
void xpSaiSwitchMirrorSessionIdRelease(uint32_t sessionId)
{
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle, &key,
                       (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve default hash id from DB\n");
        return;
    }

    if (sessionId >= XP_SAI_MAX_MIRROR_SESSIONS_NUM)
    {
        XP_SAI_LOG_ERR("Invalid mirror session ID specified.\n");
        return;
    }

    pSaiSwitchStaticVar->mirrorSessionIds[sessionId] = false;
}

/**
 * \brief Allocates sample session ID
 *
 * \param sessionId - Session ID
 *
 * This API is used to allocate mirror session ID
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchSampleSessionIdAllocate(uint32_t* sessionId)
{
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;
    uint32_t i = 0;

    memset(&key, 0x00, sizeof(key));

    if (sessionId == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle, &key,
                       (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve default hash id from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for (i = 0; i < XP_SAI_MAX_MIRROR_SESSIONS_NUM; i++)
    {
        if (!pSaiSwitchStaticVar->sampleSessionIds[i])
        {
            *sessionId = i;
            pSaiSwitchStaticVar->sampleSessionIds[i] = true;

            return SAI_STATUS_SUCCESS;
        }
    }

    return SAI_STATUS_INSUFFICIENT_RESOURCES;
}

/**
 * \brief Releases mirror session ID
 *
 * \param sessionId - Session ID
 *
 * This API is used to release mirror session ID
 *
 * \return
 */
void xpSaiSwitchSampleSessionIdRelease(uint32_t sessionId)
{
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle, &key,
                       (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve default hash id from DB\n");
        return;
    }

    if (sessionId >= XP_SAI_MAX_MIRROR_SESSIONS_NUM)
    {
        XP_SAI_LOG_ERR("Invalid sample session ID specified.\n");
        return;
    }

    pSaiSwitchStaticVar->sampleSessionIds[sessionId] = false;
}

/**
 * \brief Gets SAI Switch symmetric hash status from DB
 *
 * \param pEnabled - true/false
 *
 * This API is used to get symmetric hash status from DB
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchSymmetricHashStatusGet(bool* pEnabled)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (pEnabled == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve symmetric hash status from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *pEnabled = pSaiSwitchStaticVar->symmetricHashEnabled;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Stores SAI Switch Symmetric Hash status to DB
 *
 * \param enable - true/false
 *
 * This API is used to store symmetric hash status to DB
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchSymmetricHashStatusSet(bool enable)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to set symmetric hash status to DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    pSaiSwitchStaticVar->symmetricHashEnabled = enable;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief get hash Algorithm from DB
 *
 * \param hashAlgotiryhm - CRC/XOR/RANDOM/CRC32H/CRC32L
 * \param isLag - lag/ecmp
 *
 * This API is used to get hash Algorithm from DB
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchHashAlgorithmGet(sai_int32_t* hashAlogrithm, bool isLag)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (hashAlogrithm == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve hash Algorithm from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (isLag)
    {
        *hashAlogrithm = pSaiSwitchStaticVar->lagHashAlgorithm;
    }
    else
    {
        *hashAlogrithm = pSaiSwitchStaticVar->ecmpHashAlgorithm;
    }


    return xpsStatus2SaiStatus(status);
}

/**hash
 * \brief store hash Algorithm into DB
 *
 * \param hashAlogrithm - CRC/XOR/RANDOM/CRC32H/CRC32L
 * \param isLag - lag/ecmp
 *
 * This API is used to store hash Algorithm into DB
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchHashAlgorithmSet(sai_int32_t hashAlogrithm, bool isLag)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    bool randomEnable = false;
    if (hashAlogrithm == SAI_HASH_ALGORITHM_RANDOM)
    {
        randomEnable = true;
    }

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve hash Algorithm from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (status != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(status);
    }

    if (isLag)
    {
        pSaiSwitchStaticVar->lagHashAlgorithm = hashAlogrithm;
        xpsLagRandomEnableSet(randomEnable);
    }
    else
    {
        pSaiSwitchStaticVar->ecmpHashAlgorithm = hashAlogrithm;
        xpsL3EcmpRandomEnableSet(randomEnable);
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief Gets SAI Switch LAG hash seed from XPS
 *
 * \param pSeed - hash seed
 *
 * This API is used to get LAG hash seed from XPS
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetSwitchAttrLagHashSeed(uint32_t* pSeed)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (pSeed == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve lag hash seed from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *pSeed = pSaiSwitchStaticVar->lagHashSeed;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Sets SAI Switch LAG Hash seed to XPS
 *
 * \param seed - hash seed
 *
 * This API is used to set LAG hash seed to XPS
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSetSwitchAttrLagHashSeed(uint32_t seed)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;
    xpsDevice_t devId =  xpSaiGetDevId();
    bool isLag = true;
    sai_int32_t curAlgo;
    sai_status_t  saiStatus = SAI_STATUS_SUCCESS;

    saiStatus = xpSaiSwitchHashAlgorithmGet(&curAlgo, isLag);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get Hash Algorithm");
        return saiStatus;
    }

    if (curAlgo == (sai_int32_t)SAI_HASH_ALGORITHM_RANDOM)
    {
        /*only if we have a non-zero hashSeed, set it in hardware*/
        if (seed != 0)
        {
            status = xpsLagSetRandomHashSeed(devId, seed);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not set Lag random hash seed");
                return xpsStatus2SaiStatus(status);
            }
        }
        else
        {
            /* we still want to allow Algo change*/
            XP_SAI_LOG_WARNING("For random Algo, hashseed has to be non-zero\n");
        }
    }
    else
    {
        status = xpsLagSetHashSeed(devId, seed);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsLagSetHashSeed failed devId %d \n", devId);
            return xpsStatus2SaiStatus(status);
        }
    }

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to set ECMP hash seed to DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    pSaiSwitchStaticVar->ecmpHashSeed = seed;
    pSaiSwitchStaticVar->lagHashSeed = seed;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Gets SAI Switch ECMP hash seed from DB
 *
 * \param pSeed - hash seed
 *
 * This API is used to get ECMP hash seed from DB
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetSwitchAttrEcmpHashSeed(uint32_t* pSeed)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (pSeed == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve ECMP hash seed from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *pSeed = pSaiSwitchStaticVar->ecmpHashSeed;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Gets SAI Switch restart type supported from DB
 *
 * \param sai_switch_restart_type_t  restartType
 *
 * This API is used to get restart type supported from DB
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchRestartTypeSupportedGet(sai_switch_restart_type_t*
                                                restartType)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (restartType == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve ECMP hash seed from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *restartType = pSaiSwitchStaticVar->restartTypeSupported;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Stores SAI Switch restart Type supported to DB
 *
 * \param sai_switch_restart_type_t  restartType
 *
 * This API is used to store restart Type supported to DB
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchRestartTypeSupportedSet(sai_switch_restart_type_t
                                                restartType)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to set ECMP hash seed to DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    pSaiSwitchStaticVar->restartTypeSupported = restartType;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Gets SAI Switch non volatile memory size for Warmboot from DB
 *
 * \param uint32_t* nv_storage_size
 *
 * This API is used to get non volatile memory size for Warmboot from DB
 *
 * \return sai_status_t
 */

sai_status_t xpSaiSwitchNvStorageSizeGet(uint32_t* nv_storage_size)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (nv_storage_size == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve ECMP hash seed from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *nv_storage_size = pSaiSwitchStaticVar->nv_storage_size_warmboot;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Stores SAI Switch non volatile memory size for Warmboot to DB
 *
 * \param uint32_t nv_storage_size
 *
 * This API is used to store non volatile memory size for Warmboot to DB
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchNvStorageSizeSet(uint32_t nv_storage_size)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to set ECMP hash seed to DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    pSaiSwitchStaticVar->nv_storage_size_warmboot = nv_storage_size;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Gets SAI Switch min planned restart interval for Warmboot from DB
 *
 * \param uint32_t* min_planned_restart_interval
 *
 * This API is used to get min planned restart interval for Warmboot from DB
 *
 * \return sai_status_t
 */

sai_status_t xpSaiSwitchMinPlannedRestartIntervalGet(uint32_t*
                                                     min_planned_restart_interval)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (min_planned_restart_interval == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve ECMP hash seed from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *min_planned_restart_interval =
        pSaiSwitchStaticVar->min_planned_restart_interval;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Stores SAI Switch min planned restart interval for Warmboot to DB
 *
 * \param uint32_t min_planned_restart_interval
 *
 * This API is used to store min planned restart interval for Warmboot to DB
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchMinPlannedRestartIntervalSet(uint32_t
                                                     min_planned_restart_interval)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to set ECMP hash seed to DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    pSaiSwitchStaticVar->min_planned_restart_interval =
        min_planned_restart_interval;

    return xpsStatus2SaiStatus(status);
}

/**
 * \brief Gets SAI Switch Profile ID from DB
 *
 * \param sai_uint32_t* profileId
 *
 * This API is used to get profile id from DB
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSwitchProfileIdGet(sai_uint32_t *profileId)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    xpSaiSwitchEntry_t keyEntry, *dataEntry = NULL;

    if (profileId == NULL)
    {
        XP_SAI_LOG_ERR("NULL pointer provided as an argument.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&keyEntry, 0x00, sizeof(xpSaiSwitchEntry_t));
    keyEntry.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                  &keyEntry, (void**)&dataEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed: status=%i.\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (dataEntry == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve profile id from DB.\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *profileId = dataEntry->profileId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSwitchGetDefaultQueueValues

sai_status_t xpSaiSwitchGetDefaultQueueValues(uint32_t *numQs,
                                              uint32_t *numUcastQs, uint32_t *numMcastQs, uint32_t *numCpuQs)
{
    XP_STATUS           status              = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %d!\n", status);
        return xpsStatus2SaiStatus(status);
    }

    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Could not find switch static variables entry!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (numQs)
    {
        *numQs = pSaiSwitchStaticVar->numQueues;
    }

    if (numUcastQs)
    {
        *numUcastQs = pSaiSwitchStaticVar->numUcastQueues;
    }

    if (numMcastQs)
    {
        *numMcastQs = pSaiSwitchStaticVar->numMcastQueues;
    }

    if (numCpuQs)
    {
        *numCpuQs = pSaiSwitchStaticVar->numCpuQueues;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrNumberOfUnicastQueues

sai_status_t xpSaiGetSwitchAttrNumberOfUnicastQueues(xpsDevice_t xpsDevId,
                                                     sai_attribute_value_t* value)
{
    sai_uint32_t numUcQs = 0;
    XP_STATUS    retVal = XP_NO_ERR;

    if ((retVal = xpsQosAqmGetNumberOfUnicastQueuesPerPort(xpsDevId, 0,
                                                           &numUcQs)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get number of unicast queues per port, retVal:%d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    value->u32 = numUcQs;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrNumberOfMulticastQueues

sai_status_t xpSaiGetSwitchAttrNumberOfMulticastQueues(xpsDevice_t xpsDevId,
                                                       sai_attribute_value_t* value)
{
    sai_uint32_t numMcQs = 0;
    XP_STATUS    retVal = XP_NO_ERR;

    if ((retVal = xpsQosAqmGetNumberOfMulticastQueuesPerPort(xpsDevId, 0,
                                                             &numMcQs)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get number of multicast queues per port, retVal:%d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    value->u32 = numMcQs;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrNumberOfQueues

sai_status_t xpSaiGetSwitchAttrNumberOfQueues(xpsDevice_t xpsDevId,
                                              sai_attribute_value_t* value)
{
    sai_uint32_t numQs = 0;
    XP_STATUS    retVal = XP_NO_ERR;

    if ((retVal = xpsQosAqmGetNumberOfQueuesPerPort(xpsDevId, 0,
                                                    &numQs)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get number of queues per port, retVal:%d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    value->u32 = numQs;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrNumberOfCpuQueues

sai_status_t xpSaiGetSwitchAttrNumberOfCpuQueues(xpsDevice_t xpsDevId,
                                                 sai_attribute_value_t* value)
{
    sai_uint32_t    numQs       = 0;
    XP_STATUS       retVal      = XP_NO_ERR;
    uint32_t        cpuPortNum  = 0;

    retVal = xpsGlobalSwitchControlGetCpuPhysicalPortNum(xpsDevId, &cpuPortNum);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get CPU physical port number, retVal:%d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /* Get total number of queues per port */
    retVal = xpsQosAqmGetNumberOfQueuesPerPort(xpsDevId, cpuPortNum, &numQs);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get number of queues per port, retVal:%d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    value->u32 = numQs;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSwitchAttrOperStatus

sai_status_t xpSaiSetSwitchAttrOperStatus(sai_attribute_value_t value)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiSetSwitchAttrOperStatus\n");

    return retVal;
}


//Func: xpSaiSetSwitchAttrMaxTemp

sai_status_t xpSaiSetSwitchAttrMaxTemp(sai_attribute_value_t value)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiSetSwitchAttrMaxTemp\n");

    return retVal;
}


//Func: xpSaiSetSwitchAttrSwitchingMode

sai_status_t xpSaiSetSwitchAttrSwitchingMode(sai_attribute_value_t value,
                                             xpsDevice_t devId)
{

    XP_STATUS           xpStatus    = XP_NO_ERR;
    xpBypassMode_e      byPassMode  = XP_TXQ_EQ_BYPASS_MODE_NONE;

    XP_SAI_LOG_DBG("Calling xpSaiSetSwitchAttrSwitchingMode\n");

    switch (value.s32)
    {
        case SAI_SWITCH_SWITCHING_MODE_CUT_THROUGH:
            byPassMode = XP_TXQ_EQ_BYPASS_MODE_ALL;
            break;

        case SAI_SWITCH_SWITCHING_MODE_STORE_AND_FORWARD:
            byPassMode = XP_TXQ_EQ_BYPASS_MODE_NONE;
            break;

        default:
            return SAI_STATUS_INVALID_PARAMETER;
    }

    xpStatus = xpsGlobalSwitchControlSetTxCutThruMode(devId, byPassMode);

    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Switch Mode could not be set in xpSaiSetSwitchAttrSwitchingMode\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrSwitchingMode

sai_status_t xpSaiGetSwitchAttrSwitchingMode(sai_attribute_value_t* value,
                                             xpsDevice_t devId)
{

    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus    = XP_NO_ERR;
    xpBypassMode_e          byPassMode = XP_TXQ_EQ_BYPASS_MODE_NONE;

    XP_SAI_LOG_DBG("Calling xpSaiGetSwitchAttrSwitchingMode\n");

    xpStatus = xpsGlobalSwitchControlGetTxCutThruMode(devId, &byPassMode);

    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Switch Mode could not be get in xpSaiGetSwitchAttrSwitchingMode\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    switch (byPassMode)
    {
        case XP_TXQ_EQ_BYPASS_MODE_ALL:
            value->s32 = SAI_SWITCH_SWITCHING_MODE_CUT_THROUGH;
            break;

        case XP_TXQ_EQ_BYPASS_MODE_NONE:
            value->s32 = SAI_SWITCH_SWITCHING_MODE_STORE_AND_FORWARD;
            break;

        default:
            XP_SAI_LOG_ERR("Invalid SwitchingMode value retrived\n");
            return SAI_STATUS_INVALID_PARAMETER;
    }

    saiStatus = xpsStatus2SaiStatus(xpStatus);

    return saiStatus;

}

//Func: xpSaiGetSwitchAttrBcastCpuFloodEnable
sai_status_t xpSaiGetSwitchAttrBcastCpuFloodEnable(xpsDevice_t devId,
                                                   bool* value)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer received.\n");
        return SAI_STATUS_FAILURE;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);

    if (pSaiSwitchStaticVar == NULL)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *value = pSaiSwitchStaticVar->bcastCpuFloodEnable;
    return xpsStatus2SaiStatus(status);
}

//Func: xpSaiSetSwitchAttrBcastCpuFloodEnable
sai_status_t xpSaiSetSwitchAttrBcastCpuFloodEnable(xpsDevice_t devId,
                                                   sai_attribute_value_t value)
{
    XP_STATUS           xpsStatus               = XP_NO_ERR;
    sai_status_t        saiRetVal               = SAI_STATUS_SUCCESS;
    sai_int32_t         currentFloodControlType = 0;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar     = NULL;
    xpSaiSwitchEntry_t  key;
    uint32_t            objCount                = 0;
    xpsVlan_t           *vlanIdList;

    memset(&key, 0, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    XP_SAI_LOG_DBG("Calling xpSaiSetSwitchAttrBcastCpuFloodEnable\n");

    xpsStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                   &key, (void**)&pSaiSwitchStaticVar);
    if (xpsStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find switch entry in DB!\n");
        return xpsStatus2SaiStatus(xpsStatus);
    }

    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Could not get switch static variables!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* Update unknownUcCmd/broadcastCmd for each Vlan */
    saiRetVal = xpSaiCountVlanObjects(&objCount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Vlan object count!\n");
        return saiRetVal;
    }

    if (!objCount)
    {
        /* No Vlan objects found - just return with success */
        XP_SAI_LOG_DBG("No Vlan objects found\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    vlanIdList = (xpsVlan_t *)xpMalloc(sizeof(xpsVlan_t) * objCount);
    if (vlanIdList == NULL)
    {
        XP_SAI_LOG_ERR("Could not allocate memory for vlanId array!\n");
        return SAI_STATUS_NO_MEMORY;
    }

    xpsStatus = xpsVlanGetVlanIdList(devId, vlanIdList);
    if (xpsStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get VlanIdList!\n");
        xpFree(vlanIdList);
        return xpsStatus2SaiStatus(xpsStatus);
    }

    for (uint32_t i = 0; i < objCount; i++)
    {
        saiRetVal = xpSaiGetVlanAttrUnknownUnicastFloodControlType(vlanIdList[i],
                                                                   &currentFloodControlType);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not get Unknown unicast flood control type for vlanId = %d!\n",
                           vlanIdList[i]);
            xpFree(vlanIdList);
            return saiRetVal;
        }

        saiRetVal = xpSaiSetVlanAttrUnknownUnicastFloodControlType(vlanIdList[i],
                                                                   currentFloodControlType, value.booldata);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not set UnknownUcCmd for vlanId = %d!\n", vlanIdList[i]);
            xpFree(vlanIdList);
            return saiRetVal;
        }

        saiRetVal = xpSaiGetVlanAttrBroadcastFloodControlType(vlanIdList[i],
                                                              &currentFloodControlType);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not get Broadcast flood control type for vlanId = %d!\n",
                           vlanIdList[i]);
            xpFree(vlanIdList);
            return saiRetVal;
        }

        saiRetVal = xpSaiSetVlanAttrBroadcastFloodControlType(vlanIdList[i],
                                                              currentFloodControlType, value.booldata);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not set bcastCmd for vlanId = %d!\n", vlanIdList[i]);
            xpFree(vlanIdList);
            return saiRetVal;
        }
    }

    XP_SAI_LOG_DBG("Switch Broadcast-cpu flood is %s\n",
                   (value.booldata) ? "enable" : "disable");

    pSaiSwitchStaticVar->bcastCpuFloodEnable = value.booldata;
    xpFree(vlanIdList);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrMcastCpuFloodEnable
sai_status_t xpSaiGetSwitchAttrMcastCpuFloodEnable(xpsDevice_t devId,
                                                   bool* value)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer received.\n");
        return SAI_STATUS_FAILURE;
    }

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);

    if (pSaiSwitchStaticVar == NULL)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *value = pSaiSwitchStaticVar->mcastCpuFloodEnable;
    return xpsStatus2SaiStatus(status);
}

//Func: xpSaiSetSwitchAttrMcastCpuFloodEnable
sai_status_t xpSaiSetSwitchAttrMcastCpuFloodEnable(xpsDevice_t devId,
                                                   sai_attribute_value_t value)
{
    XP_STATUS           xpsStatus           = XP_NO_ERR;
    sai_status_t        saiRetVal           = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar = NULL;
    xpSaiSwitchEntry_t  key;
    uint32_t            objCount            = 0;
    xpsVlan_t           *vlanIdList;
    sai_attribute_value_t tempValue;

    memset(&key, 0, sizeof(key));
    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    XP_SAI_LOG_DBG("Calling xpSaiSetSwitchAttrMcastCpuFloodEnable\n");

    xpsStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                   &key, (void**)&pSaiSwitchStaticVar);
    if (xpsStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find switch entry in DB!\n");
        return xpsStatus2SaiStatus(xpsStatus);
    }

    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Could not get switch static variables!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* Update multicastCmd for each Vlan */
    saiRetVal = xpSaiCountVlanObjects(&objCount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Vlan object count!\n");
        return saiRetVal;
    }

    if (!objCount)
    {
        /* No Vlan objects found - just return with success */
        XP_SAI_LOG_DBG("No Vlan objects found\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    vlanIdList = (xpsVlan_t *)xpMalloc(sizeof(xpsVlan_t) * objCount);
    if (vlanIdList == NULL)
    {
        XP_SAI_LOG_ERR("Could not allocate memory for vlanId array!\n");
        return SAI_STATUS_NO_MEMORY;
    }

    xpsStatus = xpsVlanGetVlanIdList(devId, vlanIdList);
    if (xpsStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get VlanIdList!\n");
        xpFree(vlanIdList);
        return xpsStatus2SaiStatus(xpsStatus);
    }

    for (uint32_t i = 0; i < objCount; i++)
    {
        saiRetVal = xpSaiGetVlanAttrUnknownMulticastFloodControlType(vlanIdList[i],
                                                                     &tempValue.s32);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not get Unknown multicast flood control type for vlanId = %d!\n",
                           vlanIdList[i]);
            xpFree(vlanIdList);
            return saiRetVal;
        }

        saiRetVal = xpSaiSetVlanAttrUnknownMulticastFloodControlType(vlanIdList[i],
                                                                     tempValue.s32, value.booldata);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not set UnregMcCmd for vlanId = %d!\n", vlanIdList[i]);
            xpFree(vlanIdList);
            return saiRetVal;
        }
    }

    /*Multicast flood control to CPU port is enable*/
    if (value.booldata == true)
    {
        XP_SAI_LOG_DBG("Switch multicast-cpu flood is enable\n");
    }
    else
    {
        XP_SAI_LOG_DBG("Switch multicast-cpu flood is disable\n");
    }

    pSaiSwitchStaticVar->mcastCpuFloodEnable = value.booldata;
    xpFree(vlanIdList);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSwitchAttrSrcMacAddress

sai_status_t xpSaiSetSwitchAttrSrcMacAddress(sai_attribute_value_t value)
{
    XP_STATUS           xpStatus = XP_NO_ERR;
    sai_mac_t           mac;
    xpSaiSwitchEntry_t* pSwitchStaticVar = NULL;
    xpSaiSwitchEntry_t  key;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    memset(&key, 0, sizeof(key));
    memcpy(mac, value.mac, sizeof(mac));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                  &key, (void**)&pSwitchStaticVar);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed with xpStatus: %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (pSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Could not find switch static variables entry!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    if (memcmp(pSwitchStaticVar->switchSrcMacAddress, mac, sizeof(sai_mac_t)) == 0)
    {
        XP_SAI_LOG_NOTICE("Existing mac entry!\n");
        return SAI_STATUS_SUCCESS;
    }

    //Remove existing mac
    if (!(xpSaiEthAddrIsZero(pSwitchStaticVar->switchSrcMacAddress)))
    {
        xpStatus = xpsL3RemoveIngressRouterMac(xpSaiGetDevId(),
                                               pSwitchStaticVar->switchSrcMacAddress);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to remove source mac address  %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }

    xpStatus = xpsPortSAMacBaseSet(xpSaiGetDevId(), mac);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsPortSAMacBaseSet Failed :%d \n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /*All validation checks supposed to be done on XPS layer.*/
    xpStatus = xpsL3AddIngressRouterMac(xpSaiGetDevId(), mac);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Switch MAC address could not be set for ingress interface!\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsL3SetEgressRouterMacMSbs(xpSaiGetDevId(), mac);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Switch MAC address could not be set for egress interface!\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    memcpy(pSwitchStaticVar->switchSrcMacAddress, mac, sizeof(sai_mac_t));
    /*Update default Vrf src mac address.*/

    xpStatus = xpSaiVrfMacAddressSet(xpSaiGetDevId(), XP_SAI_VRF_DEFAULT_ID, mac);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfMacAddressSet() failed with error code: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSwitchAttrFdbUnicastMissAction

sai_status_t xpSaiSetSwitchAttrFdbUnicastMissAction(xpsDevice_t devId,
                                                    sai_attribute_value_t value)
{
    XP_STATUS           xpsStatus           = XP_NO_ERR;
    sai_status_t        saiRetVal           = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar = NULL;
    xpSaiSwitchEntry_t  key;

    uint32_t            objCount            = 0;
    xpsVlan_t           *vlanIdList;
    xpsPktCmd_e         unknownUcCmd;
    xpsInterfaceType_e intfType;

    memset(&key, 0, sizeof(key));
    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpsStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                   &key, (void**)&pSaiSwitchStaticVar);
    if (xpsStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find switch entry in DB!\n");
        return xpsStatus2SaiStatus(xpsStatus);
    }

    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Could not get switch static variables!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (pSaiSwitchStaticVar->fdbUnicastMissAction == (sai_packet_action_t)value.s32)
    {
        XP_SAI_LOG_NOTICE("No Change in Action.\n");
        return SAI_STATUS_SUCCESS;
    }

    if (((sai_packet_action_t)value.s32 != SAI_PACKET_ACTION_FORWARD) &&
        ((sai_packet_action_t)value.s32 != SAI_PACKET_ACTION_DROP))
    {
        XP_SAI_LOG_NOTICE("Only Forward/Drop action supported\n");
        return SAI_STATUS_SUCCESS;
    }

    /* Update unknownUcCmd for each Vlan */
    saiRetVal = xpSaiCountVlanObjects(&objCount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Vlan object count!\n");
        return saiRetVal;
    }

    if (!objCount)
    {
        /* No Vlan objects found - just return with success */
        return SAI_STATUS_SUCCESS;
    }

    vlanIdList = (xpsVlan_t *)xpMalloc(sizeof(xpsVlan_t) * objCount);
    if (vlanIdList == NULL)
    {
        XP_SAI_LOG_ERR("Could not allocate memory for vlanId array!\n");
        return SAI_STATUS_NO_MEMORY;
    }

    xpsStatus = xpsVlanGetVlanIdList(xpSaiGetDevId(), vlanIdList);
    if (xpsStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get VlanIdList!\n");
        xpFree(vlanIdList);
        return xpsStatus2SaiStatus(xpsStatus);
    }

    saiRetVal = xpSaiConvertSaiPacketAction2xps((sai_packet_action_t)value.s32,
                                                &unknownUcCmd);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertSaiPacketAction2xps() failed!\n");
        xpFree(vlanIdList);
        return saiRetVal;
    }

    for (uint32_t i = 0; i < objCount; i++)
    {
        /*Skip standard Vlan*/
        if (vlanIdList[i] <=1 || vlanIdList[i] >= XPS_L3_RESERVED_ROUTER_VLAN(devId))
        {
            continue;
        }

        /*If L3 Enabled, ie VLAN_ROUTER, skip this. */
        xpsStatus = xpsInterfaceGetType(XPS_INTF_MAP_BD_TO_INTF(vlanIdList[i]),
                                        &intfType);
        if ((xpsStatus != XP_NO_ERR) && (intfType == XPS_VLAN_ROUTER))
        {
            continue;
        }

        xpsStatus = xpsVlanSetUnknownUcCmd(devId, vlanIdList[i], unknownUcCmd);
        if (xpsStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not set UnknownUcCmd for vlanId = %d!\n", vlanIdList[i]);
            xpFree(vlanIdList);
            return xpsStatus2SaiStatus(xpsStatus);
        }
    }

    if ((unknownUcCmd == XP_PKTCMD_DROP) ||
        (unknownUcCmd == XP_PKTCMD_FWD))
    {
        saiRetVal = xpSaiUpdateNbr(devId, unknownUcCmd);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiUpdateNbr failed! :%d \n", saiRetVal);
            xpFree(vlanIdList);
            return saiRetVal;
        }
    }

    pSaiSwitchStaticVar->fdbUnicastMissAction = (sai_packet_action_t)value.s32;

    xpFree(vlanIdList);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrFdbUnicastMissAction

sai_status_t xpSaiGetSwitchAttrFdbUnicastMissAction(sai_attribute_value_t
                                                    *value)
{
    XP_STATUS           status              = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar = NULL;
    xpSaiSwitchEntry_t  key;

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Invalid parameters received!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&key, 0, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find switch entry in DB!\n");
        return xpsStatus2SaiStatus(status);
    }

    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Could not get switch static variables!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    value->s32 = (sai_int32_t)pSaiSwitchStaticVar->fdbUnicastMissAction;

    return xpsStatus2SaiStatus(status);
}


//Func: xpSaiSetSwitchAttrFdbBroadcastMissAction

sai_status_t xpSaiSetSwitchAttrFdbBroadcastMissAction(sai_attribute_value_t
                                                      value)
{
    sai_status_t retVal = SAI_STATUS_ATTR_NOT_SUPPORTED_0;

    XP_SAI_LOG_DBG("Calling xpSaiSetSwitchAttrFdbBroadcastMissAction\n");

    return retVal;
}


//Func: xpSaiSetSwitchAttrFdbMulticastMissAction

sai_status_t xpSaiSetSwitchAttrFdbMulticastMissAction(sai_attribute_value_t
                                                      value)
{
    sai_status_t retVal = SAI_STATUS_ATTR_NOT_SUPPORTED_0;

    XP_SAI_LOG_DBG("Calling xpSaiSetSwitchAttrFdbMulticastMissAction\n");

    return retVal;
}


//Func: xpSaiSetSwitchAttrHashAlgo

sai_status_t xpSaiSetSwitchAttrHashAlgo(sai_int32_t newAlgo, bool isLag)
{
    XP_STATUS status = XP_NO_ERR;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpsDevice_t devId =  xpSaiGetDevId();
    sai_int32_t curAlgo;
    GT_U32 hashSeed = 0;
    bool randomEnable = false;
    sai_object_id_t hashoid = SAI_NULL_OBJECT_ID;

    saiStatus = xpSaiSwitchHashAlgorithmGet(&curAlgo, isLag);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get Hash Algorithm");
        return saiStatus;
    }
    if (curAlgo == newAlgo)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (isLag)
    {
        saiStatus =  xpSaiGetSwitchAttrLagHashSeed(&hashSeed);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("get seed failed");
            return saiStatus;
        }
    }
    else
    {
        saiStatus =  xpSaiGetSwitchAttrEcmpHashSeed(&hashSeed);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("get seed failed");
            return saiStatus;
        }
    }

    saiStatus = xpSaiSwitchHashGet(XPSAI_L2_HASH_OBJECT_INDEX, &hashoid);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get default hash id\n");
        return saiStatus;
    }

    if (newAlgo == (sai_int32_t)SAI_HASH_ALGORITHM_RANDOM)
    {
        /*bit selection still take effect on random hash, the randon vlaue is 16 bits*/
        status = xpsLagSetHashBit(devId, XPS_LAG_HASH_ALGO_CRC_32LO_E, isLag);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not hashSeedset Hash Algorithm");
            return xpsStatus2SaiStatus(status);
        }
    }
    else if (newAlgo == (sai_int32_t)SAI_HASH_ALGORITHM_XOR)
    {
        /*if the old algo is CRC, there is no setting for XOR field, so we set XOR field again here.
        if we change algo from CRC to XOR, we should change the hash field firstly.
        For example, CRC field is (etherType), it can not be the field of XOR.
        we should change hash field to (dip,sip) or other fields that XOR supported firstly,
        and then change the algo to XOR. If not, it will thrown an error */
        saiStatus = xpSaiHashFieldSet(XPSAI_L2_HASH_OBJECT_INDEX, hashoid, true, false);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to Set HasField for XOR of default HASH, please set the hash field firstly\n");
            return saiStatus;
        }

        status =  xpsLagSetHashAlgo(devId, XPS_LAG_HASH_ALGO_XOR_E, 0);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not set Hash Algorithm");
            return xpsStatus2SaiStatus(status);
        }
    }
    else if ((newAlgo == (sai_int32_t)SAI_HASH_ALGORITHM_CRC) ||
             (newAlgo == (sai_int32_t)SAI_HASH_ALGORITHM_CRC_32LO)
             || (newAlgo == (sai_int32_t)SAI_HASH_ALGORITHM_CRC_32HI))
    {
        /*if the old algo is XOR, there is no setting for CRC field, so we set CRC field again here*/
        saiStatus = xpSaiHashFieldSet(XPSAI_L2_HASH_OBJECT_INDEX, hashoid, false, true);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to Set HasField for CRC of default HASH\n");
            return saiStatus;
        }

        status =  xpsLagSetHashAlgo(devId, XPS_LAG_HASH_ALGO_CRC_E, hashSeed);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not set Hash Algorithm");
            return xpsStatus2SaiStatus(status);
        }

        if (newAlgo == (sai_int32_t)SAI_HASH_ALGORITHM_CRC_32HI)
        {
            status = xpsLagSetHashBit(devId, XPS_LAG_HASH_ALGO_CRC_32HI_E, isLag);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not set Hash Algorithm");
                return xpsStatus2SaiStatus(status);
            }
        }
        else if (newAlgo == (sai_int32_t)SAI_HASH_ALGORITHM_CRC_32LO)
        {
            status = xpsLagSetHashBit(devId, XPS_LAG_HASH_ALGO_CRC_32LO_E, isLag);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not hashSeedset Hash Algorithm");
                return xpsStatus2SaiStatus(status);
            }
        }
    }
    else
    {
        XP_SAI_LOG_ERR("Hash ALgorithm not supported");
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    /*random turn to non-random, non-random turn to random ,all need to rewrite LAG or ECMP Tables*/
    if ((curAlgo != SAI_HASH_ALGORITHM_RANDOM) &&
        (newAlgo == SAI_HASH_ALGORITHM_RANDOM))
    {
        randomEnable = true;
        if (isLag)
        {
            /*only if we have a non-zero hashSeed, set it in hardware*/
            if (hashSeed != 0)
            {
                status = xpsLagSetRandomHashSeed(devId, hashSeed);
                if (status != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Could not set Lag random hash seed");
                    return xpsStatus2SaiStatus(status);
                }
            }
            else
            {
                /* we still want to allow Algo change*/
                XP_SAI_LOG_WARNING("For random Algo, hashseed has to be non-zero\n");
            }
        }
        else
        {
            /*only if we have a non-zero hashSeed, set it in hardware*/
            if (hashSeed != 0)
            {
                status = xpsL3SetRandomHashSeed(devId, hashSeed);
                if (status != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Could not set ECMP random hash seed");
                    return xpsStatus2SaiStatus(status);
                }
            }
            else
            {
                /* we still want to allow Algo change*/
                XP_SAI_LOG_WARNING("For random Algo, hashseed has to be non-zero\n");
            }
        }
    }
    if ((curAlgo == SAI_HASH_ALGORITHM_RANDOM) &&
        (newAlgo != SAI_HASH_ALGORITHM_RANDOM))
    {
        randomEnable = false;
    }
    if ((curAlgo == SAI_HASH_ALGORITHM_RANDOM) ||
        (newAlgo == SAI_HASH_ALGORITHM_RANDOM))
    {
        /*need rewrite LAG or ECMP Tables*/
        if (isLag)
        {
            status = xpsLagRandomEnableRewrite(devId, randomEnable);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not rewrite LagRandomEnable");
                return xpsStatus2SaiStatus(status);
            }
        }
        else
        {
            status = xpsL3RandomEnableRewrite(devId, randomEnable);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not rewrite L3EcmpRandomEnable");
                return xpsStatus2SaiStatus(status);
            }
        }
    }

    /*since lag and ecmp use the same hash mask table, when the global hash mode change from CRC/32H/32L to XOR
    , or from XOR to CRC/32H/32L, both lag and ecmp hash algorithm will change*/
    sai_int32_t anotherHashAlgo;
    saiStatus = xpSaiSwitchHashAlgorithmGet(&anotherHashAlgo, !isLag);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get Hash Algorithm from DB");
        return saiStatus;
    }
    if (anotherHashAlgo == SAI_HASH_ALGORITHM_XOR)
    {
        if ((newAlgo == SAI_HASH_ALGORITHM_CRC) ||
            (newAlgo == SAI_HASH_ALGORITHM_CRC_32LO) ||
            (newAlgo == SAI_HASH_ALGORITHM_CRC_32HI))
        {
            anotherHashAlgo = SAI_HASH_ALGORITHM_CRC;
        }
    }
    if (newAlgo == SAI_HASH_ALGORITHM_XOR)
    {
        if ((anotherHashAlgo == SAI_HASH_ALGORITHM_CRC) ||
            (anotherHashAlgo == SAI_HASH_ALGORITHM_CRC_32LO) ||
            (anotherHashAlgo == SAI_HASH_ALGORITHM_CRC_32HI))
        {
            anotherHashAlgo = SAI_HASH_ALGORITHM_XOR;
        }
    }

    saiStatus = xpSaiSwitchHashAlgorithmSet(newAlgo, isLag);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not set Hash Algorithm to DB");
        return saiStatus;
    }

    saiStatus = xpSaiSwitchHashAlgorithmSet(anotherHashAlgo, !isLag);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not set Hash Algorithm to DB");
        return saiStatus;
    }

    return xpsStatus2SaiStatus(status);
}


/**
 * \brief Gets SAI Symmetric Hash status from DB
 *
 * This API is used as a method to get SAI Symmetric Hash status from DB
 *
 * \param [in] sai_object_id_t switchId
 * \param [in] bool* pEnable
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetSwitchAttrSymmetricHash(sai_object_id_t switchId,
                                             bool *pEnable)
{
    XP_SAI_LOG_DBG("Calling xpSaiGetSwitchAttrSymmetricHash\n");

    return xpSaiSwitchSymmetricHashStatusGet(pEnable);
}

/**
 * \brief Sets SAI Symmetric Hash status to HW
 *
 * This API is used as a method to enable/disable SAI Symmetric Hash on HW
 *
 * \param [in] sai_object_id_t switchId
 * \param [in] bool enable
 *
 * \return sai_status_t
 */
sai_status_t xpSaiSetSwitchAttrSymmetricHash(sai_object_id_t switchId,
                                             bool enable)
{
    sai_status_t  saiRetVal = SAI_STATUS_SUCCESS;
    bool          isEnabled = false;

    XP_SAI_LOG_DBG("Calling xpSaiSetSwitchAttrSymmetricHash\n");

    saiRetVal = xpSaiSwitchSymmetricHashStatusGet(&isEnabled);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }

    if (isEnabled == enable)
    {
        return SAI_STATUS_SUCCESS;
    }

    saiRetVal = xpSaiSwitchSymmetricHashStatusSet(enable);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }

    /*reConfigure hash mask tables*/
    saiRetVal = xpSaiHashReconfigueHashMaskTables();
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiHashReconfigueHashMaskTables() failed!\n");
        return saiRetVal;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiSetSwitchAttrEcmpHashSeed(sai_object_id_t switchId,
                                            uint32_t seed)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;
    xpsDevice_t devId =  xpSaiGetDevId();
    bool isLag = false;
    sai_int32_t curAlgo;
    sai_status_t  saiStatus = SAI_STATUS_SUCCESS;

    saiStatus = xpSaiSwitchHashAlgorithmGet(&curAlgo, isLag);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get Hash Algorithm");
        return saiStatus;
    }

    if (curAlgo == (sai_int32_t)SAI_HASH_ALGORITHM_RANDOM)
    {
        /*only if we have a non-zero hashSeed, set it in hardware*/
        if (seed != 0)
        {
            status = xpsL3SetRandomHashSeed(devId, seed);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Could not set Lag random hash seed");
                return xpsStatus2SaiStatus(status);
            }
        }
        else
        {
            /* we still want to allow Algo change*/
            XP_SAI_LOG_WARNING("For random Algo, hashseed has to be non-zero\n");
        }
    }
    else
    {
        status = xpsLagSetHashSeed(devId, seed);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsLagSetHashSeed failed devId %d \n", devId);
            return xpsStatus2SaiStatus(status);
        }
    }

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to set ECMP hash seed to DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    pSaiSwitchStaticVar->ecmpHashSeed = seed;
    pSaiSwitchStaticVar->lagHashSeed = seed;

    return xpsStatus2SaiStatus(status);
}

//Func: xpSaiSetSwitchAttrQosDefaultTc

static sai_status_t xpSaiSetSwitchAttrQosDefaultTc(sai_object_id_t switchId,
                                                   sai_uint8_t tc)
{
    xpSaiSwitchEntry_t  entry;
    xpSaiSwitchEntry_t *entryPtr     = NULL;
    XP_STATUS           xpStatus     = XP_NO_ERR;
    xpsDevice_t         xpsDevId     = 0;
    xpsDevice_t         xpsIntfDevId = 0;
    xpsInterfaceId_t    xpsIntf      = XPS_INTF_INVALID_ID;
    xpsInterfaceId_t    cpuIfId      = XPS_INTF_INVALID_ID;
    sai_object_id_t     portId       = SAI_NULL_OBJECT_ID;
    sai_status_t        saiStatus    = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Args: switchId = %" PRIu64 " tc = %" PRIu8 "\n", switchId, tc);
    memset(&entry, 0, sizeof(entry));

    entry.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                  &entry, (void**)&entryPtr);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %" PRIi32 "!\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (entryPtr == NULL)
    {
        XP_SAI_LOG_ERR("Could not find switch static variables entry!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* Get xpsDevId from OID switchId */
    xpsDevId = xpSaiObjIdSwitchGet(switchId);

    xpStatus = xpsPortGetCPUPortIntfId(xpsDevId, &cpuIfId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get interface ID of CPU port, error: %" PRIi32 "\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Before iteration, retrieve first item */
    xpStatus = xpsPortGetFirst(&xpsIntf);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("xpsPortGetFirst call failed! xpsIntf = %" PRIu32 "\n", xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    sai_attribute_value_t value;
    value.u8 = tc;

    /* For each port update Default TC */
    do
    {
        /* Check if port is active. Mandatory step. */
        if (!xpSaiIsPortActive(xpsIntf) ||
            (xpsIntf == cpuIfId))
        {
            continue;
        }

        /* Multidevice related */
        /* Check if xpsDevId from switchId is the same as xpsIntfDevId. */
        if (memcmp(&xpsDevId, &xpsIntfDevId, sizeof(xpsDevice_t)) == 0)
        {
            saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, xpsDevId, xpsIntf, &portId);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("SAI port object could not be created, sai_error: %" PRIi32 "\n",
                               saiStatus);
                return saiStatus;
            }

            saiStatus = xpSaiSetPortAttrQosDefaultTc(portId, value);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiSetPortAttrQosDefaultTc failed with an error: %" PRIi32
                               "\n", saiStatus);
                return saiStatus;
            }
        }
        /* Update next interface */
    }
    while (xpsPortGetNext(xpsIntf, &xpsIntf) == XP_NO_ERR);

    entryPtr->defaultTc = tc;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSwitchUpdateQosMap

static sai_status_t xpSaiSwitchUpdateQosMap(sai_object_id_t switchId,
                                            sai_attribute_value_t mapValue, int32_t mapType)
{
    xpsDevice_t         xpsDevId     = 0;
    xpsDevice_t         xpsIntfDevId = 0;
    xpsPort_t           xpsPort      = 0;
    XP_STATUS           xpStatus     = XP_NO_ERR;
    xpsInterfaceId_t    xpsIntf      = XPS_INTF_INVALID_ID;
    xpsInterfaceId_t    cpuIfId      = XPS_INTF_INVALID_ID;
    sai_status_t        saiStatus    = SAI_STATUS_SUCCESS;
    sai_object_id_t     portId       = SAI_NULL_OBJECT_ID;
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;

    xpStatus = xpsPortGetCPUPortIntfId(xpsDevId, &cpuIfId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get interface ID of CPU port, error: %" PRIi32 "\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Before iteration, retrieve first item */
    xpStatus = xpsPortGetFirst(&xpsIntf);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsPortGetFirst call failed! xpsIntf = %" PRIu32
                       ", xpStatus = %" PRIi32 "\n", xpsIntf, xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Get xpsDevId from OID switchId */
    xpsDevId = xpSaiObjIdSwitchGet(switchId);

    /* For each port update mapId */
    do
    {
        /* Check if port is active and check if it is not CPU interface. */
        if (!xpSaiIsPortActive(xpsIntf) ||
            (xpsIntf == cpuIfId))
        {
            continue;
        }

        /* Fetch device and port information for this particular port interface */
        xpStatus = xpsPortGetDevAndPortNumFromIntf(xpsIntf, &xpsIntfDevId, &xpsPort);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Fetch device and port information failed, interface id(%" PRIu32
                           ")", xpsPort);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* Multidevice related */
        /* Check if xpsDevId from switchId is the same as xpsIntfDevId. */
        if (memcmp(&xpsDevId, &xpsIntfDevId, sizeof(xpsDevice_t)) == 0)
        {
            saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, xpsDevId, xpsPort, &portId);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("SAI port object could not be created, sai_error: %" PRIi32 "\n",
                               saiStatus);
                return saiStatus;
            }

            /* Get port qos state data */
            xpStatus = xpSaiGetPortQosInfo(portId, &pPortQosInfo);
            if (xpStatus != XP_NO_ERR)
            {
                continue;
            }

            switch (mapType)
            {
                case (SAI_QOS_MAP_TYPE_DOT1P_TO_TC):
                    {
                        saiStatus = xpSaiSetPortAttrQosDot1pToTcMap(portId, mapValue);
                        if (saiStatus != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("xpSaiSetPortAttrQosDot1pToTcMap failed with an error: %" PRIi32
                                           ", portId: %" PRIu64 ", mapValue: %" PRIu64 "\n", saiStatus, portId,
                                           mapValue.oid);
                            return saiStatus;
                        }
                        break;
                    }
                case (SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR):
                    {
                        saiStatus = xpSaiSetPortAttrQosDot1pToColorMap(portId, mapValue);
                        if (saiStatus != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("xpSaiSetPortAttrQosDot1pToColorMap failed with an error: %"
                                           PRIi32 ", portId: %" PRIu64 ", mapValue: %" PRIu64 "\n", saiStatus, portId,
                                           mapValue.oid);
                            return saiStatus;
                        }
                        break;
                    }
                case (SAI_QOS_MAP_TYPE_DSCP_TO_TC):
                    {
                        saiStatus = xpSaiSetPortAttrQosDscpToTcMap(portId, mapValue);
                        if (saiStatus != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("xpSaiSetPortAttrQosDscpToTcMap failed with an error: %" PRIi32
                                           ", portId: %" PRIu64 ", mapValue: %" PRIu64 "\n", saiStatus, portId,
                                           mapValue.oid);
                            return saiStatus;
                        }
                        break;
                    }
                case (SAI_QOS_MAP_TYPE_DSCP_TO_COLOR):
                    {
                        saiStatus = xpSaiSetPortAttrQosDscpToColorMap(portId, mapValue);
                        if (saiStatus != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("xpSaiSetPortAttrQosDscpToColorMap failed with an error: %"
                                           PRIi32 ", portId: %" PRIu64 ", mapValue: %" PRIu64 "\n", saiStatus, portId,
                                           mapValue.oid);
                            return saiStatus;
                        }
                        break;
                    }
                case (SAI_QOS_MAP_TYPE_TC_TO_QUEUE):
                    {
                        saiStatus = xpSaiSetPortAttrQosTcToQueueMap(portId, mapValue);
                        if (saiStatus != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("xpSaiSetPortAttrQosTcToQueueMap failed with an error: %" PRIi32
                                           ", portId: %" PRIu64 ", mapValue: %" PRIu64 "\n", saiStatus, portId,
                                           mapValue.oid);
                            return saiStatus;
                        }
                        break;
                    }
                case (SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P):
                    {
                        saiStatus = xpSaiSetPortAttrQosTcAndColorToDot1pMap(portId, mapValue);
                        if (saiStatus != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("xpSaiSetPortAttrQosTcAndColorToDot1pMap failed with an error: %"
                                           PRIi32 ", portId: %" PRIu64 ", mapValue: %" PRIu64 "\n", saiStatus, portId,
                                           mapValue.oid);
                            return saiStatus;
                        }
                        break;
                    }
                case (SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP):
                    {
                        saiStatus = xpSaiSetPortAttrQosTcAndColorToDscpMap(portId, mapValue);
                        if (saiStatus != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("xpSaiSetPortAttrQosTcAndColorToDscpMap failed with an error: %"
                                           PRIi32 ", portId: %" PRIu64 ", mapValue: %" PRIu64 "\n", saiStatus, portId,
                                           mapValue.oid);
                            return saiStatus;
                        }
                        break;
                    }
                default:
                    {
                        XP_SAI_LOG_ERR("Unknown QoS Map type! mapType: %" PRIi32 "\n", mapType);
                        return SAI_STATUS_FAILURE;
                    }
            }
        }
        /* Update next interface */
    }
    while (xpsPortGetNext(xpsIntf, &xpsIntf) == XP_NO_ERR);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSwitchGetEntry

static sai_status_t xpSaiSwitchGetEntry(sai_object_id_t switchId,
                                        xpSaiSwitchEntry_t **entryPtr)
{
    xpSaiSwitchEntry_t  entry;
    XP_STATUS           xpStatus     = XP_NO_ERR;

    /* Multidevice related */
    //TODO: switchId should be stored on StateDB
    (void) switchId;

    memset(&entry, 0, sizeof(entry));

    entry.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                  &entry, (void**)entryPtr);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %" PRIi32 "!\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (*entryPtr == NULL)
    {
        XP_SAI_LOG_ERR("Could not find switch static variables entry!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    memset(&((*entryPtr)->qosMapIds), 0, sizeof(xpSaiSwitchQoSMapIds_t));
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSwitchAttrQosDot1pToTcMap

static sai_status_t xpSaiSetSwitchAttrQosDot1pToTcMap(sai_object_id_t switchId,
                                                      sai_attribute_value_t mapValue)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t *entryPtr    = NULL;

    XP_SAI_LOG_DBG("Args: switchId = %" PRIu64 " tc = %" PRIu64 "\n", switchId,
                   mapValue.oid);

    saiStatus = xpSaiSwitchUpdateQosMap(switchId, mapValue,
                                        SAI_QOS_MAP_TYPE_DOT1P_TO_TC);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update QoS Map on Switch, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiSwitchGetEntry(switchId, &entryPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get switch entry, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    entryPtr->qosMapIds.dot1pToTcMapId = mapValue.oid;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrQosDot1pToTcMap

static sai_status_t xpSaiGetSwitchAttrQosDot1pToTcMap(sai_object_id_t switchId,
                                                      sai_object_id_t *mapId)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t *entryPtr    = NULL;

    if (mapId == NULL)
    {
        XP_SAI_LOG_ERR("mapId is NULL!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiStatus = xpSaiSwitchGetEntry(switchId, &entryPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get switch entry, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    *mapId = entryPtr->qosMapIds.dot1pToTcMapId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSwitchAttrQosDot1pToColorMap

static sai_status_t xpSaiSetSwitchAttrQosDot1pToColorMap(
    sai_object_id_t switchId, sai_attribute_value_t mapValue)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t *entryPtr    = NULL;

    XP_SAI_LOG_DBG("Args: switchId = %" PRIu64 " tc = %" PRIu64 "\n", switchId,
                   mapValue.oid);

    saiStatus = xpSaiSwitchUpdateQosMap(switchId, mapValue,
                                        SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update QoS Map on Switch, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiSwitchGetEntry(switchId, &entryPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get switch entry, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    entryPtr->qosMapIds.dot1pToColorMapId = mapValue.oid;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrQosDot1pToColorMap

static sai_status_t xpSaiGetSwitchAttrQosDot1pToColorMap(
    sai_object_id_t switchId, sai_object_id_t *mapId)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t *entryPtr    = NULL;

    if (mapId == NULL)
    {
        XP_SAI_LOG_ERR("mapId is NULL!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiStatus = xpSaiSwitchGetEntry(switchId, &entryPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get switch entry, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    *mapId = entryPtr->qosMapIds.dot1pToColorMapId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSwitchAttrQosDscpToTcMap

static sai_status_t xpSaiSetSwitchAttrQosDscpToTcMap(sai_object_id_t switchId,
                                                     sai_attribute_value_t mapValue)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t *entryPtr    = NULL;

    XP_SAI_LOG_DBG("Args: switchId = %" PRIu64 " tc = %" PRIu64 "\n", switchId,
                   mapValue.oid);

    saiStatus = xpSaiSwitchUpdateQosMap(switchId, mapValue,
                                        SAI_QOS_MAP_TYPE_DSCP_TO_TC);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update QoS Map on Switch, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiSwitchGetEntry(switchId, &entryPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get switch entry, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    entryPtr->qosMapIds.dscpToTcMapId = mapValue.oid;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrQosDscpToTcMap

static sai_status_t xpSaiGetSwitchAttrQosDscpToTcMap(sai_object_id_t switchId,
                                                     sai_object_id_t *mapId)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t *entryPtr    = NULL;

    if (mapId == NULL)
    {
        XP_SAI_LOG_ERR("mapId is NULL!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiStatus = xpSaiSwitchGetEntry(switchId, &entryPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get switch entry, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    *mapId = entryPtr->qosMapIds.dscpToTcMapId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSwitchAttrQosDscpToColorMap

static sai_status_t xpSaiSetSwitchAttrQosDscpToColorMap(
    sai_object_id_t switchId, sai_attribute_value_t mapValue)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t *entryPtr    = NULL;

    XP_SAI_LOG_DBG("Args: switchId = %" PRIu64 " tc = %" PRIu64 "\n", switchId,
                   mapValue.oid);

    saiStatus = xpSaiSwitchUpdateQosMap(switchId, mapValue,
                                        SAI_QOS_MAP_TYPE_DSCP_TO_COLOR);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update QoS Map on Switch, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiSwitchGetEntry(switchId, &entryPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get switch entry, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    entryPtr->qosMapIds.dscpToColorMapId = mapValue.oid;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrQosDscpToColorMap

static sai_status_t xpSaiGetSwitchAttrQosDscpToColorMap(
    sai_object_id_t switchId, sai_object_id_t *mapId)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t *entryPtr    = NULL;

    if (mapId == NULL)
    {
        XP_SAI_LOG_ERR("mapId is NULL!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiStatus = xpSaiSwitchGetEntry(switchId, &entryPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get switch entry, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    *mapId = entryPtr->qosMapIds.dscpToColorMapId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSwitchAttrQosTcToQueueMap

static sai_status_t xpSaiSetSwitchAttrQosTcToQueueMap(sai_object_id_t switchId,
                                                      sai_attribute_value_t mapValue)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t *entryPtr    = NULL;

    XP_SAI_LOG_DBG("Args: switchId = %" PRIu64 " tc = %" PRIu64 "\n", switchId,
                   mapValue.oid);

    saiStatus = xpSaiSwitchUpdateQosMap(switchId, mapValue,
                                        SAI_QOS_MAP_TYPE_TC_TO_QUEUE);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update QoS Map on Switch, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiSwitchGetEntry(switchId, &entryPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get switch entry, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    entryPtr->qosMapIds.tcToQueueMapId = mapValue.oid;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrQosTcToQueueMap

static sai_status_t xpSaiGetSwitchAttrQosTcToQueueMap(sai_object_id_t switchId,
                                                      sai_object_id_t *mapId)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t *entryPtr    = NULL;

    if (mapId == NULL)
    {
        XP_SAI_LOG_ERR("mapId is NULL!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiStatus = xpSaiSwitchGetEntry(switchId, &entryPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get switch entry, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    *mapId = entryPtr->qosMapIds.tcToQueueMapId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSwitchAttrQosTcAndColorToDot1pMap

static sai_status_t xpSaiSetSwitchAttrQosTcAndColorToDot1pMap(
    sai_object_id_t switchId, sai_attribute_value_t mapValue)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t *entryPtr    = NULL;

    XP_SAI_LOG_DBG("Args: switchId = %" PRIu64 " tc = %" PRIu64 "\n", switchId,
                   mapValue.oid);

    saiStatus = xpSaiSwitchUpdateQosMap(switchId, mapValue,
                                        SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update QoS Map on Switch, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiSwitchGetEntry(switchId, &entryPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get switch entry, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    entryPtr->qosMapIds.tcAndColorToDot1pMapId = mapValue.oid;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrQosTcAndColorToDot1pMap

static sai_status_t xpSaiGetSwitchAttrQosTcAndColorToDot1pMap(
    sai_object_id_t switchId, sai_object_id_t *mapId)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t *entryPtr    = NULL;

    if (mapId == NULL)
    {
        XP_SAI_LOG_ERR("mapId is NULL!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiStatus = xpSaiSwitchGetEntry(switchId, &entryPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get switch entry, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    *mapId = entryPtr->qosMapIds.tcAndColorToDot1pMapId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSwitchAttrQosTcAndColorToDscpMap

static sai_status_t xpSaiSetSwitchAttrQosTcAndColorToDscpMap(
    sai_object_id_t switchId, sai_attribute_value_t mapValue)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t *entryPtr    = NULL;

    XP_SAI_LOG_DBG("Args: switchId = %" PRIu64 " tc = %" PRIu64 "\n", switchId,
                   mapValue.oid);

    saiStatus = xpSaiSwitchUpdateQosMap(switchId, mapValue,
                                        SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update QoS Map on Switch, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiSwitchGetEntry(switchId, &entryPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get switch entry, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    entryPtr->qosMapIds.tcAndColorToDscpMapId = mapValue.oid;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrQosTcAndColorToDscpMap

static sai_status_t xpSaiGetSwitchAttrQosTcAndColorToDscpMap(
    sai_object_id_t switchId, sai_object_id_t *mapId)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    xpSaiSwitchEntry_t *entryPtr    = NULL;

    if (mapId == NULL)
    {
        XP_SAI_LOG_ERR("mapId is NULL!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiStatus = xpSaiSwitchGetEntry(switchId, &entryPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get switch entry, saiStatus = %" PRIi32 "\n",
                       saiStatus);
        return saiStatus;
    }

    *mapId = entryPtr->qosMapIds.tcAndColorToDscpMapId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrQosDefaultTc

static sai_status_t xpSaiGetSwitchAttrQosDefaultTc(sai_object_id_t switchId,
                                                   sai_uint8_t *tc)
{
    xpSaiSwitchEntry_t  entry;
    xpSaiSwitchEntry_t *entryPtr = NULL;
    XP_STATUS           xpStatus = XP_NO_ERR;

    XP_SAI_LOG_DBG("Args: switchId = %" PRIu64 "\n", switchId);

    if (tc == NULL)
    {
        XP_SAI_LOG_ERR("tc is NULL! switchId = %" PRIu64 "\n", switchId);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&entry, 0, sizeof(entry));

    entry.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                  &entry, (void**)&entryPtr);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %" PRIi32 "!\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (entryPtr == NULL)
    {
        XP_SAI_LOG_ERR("Could not find switch static variables entry!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *tc = entryPtr->defaultTc;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrQosMaxNumberOfTrafficClasses

sai_status_t xpSaiGetSwitchAttrQosMaxNumberOfTrafficClasses(
    sai_attribute_value_t* value)
{
    value->u32 = XPSAI_QOSMAP_MAX_TC_VALUE;
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrQosMaxNumberOfSchedulerGroupHierarchyLevels

sai_status_t xpSaiGetSwitchAttrQosMaxNumberOfSchedulerGroupHierarchyLevels(
    sai_object_id_t switch_id, sai_attribute_value_t* value)
{
    /*
     * Level-0: Port scheduler group, schedules queues
     */
    value->u32 = XPSAI_SG_NUM_HIERARCHY_LEVELS;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrQosMaxNumberOfSchedulerGroupsPerHierarchyLevel

sai_status_t xpSaiGetSwitchAttrQosMaxNumberOfSchedulerGroupsPerHierarchyLevel(
    sai_object_id_t switch_id, sai_attribute_value_t* value)
{
    if (value->s32list.count < XPSAI_SG_NUM_HIERARCHY_LEVELS)
    {
        XP_SAI_LOG_ERR("Unable to fit %u OIDs into the buffer with size %u\n",
                       XPSAI_SG_NUM_HIERARCHY_LEVELS, value->s32list.count);
        value->s32list.count = XPSAI_SG_NUM_HIERARCHY_LEVELS;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    value->s32list.count = XPSAI_SG_NUM_HIERARCHY_LEVELS;

    /* Populate max number of scheduler groups per hierarchy level */
    value->s32list.list[0] =
        XPSAI_SG_MAX_CHILDS_AT_LEVEL0;   /* Max number of scheduler groups at Level-0 (Port) */
    value->s32list.list[1] =
        XPSAI_SG_MAX_CHILDS_AT_LEVEL1;   /* Max number of scheduler groups at Level-1 */

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrQosMaxNumberOfChildsPerSchedulerGroup

sai_status_t xpSaiGetSwitchAttrQosMaxNumberOfChildsPerSchedulerGroup(
    sai_object_id_t switch_id, sai_attribute_value_t* value)
{
    /* Each level supports different max number of childs supported per scheudler group.
     * As this attribute is not a list, returning maximum number of childs supported at Level-0 */
    value->u32 = XPSAI_SG_MAX_CHILDS_AT_LEVEL0;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrTotalBufferSize

sai_status_t xpSaiGetSwitchAttrTotalBufferSize(sai_attribute_value_t* value)
{
    value->u64 = xpSaiBufferGetPacketMemorySize();

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrIngressBufferPoolNum

sai_status_t xpSaiGetSwitchAttrIngressBufferPoolNum(xpsDevice_t devId,
                                                    sai_attribute_value_t* value)
{
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSwitchAttrEgressBufferPoolNum

sai_status_t xpSaiGetSwitchAttrEgressBufferPoolNum(xpsDevice_t devId,
                                                   sai_attribute_value_t* value)
{
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiAcmProfileTypeGet

sai_status_t xpSaiAcmProfileTypeGet(XP_PROFILE_TYPE profileType,
                                    uint8_t withBillingCntr, XP_ACM_PROFILE_TYPE *acmProfileType)
{
    /*
     * Some pipeline profiles has two ACM profile. One supporting billing counters and
     * another which doesnt. Choose based on the flag passed.
     */
    if (withBillingCntr)
    {
        switch (profileType)
        {
            case XP_DEFAULT_SINGLE_PIPE_PROFILE:
            case XP_DEFAULT_TWO_PIPE_PROFILE:
                *acmProfileType = XP_DEFAULT_ACM_BILLING_CNTR_PROFILE;
                break;

            // Other profile types doesnt support billing counter profile
            default:
                return SAI_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        // Set the default ACM profile according to the profile:
        switch (profileType)
        {
            case XP_DEFAULT_SINGLE_PIPE_PROFILE:
                *acmProfileType = XP_DEFAULT_SINGLE_PIPE_ACM_PROFILE;
                break;
            case XP_DEFAULT_TWO_PIPE_PROFILE:
                *acmProfileType = XP_DEFAULT_TWO_PIPE_ACM_PROFILE;
                break;
            case XP_ROUTE_CENTRIC_SINGLE_PIPE_PROFILE:
                *acmProfileType = XP_ROUTE_CENTRIC_SINGLE_PIPE_ACM_PROFILE;
                break;
            case XP_TUNNEL_CENTRIC_SINGLE_PIPE_PROFILE:
                *acmProfileType = XP_TUNNEL_CENTRIC_SINGLE_PIPE_ACM_PROFILE;
                break;
            case XP_QACL_SAI_PROFILE:
            case XP_QACL_SAI_MCAST_PROFILE:
                *acmProfileType = XP_QACL_SAI_ACM_PROFILE;
                break;

            default:
                return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiDefaultConfigSet

sai_status_t xpSaiDefaultConfigSet(xpSaiSwitchProfile_t* pProfile)
{
    XP_SAI_LOG_DBG("Calling %s", __FUNCNAME__);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiDefaultProfileSet

sai_status_t xpSaiDefaultProfileSet(const char* switch_hardware_id,
                                    xpSaiSwitchProfile_t* pProfile)
{
    sai_status_t status = SAI_STATUS_FAILURE;

    XP_SAI_LOG_DBG("Calling %s", __FUNCNAME__);

    status = sai_sdk_dev_type_get(switch_hardware_id, &pProfile->sdkDevType);
    if (status != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_DBG("Could not recognize a hardware type, thus set to WM\n");
        pProfile->sdkDevType = XP_SDK_DEV_TYPE_WM;
    }

    pProfile->initType      = INIT_COLD;
    pProfile->diagMode      = false;
    //pProfile->userCli       = XP_CLI_MULTISHELL;
    pProfile->defaultVlanId = XPSAI_DEFAULT_VLAN_ID;
    pProfile->hwCfgPath[0] = '\0';
    pProfile->withBillingCntrs = 0;
    pProfile->warmBootReadFile[0] = '\0';
    pProfile->warmBootWriteFile[0] = '\0';
    pProfile->numUcastQueues = 0;
    pProfile->numMcastQueues = 0;
    pProfile->numQueues      = 8;
    pProfile->numCpuQueues   = 8;
    pProfile->numNhGrpEcmpMember  = 64;
    switch (pProfile->sdkDevType)
    {
        case XP_SDK_DEV_TYPE_SVB:
        case XP_SDK_DEV_TYPE_CRB1_REV1:
        case XP_SDK_DEV_TYPE_CRB2:
            pProfile->devType = XP80;
            break;

        case XP_SDK_DEV_TYPE_SHADOW:
        case XP_SDK_DEV_TYPE_CRB1_REV2:
        case XP_SDK_DEV_TYPE_CRB3_REV1:
            pProfile->devType = XP80B0;
            break;
        case XP_SDK_DEV_TYPE_CRB1_XP70_REV1:
        case XP_SDK_DEV_TYPE_CRB1_XP70_REV2:
        case XP_SDK_DEV_TYPE_CRB3_REV2:
            pProfile->devType = XP70;
            break;
        case XP_SDK_DEV_TYPE_WM:
            pProfile->devType = ALD;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "ALD", ALD);
            break;
        case XP_SDK_DEV_TYPE_ET6448M:
            pProfile->devType = ALDB2B;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "ALDB2B", ALDB2B);
            break;
        case XP_SDK_DEV_TYPE_ALDRIN_DB:
            pProfile->devType = ALDDB;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "ALDDB", ALDDB);
            break;
        case XP_SDK_DEV_TYPE_CYGNUS:
            pProfile->devType = CYGNUS;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "CYGNUS", CYGNUS);
            break;
        case XP_SDK_DEV_TYPE_ALDRIN2_XL:
            pProfile->devType = ALDRIN2XL;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "ALDRIN2XL", ALDRIN2XL);
            break;
        case XP_SDK_DEV_TYPE_TG48M_P:
            pProfile->devType = TG48M_P;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "TG48M_P", TG48M_P);
            break;
        case XP_SDK_DEV_TYPE_FALCON_32X25G_6_4:
            pProfile->devType = FALCON32x25G64;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FALCON32X25G_6_4", FALCON32x25G64);
            break;
        case XP_SDK_DEV_TYPE_FALCON_32:
            pProfile->devType = FALCON32;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FALCON32", FALCON32);
            break;
        case XP_SDK_DEV_TYPE_FALCON_32X400G:
            pProfile->devType = FALCON32x400G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FALCON32x400G", FALCON32x400G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_128X10G:
            pProfile->devType = FALCON128x10G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FALCON128x10G", FALCON128x10G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_128X25G:
            pProfile->devType = FALCON128x25G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FALCON128x25G", FALCON128x25G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_128X50G:
            pProfile->devType = FALCON128x50G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FALCON128x50G", FALCON128x50G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_64X100G:
            pProfile->devType = FALCON64x100G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FALCON64x100G", FALCON64x100G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_64X100G_R4:
            pProfile->devType = FALCON64x100GR4;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FALCON64x100GR4", FALCON64x100GR4);
            break;
        case XP_SDK_DEV_TYPE_FALCON_64X100G_R4_IXIA:
            pProfile->devType = FC64x100GR4IXIA;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC64x100GR4IXIA", FC64x100GR4IXIA);
            break;
        case XP_SDK_DEV_TYPE_FALCON_32X100G_R4_IXIA_6_4:
            pProfile->devType = FC32x100GR4IXIA;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC32x100GR4IXIA", FC32x100GR4IXIA);
            break;
        case XP_SDK_DEV_TYPE_FALCON_24X25G_6X100G_3_2:
            pProfile->devType = FALCONEBOF;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FALCON24X25G_6X100G_3_2", FALCONEBOF);
            break;
        case XP_SDK_DEV_TYPE_FALCON_24X25G_8X100G_6_4:
            pProfile->devType = FC24x258x100G64;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC24X25G_8X100G_6_4",
                            FC24x258x100G64);
            break;
        case XP_SDK_DEV_TYPE_FALCON_24X25G_4X200G:
            pProfile->devType = FC24x25G8x200G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC24X25G_4X200G", FC24x25G8x200G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_24X25G_8X100G:
            pProfile->devType = FC24x25G8x100G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC24X25G_8X100G", FC24x25G8x100G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_24X25G_4X200G_6_4:
            pProfile->devType = FC24x254x200G64;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC24X25G_4X200G_6_4",
                            FC24x254x200G64);
            break;
        case XP_SDK_DEV_TYPE_FALCON_24X100G_4X400G:
            pProfile->devType = FC24x100G4x400G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC24X100G_4X400G", FC24x100G4x400G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_16X100G_8X400G:
            pProfile->devType = FC16x100G8x400G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC16x100_G8x400G", FC16x100G8x400G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_48X100G_8X400G:
            pProfile->devType = FC48x100G8x400G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC48X100G_8X400G", FC48x100G8x400G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_32X100G_8X400G:
            pProfile->devType = FC32x100G8x400G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC32X100G_8X400G", FC32x100G8x400G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_32X100G_16X400G:
            pProfile->devType = FC32x10016x400G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC32X100G_16X400G", FC32x10016x400G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_96X100G_8X400G:
            pProfile->devType = FC96x100G8x400G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC96X100G_8X400G", FC96x100G8x400G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_128X100G:
            pProfile->devType = FALCON128x100G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FALCON128X100G", FALCON128x100G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_128:
            pProfile->devType = FALCON128;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FALCON128", FALCON128);
            break;
        case XP_SDK_DEV_TYPE_FALCON_32X25G:
            pProfile->devType = FALCON32X25G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FALCON32X25", FALCON32X25G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_64X25G:
            pProfile->devType = FALCON64x25G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FALCON64X25", FALCON64x25G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_16X400G:
            pProfile->devType = FALCON16x400G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FALCON16X400G", FALCON16x400G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_16X25G:
            pProfile->devType = FALCON16X25G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FALCON16X25", FALCON16X25G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_32X25G_IXIA:
            pProfile->devType = FC32x25GIXIA;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC32x25GIXIA", FC32x25GIXIA);
            break;
        case XP_SDK_DEV_TYPE_FALCON_16X25G_IXIA:
            pProfile->devType = FC16x25GIXIA;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC16x25GIXIA", FC16x25GIXIA);
            break;
        case XP_SDK_DEV_TYPE_FALCON_48x10G_8x100G:
            pProfile->devType = FC48x10G8x100G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC48x10G8x100G", FC48x10G8x100G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_48x25G_8x100G:
            pProfile->devType = FC48x25G8x100G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC48x25G8x100G", FC48x25G8x100G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_64x25G_64x10G:
            pProfile->devType = FC64x25G64x10G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC64x25G64x10G", FC64x25G64x10G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_32x25G_8x100G:
            pProfile->devType = FC32x25G8x100G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "FC32x25G8x100G", FC32x25G8x100G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_F2T80x25G:
            pProfile->devType = F2T80x25G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "F2T80x25G", F2T80x25G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_F2T80x25GIXIA:
            pProfile->devType = F2T80x25GIXIA;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "F2T80x25GIXIA", F2T80x25GIXIA);
            break;
        case XP_SDK_DEV_TYPE_FALCON_F2T48x10G8x100G:
            pProfile->devType = F2T48x10G8x100G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "F2T48x10G8x100G", F2T48x10G8x100G);
            break;
        case XP_SDK_DEV_TYPE_FALCON_F2T48x25G8x100G:
            pProfile->devType = F2T48x25G8x100G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "F2T48x25G8x100G", F2T48x25G8x100G);
            break;
        case XP_SDK_DEV_TYPE_AC5XRD:
            pProfile->devType = AC5XRD;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "AC5XRD", AC5XRD);
            break;
        case XP_SDK_DEV_TYPE_AC5XRD_48x1G6x10G:
            pProfile->devType = AC5X48x1G6x10G;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "AC5X48x1G6x10G", AC5X48x1G6x10G);
            break;
        case XP_SDK_DEV_TYPE_ALDRIN2_XL_FL:
            pProfile->devType = ALDRIN2XLFL;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "ALDRIN2XLFL", ALDRIN2XLFL);
            break;
        case XP_SDK_DEV_TYPE_ALDRIN2_EVAL:
            pProfile->devType = ALDRIN2EVAL;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "ALDRIN2EVAL", ALDRIN2EVAL);
            break;
        case XP_SDK_DEV_TYPE_AC3X_FS:
            pProfile->devType = AC3XFS;
            XP_SAI_LOG_INFO("Device Type %s [%d]\n", "AC3XFS", AC3XFS);
            break;
        default:
            XP_SAI_LOG_ERR("Wrong sdkDevType is passed!\n");
            return SAI_STATUS_FAILURE;
    }

    /* Default: disable SCPU */
    pProfile->scpuFwFile[0] = '\0';
    pProfile->enableScpuFw = 0;
    pProfile->z80ANFwFile[0] = '\0';

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiDefaultPortConfigApply
sai_status_t xpSaiDefaultPortConfigApply(xpsDevice_t xpsDevId,
                                         xpsInterfaceId_t xpsIntf)
{
    XP_STATUS    xpStatus  = XP_NO_ERR;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpSaiPortDbEntryT *pPortEntry = NULL;

    xpStatus = xpsMacPortEnable(xpsDevId, xpsIntf, false);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Could not disable port %u.", xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsMacSetTxPriorityPauseVector(xpsDevId, xpsIntf, 0);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Could not set priority flow control for port %u.", xpsIntf);
        return xpsStatus2SaiStatus(xpStatus);
    }

    saiStatus = xpSaiPortDbInfoGet(xpsIntf, &pPortEntry);
    if (saiStatus == SAI_STATUS_ITEM_NOT_FOUND)
    {
        XP_SAI_LOG_ERR("Could not retrieve port info from DB");
        return saiStatus;
    }

    xpStatus = xpsMtuSetInterfaceMtuSize(xpsDevId, xpsIntf, XPSAI_PORT_DEFAULT_MTU);
    if (XP_NO_ERR != xpStatus)
    {
        if (xpsGetInitType() == INIT_COLD)
        {

            XP_SAI_LOG_ERR("Could not be set a default MTU to the port %u.", xpsIntf);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiApplySwitchPortsDefaultConfig
static sai_status_t xpSaiApplySwitchPortsDefaultConfig(xpsDevice_t xpsDevId)
{
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    XP_STATUS           xpStatus    = XP_NO_ERR;
    GT_STATUS           rc          = GT_OK;
    xpsInterfaceId_t    portIfId    = 0;
    uint32_t            attrCount   = 2;
    sai_attribute_t     *attrList   = NULL;
    sai_attribute_t     *queue_attr = NULL;
    xpSaiPortMgrInfo_t  *info       = NULL;
    xpsPort_t           portNum     = 0;
    sai_object_id_t     switchObjId = SAI_NULL_OBJECT_ID;
    sai_object_id_t     port_id     = 0;
    sai_object_id_t     queueObjId  = 0;
    sai_object_id_t     defSchedulerObjId = SAI_NULL_OBJECT_ID;
    uint32_t            numCpuQs    = 0;
    xpSaiPortStatisticDbEntryT *portStatistics = NULL;

    attrList = (sai_attribute_t *)xpMalloc(sizeof(sai_attribute_t)*attrCount);
    if (!attrList)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for attrList\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(attrList, 0, sizeof(sai_attribute_t)*attrCount);

    queue_attr = (sai_attribute_t *)xpMalloc(sizeof(sai_attribute_t)*4);
    if (!queue_attr)
    {
        xpFree(attrList);
        XP_SAI_LOG_ERR("Error: allocation failed for queue_attr\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(queue_attr, 0, sizeof(sai_attribute_t)*4);

    /* Get the queue key-value pair info */
    saiStatus = xpSaiSwitchGetDefaultQueueValues(NULL, NULL, NULL,
                                                 &numCpuQs);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        xpFree(attrList);
        xpFree(queue_attr);
        XP_SAI_LOG_ERR("Failed to get queue key-value pair info \n");
        return saiStatus;
    }

    xpStatus = xpsPortGetFirst(&portIfId);
    if (XP_NO_ERR != xpStatus)
    {
        xpFree(attrList);
        xpFree(queue_attr);
        XP_SAI_LOG_ERR("xpsPortGetFirst call failed!\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    /*populate attribute list to create port*/
    //attrList[0].value.u32list.list = hwLanes;
    attrList[0].value.u32list.list = (uint32_t *)xpMalloc(8 * sizeof(uint32_t));
    memset(attrList[0].value.u32list.list, ~0, 8 * sizeof(uint32_t));
    attrList[0].id = SAI_PORT_ATTR_HW_LANE_LIST;

    attrList[1].id = SAI_PORT_ATTR_SPEED;

    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_SWITCH, xpsDevId, 0, &switchObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        xpFree(attrList);
        xpFree(queue_attr);
        XP_SAI_LOG_ERR("Error: Could not create switch object id, devId: %d, saiStatus: %d\n",
                       xpsDevId, saiStatus);
        return saiStatus;
    }
    /* Create a default Scheduler */
    sai_attribute_t schedulerAttr[4];

    memset(schedulerAttr, 0, sizeof(schedulerAttr));

    /* Populate scheduling type (Strict/Wrr/Dwrr). Setting default to DWRR */
    schedulerAttr[0].id        = SAI_SCHEDULER_ATTR_SCHEDULING_TYPE;
    schedulerAttr[0].value.s32 = SAI_SCHEDULING_TYPE_DWRR;

    /* Populate scheduling weight. Setting default to 100 */
    schedulerAttr[1].id       = SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT;
    schedulerAttr[1].value.u8 = 100;

    saiStatus = xpSaiCreateScheduler(&defSchedulerObjId, switchObjId, 2,
                                     schedulerAttr);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        xpFree(attrList);
        xpFree(queue_attr);
        XP_SAI_LOG_ERR("Error: Could not create switch deafult scheduler, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }
    /* Insert default scheduler to Database */
    saiStatus = xpSaiSwitchSetDefaultScheduler(defSchedulerObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        xpFree(attrList);
        xpFree(queue_attr);
        XP_SAI_LOG_ERR("Failed to set the default scheduler into the DB\n");
        return saiStatus;
    }

    /* Create default H1 Schedulers based on type */
    saiStatus = xpSaiSwitchCreateH1SchedulerBasedOnType(switchObjId,
                                                        XP_SAI_SCHEDULER_PROFILE_TYPE_H1UC);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create the H1 UC scheduler\n");
        xpFree(attrList);
        xpFree(queue_attr);
        return saiStatus;
    }

    saiStatus = xpSaiSwitchCreateH1SchedulerBasedOnType(switchObjId,
                                                        XP_SAI_SCHEDULER_PROFILE_TYPE_H1CTRL);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create the H1 CTRL scheduler\n");
        xpFree(attrList);
        xpFree(queue_attr);
        return saiStatus;
    }

    saiStatus = xpSaiSwitchCreateH1SchedulerBasedOnType(switchObjId,
                                                        XP_SAI_SCHEDULER_PROFILE_TYPE_H1MC);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create the H1 MC scheduler\n");
        xpFree(attrList);
        xpFree(queue_attr);
        return saiStatus;
    }

    info = (xpSaiPortMgrInfo_t *)xpMalloc(sizeof(xpSaiPortMgrInfo_t));
    if (!info)
    {
        xpFree(attrList);
        xpFree(queue_attr);
        XP_SAI_LOG_ERR("Error: allocation failed for queue_attr\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    do
    {
        if (xpSaiIsPortActive(portIfId) == 0)
        {
            continue;
        }

        xpStatus = xpsPortGetDevAndPortNumFromIntf(portIfId, &xpsDevId, &portNum);
        if (XP_NO_ERR != xpStatus)
        {
            xpFree(attrList);
            xpFree(queue_attr);
            xpFree(info);
            XP_SAI_LOG_ERR("Failed to get devId and port num for port interface %u!\n",
                           portIfId);
            return xpsStatus2SaiStatus(xpStatus);
        }

        xpSaiPortInfoInit(info);
        xpStatus = xpSaiPortCfgGet(xpsDevId, portNum, info);
        if (XP_NO_ERR != xpStatus)
        {
            xpFree(attrList);
            xpFree(queue_attr);
            xpFree(info);
            XP_SAI_LOG_ERR("xpSaiPortCfgGet call failed!\n");
            return xpsStatus2SaiStatus(xpStatus);
        }

        /* populate attribute list to create port */
        if (info->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_4X)
        {
            attrList[0].value.u32list.list[0] = info->hwLaneList[0];
            attrList[0].value.u32list.list[1] = info->hwLaneList[1];
            attrList[0].value.u32list.list[2] = info->hwLaneList[2];
            attrList[0].value.u32list.list[3] = info->hwLaneList[3];
            attrList[0].value.u32list.count = 4;
        }
        else if (info->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_2X)
        {
            attrList[0].value.u32list.list[0] = info->hwLaneList[0];
            attrList[0].value.u32list.list[1] = info->hwLaneList[1];
            attrList[0].value.u32list.count = 2;
        }
        else if (info->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_8X)
        {
            attrList[0].value.u32list.list[0] = info->hwLaneList[0];
            attrList[0].value.u32list.list[1] = info->hwLaneList[1];
            attrList[0].value.u32list.list[2] = info->hwLaneList[2];
            attrList[0].value.u32list.list[3] = info->hwLaneList[3];
            attrList[0].value.u32list.list[4] = info->hwLaneList[4];
            attrList[0].value.u32list.list[5] = info->hwLaneList[5];
            attrList[0].value.u32list.list[6] = info->hwLaneList[6];
            attrList[0].value.u32list.list[7] = info->hwLaneList[7];
            attrList[0].value.u32list.count = 8;

        }
        else
        {
            attrList[0].value.u32list.list[0] = info->hwLaneList[0];
            attrList[0].value.u32list.count = 1;
        }
        attrList[0].value.u32list.list[0] = info->hwLaneList[0];
        attrList[0].value.u32list.list[1] = info->hwLaneList[1];
        attrList[0].value.u32list.list[2] = info->hwLaneList[2];
        attrList[0].value.u32list.list[3] = info->hwLaneList[3];
        attrList[0].value.u32list.list[4] = info->hwLaneList[4];
        attrList[0].value.u32list.list[5] = info->hwLaneList[5];
        attrList[0].value.u32list.list[6] = info->hwLaneList[6];
        attrList[0].value.u32list.list[7] = info->hwLaneList[7];
        attrList[1].value.u32 = info->speed;

        saiStatus = xpSaiCreatePort(&port_id, switchObjId, attrCount, attrList);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            xpFree(attrList);
            xpFree(queue_attr);
            xpFree(info);
            XP_SAI_LOG_ERR("Could not create a sai port for port Id %d\n", portNum);
            return saiStatus;
        }

    }
    while (xpsPortGetNext(portIfId, &portIfId) == XP_NO_ERR);
    xpFree(info);

    /* Create cpu port and queues */
    sai_object_id_t cpuPortObjId = 0;
    uint32_t cpuPortNum = 0;

    xpStatus = xpsGlobalSwitchControlGetCpuPhysicalPortNum(xpsDevId, &cpuPortNum);
    if (XP_NO_ERR != xpStatus)
    {
        xpFree(attrList);
        xpFree(queue_attr);
        XP_SAI_LOG_ERR("xpsGlobalSwitchControlGetCpuPhysicalPortNum call failed! Error %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Create cpu object id */
    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, xpsDevId,
                                 (sai_uint64_t)cpuPortNum, &cpuPortObjId);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        xpFree(attrList);
        xpFree(queue_attr);
        XP_SAI_LOG_ERR("Error : SAI object can not be created\n");
        return saiStatus;
    }

    /* Insert entry in QOS port DB */
    xpStatus = xpSaiInsertPortQosDbEntry(cpuPortObjId);
    if (XP_NO_ERR != xpStatus)
    {
        xpFree(attrList);
        xpFree(queue_attr);
        XP_SAI_LOG_ERR("Could not insert entry in QOS port DB for CPU port %u.",
                       cpuPortNum);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Loop through all the queues per port */
    for (uint32_t qNum = 0; qNum < XPS_MAX_QUEUES_PER_PORT; qNum++)
    {
        /* Disable enqueue */
        if ((rc = cpssHalPortTxQueueTxEnableSet(xpsDevId, cpuPortNum, qNum,
                                                GT_FALSE)) != GT_OK)
        {
            xpFree(attrList);
            xpFree(queue_attr);
            XP_SAI_LOG_ERR("Failed to set enqueue enable, device: %d, port: %d, queue: %d\n",
                           xpsDevId, cpuPortNum, qNum);
            return cpssStatus2SaiStatus(rc);
        }
    }

    /* Enable enqueue for user created cpu queues */
    for (uint32_t qNum = 0; qNum < numCpuQs; qNum++)
    {
        /* Populate queue type (All/Unicast/Multicast) */
        queue_attr[0].id = SAI_QUEUE_ATTR_TYPE;
        queue_attr[0].value.s32 = SAI_QUEUE_TYPE_ALL;

        /* Populate port object id */
        queue_attr[1].id = SAI_QUEUE_ATTR_PORT;
        queue_attr[1].value.oid = cpuPortObjId;

        /* Populate queue index with in port */
        queue_attr[2].id = SAI_QUEUE_ATTR_INDEX;
        queue_attr[2].value.u8 = qNum;

        /* Populate parent scheduler node. In case of Hierarchical Qos not supported, the parent node is the port */
        queue_attr[3].id = SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE;
        queue_attr[3].value.oid = cpuPortObjId;

        /* Create queue object */
        if ((saiStatus = xpSaiCreateQueue(&queueObjId, switchObjId, 4,
                                          queue_attr)) != SAI_STATUS_SUCCESS)
        {
            xpFree(attrList);
            xpFree(queue_attr);
            XP_SAI_LOG_ERR("Failed to create queue, device: %d, port: %d, queue: %d\n",
                           xpsDevId, cpuPortNum, qNum);
            return saiStatus;
        }
    }

    /*insert entry in port statistic DB for CPU port*/
    xpStatus = xpSaiInsertPortStatisticDbEntry(cpuPortNum);
    if (XP_NO_ERR != xpStatus)
    {
        xpFree(attrList);
        xpFree(queue_attr);
        XP_SAI_LOG_ERR("Could not insert entry in port statistic DB for portIntf %u.",
                       cpuPortNum);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpSaiGetPortStatisticInfo(cpuPortNum, &portStatistics);
    if (XP_NO_ERR != xpStatus)
    {
        xpFree(attrList);
        xpFree(queue_attr);
        XP_SAI_LOG_ERR("Error: Failed to get the port statistic data, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /*create v4/v6 acl rule for CPU port*/
    xpStatus = xpsMacStatCounterCreateAclRuleForV4V6(xpsDevId, cpuPortNum,
                                                     &portStatistics->counterId_Ingress_v4, &portStatistics->counterId_Ingress_v6,
                                                     &portStatistics->counterId_Egress_v4,  &portStatistics->counterId_Egress_v6);
    if (XP_NO_ERR != xpStatus)
    {
        xpFree(attrList);
        xpFree(queue_attr);
        XP_SAI_LOG_ERR("Could not create Acl rule for port v4v6 statistics for the port %u.\n",
                       cpuPortNum);
        return  xpsStatus2SaiStatus(xpStatus);
    }

    xpFree(attrList);
    xpFree(queue_attr);
    return SAI_STATUS_SUCCESS;
}

void xpSaiSwitchLinkUpEventNotification(xpsDevice_t xpsDevId, uint32_t port_num)
{
    sai_status_t    saiStatus   = SAI_STATUS_SUCCESS;
    sai_object_id_t port_id;
    xpsDevice_t     devId       = xpSaiGetDevId();

    sai_port_oper_status_notification_t data;

    XP_SAI_LOG_DBG("**********************************************************\n");
    XP_SAI_LOG_DBG("SAI: Link up interrupt received on port: %d, devId: %d\n",
                   port_num, xpsDevId);
    XP_SAI_LOG_DBG("***********************************************************\n");

    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                 (sai_uint64_t) port_num, &port_id);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Error : SAI object id creation failed for port %d\n", port_num);
        return;
    }

    if (switch_notifications_g[xpsDevId].on_port_state_change != NULL)
    {
        data.port_id = port_id;
        data.port_state = SAI_PORT_OPER_STATUS_UP;
        switch_notifications_g[xpsDevId].on_port_state_change(1, &data);
    }
    xpSaiHostIfLinkStatusSet(xpsDevId, port_num, true);
}

void xpSaiSwitchLinkDownEventNotification(xpsDevice_t xpsDevId,
                                          uint32_t port_num)
{
    sai_status_t    saiStatus   = SAI_STATUS_SUCCESS;
    sai_object_id_t port_id;
    xpsDevice_t     devId       = xpSaiGetDevId();

    sai_port_oper_status_notification_t data;

    XP_SAI_LOG_DBG("************************************************************\n");
    XP_SAI_LOG_DBG("SAI: Link down interrupt received on port: %d, devId: %d\n",
                   port_num, xpsDevId);
    XP_SAI_LOG_DBG("*************************************************************\n");

    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                 (sai_uint64_t) port_num, &port_id);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Error : SAI object id creation failed for port %d\n", port_num);
        return;
    }

    if (switch_notifications_g[xpsDevId].on_port_state_change != NULL)
    {
        data.port_id = port_id;
        data.port_state = SAI_PORT_OPER_STATUS_DOWN;
        switch_notifications_g[xpsDevId].on_port_state_change(1, &data);
    }
    xpSaiHostIfLinkStatusSet(xpsDevId, port_num, false);
}

//Func: xpSaiPortEventHandlerRegister
sai_status_t xpSaiPortEventHandlerRegister(uint16_t port_num)
{
    XP_STATUS status = XP_NO_ERR;

    // Link operational status:UP event handler
    status = xpsMacEventHandlerRegister(xpSaiGetDevId(), port_num, LINK_UP,
                                        xpSaiSwitchLinkUpEventNotification);
    if (status != XP_NO_ERR && status != XP_PORT_NOT_INITED)
    {
        XP_SAI_LOG_ERR("xpSaiPortEventHandlerRegister Failed for device id %d Error #%1d",
                       xpSaiGetDevId(), status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    // Link operational status:DOWN event handler
    status = xpsMacEventHandlerRegister(xpSaiGetDevId(), port_num, LINK_DOWN,
                                        xpSaiSwitchLinkDownEventNotification);
    if (status != XP_NO_ERR && status != XP_PORT_NOT_INITED)
    {
        XP_SAI_LOG_ERR("xpSaiPortEventHandlerRegister Failed for device id %d Error #%1d",
                       xpSaiGetDevId(), status);
        return xpsStatus2SaiStatus((XP_STATUS)status);
    }

    return SAI_STATUS_SUCCESS;
}

/* Get switch initialization parameters from SAI attributes */
sai_status_t xpSaiSwitchInitParamsGet(uint32_t attr_count,
                                      const sai_attribute_t *attr_list,
                                      xpSaiInitParams_t *switchInitParams)
{
    xpSdkDevType_t devType;
    char           inputHwStr[MAX_STR_LEN_64] = {0};
    char           *pEnd    = 0;
    const char *envVarName  = "XP_SAI_CONFIG_PATH";
    const char *envVarValue = NULL;

    sai_status_t   saiRetVal = SAI_STATUS_SUCCESS;

    if ((attr_list == 0) || (switchInitParams == 0))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* set defaults */
    memset(switchInitParams, 0x00, sizeof(*switchInitParams));

    /* parse configuration parameters first */
    for (uint32_t i = 0; i < attr_count; i++)
    {
        switch (attr_list[i].id)
        {
            case SAI_SWITCH_ATTR_SWITCH_PROFILE_ID:
                switchInitParams->profileId = attr_list[i].value.u32;
                break;

            case SAI_SWITCH_ATTR_SWITCH_HARDWARE_INFO:
                if (attr_list[i].value.s8list.list == 0)
                {
                    return SAI_STATUS_INVALID_PARAMETER;
                }
                else
                {
                    strncpy(inputHwStr, (const char *) attr_list[i].value.s8list.list,
                            sizeof(inputHwStr));
                    inputHwStr[sizeof(inputHwStr)-1] = '\0';
                }
                break;

            case SAI_SWITCH_ATTR_FIRMWARE_PATH_NAME:
                /* attribute to set the xdk root path */
                envVarValue = (const char *) attr_list[i].value.s8list.list;
                xpSetEnv(envVarName, envVarValue);

                break;

            case SAI_SWITCH_ATTR_INIT_SWITCH:
                switchInitParams->switchInit = attr_list[i].value.booldata;
                break;

            /* These are CREATE_AND_SET attributes which can be set during switch init.
             * Save them now and apply after switch init is complete */

            case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
                /* Check whether MAC is valid and then fall through to following cases */
                if ((saiRetVal = xpSaiIsValidUcastMac((uint8_t *) attr_list[i].value.mac)) !=
                    SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_SRC_MAC_ADDRESS)\n");
                    return saiRetVal;
                }
            case SAI_SWITCH_ATTR_RESTART_WARM:
            case SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY:
            case SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY:
            case SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY:
            case SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY:
            case SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY:
            case SAI_SWITCH_ATTR_BFD_SESSION_STATE_CHANGE_NOTIFY:
            case SAI_SWITCH_ATTR_SWITCHING_MODE:
            case SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE:
            case SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE:
            case SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES:
            case SAI_SWITCH_ATTR_FDB_AGING_TIME:
            case SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION:
            case SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION:
            case SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION:
            case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM:
            case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED:
            case SAI_SWITCH_ATTR_ECMP_DEFAULT_SYMMETRIC_HASH:
            case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM:
            case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED:
            case SAI_SWITCH_ATTR_LAG_DEFAULT_SYMMETRIC_HASH:
            case SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL:
            case SAI_SWITCH_ATTR_QOS_DEFAULT_TC:
            case SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_MAP:
            case SAI_SWITCH_ATTR_QOS_DOT1P_TO_COLOR_MAP:
            case SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_MAP:
            case SAI_SWITCH_ATTR_QOS_DSCP_TO_COLOR_MAP:
            case SAI_SWITCH_ATTR_QOS_TC_TO_QUEUE_MAP:
            case SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
            case SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
            case SAI_SWITCH_ATTR_SWITCH_SHELL_ENABLE:
            case SAI_SWITCH_ATTR_FAST_API_ENABLE:
                switchInitParams->set_attr_list[switchInitParams->set_attr_count].id =
                    attr_list[i].id;
                switchInitParams->set_attr_list[switchInitParams->set_attr_count].value =
                    attr_list[i].value;
                switchInitParams->set_attr_count++;

                if (switchInitParams->set_attr_count >= ATTR_LIST_LEN)
                {
                    XP_SAI_LOG_ERR("Too many SET attributes specified - %u\n",
                                   switchInitParams->set_attr_count);
                    return SAI_STATUS_NOT_SUPPORTED;
                }
                break;

            default:
                XP_SAI_LOG_ERR("Specified attribute %u couldn't be set during switch create\n",
                               attr_list[i].id);
                return SAI_STATUS_NOT_SUPPORTED;
        }
    }

    /* Parse string with HW details. String format: platform-rev_[npu-number] */
    if (strnlen(inputHwStr, sizeof(inputHwStr)))
    {
        char *subStr = strtok(inputHwStr, "_");

        if (sai_sdk_dev_type_get(subStr, &devType) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Incorrect platform %s\n", subStr);
            return SAI_STATUS_INVALID_PARAMETER;
        }
        else
        {
            /* save platform string to be used for switch init later */
            memcpy(switchInitParams->switchHwStr, subStr,
                   sizeof(switchInitParams->switchHwStr));
        }

        subStr = strtok(NULL, "");

        /* NPU part is optional and may not be provided */
        if (subStr != 0)
        {
            /* retrieve NPU number (switch ID). Return error otherwise */
            if (strncmp(subStr, NPU_STR, strlen(NPU_STR)) == 0)
            {
                switchInitParams->xpSaiDevId = strtol(subStr + strlen(NPU_STR), &pEnd, 10);
                if ((pEnd != 0) && (*pEnd != '\0'))
                {
                    XP_SAI_LOG_ERR("NPU number is incorrect: %s \n", subStr + strlen(NPU_STR));
                    return SAI_STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                XP_SAI_LOG_ERR("Incorrect NPU string specified: %s \n", subStr);
                return SAI_STATUS_INVALID_PARAMETER;
            }
        }
    }

    XP_SAI_LOG_DBG("Platform string %s, profile id %u, switchInit %u\n",
                   switchInitParams->switchHwStr,
                   switchInitParams->profileId,
                   switchInitParams->switchInit);

    return SAI_STATUS_SUCCESS;
}

/* Func: xpSaiSetSwitchAttributes
 * to set switch attributes called on create
 */
static sai_status_t xpSaiSetSwitchAttributes(sai_object_id_t switchId,
                                             const sai_attribute_t *attr_list, uint32_t attr_count)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    /* Check incoming parameters */
    if ((attr_list == NULL) || (attr_count == 0))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiSetSwitchAttribute(switchId, &attr_list[count]);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not set attribute %u\n", attr_list[count].id);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetSwitchAttrPortList(xpsDevice_t devId,
                                        sai_object_list_t* pSaiObjList)
{
    sai_status_t        saiStatus  = SAI_STATUS_SUCCESS;
    uint32_t            portNum    = 0;
    uint32_t            i          = 0;
    sai_object_key_t   *keyOidList = NULL;

    if (NULL == pSaiObjList)
    {
        XP_SAI_LOG_ERR("Invalid parameters received.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiStatus = xpSaiCountPortObjects(&portNum);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port count.\n");
        return saiStatus;
    }

    if (portNum > pSaiObjList->count)
    {
        pSaiObjList->count = portNum;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if ((portNum != 0) && (pSaiObjList->list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameters received.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    keyOidList = (sai_object_key_t *)xpMalloc(sizeof(sai_object_key_t) * portNum);
    if (keyOidList == NULL)
    {
        XP_SAI_LOG_ERR("Could not allocate memory!\n");
        return SAI_STATUS_NO_MEMORY;
    }

    saiStatus = xpSaiGetPortObjectList(&portNum, keyOidList);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not retrieve port list.\n");
        xpFree(keyOidList);
        return saiStatus;
    }

    for (i = 0; i < portNum; i++)
    {
        pSaiObjList->list[i] = keyOidList[i].key.object_id;
    }

    /*update valid number of objects count in the list*/
    pSaiObjList->count = portNum;

    xpFree(keyOidList);

    XP_SAI_LOG_DBG("Retrieved %u ports.\n", pSaiObjList->count);

    return SAI_STATUS_SUCCESS;
}

static XP_STATUS xpSaiDevConfig(xpDevice_t devId, void* arg)
{
    sai_status_t  status = SAI_STATUS_SUCCESS;

    if (devId >= XP_MAX_DEVICES)
    {
        return XP_ERR_INIT;
    }

    status = sai_dev_config(devId, (void *) &xpSaiInitParam_g);
    if (status != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("sai_dev_config Failed for device id %d Error #%1d\n", devId,
                       status);
        return XP_ERR_INIT;
    }

    status = xpSaiVlanCreateDefaultVlanState(devId, xpSaiInitParam_g.defaultVlanId);
    if (status != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create default VLAN id %u. Error #%1d\n",
                       xpSaiInitParam_g.defaultVlanId, status);
        return XP_ERR_INIT;
    }

    if (xpSaiGetProfileType(devId) != XP_QACL_SAI_PROFILE)
    {
        if ((status = xpSaiL2McCreateDefaultGroup(devId)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to create default L2 Multicast Group (Error: %u)\n",
                           status);
            return XP_ERR_INIT;
        }
    }

    status = xpSaiCreateDefaultVirtualRouter(devId);
    if (status != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create default VRF. Error #%1d\n", status);
        return XP_ERR_INIT;
    }

    if (xpsGetInitType() == INIT_COLD)
    {
        status = xpSaiApplySwitchPortsDefaultConfig(devId);
        if (status != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not apply a default configuration\n");
            return XP_ERR_INIT;
        }

        /* Apply default configuration */
        status = xpSaiCreateDefaultBridgePorts(devId);
        if (status != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to create default .1Q bridge ports. Error #%1d\n",
                           status);
            return XP_ERR_INIT;
        }

        status = xpSaiVlanCreateVlanMembersState(devId, xpSaiInitParam_g.defaultVlanId);
        if (status != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to create default VLAN members. Error #%1d\n",
                           status);
            return XP_ERR_INIT;
        }
    }

    gSaiInitStat[devId] = XP_SAI_INITIALIZED;

    return XP_NO_ERR;
}

sai_status_t xpSaiSwitchConfigInbandMgmtPort(xpDevice_t xpSaiDevId,
                                             uint32_t portNum, uint32_t rateLimit)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS    xpStatus  = XP_NO_ERR;
    char         mgmtIntf[16]; //same as IFNAMSIZ
    uint32_t     maxPortNum = 0;

    xpStatus = xpsPortGetMaxNum(xpSaiDevId, &maxPortNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Max port Num\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (portNum >= maxPortNum)
    {
        XP_SAI_LOG_ERR("Invalid port Num:%d\n", portNum);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    strncpy(mgmtIntf, XPSAI_INBAND_MGMT_INTF_NAME, sizeof(mgmtIntf));
    XP_SAI_LOG_NOTICE("Create mgmt inband mgmt Interface:%s, port %d\n", mgmtIntf,
                      portNum);
    saiStatus = xpSaiHostIfNetdevCreate(xpSaiDevId, portNum,
                                        (uint8_t *)mgmtIntf);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to host interface for inband mgmt port:%d\n" \
                       "Possibly OOB %s is present. Skipping inband config\n",
                       portNum, mgmtIntf);
        return SAI_STATUS_SUCCESS;
    }

    xpStatus = xpsCoppInbandMgmtRatelimitSet(xpSaiDevId, rateLimit);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to set Inband mgmt rate limit:%d, Err code: %d\n",
                       rateLimit, xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Enable inband mgmt port */
    xpStatus = xpsMacPortEnable(xpSaiDevId, portNum, 1);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to enable inband mgmt port:%d\n", portNum);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsMtuSetInterfaceMtuSize(xpSaiDevId, portNum,
                                         XPSAI_INBAND_MGMT_PORT_MTU);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to MTU size for mgmt port:%d\n", portNum);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsAclSetRuleInbandMgmt(xpSaiDevId, portNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to set ACL rultes for inband mgmt:%d\n", portNum);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return saiStatus;
}

static void xpSaiInitializePortProfile(xpSaiSwitchProfile_t * profile,
                                       uint32_t profileId, xpsDevice_t xpSaiDevId)
{
    char  portConfig[256] = {0};
    char *findChar = NULL;
    int idx = 0, portIdx = 0, numOfPorts = 0;
    bool idxExist = 0;
    xpsPortProfile_t singlePort;
    bool cnpEnabled = true, dropEgressPackets = false;

    memset(&singlePort, 0, sizeof(xpsPortProfile_t));

    if (SAI_STATUS_SUCCESS == xpSaiSwitchProfileStrValueGet(profileId, "portConfig",
                                                            portConfig, sizeof(portConfig)))
    {
#ifdef ASIC_SIMULATION
        FILE * fp = fopen("./sonic/x86_64-irene-r0/irene/portsProfile.cfg", "r");
#else
        FILE * fp = fopen(portConfig, "r");
#endif

        XP_SAI_LOG_DBG("reading port configuration from file <%s>\n", portConfig);

        if (fp == NULL)
        {
            XP_SAI_LOG_ERR("DYNAMIC:Reading port configuration from file <%s> failed\n",
                           portConfig);
        }
        else
        {
            char * pch;
            int linePart, numOfLanes = 1, numOfParts;
            char * line = (char *)xpMalloc(sizeof(char)*XP_MAX_CONFIG_LINE_LEN);
            if (line == NULL)
            {
                XP_SAI_LOG_ERR("DYNAMIC: allocation failed for line\n");
                fclose(fp);
                return;
            }
            while (!feof(fp))
            {
                if (fgets(line, XP_MAX_CONFIG_LINE_LEN, fp) != NULL)
                {
                    line[strlen(line) - 1] = '\0';
                    if (strlen(line) < 2 || line[0] == ';' || line[0] == '#' || (line[0] == '/' &&
                                                                                 line[1] == '/'))
                    {
                        continue;
                    }

                    linePart = 0;
                    pch = strtok(line, " ,=\n\r\t");
                    while (pch != NULL)
                    {
                        if (linePart == 0)
                        {
                            findChar = strchr(pch, ')');
                            if (findChar)
                            {
                                pch = strtok(NULL, " ,=\n\r\t");
                            }
                        }

                        linePart ++;

                        pch = strtok(NULL, " ,=\n\r\t");
                    }

                    if (linePart < 3)
                    {
                        XP_SAI_LOG_WARNING("Detected error in line %d\n", idx);
                        continue;
                    }

                    if (findChar)
                    {
                        idxExist = 1;
                    }
                    numOfPorts++;
                }
                idx ++;
            }

            if (numOfPorts)
            {

                if (XP_NO_ERR == xpsCreatePortMappingProfile(profile->devType, numOfPorts))
                {
                    rewind(fp);

                    while (!feof(fp))
                    {
                        if (fgets(line, XP_MAX_CONFIG_LINE_LEN, fp) != NULL)
                        {
                            line[strlen(line) - 1] = '\0';
                            if (strlen(line) < 2 || line[0] == ';' || line[0] == '#' || (line[0] == '/' &&
                                                                                         line[1] == '/'))
                            {
                                continue;
                            }

                            linePart = 0;
                            numOfParts = 3;
                            pch = strtok(line, " ,=\n\r\t");
                            while (pch != NULL)
                            {
                                if (idxExist && linePart == 0)
                                {
                                    pch = strtok(NULL, " ,=\n\r\t");
                                    if (pch == NULL)
                                    {
                                        break;
                                    }
                                }

                                linePart ++;
                                if (linePart == 1)
                                {
                                    sscanf(pch, "port%u", &singlePort.portNum);
                                }
                                else if (linePart == 2)
                                {
                                    singlePort.speed = xpSaiSwitchIbufferSpeedStrConvert(pch);
                                }
                                else if (linePart == 3)
                                {
                                    if (strcmp(pch, "1000BaseX") == 0 || strcmp(pch, "SGMII") == 0 ||
                                        strcmp(pch, "SR_LR") == 0 ||
                                        strcmp(pch, "KR") == 0 || strcmp(pch, "CR") == 0)
                                    {
                                        sscanf(pch, "%63s", singlePort.mode);
                                        numOfLanes = 1;
                                    }
                                    else if (strcmp(pch, "KR2") == 0 || strcmp(pch, "CR2") == 0)
                                    {
                                        sscanf(pch, "%63s", singlePort.mode);
                                        numOfLanes = 2;
                                    }
                                    else if (strcmp(pch, "KR4") == 0 || strcmp(pch, "CR4") == 0)
                                    {
                                        sscanf(pch, "%63s", singlePort.mode);
                                        numOfLanes = 4;
                                    }
                                    else if (strcmp(pch, "KR8") == 0 || strcmp(pch, "CR8") == 0)
                                    {
                                        sscanf(pch, "%63s", singlePort.mode);
                                        numOfLanes = 8;
                                    }
                                    else
                                    {
                                        strcat(singlePort.mode, "unknown");
                                    }
                                }
                                else if (linePart == 4)
                                {
                                    numOfParts ++;
                                    if (strcasecmp("cnp_on", pch) == 0)
                                    {
                                        cnpEnabled = true;
                                    }
                                    else
                                    {
                                        cnpEnabled = false;
                                    }
                                }
                                else if (linePart == 5)
                                {
                                    numOfParts ++;
                                    if (strcasecmp("drop_on", pch) == 0)
                                    {
                                        dropEgressPackets = true;
                                    }
                                    else
                                    {
                                        dropEgressPackets = false;

                                    }
                                }

                                pch = strtok(NULL, " ,=\n\r\t");
                            }

                            if (linePart != numOfParts)
                            {
                                continue;
                            }

                            XP_SAI_LOG_DBG("Detected port %d mode %s CNP %s Drop Egress Packets %s speed %d\n",
                                           singlePort.portNum,
                                           singlePort.mode, cnpEnabled ? "enabled" : "disabled",
                                           dropEgressPackets ? "enabled" : "disabled", singlePort.speed);

                            if (XP_NO_ERR != xpsUpdatePortMappingProfile(profile->devType, portIdx,
                                                                         singlePort.portNum, singlePort.mode, singlePort.speed))
                            {
                                XP_SAI_LOG_ERR("DYNAMIC: port idx [%d] num [%d] mode [%s] update failed\n",
                                               portIdx, singlePort.portNum, singlePort.mode);
                            }

                            xpSaiPortCnpRateLimitParamSet(xpSaiDevId, portIdx, cnpEnabled);
                            xpsAclDropEgressPacketsParamSet(xpSaiDevId, portIdx, dropEgressPackets);

                            portIdx += numOfLanes;
                        }
                        idx ++;
                    }
                }
                else
                {
                    XP_SAI_LOG_ERR("xpsCreatePortMappingProfile: Creating port profile for devType %d numOfPorts %d failed\n",
                                   profile->devType, numOfPorts);
                }
            }
            xpFree(line);
        }
        if (fp)
        {
            fclose(fp);
        }
    }
}

//Func: xpSaiInitializeSwitch
sai_status_t xpSaiInitializeSwitch(sai_object_id_t* switchId,
                                   uint32_t attr_count,
                                   const sai_attribute_t *attr_list)
{
    sai_status_t            saiStatus       = SAI_STATUS_SUCCESS;
    XP_STATUS               xpStatus        = XP_NO_ERR;
    xpsRangeProfileType_t   rpType          = RANGE_PROFILE_MAX;
    static xpSaiSwitchProfile_t    profile;
    xpsDevice_t             xpSaiDevId      = 0;
    xpsScope_t              xpsScopeId      = 0;
    uint32_t                profileId       = 0;
    char                    saiConfigFile[XP_MAX_FILE_NAME_LEN];
    static xpSaiInitParams_t       switchInitParams;
    const sai_attribute_t   *attr           = NULL;
    sai_log_level_t         logLevel        = XPSAI_DEFAULT_LOG_LEVEL;
    xpSaiLogDest_t          logDest         = XPSAI_DEFAULT_LOG_DEST;
    sai_mac_t               xpSaiMacAddr;
    int                     isXpSaiMacAddrValid = 0;
    char                    ledModeStr[32]  = {0};

    //Use to set fdbAgingTime
    xpSaiSwitchAttributesT  *switchattributes = NULL;
    memset(&switchInitParams, 0, sizeof(switchInitParams));
    memset(&profile, 0, sizeof(xpSaiSwitchProfile_t));

    XP_SAI_LOG_INFO("Calling xpSaiInitializeSwitch attr_count %u", attr_count);

    switchattributes = (xpSaiSwitchAttributesT *)xpMalloc(sizeof(
                                                              xpSaiSwitchAttributesT));
    if (!switchattributes)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiSwitchApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(switchattributes, 0, sizeof(xpSaiSwitchAttributesT));
    // Default Value for fdbAgingTime is 0 and aging is disable for this value
    // Aging is enable only for non-zero value
    switchattributes->fdbAgingTime.u32 = XPSAI_DEFAULT_FDB_AGING_TIME_IN_SEC;

    if ((switchId == 0) || (attr_list == 0))
    {
        xpFree(switchattributes);
        XP_SAI_LOG_ERR("No input parameters passed\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               SWITCH_VALIDATION_ARRAY_SIZE, switch_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        xpFree(switchattributes);
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiSwitchInitParamsGet(attr_count, attr_list, &switchInitParams);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        xpFree(switchattributes);
        XP_SAI_LOG_ERR("Validation of input parameters failed\n");
        return saiStatus;
    }

    xpSaiDevId = switchInitParams.xpSaiDevId;
    profileId  = switchInitParams.profileId;

    profile.profileId = switchInitParams.profileId;

    /* sanity check that such device exists */
    if (xpSaiDevId >= XP_MAX_DEVICES)
    {
        xpFree(switchattributes);
        XP_SAI_LOG_ERR("Device ID exceeds supported device number\n");
        return SAI_STATUS_FAILURE;
    }

    if (gSaiInitStat[xpSaiDevId] == XP_SAI_INITIALIZED)
    {
        xpFree(switchattributes);
        XP_SAI_LOG_ERR("Switch already initialized\n");
        return SAI_STATUS_FAILURE;
    }

    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_SWITCH, xpSaiDevId, 0, switchId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        xpFree(switchattributes);
        XP_SAI_LOG_ERR("SAI switch object could not be created.\n");
        return saiStatus;
    }
    XP_SAI_LOG_INFO("Created switchId %" PRIx64, *switchId);

    /* switchInit has two meanings:
       true - initialize switch, false - connect to SDK */
    if (switchInitParams.switchInit == false)
    {
        XP_SAI_LOG_INFO("Calling xpSaiInitializeSwitch with init FALSE for dev id %u with profile id %u\n",
                        xpSaiDevId, profileId);
    }
    else
    {
        saiStatus = xpSaiSwitchProfileStrValueGet(profileId, "SAI_INIT_CONFIG_FILE",
                                                  saiConfigFile, XP_MAX_FILE_NAME_LEN);
        if ((SAI_STATUS_SUCCESS == saiStatus) && (strlen(saiConfigFile) > 0))
        {
            xpStatus = xpSaiProfileConfigLoadFromFile(profileId, saiConfigFile);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Can't load SAI config from file %s\n", saiConfigFile);
            }
        }

        profile.logDest = logDest;
        saiStatus = xpSaiSwitchProfileIntValueGet(profileId, "logDest",
                                                  (int32_t*) &logDest);
        if ((saiStatus == SAI_STATUS_SUCCESS) && (profile.logDest != logDest))
        {
            profile.logDest = logDest;
            xpSaiLogDeInit();
            xpSaiLogInit((logDest == XP_SAI_LOG_CONSOLE)?NULL:XP_SAI_LOG_FILENAME);
        }

        saiStatus = xpSaiSwitchProfileIntValueGet(profileId, "logLevel",
                                                  (int32_t*) &logLevel);
        if (saiStatus == SAI_STATUS_SUCCESS)
        {
            xpSaiLogSetLevel(logLevel);
        }

        if (xpSaiSwitchProfileIntValueGet(profileId, "inbandMgmtPortNum",
                                          (int32_t*)&inbandPortNum_g) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_NOTICE("Inband Mgmt port is not configured\n");
            inbandPortNum_g = 0xFFFF; /* Disable */
        }

        //Led mode
        if (SAI_STATUS_SUCCESS == xpSaiSwitchProfileStrValueGet(profileId,
                                                                "ledMode", ledModeStr, sizeof(ledModeStr)))
        {
            XP_SAI_LOG_NOTICE("LED Profile Mode %s\n", ledModeStr);
            xpsLedModeSet(xpSaiDevId, ledModeStr);
        }
        else
        {
            XP_SAI_LOG_NOTICE("LED Profile was not set\n");
        }


        /* Get platform and NPU number */
        if (switchInitParams.switchHwStr[0] == 0)
        {
            /* Get values from config file */
            saiStatus = xpSaiSwitchProfileStrValueGet(switchInitParams.profileId, "hwId",
                                                      switchInitParams.switchHwStr, XPSAI_SWITCH_HW_NAME_LEN);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_DBG("Could not get switch HW name from profile, status: %d!\n",
                               saiStatus);
            }

            memcpy(profile.hwId, switchInitParams.switchHwStr, sizeof(profile.hwId));

            saiStatus = xpSaiSwitchProfileIntValueGet(switchInitParams.profileId, NPU_STR,
                                                      (int32_t *) &switchInitParams.xpSaiDevId);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_DBG("Could not get NPU number from profile, status: %d!\n",
                               saiStatus);
            }
        }

        XP_SAI_LOG_INFO("SAI: hwId %s, device ID %u \n", switchInitParams.switchHwStr,
                        switchInitParams.xpSaiDevId);

        saiStatus = xpSaiDefaultProfileSet(switchInitParams.switchHwStr, &profile);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            xpFree(switchattributes);
            XP_SAI_LOG_ERR("xpSaiDefaultProfileSet() failed with status: %d!\n", saiStatus);
            return saiStatus;
        }

        saiStatus = xpSaiDefaultConfigSet(&profile);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            xpFree(switchattributes);
            XP_SAI_LOG_ERR("Failed to set default config!\n");
            return saiStatus;
        }

        if (xpSaiSwitchProfileIntValueGet(profileId, SAI_KEY_NUM_ECMP_MEMBERS,
                                          (int32_t*)&profile.numNhGrpEcmpMember) != SAI_STATUS_SUCCESS)
        {
            //Setting it to defaults.
            profile.numNhGrpEcmpMember = 64;
        }
        // Get the Switch MAC address value from the profile config file passed by NOS
        {
            char  macAddrStr[32] = {0};
            uint32_t swMacAddr[6] = {0, 0, 0, 0, 0, 0};
            int idx;
            uint8_t * ptrMac = &xpSaiMacAddr[0];
            if (SAI_STATUS_SUCCESS == xpSaiSwitchProfileStrValueGet(profileId,
                                                                    "switchMacAddress", macAddrStr, sizeof(macAddrStr)))
            {
                sscanf(macAddrStr, "%x:%x:%x:%x:%x:%x", &swMacAddr[0], &swMacAddr[1],
                       &swMacAddr[2], &swMacAddr[3], &swMacAddr[4], &swMacAddr[5]);
                for (idx = 0; idx< 6; idx++)
                {
                    ptrMac[idx] = (uint8_t)(swMacAddr[idx] & 0xFF);
                }
                memcpy(preInitMacAddr, ptrMac, sizeof(preInitMacAddr));
                isXpSaiMacAddrValid = 1;
            }
            else
            {
                XP_SAI_LOG_INFO("Switch MAC is not present 'switchMacAddresst' in profile.ini");
            }
        }

        XP_SAI_LOG_DBG("Profile id %u, range type %d, sdkDevType %d devId %u \n",
                       profileId, rpType, profile.sdkDevType, xpSaiDevId);

        if (IS_DEVICE_EBOF_PROFILE(profile.devType))
        {
            xpSaiInitializePortProfile(&profile, profileId, xpSaiDevId);
        }
        else
        {
            // Get the AP ports.
            //pass the port in format : apPortList=000 001 002 003 004 005 006 007 008 009 in profile.ini
            {
                char  apPortStr[520] = {0};
                uint32_t  a, p=0;

                if (SAI_STATUS_SUCCESS == xpSaiSwitchProfileStrValueGet(profileId, "apPortList",
                                                                        apPortStr, sizeof(apPortStr)))
                {
                    XP_SAI_LOG_ERR("AP Port list %s", apPortStr);
                    while (apPortStr[p]!='\0')
                    {

                        sscanf(apPortStr+p, "%u", &a);
                        XP_SAI_LOG_DBG("p %x a 0%3x\n", p, a);
                        p += 4;
                        XP_SAI_LOG_INFO("Enabling Autoneg for port %d", a);
                        xpsMacSetPortAnEnable(0, a, 1);
                    }
                }
                else
                {
                    XP_SAI_LOG_INFO("AP PortInfo is not present 'apPortList' in profile.ini");
                }
            }
        }
        /*
         * 3 profiles are supported.
         * MAX_L3_MIN_L2_NO_EM = 288K/144K_132K_0K
         * MID_L3_MID_L2_MIN_EM = 216K/108K_132K/32K
         * MID_L3_MID_L2_NO_EM = 504K/252K_32K_0K
         * MAX_L3_MID_LOW_L2_NO_EM = 432K/216K_32K_0k
         * NOTE: If no profile set or invalid values are given, we will init with
         *       profile MAX_L3_MIN_L2_NO_EM.
         */
        {
            char profStr[32]    = {0};
            int32_t maxRouteIp4 = 0;
            int32_t maxRouteIp6 = 0;
            int sharedProfId    = -1;
            if (SAI_STATUS_SUCCESS == xpSaiSwitchProfileStrValueGet(profileId,
                                                                    "lpm_fdb_em_profile", profStr, sizeof(profStr)))
            {
                XP_SAI_LOG_NOTICE("lpm_fdb_em_profile : %s \n", profStr);
                if (strcmp(profStr, "MAX_L3_MIN_L2_NO_EM") == 0)
                {
                    sharedProfId = CPSS_HAL_MAX_L3_MIN_L2_NO_EM;
                }
                else if (strcmp(profStr, "MID_L3_MID_L2_MIN_EM") == 0)
                {
                    sharedProfId = CPSS_HAL_MID_L3_MID_L2_MIN_EM;
                }
                else if (strcmp(profStr, "MID_L3_MID_L2_NO_EM") == 0)
                {
                    sharedProfId = CPSS_HAL_MID_L3_MID_L2_NO_EM;
                }
                else if (strcmp(profStr, "LOW_MAX_L3_MID_LOW_L2_NO_EM") == 0)
                {
                    sharedProfId = CPSS_HAL_LOW_MAX_L3_MID_LOW_L2_NO_EM;
                }
                if (sharedProfId != CPSS_HAL_MID_L3_MID_L2_NO_EM &&
                    sharedProfId != CPSS_HAL_MID_L3_MID_L2_MIN_EM &&
                    sharedProfId != CPSS_HAL_LOW_MAX_L3_MID_LOW_L2_NO_EM &&
                    sharedProfId != CPSS_HAL_MAX_L3_MIN_L2_NO_EM)
                {
                    XP_SAI_LOG_ERR("Invalid lpm_fdb_em_profile_id : %d Setting Defaults\n",
                                   sharedProfId);
                    sharedProfId = CPSS_HAL_MID_L3_MID_L2_NO_EM;
                }
            }
            else
            {
                XP_SAI_LOG_NOTICE("profile.ini:lpm_fdb_em_profile_id NOT Found. Setting Defaults\n");
                sharedProfId = CPSS_HAL_MID_L3_MID_L2_NO_EM;
            }
            XP_SAI_LOG_NOTICE("profile.ini:lpm_fdb_em_profile_id : %d \n", sharedProfId);
            SetSharedProfileIdx(sharedProfId);

            if (SAI_STATUS_SUCCESS == xpSaiSwitchProfileIntValueGet(profileId,
                                                                    "max_routes_ipv4", (int32_t*)&maxRouteIp4))
            {
                XP_SAI_LOG_NOTICE("profile.ini:max_routes_ipv4 : %d \n", maxRouteIp4);
                SetRouteIpv4MaxNum(maxRouteIp4);
            }

            if (SAI_STATUS_SUCCESS == xpSaiSwitchProfileIntValueGet(profileId,
                                                                    "max_routes_ipv6", (int32_t*)&maxRouteIp6))
            {
                XP_SAI_LOG_NOTICE("profile.ini:max_routes_ipv6 : %d \n", maxRouteIp6);
                SetRouteIpv6MaxNum(maxRouteIp6);
            }

            if ((maxRouteIp4 = 0) || (maxRouteIp6 == 0))
            {
                XP_SAI_LOG_NOTICE("profile.ini:max_routes_ipv4 or max_routes_ipv6 is not given"
                                  " or given as 0.\n");
                xpSaiSwitchUpdateDefaultMaxRouteNum(sharedProfId, &maxRouteIp4, &maxRouteIp6);
                XP_SAI_LOG_NOTICE("profile.ini:Reverting max_routes_ipv4 to %d, max_routes_ipv6 to %d",
                                  maxRouteIp4, maxRouteIp6);
                SetRouteIpv4MaxNum(maxRouteIp4);
                SetRouteIpv6MaxNum(maxRouteIp6);
            }
        }
        {
            uint32_t pbrNum = 0;
            if (xpSaiSwitchProfileIntValueGet(profileId, "pbrMaxEntries",
                                              (int32_t*)&pbrNum) != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_NOTICE("PBR is not configured.Setting Default 0\n");
                pbrNum = 0;//XPSAI_PBR_MAX_SIZE;
            }

            XP_SAI_LOG_NOTICE("pbrMaxEntries : %d \n", pbrNum);
            SetPbrMaxNum(pbrNum);
        }
        {
            char l3CntStr[32] = {0};
            int l3CntIdxBase = -1;
            char *tok;
            int i = 0;
            /* M0 and Falcon (profiles with >25G ports) - first pos is valid
               Eg: l3_counter_index_base=0;
                   l3_counter_index_base=4K
               Falcon10G/25G profile - first 2 pos are valid.
               Eg: l3_counter_index_base=0 3K;
                   l3_counter_index_base=1K 4K
               Aldrin2XL- first five pos are valid.
                  Eg: l3_counter_index_base=0 3K;
                      l3_counter_index_base=0 1K 2K 3K 4K
             */
            if (SAI_STATUS_SUCCESS == xpSaiSwitchProfileStrValueGet(profileId,
                                                                    "l3_counter_index_base",
                                                                    l3CntStr,
                                                                    sizeof(l3CntStr)))
            {
                XP_SAI_LOG_NOTICE("l3_counter_index_base : %s \n", l3CntStr);
                tok = strtok(l3CntStr, " ");
                while (tok != NULL)
                {
                    if (strcmp(tok, "0") == 0)
                    {
                        l3CntIdxBase = 0;
                    }
                    else if (strcmp(tok, "1K") == 0)
                    {
                        l3CntIdxBase = _1K;
                    }
                    else if (strcmp(tok, "2K") == 0)
                    {
                        l3CntIdxBase = _2K;
                    }
                    else if (strcmp(tok, "3K") == 0)
                    {
                        l3CntIdxBase = _3K;
                    }
                    else if (strcmp(tok, "4K") == 0)
                    {
                        l3CntIdxBase = _4K;
                    }
                    else
                    {
                        XP_SAI_LOG_ERR("Invalid l3_counter_index_base : %s Don't Configure\n",
                                       tok);
                        l3CntIdxBase = -1;
                    }
                    SetL3CounterIdBase(l3CntIdxBase, i);
                    i++;
                    if (!(IsValidCounterIdBase(i, profile.devType)))
                    {
                        break;
                    }
                    tok = strtok(NULL, " ");
                }
            }
            else
            {
                XP_SAI_LOG_NOTICE("profile.ini:l3_counter_index_base NOT Found. Setting Defaults\n");
                l3CntIdxBase = 0;
                SetL3CounterIdBase(l3CntIdxBase, i);
            }
        }
        //get port info
        {
            char saiPlatformConfigFile[XP_MAX_FILE_NAME_LEN];

            if (SAI_STATUS_SUCCESS == xpSaiSwitchProfileStrValueGet(profileId,
                                                                    "platformConfigFile",
                                                                    saiPlatformConfigFile, sizeof(saiPlatformConfigFile)))
            {
                XP_SAI_LOG_NOTICE("Platform configuration file %s\n", saiPlatformConfigFile);
                //add config file name
                xpsMacSetPlatformConfigFileName(saiPlatformConfigFile);
            }
            else
            {

                XP_SAI_LOG_INFO("Platform config file name not found 'platformConfigFile' in profile.ini");

            }

        }
        {
            char  apPortListWithCableLen[MAX_PORTNUM*6] = {0};
            uint32_t  a[2], p=0;

            if (SAI_STATUS_SUCCESS == xpSaiSwitchProfileStrValueGet(profileId,
                                                                    "apPortListWithCableLen",
                                                                    apPortListWithCableLen, sizeof(apPortListWithCableLen)))
            {
                XP_SAI_LOG_NOTICE("cable list %s\n", apPortListWithCableLen);
                while (apPortListWithCableLen[p]!='\0')
                {
                    sscanf(apPortListWithCableLen+p, "%u:%u", &a[0], &a[1]);
                    p+=6;
                    //set cable len
                    xpsMacSetPortCableLen(0, a[0], a[1]);
                    //set AP enable, No need to enable AP if its Optic cable(cableType value 2)
                    if (a[1]!=2)
                    {
                        xpsMacSetPortAnEnable(0, a[0], 1);
                    }
                }
            }
            else
            {
                XP_SAI_LOG_INFO("AP/CableLength/Fiber info not found 'apPortListWithCableLen' in profile.ini");
            }
        }
        //regular port len list.
        {
            char  portCableLenList[MAX_PORTNUM*6] = {0};
            uint32_t  a[2], p=0;

            if (SAI_STATUS_SUCCESS == xpSaiSwitchProfileStrValueGet(profileId,
                                                                    "portListWithCableLen",
                                                                    portCableLenList, sizeof(portCableLenList)))
            {
                XP_SAI_LOG_NOTICE("cable list %s\n", portCableLenList);
                while (portCableLenList[p]!='\0')
                {
                    sscanf(portCableLenList+p, "%u:%u", &a[0], &a[1]);
                    p+=6;
                    //set cable len
                    xpsMacSetPortCableLen(0, a[0], a[1]);
                }
            }
            else
            {
                XP_SAI_LOG_INFO("CableLength/Fiber info not found 'portCableLenList' in profile.ini");
            }
        }
        xpSaiSwitchIbufferInit(&profile);

        xpSaiInitParam_g.numNhGrpEcmpMember = profile.numNhGrpEcmpMember;
        if ((xpStatus = xpsL3SetNextHopGroupMaxEcmpSize(
                            xpSaiInitParam_g.numNhGrpEcmpMember)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to set next hop group max ecmp size (Error: %u)\n",
                           xpStatus);
            return XP_ERR_INIT;
        }
        XP_SAI_LOG_INFO("Ecmp size is set to %d\n",
                        xpSaiInitParam_g.numNhGrpEcmpMember);

        /* do basic init only first time */
        if (switchInitialized == false)
        {
            xpStatus = (XP_STATUS) sai_sdk_init(switchInitParams.switchHwStr, &profile,
                                                rpType);
            if (XP_NO_ERR != xpStatus)
            {
                xpFree(switchattributes);
                XP_SAI_LOG_ERR("XDK Init failed\n");
                return xpsStatus2SaiStatus(xpStatus);
            }

            switchInitialized = true;
        }
        else
        {
            /* get next scope ID */
            xpStatus = xpsScopeGetFreeScope(&xpsScopeId);
            if (xpStatus != XP_NO_ERR)
            {
                xpFree(switchattributes);
                XP_SAI_LOG_ERR("Failed to get new scope ID\n");
                return xpsStatus2SaiStatus(xpStatus);
            }

            XP_SAI_LOG_DBG("Create scope %u for device %u\n", xpsScopeId, xpSaiDevId);

            /* create separate scope for each new device */
            xpStatus = xpsSdkInitScope(profile.devType, xpsScopeId, rpType,
                                       profile.initType);
            if (xpStatus != XP_NO_ERR)
            {
                xpFree(switchattributes);
                XP_SAI_LOG_ERR("Failed to create new scope\n");
                return xpsStatus2SaiStatus(xpStatus);
            }

            xpStatus = xpsScopeAddDevice(xpSaiDevId, xpsScopeId);
            if (xpStatus != XP_NO_ERR)
            {
                xpFree(switchattributes);
                XP_SAI_LOG_ERR("Failed to add device to created scope");
                return xpsStatus2SaiStatus(xpStatus);
            }
        }


        /* Initialize XP80/WM configuration profile */

        XP_SAI_LOG_DBG("XP configuration profile initialized! devType = %d\n",
                       profile.sdkDevType);

        /* TBD : Init global */
        saiStatus = xpSaiSwitchDbInit(&profile);
        if (saiStatus != XP_NO_ERR)
        {
            xpFree(switchattributes);
            XP_SAI_LOG_ERR("xpSaiSwitchInit Failed for device id %d Error #%1d", xpSaiDevId,
                           saiStatus);
            return saiStatus;
        }

        /* Cfg Table entry count DB Init */
        if ((xpStatus = xpSaiTableEntryCountDbInit()) != XP_NO_ERR)
        {
            xpFree(switchattributes);
            XP_SAI_LOG_ERR("Failed to init table entry count DB\n");
            return xpsStatus2SaiStatus(xpStatus);
        }

        xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;
        xpStatus = xpSaiInsertTableEntryCountCtxDb(XP_SCOPE_DEFAULT, xpSaiDevId,
                                                   &entryCountCtxPtr);
        if (xpStatus != XP_NO_ERR)
        {
            xpFree(switchattributes);
            return xpsStatus2SaiStatus(xpStatus);
        }

        /*
         *  SAI_SWITCH_ATTR_FDB_AGING_TIME attribute is not mandatory in create_switch; Optional for user application
         *  1) Attribute value is zero;Aging is Disable
         *  2) Attribute value is non-zero;Aging is Enable
         *  3) Not-used SAI_SWITCH_ATTR_FDB_AGING_TIME attribute in application; Aging is Disable by default
        */
        attr = xpSaiFindAttrById(SAI_SWITCH_ATTR_FDB_AGING_TIME,
                                 switchInitParams.set_attr_count, switchInitParams.set_attr_list, NULL);
        if (NULL != attr)
        {
            switchattributes->fdbAgingTime.u32 = attr->value.u32;
            XP_SAI_LOG_DBG("AGING time is %d \n\n", switchattributes->fdbAgingTime.u32);
        }
        else
        {
            XP_SAI_LOG_DBG("AGING time is %d and aging is disable\n\n",
                           switchattributes->fdbAgingTime.u32);
        }

        gSaiFdbParams.xpDevId = xpSaiDevId;
        gSaiFdbParams.ageTime = switchattributes->fdbAgingTime.u32;
        XP_SAI_LOG_DBG("AGING time is %d \n\n", gSaiFdbParams.ageTime);
        XP_SAI_LOG_DBG("Dev-id is %d \n\n", gSaiFdbParams.xpDevId);
        xpSaiInitParam_g.devType = profile.devType;
        xpSaiInitParam_g.initType = profile.initType;
        xpSaiInitParam_g.defaultVlanId = profile.defaultVlanId;
        if (profile.portScanMs)
        {
            xpSaiInitParam_g.portScanMs = profile.portScanMs;
        }
        else
        {
            xpSaiInitParam_g.portScanMs = XPSAI_DEFAULT_LINKSCAN_DELAY;
        }
        xpSaiInitParam_g.xpSaiSwitchObjId = (uint64_t)*switchId;

        /* Setup XDK IPC server */
        if (XP_SDK_DEV_TYPE_WM == profile.sdkDevType)
        {
            xpStatus = xpSaiDevConfig(xpSaiDevId, (void *) &xpSaiInitParam_g);
            if (xpStatus != XP_NO_ERR)
            {
                xpFree(switchattributes);
                XP_SAI_LOG_ERR("sai_dev_config Failed for device id %d Error #%1d\n",
                               xpSaiDevId, xpStatus);
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
        else
        {
            xpStatus = xpSaiDevConfig(xpSaiDevId, (void *) &xpSaiInitParam_g);
            if (xpStatus != XP_NO_ERR)
            {
                xpFree(switchattributes);
                XP_SAI_LOG_ERR("sai_dev_config Failed for device id %d Error #%1d\n",
                               xpSaiDevId, xpStatus);
                return xpsStatus2SaiStatus(xpStatus);
            }
            /* Load Z80 Firmware */
            if (profile.z80ANFwFile[0] != '\0')
            {
                XP_SAI_LOG_WARNING("Loading Z80 Firmware <%s>\n", profile.z80ANFwFile);
                xpStatus = xpsMacTv80RunFirmware(xpSaiDevId, profile.z80ANFwFile);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Z80 Firmware <%s> Load Failed for device id %d Error #%d\n",
                                   profile.z80ANFwFile, xpSaiDevId, xpStatus);
                }
            }

            /* Load LED firmware in SCPU */
            if (profile.enableScpuFw)
            {
                xpStatus = xpsServiceCpuEnable(xpSaiDevId, false);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("SCPU Reset Failed for device id %d Error #%d\n", xpSaiDevId,
                                   xpStatus);
                }
                else
                {
                    XP_SAI_LOG_WARNING("Loading SCPU Firmware <%s>\n", profile.scpuFwFile);
                    xpStatus = xpsServiceCpuLoadFirmware(xpSaiDevId, profile.scpuFwFile);
                    if (xpStatus != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("SCPU Firmware <%s> Load Failed for device id %d Error #%d\n",
                                       profile.scpuFwFile, xpSaiDevId, xpStatus);
                    }
                    else
                    {
                        xpStatus = xpsServiceCpuEnable(xpSaiDevId, true);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("SCPU Enable Failed for device id %d Error #%d\n", xpSaiDevId,
                                           xpStatus);
                        }
                    }
                }
            }
        } // End if else XP_SDK_DEV_TYPE_WM
    } // End if else switchinit

    XP_SAI_LOG_DBG("SaiDevId is %u\n\n", xpSaiDevId);

    /*Set Switch mac address */
    if (isXpSaiMacAddrValid)
    {
        sai_attribute_value_t attr;
        memcpy(&attr.mac, &xpSaiMacAddr, sizeof(xpSaiMacAddr));
        saiStatus = xpSaiSetSwitchAttrSrcMacAddress(attr);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to set Switch MAC Address\n");
        }
    }

    /* parse and handle notification attributes if any */
    if (switchInitParams.set_attr_count > 0)
    {
        saiStatus = xpSaiSetSwitchAttributes(*switchId, switchInitParams.set_attr_list,
                                             switchInitParams.set_attr_count);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to apply CREATE_SET attributes\n");
        }
    }
    //enable shell
    {
        sai_attribute_t tmpAttr;
        tmpAttr.id = SAI_SWITCH_ATTR_SWITCH_SHELL_ENABLE;
        tmpAttr.value.booldata = 1;
        xpSaiSetSwitchAttribute(*switchId, &tmpAttr);
    }


    if (inbandPortNum_g != 0xFFFF)
    {
        uint32_t rateLimit = 0;
        if (xpSaiSwitchProfileIntValueGet(profileId, "inbandMgmtRateLimit",
                                          (int32_t*)&rateLimit) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_NOTICE("Inband Mgmt rate limiter is not configured. using 2500 pkt/sec\n");
            rateLimit = XPSAI_INBAND_MGMT_RATELIMIT;
        }

        saiStatus = xpSaiSwitchConfigInbandMgmtPort(xpSaiDevId, inbandPortNum_g,
                                                    rateLimit);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            xpFree(switchattributes);
            XP_SAI_LOG_ERR("Failed to config inband mgmt port\n");
            return saiStatus;
        }
    }

    xpSaiSignalRegister();

    /* notify port thread that switch init is complete */
    xpSaiSwitchInitDone();

    /* Useful especially in the  case of WARM boot ,i.e after the
     * recovery and initialization is complete
     * the initType for the device is set to INIT_COLD
     */
    if (xpsIsDevInitDone(xpSaiDevId))
    {
        xpsSetInitType(INIT_COLD);
        XP_SAI_LOG_DBG(" InitType set to INIT_COLD \n");
    }

    xpSaiSetSwitchOperStatus(SAI_SWITCH_OPER_STATUS_UP);
    xpFree(switchattributes);
    return SAI_STATUS_SUCCESS;
}

extern bool gResetInProgress;
//Func: xpSaiShutdownSwitch
sai_status_t xpSaiShutdownSwitch(sai_object_id_t switchId)
{
    XP_STATUS    status           = XP_NO_ERR;
    sai_status_t saiStatus        = SAI_STATUS_SUCCESS;
    xpsDevice_t  xpSaiDevId       = 0;

    XP_SAI_LOG_NOTICE("xpSaiShutdownSwitch switchId %" PRIx64, switchId);
    /* get xpsDevId from OID switchId*/
    xpSaiDevId = xpSaiObjIdSwitchGet(switchId);

    /* sanity check that such device exists */
    if (xpSaiDevId >= XP_MAX_DEVICES)
    {
        XP_SAI_LOG_ERR("Device ID exceeds supported device number\n");
        return SAI_STATUS_FAILURE;
    }

    if (gSaiInitStat[xpSaiDevId] == XP_SAI_UNINITIALIZED)
    {
        XP_SAI_LOG_ERR("Device is not initialized\n");
        return SAI_STATUS_FAILURE;
    }

    if (gResetInProgress == 0)
    {
        gResetInProgress = 1;
    }



    /* CPSS expects all VRs to be deleted before LPM DB delete.
       Note, routes part of VR are note cleaned up in SAI/XPS.
       This is not mandatory as we do cold shutdown.
       CPSS flushes routes as part of VR delete.
       Hence flush all VRs in CPSS using below API for swith remove to succeed.
    */
    saiStatus = xpSaiVrfFlushAll(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove vrf Error #%1d", saiStatus);
        return saiStatus;
    }

    xpSaiSetSwitchOperStatus(SAI_SWITCH_OPER_STATUS_DOWN);

    saiStatus = sai_sdk_dev_remove(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove device %u. Error #%1d", xpSaiDevId, saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiSwitchDbDeInit(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to deinitialize SAI switch DB for device %u. Error #%1d",
                       xpSaiDevId, saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiTableEntryCountDbDeInit(xpSaiDevId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to deinitialize SAI table entry count DB for device %u. Error #%1d",
                       xpSaiDevId, saiStatus);
        return saiStatus;
    }

    /* clear notifications */
    memset(switch_notifications_g, 0x00, sizeof(switch_notifications_g));

    gSaiInitStat[xpSaiDevId] = XP_SAI_UNINITIALIZED;

    /* TDB: deinitialize XDK only during shutdown of last device */
    status = xpsSdkDeInit();
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to deinitialize switch. Error #%1d", status);
        return xpsStatus2SaiStatus(status);
    }

    /* TDB: unset switchInitialized only during shutdown of last device */
    switchInitialized = false;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSwitchAttribute
sai_status_t xpSaiSetSwitchAttribute(sai_object_id_t switchId,
                                     const sai_attribute_t *attr)
{
    sai_status_t    saiRetVal        = SAI_STATUS_SUCCESS;
    xpsDevice_t     xpsDevId         = 0;

    XP_SAI_LOG_DBG("Called.\n");

    saiRetVal = xpSaiAttrCheck(1, attr,
                               SWITCH_VALIDATION_ARRAY_SIZE, switch_attribs,
                               SAI_COMMON_API_SET);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }
    xpsDevId = xpSaiObjIdSwitchGet(switchId);

    switch (attr->id)
    {
        case SAI_SWITCH_ATTR_SWITCHING_MODE:
            {
                saiRetVal = xpSaiSetSwitchAttrSwitchingMode(attr->value, xpsDevId);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_SWITCHING_MODE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE:
            {
                saiRetVal = xpSaiSetSwitchAttrBcastCpuFloodEnable(xpsDevId, attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE:
            {
                saiRetVal = xpSaiSetSwitchAttrMcastCpuFloodEnable(xpsDevId, attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
            {
                /* Check whether MAC is valid */
                if ((saiRetVal = xpSaiIsValidUcastMac((uint8_t *) attr->value.mac)) !=
                    SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_SRC_MAC_ADDRESS)\n");
                    return saiRetVal;
                }
                saiRetVal = xpSaiSetSwitchAttrSrcMacAddress(attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_SRC_MAC_ADDRESS)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES:
            {
                saiRetVal = xpSaiSwitchMaxFdbEntriesSet(xpsDevId, attr->value.u32);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set max number of learned FDB entries\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_FDB_AGING_TIME:
            {
                saiRetVal = xpSaiSetSwitchAttrFdbAgingTime(attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_FDB_AGING_TIME)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION:
            {
                saiRetVal = xpSaiSetSwitchAttrFdbUnicastMissAction(xpsDevId, attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_FDB_UNICAST_MISS_ACTION) | retVal : %d\n",
                                   saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION:
            {
                saiRetVal = xpSaiSetSwitchAttrFdbBroadcastMissAction(attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_ACTION)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION:
            {
                saiRetVal = xpSaiSetSwitchAttrFdbMulticastMissAction(attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_ACTION)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM:
            {
                saiRetVal = xpSaiSetSwitchAttrHashAlgo(attr->value.s32, 1);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM.\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED:
            {
                saiRetVal = xpSaiSetSwitchAttrLagHashSeed(attr->value.u32);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED.\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_ECMP_HASH_IPV4:
            {
                saiRetVal = xpSaiHashSetSwitchHash(XPSAI_IPV4_HASH_OBJECT_INDEX,
                                                   attr->value.oid);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_SWITCH_ATTR_LAG_HASH_IPV4.\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_ECMP_HASH_IPV4_IN_IPV4:
            {
                XP_SAI_LOG_ERR("Failed to set SAI_SWITCH_ATTR_ECMP_HASH_IPV4_IN_IPV4. Attribute not supported.\n");
                return SAI_STATUS_NOT_SUPPORTED;
            }
        case SAI_SWITCH_ATTR_ECMP_HASH_IPV6:
            {
                saiRetVal = xpSaiHashSetSwitchHash(XPSAI_IPV6_HASH_OBJECT_INDEX,
                                                   attr->value.oid);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_SWITCH_ATTR_LAG_HASH_IPV4.\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_LAG_DEFAULT_SYMMETRIC_HASH:
            {
                /* As symmetric hash config is global on XP80 this call will override the similar ECMP config */
                saiRetVal = xpSaiSetSwitchAttrSymmetricHash(switchId, attr->value.booldata);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_SWITCH_ATTR_LAG_DEFAULT_SYMMETRIC_HASH.\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED:
            {
                saiRetVal = xpSaiSetSwitchAttrEcmpHashSeed(switchId, attr->value.u32);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED.\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_ECMP_DEFAULT_SYMMETRIC_HASH:
            {
                /* As symmetric hash config is global on XP80 this call will override the similar LAG config */
                saiRetVal = xpSaiSetSwitchAttrSymmetricHash(switchId, attr->value.booldata);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_SWITCH_ATTR_ECMP_DEFAULT_SYMMETRIC_HASH.\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM:
            {
                saiRetVal = xpSaiSetSwitchAttrHashAlgo(attr->value.s32, 0);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM.\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_LAG_HASH_IPV4:
            {
                saiRetVal = xpSaiHashSetSwitchHash(XPSAI_IPV4_HASH_OBJECT_INDEX,
                                                   attr->value.oid);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_SWITCH_ATTR_LAG_HASH_IPV4.\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_LAG_HASH_IPV4_IN_IPV4:
            {
                XP_SAI_LOG_ERR("Failed to set SAI_SWITCH_ATTR_LAG_HASH_IPV4_IN_IPV4. Attribute not supported.\n");
                return SAI_STATUS_NOT_SUPPORTED;
            }
        case SAI_SWITCH_ATTR_LAG_HASH_IPV6:
            {
                saiRetVal = xpSaiHashSetSwitchHash(XPSAI_IPV6_HASH_OBJECT_INDEX,
                                                   attr->value.oid);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_SWITCH_ATTR_LAG_HASH_IPV4.\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_NUMBER_OF_UNICAST_QUEUES:
        case SAI_SWITCH_ATTR_NUMBER_OF_MULTICAST_QUEUES:
        case SAI_SWITCH_ATTR_NUMBER_OF_QUEUES:
        case SAI_SWITCH_ATTR_NUMBER_OF_CPU_QUEUES:
            {
                XP_SAI_LOG_ERR("Read only attribute %u received on set attribute call\n",
                               attr->id);
                return SAI_STATUS_INVALID_ATTRIBUTE_0;
            }
        case SAI_SWITCH_ATTR_QOS_DEFAULT_TC:
            {
                saiRetVal = xpSaiSetSwitchAttrQosDefaultTc(switchId, attr->value.u8);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_QOS_DEFAULT_TC)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_MAP:
            {
                saiRetVal = xpSaiSetSwitchAttrQosDot1pToTcMap(switchId, attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_MAP)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_QOS_DOT1P_TO_COLOR_MAP:
            {
                saiRetVal = xpSaiSetSwitchAttrQosDot1pToColorMap(switchId, attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_QOS_DOT1P_TO_COLOR_MAP)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_MAP:
            {
                saiRetVal = xpSaiSetSwitchAttrQosDscpToTcMap(switchId, attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_MAP)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_QOS_DSCP_TO_COLOR_MAP:
            {
                saiRetVal = xpSaiSetSwitchAttrQosDscpToColorMap(switchId, attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_QOS_DSCP_TO_COLOR_MAP)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_QOS_TC_TO_QUEUE_MAP:
            {
                saiRetVal = xpSaiSetSwitchAttrQosTcToQueueMap(switchId, attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_QOS_TC_TO_QUEUE_MAP)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
            {
                saiRetVal = xpSaiSetSwitchAttrQosTcAndColorToDot1pMap(switchId, attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
            {
                saiRetVal = xpSaiSetSwitchAttrQosTcAndColorToDscpMap(switchId, attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_RESTART_WARM:
            {
                if (attr->value.booldata == true)
                {
                    xpsSetInitType(INIT_WARM);
                }
                else
                {
                    xpsSetInitType(INIT_COLD);
                }
                break;
            }
        case SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY:
            switch_notifications_g[xpsDevId].on_switch_state_change =
                (sai_switch_state_change_notification_fn) attr->value.ptr;
            XP_SAI_LOG_DBG("SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY %p\n",
                           switch_notifications_g[xpsDevId].on_switch_state_change);
            break;

        case SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY:
            switch_notifications_g[xpsDevId].on_switch_shutdown_request =
                (sai_switch_shutdown_request_notification_fn) attr->value.ptr;
            XP_SAI_LOG_DBG("SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY %p\n",
                           switch_notifications_g[xpsDevId].on_switch_shutdown_request);
            break;

        case SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY:
            switch_notifications_g[xpsDevId].on_fdb_event = (sai_fdb_event_notification_fn)
                                                            attr->value.ptr;
            XP_SAI_LOG_DBG("SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY %p\n",
                           switch_notifications_g[xpsDevId].on_fdb_event);
            break;

        case SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY:
            switch_notifications_g[xpsDevId].on_port_state_change =
                (sai_port_state_change_notification_fn) attr->value.ptr;
            XP_SAI_LOG_DBG("SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY %p\n",
                           switch_notifications_g[xpsDevId].on_port_state_change);
            break;

        case SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY:
            switch_notifications_g[xpsDevId].on_packet_event =
                (sai_packet_event_notification_fn) attr->value.ptr;
            XP_SAI_LOG_DBG("SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY %p\n",
                           switch_notifications_g[xpsDevId].on_packet_event);
            break;

        case SAI_SWITCH_ATTR_BFD_SESSION_STATE_CHANGE_NOTIFY:
            switch_notifications_g[xpsDevId].on_bfd_state_change =
                (sai_bfd_session_state_change_notification_fn) attr->value.ptr;
            XP_SAI_LOG_DBG("SAI_SWITCH_ATTR_BFD_SESSION_STATE_CHANGE_NOTIFY %p\n",
                           switch_notifications_g[xpsDevId].on_bfd_state_change);
            break;
        case SAI_SWITCH_ATTR_QUEUE_PFC_DEADLOCK_NOTIFY:
            switch_notifications_g[xpsDevId].on_queue_deadlock_event =
                (sai_queue_pfc_deadlock_notification_fn) attr->value.ptr;
            XP_SAI_LOG_DBG("SAI_SWITCH_ATTR_QUEUE_PFC_DEADLOCK_NOTIFY %" PRIx64 "\n",
                           switch_notifications_g[xpsDevId].on_queue_deadlock_event);
            break;

        case SAI_SWITCH_ATTR_FAST_API_ENABLE:
            {
                XP_SAI_LOG_ERR("Failed to set SAI_SWITCH_ATTR_FAST_API_ENABLE. Attribute not supported.\n");
                return SAI_STATUS_ATTR_NOT_SUPPORTED_0;
            }

        case SAI_SWITCH_ATTR_SWITCH_SHELL_ENABLE:
            {
                saiRetVal = xpSaiSetSwitchAttrShellEnable(attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_SWITCH_SHELL_ENABLE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_INGRESS_ACL:
            {
                saiRetVal = xpSaiSetSwitchAttrIngressAcl(switchId, attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SWITCH_ATTR_INGRESS_ACL)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_SWITCH_ATTR_EGRESS_ACL:
            {
                XP_SAI_LOG_ERR("Unsupported attribute %u\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
                break;
            }
        case SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL:
            {
                XP_SAI_LOG_ERR("Unsupported attribute %d\n", attr->id);
                return SAI_STATUS_ATTR_NOT_SUPPORTED_0;
            }

        case SAI_SWITCH_ATTR_VXLAN_DEFAULT_PORT:
            {
                sai_status_t       status;
                status = xpSaiSetSwitchAttrVxlanDefaultPort(xpsDevId,
                                                            attr->value);
                if (status != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to xpSaiSetSwitchAttrVxlanDefaultPort: Err code: %d\n",
                                   status);
                    return status;
                }
                break;
            }
        case SAI_SWITCH_ATTR_VXLAN_DEFAULT_ROUTER_MAC:
            {
                sai_status_t       status;
                status = xpSaiSetSwitchAttrVxlanDefaultRouterMac(xpsDevId,
                                                                 attr->value);
                if (status != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to xpSaiSetSwitchAttrVxlanDefaultRouterMac: Err code: %d\n",
                                   status);
                    return status;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unsupported attribute %d\n", attr->id);
                return SAI_STATUS_NOT_SUPPORTED;
            }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetSwitchAttrPortNumber(uint32_t *portNum)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpsDevice_t devId = xpSaiGetDevId();
    uint32_t startPort;

    uint32_t maxNumPhyPorts = 0;
    uint32_t activePortCount = 0;

    if (portNum == NULL)
    {
        XP_SAI_LOG_ERR("NULL pointer received.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpStatus = xpsPortGetMaxNum(devId, &maxNumPhyPorts);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsPortGetMaxNum failed: status=%i.\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }
    startPort = xpsGetGlobalStartPort();

    /*This return as a number of initialised ports.*/
    for (uint32_t idx = 0; idx < ((uint32_t)MAX_PORTNUM  + startPort); idx++)
    {
        if (xpSaiIsPortActive(idx))
        {
            activePortCount++;
        }
    }

    *portNum = activePortCount;

    XP_SAI_LOG_DBG("portNum=%u.\n", *portNum);
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetSwitchAttrCPUPort(sai_object_id_t *cpuPort)
{
    XP_STATUS   xpStatus = XP_NO_ERR;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    uint32_t cpuPortNum = 0;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    if (NULL == cpuPort)
    {
        XP_SAI_LOG_ERR("Null pointer received for cpuPort.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpStatus = xpsGlobalSwitchControlGetCpuPhysicalPortNum(xpsDevId, &cpuPortNum);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("xpsGlobalSwitchControlGetCpuPhysicalPortNum call failed! Error %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, xpsDevId,
                                 (sai_uint64_t) cpuPortNum, cpuPort);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Error : SAI object can not be created.\n");
        return saiStatus;
    }

    /* QOS port db for front pannel ports will be done as part og sai_create_port flow
     * but, for CPU port we can't go with that flow as it contains mac enable, port speed setting etc
     * temporily we are inserting qos DB for CPU port in this flow, where for first get CPU port call
     * QOS DB entry will be created and there after it will just skip
     */
    xpSaiPortQosInfo_t *pPortQosInfo = NULL;
    xpStatus = xpSaiGetPortQosInfo(*cpuPort, &pPortQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetPortQosInfo call failed! Error %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }
    if (pPortQosInfo == NULL)
    {
        /*insert entry in QOS port DB*/
        xpStatus = xpSaiInsertPortQosDbEntry(*cpuPort);
        if (XP_NO_ERR != xpStatus)
        {
            XP_SAI_LOG_ERR("Could not insert entry in QOS port DB for CPU port %u.",
                           cpuPortNum);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }

    XP_SAI_LOG_DBG("xpSaiGetSwitchAttrCPUPort, cpuPort = %lu.\n",
                   (long unsigned int)*cpuPort);
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetSwitchAttrFdbSize(xpsDevice_t xpsDevId,
                                       sai_attribute_value_t* value)
{
    XP_STATUS xpStatus   = XP_NO_ERR;
    uint32_t  maxEntries = 0;

    if (NULL == value)
    {
        XP_SAI_LOG_ERR("Invalid parameters received.");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpStatus = xpsFdbGetTableSize(xpsDevId, &maxEntries);
    if (XP_NO_ERR != xpStatus)
    {
        XP_SAI_LOG_ERR("Failed to get FDB table size: Error %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    value->u32 = maxEntries * sizeof(xpsFdbEntry_t);

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetSwitchAttrDefaultVlanId(xpsDevice_t devId,
                                             sai_object_id_t *vlanObjId)
{
    XP_STATUS   xpStatus = XP_NO_ERR;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;
    xpsVlan_t vlanId;

    xpStatus = xpsVlanGetDefault(devId, &vlanId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve default VLAN ID\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VLAN, devId, vlanId, vlanObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create SAI VLAN object ID\n");
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

// Func: xpSaiGetSwitchAttrAclStageIngress

sai_status_t xpSaiGetSwitchAttrAclStageIngress(sai_object_id_t switch_id,
                                               sai_attribute_value_t *value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    if ((value->aclcapability.action_list.count < ingress_action_list_count) ||
        (value->aclcapability.action_list.list == NULL))
    {
        value->aclcapability.action_list.count = ingress_action_list_count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    // Return list of mandatory attributes
    value->aclcapability.is_action_list_mandatory = false;

    // List Ingress mandatory attributes
    value->aclcapability.action_list.count = ingress_action_list_count;
    memcpy(value->aclcapability.action_list.list, ingress_action_list_arr,
           sizeof(int32_t)*ingress_action_list_count);

    return saiRetVal;
}

// Func: xpSaiGetSwitchAttrAclStageEgress

sai_status_t xpSaiGetSwitchAttrAclStageEgress(sai_object_id_t switch_id,
                                              sai_attribute_value_t *value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    if ((value->aclcapability.action_list.count < egress_action_list_count) ||
        (value->aclcapability.action_list.list == NULL))
    {
        value->aclcapability.action_list.count = egress_action_list_count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    // Return list of mandatory attributes
    value->aclcapability.is_action_list_mandatory = false;

    // List Egress mandatory attributes
    value->aclcapability.action_list.count = egress_action_list_count;
    memcpy(value->aclcapability.action_list.list, egress_action_list_arr,
           sizeof(int32_t)*egress_action_list_count);

    return saiRetVal;
}

// Func: xpSaiGetSwitchAttrQosNumLosslessQueues

sai_status_t xpSaiGetSwitchAttrQosNumLosslessQueues(sai_object_id_t switch_id,
                                                    sai_attribute_value_t *value)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    if (NULL == value)
    {
        XP_SAI_LOG_ERR("Invalid parameters received.");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    value->s32 = XP_MAX_NUM_QUEUES_PER_PORT;

    return saiRetVal;
}

sai_status_t xpSaiGetSwitchAttribute(sai_object_id_t switchId,
                                     sai_uint32_t attr_count,
                                     sai_attribute_t *attr_list)
{
    XP_STATUS       xpStatus            = XP_NO_ERR;
    sai_status_t    saiStatus           = SAI_STATUS_SUCCESS;
    xpSaiAclEntry_t xpSaiAclEntryVar;
    xpsDevice_t     devId               = xpSaiGetDevId();
    XP_DEV_TYPE_T   devType             = xpSaiSwitchDevTypeGet();

    if ((attr_count < 1) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameters received.");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               SWITCH_VALIDATION_ARRAY_SIZE, switch_attribs,
                               SAI_COMMON_API_GET);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    /* Validate SAI Switch Object Id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(switchId, SAI_OBJECT_TYPE_SWITCH))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(switchId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get the device id from switch object id */
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(switchId);

    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;
    saiStatus = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB\n");
        return saiStatus;
    }
    if (entryCountCtxPtr == NULL)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count from DB\n");
        return SAI_STATUS_FAILURE;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            /* return inside switch is required. As current function desing does not allows us to return
               result of operation for each attribute. So we need to push user to read items one by one
               to get operation status.
            */
            case SAI_SWITCH_ATTR_PORT_NUMBER:
                {
                    saiStatus = xpSaiGetSwitchAttrPortNumber(&attr_list[count].value.u32);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("xpSaiGetSwitchAttrPortNumber call failed!\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_PORT_LIST:
                {
                    saiStatus = xpSaiGetSwitchAttrPortList(xpsDevId,
                                                           &attr_list[count].value.objlist);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("xpSaiGetSwitchAttrPortList call failed!\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_CPU_PORT:
                {
                    saiStatus = xpSaiGetSwitchAttrCPUPort(&attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("xpSaiGetSwitchAttrCPUPort call failed!\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS:
                {
                    xpStatus = xpSaiVrfMaxIdGet(devId, &attr_list[count].value.u32);
                    if (xpStatus != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("xpSaiVrfMaxIdGet failed, status %d\n", xpStatus);
                        return xpsStatus2SaiStatus(xpStatus);
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_FDB_TABLE_SIZE:
                {
                    saiStatus = xpSaiGetSwitchAttrFdbSize(xpsDevId, &attr_list[count].value);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_FDB_TABLE_SIZE)\n");
                        return saiStatus;
                    }
                    break;
                }
            /** The L3 Host Table size [sai_uint32_t] */
            case SAI_SWITCH_ATTR_L3_NEIGHBOR_TABLE_SIZE:
                {
                    XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr_list[count].id);
                    return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + count;
                }
            /** The L3 Route Table size [sai_uint32_t] */
            case SAI_SWITCH_ATTR_L3_ROUTE_TABLE_SIZE:
                {
                    XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr_list[count].id);
                    return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + count;
                }
            /** Number of ports that can be part of a LAG [sai_uint32_t] */
            case SAI_SWITCH_ATTR_LAG_MEMBERS:
                {

                    /*We do not have such value, but it's possible to create number of LAGS equal to number of ports. */
                    attr_list[count].value.u32 = entryCountCtxPtr->max_lagMemberPerGroup;
                    XP_SAI_LOG_DBG("Number of supported LAG members is %u.\n",
                                   attr_list[count].value.u32);
                    return SAI_STATUS_SUCCESS;
                }
            /** Number of LAGs that can be created [sai_uint32_t] */
            case SAI_SWITCH_ATTR_NUMBER_OF_LAGS:
                {

                    /*We do not have such value, but it's possible to create number of LAGS equal to number of ports. */
                    attr_list[count].value.u32 = entryCountCtxPtr->max_lagGroups;
                    XP_SAI_LOG_DBG("Number of supported LAGs is %u.\n", attr_list[count].value.u32);
                    return SAI_STATUS_SUCCESS;
                }
            /** ECMP number of members per group [sai_uint32_t] (default is 64) */
            case SAI_SWITCH_ATTR_ECMP_MEMBERS:
                {
                    uint32_t maxEcmpSize = 0;

                    xpStatus = xpsL3GetNextHopGroupMaxEcmpSize(&maxEcmpSize);
                    if (xpStatus != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_ECMP_MEMBERS) | saiStatus : %d\n",
                                       xpsStatus2SaiStatus(xpStatus));
                        return xpsStatus2SaiStatus(xpStatus);
                    }

                    attr_list[count].value.u32 = maxEcmpSize;
                    XP_SAI_LOG_DBG("Number of supported ECMP Members is %u.\n",
                                   attr_list[count].value.u32);
                    return SAI_STATUS_SUCCESS;
                }
            /** ECMP number of group [sai_uint32_t] */
            case SAI_SWITCH_ATTR_NUMBER_OF_ECMP_GROUPS:
                {
                    saiStatus = xpSaiGetSwitchAttrNumberOfNhGroups(&attr_list[count].value);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_NUMBER_OF_ECMP_GROUPS) | saiStatus : %d\n",
                                       saiStatus);
                        return saiStatus;
                    }
                    break;
                }
            /** The number of Unicast Queues per port [sai_uint32_t] */
            case SAI_SWITCH_ATTR_NUMBER_OF_UNICAST_QUEUES:
                {
                    saiStatus = xpSaiGetSwitchAttrNumberOfUnicastQueues(xpsDevId,
                                                                        &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_NUMBER_OF_UNICAST_QUEUES)\n");
                        return saiStatus;
                    }
                    break;
                }
            /** The number of Multicast Queues per port [sai_uint32_t] */
            case SAI_SWITCH_ATTR_NUMBER_OF_MULTICAST_QUEUES:
                {
                    saiStatus = xpSaiGetSwitchAttrNumberOfMulticastQueues(xpsDevId,
                                                                          &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_NUMBER_OF_MULTICAST_QUEUES)\n");
                        return saiStatus;
                    }
                    break;
                }
            /** The total number of Queues per port [sai_uint32_t] */
            case SAI_SWITCH_ATTR_NUMBER_OF_QUEUES:
                {
                    saiStatus = xpSaiGetSwitchAttrNumberOfQueues(xpsDevId, &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_NUMBER_OF_QUEUES)\n");
                        return saiStatus;
                    }
                    break;
                }
            /** The number of CPU Queues [sai_uint32_t] */
            case SAI_SWITCH_ATTR_NUMBER_OF_CPU_QUEUES:
                {
                    saiStatus = xpSaiGetSwitchAttrNumberOfCpuQueues(xpsDevId,
                                                                    &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_NUMBER_OF_CPU_QUEUES)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_ON_LINK_ROUTE_SUPPORTED:
                {
                    XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr_list[count].id);
                    return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + count;
                }
            case SAI_SWITCH_ATTR_OPER_STATUS:
                {
                    saiStatus = xpSaiGetSwitchOperStatus(&attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_OPER_STATUS)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_MAX_TEMP:
                {
                    int32_t temp = 0;

                    xpStatus = xpsSerdesSensorGetTemperature(xpsDevId, &temp);
                    if (XP_NO_ERR != xpStatus)
                    {
                        XP_SAI_LOG_ERR("Could not get system temperature, error %d\n", xpStatus);
                        return xpsStatus2SaiStatus(xpStatus);
                    }
                    attr_list[count].value.s32 = temp/1000; //Convert the milli-celsius to celsius
                    break;
                }
            case SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY:
                {
                    xpSaiAclGetStaticVariables(&xpSaiAclEntryVar);
                    attr_list[count].value.u32 = xpSaiAclEntryVar.saiAclTableMinimumPriority;
                    break;
                }
            case SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY:
                {
                    xpSaiAclGetStaticVariables(&xpSaiAclEntryVar);
                    attr_list[count].value.u32 = xpSaiAclEntryVar.saiAclTableMaximumPriority;
                    break;
                }
            case SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY:
                {
                    xpSaiAclGetStaticVariables(&xpSaiAclEntryVar);
                    attr_list[count].value.u32 = xpSaiAclEntryVar.saiAclEntryMinimumPriority;
                    break;
                }
            case SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY:
                {
                    xpSaiAclGetStaticVariables(&xpSaiAclEntryVar);
                    attr_list[count].value.u32 = xpSaiAclEntryVar.saiAclEntryMaximumPriority;
                    break;
                }
            case SAI_SWITCH_ATTR_ACL_USER_TRAP_ID_RANGE:
                {
                    saiStatus = xpSaiHostInterfaceUserTrapIdRangeGet(
                                    &attr_list[count].value.u32range.min,
                                    &attr_list[count].value.u32range.max);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_ACL_USER_TRAP_ID_RANGE)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_DEFAULT_VLAN_ID:
                {
                    saiStatus = xpSaiGetSwitchAttrDefaultVlanId(xpsDevId,
                                                                &attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("xpSaiGetSwitchAttrDefaultVlanId call failed!\n");
                        return saiStatus;
                    }

                    break;
                }
            case SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID:
                {
                    xpsStp_t stpId = 0;

                    xpStatus = xpsStpGetDefault(xpsDevId, &stpId);
                    if (XP_NO_ERR != xpStatus)
                    {
                        XP_SAI_LOG_ERR("Could not get system default VLAN!\n");
                        return xpsStatus2SaiStatus(xpStatus);
                    }

                    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_STP, devId, (sai_uint64_t)stpId,
                                                 &attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Error : SAI object can not be created(type=%u,stpId=%u).\n",
                                       SAI_OBJECT_TYPE_STP, stpId);
                        return saiStatus;
                    }

                    break;
                }
            case SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID:
                {
                    saiStatus = xpSaiSwitchDefaultVrfGet(&attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Could not get default VRF ID.\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID:
                {
                    saiStatus = xpSaiSwitchDefaultBridgeIdGet(&attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Could not get default 1Q bridge ID.\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_TOTAL_BUFFER_SIZE:
                {
                    saiStatus = xpSaiGetSwitchAttrTotalBufferSize(&attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_TOTAL_BUFFER_SIZE)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_INGRESS_BUFFER_POOL_NUM:
                {
                    saiStatus = xpSaiGetSwitchAttrIngressBufferPoolNum(xpsDevId,
                                                                       &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_INGRESS_BUFFER_POOL_NUM)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_EGRESS_BUFFER_POOL_NUM:
                {
                    saiStatus = xpSaiGetSwitchAttrEgressBufferPoolNum(xpsDevId,
                                                                      &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_EGRESS_BUFFER_POOL_NUM)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP:
                {
                    saiStatus = xpSaiSwitchDefaultTrapGroupGet(&attr_list[count].value.oid);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_SWITCHING_MODE:
                {
                    saiStatus = xpSaiGetSwitchAttrSwitchingMode(&attr_list[count].value, xpsDevId);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("failed to get (SAI_SWITCH_ATTR_SWITCHING_MODE)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_RESTART_TYPE:
                {
                    saiStatus = xpSaiSwitchRestartTypeSupportedGet((sai_switch_restart_type_t*)
                                                                   &attr_list[count].value.u32);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("failed to get (SAI_SWITCH_ATTR_RESTART_TYPE)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_MIN_PLANNED_RESTART_INTERVAL:
                {
                    saiStatus = xpSaiSwitchMinPlannedRestartIntervalGet(
                                    &attr_list[count].value.u32);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("failed to get (SAI_SWITCH_ATTR_MIN_PLANNED_RESTART_INTERVAL)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_NV_STORAGE_SIZE:
                {
                    saiStatus = xpSaiSwitchNvStorageSizeGet(&attr_list[count].value.u32);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("failed to get (SAI_SWITCH_ATTR_NV_STORAGE_SIZE:)\n");
                        return saiStatus + SAI_STATUS_CODE(count);
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE:
                {
                    saiStatus = xpSaiGetSwitchAttrBcastCpuFloodEnable(xpsDevId,
                                                                      &attr_list[count].value.booldata);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("failed to get (SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE:
                {
                    saiStatus = xpSaiGetSwitchAttrMcastCpuFloodEnable(xpsDevId,
                                                                      &attr_list[count].value.booldata);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("failed to get (SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_SRC_MAC_ADDRESS:
                {
                    saiStatus = xpSaiGetSwitchSrcMacAddress(attr_list[count].value.mac);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("failed to get (SAI_SWITCH_ATTR_SRC_MAC_ADDRESS)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES:
                {
                    saiStatus = xpSaiSwitchMaxFdbEntriesGet(xpsDevId, &attr_list[count].value.u32);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to get max number of learned FDB entries\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_FDB_AGING_TIME:
                {
                    saiStatus = xpSaiGetSwitchAttrFdbAgingTime(&attr_list[count].value);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_FDB_AGING_TIME)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION:
                {
                    saiStatus = xpSaiGetSwitchAttrFdbUnicastMissAction(&attr_list[count].value);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION) | saiStatus : %d\n",
                                       saiStatus);
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION:
                {
                    XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr_list[count].id);
                    return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + count;
                }
            case SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION:
                {
                    XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr_list[count].id);
                    return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + count;
                }
            case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED:
                {
                    saiStatus = xpSaiGetSwitchAttrEcmpHashSeed(&attr_list[count].value.u32);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED.\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_ECMP_HASH:
                {
                    saiStatus = xpSaiSwitchHashGet(XPSAI_L2_HASH_OBJECT_INDEX,
                                                   &attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Could not get SAI_SWITCH_ATTR_ECMP_HASH\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_ECMP_HASH_IPV4:
                {
                    saiStatus = xpSaiSwitchHashGet(XPSAI_IPV4_HASH_OBJECT_INDEX,
                                                   &attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Could not get SAI_SWITCH_ATTR_ECMP_HASH_IPV4\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_ECMP_HASH_IPV4_IN_IPV4:
                {
                    XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr_list[count].id);
                    return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + count;
                }
            case SAI_SWITCH_ATTR_ECMP_HASH_IPV6:
                {
                    saiStatus = xpSaiSwitchHashGet(XPSAI_IPV6_HASH_OBJECT_INDEX,
                                                   &attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Could not get SAI_SWITCH_ATTR_ECMP_HASH\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM:
                {
                    saiStatus = xpSaiGetSwitchAttrHashAlgo(&attr_list[count].value, 0);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Could not get SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_ECMP_DEFAULT_SYMMETRIC_HASH:
                {
                    saiStatus = xpSaiGetSwitchAttrSymmetricHash(switchId,
                                                                &attr_list[count].value.booldata);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Could not get SAI_SWITCH_ATTR_ECMP_DEFAULT_SYMMETRIC_HASH\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_LAG_DEFAULT_SYMMETRIC_HASH:
                {
                    saiStatus = xpSaiGetSwitchAttrSymmetricHash(switchId,
                                                                &attr_list[count].value.booldata);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Could not get SAI_SWITCH_ATTR_LAG_DEFAULT_SYMMETRIC_HASH\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM:
                {
                    saiStatus = xpSaiGetSwitchAttrHashAlgo(&attr_list[count].value, 1);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Could not get SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_LAG_HASH:
                {
                    saiStatus = xpSaiSwitchHashGet(XPSAI_L2_HASH_OBJECT_INDEX,
                                                   &attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Could not get SAI_SWITCH_ATTR_LAG_HASH\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_LAG_HASH_IPV4:
                {
                    saiStatus = xpSaiSwitchHashGet(XPSAI_IPV4_HASH_OBJECT_INDEX,
                                                   &attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Could not get SAI_SWITCH_ATTR_LAG_HASH_IPV4\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_LAG_HASH_IPV4_IN_IPV4:
                {
                    XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr_list[count].id);
                    return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + count;
                }
            case SAI_SWITCH_ATTR_LAG_HASH_IPV6:
                {
                    saiStatus = xpSaiSwitchHashGet(XPSAI_IPV6_HASH_OBJECT_INDEX,
                                                   &attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Could not get SAI_SWITCH_ATTR_LAG_HASH_IPV6\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED:
                {
                    saiStatus = xpSaiGetSwitchAttrLagHashSeed(&attr_list[count].value.u32);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED.\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_QOS_DEFAULT_TC:
                {
                    saiStatus = xpSaiGetSwitchAttrQosDefaultTc(switchId,
                                                               &attr_list[count].value.u8);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_QOS_DEFAULT_TC)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_MAP:
                {
                    saiStatus = xpSaiGetSwitchAttrQosDot1pToTcMap(switchId,
                                                                  &attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to retrieve QoS Map Id: saiStatus = %" PRIi32 "\n",
                                       saiStatus);
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_QOS_DOT1P_TO_COLOR_MAP:
                {
                    saiStatus = xpSaiGetSwitchAttrQosDot1pToColorMap(switchId,
                                                                     &attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to retrieve QoS Map Id: saiStatus = %" PRIi32 "\n",
                                       saiStatus);
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_MAP:
                {
                    saiStatus = xpSaiGetSwitchAttrQosDscpToTcMap(switchId,
                                                                 &attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to retrieve QoS Map Id: saiStatus = %" PRIi32 "\n",
                                       saiStatus);
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_QOS_DSCP_TO_COLOR_MAP:
                {
                    saiStatus = xpSaiGetSwitchAttrQosDscpToColorMap(switchId,
                                                                    &attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to retrieve QoS Map Id: saiStatus = %" PRIi32 "\n",
                                       saiStatus);
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_QOS_TC_TO_QUEUE_MAP:
                {
                    saiStatus = xpSaiGetSwitchAttrQosTcToQueueMap(switchId,
                                                                  &attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to retrieve QoS Map Id: saiStatus = %" PRIi32 "\n",
                                       saiStatus);
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP:
                {
                    saiStatus = xpSaiGetSwitchAttrQosTcAndColorToDot1pMap(switchId,
                                                                          &attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to retrieve QoS Map Id: saiStatus = %" PRIi32 "\n",
                                       saiStatus);
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP:
                {
                    saiStatus = xpSaiGetSwitchAttrQosTcAndColorToDscpMap(switchId,
                                                                         &attr_list[count].value.oid);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to retrieve QoS Map Id: saiStatus = %" PRIi32 "\n",
                                       saiStatus);
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_TRAFFIC_CLASSES:
                {
                    saiStatus = xpSaiGetSwitchAttrQosMaxNumberOfTrafficClasses(
                                    &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_TRAFFIC_CLASSES) | retVal = %d\n",
                                       saiStatus);
                        return saiStatus + SAI_STATUS_CODE(count);
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUP_HIERARCHY_LEVELS:
                {
                    saiStatus = xpSaiGetSwitchAttrQosMaxNumberOfSchedulerGroupHierarchyLevels(
                                    switchId, &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUP_HIERARCHY_LEVELS)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUPS_PER_HIERARCHY_LEVEL:
                {
                    saiStatus = xpSaiGetSwitchAttrQosMaxNumberOfSchedulerGroupsPerHierarchyLevel(
                                    switchId, &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUPS_PER_HIERARCHY_LEVEL)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_CHILDS_PER_SCHEDULER_GROUP:
                {
                    saiStatus = xpSaiGetSwitchAttrQosMaxNumberOfChildsPerSchedulerGroup(switchId,
                                                                                        &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_CHILDS_PER_SCHEDULER_GROUP)\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_MAX_ACL_ACTION_COUNT:
                {
                    attr_list[count].value.u32 = (ingress_action_list_count >
                                                  egress_action_list_count) ? ingress_action_list_count :
                                                 egress_action_list_count;
                    break;
                }
            case SAI_SWITCH_ATTR_FAST_API_ENABLE:
                {
                    XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr_list[count].id);
                    return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + count;
                }
            case SAI_SWITCH_ATTR_SWITCH_SHELL_ENABLE:
                {
                    attr_list[count].value.booldata = xpsIsXpshellthreadActive();
                    break;
                }
            case SAI_SWITCH_ATTR_RESTART_WARM:
                {
                    xpsInitType_t initType = xpsGetInitType();
                    if (initType == INIT_WARM)
                    {
                        attr_list[count].value.booldata = true;
                    }
                    else if (initType == INIT_COLD)
                    {
                        attr_list[count].value.booldata = false;
                    }

                    break;
                }
            case SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY:
                {
                    attr_list[count].value.ptr = (sai_pointer_t *)
                                                 switch_notifications_g[xpsDevId].on_switch_state_change;
                    break;
                }
            case SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY:
                {
                    attr_list[count].value.ptr = (sai_pointer_t *)
                                                 switch_notifications_g[xpsDevId].on_switch_shutdown_request;
                    break;
                }
            case SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY:
                {
                    attr_list[count].value.ptr = (sai_pointer_t *)
                                                 switch_notifications_g[xpsDevId].on_fdb_event;
                    break;
                }
            case SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY:
                {
                    attr_list[count].value.ptr = (sai_pointer_t *)
                                                 switch_notifications_g[xpsDevId].on_port_state_change;
                    break;
                }
            case SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY:
                {
                    attr_list[count].value.ptr = (sai_pointer_t *)
                                                 switch_notifications_g[xpsDevId].on_packet_event;
                    break;
                }

            case SAI_SWITCH_ATTR_BFD_SESSION_STATE_CHANGE_NOTIFY:
                {
                    attr_list[count].value.ptr = (sai_pointer_t *)
                                                 switch_notifications_g[xpsDevId].on_bfd_state_change;
                    break;
                }
            case SAI_SWITCH_ATTR_INGRESS_ACL:
                {
                    XP_SAI_LOG_ERR("Unsupported attribute %u\n", attr_list[count].id);
                    return SAI_STATUS_INVALID_PARAMETER;
                    break;
                }
            case SAI_SWITCH_ATTR_EGRESS_ACL:
                {
                    XP_SAI_LOG_ERR("Unsupported attribute %u\n", attr_list[count].id);
                    return SAI_STATUS_INVALID_PARAMETER;
                    break;
                }
            case SAI_SWITCH_ATTR_PORT_MAX_MTU:
                {
                    saiStatus = xpSaiGetSwitchAttrMtuMax(switchId, &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Could not get Max MTU value supported by switch\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL:
                {
                    XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr_list[count].id);
                    return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + count;
                }
            case SAI_SWITCH_ATTR_ACL_STAGE_INGRESS:
                {
                    saiStatus = xpSaiGetSwitchAttrAclStageIngress(switchId,
                                                                  &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Could not get ACL ingress stage supported attributes\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_ACL_STAGE_EGRESS:
                {
                    saiStatus = xpSaiGetSwitchAttrAclStageEgress(switchId, &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Could not get ACL egress stage supported attributes\n");
                        return saiStatus;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_QOS_NUM_LOSSLESS_QUEUES:
                {
                    saiStatus = xpSaiGetSwitchAttrQosNumLosslessQueues(switchId,
                                                                       &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Could not get number of QOS lossless queues\n");
                        return saiStatus;
                    }
                    break;
                }

            case SAI_SWITCH_ATTR_AVAILABLE_IPV4_ROUTE_ENTRY:
                {
                    if (IS_DEVICE_FALCON(devType))
                    {
                        attr_list[count].value.u32 = entryCountCtxPtr->max_ipv4RouteEntries -
                                                     entryCountCtxPtr->ipv4RouteEntries;
                    }
                    else
                    {
                        uint32_t ip4UsedPoolSize = entryCountCtxPtr->ipv4RouteEntries +
                                                   entryCountCtxPtr->ipv6RouteEntries*4;
                        if (ip4UsedPoolSize <=
                            entryCountCtxPtr->max_ipv4RouteEntries)
                        {
                            if ((ALD == devType) ||(ALDDB == devType) ||
                                (ALDRIN2XL == devType) || (IS_DEVICE_AC3X(devType)) ||
                                (IS_DEVICE_AC5X(devType)) || IS_DEVICE_FUJITSU_LARGE(devType) ||
                                IS_DEVICE_FUJITSU_SMALL(devType))
                            {
                                attr_list[count].value.u32 = entryCountCtxPtr->max_ipv4RouteEntries -
                                                             entryCountCtxPtr->ipv4RouteEntries;
                            }
                            else
                            {
                                XP_SAI_LOG_WARNING("Ipv4 route count not supportted for device %d\n", devType);
                                attr_list[count].value.u32 = 0;
                            }
                        }
                        else
                        {
                            XP_SAI_LOG_WARNING("Used count more than maximum count.\n");
                            attr_list[count].value.u32 = 0;
                        }
                    }
                    break;
                }

            case SAI_SWITCH_ATTR_AVAILABLE_IPV6_ROUTE_ENTRY:
                {
                    if (IS_DEVICE_FALCON(devType))
                    {
                        attr_list[count].value.u32 = entryCountCtxPtr->max_ipv6RouteEntries -
                                                     entryCountCtxPtr->ipv6RouteEntries;
                    }
                    else
                    {
                        uint32_t ip6UsedPoolSize = entryCountCtxPtr->ipv6RouteEntries +
                                                   entryCountCtxPtr->ipv4RouteEntries/4;
                        if (ip6UsedPoolSize <=
                            entryCountCtxPtr->max_ipv6RouteEntries)
                        {
                            if ((ALD == devType) ||(ALDDB == devType) ||
                                (ALDRIN2XL == devType) || (IS_DEVICE_AC3X(devType)) ||
                                (IS_DEVICE_AC5X(devType)) || (IS_DEVICE_FUJITSU_LARGE(devType)) ||
                                (IS_DEVICE_FUJITSU_SMALL(devType)))
                            {
                                attr_list[count].value.u32 = entryCountCtxPtr->max_ipv6RouteEntries -
                                                             entryCountCtxPtr->ipv6RouteEntries;
                            }
                            else
                            {
                                XP_SAI_LOG_WARNING("Ipv6 route count not supportted for device %d\n", devType);
                                attr_list[count].value.u32 = 0;
                            }
                        }
                        else
                        {
                            XP_SAI_LOG_WARNING("Used count more than maximum count.\n");
                            attr_list[count].value.u32 = 0;
                        }
                    }
                    break;
                }

            case SAI_SWITCH_ATTR_AVAILABLE_IPV4_NEXTHOP_ENTRY:
            case SAI_SWITCH_ATTR_AVAILABLE_IPV6_NEXTHOP_ENTRY:
                {
                    extern XP_STATUS xpsL3GetHwNhIdCount(uint32_t*);
                    uint32_t nextHopEntries = 0;
                    uint32_t ecmpBlkSize = 0;
                    XP_STATUS ret;
                    /*get the HW NH table size*/
                    ret = xpsL3GetHwNhIdCount(&nextHopEntries);
                    if (ret != XP_NO_ERR)
                    {
                        XP_SAI_LOG_WARNING("get NH HW count failed \n");
                    }
                    /*free NH depends on single NHId counts and ecmp blocks*/
                    ret = xpsL3GetMaxEcmpSize(devId, &ecmpBlkSize);
                    if (ret != XP_NO_ERR)
                    {
                        XP_SAI_LOG_WARNING("get ECMP block size failed \n");
                    }
                    int32_t nhFreePoolSize = entryCountCtxPtr->max_nextHopEntries -
                                             (nextHopEntries +
                                              (ecmpBlkSize*entryCountCtxPtr->nextHopGroupEntries));

                    if (nhFreePoolSize > 0)
                    {
                        attr_list[count].value.u32 = nhFreePoolSize;
                    }
                    else
                    {
                        XP_SAI_LOG_WARNING("nh Used count more than maximum count.\n");
                        attr_list[count].value.u32 = 0;
                    }
                    break;
                }

            case SAI_SWITCH_ATTR_AVAILABLE_NEXT_HOP_GROUP_MEMBER_ENTRY:
                {
                    /*Nh grp member entry depends on single NH ids and
                        unique NH group members, as we can create and add
                        new NH group member to existing NH group upto ecmp block size*/
                    extern XP_STATUS xpsL3GetHwNhIdCount(uint32_t*);
                    uint32_t nextHopEntries = 0;
                    XP_STATUS ret ;
                    /*get the HW NH table size*/
                    ret = xpsL3GetHwNhIdCount(&nextHopEntries);
                    if (ret != XP_NO_ERR)
                    {
                        XP_SAI_LOG_WARNING("get NH HW count failed \n");
                    }
                    int32_t nhFreePoolSize = entryCountCtxPtr->max_nextHopEntries -
                                             (nextHopEntries +
                                              entryCountCtxPtr->nextHopGroupMemberEntries);

                    if (nhFreePoolSize > 0)
                    {
                        attr_list[count].value.u32 = nhFreePoolSize;
                    }
                    else
                    {
                        XP_SAI_LOG_WARNING("nh grp mbr Used count more than maximum count.\n");
                        attr_list[count].value.u32 = 0;
                    }
                    break;
                }
                break;
            /*Neigh count depends on Hash and/or LPM .
              as this is a shared resource between l2 FDB or l3 LPM tables*/
            case SAI_SWITCH_ATTR_AVAILABLE_IPV4_NEIGHBOR_ENTRY:
            case SAI_SWITCH_ATTR_AVAILABLE_IPV6_NEIGHBOR_ENTRY:
                {
                    /*for now, we are using FDB table for Neigh entries,
                    so, remaining count is min(avaibale hash , arp table)*/
                    int32_t fdbFreePoolSize = entryCountCtxPtr->max_fdbEntries -
                                              (entryCountCtxPtr->fdbEntries +
                                               entryCountCtxPtr->ipv4HostEntries +
                                               2*entryCountCtxPtr->ipv6HostEntries);

                    int32_t arpFreeSize = entryCountCtxPtr->max_arpEntries -
                                          entryCountCtxPtr->arpEntries;


                    if (fdbFreePoolSize < 0)
                    {
                        fdbFreePoolSize = 0;
                        XP_SAI_LOG_WARNING("neigh Used count more than maximum count.\n");
                    }
                    if (arpFreeSize < 0)
                    {
                        arpFreeSize = 0;
                        XP_SAI_LOG_WARNING("arp Used count more than maximum count.\n");
                    }
                    else if (attr_list[count].id == SAI_SWITCH_ATTR_AVAILABLE_IPV6_NEIGHBOR_ENTRY)
                    {
                        fdbFreePoolSize /= 2;
                    }
                    attr_list[count].value.u32 = fdbFreePoolSize < arpFreeSize ? fdbFreePoolSize :
                                                 arpFreeSize;
                    break;
                }

            case SAI_SWITCH_ATTR_AVAILABLE_NEXT_HOP_GROUP_ENTRY:
                {
                    if (entryCountCtxPtr->nextHopGroupEntries <=
                        entryCountCtxPtr->max_nextHopGroupEntries)
                    {
                        attr_list[count].value.u32 = entryCountCtxPtr->max_nextHopGroupEntries -
                                                     entryCountCtxPtr->nextHopGroupEntries;
                    }
                    else
                    {
                        XP_SAI_LOG_WARNING("Used count more than maximum count.\n");
                        attr_list[count].value.u32 = 0;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_AVAILABLE_FDB_ENTRY:
                {
                    int32_t fdbFreePoolSize = entryCountCtxPtr->max_fdbEntries -
                                              (entryCountCtxPtr->fdbEntries +
                                               entryCountCtxPtr->ipv4HostEntries +
                                               2*entryCountCtxPtr->ipv6HostEntries);

                    if (fdbFreePoolSize > 0)
                    {
                        attr_list[count].value.u32 = fdbFreePoolSize ;
                    }
                    else
                    {
                        XP_SAI_LOG_WARNING("fdb Used count more than maximum count\n");
                        attr_list[count].value.u32 = 0;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_AVAILABLE_ACL_TABLE:
                {
                    sai_acl_resource_t *acl_resource_list = NULL;
                    if (entryCountCtxPtr->aclTables <= entryCountCtxPtr->max_aclTables)
                    {
                        attr_list[count].value.aclresource.count = 4;
                        acl_resource_list = (sai_acl_resource_t *)calloc(
                                                attr_list[count].value.aclresource.count,
                                                sizeof(sai_acl_resource_t));
                        if (acl_resource_list == NULL)
                        {
                            return XP_ERR_MEM_ALLOC_ERROR;
                        }
                        acl_resource_list[0].stage = SAI_ACL_STAGE_INGRESS;
                        acl_resource_list[0].bind_point = SAI_ACL_BIND_POINT_TYPE_PORT;
                        acl_resource_list[1].stage = SAI_ACL_STAGE_INGRESS;
                        acl_resource_list[1].bind_point = SAI_ACL_BIND_POINT_TYPE_LAG;
                        acl_resource_list[2].stage = SAI_ACL_STAGE_EGRESS;
                        acl_resource_list[2].bind_point = SAI_ACL_BIND_POINT_TYPE_PORT;
                        acl_resource_list[3].stage = SAI_ACL_STAGE_EGRESS;
                        acl_resource_list[3].bind_point = SAI_ACL_BIND_POINT_TYPE_LAG;

                        for (uint32_t i = 0; i < attr_list[count].value.aclresource.count; i++)
                        {
                            acl_resource_list[i].avail_num =
                                entryCountCtxPtr->max_aclTables - entryCountCtxPtr->aclTables;
                        }
                        attr_list[count].value.aclresource.list = acl_resource_list;
                    }
                    else
                    {
                        XP_SAI_LOG_WARNING("Used count more than maximum count.\n");
                        attr_list[count].value.aclresource.list[0].avail_num = 0;
                        attr_list[count].value.aclresource.count = 1;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_AVAILABLE_ACL_TABLE_GROUP:
                {
                    sai_acl_resource_t *acl_resource_list = NULL;
                    if (entryCountCtxPtr->aclTableGroups <= entryCountCtxPtr->max_aclTableGroups)
                    {
                        attr_list[count].value.aclresource.count = 4;
                        acl_resource_list = (sai_acl_resource_t *)calloc(
                                                attr_list[count].value.aclresource.count,
                                                sizeof(sai_acl_resource_t));
                        if (acl_resource_list == NULL)
                        {
                            return XP_ERR_MEM_ALLOC_ERROR;
                        }
                        acl_resource_list[0].stage = SAI_ACL_STAGE_INGRESS;
                        acl_resource_list[0].bind_point = SAI_ACL_BIND_POINT_TYPE_PORT;
                        acl_resource_list[1].stage = SAI_ACL_STAGE_INGRESS;
                        acl_resource_list[1].bind_point = SAI_ACL_BIND_POINT_TYPE_LAG;
                        acl_resource_list[2].stage = SAI_ACL_STAGE_EGRESS;
                        acl_resource_list[2].bind_point = SAI_ACL_BIND_POINT_TYPE_PORT;
                        acl_resource_list[3].stage = SAI_ACL_STAGE_EGRESS;
                        acl_resource_list[3].bind_point = SAI_ACL_BIND_POINT_TYPE_LAG;

                        for (uint32_t i = 0; i < attr_list[count].value.aclresource.count; i++)
                        {
                            acl_resource_list[i].avail_num =
                                entryCountCtxPtr->max_aclTableGroups - entryCountCtxPtr->aclTableGroups;
                        }
                        attr_list[count].value.aclresource.list = acl_resource_list;
                    }
                    else
                    {
                        XP_SAI_LOG_WARNING("Used count more than maximum count.\n");
                        attr_list[count].value.aclresource.list[0].avail_num = 0;
                        attr_list[count].value.aclresource.count = 1;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_MCAST_SNOOPING_CAPABILITY:
                {
                    attr_list[count].value.s32 = (sai_int32_t)
                                                 SAI_SWITCH_MCAST_SNOOPING_CAPABILITY_XG_AND_SG;
                }
                break;

            case SAI_SWITCH_ATTR_TYPE:
                {
                    attr_list[count].value.u32 = SAI_SWITCH_TYPE_NPU;
                    break;
                }
            case SAI_SWITCH_ATTR_NUMBER_OF_SYSTEM_PORTS:
            case SAI_SWITCH_ATTR_SYSTEM_PORT_LIST:
            case SAI_SWITCH_ATTR_MAX_NUMBER_OF_TEMP_SENSORS:
            case SAI_SWITCH_ATTR_AVAILABLE_IPMC_ENTRY:
            case SAI_SWITCH_ATTR_AVAILABLE_SNAT_ENTRY:
            case SAI_SWITCH_ATTR_AVAILABLE_DNAT_ENTRY:
                {
                    return SAI_STATUS_NOT_SUPPORTED;
                }

            case SAI_SWITCH_ATTR_MAX_MIRROR_SESSION:
            case SAI_SWITCH_ATTR_MAX_SAMPLED_MIRROR_SESSION:
                {
                    attr_list[count].value.u32 = XP_SAI_MAX_MIRROR_SESSIONS_NUM;
                    break;
                }
            case SAI_SWITCH_ATTR_VXLAN_DEFAULT_PORT:
                {
                    sai_status_t       status;
                    status = xpSaiGetSwitchAttrVxlanDefaultPort(xpsDevId,
                                                                &attr_list[count].value);
                    if (status != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed xpSaiGetSwitchAttrVxlanDefaultPort: Err code: %d\n",
                                       status);
                        return status;
                    }
                    break;
                }
            case SAI_SWITCH_ATTR_VXLAN_DEFAULT_ROUTER_MAC:
                {
                    sai_status_t       status;
                    status = xpSaiGetSwitchAttrVxlanDefaultRouterMac(xpsDevId,
                                                                     &attr_list[count].value);
                    if (status != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed xpSaiGetSwitchAttrVxlanDefaultRouterMac: Err code: %d\n",
                                       status);
                        return status;
                    }
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Invalid parameter %d for function %s received.\n",
                                   attr_list[count].id, __FUNCNAME__);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiSwitchCreateH1SchedulerBasedOnType(sai_object_id_t
                                                     switchObjId, xpSaiSchedulerProfileType_t schedProfileType)
{
    sai_attribute_t schedulerAttr[4];
    sai_object_id_t schedObjId = SAI_NULL_OBJECT_ID;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    memset(schedulerAttr, 0, sizeof(schedulerAttr));

    switch (schedProfileType)
    {
        case XP_SAI_SCHEDULER_PROFILE_TYPE_H1UC:
        case XP_SAI_SCHEDULER_PROFILE_TYPE_H1MC:
            /* Populate scheduling type (Strict/Wrr/Dwrr). Setting default to DWRR */
            schedulerAttr[0].id        = SAI_SCHEDULER_ATTR_SCHEDULING_TYPE;
            schedulerAttr[0].value.s32 = SAI_SCHEDULING_TYPE_DWRR;

            /* Populate scheduling weight. Setting default to 50 */
            schedulerAttr[1].id       = SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT;
            schedulerAttr[1].value.u8 = 50;
            break;
        case XP_SAI_SCHEDULER_PROFILE_TYPE_H1CTRL:
            /* Populate scheduling type (Strict/Wrr/Dwrr). Setting default to DWRR */
            schedulerAttr[0].id        = SAI_SCHEDULER_ATTR_SCHEDULING_TYPE;
            schedulerAttr[0].value.s32 = SAI_SCHEDULING_TYPE_STRICT;

            /* Populate scheduling weight. Setting default to 50 */
            schedulerAttr[1].id       = SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT;
            schedulerAttr[1].value.u8 = 1;
            break;
        default:
            return saiStatus;
    }

    saiStatus = xpSaiCreateScheduler(&schedObjId, switchObjId, 2, schedulerAttr);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Error: Could not create switch deafult scheduler, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiSwitchSetSchedulerProfileBasedonType(schedProfileType,
                                                          schedObjId);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Error: Could not update switch entry with scheduler, saiStatus: %d\n",
                       saiStatus);
    }
    return saiStatus;
}

sai_status_t xpSaiSwitchSetSchedulerProfileBasedonType(
    xpSaiSchedulerProfileType_t schedProfileType,
    sai_object_id_t schedObjId)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %d!\n", status);
        return xpsStatus2SaiStatus(status);
    }
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve default scheduler from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    switch (schedProfileType)
    {
        case XP_SAI_SCHEDULER_PROFILE_TYPE_DEFAULT:
            pSaiSwitchStaticVar->defaultScheduler = schedObjId;
            break;
        case XP_SAI_SCHEDULER_PROFILE_TYPE_H1UC:
            pSaiSwitchStaticVar->h1UcastScheduler = schedObjId;
            break;
        case XP_SAI_SCHEDULER_PROFILE_TYPE_H1CTRL:
            pSaiSwitchStaticVar->h1CtrlScheduler = schedObjId;
            break;
        case XP_SAI_SCHEDULER_PROFILE_TYPE_H1MC:
            pSaiSwitchStaticVar->h1McastScheduler = schedObjId;
            break;
        case XP_SAI_SCHEDULER_PROFILE_TYPE_NONE:
            return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSwitchSetDefaultScheduler

sai_status_t xpSaiSwitchSetDefaultScheduler(sai_object_id_t defSchedObjId)
{
    return xpSaiSwitchSetSchedulerProfileBasedonType(
               XP_SAI_SCHEDULER_PROFILE_TYPE_DEFAULT,
               defSchedObjId);
}

sai_status_t xpSaiSwitchGetSchedulerProfileBasedonType(
    xpSaiSchedulerProfileType_t schedProfileType,
    sai_object_id_t *schedObjId)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar  = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&key, 0x00, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %d!\n", status);
        return xpsStatus2SaiStatus(status);
    }
    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Failed to retrieve default scheduler from DB\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    switch (schedProfileType)
    {
        case XP_SAI_SCHEDULER_PROFILE_TYPE_DEFAULT:
            *schedObjId = pSaiSwitchStaticVar->defaultScheduler;
            break;
        case XP_SAI_SCHEDULER_PROFILE_TYPE_H1UC:
            *schedObjId = pSaiSwitchStaticVar->h1UcastScheduler;
            break;
        case XP_SAI_SCHEDULER_PROFILE_TYPE_H1CTRL:
            *schedObjId = pSaiSwitchStaticVar->h1CtrlScheduler;
            break;
        case XP_SAI_SCHEDULER_PROFILE_TYPE_H1MC:
            *schedObjId = pSaiSwitchStaticVar->h1McastScheduler;
            break;
        case XP_SAI_SCHEDULER_PROFILE_TYPE_NONE:
            return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSwitchGetDefaultScheduler

sai_status_t xpSaiSwitchGetDefaultScheduler(sai_object_id_t *defSchedObjId)
{
    return xpSaiSwitchGetSchedulerProfileBasedonType(
               XP_SAI_SCHEDULER_PROFILE_TYPE_DEFAULT,
               defSchedObjId);
}


//Func: xpSaiGetSwitchAttrNumberOfNhGroups

sai_status_t xpSaiGetSwitchAttrNumberOfNhGroups(sai_attribute_value_t *value)
{
    XP_STATUS           status              = XP_NO_ERR;
    xpSaiSwitchEntry_t* pSaiSwitchStaticVar = NULL;
    xpSaiSwitchEntry_t  key;

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Invalid parameters received!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&key, 0, sizeof(key));

    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSaiSwitchStaticVar);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find switch entry in DB!\n");
        return xpsStatus2SaiStatus(status);
    }

    if (pSaiSwitchStaticVar == NULL)
    {
        XP_SAI_LOG_ERR("Could not get switch static variables!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    status = xpsL3GetNextHopGroupsMaxNum(&(value->u32));
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get next hop groups max num!\n");
        return xpsStatus2SaiStatus(status);
    }

    return xpsStatus2SaiStatus(status);
}

//Func: xpSaiSwitchApiInit

XP_STATUS xpSaiSwitchApiInit(uint64_t flag,
                             const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiSwitchApiInit\n");

    _xpSaiSwitchApi = (sai_switch_api_t *) xpMalloc(sizeof(sai_switch_api_t));
    if (NULL == _xpSaiSwitchApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiSwitchApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiSwitchApi->create_switch = xpSaiInitializeSwitch;
    _xpSaiSwitchApi->remove_switch = xpSaiShutdownSwitch;
    _xpSaiSwitchApi->set_switch_attribute = xpSaiSetSwitchAttribute;
    _xpSaiSwitchApi->get_switch_attribute = xpSaiGetSwitchAttribute;

    saiRetVal = xpSaiApiRegister(SAI_API_SWITCH, (void*)_xpSaiSwitchApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register SAI switch API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}

//Func: xpSaiSwitchApiDeinit

XP_STATUS xpSaiSwitchApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiSwitchApiDeinit\n");

    xpFree(_xpSaiSwitchApi);
    _xpSaiSwitchApi = NULL;

    return retVal;
}


//Func: xpSaiSwitchUpdateDefaultMaxRouteNum
static void xpSaiSwitchUpdateDefaultMaxRouteNum(int profileId,
                                                int32_t *maxRouteIp4,
                                                int32_t *maxRouteIp6)
{
    switch (profileId)
    {
        case CPSS_HAL_MID_L3_MID_L2_NO_EM:
            *maxRouteIp4 = 288*1024;
            *maxRouteIp6 = 144*1024;
            break;
        case CPSS_HAL_MID_L3_MID_L2_MIN_EM:
            *maxRouteIp4 = 216*1024;
            *maxRouteIp6 = 108*1024;
            break;
        case CPSS_HAL_LOW_MAX_L3_MID_LOW_L2_NO_EM:
            *maxRouteIp4 = 432*1024;
            *maxRouteIp6 = 216*1024;
            break;
        case CPSS_HAL_MAX_L3_MIN_L2_NO_EM:
            *maxRouteIp4 = 504*1024;
            *maxRouteIp6 = 252*1024;
            break;
        default: //* Profile CPSS_HAL_MID_L3_MID_L2_NO_EM */
            *maxRouteIp4 = 288*1024;
            *maxRouteIp6 = 144*1024;
            break;
    }
}


