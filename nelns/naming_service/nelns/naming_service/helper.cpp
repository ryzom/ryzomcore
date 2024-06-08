#include <nelns/naming_service/helper.h>

#include <list>
#include <nelns/naming_service/service_entry.h>
#include <nelns/naming_service/variables.h>

using std::list;
using std::string;

using NLNET::TServiceId;

/*
 * Helper that emulate layer5's getServiceName()
 */
string getServiceName( TServiceId  sid )
{
	list<CServiceEntry>::iterator it;
	for (it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
	{
		if ((*it).SId == sid)
		{
			return (*it).Name;
		}
	}
	return ""; // not found
}

