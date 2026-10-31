// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef NL_WINSOCK_VC6_H
#define NL_WINSOCK_VC6_H

// The Windows Sockets IPv6 interface (RFC 2553) for the Visual C++ 6.0 headers, which predate it:
// in6_addr, sockaddr_in6 with its scope, sockaddr_storage and addrinfo, and getaddrinfo, freeaddrinfo
// and getnameinfo, taken from Windows where it has them (Windows XP on) and emulated for IPv4 where
// it has not, as wspiapi.h does with later SDKs. Included in place of ws2tcpip.h, ws2ipdef.h and
// wspiapi.h.

#include "nel/misc/types_nl.h"

#include <winsock2.h>

// The VC98 ws2tcpip.h declares an early draft of sockaddr_in6, without sin6_scope_id
#define sockaddr_in6 nlVc98SockAddrIn6
#include <ws2tcpip.h>
#undef sockaddr_in6

struct in6_addr
{
	u_char s6_addr[16];
};

struct sockaddr_in6
{
	short sin6_family;
	u_short sin6_port;
	u_long sin6_flowinfo;
	struct in6_addr sin6_addr;
	u_long sin6_scope_id;
};

struct sockaddr_storage
{
	short ss_family;
	char __ss_pad1[6];
	__int64 __ss_align;
	char __ss_pad2[112];
};

struct addrinfo
{
	int ai_flags;
	int ai_family;
	int ai_socktype;
	int ai_protocol;
	size_t ai_addrlen;
	char *ai_canonname;
	struct sockaddr *ai_addr;
	struct addrinfo *ai_next;
};

#ifndef IPPROTO_IPV6
#define IPPROTO_IPV6 41
#endif
#ifndef IPV6_V6ONLY
#define IPV6_V6ONLY 27
#endif

#define AI_PASSIVE 0x01
#define AI_CANONNAME 0x02
#define AI_NUMERICHOST 0x04

#define INET_ADDRSTRLEN 22
#define INET6_ADDRSTRLEN 65

#define NI_MAXHOST 1025
#define NI_MAXSERV 32
#define NI_NOFQDN 0x01
#define NI_NUMERICHOST 0x02
#define NI_NAMEREQD 0x04
#define NI_NUMERICSERV 0x08
#define NI_DGRAM 0x10

#define EAI_AGAIN WSATRY_AGAIN
#define EAI_BADFLAGS WSAEINVAL
#define EAI_FAIL WSANO_RECOVERY
#define EAI_FAMILY WSAEAFNOSUPPORT
#define EAI_MEMORY WSA_NOT_ENOUGH_MEMORY
#define EAI_NONAME WSAHOST_NOT_FOUND
#define EAI_SERVICE WSATYPE_NOT_FOUND
#define EAI_SOCKTYPE WSAESOCKTNOSUPPORT

namespace NLNET {

int nlGetAddrInfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
void nlFreeAddrInfo(struct addrinfo *ai);
int nlGetNameInfo(const struct sockaddr *sa, int salen, char *host, DWORD hostlen, char *serv, DWORD servlen, int flags);
const char *nlGaiStrError(int error);

} // NLNET

#define getaddrinfo NLNET::nlGetAddrInfo
#define freeaddrinfo NLNET::nlFreeAddrInfo
#define getnameinfo NLNET::nlGetNameInfo
#define gai_strerror NLNET::nlGaiStrError

#endif // NL_WINSOCK_VC6_H

/* end of file */
