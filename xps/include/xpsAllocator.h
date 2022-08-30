// xpsAllocator.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpsAllocator_h_
#define _xpsAllocator_h_

#include "xpTypes.h"
#include "xpEnums.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPS_ALLOCATOR_ID_START XP_USR_DEFINED_ALLOCATOR_OID_START
#define XPS_ALLOCATOR_ID_END   XP_USR_DEFINED_ALLOCATOR_OID_END
#define XPS_MEM_POOL_ID_START  XP_USR_DEFINED_MEM_POOL_OID_START
#define XPS_MEM_POOL_ID_END    XP_USR_DEFINED_MEM_POOL_OID_END

/**
 * \brief Initializes an ID allocator.
 *
 * Initializes an ID allocator with max IDs which can be allocated
 * and the range start value. The valid IDs of allocators are
 * from XPS_ALLOCATOR_ID_START to XPS_ALLOCATOR_ID_END.
 *
 * \param [in] scopeId - scope ID
 * \param [in] allocatorId - ID of the allocator
 * \param [in] maxIds - max number of IDs
 * \param [in] rangeStart - range start value
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAllocatorInitIdAllocator(xpsScope_t scopeId, uint16_t allocatorId,
                                      uint32_t maxIds, uint32_t rangeStart);

/**
 * \brief Allocates ID from the allocator's pool.
 *
 * Given the ID of the allocator will allocate ID
 * from its pool. The valid IDs of allocators
 * are from XPS_ALLOCATOR_ID_START to XPS_ALLOCATOR_ID_END.
 *
 * \param [in] scopeId - scope ID
 * \param [in] allocatorId - ID of the allocator
 * \param [out] id - ID allocated from the pool
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAllocatorAllocateId(xpsScope_t scopeId, uint16_t allocatorId,
                                 uint32_t* id);

/**
 * \brief Allocates ID from the allocator's pool.
 *
 * Given the ID of the allocator will allocate ID
 * from its pool. The valid IDs of allocators
 * are from XPS_ALLOCATOR_ID_START to XPS_ALLOCATOR_ID_END.
 *
 * \param [in] scopeId - scope ID
 * \param [in] allocatorId - ID of the allocator
 * \param [in] id - ID to be allocated from the pool
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAllocatorAllocateWithId(xpsScope_t scopeId, uint16_t allocatorId,
                                     uint32_t id);

/**
 * \brief Releases ID back to the allocator's pool.
 *
 * Releases an allocated ID back to the allocator's pool.
 * Will return an error if the ID being released was already freed.
 * The caller can choose to ignore this error if need be.
 * The valid IDs of allocators are
 * from XPS_ALLOCATOR_ID_START to XPS_ALLOCATOR_ID_END.
 *
 * \param [in] scopeId - scope ID
 * \param [in] allocatorId - ID of the allocator
 * \param [in] id - ID to be released back to the pool
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAllocatorReleaseId(xpsScope_t scopeId, uint16_t allocatorId,
                                uint32_t id);

/**
 * \brief Allocates a persistent memory from a memory pool.
 *
 * Given the ID of the memory pool will allocate a persistent memory block
 * from this pool. Will return a pointer to the memory block allocated
 * or NULL in case of fail. The valid IDs of pools are
 * from XPS_MEM_POOL_ID_START to XPS_MEM_POOL_ID_END.
 *
 * \param [in] scopeId - scope ID
 * \param [in] poolId - ID of the memory pool
 * \param [in] size - size of the memory block to be allocated
 *
 * \return void*
 */
void* xpsAllocatorIdBasedPersistentMalloc(xpsScope_t scopeId, uint16_t poolId,
                                          size_t size);

/**
 * \brief Fress a persistent memory from a given memory pool.
 *
 * Given the ID of the memory pool will free a persistent memory block
 * from this pool. The valid IDs of pools are
 * from XPS_MEM_POOL_ID_START to XPS_MEM_POOL_ID_END.
 *
 * \param [in] ptr - pointer the memory block to be released
 *
 * \return XP_STATUS
 */
XP_STATUS xpsAllocatorIdBasedPersistentFree(void* ptr);

XP_STATUS xpsAllocatorAllocateIdGroup(xpsScope_t scopeId, uint16_t allocatorId,
                                      uint32_t size,
                                      uint32_t* id);

XP_STATUS xpsAllocatorReleaseIdGroup(xpsScope_t scopeId, uint16_t allocatorId,
                                     uint32_t size,
                                     uint32_t id);

#ifdef __cplusplus
}
#endif

#endif // _xpsAllocator_h_
