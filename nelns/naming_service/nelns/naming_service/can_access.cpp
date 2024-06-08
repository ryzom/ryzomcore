#include <nelns/naming_service/can_access.h>

#include <nel/misc/types_nl.h>

using std::vector;

using NLNET::CInetAddress;

bool canAccess (const vector<CInetAddress> &addr, const CServiceEntry &entry, vector<CInetAddress> &accessibleAddr)
{
	accessibleAddr.clear ();

	if (entry.WaitingUnregistration)
		return false;

	for (uint i = 0; i < addr.size(); i++)
	{
		uint32 net = addr[i].internalNetAddress();
		for (uint j = 0; j < entry.Addr.size(); j++)
		{
			if (net == entry.Addr[j].internalNetAddress())
			{
				accessibleAddr.push_back (entry.Addr[j]);
			}
		}
	}

	if (accessibleAddr.empty())
	{
		nldebug ("service %s-%hu is not accessible by '%s'", entry.Name.c_str(), entry.SId.get(), vectorCInetAddressToString (addr).c_str ());
	}
	else
	{
		nldebug ("service %s-%hu is accessible by '%s'", entry.Name.c_str(), entry.SId.get(), vectorCInetAddressToString (accessibleAddr).c_str ());
	}

	return !accessibleAddr.empty ();
}
