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

#include <queue>
#include "named_entity_manager.h"

CNamedEntityManager* CNamedEntityManager::_instance = 0;

CNamedEntityManager* CNamedEntityManager::getInstance()
{
	if (_instance==0)
		_instance = new CNamedEntityManager();
	return _instance;
}

void CNamedEntityManager::destroyInstance()
{
	delete _instance;
	_instance = 0;
}

CNamedEntity& CNamedEntityManager::create(std::string const& name)
{
	return get(name);
}

CNamedEntity& CNamedEntityManager::get(std::string const& name)
{
	entity_name_container_t::iterator it = _nameIndex.find(name);
	if (it!=_nameIndex.end())
		return *it->second;
	else
	{
		CNamedEntity* ptr = new CNamedEntity(name);
		_entities.insert(ptr);
		_nameIndex.insert(std::make_pair(ptr->name(), ptr));
		_idIndex.insert(std::make_pair(ptr->id(), ptr));
		return *ptr;
	}
}

CNamedEntity* CNamedEntityManager::get(NLMISC::CEntityId const& id)
{
	entity_id_container_t::iterator it = _idIndex.find(id);
	if (it!=_idIndex.end())
		return it->second;
	else
		return 0;
}

void CNamedEntityManager::destroy(CNamedEntity const& entity)
{
	destroy(entity.name());
}

void CNamedEntityManager::destroy(std::string const& name)
{
	entity_name_container_t::iterator it = _nameIndex.find(name);
	if (it!=_nameIndex.end())
	{
		CNamedEntity* ptr = it->second;
		_nameIndex.erase(ptr->name());
		_idIndex.erase(ptr->id());
		_entities.erase(ptr);
		delete ptr;
	}
}

void CNamedEntityManager::destroy(NLMISC::CEntityId const& id)
{
	entity_id_container_t::iterator it = _idIndex.find(id);
	if (it!=_idIndex.end())
	{
		CNamedEntity* ptr = it->second;
		_nameIndex.erase(ptr->name());
		_idIndex.erase(ptr->id());
		_entities.erase(ptr);
		delete ptr;
	}
}

bool CNamedEntityManager::exists(std::string const& name)
{
	return _nameIndex.find(name) != _nameIndex.end();
}

bool CNamedEntityManager::exists(NLMISC::CEntityId const& id)
{
	return _idIndex.find(id) != _idIndex.end();
}

/**	Valid format for request string is:
	- *				Select all entities
	- <entity id>	Select the specified entity using his eid (format is "(id:type:crea:dyn)")
	- <name>		Select an entity with its name (any name allowed for a named entity)
*/
void CNamedEntityManager::select(std::string const& request, std::vector<NLMISC::CEntityId>& entities)
{
	if (request.empty())
		return;
	
	if (request == "*")
	{
		// we want all entities
		FOREACHC(it, entity_container_t, _entities)
		{
			entities.push_back((*it)->id());
		}
	}
	else if (request[0] == '(')
	{
		// we want a specific entity id
		NLMISC::CEntityId id(request);
		if (exists(id))
			entities.push_back(id);
	}
	else
	{
		// try with the name
		if (exists(request))
			entities.push_back(get(request).id());
	}
}

#define ENTITY_VARIABLE(__name,__help) \
struct __name##Class : public NLMISC::ICommand \
{ \
__name##Class () : NLMISC::ICommand("variables",#__name, __help, "<entity> [<value>]") { Type = Variable; } \
	virtual bool execute(const std::string &rawCommandString, const std::vector<std::string> &args, NLMISC::CLog &log, bool quiet, bool human) \
	{ \
		if (args.size() != 1 && args.size() != 2) \
			return false; \
 \
		std::vector<NLMISC::CEntityId> entities; \
		CNamedEntityManager::getInstance()->select(args[0], entities); \
 \
		for (uint i = 0; i < entities.size(); i++) \
		{ \
			std::string value; \
			if (args.size()==2) \
				value = args[1]; \
			else \
				value = "???"; \
			pointer(entities[i], (args.size()==1), value); \
			if (quiet) \
				log.displayNL("%s %s", entities[i].toString().c_str(), value.c_str()); \
			else \
				log.displayNL("Entity %s Variable %s = %s", entities[i].toString().c_str(), _CommandName.c_str(), value.c_str()); \
		} \
		return true; \
	} \
	void pointer(NLMISC::CEntityId entity, bool get, std::string &value); \
}; \
__name##Class __name##Instance; \
void __name##Class::pointer(NLMISC::CEntityId entity, bool get, std::string &value)

#define ENTITY_GET_NAMEDENTITY \
	CNamedEntity* namedEntity = CNamedEntityManager::getInstance()->get(entity); \
	if(namedEntity == 0) \
	{ \
		nlwarning("Unknown entity '%s'", entity.toString().c_str()); \
		if (get) value = "UnknownEntity"; \
		return; \
	} \


struct NamedEntityChange
{
	NamedEntityChange(NLMISC::CEntityId _id, std::string _prop, std::string _value)
		: id(_id), prop(_prop), value(_value) { }
	NLMISC::CEntityId id;
	std::string prop;
	std::string value;
};
std::queue<NamedEntityChange> namedEntityChangeQueue;

void execNamedEntityChanges()
{
	CNamedEntityManager* manager = CNamedEntityManager::getInstance();
	while (!namedEntityChangeQueue.empty())
	{
		NamedEntityChange& namedEntityChange = namedEntityChangeQueue.front();
		CNamedEntity* namedEntity = manager->get(namedEntityChange.id);
		if(namedEntity != 0)
		{
			namedEntity->set(namedEntityChange.prop, namedEntityChange.value, true);
		}
		namedEntityChangeQueue.pop();
	}
}



ENTITY_VARIABLE(NamedEntityName, "Name of a named entity")
{
	ENTITY_GET_NAMEDENTITY
	
	if (get)
		value = namedEntity->name();
	else
		nlwarning("Trying to change a named entity's name: '%s' to '%s'", namedEntity->name().c_str(), value.c_str());
}

ENTITY_VARIABLE(NamedEntityState, "State of a named entity")
{
	ENTITY_GET_NAMEDENTITY
	
	if (get)
		value = namedEntity->getState();
	else
	//	namedEntity->set("state", value, true);
		namedEntityChangeQueue.push(NamedEntityChange(entity, "state", value));
}

ENTITY_VARIABLE(NamedEntityParam1, "Param1 of a named entity")
{
	ENTITY_GET_NAMEDENTITY
	
	if (get)
		value = namedEntity->getParam1();
	else
	//	namedEntity->set("param1", value, true);
		namedEntityChangeQueue.push(NamedEntityChange(entity, "param1", value));
}

ENTITY_VARIABLE(NamedEntityParam2, "Param1 of a named entity")
{
	ENTITY_GET_NAMEDENTITY
	
	if (get)
		value = namedEntity->getParam2();
	else
	//	namedEntity->set("param2", value, true);
		namedEntityChangeQueue.push(NamedEntityChange(entity, "param2", value));
}
