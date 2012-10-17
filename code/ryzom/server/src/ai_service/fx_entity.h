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

#ifndef RYAI_FX_ENTITY_H
#define RYAI_FX_ENTITY_H

#include <string>
#include "nel/misc/entity_id.h"
#include "ai_pos.h"

class CGroupNpc;

class CFxEntity
: public NLMISC::CRefCount
{
public:
	CFxEntity(CAIPos const& pos, NLMISC::CSheetId const& sheet);
	virtual ~CFxEntity();
	NLMISC::CEntityId const& id() const;
	bool spawn();
	void despawn();
	bool update() { }
	void set(std::string const& prop, std::string const& value, bool reportChange=false) { }
	std::string get(std::string const& prop);
	std::string getIndexString() const { return _EntityId.toString(); }
	virtual std::string getOneLineInfoString() const;
	virtual std::vector<std::string> getMultiLineInfoString() const;
	
private:
	NLMISC::CEntityId	_EntityId;
	TDataSetRow			_DataSetRow;
	CAIPos				_Pos;
	NLMISC::CSheetId	_Sheet;
};
typedef NLMISC::CSmartPtr<CFxEntity> CFxEntityPtr;
typedef NLMISC::CSmartPtr<CFxEntity const> CFxEntityCPtr;

#endif
