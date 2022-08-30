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
* @file stubs.c
*
* @brief This is Linux simulation packet receive and interrupt implementation
*
* @version   14
********************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/select.h>

#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/gtGenTypes.h>
#include <gtOs/simStubs.h>

#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsTask.h>
#include <gtOs/gtOsSem.h>
#ifdef SHARED_MEMORY
# include <gtOs/gtOsSharedData.h>
#endif
#ifndef __SHARED_DATA_MAINOS
# define __SHARED_DATA_MAINOS
#endif

/* the reason for : LINUX_SIM_CRASH_ON_osTaskSuspend :
as we get next CRASH during the 'system reset'

        ====== The test "gtShutdownAndCoreRestart" started
luaShellExecute: fromLua_gtShutdownAndCoreRestart
prvTgfResetAndInitSystem :started . doHwReset[0]
cpssResetSystem [CPSS_SYSTEM_RESET_TYPE_RESET_SW_IN_gtShutdownAndCoreRestart_E]: started
cpssResetSystem : indicate the supporting tasks to be ready for termination that will soon follow ...
APPDEMO_TASK_MGR:  : thread [portManagerTask] is notified to gracefully exit (wait for osTaskDelete())
cpssResetSystem : reset events engine and kill event handlers tasks
appDemoEventRequestDrvnModeReset: mask HW events
APPDEMO_TASK_MGR:  : thread [portManagerTask] is waiting to gracefully exit (wait for osTaskDelete())
appDemoEventRequestDrvnModeReset: destroy task handlers
[Thread 0x45446b40 (LWP 31876) exited]
[Thread 0x454ecb40 (LWP 31877) exited]
[Thread 0x46427b40 (LWP 31878) exited]
[Thread 0x46448b40 (LWP 31879) exited]
[Thread 0x46469b40 (LWP 31880) exited]
[Thread 0x4648ab40 (LWP 31881) exited]
[Thread 0x464abb40 (LWP 31882) exited]
[Thread 0x464ccb40 (LWP 31883) exited]
[Thread 0x464edb40 (LWP 31884) exited]
appDemoEventRequestDrvnModeReset: clean DB
appDemoEventRequestDrvnModeReset: destroy the counters DB + related semaphore
cpssResetSystem : terminate more tasks
APPDEMO_TASK_MGR: appDemoWrap_osTaskDelete : start terminate thread [portManagerTask]
APPDEMO_TASK_MGR: appDemoWrap_osTaskDelete : ended terminate thread [portManagerTask]
Disable interrupt line [0x00000000]
cpssResetSystem : start remove device[0]

Program received signal SIGSTOP, Stopped (signal).
[Switching to Thread 0x454cbb40 (LWP 31875)]
0x40022420 in __kernel_vsyscall ()
(gdb) bt
#0  0x40022420 in __kernel_vsyscall ()
#1  0x40040d4b in pthread_cond_wait@@GLIBC_2.3.2 () from /lib/i386-linux-gnu/libpthread.so.0
#2  0x0841a094 in osSemWait ()
#3  0x0841bb57 in SHOSTP_intr_thread ()
#4  0x0841a425 in os_task_wrapper ()
#5  0x4003cf70 in start_thread () from /lib/i386-linux-gnu/libpthread.so.0
#6  0x4019347e in clone () from /lib/i386-linux-gnu/libc.so.6
*/
#define LINUX_SIM_CRASH_ON_osTaskSuspend

#undef DO_SUSPEND_TASKS_FOR_INTERRUPT

#ifndef FALSE
    #define FALSE               0
#endif
#ifndef TRUE
    #define TRUE                1
#endif

#define PRV_SHARED_DB osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.gtLinuxSimStubsSrc

/* commandLine should belong to this module to make mainOs module workable
 * without CPSS Enabler into another third-party client. */
char  commandLine[1280] __SHARED_DATA_MAINOS;

extern void* simOsTaskCreate (
    int prio,
    unsigned (__TASKCONV *startaddrPtr)(void*),
    void    *arglistPtr
);
extern GT_STATUS simOsTaskDelete
(
    void* hThread
);
extern void SASICG_exit(
    void
);



/**
* @internal SHOSTG_abort function
* @endinternal
*
* @brief   Called in ordrer to terminate the program.
*/
void SHOSTG_abort(void)
{
    SASICG_exit();
    exit(0);
}

/**
* @internal SHOSTC_exit function
* @endinternal
*
* @brief   Called in ordrer to terminate the program.
*/
void SHOSTC_exit(UINT_32  status)
{
   SHOSTG_abort();
}

/*******************************************************************************
* SHOSTG_reset
*
* DESCRIPTION:
*
* INPUTS:
*      status
*
* OUTPUTS:
*       None
*
* RETURNS:
*      None
*
* COMMENTS:
*       None
*
*******************************************************************************/
#ifndef SHARED_MEMORY
extern char **cmdArgv;
extern char **cmdEnv;
#endif
extern void SHOSTG_reset (

    /*!     INPUTS:             */

    unsigned int status

)
{
#ifndef SHARED_MEMORY
    int i;
    /*SHOSTG_abort();*/
    SASICG_exit();
    /* all descriptors except 0,1,2 should be closed on exec */
    for (i = 3; i < FD_SETSIZE; i++)
    {
        /* ignore EBADF */
        fcntl(i, F_SETFD, FD_CLOEXEC);
    }
    execve(cmdArgv[0],cmdArgv,cmdEnv);
    exit(0);
#else
    osPrintf("SHOSTG_reset is not supported in SHARED_MEMORY mode\n");
#endif
}

/**
* @internal SHOSTC_dll_abort function
* @endinternal
*
* @brief   Called in ordrer to terminate the program with a printed message.
*
* @param[in] msg                      - the message to be print out,
*                                       None
*/
extern void SHOSTC_dll_abort(
    /* INPUT  */
    char* msg
)
{
   osPrintf("%s",msg) ;

   SHOSTG_abort() ;
}

/*******************************************************************************
* SHOSTC_intr_table_lock
*
* DESCRIPTION:
*
* INPUTS:
*       None
*
* OUTPUTS:
*      None
*
* RETURNS:
*      None
*
* COMMENTS:
*    The name source is historical.
*******************************************************************************/
#define SHOSTC_intr_table_lock() osSemWait(PRV_SHARED_DB.SHOSTP_intr_table_mtx,OS_WAIT_FOREVER)

/*******************************************************************************
* SHOSTC_intr_table_unlock
*
* DESCRIPTION:
*
* INPUTS:
*       None
*
* OUTPUTS:
*      None
*
* RETURNS:
*      None
*
* COMMENTS:
*    The name source is historical.
*******************************************************************************/
#define SHOSTC_intr_table_unlock() osSemSignal(PRV_SHARED_DB.SHOSTP_intr_table_mtx)

#ifdef  DO_SUSPEND_TASKS_FOR_INTERRUPT
/**
* @internal SHOSTP_do_intr_susp function
* @endinternal
*
* @brief   Suspend all interrupt tasks that have lower priority than the specified
*         interrupt.
* @param[in] intr_ptr                 - pointer to interrupt structure.
*                                       None
*
* @note The name source is historical.
*
*/
static void SHOSTP_do_intr_susp (

    /*!     INPUTS:             */

    SHOSTP_intr_STC *intr_ptr
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

    SHOSTP_intr_STC *tmp_ptr ;
    UINT_32 i;
    DWORD thd_id;

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    osTaskGetSelf(&thd_id);
    /* suspend all interrupts with less or equal priority */
    for(i=0 ; i < SHOSTP_max_intr_CNS; i++)
    {
      if(PRV_SHARED_DB.SHOSTP_intr_table[i].hnd)
      {
        tmp_ptr = &PRV_SHARED_DB.SHOSTP_intr_table[i];
        if(tmp_ptr->thd_id == thd_id ||
           (intr_ptr && tmp_ptr->priority > intr_ptr->priority))
            continue ;
        if ( tmp_ptr->mask == FALSE ){
            if ( osTaskSuspend( tmp_ptr->thd_id ) != 0 )
                SHOSTC_dll_abort("\nSuspend inter failed.\n");

        }
        tmp_ptr->susp_flag++ ;
      }
    }
}
#endif /* DO_SUSPEND_TASKS_FOR_INTERRUPT */

/**
* @internal SHOSTC_init_intr_table function
* @endinternal
*
*
* @note The name source is historical.
*
*/
void SHOSTC_init_intr_table (

    void
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    memset((void *)PRV_SHARED_DB.SHOSTP_intr_table, 0, sizeof(PRV_SHARED_DB.SHOSTP_intr_table));
    if (osSemMCreate("ShostpIntTblMtx", &PRV_SHARED_DB.SHOSTP_intr_table_mtx ) != GT_OK)
        exit (0);
}

/**
* @internal SHOSTP_do_intr_resume function
* @endinternal
*
* @brief   Resumes all interrupt tasks.
*
* @param[in] intr_ptr                 - pointer to interrupt structure.
*                                       None
*
* @note The name source is historical.
*
*/
static void SHOSTP_do_intr_resume (

    /*!     INPUTS:             */

    SHOSTP_intr_STC *intr_ptr
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

    SHOSTP_intr_STC *tmp_ptr ;
    UINT_32 i;
    DWORD thd_id;

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    osTaskGetSelf(&thd_id) ;

    /* resume all interrupts with less or equal priority */
    for(i=0; i < SHOSTP_max_intr_CNS; i++)
    {
        if(PRV_SHARED_DB.SHOSTP_intr_table[i].hnd)
        {
            tmp_ptr = &PRV_SHARED_DB.SHOSTP_intr_table[i] ;
            if(tmp_ptr->thd_id == thd_id ||  !tmp_ptr->susp_flag ||
              (intr_ptr && tmp_ptr->priority > intr_ptr->priority))
                continue ;
            if ( tmp_ptr->mask == FALSE )
            {
                if (( tmp_ptr->susp_flag == 0 )||( osTaskResume((GT_TASK)tmp_ptr->hnd) != 0 ))
                    SHOSTC_dll_abort("\nResume inter failed.\n");
            }

            tmp_ptr->susp_flag-- ;
        }
    }
}

/*******************************************************************************
* SHOSTP_intr_thread
*
* DESCRIPTION:
*       This tasks is the ISR simulation.
*
* INPUTS:
*       intr_ptr - pointer to interrupt structure.
*
* OUTPUTS:
*      None
*
* RETURNS:
*      None
*
* COMMENTS:
*    The name source is historical.
*******************************************************************************/
static unsigned __TASKCONV  SHOSTP_intr_thread(void* pPtr)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

  SHOSTP_intr_STC *intr_ptr = (SHOSTP_intr_STC *) pPtr ;
  SHOSTP_intr_FUN *fun ;
  SHOSTP_intr_param_FUN *fun_param;

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    for( ;intr_ptr && intr_ptr->ev ; )
    {

        osSemWait(intr_ptr->ev,0) ; /* wait forever */
        fun = intr_ptr->fun;
        fun_param = (SHOSTP_intr_param_FUN *)intr_ptr->fun;

        if(!fun) /* || !SHOSTP_intr_enable) */
            continue ;

        if(intr_ptr->disabled)
        {
            /* wait for next signal */
            continue;
        }
        /*
         * Do not suspend other interrupt tasks osTaskLock() is
         * enough
         */
#ifdef  DO_SUSPEND_TASKS_FOR_INTERRUPT
        SHOSTP_do_intr_susp(intr_ptr) ;
#endif

        /* KEEP THE TASKS LOCKED -
           DO NOT LET OTHER TASKS INTERRUPT IN THE MIDDLE OF THE "ISR" */
        if ( intr_ptr->mask == FALSE )
        {
            if(intr_ptr->param_exist == TRUE)
                (*fun_param)(intr_ptr->param);
            else
                (*fun)();
        }
        else
            intr_ptr->was_ev = TRUE;

#ifdef  DO_SUSPEND_TASKS_FOR_INTERRUPT
        SHOSTP_do_intr_resume(intr_ptr) ;
#endif
    }
    return 0 ;
}


/**
* @internal SHOSTG_interrupt_enable_one function
* @endinternal
*
* @brief   Enables one interrupt.
*
* @param[in] intr                     - interrupt to be enabled.
*                                       None
*
* @note The name source is historical.
*
*/
extern void SHOSTG_interrupt_enable_one (

    /*!     INPUTS:             */

    UINT_32    intr
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

    SHOSTP_intr_STC *tmp_ptr ;

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

 /* take interrupts simulation semaphore   */
   SHOSTC_intr_table_lock();

   tmp_ptr = &PRV_SHARED_DB.SHOSTP_intr_table[intr] ;

   if ( tmp_ptr->mask == FALSE )
   {
       SHOSTC_intr_table_unlock();
       return;
   }

   tmp_ptr->mask = FALSE;

   if( tmp_ptr->susp_flag )
   {
       SHOSTC_intr_table_unlock();
       return;
   }

#ifdef LINUX_SIM_CRASH_ON_osTaskSuspend
   tmp_ptr->disabled = 0;
#else

   if ( osTaskResume(tmp_ptr->thd_id) != 0 )
       SHOSTC_dll_abort("\nResume inter failed.\n");

   if((intr) && (intr <= SHOSTP_max_intr_CNS) && (tmp_ptr->thd_id) && (tmp_ptr->ev) )
   {
       if (tmp_ptr->was_ev)
           osSemSignal((GT_SEM)tmp_ptr->ev) ;

       tmp_ptr->was_ev = FALSE;
   }
   else
   {
       SHOSTC_dll_abort("\nResume spec int failed.\n");
   }
#endif

   SHOSTC_intr_table_unlock();

}

/**
* @internal SHOSTG_interrupt_disable_one function
* @endinternal
*
* @brief   disables one interrupt.
*
* @param[in] intr                     - interrupt to be disabled.
*                                       None
*
* @note The name source is historical.
*
*/
extern void SHOSTG_interrupt_disable_one (

    /*!     INPUTS:             */

    UINT_32    intr
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/
    SHOSTP_intr_STC *tmp_ptr ;
    DWORD thd_id;

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    /* take interrupts simulation semaphore   */
    SHOSTC_intr_table_lock();

    /* suspend specific interrupt */
    osTaskGetSelf(&thd_id) ;

    tmp_ptr = &PRV_SHARED_DB.SHOSTP_intr_table[intr];

    if ( tmp_ptr->mask == TRUE )
    {
        SHOSTC_intr_table_unlock();
        return;
    }

    tmp_ptr->mask = TRUE;

    if( tmp_ptr->susp_flag )
    {
        SHOSTC_intr_table_unlock();
        return;
    }

    if ( tmp_ptr->thd_id == thd_id )
    {
        SHOSTC_intr_table_unlock();
        return;
    }

#ifdef LINUX_SIM_CRASH_ON_osTaskSuspend
    tmp_ptr->disabled = 1;
#else
    if ( osTaskSuspend( tmp_ptr->thd_id ) != 0 )
        SHOSTC_dll_abort("\nSuspend inter failed.\n");
#endif

    SHOSTC_intr_table_unlock();
}

/**
* @internal SHOSTG_interrupt_do_enable function
* @endinternal
*
* @param[in] new_val                  -
*                                       None
*
* @note The name source is historical.
*
*/
extern  void  SHOSTG_interrupt_do_enable (

    UINT_32     new_val
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/

    SHOSTC_intr_table_lock();

    SHOSTP_do_intr_resume(0);

    SHOSTC_intr_table_unlock();
}

/**
* @internal SHOSTG_interrupt_enable function
* @endinternal
*
* @param[in] new_val                  -
*                                       None
*
* @note The name source is historical.
*
*/
extern  void  SHOSTG_interrupt_enable (

    UINT_32     new_val
)
{
}

/**
* @internal SHOSTG_interrupt_disable function
* @endinternal
*
*
* @note TBD: Implement
*
*/
UINT_32 SHOSTG_interrupt_disable (void)
{
    return 0;
}


/**
* @internal SHOSTG_bind_interrupt_with_param function
* @endinternal
*
* @param[in] priority                 - the  to be given to the interrupt task.
* @param[in] intr                     - interrupt number.
* @param[in] fun                      - interrupt handler.
* @param[in] ev                       - pointer to a semaphore ID (to be allocated) for the interrupt task.
* @param[in] param                    - any parameter to be transffered to the interrupt task.
*                                       None
*
* @note The name source is historical.
*
*/
extern UINT_32 SHOSTG_bind_interrupt_with_param (

    /*!     INPUTS:             */

    UINT_32 priority,

    UINT_32 intr,

    void *fun,

    HANDLE ev,

    UINT_32 param
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/
    UINT_32  ret_val = 0 ;
    GT_SEM   semId = (GT_SEM)0;
    SHOSTP_intr_STC *intr_ptr ;
    static int cnt = 0;
    char   semName[50];
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    SHOSTC_intr_table_lock();

    intr_ptr = &PRV_SHARED_DB.SHOSTP_intr_table[intr] ;

    if(!intr_ptr->hnd)
    {
        intr_ptr->id = intr ;
        intr_ptr->mask = FALSE ;
        intr_ptr->priority = priority ;
        intr_ptr->susp_flag = 0 ;
        intr_ptr->fun = (SHOSTP_intr_FUN *) fun ;
        intr_ptr->param_exist = TRUE;
        intr_ptr->param = param;
        intr_ptr->disabled = 0;

        /* We use semaphore instead of event */
        if(!ev)
        {
            osSprintf(semName, "intSem_%d", cnt);
            if (osSemBinCreate(semName, 0, &semId) != 0)
            {
                SHOSTC_intr_table_unlock();

                SHOSTC_dll_abort("\nCan't create interrupt semaphore.\n");
            }
        }

        intr_ptr->ev = (HANDLE)semId;

        intr_ptr->thd_id = (GT_UINTPTR)simOsTaskCreate(7/*GT_TASK_PRIORITY_TIME_CRITICAL*/,
                                                 (unsigned (*)(void*))SHOSTP_intr_thread,
                                                 intr_ptr);

        intr_ptr->hnd = (HANDLE)intr_ptr->thd_id;

        if(!intr_ptr->thd_id)
        {
            SHOSTC_dll_abort("\nCan't create inter thread.\n");
        }

        ret_val = intr ;
    } /* if(!intr_ptr->hnd) */
    else
    {
        SHOSTC_intr_table_unlock();
        SHOSTC_dll_abort("\nInterrupt index already in use.\n");
    }

    SHOSTC_intr_table_unlock();

    return ret_val;
}

/**
* @internal SHOSTG_unbind_interrupt_with_param function
* @endinternal
*
* @param[in] intr     - interrupt number.
*
*/
extern UINT_32 SHOSTG_unbind_interrupt_with_param (

    /*!     INPUTS:             */

    UINT_32 intr
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/
    UINT_32  ret_val = 0 ;
    SHOSTP_intr_STC *intr_ptr ;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    SHOSTC_intr_table_lock();

    intr_ptr = &PRV_SHARED_DB.SHOSTP_intr_table[intr] ;

    if(intr_ptr->hnd)
    {
        if(intr_ptr->id == intr)
        {
            intr_ptr->mask = TRUE ;
            intr_ptr->priority = 0 ;
            intr_ptr->susp_flag = 0 ;
            intr_ptr->fun = NULL ;
            intr_ptr->param_exist = FALSE;
            intr_ptr->param = 0;

            simOsTaskDelete((void*)intr_ptr->thd_id);

            intr_ptr->thd_id = 0;

            osSemDelete(intr_ptr->ev);

            memset(&PRV_SHARED_DB.SHOSTP_intr_table[intr],0,sizeof(SHOSTP_intr_STC));
        }
        else
        {
            SHOSTC_intr_table_unlock();
            SHOSTC_dll_abort("\nInvalid Interrupt index.\n");
        }

    } /* if(intr_ptr->hnd) */
    else
    {
        SHOSTC_intr_table_unlock();
        SHOSTC_dll_abort("\nNo Binded Interrupts exist.\n");
    }

    SHOSTC_intr_table_unlock();

    return ret_val;
}

extern UINT_32  SHOSTG_set_interrupt
(
    /*!     INPUTS:             */
    UINT_32 intr
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    SHOSTC_intr_table_lock();

    if(intr && intr <= SHOSTP_max_intr_CNS &&
       PRV_SHARED_DB.SHOSTP_intr_table[intr].hnd && PRV_SHARED_DB.SHOSTP_intr_table[intr].ev)
    {
        if (PRV_SHARED_DB.SHOSTP_intr_table[intr].mask == TRUE )
        {
            PRV_SHARED_DB.SHOSTP_intr_table[intr].was_ev = TRUE;
        }
        else
        {
            osSemSignal(PRV_SHARED_DB.SHOSTP_intr_table[intr].ev) ;
        }

       SHOSTC_intr_table_unlock();
       return 1 ;
    }

    SHOSTC_intr_table_unlock();

    return 0 ;
}

char* strlwr (char* str)
{
    char        *p;

    for (p = str; *p; p++)
        if (*p >= 'A' && *p <= 'Z')
            *p += 'a' - 'A';
        /*  *p = tolower((unsigned char)(*p)); */

    return str;
}

char *GetCommandLine(void)
{
    return commandLine;
}

void LinuxSim_stubs_init(void)
{
}


