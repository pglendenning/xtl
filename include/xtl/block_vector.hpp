#ifndef BLOCK_VECTOR_FF2E591B_C37E_4752_AE62_48BC0DA12541
#define BLOCK_VECTOR_FF2E591B_C37E_4752_AE62_48BC0DA12541
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


// Vector class which will never invalidate memory pointers when growing
// the array. It does this by maintaining an array of memory blocks.
//
// Author Paul Glendenning

#include <vector>
#include <cassert>
#include "property.hpp"
#include "bitmagic.hpp"

namespace xtl {
// ----------------------------------------------------------------------------
template<class T, class A, unsigned BS> class block_vector;

/// @cond
template<class T>
struct block_vector_node
{
	typedef T* node_pointer;
	typedef T& node_reference;
	typedef T node_value_type;

	T*	_begin;
	T*	_end;

	size_t size() const { return _end - _begin; }
	void clear() { _begin = _end = 0; }

	block_vector_node(): _begin(0), _end(0) {}
};
/// @endcond

// Bidirectional iterator pattern
/// @cond
template<class BVec, class Iter>
class block_vector_iterator_base
{
	template<class T, class A, unsigned BS> friend class block_vector;
	template<class T, class U> friend bool operator == (const block_vector_iterator_base<T,U>& a, const block_vector_iterator_base<T,U>& b);
	template<class T, class U> friend bool operator != (const block_vector_iterator_base<T,U>& a, const block_vector_iterator_base<T,U>& b);
	template<class T, class U> friend bool operator < (const block_vector_iterator_base<T,U>& a, const block_vector_iterator_base<T,U>& b);
	template<class T, class U> friend bool operator <= (const block_vector_iterator_base<T,U>& a, const block_vector_iterator_base<T,U>& b);
	template<class T, class U> friend bool operator > (const block_vector_iterator_base<T,U>& a, const block_vector_iterator_base<T,U>& b);
	template<class T, class U> friend bool operator >= (const block_vector_iterator_base<T,U>& a, const block_vector_iterator_base<T,U>& b);
	template<class T, class U> friend std::ptrdiff_t operator - (const block_vector_iterator_base<T,U>& a, const block_vector_iterator_base<T,U>& b);

public:
	typedef typename BVec::vector_type	vector_type;
	typedef Iter node_iterator;
	typedef typename std::iterator_traits<Iter>::value_type	node_type;
	typedef typename std::iterator_traits<Iter>::value_type::node_pointer	node_pointer;
	typedef typename std::iterator_traits<Iter>::value_type::node_reference	node_reference;

	block_vector_iterator_base(): _inner(0) { }

protected:
	void _Inc()
	{
		XTL_ITERATOR_ASSERT1(0 != _inner); 
		if (++_inner == _outer->_end)
		{
			++_outer;
			_inner = _outer->_begin;
		}
	}

	void _Dec()
	{
		if (!_inner)
		{
			--_outer;
			XTL_ITERATOR_ASSERT1(0 != _outer->_end); 
			_inner = _outer->_end - 1;
		}
		else if (_inner == _outer->_begin)
		{
			--_outer;
			XTL_ITERATOR_ASSERT1(0 != _outer->_end); 
			_inner = _outer->_end - 1;
		}
		else
		{
			--_inner;
		}
	}

	void _PlusEq(std::ptrdiff_t count)
	{
		if (count > 0)
		{
			XTL_ITERATOR_ASSERT1(0 != _inner); 
			std::ptrdiff_t n = std::min(_outer->_end - _inner, count);
			count -= n;
			_inner += n;
			if (_inner == _outer->_end)
			{
				++_outer;
				_inner = _outer->_begin;
			}
#if XTL_ITERATOR_CHECKS	== 3
			for (std::ptrdiff_t k=count>>BVec::METRICS.BLOCK_SHIFT; k; --k)
			{
				++_outer;
				_inner = _outer->_begin;
				count -= (std::ptrdiff_t)BVec::METRICS.BLOCK_SIZE;
			}
#else
			std::ptrdiff_t k = count >> BVec::METRICS.BLOCK_SHIFT;
			if (k)
			{
				_outer += k;
				count -= (std::ptrdiff_t)(k << BVec::METRICS.BLOCK_SHIFT);
				_inner = _outer->_begin;
			}
#endif
			_inner += count;
			if (_inner && _inner == _outer->_end)
			{
				++_outer;
				_inner = _outer->_begin;
			}
		}
		else if (count < 0)
		{
			_Dec();
			count = -count - 1;
			std::ptrdiff_t n = std::min(_inner - _outer->_begin, count);
			count -= n;
			_inner -= n;
			if (_inner == _outer->_begin && count)
			{
				--_outer;
				--count;
				XTL_ITERATOR_ASSERT1(0 != _outer->_end); 
				_inner = _outer->_end - 1;
			}
#if XTL_ITERATOR_CHECKS	== 3
			for (std::ptrdiff_t k=count>>BVec::METRICS.BLOCK_SHIFT; k; --k);
			{
				--_outer;
				count -= (std::ptrdiff_t)BVec::METRICS.BLOCK_SIZE;
				XTL_ITERATOR_ASSERT1(0 != _outer->_end); 
				_inner = _outer->_end - 1;
			}
#else
			std::ptrdiff_t k = count >> BVec::METRICS.BLOCK_SHIFT;
			if (k)
			{
				_outer -= k;
				count -= (std::ptrdiff_t)(k << BVec::METRICS.BLOCK_SHIFT);
				_inner = _outer->_end - 1;
			}
#endif
			_inner -= count;
			XTL_ITERATOR_ASSERT1(_inner >= _outer->_begin);
		}
	}

	// return (this < that)
	bool _Less(const block_vector_iterator_base& that) const
	{
		XTL_ITERATOR_ASSERT1(_outer != that._outer ||
				(!_inner && !that._inner) ||
				(_inner && that._inner));
		return _outer < that._outer || 
			(_outer == that._outer && _inner && _inner < that._inner);
	}

	// return (this - that)
	std::ptrdiff_t _Diff(const block_vector_iterator_base& that) const
	{
		// Calc fixup for last partially constructed block if its in the range
		std::ptrdiff_t ethat = that._inner? 0: std::ptrdiff_t(BVec::METRICS.BLOCK_SIZE - (that._outer-1)->size());
		std::ptrdiff_t ethis = _inner? 0: std::ptrdiff_t(BVec::METRICS.BLOCK_SIZE - (_outer-1)->size());

		if (_Less(that))
		{
			return -( ((that._outer - _outer) << BVec::METRICS.BLOCK_SHIFT) - ethat + ethis +
							(that._inner - that._outer->_begin) - (_inner - _outer->_begin) );
		}
		else if (that._Less(*this))
		{
			return ((_outer - that._outer) << BVec::METRICS.BLOCK_SHIFT) - ethis + ethat +
							(_inner - _outer->_begin) - (that._inner - that._outer->_begin);
		}
		return 0;
	}

	block_vector_iterator_base(node_iterator outer, node_pointer inner):
					_outer(outer), _inner(inner) { }
	node_iterator	_outer;
	node_pointer	_inner;
};

template<class BVec, class Iter> static inline bool operator == (const block_vector_iterator_base<BVec, Iter>& a, const block_vector_iterator_base<BVec, Iter>& b)
{
	return a._outer == b._outer && a._inner == b._inner;
}

template<class BVec, class Iter> static inline bool operator != (const block_vector_iterator_base<BVec, Iter>& a, const block_vector_iterator_base<BVec, Iter>& b)
{
	return a._outer != b._outer || a._inner != b._inner;
}

template<class BVec, class Iter> static inline bool operator < (const block_vector_iterator_base<BVec, Iter>& a, const block_vector_iterator_base<BVec, Iter>& b)
{
	return a._Less(b);
}

template<class BVec, class Iter> static inline bool operator <= (const block_vector_iterator_base<BVec, Iter>& a, const block_vector_iterator_base<BVec, Iter>& b)
{
	return !b._Less(a);
}

template<class BVec, class Iter> static inline bool operator > (const block_vector_iterator_base<BVec, Iter>& a, const block_vector_iterator_base<BVec, Iter>& b)
{
	return b._Less(a);
}

template<class BVec, class Iter> static inline bool operator >= (const block_vector_iterator_base<BVec, Iter>& a, const block_vector_iterator_base<BVec, Iter>& b)
{
	return !a._Less(b);
}

template<class BVec, class Iter> static inline std::ptrdiff_t operator - (const block_vector_iterator_base<BVec, Iter>& a, const block_vector_iterator_base<BVec, Iter>& b)
{
	return a._Diff(b);
}

template<class BVec> class block_vector_iterator;
/// @endcond

/// Block vector constant iterator.
/// @remarks Models a bidirectional iterator.
template<class BVec> 
class const_block_vector_iterator: public block_vector_iterator_base<BVec, typename BVec::vector_type::const_iterator>,
						public std::iterator<std::bidirectional_iterator_tag, const typename BVec::value_type>
{
	/// @cond
	typedef block_vector_iterator_base<BVec, typename BVec::vector_type::const_iterator> _super;
	typedef typename BVec::vector_type::const_iterator				node_iterator;
	typedef typename std::iterator_traits<const_block_vector_iterator>::pointer	node_pointer;

	template<class T, class A, unsigned BS> friend class block_vector;
	friend class block_vector_iterator<BVec>;

public:
	// STL iterator patterns
	typename std::iterator_traits<const_block_vector_iterator>::reference operator * () const
	{
		XTL_ITERATOR_ASSERT1(_super::_inner != 0);
		return *_super::_inner;
	}
	typename std::iterator_traits<const_block_vector_iterator>::pointer operator -> () const
	{
		XTL_ITERATOR_ASSERT1(_super::_inner != 0);
		return _super::_inner;
	}

	const_block_vector_iterator& operator ++ ()
	{ 
		_super::_Inc();
		return *this;
	}
	const_block_vector_iterator operator ++ (int)
	{
		const_block_vector_iterator prev(*this);
		_super::_Inc();
		return prev;
	}
	const_block_vector_iterator& operator -- ()
	{
		_super::_Dec();
		return *this;
	}
	const_block_vector_iterator operator -- (int)
	{
		const_block_vector_iterator prev(*this);
		_super::_Dec();
		return prev;
	}
	const_block_vector_iterator operator + (std::ptrdiff_t count) const
	{
		const_block_vector_iterator tmp(*this);
		tmp._PlusEq(count);
		return tmp;
	}
	const_block_vector_iterator operator - (std::ptrdiff_t count) const
	{
		const_block_vector_iterator tmp(*this);
		tmp._PlusEq(-count);
		return tmp;
	}
	const_block_vector_iterator& operator += (std::ptrdiff_t count)
	{
		_super::_PlusEq(count);
		return *this;
	}
	const_block_vector_iterator& operator -= (std::ptrdiff_t count)
	{
		_super::_PlusEq(-count);
		return *this;
	}

	const_block_vector_iterator() {}
private:
	const_block_vector_iterator(node_iterator outer, node_pointer inner)
		: _super(outer, inner) {}
	const_block_vector_iterator(node_iterator outer)
		: _super(outer, outer->_begin) {}
	/// @endcond
};

/// Block vector iterator.
/// @remarks Models a bidirectional iterator.
template<class BVec> 
class block_vector_iterator: public block_vector_iterator_base<BVec, typename BVec::vector_type::iterator>,
						public std::iterator<std::bidirectional_iterator_tag, typename BVec::value_type>
{
	/// @cond
	typedef block_vector_iterator_base<BVec, typename BVec::vector_type::iterator> _super;
	typedef typename BVec::vector_type::iterator	node_iterator;
	typedef typename std::iterator_traits<node_iterator>::pointer	node_pointer;

	template<class T, class A, unsigned BS> friend class block_vector;
public:
	// STL iterator patterns
	typename std::iterator_traits<block_vector_iterator>::reference operator * () const
	{
		XTL_ITERATOR_ASSERT1(_super::_inner != 0);
		return *_super::_inner;
	}
	typename std::iterator_traits<block_vector_iterator>::pointer operator -> () const
	{
		XTL_ITERATOR_ASSERT1(_super::_inner != 0);
		return _super::_inner;
	}

	operator const_block_vector_iterator<BVec> () const
	{
		return const_block_vector_iterator<BVec>(_super::_outer, _super::_inner);
	}

	block_vector_iterator& operator ++ ()
	{ 
		_super::_Inc();
		return *this;
	}
	block_vector_iterator operator ++ (int)
	{
		block_vector_iterator prev(*this);
		_super::_Inc();
		return prev;
	}
	block_vector_iterator& operator -- ()
	{
		_super::_Dec();
		return *this;
	}
	block_vector_iterator operator -- (int)
	{
		block_vector_iterator prev(*this);
		_super::_Dec();
		return prev;
	}
	block_vector_iterator operator + (std::ptrdiff_t count) const
	{
		block_vector_iterator tmp(*this);
		tmp._PlusEq(count);
		return tmp;
	}
	block_vector_iterator operator - (std::ptrdiff_t count) const
	{
		block_vector_iterator tmp(*this);
		tmp._PlusEq(-count);
		return tmp;
	}
	block_vector_iterator& operator += (std::ptrdiff_t count)
	{
		_super::_PlusEq(count);
		return *this;
	}
	block_vector_iterator& operator -= (std::ptrdiff_t count)
	{
		_super::_PlusEq(-count);
		return *this;
	}

	block_vector_iterator() {}
private:
	block_vector_iterator(node_iterator outer, node_pointer inner)
		: _super(outer, inner) {}
	block_vector_iterator(node_iterator outer)
		: _super(outer, outer->_begin) {}
	/// @endcond
};

/// @cond
template<unsigned BS>
struct block_metrics
{
	const unsigned BLOCK_SIZE;
	const unsigned BLOCK_SHIFT;
	const unsigned BLOCK_MASK;
	block_metrics():
		BLOCK_SIZE(bitmagic<unsigned>::nextpow2(BS)),
		BLOCK_SHIFT(bitmagic<unsigned>::ceil_log2(BS)),
		BLOCK_MASK(bitmagic<unsigned>::nextpow2(BS)-1)
	{
	}
};
/// @endcond

/// A block vector models std::vector but guarantees to never invalidate memory
/// pointers when growing the array. The block vector maintains an array of
/// memory blocks whose size is specified by the template parameter BS.
///
/// This vector pattern is useful for managing the memory for intrusive lists. 
///
/// @remarks Vector insert and erase are not supported.
template<class T, class A=std::allocator<T>, unsigned BS=1024>
class block_vector
{
public:
	static const block_metrics<BS>		METRICS;
	typedef	A							allocator_type;
	typedef	T							value_type;
	typedef	size_t						size_type;
	typedef	typename A::reference		reference;
	typedef	typename A::pointer			pointer;
	typedef	typename A::const_reference	const_reference;
	typedef	typename A::const_pointer	const_pointer;
	typedef block_vector_node<T>		node_type;
	typedef std::vector<node_type,typename A::template rebind<node_type>::other> vector_type;
	typedef block_vector_iterator<block_vector>			iterator;
	typedef const_block_vector_iterator<block_vector>	const_iterator;
	typedef std::reverse_iterator<iterator>				reverse_iterator;
	typedef std::reverse_iterator<const_iterator>		const_reverse_iterator;

private:
	/// @cond
	vector_type		_vecs;
	allocator_type	_alloc;

	// Get the block before the end marker
	node_type& back_node() { return _vecs[_vecs.size()-2]; }
	const node_type& back_node() const { return _vecs[_vecs.size()-2]; }
	node_type& front_node() { return _vecs[1]; }
	const node_type& front_node() const { return _vecs[1]; }

	// Grow one element and copy construct with val
	void grow(const_reference val)
	{
		// There are always two empty node_types to mark begin and end
		if (_vecs.empty()) _vecs.resize(2);
		if (back_node().size() == METRICS.BLOCK_SIZE || _vecs.size() == 2)
		{
			_vecs.back()._begin = _vecs.back()._end = _alloc.allocate(METRICS.BLOCK_SIZE);
			_vecs.resize(_vecs.size()+1);
		}
		_alloc.construct(back_node()._end++, val);
	}

	// Grow size elements and copy construct with val
	void grow(size_t size, const_reference val)
	{
		assert(size);
		grow(val);
		--size;
		while (size)
		{
			if (back_node().size() == METRICS.BLOCK_SIZE)
			{
				_vecs.back()._begin = _vecs.back()._end = _alloc.allocate(METRICS.BLOCK_SIZE);
				_vecs.resize(_vecs.size()+1);
			}
			size_t n = std::min(size, (size_t)METRICS.BLOCK_SIZE-back_node().size());
			size -= n;
			for (T *p=back_node()._end, *pend=p+n; p!=pend; )
				_alloc.construct(p++, val);
			back_node()._end += n;
		}
	}

	// Reduce size by one element
	void shrink()
	{
		// Always have two empty node_types to mark begin and end
		XTL_ITERATOR_ASSERT1(_vecs.size() > 2);
		XTL_ITERATOR_ASSERT1(back_node().size());
		_alloc.destroy(--back_node()._end);
		if (0 == back_node().size())
		{
			_vecs.pop_back();
			_alloc.deallocate(_vecs.back()._begin, METRICS.BLOCK_SIZE);
			_vecs.back().clear();	// new end marker
		}
	}

	// Reduce size by size eelements and destruct size elements
	void shrink(size_t size)
	{
		while (size)
		{
			// Always have two empty node_types to mark begin and end
			XTL_ITERATOR_ASSERT1(_vecs.size() > 2);
			size_t n = std::min(size, back_node().size());
			size -= n;
			for (T *p=back_node()._end, *pend=p-n; p != pend; )
				_alloc.destroy(--p);
			back_node()._end -= n;
			if (0 == back_node().size())
			{
				_vecs.pop_back();
				_alloc.deallocate(_vecs.back()._begin, METRICS.BLOCK_SIZE);
				_vecs.back().clear();	// new end marker
			}
		}
	}
	/// @endcond
public:
	block_vector(size_t size=0) { resize(size); }
	~block_vector() { clear(); }

	/// STL Random access operator
	const_reference operator [] (unsigned i) const
	{
		unsigned block = (i >> METRICS.BLOCK_SHIFT) + 1;
		i &= METRICS.BLOCK_MASK;
		XTL_ITERATOR_ASSERT1(block < _vecs.size()-1  && i < _vecs[block].size());
		return _vecs[block]._begin[i];
	}

	/// STL Random access operator
	reference operator [] (unsigned i)
	{
		unsigned block = (i >> METRICS.BLOCK_SHIFT) + 1;
		i &= METRICS.BLOCK_MASK;
		XTL_ITERATOR_ASSERT1(block < _vecs.size()-1  && i < _vecs[block].size());
		return _vecs[block]._begin[i];
	}

	/// @{
	/// STL container properties
	size_t size() const { return (_vecs.size() <= 2)? 0: METRICS.BLOCK_SIZE*(_vecs.size()-3) + back_node().size(); }
	bool empty() const { return _vecs.size() <= 2 || (_vecs.size() == 3 && back_node().size() == 0); }
	reference front()
	{
		XTL_ITERATOR_ASSERT1(!empty());
		return *front_node()._begin;
	}
	const_reference front() const
	{
		XTL_ITERATOR_ASSERT1(!empty());
		return *front_node()._begin;
	}
	reference back()
	{
		XTL_ITERATOR_ASSERT1(!empty());
		return *(back_node()._end-1); 
	}
	const_reference back() const
	{
		XTL_ITERATOR_ASSERT1(!empty());
		return *(back_node()._end-1);
	}
	/// @}

	/// Append an item to the vector and copy construct
	void push_back(const_reference x) { grow(x); }

	/// Append an item to the vector and default construct
	void push_back() { grow(value_type()); }

	/// Remove an item from the vector
	void pop_back()
	{
		XTL_ITERATOR_ASSERT1(!empty());
		shrink();
	}

	/// Modify the container size
	void resize(size_t newSize)
	{
		if (newSize > size())
			grow(newSize - size(), value_type());
		else if (newSize < size())
			shrink(size() - newSize);
	}

	/// Reserve space for main vector
	void reserve(size_t cap)
	{
		_vecs.reserve(2 + (cap >> METRICS.BLOCK_SHIFT));
	}

	/// Clear all elements
	void clear()
	{
		shrink(size());
	}

	/// Copy all elements from other.
	block_vector& operator = (const block_vector& other)
	{
		clear();
		for (const_iterator it=other.begin(); it != other.end(); ++it)
		{
			grow(*it);
		}
		return *this;
	}

	/// Exchange block vector contents with other
	void swap(block_vector& other)
	{
		_vecs.swap(other._vecs);
	}

	/// @{
	/// STL iterators
	iterator begin() { return iterator(_vecs.begin()+1); }
	iterator end() { return iterator(_vecs.end()-1); }
	const_iterator begin() const { return const_iterator(typename vector_type::const_iterator(_vecs.begin())+1); }
	const_iterator end() const { return const_iterator(typename vector_type::const_iterator(_vecs.end())-1); }
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
	/// @}
};

/// @cond
template<class T, class A, unsigned BS>
const block_metrics<BS> block_vector<T,A,BS>::METRICS;
/// @endcond

//-----------------------------------------------------------------------------
// END DEFINITION
//
}		// namespace xtl
#endif	// defined(BLOCK_VECTOR_FF2E591B_C37E_4752_AE62_48BC0DA12541)
