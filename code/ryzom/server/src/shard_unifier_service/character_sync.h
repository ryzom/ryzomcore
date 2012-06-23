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

#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"
#include "nel/misc/speaker_listener.h"

#ifndef CHARACTER_SYNCH_H
#define CHARACTER_SYNCH_H

namespace CHARSYNC
{



	class ICharacterSync;

	/** Callback interface to implement and register in the
	 *	entity character synchronizer to receive character update events.
	 */
	class ICharacterSyncCb : public NLMISC::CListener<ICharacterSync>
	{
	public:

		/** Callback called when the name of a character have been changed */
		virtual void onCharacterNameUpdated(uint32 charId, const std::string &oldName, const std::string &newName) =0;

		/** Callback called when a character is deleted/removed */
		virtual void onBeforeCharacterDelete(uint32 charId) =0;

	};


	/** Local interface for the character synchronizer */
	class ICharacterSync 
		:	public NLMISC::CManualSingleton<ICharacterSync>,
			public NLMISC::CSpeaker<ICharacterSyncCb>
	{
	public:
		/** Get the name of a user */
		virtual std::string getUserName(uint32 userId) =0;
		/** Get the name of a character */
		virtual ucstring getCharacterName(uint32 charId) =0;
		/// Try to find a shard id from a name and session id. Return 0 if not found
		virtual uint32 findCharId(const std::string &charName, uint32 homeSessionId) =0;
		/// Try to find a shard id from a name without session id, return 0 if 0 or more than one match.
		virtual uint32 findCharId(const std::string &charName) =0;

	};	

} // namespace ENTITYLOC

#endif // CHARACTER_SYNCH_H
