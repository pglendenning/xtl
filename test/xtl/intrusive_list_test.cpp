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

// Author Paul Glendenning

#include <test.h>
#include <xtl/unordered_vector_set.hpp>
#include <xtl/intrusive_list.hpp>

using namespace xtl;
using namespace std;

namespace { 
// ----------------------------------------------------------------------------

template<class T> static int listsize(const T&) { return 1; }

REGISTER_TEST(INTRUSIVE_LIST)
{
    static const unsigned N = 8*1024;
    static const unsigned ITER = 64*1024;
    vector< intrusive_list_item<int> >  storage1(N);
    vector< intrusive_list_item<int*> > storage2(N);
    vector< intrusive_list_item<const int*> > storage3(N);
    // Use unordered_vector_set to check which items are free 
    unordered_vector_set<unsigned> check_free(N);
    unordered_vector_set<unsigned> check_used(N);

    intrusive_list<int>  lst1;
    intrusive_list<int*> lst2;
    intrusive_list<const int*> lst3;

    // Initialize
    for (unsigned i=0; i<N; ++i)
    {
        storage1[i].assign(i);
        storage2[i].assign(storage1[i].pointer_cast());
        storage3[i].assign(storage1[i].pointer_cast());
        check_free.insert(i);
        TEST_ASSERT(&storage1[i] == intrusive_list_item<int>::value2item_cast(storage1[i].pointer_cast()));
    }

    srand(5417);	// Make output predicable independent of test order
    for (unsigned i=0; i<ITER; ++i)
    {
        double action = double(rand())/RAND_MAX;
        if ((action > 0.67 && !check_free.empty()) || check_used.size() < 2)
        {
            // insert
            int r = rand() % (int)check_free.size();	// get another random int [0,check_free.size())
            r = *(check_free.begin()+r);
            check_free.erase(r);

            TEST_ASSERT(!storage1[r].is_linked());
            TEST_ASSERT(!storage2[r].is_linked());
            TEST_ASSERT(!storage3[r].is_linked());

            action = double(rand())/RAND_MAX;
            if (action > 0.5)
            {
                // insert before
                if (check_used.size() == 0)
                {
                    lst1.insert_before(lst1.end(), &storage1[r]);
                    lst2.insert_before(lst2.end(), &storage2[r]);
                    lst3.insert_before(lst3.end(), &storage3[r]);
                }
                else
                {
                    int k = rand() % (int)check_used.size();
                    k = *(check_used.begin()+k);
                    TEST_ASSERT(storage1[k].is_linked());
                    TEST_ASSERT(storage2[k].is_linked());
                    TEST_ASSERT(storage3[k].is_linked());
                    lst1.insert_before(&storage1[k], &storage1[r]);
                    lst2.insert_before(&storage2[k], &storage2[r]);
                    lst3.insert_before(&storage3[k], &storage3[r]);
                }
            }
            else
            {
                // insert after
                if (check_used.size() == 0)
                {
                    lst1.insert_after(lst1.end(), &storage1[r]);
                    lst2.insert_after(lst2.end(), &storage2[r]);
                    lst3.insert_after(lst3.end(), &storage3[r]);
                }
                else
                {
                    int k = rand() % (int)check_used.size();
                    k = *(check_used.begin()+k);
                    TEST_ASSERT(storage1[k].is_linked());
                    TEST_ASSERT(storage2[k].is_linked());
                    TEST_ASSERT(storage3[k].is_linked());
                    lst1.insert_after(&storage1[k], &storage1[r]);
                    lst2.insert_after(&storage2[k], &storage2[r]);
                    lst3.insert_after(&storage3[k], &storage3[r]);
                }
            }
            TEST_ASSERT(storage1[r].is_linked());
            TEST_ASSERT(storage2[r].is_linked());
            TEST_ASSERT(storage3[r].is_linked());
            check_used.insert(r);
        }
        else if (action > 0.33)
        {
            // erase at iterator
            int r = rand() % (int)check_used.size();	// get another random int [0,check_used.size())
            r = *(check_used.begin()+r);
            check_used.erase(r);
            check_free.insert(r);

            lst1.erase(lst1.cast_it(&storage1[r]));
            lst2.erase(lst2.cast_it(&storage2[r]));
            lst3.erase(lst3.cast_it(&storage3[r]));
        }
        else
        {
            // erase iterator range
            int b = rand() % int(check_used.size()-1);
            int e = rand() % int(check_used.size()-1);
            int x;

            if (b > e) swap(b, e);
            x = *(check_used.begin()+b);

            intrusive_list<int>::iterator first1 = lst1.cast_it(&storage1[x]);
            intrusive_list<int*>::iterator first2 = lst2.cast_it(&storage2[x]);
            intrusive_list<const int*>::iterator first3 = lst3.cast_it(&storage3[x]);
            intrusive_list<int>::iterator last1  = first1;
            intrusive_list<int*>::iterator last2  = first2;
            intrusive_list<const int*>::iterator last3  = first3;
            for (x = 0; last1 != lst1.end() && b < e; ++b, ++last1, ++last2, ++last3, ++x)
            {
                check_used.erase(*last1);
                check_free.insert(*last1);
            }

            lst1.erase(first1, last1);
            lst2.erase(first2, last2);
            lst3.erase(first3, last3);
        }
        int c1 = count_if(lst1.begin(), lst1.end(), listsize<int>);
        int u1 = (int)check_used.size();
        TEST_ASSERT(count_if(lst1.begin(), lst1.end(), listsize<int>) == (int)check_used.size());
        TEST_ASSERT(count_if(lst2.begin(), lst2.end(), listsize<int>) == (int)check_used.size());
        TEST_ASSERT(count_if(lst3.begin(), lst3.end(), listsize<int>) == (int)check_used.size());
        TEST_ASSERT(std::equal(lst1.begin(), lst1.end(), lst2.begin()));
        TEST_ASSERT(std::equal(lst1.begin(), lst1.end(), lst3.begin()));

        // Verify iteration
        intrusive_list<int>::iterator i1 = lst1.begin();
        intrusive_list<int*>::iterator i2 = lst2.begin();
        intrusive_list<const int*>::iterator i3 = lst3.begin();
        unsigned count = 0;
        while (i1 != lst1.end() && i2 != lst2.end() && i3 != lst3.end())
        {
            TEST_ASSERT(*i1 == *i2);
            TEST_ASSERT(*i1 == *i3);
            ++i1;
            ++i2;
            ++i3;
            ++count;
        }
        TEST_ASSERT(i1 == lst1.end() && i2 == lst2.end() && i3 == lst3.end());
        unsigned listSize = count;

        // Verify iteration
        intrusive_list<int>::reverse_iterator r1(i1);
        intrusive_list<int*>::reverse_iterator r2(i2);
        intrusive_list<const int*>::reverse_iterator r3(i3);
        while (r1 != lst1.rend() && r2 != lst2.rend() && r3 != lst3.rend())
        {
            TEST_ASSERT(*r1 == *r2);
            TEST_ASSERT(*r1 == *r3);
            ++r1;
            ++r2;
            ++r3;
            --count;
        }
        TEST_ASSERT(r1 == lst1.rend() && r2 == lst2.rend() && r3 == lst3.rend());
        TEST_ASSERT(count == 0);

        // Verify iteration
        i1 = r1.base();
        i2 = r2.base();
        i3 = r3.base();
        while (i1 != lst1.end() && i2 != lst2.end() && i3 != lst3.end())
        {
            TEST_ASSERT(*i1 == *i2);
            TEST_ASSERT(*i1 == *i3);
            i1 = i1++;	++i1;
            i2 = i2++;	++i2;
            i3 = i3++;	++i3;
            ++count;
        }
        TEST_ASSERT(i1 == lst1.end() && i2 == lst2.end() && i3 == lst3.end());
        TEST_ASSERT(count == listSize);

        r1 = intrusive_list<int>::reverse_iterator(i1);
        r2 = intrusive_list<int*>::reverse_iterator(i2);
        r3 = intrusive_list<const int*>::reverse_iterator(i3);
        while (r1 != lst1.rend() && r2 != lst2.rend() && r3 != lst3.rend())
        {
            TEST_ASSERT(*r1 == *r2);
            TEST_ASSERT(*r1 == *r3);
            r1 = r1++;	++r1;
            r2 = r2++;	++r2;
            r3 = r3++;	++r3;
            --count;
        }
        TEST_ASSERT(r1 == lst1.rend() && r2 == lst2.rend() && r3 == lst3.rend());
        TEST_ASSERT(count == 0);

        count = 0;
        for (intrusive_list<int>::iterator it = lst1.begin(); it != lst1.end(); ++it)
            ++count;
        TEST_ASSERT(lst1.size() == count);

        count = 0;
        for (intrusive_list<int*>::iterator it = lst2.begin(); it != lst2.end(); ++it)
            ++count;
        TEST_ASSERT(lst2.size() == count);

        count = 0;
        for (intrusive_list<const int*>::iterator it = lst3.begin(); it != lst3.end(); ++it)
            ++count;
        TEST_ASSERT(lst3.size() == count);
    }
}

REGISTER_TEST(INTRUSIVE_UNLINKABLE_LIST)
{
    static const unsigned N = 8*1024;
    static const unsigned ITER = 64*1024;
    vector< intrusive_list_unlinkable_item<int> >  storage1(N);
    vector< intrusive_list_unlinkable_item<int*> > storage2(N);
    vector< intrusive_list_unlinkable_item<const int*> > storage3(N);
    // Use unordered_vector_set to check which items are free 
    unordered_vector_set<unsigned> check_free(N);
    unordered_vector_set<unsigned> check_used(N);

    intrusive_list<int, intrusive_list_unlinkable_item<int> >  lst1;
    intrusive_list<int*, intrusive_list_unlinkable_item<int*> > lst2;
    intrusive_list<const int*, intrusive_list_unlinkable_item<const int*> > lst3;

    // Initialize
    for (unsigned i=0; i<N; ++i) {
        storage1[i].assign(i);
        storage2[i].assign(storage1[i].pointer_cast());
        storage3[i].assign(storage1[i].pointer_cast());
        check_free.insert(i);
        TEST_ASSERT(&storage1[i] == intrusive_list_unlinkable_item<int>::value2item_cast(storage1[i].pointer_cast()));
    }

    srand(5417);	// Make output predicable independent of test order
    for (unsigned i=0; i<ITER; ++i)
    {
        double action = double(rand())/RAND_MAX;
        if ((action > 0.67 && !check_free.empty()) || check_used.size() < 2)
        {
            // insert
            int r = rand() % (int)check_free.size();	// get another random int [0,check_free.size())
            r = *(check_free.begin()+r);
            check_free.erase(r);

            TEST_ASSERT(!storage1[r].is_linked());
            TEST_ASSERT(!storage2[r].is_linked());
            TEST_ASSERT(!storage3[r].is_linked());

            action = double(rand())/RAND_MAX;
            if (action > 0.5)
            {
                // insert before
                if (check_used.size() == 0)
                {
                    lst1.insert_before(lst1.end(), &storage1[r]);
                    lst2.insert_before(lst2.end(), &storage2[r]);
                    lst3.insert_before(lst3.end(), &storage3[r]);
                }
                else
                {
                    int k = rand() % (int)check_used.size();
                    k = *(check_used.begin()+k);
                    TEST_ASSERT(storage1[k].is_linked());
                    TEST_ASSERT(storage2[k].is_linked());
                    TEST_ASSERT(storage3[k].is_linked());
                    lst1.insert_before(&storage1[k], &storage1[r]);
                    lst2.insert_before(&storage2[k], &storage2[r]);
                    lst3.insert_before(&storage3[k], &storage3[r]);
                }
            }
            else
            {
                // insert after
                if (check_used.size() == 0)
                {
                    lst1.insert_after(lst1.end(), &storage1[r]);
                    lst2.insert_after(lst2.end(), &storage2[r]);
                    lst3.insert_after(lst3.end(), &storage3[r]);
                }
                else
                {
                    int k = rand() % (int)check_used.size();
                    k = *(check_used.begin()+k);
                    TEST_ASSERT(storage1[k].is_linked());
                    TEST_ASSERT(storage2[k].is_linked());
                    TEST_ASSERT(storage3[k].is_linked());
                    lst1.insert_after(&storage1[k], &storage1[r]);
                    lst2.insert_after(&storage2[k], &storage2[r]);
                    lst3.insert_after(&storage3[k], &storage3[r]);
                }
            }
            TEST_ASSERT(storage1[r].is_linked());
            TEST_ASSERT(storage2[r].is_linked());
            TEST_ASSERT(storage3[r].is_linked());
            check_used.insert(r);
        }
        else if (action > 0.33)
        {
            // erase at iterator
            int r = rand() % (int)check_used.size();	// get another random int [0,check_used.size())
            r = *(check_used.begin()+r);
            check_used.erase(r);
            check_free.insert(r);

            lst1.erase(lst1.cast_it(&storage1[r]));
            lst2.erase(lst2.cast_it(&storage2[r]));
            lst3.erase(lst3.cast_it(&storage3[r]));
        }
        else
        {
            // erase iterator range
            int b = rand() % int(check_used.size()-1);
            int e = rand() % int(check_used.size()-1);
            int x;

            if (b > e) swap(b, e);
            x = *(check_used.begin()+b);

            intrusive_list<int, intrusive_list_unlinkable_item<int> >::iterator first1 = lst1.cast_it(&storage1[x]);
            intrusive_list<int*, intrusive_list_unlinkable_item<int*> >::iterator first2 = lst2.cast_it(&storage2[x]);
            intrusive_list<const int*, intrusive_list_unlinkable_item<const int*> >::iterator first3 = lst3.cast_it(&storage3[x]);
            intrusive_list<int, intrusive_list_unlinkable_item<int> >::iterator last1  = first1;
            intrusive_list<int*, intrusive_list_unlinkable_item<int*> >::iterator last2  = first2;
            intrusive_list<const int*, intrusive_list_unlinkable_item<const int*> >::iterator last3  = first3;
            for (x = 0; last1 != lst1.end() && b < e; ++b, ++last1, ++last2, ++last3, ++x)
            {
                check_used.erase(*last1);
                check_free.insert(*last1);
            }

            lst1.erase(first1, last1);
            lst2.erase(first2, last2);
            lst3.erase(first3, last3);
        }
        int c1 = count_if(lst1.begin(), lst1.end(), listsize<int>);
        int u1 = (int)check_used.size();
        TEST_ASSERT(count_if(lst1.begin(), lst1.end(), listsize<int>) == (int)check_used.size());
        TEST_ASSERT(count_if(lst2.begin(), lst2.end(), listsize<int>) == (int)check_used.size());
        TEST_ASSERT(count_if(lst3.begin(), lst3.end(), listsize<int>) == (int)check_used.size());
        TEST_ASSERT(std::equal(lst1.begin(), lst1.end(), lst2.begin()));
        TEST_ASSERT(std::equal(lst1.begin(), lst1.end(), lst3.begin()));

        // Verify iteration
        intrusive_list<int, intrusive_list_unlinkable_item<int> >::iterator i1 = lst1.begin();
        intrusive_list<int*, intrusive_list_unlinkable_item<int*> >::iterator i2 = lst2.begin();
        intrusive_list<const int*, intrusive_list_unlinkable_item<const int*> >::iterator i3 = lst3.begin();
        unsigned count = 0;
        while (i1 != lst1.end() && i2 != lst2.end() && i3 != lst3.end())
        {
            TEST_ASSERT(*i1 == *i2);
            TEST_ASSERT(*i1 == *i3);
            ++i1;
            ++i2;
            ++i3;
            ++count;
        }
        TEST_ASSERT(i1 == lst1.end() && i2 == lst2.end() && i3 == lst3.end());
        unsigned listSize = count;

        // Verify iteration
        intrusive_list<int, intrusive_list_unlinkable_item<int> >::reverse_iterator r1(i1);
        intrusive_list<int*, intrusive_list_unlinkable_item<int*> >::reverse_iterator r2(i2);
        intrusive_list<const int*, intrusive_list_unlinkable_item<const int*> >::reverse_iterator r3(i3);
        while (r1 != lst1.rend() && r2 != lst2.rend() && r3 != lst3.rend())
        {
            TEST_ASSERT(*r1 == *r2);
            TEST_ASSERT(*r1 == *r3);
            ++r1;
            ++r2;
            ++r3;
            --count;
        }
        TEST_ASSERT(r1 == lst1.rend() && r2 == lst2.rend() && r3 == lst3.rend());
        TEST_ASSERT(count == 0);

        // Verify iteration
        i1 = r1.base();
        i2 = r2.base();
        i3 = r3.base();
        while (i1 != lst1.end() && i2 != lst2.end() && i3 != lst3.end())
        {
            TEST_ASSERT(*i1 == *i2);
            TEST_ASSERT(*i1 == *i3);
            i1 = i1++;	++i1;
            i2 = i2++;	++i2;
            i3 = i3++;	++i3;
            ++count;
        }
        TEST_ASSERT(i1 == lst1.end() && i2 == lst2.end() && i3 == lst3.end());
        TEST_ASSERT(count == listSize);

        r1 = intrusive_list<int, intrusive_list_unlinkable_item<int> >::reverse_iterator(i1);
        r2 = intrusive_list<int*, intrusive_list_unlinkable_item<int*> >::reverse_iterator(i2);
        r3 = intrusive_list<const int*, intrusive_list_unlinkable_item<const int*> >::reverse_iterator(i3);
        while (r1 != lst1.rend() && r2 != lst2.rend() && r3 != lst3.rend())
        {
            TEST_ASSERT(*r1 == *r2);
            TEST_ASSERT(*r1 == *r3);
            r1 = r1++;	++r1;
            r2 = r2++;	++r2;
            r3 = r3++;	++r3;
            --count;
        }
        TEST_ASSERT(r1 == lst1.rend() && r2 == lst2.rend() && r3 == lst3.rend());
        TEST_ASSERT(count == 0);

    }

    for (unsigned n = 0; n < storage1.size(); ++n)
        storage1[n].unlink();
    TEST_ASSERT(lst1.begin() == lst1.end());

    for (unsigned n = 0; n < storage2.size(); ++n)
        storage2[n].unlink();
    TEST_ASSERT(lst2.begin() == lst2.end());

    for (unsigned n = 0; n < storage3.size(); ++n)
        storage3[n].unlink();
    TEST_ASSERT(lst3.begin() == lst3.end());
}
// ----------------------------------------------------------------------------
} // namespace
