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
#include "fx_entity_manager.h"

CFxEntityManager* CFxEntityManager::_Instance = NULL;

CFxEntityManager* CFxEntityManager::getInstance()
{
	if (!_Instance)
		_Instance = new CFxEntityManager();
	return _Instance;
}

void CFxEntityManager::destroyInstance()
{
	delete _Instance;
	_Instance = NULL;
}

CFxEntityPtr CFxEntityManager::create(CAIPos const& pos, NLMISC::CSheetId const& sheet)
{
	CFxEntityPtr entity(new CFxEntity(pos, sheet));
	_Entities.insert(entity);
	return entity;
}

void CFxEntityManager::registerEntity(CFxEntityPtr const& entity)
{
	_IdIndex.insert(std::make_pair(entity->id(), entity));
}

void CFxEntityManager::unregisterEntity(CFxEntityPtr const& entity)
{
	_IdIndex.erase(entity->id());
}

void CFxEntityManager::destroy(CFxEntityPtr const& entity)
{
	_Entities.erase(entity);
}

CFxEntityPtr CFxEntityManager::get(NLMISC::CEntityId const& id)
{
	TEntityIdContainer::iterator it = _IdIndex.find(id);
	if (it!=_IdIndex.end())
		return it->second;
	else
		return NULL;
}

void CFxEntityManager::init(TDataSetIndex baseRowIndex, TDataSetIndex size)
{
}

void CFxEntityManager::tickUpdate()
{
	/*
	// The following loop is there to allow entities to destroy themselves
	FOREACH(itEntity, TFxEntityContainer, _Entities)
	{
		CFxEntityPtr entity = (*itEntity);
		if (entity.isNull())
			continue;
		
		if (!entity->update())
		{
			entity->despawn();
			(*itEntity) = NULL;
			--_NbEntities;
		}
	}
	*/
}

bool CFxEntityManager::exists(NLMISC::CEntityId const& id)
{
	return _IdIndex.find(id) != _IdIndex.end();
}

/// Valid format for request string is:
/// - *				Select all entities
/// - <entity id>	Select the specified entity using his eid (format is "(id:type:crea:dyn)")
/// - <name>		Select an entity with its name (any name allowed for a fx entity)
void CFxEntityManager::select(std::string const& request, std::vector<NLMISC::CEntityId>& entities)
{
	if (request.empty())
		return;
	
	if (request == "*")
	{
		// we want all entities
		FOREACHC(it, TEntityIdContainer, _IdIndex)
		{
			entities.push_back(it->first);
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
		// try with the name (when we add a name to these entities)
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
		CFxEntityManager::getInstance()->select(args[0], entities); \
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

#define ENTITY_GET_FXENTITY \
	CFxEntity* fxEntity = CFxEntityManager::getInstance()->get(entity); \
	if(fxEntity == 0) \
	{ \
		nlwarning("Unknown entity '%s'", entity.toString().c_str()); \
		if (get) value = "UnknownEntity"; \
		return; \
	} \


struct FxEntityChange
{
	FxEntityChange(NLMISC::CEntityId _id, std::string _prop, std::string _value)
		: id(_id), prop(_prop), value(_value) { }
	NLMISC::CEntityId id;
	std::string prop;
	std::string value;
};
std::queue<FxEntityChange> fxEntityChangeQueue;

void execFxEntityChanges()
{
	CFxEntityManager* manager = CFxEntityManager::getInstance();
	while (!fxEntityChangeQueue.empty())
	{
		FxEntityChange& fxEntityChange = fxEntityChangeQueue.front();
		CFxEntityPtr fxEntity = manager->get(fxEntityChange.id);
		if(fxEntity != 0)
		{
			fxEntity->set(fxEntityChange.prop, fxEntityChange.value, true);
		}
		fxEntityChangeQueue.pop();
	}
}



ENTITY_VARIABLE(FxEntitySheet, "Sheet of a fx entity")
{
	ENTITY_GET_FXENTITY
	
	if (get)
		value = fxEntity->get("sheet");
	else
		nlwarning("Trying to change a fx entity's sheet: '%s' to '%s'", fxEntity->get("sheet").c_str(), value.c_str());
}

ENTITY_VARIABLE(FxEntityPosition, "Position of a fx entity")
{
	ENTITY_GET_FXENTITY
		
		if (get)
			value = fxEntity->get("position");
		else
			nlwarning("Trying to change a fx entity's position: '%s' to '%s'", fxEntity->get("position").c_str(), value.c_str());
}

NLMISC_COMMAND(fxCreateEntity, "Create an fx entity", "<sheet> <x> <y>")
{
	if (args.size()<3)
		return false;
	NLMISC::CSheetId sheetId(args[0]);
	if (sheetId==NLMISC::CSheetId::Unknown)
	{
		log.displayNL("'%s' is not a valid fx sheet id", args[0].c_str());
		return true;
	}
	nldebug("Using sheet %s (%d 0x%08x)", sheetId.toString().c_str(), sheetId.asInt(), sheetId.asInt());
	double x = 0;
	double y = 0;
	NLMISC::fromString(args[1], x);
	NLMISC::fromString(args[2], y);
	CFxEntityPtr fx = CFxEntityManager::getInstance()->create(CAIPos(x, y, 0, 0.f), sheetId);
	if (fx->spawn())
		log.displayNL("Created entity %s", fx->id().toString().c_str());
	else
		log.displayNL("Unable to spawn fx entity (mirror range full?)");
	return true;
}

NLMISC_COMMAND(fxDestroyEntity, "Create an fx entity", "<entity id>")
{
	if (args.size()<1)
		return false;
	NLMISC::CEntityId entityId(args[0]);
	if (entityId==NLMISC::CEntityId::Unknown)
	{
		log.displayNL("'%s' is not a valid entity id", args[0].c_str());
		return true;
	}
	
	CFxEntityPtr fx = CFxEntityManager::getInstance()->get(entityId);
	if (fx.isNull())
	{
		log.displayNL("'%s' is not a valid fx entity", entityId.toString().c_str());
		return true;
	}
	fx->despawn();
	CFxEntityManager::getInstance()->destroy(fx);
	return true;
}

NLMISC_COMMAND(fxManagerDumpIndex, "", "")
{
	CFxEntityManager::getInstance()->dumpIndex();
	return true;
}
