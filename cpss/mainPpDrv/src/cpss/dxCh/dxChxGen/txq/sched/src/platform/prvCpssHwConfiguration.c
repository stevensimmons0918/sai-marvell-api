/*
 * (c), Copyright 2009-2014, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief basic H/W configuration functions implementation
 *
* @file cpss_hw_configuration.c
*
* $Revision: 2.0 $
 */

#include <string.h>


#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedHwConfigurationInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvShedPlatformImplementationDefinitions.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

extern GT_STATUS  prvCpssSip6TxqPdqSchedulerHwParamsByLevelGet
(
    IN GT_U32 devNum,
    OUT  GT_U32 *maxPnodePtr,
    OUT  GT_U32 *maxCnodePtr,
    OUT  GT_U32 *maxBnodePtr,
    OUT  GT_U32 *maxAnodePtr,
    OUT  GT_U32 *maxQnodePtr
);


/***********************************************************************************/

void*   prvCpssCreateNewEnvironmentHandle(GT_U8 devNumber)
{
    struct cpssSchedEnv * hEnv;
    hEnv = (struct cpssSchedEnv *)schedMalloc(sizeof(struct cpssSchedEnv));
    hEnv->magicNumber= HENV_MAGIC;
    hEnv->devNumber = devNumber;
    return (void *) hEnv;
}




int prvCpssSchedSetGlobalAddress(void * hEnv,GT_U32 pdqNum,GT_U32 globalAddress)
{
    CPSS_ENV(env,hEnv, -1, -1)
    env->globalAddress = globalAddress;
    env->pdqNumber = pdqNum;

    return 0;
}


int get_tm_globalAddress(void * hEnv,GT_U32 * globalAddressPtr)
{
    CPSS_ENV(env,hEnv, -1, -1)
    *globalAddressPtr= env->globalAddress;

    return 0;
}



unsigned int prvCpssSchedInitHardwareConfiguration(void * hEnv)
{
    CPSS_ENV(env, hEnv, 1, 1)

    return 0;
}



int prvCpssSchedGetPortCount(void * hEnv)
{
    CPSS_ENV(env,hEnv, -1, -1)
    return env->tree_structure.numOfPorts;
}
int prvCpssSchedGetAnodesCount(void * hEnv)
{
    CPSS_ENV(env,hEnv, -1, -1)
    return env->tree_structure.numOfAnodes;
}
int prvCpssSchedGetBnodesCount(void * hEnv)
{
    CPSS_ENV(env,hEnv, -1, -1)
    return env->tree_structure.numOfBnodes;
}
int prvCpssSchedGetCnodesCount(void * hEnv)
{
    CPSS_ENV(env,hEnv, -1, -1)
    return env->tree_structure.numOfCnodes;
}
int prvCpssSchedGetQNodesCount(void * hEnv)
{
    CPSS_ENV(env,hEnv, -1, -1)
    return env->tree_structure.numOfQueues;
}

int prvCpssSchedDevNumGet(void * hEnv)
{
    CPSS_ENV(env,hEnv, -1, -1)
    return env->devNumber;
}

int prvCpssSchedGetAnodesNullNode(void * hEnv)
{
    int data;
    GT_U8 devNum =0 ;
    GT_U32  maxAnode;

    data = prvCpssSchedDevNumGet(hEnv);

    if(-1!=data)
    {
        devNum = (GT_U8)data;
    }

    (void)prvCpssSip6TxqPdqSchedulerHwParamsByLevelGet(devNum,NULL,NULL,NULL,&maxAnode,NULL);

    return maxAnode-1;

}

int prvCpssSchedGetBnodesNullNode(void * hEnv)
{
    int data;
    GT_U8 devNum =0 ;
    GT_U32  maxBnode;

    data = prvCpssSchedDevNumGet(hEnv);

    if(-1!=data)
    {
        devNum = (GT_U8)data;
    }

    (void)prvCpssSip6TxqPdqSchedulerHwParamsByLevelGet(devNum,NULL,NULL,&maxBnode,NULL,NULL);

    return maxBnode-1;

}

int prvCpssSchedGetQnodesNullNode(void * hEnv)
{
    int data;
    GT_U8 devNum =0 ;
    GT_U32  maxQnode;

    data = prvCpssSchedDevNumGet(hEnv);

    if(-1!=data)
    {
        devNum = (GT_U8)data;
    }

    (void)prvCpssSip6TxqPdqSchedulerHwParamsByLevelGet(devNum,NULL,NULL,NULL,NULL,&maxQnode);

    return maxQnode-1;

}


/****************************************************************************************/

#define TM_MAX_PORT_QUANTUM  0xfff  /* Field is  12  bits. */
#define TM_MAX_NODE_QUANTUM 0x3fff  /* Field is 14 bits. */

#define NODE_QUANTUM_CHUNK_SIZE     256
#define NODE_QUANTUM_RANGE          256

#define PORT_QUANTUM_CHUNK_SIZE     128 /*64 - legacy*/

unsigned int    prvCpssSchedGetNodeQuantumChunkSize() { return NODE_QUANTUM_CHUNK_SIZE; }
unsigned int    prvCpssSchedGetNodeQuantumRange()     { return NODE_QUANTUM_RANGE; }
unsigned int    prvCpssSchedGetPortQuantumChunkSize() { return PORT_QUANTUM_CHUNK_SIZE; }

unsigned int    prvCpssSchedGetNodeMaxQuantumChunks()     { return TM_MAX_NODE_QUANTUM; }
unsigned int    prvCpssSchedGetPortMaxQuantumChunks()     { return TM_MAX_PORT_QUANTUM; }
