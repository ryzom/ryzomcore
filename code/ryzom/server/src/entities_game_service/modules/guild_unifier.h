
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
