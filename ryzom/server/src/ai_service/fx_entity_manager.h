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

#ifndef RYAI_FX_ENTITY_MANAGER_H
#define RYAI_FX_ENTITY_MANAGER_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include "fx_entity.h"
#include "fx_entity_manager.h"

/**	Manager for fx entities
	
	This class is a singleton.
	
	@author vuarand
*/
class CFxEntityManager
{
public:
	/// Returns a pointer to the manager.
	static CFxEntityManager* getInstance();
	/// Destroys the manager.
	static void destroyInstance();
	
	void init(TDataSetIndex baseRowIndex, TDataSetIndex size);
	void tickUpdate();
	
	/// Creates a fx entity or return an existing one with that name.
	CFxEntityPtr create(CAIPos const& pos, NLMISC::CSheetId const& sheet);
	/// Removes a fx entity from the manager.
	void destroy(CFxEntityPtr const& entity);
	/// Tells if an entity with the specified id exists in the manager.
	bool exists(NLMISC::CEntityId const& id);
	/// Select a set of entities depending of a request string.
	void select(std::string const& request, std::vector<NLMISC::CEntityId>& entities);
	/// Returns a fx entity with that id or 0 (NULL) if it doesn't exist.
	CFxEntityPtr get(NLMISC::CEntityId const& id);
	
	void registerEntity(CFxEntityPtr const& entity);
	void unregisterEntity(CFxEntityPtr const& entity);

	typedef std::set<CFxEntityPtr> TFxEntityContainer;
	typedef std::map<NLMISC::CEntityId, CFxEntityPtr> TEntityIdContainer;
	TFxEntityContainer const& getEntities() { return _Entities; }
	void dumpIndex()
	{
		FOREACH(itEntity, TEntityIdContainer, _IdIndex)
		{
			nldebug("Entity %s %s", itEntity->first.toString().c_str(), itEntity->second->id().toString().c_str());
		}
	}
	
protected:
	static CFxEntityManager* _Instance;
	
private:
	TFxEntityContainer	_Entities;
	TEntityIdContainer	_IdIndex;
};

#endif
