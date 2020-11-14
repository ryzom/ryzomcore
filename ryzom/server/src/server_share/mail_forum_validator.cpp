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



// pch
#include "stdpch.h"

#include "mail_forum_validator.h"
#include "game_share/backup_service_interface.h"

#include <nel/misc/file.h>
#include <nel/misc/config_file.h>
#include <nel/misc/path.h>
#include <nel/net/service.h>
#include <nel/misc/variable.h>
#include "game_share/shard_names.h"
#include "game_share//r2_basic_types.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;



// Mail and forum shard id (default is 666, d'uh) -- DEPRECATED
uint32			CMailForumValidator::_ShardId = 666;

// Initialised yet?
bool			CMailForumValidator::_Initialised = false;

// Mail notification
CMailForumValidator::TMailNotification	CMailForumValidator::_Notification = NULL;


// Use Mail/Forum
CVariable<bool>		UseMailForum("web", "UseMailForum", "Allow mail/forum validation", true, 0, true);

// For a mainland shard (non ring), we need the session id to initialize the normal position stack with
// the current far position after loading an old character file with no stored session id.
// (Alternatively, the ServerAnimationModule could sent it to any mainland shard, looking up in the DB
// ring:sessions).
NLMISC::CVariable<uint32> FixedSessionId( "egs", "FixedSessionId", "For a mainland shard, the session id", 0, 0, true );

/*
 * Constructor
 */
CMailForumValidator::CMailForumValidator()
{
}

// Set mail notification
void	CMailForumValidator::setNotification(TMailNotification callback)
{
	_Notification = callback;
}

/* get shard relative name */
std::string		CMailForumValidator::getShardRelativeName(const std::string &s)
{
	std::string		relName;
	TSessionId		dummy;
	CShardNames::getInstance().parseRelativeName(TSessionId(FixedSessionId), s, relName, dummy);
	return relName;
}


/*
 * Validate mail/forum user entry
 */
void CMailForumValidator::validateUserEntry(uint32 homeMainlandId, const string &userName, const string &cookie)
{
	if (!UseMailForum.get())
		return;

	if (!init())
		return;

	CMessage	msgout("OPEN_SESSION");

//	uint32	shardid = IService::getInstance()->getShardId();
	string	username = getShardRelativeName(userName);
	string	c = cookie;

	msgout.serial(homeMainlandId, username, c);
	CUnifiedNetwork::getInstance()->send("MFS", msgout);
}

/*
 * Unvalidate mail/forum user entry
 */
void CMailForumValidator::unvalidateUserEntry(uint32 homeMainlandId, const string &userName)
{
	if (!UseMailForum.get())
		return;

	if (!init())
		return;

	CMessage	msgout("CLOSE_SESSION");

//	uint32	shardid = IService::getInstance()->getShardId();
	string	username = getShardRelativeName(userName);

	msgout.serial(homeMainlandId, username);
	CUnifiedNetwork::getInstance()->send("MFS", msgout);
}


/*
 * Change player/guild name
 */
void	CMailForumValidator::changeUserName(uint32 homeMainlandId, const std::string& oldname, const std::string& newname)
{
	if (!UseMailForum.get())
		return;

	if (!init())
		return;

	CMessage	msgout("CHANGE_UNAME");

//	uint32	shardid = IService::getInstance()->getShardId();
	string	oldName = getShardRelativeName(oldname);
	string	newName = getShardRelativeName(newname);

	msgout.serial(homeMainlandId, oldName, newName);
	CUnifiedNetwork::getInstance()->send("MFS", msgout);
}

void CMailForumValidator::removeUser(uint32 homeMainlandId, const std::string &userName)
{
	if (!init())
		return;

	CMessage msgout("REMOVE_USER");

//	uint32 shardid = IService::getInstance()->getShardId();
	string username = getShardRelativeName(userName);

	msgout.serial(homeMainlandId, username);
	CUnifiedNetwork::getInstance()->send("MFS", msgout);
}

void CMailForumValidator::removeGuild(const std::string &guildName)
{
	if (!init())
		return;

	CMessage msgout("REMOVE_GUILD");

	uint32 shardid = IService::getInstance()->getShardId();
	string guildname = guildName;

	msgout.serial(shardid, guildname);
	CUnifiedNetwork::getInstance()->send("MFS", msgout);
}

static TUnifiedCallbackItem CbArray[]=
{
	{ "MAIL_NOTIF",			CMailForumValidator::cbMailNotification },
};

// received new mail!
void	CMailForumValidator::cbMailNotification( NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	uint32	shardId;
	msgin.serial(shardId);
	if (shardId != IService::getInstance()->getShardId())
		return;

	string	toUser;
	msgin.serial(toUser);

	string	fromUser;
	if (msgin.getPos() < (sint32)msgin.length())
	{
		msgin.serial(fromUser);
	}

	if (_Notification != NULL)
	{
		_Notification(toUser, fromUser);
	}
	else
	{
		nldebug("Received mail '@%s' (from '@%s'), event is not notified (callback not set)", toUser.c_str(), fromUser.c_str());
	}
}


/*
 * init
 */
bool	CMailForumValidator::init()
{
	if (_Initialised)
		return true;

	_Initialised = true; // must be set immediately to avoid infinite recursion (see addService() below)
						 // because init() called by service init() AND by CStatDB::cbMFServiceUp()

	IService	*service = IService::getInstance();

	if (service == NULL)
	{
		nlwarning("WEB: Can't initialise Mail/Forum validator, service not started yet.");
		_Initialised = false;
		return false;
	}

	CConfigFile::CVar	*hostvar = service->ConfigFile.getVarPtr("MFSHost");

	if (hostvar != NULL && !hostvar->asString().empty())
	{
		string host = hostvar->asString();
		if (host.find (":") == string::npos)
			host+= ":49980";

		CUnifiedNetwork::getInstance()->addService("MFS", CInetAddress(host)); // warning: can call the serviceUp callback => initMFS() again
	}
	else
	{
		_Initialised = false;
		return false;
	}

	CUnifiedNetwork::getInstance()->addCallbackArray(CbArray, sizeof(CbArray)/sizeof(CbArray[0]));

	return true;
}
