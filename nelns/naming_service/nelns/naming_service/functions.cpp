#include <nelns/naming_service/functions.h>

#include <nelns/naming_service/do_unregister_service.h>

using NLNET::CInetAddress;
using NLNET::TServiceId;
using NLNET::TSockId;

/*
 * Unregisters a service if it has not been done before.
 * Note: this callback is called whenever someone disconnects from the NS.
 * May be there are too many calls if many clients perform many transactional lookups.
 */
void cbDisconnect(TSockId from, void *arg)
{
	doUnregisterService(from);
	// displayRegisteredServices ();
}

/*
 * a service is connected, send him all services infos
 */
void cbConnect(TSockId from, void *arg)
{
	// we have to wait the registred services message to send all services because it this points, we can't know which sub net
	// the service can use

	// displayRegisteredServices ();

	// set the appid with a bad id (-1)
	from->setAppId(~0);
}
