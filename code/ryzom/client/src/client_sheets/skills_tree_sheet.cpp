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
#include "skills_tree_sheet.h"
#include "nel/georges/u_form_elm.h"
#include "nel/misc/common.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;

// ***************************************************************************
void CSkillsTreeSheet::build(const UFormElm &item)
{
	const UFormElm *arraySkillElt = NULL;
	if( item.getNodeByName( &arraySkillElt, "SkillData" ) )
	{
		if( arraySkillElt )
		{
			uint NbSkills;
			nlverify( arraySkillElt->getArraySize( NbSkills ) );

			//nlassert( NbSkills == SKILLS::NUM_SKILLS );

			SkillsTree.resize( std::max(NbSkills, (uint) SKILLS::NUM_SKILLS));

			for( uint i = 0; i < NbSkills; ++i )
			{
				const UFormElm* SkillElt = NULL;
				if( ! ( arraySkillElt->getArrayNode( &SkillElt, i ) && SkillElt ) )
				{
					nlwarning("<CSkillsTreeSheet::build> can't get array node of SkillElt in sheet");
				}
				else
				{
					// Skill
					string SkillName;
					SkillElt->getValueByName( SkillName, "Skill" );
					SKILLS::ESkills skill = SKILLS::toSkill( SkillName );
					//nlassert( skill != SKILLS::unknown );
					if (skill == SKILLS::unknown)
					{
						continue;
					}
					SkillsTree[ skill ].Skill = skill;

					// Skill Code
					if( ! SkillElt->getValueByName( SkillsTree[ skill ].SkillCode, "SkillCode" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node SkillCode in sheet");
					}

					// Max skill value
					if( ! SkillElt->getValueByName( SkillsTree[ skill ].MaxSkillValue, "MaxSkillValue" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node MaxSkillValue in sheet");
					}

					// Type of stage
					if( ! SkillElt->getValueByName( SkillsTree[ skill ].StageType, "Type of Stage" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node 'Type of Stage' in sheet");
					}

					// ParentSkill
					if( ! SkillElt->getValueByName( SkillName, "ParentSkill" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node ParentSkills in sheet");
					}
					else
					{
						SkillsTree[ skill ].ParentSkill = SKILLS::toSkill( SkillName );
					}

					// ChildSkills
					const UFormElm *arrayChildSkillElt = NULL;
					if( SkillElt->getNodeByName( &arrayChildSkillElt, "ChildSkills" ) )
					{
						if( arrayChildSkillElt )
						{
							uint NbChildSkills;
							nlverify( arrayChildSkillElt->getArraySize( NbChildSkills ) );

							SkillsTree[ skill ].ChildSkills.resize( NbChildSkills );

							for( uint j = 0; j < NbChildSkills; ++j )
							{
								string childSkillName;
								arrayChildSkillElt->getArrayValue( childSkillName, j );
								SKILLS::ESkills childSkill = SKILLS::toSkill( childSkillName );
								//nlassert( childSkill != SKILLS::unknown );
								SkillsTree[ skill ].ChildSkills[ j ] = childSkill;
							}
						}
					}
				}
			}
		}
	}
}

