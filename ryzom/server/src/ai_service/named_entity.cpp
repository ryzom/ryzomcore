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

#include "named_entity.h"
#include "ai_instance.h"
#include "game_share/ryzom_entity_id.h"
#include "ai_grp_npc.h"

CNamedEntity::CNamedEntity(std::string const& name)
: _name(name)
, _id(NLMISC::CEntityId::getNewEntityId(RYZOMID::ais_named_entity))
{
}

std::string const& CNamedEntity::name() const
{
	return _name;
}

NLMISC::CEntityId const& CNamedEntity::id() const
{
	return _id;
}

std::string const& CNamedEntity::getState()
{
	return _state;
}

std::string const& CNamedEntity::getParam1()
{
	return _param1;
}

std::string const& CNamedEntity::getParam2()
{
	return _param2;
}

void CNamedEntity::set(std::string const& prop, std::string const& value, bool reportChange)
{
	if (prop=="state")
		_state = value;
	else if (prop=="param1")
		_param1 = value;
	else if (prop=="param2")
		_param2 = value;
	else
		nlwarning("Trying to set a bad property ('%s') on named entity '%s'", prop.c_str(), _name.c_str());
	if (reportChange)
		namedEntityCb(prop);
}

std::string CNamedEntity::get(std::string const& prop)
{
	if (prop=="state")
		return _state;
	else if (prop=="param1")
		return _param1;
	else if (prop=="param2")
		return _param2;
	else
		nlwarning("Trying to get a bad property ('%s') on named entity '%s'", prop.c_str(), _name.c_str());
	return std::string();
}

void CNamedEntity::addListenerGroup(std::string const& prop, CGroupNpc* persGrp)
{
	_listenerGroups.insert(std::make_pair(prop, persGrp));
}

void CNamedEntity::delListenerGroup(std::string const& prop, CGroupNpc* persGrp)
{
	TListenerGroupList::iterator it = _listenerGroups.find(prop);
	if (it!=_listenerGroups.end())
	{
		_listenerGroups.erase(it);
	}
}

void CNamedEntity::namedEntityCb(std::string const& prop)
{
	TListenerGroupList::const_iterator first, last, grp;
	first = _listenerGroups.lower_bound(prop);
	last = _listenerGroups.upper_bound(prop);
	std::set<CGroupNpc*> groups;
	for (grp=first; grp!=last; ++grp)
	{
		groups.insert(grp->second);
	}
	std::set<CGroupNpc*>::iterator sfirst=groups.begin(), slast=groups.end(), sgrp;
	for (sgrp=sfirst; sgrp!=slast; ++sgrp)
	{
		(*sgrp)->namedEntityListenerCb(_name, prop);
	}
}

