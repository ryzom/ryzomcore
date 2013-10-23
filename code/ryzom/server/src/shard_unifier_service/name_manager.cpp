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

#include "nel/net/service.h"
#include "nel/net/message.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/algo.h"
#include "nel/misc/sstring.h"
#include "nel/misc/random.h"

#include "game_share/backup_service_interface.h"
#include "game_share/utils.h"

#include "name_manager.h"
#include "database_mapping.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace MSW;
using namespace CHARSYNC;

CVariable<bool> NameManagerCheckOnStatup("name_mgr", "NameManagerCheckOnStatup", "If true name manager will do a complete check on character name and rename any invalid one", true, 0, true );

NLMISC::CRandom RandomGenerator;


const char *GUILD_NAME_FILE = "guild_names.txt";


//-----------------------------------------------------------------------------
bool CNameManager::assignName(uint32 charId, const ucstring & ucName, uint32 homeSessionId, bool skipTest)
{
//	if (eId == NLMISC::CEntityId::Unknown)
//		return false;


	const TCharSlot charSlot( charId );
	const uint32 playerId = charSlot.UserId;
	const uint8 charIndex = charSlot.CharIndex;

	// check that name is usable
	if (!skipTest && isNameUsable( ucName, playerId, charIndex, homeSessionId ) != TCharacterNameResult::cnr_ok)
		return false;

	// assign name and save
	const string name = toLower( ucName.toUtf8() );


	// remove any temporary reserve for this name
	_TemporaryReservedNames.erase(name);

	TFullName fullname(name, homeSessionId);

	const TCharSlot *owner = _Names.getB(fullname);
	if (owner != NULL && *owner == charSlot)
		// association already exist, nothing more to do
		return true;
	else if (owner != NULL)
	{
		// we need to remove this assoc
		_Names.removeWithA(fullname);
		_ReleasedNames.insert((owner->UserId<<4)+owner->CharIndex);
	}

	if (_Names.getBToAMap().find(charSlot) != _Names.getBToAMap().end())
	{
		// the character is associated to another name
		_Names.removeWithB(charSlot);
	}

	// insert the new association
	_Names.add(fullname, charSlot);
	_ChangedNames.insert(charId);

//	saveCharacterNames();

	nlinfo("NAMEMGR: assigned name '%s' to char %u", name.c_str(), charId );

	return true;
}

//-----------------------------------------------------------------------------
void CNameManager::liberateName(uint32 charId, const ucstring & ucName)
{
	const TCharSlot charSlot( charId );
	const string name = toLower( ucName.toUtf8() );

	// remove the name from the reservation if it belong to the specified user
	{
		TTempReservedNames::iterator it(_TemporaryReservedNames.find(name));
		if (it != _TemporaryReservedNames.end() && it->second.UserId == (charId >> 4))
		{
			_TemporaryReservedNames.erase(it);
		}
	}

	const TFullName *fullname = _Names.getA(charId);
	if (fullname == NULL || fullname->Name != ucName.toUtf8())
	{
		nlwarning("NAMEMGR: char %u is trying to liberate a name he does not own: '%s'", charId, name.c_str() );
		return;
	}

	_Names.removeWithB( charSlot );
	_ReleasedNames.insert(charId);
//	saveCharacterNames();
	nlinfo("NAMEMGR: char %u liberated his name '%s'", charId, name.c_str() );
}

//-----------------------------------------------------------------------------
// liberate any name associated to a character
void CNameManager::liberateName(uint32 charId)
{
	const TCharSlot charSlot( charId );

	TNamesIndex::TBToAMap::const_iterator it = _Names.getBToAMap().find( charSlot );
	if ( it == _Names.getBToAMap().end() )
		return;

	nlinfo("NAMEMGR: char %u liberated his name '%s'", charId, it->second.Name.c_str() );
	// ok, association found, remove it
	_Names.removeWithB(charSlot);
	_ReleasedNames.insert(charId);
	
//	saveCharacterNames();
}


//-----------------------------------------------------------------------------
//void CNameManager::checkCharacterSlot(uint32 charId, const ucstring & ucName)
//{
////	if (eId == CEntityId::Unknown)
////		return;
//
//	const TCharSlot charSlot( charId );
//	const TName name = toLower( ucName.toUtf8() );
//
//	// verify that name is assigned to the character
//	{
//		TNamesIndex::TAToBMap::const_iterator it = _Names.getAToBMap().find( name );
//		if ( it != _Names.getAToBMap().end() )
//		{
//			if( it->second != charSlot )
//			{
//				nlwarning("NAMEMGR: char %u has name '%s', but this name is assigned to another character", charId, name.c_str() );
//				renameCharacter(charId);
//			}
//		}
//		else
//		{
//			nlwarning("NAMEMGR: char %u has name '%s', but this name is not assigned", charId, name.c_str() );
//			assignName(charId, ucName);
//		}
//	}
//
//	// if this slot is not duplicated, just return
//	TDuplicatedSlots::iterator it = _DuplicatedSlots.find( charSlot );
//	if ( it == _DuplicatedSlots.end() )
//		return;
//
//	const std::set<TName> & dupNames = (*it).second;
//
//	// erase other names associated to the same slot
//	std::set<TName>::const_iterator itDupName;
//	for (itDupName = dupNames.begin(); itDupName != dupNames.end(); ++itDupName)
//	{
//		const TName & dupName = (*itDupName);
//		if (dupName != name)
//		{
//			const TCharSlot *cs = _Names.getB(dupName);
//			if (cs != NULL)
//			{
//				_ReleasedNames.insert((cs->UserId<<4)+cs->CharIndex);
//			}
//			_Names.removeWithA( dupName );
//		}
//	}
//
//	// save changes
//	saveCharacterNames();
//
//	// slot has been fixed
//	_DuplicatedSlots.erase( it );
//
//	nlinfo("NAMEMGR: fixed character slot: userId=%u, slot=%u, name='%s'", charSlot.UserId, charSlot.CharIndex, name.c_str() );
//
//	if ( !_DuplicatedSlots.empty() )
//		nlinfo("NAMEMGR: there still are %u duplicated character slots", _DuplicatedSlots.size() );
//}

//-----------------------------------------------------------------------------
ucstring CNameManager::renameCharacter(uint32 charId, uint32 homeSessionId)
{
	const ucstring ucDefaultName = generateDefaultName(charId, homeSessionId);
	nlverify(assignName(charId, ucDefaultName, homeSessionId, true));

	return ucDefaultName;
}

//-----------------------------------------------------------------------------
ucstring CNameManager::generateDefaultName(uint32 charId, uint32 homeSessionId)
{
	const TCharSlot charSlot(charId);

	string defaultName;
	string randomLetters(5, 'a');

	uint count = 0;
	while (1)
	{
		for (uint i = 0; i < randomLetters.size(); i++)
		{
			randomLetters[i] = 'a' + (char)RandomGenerator.rand(uint16('z'-'a'));
		}
		defaultName = "Default" + randomLetters;

		if ( isNameUsable(defaultName, charSlot.UserId, charSlot.CharIndex, homeSessionId) == TCharacterNameResult::cnr_ok)
			break;

		// anti freeze
		if (++count > 100)
			break;
	}

	return defaultName;
}

//-----------------------------------------------------------------------------
ucstring CNameManager::generateDefaultGuildName(uint32 guildId)
{
	string defaultName;
	string randomLetters(5, 'a');

	uint count = 0;
	while (1)
	{
		for (uint i = 0; i < randomLetters.size(); i++)
		{
			randomLetters[i] = 'a' + (char)RandomGenerator.rand(uint16('z'-'a'));
		}
		defaultName = "DefaultGuild" + randomLetters;

		if ( isGuildNameUsable(defaultName, guildId) == TCharacterNameResult::cnr_ok)
			break;

		// anti freeze
		if (++count > 100)
			break;
	}

	return defaultName;
}

//-----------------------------------------------------------------------------
/** This methods implemented by CCommandHandler is used by the 
 *	command registry to retrieve the name of the object instance.
 */
const std::string &CNameManager::getCommandHandlerName() const
{
	const static string name = "nameManager";
	return name;
}

//-----------------------------------------------------------------------------
CHARSYNC::TCharacterNameResult CNameManager::isNameUsable(const ucstring & ucNameIn, uint32 userId, uint8 charIndex, uint32 homeSessionId)
{
	// WARNING: if you change validity checks here,
	// please also change them in CEntityIdTranslator::isValidEntityName() (nel/misc/eid_translator.cpp)

	// if the name contains a shard specification, remove it first
	ucstring ucName;
	ucstring::size_type pos = ucNameIn.find('(');
	if (pos != ucstring::npos)
	{
		//only keep the simple name for the test
		ucName = ucNameIn.substr(0, pos);
	}
	else
	{
		// test the whole input name
		ucName = ucNameIn;
	}
		

	// perform a first validity check on the name length
	if (ucName.size() < 3)
	{
		nldebug("VALID_NAME::CNameManager::isNameUsable name '%s' rejected because is too short", ucName.toString().c_str());
		return TCharacterNameResult::cnr_invalid_name;
	}

	// perform a first validity check on the name length
	if (ucName.size() > 15)
	{
		nldebug("VALID_NAME::CNameManager::isNameUsable name '%s' rejected because is too long", ucName.toString().c_str());
		return TCharacterNameResult::cnr_invalid_name;
	}

	// make sure the name is only composed of valid characters
	for (uint32 i = 0; i < ucName.size(); ++i)
	{
		if ( (ucName[i] < (uint16)'A' || ucName[i] > (uint16)'Z') && (ucName[i] < (uint16)'a' || ucName[i] > (uint16)'z') )
		{
			nldebug("VALID_NAME::CNameManager::isNameUsable name '%s' rejected because it contains invalid character", ucName.toString().c_str());
			return TCharacterNameResult::cnr_invalid_name;
		}
	}

	// it's now safe to convert the name to 8 bit lower case
	const string name = toLower( ucName.toUtf8() );

	// make sure the name isn't forbidden
	for (uint32 i = 0; i < _ForbiddenNames.size(); ++i)
	{
		if ( NLMISC::testWildCard( name, _ForbiddenNames[i] ) )
		{
			nldebug("VALID_NAME::CNameManager::isNameUsable name '%s' rejected because it contains the forbidden string '%s'", 
				ucName.toString().c_str(),
				_ForbiddenNames[i].c_str());
			return TCharacterNameResult::cnr_invalid_name;
		}
	}

	// make sure the name isn't reserved
	TReservedNames::iterator rit = _ReservedNames.find( name );
	if ( rit != _ReservedNames.end() )
	{
		if ( (*rit).second != userId )
		{
			nldebug("VALID_NAME::CNameManager::isNameUsable name '%s' rejected because it's reserved", ucName.toString().c_str());
			return TCharacterNameResult::cnr_already_exist;
		}
	}
	
	// make sure the name isn't temporary reserved
	{
		TTempReservedNames::iterator rit = _TemporaryReservedNames.find( name );
		if ( rit != _TemporaryReservedNames.end() )
		{
			if ( rit->second.UserId != userId && rit->second.UserId == homeSessionId)
			{
				nldebug("VALID_NAME::CNameManager::isNameUsable name '%s' rejected because it's temporary reserved", ucName.toString().c_str());
				return TCharacterNameResult::cnr_already_exist;
			}
		}
	}
	
	TFullName fullname(name, homeSessionId);
	// make sure the name isn't used by another character
	TNamesIndex::TAToBMap::const_iterator it = _Names.getAToBMap().find( fullname );
	if ( it != _Names.getAToBMap().end())
	{
		if( it->second.UserId != userId || it->second.CharIndex != charIndex )
		{
			nldebug("VALID_NAME::CNameManager::isNameUsable name '%s' rejected because it's already used", ucName.toString().c_str());
			return TCharacterNameResult::cnr_already_exist;
		}
	}

	// make sure the name is not used by a guild
	{
		TGuildNames::iterator it = _GuildNames.find(name);
		if (it != _GuildNames.end())
		{
			nldebug("VALID_NAME::CNameManager::isNameUsable name '%s' rejected because it's a guild name", ucName.toString().c_str());
			return TCharacterNameResult::cnr_already_exist;
		}
	}

	// the name is valid, insert it in the temporary reserve

	TTemporaryReservedNameInfo trni;
	trni.UserId = userId;
	trni.ReserveDate = CTime::getSecondsSince1970();
	trni.HomeSessionId = homeSessionId;
	_TemporaryReservedNames.insert(make_pair(name, trni));

	nldebug("VALID_NAME::CNameManager::isNameUsable name '%s' accepted", ucName.toString().c_str());

	return TCharacterNameResult::cnr_ok;
}

//-----------------------------------------------------------------------------
TCharacterNameResult CNameManager::isGuildNameUsable(const ucstring & ucName, uint32 guildId)
{
	// perform a first validity check on the name length
	if (ucName.size() < 3)
		return TCharacterNameResult::cnr_invalid_name;

	// perform a first validity check on the name length
	if (ucName.size() > 50)
		return TCharacterNameResult::cnr_invalid_name;

	// make sure the name is only composed of valid characters
	bool prevBlank = false;
	for (uint i = 0; i < ucName.size(); i++)
	{
		if ( ucName[i] == ucchar(' ') )
		{
			if ( prevBlank )
			{
				return TCharacterNameResult::cnr_invalid_name;
			}	
			prevBlank = true;
		}
		else
		{
			prevBlank = false;
			if (!isalpha (ucName[i]))
			{
				return TCharacterNameResult::cnr_invalid_name;
			}
		}
	}


	// it's now safe to convert the name to 8 bit lower case
	const string name = toLower( ucName.toUtf8() );

	// make sure the name isn't forbidden
	for (uint32 i = 0; i < _ForbiddenNames.size(); ++i)
	{
		if ( NLMISC::testWildCard( name, _ForbiddenNames[i] ) )
			return TCharacterNameResult::cnr_invalid_name;
	}

	// make sure the name isn't reserved
	TReservedNames::iterator rit = _ReservedNames.find( name );
	if ( rit != _ReservedNames.end() )
	{
		return TCharacterNameResult::cnr_already_exist;
	}
	
	// make sure the name isn't temporary reserved
	{
		TTempReservedNames::iterator rit = _TemporaryReservedNames.find( name );
		if ( rit != _TemporaryReservedNames.end() )
		{
			return TCharacterNameResult::cnr_already_exist;
		}
	}
	
	// make sure the name isn't used by a character
	TNamesIndex::TAToBMap::const_iterator it = _Names.getAToBMap().lower_bound( TFullName(name, 0) );
	if ( it != _Names.getAToBMap().end()  && it->first.Name == name)
	{
		return TCharacterNameResult::cnr_already_exist;
	}

	// make sure the name is not used by another guild
	{
		TGuildNames::iterator it = _GuildNames.find(name);
		if (it != _GuildNames.end() && it->second.GuildId != guildId)
			return TCharacterNameResult::cnr_already_exist;
	}

	// the name is valid
	return TCharacterNameResult::cnr_ok;
}


//-----------------------------------------------------------------------------
void CNameManager::registerLoadedGuildNames(uint32 shardId, const std::map<uint32, ucstring> &guilds, vector<uint32> &renamedGuildIds)
{
	bool saveFile = false;

	std::map<uint32, ucstring>::const_iterator first(guilds.begin()), last(guilds.end());

	for (; first != last; ++first)
	{
		uint32 guildId = first->first;
		const ucstring &guildName = first->second;

		// check the name
		if (isGuildNameUsable(guildName, guildId) == TCharacterNameResult::cnr_ok)
		{
			// convert the name into the standard utf8 low case version
			string name = toLower(guildName.toUtf8());
			// ok, the name is correct, check if it already exist
			TGuildNames::iterator it(_GuildNames.find(name));
			if (it == _GuildNames.end())
			{
				// this is a new name
				_GuildNames.insert(make_pair(name, TGuildSlot(shardId, guildId)));
				_GuildIndex.insert(make_pair(guildId, name));
				saveFile = true;
			}
			else
			{
				BOMB_IF(it->second.GuildId != guildId, "Guild "<<guildId<<" is just loaded by shard "<<shardId<<" but name already in use by guild "<<it->second.GuildId, continue);
				BOMB_IF(it->second.ShardId != shardId, "Guild "<<guildId<<" is just loaded by shard "<<shardId<<" but already registered by shard "<<it->second.ShardId, continue);
			}
		}
		else
		{
			// we need to rename the guild
			ucstring newName = generateDefaultGuildName(guildId);

			nlinfo("NM:registerLoadedGuildNames : Guild %u has a conflicting name '%s', renamed to '%s'", 
				guildId, 
				guildName.toUtf8().c_str(),
				newName.toUtf8().c_str());
			
			// save it in the container
			std::string name = toLower(newName.toUtf8());
			_GuildNames.insert(make_pair(name, TGuildSlot(shardId, guildId)));
			_GuildIndex.insert(make_pair(guildId, name));

			saveFile = true;

			// put it in the renamed guild list return vector
			renamedGuildIds.push_back(guildId);
		}
	}

	if (saveFile)
	{
		saveGuildNames();
	}
}

//-----------------------------------------------------------------------------
bool CNameManager::assignGuildName(uint32 shardId, uint32 guildId, const ucstring &guildName)
{
	bool ret;
	if (isGuildNameUsable(guildName, guildId) != TCharacterNameResult::cnr_ok)
	{
		// oups, bad name, we need to generate a new valid name
		ucstring newName = generateDefaultGuildName(guildId);

		string name = toLower(newName.toUtf8());
		_GuildNames.insert(make_pair(name, TGuildSlot(shardId, guildId)));
		_GuildIndex.insert(make_pair(guildId, name));

		ret = false;
	}
	else
	{
		// ok, the name is correct, do a simple insertion
		string name = toLower(guildName.toUtf8());
		_GuildNames.insert(make_pair(name, TGuildSlot(shardId, guildId)));
		_GuildIndex.insert(make_pair(guildId, name));

		ret = true;
	}

	// save the file
	saveGuildNames();

	return ret;
}

//-----------------------------------------------------------------------------
void CNameManager::releaseGuildName(uint32 shardId, uint32 guildId)
{
	TGuildIndex::iterator it(_GuildIndex.find(guildId));
	if (it == _GuildIndex.end())
	{
		nlwarning("CNameManager::releaseGuildName : no guild %s registered in name manager", guildId);
		return;
	}

	// erase the name entry
	_GuildNames.erase(it->second);
	// erase the index entry
	_GuildIndex.erase(it);

	// save the file
	saveGuildNames();
}


//-----------------------------------------------------------------------------
//void CNameManager::saveCharacterNames()
//{
//	// save the character names
//	{
//		string fileName = "character_names.txt";
//		
//		CSString s;
//		nlinfo("NAMEMGR::save: building file content: %s",fileName.c_str());
//		for (TNamesIndex::TAToBMap::const_iterator it=_Names.getAToBMap().begin(); it!=_Names.getAToBMap().end(); ++it)
//		{
//			s << it->first.Name << " " << it->second.UserId << " " << it->second.CharIndex << "  " << it->first.HomeSessionId << "\n";
//		}
//		
//		if( s.empty() )
//			return;
//
//		nlinfo("NAMEMGR::save: send message to BS");
//		
//		CBackupMsgSaveFile msg( fileName, CBackupMsgSaveFile::SaveFile, BsiGlobal );
//		msg.DataMsg.serialBuffer((uint8*)s.c_str(), s.size());
//		BsiGlobal.sendFile(msg);
//	}
//
//	// save account names
//	{
//		string fileName = "account_names.txt";
//		
//		string s;
//		nlinfo("NAMEMGR::save: building file content: %s",fileName.c_str());
//		for (TAccountNames::iterator it=_AccountNames.begin();it!=_AccountNames.end();++it)
//		{
//			s+=NLMISC::toString("%s %i\n",(*it).second.c_str(),(*it).first);
//		}
//		
//		if( s.empty() )
//			return;
//
//		nlinfo("NAMEMGR::save: send message to BS");
//		
//		CBackupMsgSaveFile msg( fileName, CBackupMsgSaveFile::SaveFile, BsiGlobal );
//		msg.DataMsg.serialBuffer((uint8*)s.c_str(), s.size());
//		BsiGlobal.sendFile(msg);
//	}
//}

//-----------------------------------------------------------------------------
void CNameManager::saveGuildNames()
{
	// save the guild names
	{
		string fileName = GUILD_NAME_FILE;
		
		string s;
		nlinfo("NAMEMGR::save: building file content: %s",fileName.c_str());
		for (TGuildNames::const_iterator it=_GuildNames.begin(); it!=_GuildNames.end(); ++it)
		{
			s+=NLMISC::toString("%s %i %i\n", it->first.c_str(), it->second.ShardId, it->second.GuildId);
		}
		
		if( s.empty() )
			return;

		nlinfo("NAMEMGR::save: send message to BS");
		
		CBackupMsgSaveFile msg( fileName, CBackupMsgSaveFile::SaveFile, Bsi );
		msg.DataMsg.serialBuffer((uint8*)s.c_str(), (uint)s.size());
		Bsi.sendFile(msg);
	}

}
//-----------------------------------------------------------------------------
void CNameManager::loadAllNames()
{
	bool result;

	// load account names
	result=loadAccountNamesFromDatabase();
	if (result==false)
		result=loadAccountNamesFromTxt();
	if (result==false)
		nlinfo("NAMEMGR::load: Failed to load account names from txt file");

	// load character names
	result=loadCharacterNamesFromDatabase();
	if (result==false)
		result=loadCharacterNamesFromTxt();
//	if (result==false)
//		result=loadCharacterNamesFromXML();
	if (result==false)
		nlinfo("NAMEMGR::load: Failed to load character names from txt file");

	// load character names
	result=loadGuildsNamesFromTxt();
	if (result==false)
		nlinfo("NAMEMGR::load: Failed to load guilds names from txt file");

	// load forbidden names
	result=loadForbiddenNames();
	if (result==false)
		nlwarning("NAMEMGR::load: Failed to load forbidden name file");

	// load reserved names
	result=loadReservedNames("reserved_names.xml");
	if (result==false)
		nlwarning("NAMEMGR::load: Failed to load reserved player name file");

	// load dev and gm names
	result=loadReservedNames("dev_gm_names.xml");
	if (result==false)
		nlwarning("NAMEMGR::load: Failed to load reserved dev and gm name file");

	nlinfo("NameManager : checking %u loaded names...", _Names.getAToBMap().size());

	// revalidate all character names
	TNamesIndex::TAToBMap::const_iterator first(_Names.getAToBMap().begin()), last(_Names.getAToBMap().end());
	for (uint i=0; first != last; ++i)
	{
		TNamesIndex::TAToBMap::const_iterator next(first);
		++next;

		const TFullName &fullName = first->first;
		const TCharSlot &charSlot = first->second;
		if (isNameUsable(fullName.Name, charSlot.UserId, charSlot.CharIndex, fullName.HomeSessionId) != CHARSYNC::TCharacterNameResult::cnr_ok)
		{
			nlinfo("NameManager : renaming invalid character name '%s' for char %u", fullName.Name.c_str(), charSlot.getCharId());

			// get the shard id
			uint32 charId = charSlot.getCharId();
			// the name is invalid !
			ucstring newName = renameCharacter(charSlot.getCharId(), fullName.HomeSessionId);
			// do not access the fullName and charSlot var from now

			RSMGR::CCharacterPtr character = RSMGR::CCharacter::load(*_Database, charId, __FILE__, __LINE__);
			if (character != NULL)
			{
				character->setCharName(newName.toUtf8());
				character->update(*_Database);
			}
		}

		first = next;

		if (i%1000 == 0)
		{
			nldebug("NameManager : checking name advance %u/%u", i,  _Names.getAToBMap().size());
		}

	}
}

//-----------------------------------------------------------------------------
bool CNameManager::loadAccountNamesFromTxt()
{
	FILE* f;
	string fileName;

	// open the file
	fileName = BsiGlobal.getLocalPath() + "account_names.txt";
	f=fopen(fileName.c_str(),"rb");
	if (f == NULL)
	{
		nlwarning("Failed to open file for reading: %s", fileName.c_str() );
		return false;
	}

	CSString input;

	// read the file content into a buffer
	uint32 size=NLMISC::CFile::getFileSize(f);
	input.resize(size);
	uint32 readSize= (uint32)fread(&input[0],1,size,f);
	fclose(f);
	BOMB_IF(readSize!=size,"Failed to read file content for file: "+fileName,return false);

	// scan the content
	while (!input.empty())
	{
		CSString line=input.firstLine(true);
		if (line.strip().empty())
			continue;
		DROP_IF (line.countWords()!=2,"Invalid line found in account names file: "+line,continue);
		DROP_IF (line.word(1).atoi()==0,"Invalid user id in account names file line: "+line,continue);
		_AccountNames[line.word(1).atoi()]=line.word(0);
	}

	return true;
}

//-----------------------------------------------------------------------------
bool CNameManager::loadAccountNamesFromDatabase()
{
	if (_Database == NULL)
		return false;

	// request the database to extract the complete list of account names
	CSString query;
	query << "SELECT user_id, user_name FROM ring_users";
	BOMB_IF(!_Database->query(query), "Failed to load account names from the database", return false);

	auto_ptr<CUseResult> result = _Database->useResult();

	while (result->fetchRow())
	{
		uint32 userId;
		string accountName;
		result->getField(0, userId);
		result->getField(1, accountName);

		_AccountNames[userId]=accountName;
	}

	return true;
}

//-----------------------------------------------------------------------------
//bool CNameManager::loadAccountNamesFromXML()
//{
//	CIFile f;
//	string fileName;
//
//	fileName = BsiGlobal.getLocalPath() + "account_names.xml";
//	if( !f.open( fileName ) )
//		return false;
//
//	nlinfo("NAMEMGR: Loading file: %s",fileName.c_str());
//
//	CIXml input;
//	if(!input.init( f ))
//	{
//		nlwarning("NAMEMGR:loadAccountNamesFromXML Can't init xml input for file %s", fileName.c_str());
//		return false;
//	}
//	input.serialCont( _AccountNames );
//	f.close();
//
//	return true;
//}

//-----------------------------------------------------------------------------
//bool CNameManager::loadAccountNamesFromEIDTranslator()
//{
//	nlinfo("NAMEMGR: Build account names from EIDTranslator");
//
//	try
//	{
//		CEntityIdTranslator::getInstance()->load(BsiGlobal.getLocalPath() + "eid_translation.data", IService::getInstance()->ConfigFile.getVar("InvalidEntityNamesFilename").asString());
//	}
//	catch(const Exception &)
//	{
//		// if we can't load the file, we force a check coherency
//		nlwarning("Can't load the eid_translation.data");
//		return false;
//	}
//
//	for( map<CEntityId, CEntityIdTranslator::CEntity>::const_iterator it = CEntityIdTranslator::getInstance()->getRegisteredEntities().begin(); it != CEntityIdTranslator::getInstance()->getRegisteredEntities().end(); ++it )
//	{
//		_AccountNames[(*it).second.UId]= (*it).second.UserName;
//	}
//	saveCharacterNames();
//
//	return true;
//}

//-----------------------------------------------------------------------------
bool CNameManager::loadCharacterNamesFromTxt()
{
	FILE* f;
	string fileName;

	// open the file
	fileName = BsiGlobal.getLocalPath() + "character_names.txt";
	f=fopen(fileName.c_str(),"rb");
	if (f == NULL)
	{
		nlwarning("Failed to open file for reading: %s", fileName.c_str() );
		return false;
	}

	CSString input;

	// read the file content into a buffer
	uint32 size=NLMISC::CFile::getFileSize(f);
	input.resize(size);
	uint32 readSize= (uint32)fread(&input[0],1,size,f);
	fclose(f);
	BOMB_IF(readSize!=size,"Failed to read file content for file: "+fileName,return false);

	_Names.clear();
	_DuplicatedSlots.clear();
	typedef map<TCharSlot, TName> TCharSlotToName;
	TCharSlotToName charSlotToName;

	vector<string> lines;
	NLMISC::explode(string(input), string("\n"), lines, true);

	// scan the content
//	while (!input.empty())

//	bool mustSaveFile = false;
	for (uint i=0; i<lines.size(); ++i)
	{
		if (i > 0 && (i%100 == 0))
			nldebug("Loading character names : %u/%u (%.2f%%)", i, lines.size(), float(i)/float(lines.size())*100.0f);
//		CSString line=input.firstLine(true);
		CSString line = lines[i];
		vector<string> words;
		NLMISC::explode(string(line), string(" "), words, true);

		if (words.empty())
			continue;
//		if (line.strip().empty())
//			continue;
//		BOMB_IF (line.countWords()!=3,"Invalid line found in character names file: "+line,continue);
		BOMB_IF (words.size()!=3 && words.size()!=4,"Invalid line found in character names file: "+line,continue);

		sint i1, i2;
		NLMISC::fromString(words[1], i1);
		NLMISC::fromString(words[2], i2);

		BOMB_IF (i1==0,"Invalid user id in character names file line: "+line,continue);
		BOMB_IF (i2>15 || (i2==0 && words[2] != "0"),"Invalid slot id in character names file line: "+line,continue);

		sint sessionId =0;
		if (words.size() > 3)
			NLMISC::fromString(words[3], sessionId);

		const TName name = words[0];
		const TCharSlot charSlot = TCharSlot(i1, i2);
		TFullName fullname(name, sessionId);

		// Check the name is usable, otherwise skip (to resolve corrupted names)
		if ( isNameUsable( ucstring(name), charSlot.UserId, charSlot.CharIndex, sessionId ) != TCharacterNameResult::cnr_ok)
		{
			nlwarning( "Invalid character name '%s' for user %u char slot %u in %s, will be reset to default", name.c_str(), charSlot.UserId, (uint)charSlot.CharIndex, fileName.c_str() );
//			mustSaveFile = true;
			continue;
		}
		
		if (_Names.getAToBMap().find(fullname) != _Names.getAToBMap().end())
			_Names.removeWithA(fullname);
		if (_Names.getBToAMap().find(charSlot) != _Names.getBToAMap().end())
			_Names.removeWithB(charSlot);

		_Names.add(fullname, charSlot);
//		TNames::const_iterator it = _Names.find( name );
//		nlassert(it != _Names.end());

		// check duplicated character slots
		TCharSlotToName::iterator itSlot = charSlotToName.find( charSlot );
		if ( itSlot != charSlotToName.end() )
		{
			_DuplicatedSlots[charSlot].insert( (*itSlot).second );
			_DuplicatedSlots[charSlot].insert( name );
		}
		else
		{
			charSlotToName[charSlot] = name;
		}

//		// update the EID translator
//		uint64 lid = ( (*it).second.UserId<<4 ) | (*it).second.CharId;
//		CEntityId id( RYZOMID::player, lid );
//		id.setDynamicId( 0 );
//		id.setCreatorId( 0 );
//
//		if (CEntityIdTranslator::getInstance()->isEntityRegistered(id))
//			CEntityIdTranslator::getInstance()->unregisterEntity( id );
//		CEntityIdTranslator::getInstance()->registerEntity( id, capitalize( (*it).first ), (*it).second.CharId, (*it).second.UserId, _AccountNames[(*it).second.UserId] );
	}

	if ( !_DuplicatedSlots.empty() )
	{
		nlwarning("NAMEMGR: file '%s' contains %u duplicated character slots", fileName.c_str(), _DuplicatedSlots.size() );
	}

//	if ( mustSaveFile )
//	{
//		saveCharacterNames();
//	}

	return true;
}


bool CNameManager::loadCharacterNamesFromDatabase()
{
	if (_Database == NULL)
		return false;

	_Names.clear();

	// request the database to extract the complete list of character names
	CSString query;
	query << "SELECT char_id, char_name, home_mainland_session_id FROM characters";
	BOMB_IF(!_Database->query(query), "Failed to load character names from the database", return false);

	auto_ptr<CUseResult> result = _Database->useResult();

	while (result->fetchRow())
	{
		uint32 charId;
		string charName;
		uint32 sessionNum;
		result->getField(0, charId);
		result->getField(1, charName);
		result->getField(2, sessionNum);

//		uint32 userId = charId >> 4;
//		uint8 charSlot = uint8(charId & 0xf);

		_Names.add(TFullName(charName, sessionNum), TCharSlot(charId));
	}

	return true;

}

//-----------------------------------------------------------------------------
//bool CNameManager::loadCharacterNamesFromXML()
//{
//	CIFile f;
//	string fileName;
//
//	// load the character names and insert them into the eid translator
//	fileName = BsiGlobal.getLocalPath() + "character_names.xml";
//	if( !f.open( fileName ) )
//		return false;
//
//	nlinfo("NAMEMGR: Loading file: %s",fileName.c_str());
//	CIXml input;
//	if(!input.init( f ))
//	{
//		nlwarning("<NAMEMGR::loadCharacterNamesFromXML>Can't init xml input for file %s", fileName.c_str());
//		return false;
//	}
//	TNames namesToCheckAndAdd;
//	input.serialCont( namesToCheckAndAdd );
//	f.close();
//
//	_DuplicatedSlots.clear();
//	map<TCharSlot,TName> charSlotToName;
//
//	// update the EID translator
//	bool mustSaveFile = false;
//	for( TNames::iterator it = namesToCheckAndAdd.begin(); it != namesToCheckAndAdd.end(); ++it )
//	{
//		const TName & name = (*it).first;
//		const TCharSlot charSlot = TCharSlot( (*it).second.UserId, (*it).second.CharIndex );
//
//		// Check the name is usable, otherwise skip (to resolve corrupted names)
//		if ( isNameUsable( ucstring(name), charSlot.UserId, charSlot.CharIndex ) != TCharacterNameResult::cnr_ok)
//		{
//			nlwarning( "Invalid character name '%s' for user %u char slot %u in %s, will be reset to default", name.c_str(), charSlot.UserId, (uint)charSlot.CharIndex, fileName.c_str() );
//			mustSaveFile = true;
//			continue;
//		}
//		
//		if (_Names.getAToBMap().find(it->first) != _Names.getAToBMap().end())
//			_Names.removeWithA(it->first);
//		if (_Names.getBToAMap().find(it->second) != _Names.getBToAMap().end())
//			_Names.removeWithB(it->second);
//
//		_Names.add( it->first, it->second );
//
//		// check duplicated character slots
//		map<TCharSlot,TName>::iterator itSlot = charSlotToName.find( charSlot );
//		if ( itSlot != charSlotToName.end() )
//		{
//			_DuplicatedSlots[charSlot].insert( (*itSlot).second );
//			_DuplicatedSlots[charSlot].insert( name );
//		}
//		else
//		{
//			charSlotToName[charSlot] = name;
//		}
//
////		uint64 lid = ( (*it).second.UserId<<4 ) | (*it).second.CharId;
////		CEntityId id( RYZOMID::player, lid );
////		id.setDynamicId( 0 );
////		id.setCreatorId( 0 );
////
////		if (CEntityIdTranslator::getInstance()->isEntityRegistered(id))
////			CEntityIdTranslator::getInstance()->unregisterEntity( id );
////		CEntityIdTranslator::getInstance()->registerEntity( id, capitalize( name ), (*it).second.CharId, (*it).second.UserId, _AccountNames[(*it).second.UserId] );
//	}
//
//	if ( !_DuplicatedSlots.empty() )
//	{
//		nlwarning("NAMEMGR: file '%s' contains %u duplicated character slots", fileName.c_str(), _DuplicatedSlots.size() );
//	}
//
//	if ( mustSaveFile )
//	{
//		saveCharacterNames();
//	}
//
//	return true;
//}

//-----------------------------------------------------------------------------
bool CNameManager::loadGuildsNamesFromTxt()
{
	FILE* f;
	string fileName;

	// open the file
	fileName = Bsi.getLocalPath() + GUILD_NAME_FILE;
	f=fopen(fileName.c_str(),"rb");
	if (f == NULL)
	{
		nlinfo("Failed to open file for reading: %s", fileName.c_str() );
		return false;
	}

	CSString input;

	// read the file content into a buffer
	uint32 size=NLMISC::CFile::getFileSize(f);
	input.resize(size);
	uint32 readSize= (uint32)fread(&input[0],1,size,f);
	fclose(f);
	BOMB_IF(readSize!=size,"Failed to read file content for file: "+fileName,return false);

	_GuildNames.clear();
	_GuildIndex.clear();
	typedef map<TGuildSlot, TName> TGuildSlotToName;
	TGuildSlotToName guildSlotToName;

	vector<string> lines;
	NLMISC::explode(string(input), string("\n"), lines, true);

	// scan the content
//	while (!input.empty())

//	bool mustSaveFile = false;
	for (uint i=0; i<lines.size(); ++i)
	{
		if (i > 0 && (i%100 == 0))
			nldebug("Loading guild names : %u/%u (%.2f%%)", i+1, lines.size(), float(i+1)/float(lines.size())*100.0f);
		CSString line = lines[i];
		vector<string> words;
		NLMISC::explode(string(line), string(" "), words, true);

		if (words.empty())
			continue;
		// merge the first words until we have only 3 words
		while (words.size() > 3)
		{
			words[0] += words[1];
			words.erase(words.begin()+1);
		}
		BOMB_IF (words.size()!=3,"Invalid line "<<i+1<<" found in guild names file : '"<<line<<"'", continue);

		sint i1, i2;
		NLMISC::fromString(words[1], i1);
		NLMISC::fromString(words[2], i2);

		BOMB_IF (i1==0, "Invalid shardId in guild names file line "<<i+1<<" : '"<<line<<"'", continue);
		BOMB_IF (i2==0, "Invalid guildId in guild names file line "<<i+1<<" : '"<<line<<"'", continue);
		TName name = words[0];
		const TGuildSlot guildSlot = TGuildSlot(i1, i2);

		// Check the name is usable, i.e valid regarding guild name rules AND not already used
		if ( isGuildNameUsable( ucstring(name), guildSlot.GuildId) != TCharacterNameResult::cnr_ok)
		{
			nlwarning( "Invalid guild name '%s' for guild %u on from shard %u will be reset to default", name.c_str(), guildSlot.GuildId, guildSlot.ShardId);
		
			name = toLower(generateDefaultGuildName(guildSlot.GuildId).toUtf8());
		}
		
		_GuildNames.insert(make_pair(name, guildSlot));
		_GuildIndex.insert(make_pair(guildSlot.GuildId, name));
	}

//	if (mustSaveFile)
//	{
//		saveGuildNames();
//	}
	return true;

}

//-----------------------------------------------------------------------------
bool CNameManager::loadForbiddenNames()
{
	string fileName;

	// read the invalid entity name list
	fileName = IService::getInstance()->WriteFilesDirectory.toString() + "invalid_entity_names.txt";
	FILE *fp = fopen (fileName.c_str(), "r");
	if (fp == NULL)
		return false;

	nlinfo("NAMEMGR: Loading file: %s",fileName.c_str());
	while (true)
	{
		char str[512];
		char *fgres = fgets(str, 511, fp);
		if(feof(fp))
			break;
		if (fgres == NULL)
		{
			nlwarning("NAMEMGR: Error reading file");
			break;
		}
		if (strlen(str) > 0)
		{
			str[strlen(str)-1] = '\0';
			toLower( str );
			_ForbiddenNames.push_back(string());
			_ForbiddenNames.back() = str;
		}
	}
	fclose (fp);

	return true;
}

//-----------------------------------------------------------------------------
bool CNameManager::loadReservedNames(const char* fileNameWithoutPath)
{
	CIFile f;
	string fileName;

	// read the reserved name list
	fileName = IService::getInstance()->WriteFilesDirectory.toString() + fileNameWithoutPath;
	if( !f.open( fileName ) )
		return false;

	nlinfo("NAMEMGR: Loading file: %s",fileName.c_str());
	CIXml input;
	if(!input.init( f ))
	{
		nlwarning("<NAMEMGR::loadReservedNames>Can't init xml input for file %s", fileName.c_str());
		return false;
	}
	TReservedNames reservedName;
	input.serialCont( reservedName );
	f.close();

	for( TReservedNames::iterator it = reservedName.begin(); it != reservedName.end(); ++it )
	{
		_ReservedNames[ toLower( (*it).first ) ] = (*it).second;
	}

	return true;
}


void CNameManager::update()
{
	TTempReservedNames::iterator first(_TemporaryReservedNames.begin()), last(_TemporaryReservedNames.end());

	uint32 now = CTime::getSecondsSince1970();

	for (; first != last; ++first)
	{
		if (first->second.ReserveDate+TEMPORARY_RESERVED_NAME_EXPIRATION > now)
		{
			// this reservation has expired, release it
			_TemporaryReservedNames.erase(first);
			// next release at next update
			break;
		}
	}
}


const CNameManager::TName &CNameManager::getGuildName(uint32 guildId) const
{
	TGuildIndex::const_iterator it(_GuildIndex.find(guildId));

	if (it == _GuildIndex.end())
	{
		static string emptyString;
		return emptyString;
	}
	else
		return it->second;
}


uint32 CNameManager::findCharId(const std::string &charName, uint32 homeSessionId)
{
	TFullName fullname(charName, homeSessionId);

	TNamesIndex::TAToBMap::const_iterator it(_Names.getAToBMap().find(fullname));

	if (it != _Names.getAToBMap().end())
	{
		return it->second.getCharId();
	}

	// not found
	return 0;
}

/// Try to find a shard id from a name without session id, return 0 if 0 or more than one match.
uint32 CNameManager::findCharId(const std::string &charName)
{
	TFullName fullname(charName, 0);

	TNamesIndex::TAToBMap::const_iterator it(_Names.getAToBMap().upper_bound(fullname));
	TNamesIndex::TAToBMap::const_iterator found(_Names.getAToBMap().end());

	while (it->first.Name == charName)
	{
		if (found != _Names.getAToBMap().end())
		{
			// more than one match
			return 0;
		}
		// first found
		found = it;
	}

	if (it != _Names.getAToBMap().end())
	{
		// we found one
		return it->second.getCharId();
	}

	// not found 0
	return 0;

}



NLMISC_CLASS_COMMAND_IMPL(CNameManager, dump)
{
	if (args.size() == 1 && args[0] == "all")
	{
		// dump the complete name table (heavy !)
		const TNamesIndex::TBToAMap &ba = _Names.getBToAMap();

		log.displayNL("Dumping %u character names :", ba.size());

		TNamesIndex::TBToAMap::const_iterator first(ba.begin()), last(ba.end());
		for (; first != last; ++first)
		{
			const TCharSlot &charSlot = first->first;
			const TFullName &fullName = first->second;

			log.displayNL("  User %u, Slot %u (char_id %u) : Name '%s' on session %u",
				charSlot.UserId,
				charSlot.CharIndex,
				charSlot.UserId*16+charSlot.CharIndex,
				fullName.Name.c_str(),
				fullName.HomeSessionId.asInt());
		}

		return true;
	}

	if (args.size() != 0)
		return false;

	log.displayNL("Dumping NameManager internal state :");
	log.displayNL(" - %u character names", _Names.getAToBMap().size());
	log.displayNL(" - %u account names", _AccountNames.size());
	log.displayNL(" - %u guild names", _GuildNames.size());
	log.displayNL(" - %u guild index", _GuildIndex.size());
	log.displayNL(" - %u reserved names", _ReservedNames.size());
	log.displayNL(" - %u forbidden names", _ForbiddenNames.size());
	log.displayNL("End of dump");

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CNameManager, releaseGuildNamesForShard)
{
	if (args.size() != 1)
		return false;

	uint32 shardId = atoui(args[1].c_str());

	vector<uint32> guildNameToRemove;

	TGuildNames::iterator first(_GuildNames.begin()), last(_GuildNames.end());
	for (; first != last; ++first)
	{
		if (first->second.ShardId == shardId)
			guildNameToRemove.push_back(first->second.GuildId);
	}

	log.displayNL("Releasing %u guild name for shard %u", guildNameToRemove.size(), shardId);

	for (uint i=0; i<guildNameToRemove.size(); ++i)
	{
		uint32 guildId = guildNameToRemove[i];
		TGuildIndex::iterator it(_GuildIndex.find(guildId));
		BOMB_IF(it == _GuildIndex.end(), "Error in guildname/index management", continue);

		_GuildNames.erase(it->second);
		_GuildIndex.erase(it);
	}

	return true;
}
