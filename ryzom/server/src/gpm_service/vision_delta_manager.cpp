// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"

#include "nel/net/unified_network.h"
#include "vision_delta_manager.h"
#include "gpm_service.h"

const std::string FrontEndVisionMessageType = std::string("VISIONS_DELTA_2");


void CVisionDeltaManager::addVisionDelta(const CPlayerVisionDelta& visionDelta)
{
	// identify the correct front end for the player in question from the player id in the vision delta
	NLMISC::CEntityId eid= TheDataset.getEntityId(visionDelta.PlayerIndex);

	// serialise the result into the message
	NLNET::TServiceId dynamicId(eid.getDynamicId());
	if (_FrontEnds.find(dynamicId)==_FrontEnds.end())
	{
		// if the message didn't exist then intialise it
		_FrontEnds[dynamicId].setType(FrontEndVisionMessageType);
	}
	_FrontEnds[dynamicId].serial(const_cast<CPlayerVisionDelta&>(visionDelta));
}

void CVisionDeltaManager::update()
{
	TFrontEnds::iterator it;
	for (it=_FrontEnds.begin();it!=_FrontEnds.end();++it)
	{
		// extract the message and destination values from the map iterator
		const NLNET::TServiceId& destination	= it->first;
		NLNET::CMessage& msg	= it->second;

		// send the message
		sendMessageViaMirror( destination, msg );

		// clear out the message and prepare it for next time
		msg.clear();
		msg.setType(FrontEndVisionMessageType);
	}
}
