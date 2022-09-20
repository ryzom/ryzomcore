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

#ifndef RYAI_NAMED_ENTITY_MANAGER_H
#define RYAI_NAMED_ENTITY_MANAGER_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include "named_entity.h"
#include "named_entity_manager.h"

/**	Manager for named entities
	
	This class is a singleton.
	
	@author vuarand
*/
class CNamedEntityManager
{
public:
	/// Returns a pointer to the manager.
	static CNamedEntityManager* getInstance();
	/// Destroys the manager.
	static void destroyInstance();
	
	/// Creates a named entity or return an existing one with that name.
	CNamedEntity& create(std::string const& name);
	/// Returns a named entity with that name or create one if it doesn't exist.
	CNamedEntity& get(std::string const& name);
	/// Returns a named entity with that id or 0 (NULL) if it doesn't exist.
	CNamedEntity* get(NLMISC::CEntityId const& id);
	/// Removes a named entity from the manager.
	void destroy(CNamedEntity const& entity);
	/// Removes a named entity from the manager.
	void destroy(std::string const& name);
	/// Removes a named entity from the manager.
	void destroy(NLMISC::CEntityId const& name);
	/// Tells if an entity with the specified name exists in the manager.
	bool exists(std::string const& name);
	/// Tells if an entity with the specified id exists in the manager.
	bool exists(NLMISC::CEntityId const& id);
	/// Select a set of entities depending of a request string.
	void select(std::string const& request, std::vector<NLMISC::CEntityId>& entities);
	
private:
	static CNamedEntityManager* _instance;
	typedef std::set<CNamedEntity*> entity_container_t;
	typedef std::map<std::string, CNamedEntity*> entity_name_container_t;
	typedef std::map<NLMISC::CEntityId, CNamedEntity*> entity_id_container_t;
	entity_container_t _entities;
	entity_name_container_t _nameIndex;
	entity_id_container_t _idIndex;
};

#endif // RYAI_NAMED_ENTITY_MANAGER_H
