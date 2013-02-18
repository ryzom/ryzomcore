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



#ifndef RY_SKILL_MANAGER_H
#define RY_SKILL_MANAGER_H

#include "nel/misc/types_nl.h"
#include "game_share/skills.h"
//#include "game_share/jobs.h"
#include "game_share/roles.h"
#include "nel/misc/cdb.h"
#include "brick_learned_callback.h"
#include "skill_change_callback.h"

#include "game_share/skills.h"
#include "game_share/memorization_set_types.h"
#include "../client_sheets/skills_tree_sheet.h"
#include "../client_sheets/unblock_titles_sheet.h"

// ***************************************************************************
/**
 * class used to manage the skill tree
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2003 September
 */
class CSkillManager
{

public:

	/// The singleton's instance
	static CSkillManager* getInstance()
	{
		if (!_Instance)
			_Instance = new CSkillManager;
		return _Instance;
	}

	// release singleton
	static void releaseInstance()
	{
		delete _Instance;
		_Instance = NULL;
	}

	/// Destructor
	virtual ~CSkillManager();

	// Initialize by loading skills (load the skill_tree stuffs) and organize so in the vector skill == index
	void initInGame();
	void uninitInGame();

	bool isUnknown (SKILLS::ESkills eSkill);

	SKILLS::ESkills getParent (SKILLS::ESkills eSkill);

	const std::vector<SKILLS::ESkills> &getChildren (SKILLS::ESkills eSkill);

	/// return true if 2 skills are on the same Branch (ie compatible). NB: return false if one is unknown
	bool	areSkillOnSameBranch(SKILLS::ESkills s0, SKILLS::ESkills s1);

	/// return true if the skill s0 is an ancestor of skill s1 (true if s0==s1)
	bool	isSkillAncestor(SKILLS::ESkills s0, SKILLS::ESkills s1);

	/// Get the MinSkillValue (ie the max skill value of the parent)
	uint32	getMinSkillValue(SKILLS::ESkills eSkill);

	/// Get the MaxSkillValue
	uint32	getMaxSkillValue(SKILLS::ESkills eSkill);

	/// Get the SkillValue in the database (BUFFED one)
	uint32	getSkillValue(SKILLS::ESkills eSkill);

	/// Get the SkillValue in the database (unBUFFED one)
	uint32	getBaseSkillValue(SKILLS::ESkills eSkill);


	/// Get the SkillValue in the database (BUFFED one). Get the max of the skill and its parent
	uint32	getSkillValueMaxBranch(SKILLS::ESkills eSkill);

	/// Get the SkillValue in the database (unBUFFED one). Get the max of the skill and its parent
	uint32	getBaseSkillValueMaxBranch(SKILLS::ESkills eSkill);

	/// Get the SkillValue in the database (BUFFED one). Get the max of the skill and its children
	/// NB: warning: slow since recursive
	uint32	getSkillValueMaxChildren(SKILLS::ESkills eSkill);

	/// Get the MaxSkillValue in the branch (from parent to children) (BUFFED one)
	/// NB: warning: slow since use getSkillValueMaxChildren
	uint32	getBestSkillValue(SKILLS::ESkills eSkill);

	/// Get the BaseSkillValue in the database (unBUFFED one). Get the max of the skill and its children
	/// NB: fast O(1) since BaseSkillValue
	uint32	getBaseSkillValueMaxChildren(SKILLS::ESkills eSkill);

	/// return true if base skill max branch value is > required skill value
	bool checkBaseSkillMetRequirement(SKILLS::ESkills eSkill, uint32 value);

	/// Callback called when any skill change
	void	appendSkillChangeCallback(ISkillChangeCallback *cb);
	void	removeSkillChangeCallback(ISkillChangeCallback *cb);

	/// CHARACTER TITLE

	/// Append the brick learned callback to the brick manager
	void initTitles();

	/// Remove the callback
	void uninitTitles();

	/// Called when a skill change to look if we can unblock a title
	void tryToUnblockTitleFromSkill(SKILLS::ESkills eSkill, sint32 value);

	/// The same but with parsing all bricks wanted
	void tryToUnblockTitleFromBricks(bool show_message = true);

	/// The same but with civ allegiance
	void tryToUnblockTitleFromCiv(bool show_message = true);

	/// The same but with cult allegiance
	void tryToUnblockTitleFromCult(bool show_message = true);

	/// The same but with parsing all items wanted
	void tryToUnblockTitleFromItems(bool show_message = true);

	/// Called when a fame changes to look if we can unblock a title
	void tryToUnblockTitleFromMinFames(uint32 factionIndex, sint32 fameValue);
	void tryToUnblockTitleFromMaxFames(uint32 factionIndex, sint32 fameValue);

	/// called after user chose a char, unblock titles from char time properties
	void tryToUnblockTitleFromCharOldness( uint32 firstConnectedTime );
	void tryToUnblockTitleFromCharPlayedTime( uint32 playedTime );
	void tryToUnblockTitleFromAccountOldness( uint32 accountCreationTime );

	/// called when ring ratings change
	void tryToUnblockTitleFromRingRatings(uint32 authorRating, uint32 amRating, uint32 masterlessRating);

	/// Unblock a reserved title from the server
	void unblockTitleFromServer(CHARACTER_TITLE::ECharacterTitle ct);

	/// Block a reserved title from the server
	void blockTitleFromServer(CHARACTER_TITLE::ECharacterTitle ct);

	/// Is the title unblocked and accessible to the player ?
	bool isTitleUnblocked(CHARACTER_TITLE::ECharacterTitle ct) { return _TitlesUnblocked[ct].Unblocked; }

	/// Is the title reserved (can be unblocked only by a server message)
	bool isTitleReserved(CHARACTER_TITLE::ECharacterTitle ct) { return _UnblockTitle->TitlesUnblock[ct].Reserved; }

	void setPlayerTitle(const std::string &name);
	uint8 getPlayerTitle() const { return _CurrentTitle; }

protected:

	void setCurrentTitle(uint8 title);

private:
	// update parent skills max child value for given skill
	void updateParentSkillsMaxChildValue(SKILLS::ESkills eSkill);

	/// compute parent skills max child value for all skill tree
	void computeMaxChildValues();

	/// Constructor
	CSkillManager();

	/// Singleton's instance
	static CSkillManager	*_Instance;

	CSkillsTreeSheet		*_Tree;

	// Minimum skills values
	uint32					_MinSkillValue[SKILLS::NUM_SKILLS];

	/// Nodes on skill values and base values
	NLMISC::CCDBNodeLeaf			*_SkillValues[SKILLS::NUM_SKILLS];
	NLMISC::CCDBNodeLeaf			*_SkillBaseValues[SKILLS::NUM_SKILLS];

	// Max child baseskill value (used when checking requirements)
	uint32					_MaxChildBaseSkillValue[SKILLS::NUM_SKILLS];

	// CallBack set for skill changes
	struct CSkillChangeObs : public NLMISC::ICDBNode::IPropertyObserver
	{
		virtual void update (NLMISC::ICDBNode * /* node */)
		{
			CSkillManager	*pSM= CSkillManager::getInstance();
			pSM->onSkillChange();
		}
	};
	friend struct CSkillChangeObs;
	CSkillChangeObs			_SkillChangeObs;
	typedef	std::set<ISkillChangeCallback*>	TSCCBSet;
	TSCCBSet				_SkillChangeCallbackSet;
	void		onSkillChange();
	// Cache to know if skill really changed (not PROGRESS_BAR)
	sint32					_CacheSkillValues[SKILLS::NUM_SKILLS];
	sint32					_CacheSkillBaseValues[SKILLS::NUM_SKILLS];

	// A node incremented at each change of skill (the number is not relevant)
	NLMISC::CCDBNodeLeaf			*_TrackSkillChange;

	// "Title of the player" Management
	// -----------------------------------------------------------------------------
	friend class CHandlerTitleInit;
	friend class CHandlerTitleButton;
	friend class CHandlerTitleChanged;

	struct SUnblockingTitle
	{
		bool Unblocked;
		std::vector<bool> UnblockedSkillLists;
		std::vector<bool> UnblockedBricks;
		std::vector<bool> UnblockedMinFames;
		std::vector<bool> UnblockedMaxFames;
		std::vector<bool> UnblockedItemLists;
		bool UnblockedCiv;
		bool UnblockedCult;
		bool UnblockedCharOldness;
		bool UnblockedCharPlayedTime;
		bool UnblockedAccountOldness;
		bool UnblockedAuthorRating;
		bool UnblockedAMRating;
		bool UnblockedOrganizerRating;

	};

	std::vector<SUnblockingTitle> _TitlesUnblocked;
	void checkTitleUnblocked(CHARACTER_TITLE::ECharacterTitle i, bool show_message = true);

	uint8	_CurrentTitle;
	std::vector<sint32>	_UIUnblockedTitles;
	CUnblockTitlesSheet *_UnblockTitle;

	class CBrickLearnedCB : public IBrickLearnedCallback
	{
	public:
		virtual	void onBrickLearned()
		{
			CSkillManager::getInstance()->tryToUnblockTitleFromBricks();
		}
	};

	CBrickLearnedCB BrickLearnedCB;

};

#define WIN_FAME_LIST "ui:interface:fame:content:fame_list:list"
#define TEMPLATE_FAME "fame_charac"

#endif // RY_SKILL_MANAGER_H

/* End of skill_manager.h */
