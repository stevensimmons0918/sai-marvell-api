/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/


/************* Includes *******************************************************/
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/init/gtOsGlobalDbEnablerInitVars.h>
#include <gtUtil/gtBmPool.h>

/*************** Globals ******************************************************/

static GT_BOOL gtOsWrapperOpen = GT_FALSE;


PRV_OS_SHARED_GLOBAL_DB       *osSharedGlobalVarsPtr = NULL;
PRV_OS_SHARED_GLOBAL_DB       osSharedGlobalVars ;

PRV_OS_NON_SHARED_GLOBAL_DB   *osNonSharedGlobalVarsPtr = NULL;
PRV_OS_NON_SHARED_GLOBAL_DB    osNonSharedGlobalVars;


/**
 * @internal osNonSharedDbDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global non-shared variables .
 *
 */
static GT_STATUS osNonSharedDbDataSectionInit
(
    GT_VOID
)
{
    PRV_OS_NON_SHARED_NON_VOLATILE_DB nonVolatileDb;
    GT_U32 size = sizeof(PRV_OS_NON_SHARED_GLOBAL_DB);
    /*PRV_OS_NON_SHARED_NON_VOLATILE_DB should not be cleared*/
    osMemCpy(&nonVolatileDb,&osNonSharedGlobalVarsPtr->osNonVolatileDb,sizeof(PRV_OS_NON_SHARED_NON_VOLATILE_DB));
    /*helper non-shared*/
    osMemSet(osNonSharedGlobalVarsPtr,0,size);
    osNonSharedGlobalVarsPtr->magic = NON_SHARED_DB_MAGIC;
    osMemCpy(&osNonSharedGlobalVarsPtr->osNonVolatileDb,&nonVolatileDb,sizeof(PRV_OS_NON_SHARED_NON_VOLATILE_DB));

    /*add here modules initializers*/
    osHelperGlobalNonSharedDbEnablerModDataSectionInit();

    return GT_OK;
}

/**
 * @internal osSharedDbDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables .
 *
 */
static GT_STATUS osSharedDbDataSectionInit
(
    const char * callerFuncName,
    GT_U32       callerLine
)

{
    if(callerFuncName!=NULL)
    {
       osPrintf("Data section init called from  %s line: %d\n",callerFuncName,callerLine);
    }

    osMemSet(osSharedGlobalVarsPtr,0,sizeof(PRV_OS_SHARED_GLOBAL_DB));
    /*helper shared*/
    osSharedGlobalVarsPtr->magic = SHARED_DB_MAGIC;
    osSharedGlobalVarsPtr->clientNum = 1;
    /*add here modules initializers*/
    osHelperGlobalSharedDbEnablerModDataSectionInit();

    return GT_OK;
}



/************* Functions ******************************************************/



/**
* @internal osWrapperOpen function
* @endinternal
*
* @brief   Initialize OS wrapper facility.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osWrapperOpen(void * osSpec)
{
    GT_UNUSED_PARAM(osSpec);
    if (gtOsWrapperOpen == GT_TRUE)
    {
        return GT_OK;
    }
    else
    {
        /* Must be set to GT_TRUE before use any of OS functions. */
        gtOsWrapperOpen = GT_TRUE;

        osSharedGlobalVarsPtr = &osSharedGlobalVars;
        osSharedDbDataSectionInit(__FUNCTION__,__LINE__);

        osNonSharedGlobalVarsPtr = &osNonSharedGlobalVars;
        osNonSharedDbDataSectionInit();

        gtPoolInit(500 ); /* Number of pools to support */
    }

    return GT_OK;
}

/**
* @internal osWrapperClose function
* @endinternal
*
* @brief   Close OS wrapper facility and free all used resources.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osWrapperClose(void)
{
    return GT_OK;
}

/*******************************************************************************
* osWrapperIsOpen
*
* DESCRIPTION:
*       Returns GT_TRUE if OS wrapper facility was initialized .
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_TRUE          - on success
*       GT_FALSE         - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_INLINE GT_BOOL osWrapperIsOpen(void)
{
    return gtOsWrapperOpen;
}



