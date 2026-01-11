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
#include "skills_build.h"
#include "skills.h"
#include "nel/georges/u_form_elm.h"

void loadSkillsSummaryFromSheet(const NLGEORGES::UFormElm &item,const std::string &prefix,std::vector<CSkillSummary> &dest)
{
	dest.clear();
	for(uint k = 0; k < SKILLS::NUM_SKILLS; ++k)
	{
		uint16 skillValue;
		std::string skillName = prefix;
		const std::string &rTmp = SKILLS::getSkillCategoryName(k);
		skillName += rTmp;
		skillName += ".";
		skillName += SKILLS::toString(k);
		if(item.getValueByName(skillValue, skillName.c_str()))
		{
			if (skillValue != 0)
			{
				dest.push_back(CSkillSummary(k, skillValue));
			}
		}
	}

}
