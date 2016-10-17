#ifndef	MAP_16B182B6_B29E_413D_A200_27EBABCC05B4
#define	MAP_16B182B6_B29E_413D_A200_27EBABCC05B4
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
/// @brief	Property extension to STL maps. 
/// @author Paul Glendenning
/// @date

#include <map>
#include "property.hpp"

namespace xtl {
//-----------------------------------------------------------------------------

template<class K, class T, class C, class A>
struct container_traits<std::map<K,T,C,A> >: public __map_traits<std::map<K,T,C,A> >
{
	typedef associative_container_tag category;
	UNSUPPORTED_PROPERTY(allow_duplicate_keys);
	SUPPORTED_PROPERTY(sorted);
	typedef key_properties<K> key_props;
};

template<class K, class T, class C, class A>
struct container_traits<std::multimap<K,T,C,A> >: public __map_traits<std::multimap<K,T,C,A> >
{
	typedef associative_container_tag category;
	SUPPORTED_PROPERTY(allow_duplicate_keys);
	SUPPORTED_PROPERTY(sorted);
	typedef key_properties<K> key_props;
};

//-----------------------------------------------------------------------------
// END OF DECLARATIONS
}		// namespace xtl
#endif
