// xpSaiDev.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiDev_h_
#define _xpSaiDev_h_

#ifndef MAC
#endif /* MAC */
#include "xpDevTypes.h"

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif  /*__cplusplus*/

typedef struct xpSaiWmIpcParam_t
{
    xpInitType_t initType;      /**< Init Type */
    XP_DEV_TYPE_T devType;      /**< Device Type */
    xpsVlan_t defaultVlanId;    /**< Default VLAN ID */
    uint32_t portScanMs;        /**< port state machine polling cycle time */
    uint64_t xpSaiSwitchObjId;  /**< sai switch OID */
    uint32_t numNhGrpEcmpMember; /**< Default next hop group max ecmp size */
} xpSaiWmIpcParam_t;

const char *sai_sdk_dev_name_get(sai_switch_profile_id_t profile_id);

int sai_sdk_dev_type_get(const char* switch_hardware_id,
                         xpSdkDevType_t* devType);

int sai_sdk_init(const char* devName,
                 const xpSaiSwitchProfile_t* profile,
                 xpsRangeProfileType_t rpType);

sai_status_t sai_sdk_dev_add(xpsDevice_t devId, xpsInitType_t initType,
                             xpDevConfigStruct *devConfig);

sai_status_t sai_dev_config(xpDevice_t deviceId, void* arg);

sai_status_t xpSaiComponentsInit(xpsDevice_t xpSaiDevId);

sai_status_t xpSaiComponentsDeInit(xpsDevice_t xpSaiDevId);

XP_STATUS sai_sdk_dev_remove(xpsDevice_t devId);

void *sai_dev_ipc_handler(void *arg);

pthread_t sai_thread_create(const char *name, void *(*start)(void *),
                            void *arg);
XP_STATUS  sai_thread_delete(pthread_t thread);
typedef void * std_parsed_string_t;
typedef bool (*sai_shell_check_run_function)(void * param,
                                             const char *cmd_string);
typedef void (*sai_shell_function)(std_parsed_string_t handle);
bool sai_shell_cmd_add_flexible(void * param, sai_shell_check_run_function fun);
bool sai_shell_cmd_add(const char *cmd_name, sai_shell_function fun,
                       const char *description);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*_xpSaiDev_h_*/

