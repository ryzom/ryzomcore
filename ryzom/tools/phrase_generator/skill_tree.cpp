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

#include "skill_tree.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;


//-----------------------------------------------
// readGeorges for CStaticSkillsTree
//
//-----------------------------------------------
void CStaticSkillsTree::readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId )
{
	if( form )
	{
		UFormElm& root = form->getRootNode();
		
		UFormElm *arraySkillElt = NULL;
		if( root.getNodeByName( &arraySkillElt, "SkillData" ) )
		{
			if( arraySkillElt )
			{
				uint NbSkills;
				nlverify( arraySkillElt->getArraySize( NbSkills ) );

				nlassertex( NbSkills == SKILLS::NUM_SKILLS, ("(%u != %u) Please synchronise game_share/skill.* with leveldesign/game_element/xp_table/skills.skill_tree (use skill_extractor.exe)", NbSkills, SKILLS::NUM_SKILLS));

				SkillsTree.resize( NbSkills );
				
				for( uint i = 0; i < NbSkills; ++i )
				{
					UFormElm* SkillElt = NULL;
					if( ! ( arraySkillElt->getArrayNode( &SkillElt, i ) && SkillElt ) )
					{
						nlwarning("<CStaticSkillsTree::readGeorges> can't get array node of SkillElt in sheet %s", sheetId.toString().c_str() );
					}
					else
					{
						// Skill
						string SkillName;
						SkillElt->getValueByName( SkillName, "Skill" );
						SKILLS::ESkills skill = SKILLS::toSkill( SkillName );
						nlassert( skill != SKILLS::unknown );
						if (skill == SKILLS::unknown)
						{
							continue;
						}
						SkillsTree[ skill ].Skill = skill;

						if( ! SkillElt->getValueByName( SkillsTree[ skill ].SkillCode, "SkillCode" ) )
						{
							nlwarning("<CStaticSkillsTree::readGeorges> can't get node SkillCode in sheet %s", sheetId.toString().c_str() );
						}

						// Skill Code
						if( ! SkillElt->getValueByName( SkillsTree[ skill ].SkillCode, "SkillCode" ) )
						{
							nlwarning("<CStaticSkillsTree::readGeorges> can't get node SkillCode in sheet %s", sheetId.toString().c_str() );
						}

						// Max skill value
						if( ! SkillElt->getValueByName( SkillsTree[ skill ].MaxSkillValue, "MaxSkillValue" ) )
						{
							nlwarning("<CStaticSkillsTree::readGeorges> can't get node MaxSkillValue in sheet %s", sheetId.toString().c_str() );
						}

						// Type of stage
						if( ! SkillElt->getValueByName( SkillsTree[ skill ].StageType, "Type of Stage" ) )
						{
							nlwarning("<CStaticSkillsTree::readGeorges> can't get node 'Type of Stage' in sheet %s", sheetId.toString().c_str() );
						}

						// ParentSkill
						if( ! SkillElt->getValueByName( SkillName, "ParentSkill" ) )
						{
							nlwarning("<CStaticSkillsTree::readGeorges> can't get node ParentSkills in sheet %s", sheetId.toString().c_str() );
						}
						else
						{	
							SkillsTree[ skill ].ParentSkill = SKILLS::toSkill( SkillName );
						}

						// ChildSkills
						UFormElm *arrayChildSkillElt = NULL;
						if( SkillElt->getNodeByName( &arrayChildSkillElt, "ChildSkills" ) )
						{
							if( arrayChildSkillElt )
							{
								uint NbChildSkills;
								nlverify( arrayChildSkillElt->getArraySize( NbChildSkills ) );
								
								SkillsTree[ skill ].ChildSkills.resize( NbChildSkills );
								
								for( uint i = 0; i < NbChildSkills; ++i )
								{
									string childSkillName;
									arrayChildSkillElt->getArrayValue( childSkillName, i );
									SKILLS::ESkills childSkill = SKILLS::toSkill( childSkillName );
									nlassert( childSkill != SKILLS::unknown );
									if (skill == SKILLS::unknown)
									  {
										continue;
									  }
									SkillsTree[ skill ].ChildSkills[ i ] = childSkill;
								}
							}
						}
					}
				}
			}
		}
	}
}

