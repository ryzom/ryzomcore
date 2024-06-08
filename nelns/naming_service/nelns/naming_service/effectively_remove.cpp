#include <nelns/naming_service/effectively_remove.h>

#include <nelns/naming_service/variables.h>

using std::list;

list<CServiceEntry>::iterator effectivelyRemove(list<CServiceEntry>::iterator &it)
{
	// remove the service from the registered service list
	nlinfo ("Effectively remove the service %s-%hu", (*it).Name.c_str(), it->SId.get());
	return RegisteredServices.erase (it);
}
