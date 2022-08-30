#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsTask.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define UNUSED_PARAMETER(x) x = x

#if defined  CPSS_USE_MUTEX_PROFILER

#if (defined(LINUX)&&defined (V2L_PTHREADS)) ||  defined(WIN32)

#include <gtOs/gtOsSemDefs.h>

#define GT_PRINT_DEBUG_MSG(_counterName)  printf("Granular locking violation counter %s  incremented. \nTo debug put breakpoint in  file gtMutexProfiler.c line %d \n",#_counterName,  __LINE__)


#if  defined(WIN32)
#define MUTEX_ARRAY_SIZE  userspaceMutexes.allocated
#define MUTEX_ARRAY_LIST(_index)  userspaceMutexes.list[_index]
#define MUTEX_TYPE    GT_WIN32_MUTEX
#elif defined(LINUX)
#define MUTEX_ARRAY_SIZE PRV_SHARED_DB_SEMAPHORES.allocated
#define MUTEX_ARRAY_LIST(_index)  PRV_SHARED_DB_SEMAPHORES.list[_index]
#define MUTEX_TYPE    _V2L_semSTC
#endif

extern GT_BOOL  cpssInitSystemIsDone;

static GT_U32  violationGlobalAfterDeviceSpecificCount = 0;         /* global after device specific*/
static GT_U32  violationRxTxConfigurationMixCount = 0;              /*rx/tx mixed with configuration*/
static GT_U32  violationZeroLevelCount = 0;                         /* zero level violation*/
static GT_U32  violationSystemRecoveryCount = 0;                    /* system recovery violation*/
static GT_U32  violationMangersMixCount = 0;                        /*manager mixed with other manager*/
static GT_U32  violationMangersRxTxConfigurationMixCount = 0;       /*manager mix with rx/tx or configuration*/
static GT_U32  violationUnprotectedDbAccessCount = 0;               /*access to device data base is not protected by mutex*/

#endif

#if  defined(WIN32)
extern OS_OBJECT_LIST_STC userspaceMutexes ;
#elif (defined(LINUX)&&defined (V2L_PTHREADS))
extern OS_OBJECT_LIST_STC semaphores;
#endif

/*the fuction gets mutex iterator and return GT_TRUE if it needs to be skip*/
static GT_BOOL mtxIteratorCheckAndSkip
(
    MUTEX_TYPE * mtxPtr
)
{
    GT_BOOL ownedBySameThread=GT_FALSE;

    /*skip undefined mutex*/
    if (!mtxPtr || mtxPtr->header.type == 0)
    {
        return GT_TRUE;
    }
    /*skip non granular locking*/
#if  defined(WIN32)
    if (mtxPtr->header.glType == GRANULAR_LOCKING_TYPE_NONE)
    {
        return GT_TRUE;
    }
    ownedBySameThread = ((GT_U32)mtxPtr->mtx.OwningThread == GetCurrentThreadId());
#elif defined(LINUX)
    /*skip non mutex and non granular locking*/
    if( (mtxPtr->header.type != SEMTYPE_MUTEX)||(mtxPtr->header.glType == GRANULAR_LOCKING_TYPE_NONE))
    {
        return GT_TRUE;
    }
    ownedBySameThread = ( pthread_equal(mtxPtr->mtxOwner,pthread_self()))?GT_TRUE:GT_FALSE;
#endif

    if(ownedBySameThread == GT_FALSE)
    {
        return GT_TRUE;
    }
    return GT_FALSE;
}

/*******************************************************************************
* osMutexSetGlAttributes
*
* DESCRIPTION:
*       Set granular locking attributes to mutex. Such as type of the mutex related to the granular locking
*       and device number.
*
* INPUTS:
*       mtxid    - mutex id
*       isZeroLevel - GT_TRUE if the mutex is for zero level protection ,GT_FALSE otherwise
*       isDeviceOriented - GT_TRUE if the mutex is per device protection ,GT_FALSE otherwise
*       isRxTxFunctionality - GT_TRUE if the mutex is per rx/tx functionality protection ,GT_FALSE otherwise
*       devNum - device number,relevant only if isDeviceOriented is GT_TRUE
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*******************************************************************************/

GT_STATUS osMutexSetGlAttributes
(
    IN GT_MUTEX                           mtxId,
    IN GT_BOOL                            isZeroLevel,
    IN GT_BOOL                            isDeviceOriented,
    IN GT_BOOL                            isRxTxFunctionality,
    IN GT_U8                              devNum,
    IN GT_BOOL                            isSystemRecovery,
    IN GT_BOOL                            isManager,
    IN GT_U32                             manager
)
{

#if (defined(LINUX)&&defined (V2L_PTHREADS)) ||  defined(WIN32)

    CHECK_MTX(mtxId);

    if(isZeroLevel == GT_TRUE)
    {
        mtx->header.glType = GRANULAR_LOCKING_TYPE_ZERO_LEVEL;
        return GT_OK;
    }
    if(isSystemRecovery == GT_TRUE)
    {
        mtx->header.glType = GRANULAR_LOCKING_TYPE_SYSTEM_RECOVERY;
        return GT_OK;
    }
    if (isManager == GT_TRUE)
    {
        /* Per Device managers */
        if (isDeviceOriented == GT_TRUE)
        {
            switch (manager)
            {
                case PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRUNK_MANAGER_CNS:
                {
                    mtx->header.glType = GRANULAR_LOCKING_TYPE_TRUNK_MANAGER_CONFIGURATION;
                }
                break;
                case PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS:
                {
                    mtx->header.glType = GRANULAR_LOCKING_TYPE_IPFIX_MANAGER_CONFIGURATION;
                }
                break;
                default:
                break;
            }
        }
        else
        {
            switch(manager)
            {
                case PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS:
                {
                    mtx->header.glType = GRANULAR_LOCKING_TYPE_FDB_MANAGER_CONFIGURATION;
                }
                break;
                case PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS:
                {
                    mtx->header.glType = GRANULAR_LOCKING_TYPE_EXACT_MATCH_MANAGER_CONFIGURATION;
                }
                break;
                case PRV_CPSS_FUNCTIONALITY_LPM_MANAGER_CNS:
                {
                    mtx->header.glType = GRANULAR_LOCKING_TYPE_LPM_MANAGER_CONFIGURATION;
                }
                break;
                case PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS:
                {
                    mtx->header.glType = GRANULAR_LOCKING_TYPE_TCAM_MANAGER_CONFIGURATION;
                }
                break;
                default:
                break;
            }
        }
        if (isDeviceOriented==GT_TRUE)
        {
            mtx->header.devNum = devNum;
        }
        return GT_OK;
    }

    if(isDeviceOriented == GT_TRUE)
    {
        if(isRxTxFunctionality == GT_TRUE)
        {
            mtx->header.glType = GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_RX_TX;
        }
        else
        {
            mtx->header.glType = GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_CONFIGURATION;
        }
        mtx->header.devNum = devNum;
    }
    else /*deviceOriented == GT_FALSE*/
    {
        if(isRxTxFunctionality == GT_TRUE)
        {
            mtx->header.glType = GRANULAR_LOCKING_TYPE_DEVICELESS_RX_TX;
        }
        else
        {
            mtx->header.glType = GRANULAR_LOCKING_TYPE_DEVICELESS_CONFIGURATION;
        }
    }
#endif

    return GT_OK;
}
/*******************************************************************************
* osMutexLockGlValidation
*
* DESCRIPTION:
*       This function check that locking mutex does not violate granular locking convention
*
* INPUTS:
*       mtxid    - mutex id
*
*
* OUTPUTS:
*       none
*
* RETURNS:
*       GT_OK - validation succeeded
*       GT_FAIL -  mutex id is not ok
*
* COMMENTS:
*      The rules of granular locking :
*           1. When thread take zero level mutex, this thread can not take additional mutexes until this mutex is unlocked
*           2. Thread can not take non device oriented mutex after it took device oriented mutex
*           3. Thread can not mix rx/tx mutexes with configuration mutexes.
*
*******************************************************************************/
GT_STATUS osMutexLockGlValidation
(
    IN GT_MUTEX mtxid
)
{
#if (defined(LINUX)&&defined (V2L_PTHREADS)) ||  defined(WIN32)


    int i;
    MUTEX_TYPE *mtxIterator;
    MUTEX_TYPE *tmpMutex;

    GT_BOOL ownedBySameThread,skip;


    CHECK_MTX(mtxid);

    /*skip non granular locking*/
#if  defined(WIN32)
    if (mtx->header.glType == GRANULAR_LOCKING_TYPE_NONE)
    {
        return GT_TRUE;
    }
#elif defined(LINUX)
    /*skip non mutex and non granular locking*/
    if( (mtx->header.type != SEMTYPE_MUTEX)||(mtx->header.glType == GRANULAR_LOCKING_TYPE_NONE))
    {
        return GT_TRUE;
    }
#endif


    /*check zero level violation*/
    tmpMutex =  (MUTEX_TYPE * )MUTEX_ARRAY_LIST(
        PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtxZeroLevel));
#if  defined(WIN32)
    ownedBySameThread = ((GT_U32)tmpMutex->mtx.OwningThread == GetCurrentThreadId());
#elif defined(LINUX)
    ownedBySameThread = ( pthread_equal(tmpMutex->mtxOwner,pthread_self()))?GT_TRUE:GT_FALSE;
#endif
    if(ownedBySameThread && (mtx->header.glType != GRANULAR_LOCKING_TYPE_ZERO_LEVEL))
    {
        violationZeroLevelCount++;
        GT_PRINT_DEBUG_MSG(violationZeroLevelCount);
    }

    /*check system recovery violation*/
    tmpMutex =  (MUTEX_TYPE * )MUTEX_ARRAY_LIST(
        PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS]));
    if (tmpMutex)
    {
    #if  defined(WIN32)
        ownedBySameThread = ((GT_U32)tmpMutex->mtx.OwningThread == GetCurrentThreadId());
    #elif defined(LINUX)
        ownedBySameThread = ( pthread_equal(tmpMutex->mtxOwner,pthread_self()))?GT_TRUE:GT_FALSE;
    #endif
        if(ownedBySameThread && (mtx->header.glType != GRANULAR_LOCKING_TYPE_SYSTEM_RECOVERY))
        {
            violationSystemRecoveryCount++;
            GT_PRINT_DEBUG_MSG(violationSystemRecoveryCount);
        }
    }

    switch (mtx->header.glType)
    {
    case GRANULAR_LOCKING_TYPE_DEVICELESS_CONFIGURATION:
        for (i = 1; i < MUTEX_ARRAY_SIZE; i++)
        {
            mtxIterator = (MUTEX_TYPE * )MUTEX_ARRAY_LIST(i);
            skip = mtxIteratorCheckAndSkip(mtxIterator);
            if (skip == GT_TRUE)
            {
                continue;
            }

            /*check that currently there is no per device mutex that is taken by the same thread*/
            if( mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_CONFIGURATION)
            {
                violationGlobalAfterDeviceSpecificCount++;
                GT_PRINT_DEBUG_MSG(violationGlobalAfterDeviceSpecificCount);
            }

            /*check that same thread is not taken rx/tx functionality mutex*/
            if( (mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICELESS_RX_TX) ||(mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_RX_TX))
            {
                violationRxTxConfigurationMixCount++;
                GT_PRINT_DEBUG_MSG(violationRxTxConfigurationMixCount);
            }
        }
        break;
    case GRANULAR_LOCKING_TYPE_DEVICELESS_RX_TX:
        for (i = 1; i < MUTEX_ARRAY_SIZE; i++)
        {
            mtxIterator = (MUTEX_TYPE * )MUTEX_ARRAY_LIST(i);
            skip = mtxIteratorCheckAndSkip(mtxIterator);
            if (skip == GT_TRUE)
            {
                continue;
            }

            /*check that currently there is no per device mutex that is taken by the same thread*/
            if( mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_RX_TX)
            {
                violationGlobalAfterDeviceSpecificCount++;
                GT_PRINT_DEBUG_MSG(violationGlobalAfterDeviceSpecificCount);
            }

            /*check that same thread is not taken rx/tx functionality mutex*/
            if((mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICELESS_CONFIGURATION) ||(mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_CONFIGURATION))
            {
                violationRxTxConfigurationMixCount++;
                GT_PRINT_DEBUG_MSG(violationRxTxConfigurationMixCount);
            }
        }
        break;
    case GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_CONFIGURATION:
        for (i = 1; i < MUTEX_ARRAY_SIZE; i++)
        {
            mtxIterator = (MUTEX_TYPE * )MUTEX_ARRAY_LIST(i);
            skip = mtxIteratorCheckAndSkip(mtxIterator);
            if (skip == GT_TRUE)
            {
                continue;
            }

            /*check that same thread is not taken rx/tx functionality mutex*/
            if((mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICELESS_RX_TX) ||(mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_RX_TX))
            {
                violationRxTxConfigurationMixCount++;
                GT_PRINT_DEBUG_MSG(violationRxTxConfigurationMixCount);
            }
        }
        break;
    case GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_RX_TX:
        for (i = 1; i < MUTEX_ARRAY_SIZE; i++)
        {
            mtxIterator = (MUTEX_TYPE * )MUTEX_ARRAY_LIST(i);
            skip = mtxIteratorCheckAndSkip(mtxIterator);
            if (skip == GT_TRUE)
            {
                continue;
            }

            /*check that same thread is not taken rx/tx functionality mutex*/
            if ((mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICELESS_CONFIGURATION) ||(mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_CONFIGURATION))
            {
                violationRxTxConfigurationMixCount++;
                GT_PRINT_DEBUG_MSG(violationRxTxConfigurationMixCount);
            }
        }
        break;
    case GRANULAR_LOCKING_TYPE_TRUNK_MANAGER_CONFIGURATION:
    case GRANULAR_LOCKING_TYPE_IPFIX_MANAGER_CONFIGURATION:
    case GRANULAR_LOCKING_TYPE_FDB_MANAGER_CONFIGURATION:
    case GRANULAR_LOCKING_TYPE_EXACT_MATCH_MANAGER_CONFIGURATION:
    case GRANULAR_LOCKING_TYPE_LPM_MANAGER_CONFIGURATION:
    case GRANULAR_LOCKING_TYPE_TCAM_MANAGER_CONFIGURATION:
        for (i = 1; i < MUTEX_ARRAY_SIZE; i++)
        {
            mtxIterator = (MUTEX_TYPE * )MUTEX_ARRAY_LIST(i);
            skip = mtxIteratorCheckAndSkip(mtxIterator);
            if (skip == GT_TRUE)
            {
                continue;
            }

            /*check that same thread is not taken by any type of mutex that is not manager mutex*/
            if ((mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICELESS_CONFIGURATION) ||
                (mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_CONFIGURATION)||
                (mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICELESS_RX_TX) ||
                (mtxIterator->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_RX_TX))
            {
                violationMangersRxTxConfigurationMixCount++;
                GT_PRINT_DEBUG_MSG(violationMangersRxTxConfigurationMixCount);
            }
            else
            {
                /*check that same thread is not taken by any other manager mutex */
                if ((mtxIterator->header.glType != GRANULAR_LOCKING_TYPE_ZERO_LEVEL )&&
                    (mtxIterator->header.glType != GRANULAR_LOCKING_TYPE_SYSTEM_RECOVERY)&&
                    (mtxIterator->header.glType != mtx->header.glType))
                {
                    violationMangersMixCount++;
                    GT_PRINT_DEBUG_MSG(violationMangersMixCount);
                }
            }
        }
        break;
    default:
        break;
    }

#endif

    return GT_OK;

}

/*******************************************************************************
* osCheckUnprotectedPerDeviceDbAccess
*
* DESCRIPTION:
*      This function checks that device related mutex is locked when accessing device
*       related software database
*
* INPUTS:
*       devNum              - device mumber
*       functionName    - name of the function that is being checked
*
* OUTPUTS:
*       None
*
* RETURNS:
*      NONE
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID osCheckUnprotectedPerDeviceDbAccess
(
    IN GT_U32                                 devNum,
    IN const char *                           functionName
)
{

#if (defined(LINUX)&&defined (V2L_PTHREADS)) ||  defined(WIN32)


    int m;
    GT_BOOL dbProtected = GT_FALSE;
    GT_BOOL devicelessMutexLocked = GT_FALSE;
    GT_BOOL skip;
#if  defined(WIN32)
    GT_WIN32_MUTEX *mtx;
#elif defined(LINUX)
    _V2L_semSTC *mtx;
#endif

    /*start count only after system is initialized*/
    if(cpssInitSystemIsDone == GT_FALSE)
    {
        return ;
    }

     for (m = 1; m< MUTEX_ARRAY_SIZE; m++)
    {
        mtx = (MUTEX_TYPE * )MUTEX_ARRAY_LIST(m);

        skip = mtxIteratorCheckAndSkip(mtx);
        if (skip == GT_TRUE)
        {
            continue;
        }

        if( (mtx->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_CONFIGURATION)||(mtx->header.glType == GRANULAR_LOCKING_TYPE_DEVICE_ORIENTED_RX_TX))
        {
            if (mtx->header.devNum == devNum)
            {
                dbProtected = GT_TRUE;
                break;
            }
        }

        if( (mtx->header.glType == GRANULAR_LOCKING_TYPE_DEVICELESS_CONFIGURATION)||(mtx->header.glType == GRANULAR_LOCKING_TYPE_DEVICELESS_RX_TX))
        {
                devicelessMutexLocked = GT_TRUE;
        }
    }

    if((dbProtected == GT_FALSE)&&(devicelessMutexLocked == GT_TRUE))
    {
        /*Need to check that general mutex is locked in order to negate cases where macro is called directly from enhnaced UT of from LUA.
                    Those cases are not intresting*/

        /*Filter some function because those are known cases*/
        if(cpssOsStrCmp("deviceIsPrestera",functionName)!=0)
        {
                printf("Unprotected db access in function %s\n",functionName);
                violationUnprotectedDbAccessCount++;
                GT_PRINT_DEBUG_MSG(violationUnprotectedDbAccessCount);
        }



    }
#endif

}

/*******************************************************************************
* osMutexCountersStatistic
*
* DESCRIPTION:
*      This function update violation counters pointers.
*
* INPUTS:
*       None
*
*
* OUTPUTS:
*       violationGlobalAfterDeviceSpecificCountPtr -Get the value of violationGlobalAfterDeviceSpecificCount
*       violationRxTxConfigurationMixCountPtr -Get the value of violationRxTxConfigurationMixCount
*       violationZeroLevelCountPtr - Get the value of violationZeroLevelCount
*       violationUnprotectedDbAccessCountPtr - Get the value of violationUnprotectedDbAccessCount
*       violationSystemRecoveryCountPtr - Get the value of violationSystemRecoveryCount
*       violationMangersMixCountPtr - Get the value of violationMangersMixCount
*       violationMangersRxTxConfigurationMixCountPtr - Get the value of violationMangersRxTxConfigurationMixCount
* RETURNS:
*      GT_OK - in case of success
*      GT_NOT_IMPLEMENTED - in case that not correct library is used while linking
*      GT_NOT_SUPPORTED -   in case  that granular locking feature flag is not raised during the compilation
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS  osMutexCountersStatistic
(
    OUT GT_U32  * violationGlobalAfterDeviceSpecificCountPtr,
    OUT GT_U32  * violationRxTxConfigurationMixCountPtr,
    OUT GT_U32  * violationZeroLevelCountPtr,
    OUT GT_U32  * violationUnprotectedDbAccessCountPtr,
    OUT GT_U32  * violationSystemRecoveryCountPtr,
    OUT GT_U32  * violationMangersMixCountPtr,
    OUT GT_U32  * violationMangersRxTxConfigurationMixCountPtr
)
{
#if (defined(LINUX)&&defined (V2L_PTHREADS)) ||  defined(WIN32)

    *violationGlobalAfterDeviceSpecificCountPtr = violationGlobalAfterDeviceSpecificCount;
    *violationRxTxConfigurationMixCountPtr  = violationRxTxConfigurationMixCount;
    *violationZeroLevelCountPtr = violationZeroLevelCount;
    *violationUnprotectedDbAccessCountPtr = violationUnprotectedDbAccessCount;
    *violationSystemRecoveryCountPtr = violationSystemRecoveryCount;
    *violationMangersMixCountPtr = violationMangersMixCount;
    *violationMangersRxTxConfigurationMixCountPtr = violationMangersRxTxConfigurationMixCount;
    return GT_OK;
#else
    UNUSED_PARAMETER(violationGlobalAfterDeviceSpecificCountPtr);
    UNUSED_PARAMETER(violationRxTxConfigurationMixCountPtr);
    UNUSED_PARAMETER(violationZeroLevelCountPtr);
    UNUSED_PARAMETER(violationUnprotectedDbAccessCountPtr);
    UNUSED_PARAMETER(violationSystemRecoveryCountPtr);
    UNUSED_PARAMETER(violationMangersMixCountPtr);
    UNUSED_PARAMETER(violationMangersRxTxConfigurationMixCountPtr);
    return GT_NOT_IMPLEMENTED;
#endif


}

/*******************************************************************************
* osMutexErrorsCreate
*
* DESCRIPTION:
*      This function update violation counters pointers.
*
*
* INPUTS:
*       violationGlobalAfterDeviceSpecificCountError -if GT_TRUE then create global after device specific error
*       violationRxTxConfigurationMixCountError -if GT_TRUE then create Rx/Tx configuration Mix error
*       violationZeroLevelCountError -if GT_TRUE then create zero level error
*       violationUnprotectedDbAccessCountError - if GT_TRUE then create unprotected db access error
* OUTPUTS:
*   None
*
* RETURNS:
*      GT_OK - in case of success
*      GT_NOT_IMPLEMENTED - in case that not correct library is used while linking
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS  osMutexErrorsCreate
(
    IN GT_BOOL   violationGlobalAfterDeviceSpecificCountError,
    IN GT_BOOL   violationRxTxConfigurationMixCountError,
    IN GT_BOOL   violationZeroLevelCountError,
    IN GT_BOOL   violationUnprotectedDbAccessCountError,
    IN GT_BOOL   violationSystemRecoveryCountError,
    IN GT_BOOL   violationMangersMixCountError,
    IN GT_BOOL   violationMangersRxTxConfigurationMixCountError
)
{
#if (defined(LINUX)&&defined (V2L_PTHREADS)) ||  defined(WIN32)

    if(violationGlobalAfterDeviceSpecificCountError == GT_TRUE)
    {
       /*take device oriented mutex*/
       CPSS_API_LOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
       /*take non - device oriented mutex*/
       CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
       /*release non - device oriented mutex*/
       CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
       /*release device oriented mutex*/
       CPSS_API_UNLOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }

    if(violationRxTxConfigurationMixCountError == GT_TRUE)
    {
       /*take device oriented mutex - configuration*/
       CPSS_API_LOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        /*take device oriented mutex - rx-tx*/
       CPSS_API_LOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
       /*release device oriented mutex - rx-tx*/
       CPSS_API_UNLOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
       /*release device oriented mutex - configuration*/
       CPSS_API_UNLOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }

    if(violationZeroLevelCountError == GT_TRUE)
    {
       /*take zero level mutex*/
       CPSS_ZERO_LEVEL_API_LOCK_MAC
       /*take device oriented mutex */
       CPSS_API_LOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
       /*release device oriented mutex */
       CPSS_API_UNLOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
       /*release zero level mutex*/
       CPSS_ZERO_LEVEL_API_UNLOCK_MAC
    }

    if(violationUnprotectedDbAccessCountError == GT_TRUE)
    {
       /*take non - device oriented mutex*/
       CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

 #ifdef  WIN32
       osCheckUnprotectedPerDeviceDbAccess(10,__FUNCTION__);
 #else
       osCheckUnprotectedPerDeviceDbAccess(10,__func__);
 #endif

       /*release non - device oriented mutex*/
       CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }

    if(violationSystemRecoveryCountError == GT_TRUE)
    {
        /*take system recovery mutex*/
        CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
        /*take device oriented mutex */
        CPSS_API_LOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        /*release device oriented mutex */
        CPSS_API_UNLOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        /*release system recovery mutex*/
        CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
    }

    if(violationMangersMixCountError == GT_TRUE)
    {
        /*take LPM manager mutex*/
        CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_LPM_MANAGER_CNS);
        /*take TCAM manager mutex*/
        CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
        /*release TCAM manager mutex*/
        CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
        /*release LPM manager mutex*/
        CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_LPM_MANAGER_CNS);
    }

    if(violationMangersRxTxConfigurationMixCountError == GT_TRUE)
    {
        /*take device oriented mutex - configuration*/
        CPSS_API_LOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        /*take TCAM manager mutex*/
        CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
        /*release TCAM manager mutex*/
        CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
        /*take device oriented mutex - configuration*/
        CPSS_API_UNLOCK_MAC(0,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }

    return GT_OK;
#else
    return GT_NOT_IMPLEMENTED;
#endif

}

#else /*defined  CPSS_USE_MUTEX_PROFILER   */
/*******************************************************************************
* osMutexCountersStatistic
*
* DESCRIPTION:
*      This function update violation counters pointers.
*
* INPUTS:
*       None
*
*
* OUTPUTS:
*       violationGlobalAfterDeviceSpecificCountPtr -Get the value of violationGlobalAfterDeviceSpecificCount
*       violationRxTxConfigurationMixCountPtr -Get the value of violationRxTxConfigurationMixCount
*       violationZeroLevelCountPtr - Get the value of violationZeroLevelCount
*       violationUnprotectedDbAccessCountPtr - Get the value of violationUnprotectedDbAccessCount
*       violationSystemRecoveryCountPtr - Get the value of violationSystemRecoveryCount
*       violationMangersMixCountPtr - Get the value of violationMangersMixCount
*       violationMangersRxTxConfigurationMixCountPtr - Get the value of violationMangersRxTxConfigurationMixCount
*
* RETURNS:
*      GT_OK - in case of success
*      GT_NOT_IMPLEMENTED - in case that not correct library is used while linking
*      GT_NOT_SUPPORTED -   in case  that granular locking feature flag is not raised during the compilation
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS  osMutexCountersStatistic
(
    OUT GT_U32  * violationGlobalAfterDeviceSpecificCountPtr,
    OUT GT_U32  * violationRxTxConfigurationMixCountPtr,
    OUT GT_U32  * violationZeroLevelCountPtr,
    OUT GT_U32  * violationUnprotectedDbAccessCountPtr,
    OUT GT_U32  * violationSystemRecoveryCountPtr,
    OUT GT_U32  * violationMangersMixCountPtr,
    OUT GT_U32  * violationMangersRxTxConfigurationMixCountPtr
)
{
    UNUSED_PARAMETER(violationGlobalAfterDeviceSpecificCountPtr);
    UNUSED_PARAMETER(violationRxTxConfigurationMixCountPtr);
    UNUSED_PARAMETER(violationZeroLevelCountPtr);
    UNUSED_PARAMETER(violationUnprotectedDbAccessCountPtr);
    UNUSED_PARAMETER(violationSystemRecoveryCountPtr);
    UNUSED_PARAMETER(violationMangersMixCountPtr);
    UNUSED_PARAMETER(violationMangersRxTxConfigurationMixCountPtr);
    return GT_NOT_SUPPORTED;
}

/*******************************************************************************
* osMutexErrorsCreate
*
* DESCRIPTION:
*      This function update violation counters pointers.
*
*
* INPUTS:
*       violationGlobalAfterDeviceSpecificCountError -if GT_TRUE then create global after device specific error
*       violationRxTxConfigurationMixCountError -if GT_TRUE then create Rx/Tx configuration Mix error
*       violationZeroLevelCountError -if GT_TRUE then create zero level error
*       violationUnprotectedDbAccessCountError - if GT_TRUE then create unprotected db access error
* OUTPUTS:
*   None
*
* RETURNS:
*      GT_OK - in case of success
*      GT_NOT_IMPLEMENTED - in case that not correct library is used while linking
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS  osMutexErrorsCreate
(
    IN GT_BOOL   violationGlobalAfterDeviceSpecificCountError,
    IN GT_BOOL   violationRxTxConfigurationMixCountError,
    IN GT_BOOL   violationZeroLevelCountError,
    IN GT_BOOL   violationUnprotectedDbAccessCountError,
    IN GT_BOOL   violationSystemRecoveryCountError,
    IN GT_BOOL   violationMangersMixCountError,
    IN GT_BOOL   violationMangersRxTxConfigurationMixCountError
)
{
    UNUSED_PARAMETER(violationGlobalAfterDeviceSpecificCountError);
    UNUSED_PARAMETER(violationRxTxConfigurationMixCountError);
    UNUSED_PARAMETER(violationZeroLevelCountError);
    UNUSED_PARAMETER(violationUnprotectedDbAccessCountError);
    UNUSED_PARAMETER(violationSystemRecoveryCountError);
    UNUSED_PARAMETER(violationMangersMixCountError);
    UNUSED_PARAMETER(violationMangersRxTxConfigurationMixCountError);
    return GT_NOT_SUPPORTED;
}

#endif /*else - defined  CPSS_USE_MUTEX_PROFILER   */
