#ifndef NELNS_NAMING_SERVICE_EFFECTIVELY_REMOVE_H
#define NELNS_NAMING_SERVICE_EFFECTIVELY_REMOVE_H

#include <list>

#include <nelns/naming_service/service_entry.h>

std::list<CServiceEntry>::iterator effectivelyRemove (std::list<CServiceEntry>::iterator &it);

#endif // NELNS_NAMING_SERVICE_EFFECTIVELY_REMOVE_H
