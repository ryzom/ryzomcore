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

#include "stdnet.h"

#ifdef NL_COMP_VC6

#include "winsock_vc6.h"

namespace NLNET {

namespace /* anonymous */ {

typedef int (WSAAPI *TGetAddrInfo)(const char *, const char *, const struct addrinfo *, struct addrinfo **);
typedef void (WSAAPI *TFreeAddrInfo)(struct addrinfo *);
typedef int (WSAAPI *TGetNameInfo)(const struct sockaddr *, int, char *, DWORD, char *, DWORD, int);

void WSAAPI legacyFreeAddrInfo(struct addrinfo *ai)
{
	while (ai)
	{
		struct addrinfo *next = ai->ai_next;
		free(ai);
		ai = next;
	}
}

// IPv4 emulation where Windows has no getaddrinfo (before Windows XP); numeric services only
int WSAAPI legacyGetAddrInfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res)
{
	*res = NULL;
	int flags = hints ? hints->ai_flags : 0;
	int family = hints ? hints->ai_family : AF_UNSPEC;
	if (family != AF_UNSPEC && family != AF_INET)
		return EAI_FAMILY;
	if (!node && !service)
		return EAI_NONAME;

	u_short port = 0;
	if (service)
	{
		char *end;
		unsigned long p = strtoul(service, &end, 10);
		if (*end || p > 0xFFFF)
			return EAI_SERVICE;
		port = htons((u_short)p);
	}

	u_long single;
	u_long *singleList[2] = { &single, NULL };
	char **addresses = (char **)singleList;
	if (!node)
	{
		single = htonl((flags & AI_PASSIVE) ? INADDR_ANY : INADDR_LOOPBACK);
	}
	else if ((single = inet_addr(node)) == INADDR_NONE && strcmp(node, "255.255.255.255"))
	{
		if (flags & AI_NUMERICHOST)
			return EAI_NONAME;
		hostent *h = gethostbyname(node);
		if (!h)
		{
			int error = WSAGetLastError();
			return (error == WSATRY_AGAIN || error == WSANO_RECOVERY) ? error : EAI_NONAME;
		}
		if (h->h_addrtype != AF_INET || h->h_length != sizeof(u_long))
			return EAI_FAMILY;
		addresses = h->h_addr_list;
	}

	struct addrinfo **next = res;
	for (char **address = addresses; *address; ++address)
	{
		// One block per address, the sockaddr_in after the addrinfo
		struct addrinfo *ai = (struct addrinfo *)calloc(1, sizeof(struct addrinfo) + sizeof(sockaddr_in));
		if (!ai)
		{
			legacyFreeAddrInfo(*res);
			*res = NULL;
			return EAI_MEMORY;
		}
		sockaddr_in *sin = (sockaddr_in *)(ai + 1);
		sin->sin_family = AF_INET;
		sin->sin_port = port;
		memcpy(&sin->sin_addr, *address, sizeof(u_long));
		ai->ai_family = AF_INET;
		ai->ai_socktype = hints ? hints->ai_socktype : 0;
		ai->ai_protocol = hints ? hints->ai_protocol : 0;
		ai->ai_addrlen = sizeof(sockaddr_in);
		ai->ai_addr = (struct sockaddr *)sin;
		*next = ai;
		next = &ai->ai_next;
	}
	return 0;
}

// IPv4 emulation where Windows has no getnameinfo (before Windows XP)
int WSAAPI legacyGetNameInfo(const struct sockaddr *sa, int salen, char *host, DWORD hostlen, char *serv, DWORD servlen, int flags)
{
	if (!sa || sa->sa_family != AF_INET || salen < (int)sizeof(sockaddr_in))
		return EAI_FAMILY;
	const sockaddr_in *sin = (const sockaddr_in *)sa;

	if (host && hostlen)
	{
		const char *name = NULL;
		if (!(flags & NI_NUMERICHOST))
		{
			hostent *h = gethostbyaddr((const char *)&sin->sin_addr, sizeof(sin->sin_addr), AF_INET);
			if (h)
				name = h->h_name;
			else if (flags & NI_NAMEREQD)
				return EAI_NONAME;
		}
		if (!name)
			name = inet_ntoa(sin->sin_addr);
		if (strlen(name) >= hostlen)
			return EAI_FAIL;
		strcpy(host, name);
	}

	if (serv && servlen)
	{
		char port[8];
		sprintf(port, "%u", (unsigned int)ntohs(sin->sin_port));
		if (strlen(port) >= servlen)
			return EAI_FAIL;
		strcpy(serv, port);
	}
	return 0;
}

// Set on first use, s_GetAddrInfo last, so freeaddrinfo always matches the getaddrinfo that allocated
volatile TGetAddrInfo s_GetAddrInfo = NULL;
volatile TFreeAddrInfo s_FreeAddrInfo = NULL;
volatile TGetNameInfo s_GetNameInfo = NULL;

void initFunctions()
{
	// ws2_32.dll has them from Windows XP on, wship6.dll from the Windows 2000 IPv6 preview
	static const char *const modules[] = { "ws2_32.dll", "wship6.dll" };
	for (uint i = 0; i < sizeof(modules) / sizeof(modules[0]); ++i)
	{
		HMODULE module = LoadLibraryA(modules[i]);
		if (!module)
			continue;
		TGetAddrInfo getAddrInfo = (TGetAddrInfo)GetProcAddress(module, "getaddrinfo");
		TFreeAddrInfo freeAddrInfo = (TFreeAddrInfo)GetProcAddress(module, "freeaddrinfo");
		TGetNameInfo getNameInfo = (TGetNameInfo)GetProcAddress(module, "getnameinfo");
		if (getAddrInfo && freeAddrInfo && getNameInfo)
		{
			s_FreeAddrInfo = freeAddrInfo;
			s_GetNameInfo = getNameInfo;
			s_GetAddrInfo = getAddrInfo;
			return; // the module stays loaded
		}
		FreeLibrary(module);
	}
	s_FreeAddrInfo = legacyFreeAddrInfo;
	s_GetNameInfo = legacyGetNameInfo;
	s_GetAddrInfo = legacyGetAddrInfo;
}

} /* anonymous namespace */

int nlGetAddrInfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res)
{
	if (!s_GetAddrInfo)
		initFunctions();
	return s_GetAddrInfo(node, service, hints, res);
}

void nlFreeAddrInfo(struct addrinfo *ai)
{
	// Only results of nlGetAddrInfo come here, so the functions are set
	s_FreeAddrInfo(ai);
}

int nlGetNameInfo(const struct sockaddr *sa, int salen, char *host, DWORD hostlen, char *serv, DWORD servlen, int flags)
{
	if (!s_GetAddrInfo)
		initFunctions();
	return s_GetNameInfo(sa, salen, host, hostlen, serv, servlen, flags);
}

const char *nlGaiStrError(int error)
{
	switch (error)
	{
	case EAI_AGAIN: return "Temporary failure in name resolution";
	case EAI_BADFLAGS: return "Invalid flags";
	case EAI_FAIL: return "Non-recoverable failure in name resolution";
	case EAI_FAMILY: return "Address family not supported";
	case EAI_MEMORY: return "Memory allocation failure";
	case EAI_NONAME: return "Name or service not known";
	case EAI_SERVICE: return "Service not supported for socket type";
	case EAI_SOCKTYPE: return "Socket type not supported";
	default: return "Unknown error";
	}
}

} /* namespace NLNET */

#endif /* NL_COMP_VC6 */

/* end of file */
