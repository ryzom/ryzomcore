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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"
// Client.
#include "game_context_menu.h"
#include "interface_v3/interface_manager.h"
#include "nel/gui/ctrl_text_button.h"
#include "nel/gui/group_menu.h"
#include "entities.h"
#include "interface_v3/bot_chat_manager.h"
#include "interface_v3/guild_manager.h"
#include "interface_v3/people_interraction.h"
#include "main_loop.h"
#include "interface_v3/inventory_manager.h"
#include "motion/user_controls.h"
#include "sheet_manager.h"
// GAME SHARE
#include "game_share/constants.h"
#include "game_share/properties.h"
#include "game_share/bot_chat_types.h"
#include "game_share/animal_type.h"
//
#include "r2/editor.h"


using namespace	NLMISC;
using namespace std;



// filter available programs depending on R2 mode
static uint32 filterAvailablePrograms(uint32 src)
{
	return R2::getEditor().isDMing() ? 0 : src;
}

// ***************************************************************************
CGameContextMenu::CGameContextMenu()
{
	_GroupMenu = NULL;

	_MilkoAttackDisengage = NULL;
	_ContextVal = NULL;
	_AvailablePrograms = NULL;
	_ServerTeamPresent = NULL;
	for(uint i=0;i<NUM_MISSION_OPTIONS;i++)
		_MissionOption[i]= NULL;
	_ServerInDuel = NULL;
	_ServerInPvpChallenge = NULL;
	_WebPageTitle = NULL;
	_OutpostSheet= NULL;
	_OutpostRightToBannish= NULL;
	for(uint i=0;i<BOTCHATTYPE::MaxR2MissionEntryDatabase;i++)
		_MissionRing[i]= NULL;
}


// ***************************************************************************
void		CGameContextMenu::init(const std::string &srcMenuId)
{
	std::string menuId = srcMenuId.empty() ?  std::string("game_context_menu") : srcMenuId;
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	_GroupMenu = dynamic_cast<CGroupMenu*>(CWidgetManager::getInstance()->getWindowFromId ("ui:interface:" + menuId + ""));
	_ContextVal = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:TARGET:CONTEXT_VAL", false);

	if(_GroupMenu == NULL)
	{
		nlwarning(("gamecontextmenu:init: 'ui:interface:" + menuId + "' window does not exist.").c_str());
		return;
	}

	_GroupMilkoPad = "ui:interface:milko_pad";
	if(_GroupMilkoPad != NULL)
	{
		_MilkoAttackDisengage = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:MK_ATTACK", false);
		_MilkoAttDisBut1 = "ui:interface:milko_pad:content:mode1:mode1_content:milko_actions:action5";
		_MilkoAttDisBut2 = "ui:interface:milko_pad:content:mode2:action5";
	}
	else
	{
		nlwarning("gamecontextmenu:init: 'ui:interface:milko_pad' window does not exist.");
	}

	if(_ContextVal == NULL)
	{
		nlwarning("gamecontextmenu:init: 'SERVER:TARGET:CONTEXT_VAL' node does not exist.");
		return;
	}

	// Some DB links
	_AvailablePrograms = NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TARGET:CONTEXT_MENU:PROGRAMMES");
	_ServerTeamPresent = NLGUI::CDBManager::getInstance()->getDbProp(TEAM_DB_PATH ":0:PRESENT", false);
	_ServerInDuel = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:IN_DUEL", false);
	_ServerInPvpChallenge = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:IN_PVP_CHALLENGE", false);
	_WebPageTitle = NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TARGET:CONTEXT_MENU:WEB_PAGE_TITLE", false);
	_OutpostSheet = NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TARGET:CONTEXT_MENU:OUTPOST", false);
	_OutpostRightToBannish = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:CHARACTER_INFO:PVP_OUTPOST:RIGHT_TO_BANISH", false);


	// Some text Menu
	_TextLootAction = "ui:interface:" + menuId + ":loot_action";
	_TextQuartering = "ui:interface:" + menuId + ":quartering";
	_TextAttack = "ui:interface:" + menuId + ":attack";
	_TextDuel = "ui:interface:" + menuId + ":duel";
	_TextUnDuel = "ui:interface:" + menuId + ":unduel";
	_TextPvpChallenge = "ui:interface:" + menuId + ":pvp_challenge";
	_TextUnPvpChallenge = "ui:interface:" + menuId + ":unpvp_challenge";
	_TextExchange = "ui:interface:" + menuId + ":exchange";
	_TextMount = "ui:interface:" + menuId + ":mount";
	_TextFreeLook = "ui:interface:" + menuId + ":free_look";
	_TextMove = "ui:interface:" + menuId + ":move";
	_TextStop = "ui:interface:" + menuId + ":stop";
	_TextDisengage = "ui:interface:" + menuId + ":disengage";
	_TextUnseat = "ui:interface:" + menuId + ":unseat";
	_TextInfo = "ui:interface:" + menuId + ":info";
	_TextFollow = "ui:interface:" + menuId + ":follow";
	_TextAssist = "ui:interface:" + menuId + ":assist";
	_TextInvit = "ui:interface:" + menuId + ":invit";
	_TextGuildInvit = "ui:interface:" + menuId + ":guild_invit";
	_TextQuitTeam = "ui:interface:" + menuId + ":quit_team";
	_TextAddToFriendList = "ui:interface:" + menuId + ":add_to_friend_list";
	_TextTalk = "ui:interface:" + menuId + ":talk";


	// Mission DB and Text link
	for(uint k = 0; k < NUM_MISSION_OPTIONS; ++k)
	{
		_MissionOption[k] = NLGUI::CDBManager::getInstance()->getDbProp(toString("LOCAL:TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%d:TITLE", (int) k), false);
		_TextMission[k] = toString(("ui:interface:" + menuId + ":mo%d").c_str(), (int) k);
	}

	// Mission Ring DB and Text link
	for(uint k = 0; k < BOTCHATTYPE::MaxR2MissionEntryDatabase; ++k)
	{
		_MissionRing[k]= NLGUI::CDBManager::getInstance()->getDbProp(toString("LOCAL:TARGET:CONTEXT_MENU:MISSION_RING:%d:TITLE", k), false);
		_TextMissionRing[k] = toString(("ui:interface:" + menuId + ":mr%d").c_str(), (int) k);
	}


	// BotChat menus

	_TextNews = "ui:interface:" + menuId + ":news";
	_TextTradeItem = "ui:interface:" + menuId + ":trade_item";
	_TextTradeTeleport = "ui:interface:" + menuId + ":trade_teleport";
	_TextTradeFaction = "ui:interface:" + menuId + ":trade_faction";
	_TextTradeCosmetic = "ui:interface:" + menuId + ":trade_cosmetic";
	_TextTradeGuildOptions = "ui:interface:" + menuId + ":trade_guild_options";
	_TextTradeOutpostBuilding = "ui:interface:" + menuId + ":trade_outpost_building";
	_TextTradeGuildRoleMaster = "ui:interface:" + menuId + ":trade_guild_role_master";
	_TextTradePact = "ui:interface:" + menuId + ":trade_pact";
	_TextTradePhrase = "ui:interface:" + menuId + ":trade_phrase";
	_TextChooseMission = "ui:interface:" + menuId + ":choose_mission";
	_TextCreateGuild = "ui:interface:" + menuId + ":create_guild";
	_TextDynamicMission = "ui:interface:" + menuId + ":dynamic_mission";
	_TextChooseZCCharge= "ui:interface:" + menuId + ":choose_zc_charge";
	_TextChooseBuilding= "ui:interface:" + menuId + ":choose_building";
	_TextBuyRM= "ui:interface:" + menuId + ":buy_rm";
	_TextUpgradeRM= "ui:interface:" + menuId + ":upgrade_rm";
	_TextCancelZCCharge= "ui:interface:" + menuId + ":cancel_zc_charge";
	_TextDestroyBuilding= "ui:interface:" + menuId + ":destroy_building";
	_TextOutpostState= "ui:interface:" + menuId + ":outpost_state";
	_TextWebPage= "ui:interface:" + menuId + ":web_page";
	_TextOutpostBanishPlayer= "ui:interface:" + menuId + ":outpost_bannish_player";
	_TextOutpostBanishGuild= "ui:interface:" + menuId + ":outpost_bannish_guild";


	// Pack Animals
	_TextPAFollow= "ui:interface:" + menuId + ":pa_follow";
	_TextPAStop= "ui:interface:" + menuId + ":pa_stop";
	_TextPAFree= "ui:interface:" + menuId + ":pa_free";
	_TextPAEnterStable= "ui:interface:" + menuId + ":pa_enter_stable";

	// Forage source
	_TextExtractRM= "ui:interface:" + menuId + ":extract_rm";

	// Build Spire
	_TextBuildTotem= "ui:interface:" + menuId + ":build_totem";
}


// ***************************************************************************
// tmp for debug
bool ForceTalkWithBot = false;
bool ForceTalkWithPlayer = false;

// ***************************************************************************
void		CGameContextMenu::update()
{
	H_AUTO (RZ_Interface_updateContextMenu )

	if( !_ContextVal || !_GroupMenu )
		return;

	CProperties propValidation = CProperties(_ContextVal->getValue16());

	// botChatPrograms
	uint32 availablePrograms = filterAvailablePrograms((uint32) _AvailablePrograms->getValue32());

	CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());

	// Milko Pad
	if ((_GroupMilkoPad != NULL) && (_GroupMilkoPad->getActive()))
	{
		if (canDisengage())
		{
			_MilkoAttackDisengage->setValue32(1);
			if (_MilkoAttDisBut1)
				_MilkoAttDisBut1->setHardText("uiMk_action5_1");
			if (_MilkoAttDisBut2)
				_MilkoAttDisBut2->setHardText("uiMk_action5_1");
		}
		else
		{
			_MilkoAttackDisengage->setValue32(0);
			if (_MilkoAttDisBut1)
				_MilkoAttDisBut1->setHardText("uiMk_action5_0");
			if (_MilkoAttDisBut2)
				_MilkoAttDisBut2->setHardText("uiMk_action5_0");
		}
	}

	// Is the menu active
	if(!_GroupMenu->getActive())
		return;

	updateContextMenuMissionsOptions( UserEntity->isRiding() );

	updateContextMenuWebPage(availablePrograms);

	updateContextMenuOutpostState(availablePrograms);

	updateContextMenuOutpostBanish();

	updateContextMenuMissionRing();

	setupContextMenuCantTalk(); // can't talk by default

	// If mode Combat (no talk, no give, no mount, no extract_rm)
	if(UserEntity->isFighting())
	{
		// Loot
		if(_TextLootAction)
			_TextLootAction->setActive(false);
		// Quartering
		if(_TextQuartering)
			_TextQuartering->setActive(false);
		// Disable talk, unless with a player
		if (ForceTalkWithBot) // tmp
		{
			updateContextMenuTalkEntries(availablePrograms);
		}
		else if (ForceTalkWithPlayer)
		{
			setupContextMenuTalkWithPlayer();
		}
		else
		if(selection && selection->Type == CEntityCL::Player)
		{
			setupContextMenuTalkWithPlayer();
		}

		// Exchange
		if(_TextExchange)
			_TextExchange->setActive(false);
		// Disable mount
		if(_TextMount)
			_TextMount->setActive(false);


		// Disable FreeLook.
		if(_TextFreeLook)
			_TextFreeLook->setActive(false);
		// Disable unseat.
		if(_TextUnseat)
			_TextUnseat->setActive(false);
		// Disable extraction
		if(_TextExtractRM)
			_TextExtractRM->setActive(false);
	}
	// Mount
	else if(UserEntity->isRiding())
	{
		// Loot
		if(_TextLootAction)
			_TextLootAction->setActive(false);
		// Quartering
		if(_TextQuartering)
			_TextQuartering->setActive(false);
		// No talk when riding
		// No exchange when riding
		if(_TextExchange)
			_TextExchange->setActive(false);

		// Disable mount
		if(_TextMount)
			_TextMount->setActive(false);


		// Disable Free Look
		if(_TextFreeLook)
			_TextFreeLook->setActive(false);
		// Enable unseat
		if(_TextUnseat)
			_TextUnseat->setActive(true);
		// Disable extraction
		if(_TextExtractRM)
			_TextExtractRM->setActive(false);
	}
	// Not in combat or riding mode.
	else
	{
		// Loot
		if(_TextLootAction)
		{
			_TextLootAction->setActive(false);
			// Action possible only if the client is not already busy and the selection is able to do this with you..
			if(selection && selection->isDead() && (selection->properties().lootable() || selection->properties().liftable())
			   && !R2::getEditor().isDMing()
			  )
				_TextLootAction->setActive(true);
		}
		// Quartering
		if(_TextQuartering)
		{
			_TextQuartering->setActive(false);
			// Action possible only if the client is not already busy and the selection is able to do this with you..
			if(selection && selection->isDead() && selection->properties().harvestable() && !R2::getEditor().isDMing())
				_TextQuartering->setActive(true);
		}
		// talk ?
		if (ForceTalkWithBot) // tmp
		{
			updateContextMenuTalkEntries(availablePrograms);
		}
		else if (ForceTalkWithPlayer)
		{
			setupContextMenuTalkWithPlayer();
		}
		else
		if(UserEntity->selection() != UserEntity->slot())	// Talking again to yourself ?
		{
			if(selection)
			{
				if (selection->Type == CEntityCL::Player)
				{
					setupContextMenuTalkWithPlayer();
				}
				else
				if(selection->properties().talkableTo())
				{
					if(!CBotChatManager::getInstance()->getCurrPage())	// Not talking
					{
						// Can talk with bot if not too far
						CVectorD vect1 = selection->pos();
						CVectorD vect2 = UserEntity->pos();
						double distanceSquare = pow(vect1.x-vect2.x,2) + pow(vect1.y-vect2.y,2);
						if (distanceSquare <= MaxTalkingDistSquare)
						{
							updateContextMenuTalkEntries(availablePrograms);
						}
						// Special case for buildings
						if (distanceSquare <= MaxTalkingOutpostBuildingDistSquare)
							_OkTextTradeOutpostBuilding = ((availablePrograms & (1 << BOTCHATTYPE::TradeOutpostBuilding)) != 0);
					}
				}
			}
		}
		// Exchange
		if(_TextExchange)
		{
			// Action possible only if the client is not already busy and the selection is able to do this with you..
			if(selection && selection->properties().canExchangeItem())
				_TextExchange->setActive(!UserEntity->isBusy());
			else
				_TextExchange->setActive(false);
		}
		// Enable Free Look
		if(_TextFreeLook)
			_TextFreeLook->setActive(true);
		// Disable unseat
		if(_TextUnseat)
			_TextUnseat->setActive(false);
		// Enable extraction for a forage source
		if(_TextExtractRM)
		{
			_TextExtractRM->setActive(false);
			if ( selection && selection->isForageSource() )
				_TextExtractRM->setActive(true);
		}
	}

	// Enable attack mode
	if (_TextAttack)
		_TextAttack->setActive(canAttack());

	if (_TextDuel && _TextUnDuel)
	{
		if ((!UserEntity->isRiding()) && (_ServerInDuel->getValue8() != 0))
		{
			_TextDuel->setActive(false);
			_TextUnDuel->setActive(true);
		}
		else if (canDuel())
		{
			_TextDuel->setActive(true);
			_TextUnDuel->setActive(false);
		}
		else
		{
			_TextDuel->setActive(false);
			_TextUnDuel->setActive(false);
		}
	}

	if (_TextPvpChallenge && _TextUnPvpChallenge)
	{
		if ((!UserEntity->isRiding()) && (_ServerInPvpChallenge->getValue8() != 0))
		{
			_TextPvpChallenge->setActive(false);
			_TextUnPvpChallenge->setActive(true);
		}
		else if (canPvpChallenge())
		{
			_TextPvpChallenge->setActive(true);
			_TextUnPvpChallenge->setActive(false);
		}
		else
		{
			_TextPvpChallenge->setActive(false);
			_TextUnPvpChallenge->setActive(false);
		}
	}

	// Enable disengage mode
	if (_TextDisengage)
		_TextDisengage->setActive(canDisengage());

	// Enable Autowalk
	if(_TextMove)
		_TextMove->setActive(!UserControls.autowalkState());
	// Enable Stop Autowalk
	if(_TextStop)
		_TextStop->setActive(UserControls.autowalkState());

	// Disable player properties
	if(_TextInfo)
		_TextInfo->setActive(selection && (!selection->isForageSource()));

	// Follow
	if(_TextFollow)
	{
		// Action possible only if the client is not already busy and the selection is able to do this with you..
		_TextFollow->setActive(
			selection &&
			(selection->slot() != UserEntity->slot()) &&
			(!selection->isForageSource()) &&
			(selection->isDead()==false) &&
			(((availablePrograms & (1 << BOTCHATTYPE::DontFollow)) == 0)));
		// See also below for mount/packer
	}

	// Assist
	if(_TextAssist)
	{
		// Action possible only if the client is not already busy and the selection is able to do this with you..
		_TextAssist->setActive(!R2::getEditor().isDMing() && selection && (selection->slot() != UserEntity->slot()) && (!selection->isForageSource()) && (selection->isDead()==false));
		// See also below for mount/packer
	}

	// Invite
	if(_TextInvit)
	{
		bool invitable = false;
		// User should not be flagged as invitable by himself, so no need to check that selection is not the user
		if(selection && selection->properties().invitable() && propValidation.invitable() )
		{
			invitable = true;
		}
		// Grey/Ungrey the entry.
		_TextInvit->setActive(invitable);
	}

	// Guild Invit
	if(_TextGuildInvit)
	{
		// if server OK.
		if(filterAvailablePrograms(_AvailablePrograms->getValue32())&(1 << BOTCHATTYPE::GuildInviteFlag))
			_TextGuildInvit->setActive (true);
		else
			_TextGuildInvit->setActive (false);
	}

	// Active "Quit Team" if you are a member of one.
	if(_TextQuitTeam)
	{
		if (_ServerTeamPresent)
		{
			_TextQuitTeam->setActive( _ServerTeamPresent->getValueBool() );
		}
	}

	// Invite to friend list (only if target is a player, and not a friend already)
	if(_TextAddToFriendList)
	{
		_TextAddToFriendList->setActive(false);
		if (selection && selection->Type == CEntityCL::Player)
		{
			if (!selection->getEntityName().empty())
			{
				sint peopleIndex = PeopleInterraction.FriendList.getIndexFromName(selection->getEntityName());
				if (peopleIndex == -1) // not already in friend list
				{
					_TextAddToFriendList->setActive(true);
				}
			}
		}
	}

	// Pack/Mount Animal
	if ( _TextMount )
		_TextMount->setActive(false);
	if(selection && (selection->isUserPackAnimal() || (selection->isUserMount())))
	{
		// Get the index of the selected animal, if owned by the player
		sint animalIndex;
		if ( ! selection->getPackAnimalIndexInDB( animalIndex ) )
			animalIndex = -1;

		// Disable 'Mount' if server says so
		CViewTextMenu *pTextMount = NULL;
		if ( propValidation.mountable() )
			pTextMount = _TextMount;
		else if (_TextMount)
			_TextMount->setActive( false ); // pTextMount remains NULL

		// Enable/disable various menu items
		bool ok = testMenuOptionForPackAnimal( selection, animalIndex, true, _TextPAFollow, _TextPAStop, _TextPAFree,
			_TextPAEnterStable, NULL /*no 'leave stable' in context menu*/, pTextMount,
			NULL /*unmount always active in context menu when the character is riding*/);

		// Follow & assist special case
		if ( _TextFollow )
			_TextFollow->setActive( false ); // can't follow own pet
		if ( _TextAssist )
			_TextAssist->setActive( false ); // can't assist a pet

		// Write the selected pack animal, for future "beast_order" action handler
		if(ok)
		{
			CInterfaceManager	*pIM= CInterfaceManager::getInstance();
			CCDBNodeLeaf		*node= NLGUI::CDBManager::getInstance()->getDbProp("UI:GCM_BEAST_SELECTED", false);
			// beast_order AH start with 1 (0 for ALL beasts)
			if(node)
				node->setValue32(animalIndex+1);
		}
	}
	else
	{
		if (_TextPAFollow)
			_TextPAFollow->setActive(false);
		if (_TextPAStop)
			_TextPAStop->setActive(false);
		if (_TextPAFree)
			_TextPAFree->setActive(false);
		if (_TextPAEnterStable)
			_TextPAEnterStable->setActive(false);
	}

	// build spire
	if(_TextBuildTotem)
	{
		_TextBuildTotem->setActive(false);
		if( availablePrograms & ( 1 << BOTCHATTYPE::Totem ) )
		{
			_TextBuildTotem->setActive(true);
		}
	}

	// Apply real activation of Talk Texts.
	applyTextTalk();
}

// ***************************************************************************
bool CGameContextMenu::canAttack()
{
	if (R2::getEditor().isDMing()) return false;
	bool ret = false;
	CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());

	if (UserEntity->isFighting() || UserEntity->isRiding())
		return ret;

	// To enter in combat, the user should not be talking.
	if(!CBotChatManager::getInstance()->getCurrPage())	// Not talking
	{
		// If there is a selection.
		if (selection && selection->isDead() == false)
		{
			uint32 availablePrograms = (uint32) filterAvailablePrograms(_AvailablePrograms->getValue32());

			if(selection->properties().attackable() ||  (availablePrograms & (1<<BOTCHATTYPE::Attackable)))
				ret = true;
		}
	}
	return ret;
}

// ***************************************************************************
bool CGameContextMenu::canDuel()
{
	if (UserEntity->isFighting() || UserEntity->isRiding())
		return false;

	// To enter in combat, the user should not be talking.
	if(CBotChatManager::getInstance()->getCurrPage())	// Talking ?
		return false;

	CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());
	if (selection == NULL)
		return false;

	if (selection->isPlayer())
	if (selection->isDead() == false)
		return true;

	return false;
}

// ***************************************************************************
bool CGameContextMenu::canPvpChallenge()
{
	if (UserEntity->isFighting() || UserEntity->isRiding())
		return false;

	// To enter in combat, the user should not be talking.
	if(CBotChatManager::getInstance()->getCurrPage())	// Talking ?
		return false;

	CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());
	if (selection == NULL)
		return false;

	if (selection->isPlayer())
		if (selection->isDead() == false)
			return true;

	return false;
}

// ***************************************************************************
bool CGameContextMenu::canDisengage()
{
	if (UserEntity->isFighting())
		return true;
	else
		return false;
}

// ***************************************************************************
bool CGameContextMenu::canTeamKick()
{
	CEntityCL *selection = EntitiesMngr.entity(UserEntity->selection());
	if (selection != NULL)
	{
		sint n = PeopleInterraction.TeamList.getIndexFromName(selection->getEntityName());
		if (n >= 0)
			return true;
	}
	return false;
}

// ***************************************************************************
void CGameContextMenu::updateContextMenuMissionsOptions( bool forceHide )
{
	for(uint k = 0; k < NUM_MISSION_OPTIONS; ++k)
	{
		CViewTextMenu *pVTM = _TextMission[k];
		if (pVTM)
		{
			if ( _MissionOption[k] && (!forceHide) )
			{
				uint32 textID = (uint32) _MissionOption[k]->getValue32();
				if (textID)
				{
					ucstring result;
					bool res = STRING_MANAGER::CStringManagerClient::instance()->getDynString(textID, result);
					if (!res)
					{
						result = NLMISC::CI18N::get("uiMissionOptionNotReceived");
					}
					pVTM->setText(result);
					pVTM->setActive(true);
				}
				else
				{
					pVTM->setText(ucstring(""));
					pVTM->setActive(false);
				}
			}
			else
			{
				pVTM->setText(ucstring(""));
				pVTM->setActive(false);
			}
		}
	}
}

// ***************************************************************************
void CGameContextMenu::updateContextMenuWebPage(uint options)
{
	// Show the web page (title must be present)
	_OkTextWebPage= ((options & (1 << BOTCHATTYPE::WebPageFlag)) != 0);
	if(_OkTextWebPage)
		_OkTextWebPage= _WebPageTitle && _WebPageTitle->getValue32()!=0;

	// Change the WebPage text, when available
	CViewTextMenu *pVTM = _TextWebPage;
	if (pVTM)
	{
		if ( _WebPageTitle )
		{
			uint32 textID = (uint32) _WebPageTitle->getValue32();
			if (textID)
			{
				ucstring result;
				bool res = STRING_MANAGER::CStringManagerClient::instance()->getDynString(textID, result);
				if (!res)
				{
					result = NLMISC::CI18N::get("uiWebPageNameNotReceived");
				}
				pVTM->setText(result);
			}
			else
			{
				pVTM->setText(ucstring(""));
			}
		}
		else
		{
			pVTM->setText(ucstring(""));
		}
	}

	// apply the active
	if (_TextWebPage) _TextWebPage->setActive(_OkTextWebPage);
}

// ***************************************************************************
void CGameContextMenu::updateContextMenuOutpostState(uint options)
{
	CSheetId	outpostSheet;

	// Outpost ok?
	_OkTextOutpostState= false;
	if ( (options & (1 << BOTCHATTYPE::OutpostFlag)) != 0 && _OutpostSheet)
	{
		// check that the outpost sheet is correct
		outpostSheet= _OutpostSheet->getValue32();
		CEntitySheet	*s= SheetMngr.get(outpostSheet);
		if(s && s->Type == CEntitySheet::OUTPOST)
			_OkTextOutpostState= true;
	}

	// Fill the text
	if(_OkTextOutpostState)
	{
		CViewTextMenu *pVTM = _TextOutpostState;
		if (pVTM)
			pVTM->setText(ucstring(STRING_MANAGER::CStringManagerClient::getOutpostLocalizedName(outpostSheet)));
	}

	// apply the active
	if (_TextOutpostState) _TextOutpostState->setActive(_OkTextOutpostState);
}

// ***************************************************************************
void	CGameContextMenu::updateContextMenuOutpostBanish()
{
	bool	okForBanPlayer= false;
	bool	okForBanGuild= false;

	// Test if the player can ban the targeted player / player's guild
	extern void	outpostTestUserCanBan(CCDBNodeLeaf *dbBanRight, bool &, bool &);
	outpostTestUserCanBan(_OutpostRightToBannish, okForBanPlayer, okForBanGuild);

	// validate text
	if(_TextOutpostBanishPlayer)	_TextOutpostBanishPlayer->setActive(okForBanPlayer);
	if(_TextOutpostBanishGuild)		_TextOutpostBanishGuild->setActive(okForBanGuild);
}

// ***************************************************************************
void CGameContextMenu::updateContextMenuMissionRing()
{
	for(uint k = 0; k < BOTCHATTYPE::MaxR2MissionEntryDatabase; ++k)
	{
		CViewTextMenu *pVTM = _TextMissionRing[k];
		if (pVTM)
		{
			uint32	textId= 0;
			if ( _MissionRing[k] )
				textId= _MissionRing[k]->getValue32();
			// if the textId is ok and Flag is set.
			if ( textId )
			{
				ucstring result;
				bool res = STRING_MANAGER::CStringManagerClient::instance()->getDynString(textId, result);
				if (!res)
				{
					result = NLMISC::CI18N::get("uiMissionRingNameNotReceived");
				}
				pVTM->setText(result);
				pVTM->setActive(true);
			}
			else
			{
				pVTM->setText(ucstring(""));
				pVTM->setActive(false);
			}
		}
	}
}

// ***************************************************************************
void CGameContextMenu::updateContextMenuTalkEntries(uint options)
{
	if (ClientCfg.Local)
	{
		options = std::numeric_limits<uint>::max(); // in local mode, force all options to be shown (for debug)
	}
	// news
	_OkTextNews= ((options & (1 << BOTCHATTYPE::NewsFlag)));
	// trade
	_OkTextTradeItem= ((options & (1 << BOTCHATTYPE::TradeItemFlag)) != 0);
 	_OkTextTradeTeleport= ((options & (1 << BOTCHATTYPE::TradeTeleportFlag)) != 0);
 	_OkTextTradeFaction= ((options & (1 << BOTCHATTYPE::TradeFactionFlag)) != 0);
	_OkTextTradeCosmetic= ((options & (1 << BOTCHATTYPE::TradeCosmeticFlag)) != 0);
	_OkTextTradeGuildOptions = ((options & (1 << BOTCHATTYPE::TradeBuildingOptions)) != 0);
	_OkTextTradeOutpostBuilding = ((options & (1 << BOTCHATTYPE::TradeOutpostBuilding)) != 0);
	_OkTextTradeGuildRoleMaster = ((options & (1 << BOTCHATTYPE::GuildRoleMaster)) != 0);
	_OkTextTradePact= ((options & (1 << BOTCHATTYPE::TradePactFlag)) != 0);
	_OkTextTradePhrase= ((options & (1 << BOTCHATTYPE::TradePhraseFlag)) != 0);
	// choose mission
	_OkTextChooseMission= ((options & (1 << BOTCHATTYPE::ChooseMissionFlag)) != 0);
	// create guild
	_OkTextCreateGuild= ((options & (1 << BOTCHATTYPE::CreateGuildFlag)) != 0);
	// New Yoyo BotChat
	_OkTextChooseZCCharge= ((options & (1 << BOTCHATTYPE::AskGuildDutyFlag)) != 0);
	_OkTextCancelZCCharge= ((options & (1 << BOTCHATTYPE::CancelGuildDutyFlag)) != 0);

	/*_OkTextBuyRM= ((options & (1 << BOTCHATTYPE::)) != 0);
	_OkTextUpgradeRM= ((options & (1 << BOTCHATTYPE::)) != 0);
	_OkTextDestroyBuilding= ((options & (1 << BOTCHATTYPE::)) != 0);*/
	// TODO BOTCHAT: RM Buy/Upgrade.
	// TODO BOTCHAT: Declare war no more through "Mission Option"
	// TODO BOTCHAT: Destroy Building only if already created
	// TODO BOTCHAT: ZCBuilder is deprecated, as is _OkTextChooseBuilding
	// TODO BOTCHAT: _OkTextDynamicMission is deprecated
	_OkTextDynamicMission= false;
	_OkTextBuyRM= false;
	_OkTextUpgradeRM= false;
	_OkTextChooseBuilding= false;
	_OkTextDestroyBuilding= _OkTextChooseBuilding;
}


// ***************************************************************************
void CGameContextMenu::setupContextMenuCantTalk()
{
	updateContextMenuTalkEntries(0);
	_OkTextTalk= false;
}

// ***************************************************************************
void CGameContextMenu::setupContextMenuTalkWithPlayer()
{
	_OkTextTalk= true;
}

// ***************************************************************************
void CGameContextMenu::applyTextTalk()
{
	if (_TextNews) _TextNews->setActive(_OkTextNews);
	if (_TextTradeItem) _TextTradeItem->setActive(_OkTextTradeItem);
	if (_TextTradeTeleport) _TextTradeTeleport->setActive(_OkTextTradeTeleport);
	if (_TextTradeFaction) _TextTradeFaction->setActive(_OkTextTradeFaction);
	if (_TextTradeCosmetic) _TextTradeCosmetic->setActive(_OkTextTradeCosmetic);
	if (_TextTradeGuildOptions) _TextTradeGuildOptions->setActive(_OkTextTradeGuildOptions);
	if (_TextTradeOutpostBuilding) _TextTradeOutpostBuilding->setActive(_OkTextTradeOutpostBuilding);
	if (_TextTradeGuildRoleMaster) _TextTradeGuildRoleMaster->setActive(_OkTextTradeGuildRoleMaster);
	if (_TextTradePact) _TextTradePact->setActive(_OkTextTradePact);
	if (_TextTradePhrase) _TextTradePhrase->setActive(_OkTextTradePhrase);
	if (_TextChooseMission) _TextChooseMission->setActive(_OkTextChooseMission);
	if (_TextCreateGuild)
	{
		_TextCreateGuild->setActive(_OkTextCreateGuild);
		// If the player is already in a guild set the text grayed
		_TextCreateGuild->setGrayed(CGuildManager::getInstance()->isInGuild());
	}

	if (_TextDynamicMission) _TextDynamicMission->setActive(_OkTextDynamicMission);
	if (_TextTalk) _TextTalk->setActive(_OkTextTalk);
	if (_TextChooseZCCharge) _TextChooseZCCharge->setActive(_OkTextChooseZCCharge);
	if (_TextChooseBuilding) _TextChooseBuilding->setActive(_OkTextChooseBuilding);
	if (_TextBuyRM) _TextBuyRM->setActive(_OkTextBuyRM);
	if (_TextUpgradeRM) _TextUpgradeRM->setActive(_OkTextUpgradeRM);
	if (_TextCancelZCCharge) _TextCancelZCCharge->setActive(_OkTextCancelZCCharge);
	if (_TextDestroyBuilding) _TextDestroyBuilding->setActive(_OkTextDestroyBuilding);
}

// ***************************************************************************

/**
 * Enable/disable various menu items for animals.
 * selectedAnimalInVision can be NULL (out of vision)
 * index can be -1 (no owned animal)
 * Any CViewTextMenu* can be NULL
 */
bool testMenuOptionForPackAnimal( CEntityCL* selectedAnimalInVision, uint index, bool clearAll,
								  CViewTextMenu *pFollow, CViewTextMenu *pStop, CViewTextMenu *pFree,
								  CViewTextMenu *pEnterStable, CViewTextMenu *pLeaveStable,
								  CViewTextMenu *pMount, CViewTextMenu *pUnmount )
{
	if ( clearAll )
	{
		// Disable and gray all by default
		if(pFollow)			{ pFollow->setActive(false); pFollow->setGrayed(true); }
		if(pStop)			{ pStop->setActive(false); pStop->setGrayed(true); }
		if(pFree)			{ pFree->setActive(false); pFree->setGrayed(true); }
		if(pEnterStable)	{ pEnterStable->setActive(false); pEnterStable->setGrayed(true); }
		if(pLeaveStable)	{ pLeaveStable->setActive(false); pLeaveStable->setGrayed(true); }
		if(pMount)			{ pMount->setActive(false); pMount->setGrayed(true); }
		if(pUnmount)		{ pUnmount->setActive(false); pUnmount->setGrayed(true); }
	}

	// Don't enable anything if index not found (e.g. the character is not the owner of the animal)
	if ( (index == (uint)-1) || (!UserEntity) )
		return false;

	// Get animal status and type
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:STATUS", index), false);
	if(!node)	return false;
	ANIMAL_STATUS::EAnimalStatus	status= (ANIMAL_STATUS::EAnimalStatus)node->getValue32();
	node= NLGUI::CDBManager::getInstance()->getDbProp(toString("SERVER:PACK_ANIMAL:BEAST%d:TYPE", index), false);
	if(!node)	return false;
	ANIMAL_TYPE::EAnimalType		anitype= (ANIMAL_TYPE::EAnimalType)node->getValue32();

	// COMMON PART FOR ALL TYPES OF ANIMAL

	// Is the character mounted on the animal?
	bool userIsMountedOnIt = false;
	if ( selectedAnimalInVision && (anitype == ANIMAL_TYPE::Mount) && UserEntity->isRiding() )
	{
		uint8 mountSlot = (uint8)IngameDbMngr.getProp( "Entities:E" + toString(UserEntity->slot()) + ":P" + toString(CLFECOMMON::PROPERTY_ENTITY_MOUNTED_ID));
		userIsMountedOnIt = (mountSlot == selectedAnimalInVision->slot());
	}

	bool onLandscape = ANIMAL_STATUS::isInLandscape(status);
	double distanceSquare = 0;
	if ( selectedAnimalInVision )
	{
		CVectorD vect1 = selectedAnimalInVision->pos();
		CVectorD vect2 = UserEntity->pos();
		distanceSquare = pow(vect1.x-vect2.x,2) + pow(vect1.y-vect2.y,2);
	}


	// Enable option only if pack animal present
	if(ANIMAL_STATUS::isSpawned(status))
	{
		bool	canEnterLeaveStable= ANIMAL_STATUS::canEnterLeaveStable(status);

		if (!userIsMountedOnIt)
		{
			if(pFollow && onLandscape)	pFollow->setActive(true);
			if(pStop && onLandscape)	pStop->setActive(true);

			// Enter/leave stable are only displayed when near a stable ('canEnterLeaveStable')
			if(pEnterStable && onLandscape && canEnterLeaveStable)	pEnterStable->setActive(true);
			if(pLeaveStable && ANIMAL_STATUS::isInStable(status) && canEnterLeaveStable)	pLeaveStable->setActive(true);

			// Can always free, dead/alive, in or out stable animals
			if(pFree)
			{
				pFree->setActive(true);
				pFree->setGrayed(false);
			}
		}

		// If alive (and close to the character), ungray all
		if(ANIMAL_STATUS::isAlive(status))
		{
			if (onLandscape && selectedAnimalInVision && (distanceSquare <= MaxAnimalCommandDistSquare))
			{
				if(pFollow)			pFollow->setGrayed(false);
				if(pStop)			pStop->setGrayed(false);
				if(pEnterStable)	pEnterStable->setGrayed(false);
			}
			if(pLeaveStable)		pLeaveStable->setGrayed(false);
		}
	}

	// Specific part for mountable animal
	if ((anitype == ANIMAL_TYPE::Mount) &&
		(ANIMAL_STATUS::isSpawned(status)) && onLandscape)
	{
		if ( userIsMountedOnIt )
		{
			// Always allow to unmount the mounted mount
			if(pUnmount)			pUnmount->setActive(true);
			if(pUnmount)			pUnmount->setGrayed(false);
		}
		else
		{
			if(pMount &&
			   ((!selectedAnimalInVision) || selectedAnimalInVision->properties().mountable()))
			{
				// Display 'Mount' if the entity is of mountable type
				pMount->setActive(true);

				// Enable 'Mount' if the character is not busy and the entity is alive and close enough
				if(ANIMAL_STATUS::isAlive(status) && (!UserEntity->isRiding()) &&
				   (!UserEntity->isFighting()) && (!UserEntity->isBusy()))
				{
					if(selectedAnimalInVision && (distanceSquare <= MaxAnimalCommandDistSquare))
					{
						pMount->setGrayed(false);
					}
				}
			}
		}
	}

	return true;
}
