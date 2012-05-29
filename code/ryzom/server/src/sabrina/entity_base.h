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

#include "nel/misc/sheet_id.h"
#include "nel/misc/entity_id.h"
#include "game_share/base_types.h"

#ifndef SAB_ENTITY_BASE_H
#define SAB_ENTITY_BASE_H

class ISabrinaActor;

class CEntityBase
{
public:
	CEntityBase(): _Target(NULL), _EntityRowId(0) {}

	virtual	ISabrinaActor*		getSabrinaActor()	=0;

	NLMISC::CSheetId			getSheetId()		const { return NLMISC::CSheetId(); }
	const NLMISC::CEntityId		getId()				const { return NLMISC::CEntityId(); }

	TDataSetRow					getEntityRowId()	const { return TDataSetRow::createFromRawIndex(_EntityRowId); }
	void						setEntityRowId(uint32 i)  { _EntityRowId=i; }

	CEntityBase*				getTarget()			const { return _Target; }
	void						setTarget(CEntityBase* t) { _Target=t; }

private:
	CEntityBase*	_Target;
	uint32			_EntityRowId;
};

#endif