// xpsBitVector.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef XP_BIT_VECTOR_H_
#define XP_BIT_VECTOR_H_

#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <iostream>
#include <stdexcept>
//#include "xpMacro.h"
//#include "xpSaiLog.h"
//#ifdef __cplusplus
//extern "C++"{
//#endif
#include "xpsGeneric.h"
#include "xpsExceptions.h"
#include "xpsSal.h"
//#ifdef __cplusplus
//}
//#endif

#define _STRINGIZE(x) #x
#define STRINGIZE(x) _STRINGIZE(x)

#define _FILE_LINE \
        "file " __FILE__ "\tline " STRINGIZE(__LINE__)
#define IOS_HEX hex
#define IOS_DEC dec

#ifdef __MACH__
#include <libkern/OSByteOrder.h>
#define htole16(x) OSSwapHostToLittleInt16(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)
#define htobe64(x) OSSwapHostToBigInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define htobe32(x) OSSwapHostToBigInt32(x)
#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#endif

using namespace std;

class xpsBitVector;


//======================================
// dump bit vector b to stream os
//======================================
ostream&
operator<<(ostream& os, const xpsBitVector& b);

void printBv(const xpsBitVector& b);

//=============================================================
// word phase shift for big-endian
//=============================================================
#define BIT_PHASE_SHIFT          (31)

//=============================================================
// word bit width big-endian
//=============================================================
#define BIT_PHASE_INT_WIDTH      (32)

//=============================================================
// word bit width big-endian
//=============================================================
#define BIT_PHASE_CHAR_WIDTH      (8)

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
//=============================================================
// position for little-endian
//=============================================================
#define ENDIAN_BIT_POS(p)     (p)
#else
//
//=============================================================
// big endian position for given little-endian
//=============================================================
#define ENDIAN_BIT_POS(p)    (BIT_PHASE_INT_WIDTH - (((((p)/BIT_PHASE_CHAR_WIDTH)+1)*BIT_PHASE_CHAR_WIDTH) - ((p) % BIT_PHASE_CHAR_WIDTH)))
#endif




//======================================
// for T array type of unsigned integer
//  T arr
//  unsigned size
//  unsigned *data
// copy boolean arr[i] as bit i to *data
// for size bits starting from i = 0
// and advance arr to increment by size
//======================================
#define  CP_ADVANCE_ARR_BOOL_TO_BITS(T, arr, size, data) \
    { \
        T end = arr + (size); \
        unsigned datum = 0; \
        for (unsigned shift = 0; arr < end; arr++, shift++) \
            datum |= (*arr != 0) << ENDIAN_BIT_POS(shift); \
        *(data) = datum; \
    }


#ifndef MIN
// minimum of integers a , b
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
// maximum of integers a , b
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif

// size of int in Bytes
static const unsigned INT_SIZE = sizeof(unsigned);

// size of int in bits
static const unsigned INT_BITS = INT_SIZE * CHAR_BIT;

// size of positive part of int in bits
static const unsigned POSITIVE_INT_BITS = INT_BITS - 1;


//======================================
// shift unsigned integer type x by s, if s negative, shift to the right
//
// both x and s should be evaluated, not expressions, to
// save time
//======================================
#define SHIFT_INT(x, s) ((s) < 0 ? (x) >> -(s) : (x) << (s))


//======================================
// write val to an unsigned type *addr only inside mask bits
//
// addr and mask should be evaluated, not expressions, to save time
//
// note how this works for unsigned char - the right side is unsigned int
// that gets correctly converted
//======================================
#define WRITE_MASK_BITS(addr, val, mask) { *(addr) = htole32((le32toh(*(addr)) & ~(mask)) | ((val) & (mask))); }

// #define WRITE_MASK_BITS_LE(addr, val, mask) { *(addr) = (*(addr) & ~(mask)) | ((val) & (mask)); }

//======================================
// write val to an unsigned type *addr only inside mask bits
// assuming val has only 1 bits inside mask
//
// addr should be evaluated, not expressions, to save time
//
// note how this works for unsigned char - the right side is unsigned int
// that gets correctly converted
//======================================
#define WRITE_BITS_IN_MASK(addr, val, mask)\
{\
    assert(!((val) & ~(mask)));\
    *(addr) = htole32(\
        (le32toh(*(addr)) & ~(mask)) | (val));\
}

//======================================
// only keep bits unchanged in *addr
// reset the rest
//
// addr should be evaluated, not an expression, to save time
//======================================
#define KEEP_BITS(addr, bits) { *(addr) = htole32(le32toh(*(addr)) & (bits)); }

//======================================
// set bits in *addr
//
// addr should be evaluated, not an expression, to save time
//======================================
#define SET_BITS(addr, bits) { *(addr) = htole32(le32toh(*(addr)) | (bits)); }

//======================================
// reset bits in *addr
//
// addr should be evaluated, not an expression, to save time
//======================================
#define RESET_BITS(addr, bits) { *(addr) = htole32(le32toh(*(addr)) & ~(bits)); }


//======================================
// unsigned with only bits 0, ..., n-1 set
//
// n can be from 0 to INT_BITS - 1
//======================================
#define LOWER_BITS_LOW(n) ((1U << (n)) - 1)

//======================================
// unsigned with only bits 0, ..., n-1 set
//
// n can be from 0 to INT_BITS
//
// n should be evaluated, not expression, to save time
//======================================
#define LOWER_BITS(n) (INT_BITS == (n) ? ~0ul : LOWER_BITS_LOW(n))


//======================================
// unsigned with bits up to n set
//
// n can be from 0 to INT_BITS - 2
//======================================
#define EQ_LOWER_BITS_LOW(n) ((1U << ((n) + 1)) - 1)

//======================================
// unsigned with bits up to n set
//
// n can be from 0 to INT_BITS - 1
//
// n should be evaluated, not expression, to save time
//======================================
#define EQ_LOWER_BITS(n) (INT_BITS - 1 == (n) ? ~0 : EQ_LOWER_BITS_LOW(n))


//======================================
// unsigned with only bits m to n set
//
// m, n can be from 0 to INT_BITS - 2
//======================================
#define RANGE_BITS_LOW(m, n) (EQ_LOWER_BITS_LOW(n) & ~LOWER_BITS_LOW(m))

//======================================
// unsigned with only bits m to n set
//
// m, n can be from 0 to INT_BITS - 1
//======================================
#define RANGE_BITS(m, n) (INT_BITS - 1 == (n) ? ~LOWER_BITS(m) : RANGE_BITS_LOW(m, n))


//======================================
// unsigned with only bits m and higher set
//
// m can be from 0 to INT_BITS - 1
//======================================
#define EQ_HIGHER_BITS(m) (~LOWER_BITS(m))


//======================================
// how many n bit quantities cover m bit quantity
// both should be evaluated, not an expression, to save time
//======================================
#define N_COVERS(m, n) ((m) % (n) ? (m) / (n) + 1 : (m) / (n))

//======================================
// how many characters cover m bit quantity
// m should be evaluated, not an expression, to save time
//======================================
#if (CHAR_BIT == 8)
#define N_CHAR_COVERS(m) ((m) & 0x7U ? ((m) >> 3) + 1 : (m) >> 3)
#else
#define N_CHAR_COVERS(m) (N_COVERS(m, CHAR_BIT))
#endif

//======================================
// how many integers cover m bit quantity
// m should be evaluated, not an expression, to save time
//======================================
#if (INT_BITS == 64)
#define N_INT_COVERS(m) ((m) & 0x3FU ? ((m) >> 6) + 1 : (m) >> 6)
#else
#if (INT_BITS == 32)
#define N_INT_COVERS(m) ((m) & 0x1FU ? ((m) >> 5) + 1 : (m) >> 5)
#else
#define N_INT_COVERS(m) (N_COVERS(m, INT_BITS))
#endif
#endif


//======================================
// shift down by this to get number of whole ints from number of bits
//
// works only for 32 or 64 bit target machine
//======================================
static const unsigned BIT_INT_SHIFT = INT_BITS >> 6 ? 6 : 5;

//======================================
// to get the bit position within an unsigned integer
// and the position within an unsigned integer array, with this
//
// works only for 32 or 64 bit target machine
//======================================
static const unsigned INT_BIT_MASK = LOWER_BITS_LOW(BIT_INT_SHIFT);


//======================================
// for contigous array of unsigned started at int_arr
// treated as contiguous array of bits, starting at bit int_arr[0] & 1U
// bit at index position
//
// return bool true only if that bit is set, false if not set
//
// position should be evaluated, not an expression, to save time
//======================================
#define INT_ARR_BOOL_BIT(int_arr, position) \
    (htole32(*((int_arr) + ((position) >> BIT_INT_SHIFT))) & ( 1U << ((position) & INT_BIT_MASK)))


//======================================
// assign to value,
// how many bits are in the last segment of n bit quantity coverd by ints
//
// temp is a temporary unsigned int variable
//======================================
# if 0
#define LAST_INT_BITS(value, n) \
    if (! (value = ((n) & INT_BIT_MASK))) \
            value = INT_BITS;

#endif
//======================================
//! array of bits
//!
//! this is usually a software representation of a hardware
//! register or area
//! you can access and change individual bits or bit ranges efficiently
//======================================

class xpsBitVector
#if !DISABLE_LOG

#endif
{
public:


    //======================================
    //
    //! class representing a range of bits in existing xpsBitVector object
    //
    /// this class should be transparent to the user - that is, the user need not use the name of the class
    //======================================
    class xpsBitVector_range_ref
    {
    public:

        //======================================
        //! constructor
        //
        /// bits indexed between and including end0 and end1 in existing xpsBitVector *pObj
        /// end1 can be greater, equal or less than end0
        //======================================
        xpsBitVector_range_ref(unsigned lo, unsigned hi, xpsBitVector* pObj)
            : mpObj(pObj)
        {
            // assign mLo and mHi correctly regardless of order of ends
            if (lo < hi)
            {
                mLo = lo;
                mHi = hi;
            }
            else
            {
                mLo = hi;
                mHi = lo;
            }
            mBits = mHi - mLo + 1;
        }

        //======================================
        //! assignment from a bit range
        //
        /// could be within the same vector as long as the ranges do not overlap
        ///
        /// also, could be within the same vector and overlap as long as
        /// the destination range is lower
        /// (this behaviour should work but has not been thoroughly tested)
        ///
        /// the two ranges must have the same size or else
        /// xwe pThrow(std::range_error(<file name and line>)
        //======================================
        void operator=(const xpsBitVector_range_ref& range)
        {
            if (mBits != range.mBits)
            {
                xpThrow((range_error(_FILE_LINE)));
            }

            cpBits(
                mpObj->mData,
                mLo,    mHi,
                range.mpObj->mData + (range.mLo >> BIT_INT_SHIFT),
                range.mLo & INT_BIT_MASK);
        }
        //======================================
        //! assignment from a bit vector
        //
        /// assign from the start of the bit array
        /// to all the bits in this range
        ///
        /// the sizes must be the same or else
        /// we xpThrow((std::range_error(<file name and line>)))
        ///
        /// the source vector must be different than this one
        //======================================
        void operator= (const xpsBitVector& oth)
        {
            if (mBits != oth.mBits)
            {
                xpThrow((range_error(_FILE_LINE)));
            }

            cpBits(mpObj->mData, mLo, mHi, oth.mData, 0);
        }


        //======================================
        //! assignment from uint32_t
        //
        /// the assignment works on significant bits of the right-hand side
        /// the insignificant top 0 bits are truncated or padded
        //
        /// if there are more significant bits than the size of the left hand side
        /// then xpThrow((std::range_error(<this file name and line>)))
        ///
        /// assume little endian target
        //======================================
        void operator= (uint32_t n)
        {
            if (mBits < 32 && (n >= 1U << mBits))
            {
                xpThrow((range_error(_FILE_LINE)));
            }

#if 64 == INT_BITS
            *this = &n;
#else
            cpFrom(&n, 1);
#endif
        }

        //======================================
        //! assignment from uint64_t
        //
        /// the assignment works on significant bits of the right-hand side
        /// the insignificant top 0 bits are truncated or padded
        //
        /// if there are more significant bits than the size of the left hand side
        /// then xpThrow((std::range_error(<this file name and line>)))
        ///
        /// assume little endian target
        //======================================
        void operator= (uint64_t n)
        {
            if (mBits < 64 && (n >= (uint64_t)1U << mBits))
            {
                xpThrow((range_error(_FILE_LINE)));
            }

            uint32_t *x = (uint32_t*)&n;
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
            uint64_t n1;
            uint32_t *x1 = (uint32_t*)&n1;
            *(x1) = *(x+1);
            *(x1+1) = *(x);
            x = x1;
#endif
            cpFrom(x, 64 >> BIT_INT_SHIFT);
        }

        //======================================
        //! assignment from a positive int
        //
        /// needed for some compilers who won't automatically cast to a fixed size unsigned type
        //
        /// the assignment works on significant bits of the right-hand side
        /// the insignificant top 0 bits are truncated or padded
        //
        /// if right-hand side is negative, or
        /// if there are more significant bits than the size of the left hand side
        /// then xpThrow((std::range_error(<this file name and line>)))
        ///
        /// assume little endian target
        //======================================
        void operator= (int n)
        {
            if (n < 0 || (mBits < POSITIVE_INT_BITS && ((unsigned)n >= 1U << mBits)))
            {
                xpThrow((range_error(_FILE_LINE)));
            }
            unsigned uint = n;
            cpFrom(&uint, 1);
        }


        //======================================
        //! assignment from void * which is unknown integer type array
        //
        /// assign as many bits as fits in our range
        /// assume src is unsigned integers in little-endian fashion
        //
        /// assume there is enough space in our array
        //======================================
        void operator=(const void *src);
        void operator=(const uint16_t *src);
        void operator=(const uint32_t *src);
        void operator=(const uint64_t *src);

        //======================================
        //! convert range to 32-bit unsigned
        //
        /// start with bit 0 of the return and assume we have at most
        /// 32 bits
        /// if fewer, pad return with 0s
        //======================================
        uint32_t toUint32()
        {
            return mpObj->toUint32(mHi, mLo);
        }

        //======================================
        //! convert range to 64-bit unsigned
        //
        /// start with bit 0 of the return and assume we have at most
        /// 64 bits
        /// if fewer, pad return with 0s
        //======================================
        uint64_t toUint64()
        {
            return mpObj->toUint64(mHi, mLo);
        }

        //======================================
        //! return *this & other
        //
        /// same sizes or core!
        //======================================
        xpsBitVector operator&(const xpsBitVector_range_ref& other) const
        {
            return xpsBitVector(*this) & xpsBitVector(other);
        }


        friend class xpsBitVector;

    private:

        // pointer to the existing xpsBitVector
        xpsBitVector* const mpObj;

        // indices of the lowest and highest bits in  *mpObj
        // we always have mLo <= mHi
        unsigned mLo;
        unsigned mHi;

        unsigned mBits; // number of bits in range

        // copy only significant bits from array src of n elements
        // significant bits means, skipping top 0 bits
        // assume the range has enough space for the significant bits
        // pad with 0 if there are bits left in the range
        void cpFrom(const unsigned *src, unsigned n);

    }; // xpsBitVector_range_ref


    //======================================
    //! class representing one bit in existing xpsBitVector object
    //
    /// this class should be transparent to the user - that is, the user need not use the name of the class
    //======================================
    class xpsBitVector_bit_ref
    {
    public:

        //======================================
        //! constructor
        //
        /// position is the index of the bit
        /// pObj is pointer to an existing xpsBitVector
        //======================================
        xpsBitVector_bit_ref(xpsBitVector * pObj, unsigned position): mpObj(pObj),
            mPosition(position) {}


        //======================================
        //! assignment from another
        //======================================
        void operator= (const xpsBitVector_bit_ref& bit)
        {
            unsigned *addr = mpObj->mData + (mPosition >> BIT_INT_SHIFT);
            const unsigned mask = 1U << (mPosition & INT_BIT_MASK);

            if (INT_ARR_BOOL_BIT(bit.mpObj->mData, bit.mPosition))
            {
                SET_BITS(addr, mask);
            }
            else
            {
                RESET_BITS(addr, mask);
            }
        }

        //======================================
        //! assignment from bool
        //======================================
        void operator= (bool bit)
        {
            unsigned *addr = mpObj->mData + (mPosition >> BIT_INT_SHIFT);
            const unsigned value = *addr;

            *addr = value ^ (((-bit) ^ value) & (1U << (mPosition & INT_BIT_MASK)));
        }

        //======================================
        //! true only if bit at position is 1, false if 0
        //
        /// "const" operator not used so to disambiguate from the
        /// const operator[] above with a different ret value
        //======================================
        operator bool() const
        {
            return INT_ARR_BOOL_BIT(mpObj->mData, mPosition);
        }


    private:

        // pointer to the existing xpsBitVector
        xpsBitVector* const mpObj;

        // index of the bit in *mpObj
        const unsigned mPosition;

    }; // xpsBitVector_bit_ref


    // debug dump
    void dump(unsigned lineNum);

    // randomize vector
    void rand();

    //======================================
    //! Constructor
    //
    //! create an instance with all zero bits
    //! \param bits number of bits
    //
    /// xpThrow((std::range_error(<file name and line>))) if number of bits is 0
    //======================================
    explicit
    xpsBitVector(unsigned bits)
        : mStorage(0)
    {
        init(bits);
        reset();
    }

    //======================================
    //! Constructor
    //
    //! copy data segment of srcBits behind src to beginning of bitVector of size destBits
    //
    /// pad remaining bits with 0s
    //
    //! \param destBits destination bits
    //! \param src source data
    //! \param srcBits number of bits from the source to be used
    //
    /// src array is stored little-endian fashion
    //======================================
    explicit
    xpsBitVector(unsigned destBits, uint8_t* src, unsigned srcBits)
        : mStorage(0)
    {
        init(destBits);
        reset();
        setDataToRange(0, srcBits, src);
    }

    //======================================
    //! Constructor
    //
    //! copy data segment of srcBits behind src to beginning of bitVector of size destBits
    //
    /// pad remaining bits with 0s
    //
    //! \param destBits destination bits
    //! \param src source data
    //! \param srcBits number of bits from the source to be used
    //
    /// src array is stored little-endian fashion
    //======================================
    explicit
    xpsBitVector(unsigned destBits, uint16_t* src, unsigned srcBits)
        : mStorage(0)
    {
        init(destBits);
        reset();
        setDataToRange(0, srcBits, src);
    }

    //======================================
    //! Constructor
    //
    //! copy data segment of srcBits behind src to beginning of bitVector of size destBits
    //
    /// pad remaining bits with 0s
    //
    //! \param destBits destination bits
    //! \param src source data
    //! \param srcBits number of bits from the source to be used
    //
    /// src array is stored little-endian fashion
    //======================================
    explicit
    xpsBitVector(unsigned destBits, uint32_t* src, unsigned srcBits);

    //======================================
    //! Constructor
    //
    //! copy data segment of srcBits behind src to beginning of bitVector of size destBits
    //
    /// pad remaining bits with 0s
    //
    //! \param destBits destination bits
    //! \param src source data
    //! \param srcBits number of bits from the source to be used
    //
    /// src array is stored little-endian fashion
    //======================================
    explicit
    xpsBitVector(unsigned destBits, uint64_t* src, unsigned srcBits);


    //======================================
    //! Constructor
    //
    //! merely allocate space for variables without initializing anything
    //======================================
    xpsBitVector();


    //======================================
    //! Copy constructor
    //======================================
    xpsBitVector(const xpsBitVector &other);


    //======================================
    //! Copy constructor from range
    //======================================
    xpsBitVector(const xpsBitVector_range_ref& range);


    //======================================
    //! resize the bit array
    //
    /// copy as much as possible of the old bits data from
    /// the beginning to the new data
    //======================================
    void resize(unsigned bits)
    {
        init(bits);
    }


    //======================================
    //! assignment
    //
    /// make *this vector identical to other
    //
    /// sizes should be the same beforehand
    /// xpThrow((std::range_error(<this file name and line>))) if not
    //======================================
    xpsBitVector& operator=(const xpsBitVector &other);


    //======================================
    //! assign to this vector from bits range
    //
    /// size should already be the same as the range
    /// xpThrow((std::range_error(<this file name and line>))) if not
    //
    /// it is allowed to assign to vector from the whole bit range of itself
    /// (nothing happens then)
    //======================================
    void operator=(const xpsBitVector_range_ref &range)
    {
        if (mBits != range.mBits)
        {
            xpThrow((range_error(_FILE_LINE)));
        }

        cpBits(
            mData,
            0, mBits - 1,
            range.mpObj->mData + (range.mLo >> BIT_INT_SHIFT),
            range.mLo & INT_BIT_MASK);
    }


    //======================================
    //! bit selection
    //
    /// the caller should not care about the return type
    ///
    /// position should be less than number of bits or else
    /// xpThrow((std::range_error(<this file name and line>)))
    ///
    /// vector[n] can be used as an lvalue
    /// and can be used as if it were bool
    //======================================
    xpsBitVector_bit_ref operator[](unsigned position) const
    {
        if (mBits <= position)
        {
            cout << "Allocated mBits = " << mBits << " Accessed position " << position <<
                 endl;
            xpThrow((range_error(_FILE_LINE)));
        }
        return xpsBitVector_bit_ref(const_cast<xpsBitVector *>(this), position);
    }


    //======================================
    //! return the abstraction of range of bits from the vector
    //
    /// range of bits indexed between and including end0 and end1
    /// end1 can be greater, equal or less than end0
    ///
    /// the caller should not care about the exact return type
    ///
    /// you can do the following, as long as sizes are the same:
    ///
    /// v and w are vectors:
    ///
    /// v(a, b) = w(c, d)
    ///
    /// v = w(c, d)
    ///
    /// v(a, b) = w
    ///
    /// construct vector:
    /// v(w(c, d))
    ///
    /// assign between 32-bit unsigned integer and the same size range:
    /// v(a, b) = u32
    /// u32 = v(a, b)
    ///
    /// const void * arr is a pointer to little-endian unsigned int array
    /// that is assumed to hold enough bits to fill the range:
    ///
    /// v(a, b) = arr8
    //======================================
    xpsBitVector_range_ref operator()(unsigned end0, unsigned end1) const
    {
        return xpsBitVector_range_ref(end0, end1, const_cast<xpsBitVector *>(this));
    }


    //======================================
    ///dump the bits into already allocated space after pNewData
    //
    /// starting bit 0 goes into pNewData[0] & 1U
    /// and so forth
    //
    /// \param pNewData allocated space assumed enough to hold destBits bits
    //
    /// xpThrow((std::range_error(<this file name and line>))) if trying to dump more
    /// than destBits
    //======================================
    void getUint8Array(uint8_t *pNewData, unsigned destBits) const
    {
        if (destBits < mBits)
        {
            xpThrow((range_error(_FILE_LINE)));
        }
        /* case mBytes * 8 > mBits is possible, so, calculate bytes to copy using mBits */
        uint32_t bytesBv = (mBits % 8 == 0)? mBits / 8 : mBits / 8 + 1;
        memcpy(pNewData, mData, bytesBv);
    }
    void hex_dump();
    void bit_dump();
    //======================================
    ///dump the bits into already allocated space after pNewData
    //
    /// starting bit 0 goes into pNewData[0] & 1U
    /// and so forth
    //
    /// \param pNewData allocated space assumed enough to hold destBits bits
    //
    /// xpThrow((std::range_error(<this file name and line>))) if trying to dump more
    /// than destBits
    //
    // assuming little endian target
    //======================================
    void getUint32Array(uint32_t *pNewData, unsigned destBits) const
    {
        if (destBits < mBits)
        {
            xpThrow((range_error(_FILE_LINE)));
        }
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        memcpy(pNewData, mData, mBytes);
#else
        uint32_t *data = mData;
        uint32_t *last = mData + mLastIntIndex;
        for (; data<last; data++, pNewData++)
        {
            *pNewData = le32toh(*data);
        }
        *pNewData = this->toUint32(mBits, (data - mData)*8*4);
#endif
    }

    //======================================
    ///dump the bits into already allocated space after pNewData
    //
    /// starting bit 0 goes into pNewData[0] & 1U
    /// and so forth
    //
    /// \param pNewData allocated space assumed enough to hold destBits bits
    //
    /// xpThrow((std::range_error(<this file name and line>))) if trying to dump more
    /// than destBits
    //
    // assuming little endian target
    //======================================
    void getUintArray(unsigned *pNewData, unsigned destBits) const
    {
        if (destBits < mBits)
        {
            xpThrow((range_error(_FILE_LINE)));
        }
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        memcpy(pNewData, mData, mBytes);
#else
        unsigned *data = mData;
        unsigned *last = mData + mLastIntIndex;
        for (; data<last; data++, pNewData++)
        {
            *pNewData = le32toh(*data);
        }
        *pNewData = this->toUint32(mBits, (data - mData)*8*4);
#endif
    }

    //======================================
    //! assignment to bit vector from an unsigned integer type T
    //
    /// the assignment works on significant bits of the right-hand side
    /// the insignificant top 0 bits are truncated or padded
    //
    /// if there are more significant bits than the size of the left hand side
    /// then xpThrow((std::range_error(<this file name and line>)))
    ///
    /// assume little endian target
    //======================================
    //  template <typename T> xpsBitVector operator=(T val);

    //======================================
    //! assignment from a positive int
    //
    /// needed for some compilers who won't automatically cast to a fixed size unsigned type
    //
    /// the assignment works on significant bits of the right-hand side
    /// the insignificant top 0 bits are truncated or padded
    //
    /// if right-hand side is negative, or
    /// if there are more significant bits than the size of the left hand side
    /// then xpThrow((std::range_error(<this file name and line>)))
    ///
    /// assume little endian target
    //======================================
    xpsBitVector& operator=(int);
    xpsBitVector& operator=(uint32_t);
    xpsBitVector& operator=(uint16_t);
    xpsBitVector& operator=(uint8_t);
    xpsBitVector& operator=(uint64_t);


    //======================================
    //! flip itself all bits
    //
    /// return itself
    //======================================
    xpsBitVector& flip()
    {
        flipTo(mData);
        return *this;
    }

    //======================================
    //! return all bits flipped
    //======================================
    xpsBitVector operator~() const;



    //======================================
    //! return shifted up
    //
    /// shift uppermost bits out and do not increase size
    /// of bitVector
    //
    /// \param shift by this many bits shift up
    //======================================
    xpsBitVector operator<<(unsigned shift) const;

    //======================================
    //! shift itself up
    //
    /// shift uppermost bits out and do not increase size
    /// of bitVector
    ///
    /// yes this is slower than it could be, as we return << value and copy it
    /// we cannot <<= in place using xpsBitVector_range_ref operator=
    /// because that works in place only in the shift down direction
    /// not up
    /// to fix this we could decrease performance and increase size of
    /// xpsBitVector_range_ref operator= code
    /// to make it work in both directions, but we prefer to slow down here
    //
    /// \param shift by this many bits shift up
    //
    /// return itself
    //======================================
    xpsBitVector& operator<<=(unsigned shift)
    {
        return *this = *this << shift;
    }


    //======================================
    //! return shifted down
    //
    /// \param shift by this many bits shift down
    //======================================
    xpsBitVector operator>>(unsigned shift) const;

    //======================================
    //! shift itself down
    //
    /// \param shift by this many bits shift down
    //
    /// return itself
    //======================================
    xpsBitVector& operator>>=(unsigned shift);

    //======================================
    //! return true if no bits in 1 state
    //
    // assume int 32 bits or more
    //======================================
    bool zero1s() const
    {
        // pointer to data
        uint32_t *data = mData;

        // pointer to next integer past the ones we use
        uint32_t * const pOver = mData + mLastIntIndex + 1;

        do
        {
            if (*data++)
            {
                return false;
            }
        }
        while (data < pOver);

        return true;
    }

    //======================================
    //! return number of 1s
    //
    // assume int 32 bits or more
    //======================================
    unsigned count1s() const;

    //======================================
    //! return index of first 1
    //
    // or invalid index (number of bits or more) if none
    //
    // assume int 32 bits or more
    //======================================
    unsigned find_first() const;

    //======================================
    //! return index of last 1
    //
    // or invalid index (number of bits or more) if none
    //
    // assume int 32 bits or more
    //======================================
    unsigned find_last() const;

    //======================================
    //! return *this & other
    //
    /// same sizes or core!
    //======================================
    xpsBitVector operator&(const xpsBitVector& other) const;

    //======================================
    //! & yourself with other
    //
    /// return yourself
    //
    /// same sizes or core!
    //======================================
    xpsBitVector& operator&=(const xpsBitVector& other);

    //======================================
    //! return *this + other
    //
    /// same sizes or core!
    //======================================
    xpsBitVector operator+(const xpsBitVector& other) const;


    //======================================
    //! | yourself with other
    //
    /// return yourself
    //
    /// same sizes or core!
    //======================================
    xpsBitVector& operator+=(const xpsBitVector& other);

    //======================================
    //! return *this | other
    //
    /// same sizes or core!
    //======================================
    xpsBitVector operator|(const xpsBitVector& other) const;


    //======================================
    //! | yourself with other
    //
    /// return yourself
    //
    /// same sizes or core!
    //======================================
    xpsBitVector& operator|=(const xpsBitVector& other);


    //======================================
    //! return *this ^ other
    //
    /// same sizes or core!
    //======================================
    xpsBitVector operator^(const xpsBitVector& other) const;


    //======================================
    //! ^ yourself with other
    //
    /// return yourself
    //
    /// same sizes or core!
    //======================================
    xpsBitVector& operator^=(const xpsBitVector& other);


    //======================================
    //! if == with other
    //======================================
    bool operator==(const xpsBitVector& other) const;

    //======================================
    //! if != with other
    //======================================
    bool operator!=(const xpsBitVector& other) const
    {
        return !((*this)==other);
    }


    //======================================
    //! set to all bits 1
    //
    // we are assuming little endian target
    //======================================
    void set();


    //======================================
    //! reset to all bits 0
    //
    // we are assuming little endian target
    //======================================
    void reset()
    {
        mData[mLastIntIndex] = 0;
        memset(mData, 0, mBytes);
    }


    //======================================
    //! number of bits in the array
    //======================================
    unsigned size() const
    {
        return mBits;
    }


    //======================================
    //! convert to 32-bit unsigned
    //
    /// assume the vector has at most 32 bits
    /// pad result with 0s if necessary
    //
    /// xpThrow((std::range_error(<this file name and line>))) if vector is more
    /// than 32 bits
    //
    // here we assume 32 or more bit target machine
    //======================================
    uint32_t toUint32() const
    {
        if (mBits > 32)
        {
            xpThrow((range_error(_FILE_LINE)));
        }

        return (le32toh((*(uint32_t*)mData)));
    }


    //======================================
    //! convert to 64-bit unsigned
    //
    /// assume the vector has at most 64 bits
    /// pad result with 0s if necessary
    //
    /// xpThrow((std::range_error(<this file name and line>))) if vector is more
    /// than 64 bits
    //
    // assume 32 or 64 bit target
    //======================================
    uint64_t toUint64() const
    {
        if (mBits > 64)
        {
            xpThrow((range_error(_FILE_LINE)));
        }

        return toUint64(mBits - 1, 0);
    }


    //======================================
    //! copy data starting from a uint8_t * to the beginning of our array
    //
    /// \param src copy from that
    //
    /// \param bits number of bits to copy
    ///
    /// xpThrow((std::range_error(<this file name and line>))) if our vector
    /// does not have enough size
    //
    /// assuming little endian target
    //======================================
    void setData(const uint8_t *src, unsigned bits)
    {
        setDataToRange(0, bits, src);
    }

    void setData(const uint16_t *src, unsigned bits)
    {
        setDataToRange(0, bits, src);
    }
    //======================================
    //! copy data starting from  a uint32_t * to the beginning of our array
    //
    /// \param src copy from that
    //
    /// \param bits number of bits to copy
    //
    /// xpThrow((std::range_error(<this file name and line>))) if our vector
    /// does not have enough size
    //
    /// memory after src is arranged in little-endian fashion
    /// and we assume little endian target
    //======================================
    void setData(const uint32_t *src, unsigned bits)
    {
        setDataToRange(0, bits, src);
    }


    void setData(const uint64_t *src, unsigned bits)
    {
        setDataToRange(0, bits, src);
    }

    void setDataToRange(unsigned lo, unsigned bits, const uint8_t *src);

    //======================================
    //! copy data starting from  a uint16_t * to inside our array starting at bit lo
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
    // assuming little-endian target
    //======================================
    void setDataToRange(unsigned lo, unsigned bits, const uint16_t *src);

    //======================================
    //! copy data starting from  a uint32_t * to inside our array starting at bit lo
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
    /// memory after src is arranged in little-endian fashion
    /// here we assume little endian target
    //======================================
    void setDataToRange(uint32_t lo, uint32_t bits, const uint32_t *src);

    //======================================
    //! copy data starting from  a uint64_t * to inside our array starting at bit lo
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
    /// memory after src is arranged in little-endian fashion
    /// here we assume little endian target
    //======================================
    void setDataToRange(unsigned lo, unsigned bits, const uint64_t *src);

    //======================================
    //! return pointer to bit vector instance with bit n boolean equal to arr[n]
    //
    // T is an array type of integers
    //
    // \param bits number of bits
    // \param arr array of integer type, to convert to bit vector
    template <typename T> static xpsBitVector* create(unsigned bits, T arr);


    //======================================
    //! convert range lo through hi, to (lowest bits of) uint32_t
    //
    // pad with 0s if necessary
    //
    // we need here that int size is at least 32 bits
    //======================================
    uint32_t toUint32(unsigned hi, unsigned lo) const;


    //======================================
    //! convert range lo through hi, to (lowest bits of) uint64_t
    //
    // pad with 0s if necessary
    //
    // little endian target needed here
    //======================================
    uint64_t toUint64(unsigned hi, unsigned lo) const;

    //======================================
    //! return pointer data as words (internal representation)
    //======================================
    const unsigned * getWordData()
    {
        return mData;
    }


private:

    uint32_t convert_u64_to_u32le(uint32_t *u32p, void *u64p, uint32_t bits2cpy);
    // balance speed and size
    // store data needed in a somewhat redundant manner

    // actual data storage of our array of bits
    // bits are stored consecutively beginning at
    // bit 0 stored at the location *mData & 0x1U
    // (so "endianness" is currently determined by target, not hardware)
    //
    // data is stored from mData[0] to mData[mLastIntIndex]
    // with the last integer using the first mLastIntBits :
    // from 0x1U << 0 to 0x1U << (mLastIntBits - 1)
    // the rest of its bits are filled with 0s
    Generic::StackObjAllocator<uint32_t, 5> mStorage;

    uint32_t *mData;
    unsigned mLastIntIndex;
    unsigned mLastIntBits;

    // mask cover the bits used in the last integer
    unsigned mLastIntMask;

    // number of bits
    unsigned mBits;

    // number of bytes
    unsigned mBytes;

    // initialize all data members based on bits
    // including allocate mData (but do not initialize inside mData)
    void init(unsigned bits);

    // flip all bits and store them in *dest
    // dest could be the same as mData or disjoint
    // but not overlap
    void flipTo(unsigned *dest) const;

    // copy bits
    // from *src to
    // *dest bit range from destLo through destHi bit positions
    // the source bits start inside *src at bit offset srcLo
    // such that intBitShift is (int)destLo - (int)srcLo
    //
    // destination bits occupy space completely inside *dest
    // but source bits can go to next integer after src
    // (can only go to one extra integer)
    static void cpBitsTo1Int(unsigned *dest, unsigned destLo, unsigned destHi,
                             unsigned *src, int intBitShift)
    {

        // mask to write bits to in the destination int
        const unsigned mask = RANGE_BITS(destLo, destHi);

        assert(destHi >= destLo);
        WRITE_MASK_BITS(
            dest,
            destHi - intBitShift < INT_BITS ?
            SHIFT_INT(le32toh(*src), intBitShift) :
            le32toh((*src)) >> -intBitShift | le32toh((*(src + 1))) <<
            (INT_BITS + intBitShift),
            mask);
    }


    // copy bits
    // from *src
    // to *dest bit range from destLo through destHi bit positions
    // destination bits do not have to start inside the first element of the array: *dest -
    // they can be further
    // the source bits start do start inside the first element of the array: *src
    // at bit offset srcLoIntBit (and can extend further)
    // destination can overlap with source, as long as the destination is "lower"
    // that is, if the overlap is a beginning part of source
public:
    static void cpBits(unsigned *dest, unsigned destLo, unsigned destHi,
                       unsigned *src, int srcLoIntBit);
    static int lshift_u32le(uint32_t *hdst, uint32_t d32s, const uint32_t *hsrc,
                            uint32_t s32s, uint32_t lshift);
    static int rshift_u32le(uint32_t *hdst, uint32_t d32s, const uint32_t *hsrc,
                            uint32_t s32s, uint32_t rshift);
    static uint32_t get1mask_u32host(uint32_t hi, uint32_t lo);
};


// for arguments:
//
// unsigned val
//
// array of bits in little-endian fashion, starting with bit 0 at *src & 1U :
// uint8_t *src
//
// unsigned lo_begin
// unsigned bits
// char* inc_string
//
// assign to little endian unsigned val ,
// bits bits from array src, starting at bit lo_begin
//
// assume lo_begin < 8,
// so that the bits assigned from, start in *src
// and src is uniquely determined
//
// assume bits bits fits in the unsigned integer
//
// if boolean advance :
// advance src to cover the next bit after
// the ones assigned here
// and establish lo_end so the next call to this macro can use
// lo_begin equal to this lo_end
//
// val , src, lo_begin appear multiple times and you must
// have them evaluated, not expressions

// we loop over uint8_t entries to successively copy them
//
// unsigned over is bit index one more than the last bit to copy
// if 8 < over there is more than one uint8_t to copy from
// we copy the first one
// then middle ones if any
// then last one if needed
//
// middle is the number of middle uint8_t entries to copy
// shift is how much up we shift the current uint8_t to the destination integer
//
// otherwise there is only one uint8_t to copy from, one step

#define UINT_BITS_ARR_ADVANCE_PLAIN(val, src, lo_begin, lo_end, bits, advance) \
    { \
        unsigned over = lo_begin + (bits); \
        unsigned over8 = over & (8 - 1); \
        \
        if (8 < over) { \
            unsigned middle = (over - 9) >> 3; \
            unsigned shift = 8 - lo_begin; \
            \
            val = *src++ >> lo_begin; \
            while (middle--) { \
                val |= *src++ << shift; \
                shift += 8; \
            } \
            val |= (*src & LOWER_BITS_LOW(over8 ? over8 : 8)) << shift; \
        } \
        else \
            val = (*src & LOWER_BITS_LOW(over)) >> lo_begin; \
        \
        if (advance) { \
            if (!over8) \
                src++; \
            lo_end = over8; \
        } \
    }


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define UINT_BITS_ARR_ADVANCE UINT_BITS_ARR_ADVANCE_PLAIN

#else

#define UINT_BITS_ARR_ADVANCE(val, src, lo_begin, lo_end, bits, advance) \
    { \
        unsigned over = lo_begin + (bits); \
        unsigned over8 = over & (8 - 1); \
        int x = 0; \
        uint8_t *src1 = src; \
        src = src1 + ((x / 4)*4) - (x % 4) + 3; \
        \
        if (8 < over) { \
            unsigned middle = (over - 9) >> 3; \
            unsigned shift = 8 - lo_begin; \
            \
            val = *src >> lo_begin; \
            x++; \
            src = src1 + ((x / 4)*4) - (x % 4) + 3; \
            while (middle--) { \
                val |= *src << shift; \
                x++; \
                src = src1 + ((x / 4)*4) - (x % 4) + 3; \
                shift += 8; \
            } \
            val |= (*src & LOWER_BITS_LOW(over8 ? over8 : 8)) << shift; \
        } \
        else \
            val = (*src & LOWER_BITS_LOW(over)) >> lo_begin; \
        \
        if (advance) { \
            if (!over8) \
                x++; \
                src = src1 + ((x / 4)*4) - (x % 4) + 3; \
            lo_end = over8; \
        } \
    }

#endif

#endif /* XP_BIT_VECTOR_H_ */
