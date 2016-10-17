// Copyright (C) 2008-2016, solidra LLC. All rights reserved.
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

#include <test.h>
#include <xtl/bitmagic.hpp>

using namespace xtl;
namespace { 
//-----------------------------------------------------------------------------

REGISTER_TEST(BITMAGIC64)
{
    uint64_t X = 0x000f0f0f00f0f0f0ULL;
    size_t ones = bitmagic<uint64_t>::ones(X);
    TEST_ASSERT(ones == 24);
    ones = bitmagic<uint64_t>::ones(0x000fffffffffffffULL);
    TEST_ASSERT(ones == 13*4);
    ones = bitmagic<uint64_t>::ones(0x0000000000000001ULL);
    TEST_ASSERT(ones == 1);
    ones = bitmagic<uint64_t>::ones(0x1000000000000001ULL);
    TEST_ASSERT(ones == 2);
    ones = bitmagic<uint64_t>::ones(0x1000000000000000ULL);
    TEST_ASSERT(ones == 1);
    ones = bitmagic<uint64_t>::ones(0x0000000100000000ULL);
    TEST_ASSERT(ones == 1);
    size_t lzc  = bitmagic<uint64_t>::lzc(X);
    TEST_ASSERT(lzc == 12);
    uint64_t lsb = bitmagic<uint64_t>::lsb(X);
    TEST_ASSERT(lsb == 0x10);
    uint64_t msb = bitmagic<uint64_t>::msb(X);
    TEST_ASSERT(msb == 0x0008000000000000ULL);
    size_t l2 = bitmagic<uint64_t>::floor_log2(X);
    TEST_ASSERT(l2 == 51);
}

REGISTER_TEST(BITMAGIC)
{
	// set tests
	TEST_ASSERT(bitmagic<unsigned char>::set() == 0xff);
	TEST_ASSERT(bitmagic<unsigned char>::set(0,40) == 0xff);
	TEST_ASSERT(bitmagic<unsigned char>::set(2) == 0xfc);
	TEST_ASSERT(bitmagic<unsigned char>::set(8) == 0x00);
	TEST_ASSERT(bitmagic<unsigned char>::set(0,3) == 0x07);
	TEST_ASSERT(bitmagic<unsigned char>::set(3,4) == 0x78);
	TEST_ASSERT(bitmagic<unsigned char>::set(8,40) == 0x00);
	TEST_ASSERT(bitmagic<unsigned>::set() == 0xffffffff);
	TEST_ASSERT(bitmagic<unsigned>::set(0,40) == 0xffffffff);
	TEST_ASSERT(bitmagic<unsigned>::set(8) == 0xffffff00);
	TEST_ASSERT(bitmagic<unsigned>::set(32) == 0x00000000);
	TEST_ASSERT(bitmagic<unsigned>::set(0,15) == 0x00007fff);
	TEST_ASSERT(bitmagic<unsigned>::set(3,15) == 0x0003fff8);
	TEST_ASSERT(bitmagic<unsigned>::set(32,40) == 0x00000000);
#if defined(_MSC_VER)
	TEST_ASSERT(bitmagic<unsigned long long>::set() == 0xffffffffffffffff);
	TEST_ASSERT(bitmagic<unsigned long long>::set(0,70) == 0xffffffffffffffff);
	TEST_ASSERT(bitmagic<unsigned long long>::set(8) == 0xffffffffffffff00);
	TEST_ASSERT(bitmagic<unsigned long long>::set(64) == 0x0000000000000000);
	TEST_ASSERT(bitmagic<unsigned long long>::set(0,40) == 0x000000ffffffffff);
	TEST_ASSERT(bitmagic<unsigned long long>::set(3,40) == 0x000007fffffffff8);
	TEST_ASSERT(bitmagic<unsigned long long>::set(64,70) == 0x0000000000000000);
#else
	TEST_ASSERT(bitmagic<unsigned long>::set() == 0xffffffffffffffff);
	TEST_ASSERT(bitmagic<unsigned long>::set(0,70) == 0xffffffffffffffff);
	TEST_ASSERT(bitmagic<unsigned long>::set(8) == 0xffffffffffffff00);
	TEST_ASSERT(bitmagic<unsigned long>::set(64) == 0x0000000000000000);
	TEST_ASSERT(bitmagic<unsigned long>::set(0,40) == 0x000000ffffffffff);
	TEST_ASSERT(bitmagic<unsigned long>::set(3,40) == 0x000007fffffffff8);
	TEST_ASSERT(bitmagic<unsigned long>::set(64,70) == 0x0000000000000000);
#endif

}

// ----------------------------------------------------------------------------
} 
