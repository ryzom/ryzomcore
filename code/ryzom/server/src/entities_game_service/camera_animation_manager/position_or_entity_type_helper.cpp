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
#include "game_share/mirror.h"
#include "game_share/mirrored_data_set.h"
#include "game_share/base_types.h"
#include "mission_manager/ai_alias_translator.h"
#include "mission_manager/mission_parser.h"
#include "egs_mirror.h"

#define POS_OR_ENTITY_PREVIOUS_POS_VALUE "_PreviousPos_"
#define POS_OR_ENTITY_RETURN_POS_VALUE "_ReturnPos_"

const CPositionOrEntityHelper CPositionOrEntityHelper::Invalid = CPositionOrEntityHelper();

CPositionOrEntityHelper CPositionOrEntityHelper::fromString(const std::string& s)
{
	// If the string is empty it's a previous pos
	if (s.empty())
	{
		return CPositionOrEntityHelper(CPositionOrEntity::EPreviousPos);
	}

	std::string str = s;
	CMissionParser::removeBlanks(str);

	// We already check if it's a special position or entity object
	if (NLMISC::toLower(str) == POS_OR_ENTITY_PREVIOUS_POS_VALUE)
	{
		return CPositionOrEntityHelper(CPositionOrEntity::EPreviousPos);
	}
	else if (NLMISC::toLower(str) == POS_OR_ENTITY_RETURN_POS_VALUE)
	{
		return CPositionOrEntityHelper(CPositionOrEntity::EReturnPos);
	}

	std::vector<std::string> resS;
	NLMISC::splitString(str, ";", resS);
	// If we don't have 3 components, it's an entity
	if (resS.size() != 3)
	{
		std::vector<TAIAlias> res;
		CAIAliasTranslator::getInstance()->getNPCAliasesFromName(str, res);
		if (res.size() != 1)
		{
			nlerror("TPositionOrentityHelper : no alias for entity name %s", str.c_str());
			return CPositionOrEntityHelper();
		}
		TAIAlias alias = res[0];
		if (alias == CAIAliasTranslator::Invalid)
		{
			nlerror("TPositionOrentityHelper : invalid alias for entity name %s", str.c_str());
			return CPositionOrEntityHelper();
		}
		NLMISC::CEntityId eid = CAIAliasTranslator::getInstance()->getEntityId(alias);
		if (eid == NLMISC::CEntityId::Unknown)
		{
			nlerror("TPositionOrentityHelper : invalid entity id from alias %d", alias);
			return CPositionOrEntityHelper();
		}
		TDataSetIndex compressedId = TheDataset.getDataSetRow(eid).getCompressedIndex();

		return CPositionOrEntityHelper(compressedId);
	}
	else
	{
		// It's a position
		std::string xStr = resS[0];
		std::string yStr = resS[1];
		std::string zStr = resS[2];

		CMissionParser::removeBlanks(xStr);
		CMissionParser::removeBlanks(yStr);
		CMissionParser::removeBlanks(zStr);

		float x = 0.f;
		float y = 0.f;
		float z = 0.f;

		if (!NLMISC::fromString(xStr, x))
		{
			nlerror("TPositionOrentityHelper : invalid x component from string %s", xStr.c_str());
			return CPositionOrEntityHelper();
		}
		if (!NLMISC::fromString(yStr, y))
		{
			nlerror("TPositionOrentityHelper : invalid y component from string %s", yStr.c_str());
			return CPositionOrEntityHelper();
		}
		if (!NLMISC::fromString(yStr, x))
		{
			nlerror("TPositionOrentityHelper : invalid z component from string %s", zStr.c_str());
			return CPositionOrEntityHelper();
		}

		return CPositionOrEntityHelper(NLMISC::CVector(x, y, z));
	}

	return CPositionOrEntityHelper();
}

NLMISC::CVector CPositionOrEntityHelper::getDiffPos(const NLMISC::CVector& targetPos) const
{
	return Position;
}

NLMISC::CVector CPositionOrEntityHelper::setPositionFromDiffPos(const NLMISC::CVector& diffPos)
{
	return Position;
}