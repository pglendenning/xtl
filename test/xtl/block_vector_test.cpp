// Copyright (C) 2008-2016, Solidra LLC. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer. Redistributions in binary
// form must reproduce the above copyright notice, this list of conditions and
// the following disclaimer in the documentation and/or other materials provided
// with the distribution. Neither the name of the Solidra LLC nor the names of
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
#include <xtl/block_vector.hpp>

using namespace xtl;

namespace { 
// ----------------------------------------------------------------------------
struct TINT
{
    unsigned n;
    static unsigned cc;

    TINT(): n(0) 
    {
        ++cc;
    }
    TINT(const TINT& k): n(k.n) 
    {
        int xx = cc;
        ++cc;
    }
    ~TINT() { --cc; }
};
unsigned TINT::cc = 0;

REGISTER_TEST(BLOCK_VECTOR_TEST)
{
    block_vector<unsigned,std::allocator<unsigned>, 64> vec1;
    block_vector<TINT,std::allocator<TINT>, 64> vec2;

    vec1.resize(2048);
    vec2.resize(2048);
    for (unsigned i=1; i<=2048; ++i) {
        vec1[i-1] = i;
        vec2[i-1].n = i;
    }
    TEST_ASSERT(vec1.back() == 2048);
    TEST_ASSERT(vec1.front() == 1);
    TEST_ASSERT(vec2.back().n == 2048);
    TEST_ASSERT(vec2.front().n == 1);
    TEST_ASSERT(TINT::cc == 2048);

    for (unsigned i=vec1.size(); i > 1024; --i) {
        TEST_ASSERT(vec1.back() == i);
        TEST_ASSERT(vec1[i-1] == i);
        vec1.pop_back();
        vec2.pop_back();
    }
    TEST_ASSERT(vec1.size() == 1024);
    TEST_ASSERT(vec2.size() == 1024);
    TEST_ASSERT(TINT::cc == 1024);

    vec1.resize(993);
    vec2.resize(993);
    TEST_ASSERT(TINT::cc == 993);
    TEST_ASSERT(vec1.size() == 993);
    TEST_ASSERT(vec2.size() == 993);
    for (unsigned i=vec1.size(); i > 665; --i) {
        TEST_ASSERT(vec1.back() == i);
        TEST_ASSERT(vec1[i-1] == i);
        vec1.pop_back();
        vec2.pop_back();
    }
    TEST_ASSERT(vec1.size() == 665);
    TEST_ASSERT(vec2.size() == 665);
    TEST_ASSERT(TINT::cc == 665);

    for (unsigned i=0; i<100; ++i) {
        vec1.push_back(666+i);
        vec2.push_back();
    }
    TEST_ASSERT(vec1.size() == 765);
    TEST_ASSERT(vec2.size() == 765);
    TEST_ASSERT(TINT::cc == 765);

    unsigned k = 1;
    block_vector<unsigned, std::allocator<unsigned>, 64>::iterator it;
    for (it = vec1.begin(); it != vec1.end(); ++it) {
        TEST_ASSERT(*it == k);
        ++k;
    }

    TEST_ASSERT((vec1.end()-vec1.begin()) == 765);
    TEST_ASSERT((vec1.rend()-vec1.rbegin()) == 765);
    it = vec1.begin();
    it += 103;
    TEST_ASSERT(*it == 104);
    it -= 20;
    TEST_ASSERT(*it == 84);
    TEST_ASSERT((it - vec1.begin()) == 83); 
    TEST_ASSERT((vec1.begin() - it) == -83);
    TEST_ASSERT(it > vec1.begin());
    TEST_ASSERT(it < vec1.end());
    TEST_ASSERT((vec1.end()-it) == (765-83));
    TEST_ASSERT((it-vec1.end()) == -(765-83));
    TEST_ASSERT((vec1.end()-it) == (765-83));
    TEST_ASSERT((it-vec1.end()) == -(765-83));

    // Check iterator difference from endpoints
    for (unsigned n=0; n<vec1.size(); ++n) {
        unsigned& kb = vec1[n];
        unsigned& ke = vec1[765-n-1];
        TEST_ASSERT(kb == (n+1));
        TEST_ASSERT(ke == (765-n));
        unsigned& ie = *(vec1.end()-(n+1));
        TEST_ASSERT(ie == 765-n);
        TEST_ASSERT(*(vec1.begin()+n) == n+1);
        TEST_ASSERT(*(vec1.end()-n-1) == 765-n);
    }

    // Check sort works on block vectors
    std::sort(vec1.rbegin(), vec1.rend());
    TEST_ASSERT(vec1.size() == 765);
    for (unsigned n=0; n<vec1.size(); ++n) {
        unsigned& kb = vec1[n];
        unsigned& ke = vec1[765-n-1];
        TEST_ASSERT(ke == (n+1));
        TEST_ASSERT(kb == (765-n));
        TEST_ASSERT(*(vec1.end()-n-1) == n+1);
        TEST_ASSERT(*(vec1.begin()+n) == 765-n);
    }

    // Check iterator less
    TEST_ASSERT(vec1.begin() < vec1.end());
    TEST_ASSERT(vec1.begin()+764 < vec1.end());
    TEST_ASSERT(vec1.begin()+765 == vec1.end());
    TEST_ASSERT(vec1.begin()+765 > (vec1.end()-1));

    // Check pop back
    while (vec2.size())
        vec2.pop_back();

    // Check resize
    while (vec1.size())
        vec1.resize(vec1.size()-1);
}

// ----------------------------------------------------------------------------
} // namespace 
