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

#include <cstdlib>
#include <set>
#include <test.h>
#include <xtl/unordered_vector_set.hpp>

using namespace xtl;

namespace { 
// ----------------------------------------------------------------------------

REGISTER_TEST(UNORDERED_VECTOR_SET_TEST)
{
    const unsigned N = 8*1024;
    const unsigned ITER = 64*1024;
    unordered_vector_set<int> vset(N);
    std::set<int> check;

    std::srand(5417);	// Make output predicable independent of test order
    for (unsigned i=0; i<ITER; ++i) {
        int r = std::rand();
        double action = double(r)/RAND_MAX;
        r = std::rand() & (N - 1);	// get another std::random int [0,N)

        if (action > 0.80 || vset.size() < 2) {
            // insert
            std::pair<std::set<int>::iterator, bool> sresult = check.insert(r);
            std::pair<unordered_vector_set<int>::iterator, bool> vresult = vset.insert(r);

            TEST_ASSERT(vresult.second == sresult.second);
            TEST_ASSERT(*vresult.first == *sresult.first);
        } else if (action > 0.60) {
            // erase at value compare
            check.erase(r);
            vset.erase(r);
            TEST_ASSERT(vset.find(r) == vset.end());
        } else if (action > 0.40){
            // erase iterator range
            int b = std::rand() % int(vset.size()-1);
            int e = std::rand() % int(vset.size()-1);

            if (b > e) std::swap(b,e);

            vset.sort();
            check.erase(check.find(*(vset.begin()+b)), check.find(*(vset.begin()+e)));

            vset.erase(vset.begin()+b, vset.begin()+e);
            vset.sort();

            TEST_ASSERT(vset.size() == check.size());
            TEST_ASSERT(std::equal(vset.begin(), vset.end(), check.begin()));
        } else if (action > 0.2) {
             // erase at iterator
            int n = *(vset.begin()+vset.size()/2);
            check.erase(n);
            vset.erase(vset.begin()+vset.size()/2);
            TEST_ASSERT(vset.size() == check.size());
            TEST_ASSERT(vset.find(n) == vset.end());
        } else {
            // find
            std::set<int>::iterator sit = check.find(r);
            unordered_vector_set<int>::iterator vit = vset.find(r);

            TEST_ASSERT((sit == check.end() && vit == vset.end()) ||
                        (sit != check.end() && vit != vset.end()));
            TEST_ASSERT(sit == check.end() || *sit == *vit);
        }
        TEST_ASSERT(vset.size() == check.size());
    }
    // Verify contents
    vset.sort();
    TEST_ASSERT(std::equal(vset.begin(), vset.end(), check.begin()));
    vset.clear();
    TEST_ASSERT(vset.begin() == vset.end());
}

// ----------------------------------------------------------------------------
} 
