// xpsIdAllocatorCtx.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpsIdAllocatorCtx_h_
#define _xpsIdAllocatorCtx_h_

#include "xpTypes.h"
#include "xpsPersistBitVector.h"

class xpsIdAllocatorCtx
{
public:
    // Constructors

    /**
     * \public
     * \brief Default Constructor Implementation
     *
     * Use this constructor when requiring "deferred" initialization
     */
    xpsIdAllocatorCtx();

    /**
     * \public
     * \brief Parameterized Constructor Implementation
     *
     * Use this constructor when initializing during construction
     *
     * \param [in] maxIds
     * \param [in] numRanges
     */
    xpsIdAllocatorCtx(uint32_t maxIds, uint32_t numRanges);

    // Copy constructor

    /**
     * \public
     * \brief Copy Constructor Implementation
     *
     * Performs a deep copy of context information
     *
     * \param [in] ctx
     */
    xpsIdAllocatorCtx(const xpsIdAllocatorCtx& ctx);

    // Destructor

    /**
     * \public
     * \brief Default Destructor Implementation
     *
     * Cleans up all private members
     *
     */
    ~xpsIdAllocatorCtx();

    /**
     * \public
     * \brief Iniitalize the allocator context class, assuming
     *        required parameters are passed in upon construction
     *
     *
     * \return XP_STATUS
     */
    XP_STATUS init();

    /**
     * \public
     * \brief Initialize the allocator context class, assuming the
     *        class was constructed without passing in parameters
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
    XP_STATUS init(uint32_t maxIds, uint32_t numRanges);

    /**
     * \public
     * \brief Sets the start offset of a specified range
     *
     * This routine will store the start offset of a specified range
     *
     * \param [in] range
     * \param [in] start
     *
     * \return XP_STATUS
     */
    XP_STATUS setRangeStartOffset(uint32_t range, uint32_t start);

    /*
     * Accessor routines
     */

    /**
     * \public
     * \brief Accessor routine for Range Start Vector
     *
     * Returns a pointer ot a vector containing the Range Start
     * Offset information
     *
     *
     * \return std::vector<uint32_t>*
     */
    std::vector<uint32_t>*    getRangeStarts()
    {
        return &rangeStarts;
    }

    /**
     * \public
     * \brief Accessor routine for Max Ids
     *
     * Returns the Maximum ID value managed by the Id Allocator
     *
     * \return uint32_t
     */
    uint32_t                  getMaxIds()
    {
        return maxIds;
    }

    /**
     * \public
     * \brief Accessor routine for Number of Ranges
     *
     * Returns the number of ranges managed by the Id Allocator
     *
     * \return uint32_t
     */
    uint32_t                  getNumRanges()
    {
        return numRanges;
    }

    /**
     * \public
     * \brief Assignment Operator Overload
     *
     * Used to assign data from one Allocator Context to the current
     * one
     *
     * \param [in] ctx
     *
     * \return xpsIdAllocatorCtx&
     */
    xpsIdAllocatorCtx& operator=(xpsIdAllocatorCtx ctx);


private:

    /**
     * \private
     * \brief Vector containing the range offset information
     *
     * This vector is populated with the start offset of each range
     * managed by an Id Allocator
     *
     */
    std::vector<uint32_t>               rangeStarts;

    /**
     * \private
     * \brief Maximum number of IDs managed by the Id Allocator
     *
     */
    uint32_t                            maxIds;

    /**
     * \private
     * \brief Number of ranges in id allocation pool
     *
     * 1-based number, this should never be 0
     *
     */
    uint32_t                            numRanges;

};




#endif // _xpsIdAllocatorCtx_h_

