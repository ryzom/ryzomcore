// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef	NL_EID_TRANSLATOR_H
#define	NL_EID_TRANSLATOR_H

#include <string>
#include <vector>
#include <map>
#include <limits>

#include "types_nl.h"
#include "entity_id.h"
#include "ucstring.h"
#include "command.h"

namespace	NLMISC
{

class CEntityIdTranslator
{
	NLMISC_SAFE_SINGLETON_DECL_PTR(CEntityIdTranslator);
public:

	/** Descriptor for an entity in the translator */
	struct CEntity
	{
		CEntity ()
			:	EntityNameStringId(0),
				EntitySlot(-1),
				ShardId(0),
				UId(std::numeric_limits<uint32>::max()),
				Online(false)
		{ }

		CEntity (const ucstring &entityName, uint32 uid, const std::string &userName, sint8 entitySlot, uint32 shardId =0)
			:	EntityName(entityName),
				EntityNameStringId(0),
				EntitySlot(entitySlot),
				ShardId(shardId),
				UId(uid),
				UserName(userName),
				Online(false)
		{ }

		/// The display name of the entity
		ucstring	EntityName;
		/// The mapped name of the entity (used to store IOS generated string id)
		uint32		EntityNameStringId;
		/// the character slot
		sint8		EntitySlot;
		/// Shard id of the entity (for multi shard systems)
		uint32		ShardId;

		/// User id the character owner (aka account id)
		uint32		UId;
		/// User name of the character owner (aka account name)
		std::string UserName;

		/// A flag stating if the character is online
		bool		Online;

		void serial (NLMISC::IStream &s);
	};

	typedef std::map<NLMISC::CEntityId, CEntity>	TEntityCont;

	/// clear all the registered entities from the translator.
	void				clear();
	// performs all check on a name ( name validity + uniqueness )
	bool				checkEntityName (const ucstring &entityName);
	/// return true if a name already exists
	bool				entityNameExists(const ucstring &entityName);
	// register an entity in this manager
	void				registerEntity (const CEntityId &eid, const ucstring &entityName, sint8 entitySlot, uint32 uid, const std::string &userName, uint32 shardId =0);
	// register or update an entity in this manager
	void				updateEntity (const CEntityId &eid, const ucstring &entityName, sint8 entitySlot, uint32 uid, const std::string &userName, uint32 shardId =0);
	// unregister an entity from this manager
	void				unregisterEntity (const CEntityId &eid);
	// Check if an entity is registered
	bool				isEntityRegistered(const CEntityId &eid);
	// set an association entityName / entityStringId, return true if association has been set
	bool				setEntityNameStringId(const ucstring &entityName, uint32 stringId);
	// set an association entityId / entityStringId, return true if association has been set
	bool				setEntityNameStringId(const CEntityId &eid, uint32 stringId);
	// get string id for entityId
	uint32				getEntityNameStringId(const CEntityId &eid);
	// get the shard id of an entity
	uint32				getEntityShardId(const CEntityId &eid);

	// set an eid to online or not
	void				setEntityOnline (const CEntityId &eid, bool online);

	// is an entity in online
	bool				isEntityOnline (const CEntityId &eid);

	// check if parameters are coherent with the content of the class, if not, set with the parameters and warn
	void				checkEntity (const CEntityId &eid, const ucstring &entityName, uint32 uid, const std::string &userName);

	// the first param is the file where are all entities information, the second is a text file (one line per pattern using * and ?) with invalid entity name
	void				load (const std::string &fileName, const std::string &invalidEntityNamesFilename);

	// you must call this function to save the data into the hard drive
	void				save ();

	// get eid using the entity name
	CEntityId			getByEntity (const ucstring &entityName);

	// get entity name using the eid
	const ucstring		&getByEntity (const NLMISC::CEntityId &eid);

	void				getEntityIdInfo (const CEntityId &eid, ucstring &entityName, sint8 &entitySlot, uint32 &uid, std::string &userName, bool &online, std::string* additional = NULL);

	// transform a username ucstring into a string that can be compared with registered string
	std::string			getRegisterableString( const ucstring & entityName);

	/// return a vector of invalid names
	const std::vector<std::string> & getInvalidNames(){ return InvalidEntityNames; }


	const TEntityCont	&getRegisteredEntities () { return RegisteredEntities; }

	static const uint Version;

	uint FileVersion;

	/**
	 * Callback called when getEntityIdInfo called, so service may add additional info
	 * Format MUST be [InfoName InfoValue]* (e.g. a list of 2 strings, first being name for
	 * the retrieved info, and second being the value of the info
	 */
	typedef std::string	(*TAdditionalInfoCb)(const CEntityId &eid);

	TAdditionalInfoCb	EntityInfoCallback;

	static void removeShardFromName(ucstring& name);

private:
	// get all eid for a user using the user name or the user id
	void				getByUser (uint32 uid, std::vector<NLMISC::CEntityId> &res);
	void				getByUser (const std::string &userName, std::vector<NLMISC::CEntityId> &res, bool exact=true);

	void				getByEntity (const ucstring &entityName, std::vector<NLMISC::CEntityId> &res, bool exact);

	// return the user id and 0 if not found
	uint32				getUId (const std::string &userName);
	std::string			getUserName (uint32 uid);

	// Returns true if the username is valid.
	// It means that there only alphabetic and numerical character and the name is at least 3 characters long.
	bool isValidEntityName (const ucstring &entityName, NLMISC::CLog *log = NLMISC::InfoLog );


	/// The container for all entity in the translator
	TEntityCont			RegisteredEntities;

	typedef std::map<ucstring, NLMISC::CEntityId>	TNameIndexCont;
	/// the reverse index to retreive entity by name
	TNameIndexCont	NameIndex;

	// Singleton, no ctor access
	CEntityIdTranslator() { EntityInfoCallback = NULL; }

	std::string FileName;

	std::vector<std::string> InvalidEntityNames;

	friend void cbInvalidEntityNamesFilename(const std::string &filename);
	friend struct entityNameValidClass;
	NLMISC_CATEGORISED_COMMAND_FRIEND(nel,findEIdByUser);
	NLMISC_CATEGORISED_COMMAND_FRIEND(nel,findEIdByEntity);
	NLMISC_CATEGORISED_COMMAND_FRIEND(nel,entityNameValid);
	NLMISC_CATEGORISED_COMMAND_FRIEND(nel,playerInfo);
};

}

#endif // NL_EID_TRANSLATOR_H

/* End of eid_translator.h */

