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

#ifndef RYAI_NAMED_ENTITY_H
#define RYAI_NAMED_ENTITY_H

#include <string>
#include "nel/misc/entity_id.h"

class CGroupNpc;

class CNamedEntity
{
public:
	CNamedEntity(std::string const& name);
	std::string const& name() const;
	NLMISC::CEntityId const& id() const;
	std::string const& getState();
	std::string const& getParam1();
	std::string const& getParam2();
	std::string get(std::string const& prop);
	void set(std::string const& prop, std::string const& value, bool reportChange=false);
	void addListenerGroup(std::string const& prop, CGroupNpc* persGrp);
	void delListenerGroup(std::string const& prop, CGroupNpc* persGrp);
private:
	void namedEntityCb(std::string const& prop);
	std::string _name;
	NLMISC::CEntityId _id;
	std::string _state;
	std::string _param1;
	std::string _param2;
	typedef std::multimap<std::string, CGroupNpc*> TListenerGroupList;
	TListenerGroupList _listenerGroups;
};

#endif // RYAI_NAMED_ENTITY_H
