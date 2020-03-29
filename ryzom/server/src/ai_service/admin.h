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



#ifndef RYAI_ADMIN_H
#define RYAI_ADMIN_H

#include "nel/misc/types_nl.h"
#include "ai_share/ai_types.h"


void selectEntities (const string &entityName, vector <uint32> &entities)
{
	if (entityName.empty ())
		return;

	/*

	valid name formats:
	ALL MGRS:
	- *

	MGR: 
	- <mgr id> =
		- <mgr idx>
		- <mgr name>
		- <8 digit hex mgr CAIEntityId>
	
	ALL GRPS IN MGR:
	- <mgr id>*
	
	GRP: 
	- <grp id> =
		- <grp name>
		- <8 digit hex grp CAIEntityId>
		- <mgr id>:<grp index>
	
	ALL BOTS IN GRP:
	- <grp id>*
	
	INDIVIDUAL:
	- <bot id> =
		- <bot name>
		- <8 digit hex bot CAIEntityId>
		- <grp id>:<bot index>
	
	RANGE OF INDIVIDUALS:
	- <bot id>-<bot id>

	*/

	uint32 entity = atoi (entityName.c_str());

	if (entityName == "*")
	{
		// we want all entities
		for (uint i = 0; i < Entities.size(); i++)
			entities.push_back (i);
	}
	else if (entityName.find ("-") != string::npos)
	{
		// it's a range
		uint ent2;
		NLMISC::fromString(entityName.substr(entityName.find("-")+1), ent2);
		for (uint i = entity; i <= ent2; i++)
			entities.push_back (i);
	}
	else
	{
		// we want a specific entity
		entities.push_back (entity);
	}
}



#define ENTITY_VARIABLE(__name,__help) \
struct __name##Class : public NLMISC::ICommand \
{ \
__name##Class () : NLMISC::ICommand(#__name, __help, "<entity>
[<value>]") { Type = Variable; } \
	virtual bool execute(const std::vector<std::string> &args,
NLMISC::CLog &log) \
	{ \
		if (args.size () != 1 && args.size () != 2) \
			return false; \
 \
		vector <uint32> entities; \
		selectEntities	(args[0], entities); \
 \
		for (uint i = 0; i < entities.size(); i++) \
		{ \
			string value; \
			if (args.size()==2) \
				value = args[1]; \
			else \
				value = "???"; \
			pointer (entities[i], (args.size()==1), value); \
			log.displayNL ("Entity %d Variable %s = %s", entities[i], _CommandName.c_str(), value.c_str()); \
		} \
		return true; \
	} \
	void pointer(uint32 entity, bool get, std::string &value); \
}; \
__name##Class __name##Instance; \
void __name##Class::pointer(uint32 entity, bool get, std::string &value)

ENTITY_VARIABLE(test, "test")
{
	if (get)
	{
		// get the value if available
		if(entity < Entities.size())
			value = toString(Entities[entity].first);
	}
	else
	{
		// set the variable with the new value
		if(entity >= Entities.size())
			Entities.resize(entity+1);

		NLMISC::fromString(value, Entities[entity].first);
	}
}

ENTITY_VARIABLE(test2, "test2")
{
	if (get)
	{
		// get the value if available
		if(entity < Entities.size())
			value = toString(Entities[entity].second);
	}
	else
	{
		// set the variable with the new value
		if(entity >= Entities.size())
			Entities.resize(entity+1);
		
		NLMISC::fromString(value, Entities[entity].second);
	}
}

#endif
