// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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
