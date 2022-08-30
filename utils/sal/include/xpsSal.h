// xpsSal.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifdef __cplusplus
#include <iostream>
#include <exception>
#endif
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>         //To support directory related operations (i.e. openDir,clodeDir) - Currently used in serdesInit
#include "xpEnums.h"
#include "xpsEnums.h"
#include "xpTypes.h"



/**
* \brief xpMalloc
* \param [in] size
*/
void* xpMalloc(size_t size);

/**
 * \brief xpFree
 * \param [in] ptr
 */
void xpFree(void *ptr);

/**
 * \brief xpShadowMalloc
 * \param [in] id
 * \param [in] size
 */
void* xpShadowMalloc(size_t id, size_t size);

/**
 * \brief xpShadowFree
 * \param [in] ptr
 */
void xpShadowFree(void *ptr);
/**
 * \brief xpOpen
 * \param [in] path
 * \param [in] flag
 * \return int
 */

int xpOpen(const char* path, int flag);

/**
 * \brief xpClose
 * \param [in] fd
 * \return int
 */
int xpClose(int fd);
/**
 * \brief xpGetCurrentDir
 * \param [out] buf
 * \param [in] size
 * \return int value
 */
int xpGetCurrentDir(char* buf, size_t size);

/**
 * \brief xpOpenDir
 * \param [in] name
 * \return void* dirPtr
 */
void* xpOpenDir(const char* name);

/**
  *\brief xpCloseDir
 * \param [in] dirPtr
 */
void  xpCloseDir(void* dirPtr);

/**
 * \brief xpSetEnv
 * \param [in] envVarName
 * \param [in] envVarValue
 */
void xpSetEnv(const char* envVarName, const char* envVarValue);
/**
*  * \brief Translate Address by an Offset in binary tree.
*   *
*    * \param [in] addr
*     *
*      * \return XP_STATUS
*       */
pthread_t xpOsThreadSelf();

void* xpSwPersistentMalloc(size_t size, uint64_t unique_object_id);

XP_STATUS xpSwPersistentFree(void * ptr);

XP_STATUS xpUpdateAddress(void **addr);

XP_STATUS xpSwPersistentAddressTranslation(void *inAddress, void **outAddress);

XP_STATUS xpPersistentGetMemoryOffsetData(uint64_t *offset, uint8_t *direction);

XP_STATUS xpSwPersistentSetUniqueOID(uint64_t unique_object_id,
                                     void** allocatedPtr);


#define xpThreadSelf() xpOsThreadSelf();
#define XP_PERSISTENT_MALLOC(SIZE)  xpMalloc(SIZE);
#define XP_PERSISTENT_FREE(PTR)  xpFree(PTR);
#define XP_ID_BASED_PERSISTENT_MALLOC(id, X) xpSwPersistentMalloc(X , id);
#define XP_ID_BASED_PERSISTENT_FREE(X)  xpSwPersistentFree((X));
#define XP_PERSISTENT_ADDRESS_TRANSLATION(in, out)  xpSwPersistentAddressTranslation(in , out);
#define XP_SW_PERSISTENT_SET_ID(id, ptr) xpSwPersistentSetUniqueOID(id , ptr);
#define XP_PERSISTENT_GET_OFFSET_DIR(offset, dir)  xpPersistentGetMemoryOffsetData(offset ,dir);
