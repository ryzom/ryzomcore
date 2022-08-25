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



#ifndef NL_VISION_RECEIVER_H
#define NL_VISION_RECEIVER_H

#include "nel/misc/types_nl.h"
#include "game_share/player_vision_delta.h"
#include "entity_container.h"
#include <queue>


/**
 * <Class description>
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CVisionReceiver
{
public:

	/// Constructor
	CVisionReceiver();

	/// Initialisation
	void					init();

	/// Vision management : Get the first modified vision since the last EndUpdatedVision
	inline TEntityIndex		getFirstUpdatedVision()
	{
		return( _FirstUpdatedEntityVision );
	}

	/// Vision management : Get the next modified vision
	inline TEntityIndex		getNextUpdatedVision( const TEntityIndex& entityIndex )
	{
		return( TheEntityContainer->getEntity(entityIndex)->NextUpdatedEntityVision );
	}

	/// Vision management : Allow to loop on vision updates
	inline bool				visionChanged()
	{
		return _HasPendingDelta;
	}

	/// End an updating session
	void					endUpdatedVision();

protected:

	/// Apply a new delta of vision
	bool					setVision( const CPlayerVisionDelta &visionDelta );

	/// UpdateVision, unserial update vision message and process it
	void					updateNewVision( NLNET::CMessage& msgin );

	friend void cbDeltaNewVision( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

private:

	TEntityIndex										_FirstUpdatedEntityVision;

	// Queue to allow merging of several vision updates receives within the same cycle
	bool												_HasPendingDelta;
	std::queue< std::list<CPlayerVisionDelta> >			_NextDeltas;
};


// Callback for delta vision update received
void cbDeltaNewVision( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

#endif // NL_VISION_RECEIVER_H

/* End of vision_receiver.h */
