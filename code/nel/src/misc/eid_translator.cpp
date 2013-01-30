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

//
// Includes
//

#include "stdmisc.h"

#include "nel/misc/algo.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/eid_translator.h"
#include "nel/misc/hierarchical_timer.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

//
// Variables
//

//CEntityIdTranslator *CEntityIdTranslator::Instance = NULL;
NLMISC_SAFE_SINGLETON_IMPL(CEntityIdTranslator);

// don't forget to increment the number when you change the file format
const uint CEntityIdTranslator::Version = 1;

//
// Functions
//

void CEntityIdTranslator::CEntity::serial (NLMISC::IStream &s)
{
	H_AUTO(EIdTrans_serial);
	s.serial (EntityName);

	if (CEntityIdTranslator::getInstance()->FileVersion >= 1)
		s.serial (EntitySlot);
	else
	{
		if(s.isReading())
		{
			EntitySlot = -1;
		}
		else
		{
			sint8 slot = -1;
			s.serial (slot);
		}
	}
	s.serial (UId);
	s.serial (UserName);
}

//CEntityIdTranslator *CEntityIdTranslator::getInstance ()
//{
//	if(Instance == NULL)
//	{
//		Instance = new CEntityIdTranslator;
//	}
//	return Instance;
//}

void CEntityIdTranslator::getByUser (uint32 uid, vector<CEntityId> &res)
{
	H_AUTO(EIdTrans_getByUser);
	for (TEntityCont::iterator it = RegisteredEntities.begin(); it != RegisteredEntities.end(); it++)
	{
		CEntity &entity = it->second;
		if (entity.UId == uid)
		{
			res.push_back(it->first);
		}
	}
}

void CEntityIdTranslator::getByUser (const string &userName, vector<CEntityId> &res, bool exact)
{
	H_AUTO(EIdTrans_getByUser2);
	string lowerName = toLower(userName);

	for (TEntityCont::iterator it = RegisteredEntities.begin(); it != RegisteredEntities.end(); it++)
	{
		if (exact)
		{
			CEntity &entity = it->second;
			if (toLower(entity.UserName) == lowerName)
			{
				res.push_back(it->first);
			}
		}
		else
		{
			CEntity &entity = it->second;
			if (toLower(entity.UserName).find(lowerName) != string::npos)
			{
				res.push_back(it->first);
			}
		}
	}
}

const ucstring &CEntityIdTranslator::getByEntity (const CEntityId &eid)
{
	H_AUTO(EIdTrans_getByEntity);
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	TEntityCont::iterator it = RegisteredEntities.find (reid);
	if (it == RegisteredEntities.end ())
	{
		static ucstring emptyString;
		return emptyString;
	}
	else
	{
		CEntity &entity = it->second;
		return entity.EntityName;
	}
}

CEntityId CEntityIdTranslator::getByEntity (const ucstring &entityName)
{
	H_AUTO(EIdTrans_getByEntity2);
	vector<CEntityId> res;
	getByEntity (entityName, res, true);
	if (res.empty())
		return CEntityId::Unknown;
	else
		return res[0];
}

void CEntityIdTranslator::getByEntity (const ucstring &entityName, vector<CEntityId> &res, bool exact)
{
	H_AUTO(EIdTrans_getByEntity3);
	string lowerName = toLower(entityName.toString());

	if (exact)
	{
		// use the reverse index to speed up search
		TNameIndexCont::iterator it(NameIndex.find(lowerName));
		if (it != NameIndex.end())
			res.push_back(it->second);

		return;
	}
	// parse the entire container to match all entities
	for (TEntityCont::iterator it = RegisteredEntities.begin(); it != RegisteredEntities.end(); ++it)
	{
		CEntity &entity = it->second;
		if (toLower(entity.EntityName.toString()).find(lowerName) != string::npos)
		{
			res.push_back(it->first);
		}
	}
}

bool CEntityIdTranslator::isValidEntityName (const ucstring &entityName,CLog *log)
{
	H_AUTO(EIdTrans_isValidEntityName);
	// 3 char at least
	if (entityName.size() < 3)
	{
		log->displayNL("Bad entity name '%s' (less than 3 char)", entityName.toString().c_str());
		return false;
	}

	if (entityName.size() > 15)
	{
		// if a parenthesis is found before 15 chars, the name is valid
		if (entityName.find(ucstring("(")) > 15 || entityName[entityName.size()-1] != ucchar(')'))
		{
			log->displayNL("EIT: Bad entity name '%s' (more than 15 char)", entityName.toString().c_str());
			return false;
		}
	}

	bool allowNumeric = false;
	for (uint i = 0; i < entityName.size(); i++)
	{
		if (entityName[i] == '(')
		{
			// starting from shard name, allow alphanumeric character
			allowNumeric = true;
		}
		// only accept name with alphabetic and numeric value [a-zA-Z] and parenthesis
		if (!allowNumeric && !isalpha (entityName[i]) && entityName[i] != '(' && entityName[i] != ')')
		{
			log->displayNL("Bad entity name '%s' (only char and num)", entityName.toString().c_str());
			return false;
		}
	}

	// now check with the invalid name list
	string en = getRegisterableString( entityName );

	for (uint i = 0; i < InvalidEntityNames.size(); i++)
	{
		if(testWildCard(en, InvalidEntityNames[i]))
		{
			log->displayNL("Bad entity name '%s' (match the invalid entity name pattern '%s')", entityName.toString().c_str(), InvalidEntityNames[i].c_str());
			return false;
		}
	}

	return true;
}

void CEntityIdTranslator::clear()
{
	NameIndex.clear();
	RegisteredEntities.clear();
}


bool CEntityIdTranslator::checkEntityName (const ucstring &entityName )
{
	H_AUTO(EIdTrans_entityNameExists);
	// if bad name, don't accept it
	if (!isValidEntityName (entityName,NLMISC::InfoLog)) return false;
	return !entityNameExists( entityName );
}

bool CEntityIdTranslator::entityNameExists (const ucstring &entityName )
{
	// Names are stored in case dependant, so we have to test them without case.
	ucstring registerable = getRegisterableString (entityName);

	return NameIndex.find(registerable) !=NameIndex.end();
/*	for (TEntityCont::iterator it = RegisteredEntities.begin(); it != RegisteredEntities.end(); it++)
	{
		if (getRegisterableString ((*it).second.EntityName) == registerable)
		{
			return true;
		}
	}
	return false;
*/
}

void CEntityIdTranslator::registerEntity (const CEntityId &eid, const ucstring &entityName, sint8 entitySlot, uint32 uid, const string &userName, uint32 shardId)
{
	H_AUTO(EIdTrans_registerEntity);
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	if (RegisteredEntities.find (reid) != RegisteredEntities.end ())
	{
		nlwarning ("EIT: Can't register EId %s EntityName '%s' UId %d UserName '%s' because EId is already in the map", reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
		return;
	}

	if (!checkEntityName(entityName))
	{
		if (isValidEntityName(entityName))
			nlwarning ("EIT: Can't register EId %s EntityName '%s' UId %d UserName '%s' because EntityName is already in the map", reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
		else
			nlwarning ("EIT: Can't register EId %s EntityName '%s' UId %d UserName '%s' because EntityName is invalid", reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
		return;
	}

	nlinfo ("EIT: Register EId %s EntityName '%s' UId %d UserName '%s'", reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
	RegisteredEntities.insert (make_pair(reid, CEntityIdTranslator::CEntity(entityName, uid, userName, entitySlot, shardId)));
	NameIndex.insert(make_pair(toLower(entityName), reid));
}

void CEntityIdTranslator::updateEntity (const CEntityId &eid, const ucstring &entityName, sint8 entitySlot, uint32 uid, const std::string &userName, uint32 shardId)
{
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	TEntityCont::iterator it = RegisteredEntities.find (reid);

	if (it == RegisteredEntities.end())
	{
		// just register
		registerEntity(eid, entityName, entitySlot, uid, userName, shardId);
	}
	else
	{
		// update entity entry and name index
		CEntity &entity = it->second;
		if (entity.EntityName != entityName)
		{
			if (!checkEntityName(entityName))
			{
				nlwarning ("EIT: Can't update EId %s EntityName '%s' UId %d UserName '%s' with new name '%s' because EntityName is already in the map",
					reid.toString().c_str(),
					entity.EntityName.toString().c_str(),
					uid,
					userName.c_str(),
					entityName.toString().c_str());
				return;
			}
			// update the name and name index
			NameIndex.erase(toLower(entity.EntityName));
			NameIndex.insert(make_pair(toLower(entityName), reid));
			entity.EntityName = entityName;
			entity.EntityNameStringId = 0;
		}
		entity.EntitySlot = entitySlot;
		entity.UId = uid;
		entity.UserName = userName;
		entity.ShardId = shardId;
	}
}


void CEntityIdTranslator::unregisterEntity (const CEntityId &eid)
{
	H_AUTO(EIdTrans_unregisterEntity);
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	TEntityCont::iterator it = RegisteredEntities.find (reid);

	if (it == RegisteredEntities.end ())
	{
		nlwarning ("EIT: Can't unregister EId %s because EId is not in the map", reid.toString().c_str());
		return;
	}

	CEntity &entity = it->second;

	nldebug ("EIT: Unregister EId %s EntityName '%s' UId %d UserName '%s'", reid.toString().c_str(), entity.EntityName.toString().c_str(), entity.UId, entity.UserName.c_str());
	NameIndex.erase(toLower(entity.EntityName));
	RegisteredEntities.erase (reid);
}

bool CEntityIdTranslator::isEntityRegistered(const CEntityId &eid)
{
	H_AUTO(EIdTrans_unregisterEntity);
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	TEntityCont::iterator it = RegisteredEntities.find (reid);

	return it != RegisteredEntities.end ();
}


void CEntityIdTranslator::checkEntity (const CEntityId &eid, const ucstring &entityName, uint32 uid, const string &userName)
{
	H_AUTO(EIdTrans_checkEntity);
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	map<CEntityId, CEntityIdTranslator::CEntity>::iterator it = RegisteredEntities.find (reid);

	nlinfo ("EIT: Checking EId %s EntityName '%s' UId %d UserName '%s'", reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());

	if (it == RegisteredEntities.end ())
	{
		nlwarning ("EIT: Check failed because EId is not in the CEntityIdTranslator map for EId %s EntityName '%s' UId %d UserName '%s'", reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());

		if (checkEntityName(entityName))
		{
			nlwarning ("EIT: Check failed because entity name already exist '%s' for EId %s EntityName '%s' UId %d UserName '%s'", getByEntity(entityName).toString().c_str(), reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
		}
	}
	else
	{
		CEntity &entity = it->second;
		if (entity.EntityName != entityName)
		{
			nlwarning ("EIT: Check failed because entity name not identical '%s' in the CEntityIdTranslator map for EId %s EntityName '%s' UId %d UserName '%s'", entity.EntityName.toString().c_str(), reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
			if(!entityName.empty())
			{
				entity.EntityName = entityName;
			}
		}
		if (entity.UId != uid)
		{
			nlwarning ("EIT: Check failed because uid not identical (%d) in the CEntityIdTranslator map for EId %s EntityName '%s' UId %d UserName '%s'", entity.UId, reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
			if (uid != 0)
			{
				entity.UId = uid;
			}
		}
		if (entity.UserName != userName)
		{
			nlwarning ("EIT: Check failed because user name not identical '%s' in the CEntityIdTranslator map for EId %s EntityName '%s' UId %d UserName '%s'", entity.UserName.c_str(), reid.toString().c_str(), entityName.toString().c_str(), uid, userName.c_str());
			if(!userName.empty())
			{
				entity.UserName = userName;
			}
		}
	}
}

void CEntityIdTranslator::removeShardFromName(ucstring& name)
{
	// The string must contain a '(' and a ')'
	ucstring::size_type	p0= name.find('(');
	ucstring::size_type	p1= name.find(')');
	if (p0 == ucstring::npos || p1 == ucstring::npos || p1 <= p0)
		return;

	name = name.substr(0, p0) + name.substr(p1 + 1);
}

// this callback is call when the file is changed
void cbInvalidEntityNamesFilename(const std::string &invalidEntityNamesFilename)
{
	CEntityIdTranslator::getInstance()->InvalidEntityNames.clear ();

	string fn = CPath::lookup(invalidEntityNamesFilename, false);

	if (fn.empty())
	{
		nlwarning ("EIT: Can't load filename '%s' for invalid entity names filename (not found)", invalidEntityNamesFilename.c_str());
		return;
	}

	FILE *fp = fopen (fn.c_str(), "r");
	if (fp == NULL)
	{
		nlwarning ("EIT: Can't load filename '%s' for invalid entity names filename", fn.c_str());
		return;
	}

	for(;;)
	{
		char str[512];
		if (!fgets(str, 511, fp))
			break;
		if(feof(fp))
			break;
		if (strlen(str) > 0)
		{
			str[strlen(str)-1] = '\0';
			CEntityIdTranslator::getInstance()->InvalidEntityNames.push_back(str);
		}
	}

	fclose (fp);
}

void CEntityIdTranslator::load (const string &fileName, const string &invalidEntityNamesFilename)
{
	H_AUTO(EIdTrans_load);
	if (fileName.empty())
	{
		nlwarning ("EIT: Can't load empty filename for EntityIdTranslator");
		return;
	}

	if (!FileName.empty())
	{
		nlwarning ("EIT: Can't load file '%s' for EntityIdTranslator because we already load the file '%s'", fileName.c_str(), FileName.c_str());
		return;
	}

	nlinfo ("EIT: CEntityIdTranslator: load '%s'", fileName.c_str());

	FileName = fileName;

	if(CFile::fileExists(FileName))
	{
		CIFile ifile;
		if( ifile.open(FileName) )
		{
			FileVersion = Version;
			ifile.serialVersion (FileVersion);
			ifile.serialCont (RegisteredEntities);

			ifile.close ();

			// fill the entity name index container
			NameIndex.clear();
			TEntityCont::iterator first(RegisteredEntities.begin()), last(RegisteredEntities.end());
			for (; first != last; ++first)
			{
				NameIndex.insert(make_pair(toLower(first->second.EntityName), first->first));
			}
		}
		else
		{
			nlwarning ("EIT: Can't load filename '%s' for EntityIdTranslator", FileName.c_str());
		}
	}

	cbInvalidEntityNamesFilename (invalidEntityNamesFilename);

	NLMISC::CFile::addFileChangeCallback (invalidEntityNamesFilename, cbInvalidEntityNamesFilename);
}

void CEntityIdTranslator::save ()
{
	H_AUTO(EIdTrans_save);

	if (FileName.empty())
	{
		nlwarning ("EIT: Can't save empty filename for EntityIdTranslator (you forgot to load() it before?)");
		return;
	}

	nlinfo ("EIT: CEntityIdTranslator: save");

	COFile ofile;
	if( ofile.open(FileName) )
	{
		ofile.serialVersion (Version);
		FileVersion = Version;
		ofile.serialCont (RegisteredEntities);

		ofile.close ();
	}
	else
	{
		nlwarning ("EIT: Can't save filename '%s' for EntityIdTranslator", FileName.c_str());
	}
}

uint32 CEntityIdTranslator::getUId (const string &userName)
{
	const TEntityCont::iterator itEnd = RegisteredEntities.end();
	for (TEntityCont::iterator it = RegisteredEntities.begin(); it != itEnd ; ++it)
	{
		CEntity &entity = it->second;
		if (entity.UserName == userName)
		{
			return entity.UId;
		}
	}
	return 0;
}

string CEntityIdTranslator::getUserName (uint32 uid)
{
	const TEntityCont::iterator itEnd = RegisteredEntities.end();
	for (TEntityCont::iterator it = RegisteredEntities.begin(); it != itEnd ; ++it)
	{
		CEntity &entity = it->second;
		if (entity.UId == uid)
		{
			return entity.UserName;
		}
	}
	return 0;
}

void CEntityIdTranslator::getEntityIdInfo (const CEntityId &eid, ucstring &entityName, sint8 &entitySlot, uint32 &uid, string &userName, bool &online, std::string* additional)
{
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	if (additional != NULL)
		additional->clear();

	TEntityCont::iterator it = RegisteredEntities.find (reid);
	if (it == RegisteredEntities.end ())
	{
		nlwarning ("EIT: %s is not registered in CEntityIdTranslator", reid.toString().c_str());
		entityName = "";
		entitySlot = -1;
		uid = std::numeric_limits<uint32>::max();
		userName = "";
		online = false;
	}
	else
	{
		CEntity &entity = it->second;
		entityName = entity.EntityName;
		entitySlot = entity.EntitySlot;
		uid = entity.UId;
		userName = entity.UserName;
		online = entity.Online;

		if (EntityInfoCallback != NULL && additional != NULL)
			*additional = EntityInfoCallback(eid);
	}
}

bool CEntityIdTranslator::setEntityNameStringId(const ucstring &entityName, uint32 stringId)
{
	const TEntityCont::iterator itEnd = RegisteredEntities.end();
	for (TEntityCont::iterator it = RegisteredEntities.begin(); it != itEnd ; ++it)
	{
		CEntity &entity = it->second;
		if (entity.EntityName == entityName)
		{
			entity.EntityNameStringId = stringId;
			return true;
		}
	}

	return false;
}

bool CEntityIdTranslator::setEntityNameStringId(const CEntityId &eid, uint32 stringId)
{
	TEntityCont::iterator it (RegisteredEntities.find(eid));
	if (it == RegisteredEntities.end())
	{
		// there is nothing we can do !
		return false;
	}

	CEntity &entity = it->second;
	entity.EntityNameStringId = stringId;

	return true;
}

uint32 CEntityIdTranslator::getEntityNameStringId(const CEntityId &eid)
{
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	const TEntityCont::iterator it = RegisteredEntities.find (reid);
	if (it == RegisteredEntities.end ())
	{
		return 0;
	}
	else
	{
		CEntity &entity = it->second;
		return entity.EntityNameStringId;
	}
}

// get the shard id of an entity
uint32	CEntityIdTranslator::getEntityShardId(const CEntityId &eid)
{
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	const TEntityCont::iterator it = RegisteredEntities.find (reid);
	if (it == RegisteredEntities.end ())
	{
		return 0;
	}
	else
	{
		CEntity &entity = it->second;
		return entity.ShardId;
	}
}


void CEntityIdTranslator::setEntityOnline (const CEntityId &eid, bool online)
{
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	TEntityCont::iterator it = RegisteredEntities.find (reid);
	if (it == RegisteredEntities.end ())
	{
		nlwarning ("EIT: %s is not registered in CEntityIdTranslator", reid.toString().c_str());
	}
	else
	{
		CEntity &entity = it->second;
		entity.Online = online;
	}
}

bool CEntityIdTranslator::isEntityOnline (const CEntityId &eid)
{
	// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
	CEntityId reid(eid);
	reid.setCreatorId(0);
	reid.setDynamicId(0);

	TEntityCont::iterator it = RegisteredEntities.find (reid);
	if (it == RegisteredEntities.end ())
	{
		nlwarning ("EIT: %s is not registered in CEntityIdTranslator", reid.toString().c_str());
		return false;
	}
	else
	{
		CEntity &entity = it->second;
		return entity.Online;
	}
}

std::string CEntityIdTranslator::getRegisterableString( const ucstring & entityName )
{
	string ret = toLower( entityName.toString() );
	string::size_type pos = ret.find( 0x20 );
	while( pos != string::npos )
	{
		ret.erase( pos,1 );
		pos = ret.find( 0x20 );
	}
	return ret;
}


NLMISC_CATEGORISED_COMMAND(nel,findEIdByUser,"Find entity ids using the user name","<username>|<uid>")
{
	if (args.size () != 1)
		return false;

	vector<CEntityId> res;

	string userName = args[0];
	uint32 uid = atoi (userName.c_str());

	if (uid != 0)
	{
		CEntityIdTranslator::getInstance()->getByUser(uid, res);
		userName = CEntityIdTranslator::getInstance()->getUserName(uid);
	}
	else
	{
		CEntityIdTranslator::getInstance()->getByUser(userName, res);
		CEntityIdTranslator::getInstance()->getUId(userName);
	}

	log.displayNL("User Name '%s' (uid=%d) has %d entities:", userName.c_str(), uid, res.size());
	for (uint i = 0 ; i < res.size(); i++)
	{
		log.displayNL(">  %s '%s'", res[i].toString().c_str(), CEntityIdTranslator::getInstance()->getByEntity (res[i]).c_str());
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(nel,findEIdByEntity,"Find entity id using the entity name","<entityname>|<eid>")
{
	if (args.size () != 1)
		return false;

	CEntityId eid (args[0].c_str());

	if (eid == CEntityId::Unknown)
	{
		eid = CEntityIdTranslator::getInstance()->getByEntity(args[0]);
	}

	if (eid == CEntityId::Unknown)
	{
		log.displayNL("'%s' is not an eid or an entity name", args[0].c_str());
		return false;
	}

	ucstring entityName;
	sint8 entitySlot;
	uint32 uid;
	string userName;
	bool online;
	std::string	extinf;

	CEntityIdTranslator::getInstance()->getEntityIdInfo(eid, entityName, entitySlot, uid, userName, online, &extinf);

	log.displayNL("UId %d UserName '%s' EId %s EntityName '%s' EntitySlot %hd %s%s%s", uid, userName.c_str(), eid.toString().c_str(), entityName.toString().c_str(), (sint16)entitySlot, (extinf.c_str()), (extinf.empty() ? "" : " "), (online?"Online":"Offline"));

	return true;
}

NLMISC_CATEGORISED_COMMAND(nel,entityNameValid,"Tell if an entity name is valid or not using CEntityIdTranslator validation rulez","<entityname>")
{
	if (args.size () != 1) return false;

	if(!CEntityIdTranslator::getInstance()->isValidEntityName(args[0], &log))
	{
		log.displayNL("Entity name '%s' is not valid", args[0].c_str());
	}
	else
	{
		if (CEntityIdTranslator::getInstance()->checkEntityName(args[0]))
		{
			log.displayNL("Entity name '%s' is already used by another player", args[0].c_str());
		}
		else
		{
			log.displayNL("Entity name '%s' is available", args[0].c_str());
		}
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(nel,playerInfo,"Get information about a player or all players in CEntityIdTranslator","[<entityname>|<eid>|<username>|<uid>]")
{
	if (args.size () == 0)
	{
		const map<CEntityId, CEntityIdTranslator::CEntity>	&res = CEntityIdTranslator::getInstance()->getRegisteredEntities ();
		log.displayNL("%d result(s) for 'all players information'", res.size());
		for (map<CEntityId, CEntityIdTranslator::CEntity>::const_iterator it = res.begin(); it != res.end(); it++)
		{
			const CEntityIdTranslator::CEntity &entity = it->second;
			log.displayNL("UId %d UserName '%s' EId %s EntityName '%s' EntitySlot %hd %s", entity.UId, entity.UserName.c_str(), it->first.toString().c_str(), entity.EntityName.toString().c_str(), (sint16)(entity.EntitySlot), (entity.Online?"Online":"Offline"));
		}

		return true;
	}
	else if (args.size () == 1)
	{
		vector<CEntityId> res;

		CEntityId eid (args[0].c_str());
		uint32 uid = atoi (args[0].c_str());

		if (eid != CEntityId::Unknown)
		{
			// we have to remove the crea and dyna because it can changed dynamically and will not be found in the storage array
			eid.setCreatorId(0);
			eid.setDynamicId(0);

			res.push_back(eid);
		}
		else if (uid != 0)
		{
			// the parameter is an uid
			CEntityIdTranslator::getInstance()->getByUser (uid, res);
		}
		else
		{
			CEntityIdTranslator::getInstance()->getByUser (args[0], res, false);

			CEntityIdTranslator::getInstance()->getByEntity (args[0], res, false);
		}

		log.displayNL("%d result(s) for '%s'", res.size(), args[0].c_str());
		for (uint i = 0; i < res.size(); i++)
		{
			ucstring entityName;
			sint8 entitySlot;
			uint32 uid2;
			string userName;
			bool online;
			std::string	extinf;
			CEntityIdTranslator::getInstance()->getEntityIdInfo (res[i], entityName, entitySlot, uid2, userName, online, &extinf);

			log.displayNL("UId %d UserName '%s' EId %s EntityName '%s' EntitySlot %hd %s%s%s", uid2, userName.c_str(), res[i].toString().c_str(), entityName.toString().c_str(), (sint16)entitySlot, (extinf.c_str()), (extinf.empty() ? "" : " "), (online?"Online":"Offline"));
		}

		return true;
	}

	return false;
}

}
