#ifndef BITMAGIC_H_4F7A5EF1_8A3F_4046_B6C7_8F85008F781B
#define BITMAGIC_H_4F7A5EF1_8A3F_4046_B6C7_8F85008F781B
// Copyright (C) 2008-2016, Solidra LLC. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer. Redistributions in binary
// form must reproduce the above copyright notice, this list of conditions and
// the following disclaimer in the documentation and/or other materials provided
// with the distribution. Neither the name of the solidra LLC nor the names of
// its contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// Author:       Paul Glendenning
// Reference:    SWAR algorithms - http://aggregate.org/MAGIC

#ifdef  _MSC_VER
// Microsoft intrinsics
// VC2008 x64 has a bug - have to include math.h before intrin.h else we get an error
// for the ceil() definiton. Include math.h here to avoid this situation
#ifdef _WIN64
#include <math.h>
#endif
#include <intrin.h>
#include <emmintrin.h>
#pragma intrinsic(_BitScanReverse, _BitScanForward)
#ifdef _WIN64
#pragma intrinsic(_BitScanReverse64, _BitScanForward64)
#endif
#endif

#include <limits>
#include <type_traits>

namespace xtl {
template<class T> struct bitmagic;

//-----------------------------------------------------------------------------
// Generic bit iterator

template<typename T> class __bit_iterator {
public:
	__bit_iterator(): _mask(0) {}
	__bit_iterator(const __bit_iterator& it): _mask(it._mask) {}
	__bit_iterator(T m): _mask(m) {}
	__bit_iterator& operator = (const __bit_iterator& it) { _mask=it._mask; return *this; }
	__bit_iterator& operator ++() { _mask &= _mask-1; return *this; }
	unsigned operator * () const { return bitmagic<T>::tzc(_mask); }
	bool at_end() const { return _mask == 0; }
	T mask() const { return _mask; }
private:
	T _mask;
};

//-----------------------------------------------------------------------------
// For 8 and 16 bit widths its cheaper to count ones using this table

template<typename T> struct byte_table {
    static const T ones[256];
    static const T rev[256];
};

template<typename T> const T byte_table<T>::ones[256] = {
    0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
};

template<typename T> const T byte_table<T>::rev[256] = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

//-----------------------------------------------------------------------------
template<class T> struct bitmagic;

/// Bitmagic implemention common to all word sizes
template<class T, class BMagic>
struct __bitmagic_common {
    typedef T word_type;

    /// Return a word with the least significant bit of x set
    static word_type lsb(word_type x) noexcept {
        return (x^(x&(x-1)));
    }

    /// Return the number of leading one (non-zero) bits
    static size_t lnzc(word_type x) noexcept {
        return BMagic::lzc(~x);
    }

    /// Return true if the number is a power of 2 
	static bool is_pow2(word_type x) noexcept {
		return x && 0 == (x&(x-1));
	}

    /// Test if a bit is set
	static bool test(word_type x, size_t bitIndex) noexcept {
		return ((x >> bitIndex) & 1) != 0;
	}

	/// Return the minimum value 2^k >= x
	static word_type nextpow2(word_type x) noexcept {
        return static_cast<word_type>(1) << ceil_log2(x);
	}

    /// Return the log2 of x rounded toward zero.
    static size_t ceil_log2(word_type x) noexcept {
        return BMagic::floor_log2(x-1+x);
    }

	/// Sets bits from bitIndex for width bits.
	static word_type set(size_t bitIndex=0, size_t width=BMagic::word_bits) noexcept {
		if (bitIndex < BMagic::word_bits) {
            width = std::min(width, BMagic::word_bits-bitIndex);
			word_type value = (std::is_unsigned<word_type>::value)?
                        std::numeric_limits<word_type>::max():
                        static_cast<word_type>(-1);
			return ((value >> (BMagic::word_bits - width)) << bitIndex);
		}
		return 0;
	}
};


/// Generic 8 bit model - good for all compilers.
template<typename T>
struct __bitmagic8: public __bitmagic_common<T, __bitmagic8<T>> {
    typedef T                   word_type;
	typedef __bit_iterator<T>	iterator;
    static const size_t         word_bits  = 8;
    static const size_t         shift_size = 3;
    static const size_t         shift_mask = 0x7;
    static const T              _fnv_prime = 247;
    static const T              _fnv_offset = 123;
    static const byte_table<T>  _table;

    /// Variant on the FNV hash
    static size_t FNV_hash(T x) noexcept {
        T hash = _fnv_offset;
        hash ^= x;
        hash *= _fnv_prime;
        return size_t(hash);
    }

    /// Return the number of bits set
    static size_t ones(T x) noexcept {
        return _table.ones[x];
    }

    /// Reverse bit order
    static uint8_t reverse(T x) noexcept {
    #if 0
	    return static_cast<T>((((static_cast<uint64_t>(x)*0x80200802ULL)&0x0884422110ULL)*0x0101010101ULL)>>32);
    #else
        return _table.rev[x];
    #endif
    }

    /// Return a word with the most significant bit of x set
    static T msb(T x) noexcept {
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        return (x & ~(x >> 1));
    }

    /// Return the number of leading zero bits
    static size_t lzc(T x) noexcept {
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        return 8 - ones(x);
    }

    /// Return the number of trailing zero bits
    static size_t tzc(T x) noexcept {
        return x == 0? word_bits: word_bits - lzc(lsb(x)) - 1;
    }

    /// Return the log2 of x rounded toward zero.
    static size_t floor_log2(T x) noexcept {
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        return ones(x >> 1);
    }
};


/// Generic 16 bit model - good for all compilers.
template<typename T>
struct __bitmagic16: public __bitmagic_common<T, __bitmagic16<T>> {
    typedef T               word_type;
	typedef __bit_iterator<T> iterator;
    static const size_t     word_bits  = 16;
    static const size_t     shift_size = 4;
    static const size_t     shift_mask = 0x1F;
    static const T          _fnv_prime = 5051U;
    static const T          _fnv_offset = 7919U;
    
    /// Variant on the FNV hash
    static size_t FNV_hash(T x) noexcept {
        T hash = _fnv_offset;
        unsigned char* xp = reinterpret_cast<unsigned char*>(&x);
        hash ^= T(*xp++);
        hash *= _fnv_prime;
        hash ^= T(*xp++);
        hash *= _fnv_prime;
        return size_t(hash);
    }

    /// Return the number of bits set
    static size_t ones(T x) noexcept {
	#ifdef __GNUC__
		return __builtin_popcount(x);
	#else
        return __bitmagic8<uint8_t>::ones(static_cast<uint8_t>(x & 0xf)) +
            __bitmagic8<uint8_t>::ones(static_cast<uint8_t>((x>>8) & 0xf));
	#endif
    }

    /// Reverse bit order
    static T reverse(T x) noexcept {
        (static_cast<T>(__bitmagic8<uint8_t>::reverse(static_cast<uint8_t>(x & 0xf))) << 8) |
        static_cast<T>(__bitmagic8<uint8_t>::reverse(static_cast<uint8_t>((x>>8) & 0xf)));
    }

    /// Return a word with the most significant bit of x set
    static T msb(T x) noexcept {
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        return (x & ~(x >> 1));
    }

    /// Return the number of leading zero bits
    static size_t lzc(T x) noexcept {
	#ifdef _MSC_VER
		unsigned long b;
		return _BitScanReverse(&b,x)? T(15-b): 16;
	#elif defined(__GNUC__)
		return x == 0? 16: __builtin_clz(x);
	#else
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        return 16 - ones(x);
	#endif
    }

    // Return the number of trailing zero bits
    static size_t tzc(T x) noexcept {
	#ifdef _MSC_VER
		unsigned long b;
		return _BitScanForward(&b,x)? T(b): 16;
	#elif defined(__GNUC__)
		return x == 0? 16: __builtin_ctz(x);
	#else
        return ones(lsb(x)-1);
	#endif
    }

    /// Return the log2 of x rounded toward zero.
    static size_t floor_log2(T x) noexcept {
	#ifdef _MSC_VER
		unsigned long b;
		return _BitScanReverse(&b,x)? size_t(b): 0;
	#elif defined(__GNUC__)
		return x == 0? 0: size_t(15-__builtin_clz(x));
	#else
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        return ones(x >> 1);
	#endif
    }
};


/// Generic 32 bit model
template<typename T>
struct __bitmagic32: public __bitmagic_common<T, __bitmagic32<T>> {
    typedef T               word_type;
	typedef __bit_iterator<T> iterator;
    static const size_t     word_bits  = 32;
    static const size_t     shift_size = 5;
    static const size_t     shift_mask = 0x1F;
    static const T          _fnv_prime = 16777619U;
    static const T          _fnv_offset = 2166136261U;

    /// Variant on the FNV hash
    static size_t FNV_hash(T x) noexcept {
        T hash = _fnv_offset;
        unsigned char* xp = reinterpret_cast<unsigned char*>(&x);
        hash ^= T(*xp++);
        hash *= _fnv_prime;
        hash ^= T(*xp++);
        hash *= _fnv_prime;
        hash ^= T(*xp++);
        hash *= _fnv_prime;
        hash ^= T(*xp++);
        hash *= _fnv_prime;
        return size_t(hash);
    }

    /// Return the number of bits set
    static size_t ones(T x) noexcept {
	#ifdef __GNUC__
		return __builtin_popcount(x);
	#else
        x -= ((x >> 1) & 0x55555555);
        x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
        x = (((x >> 4) + x) & 0x0f0f0f0f);
        x += (x >> 8);
        x += (x >> 16);
        return (x & 0x0000003f);
	#endif
    }

    /// Reverse the bit order
    static T reverse(T x) noexcept {
	#if 0
	    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
	#else
        T y = 0x55555555;
        x = (((x >> 1) & y) | ((x & y) << 1));
        y = 0x33333333;
        x = (((x >> 2) & y) | ((x & y) << 2));
        y = 0x0f0f0f0f;
        x = (((x >> 4) & y) | ((x & y) << 4));
        y = 0x00ff00ff;
        x = (((x >> 8) & y) | ((x & y) << 8));
	#endif
        return((x >> 16) | (x << 16));
    }

    /// Return a word with the most significant bit of x set
    static T msb(T x) noexcept {
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        x |= (x >> 16);
        return (x & ~(x >> 1));
    }

    /// Return the number of leading zero bits
    static size_t lzc(T x) noexcept {
	#ifdef _MSC_VER
		unsigned long b;
		return _BitScanReverse(&b,x)? T(31-b): 32;
	#elif defined(__GNUC__)
		return x == 0? 32: __builtin_clz(x);
	#else
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        x |= (x >> 16);
        return 32 - ones(x);
	#endif
    }

    // Return the number of trailing zero bits
    static size_t tzc(T x) noexcept {
	#ifdef _MSC_VER
		unsigned long b;
		return _BitScanForward(&b,x)? T(b): 32;
	#elif defined(__GNUC__)
		return x == 0? 32: __builtin_ctz(x);
	#else
        return ones(lsb(x)-1);
	#endif
    }

    /// Return the log2 of x rounded toward zero.
    static size_t floor_log2(T x) noexcept {
	#ifdef _MSC_VER
		unsigned long b;
		return _BitScanReverse(&b,x)? size_t(b): 0;
	#elif defined(__GNUC__)
		return x == 0? 0: size_t(31-__builtin_clz(x));
	#else
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        x |= (x >> 16);
        return ones(x >> 1);
	#endif
    }
};


/// Generic 64 bit model
template<typename T>
struct __bitmagic64: public __bitmagic_common<T, __bitmagic64<T>> {
    typedef T               word_type;
	typedef __bit_iterator<T> iterator;
    static const size_t     word_bits  = 64;
    static const size_t     shift_size = 6;
    static const size_t     shift_mask = 0x3F;
    static const T          _fnv_prime = 1099511628211ULL;
    static const T          _fnv_offset = 14695981039346656037ULL;

    /// Variant on the FNV hash
    static size_t FNV_hash(T x) noexcept {
        T hash = _fnv_offset;
        unsigned char* xp = reinterpret_cast<unsigned char*>(&x);
        hash ^= T(*xp++);
        hash *= _fnv_prime;
        hash ^= T(*xp++);
        hash *= _fnv_prime;
        hash ^= T(*xp++);
        hash *= _fnv_prime;
        hash ^= T(*xp++);
        hash *= _fnv_prime;
        hash ^= T(*xp++);
        hash *= _fnv_prime;
        hash ^= T(*xp++);
        hash *= _fnv_prime;
        hash ^= T(*xp++);
        hash *= _fnv_prime;
        hash ^= T(*xp++);
        hash *= _fnv_prime;
        return size_t(hash);
    }

    /// Return the number of bits set
    static size_t ones(T x) noexcept {
	#ifdef __GNUC__
		return __builtin_popcountll(x);
	#else
        x -= ((x >> 1) & 0x5555555555555555ULL);
        x =  ((x >> 2) & 0x3333333333333333ULL) + (x & 0x3333333333333333ULL);
        x =  ((x >> 4) + x) & 0x0f0f0f0f0f0f0f0fULL;
        x += (x >> 8);
        x += (x >> 16);
        x += (x >> 32);
        return size_t(x & 0x0000007f);
	#endif
    }

    /// Return a word with the most significant bit of x set
    static T msb(T x) noexcept {
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        x |= (x >> 16);
        x |= (x >> 32);
        return (x & ~(x >> 1));
    }

    /// Return the number of leading zero bits
    static size_t lzc(T x) noexcept {
	#ifdef _MSC_VER
		unsigned long b;
		return _BitScanReverse64(&b,x)? T(63-b): 64;
	#elif defined(__GNUC__)
		return x == 0? 64: __builtin_clzll(x);
	#else
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        x |= (x >> 16);
        x |= (x >> 32);
        return 64 - ones(x);
	#endif
    }

    // Return the number of trailing zero bits
    static size_t tzc(T x) noexcept {
	#ifdef _MSC_VER
		unsigned long b;
		return _BitScanForward64(&b,x)? T(b): 64;
	#elif defined(__GNUC__)
		return x == 0? 64: __builtin_ctzll(x);
	#else
        return ones(lsb(x)-1);
	#endif
    }

    /// Return the log2 of x rounded toward zero.
    static size_t floor_log2(T x) noexcept {
	#ifdef _MSC_VER
		unsigned long b;
		return _BitScanReverse64(&b,x)? size_t(b): 0;
	#elif defined(__GNUC__)
		return x == 0? 0: size_t(63-__builtin_clzll(x));
	#else
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        x |= (x >> 16);
        x |= (x >> 32);
        return ones(x >> 1);
	#endif
    }
};


//-----------------------------------------------------------------------------
// Class specialization templates for 8, 16, 32 and 64 bit word sizes

template<class _Word> struct bitmagic { };

template<> struct bitmagic<char>: public __bitmagic8<char> { };
template<> struct bitmagic<unsigned char>: public __bitmagic8<unsigned char> { };
template<> struct bitmagic<short>: public __bitmagic16<short> { };
template<> struct bitmagic<unsigned short>: public __bitmagic16<unsigned short> { };
template<> struct bitmagic<int>: public __bitmagic32<int> { };
template<> struct bitmagic<unsigned int>: public __bitmagic32<unsigned int> { };
#ifdef _MSC_VER
template<> struct bitmagic<long>: public __bitmagic32<long> { };
template<> struct bitmagic<unsigned long>: public __bitmagic32<unsigned long> { };
#else
template<> struct bitmagic<long>: public __bitmagic64<long> { };
template<> struct bitmagic<unsigned long>: public __bitmagic64<unsigned long> { };
#endif
template<> struct bitmagic<long long>: public __bitmagic64<long long> { };
template<> struct bitmagic<unsigned long long>: public __bitmagic64<unsigned long long> { };
#if 0
template<> struct bitmagic<uint8_t>: public __bitmagic8<uint8_t> { };
template<> struct bitmagic<uint32_t>: public __bitmagic32<uint32_t> { };
template<> struct bitmagic<uint16_t>: public __bitmagic16<uint16_t> { };
template<> struct bitmagic<uint64_t>:public __bitmagic64<uint64_t> { };
template<> struct bitmagic<int8_t>: public __bitmagic8<int8_t> { };
template<> struct bitmagic<int32_t>: public __bitmagic32<int32_t> { };
template<> struct bitmagic<int16_t>: public __bitmagic16<int16_t> { };
template<> struct bitmagic<int64_t>:public __bitmagic64<int64_t> { };
#endif

//-----------------------------------------------------------------------------
// END DEFINITION
//
}       // namespace xtl
#endif  // BITMAGIC_H_4F7A5EF1_8A3F_4046_B6C7_8F85008F781B

