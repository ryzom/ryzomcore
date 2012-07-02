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

#include "nel/misc/path.h"
#include "nel/misc/file.h"

#include "server_share/mail_forum_validator.h"

#include "shard_stat_db_manager.h"
#include "hof_generator.h"

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif // NL_OS_WINDOWS

using namespace NLNET;
using namespace NLMISC;
using namespace std;


// force admin module to link in
extern void admin_modules_forceLink();
void foo()
{
	admin_modules_forceLink();
}


CVariable<std::string>	WebRootDirectory("web", "WebRootDirectory", "Set Web directory access", "www/", 0, true);
CVariable<std::string>	IncomingMailDirectory("web", "IncomingMailDirectory", "Directory to check for new mail files (in WebRootDirectory)", "incoming", 0, true);


/**
 * CMailForumService
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2004
 */
class CMailForumService : public NLNET::IService
{
public:

	/** 
	 * init the service
	 */
	void init()
	{
		LastCheckMail = 0;
		MailFile = NULL;
		CShardStatDBManager::init();
	}

	/**
	 * main loop
	 */
	bool update()
	{
		checkMail();
		CHoFGenerator::getInstance()->serviceUpdate();

		return true;
	}
	
	/**
	 * release
	 */
	void release()
	{
	}

	TTime		LastCheckMail;
	FILE*		MailFile;

	// Open/close session
	static void	openSession( uint32 shardid, string username, string cookie );
	static void	closeSession( uint32 shardid, string username );
	static void	cbOpenSession( CMessage& msgin, const std::string &serviceName, TServiceId serviceId );
	static void	cbCloseSession( CMessage& msgin, const std::string &serviceName, TServiceId serviceId );

	// Remove user/guild
	static void	removeUser( uint32 shardid, string username);
	static void	removeGuild( uint32 shardid, string username );
	static void	cbRemoveUser( CMessage& msgin, const std::string &serviceName, TServiceId serviceId );
	static void	cbRemoveGuild( CMessage& msgin, const std::string &serviceName, TServiceId serviceId );

	//
	static void	changeUserName(uint32 shardid, const string& oldName, const string& newName);
	static void	cbChangeUserName( CMessage& msgin, const std::string &serviceName, TServiceId serviceId );

	//
	void		checkMail();
	void		checkFile(const std::string& file);
};





TUnifiedCallbackItem CbArray[]=
{
	{ "OPEN_SESSION",		CMailForumService::cbOpenSession },
	{ "CLOSE_SESSION",		CMailForumService::cbCloseSession },

	{ "REMOVE_USER",		CMailForumService::cbRemoveUser },
	{ "REMOVE_GUILD",		CMailForumService::cbRemoveGuild },

	{ "CHANGE_UNAME",		CMailForumService::cbChangeUserName },
};

NLNET_SERVICE_MAIN( CMailForumService, "MFS", "mail_forum_service", 49980, CbArray, "", "" )





void	CMailForumService::checkMail()
{
	TTime	now = CTime::getLocalTime();
	if (now - LastCheckMail < 10*1000)
		return;

	LastCheckMail = now;
	string	dir = CPath::standardizePath(WebRootDirectory)+IncomingMailDirectory.get();

	// no path, no mails
	if (!CFile::isExists(dir))
		return;

	std::vector<std::string>	files;
	CPath::getPathContent(dir, false, false, true, files);

	if (files.empty())
		return;

	uint	i;
	for (i=0; i<files.size(); ++i)
		checkFile(files[i]);
}

void	CMailForumService::checkFile(const std::string& file)
{
	uint	fsz = CFile::getFileSize(file);

	if (fsz == 0)
		return;

	vector<uint8>	buffer(fsz);

	CIFile	fi;
	if (!fi.open(file))
		return;

	fi.serialBuffer(&(buffer[0]), fsz);
	fi.close();

	char*	pb = (char*)(&(buffer[0]));
	char*	pt = pb;

	while (*pt != '\0' && strncmp(pt, "$$$$", 4))
		++pt;

	// file contents "$$$$" -> end of file marker, file is complete, can be deleted
	if (pt != '\0')
	{
		CFile::deleteFile(file);

		int		shard_id;
		char	to_username[256];
		char	from_username[256];

		int		scanned = sscanf(pb, "shard=%d to=%s from=%s", &shard_id, to_username, from_username);

		CMessage	msgout("MAIL_NOTIF");

		uint32	shardId = (uint32)shard_id;
		string	toUserName = to_username;
		string	fromUserName = from_username;

		nldebug("MAIL: sent notification for user '%s' on shard '%d'", toUserName.c_str(), shardId);

		msgout.serial(shardId, toUserName, fromUserName);

		CUnifiedNetwork::getInstance()->send("EGS", msgout);
	}
}



string	getUserDirectory(uint32 shardid, const string& userName)
{
	string	un = toLower(CMailForumValidator::nameToFile(userName));

	string	dir = CPath::standardizePath(WebRootDirectory)+toString(shardid)+"/"+un.substr(0, 2)+"/"+un+"/";

	if (!CFile::isExists(dir))
	{
		CFile::createDirectoryTree(dir);
		CFile::setRWAccess(dir);
	}

	return dir;
}

string getGuildDirectory(uint32 shardid, const string &guildName)
{
	string	un = toLower(CMailForumValidator::nameToFile(guildName));

	string	dir = CPath::standardizePath(WebRootDirectory)+toString(shardid)+"/"+un.substr(0, 2)+"/"+un+"/";

	if (!CFile::isExists(dir))
	{
		CFile::createDirectoryTree(dir);
		CFile::setRWAccess(dir);
	}

	return dir;
}

void	CMailForumService::openSession( uint32 shardid, string username, string cookie )
{
	string	sessionfile = getUserDirectory(shardid, username) + "session";
	string	checkmailfile = getUserDirectory(shardid, username) + "new_mails";

	COFile	ofile;
	if (ofile.open(sessionfile))
	{
		cookie += "\n";
		ofile.serialBuffer((uint8*)(&cookie[0]), (uint)cookie.size());
	}

	if (CFile::fileExists(checkmailfile))
	{
		CFile::deleteFile(checkmailfile);
		CMessage	msgout("MAIL_NOTIF");
		msgout.serial(shardid, username);
		CUnifiedNetwork::getInstance()->send("EGS", msgout);
	}
}

void	CMailForumService::closeSession( uint32 shardid, string username )
{
	string	sessionfile = getUserDirectory(shardid, username) + "session";
	CFile::deleteFile(sessionfile);
}



void	CMailForumService::changeUserName(uint32 shardid, const string& oldName, const string& newName)
{
	string	olddir = getUserDirectory(shardid, oldName);
	string	newdir = getUserDirectory(shardid, newName);

	std::vector<std::string>	files;
	CPath::getPathContent(olddir, false, false, true, files);

	uint	i;
	for (i=0; i<files.size(); ++i)
		CFile::moveFile((newdir+CFile::getFilename(files[i])).c_str(), files[i].c_str());
}



//

void	CMailForumService::cbOpenSession( CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	try
	{
		uint32	shardId;
		string	userName;
		string	cookie;

		msgin.serial(shardId);
		msgin.serial(userName);
		msgin.serial(cookie);

		openSession(shardId, userName, cookie);
	}
	catch(const Exception& e)
	{
		nlwarning("Failed to open session: %s", e.what());
	}
}

void	CMailForumService::cbCloseSession( CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	try
	{
		uint32	shardId;
		string	userName;

		msgin.serial(shardId);
		msgin.serial(userName);

		closeSession(shardId, userName);
	}
	catch(const Exception& e)
	{
		nlwarning("Failed to close session: %s", e.what());
	}
}

// ****************************************************************************
void CMailForumService::cbRemoveUser( CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	try
	{
		uint32	shardId;
		string	userName;

		msgin.serial(shardId);
		msgin.serial(userName);

		removeUser(shardId, userName);
	}
	catch(const Exception& e)
	{
		nlwarning("Failed to remove user: %s", e.what());
	}
}

// ****************************************************************************
void CMailForumService::cbRemoveGuild( CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	try
	{
		uint32	shardId;
		string	guildName;

		msgin.serial(shardId);
		msgin.serial(guildName);

		removeGuild(shardId, guildName);
	}
	catch(const Exception& e)
	{
		nlwarning("Failed to remove guild: %s", e.what());
	}
}

// ****************************************************************************
void CMailForumService::removeUser( uint32 shardid, string username )
{
	string	userDir = getUserDirectory(shardid, username);
	if (userDir[userDir.size()-1] == '/')
		userDir = userDir.substr(0, userDir.size()-1);

	uint32 i = 0;
	string userDelDir;
	do
	{
		userDelDir = userDir + ".deleted." +toString("%05d", i);
		++i;
	}
	while (CFile::isExists(userDelDir));

	CFile::moveFile(userDelDir.c_str(), userDir.c_str());
}

// ****************************************************************************
void CMailForumService::removeGuild( uint32 shardid, string guildname )
{
	string	guildDir = getGuildDirectory(shardid, guildname);
	if (guildDir[guildDir.size()-1] == '/')
		guildDir = guildDir.substr(0, guildDir.size()-1);

	uint32 i = 0;
	string guildDelDir;
	do
	{
		guildDelDir = guildDir + ".deleted." +toString("%05d", i);
		++i;
	}
	while (CFile::isExists(guildDelDir));

	CFile::moveFile(guildDelDir.c_str(), guildDir.c_str());
}


void	CMailForumService::cbChangeUserName( CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	try
	{
		uint32	shardId;
		string	oldName;
		string	newName;

		msgin.serial(shardId);
		msgin.serial(oldName);
		msgin.serial(newName);

		changeUserName(shardId, oldName, newName);
	}
	catch(const Exception& e)
	{
		nlwarning("Failed to close session: %s", e.what());
	}
}

//

NLMISC_COMMAND(openSession,"open a mail/forum session for a player", "shardid username cookie")
{
	if (args.size() != 3)
		return false;

	uint32 shardId;
	NLMISC::fromString(args[0], shardId);

	CMailForumService::openSession(shardId, args[1], args[2]);

	return true;
}

NLMISC_COMMAND(closeSession, "close a mail/forum session for a player", "shardid username")
{
	if (args.size() != 2)
		return false;

	uint32 shardId;
	NLMISC::fromString(args[0], shardId);

	CMailForumService::closeSession(shardId, args[1]);

	return true;
}

NLMISC_COMMAND(changeUserName, "change a user's name (guild or player)", "shardid oldname newname")
{
	if (args.size() != 3)
		return false;

	uint32 shardId;
	NLMISC::fromString(args[0], shardId);

	CMailForumService::changeUserName( shardId, args[1], args[2] );

	return true;
}

