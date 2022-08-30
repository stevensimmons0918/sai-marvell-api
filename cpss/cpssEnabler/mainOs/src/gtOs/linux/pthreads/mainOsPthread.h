/**
********************************************************************************
* @file mainOsPthread.h
*
* @brief Pthread implementation of mainOs tasks and semaphores
*
* @version   4
********************************************************************************
*/
/*******************************************************************************
* mainOsPthread.h
*
* DESCRIPTION:
*       Pthread implementation of mainOs tasks and semaphores
*
* DEPENDENCIES:
*       Linux, CPU independed, and posix threads implementation.
*
* FILE REVISION NUMBER:
*       $Revision: 4 $
*
*******************************************************************************/
#ifndef __mainOsPthread_h
#define __mainOsPthread_h

#include <sched.h>
#include <sys/types.h>
#include <time.h>
#include <inttypes.h>
#include <errno.h>

/************* Defines ********************************************************/

#ifndef IN
# define IN
#endif
#ifndef OUT
# define OUT
#endif

#ifndef CPSS_CLOCK
#define CPSS_CLOCK CLOCK_MONOTONIC
#endif

#if __GLIBC_PREREQ(2,13)
# ifdef __USE_XOPEN2K
#  define HAVE_ROBUST_MUTEXES 1
# endif
#endif

#ifdef HAVE_ROBUST_MUTEXES
#  define MUTEXATTR_SETROBUST(_mattr) pthread_mutexattr_setrobust((_mattr), PTHREAD_MUTEX_ROBUST)
#else
#  define MUTEXATTR_SETROBUST(_mattr)
#endif
#if defined(SHARED_MEMORY) && defined(HAVE_ROBUST_MUTEXES)
#  define DO_LOCK_MUTEX(_mtxPtr) \
        ({  int _rc = pthread_mutex_lock((_mtxPtr)); \
            if (_rc == EOWNERDEAD) \
                pthread_mutex_consistent((_mtxPtr)); \
            _rc; })
#  define DO_COND_WAIT(_condPtr,_mtxPtr) \
        ({  int _rc = pthread_cond_wait((_condPtr), (_mtxPtr)); \
            if (_rc == EOWNERDEAD) \
                pthread_mutex_consistent((_mtxPtr)); \
            _rc; })
#  define DO_COND_TIMEDWAIT(_condPtr,_mtxPtr,_tsPtr) \
        ({  int _rc = pthread_cond_timedwait((_condPtr), (_mtxPtr), (_tsPtr)); \
            if (_rc == EOWNERDEAD) \
                pthread_mutex_consistent((_mtxPtr)); \
            _rc; })

#else /* ! robust mutex */
#  define DO_LOCK_MUTEX(_mtxPtr) \
        pthread_mutex_lock((_mtxPtr))
#  define DO_COND_WAIT(_condPtr,_mtxPtr) \
        pthread_cond_wait((_condPtr), (_mtxPtr))
#  define DO_COND_TIMEDWAIT(_condPtr,_mtxPtr,_tsPtr) \
        pthread_cond_timedwait((_condPtr), (_mtxPtr), (_tsPtr))
#endif

#ifdef SHARED_MEMORY
#  define CONDATTR_SETPSHARED(_cattr)   pthread_condattr_setpshared(_cattr, PTHREAD_PROCESS_SHARED)
#else
#  define CONDATTR_SETPSHARED(_cattr)
#endif
#ifdef __USE_XOPEN2K
#  define CONDATTR_SETCLOCK(_cattr)     pthread_condattr_setclock(_cattr, CPSS_CLOCK)
#else
#  define CONDATTR_SETCLOCK(_cattr)
#  warning "pthread_condattr_setclock() not defined, undefined behavior in case of system clock changed"
#endif

#ifdef SHARED_MEMORY
#  define CREATE_MTX(mtx) do { \
        pthread_mutexattr_t mattr; \
        pthread_mutexattr_init(&mattr); \
        pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED); \
        MUTEXATTR_SETROBUST(&mattr); \
        pthread_mutex_init(mtx, &mattr); \
    } while(0)
#else
#  define CREATE_MTX(mtx) pthread_mutex_init(mtx, NULL)
#endif

#if defined(SHARED_MEMORY) || defined(__USE_XOPEN2K)
#  define CREATE_COND(cond) do { \
        pthread_condattr_t cattr; \
        pthread_condattr_init(&cattr); \
        CONDATTR_SETPSHARED(&cattr); \
        CONDATTR_SETCLOCK(&cattr); \
        pthread_cond_init(cond, &cattr); \
        pthread_condattr_destroy(&cattr); \
    } while(0)
#else
#  define CREATE_COND(cond) pthread_cond_init(cond, NULL)
#endif

/************* lkernelLib.c ***************************************************/
/* internals */

void *V2L_ts_malloc( size_t blksize );
void V2L_ts_free( void *blkaddr );


/************* ltaskLib.c *****************************************************/

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
    IN  const char *name
);


/************* ltimerLib.c ****************************************************/

/********************************
** timespec arithmetic
********************************/

/*****************************************************************************
** timespec_is_zero - returns 1 if value is zero
*****************************************************************************/
extern int timespec_is_zero(struct timespec *value);

/*****************************************************************************
** timespec_gt - returns 1 if a greater than b
*****************************************************************************/
extern int timespec_gt(struct timespec *a, struct timespec *b);

/*****************************************************************************
** milliseconds2timespec - convert time in system ticks to timespec structure
**            ( one tick is currently implemented as ten milliseconds )
*****************************************************************************/
extern void milliseconds2timespec(int milliseconds, struct timespec *result);

/*****************************************************************************
** timespec_add - add value to accumulator
*****************************************************************************/
extern void timespec_add(struct timespec *accumulator, struct timespec *value);

/*****************************************************************************
** timespec_sub - sub b from a, delta = 0 if b >= a, otherwise delta = a - b
*****************************************************************************/
extern void timespec_sub(struct timespec *a, struct timespec *b, struct timespec *delta);


/************* lsemLib.c ******************************************************/

/**
* @internal V2L_lsemInit function
* @endinternal
*
* @brief   Initialize semaphores
*
* @retval OK                       - on success
* @retval ERROR                    - on error
*
* @note This function called from osStartEngine()
*
*/
int V2L_lsemInit(void);

/**
* @internal V2L_lmsgQInit function
* @endinternal
*
* @brief   Initialize message queues
*
* @retval OK                       - on success
* @retval ERROR                    - on error
*
* @note This function called from osStartEngine()
*
*/
int V2L_lmsgQInit(void);

#endif /* __mainOsPthread_h */


