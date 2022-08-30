// xpsSal.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsCommon.h
 * \brief This file contains API prototypes and type definitions
 *  for use to other layer by exposing from the XPS layer
 */
#include "xpsSal.h"
/**
 * \brief xpMalloc
 * \param [in] size
 */
void* xpMalloc(size_t size)
{
    return malloc(size);
}

/**
 * \brief xpFree
 * \param [in] ptr
 */
void xpFree(void *ptr)
{
    free(ptr);
    ptr = NULL;
}

/**
 * \brief xpShadowMalloc
 * \param [in] id
 * \param [in] size
 */
void* xpShadowMalloc(size_t id, size_t size)
{
    void *m = malloc(size);
    if (m)
    {
        memset(m, 0x0, size);
    }
    return m;
}

/**
 * \brief xpShadowFree
 * \param [in] ptr
 */
void xpShadowFree(void *ptr)
{
    free(ptr);
}
/**
 * \brief xpOpen
 * \param [in] path
 * \param [in] flag
 * \return int
 */
int xpOpen(const char* path, int flag)
{
    return open(path, flag);
}

/**
 * \brief xpClose
 * \param [in] fd
 * \return int
 */
int xpClose(int fd)
{
    return close(fd);
}

/**
 * \brief xpGetCurrentDir
 * \param [out] buf
 * \param [in] size
 * \return int value
 */
int xpGetCurrentDir(char* buf, size_t size)
{
    char* ret =  NULL;
    ret =  getcwd(buf, size);
    return ((ret == NULL) ? 1 : 0);
}

/**
 * \brief xpOpenDir
 * \param [in] name
 * \return void* dirPtr
 */
void* xpOpenDir(const char* name)
{
    void* ret = NULL;
    ret = (void*) opendir(name);
    return ret;
}

/**
  *\brief xpCloseDir
 * \param [in] dirPtr
 */
void  xpCloseDir(void* dirPtr)
{
    closedir((DIR*) dirPtr);
}

/**
 * \brief xpGetEnv
 * \param [in] envVar
 */
char* xpGetEnv(const char* envVar)
{
    char* ret = NULL;
    ret = getenv(envVar);
    return ret;
}

/**
 * \brief xpSetEnv
 * \param [in] envVarName
 * \param [in] envVarValue
 */
void xpSetEnv(const char* envVarName, const char* envVarValue)
{
    setenv(envVarName, envVarValue, 1);
}

/**
 * \brief xpVprintf
 * \param [in] fmt
 * \param [in] ap
 */
#if 0
int xpVprintf(const char* fmt, va_list ap)
{
    return vprintf(fmt, ap);
    return 0;
}
#endif

pthread_t xpOsThreadSelf()
{
    return pthread_self();
}


void* xpSwPersistentMalloc(size_t size, uint64_t unique_object_id)
{
    void* allocateMemPtr = malloc(size);
    return allocateMemPtr;
}

XP_STATUS xpSwPersistentFree(void * ptr)
{

    free(ptr);
    return XP_NO_ERR;

}

XP_STATUS xpSwPersistentAddressTranslation(void *inAddress, void **outAddress)
{
    return XP_NO_ERR;

}

XP_STATUS xpPersistentGetMemoryOffsetData(uint64_t *offset, uint8_t *direction)
{
    return XP_NO_ERR;


}

XP_STATUS xpUpdateAddress(void **addr)
{
    return XP_NO_ERR;

}

XP_STATUS xpSwPersistentSetUniqueOID(uint64_t unique_object_id,
                                     void** allocatedPtr)
{

    return XP_NO_ERR;

}
