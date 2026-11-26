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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef RY_PLAYER_MORPH_ROLES_H
#define RY_PLAYER_MORPH_ROLES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"
#include "nel/misc/sheet_id.h"

/// a creature a GM prepared for an event, with its own values
class CPlayerMorphRole
{
public:
	CPlayerMorphRole() : Attackable(false), Hp(0), NextAtPercent(0) {}

	void serial(NLMISC::IStream &f);

public:
	NLMISC::CSheetId			Creature;
	bool						Attackable;
	/// 0 for the hit points of the creature
	sint32						Hp;
	/// the creature actions the role may use, empty for all of them
	std::vector<NLMISC::CSheetId> Actions;
	/// unknown for a role that drops nothing
	NLMISC::CSheetId			LootTable;
	/// role the morph turns into when its hit points fall to NextAtPercent, empty for none
	std::string					NextRole;
	uint8						NextAtPercent;
};

class CPlayerMorphRoles : public NLMISC::CSingleton<CPlayerMorphRoles>
{
public:
	typedef std::map<std::string, CPlayerMorphRole> TRoleMap;

	/// get a role, return NULL if the role does not exist
	const CPlayerMorphRole *getRole(const std::string &name) const;

	/// create or replace a role and save the roles
	void setRole(const std::string &name, const CPlayerMorphRole &role);

	/// remove a role and save the roles
	void removeRole(const std::string &name);

	const TRoleMap &getRoles() const { return _Roles; }

	void serial(NLMISC::IStream &f);

	/// load the roles saved by the backup service
	void loadFromFile();

private:
	void saveToFile();

private:
	TRoleMap	_Roles;
};

#endif // RY_PLAYER_MORPH_ROLES_H

/* End of player_morph_roles.h */
