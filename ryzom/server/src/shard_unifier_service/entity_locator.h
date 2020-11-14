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

#ifndef ENTITY_LOCATOR_H
#define	ENTITY_LOCATOR_H

namespace ENTITYLOC
{
	class IEntityLocator;

	/** Callback interface to implement and register in the
	 *	entity locator to receive character connect/disconnect events.
	 */
	class ICharacterEventCb : public NLMISC::CListener<IEntityLocator>
	{
	public:
		virtual void onUserConnection(NLNET::IModuleProxy *locatorHost, uint32 userId) =0;
		virtual void onUserDisconnection(NLNET::IModuleProxy *locatorHost, uint32 userId) =0;

		virtual void onCharacterConnection(NLNET::IModuleProxy *locatorHost, uint32 charId, uint32 lastDisconnectionDate) =0;
		virtual void onCharacterDisconnection(NLNET::IModuleProxy *locatorHost, uint32 charId) =0;

	};


	/** Local interface for the entity locator */
	class IEntityLocator 
		:	public NLMISC::CManualSingleton<IEntityLocator>,
			public NLMISC::CSpeaker<ICharacterEventCb>
	{
	public:

		/// Check if a user is online somewhere
		virtual bool isUserOnline(uint32 userId) = 0;
		
		/** Return the proxy on the locator module that claims to host the character
		 *	Return NULL if the character is not currently online.
		 */
		virtual NLNET::IModuleProxy *getLocatorModuleForChar(uint32 charId) =0;

		/** Return the proxy on the locator module that claims to host the character
		 *	Return NULL if the character is not currently online.
		 */
		virtual NLNET::IModuleProxy *getLocatorModuleForChar(const ucstring &charName) =0;

		/** Return the shard Id of the shard hosting the character (or 0 if not online)
		 */
		virtual uint32 getShardIdForChar(const ucstring &charName) =0;

		/** Return the module for a given shard id
		 *	return NULL if no module available for the specified shard id
		*/
		virtual NLNET::IModuleProxy *getLocatorModuleForShard(uint32 shardId)=0;

	};	

} // namespace ENTITYLOC

#endif // ENTITY_LOCATOR_H
