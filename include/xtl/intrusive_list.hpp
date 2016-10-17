#ifndef INTRUSIVE_LIST_C01AC2EF_69E5_4FD0_991A_428D0BD11C21
#define INTRUSIVE_LIST_C01AC2EF_69E5_4FD0_991A_428D0BD11C21
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

// Intrusive list extension to STL.
// In debug mode:
// - #define XTL_ITERATOR_CHECKS 2 for full checks. This will impact performance.
// - #define XTL_ITERATOR_CHECKS 1 for partial checks at no performance impact.
// XTL_ITERATOR_CHECKS defaults to level 2.
// Author Paul Glendenning

#include <cassert>
#include <algorithm>
#include "property.hpp"

namespace xtl {
//-----------------------------------------------------------------------------
class intrusive_list_node;
template<class T, class ListItem> class intrusive_list;
template<class T> class intrusive_list_item;
template<class T> class intrusive_list_item<T*>;
template<class T> class intrusive_list_item<const T*>;
template<class T> class intrusive_list_unlinkable_item;
template<class T> class intrusive_list_iterator;
template<class T> class const_intrusive_list_iterator;


/// Traits for intrusive list items
template<class T>
struct intrusive_list_traits
{
	typedef T	item_type;
	typedef	intrusive_list<typename item_type::value_type, item_type>	list_type;
	
	/// STL iterator definitions
	typedef intrusive_list_iterator<list_type>			iterator;
	typedef const_intrusive_list_iterator<list_type>	const_iterator;
	typedef std::reverse_iterator<iterator>				reverse_iterator;
	typedef std::reverse_iterator<const_iterator>		const_reverse_iterator;
	/// @} @{
	/// STL typedefs for value access
	typedef typename item_type::value_type	value_type;
	typedef typename item_type::pointer		pointer;
	typedef typename item_type::reference	reference;
	/// @}

	static bool supports_sizeof() { return item_type::supports_sizeof(); }
};

/// Intrusive node base class. All list items need to include this somewhere
/// in there declarations.
class intrusive_list_node
{
    template<class T, class ListItem> friend class intrusive_list;
    template<class T> friend class intrusive_list_item;
	template<class List> friend class intrusive_list_iterator_base;
protected:
	/// @cond
	intrusive_list_node* _next;
    intrusive_list_node* _prev;
#if XTL_ITERATOR_CHECKS > 1
	void*	_owner;
#endif
	/// @endcond
public:
	/// An empty list node points to itself
	intrusive_list_node()
#if XTL_ITERATOR_CHECKS > 1
		: _owner(0)
#endif
	{
		_next = _prev = this;
	}

	/// Can't copy intrusive node pointers.
	intrusive_list_node(const intrusive_list_node& v) 
#if XTL_ITERATOR_CHECKS > 1
		: _owner(0)
#endif
		{
		_next = _prev = this;
	}

	/// Can't assign intrusive nodes. Asserts in debug mode.
	intrusive_list_node& operator = (const intrusive_list_node& v) 
	{
		unlink();
		return *this;
	}

	/// True if in a list
	bool is_linked() const { return _next != _prev || _next != this; }

protected:
	// Hide since now maintaining size in container list
	//
	/// Remove from list. Safe even if the list is empty.
	void unlink()
	{
		_prev->_next = _next;
		_next->_prev = _prev;
		_prev = _next = this;
#if XTL_ITERATOR_CHECKS > 1
		_owner = 0;
#endif
	}

	void swap(intrusive_list_node& other)
	{
#if XTL_ITERATOR_CHECKS > 1
		std::swap(_owner, other._owner);
#endif
		XTL_ITERATOR_ASSERT1(is_linked() && other.is_linked());
		_next->_prev = &other;
		_prev->_next = &other;
		other._next->_prev = this;
		other._prev->_next = this;
		std::swap(_next, other._next);
		std::swap(_prev, other._next);
	}
};

/// Template for intrusive list items where the value class T is a member of this one.
/// List items must define the following:
/// -# two reference_cast()
/// -# two pointer_cast() methods,
/// -# and next()/prev() methods, and
/// -# 3 typedefs
template<class T>
class intrusive_list_item: public intrusive_list_node
{
	friend struct intrusive_list_traits<intrusive_list_item<T> >;
private:
	/// @cond
	T	_value;
	/// @endcond
public:
	/// The object the list item contains
	typedef	T value_type;
	/// A pointer to the contained object
	typedef T* pointer;
	/// A reference to the contained object
	typedef T& reference;
	/// The assign type
	typedef const T& assign_type;
	/// The swap type
	typedef T& swap_type;

	/// Set the underlying value
	void assign(const T& other) { _value = other; } 
	
	/// Swap the contained value in the current list
	void swap(T& other) { std::swap(_value, other); }

	/// Not kosher but useful 
	static intrusive_list_item* value2item_cast(T* p)
	{
		static intrusive_list_item* np = 0; 
		return (intrusive_list_item*)
			( (char*)(p) - ((char*)(&np->_value) - (char*)(np)) );
	}

	/// @{
	/// Cast to the pointer type
	T* pointer_cast() { return &_value; }
	const T* pointer_cast() const { return &_value; }
	/// @}

	/// @{
	/// Cast to the reference type
	T& reference_cast() { return _value; }
	const T& reference_cast() const { return _value; }
	/// @}

	/// @{
	/// C style list traversal
	intrusive_list_item* next() const { return static_cast<intrusive_list_item*>(_next); }
	intrusive_list_item* prev() const { return static_cast<intrusive_list_item*>(_prev); }
	/// @}

private:
	/// True if the containing list supports the size() function
	static bool supports_sizeof() { return true; }
};

/// Partial specialization for intrusive list items where the value class T pointer is
/// a member of this one. List items must define the following:
/// -# two reference_cast()
/// -# two pointer_cast() methods,
/// -# and next()/prev() methods, and
/// -# 3 typedefs
template<class T>
class intrusive_list_item<T*>: public intrusive_list_node
{
	friend struct intrusive_list_traits<intrusive_list_item<T*> >;
private:
	/// @cond
	T*	_value;
	/// @endcond
public:
	/// The object the list item contains
	typedef	T value_type;
	/// A pointer to the contained object
	typedef T* pointer;
	/// A reference to the contained object
	typedef T& reference;
	/// The assign type
	typedef T* assign_type;
	/// The swap type
	typedef T*& swap_type;

	/// Set the underlying value
	void assign(T* other) { _value = other; } 

	/// Swap the contained value in the current list
	void swap(T*& other) { std::swap(_value, other); }

	/// @{
	/// Cast to the pointer type
	T* pointer_cast() { return _value; }
	const T* pointer_cast() const { return _value; }
	/// @}

	/// @{
	/// Cast to the reference type
	T& reference_cast() { return *_value; }
	const T& reference_cast() const { return *_value; }
	/// @}

	/// @{
	/// C style list traversal
	intrusive_list_item* next() const { return static_cast<intrusive_list_item*>(_next); }
	intrusive_list_item* prev() const { return static_cast<intrusive_list_item*>(_prev); }
	/// @}
	
private:
	/// True if the containing list supports the size() function
	static bool supports_sizeof() { return true; }
};

/// Partial specialization for intrusive list items where the value class constant T 
/// pointer is a member of this one. List items must define the following:
/// -# two reference_cast()
/// -# two pointer_cast() methods,
/// -# and next()/prev() methods, and
/// -# 3 typedefs
template<class T>
class intrusive_list_item<const T*>: public intrusive_list_node
{
	friend struct intrusive_list_traits<intrusive_list_item<const T*> >;
private:
	/// @cond
	const T* _value;
	/// @endcond
public:
	/// The object the list item contains
	typedef	const T value_type;
	/// A pointer to the contained object
	typedef const T* pointer;
	/// A reference to the contained object
	typedef const T& reference;
	/// The assign type
	typedef const T* assign_type;
	/// The swap type
	typedef const T*& swap_type;

	/// Set the underlying value
	void assign(const T* other) { _value = other; } 

	/// Swap the contained value in the current list
	void swap(const T*& other) { std::swap(_value, other); }

	/// @{
	/// Cast to the pointer type
	const T* pointer_cast() const { return _value; }
	/// @}

	/// @{
	/// Cast to the reference type
	const T& reference_cast() const { return *_value; }
	/// @}

	/// @{
	/// C style list traversal
	intrusive_list_item* next() const { return static_cast<intrusive_list_item*>(_next); }
	intrusive_list_item* prev() const { return static_cast<intrusive_list_item*>(_prev); }
	/// @}

private:
	/// True if the containing list supports the size() function
	static bool supports_sizeof() { return true; }
};

/// Intrusive lists which do not maintain the size.
template<class T>
class intrusive_list_unlinkable_item: public intrusive_list_node
{
	friend struct intrusive_list_traits<intrusive_list_unlinkable_item<T> >;
	typedef intrusive_list_node	super;
private:
	/// @cond
	T	_value;
	/// @endcond
public:
	/// The object the list item contains
	typedef	T value_type;
	/// A pointer to the contained object
	typedef T* pointer;
	/// A reference to the contained object
	typedef T& reference;
	/// The assign type
	typedef const T& assign_type;
	/// The swap type
	typedef T& swap_type;

	/// Remove from list. Safe even if the list is empty.
	void unlink() { super::unlink(); }

	/// Set the underlying value
	void assign(const T& other) { _value = other; } 
	
	/// Swap the contained value in the current list
	void swap(T& other) { std::swap(_value, other); }

	/// Not kosher but useful 
	static intrusive_list_unlinkable_item* value2item_cast(T* p)
	{
		static intrusive_list_unlinkable_item* np = 0; 
		return (intrusive_list_unlinkable_item*)
			( (char*)(p) - ((char*)(&np->_value) - (char*)(np)) );
	}

	/// @{
	/// Cast to the pointer type
	T* pointer_cast() { return &_value; }
	const T* pointer_cast() const { return &_value; }
	/// @}

	/// @{
	/// Cast to the reference type
	T& reference_cast() { return _value; }
	const T& reference_cast() const { return _value; }
	/// @}

	/// @{
	/// C style list traversal
	intrusive_list_unlinkable_item* next() const { return static_cast<intrusive_list_unlinkable_item*>(_next); }
	intrusive_list_unlinkable_item* prev() const { return static_cast<intrusive_list_unlinkable_item*>(_prev); }
	/// @}

private:
	/// False - the containing list does not support the size() function
	static bool supports_sizeof() { return false; }
};

template<class T>
class intrusive_list_unlinkable_item<T*>: public intrusive_list_node
{
	friend struct intrusive_list_traits<intrusive_list_unlinkable_item<T*> >;
	typedef intrusive_list_node	super;
private:
	/// @cond
	T*	_value;
	/// @endcond
public:
	/// The object the list item contains
	typedef	T value_type;
	/// A pointer to the contained object
	typedef T* pointer;
	/// A reference to the contained object
	typedef T& reference;
	/// The assign type
	typedef T* assign_type;
	/// The swap type
	typedef T*& swap_type;

	/// Remove from list. Safe even if the list is empty.
	void unlink() { super::unlink(); }

	/// Set the underlying value
	void assign(T* other) { _value = other; } 

	/// Swap the contained value in the current list
	void swap(T*& other) { std::swap(_value, other); }

	/// @{
	/// Cast to the pointer type
	T* pointer_cast() { return _value; }
	const T* pointer_cast() const { return _value; }
	/// @}

	/// @{
	/// Cast to the reference type
	T& reference_cast() { return *_value; }
	const T& reference_cast() const { return *_value; }
	/// @}

	/// @{
	/// C style list traversal
	intrusive_list_unlinkable_item* next() const { return static_cast<intrusive_list_unlinkable_item*>(_next); }
	intrusive_list_unlinkable_item* prev() const { return static_cast<intrusive_list_unlinkable_item*>(_prev); }
	/// @}
	
private:
	/// False - the containing list does not support the size() function
	static bool supports_sizeof() { return false; }
};

/// Partial specialization for intrusive list items where the value class constant T 
/// pointer is a member of this one. List items must define the following:
/// -# two reference_cast()
/// -# two pointer_cast() methods,
/// -# and next()/prev() methods, and
/// -# 3 typedefs
template<class T>
class intrusive_list_unlinkable_item<const T*>: public intrusive_list_node
{
	friend struct intrusive_list_traits<intrusive_list_unlinkable_item<const T*> >;
	typedef intrusive_list_node	super;
private:
	/// @cond
	const T* _value;
	/// @endcond
public:
	/// The object the list item contains
	typedef	const T value_type;
	/// A pointer to the contained object
	typedef const T* pointer;
	/// A reference to the contained object
	typedef const T& reference;
	/// The assign type
	typedef const T* assign_type;
	/// The swap type
	typedef const T*& swap_type;

	/// Remove from list. Safe even if the list is empty.
	void unlink() { super::unlink(); }

	/// Set the underlying value
	void assign(const T* other) { _value = other; } 

	/// Swap the contained value in the current list
	void swap(const T*& other) { std::swap(_value, other); }

	/// @{
	/// Cast to the pointer type
	const T* pointer_cast() const { return _value; }
	/// @}

	/// @{
	/// Cast to the reference type
	const T& reference_cast() const { return *_value; }
	/// @}

	/// @{
	/// C style list traversal
	intrusive_list_unlinkable_item* next() const { return static_cast<intrusive_list_unlinkable_item*>(_next); }
	intrusive_list_unlinkable_item* prev() const { return static_cast<intrusive_list_unlinkable_item*>(_prev); }
	/// @}

private:
	/// False - the containing list does not support the size() function
	static bool supports_sizeof() { return false; }
};

// Bidirectional iterator pattern
/// @cond
template<class List>
class intrusive_list_iterator_base
{
    template<class T, class ListItem> friend class intrusive_list;
	template<class T> friend bool operator == (const intrusive_list_iterator_base<T>& a, const intrusive_list_iterator_base<T>& b);
	template<class T> friend bool operator != (const intrusive_list_iterator_base<T>& a, const intrusive_list_iterator_base<T>& b);
protected:
	typedef typename List::item_type*	ListItemPtr;
	typedef typename List::value_type	ValueT;

	void _Inc()
	{
		XTL_ITERATOR_ASSERT1(_node != _owner->last());
		_node = _node->_next;
	#if XTL_ITERATOR_CHECKS != 0
		_rend = false;
	#endif
	}

	void _Dec()
	{
		XTL_ITERATOR_ASSERT1(!_rend && !_owner->empty());
		_node = _node->_prev;
	#if XTL_ITERATOR_CHECKS != 0
		_rend = _node == _owner->last();
	#endif
	}
public:
	// Direct access to the list item node
	ListItemPtr node() const { return static_cast<ListItemPtr>(_node); }
protected:
	intrusive_list_node* _node;
#if XTL_ITERATOR_CHECKS != 0
	const List* _owner;
	mutable bool _rend;
	intrusive_list_iterator_base(intrusive_list_node* n, const List* p): _node(n), _owner(p), _rend(false) {}
	intrusive_list_iterator_base(): _node(0), _owner(0), _rend(true) {}
#else
	intrusive_list_iterator_base(intrusive_list_node* n): _node(n) {}
	intrusive_list_iterator_base(): _node(0) {}
#endif
};

template<class List> bool operator == (const intrusive_list_iterator_base<List>& a, const intrusive_list_iterator_base<List>& b)
{
	XTL_ITERATOR_ASSERT1(a._owner == b._owner);
	return a._node == b._node;
}
template<class List> bool operator != (const intrusive_list_iterator_base<List>& a, const intrusive_list_iterator_base<List>& b)
{
	XTL_ITERATOR_ASSERT1(a._owner == b._owner);
	return a._node != b._node;
}
/// @endcond

/// Constant intrusive list iterator. In addtion to the standard STL patterns
/// intrusive list iterators support the item() member function for direct
/// access to the list item.
/// @remarks Models a bidirectional iterator.
template<class List> 
class const_intrusive_list_iterator: public intrusive_list_iterator_base<List>,
				public std::iterator<std::bidirectional_iterator_tag, const typename List::value_type, int>
{
	/// @cond
    template<class T, class ListItem> friend class intrusive_list;
	typedef intrusive_list_iterator_base<List>	super;
public:
	typedef const typename List::item_type* ite_pointer;
	// STL iterator patterns
	typename List::reference operator * () const
	{
		XTL_ITERATOR_ASSERT1(super::_owner.last() != super::_node);
		return static_cast<ite_pointer>(super::_node)->reference_cast();
	}
	typename List::pointer operator -> () const
	{
		XTL_ITERATOR_ASSERT1(super::_owner.last() != super::_node);
		return static_cast<ite_pointer>(super::_node)->pointer_cast();
	}

	const_intrusive_list_iterator& operator ++ ()
	{ 
		super::_Inc();
		return *this;
	}
	const_intrusive_list_iterator operator ++ (int)
	{
		const_intrusive_list_iterator prev(*this);
		super::_Inc();
		return prev;
	}
	const_intrusive_list_iterator& operator -- ()
	{
		super::_Dec();
		return *this;
	}
	const_intrusive_list_iterator operator -- (int)
	{
		const_intrusive_list_iterator prev(*this);
		super::_Dec();
		return prev;
	}

private:
#if XTL_ITERATOR_CHECKS != 0
	const_intrusive_list_iterator(intrusive_list_node* n, const List* p): super(n,p) {}
#else
	const_intrusive_list_iterator(intrusive_list_node* n): super(n) {}
#endif
	/// @endcond
};

/// Intrusive list iterator. In addtion to the standard STL patterns intrusive
/// list iterators support the item() member function for direct access to the
/// list item.
/// @remarks Models a bidirectional iterator.
template<class List>
class intrusive_list_iterator: public intrusive_list_iterator_base<List>,
								public std::iterator<std::bidirectional_iterator_tag, typename List::value_type, int>
{
	/// @cond
    template<class T, class ListItem> friend class intrusive_list;
	typedef intrusive_list_iterator_base<List>	super;
public:
	typedef typename List::item_type* ite_pointer;
	// STL iterator patterns
	typename List::reference operator * () const 
	{
		XTL_ITERATOR_ASSERT1(super::_owner->last() != super::_node);
		return static_cast<ite_pointer>(super::_node)->reference_cast();
	}
	typename List::pointer operator-> () const
	{
		XTL_ITERATOR_ASSERT1(super::_owner->last() != super::_node);
		return static_cast<ite_pointer>(super::_node)->pointer_cast();
	}
#if XTL_ITERATOR_CHECKS != 0
	operator const_intrusive_list_iterator<List> () const { return const_intrusive_list_iterator<List>(super::_node, super::_owner); }
#else
	operator const_intrusive_list_iterator<List> () const { return const_intrusive_list_iterator<List>(super::_node); }
#endif
	intrusive_list_iterator& operator ++ ()
	{ 
		super::_Inc();
		return *this;
	}
	intrusive_list_iterator operator ++ (int)
	{
		intrusive_list_iterator prev(*this);
		super::_Inc();
		return prev;
	}
	intrusive_list_iterator& operator -- ()
	{
		super::_Dec();
		return *this;
	}
	intrusive_list_iterator operator -- (int)
	{
		intrusive_list_iterator prev(*this);
		super::_Dec();
		return prev;
	}

private:
#if XTL_ITERATOR_CHECKS != 0
	intrusive_list_iterator(intrusive_list_node* n, const List* p): super(n,p) {}
#else
	intrusive_list_iterator(intrusive_list_node* n): super(n) {}
#endif
	/// @endcond
};

/// A list where node memory management is performed outside the scope of the
/// class. Intrusive lists are circular so removal of a list item can be done
/// without knowing the container provided the size() is not required to be
/// maintained. For this case use intrusive_list_unlinkable_item<> and simply 
/// call ListItem::unlink() on any node.
///
/// The class replicates most std::list patterns with the following exceptions:
/// - The size() member returns zero for lists of unlinkable list items. 
/// - The insert() members are not supported since list node management is outside 
///   the class scope. Alternative methods insert_before() and  insert_after()
///    provide this functionality.
/// - push_back() and push_front() take a ListItem pointer as the argument.
/// - clear() does not delete list nodes, it only resets the list head and tail.
///
template<class T, class ListItem=intrusive_list_item<T> >
class intrusive_list
{
public:
	typedef intrusive_list_traits<ListItem>	traits_type;
	/// @{ 
	/// STL iterator defintions
	typedef intrusive_list_iterator<intrusive_list>			iterator;
	typedef const_intrusive_list_iterator<intrusive_list>	const_iterator;
	typedef std::reverse_iterator<iterator>					reverse_iterator;
	typedef std::reverse_iterator<const_iterator>			const_reverse_iterator;
	/// @} @{
	/// STL typedefs for value access
	typedef typename ListItem::value_type	value_type;
	typedef typename ListItem::pointer		pointer;
	typedef typename ListItem::reference	reference;
	/// @}

	/// Items stored in this list. Memory allocation of item_type is outside the
	/// scope of this class.
	typedef	ListItem	item_type;
private:
	/// List head and tail. Also serves as an end marker.
	intrusive_list_node		_end;
	/// Not used if the list items are unlinkable.
	size_t					_size;

#if XTL_ITERATOR_CHECKS > 1
	void reset_owner(intrusive_list* newOwner)
	{
		for (item_type* p = _end._next; p != &_end; p=p->next())
			p->_owner = newOwner;
	}
#endif
public:
	intrusive_list() 
	{
	#if XTL_ITERATOR_CHECKS > 1
		_end._owner = this;
	#endif
		_size = 0;
	}

	/// STL pattern. Returns zero if list item allows unlinking hence
	/// size is not maintained.
	size_t size() { return traits_type::supports_sizeof()? _size: 0; }	

	/// @}
	/// Insert new_item after item
	/// @param	item		An entry in the list.
	/// @param	new_item	The new entry to insert after item.
    void insert_after(item_type* item, item_type* new_item)
    {
		assert(!new_item->is_linked());
		XTL_ITERATOR_ASSERT2(item->_owner == this);
		XTL_ITERATOR_ASSERT2(new_item->_owner == 0);
	#if XTL_ITERATOR_CHECKS > 1
		new_item->_owner = this;
	#endif
		new_item->_next = item->_next;
		new_item->_prev = item;
		item->_next->_prev = new_item;
		item->_next = new_item;
		++_size;
    }
	void insert_after(iterator item, item_type* new_item) 
	{
		XTL_ITERATOR_ASSERT1(item._owner == this);
		insert_after(item.node(), new_item);
	}
	/// @} @{
	/// Insert new_item before item
	/// @param	item		An entry in the list.
	/// @param	new_item	The new entry to before after item.
    void insert_before(item_type* item, item_type* new_item)
    {
		assert(!new_item->is_linked());
		XTL_ITERATOR_ASSERT2(item->_owner == this);
		XTL_ITERATOR_ASSERT2(new_item->_owner == 0);
	#if XTL_ITERATOR_CHECKS > 1
		new_item->_owner = this;
	#endif
		new_item->_prev = item->_prev;
		new_item->_next = item;
		item->_prev->_next = new_item;
		item->_prev = new_item;
		++_size;
    }
	void insert_before(iterator item, item_type* new_item) 
	{
		XTL_ITERATOR_ASSERT1(item._owner == this);
		insert_before(item.node(), new_item);
	}
	/// @}

	/// Join two lists by appending other to this list The join clears the other
	/// list.
	/// @param	other	The list to append.
	void splice(intrusive_list& other)
	{
		if (!other.empty())
		{
		#if XTL_ITERATOR_CHECKS > 1
			other.reset_owner(this);
		#endif
			other._end._next->_prev = &_end;
			other._end._prev->_next = _end._next;
			_end._next->_prev = other._end._prev;
			_end._next = other._end._next;
			other._end.unlink();
			_size + other._size;
			other._size = 0;
		}
	}

	/// @{
	/// C style list access to list item types.
	/// - To iterate the pointer range [head,last), traverse the list using item_type::next(). 
	/// - To iterate the reverse pointer range [tail, last), traverse the list using
	///   item_type::prev().
	/// - To access list elements use item_type::pointer_cast() or item_type::reference_cast().
	/// - to remove list elements use item_type::unlink().
	item_type* head() { return static_cast<item_type*>(_end._next); }
	item_type* tail() { return static_cast<item_type*>(_end._prev); }
	item_type* last() { return static_cast<item_type*>(&_end); }
	const item_type* head() const { return static_cast<const item_type*>(_end._next); }
	const item_type* tail() const { return static_cast<const item_type*>(_end._prev); }
	const item_type* last() const { return static_cast<const item_type*>(&_end); }
	/// @/

	/// @{
	/// STL patterns
    bool empty() const { return _end._prev == _end._next && _end._next == &_end; }
    reference front() { assert(!empty()); return head()->reference_cast(); }
    reference front() const { assert(!empty()); return head()->reference_cast(); }
    reference back() { assert(!empty()); return tail()->reference_cast(); }
    reference back() const { assert(!empty()); return tail()->reference_cast(); }
#if XTL_ITERATOR_CHECKS != 0
    iterator begin() { return iterator(_end._next, this); }
    iterator end() { return iterator(&_end, this); }
    const_iterator begin() const { return const_iterator(_end._next, this); }
    const_iterator end() const { return const_iterator(&_end, this); }
#else
    iterator begin() { return iterator(_end._next); }
    iterator end() { return iterator(&_end); }
    const_iterator begin() const { return const_iterator(_end._next); }
    const_iterator end() const { return const_iterator(&_end); }
#endif
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
	void clear() { _end.unlink(); }
	void swap(intrusive_list& other) 
	{ 
		_end.swap(other._end);
	#if XTL_ITERATOR_CHECKS > 1
		other.reset_owner(&other);
		reset_owner(this);
	#endif
		_size = other._size;
		other._size = 0;
	}
	void pop_back()
	{
		assert(!empty());
		assert(!traits_type::supports_sizeof() || _size > 0);
		_end._prev->unlink();
		--_size;
	}
	void pop_front()
	{
		assert(!empty());
		assert(!traits_type::supports_sizeof() || _size > 0);
		_end._next->unlink();
		--_size;
	}
#if XTL_ITERATOR_CHECKS > 1
	void insert(iterator pos, iterator first, iterator last)
	{
		XTL_ITERATOR_ASSERT1(last._owner == first._owner);
		XTL_ITERATOR_ASSERT1(pos._owner == this);
		while (first != last)
		{
			item_type* node = first++;
			node->unlink();
			--last->_owner->_size;
			insert_before(pos, node);
		}
	}
#endif
	/// @}
 	
	/// Insert at the list end. Differs from std::list<>.
	void push_back(item_type* item) { insert_before(static_cast<item_type*>(&_end), item); }

	/// Insert at the list front. Differs from std::list<>.
    void push_front(item_type* item) { insert_after(static_cast<item_type*>(&_end), item); }

	/// For intrusive lists we don't maintain a count so this function could be static.
	void erase(iterator first, iterator last)
	{
		while (first != last &&  first._node->is_linked())
		{
			XTL_ITERATOR_ASSERT1(first._owner == this);
			intrusive_list_node* node = first._node;
			++first;
			--_size;
			node->unlink();
		}
	}

    void erase(iterator at)
	{
		XTL_ITERATOR_ASSERT1(at._owner == this);
		--_size;
		at._node->unlink();
	}

	/// @{
	/// Convert a ListItem* to a iterator
#if XTL_ITERATOR_CHECKS != 0
	iterator cast_it(item_type* item) { return iterator(item, this); }
	const_iterator cast_it(item_type* item) const { return const_iterator(item, this); }
	reverse_iterator rcast_it(item_type* item) { return reverse_iterator(iterator(item, this)); }
	const_reverse_iterator rcast_it(item_type* item) const { return reverse_iterator(const_iterator(item,this)); }
#else
	iterator cast_it(item_type* item) { return iterator(item); }
	const_iterator cast_it(item_type* item) const { return const_iterator(item); }
	reverse_iterator rcast_it(item_type* item) { return reverse_iterator(iterator(item)); }
	const_reverse_iterator rcast_it(item_type* item) const { return reverse_iterator(const_iterator(item)); }
#endif
	/// @}
private:
	/// Disabled copy construction
	intrusive_list(const intrusive_list& other);
	/// Disabled assignment
	intrusive_list& operator = (const intrusive_list& other);
};

/// Container traits
template<class T>
struct container_traits<intrusive_list<T> >
{
	typedef sequence_container_tag category;
	UNSUPPORTED_PROPERTY(allow_duplicate_keys);
	UNSUPPORTED_PROPERTY(sorted);
};


template<class T>
struct container_traits<intrusive_list<T, intrusive_list_unlinkable_item<T> > >
{
	typedef sequence_container_tag category;
	UNSUPPORTED_PROPERTY(allow_duplicate_keys);
	UNSUPPORTED_PROPERTY(sorted);
};

//-----------------------------------------------------------------------------
// END OF DECLARATIONS
}		// namespace xtl
#endif	// defined(INTRUSIVE_LIST_C01AC2EF_69E5_4FD0_991A_428D0BD11C21)

