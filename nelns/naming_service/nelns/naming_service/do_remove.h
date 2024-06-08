#ifndef NELNS_NAMING_SERVICE_DO_REMOVE_H
#define NELNS_NAMING_SERVICE_DO_REMOVE_H

#include <list>

#include <nelns/naming_service/service_entry.h>

/*
 * Helper procedure for cbLookupAlternate and cbUnregister.
 * Note: name is used for a LOGS.
 */
std::list<CServiceEntry>::iterator doRemove(std::list<CServiceEntry>::iterator it);

#endif // NELNS_NAMING_SERVICE_DO_REMOVE_H
