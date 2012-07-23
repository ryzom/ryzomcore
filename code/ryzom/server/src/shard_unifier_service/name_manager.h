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


#ifndef RYZOM_NAME_MANAGER_H
#define RYZOM_NAME_MANAGER_H

// Misc
#include "nel/misc/common.h"
#include "nel/misc/ucstring.h"

#include "game_share/character_sync_itf.h"

#include "server_share/mysql_wrapper.h"

namespace CHARSYNC
{
	class CCharacterSync;
}

/**
 * CNameManager
 * Manage unique name for character, forbidden names and reserved name
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2004
 *
 * Update 2005/2006 by Boris Boucher
 *	Added support for unifier guild names
 *
 */
class CNameManager : public NLMISC::ICommandsHandler
{
	friend class CHARSYNC::CCharacterSync;
public:

	typedef uint32			TCharId;
	typedef uint32			TGuildId;
	typedef uint32			TUserId;
	typedef std::string		TName;

	CNameManager()
		:	_Database(NULL)
	{}

	// assign a name to owner, return true if name is accepted
	bool assignName(uint32 charId, const ucstring & ucName, uint32 homeSessionId, bool skipTest = false);

	// liberate a name
	void liberateName(uint32 charId, const ucstring & ucName);

	// liberate any name associated to a character
	void liberateName(uint32 charId);

	// check character slot, remove duplicated entries if needed
//	void checkCharacterSlot(uint32 charId, const ucstring & ucName);

	// return true if name is free or usable
	CHARSYNC::TCharacterNameResult isNameUsable(const ucstring & ucName, uint32 userId, uint8 charIndex, uint32 homeSessionId);

	// return true if name is free or usable
	CHARSYNC::TCharacterNameResult isGuildNameUsable(const ucstring & ucName, uint32 guildId);

	// register a list of loaded guild, update the name table accordingly and fill an output
	// vector with renamed guild (because of name conflict).
	void registerLoadedGuildNames(uint32 shardId, const std::map<uint32, ucstring> &guilds, std::vector<uint32> &renamedGuildIds);

	// A new guild has been created, return true if the name is valid, false if
	// the guild has been renamed
	bool assignGuildName(uint32 shardId, uint32 guildId, const ucstring &guildName);

	// A guild as been deleted, release the name
	void releaseGuildName(uint32 shardId, uint32 guildId);

	// save names of characters
//	void saveCharacterNames();

	// save the guild names
	void saveGuildNames();

	// load names of characters, reserved and forbidden names
	void loadAllNames();

	// rename a character with a default name
	ucstring renameCharacter(uint32 charId, uint32 homeSessionId);

	// regular update (used to clean up the temporary reserved name)
	void update();


	// return the name associated to a guild
	const TName &getGuildName(uint32 guildId) const;

	/// Try to find a shard id from a name and session id. Return 0 if not found
	uint32 findCharId(const std::string &charName, uint32 homeSessionId);
	/// Try to find a shard id from a name without session id, return 0 if 0 or more than one match.
	uint32 findCharId(const std::string &charName);


private:
	bool loadAccountNamesFromTxt();
	bool loadAccountNamesFromDatabase();
//	bool loadAccountNamesFromXML();
//	bool loadAccountNamesFromEIDTranslator();
	bool loadCharacterNamesFromTxt();
	bool loadCharacterNamesFromDatabase();
//	bool loadCharacterNamesFromXML();
	bool loadGuildsNamesFromTxt();
	bool loadForbiddenNames();
	bool loadReservedNames(const char * fileNameWithoutPath);

	// generate a valid default name
	ucstring generateDefaultName(uint32 charId, uint32 homeSessionId);
	// generate a valid default guild name
	ucstring generateDefaultGuildName(uint32 guildId);


	/** This methods implemented by CCommandHandler is used by the 
	 *	command registry to retrieve the name of the object instance.
	 */
	virtual const std::string &getCommandHandlerName() const;

public:
	struct TCharSlot
	{
		uint32		UserId;
		uint8		CharIndex;

		TCharSlot()
		{
			UserId = 0xffff;
			CharIndex = 0xff;
		}

		TCharSlot(uint32 userId, uint8 charIndex)
		{
			UserId = userId;
			CharIndex = charIndex;
		}

		TCharSlot(uint32 charId)
		{
			UserId = charId >> 4;
			CharIndex = uint8( charId & 0xf );
		}

		uint32 getCharId() const
		{
			return (UserId << 4) + CharIndex;
		}

		bool operator == (const TCharSlot & charSlot) const
		{
			return (UserId == charSlot.UserId && CharIndex == charSlot.CharIndex);
		}

		bool operator != (const TCharSlot & charSlot) const
		{
			return (UserId != charSlot.UserId || CharIndex != charSlot.CharIndex);
		}

		bool operator < (const TCharSlot & charSlot) const
		{
			return ((UserId < charSlot.UserId) || (UserId == charSlot.UserId && CharIndex < charSlot.CharIndex));
		}

		void serial(NLMISC::IStream & f) throw(NLMISC::EStream)
		{
			f.serial( UserId );
			f.serial( CharIndex );
		}
	};

private:

	struct TGuildSlot
	{
		uint32		ShardId;
		uint32		GuildId;

		TGuildSlot()
			: ShardId(0), GuildId(0)
		{
		}

		TGuildSlot(uint32 shardId, uint32 guildId)
			: ShardId(shardId), GuildId(guildId)
		{
		}

		bool operator < (const TGuildSlot &other)
		{
			if (ShardId == other.ShardId)
				return GuildId < other.GuildId;

			return ShardId < other.ShardId;
		}
	
		void serial(NLMISC::IStream & f) throw(NLMISC::EStream)
		{
			f.serial( GuildId );
			f.serial( ShardId );
		}
	};

	struct TFullName
	{
		std::string	Name;
		TSessionId	HomeSessionId;

		TFullName()
			:	HomeSessionId(0)
		{}

		TFullName(const std::string &name, uint32 homeSessionId)
			:	Name(name),
				HomeSessionId(homeSessionId)
		{}

		bool operator < (const TFullName &other) const 
		{
			// use the old C style cmp that give in one operation the <, == and > relation
			int cmp = NLMISC::nlstricmp(Name, other.Name);
			if (cmp < 0)
				return true;
			else if (cmp > 0)
				return false;
			else
				return HomeSessionId < other.HomeSessionId;
		}
	};

	struct TTemporaryReservedNameInfo
	{
		uint32	UserId;
		uint32	ReserveDate;
		uint32	HomeSessionId;
	};

	enum 
	{ 
		// reserve names for 5 mn
		TEMPORARY_RESERVED_NAME_EXPIRATION = 60*5,
	};

//	typedef std::map<TName,TCharSlot>				TNames;
	typedef NLMISC::CTwinMap<TFullName,TCharSlot>	TNamesIndex;
	typedef std::map<TName,TGuildSlot>				TGuildNames;
	typedef std::map<TGuildId,TName>				TGuildIndex;
	typedef std::map<TUserId,TName>					TAccountNames;
	typedef std::map<TName,TUserId>					TReservedNames;
	typedef std::vector<TName>						TForbiddenNames;
	typedef std::map<TCharSlot,std::set<TName> >	TDuplicatedSlots;
	typedef std::map<TName, TTemporaryReservedNameInfo>	TTempReservedNames;

	TNamesIndex				_Names;
	TAccountNames			_AccountNames;
	TGuildNames				_GuildNames;
	TGuildIndex				_GuildIndex;
	TReservedNames			_ReservedNames;
	TForbiddenNames			_ForbiddenNames;
	TDuplicatedSlots		_DuplicatedSlots;
	/** Temporary reserved names. When a client create a character, it must first validate
	 *	the character name.
	 *	Then, later, the player send the character creation request.
	 *	The temporary reservation guarantee that a validated name will not be validated
	 *	or used by another client for the next 5 minutes, giving the time to the
	 *	first client to send the character creation request.
	 *	After the expiration period, the name is removed from the temporary reservation.
	 */
	TTempReservedNames		_TemporaryReservedNames;

	/// the list of changed character to broadcast to clients at next update (cleared each tick loop)
	std::set<TCharId>			_ChangedNames;
	/// The list of released names to broadcast at next update (cleared each tick loop)
	std::set<TCharId>			_ReleasedNames;

	/// the database connection to load data from database
	MSW::CConnection			*_Database;

	// commands handler table
	NLMISC_COMMAND_HANDLER_TABLE_BEGIN(CNameManager)
		NLMISC_COMMAND_HANDLER_ADD(CNameManager, dump, "display internal data about the name manager", "[all]")
		NLMISC_COMMAND_HANDLER_ADD(CNameManager, releaseGuildNamesForShard, "Release all the guild name that belong to the specified shard", "<shardId>")
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(dump);
	NLMISC_CLASS_COMMAND_DECL(releaseGuildNamesForShard);

};

#endif // RYZOM_NAME_MANAGER_H

/* name_manager.h */

