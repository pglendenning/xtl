#ifndef	PROPERTY_16B182B6_B29E_413D_A200_27EBABCC05B4
#define	PROPERTY_16B182B6_B29E_413D_A200_27EBABCC05B4
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

// Property extension to STL. 
// author Paul Glendenning

#include <cassert>

/// Property declaration
/// Use at the root namespace scope but not within structures or classes.
#define	DECLARE_PROPERTY(prop)	\
	struct prop##_supported { }; \
	struct prop##_unsupported { }

/// Supported property definition.
/// Use within templated property structures, for example container_properties<>.
#define	SUPPORTED_PROPERTY(prop) \
	typedef prop##_supported prop##_property_type;

/// Unsupported property definition.
/// Use within templated property structures, for example container_properties<>
#define	UNSUPPORTED_PROPERTY(prop) \
	typedef prop##_unsupported prop##_property_type;

/// Inherit a property from another definition.
#define INHERIT_PROPERTY(other, prop) \
	typedef typename other##::prop##_property_type prop##_property_type;

/*
template<> class vector_bitmap<uint32_t>;
template<> class vector_bitmap<uint64_t>;
template<> class uninitialized_vector_bitmap<uint32_t>;
template<> class uninitialized_vector_bitmap<uint64_t>;
template<class Key, class T, class A=std::allocator<std::pair<const Key, T> > class randomized_binary_trie;
template<class Key, class T, class A=std::allocator<std::pair<const Key, T> > class redblack_tree;
*/
#define	XTL_CONCAT_DEF(x,y)	x##y

//#if defined(XTL_ITERATOR_CHECKS)
//#if XTL_CONCAT_DEF(1,XTL_ITERATOR_CHECKS) == 1
//#undef XTL_ITERATOR_CHECKS
//#endif
//#endif

#ifndef	XTL_ITERATOR_CHECKS
#if defined(_DEBUG) || defined(DEBUG)
#define	XTL_ITERATOR_CHECKS		2
#else
#define	XTL_ITERATOR_CHECKS		0
#endif
#endif

#if XTL_ITERATOR_CHECKS != 0
#define	XTL_ITERATOR_ASSERT1	assert
#else
#define	XTL_ITERATOR_ASSERT1(x)	void(0)
#endif

#if XTL_ITERATOR_CHECKS > 1
#define	XTL_ITERATOR_ASSERT2	assert
#else
#define	XTL_ITERATOR_ASSERT2(x)	void(0)
#endif

namespace xtl {
//-----------------------------------------------------------------------------

/// This property is used to support vector maps.
DECLARE_PROPERTY(integer);


/// This property distinguishes between sorted and unsorted containers
DECLARE_PROPERTY(sorted);


/// This propery is used for maps to indicate if a reverse positive addition
/// to all keys maintains integrity of the map. True for std::map<>,
/// xtl::unorderd_vector_map<>and xtl::unordered_block_vector_map<> but
/// false for all hash maps such as std::tr1::unordered_map<>.
/// 
/// The key must be an integer
DECLARE_PROPERTY(reverse_keyadd);


/// Vector maps use these properties to ensure non-integer keys are not 
/// supported. Defaults to unsupported.
template<class T> struct key_properties
{
	UNSUPPORTED_PROPERTY(integer);
	UNSUPPORTED_PROPERTY(reverse_keyadd);
};


/// @cond
// Helper for unordered_unordered_vector_map and unordered_unordered_vector_set
template<> struct key_properties<int>
{
	SUPPORTED_PROPERTY(integer);
	SUPPORTED_PROPERTY(reverse_keyadd);
};


template<> struct key_properties<unsigned>
{
	SUPPORTED_PROPERTY(integer);
	SUPPORTED_PROPERTY(reverse_keyadd);
};

template<> struct key_properties<int64_t>
{
	SUPPORTED_PROPERTY(integer);
	SUPPORTED_PROPERTY(reverse_keyadd);
};

template<> struct key_properties<uint64_t>
{
	SUPPORTED_PROPERTY(integer);
	SUPPORTED_PROPERTY(reverse_keyadd);
};
/// @endcond


/// Container traits
struct associative_container_tag { };
struct sequence_container_tag { };
struct adaptor_container_tag { };
struct not_a_container_tag { };


DECLARE_PROPERTY(allow_duplicate_keys);


template<class T>
struct container_traits
{
	typedef not_a_container_tag category;
	UNSUPPORTED_PROPERTY(allow_duplicate_keys);
	UNSUPPORTED_PROPERTY(sorted);
	typedef key_properties<T> key_props;
};


/// @cond
template<class T>
struct __set_traits
{
	typedef T container_type;
	static const typename T::key_type& key_from_value(const typename T::value_type& val) { return val; }
};


template<class T>
struct __map_traits
{
	typedef T container_type;
	static const typename T::key_type& key_from_value(const typename T::value_type& val) { return val.first; }
};

template<class Arg1, class Arg2, class Arg3, class Result>
struct ternary_function
{
	typedef Arg1 first_argument_type;
	typedef Arg2 second_argument_type;
	typedef Arg3 third_argument_type;
	typedef Result result_type;
};
/// @endcond

//-----------------------------------------------------------------------------
// END OF DECLARATIONS
}		// namespace xtl
#endif
