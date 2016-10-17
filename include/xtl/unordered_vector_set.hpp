#ifndef	UNORDERED_VECTOR_SET_A9B22A84_6B3D_4180_BC3D_A8F45C942CE2
#define	UNORDERED_VECTOR_SET_A9B22A84_6B3D_4180_BC3D_A8F45C942CE2
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
/// @brief	Integer set class using vectors. Suitable for key domains which can be
///         safely bounded in memory using a vector class.
/// @author Paul Glendenning
/// @date

#include <vector>
#include <algorithm>
#include "property.hpp"

namespace xtl {
// ----------------------------------------------------------------------------

/// A unordered_vector_set is an unordered set of integers using two vectors, one
/// for the key and one to determine if the set entry exists. Set entries are only
/// constructed when inserted, in this sense the unordered_vector_set
/// models the concept of a uninitialized memory as described in \link uovs_ref1
/// "[1]" \endlink. The key must be an integer type.
///
/// @param T		The key type.
/// @param Alloc	Allocator function.
/// @see	 unordered_vector_map<>.
/// @remarks The space complexity is O(N), where N is the maximum key. The time 
/// complexity for insert, erase, and find is O(1).
///
template<class Key, class Alloc=std::allocator<Key> >
class unordered_vector_set
{
public:
	typedef const Key			key_type;
	typedef Key			        value_type;
	typedef key_type&	        reference;
	typedef key_type&	        const_reference;
	typedef key_type*	        pointer;
	typedef key_type*	        const_pointer;
	typedef std::vector<value_type,Alloc>				vector_type;
	typedef typename vector_type::const_iterator		iterator;
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
				_map[*i] = j++;
		}
	}

	static bool compare(const value_type& a, const value_type& b)
	{
		return a < b;
	}
	/// @endcond

	/// If the set is passed to modifying algorithms such as std::sort() or 
	/// std::remove_if() then the internal map will no longer be valid. This is
	/// not fatal but find(), insert(), erase(), and test(), may fail to execute
	/// the operation correctly  when a valid set entry exists.  A call to remap()
	/// after modification of the set is required to ensure the correct operation
	/// of these member functions.
	/// @remarks Complexity O(size()). The function always performs size() assignments.
	void remap()
	{
		unsigned j = 0;
		for (iterator i=_set.begin(); i!=_set.end(); ++i)
			_map[*i] = j++;
	}

public:
	/// Create a unordered_vector_set with capacity for N elements.
	unordered_vector_set(size_t N=0): _map(0), _mapSize(0) { reserve(N); }
	/// Copy an unordered_vector_set.
	unordered_vector_set(const unordered_vector_set& other):
		_map(0), _mapSize(0), _set(other._set)
	{
		resize_map(other._set.capacity());
	}

	/// Assignment
	unordered_vector_set& operator = (const unordered_vector_set& other)
	{
		_set = other._set;
		if (_mapSize) _mapAllocator.deallocate(_map, _mapSize);
		_mapSize = 0;
		_map = 0;
		resize_map(other._mapSize);
		return *this;
	}

	/// Reserve some space for the vector set. This avoids reallocs when inserting
	/// elements.
	void reserve(size_t capacity)
	{
		_set.reserve(capacity);
		resize_map(_set.capacity());
	}

	/// Get the current storage reserve size. Use reserve() to set the maximum size
	/// of the unordered_vector_set.
	/// @return  The storage reserve size.
	size_t capacity() const { return _mapSize; }

	/// STL pattern compatible with std::set<>
	/// @return  The number of elements in the set.
    size_t size() const { return _set.size(); }

	/// STL pattern compatible with std::set<>
	/// @return  True if the set is empty.
	bool empty() const { return _set.empty(); }

	/// STL pattern compatible with std::set<>. Remove all items.
	/// @remarks Complexity O(1). Same as std::vector<Key>.clear(), which should be O(1)
	/// for integers.
    void clear()
    {
        _set.clear();
    }

	/// STL pattern compatible with std::set<>
	void swap(unordered_vector_set& other)
	{
		std::swap(_map, other._map);
		std::swap(_mapSize, other._mapSize);
		_set.swap(other._set);
	}

	/// @{
	/// STL iterator patterns compatible with std::set<>
	iterator begin() { return _set.begin(); }
	iterator end() { return _set.end(); }
	const_iterator begin() const { return _set.begin(); }
	const_iterator end() const { return _set.end(); }
	reverse_iterator rbegin() { return _set.rbegin(); }
	reverse_iterator rend() { return _set.rend(); }
	const_reverse_iterator rbegin() const { return _set.rbegin(); }
	const_reverse_iterator rend() const { return _set.rend(); }
	/// @}

	/// STL pattern compatible with std::set<>
	/// @remarks Complexity O(1). Worst case performs one of each: test(), memory
	/// reallocate, and vector<>.push_back().  If reserve is correctly set the
	/// memory reallocation can be avoided.
	std::pair<iterator, bool> insert(const value_type& key)
	{
		if (key < (key_type)_mapSize)
		{
			unsigned& x = _map[key];
			if (x >= _set.size() || _set[x] != key)
			{
				x = _set.size();
				_set.push_back(key);
				return std::make_pair(_set.end()-1, true);
			}
			return std::make_pair(_set.begin()+x, false);
		}
		reserve(key+1);
		_map[key] = _set.size();
		_set.push_back(key);
		return std::make_pair(_set.end()-1, true);
	}

	/// STL pattern compatible with std::set<>
	/// @remarks Complexity O(1). Worst case performs one test() and one
	/// iterator addition.
	iterator find(key_type key)
	{	
		if (unsigned(key) < _mapSize)
		{
			unsigned x = _map[key];
			return (x < _set.size() && _set[x] == key)? _set.begin()+x: _set.end();
		}
		return _set.end();
	}

	/// STL pattern compatible with std::set<>
	/// @remarks Complexity O(1). Worst case performs one test() and one
	/// iterator addition.
	const_iterator find(key_type key) const
	{
		if (unsigned(key) < _mapSize)
		{
			unsigned x = _map[key];
			return (x < _set.size() && _set[x] == key)? _set.begin()+x: _set.end();
		}
		return _set.end();
	}

	/// STL pattern compatible with std::set<>
	/// @remarks Complexity O(1). Worst case the function performs two iterator
	/// compares, two assignments, and one vector<>.pop_back().
	void erase(iterator it)
	{
        if (it != end())
        {
            if (it != (end()-1))
            {
				// Exchange with back to preserve _set contiguity
                _map[ _set.back() ] = (unsigned)(it - _set.begin());
                const_cast<value_type&>(*it) = const_cast<value_type&>(_set.back());
            }
            _set.pop_back();
        }
	}

	/// STL pattern compatible with std::set<>
	/// @remarks Complexity O(|last-first|). The function performs |last-first| of each:
	/// vector<>.pop_back(), iterator compare * 2, iterator increment, 2 * assignment.
	void erase(iterator first, iterator last)
	{
		unsigned n = (unsigned)(last - first);
		if (last != end())
		{	
			// Move down
			reverse_iterator rfirst(last), rlast(first), rpos(rbegin());
			for (std::ptrdiff_t d=last-first; rfirst != rlast && rfirst != rend(); ++rfirst, ++rpos)
			{
				// Exchange with back to preserve _set contiguity
                _map[ *rpos ] = (unsigned)--d;
                const_cast<value_type&>(*rfirst) = const_cast<value_type&>(*rpos);
			}
			_set.resize(_set.size() - n);
		}
	}

	/// STL pattern compatible with std::set<>
	/// @remarks Complexity O(1). Worst case the function performs one of each: test()
	/// 2*assignment, compare, and vector<>.pop_back().
    size_t erase(const key_type& key)
    {
		if (unsigned(key) < _mapSize)
		{
			unsigned x = _map[key];
			if (x < _set.size() && _set[x] == key)
			{
				if (x != (_set.size()-1))
				{
					// Exchange with back to preserve _set contiguity
					_map[ _set.back() ] = x;
					const_cast<value_type&>(_set[x]) = const_cast<value_type&>(_set.back());
				}
				_set.pop_back();
			}
		}
		return size();
    }

	/// Check if an element exists in the set.
	/// @remarks Complexity O(1). Worst case the function performs three compares and
	/// one assignment.
    bool test(key_type key) const 
	{ 
		if (unsigned(key) < _mapSize)
		{
			unsigned x = _map[key];
			return (x < _set.size() && _set[x] == key);
		}
		return false;
	}

	/// Set intersection. 
	/// @remarks Complexity O( size() ). Worst case there are size() operations
	/// of: test(), vector<>.pop_back(), and 2 * assigment.
	void set_intersect(const unordered_vector_set& other)
	{
		if (!other.empty())
		{
			while (!_set.empty() && !other.test(_set.back()))
				_set.pop_back();

			if (!_set.empty())
			{
				reverse_iterator rfirst = _set.rbegin() + 1;
				reverse_iterator rlast = _set.rend();
				do
				{
					for (; rfirst != rlast && !other.test(*rfirst); ++rfirst)
					{
						// Exchange with back to preserve _set contiguity
						_map[ _set.back() ] = rlast - rfirst;
						const_cast<value_type&>(*rfirst) = const_cast<value_type&>(_set.back());
						_set.pop_back();					
					}
					for (; rfirst != rlast && other.test(*rfirst); ++rfirst)
						continue;
				} while (rfirst != rlast);
			}
		}
		else
			clear();
	}

	/// Set complementation.
	/// @remarks Complexity O( size() ). Worst case there are size() operations
	/// of: test(), vector<>.pop_back(), and 2*assigment.
	void set_complement(const unordered_vector_set& other)
	{
		if (!other.empty())
		{
			while (!_set.empty() && other.test(_set.back()))
				_set.pop_back();

			if (!_set.empty())
			{
				reverse_iterator rfirst = _set.rbegin() + 1;
				reverse_iterator rlast = _set.rend();
				do
				{
					for (; rfirst != rlast && other.test(*rfirst); ++rfirst)
					{
						// Exchange with back to preserve _set contiguity
						_map[ _set.back() ] = rlast - rfirst;
						const_cast<value_type&>(*rfirst) = const_cast<value_type&>(_set.back());
						_set.pop_back();					
					}
					for (; rfirst != rlast && !other.test(*rfirst); ++rfirst)
						continue;
				} while (rfirst != rlast);
			}
		}
		else
			clear();
	}


	/// In order to use upper_bound, lower_bound, sort is required.
    /// A sort is not required for the set_xxxx operations.
	void sort()
	{ 
		std::sort(_set.begin(), _set.end(), compare);
		remap();
	}

	/// Required for XTL set operations
	static key_compare key_comp() { return compare; }
	static value_compare value_comp() { return compare; }

	/// @{
	/// These functions can only be used after a sort
	iterator upper_bound() { return std::upper_bound(_set.begin(), _set.end(), compare); }
	const_iterator upper_bound() const { return std::upper_bound(_set.begin(), _set.end(), compare); }
	iterator lower_bound() { return std::lower_bound(_set.begin(), _set.end(), compare); }
	const_iterator lower_bound() const { return std::lower_bound(_set.begin(), _set.end(), compare); }
	/// @}
};

/// Vector set traits
template<class K, class A>
struct container_traits<unordered_vector_set<K,A> >: public __set_traits<unordered_vector_set<K,A> >
{
	typedef associative_container_tag category;
	UNSUPPORTED_PROPERTY(allow_duplicate_keys);
	UNSUPPORTED_PROPERTY(sorted);
};

// ----------------------------------------------------------------------------
// END OF DECLARATIONS
//
}		// namespace xtl
#endif	// defined(UNORDERED_VECTOR_SET_A9B22A84_6B3D_4180_BC3D_A8F45C942CE2)
