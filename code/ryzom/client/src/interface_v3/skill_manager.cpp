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
#include "skill_manager.h"
#include "interface_manager.h"
#include "game_share/skills.h"
#include "nel/misc/xml_auto_ptr.h"
#include "game_share/character_title.h"
#include "game_share/fame.h"
#include "../sheet_manager.h"
#include "nel/misc/cdb_leaf.h"
#include "nel/gui/action_handler.h"
#include "sbrick_manager.h"
#include "nel/gui/dbgroup_combo_box.h"
#include "nel/gui/view_bitmap.h"
#include "../net_manager.h"
#include "sbrick_manager.h"
#include "../user_entity.h"
#include "../npc_icon.h"

using namespace SKILLS;
using namespace std;
using namespace NLMISC;
using namespace STRING_MANAGER;

sint FAME_MIN_DBVALUE  = -100;
sint FAME_MAX_DBVALUE  = 100;

CSkillManager* CSkillManager::_Instance = NULL;

extern CUserEntity	*UserEntity;

// ***************************************************************************
// CSkillManager
// ***************************************************************************


// ***************************************************************************
CSkillManager::CSkillManager()
{
	_UnblockTitle = NULL;
	_Tree= NULL;

	for(uint i=0;i<SKILLS::NUM_SKILLS;i++)
	{
		_SkillValues[i]= NULL;
		_SkillBaseValues[i]= NULL;
		_MaxChildBaseSkillValue[i] = 0;
		_CacheSkillValues[i]= 0;
		_CacheSkillBaseValues[i]= 0;
	}

	_TrackSkillChange= NULL;
}

// ***************************************************************************
CSkillManager::~CSkillManager()
{

}

// ***************************************************************************
void CSkillManager::initInGame()
{
	const CSheetManager::TEntitySheetMap &mSheets = SheetMngr.getSheets();
	CSheetManager::TEntitySheetMap::const_iterator itSheet = mSheets.begin();
	while (itSheet != mSheets.end())
	{
		CEntitySheet *pES = itSheet->second.EntitySheet;
		CSkillsTreeSheet *pSTS = dynamic_cast<CSkillsTreeSheet*>(pES);
		if (pSTS != NULL)
		{
			_Tree = pSTS;
		}
		CUnblockTitlesSheet *pUTS = dynamic_cast<CUnblockTitlesSheet*>(pES);
		if (pUTS != NULL)
		{
			_UnblockTitle = pUTS;
		}
		itSheet++;
	}
	// must exist, else random access later....
	nlassert(_Tree);
	nlassert(_UnblockTitle);

	// **** Data error management
	// For each skills
	uint	i;
	for (i = 0; i < SKILLS::NUM_SKILLS; ++i)
	{
		vector<SKILLS::ESkills>		&children= _Tree->SkillsTree[i].ChildSkills;
		for (sint32 j = 0; j < (sint32)children.size(); ++j)
		{
			if (children[j] >= SKILLS::NUM_SKILLS)
			{
				children.erase(children.begin()+j);
				j--;
			}
		}
	}

	// **** Min Skill Value mgt, also update max child skill value
	for(i=0;i<NUM_SKILLS;++i)
	{
		_MinSkillValue[i]= getMaxSkillValue(getParent(SKILLS::ESkills(i)));
	}

	// **** CHARACTER TITLE management
	nlassert(_UnblockTitle->TitlesUnblock.size() == CHARACTER_TITLE::NB_CHARACTER_TITLE);
	_TitlesUnblocked.resize(CHARACTER_TITLE::NB_CHARACTER_TITLE);
	for (i = 0; i < CHARACTER_TITLE::NB_CHARACTER_TITLE; ++i)
	{
		_TitlesUnblocked[i].Unblocked = false;
		_TitlesUnblocked[i].UnblockedSkillLists.resize (_UnblockTitle->TitlesUnblock[i].SkillsNeeded.size(), false);
		_TitlesUnblocked[i].UnblockedBricks.resize (_UnblockTitle->TitlesUnblock[i].BricksNeeded.size(), false);
		_TitlesUnblocked[i].UnblockedMinFames.resize (_UnblockTitle->TitlesUnblock[i].MinFames.size(), false);
		_TitlesUnblocked[i].UnblockedMaxFames.resize (_UnblockTitle->TitlesUnblock[i].MaxFames.size(), false);
		_TitlesUnblocked[i].UnblockedCiv = _UnblockTitle->TitlesUnblock[i].CivNeeded.empty();
		_TitlesUnblocked[i].UnblockedCult = _UnblockTitle->TitlesUnblock[i].CultNeeded.empty();
		_TitlesUnblocked[i].UnblockedCharOldness = _UnblockTitle->TitlesUnblock[i].CharOldness.empty();
		_TitlesUnblocked[i].UnblockedCharPlayedTime = _UnblockTitle->TitlesUnblock[i].CharPlayedTime.empty();
		_TitlesUnblocked[i].UnblockedAccountOldness = _UnblockTitle->TitlesUnblock[i].AccountOldness.empty();
		_TitlesUnblocked[i].UnblockedAuthorRating = (_UnblockTitle->TitlesUnblock[i].AuthorRating == 0);
		_TitlesUnblocked[i].UnblockedAMRating = (_UnblockTitle->TitlesUnblock[i].AMRating == 0);
		_TitlesUnblocked[i].UnblockedOrganizerRating = (_UnblockTitle->TitlesUnblock[i].OrganizerRating == 0);
		_TitlesUnblocked[i].UnblockedItemLists.resize (_UnblockTitle->TitlesUnblock[i].ItemsNeeded.size(), false);
	}


	// **** Player State management
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	// get now the nodes on Skill values
	for(i=0;i<SKILLS::NUM_SKILLS;i++)
	{
		_SkillValues[i]= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:CHARACTER_INFO:SKILLS:%d:SKILL", i), false);
		_SkillBaseValues[i]= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:CHARACTER_INFO:SKILLS:%d:BaseSKILL", i), false);
	}

	// compute max child values
	computeMaxChildValues(); // must be called after setting all _SkillBaseValues

	// Get a node used to inform interface that a skill has changed
	_TrackSkillChange= NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:TRACK_SKILL_CHANGE", true);
	// Add a branch observer on skill value change
	NLGUI::CDBManager::getInstance()->addBranchObserver( "SERVER:CHARACTER_INFO:SKILLS", &_SkillChangeObs );

}


// ***************************************************************************
void CSkillManager::uninitInGame()
{
	_UnblockTitle = NULL;
	_Tree= NULL;

	uint i;
	for(i=0;i<SKILLS::NUM_SKILLS;i++)
	{
		_SkillValues[i]= NULL;
		_SkillBaseValues[i]= NULL;
		_MaxChildBaseSkillValue[i] = 0;
		_CacheSkillValues[i]= 0;
		_CacheSkillBaseValues[i]= 0;
	}

	_TrackSkillChange= NULL;

	contReset(_TitlesUnblocked);

}

// ***************************************************************************
bool CSkillManager::isUnknown (SKILLS::ESkills eSkill)
{
	nlassert(_Tree);
	if ((eSkill < 0) || (eSkill >= SKILLS::NUM_SKILLS))
		return true;
	return _Tree->SkillsTree[eSkill].Skill == SKILLS::unknown;
}

// ***************************************************************************
ESkills CSkillManager::getParent (ESkills eSkill)
{
	nlassert(_Tree);
	if ((eSkill < 0) || (eSkill >= SKILLS::NUM_SKILLS))
		return SKILLS::unknown;
	if ((_Tree->SkillsTree[eSkill].ParentSkill < 0) ||
		(_Tree->SkillsTree[eSkill].ParentSkill >= SKILLS::NUM_SKILLS))
		return SKILLS::unknown;
	return _Tree->SkillsTree[eSkill].ParentSkill;
}

// ***************************************************************************
const std::vector<SKILLS::ESkills> &CSkillManager::getChildren(SKILLS::ESkills eSkill)
{
	nlassert(_Tree);
	static	vector<SKILLS::ESkills>	emptyVect;
	if ((eSkill < 0) || (eSkill >= SKILLS::NUM_SKILLS))
		return emptyVect;
	return _Tree->SkillsTree[eSkill].ChildSkills;
}


// ***************************************************************************
bool	CSkillManager::areSkillOnSameBranch(SKILLS::ESkills s0, SKILLS::ESkills s1)
{
	SKILLS::ESkills	parent;

	if ((s0 < 0) || (s0 >= SKILLS::NUM_SKILLS) || (s1 < 0) || (s1 >= SKILLS::NUM_SKILLS))
		return false;

	// No if only one is unknown
	if(s0==SKILLS::unknown || s1==SKILLS::unknown)
		return false;

	// The 2 skills are on the same branch if:

	// the 2 are equals!
	if(s0==s1)
		return true;

	// one is parent of the other.
	parent= getParent(s0);
	while(parent!=SKILLS::unknown)
	{
		if(parent==s1)
			return true;
		parent= getParent(parent);
	}

	// or the other is parent of the one.
	parent= getParent(s1);
	while(parent!=SKILLS::unknown)
	{
		if(parent==s0)
			return true;
		parent= getParent(parent);
	}

	// else not on same branch
	return false;
}


// ***************************************************************************
bool	CSkillManager::isSkillAncestor(SKILLS::ESkills s0, SKILLS::ESkills s1)
{
	SKILLS::ESkills	parent;

	if ((s0 < 0) || (s0 >= SKILLS::NUM_SKILLS) || (s1 < 0) || (s1 >= SKILLS::NUM_SKILLS))
		return false;

	// No if only one is unknown
	if(s0==SKILLS::unknown || s1==SKILLS::unknown)
		return false;

	// the 2 are equals?
	if(s0==s1)
		return true;

	// or if s1 has a parent == s0.
	parent= getParent(s1);
	while(parent!=SKILLS::unknown)
	{
		if(parent==s0)
			return true;
		parent= getParent(parent);
	}

	// else
	return false;
}


// ***************************************************************************
uint32	CSkillManager::getMinSkillValue(SKILLS::ESkills eSkill)
{
	if ((eSkill < 0) || (eSkill >= SKILLS::NUM_SKILLS))
		return 0;
	return _MinSkillValue[eSkill];
}

// ***************************************************************************
uint32	CSkillManager::getMaxSkillValue(SKILLS::ESkills eSkill)
{
	nlassert(_Tree);
	if ((eSkill < 0) || (eSkill >= SKILLS::NUM_SKILLS))
		return 0;
	return _Tree->SkillsTree[eSkill].MaxSkillValue;
}

// ***************************************************************************
uint32	CSkillManager::getSkillValue(SKILLS::ESkills eSkill)
{
	if ((eSkill < 0) || (eSkill >= SKILLS::NUM_SKILLS))
		return 0;

	CCDBNodeLeaf	*node= _SkillValues[eSkill];
	if(node)
		return node->getValue32();
	else
		return 0;
}

// ***************************************************************************
uint32	CSkillManager::getBaseSkillValue(SKILLS::ESkills eSkill)
{
	if ((eSkill < 0) || (eSkill >= SKILLS::NUM_SKILLS))
		return 0;

	CCDBNodeLeaf	*node= _SkillBaseValues[eSkill];
	if(node)
		return node->getValue32();
	else
		return 0;
}

// ***************************************************************************
uint32	CSkillManager::getSkillValueMaxBranch(SKILLS::ESkills eSkill)
{
	uint32	ret= getSkillValue(eSkill);
	while( (eSkill=getParent(eSkill)) !=SKILLS::unknown)
	{
		ret= max(ret, getSkillValue(eSkill));
	}
	return ret;
}

// ***************************************************************************
uint32	CSkillManager::getBaseSkillValueMaxBranch(SKILLS::ESkills eSkill)
{
	uint32	ret= getBaseSkillValue(eSkill);
	while( (eSkill=getParent(eSkill)) !=SKILLS::unknown)
	{
		ret= max(ret, getBaseSkillValue(eSkill));
	}
	return ret;
}

// ***************************************************************************
uint32	CSkillManager::getSkillValueMaxChildren(SKILLS::ESkills eSkill)
{
	uint32	ret= getSkillValue(eSkill);
	const vector<SKILLS::ESkills> & children = getChildren(eSkill);
	for( uint i=0; i<children.size(); ++i )
	{
		ret= max(ret, getSkillValueMaxChildren(children[i]));
	}
	return ret;
}

// ***************************************************************************
uint32	CSkillManager::getBestSkillValue(SKILLS::ESkills eSkill)
{
	return max(getSkillValueMaxBranch(eSkill), getSkillValueMaxChildren(eSkill));
}

// ***************************************************************************
uint32	CSkillManager::getBaseSkillValueMaxChildren(SKILLS::ESkills eSkill)
{
	if ((eSkill < 0) || (eSkill >= SKILLS::NUM_SKILLS))
		return 0;
	return _MaxChildBaseSkillValue[eSkill];
}

// ***************************************************************************
bool	CSkillManager::checkBaseSkillMetRequirement(SKILLS::ESkills eSkill, uint32 value)
{
	if( eSkill == SKILLS::unknown )
	{
		if(_MaxChildBaseSkillValue[SKILLS::SF] >= value)
			return true;
		if(_MaxChildBaseSkillValue[SKILLS::SM] >= value)
			return true;
		if(_MaxChildBaseSkillValue[SKILLS::SC] >= value)
			return true;
		if(_MaxChildBaseSkillValue[SKILLS::SH] >= value)
			return true;

		return false;
	}

	if (_MaxChildBaseSkillValue[eSkill] >= value)
		return true;

	while( (eSkill=getParent(eSkill)) !=SKILLS::unknown)
	{
		if (_MaxChildBaseSkillValue[eSkill] >= value)
			return true;
	}
	return false;
}

// ***************************************************************************
void	CSkillManager::computeMaxChildValues()
{
	// Update all MaxBaseSkill
	for( uint i = 0 ; i < NUM_SKILLS ; ++i )
	{
		const SKILLS::ESkills skill = SKILLS::ESkills(i);
		const uint32 value = getBaseSkillValue(skill);
		// if skill value > 0 and <= max, update parents for max child value
		if (value > 0 && value <= getMaxSkillValue(skill) )
			updateParentSkillsMaxChildValue(skill);
	}
}

// ***************************************************************************
void	CSkillManager::updateParentSkillsMaxChildValue(SKILLS::ESkills eSkill)
{
	const uint32 value = getBaseSkillValue(eSkill);
	// check current skill itself
	if (_MaxChildBaseSkillValue[eSkill] < value)
		_MaxChildBaseSkillValue[eSkill] = value;

	// parent skills
	while( (eSkill=getParent(eSkill)) !=SKILLS::unknown)
	{
		// if value if below max child skill value, exit
		if (_MaxChildBaseSkillValue[eSkill] >= value)
			return;

		_MaxChildBaseSkillValue[eSkill] = value;
	}
}

// ***************************************************************************
void	CSkillManager::appendSkillChangeCallback(ISkillChangeCallback *cb)
{
	if(cb)
		_SkillChangeCallbackSet.insert(cb);
}

// ***************************************************************************
void	CSkillManager::removeSkillChangeCallback(ISkillChangeCallback *cb)
{
	if(cb)
		_SkillChangeCallbackSet.erase(cb);
}

// ***************************************************************************
void	CSkillManager::onSkillChange()
{
	// **** Check cache (don't call onSkillChange if just PROGRESS_BAR changed)
	bool	someChange= false;
	for(uint i=0;i<SKILLS::NUM_SKILLS;i++)
	{
		// SKILL
		if(_SkillValues[i])
		{
			sint32	val= _SkillValues[i]->getValue32();
			if(val!=_CacheSkillValues[i])
			{
				someChange= true;
				_CacheSkillValues[i]= val;
				// NB: cannot break, because must update all cache
			}
		}
		// BaseSKILL
		if(_SkillBaseValues[i])
		{
			sint32	val= _SkillBaseValues[i]->getValue32();
			if(val!=_CacheSkillBaseValues[i])
			{
				someChange= true;
				_CacheSkillBaseValues[i]= val;
				// NB: cannot break, because must update all cache
			}
		}
	}

	// **** only if some true change
	if(someChange)
	{
		CSkillManager::TSCCBSet::iterator	it;
		// Call onSkillChange for any callback
		for(it=_SkillChangeCallbackSet.begin();it!=_SkillChangeCallbackSet.end();it++)
		{
			(*it)->onSkillChange();
		}

		// Also increment a marker
		if(_TrackSkillChange)
		{
			sint32	val= _TrackSkillChange->getValue32();
			_TrackSkillChange->setValue32(val+1);
		}

		// re-compute max child skill values
		CSkillManager::getInstance()->computeMaxChildValues();
	}
}

// ***************************************************************************
void CSkillManager::checkTitleUnblocked(CHARACTER_TITLE::ECharacterTitle i, bool show_message)
{
	if (isTitleReserved(i)) return;

	// Is all unblocked ?
	bool bAllUnblockedSkill = false;
	uint k;
	if( _TitlesUnblocked[i].UnblockedSkillLists.size() )
	{
		for (k = 0; k < _TitlesUnblocked[i].UnblockedSkillLists.size(); ++k)
			if (_TitlesUnblocked[i].UnblockedSkillLists[k] == true)
			{
				bAllUnblockedSkill = true;
				break;
			}
	}
	else
		bAllUnblockedSkill = true;

	bool bAllUnblockedItem = false;
	if( _TitlesUnblocked[i].UnblockedItemLists.size() )
	{
		for (k = 0; k < _TitlesUnblocked[i].UnblockedItemLists.size(); ++k)
			if (_TitlesUnblocked[i].UnblockedItemLists[k] == true)
			{
				bAllUnblockedItem = true;
				break;
			}
	}
	else
		bAllUnblockedItem = true;

	bool bAllUnblockedBrick = true;
	for (k = 0; k < _TitlesUnblocked[i].UnblockedBricks.size(); ++k)
		if (_TitlesUnblocked[i].UnblockedBricks[k] == false)
		{
			bAllUnblockedBrick = false;
			break;
		}
	bool bAllUnblockedMinFame = true;
	for (k = 0; k < _TitlesUnblocked[i].UnblockedMinFames.size(); ++k)
		if (_TitlesUnblocked[i].UnblockedMinFames[k] == false)
		{
			bAllUnblockedMinFame = false;
			break;
		}
	bool bAllUnblockedMaxFame = true;
	for (k = 0; k < _TitlesUnblocked[i].UnblockedMaxFames.size(); ++k)
		if (_TitlesUnblocked[i].UnblockedMaxFames[k] == false)
		{
			bAllUnblockedMaxFame = false;
			break;
		}

	bool bUnblockedCiv = _TitlesUnblocked[i].UnblockedCiv;
	bool bUnblockedCult = _TitlesUnblocked[i].UnblockedCult;

	bool bUnblockedCharOldness = true; //_TitlesUnblocked[i].UnblockedCharOldness;
	bool bUnblockedCharPlayedTime = _TitlesUnblocked[i].UnblockedCharPlayedTime;
	bool bUnblockedAccountOldness = true; //_TitlesUnblocked[i].UnblockedAccountOldness;


	bool bAllUnblocked = bAllUnblockedSkill && bAllUnblockedBrick && bAllUnblockedItem && bAllUnblockedMinFame && bAllUnblockedMaxFame
							&& bUnblockedCiv && bUnblockedCult && bUnblockedCharOldness && bUnblockedCharPlayedTime
							&& bUnblockedAccountOldness;

	// If title availability changed
	if (bAllUnblocked != _TitlesUnblocked[i].Unblocked)
	{
		_TitlesUnblocked[i].Unblocked = bAllUnblocked;
		if (!IngameDbMngr.initInProgress())
		{
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			if (bAllUnblocked && show_message)
			{
				// This is a new title, send a message
				string titleStr = CHARACTER_TITLE::toString((CHARACTER_TITLE::ECharacterTitle)i);
				bool womenTitle = (UserEntity && UserEntity->getGender() == GSGENDER::female);
				const ucstring newtitle(CStringManagerClient::getTitleLocalizedName(titleStr, womenTitle));
				CAHManager::getInstance()->runActionHandler("message_popup", NULL, "text1="+newtitle.toUtf8()+"|text0="+CI18N::get("uiNewTitleBold").toUtf8());
			}
			else
			{
				// Title is not available anymore, change current title if needed
				if (i == CHARACTER_TITLE::ECharacterTitle(_CurrentTitle))
				{
					CBitMemStream out;
					static const char *msgName = "GUILD:SET_PLAYER_TITLE";
					if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
					{
						uint8 nNewTitle = uint8(CHARACTER_TITLE::Homin);
						setCurrentTitle(nNewTitle);
						out.serial(nNewTitle);
						NetMngr.push(out);
						//nlinfo("impulseCallBack : %s %d sent", msgName, nNewTitle);
					}
					else
					{
						nlwarning("unknown message name : '%s'.", msgName);
					}
				}
			}

			// Update title combo box
			CAHManager::getInstance()->runActionHandler("title_init_combobox", NULL);
		}
	}
}

// ***************************************************************************
void CSkillManager::initTitles()
{
	CSBrickManager *pBM = CSBrickManager::getInstance();
	pBM->appendBrickLearnedCallback(&BrickLearnedCB);
}

// ***************************************************************************
void CSkillManager::uninitTitles()
{
	CSBrickManager *pBM = CSBrickManager::getInstance();
	pBM->removeBrickLearnedCallback(&BrickLearnedCB);
}

// ***************************************************************************
void CSkillManager::tryToUnblockTitleFromSkill(SKILLS::ESkills eSkill, sint32 value)
{
	for (uint i = 0; i < CHARACTER_TITLE::NB_CHARACTER_TITLE; ++i)
	{
		if (_TitlesUnblocked[i].Unblocked) continue;

		string sSkill = SKILLS::toString(eSkill);

		CUnblockTitlesSheet::STitleUnblock rTU = _UnblockTitle->TitlesUnblock[i];

		if (rTU.Reserved) continue;

		for (uint j = 0; j < rTU.SkillsNeeded.size(); ++j) // for all skill lists
		{
			if (! _TitlesUnblocked[i].UnblockedSkillLists[j]) // Not already unblocked
			{
				bool allSkillsFromListValidated = true;

				for (uint k = 0; k < rTU.SkillsNeeded[j].size(); ++k) // for all skills in current skill list
				{
					// if skill value too low
					if (value < rTU.SkillsLevelNeeded[j][k])
						allSkillsFromListValidated = false;

					// If not the good skill (skill length test)
					if (sSkill.size() < rTU.SkillsNeeded[j][k].size())
					{
						allSkillsFromListValidated = false;
					}
					else
					{
						// If not the good skill (bis) (skill hierarchy test)
						if (strncmp(sSkill.c_str(), rTU.SkillsNeeded[j][k].c_str(), rTU.SkillsNeeded[j][k].size()) != 0)
						{
							allSkillsFromListValidated = false;
						}
					}
				}
				if( allSkillsFromListValidated )
				{
					// Ok we can unblock
					_TitlesUnblocked[i].UnblockedSkillLists[j] = true;
					checkTitleUnblocked((CHARACTER_TITLE::ECharacterTitle)i);
				}
			}
		}
	}
}

// ***************************************************************************
void CSkillManager::tryToUnblockTitleFromBricks(bool show_message)
{
	CSBrickManager *pSBM =  CSBrickManager::getInstance();

	for (uint i = 0; i < CHARACTER_TITLE::NB_CHARACTER_TITLE; ++i)
	{
		if (_TitlesUnblocked[i].Unblocked) continue;

		CUnblockTitlesSheet::STitleUnblock rTU = _UnblockTitle->TitlesUnblock[i];

		if (rTU.Reserved) continue;

		for (uint j = 0; j < rTU.BricksNeeded.size(); ++j)
		if (! _TitlesUnblocked[i].UnblockedBricks[j]) // Not already unblocked
		{
			if (pSBM->isBrickKnown(rTU.BricksNeeded[j]))
			{
				_TitlesUnblocked[i].UnblockedBricks[j] = true;
			}
		}

		checkTitleUnblocked((CHARACTER_TITLE::ECharacterTitle)i, show_message);
	}
}

// ***************************************************************************
void CSkillManager::tryToUnblockTitleFromMinFames( uint32 factionIndex, sint32 fameValue )
{
	for (uint i = 0; i < CHARACTER_TITLE::NB_CHARACTER_TITLE; ++i)
	{
		CUnblockTitlesSheet::STitleUnblock rTU = _UnblockTitle->TitlesUnblock[i];

		// skip reserved titles
		if (rTU.Reserved) continue;

		for (uint j = 0; j < rTU.MinFames.size(); ++j)
		{
			if( rTU.MinFames[j] != factionIndex )
			{
				continue;
			}
			const bool unblocked = (fameValue >= rTU.MinFameLevels[j]);
			if (unblocked != _TitlesUnblocked[i].UnblockedMinFames[j])
			{
				_TitlesUnblocked[i].UnblockedMinFames[j] = unblocked;
				checkTitleUnblocked((CHARACTER_TITLE::ECharacterTitle)i);
			}
			break; // there should not be more than one fame prerequisite per faction per title
		}
	}
}

// ***************************************************************************
void CSkillManager::tryToUnblockTitleFromMaxFames( uint32 factionIndex, sint32 fameValue )
{
	for (uint i = 0; i < CHARACTER_TITLE::NB_CHARACTER_TITLE; ++i)
	{
		CUnblockTitlesSheet::STitleUnblock rTU = _UnblockTitle->TitlesUnblock[i];

		// skip reserved titles
		if (rTU.Reserved) continue;

		for (uint j = 0; j < rTU.MaxFames.size(); ++j)
		{
			if( rTU.MaxFames[j] != factionIndex )
			{
				continue;
			}
			const bool unblocked = (fameValue <= rTU.MaxFameLevels[j]);
			if (unblocked != _TitlesUnblocked[i].UnblockedMaxFames[j])
			{
				_TitlesUnblocked[i].UnblockedMaxFames[j] = unblocked;
				checkTitleUnblocked((CHARACTER_TITLE::ECharacterTitle)i);
			}
			break; // there should not be more than one fame prerequisite per faction per title
		}
	}
}


// ***************************************************************************
void CSkillManager::tryToUnblockTitleFromCiv(bool show_message)
{
	for (uint i = 0; i < CHARACTER_TITLE::NB_CHARACTER_TITLE; ++i)
	{
		CUnblockTitlesSheet::STitleUnblock rTU = _UnblockTitle->TitlesUnblock[i];

		if (rTU.Reserved) continue;

		_TitlesUnblocked[i].UnblockedCiv = true;
		if( !rTU.CivNeeded.empty() )
		{
			CInterfaceManager *im = CInterfaceManager::getInstance();
			uint8 civNeeded = (uint8) PVP_CLAN::fromString(rTU.CivNeeded);

			if (IngameDbMngr.initInProgress())
			{
				if (NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:FAME:CIV_ALLEGIANCE")->getValue32() != civNeeded)
					_TitlesUnblocked[i].UnblockedCiv = false;
				continue;
			}
			else
			{
				CCDBNodeLeaf * civLeaf = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:FAME:CIV_ALLEGIANCE");
				uint8 civDBValue = civLeaf->getValue8();
				NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:FAME:CIV_ALLEGIANCE")->setValue32((uint32)civDBValue);

				if( civDBValue != civNeeded )
				{
					_TitlesUnblocked[i].UnblockedCiv = false;
					continue;
				}
			}
		}
		checkTitleUnblocked((CHARACTER_TITLE::ECharacterTitle)i, show_message);
	}
}

// ***************************************************************************
void CSkillManager::tryToUnblockTitleFromCult(bool show_message)
{

	for (uint i = 0; i < CHARACTER_TITLE::NB_CHARACTER_TITLE; ++i)
	{
		CUnblockTitlesSheet::STitleUnblock rTU = _UnblockTitle->TitlesUnblock[i];

		if (rTU.Reserved) continue;

		_TitlesUnblocked[i].UnblockedCult = true;
		if( !rTU.CultNeeded.empty() )
		{
			CInterfaceManager *im = CInterfaceManager::getInstance();
			uint8 cultNeeded = (uint8) PVP_CLAN::fromString(rTU.CultNeeded);

			if (IngameDbMngr.initInProgress())
			{
				if (NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:FAME:CULT_ALLEGIANCE")->getValue32() != cultNeeded)
					_TitlesUnblocked[i].UnblockedCult = false;
				continue;
			}
			else
			{
				CCDBNodeLeaf * cultLeaf = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:FAME:CULT_ALLEGIANCE");
				uint8 cultDBValue = cultLeaf->getValue8();
				NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:FAME:CULT_ALLEGIANCE")->setValue32((uint32)cultDBValue);

				if( cultDBValue != cultNeeded )
				{
					_TitlesUnblocked[i].UnblockedCult = false;
					continue;
				}
			}
		}
		checkTitleUnblocked((CHARACTER_TITLE::ECharacterTitle)i, show_message);
	}
}

// ***************************************************************************
void CSkillManager::unblockTitleFromServer(CHARACTER_TITLE::ECharacterTitle ct)
{
	if ( ! isTitleReserved(ct))
	{
		nlwarning("server tries to unblock a title that is not reserved");
		return;
	}
	_TitlesUnblocked[ct].Unblocked = true;

	// update emotes
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	pIM->updateEmotes();
}


/// ---------------------------------------------
void CSkillManager::tryToUnblockTitleFromCharOldness( uint32 firstConnectedTime )
{
	uint32 time = CTime::getSecondsSince1970();
	if( time > firstConnectedTime )
	{
		if( firstConnectedTime == 0 )
		{
			nlwarning("<CSkillManager::tryToUnblockTitleFromCharOldness> first connect time is null !");
		}

		uint32 oldness = (time - firstConnectedTime)/(60 * 60 * 24);

		for (uint i = 0; i < CHARACTER_TITLE::NB_CHARACTER_TITLE; ++i)
		{
			_TitlesUnblocked[i].UnblockedCharOldness = true;

			CUnblockTitlesSheet::STitleUnblock rTU = _UnblockTitle->TitlesUnblock[i];

			if (rTU.Reserved) continue;

			if( !rTU.CharOldness.empty() )
			{
				uint32 requiredOldness;
				fromString(rTU.CharOldness, requiredOldness);
				_TitlesUnblocked[i].UnblockedCharOldness = (oldness > requiredOldness);
			}
			checkTitleUnblocked((CHARACTER_TITLE::ECharacterTitle)i);
		}
	}
	else
	{
		nlwarning("<CSkillManager::tryToUnblockTitleFromCharOldness> bad first connect time ? %d",firstConnectedTime);
	}
}

/// ---------------------------------------------
void CSkillManager::tryToUnblockTitleFromCharPlayedTime( uint32 playedTime )
{
	for (uint i = 0; i < CHARACTER_TITLE::NB_CHARACTER_TITLE; ++i)
	{
		_TitlesUnblocked[i].UnblockedCharPlayedTime = true;

		CUnblockTitlesSheet::STitleUnblock rTU = _UnblockTitle->TitlesUnblock[i];
		if (rTU.Reserved) continue;

		if( !rTU.CharPlayedTime.empty() )
		{
			uint32 requiredPlayedTime;
			fromString(rTU.CharPlayedTime, requiredPlayedTime);
			_TitlesUnblocked[i].UnblockedCharPlayedTime = (playedTime/(60 * 60 * 24) > requiredPlayedTime);
		}
		checkTitleUnblocked((CHARACTER_TITLE::ECharacterTitle)i);
	}
}

/// ---------------------------------------------
void CSkillManager::tryToUnblockTitleFromAccountOldness( uint32 /* accountCreationTime */ )
{
}

/// ---------------------------------------------
void CSkillManager::tryToUnblockTitleFromRingRatings( uint32 authorRating, uint32 amRating, uint32 masterlessRating )
{
	for (uint i = 0; i < CHARACTER_TITLE::NB_CHARACTER_TITLE; ++i)
	{
		CUnblockTitlesSheet::STitleUnblock rTU = _UnblockTitle->TitlesUnblock[i];
		if (rTU.Reserved) continue;

		_TitlesUnblocked[i].UnblockedAuthorRating = rTU.AuthorRating <= authorRating;
		_TitlesUnblocked[i].UnblockedAMRating = rTU.AMRating <= amRating;
		_TitlesUnblocked[i].UnblockedOrganizerRating = rTU.OrganizerRating <= masterlessRating;

		checkTitleUnblocked((CHARACTER_TITLE::ECharacterTitle)i);
	}
}

// ***************************************************************************
void CSkillManager::tryToUnblockTitleFromItems(bool show_message)
{
	if (IngameDbMngr.initInProgress())
		return;

	std::string branch = "LOCAL:INVENTORY:BAG";

	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// get inventory in bag
	CCDBNodeBranch *nb = NLGUI::CDBManager::getInstance()->getDbBranch(branch);
	if (!nb)
		return;

	// get items count
	uint numItems = nb->getNbNodes();
	if (!numItems)
		return;

	for (uint i = 0; i < CHARACTER_TITLE::NB_CHARACTER_TITLE; ++i)
	{
//		if (_TitlesUnblocked[i].Unblocked)
//			continue;

		CUnblockTitlesSheet::STitleUnblock rTU = _UnblockTitle->TitlesUnblock[i];

		// don't check reserved titles or titles without items
		if (rTU.Reserved || rTU.ItemsNeeded.empty()) continue;

		for (uint j = 0; j < rTU.ItemsNeeded.size(); ++j) // for all item lists
		{
//			if (_TitlesUnblocked[i].UnblockedItemLists[j]) // Not already unblocked
//				continue;

			uint numItemsFromListToValidate = (uint)rTU.ItemsNeeded[j].size();

			for (uint k = 0; k < rTU.ItemsNeeded[j].size(); ++k) // for all items in current item list
			{
				// check items present in bag
				for (uint itemSlot = 0; itemSlot < numItems; ++itemSlot)
				{
					uint32 sheetItem = 0;
					sint32 qualityItem = 0;

					// get sheetid
					CCDBNodeLeaf *node = NLGUI::CDBManager::getInstance()->getDbProp(branch + ":" + toString(itemSlot) + ":SHEET", false);
					if (node)
						sheetItem = (uint32)node->getValue32();

					// slot empty
					if (!sheetItem)
						continue;

					// get quality
					node = NLGUI::CDBManager::getInstance()->getDbProp(branch + ":" + toString(itemSlot) + ":QUALITY", false);
					if (node)
						qualityItem = node->getValue32();

					// sheetid and quality are identical
					if (qualityItem == rTU.ItemsQualityNeeded[j][k] && sheetItem == rTU.ItemsNeeded[j][k].asInt())
						--numItemsFromListToValidate;

					// we found all items, don't process next ones
					if (!numItemsFromListToValidate)
						break;
				}
			}

			bool allItemsFromListValidated = (numItemsFromListToValidate == 0);

			// ok we can block or unblock
			if (allItemsFromListValidated != _TitlesUnblocked[i].UnblockedItemLists[j])
			{
				_TitlesUnblocked[i].UnblockedItemLists[j] = allItemsFromListValidated;
				checkTitleUnblocked((CHARACTER_TITLE::ECharacterTitle)i, show_message);
			}
		}
	}
}

// ***************************************************************************
void CSkillManager::blockTitleFromServer(CHARACTER_TITLE::ECharacterTitle ct)
{
	if ( ! isTitleReserved(ct))
	{
		nlwarning("server tries to block a title that is not reserved");
		return;
	}
	_TitlesUnblocked[ct].Unblocked = false;

	// update emotes
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	pIM->updateEmotes();
}

// ***************************************************************************
void CSkillManager::setPlayerTitle(const std::string &name)
{
	setCurrentTitle(CHARACTER_TITLE::toCharacterTitle(name));
	CAHManager::getInstance()->runActionHandler("title_init_combobox", NULL);
}


// ***************************************************************************
// ***************************************************************************
// CHARACTER TITLE
// ***************************************************************************
// ***************************************************************************

#define GROUP_TITLE_COMBO "ui:interface:info_player_skills:content:webinfos:title:player_title"

// ***************************************************************************
class CHandlerTitleInit: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CAHManager::getInstance()->runActionHandler("title_combobox_button", NULL);

		// Setup UI:TITLE from current title
		CSkillManager *pSM = CSkillManager::getInstance();
		uint i,j = 0;
		for (i = 0; i < CHARACTER_TITLE::NB_CHARACTER_TITLE; ++i)
		if (pSM->isTitleUnblocked((CHARACTER_TITLE::ECharacterTitle)i))
		{
			if (i == pSM->_CurrentTitle)
			{
				NLGUI::CDBManager::getInstance()->getDbProp("UI:TITLE")->setValue32(j);
				break;
			}
			j++;
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerTitleInit, "title_init_combobox");

// ***************************************************************************
class CHandlerTitleButton: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CSkillManager *pSM = CSkillManager::getInstance();
		// Try to unblock titles without showing the new title message
		pSM->tryToUnblockTitleFromBricks(false);
		pSM->tryToUnblockTitleFromCiv(false);
		pSM->tryToUnblockTitleFromCult(false);
		pSM->tryToUnblockTitleFromItems(false);

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CDBGroupComboBox *pCB = dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId(GROUP_TITLE_COMBO));
		if (pCB != NULL)
		{
			pCB->resetTexts();
			pSM->_UIUnblockedTitles.clear();
			for (uint i = 0; i < CHARACTER_TITLE::NB_CHARACTER_TITLE; ++i)
			{
				if (pSM->isTitleUnblocked((CHARACTER_TITLE::ECharacterTitle)i))
				{
					string titleStr = CHARACTER_TITLE::toString((CHARACTER_TITLE::ECharacterTitle)i);
					bool womenTitle = (UserEntity && UserEntity->getGender() == GSGENDER::female);
					const ucstring s(CStringManagerClient::getTitleLocalizedName(titleStr,womenTitle));
					pCB->addText(s);
					pSM->_UIUnblockedTitles.push_back((CHARACTER_TITLE::ECharacterTitle)i);
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerTitleButton, "title_combobox_button");


// ***************************************************************************
class CHandlerTitleChanged: public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CSkillManager *pSM = CSkillManager::getInstance();
		uint8 nNewTitle = 0;

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CDBGroupComboBox *pCB = dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getElementFromId(GROUP_TITLE_COMBO));
		if (pCB == NULL) return;
		if ((pCB->getSelection() < 0) || (pCB->getSelection() >= (sint32)pSM->_UIUnblockedTitles.size())) return;

		nNewTitle = (uint8)pSM->_UIUnblockedTitles[pCB->getSelection()];

		// If new title choosen is different from current title -> Send the message to the server
		if (nNewTitle != pSM->_CurrentTitle)
		{
			CBitMemStream out;
			static const char *msgName = "GUILD:SET_PLAYER_TITLE";
			if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
			{
				pSM->setCurrentTitle(nNewTitle);
				out.serial(nNewTitle);
				NetMngr.push(out);
				//nlinfo("impulseCallBack : %s %d sent", msgName, nNewTitle);
			}
			else
			{
				nlwarning("unknown message name : '%s'.", msgName);
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerTitleChanged, "title_combobox_changed");


void CSkillManager::setCurrentTitle(uint8 title)
{
	_CurrentTitle = title;
	CNPCIconCache::getInstance().onEventForMissionAvailabilityForThisChar();
}
