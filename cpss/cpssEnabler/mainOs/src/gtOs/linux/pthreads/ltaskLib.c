/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file ltaskLib.c
*
* @brief Pthread implementation of mainOs tasks
*
* @version   15
********************************************************************************
*/
#define _GNU_SOURCE
#include <errno.h>
#include <unistd.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <limits.h>

#include <gtOs/gtOsTask.h>
#include <gtOs/osObjIdLib.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/globalDb/gtOsEnablerModGlobalSharedDb.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>
#if __WORDSIZE == 64
#  ifdef ALL_ALLOCS_32BIT
#    include <sys/mman.h>
#  endif
#endif


/* for cascadig required */



#include <gtOs/gtOsSharedData.h>

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

#include "mainOsPthread.h"

/************* Defines ********************************************************/
#undef DIAG_PRINTFS

/* hash function for index calculation, 12 LSBs are constant in Linux */
#define V2L_PTHREAD_HASH_MAC(_pthreadId) (((_pthreadId) >> 12) & 0x1FF)

/************* Internal data **************************************************/

#define PRV_SHARED_DB osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.gtOsLtaskLibSrc

#define TASK(id) ((_V2L_taskSTC*)(PRV_SHARED_DB.tasks.list[id]))

/************ Forward declararions ********************************************/
#if defined(ASIC_SIMULATION) && __GLIBC_PREREQ(2,13)
    /* we don't need task_log(), gdb shows info */
#else
static GT_VOID task_log(const GT_CHAR *fmt, ...);
#endif
static GT_VOID V2L_taskUnlock_i(
    IN  pthread_t   owner,
    IN  int         force
);


/************ Public Functions ************************************************/

/**
* @internal V2L_ltaskInit function
* @endinternal
*
* @brief   Initialize tasks
*
* @param[in] name                     - root task name
*
* @retval OK                       - on success
* @retval ERROR                    - on error
*
* @note This function called from osStartEngine()
*
*/
int V2L_ltaskInit
(
    IN  const GT_CHAR *name
)
{
    OS_OBJECT_HEADER_STC *h;
    _V2L_taskSTC *tsk;

#ifdef SHARED_MEMORY
    if (!PRV_SHARED_DB.initialized)
    {
        CREATE_MTX(&PRV_SHARED_DB.tasks_mtx);
        CREATE_MTX(&PRV_SHARED_DB.taskLock_mtx);
        CREATE_COND(&PRV_SHARED_DB.taskLock_cond);
        PRV_SHARED_DB.initialized = 1;
    }
#endif
    pthread_mutex_lock(&PRV_SHARED_DB.tasks_mtx);

#ifdef SHARED_MEMORY
    osObjLibGetNewObject(&PRV_SHARED_DB.tasks, 1, name?name:"tUsrRoot", &h,V2L_ts_malloc,V2L_ts_free);
#else
    osObjLibGetNewObject(&PRV_SHARED_DB.tasks, 1, name?name:"tUsrRoot", &h);
#endif
    tsk = (_V2L_taskSTC*)h;

    tsk->pthrid = pthread_self();

    pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);
    return 0;
}

/*****************************************************************************
**  cleanup_task_list ensures that a killed pthread cleans entry in task list
*****************************************************************************/
static void cleanup_task_list(void* arg)
{
    _V2L_taskSTC *tsk = (_V2L_taskSTC*)arg;
    GT_U32 hashIdx = V2L_PTHREAD_HASH_MAC(tsk->pthrid);

    if ((PRV_SHARED_DB.pthreadHashTbl[hashIdx].tid > 0) &&
        (PRV_SHARED_DB.pthreadHashTbl[hashIdx].pthrid == tsk->pthrid))
    {
        /* clean hash table */
        PRV_SHARED_DB.pthreadHashTbl[hashIdx].pthrid = 0;
        PRV_SHARED_DB.pthreadHashTbl[hashIdx].tid = 0;
    }

    tsk->header.type = 0;
}

/*****************************************************************************
**  os_task_wrapper
*****************************************************************************/
static void* os_task_wrapper(void *arg)
{
    _V2L_taskSTC *tsk = (_V2L_taskSTC*)arg;

    pthread_cleanup_push(cleanup_task_list, (void*)tsk );

    tsk->entry_point(tsk->param);

    pthread_cleanup_pop( 1 );

    return NULL;
}


/**
* @internal osTaskCreate function
* @endinternal
*
* @brief   Create OS Task and start it.
*
* @param[in] name                     - task name, string no longer then OS_MAX_TASK_NAME_LENGTH
* @param[in] prio                     - task priority 255 - 0 => HIGH
* @param[in] stack                    - task Stack Size
*                                      start_addr - task Function to execute
*                                      arglist    - pointer to list of parameters for task function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTaskCreate
(
    IN  const GT_CHAR  *name,
    IN  GT_U32      prio,
    IN  GT_U32      stack,
    IN  unsigned    (__TASKCONV *start_addr)(GT_VOID*),
    IN  GT_VOID     *arglist,
    OUT GT_TASK     *tid
)
{
    int t;
    OS_OBJECT_HEADER_STC *h;
    _V2L_taskSTC *tsk;
    pthread_attr_t  attrs;
    GT_U32      stack_min;
    GT_U32      hashIdx;

    pthread_attr_init(&attrs);

#if !defined(ASIC_SIMULATION) && !defined(RTOS_ON_SIM)
    stack_min = PTHREAD_STACK_MIN;
#else /* simulation */
#ifdef GM_USED
    stack_min = 0x200000; /* 2M */
#else /* !GM_USED */
    stack_min = 0x20000; /* 128K */
#endif /* !GM_USED */

#endif

    if (stack)
    {
#ifdef INCLUDE_UTF
        /* The UTF allocates almost 1K in thread local storage
         * So stack size must be increased by this value
         */
        stack += 0x4000; /* 16K */
#endif

        if (stack < stack_min)
            stack = stack_min;
        pthread_attr_setstacksize(&attrs, stack);
    }

#if (__WORDSIZE == 64) && !defined(MIPS64_CPU) && !defined(INTEL64_CPU)
#  ifdef ALL_ALLOCS_32BIT
    /* workaround: allocate stack in first 2Gig address space */
    {
        size_t  stksize;
        GT_VOID *stkaddr;
        pthread_attr_getstacksize( &attrs, &stksize);
        stkaddr = mmap(NULL, stksize,
            PROT_READ | PROT_WRITE,
            MAP_32BIT | MAP_GROWSDOWN | MAP_STACK | MAP_ANONYMOUS | MAP_PRIVATE,
            0, 0);
        pthread_attr_setstack( &attrs, stkaddr, stksize);
    }
#endif
#endif

    /*!! set priority */

    pthread_mutex_lock(&PRV_SHARED_DB.tasks_mtx);
#ifdef SHARED_MEMORY
    t = osObjLibGetNewObject(&PRV_SHARED_DB.tasks, 1, name, &h,V2L_ts_malloc,V2L_ts_free);
#else
    t = osObjLibGetNewObject(&PRV_SHARED_DB.tasks, 1, name, &h);
#endif
    tsk = (_V2L_taskSTC*)h;

    if (!name)
    {
        sprintf(tsk->header.name, "t%d", t);
    }

    tsk->vxw_priority = prio;
    tsk->entry_point = start_addr;
    tsk->param = arglist;

    pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);

    if (tid != NULL)
        *tid = (GT_TASK)t;
    if ( pthread_create( &(tsk->pthrid), &attrs,
                         os_task_wrapper, (void*)tsk) != 0 )
    {
        tsk->header.type = 0;
#ifdef DIAG_PRINTFS
        perror( "\r\ntaskSpawn(): pthread_create returned error:" );
#endif
        return GT_FAIL;
    } else {
#if __GLIBC_PREREQ(2,13)
        if (name != NULL)
        {
            pthread_setname_np(tsk->pthrid, name);
        }
#endif
        pthread_detach(tsk->pthrid);
#if defined(ASIC_SIMULATION) && __GLIBC_PREREQ(2,13)
        /* we don't need task_log(), gdb shows info */
#else
        task_log("task(%s)\ttid(%d)\tthread(%d)\n", name, t, tsk->pthrid);
#endif
    }

    pthread_mutex_lock(&PRV_SHARED_DB.tasks_mtx);

    hashIdx = V2L_PTHREAD_HASH_MAC(tsk->pthrid);
    if (PRV_SHARED_DB.pthreadHashTbl[hashIdx].tid < 1)
    {
        /* add pthread id in hash table */
        PRV_SHARED_DB.pthreadHashTbl[hashIdx].tid = t;
        PRV_SHARED_DB.pthreadHashTbl[hashIdx].pthrid = tsk->pthrid;
    }

    /* Store thread name in history table */
    if (PRV_SHARED_DB.internal_tid < V2L_PTHREAD_HASH_TABLE_SIZE_CNS)
    {
        PRV_SHARED_DB.pthreadHashTblHistory[PRV_SHARED_DB.internal_tid] = (char *)osMalloc(NAME_MAX);
        /* check if osMalloc is not binded when creating task before cpssInitSystem */
        if (PRV_SHARED_DB.pthreadHashTblHistory[PRV_SHARED_DB.internal_tid])
        {
            if (name)
                osSprintf(PRV_SHARED_DB.pthreadHashTblHistory[PRV_SHARED_DB.internal_tid], "%s", name);
            else
                osSprintf(PRV_SHARED_DB.pthreadHashTblHistory[PRV_SHARED_DB.internal_tid],
                          "noname%d", PRV_SHARED_DB.internal_tid);
        }
        PRV_SHARED_DB.internal_tid++;
    }

    pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);

    return GT_OK;
}

/**
* @internal osTasksInfo function
* @endinternal
*
* @brief   Returns detailed list of the currently running tasks
*
* @param[in] buff                     - Buffer to store the dump
* @param[in] size                     - Size of the buffer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTasksInfo
(
    IN GT_CHAR *buff,
    IN GT_U32   size
)
{
    int t;
    GT_U32 curr_size = 0;
    static const int line_size = NAME_MAX + 4 + 4 + 3; /* name + itid + tid + crlf */

    if (size < 1) {
        return GT_FAIL;
    }

    buff[0] = 0;
    curr_size += osSprintf(buff, "%s%-4d%-16s\n", buff, 1, "main");

    /* Print history of tasks (i.e. also tasks that are deleted) */
    for (t = 0; t < PRV_SHARED_DB.internal_tid; t++) {

        if (curr_size + line_size > size) {
            return GT_FAIL;
        }

        /* main thread is 1 */
        curr_size += osSprintf(buff, "%s%-4d%-16s\n", buff, t + 2,
                               PRV_SHARED_DB.pthreadHashTblHistory[t]);
    }

    return GT_OK;
}

/* gain - add thread to task list - start */
static int V2L_taskIdSelf_add_new_threads(pthread_t pthrid, int task_allocated)
{
    int t;
    char name[32];
    static int ext_id=0;

    ext_id++;
    sprintf(name, "ext%d", ext_id);
    V2L_ltaskInit(name);

    for (t = task_allocated; t < PRV_SHARED_DB.tasks.allocated; t++)
        if (TASK(t) && TASK(t)->header.type && TASK(t)->pthrid == pthrid)
        {
            GT_U32 hashIdx = V2L_PTHREAD_HASH_MAC(pthrid);
            pthread_mutex_lock(&PRV_SHARED_DB.tasks_mtx);
            if (PRV_SHARED_DB.pthreadHashTbl[hashIdx].tid < 1)
            {
                /* add pthread id in hash table */
                PRV_SHARED_DB.pthreadHashTbl[hashIdx].tid = t;
                PRV_SHARED_DB.pthreadHashTbl[hashIdx].pthrid = pthrid;
            }
            pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);

            return t;
        }

    return 0;
}
/* gain - add thread to task list - End */

/**
* @internal V2L_taskIdSelf function
* @endinternal
*
* @brief   returns the identifier of the calling task
*/
static int V2L_taskIdSelf(GT_VOID)
{
    int t;
    pthread_t pthrid;
    GT_U32 hashIdx;

    pthrid = pthread_self();

    hashIdx = V2L_PTHREAD_HASH_MAC(pthrid);
    if (PRV_SHARED_DB.pthreadHashTbl[hashIdx].pthrid == pthrid)
    {
        /* use hash table */
        return PRV_SHARED_DB.pthreadHashTbl[hashIdx].tid;
    }

    for (t = 1; t < PRV_SHARED_DB.tasks.allocated; t++)
        if (TASK(t) && TASK(t)->header.type && TASK(t)->pthrid == pthrid)
            return t;
    /* gain - add thread to task list - start */
    t = V2L_taskIdSelf_add_new_threads(pthrid, PRV_SHARED_DB.tasks.allocated);
    /* gain - add thread to task list - End */

    return t;
}


/**
* @internal osTaskGetId function
* @endinternal
*
* @brief   Returns current task id.
*
* @param[in] pthrid - pointer to pthread id
*
* @retval id - Task ID on success
* @retval -1 - negative value on failure
*
*/
GT_32 osTaskGetId(IN GT_VOID *pthrid)
{
    _V2L_taskSTC *tsk;
    int id;

    for (id=1; id<PRV_SHARED_DB.tasks.allocated ; id++)
    {
        if(PRV_SHARED_DB.tasks.list[id]->type == 1)
        {
            tsk = (_V2L_taskSTC*)PRV_SHARED_DB.tasks.list[id];
            if(tsk->pthrid == *(pthread_t *)pthrid) {
                return id;
            }
        }
    }
    return -1;
}


/*******************************************************************************
* CHECK_TID
*
* DESCRIPTION:
*       Check task Id and return pointer to task struct
*
* INPUTS:
*       tid         - task Id. Zero means calling task
*
* OUTPUTS:
*       tsk         - pointer to task struct
*
* RETURNS:
*       ERROR if error
*
*******************************************************************************/
#define CHECK_TID(tid) \
    _V2L_taskSTC *tsk; \
    if ((int)tid == 0) \
        tid = (GT_TASK)V2L_taskIdSelf(); \
    if (tid == 0) \
        return GT_FAIL; \
    if ((int)tid > PRV_SHARED_DB.tasks.allocated) \
        return GT_FAIL; \
    tsk = (_V2L_taskSTC*)(PRV_SHARED_DB.tasks.list[(int)tid]); \
    if (!tsk || !tsk->header.type) \
        return GT_FAIL; \


/**
* @internal osTaskDelete function
* @endinternal
*
* @brief   Deletes existing task.
*
* @param[in] tid                      - Task ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note If tid = 0, delete calling task (itself)
*
*/
#ifdef PTP_PLUGIN_SUPPORT
GT_STATUS CPSS_osTaskDelete
#else
GT_STATUS osTaskDelete
#endif
(
    IN GT_TASK tid
)
{
    GT_U32      hashIdx;
    pthread_t   pthrid;
    CHECK_TID(tid);
    pthread_mutex_lock(&PRV_SHARED_DB.tasks_mtx);

    pthrid = pthread_self();

    hashIdx = V2L_PTHREAD_HASH_MAC(pthrid);
    if ((PRV_SHARED_DB.pthreadHashTbl[hashIdx].tid > 0) &&
        (PRV_SHARED_DB.pthreadHashTbl[hashIdx].pthrid == pthrid))
    {
        /* invalidate entry, first write pthread id */
        PRV_SHARED_DB.pthreadHashTbl[hashIdx].pthrid = 0;
        PRV_SHARED_DB.pthreadHashTbl[hashIdx].tid = 0;
    }

    tsk->header.type = 0;

    if (tsk->pthrid == pthrid)
    {
        pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);
        pthread_exit(NULL);
        return GT_FAIL;
    }

    pthread_cancel(tsk->pthrid);
    pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);

    return GT_OK;
}

/**
* @internal osTaskSuspend function
* @endinternal
*
* @brief   Suspends existing task/thread.
*
* @param[in] tid                      - Task ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note If tid = 0, suspend calling task (itself)
*
*/
GT_STATUS osTaskSuspend
(
    IN GT_TASK tid
)
{
    CHECK_TID(tid);
    /* force to unlock taskLock */
    V2L_taskUnlock_i(tsk->pthrid, 1);
    /* send SIGSTOP */
    pthread_kill(tsk->pthrid, SIGSTOP);
    return GT_OK;
}


/**
* @internal osTaskResume function
* @endinternal
*
* @brief   Resumes existing task/thread.
*
* @param[in] tid                      - Task ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTaskResume
(
    IN GT_TASK tid
)
{
    CHECK_TID(tid);
    pthread_kill(tsk->pthrid, SIGCONT);
    return GT_OK;
}


/**
* @internal osTaskGetSelf function
* @endinternal
*
* @brief   returns the current task (thread) id
*
* @param[out] tid                      -  the current task (thread) id
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - if parameter is invalid
*/
GT_STATUS osTaskGetSelf
(
    OUT GT_U32 *tid
)
{
    /* check validity of function arguments */
    if (tid == NULL)
        return GT_FAIL;
    *tid = (GT_U32)V2L_taskIdSelf();
    return GT_OK;
}


/**
* @internal osSetTaskPrior function
* @endinternal
*
* @brief   Changes priority of task/thread.
*
* @param[in] tid                      - Task ID
* @param[in] newprio                  - new priority of task
*
* @param[out] oldprio                  - old priority of task
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note If tid = 0, change priotity of calling task (itself)
*
*/
GT_STATUS osSetTaskPrior
(
    IN  GT_TASK tid,
    IN  GT_U32  newprio,
    OUT GT_U32  *oldprio
)
{
    CHECK_TID(tid);
    if ( oldprio != NULL )
        *oldprio = (GT_U32)(tsk->vxw_priority);

    tsk->vxw_priority = newprio;
    /*!! set priority */
    return GT_OK;
}

/**
* @internal osGetTaskPrior function
* @endinternal
*
* @brief   Gets priority of task/thread.
*
* @param[in] tid                      - Task ID
*
* @param[out] prio                     - priority of task
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note If tid = 0, gets priotity of calling task (itself)
*
*/
GT_STATUS osGetTaskPrior
(
    IN  GT_TASK tid,
    OUT GT_U32  *prio
)
{
    CHECK_TID(tid);
    if (prio == NULL)
        return GT_FAIL;

    *prio = (GT_U32)(tsk->vxw_priority);
    return GT_OK;
}


/**
* @internal CPSS_osTaskLock function
* @endinternal
*
* @brief   Disable task rescheduling of current task.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The following task lock implementation is not actually doing task lock
*       (i.e. prohibiting the preemption of this task), it is implementing a code
*       to protect critical section, based on Mutexes.
*       Also note that "pthread_cond_wait" enables atomic operation of signaling
*       one mutes and taking another one.
*
*/
GT_STATUS CPSS_osTaskLock(GT_VOID)
{
    pthread_t self = pthread_self();
    DO_LOCK_MUTEX(&PRV_SHARED_DB.taskLock_mtx);
    while (PRV_SHARED_DB.taskLock_count)
    {
        if (PRV_SHARED_DB.taskLock_owner == self)
            break;
        DO_COND_WAIT(&PRV_SHARED_DB.taskLock_cond, &PRV_SHARED_DB.taskLock_mtx);
    }
    PRV_SHARED_DB.taskLock_count++;
    PRV_SHARED_DB.taskLock_owner = self;
    pthread_mutex_unlock(&PRV_SHARED_DB.taskLock_mtx);
    return GT_OK;
}

/**
* @internal V2L_taskUnlock_i function
* @endinternal
*
* @brief   Global mutex unlock
*         (was unlock the scheduller)
* @param[in] owner                    - pthread id of task which should unlock
* @param[in] force                    -  to unlock recursive locks
*
* @retval OK                       - on success
* @retval ERROR                    - on error
*/
static GT_VOID V2L_taskUnlock_i(
    IN  pthread_t   owner,
    IN  int         force
)
{
    DO_LOCK_MUTEX(&PRV_SHARED_DB.taskLock_mtx);
    if (PRV_SHARED_DB.taskLock_owner == owner)
    {
        if (force)
            PRV_SHARED_DB.taskLock_count = 0;
        else
            PRV_SHARED_DB.taskLock_count--;
        if (PRV_SHARED_DB.taskLock_count == 0)
        {
            PRV_SHARED_DB.taskLock_owner = 0;
            pthread_cond_signal(&PRV_SHARED_DB.taskLock_cond);
        }
    }
    pthread_mutex_unlock(&PRV_SHARED_DB.taskLock_mtx);
}


/**
* @internal CPSS_osTaskUnLock function
* @endinternal
*
* @brief   Enable task rescheduling.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS CPSS_osTaskUnLock (GT_VOID)
{
    V2L_taskUnlock_i(pthread_self(), 0);

    return GT_OK;
}

#if defined(ASIC_SIMULATION) && __GLIBC_PREREQ(2,13)
    /* we don't need task_log(), gdb shows info */
#else
/**
* @internal task_log function
* @endinternal
*
* @brief   logs the message to /tmp/tasks or to stderr
*/
static GT_VOID
    task_log(const GT_CHAR *fmt, ...)
{
    static FILE *fpLog = 0;
    /*
    **  v2pthread_file_lock is a mutex used to make open log file a critical section
    */
    static pthread_mutex_t
        v2pthread_file_lock = PTHREAD_MUTEX_INITIALIZER;
    va_list ap;

    pthread_mutex_lock( &v2pthread_file_lock );

    if (fpLog == 0)
    {
        /* Open log file to record thread creation */
        /* We need it for debugging */
#if !defined(ASIC_SIMULATION) && !defined(__FreeBSD__)
#  define TASKS_FILE_NAME "/dev/shm/tasks"
#else
#  define TASKS_FILE_NAME "/tmp/tasks"
#endif
        fpLog = fopen(TASKS_FILE_NAME, "a");
        if (fpLog == 0)
        {
            fprintf(stderr,
                        "fopen(\"%s\", \"a\") failed, errno(%s)\n",
                        TASKS_FILE_NAME,
                        strerror(errno));
            /* fallback */
            fpLog = stderr;
        }
        else
        {
            /* log file should be line buffered */
            setbuf(fpLog, NULL);
        }
    }

    va_start( ap, fmt );
    vfprintf( fpLog, fmt, ap );
    va_end( ap );

    pthread_mutex_unlock( &v2pthread_file_lock );
}
#endif


