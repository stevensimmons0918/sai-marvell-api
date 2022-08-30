/*******************************************************************************
*              (c), Copyright 2013, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssGlobalSharedDb.c
*
* @brief This file provides handling of global variables DB.
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/*add here module initializers*/
#include <cpss/common/private/globalShared/init/prvCpssGlobalMainPpDrvInitVars.h>
#include <cpss/common/private/globalShared/init/prvCpssGlobalCommonInitVars.h>

#define CPSS_SHM_FILENAME   "CPSS_SHM_DATA"
#define SHARED_DB_PREFIX     "/dev/shm/"




#define SHARED_DB_MAGIC     0xCAFEFACE
#define NON_SHARED_DB_MAGIC 0xFACECAFE
#define SHARED_MEMORY_SEM_NAME   "CPSS_SHM_INIT_SEM"



PRV_CPSS_SHARED_GLOBAL_DB       *cpssSharedGlobalVarsPtr = NULL;
PRV_CPSS_SHARED_GLOBAL_DB        cpssSharedGlobalVars;

PRV_CPSS_NON_SHARED_GLOBAL_DB   *cpssNonSharedGlobalVarsPtr = NULL;
PRV_CPSS_NON_SHARED_GLOBAL_DB    cpssNonSharedGlobalVars ;

static GT_STATUS cpssGlobalSharedDbUnLock
(
    GT_VOID
);

static GT_STATUS cpssGlobalNonSharedDbDataSectionInit
(
    GT_VOID
);

extern GT_VOID prvBindDuplicatedAddrGetFunc(IN GT_U8 devNum);

extern GT_STATUS prvCpssPortManagerApIsConfigured
(
    IN GT_U8                  devNum,
    IN GT_U32                 portNum,
    OUT GT_BOOL               *apConfiguredPtr
);

extern GT_STATUS prvCpssDxChPortDevInfo
(
    IN  GT_U8  devNum,
    OUT GT_U32 *devId,
    OUT GT_U32 *revNum
);

extern GT_VOID hwPpPhase1DeviceSpecificIntCallbacksInit
(
    IN      GT_U8                                   devNum
);

extern void prvCpssHawkDedicatedEventsNonSharedConvertInfoSet
(
    IN GT_U8   devNum
);

extern void prvCpssHarrierDedicatedEventsNonSharedConvertInfoSet
(
    IN GT_U8   devNum
);

extern GT_STATUS hwsAc3SerdesIfInit
(
    IN GT_U8 devNum
);

extern GT_VOID prvCpssDrvPpIntMaskSetFptrFalconInit
(
    IN GT_U8   devNum
);


/**
* @internal cpssGlobalSharedDbAddProcess function
* @endinternal
*
* @brief add process to pid list(debug)
*
*/
static GT_VOID cpssGlobalSharedDbAddProcess
(
    GT_VOID
)
{
    GT_U32 index = 0;


     /*if it is not in shared library mode ,then do nothing*/
    if((cpssSharedGlobalVarsPtr!=NULL)&&
        (GT_TRUE == cpssSharedGlobalVarsPtr->sharedMemoryUsed))
    {
        for(index=0;index<CPSS_MAX_PIDS_SIZE;index++)
        {
            if(cpssSharedGlobalVarsPtr->pids[index]==0)
            {
                cpssSharedGlobalVarsPtr->pids[index]= cpssOsTaskGetPid();
                if(cpssSharedGlobalVarsPtr->clientNum<CPSS_MAX_PIDS_SIZE-1)
                {
                    /*support 255 clients*/
                    cpssSharedGlobalVarsPtr->clientNum++;
                }
                break;
            }
        }
    }
}
/**
* @internal cpssGlobalSharedDbRemoveProcess function
* @endinternal
*
* @brief remove process from pid list(debug)
*
*/
static GT_VOID cpssGlobalSharedDbRemoveProcess
(
    GT_VOID
)
{
    GT_U32 index = 0;
    GT_U32 myPid = cpssOsTaskGetPid();

    if((cpssSharedGlobalVarsPtr!=NULL)&&
        (GT_TRUE == cpssSharedGlobalVarsPtr->sharedMemoryUsed))
    {
        for(index=0;index<CPSS_MAX_PIDS_SIZE;index++)
        {
            if(cpssSharedGlobalVarsPtr->pids[index]==myPid)
            {
                cpssSharedGlobalVarsPtr->pids[index]=0;
                cpssSharedGlobalVarsPtr->clientNum--;
                break;
            }
        }
    }
}

void  cpssGlobalSharedDbCleanUp(void)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL  unlinkDone;
    if(PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.generalVars.initialization))
    {
        /*in case something will go wrong during the init*/
        rc = cpssGlobalSharedDbUnLock();
        cpssOsPrintf( "*** cpssGlobalSharedDbUnLock() called rc = %d\n",rc);
    }

    /*Try to destroy if last*/
    rc = cpssGlobalDbDestroy(GT_TRUE,&unlinkDone);
    cpssOsPrintf( "*** cpssGlobalDbDestroy () called rc = %d ,unlinkDone %d\n",rc,unlinkDone);
}



/**
* @internal osGlobalSharedDbLock function
* @endinternal
*
* @brief  Lock global data base access.
*  Use named semaphore.
*
*/
GT_STATUS cpssGlobalSharedDbLock
(
    GT_VOID
)
{
    CPSS_OS_SIG_SEM dbInitSem; /* named semaphore  to protect data base initialization in shared lib mode  */
    GT_STATUS rc = GT_OK;

    /*no race condition here  in updating this field ,
                    since either all process use shared lib or not*/

    /*if it is not in shared library mode ,then do nothing*/
    if(GT_TRUE == cpssSharedGlobalVarsPtr->sharedMemoryUsed)
    {
        /* register shared memory content cleaner in case init will go wrong */
        if(GT_FALSE==cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.cleanUpRegistered)
        {
            if (cpssOsTaskAtProcExit(cpssGlobalSharedDbCleanUp))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Cannot register cpssGlobalSharedDbCleanUp\n");
            }

            cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.cleanUpRegistered = GT_TRUE;
            cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.cleanUpIndex=
                cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.atExitIndex++;
         }

        /* Open named semaphore to guard  code from other clients. */
         rc = cpssOsNamedSemOpen(SHARED_MEMORY_SEM_NAME, &dbInitSem);
         if(rc!=GT_OK)
         {
              return rc;
         }

         rc = cpssOsNamedSemWait(dbInitSem);
         if(rc!=GT_OK)
         {
              return rc;
         }

         PRV_NON_SHARED_GLOBAL_VAR_DIRECT_SET(nonVolatileDb.generalVars.initialization,GT_TRUE);
     }

     return rc;
}

/**
* @internal osGlobalSharedDbUnLock function
* @endinternal
*
* @brief  Unlock global data base access.
*  Use named semaphore.
*
*/
GT_STATUS cpssGlobalSharedDbUnLock
(
    GT_VOID
)
{
    CPSS_OS_SIG_SEM  dbInitSem; /* named semaphore  to protect data base initialization in shared lib mode  */
    GT_STATUS rc = GT_OK;

    /*if it is not in shared library mode ,then do nothing*/
    if(GT_TRUE == cpssSharedGlobalVarsPtr->sharedMemoryUsed)
    {

       /* Open named semaphore to guard   code from other clients. */
        rc = cpssOsNamedSemOpen(SHARED_MEMORY_SEM_NAME, &dbInitSem);
        if(rc!=GT_OK)
        {
             return rc;
        }

        rc = cpssOsNamedSemPost(dbInitSem);
        if(rc!=GT_OK)
        {
           return rc;
        }

        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_SET(nonVolatileDb.generalVars.initialization,GT_FALSE);

     }

    return rc;
}


/**
 * @internal cpssGlobalSharedDbMainPpDrvModDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables .
 *
 */
static GT_STATUS cpssGlobalSharedDbDataSectionInit
(
    GT_VOID
)
{
    CPSS_NULL_PTR_CHECK_MAC(cpssSharedGlobalVarsPtr);

    /*cpss shared*/
    cpssSharedGlobalVarsPtr->magic = SHARED_DB_MAGIC;

    /*add here modules initializers*/
    cpssGlobalSharedDbMainPpDrvModDataSectionInit();
    cpssGlobalSharedDbCommonModDataSectionInit();


    return GT_OK;
}

/**
 * @internal cpssGlobalSharedDbMainPpDrvModDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize process interface to shared lib:
 *   Create a shared memory file, if one does not yet exist
 *   Map shared memory to process address space
 *
 */

GT_STATUS cpssGlobalDbShmemInit
(
    GT_BOOL *initDataSegmentPtr
)
{
    GT_VOID_PTR shmemPtr;
    GT_STATUS rc;
    GT_U32    size;
    GT_CHAR_PTR shmemName;


    size = sizeof(PRV_CPSS_SHARED_GLOBAL_DB);
    shmemName = CPSS_SHM_FILENAME;
    cpssSharedGlobalVarsPtr = NULL;

    rc = cpssOsGlobalDbShmemInit(size,shmemName,initDataSegmentPtr,&shmemPtr);
    if(rc==GT_OK)
    {
       cpssSharedGlobalVarsPtr =(PRV_CPSS_SHARED_GLOBAL_DB  *)shmemPtr;
    }

    return rc;
}

/**
* @internal cpssGlobalDbInit function
* @endinternal
*
* @brief   Initialize CPSS global variables data base.
*          Global variables data base is allocated.
*          In case useSharedMem equal GT_TRUE then also shared memory is allocated for shared variables.
*          After memory is allocated ,the global variables are being initialized to init values.
* @param[in] aslrSupport - whether shared memory should be used for shared variables.
*                           in case equal GT_FALSE dynamic memory allocation is used for shared variables,
*                           otherwise shared memory is used s used for shared variables,
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssGlobalDbInit
(
    IN GT_BOOL aslrSupport,
    IN GT_BOOL forceInit
)
{

   GT_STATUS rc = GT_OK;
   GT_BOOL   initDataSegment = GT_FALSE;
   GT_U32    initializerPid;
   GT_BOOL   sharedMemory = GT_FALSE;


#if !defined(LINUX)
    aslrSupport = GT_FALSE;
#endif

#ifdef SHARED_MEMORY
     sharedMemory = GT_TRUE;
#endif


   if(NULL == cpssSharedGlobalVarsPtr)
   {
       PRV_CPSS_SHM_PRINT("CPSS SHLIB : %s\n",sharedMemory?"Y":"N");
       if(GT_TRUE==sharedMemory)
       {
        PRV_CPSS_SHM_PRINT("CPSS ASLR  : %s\n",aslrSupport?"Y":"N");
       }
       if(GT_FALSE == aslrSupport)
       {
           cpssSharedGlobalVarsPtr = &cpssSharedGlobalVars;
           initDataSegment = GT_TRUE;
           cpssSharedGlobalVarsPtr->sharedMemoryUsed = GT_FALSE;
       }
       else
       {
           rc = cpssGlobalDbShmemInit(&initDataSegment);
           if(rc!=GT_OK)
           {
               /* Error */
               CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
           }
           if(NULL == cpssSharedGlobalVarsPtr)
           {
             /*something went wrong*/
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
           }

          PRV_CPSS_SHM_PRINT("CPSS SHMEM allocated at %p\n",cpssSharedGlobalVarsPtr);
       }


        /*no race condition here  in updating this field ,
                    since either all process use shared lib or not*/
        cpssSharedGlobalVarsPtr->sharedMemoryUsed = aslrSupport;

       /*only one  process initialize data segment*/
       rc = cpssGlobalSharedDbLock();
       if(rc!=GT_OK)
       {
             return rc;
       }

       if(GT_TRUE==forceInit||((GT_TRUE == initDataSegment)&&
            (GT_FALSE == cpssSharedGlobalVarsPtr->dbInitialized)))
       {
           /*save initialized PID*/
           initializerPid = cpssSharedGlobalVarsPtr->initializerPid;
           cpssOsMemSet(cpssSharedGlobalVarsPtr,0,sizeof(PRV_CPSS_SHARED_GLOBAL_DB));
           rc = cpssGlobalSharedDbDataSectionInit();
           if(rc!=GT_OK)
           {
               /* Error */
               CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
           }
           cpssSharedGlobalVarsPtr->sharedMemoryUsed = aslrSupport;
           cpssSharedGlobalVarsPtr->dbInitialized = GT_TRUE;
           cpssSharedGlobalVarsPtr->initializerPid = initializerPid;

       }
       else
       {
          /*shared memory already existed,check magic*/
          if(cpssSharedGlobalVarsPtr->magic != SHARED_DB_MAGIC)
          {
            /*something went wrong*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
          }
       }

        cpssGlobalSharedDbAddProcess();

        if( cpssSharedGlobalVarsPtr->sharedMemoryUsed)
        {
            if(GT_FALSE==cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.removeProcRegistered)
            {
                rc = cpssOsTaskAtProcExit(cpssGlobalSharedDbRemoveProcess);
                if(rc!=GT_OK)
                {
                    return rc;
                }

                cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.removeProcRegistered = GT_TRUE;
                cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.removeProcIndex=
                cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.atExitIndex++;
            }
        }

        rc = cpssGlobalSharedDbUnLock();
        if(rc!=GT_OK)
        {
            return rc;
        }
   }

   if(NULL == cpssNonSharedGlobalVarsPtr)
   {
       cpssNonSharedGlobalVarsPtr = &cpssNonSharedGlobalVars;
       cpssGlobalNonSharedDbDataSectionInit();
   }

    return rc;
}


static GT_VOID cpssGlobalDbNonSharedDestroy
(
   GT_VOID
)
{
   GT_U32 i;

   for(i=0;i<PRV_CPSS_MAX_PP_DEVICES_CNS;i++)
   {
     if (PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[i]) == NULL)
     {
        FREE_PTR_MAC(PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[i]));
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[i])= NULL;
     }
   }
}

/**
* @internal cpssGlobalDbDestroy function
* @endinternal
*
* @brief   UnInitialize CPSS global variables data base.
*          Global variables data base is deallocated.
*          In case  shared memory is used then shared memory is unlinked here.
*
* @param[in] unlink   - whether shared memory should be unlinked.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssGlobalDbDestroy
(
    GT_BOOL unlink,
    GT_BOOL *unlinkDonePtr
)
{
   GT_CHAR buffer[128]={'\0'};
   GT_STATUS rc;

   cpssOsStrCat(buffer,SHARED_DB_PREFIX);
   cpssOsStrCat(buffer,CPSS_SHM_FILENAME);

   cpssGlobalDbNonSharedDestroy();

   if(unlinkDonePtr)
   {
    *unlinkDonePtr = GT_FALSE;
   }

   if(NULL != cpssSharedGlobalVarsPtr)
   {
        if(GT_FALSE == cpssSharedGlobalVarsPtr->sharedMemoryUsed)
        {
            cpssOsMemSet(cpssSharedGlobalVarsPtr,0,sizeof(PRV_CPSS_SHARED_GLOBAL_DB));
        }
        else
        {
            if(GT_TRUE == unlink)
            {
                if(cpssSharedGlobalVarsPtr->clientNum == 0)
                {
                    rc = cpssOsGlobalDbShmemUnlink(buffer);
                    if(rc!=GT_OK)
                    {
                         return rc;
                    }

                    /* Remove named semaphore for clean-up */
                    rc = cpssOsNamedSemUnlink(SHARED_MEMORY_SEM_NAME);
                    if(rc!=GT_OK)
                    {
                         return rc;
                    }
                    if(unlinkDonePtr)
                    {
                     *unlinkDonePtr = GT_TRUE;
                    }
                }
            }
        }


   }

   cpssSharedGlobalVarsPtr = NULL;
   cpssNonSharedGlobalVarsPtr = NULL;


   return GT_OK;

}


/**
* @internal prvCpssGlobalDbDump function
* @endinternal
*
* @brief   Dump shared global DB attributes
*
* @retval GT_OK                    - anycase
*/
GT_STATUS prvCpssGlobalDbDump
(
    GT_VOID
)
{
    GT_U32 i,myPid;
    myPid = cpssOsTaskGetPid();

    cpssOsPrintf("Shared globals DB:\n");
    cpssOsPrintf("==================\n");

    if(cpssSharedGlobalVarsPtr)
    {
        cpssOsPrintf("Addr        :%p\n",cpssSharedGlobalVarsPtr);
        cpssOsPrintf("Magic       :0x%x\n",cpssSharedGlobalVarsPtr->magic);
        cpssOsPrintf("Master  Pid :0x%x\n",cpssSharedGlobalVarsPtr->initializerPid);
        cpssOsPrintf("Allocated by me:%d\n",(myPid==cpssSharedGlobalVarsPtr->initializerPid)?1:0);
        cpssOsPrintf("clientNum   :%d\n",cpssSharedGlobalVarsPtr->clientNum);
        cpssOsPrintf("SHMEM used  :%d\n",cpssSharedGlobalVarsPtr->sharedMemoryUsed);

        if(cpssSharedGlobalVarsPtr->sharedMemoryUsed)
        {
            cpssOsPrintf("Used by processes\n");
            for(i=0;i<CPSS_MAX_PIDS_SIZE;i++)
            {
                if(cpssSharedGlobalVarsPtr->pids[i]!=0)
                {
                  cpssOsPrintf("%d pid %d [0x%x] \n",i,cpssSharedGlobalVarsPtr->pids[i],cpssSharedGlobalVarsPtr->pids[i]);
                }
            }
        }
    }
    else
    {
        cpssOsPrintf("Not initialized\n");
    }




    cpssOsPrintf("Non-shared globals DB:\n");
    cpssOsPrintf("======================\n");

    if(cpssNonSharedGlobalVarsPtr)
    {
       cpssOsPrintf("Addr            :%p\n",cpssNonSharedGlobalVarsPtr);
       cpssOsPrintf("Magic           :0x%x\n",cpssNonSharedGlobalVarsPtr->magic);
    }
    else
    {
       cpssOsPrintf("Not initialized\n");
    }

    return GT_OK;
}
/**
* @internal prvCpssGlobalDbExistGet function
* @endinternal
*
* @brief   Check if  gllobal variables data base was allocated.
*
* @retval GT_TRUE                    global DB exist
* @retval GT_FAIL                  - global DB does not exist
*/
GT_BOOL prvCpssGlobalDbExistGet
(
    GT_VOID
)
{
    return cpssSharedGlobalVarsPtr!=NULL;
}

#ifdef SHARED_MEMORY

GT_VOID prvPerFamilySecondaryClientInit(IN GT_U8 devNum)
{

    HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */

    cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));
    funcPtrsStc.coreClockGetPtr   = (MV_CORE_CLOCK_GET)cpssDxChHwCoreClockGet;
    funcPtrsStc.sysDeviceInfo   = prvCpssDxChPortDevInfo;

    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            DUPLICATED_ADDR_GETFUNC(devNum)  = NULL;
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {
                hwsBobKPortsParamsSupModesMapSet(devNum);
                hwsBobKIfInitHwsDevFunc(devNum,&funcPtrsStc);
            }
            DUPLICATED_ADDR_GETFUNC(devNum)  = NULL;
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            DUPLICATED_ADDR_GETFUNC(devNum)  = NULL;
            hwsBobcat3IfInitHwsDevFunc(devNum, &funcPtrsStc);
            hwsBobcat3PortsParamsSupModesMapSet(devNum);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            DUPLICATED_ADDR_GETFUNC(devNum)  = NULL;
            hwsAldrin2IfInitHwsDevFunc(devNum, &funcPtrsStc);
            hwsAldrin2PortsParamsSupModesMapSet(devNum);
            break;
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
            DUPLICATED_ADDR_GETFUNC(devNum)  = NULL;
            hwsAc3IfReIoad(devNum);
            hwsAlleycat3IfInitHwsDevFunc(devNum, &funcPtrsStc);
            hwsAc3SerdesIfInit(devNum);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            DUPLICATED_ADDR_GETFUNC(devNum)  = NULL;
            hwsAldrinIfInitHwsDevFunc(devNum, &funcPtrsStc);
            hwsAldrinPortsParamsSupModesMapSet(devNum);
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            prvCpssDrvPpIntMaskSetFptrFalconInit(devNum);

            hwsFalconIfInitHwsDevFunc(devNum, &funcPtrsStc);
            hwsFalconIfReIoad(devNum);
            hwsFalconPortsParamsSupModesMapSet(devNum);
            prvCpssFalconNonSharedHwInfoFuncPointersSet(devNum);
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:

            funcPtrsStc.serdesRefClockGet = (MV_SERDES_REF_CLOCK_GET)prvCpssDxChSerdesRefClockTranslateCpss2Hws;
            hwsHawkIfInitHwsDevFunc(devNum,&funcPtrsStc);
            hwsHawkIfReIoad(devNum);
            hwsHawkPortsParamsSupModesMapSet(devNum);
            prvCpssHawkNonSharedHwInfoFuncPointersSet(devNum);
            prvCpssHawkDedicatedEventsNonSharedConvertInfoSet(devNum);
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:

            funcPtrsStc.serdesRefClockGet = (MV_SERDES_REF_CLOCK_GET)prvCpssDxChSerdesRefClockTranslateCpss2Hws;
            hwsPhoenixIfInitHwsDevFunc(devNum,&funcPtrsStc);
            hwsPhoenixIfReIoad(devNum);
            hwsPhoenixPortsParamsSupModesMapSet(devNum);
            prvCpssPhoenixNonSharedHwInfoFuncPointersSet(devNum);
            break;
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:

            funcPtrsStc.serdesRefClockGet = (MV_SERDES_REF_CLOCK_GET)prvCpssDxChSerdesRefClockTranslateCpss2Hws;
            hwsHarrierIfInitHwsDevFunc(devNum,&funcPtrsStc);
            hwsHarrierIfReIoad(devNum);
            hwsHarrierPortsParamsSupModesMapSet(devNum);
            prvCpssHarrierNonSharedHwInfoFuncPointersSet(devNum);
            prvCpssHarrierDedicatedEventsNonSharedConvertInfoSet(devNum);
            break;
        case CPSS_PP_FAMILY_DXCH_IRONMAN_E:

            prvCpssIronmanNonSharedHwInfoFuncPointersSet(devNum);
            break;
        case CPSS_PP_FAMILY_DXCH_AC5_E:
            hwsAlleycat5PortsParamsSupModesMapSet(devNum);
            hwsAlleycat5IfInitHwsDevFunc(devNum,&funcPtrsStc);
            hwsAlleycat5IfReIoad(devNum);
            break;
        default:
            PRV_CPSS_SHM_PRINT("Device %d not handled at prvPerFamilySecondaryClientInit\n",devNum);
            DUPLICATED_ADDR_GETFUNC(devNum)  = NULL;
    }
}


GT_STATUS prvCpssDxChHwsRegDbInit
(
    GT_U8 devNum
)
{
   GT_STATUS rc = GT_OK;

   if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E ||
                    PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
   {
       rc =  hwsPhoenixRegDbInit(devNum);
   }
   else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
   {
       rc =  hwsHawkRegDbInit(devNum);
   }
   else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
   {
       rc =  hwsHarrierRegDbInit(devNum);
   }
   else
   {
        PRV_CPSS_SHM_PRINT("Device %d does not use HWS regDb\n",devNum);
   }

   return rc;
}


GT_STATUS cpssGlobalNonSharedDbInit
(
    GT_VOID
)
{
    GT_U32 i;
    GT_STATUS rc;
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT portTypeOptions;/* options available for port */
    CPSS_PP_FAMILY_TYPE_ENT        devFamily;      /* device family */
    PRV_CPSS_PORT_PM_FUNC_PTRS     cpssPmFunc;
    GT_BOOL                        processInitialized = GT_FALSE;


    for(i=0;i<PRV_CPSS_MAX_PP_DEVICES_CNS;i++)
    {
        if(PRV_CPSS_IS_DEV_EXISTS_MAC(i))
        {

            devFamily = PRV_CPSS_PP_MAC(i)->devFamily - CPSS_PP_FAMILY_START_DXCH_E - 1;
            PRV_CPSS_SHM_PRINT("Found device %d [family %d] .Initializing function pointers\n",i,devFamily);

            if(cpssNonSharedGlobalVarsPtr)
            {
                if(GT_FALSE ==processInitialized)
                {
                  /*copy from prvCpssDxChPortIfCfgInit*/
                 cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsMemCopyFuncGlobalPtr =
                     (MV_OS_MEM_COPY_FUNC)cpssOsMemCpy;
                 cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsMemSetFuncGlobalPtr =
                     cpssOsMemSet;
                 cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsTimerWkFuncGlobalPtr =
                     cpssOsTimerWkAfter;
                 cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsFreeFuncGlobalPtr =
                     cpssOsFree;
                 cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsMallocFuncGlobalPtr=
                     cpssOsMalloc;
                 cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsStrCatFuncGlobalPtr =
                     cpssOsStrCat;
                 cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsTimerGetFuncGlobalPtr =
                     cpssOsTimeRT;
                 cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsRegisterSetFuncGlobalPtr =
                     (MV_REG_ACCESS_SET)genRegisterSet;
                 cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsRegisterGetFuncGlobalPtr =
                     (MV_REG_ACCESS_GET)genRegisterGet;
                 cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsSerdesRegGetFuncGlobalPtr=
                        (MV_SERDES_REG_ACCESS_GET)prvCpssGenPortGroupSerdesReadRegBitMask;
                 cpssNonSharedGlobalVarsPtr->commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsSerdesRegSetFuncGlobalPtr =
                        ( MV_SERDES_REG_ACCESS_SET)prvCpssGenPortGroupSerdesWriteRegBitMask;

                  prvCpssDrvSysConfigPhase1BindCallbacks();
                  processInitialized = GT_TRUE;
                }


                if (PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[i]) == NULL)
                {
                    /* allocate the memory for the non-shared device DB */
                    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[i]) = cpssOsMalloc(sizeof(PRV_CPSS_NON_SHARED_DEVICE_SPECIFIC_DATA));
                    if (PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[i]) == NULL)
                    {
                        CPSS_LOG_INFORMATION_MAC("hwPpPhase1Part1(..) : PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]) == NULL : GT_OUT_OF_CPU_MEM");
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }
                }
                initTablesFormatDb(i);

                hwPpPhase1DeviceSpecificIntCallbacksInit(i);

                /*From prvCpssDxChPortIfCfgInit*/

                /* port ifMode function obj init*/
                rc = prvCpssDxChPortIfFunctionsObjInit(i,  &(PORT_OBJ_FUNC(i)), PRV_CPSS_PP_MAC(i)->devFamily);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"PortIfFunctionsObjInit failed=%d", rc);
                }


                for(portTypeOptions = PRV_CPSS_XG_PORT_XG_ONLY_E; portTypeOptions < PRV_CPSS_XG_PORT_OPTIONS_MAX_E; portTypeOptions++)
                {
                    PORT_OBJ_FUNC(i).setPortSpeed[portTypeOptions] =
                        PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portSpeedSrc.portSpeedSetFuncPtrArray)[devFamily][portTypeOptions];
                }

                PORT_OBJ_FUNC(i).getPortSpeed =
                    PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portSpeedSrc.portSpeedGetFuncPtrArray)[devFamily];


                /* attach the generic code with the Dx function */
                prvBindDuplicatedAddrGetFunc(i);

                prvPerFamilySecondaryClientInit(i);

                rc = prvCpssDxChHwsRegDbInit(i);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortPmFuncBind failed=%d", rc);
                }

                prvCpssDxChPortMngEngineCallbacksInit(i);
                rc = prvCpssDxChPortSerdesFunctionsObjInit(i);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortSerdesFunctionsObjInit  failed=%d", rc);
                }

                if (PRV_CPSS_SIP_6_CHECK_MAC(i))
                {
                    /*From prvCpssDxChPortMgrInit*/
                    cpssPmFunc.cpssPmIsFwFunc = prvCpssPortManagerApIsConfigured;
                    rc = prvCpssDxChPortPmFuncBind(i, &cpssPmFunc);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortPmFuncBind failed=%d", rc);
                    }

                    /*From prvCpssDxChUnitBaseTableInit*/

                    /* bind once : HWS with function that calc base addresses of units for : Falcon,Hawk,Phoenix */
                    hwsFalconUnitBaseAddrCalcBind(prvCpssSip6HwsUnitBaseAddrCalc);

                    rc = initTablesDbSip6(i);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "initTablesDbSip6  failed=%d", rc);
                    }
                }


            }
        }
    }

    return GT_OK;
}

#endif

GT_U32  prvCpssBmpIsZero
(
    GT_VOID * portsBmpPtr
)
{
    /* BMP of ports with no ports members */
    CPSS_PORTS_BMP_STC cpssAllZeroPortsBmp = {{0}};

    cpssOsMemSet(&cpssAllZeroPortsBmp,0,sizeof(CPSS_PORTS_BMP_STC));

    return ((0 ==cpssOsMemCmp(portsBmpPtr,&cpssAllZeroPortsBmp,sizeof(CPSS_PORTS_BMP_STC))) ? 1 : 0);
}


/**
 * @internal cpssGlobalSharedDbMainPpDrvModDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global non-shared variables .
 *
 */
GT_STATUS cpssGlobalNonSharedDbDataSectionInit
(
    GT_VOID
)
{

    PRV_CPSS_NON_SHARED_NON_VOLATILE_DB saveNonVolatileDb;

    GT_U32 size = sizeof(PRV_CPSS_NON_SHARED_NON_VOLATILE_DB);
    cpssOsMemCpy(&saveNonVolatileDb,&(cpssNonSharedGlobalVarsPtr->nonVolatileDb),size);

    /*cpss non-shared*/
#undef cpssOsMemSet
    saveNonVolatileDb.osFuncDb.cpssOsMemSet(cpssNonSharedGlobalVarsPtr,0,sizeof(PRV_CPSS_NON_SHARED_GLOBAL_DB));
    cpssNonSharedGlobalVarsPtr->magic = NON_SHARED_DB_MAGIC;
#undef cpssOsMemCpy
    size = sizeof(PRV_CPSS_NON_SHARED_NON_VOLATILE_DB);
    saveNonVolatileDb.osFuncDb.cpssOsMemCpy(&(cpssNonSharedGlobalVarsPtr->nonVolatileDb),&saveNonVolatileDb,size);

    /*sanity check in order to verify everything went well*/
    if(cpssNonSharedGlobalVars.nonVolatileDb.osFuncDb.cpssOsMemCpy==NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /*initialize function pointers here*/
    cpssGlobalNonSharedDbExtDrvFuncInit();
    cpssGlobalNonSharedDbOsFuncInit();

    /*add here modules initializers*/


    cpssGlobalNonSharedDbMainPpDrvModDataSectionInit();
    cpssGlobalNonSharedDbCommonModDataSectionInit();
#ifdef SHARED_MEMORY
    /*This code is relevant only for non-first clients in shared library mode.
            Due to ASLR the function pointers should be rebinded/*/
    cpssGlobalNonSharedDbInit();
#endif

    return GT_OK;
}


