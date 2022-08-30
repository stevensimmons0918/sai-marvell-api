/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file cpssOsSem.h
*
* @brief Operating System wrapper. Semaphore facility.
*
*
* @version   8
********************************************************************************
*/

#ifndef __cpssOsSemh
#define __cpssOsSemh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <cpss/extServices/os/gtOs/gtGenTypes.h>

/************* Defines ********************************************************/

/* waiting for semaphore to be released - with no time limit */
#define CPSS_OS_SEM_WAIT_FOREVER_CNS             0
/* try to take semaphore, return immediately even if semaphore still busy */
#define CPSS_OS_SEM_NO_WAIT_CNS                  0xFFFFFFFF

/************* Typedefs *******************************************************/

/* semaphore for protecting resource */
typedef GT_UINTPTR CPSS_OS_MUTEX;

/* semaphore for signaling between tasks */
typedef GT_UINTPTR CPSS_OS_SIG_SEM;

/**
* @enum CPSS_OS_SEMB_STATE_ENT
 *
 * @brief An enumeration for semaphore creation state
*/
typedef enum{

    /** @brief create the semaphore empty
     *  a user must wait for the semaphore to be
     *  signaled before it can take it
     */
    CPSS_OS_SEMB_EMPTY_E = 0,

    /** @brief create the semaphore full
     *  a user not need to wait for a signaled before it
     *  can lock the semaphore
     */
    CPSS_OS_SEMB_FULL_E

} CPSS_OS_SEMB_STATE_ENT;


/************* Defines ********************************************************/

/*******************************************************************************
* CPSS_OS_MUTEX_CREATE_FUNC
*
* DESCRIPTION:
*       Create and initialize a mutex (mutual exclusion object).
*       This object used for protecting resources in HW or in SW , that
*       accessed from more than one SW task.
*
* INPUTS:
*       name   - mutex Name
*
* OUTPUTS:
*       mtxidPtr - (pointer to)mutex Id
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_MUTEX_CREATE_FUNC
*       2. CPSS_OS_MUTEX_DELETE_FUNC
*       3. CPSS_OS_MUTEX_LOCK_FUNC
*       4. CPSS_OS_MUTEX_UNLOCK_FUNC
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_OS_MUTEX_CREATE_FUNC)
(
    IN  const char              *name,
    OUT CPSS_OS_MUTEX           *mtxidPtr
);

/*******************************************************************************
* CPSS_OS_MUTEX_DELETE_FUNC
*
* DESCRIPTION:
*       Delete mutex object
*
* INPUTS:
*       mtxid - mutex Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_MUTEX_CREATE_FUNC
*       2. CPSS_OS_MUTEX_DELETE_FUNC
*       3. CPSS_OS_MUTEX_LOCK_FUNC
*       4. CPSS_OS_MUTEX_UNLOCK_FUNC
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_OS_MUTEX_DELETE_FUNC)
(
    IN CPSS_OS_MUTEX mtxid
);

/*******************************************************************************
* CPSS_OS_MUTEX_LOCK_FUNC
*
* DESCRIPTION:
*       Lock resource using specified mutex
*
* INPUTS:
*       mtxid    - mutex Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_TIMEOUT - on time out
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_MUTEX_CREATE_FUNC
*       2. CPSS_OS_MUTEX_DELETE_FUNC
*       3. CPSS_OS_MUTEX_LOCK_FUNC
*       4. CPSS_OS_MUTEX_UNLOCK_FUNC
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_OS_MUTEX_LOCK_FUNC)
(
    IN CPSS_OS_MUTEX  mtxid
);

/*******************************************************************************
* CPSS_OS_MUTEX_UNLOCK_FUNC
*
* DESCRIPTION:
*       Unlock mutex
*
* INPUTS:
*       mtxid    - mutex Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_MUTEX_CREATE_FUNC
*       2. CPSS_OS_MUTEX_DELETE_FUNC
*       3. CPSS_OS_MUTEX_LOCK_FUNC
*       4. CPSS_OS_MUTEX_UNLOCK_FUNC
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_OS_MUTEX_UNLOCK_FUNC)
(
    IN CPSS_OS_MUTEX smid
);

/*******************************************************************************
* CPSS_OS_SIG_SEM_BIN_CREATE_FUNC
*
* DESCRIPTION:
*       Create and initialize a binary semaphore for signaling between tasks or
*       between Interrupt context and tasks.
*
* INPUTS:
*       name   - "signaling" semaphore Name
*       init   - init value of "signaling" semaphore (full or empty)
*
* OUTPUTS:
*       smidPtr - (pointer to)semaphore "signaling" Id
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_SIG_SEM_BIN_CREATE_FUNC
*       2. CPSS_OS_SIG_SEM_DELETE_FUNC
*       3. CPSS_OS_SIG_SEM_WAIT_FUNC
*       4. CPSS_OS_SIG_SEM_SIGNAL_FUNC
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_OS_SIG_SEM_BIN_CREATE_FUNC)
(
    IN  const char              *name,
    IN  CPSS_OS_SEMB_STATE_ENT  init,
    OUT CPSS_OS_SIG_SEM         *smidPtr
);

/*******************************************************************************
* osSemMCreate
*
* DESCRIPTION:
*       Create and initialize a mutext semaphore.
*
* INPUTS:
*       name   - semaphore Name
*
* OUTPUTS:
*       smid - semaphore Id
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       This is not API. Should be used as development tool only.
*       Will be removed.
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_OS_SIG_SEM_M_CREATE_FUNC)
(
    IN  const char      *name,
    OUT CPSS_OS_SIG_SEM *smid
);

/*******************************************************************************
* osSemCCreate
*
* DESCRIPTION:
*       Create counting semaphore.
*
* INPUTS:
*       name   - semaphore Name
*       init   - init value of semaphore
*
* OUTPUTS:
*       smid - semaphore Id
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_OS_SIG_SEM_C_CREATE_FUNC)
(
    IN  const char              *name,
    IN  GT_U32                  init,
    OUT CPSS_OS_SIG_SEM         *smid
);

/*******************************************************************************
* CPSS_OS_SIG_SEM_DELETE_FUNC
*
* DESCRIPTION:
*       Delete "signaling" semaphore.
*
* INPUTS:
*       smid - semaphore Id (as created by
*                 CPSS_OS_SIG_SEM_BIN_CREATE_FUNC)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_SIG_SEM_BIN_CREATE_FUNC
*       2. CPSS_OS_SIG_SEM_DELETE_FUNC
*       3. CPSS_OS_SIG_SEM_WAIT_FUNC
*       4. CPSS_OS_SIG_SEM_SIGNAL_FUNC
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_OS_SIG_SEM_DELETE_FUNC)
(
    IN CPSS_OS_SIG_SEM smid
);

/*******************************************************************************
* CPSS_OS_SIG_SEM_WAIT_FUNC
*
* DESCRIPTION:
*       Wait on "signaling" semaphore.
*
* INPUTS:
*       smid    - "signaling" semaphore Id (as created by
*                 CPSS_OS_SIG_SEM_BIN_CREATE_FUNC)
*       timeOut - time out in milliseconds or 0 to wait forever
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_TIMEOUT - on time out
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_SIG_SEM_BIN_CREATE_FUNC
*       2. CPSS_OS_SIG_SEM_DELETE_FUNC
*       3. CPSS_OS_SIG_SEM_WAIT_FUNC
*       4. CPSS_OS_SIG_SEM_SIGNAL_FUNC
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_OS_SIG_SEM_WAIT_FUNC)
(
    IN CPSS_OS_SIG_SEM  smid,
    IN GT_U32       timeOut
);

/*******************************************************************************
* CPSS_OS_SIG_SEM_SIGNAL_FUNC
*
* DESCRIPTION:
*       Signal a "signaling" semaphore.
*
* INPUTS:
*       smid    - "signaling" semaphore Id (as created by
*                 CPSS_OS_SIG_SEM_BIN_CREATE_FUNC)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       see also :
*       1. CPSS_OS_SIG_SEM_BIN_CREATE_FUNC
*       2. CPSS_OS_SIG_SEM_DELETE_FUNC
*       3. CPSS_OS_SIG_SEM_WAIT_FUNC
*       4. CPSS_OS_SIG_SEM_SIGNAL_FUNC
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_OS_SIG_SEM_SIGNAL_FUNC)
(
    IN CPSS_OS_SIG_SEM smid
);


#if defined  CPSS_USE_MUTEX_PROFILER
/*******************************************************************************
* CPSS_OS_MUTEX_SET_GL_ATTRIBUTES_FUNC
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

typedef GT_STATUS (*CPSS_OS_MUTEX_SET_GL_ATTRIBUTES_FUNC)
(
    IN CPSS_OS_MUTEX                        mtxId,
    IN GT_BOOL                              isZeroLevel,
    IN GT_BOOL                              isDeviceOriented,
    IN GT_BOOL                              isRxTxFunctionality,
    IN GT_U8                                devNum,
    IN GT_BOOL                              isSystemRecovery,
    IN GT_BOOL                              isManager,
    IN GT_U32                               manager
);

/*******************************************************************************
* CPSS_OS_CHECK_UNPROTECTED_PER_DEVICE_DB_ACCESS
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

typedef GT_VOID (*CPSS_OS_CHECK_UNPROTECTED_PER_DEVICE_DB_ACCESS)
 (
    IN GT_U32                                 devNum,
    IN const char *                           functionName
 );
#endif
/*******************************************************************************
*@internal   CPSS_OS_NAMED_SEM_OPEN_FUNC
* @endinternal
*
* DESCRIPTION:
*  Creates a new named  semaphore or opens an existing
*      semaphore.  The semaphore is identified by name.
*
*
* INPUTS:
*       name              - semaphore name
*       smidPtr    - (pointer to)semaphore id
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
typedef GT_STATUS (*CPSS_OS_NAMED_SEM_OPEN_FUNC )
(
    IN const GT_CHAR_PTR name,
    IN CPSS_OS_SIG_SEM *smidPtr
);

/*******************************************************************************
* CPSS_OS_NAMED_SEM_WAIT_FUNC
*
* DESCRIPTION:
*      Locks the semaphore pointed to by sem.
*
* INPUTS:
*       smid              - semaphore id
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

typedef GT_STATUS (*CPSS_OS_NAMED_SEM_WAIT_FUNC )
(
    IN CPSS_OS_SIG_SEM smid
);
/*******************************************************************************
* CPSS_OS_NAMED_SEM_POST_FUNC
*
* DESCRIPTION:
*      Unlocks the semaphore pointed to by sem.
*
* INPUTS:
*       smid              - semaphore id
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

typedef GT_STATUS (*CPSS_OS_NAMED_SEM_POST_FUNC )
(
    IN CPSS_OS_SIG_SEM smid
);
/*******************************************************************************
* CPSS_OS_NAMED_SEM_CLOSE_FUNC
*
* DESCRIPTION:
*       Closes the named semaphore referred to by sem,
*       allowing any resources that the system has allocated to the
 *      calling process for this semaphore to be freed.
*
* INPUTS:
*       smid              - semaphore id
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
typedef GT_STATUS (*CPSS_OS_NAMED_SEM_CLOSE_FUNC )
(
    IN CPSS_OS_SIG_SEM  smid
);

/*******************************************************************************
* CPSS_OS_NAMED_SEM_UNLINK_FUNC
*
* DESCRIPTION:
*       Removes the named semaphore referred to by name.
*     The semaphore name is removed immediately.  The semaphore is
       destroyed once all other processes that have the semaphore open
       close it.
*
* INPUTS:
*
*        name              - semaphore name
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
typedef GT_STATUS (*CPSS_OS_NAMED_SEM_UNLINK_FUNC )
(
    IN GT_CHAR_PTR name
);


/* CPSS_OS_SEM_BIND_STC -
    structure that hold the "os semaphore" functions needed be bound to cpss.

*/
typedef struct{
    CPSS_OS_MUTEX_CREATE_FUNC           osMutexCreateFunc;
    CPSS_OS_MUTEX_DELETE_FUNC           osMutexDeleteFunc;
    CPSS_OS_MUTEX_LOCK_FUNC             osMutexLockFunc;
    CPSS_OS_MUTEX_UNLOCK_FUNC           osMutexUnlockFunc;
    CPSS_OS_SIG_SEM_BIN_CREATE_FUNC     osSigSemBinCreateFunc;
    CPSS_OS_SIG_SEM_M_CREATE_FUNC       osSigSemMCreateFunc;
    CPSS_OS_SIG_SEM_C_CREATE_FUNC       osSigSemCCreateFunc;
    CPSS_OS_SIG_SEM_DELETE_FUNC         osSigSemDeleteFunc;
    CPSS_OS_SIG_SEM_WAIT_FUNC           osSigSemWaitFunc;
    CPSS_OS_SIG_SEM_SIGNAL_FUNC         osSigSemSignalFunc;
#if defined  CPSS_USE_MUTEX_PROFILER
    CPSS_OS_MUTEX_SET_GL_ATTRIBUTES_FUNC osMutexSetGlAttributesFunc;
    CPSS_OS_CHECK_UNPROTECTED_PER_DEVICE_DB_ACCESS osCheckUnprotectedPerDeviceDbAccess;
#endif
    CPSS_OS_NAMED_SEM_OPEN_FUNC         osNamedSemOpen;
    CPSS_OS_NAMED_SEM_WAIT_FUNC         osNamedSemWait;
    CPSS_OS_NAMED_SEM_POST_FUNC         osNamedSemPost;
    CPSS_OS_NAMED_SEM_CLOSE_FUNC        osNamedSemClose;
    CPSS_OS_NAMED_SEM_UNLINK_FUNC       osNamedSemUnlink;

}CPSS_OS_SEM_BIND_STC;

#ifdef __cplusplus
}
#endif

#endif  /* __cpssOsSemh */



