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



#ifndef CL_SKILLS_BUILD_H
#define CL_SKILLS_BUILD_H


/////////////
// CLASSES //
/////////////
namespace NLGEORGES
{
	class UFormElm;
}


/** Summary of a skill, contains just the ID of a skill and its value
  */
struct CSkillSummary
{
	CSkillSummary() {}
	CSkillSummary(uint16 id, uint16 value) : ID(id), Value(value) {}
	uint16 ID; /* see enum ESkills in skills.h */
	uint16 Value;
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial(ID, Value);
	}
};


/** Helper function to load a set of skills summaries from a georges sheet
  * This can be used with sheets such as .player or .starting_role
  * \param item The georges item from which to load the skills.
  * \param prefix The prefix to get infos in georges sheet. It must be terminated by a dot (Ex : "Basic Player.").
  * \param dest A vector that will be filled with skills summaries.
  */
void loadSkillsSummaryFromSheet(const NLGEORGES::UFormElm &item, const std::string &prefix, std::vector<CSkillSummary> &dest);

#endif

