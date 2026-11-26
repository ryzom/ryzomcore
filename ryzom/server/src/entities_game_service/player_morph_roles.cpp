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

#include "stdpch.h"
#include "player_morph_roles.h"

#include "nel/misc/mem_stream.h"
#include "game_share/utils.h"

using namespace NLMISC;
using namespace std;

static const char *MorphRolesFileName = "morph_roles.bin";

void CPlayerMorphRole::serial(NLMISC::IStream &f)
{
	// sheets are kept by name, their ids can change when the sheets are rebuilt
	Creature.serialString(f, "creature");
	f.serial(Attackable);
	f.serial(Hp);
	uint32 nbActions = (uint32)Actions.size();
	f.serial(nbActions);
	if (f.isReading())
		Actions.resize(nbActions);
	for (uint i = 0; i < nbActions; ++i)
		Actions[i].serialString(f, "aiaction");
	// serialString does not take the unknown sheet of a role without loot
	string lootTable = LootTable == CSheetId::Unknown ? string() : LootTable.toString();
	f.serial(lootTable);
	if (f.isReading())
		LootTable = lootTable.empty() ? CSheetId::Unknown : CSheetId(lootTable);
	f.serial(NextRole);
	f.serial(NextAtPercent);
}

const CPlayerMorphRole *CPlayerMorphRoles::getRole(const std::string &name) const
{
	TRoleMap::const_iterator it = _Roles.find(name);
	return it != _Roles.end() ? &(*it).second : NULL;
}

void CPlayerMorphRoles::setRole(const std::string &name, const CPlayerMorphRole &role)
{
	_Roles[name] = role;
	saveToFile();
}

void CPlayerMorphRoles::removeRole(const std::string &name)
{
	_Roles.erase(name);
	saveToFile();
}

void CPlayerMorphRoles::serial(NLMISC::IStream &f)
{
	f.serialVersion(1);
	f.serialCont(_Roles);
	if (!f.isReading())
		return;
	// a role whose creature or actions were removed from the sheets is dropped: serialString can't write
	// them back, and a role without its action subset would get all the creature actions
	for (TRoleMap::iterator it = _Roles.begin(); it != _Roles.end();)
	{
		const CPlayerMorphRole &role = (*it).second;
		if (role.Creature == CSheetId::Unknown || find(role.Actions.begin(), role.Actions.end(), CSheetId::Unknown) != role.Actions.end())
		{
			nlwarning("<CPlayerMorphRoles::serial> role '%s' uses a sheet that no longer exists, removed", (*it).first.c_str());
			_Roles.erase(it++);
		}
		else
			++it;
	}
}

void CPlayerMorphRoles::saveToFile()
{
	CMemStream stream;
	serial(stream);
	CBackupMsgSaveFile msg(MorphRolesFileName, CBackupMsgSaveFile::SaveFile, Bsi);
	msg.DataMsg.serialBuffer((uint8*)stream.buffer(), stream.length());
	Bsi.sendFile(msg);
}

struct CPlayerMorphRolesFileLoadCallback : public IBackupFileReceiveCallback
{
	virtual void callback(const CFileDescription &fileDescription, NLMISC::IStream &dataStream)
	{
		DROP_IF(fileDescription.FileName.empty(), "<CPlayerMorphRoles::loadFromFile> file not found: " << MorphRolesFileName, return);

		try
		{
			CPlayerMorphRoles::getInstance().serial(dataStream);
		}
		catch (const Exception &e)
		{
			STOP("<CPlayerMorphRoles::loadFromFile> cannot parse file: " << MorphRolesFileName << ": " << e.what());
		}
	}
};

void CPlayerMorphRoles::loadFromFile()
{
	// the backup service may run on another machine, it reports a missing file to the callback
	Bsi.syncLoadFile(MorphRolesFileName, new CPlayerMorphRolesFileLoadCallback);
}
