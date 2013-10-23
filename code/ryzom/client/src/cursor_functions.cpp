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
#include "cursor_functions.h"
#include "user_entity.h"
#include "entities.h"
#include "net_manager.h"
#include "interface_v3/interface_manager.h"
#include "interface_v3/interface_3d_scene.h"
#include "nel/gui/group_container.h"
#include "sheet_manager.h"
#include "interface_v3/inventory_manager.h"
#include "interface_v3/guild_manager.h"
#include "nel/3d/u_instance.h"
#include "main_loop.h"
// GAME SHARE
#include "game_share/bot_chat_types.h"
// r2
#include "r2/editor.h"


///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;


////////////
// GLOBAL //
////////////
// Contextual Cursor.
CContextualCursor		ContextCur;
CLFECOMMON::TCLEntityId	SlotUnderCursor;
uint32 MissionId = 0;
uint32 MissionRingId = 0;
UInstance selectedInstance;
const UInstance noSelectedInstance;
string selectedInstanceURL;
static NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> s_UserCharFade;


///////////////
// FUNCTIONS //
///////////////
void checkUnderCursor();
void contextSelect			(bool rightClick, bool dblClick);
void contextAttack			(bool rightClick, bool dblClick);
void contextQuarter			(bool rightClick, bool dblClick);
void contextLoot			(bool rightClick, bool dblClick);
void contextPickUp			(bool rightClick, bool dblClick);
void contextTradeItem		(bool rightClick, bool dblClick);
void contextTradePhrase		(bool rightClick, bool dblClick);
void contextDynamicMission	(bool rightClick, bool dblClick);
void contextStaticMission	(bool rightClick, bool dblClick);
void contextTradePact		(bool rightClick, bool dblClick);
void contextCreateGuild		(bool rightClick, bool dblClick);
void contextNews			(bool rightClick, bool dblClick);
void contextTeleport		(bool rightClick, bool dblClick);
void contextFaction 		(bool rightClick, bool dblClick);
void contextCosmetic		(bool rightClick, bool dblClick);
void contextTalk			(bool rightClick, bool dblClick);
void contextExtractRM		(bool rightClick, bool dblClick);
void contextMission			(bool rightClick, bool dblClick);
void contextWebPage			(bool rightClick, bool dblClick);
void contextWebIG			(bool rightClick, bool dblClick);
void contextRingMission		(bool rightClick, bool dblClick);
void contextOutpost			(bool rightClick, bool dblClick);
void contextBuildTotem		(bool rightClick, bool dblClick);
//-----------------------------------------------
// initContextualCursor :
// Initialize Contextual Cursor.
//-----------------------------------------------
void initContextualCursor()
{
	// Create states of the Contextual Cursor.
	ContextCur.add(false,	"STAND BY",			string("curs_default.tga"),			0.0f,	checkUnderCursor,	0);
	ContextCur.add(false,	"SELECTABLE",		string("curs_pick.tga"),			0.0f,	checkUnderCursor,	contextSelect);
//	ContextCur.add(false,	"ATTACK",			string("hand_attack.TGA"),			0.0f,	checkUnderCursor,	contextAttack);
//	ContextCur.add(false,	"QUARTER",			string("hand_harvest.TGA"),			0.0f,	checkUnderCursor,	contextQuarter);
//	ContextCur.add(false,	"LOOT",				string("hand_loot.TGA"),			0.0f,	checkUnderCursor,	contextLoot);
//	ContextCur.add(false,	"PICKUP",			string("hand_loot.TGA"),			0.0f,	checkUnderCursor,	contextPickUp);
//	ContextCur.add(false,	"TALK",				string("hand_speak.TGA"),			0.0f,	checkUnderCursor,	contextTalk);

	// String Cursors
	ContextCur.add(true,	"ATTACK",			string("uimGcmAttack"),				0.0f,	checkUnderCursor,	contextAttack);
	ContextCur.add(true,	"QUARTER",			string("uimGcmQuartering"),			0.0f,	checkUnderCursor,	contextQuarter);
	ContextCur.add(true,	"LOOT",				string("uimGcmLoot"),				0.0f,	checkUnderCursor,	contextLoot);
	ContextCur.add(true,	"PICKUP",			string("uimGcmPickUp"),				0.0f,	checkUnderCursor,	contextPickUp);
	ContextCur.add(true,	"TRADE ITEM",		string("uimGcmTrade"),				0.0f,	checkUnderCursor,	contextTradeItem);
	ContextCur.add(true,	"TRADE PHRASE",		string("uimGcmTradePhase"),			0.0f,	checkUnderCursor,	contextTradePhrase);
	ContextCur.add(true,	"DYNAMIC MISSION",	string("uimGcmDynamicMission"),		0.0f,	checkUnderCursor,	contextDynamicMission);
	ContextCur.add(true,	"STATIC MISSION",	string("uimGcmMissions"),			0.0f,	checkUnderCursor,	contextStaticMission);
	ContextCur.add(true,	"TRADE PACT",		string("uimGcmTradePact"),			0.0f,	checkUnderCursor,	contextTradePact);
	ContextCur.add(true,	"CREATE GUILD",		string("uimGcmCreateGuild"),		0.0f,	checkUnderCursor,	contextCreateGuild);
	ContextCur.add(true,	"NEWS",				string("uimGcmNews"),				0.0f,	checkUnderCursor,	contextNews);
	ContextCur.add(true,	"TELEPORT",			string("uimGcmTeleport"),			0.0f,	checkUnderCursor,	contextTeleport);
	ContextCur.add(true,	"FACTION",			string("uimGcmFaction"),			0.0f,	checkUnderCursor,	contextFaction);
	ContextCur.add(true,	"COSMETIC",			string("uimGcmCosmetic"),			0.0f,	checkUnderCursor,	contextCosmetic);
	//	ContextCur.add(true,	"TALK",				string("uimGcmTalk"),				0.0f,	checkUnderCursor,	contextTalk);
	ContextCur.add(true,	"EXTRACT_RM",		string("uimGcmExtractRM"),			0.0f,	checkUnderCursor,	contextExtractRM);
	ContextCur.add(true,	"MISSION",			string(""),							0.0f,	checkUnderCursor,	contextMission);
	ContextCur.add(true,	"WEB PAGE",			string(""),							0.0f,	checkUnderCursor,	contextWebPage);
	ContextCur.add(true,	"WEBIG",			string(""),							0.0f,	checkUnderCursor,	contextWebIG);
	ContextCur.add(true,	"OUTPOST",			string(""),							0.0f,	checkUnderCursor,	contextOutpost);
	ContextCur.add(true,	"RING MISSION",		string(""),							0.0f,	checkUnderCursor,	contextRingMission);
	ContextCur.add(true,	"BUILD_TOTEM",		string("uimGcmChooseBuilding"),		0.0f,	checkUnderCursor,	contextBuildTotem);
}// initContextualCursor //

//-----------------------------------------------
// releaseContextualCursor :
// Release cursors.
//-----------------------------------------------
void releaseContextualCursor()
{
	ContextCur.release();
}// releaseContextualCursor //


//-----------------------------------------------
// testMissionOption
//-----------------------------------------------
static bool testMissionOption(sint32 priorityWanted)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	uint32 textID = 0;
	sint32 bestPriority = 0;
	for(uint k = 0; k < NUM_MISSION_OPTIONS; ++k)
	{
		std::string nodeName = toString("LOCAL:TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%d:PRIORITY", (int) k);
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(nodeName, false);
		if(pNL)
		{
			sint32 priority = pNL->getValue32();
			if(priority == priorityWanted)
			{
				// Special for priority 2 (yoyo: don't know why...). fail if more than one choice
				if(priorityWanted != 2 || textID==0)
				{
					nodeName = toString("LOCAL:TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%d:TITLE", (int) k);
					pNL = NLGUI::CDBManager::getInstance()->getDbProp(nodeName, false);
					if(pNL && pNL->getValue32())
					{
						textID = pNL->getValue32();
						MissionId = k;
						bestPriority = priority;
						// general case: found so stop. priority==2: must check all options
						if(priorityWanted!=2)
							break;
					}
				}
				// here means that priorityWanted==2 and textId!=0
				else
				{
					// More than a choice for priorityWanted=2 mission options so nothing to show.
					textID=0;
					break;
				}
			}
		}
		else
			nlwarning("checkUnderCursor: entry '%s' dose not exist.", nodeName.c_str());
	}
	// Get the Text for the cursor
	if(textID)
	{
		ucstring result;
		bool res = STRING_MANAGER::CStringManagerClient::instance()->getDynString(textID, result);
		if (!res)
			result = NLMISC::CI18N::get("uiMissionOptionNotReceived");
		if(ContextCur.context("MISSION", 0.0, result))
			return true;
	}

	return false;
}

//-----------------------------------------------
// testMissionRing
//-----------------------------------------------
static bool testMissionRing()
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();

	for(uint i=0;i<BOTCHATTYPE::MaxR2MissionEntryDatabase;i++)
	{
		// get the ring mission title textID
		CCDBNodeLeaf	*pNL = NLGUI::CDBManager::getInstance()->getDbProp(toString("LOCAL:TARGET:CONTEXT_MENU:MISSION_RING:%d:TITLE", i), false);
		if(pNL && pNL->getValue32())
		{
			uint32	textID = pNL->getValue32();

			// if the string is not received display a temp string
			ucstring	missionRingText;
			if(!STRING_MANAGER::CStringManagerClient::instance()->getDynString(textID, missionRingText))
				missionRingText = NLMISC::CI18N::get("uiMissionRingNameNotReceived");

			// display the cursor
			MissionRingId= i;
			if(ContextCur.context("RING MISSION", 0.f, missionRingText))
				return true;
		}
	}

	return false;
}


//-----------------------------------------------
// checkUnderCursor :
// Select the right context according to what is under the cursor.
//-----------------------------------------------
void checkUnderCursor()
{
	// Get the interface instance.
	CInterfaceManager *IM = CInterfaceManager::getInstance();
	if(IM == 0)
		return;

	// Get the cursor instance
	CViewPointer *cursor = static_cast< CViewPointer* >( CWidgetManager::getInstance()->getPointer() );
	if(cursor == 0)
		return;

	// No Op if screen minimized
	if(CViewRenderer::getInstance()->isMinimized())
		return;

	// Get the pointer position (in pixel)
	sint32 x, y;
	cursor->getPointerPos(x, y);

	// Over the interface ?
	if (CWidgetManager::getInstance()->getWindowUnder(x, y) == NULL)
	{
		// Is the pointer in the window ?
		if(x < 0 || y <0)
			return;
		uint32 w, h;
		CViewRenderer &viewRender = *CViewRenderer::getInstance();
		viewRender.getScreenSize(w, h);
		if(x>=(sint32)w || y>=(sint32)h)
			return;
		// Get the pointer position (in float)
		float cursX, cursY;
		cursX = (float)x/(float)w;
		cursY = (float)y/(float)h;

		// Get the entities under position
		bool	isPlayerUnderCursor;
		CEntityCL	*entity;
		entity= EntitiesMngr.getEntityUnderPos(cursX, cursY, ClientCfg.SelectionDist, isPlayerUnderCursor);

		// If the mouse is over the player make the player transparent
		CCDBNodeLeaf *pNL = s_UserCharFade ? &*s_UserCharFade
			: &*(s_UserCharFade = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:USER_CHAR_FADE", false));
		if ((pNL != NULL) && (pNL->getValue32() == 1) && UserEntity->selectable())
		{
			// If the nearest entity is the player, hide!
			if (isPlayerUnderCursor)
				UserEntity->makeTransparent(true);
			else
				UserEntity->makeTransparent(false);
		}
		else
			UserEntity->makeTransparent(false);

		// make transparent the entity under cursor?
		if (ClientCfg.TransparentUnderCursor)
		{
			for (uint32 i = 1; i < 255; ++i)
			{
				CEntityCL *pE = EntitiesMngr.entity(i);
				if (pE != NULL)
				{
					if (pE == entity)
						pE->makeTransparent(true);
					else
						pE->makeTransparent(false);
				}
			}
		}

		// If there is an entity selected -> Set as the Target.
		if(entity)
		{
			// Get the distance between the user and the entity.
			float dist = (float)(entity->pos() - UserEntity->pos()).norm()-entity->box().getRadius();
			// Entity Slot under the cursor.
			SlotUnderCursor = entity->slot();
			uint32 availablePrograms = (uint32)NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TARGET:CONTEXT_MENU:PROGRAMMES")->getValue32();
			bool entityAttackable = (availablePrograms&(1<<BOTCHATTYPE::Attackable)) || entity->properties().attackable();
			if (ClientCfg.R2EDEnabled)
			{
				if (R2::getEditor().isDMing())
				{
					availablePrograms = 0;
					entityAttackable = false;
				}
			}
			// If the selection is not the entity under the cursor.
			if(SlotUnderCursor != UserEntity->selection())
			{
				// If the user is fighting, clicking automatically attacks the new target
				if( ClientCfg.AtkOnSelect && UserEntity->isFighting() && entityAttackable)
				{
					if(ContextCur.context("ATTACK"))
						return;
				}
				// If the Entity is selectable -> select it.
				else if(entity->properties().selectable())
				{
					if(ContextCur.context("SELECTABLE", dist))
						return;
				}
			}
			// Entity Under the cursor is the entity selected.
			else
			{

				// Wait for the target is up to date. Do not display context cursor if the user is mounted.
				if( (UserEntity->selection() == UserEntity->targetSlot()) &&
					(! UserEntity->isRiding()) )
				{
					// Forage source
					if ( entity->isForageSource() )
					{
						if( !UserEntity->isFighting() )
						{
							if(ContextCur.context("EXTRACT_RM"))
								return;
						}
					}
					// Alive
					else if(entity->isDead()==false)
					{
						// Talk/Trade
						// Mission Option of priority 3
						if(testMissionOption(3))
							return;

						// other
						if(availablePrograms)
						{
							// Static Mission
							if(availablePrograms & (1 << BOTCHATTYPE::ChooseMissionFlag))
							{
								if(ContextCur.context("STATIC MISSION"))
									return;
							}
							// Trade Item
							else if     (availablePrograms & (1 << BOTCHATTYPE::TradeItemFlag))
							{
								if(ContextCur.context("TRADE ITEM"))
									return;
							}
							// Trade Phrase
							else if(availablePrograms & (1 << BOTCHATTYPE::TradePhraseFlag))
							{
								if(ContextCur.context("TRADE PHRASE"))
									return;
							}
							// Dynamic Mission
//								else if(availablePrograms & (1 << BOTCHATTYPE::DynamicMissionFlag))
//								{
//									if(ContextCur.context("DYNAMIC MISSION"))
//										return;
//								}
							// Trade Pact
							else if(availablePrograms & (1 << BOTCHATTYPE::TradePactFlag))
							{
								if(ContextCur.context("TRADE PACT"))
									return;
							}
							// Create Guild
							else if(availablePrograms & (1 << BOTCHATTYPE::CreateGuildFlag))
							{
								if (!CGuildManager::getInstance()->isInGuild())
									if(ContextCur.context("CREATE GUILD"))
										return;
							}
							// News
							else if(availablePrograms & (1 << BOTCHATTYPE::NewsFlag))
							{
								if(ContextCur.context("NEWS"))
									return;
							}
							// Teleport
							else if(availablePrograms & (1 << BOTCHATTYPE::TradeTeleportFlag))
							{
								if(ContextCur.context("TELEPORT"))
									return;
							}
							// Faction Items
							else if(availablePrograms & (1 << BOTCHATTYPE::TradeFactionFlag))
							{
								if(ContextCur.context("FACTION"))
									return;
							}
							// Cosmetic
							else if(availablePrograms & (1 << BOTCHATTYPE::TradeCosmeticFlag))
							{
								if(ContextCur.context("COSMETIC"))
									return;
							}
							// WebPage
							else if(availablePrograms & (1 << BOTCHATTYPE::WebPageFlag))
							{
								// get the web page title textID
								CCDBNodeLeaf	*pNL = NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TARGET:CONTEXT_MENU:WEB_PAGE_TITLE", false);
								if(pNL && pNL->getValue32())
								{
									uint32	textID = pNL->getValue32();

									// if the string is not received display a temp string
									ucstring	webPageText;
									if(!STRING_MANAGER::CStringManagerClient::instance()->getDynString(textID, webPageText))
										webPageText = NLMISC::CI18N::get("uiWebPageNameNotReceived");

									// display the cursor
									if(ContextCur.context("WEB PAGE", 0.f, webPageText))
										return;
								}
							}
							// OutpostFlag
							else if(availablePrograms & (1 << BOTCHATTYPE::OutpostFlag))
							{
								// get the outpost sheet
								CCDBNodeLeaf	*pNL = NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TARGET:CONTEXT_MENU:OUTPOST", false);
								if(pNL && pNL->getValue32())
								{
									// get the outpost name
									CSheetId outpostSheet(pNL->getValue32());
									ucstring outpostName;
									outpostName= ucstring(STRING_MANAGER::CStringManagerClient::getOutpostLocalizedName(outpostSheet));

									// display the cursor
									if(ContextCur.context("OUTPOST", 0.f, outpostName))
										return;
								}
							}
							// Spire Totem
							else if(availablePrograms & (1 << BOTCHATTYPE::Totem))
							{
								if(ContextCur.context("BUILD_TOTEM"))
									return;
							}

						}

						// test mission options of lesser priorities
						if(testMissionOption(2))
							return;
						if(testMissionOption(1))
							return;

						// test ring mission
						if(testMissionRing())
							return;

						// Attack
						if (entityAttackable)
						{
							if(ContextCur.context("ATTACK"))
								return;
						}

						if(testMissionOption(0))
							return;
					}
					// Dead
					else
					{
						if (!R2::getEditor().isDMing())
						{
							// Quartering
							if((entity->properties()).harvestable())
							{
								if(ContextCur.context("QUARTER"))
									return;
							}
							// Loot
							else if((entity->properties()).lootable())
							{
								if(ContextCur.context("LOOT"))
									return;
							}
							// Pick Up
							else if((entity->properties()).liftable())
							{
								if(ContextCur.context("PICKUP"))
									return;
							}
						}
					}

					if(entity->properties().selectable())
					{
						if(ContextCur.context("SELECTABLE", dist))
							return;
					}
				}
			}
		}
		else
		{
			CShapeInstanceReference instref = EntitiesMngr.getShapeInstanceUnderPos(cursX, cursY);
			
			bool cleanSelectedInstance = EntitiesMngr.instancesRemoved();
			if (cleanSelectedInstance)
				selectedInstance = noSelectedInstance;

			UInstance instance = instref.Instance;
			if (!instance.empty())
			{
				if (instance.getObjectPtr() != selectedInstance.getObjectPtr())
				{
					for(uint j=0;j<selectedInstance.getNumMaterials();j++)
					{
						// unhighlight
						selectedInstance.getMaterial(j).setEmissive(CRGBA(255,255,255,255));
						selectedInstance.getMaterial(j).setShininess( 10.0f );
					}
					selectedInstance = instance;
					// For all materials
					for(uint j=0;j<selectedInstance.getNumMaterials();j++)
					{
						// highlight
						selectedInstance.getMaterial(j).setEmissive(CRGBA(255,0,0,255));
						selectedInstance.getMaterial(j).setShininess( 1000.0f );
					}
				}
				if (!instref.ContextText.empty())
				{
					selectedInstanceURL = instref.ContextURL;
					ucstring contextText;
					contextText.fromUtf8(instref.ContextText);
					if(ContextCur.context("WEBIG", 0.f, contextText))
						return;
				}
			}
			else
			{
				if (!selectedInstance.empty())
				{
					for(uint j=0;j<selectedInstance.getNumMaterials();j++)
					{
						//unhighlight
						selectedInstance.getMaterial(j).setEmissive(CRGBA(255,255,255,255));
						selectedInstance.getMaterial(j).setShininess( 10.0f );
					}
					selectedInstance = noSelectedInstance;
					selectedInstanceURL = string("");
				}
			}
			SlotUnderCursor = CLFECOMMON::INVALID_SLOT;
		}
	}
	else
	{
		if(UserEntity)
			UserEntity->makeTransparent(false);
	}
	// Default Context.
	ContextCur.context("STAND BY");
}// checkUnderCursor //


//-----------------------------------------------
// contextSelect :
// Select what is under the cursor.
// \Warning Be sure the checkUnderCursor and contextSelect are done after entites removed.
//-----------------------------------------------
void contextSelect(bool /* rightClick */, bool /* dblClick */)
{
	// Push a message with the entity target (in slot).
	if(SlotUnderCursor != CLFECOMMON::INVALID_SLOT)
	{
		// Select the entity
		UserEntity->selection(SlotUnderCursor);
	}
}// contextSelect //

//-----------------------------------------------
// contextAttack :
//-----------------------------------------------
void contextAttack(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	// Select the entity to target.
	UserEntity->selection(SlotUnderCursor);

	if( UserEntity->canEngageCombat() )
	{
		// Move to the current selection and attack.
		UserEntity->moveToAttack();
	}
}// contextAttack //

//-----------------------------------------------
// contextQuarter :
//-----------------------------------------------
void contextQuarter(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveTo(SlotUnderCursor, 3.0, CUserEntity::Quarter);
}// contextQuarter //

//-----------------------------------------------
// contextLoot :
//-----------------------------------------------
void contextLoot(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveTo(SlotUnderCursor, 3.0, CUserEntity::Loot);
}// contextLoot //

//-----------------------------------------------
// contextPickUp :
//-----------------------------------------------
void contextPickUp(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveTo(SlotUnderCursor, 3.0, CUserEntity::PickUp);
}// contextPickUp //

//-----------------------------------------------
// contextTradeItem :
//-----------------------------------------------
void contextTradeItem(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveTo(SlotUnderCursor, 3.0, CUserEntity::TradeItem);
}// contextTradeItem //

//-----------------------------------------------
// contextTradePhrase :
//-----------------------------------------------
void contextTradePhrase(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveTo(SlotUnderCursor, 3.0, CUserEntity::TradePhrase);
}// contextTradePhrase //

//-----------------------------------------------
// contextDynamicMission :
//-----------------------------------------------
void contextDynamicMission(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveTo(SlotUnderCursor, 3.0, CUserEntity::DynamicMission);
}// contextDynamicMission //

//-----------------------------------------------
// contextStaticMission :
//-----------------------------------------------
void contextStaticMission(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveTo(SlotUnderCursor, 3.0, CUserEntity::StaticMission);
}// contextStaticMission //

//-----------------------------------------------
// contextTradePact :
//-----------------------------------------------
void contextTradePact(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveTo(SlotUnderCursor, 3.0, CUserEntity::TradePact);
}// contextTradePact //

//-----------------------------------------------
// contextCreateGuild :
//-----------------------------------------------
void contextCreateGuild(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveTo(SlotUnderCursor, 3.0, CUserEntity::CreateGuild);
}// contextCreateGuild //

//-----------------------------------------------
// contextNews :
//-----------------------------------------------
void contextNews(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveTo(SlotUnderCursor, 3.0, CUserEntity::News);
}// contextNews //

//-----------------------------------------------
// contextTeleport :
//-----------------------------------------------
void contextTeleport(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveTo(SlotUnderCursor, 3.0, CUserEntity::TradeTeleport);
}// contextTeleport //

//-----------------------------------------------
// contextFaction :
//-----------------------------------------------
void contextFaction(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveTo(SlotUnderCursor, 3.0, CUserEntity::TradeFaction);
}// contextFaction //

//-----------------------------------------------
// contextCosmetic :
//-----------------------------------------------
void contextCosmetic(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveTo(SlotUnderCursor, 3.0, CUserEntity::TradeCosmetic);
}// contextCosmetic //

//-----------------------------------------------
// contextTalk :
//-----------------------------------------------
void contextTalk(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	// Get the interface instace.
	CInterfaceManager *IM = CInterfaceManager::getInstance();
	if(IM == 0)
		return;
	// Get Entity Program
	uint32 availablePrograms = (uint32)NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:TARGET:CONTEXT_MENU:PROGRAMMES")->getValue32();
	// Static Mission
	if(availablePrograms & (1 << BOTCHATTYPE::ChooseMissionFlag))
		UserEntity->moveTo(SlotUnderCursor, 2.0, CUserEntity::StaticMission);
	// Trade Item
	else if     (availablePrograms & (1 << BOTCHATTYPE::TradeItemFlag))
		UserEntity->moveTo(SlotUnderCursor, 2.0, CUserEntity::TradeItem);
	// Trade Phrase
	else if(availablePrograms & (1 << BOTCHATTYPE::TradePhraseFlag))
		UserEntity->moveTo(SlotUnderCursor, 2.0, CUserEntity::TradePhrase);
	// Dynamic Mission DEPRECATED
/*	else if(availablePrograms & (1 << BOTCHATTYPE::DynamicMissionFlag))
		UserEntity->moveTo(SlotUnderCursor, 2.0, CUserEntity::DynamicMission);*/

	// Trade Pact
	else if(availablePrograms & (1 << BOTCHATTYPE::TradePactFlag))
		UserEntity->moveTo(SlotUnderCursor, 2.0, CUserEntity::TradePact);
	// Create Guild
	else if(availablePrograms & (1 << BOTCHATTYPE::CreateGuildFlag))
	{
		if (!CGuildManager::getInstance()->isInGuild())
			UserEntity->moveTo(SlotUnderCursor, 2.0, CUserEntity::CreateGuild);
	}
	// News
	else if(availablePrograms & (1 << BOTCHATTYPE::NewsFlag))
		UserEntity->moveTo(SlotUnderCursor, 2.0, CUserEntity::Talk);
	// Teleport
	else if(availablePrograms & (1 << BOTCHATTYPE::TradeTeleportFlag))
		UserEntity->moveTo(SlotUnderCursor, 2.0, CUserEntity::TradeTeleport);
	// Faction
	else if(availablePrograms & (1 << BOTCHATTYPE::TradeFactionFlag))
		UserEntity->moveTo(SlotUnderCursor, 2.0, CUserEntity::TradeFaction);
	// WebPage
	else if(availablePrograms & (1 << BOTCHATTYPE::WebPageFlag))
		UserEntity->moveTo(SlotUnderCursor, 2.0, CUserEntity::WebPage);
	// Outpost
	else if(availablePrograms & (1 << BOTCHATTYPE::OutpostFlag))
		UserEntity->moveTo(SlotUnderCursor, 2.0, CUserEntity::Outpost);
}// contextTalk //


//-----------------------------------------------
// contextExtractRM :
//-----------------------------------------------
void contextExtractRM(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveToExtractionPhrase(SlotUnderCursor, MaxExtractionDistance, std::numeric_limits<uint>::max(), std::numeric_limits<uint>::max(), true );
}

//-----------------------------------------------
// contextMission :
// Callback for dynamic missions
//-----------------------------------------------
void contextMission(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveToMission(SlotUnderCursor, 3.0, MissionId);
}// contextMission //

//-----------------------------------------------
// contextWebPage :
//-----------------------------------------------
void contextWebPage(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveTo(SlotUnderCursor, 3.0, CUserEntity::WebPage);
}// contextWebPage //

//-----------------------------------------------
// contextWebIG :
//-----------------------------------------------
void contextWebIG(bool rightClick, bool dblClick)
{
	CInterfaceManager *IM = CInterfaceManager::getInstance();
	CInterfaceElement *pGC = CWidgetManager::getInstance()->getElementFromId("ui:interface:bot_chat_object");
	CInterface3DShape *el= dynamic_cast<CInterface3DShape*>(CWidgetManager::getInstance()->getElementFromId("ui:interface:bot_chat_object:scene3d:object_1"));
	if (el != NULL)
	{
		el->setName(selectedInstance.getShapeName());
		el->setPosX(0.0f);
	}
	if (selectedInstanceURL.empty())
	{
		if (pGC != NULL)
			pGC->setActive(true);
	}
	else
	{
		if (pGC != NULL)
			pGC->setActive(false);
		CAHManager::getInstance()->runActionHandler("browse", NULL, "name=ui:interface:webig:content:html|url="+selectedInstanceURL);
	}
}// contextWebIG //

//-----------------------------------------------
// contextOutpost
//-----------------------------------------------
void contextOutpost(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveTo(SlotUnderCursor, 3.0, CUserEntity::Outpost);
}// contextOutpost //

//-----------------------------------------------
// contextRingMission
//-----------------------------------------------
void contextRingMission(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveToMissionRing(SlotUnderCursor, 3.0, MissionRingId);
}// contextRingMission //

//-----------------------------------------------
// contextBuildTotem :
//-----------------------------------------------
void contextBuildTotem(bool rightClick, bool dblClick)
{
	if(rightClick)
		return;
	if( ClientCfg.DblClickMode && !dblClick)
		return;
	UserEntity->moveToTotemBuildingPhrase(SlotUnderCursor,MaxExtractionDistance, std::numeric_limits<uint>::max(), std::numeric_limits<uint>::max(), true );
}// contextBuildTotem //
