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

#ifndef SKILL_TREE_H
#define SKILL_TREE_H

// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/sheet_id.h"

// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
//#include "nel/georges/u_form_dfn.h"
//#include "nel/georges/u_form_loader.h"
//#include "nel/georges/u_type.h"

#include "game_share/skills.h"

#include <vector>
#include <string>


/**
 * CStaticSkillsTree
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CStaticSkillsTree
{
public :
	struct SSkillData
	{
		SKILLS::ESkills Skill;
		std::string	SkillCode;
		uint16		MaxSkillValue;
		uint16		StageType;
		SKILLS::ESkills	ParentSkill;
		std::vector<SKILLS::ESkills> ChildSkills;

		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serialEnum( Skill );
			f.serial( SkillCode );
			f.serial( MaxSkillValue );
			f.serial( StageType );
			f.serialEnum( ParentSkill );
			
			if( f.isReading() )
			{
				uint16 size;
				f.serial( size );
				ChildSkills.resize( size );
				for( uint i = 0; i < size; ++i )
				{
					f.serialEnum( ChildSkills[ i ] );
				}
			}
			else
			{
				uint16 size = ChildSkills.size();
				f.serial( size );
				for( std::vector<SKILLS::ESkills>::iterator it = ChildSkills.begin(); it != ChildSkills.end(); ++it )
				{
					f.serialEnum( (*it) );
				}
			}
		}
	};

	/// read sheet
	virtual void readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );

	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 1 + ( SKILLS::NUM_SKILLS << 16 ); }

	/// serialize
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) { f.serialCont( SkillsTree ); }

	/// destructor
	virtual ~CStaticSkillsTree() {}

	/// called when the sheet is removed
	void removed() {}

	std::vector< SSkillData > SkillsTree;
};

#endif // SKILL_TREE_H

/* End of skill_tree.h */