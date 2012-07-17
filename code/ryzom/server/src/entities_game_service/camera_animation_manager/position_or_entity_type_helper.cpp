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
// 

#include "camera_animation_manager/position_or_entity_type_helper.h"

const TPositionOrEntity CPositionOrEntityHelper::Invalid = TPositionOrEntity();

TPositionOrEntity CPositionOrEntityHelper::fromString(const std::string& s)
{
	std::string str = s;
	CMissionParser::removeBlanks(str);

	std::vector<std::string> res;
	NLMISC::splitString(str, ";", res);
	// If we don't have 3 components, it's an entityid
	if (res.size() != 3)
	{
		std::vector<TAIAlias> res;
		CAIAliasTranslator::getInstance()->getNPCAliasesFromName(str, res);
		if (res.size() != 1)
		{
			nlerror("TPositionOrentityHelper : no alias for entity name %s", str);
			return TPositionOrEntity();
		}
		TAIAlias alias = res[0];
		if (alias == CAIAliasTranslator::Invalid)
		{
			nlerror("TPositionOrentityHelper : invalid alias for entity name %s", str);
			return TPositionOrEntity();
		}
		NLMISC::CEntityId eid = CAIAliasTranslator::getInstance()->getEntityId(alias);
		if (eid == NLMISC::CEntityId::Unknown)
		{
			nlerror("TPositionOrentityHelper : invalid entity id from alias %d", alias);
			return TPositionOrEntity();
		}

		return TPositionOrEntity(eid);
	}
	else
	{
		// It's a position
		std::string xStr = res[0];
		std::string yStr = res[1];
		std::string zStr = res[2];

		CMissionParser::removeBlanks(xStr);
		CMissionParser::removeBlanks(yStr);
		CMissionParser::removeBlanks(zStr);

		float x = 0.f;
		float y = 0.f;
		float z = 0.f;

		if (!NLMISC::fromString(xStr, x))
		{
			nlerror("TPositionOrentityHelper : invalid x component from string %s", xStr);
			return TPositionOrEntity();
		}
		if (!NLMISC::fromString(yStr, y))
		{
			nlerror("TPositionOrentityHelper : invalid y component from string %s", yStr);
			return TPositionOrEntity();
		}
		if (!NLMISC::fromString(yStr, x))
		{
			nlerror("TPositionOrentityHelper : invalid z component from string %s", zStr);
			return TPositionOrEntity();
		}

		return TPositionOrEntity(NLMISC::CVector(x, y, z));
	}

	return TPositionOrEntity();
}