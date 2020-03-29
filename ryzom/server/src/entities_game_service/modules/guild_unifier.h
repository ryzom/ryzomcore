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

#ifndef GUILD_UNIFIER_H
#define GUILD_UNIFIER_H

#include "nel/misc/entity_id.h"
#include "nel/misc/singleton.h"
#include "game_share/guild_grade.h"
#include "game_share/string_manager_sender.h"

class CGuild;
class IGuild;

/** Each EGS embed a guild unifier server and client.
 *	The server is receive update from the local real managed guild
 *	and broadcast this update to all the clients.
 *	The server also receive guild operation request from foreign
 *	guild proxies and callback the real guild implementation
 *	to do the job.
 *
 *	The client, on the other hand, receive guild operation request from
 *	local proxies and send them to the correct server (doing a first
 *	level of dispatching).
 *	They also receive update from the server and callbacks proxies
 *	implementation to update their locale cache and clients views.
 */

/** Local interface to the guild unifier module */
class IGuildUnifier : public NLMISC::CManualSingleton<IGuildUnifier>
{
	
public:
	
	/// The guild manager ask to send all guild info a all known clients
	virtual void broadcastAllGuilds() =0;
	/// A guild has been added
	virtual void guildCreated(const CGuild *guild) =0;
	/// A guild has been deleted
	virtual void guildDeleted(uint32 guildId) =0;
	/// Broadcast a guild message
	virtual void sendMessageToGuildMembers( const CGuild *guild, const std::string &  msg, const TVectorParamCheck & params ) =0;
	/// Broadcast a guild update (guilde base data and fames values)
	virtual void broadcastGuildUpdate(IGuild *guild) = 0;


	/// Called by guild manager when about to release it's resource
	virtual void guildManagerReleased() =0;
};


#endif // GUILD_UNIFIER_H
