#ifndef UNORDERED_VECTOR_MAP_A43A3D50_D300_4C26_B5FA_00F662D1CE56
#define UNORDERED_VECTOR_MAP_A43A3D50_D300_4C26_B5FA_00F662D1CE56
// Copyright (C) 2008-2012, Solidra LLC. All rights reserved.
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

/// @file
/// @brief	Mapping class using vectors. Suitable for key domains which can be
///         safely bounded in memory using a vector class.
/// @author Paul Glendenning
/// @date

#include <vector>
#include <algorithm>
#include "property.hpp"

namespace xtl {
// ----------------------------------------------------------------------------

/// A unordered_vector_map is an unordered map using two vectors, one for the
/// key value pair and one to determine if the map entry exists. Map entries
/// are only constructed when inserted, in this sense the unordered_vector_map
/// models the concept of a uninitialized memory as described in \link uov_ref1
/// "[1]" \endlink. The key must be an integer type.
///
/// @param Key		The integer key.
/// @param T		The value type.
/// @param Alloc	The value type allocator.
/// @see	 unordered_vector_set<>.
/// @remarks The space complexity is O(N), where N is the maximum key. The time 
/// complexity for insert, erase, and find is O(1).
template<class Key, class T, class Alloc=std::allocator<std::pair<Key,T> > >
class unordered_vector_map
{
private:
public:
	typedef Key					key_type;
	typedef T					mapped_type;
	typedef std::pair<Key,T>    value_type;
	typedef value_type&			reference;
	typedef const value_type&	const_reference;
	typedef value_type*			pointer;
	typedef const value_type*	const_pointer;
	typedef std::vector<value_type,Alloc>				vector_type;
	typedef typename vector_type::iterator				iterator;
	typedef typename vector_type::const_iterator		const_iterator;
	typedef typename std::reverse_iterator<iterator>	reverse_iterator;
	typedef typename std::reverse_iterator<const_iterator> const_reverse_iterator;
	typedef bool (*key_compare)(const key_type& a, const key_type& b);
	typedef bool (*value_compare)(const value_type& a, const value_type& b);
private:

	/// @cond
	// The map uses uninitialized storage so avoid std::vector here.
    unsigned*					_map;
	// The number of elements in _map.
	size_t						_mapSize;
	// The data storage set
    vector_type					_set;
	// The allocator
	typename Alloc::template rebind<unsigned>::other _mapAllocator;

	void resize_map(size_t newSize)
	{
		if (newSize > _mapSize)
		{
			if (_mapSize) _mapAllocator.deallocate(_map, _mapSize);
			_map = _mapAllocator.allocate(newSize);
			_mapSize = newSize;
			unsigned j = 0;
			for (iterator i=_set.begin(); i!=_set.end(); ++i)
				_map[i->first] = j++;
		}
	}

	static bool vcompare(const value_type& a, const value_type& b)
	{
		return a.first < b.first;
	}

	static bool kcompare(const key_type& a, const key_type& b)
	{
		return a < b;
	}
	/// @endcond

	/// If the vector map is passed to modifying algorithms such as std::sort() or 
	/// std::remove_if() then the internal map will no longer be valid. This is
	/// not fatal but find(), insert(), erase(), and test(), may fail to execute
	/// the operation correctly  when a valid set entry exists.  A call to remap()
	/// after modification of the vector map is required to ensure the correct operation
	/// of these member functions.
	/// @remarks Complexity O(size()). The function always performs size() assignments.
	void remap()
	{
		unsigned j = 0;
		for (iterator i=_set.begin(); i!=_set.end(); ++i)
			_map[i->first] = j++;
	}

public:
	/// Create a unordered_vector_map with capacity for N elements.
	unordered_vector_map(size_t N=0): _map(0), _mapSize(0) { reserve(N); }
	unordered_vector_map(const unordered_vector_map& other):
		_map(0), _mapSize(0), _set(other._set)
	{
		resize_map(other._set.capacity());
	}

	/// Assignment
	unordered_vector_map& operator = (const unordered_vector_map& other)
	{
		_set = other._set;
		if (_mapSize) _mapAllocator.deallocate(_map, _mapSize);
		_mapSize = 0;
		_map = 0;
		resize_map(other._mapSize);
		return *this;
	}


	/// Reserve some space for the vector map. This avoids reallocs when inserting
	/// elements.
	void reserve(size_t capacity)
	{
		_set.reserve(capacity);
		resize_map(capacity);
	}

	/// Get the current storage reserve size. Use reserve() to set the maximum size
	/// of the unordered_vector_map.
	/// @return  The storage reserve size.
	size_t capacity() const { return _mapSize; }

	/// STL pattern compatible with std::map<>
	/// @return  The number of elements in the map.
    size_t size() const { return _set.size(); }

	/// STL pattern compatible with std::map<>
	/// @return  True is the map is empty.
	bool empty() const { return _set.empty(); }

	/// STL pattern compatible with std::map<>. All items in the map will be destroyed.
	/// @remarks Complexity O(N), where N=size().
    void clear()
    {
        _set.clear();
    }

	/// STL pattern compatible with std::map<>
	void swap(unordered_vector_map& other)
	{
		std::swap(_map, other._map);
		std::swap(_mapSize, other._mapSize);
		_set.swap(other._set);
	}

	/// @{
	/// STL iterator patterns compatible with std::map<>
	iterator begin() { return _set.begin(); }
	iterator end() { return _set.end(); }
	const_iterator begin() const { return _set.begin(); }
	const_iterator end() const { return _set.end(); }
	reverse_iterator rbegin() { return _set.rbegin(); }
	reverse_iterator rend() { return _set.rend(); }
	const_reverse_iterator rbegin() const { return _set.rbegin(); }
	const_reverse_iterator rend() const { return _set.rend(); }
	/// @}

	/// STL pattern compatible with std::map<>
	/// @remarks Complexity O(1). Worst case performs one of each: test(), memory
	/// reallocate, and vector<>.push_back().  If reserve is correctly set the
	/// memory reallocation can be avoided.
	std::pair<iterator, bool> insert(const value_type& p)
	{
		if (p.first < (key_type)_mapSize)
		{
			unsigned& x = _map[p.first];
			if (x >= _set.size() || _set[x].first != p.first)
			{
				x = _set.size();
				_set.push_back(p);
				return std::make_pair(_set.end()-1, true);
			}
			return std::make_pair(_set.begin()+x, false);
		}
		reserve(p.first+1);
		_map[p.first] = _set.size();
		_set.push_back(p);
		return std::make_pair(_set.end()-1, true);
	}

	/// STL pattern compatible with std::map<>
	/// @remarks Iterator is ignored
	/// @see insert(const value_type& p)
	iterator insert(iterator pos, const value_type& p)
	{
		(void)pos;
		return insert(p).first;
	}

	/// STL pattern compatible with std::map<>
	/// @see insert(const value_type& p)
	template <class InputIterator>
	void insert(InputIterator first, InputIterator last)
	{
		while (first != last)
		{
			insert(*first);
			++first;
		}
	}

	/// STL pattern compatible with std::map<>
	/// @remarks Complexity O(1)
    mapped_type& operator [](key_type key) 
    { 
		if (key < (key_type)_mapSize)
		{
			unsigned& x = _map[key];
			if (x >= _set.size() || _set[x].first != key)
			{
				x = _set.size();
				_set.push_back(std::make_pair(key,mapped_type()));
				return _set.back().second;
			}
			return _set[x].second;
		}
		reserve(key+1);
		_map[key] = _set.size();
		_set.push_back(std::make_pair(key,mapped_type()));
		return _set.back().second;
    }

	/// STL pattern compatible with std::map<>
	/// @remarks Complexity O(1)
    const mapped_type& operator [](key_type key) const
    { 
        assert(unsigned(key) < _mapSize);
		unsigned x = _map[key];
        assert(x < _set.size() && _set[x].first == key);
		return _set[x].second;
    }

	/// STL pattern compatible with std::map<>
	/// @remarks Complexity O(1). Worst case performs one test() and one
	/// iterator addition.
	iterator find(key_type key)
	{	
		if (unsigned(key) < _mapSize)
		{
			unsigned x = _map[key];
			return (x < _set.size() && _set[x].first == key)? _set.begin()+x: _set.end();
		}
		return _set.end();
	}

	/// STL pattern compatible with std::map<>
	/// @remarks Complexity O(1). Worst case performs one test() and one
	/// iterator addition.
	const_iterator find(key_type key) const
	{
		if (unsigned(key) < _mapSize)
		{
			unsigned x = _map[key];
			return (x < _set.size() && _set[x].first == key)? _set.begin()+x: _set.end();
		}
		return _set.end();
	}

	/// STL pattern compatible with std::map<>
	/// @remarks Complexity O(1). Worst case the function performs two iterator
	/// compares and one of each: assignment, swap, and vector<>.pop_back().
	void erase(iterator it)
	{
        if (it != end())
        {
            if (it != (end()-1))
            {
				// Exchange with back to preserve _set contiguity
                _map[ _set.back().first ] = (unsigned)(it - _set.begin());
				// Don't copy since it may be expensive for value_type.
                std::swap(*it, _set.back());
            }
            _set.pop_back();
        }
	}

	/// STL pattern compatible with std::map<>
	/// @remarks Complexity O(|last-first|). The function performs |last-first| of each:
	/// vector<>.pop_back(), iterator compare * 2, and iterator increment.
	void erase(iterator first, iterator last)
	{
		std::ptrdiff_t n = last - first;
		XTL_ITERATOR_ASSERT1(first <= last);
		if (first != last)
		{	
			// Move down
			reverse_iterator rfirst(last), rlast(first), rpos(rbegin());
			for (std::ptrdiff_t d=last-begin(); rfirst != rlast && rfirst != rend(); ++rfirst, ++rpos)
			{
				// Exchange with back to preserve _set contiguity
                _map[ rpos->first ] = (unsigned)--d;
				// Don't copy since it may be expensive for value_type.
                std::swap(*rfirst, *rpos);
			}
			_set.resize(_set.size() - n);
		}
	}

	/// STL pattern compatible with std::map<>
	/// @remarks Complexity O(1). Worst case the function performs one of each: test()
	/// assignment, compare, swap, and vector<>.pop_back().
    size_t erase(const key_type& key)
    {
		if (unsigned(key) < _mapSize)
		{
			unsigned x = _map[key];
			if (x < _set.size() && _set[x].first == key)
			{
				if (x != (_set.size()-1))
				{
					// Exchange with back to preserve _set contiguity
					_map[ _set.back().first ] = x;
					// Don't copy since it may be expensive for value_type.
					std::swap(_set[x], _set.back());
				}
				_set.pop_back();
			}
		}
		return size();
    }

	/// Check if an element exists in the map.
	/// @remarks Complexity O(1). Worst case the function performs three compares and
	/// one assignment.
    bool test(key_type key) const 
	{ 
		if (unsigned(key) < _mapSize)
		{
			unsigned x = _map[key];
			return (x < _set.size() && _set[x].first == key);
		}
		return false;
	}

	/// In order to use upper_bound, lower_bound, or xtl set operations a sort is required.
	void sort()
	{ 
		std::sort(_set.begin(), _set.end(), vcompare);
		remap();
	}

	/// Required for XTL set operations
	static key_compare key_comp() { return kcompare; }
	static value_compare value_comp() { return vcompare; }

	/// @{
	/// These functions can only be used after a sort
	iterator upper_bound() { return std::upper_bound(_set.begin(), _set.end(), vcompare); }
	const_iterator upper_bound() const { return std::upper_bound(_set.begin(), _set.end(), vcompare); }
	iterator lower_bound() { return std::lower_bound(_set.begin(), _set.end(), vcompare); }
	const_iterator lower_bound() const { return std::lower_bound(_set.begin(), _set.end(), vcompare); }
	/// @}
};

/// Vector map traits
template<class K, class T, class A>
struct container_traits<unordered_vector_map<K,T,A> >: public __map_traits<unordered_vector_map<K,T,A> >
{
	typedef associative_container_tag category;
	UNSUPPORTED_PROPERTY(allow_duplicate_keys);
	UNSUPPORTED_PROPERTY(sorted);
	typedef key_properties<K> key_props;
};

// ----------------------------------------------------------------------------
// END OF DECLARATIONS
//
}		// namespace xtl
#endif  // defined(UNORDERED_VECTOR_MAP_A43A3D50_D300_4C26_B5FA_00F662D1CE56)
