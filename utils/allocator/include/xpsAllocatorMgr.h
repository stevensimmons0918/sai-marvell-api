// xpsAllocatorMgr.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpEnums.h"
#include <vector>
#include "xpsIdAllocator.h"
#define XP_MAX_SCOPES 64
class xpsAllocatorMgr
{

public:

    /**
     * \public
     * \brief Static method used to retrieve scope instance
     *
     *
     * \return xpsAllocatorMgr*
     */
    static xpsAllocatorMgr* instance(xpScope_t scopeId);
    /**
     * \public
     * \brief Static method used to release scope instance
     *
     *
     * \return xpAllocatorMgr*
     */
    static void releaseScopeInstance(xpScope_t scopeId);

    /**
       * \public
       * \brief Initializes a user defined allocator.
       *
       * Initializes a user defined allocator with max IDs which can be allocated
       * and the range start value. The valid IDs of custom range allocator are
       * from 0 to XP_USR_DEFINED_ALLOCATOR_MAX_NUM - 1.
       *
       * \param [in] allocatorId - ID of the allocator
       * \param [in] maxIds - max number of IDs
       * \param [in] rangeStart - range start value
       *
       * \return XP_STATUS
       */
    XP_STATUS initAllocator(uint16_t allocatorId, uint32_t maxIds,
                            uint32_t rangeStart);

    /**
     * \public
     * \brief Allocates ID from the allocator's pool.
     *
     * Given the ID of the allocator will allocate ID
     * from its pool. The valid IDs of user defined allocator
     * are from 0 to XP_USR_DEFINED_ALLOCATOR_MAX_NUM - 1.
     *
     * \param [in] allocatorId - ID of the allocator
     * \param [out] id - ID allocated from the pool
     *
     * \return XP_STATUS
     */
    XP_STATUS allocateId(uint16_t allocatorId, uint32_t& id);

    /**
     * \public
     * \brief Allocates ID from the allocator's pool.
     *
     * Given the ID of the allocator will allocate ID
     * from its pool. The valid IDs of allocator are given Id.
     *
     * \param [in] allocatorId - ID of the user defined allocator
     * \param [in] id - ID to be allocated from the pool
     *
     * \return XP_STATUS
     */
    XP_STATUS allocateWithId(uint16_t allocatorId, uint32_t id);

    XP_STATUS allocateIdGroup(uint16_t allocatorId, uint32_t size, uint32_t& id);

    /**
     * \public
     * \brief Releases ID back to the allocator's pool.
     *
     * Releases an allocated ID back to the  allocator's pool, will return
     * an error if the ID being released was already freed.
     * The caller can choose to ignore this error if need be.
     * The valid IDs of user defined allocator are
     * from 0 to XP_USR_DEFINED_ALLOCATOR_MAX_NUM - 1.
     *
     * \param [in] allocatorId - ID of the allocator
     * \param [in] id - ID to be released back to the pool
     *
     * \return XP_STATUS
     */
    XP_STATUS releaseId(uint16_t allocatorId, uint32_t id);

    XP_STATUS releaseIdGroup(uint16_t allocatorId, uint32_t size, uint32_t id);
protected:

    // Constructor and Destructor

    /**
     * \public
     * \brief Default Constructor Implementation
     *
     * Will construct but will not initialize the allocators managed
     * by this class
     *
     */
    xpsAllocatorMgr(xpScope_t scopeId);



private:

    static xpsAllocatorMgr* sInstance[XP_MAX_SCOPES];
    std::vector<xpsIdAllocator*> xpsAllocatorIdVec;
    uint32_t instanceId;

};
