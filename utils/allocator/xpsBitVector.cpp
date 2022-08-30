// xpsBitVector.cpp

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <stdlib.h>

#ifndef NDEBUG
#include <assert.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include "xpsBitVector.h"
#include "xpsCommon.h"

#ifdef MARVELL_PORT //MARVELL_PORT
XP_SAI_LOG_REGISTER_API(SAI_API_UNSPECIFIED);
#endif //MARVELL_PORT
typedef Generic::StackObjAllocator<uint16_t, 12> MemUInt16;
typedef Generic::StackObjAllocator<uint32_t, 6> MemUInt32;
typedef Generic::StackObjAllocator<uint64_t, 3> MemUInt64;

//======================================
// for argument token op
// setup bitwise operator<op> and operator<op>=
// for the class xpsBitVector
//
// requirement is that
// 0 op 0 is 0
//======================================
#define SETUP_BITWISE_OPERATORS(op)     \
xpsBitVector xpsBitVector::operator op(const xpsBitVector& other) const    \
{   \
    /* pointers to data of result and operands*/    \
    unsigned *data, *data1, *data2;     \
        \
    /* return this*/    \
        \
    /*pointer to last integer of data*/     \
    unsigned *pLastInt = mData + mLastIntIndex;     \
        \
    xpsBitVector ret(*this);     \
    for (data = ret.mData, data1 = mData, data2 = other.mData;  \
            data1 <= pLastInt;  \
            data++, data1++, data2++)   \
            *data = *data1 op *data2;   \
                \
    return ret;     \
}   \
        \
xpsBitVector& xpsBitVector::operator op##=(const xpsBitVector& other)   \
{   \
    /* pointers to data of result and operand*/     \
    unsigned *data, *data1;     \
        \
    bool isThisBv = this->size() <= other.size();   \
        \
    /* pointer to last integer of data*/    \
    unsigned * const pLastInt = ( isThisBv ? (mData + mLastIntIndex) : (other.mData + other.mLastIntIndex));    \
    unsigned *pItData = (isThisBv ? mData : other.mData);   \
        \
    /* only perform operation on the smaller bitvector otherwise the this may access out of bounds memory for data or data1 */ \
    for (data = mData, data1 = other.mData; pItData <= pLastInt; data++, data1++, pItData++)  \
        *data op##= *data1; \
                \
    return *this;   \
}


xpsBitVector xpsBitVector::operator +(const xpsBitVector& other) const
{
    /* pointers to data of result and operands*/
    unsigned *data, *data1, *data2;
    uint64_t nvalue = 0;

    /* return this*/

    /*pointer to last integer of data*/
    unsigned *pLastInt = mData + mLastIntIndex;

    xpsBitVector ret(*this);
    for (data = ret.mData, data1 = mData, data2 = other.mData;
         data1 <= pLastInt;
         data++, data1++, data2++)
    {
        nvalue = *data1 + *data2 + nvalue;
        *data = (nvalue & 0xffffffff);
        nvalue = nvalue >> 32;
    }

    return ret;
}

xpsBitVector& xpsBitVector::operator +=(const xpsBitVector& other)
{
    /* pointers to data of result and operand*/
    unsigned *data, *data1;
    uint64_t nvalue = 0;

    bool isThisBv = this->size() <= other.size();

    /* pointer to last integer of data*/
    unsigned * const pLastInt = (isThisBv ? (mData + mLastIntIndex) :
                                 (other.mData + other.mLastIntIndex));
    unsigned *pItData = (isThisBv ? mData : other.mData);

    /* only perform operation on the smaller bitvector otherwise the this may access out of bounds memory for data or data1 */
    for (data = mData, data1 = other.mData; pItData <= pLastInt;
         data++, data1++, pItData++)
    {
        nvalue = *data + *data1 + nvalue;
        *data = (nvalue & 0xffffffff);
        nvalue = nvalue >> 32;
    }
    return *this;
}

void xpsBitVector::init(unsigned bits)
{
    if (!bits)
    {
        xpThrow((std::range_error(_FILE_LINE)));
    }

    mBits = bits;

    /* Always %32 because storage type of mData is uint32_t
     * Also, we don't have to take care of bits == 0 because
     * it is already handled in the caller*/
    mLastIntBits = (bits % 32);
    mLastIntIndex = (bits / 32);
    /* always allocate mBytes in multiples of 4 */
    mBytes = (((mBits + 31) >> 5) << 2);
    mLastIntMask = LOWER_BITS(mLastIntBits);

    mStorage.resize(mLastIntIndex + 1);
    mData = mStorage;

    assert(mBits == (mLastIntIndex * INT_BITS + mLastIntBits));
}

xpsBitVector::xpsBitVector(unsigned destBits, uint32_t *src, unsigned srcBits)
    : mStorage(0)
{
    uint32_t i, bits2cpy = 0;
    uint32_t tmp, msk0_31;

    if ((destBits == 0) || (srcBits == 0) || (destBits < srcBits) || (src == NULL))
    {
        xpThrow((std::range_error(_FILE_LINE)));
    }

    init(destBits);

    for (i = 0; i < (mBytes / 4); i++)
    {
        bits2cpy = (srcBits > 32) ? 32 : srcBits;
        msk0_31 = ((1ull << bits2cpy) - 1);
        tmp = htole32(src[i] & msk0_31);
        memcpy(mData + i, &tmp, 4);
        srcBits -= bits2cpy;
    }
}

uint32_t xpsBitVector::get1mask_u32host(uint32_t hi, uint32_t lo)
{
    /* returns 1's bitmask between hi and lo bit positions */
    uint32_t mhi, mlo, ret;

    if (hi == 32)
    {
        mhi = ~0l;
        mlo = ~((1 << lo) - 1);
    }
    else
    {
        mhi = ((1 << hi) - 1);
        mlo = ~((1 << lo) - 1);
    }
    ret = (mhi & mlo);
    return ret;
}

/*
 * u64p : source pointer
 * u32p : destination pointer
 * bit2copy : #bits to copy, can be greater than 64
 */
uint32_t xpsBitVector::convert_u64_to_u32le(uint32_t *u32p, void *u64p,
                                            uint32_t bits2cpy)
{
    /* Caller must guarantee u32p has at least 2 elements */
    uint32_t bits_cpd, *u32tmp, msk0_31, msk32_63;

    if (bits2cpy == 0)
    {
        return 0;
    }

    if ((bits2cpy % 64) == 0)
    {
        /* A multiple of 64 */
        msk0_31 = msk32_63 = 0xffffffff;
        bits_cpd = 64;
    }
    else
    {
        /* a number between 0 - 63; craft masks */
        msk0_31 = (((0x1ull << bits2cpy) - 1) & 0xffffffff);
        msk32_63 = ((((0x1ull << bits2cpy) - 1) >> 32) & 0xffffffff);
        bits_cpd = bits2cpy % 64;
    }
    u32tmp = (uint32_t *)u64p;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    u32p[0] = (u32tmp[0] & msk0_31);
    u32p[1] = (u32tmp[1] & msk32_63);
#else
    u32p[1] = htole32(u32tmp[0] & msk0_31);
    u32p[0] = htole32(u32tmp[1] & msk32_63);
#endif
    return bits_cpd;
}

xpsBitVector::xpsBitVector(unsigned destBits, uint64_t *src, unsigned srcBits)
    : mStorage(0)
{
    uint32_t i, bits_cpd = 0;
    uint32_t tmp[2];

    if ((destBits == 0) || (srcBits == 0) || (destBits < srcBits) || (src == NULL))
    {
        xpThrow((std::range_error(_FILE_LINE)));
    }

    init(destBits);

    for (i = 0; i < (mBytes / 8); i++)
    {
        memset(tmp, 0, 8);
        bits_cpd = convert_u64_to_u32le(tmp, (void *)(src + i), mBits - bits_cpd);
        memcpy(mData + (i * 2), tmp, 8);    /* mData is uint32_t* */
    }
}

xpsBitVector::xpsBitVector(const xpsBitVector &other): mStorage(other.mStorage),
    mData(mStorage),
    mLastIntIndex(other.mLastIntIndex),
    mLastIntBits(other.mLastIntBits),
    mLastIntMask(other.mLastIntMask),
    mBits(other.mBits),
    mBytes(other.mBytes)
{
}

xpsBitVector::xpsBitVector(const xpsBitVector_range_ref& range) : mStorage(0),
    mData(NULL),
    mLastIntIndex(0),
    mLastIntBits(0),
    mLastIntMask(0),
    mBits(0),
    mBytes(0)
{
    init(range.mBits);
    mData[mLastIntIndex] = 0;
    cpBits(
        mData,
        0, mBits - 1,
        range.mpObj->mData + (range.mLo >> BIT_INT_SHIFT),
        range.mLo & INT_BIT_MASK);
}


xpsBitVector& xpsBitVector::operator= (const xpsBitVector &other)
{
    if (other.mBits != mBits)
    {
        xpThrow((std::range_error(_FILE_LINE)));
    }
    if (&other != this)
    {
        /* get to word boundary */
        memcpy(mData, other.mData, mBytes);
    }
    return *this;
}


xpsBitVector::xpsBitVector() :
    mStorage(0), mData(NULL), mLastIntIndex(0), mLastIntBits(0), mLastIntMask(0),
    mBits(0), mBytes(0)
{

}

//======================================
//! copy data starting from  a uint8_t * to inside our array starting at bit lo
//
/// \param src copy from that
//
/// \param bits number of bits to copy
//
/// \param lo starting bit to copy to
//
/// xpThrow((std::range_error(<this file name and line>))) if our vector
/// does not have enough space starting at lo
//
//======================================
void xpsBitVector::setDataToRange(unsigned lo, unsigned bits,
                                  const uint8_t *src)
{
    if (bits == 0)
    {
        return;
    }

    if (mBits < (bits + lo))
    {
        xpThrow((std::range_error(_FILE_LINE)));
    }

    /* uint8_t* means Little Endian in any architecture
     *     0        1        2       3         0       1       3...
     * 76543210_76543210_76543210_76543210
     */
    uint32_t src_bytes = (bits + 7) / 8;
    uint32_t src_u32s = (src_bytes + 3) / 4;

    MemUInt32 src_arr(src_u32s);
    MemUInt32 dst_arr(mBytes / 4);

    memset(dst_arr, 0, mBytes);
    memset(src_arr, 0, src_u32s * 4);

    memcpy(src_arr, src, src_bytes);    /* src_arr in LE in any arch */
    memcpy(dst_arr, mData,
           mBytes);    /* dst_arr is also in LE because mData in LE */
    lshift_u32le(dst_arr, mBytes, src_arr, bits, lo);
    memcpy(mData, dst_arr, mBytes);
}

int xpsBitVector::rshift_u32le(uint32_t *hdst, uint32_t d32s,
                               const uint32_t *hsrc, uint32_t sbits, uint32_t rshift)
{
    uint32_t hi32, lo32, sidx, didx, s32e, hibit, msk;
    uint32_t x = (rshift %
                  32); /* bit# to place the first bit , from right, irrespective of endianness */
    uint32_t cpd = 0;

    s32e = (sbits + rshift + 31)/32;
    /* sanity checks */
    if ((d32s == 0) || (sbits == 0) || (d32s < ((sbits + 31)/32)))
    {
        // printf("d32s = %d, s32s = %d, loops = %d, rshift %d. d32s must be >= loops\n",
        //      d32s, s32s, loops, rshift);
        return -1;
    }

    /* dynamically calculate masks every iteration */
    /* every u32 should be split into two : hi32 and lo32 */
    didx = 0;
    for (sidx = (rshift / 32); cpd < sbits; sidx++, didx++)
    {
        hi32 = lo32 = 0;

        lo32 = (le32toh(hsrc[sidx]) >> x);
        /* now we have (32 - x) bits out of 32 bits. Decide how many to keep */
        if ((32 - x) < (sbits - cpd))
        {
            hibit = 32 - x;
        }
        else
        {
            hibit = sbits -
                    cpd;    /* true only for the last (first could be last) iteration */
        }
        msk = get1mask_u32host(hibit, 0);
        lo32 &= msk;        /* zero out the upper bits above hibit */
        hdst[didx] &= htole32(
                          ~msk);    /* zero out the lower bits till hibit of destination */
        hdst[didx] |= htole32(lo32);
        cpd += hibit;

        if (((sidx + 1) < s32e) && (cpd != sbits))
        {
            /* copy a max of x bits , till newly calulated hibit starting (32 - x) bit */
            if (x > (sbits - cpd))
            {
                hibit = (sbits - cpd);
            }
            else
            {
                hibit = x;
            }
            msk = get1mask_u32host(hibit, 0);
            hi32 = (le32toh(hsrc[sidx + 1]) & (msk));
            hdst[didx] &= htole32(~(msk << (32 - x)));
            hdst[didx] |= htole32((hi32 << (32 -x)));
            cpd += hibit;
        }
    }
    return 0;
}

/* both destination and src in host byte order
 * Caller ensures hrsc is is at least u32s + (lshift/32) + 1
 */
int xpsBitVector::lshift_u32le(uint32_t *hdst, uint32_t d32s,
                               const uint32_t *hsrc, uint32_t sbits, uint32_t lshift)
{
    uint32_t hi32, lo32, sidx = 0, didx = 0, s32e, hibit, msk;
    uint32_t x = (lshift % 32);
    uint32_t cpd = 0;

    s32e = (sbits + 31)/32;
    /* sanity checks */
    if ((d32s == 0) || (sbits == 0) || (d32s < s32e))
    {
        // printf("d32s = %d, s32s = %d, loops = %d, rshift %d. d32s must be >= loops\n",
        //      d32s, s32s, loops, rshift);
        return -1;
    }

    /* dynamically calculate masks every iteration */
    /* every u32 should be split into two : hi32 and lo32 */
    didx = 0;
    for (didx = (lshift / 32); cpd < sbits; sidx++, didx++)
    {
        hi32 = lo32 = 0;

        lo32 = (le32toh(hsrc[sidx]) << x);
        if ((sbits - cpd) > (32 - x))
        {
            hibit = 32;
        }
        else
        {
            hibit = x + sbits - cpd;    /* bits x through x+sbits-cpd */
        }
        msk = get1mask_u32host(hibit, x);
        lo32 &= msk;
        hdst[didx] &= htole32(~(msk));
        hdst[didx] |= htole32(lo32);
        cpd += (hibit - x);

        if (((didx + 1) < d32s) && (cpd != sbits))
        {
            /* copy a max of x bits to [didx + 1]*/
            if ((sbits - cpd) < x)
            {
                hibit = (sbits - cpd);
            }
            else
            {
                hibit = x;
            }
            hi32 = (le32toh(hsrc[sidx]) >> (32 - x));
            msk = get1mask_u32host(hibit, 0);
            hi32 &= msk;
            hdst[didx + 1] &= htole32(~msk);
            hdst[didx + 1] |= htole32(hi32);
            cpd += hibit;
        }
    }
    return 0;
}

void xpsBitVector::setDataToRange(uint32_t lo, uint32_t bits,
                                  const uint64_t *src)
{
    if (mBits < (bits + lo))
    {
        xpThrow((range_error(_FILE_LINE)));
    }

    if (bits == 0)
    {
        return;
    }

    uint32_t src_elem = ((bits + 63) / 64);

    MemUInt32 src_arr(src_elem * 2);
    MemUInt32 dst_arr(mBytes / 4);

    memset(src_arr, 0, src_elem * 8);
    memcpy(dst_arr, mData, mBytes);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    memcpy(src_arr, src, src_elem * 8);
#else

    for (uint32_t i = 0; i < src_elem; i++)
    {
        src_arr[2 * i] = htole32(src[i] & 0xffffffff);
        src_arr[(2 *i)+1] = htole32((src[i] >> 32) & 0xffffffff);
    }
#endif

    if (lshift_u32le(dst_arr, mBytes/4, src_arr, bits, lo) != 0)
    {
        xpThrow((range_error(_FILE_LINE)));
    }

    memcpy(mData, dst_arr, mBytes);
}

void xpsBitVector::setDataToRange(uint32_t lo, uint32_t bits,
                                  const uint32_t *src)
{
    if (bits == 0)
    {
        return;
    }

    if (mBits < (bits + lo))
    {
        xpThrow((range_error(_FILE_LINE)));
    }

    uint32_t src_elem = (bits + 31) / 32;

    MemUInt32 src_arr(src_elem);
    MemUInt32 dst_arr(mBytes / 4);

    memset(src_arr, 0, src_elem * 4);
    memcpy(dst_arr, mData, mBytes);

    for (uint32_t i = 0; i < src_elem; i++)
    {
        src_arr[i] = htole32(src[i]);
    }

    if (lshift_u32le(dst_arr, mBytes/4, src_arr, bits, lo) != 0)
    {
        xpThrow((range_error(_FILE_LINE)));
    }

    memcpy(mData, dst_arr, mBytes);
}

void xpsBitVector::setDataToRange(uint32_t lo, uint32_t bits,
                                  const uint16_t *src)
{
    if (bits == 0)
    {
        return;
    }

    if (mBits < (bits + lo))
    {
        xpThrow((range_error(_FILE_LINE)));
    }

    uint32_t src_elem = (bits + 15) / 16;
    uint32_t tgt_u32s = (src_elem + 1) / 2;

    MemUInt32 src_arr(tgt_u32s);
    MemUInt32 dst_arr(mBytes / 4);

    memset(src_arr, 0, tgt_u32s * 4);
    memcpy(dst_arr, mData, mBytes);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    memcpy(src_arr, src, src_elem * 2);
#else
    uint32_t idx;
    for (idx = 0; idx < src_elem; idx += 2)
    {
        /* We still need this gymnastics because typecasting to uint32_t* and
         * dererencing may cross allocated byte boundaries in src */
        src_arr[idx/2] |= (((uint32_t)htole16(src[idx + 0])) << 16);
        if ((src_elem - idx) >= 2)
        {
            src_arr[idx/2] |= (((uint32_t)htole16(src[idx + 1])) << 0);
        }
    }
#endif

    lshift_u32le(dst_arr, mBytes/4, src_arr, bits, lo);
    memcpy(mData, dst_arr, mBytes);
}

xpsBitVector& xpsBitVector::operator=(uint32_t val)
{
    uint32_t tmp;

    if (mBits <= 32)
    {
        if (val >= (1ull << mBits))
        {
            /* must have allocated at least 2 * uint32_t (64 bits) */
            //printf("u64 Allocated bits (%d) not sufficient to hold val(%#llx)\n", mBits, val);
            xpThrow((range_error(_FILE_LINE)));
        }
        else
        {
            /* No need to test the size. It is well within the limit */
        }
    }
    memset(mData, 0, mBytes);
    tmp = htole32(val);
    memcpy(mData, &tmp, sizeof(uint32_t));
    return *this;
}

xpsBitVector& xpsBitVector::operator= (int val)
{
    return operator=((uint32_t) val);
}

xpsBitVector& xpsBitVector::operator=(uint16_t val)
{
    uint32_t tmp = (uint32_t) val;
    if (mBits <= 16)
    {
        if (tmp > (1ul << mBits))
        {
            // printf("u16 allocated bits (%d) not sufficient to hold val(%#lx)\n", mBits, val);
            xpThrow((range_error(_FILE_LINE)));
        }
    }
    return operator=(tmp);
}

xpsBitVector& xpsBitVector::operator=(uint8_t val)
{
    uint32_t tmp = (uint32_t) val;
    if (mBits <= 8)
    {
        if (tmp > (1ul << mBits))
        {
            // printf("u16 allocated bits (%d) not sufficient to hold val(%#lx)\n", mBits, val);
            xpThrow((range_error(_FILE_LINE)));
        }
    }
    return operator=(tmp);
}

xpsBitVector& xpsBitVector::operator=(uint64_t val)
{

    memset(mData, 0, mBytes);
    if (mBits < 64)
    {
        if (val >= (1ull << mBits))
        {
            /* must have allocated at least 2 * uint32_t (64 bits) */
            //printf("u64 Allocated bits (%d) not sufficient to hold val(%#llx)\n", mBits, val);
            xpThrow((range_error(_FILE_LINE)));
        }
        else
        {
            /* No need to test the size. It is well within the limit */
        }
    }
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    memcpy(mData, &val, sizeof(uint64_t));
#else
    /* This is a hack; albeit a decent one
     * We need hi32 and lo32 separated then copied
     */
    uint32_t tmp0_31, tmp32_63;

    tmp0_31 = htole32(val & ((1ull << 32) - 1));
    tmp32_63 = htole32((val >> 32) & ((1ull << 32) - 1));
    memcpy(mData, &tmp0_31, sizeof(uint32_t));
    memcpy(mData + 1, &tmp32_63,
           sizeof(uint32_t)); /* mData is uint32_t, +1 is good */
#endif
    return *this;
}

void xpsBitVector::flipTo(unsigned *dest) const
{
    // pointer to current integer
    unsigned *p = mData;
    // pointer to last integer
    unsigned * const pLastInt = mData + mLastIntIndex;
    uint32_t lastInt;

    for (; p < pLastInt; p++, dest++)
    {
        *dest = ~*p;
    }
    /* write_mask_bits expects the second parameter to be a constant
     * in host byte order. So, we pass it in host byte order */
    lastInt = le32toh(*p);
    WRITE_MASK_BITS(dest, ~(lastInt), mLastIntMask);
}

xpsBitVector xpsBitVector::operator~() const
{
    xpsBitVector ret(*this);
    ret.mData[mLastIntIndex] = 0;
    flipTo(ret.mData);
    return ret;
}


xpsBitVector xpsBitVector::operator<<(unsigned shift) const
{
    const int bitsMinus1 = mBits - 1;

    // index of the last shifted bit
    // meaningful only if >=0
    const int lastShiftBit = bitsMinus1 - (int)shift;

    // end of starting integers covered with 0 for sure
    unsigned *endZeroCovers;

    // bit data to copy to
    unsigned *data;

    // return this
    xpsBitVector ret(*this);

    if (lastShiftBit < 0)
    {
        ret.reset();
    }
    else
    {
        data = ret.mData;
        endZeroCovers = data + N_INT_COVERS(shift);
        while (data < endZeroCovers)
        {
            *data++ = 0;
        }
        ret.mData[mLastIntIndex] = 0;
        ret(shift, bitsMinus1) = (*this)(0, lastShiftBit);
    }

    return ret;
}


xpsBitVector xpsBitVector::operator>>(unsigned shift) const
{
    const int bitsMinus1 = mBits - 1;

    // lowest bit index from the top cleared to 0 by the shift
    const int lowZeroBit = (int)mBits - (int)shift;

    // bit data to copy to, start and end
    unsigned *data;
    unsigned * last;

    // return this
    xpsBitVector ret(*this);

    if (lowZeroBit > 0)
    {
        last = ret.mData + ret.mLastIntIndex;
        for (data = ret.mData + (lowZeroBit >> BIT_INT_SHIFT); data <= last;)
        {
            *data++ = 0;
        }
        ret(0, lowZeroBit - 1) = (*this)(shift, bitsMinus1);
    }
    else
    {
        ret.reset();
    }

    return ret;
}


xpsBitVector& xpsBitVector::operator>>=(unsigned shift)
{
    const int bitsMinus1 = mBits - 1;

    // lowest bit index from the top cleared to 0 by the shift
    const int lowZeroBit = (int)mBits - (int)shift;

    // bit data to copy to, start
    unsigned *data;

    if (lowZeroBit > 0)
    {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        unsigned * last;
        xpsBitVector ret(*this);
        last = ret.mData + ret.mLastIntIndex;
        for (data = ret.mData + (lowZeroBit >> BIT_INT_SHIFT); data <= last;)
        {
            *data++ = 0;
        }
        ret(0, lowZeroBit - 1) = (*this)(shift, bitsMinus1);

        (*this) = (ret);
#else
        // pointer to last integer of data
        // TODO: Clutch to fix error on x86/64 after commit 70a19b1be4976d9d6da6880de7c44fb305492f4b
        unsigned * const pLastInt = mData + mLastIntIndex;

        // shift by copying range
        (*this)(0, lowZeroBit - 1) = (*this)(shift, bitsMinus1);

        if (shift)
        {
            // zero above the copied range

            // first zero mask
            data = mData + (lowZeroBit >> BIT_INT_SHIFT);
            KEEP_BITS(data, LOWER_BITS(lowZeroBit & INT_BIT_MASK));

            for (++data; data <= pLastInt;)
            {
                *data++ = 0;
            }
        }
#endif
    }
    else
    {
        reset();
    }

    return *this;
}


SETUP_BITWISE_OPERATORS(&)
SETUP_BITWISE_OPERATORS(|)
SETUP_BITWISE_OPERATORS(^)


bool xpsBitVector::operator==(const xpsBitVector& other) const
{
    // pointers to data of operands
    unsigned *data, *data1;

    // pointer to last integer of data
    unsigned *pLastInt;

    if (other.mBits != mBits)
    {
        return false;
    }

    pLastInt = mData + mLastIntIndex;

    for (data = mData, data1 = other.mData;
         data < pLastInt;
         data++, data1++)
        if (*data != *data1)
        {
            return false;
        }

    if ((le32toh(*data) & mLastIntMask) != (le32toh(*data1) & mLastIntMask))
    {
        return false;
    }

    return true;
}

void xpsBitVector::cpBits(unsigned *dest, unsigned destLo, unsigned destHi,
                          unsigned *src, int srcLoIntBit)
{

    // location of first of our integers in destination data
    unsigned *destPInt = dest + (destLo >> BIT_INT_SHIFT);

    // location of last of our integers in destination data
    unsigned * const destPLastInt = dest + (destHi >> BIT_INT_SHIFT);

    // temp variable
    unsigned tmp;

    // positions in unsigned integers are arranged in "target-little endian way":
    // position 0 is <unsigned> & 1U

    // position of lowest destination bit, in its integer
    const unsigned destLoIntBit = destLo & INT_BIT_MASK;

    // position of highest destination bit, in its integer
    const unsigned destHiIntBit = destHi & INT_BIT_MASK;

    // integer difference between bit positions in their integers
    // here initialized with difference in lowest bits of ranges
    int intBitShift = (int) destLoIntBit - srcLoIntBit;

    // positive amount of shift down and up of various parts of unsigneds
    unsigned partShiftDown, partShiftUp;

    // value read from src array, and next one
    unsigned val, nextVal;

    assert(destLo <= destHi);
    assert(srcLoIntBit < (int)INT_BITS);

    if (destPLastInt == destPInt)
        // dest range in one int
    {
        xpsBitVector::cpBitsTo1Int(destPInt, destLoIntBit, destHiIntBit, src,
                                   intBitShift);
    }

    else
    {
        //dest range not in one int


        //first we shift into the first dest int

        //mask
        tmp = EQ_HIGHER_BITS(destLoIntBit);

        // shifting down from the bottom source integer
        // to the bottom dest integer,
        // then the source must be two integers at least,
        // and we also need to shift from the second (next) integer
        if (intBitShift < 0)
        {

            // for each destination integer, including this one (the bottom)
            // but also, the middle ones (possibly excluding the top)
            // we always have to shift two adjacent source integers
            // and always by the same amounts

            //shift amout down from the top of the first source integer
            partShiftDown = -intBitShift;

            // shift amount up from the bottom of the second source integer
            partShiftUp = INT_BITS + intBitShift;

            WRITE_MASK_BITS(
                destPInt,
                le32toh(*src) >> partShiftDown | le32toh(*(src + 1)) << partShiftUp,
                tmp);

            //we don't need to shift from bottom source int anymore so increment pointer
            src++;
        }

        // if we are shifting not-down from the bottom source integer
        // then that's the only part that goes into the bottom dest integer
        else
        {
            WRITE_MASK_BITS(
                destPInt,
                le32toh(*src) << intBitShift,
                tmp);

            // after the bottom destination integer,
            // the middle ones still get two shifts
            // (unless the shift amount is 0 precisely, in which case
            // one of the shifts is by INT_BITS so it does not matter)
            partShiftDown = INT_BITS - intBitShift;
            partShiftUp = intBitShift;
        }


        // now we shift into the destination integers between the first and last one

        // be careful with 0 shift, in that case shifting down by INT_BITS would bomb
        // so we split off that case

        ++destPInt;
        if (intBitShift)
        {
            val = le32toh(*src);
            while (destPInt < destPLastInt)
            {
                tmp = val >> partShiftDown;
                nextVal = le32toh(*++src);
                *destPInt++ = htole32(tmp | nextVal << partShiftUp);
                val = nextVal;
            }
        }
        else
        {
            // because whole integers are copied on a single machine, no need to
            // worry about endian
            tmp = destPLastInt - destPInt;
            if (tmp)
            {
                memcpy(destPInt, src + 1, INT_SIZE * tmp);
                destPInt = destPLastInt;
                src += tmp;
            }
        }

        // finally we shift into the last dest integer
        // similarly to the first one:
        // if shift amount is negative, we only need to shift from one source integer (last one)
        // else, we need to shift part down from the second-last one, and part up from the last one

        intBitShift = (int) destHiIntBit - ((srcLoIntBit + destHi - destLo) &
                                            INT_BIT_MASK);

        // mask
        tmp = EQ_LOWER_BITS(destHiIntBit);

        // again be careful to take care of intBitShift 0
        // where shift by INT_BITS is invalid

        WRITE_MASK_BITS(
            destPInt,
            intBitShift < 0 ?
            le32toh(*src) >> (-intBitShift) :
            intBitShift ?
            le32toh(*src) >> (INT_BITS - intBitShift) | le32toh(*(src + 1)) << intBitShift :
            le32toh(*(src + 1)),
            tmp);
    }

}

void xpsBitVector::xpsBitVector_range_ref::cpFrom(const unsigned *src,
                                                  unsigned n)
{

    //destination data
    unsigned * const data = mpObj->mData;

    // location of first of our integers in destination data
    unsigned *destPInt = data + (mLo >> BIT_INT_SHIFT);

    // location of last of our integers in destination data
    unsigned * const destPLastInt = data + (mHi >> BIT_INT_SHIFT);

    // value read from src array, and next one
    unsigned val, nextVal;

    // positions in unsigned integers are arranged in "target-little endian way":
    // position 0 is <unsigned> & 1U

    // position of lowest destination bit, in its integer
    const unsigned destLoIntBit = mLo & INT_BIT_MASK;

    // we copy to destination in two phases
    // phase 1 is completely determined by src
    // phase 2 then starts at this pointer
    unsigned * destPInt2;

    // position of highest destination bit, in its integer
    const unsigned destHiIntBit = mHi & INT_BIT_MASK;

    // how much we shift down parts of src integers to copy to destination
    unsigned shiftDown;

    unsigned *srcCpEnd;  // limit of copied source (if we copy all of it)

    unsigned mask;

    // integer to write to a destination location
    unsigned destInt;


    if (destPLastInt == destPInt)
    {
        // dest range in one int
        // we just need to shift into that integer and mask
        mask = RANGE_BITS(destLoIntBit, destHiIntBit);
        WRITE_MASK_BITS(
            destPInt,
            *src << destLoIntBit,
            mask);
    }

    else
    {
        //dest range not in one int

        // depending on destLoIntBit, we copy to until destPInt2 differently

        // if destLoIntBit then each integer from src has to be split in two
        // and the parts contribute to consecutive integers in destination
        if (destLoIntBit)
        {
            srcCpEnd = destPInt + n ;
            destPInt2 = MIN(srcCpEnd, destPLastInt);
            shiftDown = INT_BITS - destLoIntBit;

            // write the first dest integer
            val = *src;
            mask = EQ_HIGHER_BITS(destLoIntBit);
            WRITE_MASK_BITS(
                destPInt,
                val << destLoIntBit,
                mask);

            // finish all that is determined by src, below destPInt2
            while (++destPInt < destPInt2)
            {
                nextVal = *++src;
                *destPInt = htole32(val >> shiftDown | nextVal << destLoIntBit);
                val = nextVal;
            }

            // next copy to destPInt2, if that is the same as the end, finish already
            if (destPInt2 == destPLastInt)
            {
                mask = EQ_LOWER_BITS(destHiIntBit);
                destInt = val >> shiftDown;
                if (destPInt2 < srcCpEnd)
                {
                    destInt |= *(src + 1) << destLoIntBit;
                }
                WRITE_MASK_BITS(
                    destPLastInt,
                    destInt,
                    mask);
                return;
            }

            assert(destPInt2 < destPLastInt);

            // start second phase
            *destPInt2++ = htole32(val >> shiftDown);
        }


        else
        {
            // if not shifting we can just copy mem

            srcCpEnd = destPInt + n - 1;
            destPInt2 = MIN(srcCpEnd, destPLastInt);

            // copy first phase
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            memcpy(destPInt, src, (destPInt2 - destPInt) * INT_SIZE);
#else
            for (int i = 0; i < (destPInt2 - destPInt); i++)
            {
                *(destPInt + i) = htole32(*(src + i));
            }
#endif

            val = *(src + n - 1);

            if (destPInt2 == destPLastInt)
            {
                mask = EQ_LOWER_BITS(destHiIntBit);
                WRITE_MASK_BITS(
                    destPLastInt,
                    val,
                    mask);
                return;
            }

            *destPInt2++ = htole32(val);
        }

        // after the start second phase is the same regardless of destLoIntBit
        memset(destPInt2, 0, (destPLastInt - destPInt2) * INT_SIZE);
        RESET_BITS(destPLastInt, EQ_LOWER_BITS(destHiIntBit));

    }
}

void xpsBitVector::xpsBitVector_range_ref::operator=(const uint16_t *src)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // just call void xpsBitVector::xpsBitVector_range_ref::operator=(const void *src)
    *this = (const void *)src;
#else
    int fullSrcsToUse = (mHi - mLo) % 16 ? (mHi-mLo)/16+1 : (mHi-mLo)/16;

    MemUInt16 arr(fullSrcsToUse);

    for (int i = 0; i < fullSrcsToUse; i++)
    {
        arr[i] = htole16(src[i]);
    }

    *this = (const void *)arr;
#endif
}

void xpsBitVector::xpsBitVector_range_ref::operator=(const uint32_t *src)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // just call void xpsBitVector::xpsBitVector_range_ref::operator=(const void *src)
    *this = (const void *)src;
#else
    int fullSrcsToUse;
    if (mHi == mLo)
    {
        /* copy only one bit */
        fullSrcsToUse = 1;
    }
    else
    {
        fullSrcsToUse = (mHi - mLo) % 32 ? ((mHi-mLo)/32) + 1 : (mHi-mLo)/32;
    }

    MemUInt32 arr(fullSrcsToUse);

    for (int i = 0; i < fullSrcsToUse; i++)
    {
        arr[i] = htole32(src[i]);
    }

    *this = (const void *)arr;
#endif
}

void xpsBitVector::xpsBitVector_range_ref::operator=(const uint64_t *src)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // just call void xpsBitVector::xpsBitVector_range_ref::operator=(const void *src)
    *this = (const void *)src;
#else
    int fullSrcsToUse = (mHi - mLo) % 64 ? (mHi-mLo)/64+1 : (mHi-mLo)/64;

    MemUInt64 arr(fullSrcsToUse);
    uint64_t hi32, low32;

    for (int i = 0; i < fullSrcsToUse; i++)
    {
        /* ugly hack --om-- kludge */
        // arr[i] = (htole64(src[i]);
        hi32 = htole32(*((uint32_t*)src+1));
        low32 = htole32(*((uint32_t*)src));
        arr[i] = (hi32 << 32) | low32;
        // printf("arr[i] = %llx", arr[i]);
    }

    *this = (const void *)arr;
#endif
}

void xpsBitVector::xpsBitVector_range_ref::operator=(const void *src)
{
    //destination data
    unsigned * const data = mpObj->mData;

    // location of first of our integers in destination data
    unsigned *destPInt = data + (mLo >> BIT_INT_SHIFT);

    // location of last of our integers in destination data
    unsigned * const destPLastInt = data + (mHi >> BIT_INT_SHIFT);

    // positions in unsigned integers are arranged in "target-little endian way":
    // position 0 is <unsigned> & 1U

    // position of lowest destination bit, in its integer
    const unsigned destLoIntBit = mLo & INT_BIT_MASK;

    // position of highest destination bit, in its integer
    const unsigned destHiIntBit = mHi & INT_BIT_MASK;

    // value to write to any dest integer
    unsigned val;

    // bit index after src from which we start copying
    unsigned lo;

    // src as pointer that we can increment
    uint8_t *src8 = (uint8_t*)src;

    if (destPLastInt == destPInt)
    {
        // dest range in one int
        // we just need to shift into that integer and mask
        UINT_BITS_ARR_ADVANCE_PLAIN(val, src8, 0, lo, mBits, false);
        WRITE_BITS_IN_MASK(
            destPInt,
            val << destLoIntBit,
            RANGE_BITS(destLoIntBit, destHiIntBit));
    }

    else
    {
        //dest range not in one int

        // write the first dest integer
        UINT_BITS_ARR_ADVANCE_PLAIN(val, src8, 0, lo, INT_BITS - destLoIntBit, true);
        WRITE_BITS_IN_MASK(
            destPInt,
            val << destLoIntBit,
            EQ_HIGHER_BITS(destLoIntBit));

        // middle integers

        while (++destPInt < destPLastInt)
        {
            UINT_BITS_ARR_ADVANCE_PLAIN(val, src8, lo, lo, INT_BITS, true);
            *destPInt = htole32(val);
        }

        // finally last dest integer
        UINT_BITS_ARR_ADVANCE_PLAIN(val, src8, lo, lo, destHiIntBit + 1, false);
        WRITE_BITS_IN_MASK(
            destPLastInt,
            val,
            EQ_LOWER_BITS(destHiIntBit));
    }
}

template <typename T> xpsBitVector* xpsBitVector::create(unsigned bits, T arr)
{
    xpsBitVector *result = new xpsBitVector(bits);
    unsigned *data;
    unsigned * const pLastInt = result->mData + result->mLastIntIndex;

    for (data = result->mData; data < pLastInt; data++)
        CP_ADVANCE_ARR_BOOL_TO_BITS(T, arr, INT_BITS, data)

        CP_ADVANCE_ARR_BOOL_TO_BITS(T, arr, result->mLastIntBits, data)

        return result;
}


// explicit instantiate for unsigned*
template xpsBitVector* xpsBitVector::create<unsigned*>(unsigned, unsigned*);


uint32_t xpsBitVector::toUint32(unsigned hi, unsigned lo) const
{

    // assert( abs((int)(hi-lo)) <= 31 );
    // this we return
    unsigned ret = 0;

    cpBitsTo1Int(&ret, 0, hi - lo, mData + (lo >> BIT_INT_SHIFT),
                 -(int)(lo & INT_BIT_MASK));
    return le32toh(ret);
}


uint64_t xpsBitVector::toUint64(unsigned hi, unsigned lo) const
{
    // assert( abs((int)(hi-lo)) <= 63 );
    // this we return
    uint64_t ret = 0;

    cpBits((unsigned *)&ret, 0, hi - lo, mData + (lo >> BIT_INT_SHIFT),
           lo & INT_BIT_MASK);
    // ret is in little endian format
    // need to convert it to host endian format before return
    return le64toh(ret);
}


unsigned xpsBitVector::count1s() const
{

    // pointer to data
    uint32_t *data = mData;

    // return this
    unsigned ret = 0;

    // pointer to next integer past the ones we use
    uint32_t * const pOver = mData + mLastIntIndex + 1;

    do
    {
        ret += Generic::count1s(*data++);
    }
    while (data < pOver);

    return ret;
}


unsigned xpsBitVector::find_first() const
{

    // pointer to data
    uint32_t *data = mData;

    // return this
    unsigned ret = 0;

    // single step find_first
    unsigned ret1;

    // pointer to next integer past the ones we use
    uint32_t * const pOver = mData + mLastIntIndex + 1;

    for (; data < pOver; data++)
    {
        ret += ret1 = Generic::find_first(le32toh(*data));
        if (ret1 < 32)
        {
            break;
        }
    }

    return ret;
}

unsigned xpsBitVector::find_last() const
{

    // pointer to data
    uint32_t *data = mData + mLastIntIndex;
    uint32_t lastIdx = mLastIntIndex;

    // return this
    unsigned ret = 0;

    // single step find_first
    unsigned ret1;

    for (; data >= mData; data--)
    {
        ret1 = Generic::find_last(le32toh(*data));
        ret = (lastIdx * 32) + ret1;
        if (ret1 < 32)
        {
            return ret;
        }
        lastIdx--;
    }

    return (32 * (mLastIntIndex + 1));
}

void xpsBitVector::set()
{
    /* get to word boundary */
    memset(mData, -1, mBytes);
    mData[mLastIntIndex] = htole32(mLastIntMask);
}

ostream&
operator<<(ostream& os, const xpsBitVector& b)
{
    // we print BITS_IN_ROW consecutive bits in one row
    // starting from low bits
    // in each row, low bit is on the right
    // in each row, there are blocks of BITS_IN_BLOCK bits printed consecutively, separated by space
    // under each row, there is an "index row"
    // where indices of low and high bits of blocks are shown under their bits
    // if indices are more than 3 digits (>= 1000) there will not be enough space under a block to print them
    // so they will be shifted

    // some unsigned quantities are signed so we can go into invalid neg values

    // bit index
    int bit;

    // last bit
    const unsigned last = b.size() - 1;

    const unsigned BITS_IN_BLOCK = 8;  // should be at least 8
    const unsigned BITS_IN_ROW = 64;  // should be multiple of BITS_IN_BLOCK

    // left bit in a row
    // this may be higher than last, in which case not printed
    unsigned lBitInRow = BITS_IN_ROW - 1;

    // right bit in row
    unsigned rBitInRow;

    // left bit in row really printed
    unsigned lPrintedBitInRow;

    // as above but for blocks
    int lBitInBlock;
    unsigned rBitInBlock, lPrintedBitInBlock;

    const char SPACE = ' ';

    char row[BITS_IN_ROW << 2];
    char *p;  // pointer to place in the row

    char indexRow[BITS_IN_ROW << 2];
    char *pI ; // pointer to place in the index row

    // left block index string
    char lI[BITS_IN_BLOCK << 1];
    unsigned lLength; // its length

    // right block index string
    char rI[BITS_IN_BLOCK << 1];
    unsigned rLength; // its length

    // total of the lengths above left and right
    unsigned lrLength;

    // in between the left and right printed indexes is the middle padding
    // to the left of the left index there may be left padding
    // whose lengths are
    unsigned midPadLength, leftPadLength;


    // loop over rows
    while (1)
    {
        lPrintedBitInRow = MIN(lBitInRow, last);
        rBitInRow = lBitInRow - (BITS_IN_ROW - 1);

        bit = lBitInRow;
        p = row;
        pI = indexRow;

        // loop over blocks
        lBitInBlock = lBitInRow;
        while (1)
        {
            lBitInBlock = bit;
            if (lBitInBlock < (int)rBitInRow)
            {
                *p = 0;
                *pI = 0;
                break;
            }
            rBitInBlock = lBitInBlock - (BITS_IN_BLOCK - 1);

            // fill up the block of bits to be printed
            for (; bit >= (int)rBitInBlock; bit--)
            {
                *p++ = bit > (int)lPrintedBitInRow ? SPACE : b[bit] + '0';
            }

            // separate blocks
            *p++ = SPACE;

            // now fill up the part of the index row that corresponds to this block

            *lI = 0;
            *rI = 0;

            // print right index if needed
            if (last >= rBitInBlock)
            {
                sprintf(rI, "%u", rBitInBlock);
            }

            rLength = strlen(rI);

            // print left index if needed and different than right index
            if (last > rBitInBlock)
            {

                lPrintedBitInBlock = MIN((int)last, lBitInBlock);
                sprintf(lI, "%u",  lPrintedBitInBlock);
                lLength = strlen(lI);
                rLength = strlen(rI);
                lrLength = lLength + rLength;

                // length of pad between left and right index
                // at least 1, but may be more if needed to push the left index
                // to align with the left end of the bit block
                midPadLength = MAX((int)lPrintedBitInBlock - (int)rBitInBlock - (int)lrLength,
                                   0) + 1;

                // if the block is not of maximal length, we need left pad as well
                leftPadLength = MAX((int)BITS_IN_BLOCK - (int)lrLength - (int)midPadLength, 0);
            }
            else
            {
                lLength = 0;
                leftPadLength = (BITS_IN_BLOCK - rLength);
                midPadLength = 0;
            }

            memset(pI, SPACE, leftPadLength);
            pI += leftPadLength;
            memcpy(pI, lI, lLength);
            pI += lLength;
            memset(pI, SPACE, midPadLength);
            pI += midPadLength;
            memcpy(pI, rI, rLength);
            pI += rLength;
            *pI++ = SPACE;

        }

        os << endl << "val:  " << row << endl << "pos:  " << indexRow << endl << endl;

        if (lBitInRow < last)
        {
            lBitInRow += BITS_IN_ROW;
        }
        else
        {
            break;
        }
    }

    return os;
}

void printBv(const xpsBitVector& b)
{
    std::cout << b ;
}

void xpsBitVector::bit_dump()
{
    uint32_t i, j;
    uint8_t p;

    uint32_t nu8 = (((mBits + 31) >> 5) << 2);
    uint8_t *ptr = (uint8_t *)mData;

    //  printf("nu8 = %u, mBytes = %u, mBits = %u \n", nu8, mBytes, mBits);
    for (i = 0; i < nu8; i++)
    {
        p = *(uint8_t*)(ptr + i);
        for (j = 0; j < 8; j++)
        {
            printf("%s", ((p & (0x1 << (7 - j))) == 0) ? "0" : "1");
        }
        printf("%s", (i == (nu8 - 1)) ? "" : "_");
    }
    printf("\n");
    return;
}

void xpsBitVector::hex_dump()
{

    uint32_t i;
    uint8_t p;
    uint32_t nu8 = (((mBits + 31) >> 5) << 2);
    //  printf("nu8 = %u, mBytes = %u, mBits = %u \n", nu8, mBytes, mBits);
    uint8_t *ptr = (uint8_t *)mData;

    //  printf("nu8 = %u \n0x", nu8);
    for (i = 0; i < nu8; i++)
    {
        p = *(uint8_t*)(ptr + i);
        printf("%02x", p);
        printf("%s", (i == (nu8 - 1)) ? "" : "_");
    }
    printf("\n");
    return;
}

void xpsBitVector::dump(unsigned lineNum)
{

    // indices of ints and bits in int
    unsigned i = 0, j;

    // index of bit in the vector
    unsigned bit = 0;
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, "<dec = %d> \n", IOS_DEC);
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, "<line = %d> \n", lineNum);
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, "<HEX data:> \n");
    {
        char buf[255];
        buf[0] = '\0';
        unsigned char *ptr = (unsigned char *)mData;
        unsigned num = mBytes;
        while (num % 4)
        {
            num++;
        }
        unsigned i=0;
        for (i=0; i<num; i++)
        {
            sprintf(buf + strlen(buf), "%02X ", ptr[i]);
            if (0 == (i+1) % 8)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, "%d  -  %d  :  %s \n",
                      (i-7), i, buf);
                buf[0] = '\0';
            }
            else if (0 == (i+1) % 4)
            {
                sprintf(buf + strlen(buf), "| ");
            }
        }

        if (strlen(buf) > 0)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, " :   %s \n", buf);
        }
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, "xpsBitVector: \n");
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, "mData: \n");
    for (; i <= mLastIntIndex; i++)
    {
        for (j = 0; j < INT_BITS; j++, bit++)
        {
            if (bit == mBits)
            {
                break;
            }
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, "bit  %d  %d   %d:\n",
                  IOS_DEC, bit, ((le32toh(mData[i]) & (1U << j)) != 0));
        }
        if (bit == mBits)
        {
            break;
        }
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_CRITICAL,  "<dec  %d> \n", IOS_DEC);
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, "<mLastIntIndex  %d> \n",
          mLastIntIndex);
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, "<mLastIntBits  %d> \n",
          mLastIntBits);
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_CRITICAL,
          "<mLastIntMask 0x  %s  %d> \n", IOS_HEX, mLastIntMask);
    assert(!(mData[mLastIntIndex] & ~htole32(mLastIntMask)));
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, "<mBits %s %d >\n", IOS_HEX,
          mBits);
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_CRITICAL, "<mBytes %d> \n", mBytes);
}


void xpsBitVector::rand()
{
    for (unsigned i = 0; i < mBits; i++)
    {
        (*this)[i] = ::rand() & 1U ? true : false;
    }
}

/*uint32_t  int_arr_bool_bit(uint32_t *int_arr, int position)
{
        uint32_t temp = (*((int_arr) + ((position) >> BIT_INT_SHIFT)));
        uint32_t temp1;
        uint32_t temp2;
        temp1 = position & INT_BIT_MASK;
        temp2 = ENDIAN_BIT_POS(temp1);
        temp2 = 1U << temp2;
        temp = temp  & temp2;
        return (temp ? (1):(0));
}*/


