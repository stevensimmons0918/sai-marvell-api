// xpsIdAllocator.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsIdAllocator.h
 * \brief
 *
 */

#ifndef _xpsIdAllocator_h_
#define _xpsIdAllocator_h_
#include <vector>
#include "xpsIdAllocatorCtx.h"
#include "xpsLockArr.h"

/**
 * \class xpsIdAllocator
 *
 * \brief This class provides Id Allocation logic. It is
 *        designed to be generic
 *
 * The Id Allocator is responsible for managing Ids in the
 * system. It is used by multiple managers to provide this
 * service.
 *
 * The Id Allocator has the ability to manage Ids in a set of
 * ranges.
 *
 * There are some assumptions:
 *  - Range 0 starts from the lowest ID allowed (for example: 0)
 *  - Range 0 Ids < Range 1 Ids < Range 2 Ids < ... < Range N
 *    Ids
 *  - Maximum IDs represents the maximum number of IDs managed
 *    by the allocator
 *
 */
class xpsIdAllocator
{
public:
    xpsIdAllocator();

    xpsIdAllocator(uint32_t maxIds, uint32_t numRanges);

    virtual ~xpsIdAllocator();

    /**
     * \public
     * \brief Iniitalize the allocator class, assuming required
     *        parameters are passed in upon construction
     *
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       init();

    /**
     * \public
     * \brief Initialize the allocator class, assuming the class was
     *        constructed without passing in paramters
     *
     * This init routine will be used for "deferred" initalization.
     * Such as the case where the ranges or maximum ID value is not
     * known upon construction
     *
     *
     * \param [in] maxIds
     * \param [in] numRanges
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       init(uint32_t maxIds, uint32_t numRanges);

    /**
     * \public
     * \brief set 1s for all the allocator IDs in the range
     *
     * \param [in] range
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       setAllocatorIdsInRange(uint32_t range);

    /**
     * \public
     * \brief Will assign id's to a particular range
     *
     * This will need to be called after initialization on a per
     * range basis
     *
     *
     * \param [in] instanceId
     * \param [in] range
     * \param [in] depth
     * \param [in] start
     * \param [in] unique_OID)
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       configureRange(uint32_t instanceId, uint32_t range,
                                           uint32_t depth, uint32_t start, uint64_t unique_OID);

    /**
     * \fn getMaxIdForAllocator
     * \public
     * \brief Get the max Ids of the Allocator
     *
     * Assumption is currently that the full ID space is reserved to
     * a single range
     *
     * \param [out] uint32_t& maxId
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       getMaxIdForAllocator(uint32_t& maxId);

    /**
     * \fn allocateId
     * \public
     * \brief Allocates an ID over the full ID space
     *
     * Assumption is currently that the full ID space is reserved to
     * a single range
     *
     * \param [out] id
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       allocateId(uint32_t& id);

    /**
     * \public
     * \brief Allocates an ID specified by the user over the full ID
     *        space
     *
     * Assumption is currently that the full ID space is reserved to
     * a single range
     *
     * \param [in] id
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       allocateWithId(uint32_t id);

    /**
      * \public
      * \brief Prints the allocator Id bitvector
      *
      * \return XP_STATUS
      */
    virtual XP_STATUS dumpAllocator();

    /**
     * \public
     * \brief Allocates an ID from a passed in range
     *
     * Will allocate a single ID from one of possibly many ranges.
     * It will pick the first available ID in a given range and
     * return it
     *
     *
     * \param [in] range
     * \param [out] id
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       allocateIdFromRange(uint32_t range, uint32_t& id);


    /**
     * \public
     * \brief Allocates an Last Valid ID from a passed in range
     *
     * Will allocate a single ID from one of possibly many ranges.
     * It will pick the last available ID in a given range and
     * return it
     *
     *
     * \param [in] range
     * \param [out] id
     *
     * \return XP_STATUS
     */


    virtual XP_STATUS allocateIdFromRangeFromLast(uint32_t range, uint32_t& id);

    /**
     * \public
     * \brief Allocates an ID specificed by the user from a passed
     *        in range
     *
     * Will allocate a single ID from one of possibly many ranges.
     * As long as the ID passed in by the user also belongs to the
     * range passed in by the user. If not, this API will return an
     * error. It is recommended that the caller of this function
     * check against errors
     *
     *
     * \param [in] range
     * \param [in] id
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       allocateWithIdFromRange(uint32_t range, uint32_t id);

    /**
     * \public
     * \brief Allocates a group of IDs inside a specified range of a
     *        specific size
     *
     * This API differs from reserveIdBlockFromRanges in that it
     * will search for an ID group of a specified size within the
     * unallocated IDs of a range. Upon success, the starting ID
     * allocated for this group will be returned.
     *
     * The user does not control which IDs to allocate using this
     * mode, it is determined dynamically
     *
     * \param [in] range
     * \param [in] size
     * \param [out] id
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       allocateIdGroupFromRange(uint32_t range, uint32_t size,
                                                     uint32_t& id);

    /**
     * \public
     * \brief Reserve a block of IDs inside a specified range of a
     *        specific size
     *
     * This API differs from allocateIDGroupFromRange in that it
     * expects the user to specify which IDs they wish to reserve
     *
     * This API facilitates a "batch" mode of ID allocation,
     * allowing the user to maintain their own ID management schemes
     * apart from the XDK
     *
     * We define a block as a "subrange"; a continuous set of IDs
     * within the specified range
     *
     * First is an ID that should belong to the specified range, and
     * is used as the first ID to be allocated when reserving.
     *
     * It is assumed that the caller of this API understands how the
     * IDs ranges have been configured if an ID will belong to the
     * selected range. This API will validate the first and size
     * parameters before reserving the block. Caller should check
     * against an error being returned
     *
     * \param [in] range
     * \param [in] first
     * \param [in] size
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       reserveIdBlockFromRange(uint32_t range, uint32_t first,
                                                    uint32_t size);

    /**
     * \public
     * \brief Release an allocated ID back to its corresponding pool
     *
     *
     * \param [in] id
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       releaseId(uint32_t id);

    /**
     * \public
     * \brief Release an allocated ID back to the range specified
     *
     * The ID passed in must fall into the specified range
     *
     *
     * \param [in] range
     * \param [in] id
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       releaseIdFromRange(uint32_t range, uint32_t id);

    /**
     * \public
     * \brief Release a group of IDs from a specified range
     *
     *
     * \param [in] range
     * \param [in] size
     * \param [in] id
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       releaseIdGroupFromRange(uint32_t range, uint32_t size,
                                                    uint32_t id);

    /**
     * \public
     * \brief Returns the range the specified ID falls into
     *
     *
     * \param [in] id
     * \param [out] range
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       getRangeFromId(uint32_t id, uint32_t& range);

    /**
     * \public
     * \brief Returns the start offset of a range
     *
     *
     * \param [in] range
     * \param [out] start
     *
     * \return XP_STATUS
     */
    virtual XP_STATUS       getRangeStartOffset(uint32_t range, uint32_t& start);

    /**
     * \public
     * \brief checks if there are allocated ids from the range
     *
     *
     * \param [in] range
     *
     * \return 1 - all ids in range are not allocated
     *         0 - some id from the range is already allocated
     */
    virtual uint8_t         isRangeFree(uint32_t range);


    /**
     * \public
     * \brief Copies the context data into a new context passed in
     *        from the user
     *
     * This method allows a user to copy the context state over into
     * a new context. This will be used in multi process scenarios
     *
     * \param [out] newCtx
     *
     * \return XP_STATUS
     */
    XP_STATUS               copyContext(xpsIdAllocatorCtx& newCtx);

    /**
     * \public
     * \brief Accessor for context pointer
     *
     * Returns a pointer to the allocator context. Can be used in a
     * single process scenario
     *
     * \return xpsIdAllocatorCtx*
     */
    xpsIdAllocatorCtx*   getContext()
    {
        return context;
    }

    /*
     * Static service routines
     */

    /**
     * \public
     * \brief Static routine which expects an IdAllocator Context to
     *        return the range for a given ID
     *
     *
     * \param [in] ctx
     * \param [in] id
     * \param [out] range
     *
     * \return XP_STATUS
     */
    static XP_STATUS getRangeFromId(xpsIdAllocatorCtx& ctx, uint32_t id,
                                    uint32_t& range);

    /**
     * \public
     * \brief Static routine which expects and IdAllocator Context
     *        to return the start offset of a range
     *
     *
     * \param [in] ctx
     * \param [in] range
     * \param [out] start
     *
     * \return XP_STATUS
     */
    static XP_STATUS getRangeStartOffset(xpsIdAllocatorCtx& ctx, uint32_t range,
                                         uint32_t& start);

protected:

    /**
     * \protected
     * \brief Factory Method used to create a context if one is not
     *        already created
     *
     * Is used for "deferred" initialization, in the case that the
     * parameters required for ID Allocation are not known upon time
     * of construction
     *
     *
     * \return xpsIdAllocatorCtx*
     */
    virtual xpsIdAllocatorCtx* createContext()
    {
        return ((context == NULL) ? new xpsIdAllocatorCtx() : context);
    }

    /**
     * \protected
     * \brief Factory Method used to create a context if one is not
     *        already created
     *
     * Is used when parameters are passed during construction
     *
     *
     * \return xpsIdAllocatorCtx*
     */
    virtual xpsIdAllocatorCtx* createContext(uint32_t maxIds, uint32_t numRanges)
    {
        return ((context == NULL) ? new xpsIdAllocatorCtx(maxIds,
                                                          numRanges) : context);
    }

    /**
     * \protected
     * \brief Id Range Vector
     *
     * Contains a bit mask corresponding to each ID in each range
     * indicating whether or not that ID is allocated
     *
     * A value of 0 means that the corresponding ID has been
     * allocated, a value of 1 means that ID is free
     *
     */
    std::vector<xpsPersistBitVector>            idRanges;

    /**
     * \protected
     * \brief Context Object create by factory method
     *
     */
    xpsIdAllocatorCtx*               context;

private:
    /**
     * \private
     * \brief Lock to manage Id Allocator
     *
     */
    xpsGenLock* xpsIdAllocatorLock;

    /**
     * \private
     * \brief Lock Id to manage Id Allocator
     *
     */
    uint32_t lockId;

};

#endif // _xpsIdAllocator_h_

