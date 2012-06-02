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


#ifndef CL_SKILLS_TREE_SHEET_H
#define CL_SKILLS_TREE_SHEET_H

// Application
#include "entity_sheet.h"
// Game Share
#include "game_share/skills.h"
#include "game_share/character_title.h"

class CSkillsTreeSheet : public CEntitySheet
{
public :
	struct SSkillData
	{
		SKILLS::ESkills					Skill;
		std::string						SkillCode;
		uint16							MaxSkillValue;
		uint16							StageType;
		SKILLS::ESkills					ParentSkill;
		std::vector<SKILLS::ESkills>	ChildSkills;

		// ---------------------------------------------

		SSkillData()
		{
			ParentSkill = Skill = SKILLS::unknown;
			MaxSkillValue = StageType = 0;
		}

		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serialEnum (Skill);
			f.serial (SkillCode);
			f.serial (MaxSkillValue);
			f.serial (StageType);
			f.serialEnum (ParentSkill);

			if (f.isReading())
			{
				uint16 size;
				f.serial (size);
				ChildSkills.resize (size);
				for (uint i = 0; i < size; ++i)
				{
					f.serialEnum (ChildSkills[i]);
				}
			}
			else
			{
				uint16 size = uint16(ChildSkills.size());
				f.serial (size);
				for (std::vector<SKILLS::ESkills>::iterator it = ChildSkills.begin(); it != ChildSkills.end(); ++it)
				{
					f.serialEnum ((*it));
				}
			}
		}
	};


public:

	CSkillsTreeSheet()
	{
		Type = SKILLS_TREE;
	}
	/// destructor
	virtual ~CSkillsTreeSheet() {}

	virtual void build(const NLGEORGES::UFormElm &item);

	/// serialize
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialCont( SkillsTree );
	}

public:

	std::vector< SSkillData > SkillsTree;
};

#endif // CL_SKILLS_TREE_SHEET_H
