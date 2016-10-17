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

#include <iostream>
#include <map>
#include <cstdlib>
#include <test.h>
#include <xtl/unordered_vector_map.hpp>
#include <xtl/unordered_block_vector_map.hpp>

using namespace xtl;

namespace {
// ----------------------------------------------------------------------------

bool compare(const std::pair<int,double>& a, const std::pair<int,double>& b)
{
	return a.first < b.first;
}

const unsigned N = 8*1024;
const unsigned ITER = 64*1024;

template<class unordered_map_type>
void TestMap(unordered_map_type& vset)
{
    typedef typename unordered_map_type::key_type key_type;
    typedef typename unordered_map_type::mapped_type mapped_type;

    std::map<key_type, mapped_type> check;

    std::srand(5417);	// Make output predicable independent of test order
    unsigned N8 = N*4/5;

    for (unsigned i=0; i<ITER; ++i) {
        int r = std::rand();
        double action = double(r)/RAND_MAX;
        r = std::rand() & (N - 1);	// get another std::random int [0,N)

        if ((action > 0.60 && vset.size() < N8) || vset.size() < 2) {
            // insert
            std::pair<typename std::map<key_type,mapped_type>::iterator, bool> sresult = check.insert(std::make_pair((const key_type)r,action));
            std::pair<typename unordered_map_type::iterator, bool> vresult = vset.insert(std::make_pair((const key_type)r,action));

            TEST_ASSERT(vresult.second == sresult.second);
            TEST_ASSERT(vresult.first->first == sresult.first->first);
            TEST_ASSERT(vresult.first->second == sresult.first->second);
        } else if (action > 0.50) {
            // erase at value compare
            check.erase(r);
            vset.erase(r);
            TEST_ASSERT(vset.find(r) == vset.end());
        } else if (action > 0.30) {
            // erase iterator range
            unsigned vs = vset.size();
            unsigned cs = check.size();
            int b = std::rand() % int(vset.size()-1);
            int e = std::rand() % int(vset.size()-1);

            if (b > e) std::swap(b,e);

            vset.sort();

            check.erase(check.find((vset.begin()+b)->first), check.find((vset.begin()+e)->first));

            vset.erase(vset.begin()+b, vset.begin()+e);
            vset.sort();

            TEST_ASSERT(vset.size() == check.size());
            TEST_ASSERT(std::equal(reinterpret_cast<std::pair<const key_type,mapped_type>*>(&(*vset.begin())), 
                        reinterpret_cast<std::pair<const key_type,mapped_type>*>(&(*(vset.end()-1))+1), check.begin()));
        } else if (action > 0.2) {
            // erase at iterator
            int n = (vset.begin()+vset.size()/2)->first;
            check.erase(n);
            vset.erase(vset.begin()+vset.size()/2);
            TEST_ASSERT(vset.size() == check.size());
             TEST_ASSERT(vset.find(n) == vset.end());
        } else {
            // find
            typename std::map<key_type,mapped_type>::iterator sit = check.find(r);
            typename unordered_map_type::iterator vit = vset.find(r);

            TEST_ASSERT((sit == check.end() && vit == vset.end()) ||
                        (sit != check.end() && vit != vset.end()));
            TEST_ASSERT(sit == check.end() || (sit->first == vit->first && sit->second == vit->second));
        }
        TEST_ASSERT(vset.size() == check.size());
    }
    // Verify contents
    std::sort(vset.begin(), vset.end());
    TEST_ASSERT(std::equal(reinterpret_cast<std::pair<const key_type,mapped_type>*>(&(*vset.begin())), 
                        reinterpret_cast<std::pair<const key_type,mapped_type>*>(&(*(vset.end()-1))+1), check.begin()));
    vset.clear();
    TEST_ASSERT(vset.begin() == vset.end());
}


REGISTER_TEST(UNORDERED_VECTOR_MAP)
{
    unordered_vector_map<int,double> vset(N);
    TestMap(vset);
}


REGISTER_TEST(UNORDERED_BLOCK_VECTOR_MAP)
{
    unordered_block_vector_map<int,double> vset;
    TestMap(vset);
}

// ----------------------------------------------------------------------------
} 

