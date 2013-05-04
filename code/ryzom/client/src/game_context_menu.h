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



#ifndef NL_GAME_CONTEXT_MENU_H
#define NL_GAME_CONTEXT_MENU_H

#include "nel/misc/types_nl.h"
#include "main_loop.h"
#include "nel/gui/ctrl_text_button.h"
#include "nel/gui/interface_group.h"
#include "interface_v3/interface_pointer.h"
#include "game_share/bot_chat_types.h"


// ***************************************************************************
namespace NLGUI
{
	class CGroupMenu;
	class CViewTextMenu;
}

namespace NLMISC{
	class CCDBNodeLeaf;
}

// ***************************************************************************
/**
 * Update Context menu
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CGameContextMenu
{
public:

	/// Constructor
	CGameContextMenu();

	// init with IM and DB
	void		init(const std::string &menuId);

	// update the context menu
	void		update();

	bool		isBuilding() const { return _OkTextTradeOutpostBuilding; }

private:

	// Conditions
	bool canAttack();
	bool canDuel();
	bool canPvpChallenge();
	bool canDisengage();
	bool canTeamKick();

	void updateContextMenuMissionsOptions( bool forceHide );
	void updateContextMenuWebPage(uint options);
	void updateContextMenuOutpostState(uint options);
	void updateContextMenuOutpostBanish();
	void updateContextMenuMissionRing();

	// talk.
	void updateContextMenuTalkEntries(uint options);
	void setupContextMenuCantTalk();
	void setupContextMenuTalkWithPlayer();
	void applyTextTalk();

	// MilkoPad
	CInterfaceGroupPtr _GroupMilkoPad;
	NLMISC::CCDBNodeLeaf	*_MilkoAttackDisengage;
	CCtrlTextButtonPtr	_MilkoAttDisBut1;
	CCtrlTextButtonPtr	_MilkoAttDisBut2;

	CGroupMenu		*_GroupMenu;
	NLMISC::CCDBNodeLeaf	*_ContextVal;
	NLMISC::CCDBNodeLeaf	*_AvailablePrograms;
	NLMISC::CCDBNodeLeaf	*_ServerTeamPresent;
	NLMISC::CCDBNodeLeaf	*_MissionOption[NUM_MISSION_OPTIONS];
	NLMISC::CCDBNodeLeaf	*_ServerInDuel;
	NLMISC::CCDBNodeLeaf	*_ServerInPvpChallenge;
	NLMISC::CCDBNodeLeaf	*_WebPageTitle;
	NLMISC::CCDBNodeLeaf	*_OutpostSheet;
	NLMISC::CCDBNodeLeaf	*_OutpostRightToBannish;
	NLMISC::CCDBNodeLeaf	*_MissionRing[BOTCHATTYPE::MaxR2MissionEntryDatabase];


	CViewTextMenuPtr _TextLootAction;
	CViewTextMenuPtr _TextQuartering;
	CViewTextMenuPtr _TextAttack;
	CViewTextMenuPtr _TextDuel;
	CViewTextMenuPtr _TextUnDuel;
	CViewTextMenuPtr _TextPvpChallenge;
	CViewTextMenuPtr _TextUnPvpChallenge;
	CViewTextMenuPtr _TextExchange;
	CViewTextMenuPtr _TextMount;
	CViewTextMenuPtr _TextFreeLook;
	CViewTextMenuPtr _TextMove;
	CViewTextMenuPtr _TextStop;
	CViewTextMenuPtr _TextDisengage;
	CViewTextMenuPtr _TextUnseat;
	CViewTextMenuPtr _TextInfo;
	CViewTextMenuPtr _TextFollow;
	CViewTextMenuPtr _TextAssist;
	CViewTextMenuPtr _TextInvit;
	CViewTextMenuPtr _TextGuildInvit;
	CViewTextMenuPtr _TextQuitTeam;
	CViewTextMenuPtr _TextAddToFriendList;
	CViewTextMenuPtr _TextMission[NUM_MISSION_OPTIONS];

	// Pack Animal
	CViewTextMenuPtr _TextPAFollow;
	CViewTextMenuPtr _TextPAStop;
	CViewTextMenuPtr _TextPAFree;
	CViewTextMenuPtr _TextPAEnterStable;

	// BotChat and player talk
	CViewTextMenuPtr _TextNews;
	CViewTextMenuPtr _TextTradeItem;
	CViewTextMenuPtr _TextTradeTeleport;
	CViewTextMenuPtr _TextTradeFaction;
	CViewTextMenuPtr _TextTradeCosmetic;
	CViewTextMenuPtr _TextTradeGuildOptions;
	CViewTextMenuPtr _TextTradeOutpostBuilding;
	CViewTextMenuPtr _TextTradeGuildRoleMaster;
	CViewTextMenuPtr _TextTradePact;
	CViewTextMenuPtr _TextTradePhrase;
	CViewTextMenuPtr _TextChooseMission;
	CViewTextMenuPtr _TextCreateGuild;
	CViewTextMenuPtr _TextDynamicMission;
	CViewTextMenuPtr _TextTalk;
	CViewTextMenuPtr _TextChooseZCCharge;
	CViewTextMenuPtr _TextChooseBuilding;
	CViewTextMenuPtr _TextBuyRM;
	CViewTextMenuPtr _TextUpgradeRM;
	CViewTextMenuPtr _TextCancelZCCharge;
	CViewTextMenuPtr _TextDestroyBuilding;
	CViewTextMenuPtr _TextOutpostState;
	CViewTextMenuPtr _TextOutpostBanishPlayer;
	CViewTextMenuPtr _TextOutpostBanishGuild;
	CViewTextMenuPtr _TextWebPage;
	CViewTextMenuPtr _TextMissionRing[BOTCHATTYPE::MaxR2MissionEntryDatabase];

	// Forage source
	CViewTextMenuPtr _TextExtractRM;

	// Build Spire
	CViewTextMenuPtr _TextBuildTotem;

	// Use intermediate OK value. not good thing to do "text->setActive(false); text->setActive(true)" each frame for nothing.
	bool			_OkTextNews;
	bool			_OkTextTradeItem;
	bool			_OkTextTradeTeleport;
	bool			_OkTextTradeFaction;
	bool			_OkTextTradeCosmetic;
	bool			_OkTextTradeGuildOptions;
	bool			_OkTextTradeOutpostBuilding;
	bool			_OkTextTradeGuildRoleMaster;
	bool			_OkTextTradePact;
	bool			_OkTextTradePhrase;
	bool			_OkTextChooseMission;
	bool			_OkTextCreateGuild;
	bool			_OkTextDynamicMission;
	bool			_OkTextTalk;
	bool			_OkTextChooseZCCharge;
	bool			_OkTextChooseBuilding;
	bool			_OkTextBuyRM;
	bool			_OkTextUpgradeRM;
	bool			_OkTextCancelZCCharge;
	bool			_OkTextDestroyBuilding;
	bool			_OkTextOutpostState;
	bool			_OkTextWebPage;
};


class CEntityCL;

/**
 * Enable/disable various menu items for animals.
 * selectedAnimalInVision can be NULL (out of vision)
 * index can be -1 (no owned animal)
 * Any CViewTextMenu* can be NULL
 */
bool testMenuOptionForPackAnimal( CEntityCL* selectedAnimalInVision, uint index, bool clearAll,
								  CViewTextMenu *pFollow, CViewTextMenu *pStop, CViewTextMenu *pFree,
								  CViewTextMenu *pEnterStable, CViewTextMenu *pLeaveStable,
								  CViewTextMenu *pMount, CViewTextMenu *pUnmount );


#endif // NL_GAME_CONTEXT_MENU_H

/* End of game_context_menu.h */
