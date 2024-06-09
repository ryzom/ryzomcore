#ifndef NELNS_NAMING_SERVICE_FUNCTIONS_H
#define NELNS_NAMING_SERVICE_FUNCTIONS_H

#include <nel/net/inet_address.h>
#include <nel/net/buf_net_base.h>

/*
 * Unregisters a service if it has not been done before.
 * Note: this callback is called whenever someone disconnects from the NS.
 * May be there are too many calls if many clients perform many transactional lookups.
 */
void cbDisconnect(NLNET::TSockId from, void *arg);

/*
 * a service is connected, send him all services infos
 */
void cbConnect(NLNET::TSockId from, void *arg);

void checkWaitingUnregistrationServices();

#endif // NELNS_NAMING_SERVICE_FUNCTIONS_H
