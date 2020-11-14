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

#ifndef GM_TP_PENDING_COMMAND_H
#define GM_TP_PENDING_COMMAND_H

#include "server_share/entity_state.h"

#include <map>
#include <string>

class CCharacter;

/**
 * CGmTpPendingCommand
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2004
 */
class CGmTpPendingCommand
{
	NL_INSTANCE_COUNTER_DECL(CGmTpPendingCommand);
public:

	typedef std::map< std::string, COfflineEntityState > TCharacterTpPending;
	
	// constructor
	CGmTpPendingCommand();
	
	// destructor
	~CGmTpPendingCommand();
	
	// get singleton instance
	static CGmTpPendingCommand * getInstance();
	
	// return true if we have tp pending and file state
	bool getTpPendingforCharacter( const std::string& CharacterName, COfflineEntityState& state, CCharacter& character );

	// add tp pending command
	void addTpPendingforCharacter( const std::string& CharacterName, const COfflineEntityState& state );

	// save map of tp pending
	void saveMap();

	// release
	static void release() { if( _Instance != 0 ) delete _Instance; } 

protected:

private:
	TCharacterTpPending _CharacterTpPending;

	static CGmTpPendingCommand * _Instance;
};

//typedef NLMISC::CDbgPtr<IEntity> IEntityPtr;

#endif //GM_TP_PENDING_COMMAND_H
