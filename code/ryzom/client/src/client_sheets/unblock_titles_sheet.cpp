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
#include "unblock_titles_sheet.h"
#include "nel/georges/u_form_elm.h"
#include "nel/misc/common.h"
#include "game_share/fame.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;

// Tokenize a string with space as separator
static void tokenize (const string &zeString, vector<string> &tokens)
{
	tokens.clear();
	string sTmp = zeString;
	while (!sTmp.empty())
	{
		while (((sTmp[0] == ' ')||(sTmp[0] == ':')) && !sTmp.empty())
			sTmp = sTmp.substr(1,sTmp.size());

		if (sTmp.empty()) break;

		string::size_type nextseppos1 = sTmp.find(' ');
		string::size_type nextseppos2 = sTmp.find(':');

		if ((nextseppos1 == string::npos) && (nextseppos2 == string::npos))
		{
			tokens.push_back(sTmp);
			sTmp = "";
		}
		else
		{
			if ((nextseppos1 != string::npos) && (nextseppos2 != string::npos))
				nextseppos1 = min(nextseppos1, nextseppos2);
			else if (nextseppos2 != string::npos)
				nextseppos1 = nextseppos2;

			tokens.push_back (sTmp.substr(0, nextseppos1));
			sTmp = sTmp.substr(nextseppos1+1, sTmp.size());
		}
	}
}

// ***************************************************************************
void CUnblockTitlesSheet::build(const UFormElm &item)
{
	TitlesUnblock.resize (CHARACTER_TITLE::NB_CHARACTER_TITLE);

	const UFormElm *arrayTUElt = NULL;
	if( item.getNodeByName( &arrayTUElt, "Titles" ) )
	{
		if( arrayTUElt )
		{
			sint32 tmpLevel, tmpQuality;
			uint NbTitlesUnblock;
			nlverify( arrayTUElt->getArraySize( NbTitlesUnblock ) );

			for (uint i = 0; i < NbTitlesUnblock; ++i)
			{
				const UFormElm* TUElt = NULL;
				if( ! ( arrayTUElt->getArrayNode( &TUElt, i ) && TUElt ) )
				{
					nlwarning("<CSkillsTreeSheet::build> can't get array node of TitleUnblock in sheet");
				}
				else
				{
					// Skill
					string TitleName;
					TUElt->getValueByName( TitleName, "Title" );
					CHARACTER_TITLE::ECharacterTitle title = CHARACTER_TITLE::toCharacterTitle ( TitleName );
					//nlassert( skill != SKILLS::unknown );
					if (title >= CHARACTER_TITLE::NB_CHARACTER_TITLE)
						continue;

					bool bReserved = false;
					if( ! TUElt->getValueByName( bReserved, "Reserved" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node Reserved in sheet");
					}

					string strSkillsNeeded;
					if( ! TUElt->getValueByName( strSkillsNeeded, "SkillsNeeded" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node SkillsNeeded in sheet");
					}

					string strBricksNeeded;
					if( ! TUElt->getValueByName( strBricksNeeded, "BricksNeeded" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node SkillsNeeded in sheet");
					}

					string strMinFames;
					if( ! TUElt->getValueByName( strMinFames, "MinFames" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node MinFames in sheet");
					}

					string strMaxFames;
					if( ! TUElt->getValueByName( strMaxFames, "MaxFames" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node MaxFames in sheet");
					}

					// cf TPVPClan enum (pvp_clan.h)
					if( ! TUElt->getValueByName( TitlesUnblock[title].CivNeeded, "CivNeeded" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node CivNeeded in sheet");
					}

					// cf TPVPClan enum (pvp_clan.h)
					if( ! TUElt->getValueByName( TitlesUnblock[title].CultNeeded, "CultNeeded" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node CultNeeded in sheet");
					}

					if( ! TUElt->getValueByName( TitlesUnblock[title].CharOldness, "CharOldness" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node CharOldness in sheet");
					}

					if( ! TUElt->getValueByName( TitlesUnblock[title].CharPlayedTime, "CharPlayedTime" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node CharPlayedTime in sheet");
					}

					if( ! TUElt->getValueByName( TitlesUnblock[title].AccountOldness, "AccountOldness" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node AccountOldness in sheet");
					}

					if( ! TUElt->getValueByName( TitlesUnblock[title].AuthorRating, "AuthorRating" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node AuthorRating in sheet");
					}

					if( ! TUElt->getValueByName( TitlesUnblock[title].AMRating, "AMRating" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node AMRating in sheet");
					}

					if( ! TUElt->getValueByName( TitlesUnblock[title].OrganizerRating, "OrganizerRating" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node OrganizerRating in sheet");
					}

					string strItemsNeeded;
					if( ! TUElt->getValueByName( strItemsNeeded, "ItemsNeeded" ) )
					{
						nlwarning("<CSkillsTreeSheet::build> can't get node ItemsNeeded in sheet");
					}

					uint j;

					TitlesUnblock[title].Reserved = bReserved;

					if( !strSkillsNeeded.empty() )
					{
						uint16 skillsNeededSz = (uint16)TitlesUnblock[title].SkillsNeeded.size();
						TitlesUnblock[title].SkillsNeeded.resize(skillsNeededSz+1);
						TitlesUnblock[title].SkillsLevelNeeded.resize(skillsNeededSz+1);

						vector<string> vSkill;
						tokenize(strSkillsNeeded, vSkill);
						for (j = 0; j < vSkill.size()/2; ++j)
						{
							TitlesUnblock[title].SkillsNeeded[skillsNeededSz].push_back(vSkill[j*2]);
							fromString(vSkill[j*2+1], tmpLevel);
							TitlesUnblock[title].SkillsLevelNeeded[skillsNeededSz].push_back(tmpLevel);
						}
					}

					if( !strBricksNeeded.empty() )
					{
						vector<string> vBrick;
						tokenize(strBricksNeeded, vBrick);
						for (j = 0; j < vBrick.size(); ++j)
						{
							TitlesUnblock[title].BricksNeeded.push_back(CSheetId(vBrick[j]+".sbrick"));
						}
					}

					if( !strMinFames.empty() )
					{
						vector<string> vFame;
						tokenize(strMinFames, vFame);
						for (j = 0; j < vFame.size()/2; ++j)
						{
							PVP_CLAN::TPVPClan clan = PVP_CLAN::fromString(vFame[j*2]);
							if( clan != PVP_CLAN::Unknown )
							{
								uint32 factionIndex = PVP_CLAN::getFactionIndex( clan );
								nlassert(factionIndex != CStaticFames::INVALID_FACTION_INDEX);
								TitlesUnblock[title].MinFames.push_back(factionIndex);
								fromString(vFame[j*2+1], tmpLevel);
								TitlesUnblock[title].MinFameLevels.push_back(tmpLevel);
							}
						}
					}
					if( !strMaxFames.empty() )
					{
						vector<string> vFame;
						tokenize(strMaxFames, vFame);
						for (j = 0; j < vFame.size()/2; ++j)
						{
							PVP_CLAN::TPVPClan clan = PVP_CLAN::fromString(vFame[j*2]);
							if( clan != PVP_CLAN::Unknown )
							{
								uint32 factionIndex = PVP_CLAN::getFactionIndex( clan );
								nlassert(factionIndex != CStaticFames::INVALID_FACTION_INDEX);
								TitlesUnblock[title].MaxFames.push_back(factionIndex);
								fromString(vFame[j*2+1], tmpLevel);
								TitlesUnblock[title].MaxFameLevels.push_back(tmpLevel);
							}
						}
					}

					if( !strItemsNeeded.empty() )
					{
						uint16 itemsNeededSz = (uint16)TitlesUnblock[title].ItemsNeeded.size();
						TitlesUnblock[title].ItemsNeeded.resize(itemsNeededSz+1);
						TitlesUnblock[title].ItemsQualityNeeded.resize(itemsNeededSz+1);

						vector<string> vItem;
						tokenize(strItemsNeeded, vItem);
						for (j = 0; j < vItem.size()/2; ++j)
						{
							TitlesUnblock[title].ItemsNeeded[itemsNeededSz].push_back(CSheetId(vItem[j*2]+".sitem"));
							fromString(vItem[j*2+1], tmpQuality);
							TitlesUnblock[title].ItemsQualityNeeded[itemsNeededSz].push_back(tmpQuality);
						}
					}
				}
			}
		}
	}
}

