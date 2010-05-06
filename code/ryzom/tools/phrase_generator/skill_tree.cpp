
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

