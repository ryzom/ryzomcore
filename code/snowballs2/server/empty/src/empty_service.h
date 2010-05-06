/*
 * Snowballs service.
 *
 * $Id: empty_service.h 409 2007-12-28 13:24:17Z Kaetemi $
 */

#ifndef SERVICE_EMPTY_SERVICE_H
#define SERVICE_EMPTY_SERVICE_H

// This include is mandatory to use NeL. It include NeL types.
#include <nel/misc/types_nl.h>

// Some other NeL structures that can be used.
#include <nel/misc/vector.h>
#include <nel/misc/time_nl.h>

// And we're also using the NeL Service framework, layer 5.
#include <nel/net/service.h>

namespace SBSERVICE {

class CEmptyService : public NLNET::IService
{
public:
	virtual void commandStart();
	virtual void init();
	virtual bool update();
	virtual void release();

	static void msgWater(const std::string service, float water);
	static void msgFire(NLNET::TServiceId sid, ucstring fire);

	static void cbSnow(NLNET::CMessage &msgin, const std::string &serviceName, NLNET::TServiceId sid);
	static void cbIce(NLNET::CMessage &msgin, const std::string &serviceName, NLNET::TServiceId sid);

	static void cbUp(const std::string &serviceName, NLNET::TServiceId sid, void *arg);
};

}

#endif /* SERVICE_EMPTY_SERVICE_H */

/* end of file */
