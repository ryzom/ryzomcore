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




#ifndef GD_AGS_TEST_H
#define GD_AGS_TEST_H

// Nel Misc
#include "nel/misc/types_nl.h"

// Nel Net
#include "nel/net/service.h"

// Game share
#include "game_share/tick_event_handler.h"


// Callback called at service connexion
void cbServiceUp( const std::string& serviceName, uint16 serviceId, void * );

// Callback called at service down
void cbServiceDown( const std::string& serviceName, uint16 serviceId, void * );

// "Callbacks" for Tick service
void cbSync();


/**
 * <Class description>
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2001
 */
class CAgsTest : public NLNET::IService
{
public:
	// Initialisation of service
	void init (void);

	// Update net processing 
	bool update (void);

	// Update service processing
	static void serviceUpdate(void);

	// Release the service
	void release (void);
};

#endif // GD_AGS_TEST_H
/* End of ags_test.h */
