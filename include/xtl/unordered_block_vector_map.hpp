#ifndef UNORDERED_BLOCK_VECTOR_MAP_B5D93BC7_1B9E_4263_BFAD_9D4637D6250B
#define UNORDERED_BLOCK_VECTOR_MAP_B5D93BC7_1B9E_4263_BFAD_9D4637D6250B
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
///         safely bounded in memory using a block_vector class.
/// @author Paul Glendenning
/// @date

#include <vector>
#include <algorithm>
#include "property.hpp"
#include "block_vector.hpp"

namespace xtl {
// ----------------------------------------------------------------------------

template<class BV>
struct unordered_block_vector_map_entry
{
	typename BV::allocator_type::template rebind<unsigned>::other allocator;
	unsigned* ptr;
	unordered_block_vector_map_entry()
	{
		ptr = allocator.allocate(BV::METRICS.BLOCK_SIZE);
	}
	unordered_block_vector_map_entry(const unordered_block_vector_map_entry& other)
	{
		ptr = allocator.allocate(BV::METRICS.BLOCK_SIZE);
		memcpy(ptr, other.ptr, BV::METRICS.BLOCK_SIZE*sizeof(*ptr));
	}
	~unordered_block_vector_map_entry()
	{
		allocator.deallocate(ptr, BV::METRICS.BLOCK_SIZE);
	}
	void swap(unordered_block_vector_map_entry& other)
	{
		std::swap(other.ptr, ptr);
	}
};

/// A unordered_block_vector_map is an unordered map using two block vectors,
/// one for the key value pair and one to determine if the map entry exists.
/// Map entries are only constructed when inserted. The key must be an integer type.
///
/// @param Key		The integer key.
/// @param T		The value type.
/// @param Alloc	The value type allocator.
/// @param BS		The block vector size.
/// @see	 unordered_block_vector_map<>.
/// @remarks The space complexity is O(N), where N is the maximum key. The time 
/// complexity for insert, erase, and find is O(1).
template<class Key, class T, class Alloc=std::allocator<std::pair<Key,T> >, unsigned BS=1024>
class unordered_block_vector_map
{
public:
	typedef Key			        key_type;
	typedef T					mapped_type;
	typedef std::pair<Key,T>	value_type;
	typedef value_type&			reference;
	typedef const value_type&	const_reference;
	typedef value_type*			pointer;
	typedef const value_type*	const_pointer;
	typedef block_vector<value_type,Alloc,BS>			vector_type;
	typedef typename vector_type::iterator	            iterator;
	typedef typename vector_type::const_iterator        const_iterator;
	typedef typename vector_type::reverse_iterator	    reverse_iterator;
	typedef typename vector_type::const_reverse_iterator const_reverse_iterator;
	typedef bool (*key_compare)(const key_type& a, const key_type& b);
	typedef bool (*value_compare)(const value_type& a, const value_type& b);
private:

	/// @cond
	// Each map uses uninitialized storage of unsigned[] so avoid std::vector here.
    std::vector<unordered_block_vector_map_entry<vector_type> > _maps;
	// The data storage set
    vector_type	_set;

	/// Return the number of elements in _maps.
	size_t map_capacity() const
	{
		return _maps.size() * vector_type::METRICS.BLOCK_SIZE;
	}

	void map_reserve(size_t newSize)
	{
		_maps.resize((newSize+vector_type::METRICS.BLOCK_SIZE-1)/vector_type::METRICS.BLOCK_SIZE);
	}

	unsigned& map_item(size_t idx)
	{
		return _maps[idx >> vector_type::METRICS.BLOCK_SHIFT].ptr[idx & vector_type::METRICS.BLOCK_MASK];
	}

	const unsigned& map_item(size_t idx) const
	{
		return _maps[idx >> vector_type::METRICS.BLOCK_SHIFT].ptr[idx & vector_type::METRICS.BLOCK_MASK];
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
	/// @remarks Complexity O(size()).
	void remap()
	{
		unsigned j = 0;
		for (iterator i=_set.begin(); i!=_set.end(); ++i)
			map_item(i->first) = j++;
	}

public:
	/// Create a unordered_block_vector_map with capacity for N elements.
	unordered_block_vector_map() { }
	unordered_block_vector_map(const unordered_block_vector_map& other):
		_maps(other._maps), _set(other._set) { }

	/// Assignment
	unordered_block_vector_map& operator = (const unordered_block_vector_map& other)
	{
		_set = other._set;
		_maps = other._maps;
		return *this;
	}

	/// Reserve disabled for block vectors
	void reserve(size_t capacity) { }

	/// Get the current storage reserve size.
	/// @return  The storage reserve size.
	size_t capacity() const { return map_capacity(); }

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
		//_maps.clear();
    }

	/// STL pattern compatible with std::map<>
	void swap(unordered_block_vector_map& other)
	{
		_maps.swap(other._maps);
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
	/// @remarks Complexity O(1).
	std::pair<iterator, bool> insert(const value_type& p)
	{
		if (p.first < (key_type)map_capacity())
		{
			unsigned& x = map_item(p.first);
			if (x >= _set.size() || _set[x].first != p.first)
			{
				x = _set.size();
				_set.push_back(p);
				return std::make_pair(_set.end()-1, true);
			}
			return std::make_pair(_set.begin()+x, false);
		}
		map_reserve(p.first+1);
		map_item(p.first) = _set.size();
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
		if (key < (key_type)map_capacity())
		{
			unsigned& x = map_item(key);
			if (x >= _set.size() || _set[x].first != key)
			{
				x = _set.size();
				_set.push_back(std::make_pair(key,mapped_type()));
				return _set.back().second;
			}
			return _set[x].second;
		}
		map_reserve(key+1);
		map_item(key) = _set.size();
		_set.push_back(std::make_pair(key,mapped_type()));
		return _set.back().second;
    }

	/// STL pattern compatible with std::map<>
	/// @remarks Complexity O(1)
    const mapped_type& operator [](key_type key) const
    { 
        assert(unsigned(key) < map_capacity());
		const unsigned& x = map_item(key);
        assert(x < _set.size() && _set[x].first == key);
		return _set[x].second;
    }

	/// STL pattern compatible with std::map<>
	/// @remarks Complexity O(1).
	iterator find(key_type key)
	{	
		if (unsigned(key) < map_capacity())
		{
			const unsigned& x = map_item(key);
			return (x < _set.size() && _set[x].first == key)? _set.begin()+x: _set.end();
		}
		return _set.end();
	}

	/// STL pattern compatible with std::map<>
	/// @remarks Complexity O(1).
	const_iterator find(key_type key) const
	{
		if (unsigned(key) < map_capacity())
		{
			const unsigned& x = map_item(key);
			return (x < _set.size() && _set[x].first == key)? _set.begin()+x: _set.end();
		}
		return _set.end();
	}

	/// STL pattern compatible with std::map<>
	/// @remarks Complexity O(1).
	void erase(iterator it)
	{
        if (it != end())
        {
            if (it != (end()-1))
            {
				// Exchange with back to preserve _set contiguity
                map_item( _set.back().first ) = (unsigned)(it - _set.begin());
				// Don't copy since it may be expensive for value_type.
                std::swap(*it, _set.back());
            }
            _set.pop_back();
        }
	}

	/// STL pattern compatible with std::map<>
	/// @remarks Complexity O(|last-first|).
	void erase(iterator first, iterator last)
	{
		using namespace std;
		ptrdiff_t n = last - first;
		XTL_ITERATOR_ASSERT1(first <= last);
		if (first != last)
		{	
			// Move down
			reverse_iterator rfirst(last), rlast(first), rpos(rbegin());
			for (ptrdiff_t d=last-begin(); rfirst != rlast && rfirst != rend(); ++rfirst, ++rpos)
			{
				// Exchange with back to preserve _set contiguity
                map_item( rpos->first ) = (unsigned)--d;
				// Don't copy since it may be expensive for value_type.
                std::swap(*rfirst, *rpos);
			}
			_set.resize(_set.size() - n);
		}
	}

	/// STL pattern compatible with std::map<>
	/// @remarks Complexity O(1).
    size_t erase(const key_type& key)
    {
		if (unsigned(key) < map_capacity())
		{
			const unsigned& x = map_item(key);
			if (x < _set.size() && _set[x].first == key)
			{
				if (x != (_set.size()-1))
				{
					// Exchange with back to preserve _set contiguity
					map_item( _set.back().first ) = x;
					// Don't copy since it may be expensive for value_type.
					std::swap(_set[x], _set.back());
				}
				_set.pop_back();
				//map_reserve(_set.size());
			}
		}
		return size();
    }

	/// Check if an element exists in the map.
	/// @remarks Complexity O(1).
    bool test(key_type key) const 
	{ 
		if (unsigned(key) < map_capacity())
		{
			const unsigned& x = map_item(key);
			return (x < _set.size() && _set[x].first == key);
		}
		return false;
	}

	/// In order to use upper_bound, lower_bound, a sort is required.
	void sort()
	{ 
		std::sort(_set.begin(), _set.end(), vcompare);
		remap();
	}

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
struct container_traits<unordered_block_vector_map<K,T,A> >: public __map_traits<unordered_block_vector_map<K,T,A> >
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
#endif  // defined(UNORDERED_BLOCK_VECTOR_MAP_B5D93BC7_1B9E_4263_BFAD_9D4637D6250B)
