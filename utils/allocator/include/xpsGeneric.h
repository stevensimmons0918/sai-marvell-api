// xpsGeneric.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef XP_GENERIC_H_
#define XP_GENERIC_H_

#include <cstddef>
#include <algorithm>

namespace Generic
{

template<typename T, size_t SIZE = 8>
class StackObjAllocator
{
    bool m_dynamic;
    size_t m_size;

    T* m_buf;
    T m_buffer[SIZE];

public:
    StackObjAllocator(size_t size)
        : m_dynamic(false), m_size(size), m_buf(m_buffer)
    {
        if (size > SIZE)
        {
            m_buf = new T [size];
            m_dynamic = true;
        }
    }

    ~StackObjAllocator()
    {
        if (m_dynamic)
        {
            delete [] m_buf;
        }
    }

    StackObjAllocator(const StackObjAllocator& other)
        : m_dynamic(other.m_dynamic), m_size(other.m_size)
    {
        if (m_dynamic)
        {
            m_buf = new T[m_size];
        }
        else
        {
            m_buf = m_buffer;
        }

        std::copy(other.m_buf, other.m_buf + m_size, m_buf);
    }

    void resize(size_t size)
    {
        if ((size > SIZE) && (size > m_size))
        {
            T* old = m_buf;

            m_buf = new T[size];
            std::copy(old, old + m_size, m_buf);

            if (m_dynamic)
            {
                delete [] old;
            }

            m_dynamic = true;
        }

        m_size = size;
    }

    T& operator[](size_t i)
    {
        return m_buf[i];
    }

    operator void*()
    {
        return static_cast<void*>(m_buf);
    }

    operator T*()
    {
        return static_cast<T*>(m_buf);
    }
};

//======================================
// number of 1's in a uint32_t
//======================================
inline unsigned count1s(uint32_t n)
{

    // in each step, we divide n into fixed-width blocks and treat each block separately
    // put count of 1's in each block into that block

    // the basic idea is to add together two halves which are assumed to be done already,
    // however, in each step this formula is optimized for that step

    // once we have done 8-bit blocks, there is no more need to proceed with 16-bit blocks
    // because at that point there is a 1-step formula to get the final result

    // do 2-bit blocks
    n -= n >> 1 & 0x55555555U;

    // do 4-bit blocks assuming 2-bit blocks are done
    n = (n & 0x33333333U) + ((n >> 2) & 0x33333333U);

    // do 8-bit blocks assuming 4-bit blocks are done
    n = (n + (n >> 4)) & 0x0F0F0F0FU;

    // final formula
    // add the 4 8-bit block counts together
    return n * 0x01010101U >> 24;
}

//======================================
// number of 1's in a uint64_t
//======================================
inline unsigned count1s(uint64_t n)
{
    return count1s(static_cast<uint32_t>(n >> 32))
           + count1s(static_cast<uint32_t>(n & 0xFFFFFFFF));
}

//======================================
// index of the first 1 bit, or 32 if none
// assuming int is at least 32 bits
//======================================
inline unsigned find_first(uint32_t x)
{

    // accumulation of bit index after all 0 on the right;
    unsigned ret = 0;

    if (x & 1U)
    {
        return 0U;
    }

    // binary search for the first 1

    if (!(x & ((1U << 16) - 1)))
    {
        x >>= 16;
        ret |= 16U;
    }

    if (!(x & ((1U << 8) - 1)))
    {
        x >>= 8;
        ret |= 8U;
    }

    if (!(x & ((1U << 4) - 1)))
    {
        x >>= 4;
        ret |= 4U;
    }

    if (!(x & ((1U << 2) - 1)))
    {
        x >>= 2;
        ret |= 2U;
    }

    if (x & 0x1U)
    {
        return ret;
    }

    if (x & 0x2U)
    {
        return ret |= 1U;
    }

    return 32;
}

//======================================
// index of the last 1 bit, or 32 if none
// assuming int is at least 32 bits
//======================================
inline unsigned find_last(uint32_t x)
{

    // accumulation of bit index after all 0 on the left;
    unsigned ret = 0;

    if (x & 0xFFFF0000)
    {
        x >>= 16;
        ret |= 16U;
    }

    if (x & 0xFF00)
    {
        x >>= 8;
        ret |= 8U;
    }

    if (x & 0xF0)
    {
        x >>= 4;
        ret |= 4U;
    }

    if (x & 0xC)
    {
        x >>= 2;
        ret |= 2U;
    }

    if (x & 0x2)
    {
        return ret |= 1U;
    }

    if (x & 0x1)
    {
        return ret;
    }

    return 32;
}

}
#endif /* XP_GENERIC_H_ */

