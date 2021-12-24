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



#ifndef NL_MAIL_FORUM_VALIDATOR_H
#define NL_MAIL_FORUM_VALIDATOR_H

#include "nel/misc/types_nl.h"


/**
 * Allow validation of a user mail/forum account.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CMailForumValidator
{
public:

	typedef void	(*TMailNotification)(const std::string& username, const std::string& from);

	/// Initialisation
	/// It adds the MFS if not done yet and return true if ok
	static bool		init();

	/// Validate mail/forum user entry
	static void		validateUserEntry(uint32 homeMainlandId, const std::string &userName, const std::string &cookie);

	/// Unvalidate mail/forum user entry
	static void		unvalidateUserEntry(uint32 homeMainlandId, const std::string &userName);

	/// Change player/guild name
	static void		changeUserName(uint32 homeMainlandId, const std::string& oldname, const std::string& newname);

	/// Set mail notification
	static void		setNotification(TMailNotification callback);

	/// Remove a user directory (mail)
	static void		removeUser(uint32 homeMainlandId, const std::string &userName);

	// Remove a guild directory (forum)
	static void		removeGuild(const std::string &guildName);

private:

	/// Constructor
	CMailForumValidator();

	/// Shard Id
	static uint32			_ShardId;

	/// Initialised yet?
	static bool				_Initialised;

	/// Mail notification
	static TMailNotification	_Notification;

	/// return the relative name from the full name
	static std::string		getShardRelativeName(const std::string &s);
	
public:

	/// received new mail!
	static void		cbMailNotification( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );


	/**
	 * Name translator
	 * Takes an utf-8 string and returns an ascii string, so names can be represented as filenames
	 */
	static std::string		nameToFile(const std::string& name);

	/**
	 * Name translator
	 * Takes an ascii string and returns an utf-8 string, so filenames can be decoded into names
	 */
	static std::string		fileToName(const std::string& name);
};


// Name translator
inline std::string	CMailForumValidator::nameToFile(const std::string& name)
{
	std::string	f;
	uint	p = 0;
	for (p=0; p<name.size(); ++p)
	{
		if (name[p] == ' ')
			f += '_';
		else if (name[p] == '%' || name[p] <= 32 || name[p] >= 127)
			f += NLMISC::toString("%%%02X", (uint8)name[p]);
		else
			f += name[p];
	}
	return f;
}

// Name translator
inline std::string	CMailForumValidator::fileToName(const std::string& file)
{
	std::string	n;
	uint	p = 0;
	for (p=0; p<file.size(); ++p)
	{
		if (file[p] == '%' && file[p+1] != '%')
		{
			char	b[3];
			b[0] = file[++p];
			b[1] = file[++p];
			b[2] = '\0';
			uint	c;
			sscanf(b, "%x", &c);
			n += (char)c;
		}
		else if (file[p] == '_')
		{
			n += ' ';
		}
		else
		{
			n += file[p];
		}
	}
	return n;
}


#endif // NL_MAIL_FORUM_VALIDATOR_H

/* End of mail_forum_validator.h */
