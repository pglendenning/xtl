#ifndef	ERRNO_9F0836FC_E9EA_4902_B573_BD518E06FABB
#define	ERRNO_9F0836FC_E9EA_4902_B573_BD518E06FABB
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

#if defined(WIN32) || defined(WIN64)
#include <winsock2.h>
#else
#include <errno.h>
#endif

namespace xtl {

template<class T> struct error { };

#if defined(WIN32) || defined(WIN64)
template<> struct error<HANDLE>
{
	typedef	DWORD	error_type;
	
	static error_type get_last_error()
	{
		return GetLastError();
	}
	
	static void set_last_error(error_type err)
	{
		SetLastError(err);
	}

	static void get_error_text(std::string& s, error_type err)
	{
		s.resize(512);
        DWORD n = FormatMessageA
        (
         FORMAT_MESSAGE_FROM_SYSTEM,
         NULL,
         err,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
         (char*)s.data(),
         (DWORD)s.size(),
         NULL
         );
		s.resize(n);
	}

	static void get_error_text(std::string& s)
	{
		get_error_text(s, get_last_error());
	}
};

template<> struct error<SOCKET>
{
	typedef	DWORD	error_type;
	
	static error_type get_last_error()
	{
		return WSAGetLastError();
	}
	
	static void set_last_error(error_type err)
	{
		WSASetLastError(err);
	}

	static void get_error_text(std::string& s, error_type err)
	{
		s.resize(512);
        DWORD n = FormatMessageA
        (
         FORMAT_MESSAGE_FROM_SYSTEM,
         NULL,
         err,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
         (char*)s.data(),
         (DWORD)s.size(),
         NULL
         );
		s.resize(n);
	}

	static void get_error_text(std::string& s)
	{
		get_error_text(s, get_last_error());
	}

	static const error_type T_EINTR                  = WSAEINTR;
	static const error_type T_EBADF                  = WSAEBADF;
	static const error_type T_EACCES                 = WSAEACCES;
	static const error_type T_EFAULT                 = WSAEFAULT;
	static const error_type T_EINVAL                 = WSAEINVAL;
	static const error_type T_EMFILE                 = WSAEMFILE;
	static const error_type T_EAGAIN                 = WSAEWOULDBLOCK;
	static const error_type T_EWOULDBLOCK            = WSAEWOULDBLOCK;
	static const error_type T_EINPROGRESS            = WSAEINPROGRESS;
	static const error_type T_EALREADY               = WSAEALREADY;
	static const error_type T_ENOTSOCK               = WSAENOTSOCK;
	static const error_type T_EDESTADDRREQ           = WSAEDESTADDRREQ;
	static const error_type T_EMSGSIZE               = WSAEMSGSIZE;
	static const error_type T_EPROTOTYPE             = WSAEPROTOTYPE;
	static const error_type T_ENOPROTOOPT            = WSAENOPROTOOPT;
	static const error_type T_EPROTONOSUPPORT        = WSAEPROTONOSUPPORT;
	static const error_type T_ESOCKTNOSUPPORT        = WSAESOCKTNOSUPPORT;
	static const error_type T_EOPNOTSUPP             = WSAEOPNOTSUPP;
	static const error_type T_EPFNOSUPPORT           = WSAEPFNOSUPPORT;
	static const error_type T_EAFNOSUPPORT           = WSAEAFNOSUPPORT;
	static const error_type T_EADDRINUSE             = WSAEADDRINUSE;
	static const error_type T_EADDRNOTAVAIL          = WSAEADDRNOTAVAIL;
	static const error_type T_ENETDOWN               = WSAENETDOWN;
	static const error_type T_ENETUNREACH            = WSAENETUNREACH;
	static const error_type T_ENETRESET              = WSAENETRESET;
	static const error_type T_ECONNABORTED           = WSAECONNABORTED;
	static const error_type T_ECONNRESET             = WSAECONNRESET;
	static const error_type T_ENOBUFS                = WSAENOBUFS;
	static const error_type T_EISCONN                = WSAEISCONN;
	static const error_type T_ENOTCONN               = WSAENOTCONN;
	static const error_type T_ESHUTDOWN              = WSAESHUTDOWN;
	static const error_type T_ETOOMANYREFS           = WSAETOOMANYREFS;
	static const error_type T_ETIMEDOUT              = WSAETIMEDOUT;
	static const error_type T_ECONNREFUSED           = WSAECONNREFUSED;
	static const error_type T_ELOOP                  = WSAELOOP;
	static const error_type T_ENAMETOOLONG           = WSAENAMETOOLONG;
	static const error_type T_EHOSTDOWN              = WSAEHOSTDOWN;
	static const error_type T_EHOSTUNREACH           = WSAEHOSTUNREACH;
	static const error_type T_ENOTEMPTY              = WSAENOTEMPTY;
	static const error_type T_EPROCLIM               = WSAEPROCLIM;
	static const error_type T_EUSERS                 = WSAEUSERS;
	static const error_type T_EDQUOT                 = WSAEDQUOT;
	static const error_type T_ESTALE                 = WSAESTALE;
	static const error_type T_EREMOTE                = WSAEREMOTE;
};

#else

template<> struct error<int>
{
	typedef int	error_type;
	
	static error_type get_last_error()
	{
		return errno; 
	}
	
	static void set_last_error(error_type err)
	{
		errno = err;
	}

	static void get_error_text(std::string& s, error_type err)
	{
		s = strerror(err);
	}
	
	static void get_error_text(std::string& s)
	{
		get_error_text(s, get_last_error());
	}

	static const error_type T_EINTR                  = EINTR;
	static const error_type T_EBADF                  = EBADF;
	static const error_type T_EACCES                 = EACCES;
	static const error_type T_EFAULT                 = EFAULT;
	static const error_type T_EINVAL                 = EINVAL;
	static const error_type T_EMFILE                 = EMFILE;
	static const error_type T_EAGAIN                 = EAGAIN;
	static const error_type T_EWOULDBLOCK            = EWOULDBLOCK;
	static const error_type T_EINPROGRESS            = EINPROGRESS;
	static const error_type T_EALREADY               = EALREADY;
	static const error_type T_ENOTSOCK               = ENOTSOCK;
	static const error_type T_EDESTADDRREQ           = EDESTADDRREQ;
	static const error_type T_EMSGSIZE               = EMSGSIZE;
	static const error_type T_EPROTOTYPE             = EPROTOTYPE;
	static const error_type T_ENOPROTOOPT            = ENOPROTOOPT;
	static const error_type T_EPROTONOSUPPORT        = EPROTONOSUPPORT;
	static const error_type T_ESOCKTNOSUPPORT        = ESOCKTNOSUPPORT;
	static const error_type T_EOPNOTSUPP             = EOPNOTSUPP;
	static const error_type T_EPFNOSUPPORT           = EPFNOSUPPORT;
	static const error_type T_EAFNOSUPPORT           = EAFNOSUPPORT;
	static const error_type T_EADDRINUSE             = EADDRINUSE;
	static const error_type T_EADDRNOTAVAIL          = EADDRNOTAVAIL;
	static const error_type T_ENETDOWN               = ENETDOWN;
	static const error_type T_ENETUNREACH            = ENETUNREACH;
	static const error_type T_ENETRESET              = ENETRESET;
	static const error_type T_ECONNABORTED           = ECONNABORTED;
	static const error_type T_ECONNRESET             = ECONNRESET;
	static const error_type T_ENOBUFS                = ENOBUFS;
	static const error_type T_EISCONN                = EISCONN;
	static const error_type T_ENOTCONN               = ENOTCONN;
	static const error_type T_ESHUTDOWN              = ESHUTDOWN;
	static const error_type T_ETOOMANYREFS           = ETOOMANYREFS;
	static const error_type T_ETIMEDOUT              = ETIMEDOUT;
	static const error_type T_ECONNREFUSED           = ECONNREFUSED;
	static const error_type T_ELOOP                  = ELOOP;
	static const error_type T_ENAMETOOLONG           = ENAMETOOLONG;
	static const error_type T_EHOSTDOWN              = EHOSTDOWN;
	static const error_type T_EHOSTUNREACH           = EHOSTUNREACH;
	static const error_type T_ENOTEMPTY              = ENOTEMPTY;
	//static const error_type T_EPROCLIM               = EPROCLIM;
	static const error_type T_EUSERS                 = EUSERS;
	static const error_type T_EDQUOT                 = EDQUOT;
	static const error_type T_ESTALE                 = ESTALE;
	static const error_type T_EREMOTE                = EREMOTE;
};

#endif
}		// xtl
#endif	// defined(ERRNO_9F0836FC_E9EA_4902_B573_BD518E06FABB)

