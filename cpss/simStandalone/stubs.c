#define _GNU_SOURCE
#include <os/simTypesBind.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>

#ifndef CPSS_CLOCK
#define CPSS_CLOCK CLOCK_MONOTONIC
#endif



int printf(const char *format, ...);
GT_VOID *osMalloc(IN GT_U32 size)
{return malloc(size);}
GT_VOID CPSS_osFree(IN GT_VOID* const memblock)
{free(memblock);}

void (*cmdOsFree)(void *) = CPSS_osFree;
void *(*cmdOsMalloc)(GT_U32) = osMalloc;
int (*cmdOsPrintf)(const char *, ...) = printf;

GT_STATUS osMemGlobalDbShmemUnlink(GT_CHAR_PTR  name){return 0;}
GT_VOID *osCacheDmaMalloc(GT_U32 size){return malloc(size);}
GT_VOID *osCacheDmaMallocByWindow(GT_U32 windowId,GT_U32 size){return malloc(size);}
GT_STATUS osCacheDmaFree(GT_VOID *ptr){free(ptr);return GT_OK;}
GT_VOID *osStaticMalloc(GT_U32 size){return malloc(size);}

typedef enum
{
    OS_SEMB_EMPTY = 0,
    OS_SEMB_FULL
}GT_SEMB_STATE;


GT_STATUS osSemMCreate
(
    IN  const char    *name,
    OUT GT_SEM        *smid
);
GT_STATUS osSemWait
(
    IN GT_SEM smid,
    IN GT_U32 timeOut
);

GT_STATUS osSemSignal
(
    IN GT_SEM smid
);


GT_STATUS osSemBinCreate
(
    IN  const char    *name,
    IN  GT_SEMB_STATE init,
    OUT GT_SEM        *smid
);

#ifndef FALSE
    #define FALSE               0
#endif
#ifndef TRUE
    #define TRUE                1
#endif

typedef UINT_32  SHOSTP_intr_FUN(void);
typedef UINT_32  SHOSTP_intr_param_FUN(UINT_32 param);

#define SHOSTP_max_intr_CNS 32

typedef struct SHOSTP_intr_STCT
{
    HANDLE hnd ;
    DWORD  thd_id ;
    UINT_32 id ;
    UINT_32 mask ;
    UINT_32 priority ;
    UINT_32 susp_flag ;
    HANDLE  ev ;
    UINT_32 was_ev ;
    SHOSTP_intr_FUN *fun ;
    int param_exist;
    UINT_32 param;
} SHOSTP_intr_STC ;

 SHOSTP_intr_STC  SHOSTP_intr_table[SHOSTP_max_intr_CNS] ;
 GT_SEM           SHOSTP_intr_table_mtx = 0;

extern char  commandLine[] ;

GT_CHAR *osStrCat
(
    IN GT_CHAR         *str1,
    IN const GT_CHAR   *str2
)
{
    return strcat(str1, str2);
}


GT_VOID * osMemCpy
(
    IN GT_VOID *       destination,
    IN const GT_VOID * source,
    IN GT_U32       size
)
{
    if(destination == NULL || source == NULL)
        return destination;

    return memcpy(destination, source, size);
}

GT_STATUS osPrintDmaPhysicalBase(GT_VOID)
{
    return GT_OK;
}


GT_VOID * osMemSet
(
    IN GT_VOID * start,
    IN int    symbol,
    IN GT_U32 size
)
{
    if(start == NULL)
        return start;
    return memset(start, symbol, size);
}


char* shrAddNameSuffix(const char* name, char* buffer, int bufsize)
{
    const char *e;
    char  uid[64];

    if (name == NULL || buffer == NULL)
    {
        return NULL;
    }
    strncpy(buffer,name,bufsize-1);

    /* add UID as string to buffer*/
    sprintf(uid,"%d", getuid());
    strncat(buffer, uid, bufsize-1);

    /* add CPSS_SHR_INSTANCE_NUM string to buffer */
    e = getenv("CPSS_SHR_INSTANCE_NUM");
    if (e != NULL)
    {
        strncat(buffer, e, bufsize-1);
    }
    buffer[bufsize-1] = 0;
    return buffer;
}

GT_STATUS osMemGlobalDbShmemInit
(
    IN  GT_U32      size,
    IN  GT_CHAR_PTR  name,
    OUT GT_BOOL * initDataSegmentPtr,
    OUT GT_VOID **sharedGlobalVarsPtrPtr
)
{
    *sharedGlobalVarsPtrPtr = osMalloc(size);
    * initDataSegmentPtr = GT_TRUE;
    return GT_OK;
}
void *shrMemSharedMalloc(IN size_t size)
{
    return malloc(size);
}


GT_VOID shrMemSharedFree
(
    IN GT_VOID *ptr
)
{
    free(ptr);
}
void milliseconds2timespec(int milliseconds, struct timespec *result)
{
    if (milliseconds < 1000)
    {
        result->tv_sec = 0;
        result->tv_nsec = milliseconds * 1000000;
    }
    else
    {
        result->tv_sec = milliseconds / 1000;
        result->tv_nsec = (milliseconds % 1000) * 1000000;
    }
}

void timespec_add(struct timespec *accumulator, struct timespec *value)
{
    accumulator->tv_sec += value->tv_sec;
    accumulator->tv_nsec += value->tv_nsec;
    if (accumulator->tv_nsec >= 1000000000)
    {
        accumulator->tv_sec++;
        accumulator->tv_nsec -= 1000000000;
    }
}
int timespec_gt(struct timespec *a, struct timespec *b)
{
    if (a->tv_sec > b->tv_sec)
        return 1;
    if (a->tv_sec < b->tv_sec)
        return 0;
    return (a->tv_nsec > b->tv_nsec) ? 1 : 0;
}

GT_STATUS osTimerWkAfter
(
    IN GT_U32 mils
)
{
    usleep(mils);

    return GT_OK;
}



