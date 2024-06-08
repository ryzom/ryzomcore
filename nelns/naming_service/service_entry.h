#ifndef NL_SERVICE_ENTRY_H
#define NL_SERVICE_ENTRY_H

#include <list>
#include <string>

#include <nel/misc/time_nl.h>
#include <nel/net/buf_sock.h>
#include <nel/net/inet_address.h>
#include <nel/net/unified_network.h>

struct CServiceEntry
{
	CServiceEntry(NLNET::TSockId sock, const std::vector<NLNET::CInetAddress> &a, const std::string &n, NLNET::TServiceId s)
	    : SockId(sock)
	    , Addr(a)
	    , Name(n)
	    , SId(s)
	    , WaitingUnregistration(false)
	{
	}

	NLNET::TSockId SockId; // the connection between the service and the naming service
	std::vector<NLNET::CInetAddress> Addr; // address to send to the service who wants to lookup this service
	                                       // it s possible to have more than one addr, anyway, the naming service
	                                       // will send good address depending of the sub net address of the service
	std::string Name; // name of the service
	NLNET::TServiceId SId; // id of the service

	bool WaitingUnregistration; // true if this service is in unregistration process (wait other service ACK)
	NLMISC::TTime WaitingUnregistrationTime; // time of the beginning of the inregistration process
	std::list<NLNET::TServiceId> WaitingUnregistrationServices; // list of service that we wait the answer
};

#endif // NL_SERVICE_ENTRY_H
