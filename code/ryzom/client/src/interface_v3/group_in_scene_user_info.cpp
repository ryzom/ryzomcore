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
#include "interface_manager.h"
#include "nel/gui/view_bitmap.h"
#include "group_in_scene_user_info.h"
#include "nel/gui/action_handler.h"
#include "../entities.h"
#include "../user_entity.h"
#include "../forage_source_cl.h"
#include "guild_manager.h"
#include "dbctrl_sheet.h"
#include "bar_manager.h"
#include "../client_cfg.h"
#include "../npc_icon.h"
//
#include "../r2/editor.h"

using namespace std;
using namespace NLMISC;

extern CEntityManager EntitiesMngr;

uint CGroupInSceneUserInfo::_BatLength = 0;
CCDBNodeLeaf *CGroupInSceneUserInfo::_Value = NULL;
CCDBNodeLeaf *CGroupInSceneUserInfo::_ValueBegin = NULL;
CCDBNodeLeaf *CGroupInSceneUserInfo::_ValueEnd = NULL;
NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> CGroupInSceneUserInfo::_GuildIconLeaf[256];

// ***************************************************************************
NLMISC_REGISTER_OBJECT(CViewBase, CGroupInSceneUserInfo, std::string, "in_scene_user_info");
REGISTER_UI_CLASS(CGroupInSceneUserInfo)

namespace {

// Has more entries than actually in config, as not all types have all entries.
class CConfigSaveInsceneDB
{
public:
	void setPrefix(const std::string &prefix) { _DBPrefix = prefix; }
	inline NLMISC::CCDBNodeLeaf *getGuildSymbol() { return _GuildSymbol ? (&*_GuildSymbol) : &*(_GuildSymbol = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "GUILD_SYMBOL")); }
	inline NLMISC::CCDBNodeLeaf *getName() { return _Name ? (&*_Name) : &*(_Name = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "NAME")); }
	inline NLMISC::CCDBNodeLeaf *getTitle() { return _Title ? (&*_Title) : &*(_Title = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "TITLE")); }
	inline NLMISC::CCDBNodeLeaf *getRPTags() { return _RPTags ? (&*_RPTags) : &*(_RPTags = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "RPTAGS")); }
	inline NLMISC::CCDBNodeLeaf *getGuildName() { return _GuildName ? (&*_GuildName) : &*(_GuildName = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "GUILD_NAME")); }
	inline NLMISC::CCDBNodeLeaf *getHP() { return _HP ? (&*_HP) : &*(_HP = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "HP")); }
	inline NLMISC::CCDBNodeLeaf *getSta() { return _Sta ? (&*_Sta) : &*(_Sta = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "STA")); }
	inline NLMISC::CCDBNodeLeaf *getSap() { return _Sap ? (&*_Sap) : &*(_Sap = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "SAP")); }
	inline NLMISC::CCDBNodeLeaf *getFocus() { return _Focus ? (&*_Focus) : &*(_Focus = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "FOCUS")); }
	inline NLMISC::CCDBNodeLeaf *getAction() { return _Action ? (&*_Action) : &*(_Action = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "ACTION")); }
	inline NLMISC::CCDBNodeLeaf *getMessages() { return _Messages ? (&*_Messages) : &*(_Messages = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "MESSAGES")); }
	inline NLMISC::CCDBNodeLeaf *getPvPLogo() { return _PvPLogo ? (&*_PvPLogo) : &*(_PvPLogo = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "PVP_LOGO")); }
	inline NLMISC::CCDBNodeLeaf *getNPCName() { return _NPCName ? (&*_NPCName) : &*(_NPCName = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "NPCNAME")); }
	inline NLMISC::CCDBNodeLeaf *getNPCTitle() { return _NPCTitle ? (&*_NPCTitle) : &*(_NPCTitle = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "NPCTITLE")); }
	inline NLMISC::CCDBNodeLeaf *getMissionIcon() { return _MissionIcon ? (&*_MissionIcon) : &*(_MissionIcon = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "MISSION_ICON")); }
	inline NLMISC::CCDBNodeLeaf *getMiniMissionIcon() { return _MiniMissionIcon ? (&*_MiniMissionIcon) : &*(_MiniMissionIcon = NLGUI::CDBManager::getInstance()->getDbProp(_DBPrefix + "MINI_MISSION_ICON")); }
private:
	std::string _DBPrefix;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _GuildSymbol;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _Name;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _Title;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _RPTags;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _GuildName;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _HP;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _Sta;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _Sap;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _Focus;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _Action;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _Messages;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _PvPLogo;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _NPCName;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _NPCTitle;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _MissionIcon;
	NLMISC::CRefPtr<NLMISC::CCDBNodeLeaf> _MiniMissionIcon;
};

CConfigSaveInsceneDB _ConfigSaveInsceneDB[4]; // USER/FRIEND/ENEMY/SOURCE
bool _ConfigSaveInsceneDBInit = false;

#define SAVE_USER 0
#define SAVE_FRIEND 1
#define SAVE_ENEMY 2
#define SAVE_SOURCE 3

}


CGroupInSceneUserInfo::CGroupInSceneUserInfo(const TCtorParam &param)
:	CGroupInScene(param)
{
	if (!_ConfigSaveInsceneDBInit)
	{
		_ConfigSaveInsceneDB[0].setPrefix("UI:SAVE:INSCENE:USER:");
		_ConfigSaveInsceneDB[1].setPrefix("UI:SAVE:INSCENE:FRIEND:");
		_ConfigSaveInsceneDB[2].setPrefix("UI:SAVE:INSCENE:ENEMY:");
		_ConfigSaveInsceneDB[3].setPrefix("UI:SAVE:INSCENE:SOURCE:");
		_ConfigSaveInsceneDBInit = true;
	}
	_Name = NULL;
	_Title = NULL;
	_GuildName = NULL;
	_TribeName = NULL;
	_EventFaction = NULL;
	_PermanentContent = NULL;
	_Target = NULL;
	_MissionTarget = NULL;
	uint i;
	for (i=0; i<NumBars; i++)
		_Bars[i] = NULL;
	_Entity = NULL;
	_NeedGuildNameId= false;
	_NeedGuildSymbolId= false;
	_IsLeftGroupActive= false;
}

// ***************************************************************************

CGroupInSceneUserInfo::~CGroupInSceneUserInfo()
{
}

// ***************************************************************************

// Static bars data

// ***************************************************************************

CRGBA CGroupInSceneUserInfo::BarColor[NumBars]=
{
	CRGBA(255, 64, 0),
	CRGBA(72, 255, 0),
	CRGBA(255, 0, 255),
	CRGBA(0, 128, 255),
	CRGBA(255, 255, 255),
};

CRGBA CGroupInSceneUserInfo::BarColorHPNegative = CRGBA(127, 32, 0);

// ***************************************************************************

#define nlfsinfo if ( isForageSource ) nlinfo
#define nlfsinfo2 if ( _Entity->isForageSource() ) nlinfo


CGroupInSceneUserInfo *CGroupInSceneUserInfo::build (CEntityCL *entity)
{
	// Get the interface manager
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	nlassert (pIM);

	// Character type
	bool user = entity->isUser ();
	bool _friend = entity->isViewedAsFriend ();
	bool isForageSource = entity->isForageSource ();
	// Special display for roleMaster etc...
	// NB: fauna can be friend too (kami!!!)
	bool npcFriendAndNeutral = (entity->canHaveMissionIcon() || entity->isFauna()) && entity->isFriend ();
	// if the npc is an ally (outpost squad for instance) still display its bar
	if (npcFriendAndNeutral)
		npcFriendAndNeutral= npcFriendAndNeutral && !entity->isAlly();

	// Window id
	string id = "in_scene_entity_info_"+toString (entity->slot());

	bool bars[NumBars];
	bool name;
	bool symbol;
	bool title;
	bool guildName;
	bool tribeName = false;
	bool eventFaction = false;
	bool needGuildNameId= false;
	bool needGuildSymbolId= false;
	bool forageSourceBarDisplayed= false;
	bool needPvPLogo= false;
	bool permanentContent = false;
	bool rpTags = false;
	bool displayMissionIcons = _ConfigSaveInsceneDB[SAVE_FRIEND].getMissionIcon()->getValueBool();

	// Names
	const char *templateName;
	ucstring theTribeName;
	ucstring entityName = entity->getDisplayName();
	ucstring entityTitle = entity->getTitle();
	
	// For some NPC's the name is empty and only a title is given,
	// in that case, treat the title as the name.
	if (entityName.empty())
	{
		entityName = entityTitle;
		entityTitle.clear();
	}

	ucstring entityTag1 = entity->getTag(1);
	ucstring entityTag2 = entity->getTag(2);
	ucstring entityTag3 = entity->getTag(3);
	ucstring entityTag4 = entity->getTag(4);

	string entityPermanentContent = entity->getPermanentStatutIcon();

	// Active fields and bars
	if ( isForageSource )
	{
		CForageSourceCL *forageSource = static_cast<CForageSourceCL*>(entity);

		name = !entityName.empty() /*&& NLGUI::CDBManager::getInstance()->getDbProp(dbEntry+"NAME")->getValueBool()*/;
		symbol = (forageSource->getKnowledge() != 0);
		title = false;
		guildName = false;
		// Display the forage soruce bars only if I am the entity selected
		forageSourceBarDisplayed = (entity->slot() == UserEntity->selection());
		bars[Time] = forageSourceBarDisplayed;
		bars[Amount] = forageSourceBarDisplayed;
		//bool displayExtractingParams = true; //forageSource->isExtractionInProgress() /*&& NLGUI::CDBManager::getInstance()->getDbProp(dbEntry+"HP")*/;
		bars[Life] = forageSourceBarDisplayed;//displayExtractingParams;
		bars[Danger] = forageSourceBarDisplayed;//displayExtractingParams;
		bars[Spawn] = forageSourceBarDisplayed;//displayExtractingParams;
		templateName = "forage_source";
	}
	else if(npcFriendAndNeutral)
	{
		int dbEntry;
		getBarSettings( pIM, user, entity->isPlayer(), _friend, dbEntry, bars );
		// For RoleMasters, merchants etc... must display name and function, and nothing else
		for(uint i=0;i<NumBars;i++)
			bars[i]= false;
		name= !entityName.empty() && _ConfigSaveInsceneDB[dbEntry].getNPCName()->getValueBool();
		symbol= false;
		title= !entityTitle.empty() && _ConfigSaveInsceneDB[dbEntry].getNPCTitle()->getValueBool();
		guildName= false;
		templateName = "in_scene_user_info";
		rpTags = (!entityTag1.empty()  ||  !entityTag2.empty()  || !entityTag3.empty()  || !entityTag4.empty() ) && _ConfigSaveInsceneDB[dbEntry].getRPTags()->getValueBool();
	}
	else
	{
		// Base entry in database
		int dbEntry;
		getBarSettings( pIM, user, entity->isPlayer(), _friend, dbEntry, bars );
		name = !entityName.empty() && _ConfigSaveInsceneDB[dbEntry].getName()->getValueBool();
		title = !entityTitle.empty() && _ConfigSaveInsceneDB[dbEntry].getTitle()->getValueBool();
		rpTags = (!entityTag1.empty()  ||  !entityTag2.empty()  || !entityTag3.empty()  || !entityTag4.empty() ) && _ConfigSaveInsceneDB[dbEntry].getRPTags()->getValueBool();
		// if name is empty but not title, title is displayed as name
		if (!title && entityName.empty() && !entityTitle.empty() && _ConfigSaveInsceneDB[dbEntry].getName()->getValueBool())
			title = true;
		templateName = "in_scene_user_info";
		// special guild
		if(_ConfigSaveInsceneDB[dbEntry].getGuildSymbol()->getValueBool())
		{
			// if symbol not still available, wait for one when VP received
			symbol = (entity->getGuildSymbol() != 0);
			needGuildSymbolId = true;
		}
		else
		{
			symbol= false;
			needGuildSymbolId = false;
		}
		if(_ConfigSaveInsceneDB[dbEntry].getGuildName()->getValueBool())
		{
			// if guild name not still available, wait for one when VP received
			guildName = (entity->getGuildNameID() != 0);
			needGuildNameId = true;
		}
		else
		{
			guildName= false;
			needGuildNameId= false;
		}
		needPvPLogo = _ConfigSaveInsceneDB[dbEntry].getPvPLogo()->getValueBool();

		eventFaction = (entity->getEventFactionID() != 0);
	}

	permanentContent = !entityPermanentContent.empty();



	// according to entity setup, may disable some fields
	if(!entity->getDisplayOSDName())
	{
		// force no name (still symbol if setuped, is that true?)
		name= false;
		title= false;
		guildName= false;
		needGuildNameId = false;
	}
	if(!entity->getDisplayOSDBars())
	{
		// force no bar
		for(uint i=0;i<NumBars;i++)
			bars[i]= false;
	}

	// get faction name
	{
		CCharacterCL *pChar = dynamic_cast<CCharacterCL*>(entity);
		if (pChar != NULL)
		{
			const CCharacterSheet *pSheet = pChar->getSheet();
			if (pSheet != NULL)
			{
				string sFame = pSheet->getFame();
				if (strnicmp(sFame.c_str(),"tribe_",6)==0)
				{
					tribeName = true;
					//// always display title for tribe
					//title = true;
					theTribeName = STRING_MANAGER::CStringManagerClient::getFactionLocalizedName(sFame);
					// tribeName stuff disable any guild name
					guildName= false;
					needGuildNameId= false;
				}
			}
		}
	}


	// Create the template
	CGroupInSceneUserInfo *info = newGroupInScene(templateName, id);
	if (info)
	{
		// copy guild info
		info->_NeedGuildNameId= needGuildNameId;
		info->_NeedGuildSymbolId= needGuildSymbolId;

		// Get the left group
		CInterfaceGroup *leftGroup = info->getGroup ("right");
		info->_IsLeftGroupActive = true;

		if (info && leftGroup)
		{
			// *** Build it
			info->_Entity = entity;

			// Some constants
			sint barHeight, barSpace;
			fromString(CWidgetManager::getInstance()->getParser()->getDefine("in_scene_user_info_bar_h"), barHeight);
			fromString(CWidgetManager::getInstance()->getParser()->getDefine("in_scene_user_info_bar_space"), barSpace);
			fromString(CWidgetManager::getInstance()->getParser()->getDefine("in_scene_user_bar_length"), CGroupInSceneUserInfo::_BatLength);

			// Build the bars
			uint barCount = 0;
			uint i;
			for (i=0; i<NumBars; i++)
			{
				info->_Bars[i] = NULL;
				if (bars[i])
				{
					// Get the bar
					CViewBase *bar = leftGroup->getView ("bar" + toString(barCount));
					if (bar)
					{
						// Bitmap view
						CViewBitmap *bitmap = dynamic_cast<CViewBitmap *>(bar);
						if (bitmap)
						{
							if ( ! isForageSource ) // forage source: use color from widgets.xml
							{
								// Set the color
								bitmap->setColorRGBA (BarColor[i]);
							}
							info->_Bars[i] = bitmap;
							barCount++;
						}
					}
				}
			}

			// Remove others
			for (i=barCount; i<NumBars; i++)
			{
				CViewBase *bar = leftGroup->getView ("bar" + toString(i));
				if (bar)
					leftGroup->delView (bar);
				CViewBase *icon = leftGroup->getView ("icon" + toString(i));
				if (icon)
					leftGroup->delView (icon);
				CCtrlBase *toolTip = leftGroup->getCtrl ("tt" + toString(i));
				if ( toolTip )
					leftGroup->delCtrl (toolTip);
				CCtrlBase *toolTip2 = leftGroup->getCtrl ("ttb" + toString(i));
				if ( toolTip2 )
					leftGroup->delCtrl (toolTip2);
			}

			// Adjust win_jauge_mid
			sint spaceBar = (NumBars-barCount) * (barSpace+barHeight);
			CViewBase *win_jauge_mid = leftGroup->getView ("win_jauge_mid");
			if (win_jauge_mid)
			{
				win_jauge_mid->setH (win_jauge_mid->getH() - spaceBar);
			}

			// No bar ?
			if (!barCount)
			{
				// Delete
				CViewBase *view = leftGroup->getView ("win_bot");
				if (view)
					leftGroup->delView (view);
				view = leftGroup->getView ("win_mid");
				if (view)
					leftGroup->delView (view);
				view = leftGroup->getView ("win_top");
				if (view)
					leftGroup->delView (view);

				if (win_jauge_mid)
					//win_jauge_mid->setActive(false);
					leftGroup->delView (win_jauge_mid);
				view = leftGroup->getView ("win_jauge_top");

				if (view)
					//view->setActive(false);
					leftGroup->delView (view);

				view = leftGroup->getView ("win_jauge_bot");
				if (view)
					//view->setActive(false);
					leftGroup->delView (view);
			}

			// Strings
			//sint stringSpace = 0;
			sint stringCount = 0;
			if (name)
			{
				CViewBase *text = leftGroup->getView ("info"+toString(stringCount));
				if (text)
					info->_Name = dynamic_cast<CViewText*>(text);
				stringCount++;
			}
			//else
			//	stringSpace += textH;
			if (title)
			{
				CViewBase *text = leftGroup->getView ("info"+toString(stringCount));
				if (text)
					info->_Title = dynamic_cast<CViewText*>(text);
				stringCount++;
			}
			//else
			//	stringSpace += textH;

			if (rpTags)
			{
				CPlayerCL * pPlayer = dynamic_cast<CPlayerCL*>(entity);
				CViewBitmap *bitmap;
				if (pPlayer == NULL || (pPlayer != NULL && pPlayer->getPvpMode() & PVP_MODE::PvpFaction))
				{
					bitmap = dynamic_cast<CViewBitmap*>(leftGroup->getView ("rp_logo_1"));
					if (bitmap)
						bitmap->setTexture(entityTag1.toString());
					bitmap = dynamic_cast<CViewBitmap*>(leftGroup->getView ("rp_logo_2"));
					if (bitmap)
						bitmap->setTexture(entityTag2.toString());
				}
				bitmap = dynamic_cast<CViewBitmap*>(leftGroup->getView ("rp_logo_3"));
				if (bitmap)
					bitmap->setTexture(entityTag3.toString());
				bitmap = dynamic_cast<CViewBitmap*>(leftGroup->getView ("rp_logo_4"));
				if (bitmap)
					bitmap->setTexture(entityTag4.toString());
			}

			// Get the permanent content bitmap
			if(permanentContent)
			{
				CViewBase *permanent = leftGroup->getView ("permanent_content");
				if (permanent)
				{
					CViewBitmap *bitmap = dynamic_cast<CViewBitmap *>(permanent);
					if (bitmap)
					{
						info->_PermanentContent = bitmap;
						info->_PermanentContent->setTexture(entityPermanentContent);
					}
				}
			}


			// NB : guild, event faction and tribe use the same text entry
			if (guildName || eventFaction || tribeName)
			{
				if (eventFaction)
				{
					CViewBase *text = leftGroup->getView ("info"+toString(stringCount));
					if (text)
						info->_EventFaction = dynamic_cast<CViewText*>(text);
					stringCount++;
				}
				else if (guildName)
				{
					nlassert(!tribeName);
					CViewBase *text = leftGroup->getView ("info"+toString(stringCount));
					if (text)
						info->_GuildName = dynamic_cast<CViewText*>(text);
					stringCount++;
				}
				else if (tribeName)
				{
					nlassert(!guildName);
					CViewBase *text = leftGroup->getView ("info"+toString(stringCount));
					if (text)
						info->_TribeName = dynamic_cast<CViewText*>(text);
					stringCount++;
				}
			}
			//else
			//	stringSpace += textH;

			// Hide guild symbol / raw material source icon?
			if ( isForageSource )
			{
				CViewBase *logo = info->getView( "guild_logo" );
				if (!symbol)
				{
					info->delView(logo);
					leftGroup->setX(0);
				}
				else
				{
					CViewBitmap *bitmap = dynamic_cast<CViewBitmap*>(logo);
					if (bitmap)
					{
						// Set the raw material knowledge icon
						CForageSourceCL *forageSource = static_cast<CForageSourceCL*>(entity);
						const string *iconFilename = forageSource->getKnowledgeIcon();
						if ( iconFilename )
							bitmap->setTexture (*iconFilename);
					}
					leftGroup->invalidateCoords();
				}

				// Set ZBias of forage interface
				info->setZBias(ClientCfg.ForageInterfaceZBias);

				// use cursor only if source bar displayed
				info->setUseCursor(forageSourceBarDisplayed);
			}
			else if (npcFriendAndNeutral)
			{
				// Don't display any guild logo
				CCtrlBase *guildLogo = info->getCtrl ("guild_logo");
				info->delCtrl(guildLogo);

				CViewBase *logo = info->getView( "npc_mission_logo" );
				CViewBitmap *bitmap = dynamic_cast<CViewBitmap*>(logo);
				CViewBase *logoOver = info->getView( "npc_mission_logo_over" );
				CViewBitmap *bitmapOver = dynamic_cast<CViewBitmap*>(logoOver);
				if (bitmap && bitmapOver)
				{
					// Set the NPC icon
					const CNPCIconCache::CNPCIconDesc& iconDesc = CNPCIconCache::getInstance().getNPCIcon(entity, true);
					if ((!iconDesc.getTextureMain().empty()) && displayMissionIcons)
					{
						bitmap->setTexture(iconDesc.getTextureMain());
						bitmap->setActive(true);
						if (!iconDesc.getTextureOver().empty())
						{
							bitmapOver->setTexture(iconDesc.getTextureOver());
							bitmapOver->setActive(true);
						}
						else
						{
							info->delView(logoOver);
						}
						leftGroup->setW( leftGroup->getW() + 42 );
					}
					else
					{
						info->delView(logo);
						info->delView(logoOver);
						leftGroup->setX(0);
					}
					leftGroup->invalidateCoords();
				}
			}
			else
			{
				CCtrlBase *logo = info->getCtrl ("guild_logo");
				if (logo)
				{
					if (!symbol)
					{
						info->delCtrl(logo);
						leftGroup->setX(0);
					}
					else
					{
						CDBCtrlSheet *sheet = dynamic_cast<CDBCtrlSheet *>(logo);
						if (sheet)
						{
							// Set the guild symbol
							string dbLeaf = "UI:ENTITY:GUILD:"+toString (entity->slot());
							sheet->setSheet(dbLeaf);

							NLGUI::CDBManager::getInstance()->getDbProp(dbLeaf+":ICON")->setValue64(entity->getGuildSymbol());
						}
					}
				}
			}

			if (!isForageSource)
			{
				CViewBase * invisibleLogo = info->getView("invisible_logo");
				if (entity->isUser() && invisibleLogo)
				{
					bool invisible = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:USER:IS_INVISIBLE")->getValueBool();
					invisibleLogo->setActive(invisible);
				}

				// Get the target bitmap
				CViewBase *target = leftGroup->getView ("target");
				if (target)
				{
					CViewBitmap *bitmap = dynamic_cast<CViewBitmap *>(target);
					if (bitmap)
						info->_Target = bitmap;
				}

				// Get the mission target bitmap
				CViewBase *missionTarget = leftGroup->getView ("mission_target");
				if (missionTarget)
				{
					CViewBitmap *bitmap = dynamic_cast<CViewBitmap *>(missionTarget);
					if (bitmap)
						info->_MissionTarget = bitmap;
				}

				CViewBase * pvpFactionLogo = info->getView ("pvp_faction_logo");
				CViewBase * pvpOutpostLogo = info->getView ("pvp_outpost_logo");
				CViewBase * pvpDuelLogo = info->getView ("pvp_duel_logo");

				CPlayerCL * pPlayer = dynamic_cast<CPlayerCL*>(entity);
				if (pPlayer == NULL)
					needPvPLogo = false;


				if (pPlayer != NULL && needPvPLogo)
				{
					if (pvpFactionLogo) 
					{
						pvpFactionLogo->setActive(true);
						CViewBitmap * pvpFactionBitmap = dynamic_cast<CViewBitmap *>(pvpFactionLogo);
						if( pvpFactionBitmap )
						{
							if (user)
							{
								if (pPlayer->getPvpMode() & PVP_MODE::PvpChallenge)
								{
									pvpFactionBitmap->setTexture("ico_curse.tga");
								}
								else if (pPlayer->getPvpMode() & PVP_MODE::PvpFaction)
								{
									if (pPlayer->getPvpMode() & PVP_MODE::PvpZoneSafe)
										pvpFactionBitmap->setTexture("pvp_neutral.tga");
									else
										pvpFactionBitmap->setTexture("pvp_enemy_tag.tga");
								}
								else if (pPlayer->getPvpMode() & PVP_MODE::PvpFactionFlagged)
								{
									if (pPlayer->getPvpMode() & PVP_MODE::PvpSafe)
										pvpFactionBitmap->setTexture("pvp_neutral.tga");
									else
										pvpFactionBitmap->setTexture("pvp_enemy_flag.tga");
								}
								else
									pvpFactionLogo->setActive(false);
							}
							else
							{
								if (pPlayer->getPvpMode() & PVP_MODE::PvpChallenge)
									pvpFactionBitmap->setTexture("ico_curse.tga");
								else if (pPlayer->isNeutralPVP())
									pvpFactionBitmap->setTexture("pvp_neutral.tga");
								else if (pPlayer->isAlly() && (pPlayer->getPvpMode() & PVP_MODE::PvpFactionFlagged))
									pvpFactionBitmap->setTexture("pvp_ally_flag.tga");
								else if (pPlayer->isAlly() && (pPlayer->getPvpMode() & PVP_MODE::PvpFaction))
									pvpFactionBitmap->setTexture("pvp_ally_tag.tga");
								else if (pPlayer->isEnemy() && (pPlayer->getPvpMode() & PVP_MODE::PvpFactionFlagged))
									pvpFactionBitmap->setTexture("pvp_enemy_flag.tga");
								else if (pPlayer->isEnemy() && (pPlayer->getPvpMode() & PVP_MODE::PvpFaction))
									pvpFactionBitmap->setTexture("pvp_enemy_tag.tga");
								else if (pPlayer->getPvpMode() & PVP_MODE::PvpFactionFlagged)
									pvpFactionBitmap->setTexture("pvp_enemy_flag.tga");
								else if (pPlayer->getPvpMode() & PVP_MODE::PvpFaction)
									pvpFactionBitmap->setTexture("pvp_enemy_tag.tga");
								else
									pvpFactionLogo->setActive(false);
							}
						}
					}
									
					if (pvpOutpostLogo)
					{
						if( pPlayer->getOutpostId() != 0 )
							pvpOutpostLogo->setActive(true);
						else
							pvpOutpostLogo->setActive(false);
					}
	
					if (pvpDuelLogo)
					{
						if( pPlayer->getPvpMode()&PVP_MODE::PvpDuel )
							pvpDuelLogo->setActive(true);
						else
							pvpDuelLogo->setActive(false);
					}

				}
				else
				{
					CInterfaceGroup* grp = info->getGroup("right_pvp");
					if (grp)
						info->delGroup(grp);
				}
			}

			// No bar and no string ?
			if (((stringCount == 1) && !barCount) || (stringCount == 0))
			{
				CViewBase *view = leftGroup->getView ("win_bot");
				if (view)
					leftGroup->delView (view);
				view = leftGroup->getView ("win_mid");
				if (view)
					leftGroup->delView (view);
				view = leftGroup->getView ("win_top");
				if (view)
					leftGroup->delView (view);

				// Delete
				view = leftGroup->getView ("win_jauge_top");
				if (view)
					leftGroup->delView (view);
				view = leftGroup->getView ("win_jauge_mid");
				if (view)
					leftGroup->delView (view);
				view = leftGroup->getView ("win_jauge_bot");
				if (view)
					leftGroup->delView (view);

			}

			// Delete remaining strings
			for (i=std::max(1,stringCount); i<3; i++)
			{
				CViewBase *text = leftGroup->getView ("info"+toString(i));
				if (text)
					text->setActive(false);
			}

			// Adjust win_mid
			CViewBase *win_mid = leftGroup->getView ("win_mid");
			if (win_mid)
			{
				win_mid->setH (win_mid->getH() - spaceBar/2 );

			}

			// Set player name
			if (info->_Name)
			{
				info->_Name->setText(entityName);
				info->_Name->setModulateGlobalColor(false);
			}

			// Set player title
			if (info->_Title)
				info->_Title->setText(entityTitle);

			// Set tribe name
			if (info->_TribeName)
			{
				nlassert(info->_GuildName == NULL);
				info->_TribeName->setText(theTribeName);
			}

			// Init user leaf nodes
			if (entity->isUser())
			{
				_Value = NLGUI::CDBManager::getInstance()->getDbProp ("UI:VARIABLES:CURRENT_SMOOTH_SERVER_TICK");
				_ValueBegin = NLGUI::CDBManager::getInstance()->getDbProp ("UI:VARIABLES:SMOOTH_USER_ACT_START");
				_ValueEnd = NLGUI::CDBManager::getInstance()->getDbProp ("UI:VARIABLES:SMOOTH_USER_ACT_END");
			}

			// Update data
			info->updateDynamicData ();

			// Activate it
			info->setActive(true);

			// Link to the interface
			CWidgetManager::getInstance()->addWindowToMasterGroup("ui:interface", info);
			CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
			info->setParent(pRoot);
			if (pRoot)
				pRoot->addGroup (info);

		}
	}

	return info;
}

// ***************************************************************************

class CHandlerResetCharacterInScene : public IActionHandler
{
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		bool	pvpOnly= nlstricmp(sParams,"pvponly")==0;
		// Reset all entities
		uint i;
		uint numEntity = (uint)EntitiesMngr.entities().size();
		for (i=0; i<numEntity; i++)
		{
			CEntityCL *entity = EntitiesMngr.entity(i);
			if (entity)
			{
				CCharacterCL *character = dynamic_cast<CCharacterCL*>(entity);
				if (character)
				{
					// filter if needed
					bool	rebuild= true;
					if(pvpOnly)
					{
						if( character->getPvpMode()==0 &&
							character->getOutpostId()==0 )
							rebuild= false;
					}

					// rebuild if needed
					if(rebuild)
						character->buildInSceneInterface ();
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerResetCharacterInScene, "reset_character_in_scene");


// ***************************************************************************
void CGroupInSceneUserInfo::getBarSettings( CInterfaceManager* pIM, bool isUser, bool isPlayer, bool isFriend, int &dbEntry, bool *bars )
{
	dbEntry = isUser?SAVE_USER:isFriend?SAVE_FRIEND:SAVE_ENEMY;
	// if currently is edition mode, then bars are not displayed
	if (ClientCfg.R2EDEnabled && R2::isEditionCurrent())
	{
		bars[HP]     = false;
		bars[SAP]    = false;
		bars[STA]    = false;
		bars[Focus]  = false;
		bars[Action] = false;
	}
	else
	{
		bars[HP] = _ConfigSaveInsceneDB[dbEntry].getHP()->getValueBool();
		bars[SAP] = (isUser || isFriend) && (isUser || isPlayer) && _ConfigSaveInsceneDB[dbEntry].getSap()->getValueBool();
		bars[STA] = (isUser || isFriend) && (isUser || isPlayer) && _ConfigSaveInsceneDB[dbEntry].getSta()->getValueBool();
		bars[Focus] = (isUser || isFriend) && (isUser || isPlayer) && _ConfigSaveInsceneDB[dbEntry].getFocus()->getValueBool();
		bars[Action] = (isUser) && _ConfigSaveInsceneDB[dbEntry].getAction()->getValueBool();
	}
}


// ***************************************************************************
void CGroupInSceneUserInfo::setLeftGroupActive( bool active )
{
	_IsLeftGroupActive = active;

	// The user and forage source cases are handled externally
	if ( _Entity->isUser() || _Entity->isForageSource() )
		return;

	int dbEntry;
	bool barSettings [NumBars];
	getBarSettings( CInterfaceManager::getInstance(), _Entity->isUser(), _Entity->isPlayer(), _Entity->isViewedAsFriend(), dbEntry, barSettings );

	// Show/hide bars
	bool atLeastOneBar = false;
	for ( uint i=0; i!=NumBars; ++i )
	{
		if ( barSettings[i] && _Bars[i] )
		{
			_Bars[i]->setActive(active);
			atLeastOneBar = true;
		}
	}

	if ( atLeastOneBar )
	{
		// Show/hide jauge
		CInterfaceGroup *leftGroup = getGroup ("right");
		if ( leftGroup )
		{
			CViewBase *view = leftGroup->getView ("win_jauge_mid");
			if (view)
				view->setActive(active);
			view = leftGroup->getView ("win_jauge_top");
			if (view)
				view->setActive(active);
			view = leftGroup->getView ("win_jauge_bot");
			if (view)
				view->setActive(active);
		}
	}
}


// Helper for updateDynamicData()
inline double getManhattanDistance( const CVectorD& vec )
{
	return (fabs(vec.x) + fabs(vec.y));
}


// ***************************************************************************
void CGroupInSceneUserInfo::updateDynamicData ()
{
	CRGBA	entityColor= _Entity->getColor();
	bool	isPvpColor= (_Entity->getPvpMode()!=PVP_MODE::None);

	// Set state fx
	CPlayerCL *pPlayer = dynamic_cast<CPlayerCL*>(_Entity);
	if (pPlayer != NULL)
	{
		if (pPlayer->isAFK())
			pPlayer->setStateFx("sp_medit.ps");
		else if (pPlayer->getStateFx() == "sp_medit.ps")
			pPlayer->removeStateFx();
	}
	
	if (_Entity->isDead())
			_Entity->setStateFx("misc_dead.ps");

	// Set entity data
	if (_Name)
	{
		_Name->setColor(entityColor);
		_Name->setModulateGlobalColor(false);
		ucstring entityName = _Entity->getDisplayName();
		if (entityName.empty())
			entityName = _Entity->getTitle();

		if (pPlayer != NULL)
			if (pPlayer->isAFK())
				entityName += CI18N::get("uiAFK");				
		_Name->setText(entityName);

		// Title color get the PVP color
		if (_Title)
		{
			if (isPvpColor)
				_Title->setColor(entityColor);
			else
				_Title->setColor(NLMISC::CRGBA(118, 153, 195, 255));
			_Title->setModulateGlobalColor(false);
		}
	}
	else
	{
		// Always set the title color as the entity color
		if (_Title)
		{
			_Title->setColor(entityColor);
			_Title->setModulateGlobalColor(false);
		}
	}

	// Set the guild name
	if (_GuildName)
	{
		STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
		ucstring ucsTmp;
		if (pSMC->getString (_Entity->getGuildNameID(), ucsTmp))
			_GuildName->setText(ucsTmp);

		// guildname color is the pvp color
		_GuildName->setColor(entityColor);
		_GuildName->setModulateGlobalColor(!isPvpColor);
	}

	// Set the guild symbol
	if (_Entity->getGuildSymbol() != 0)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		if (!_GuildIconLeaf[_Entity->slot()])
		{
			string dbLeaf = "UI:ENTITY:GUILD:"+toString (_Entity->slot())+":ICON";
			_GuildIconLeaf[_Entity->slot()] = NLGUI::CDBManager::getInstance()->getDbProp(dbLeaf);
		}
		nlassert(&*_GuildIconLeaf[_Entity->slot()]);
		(&*_GuildIconLeaf[_Entity->slot()])->setValue64(_Entity->getGuildSymbol());
	}

	// Set the event faction
	if (_EventFaction)
	{
		STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();
		ucstring ucsTmp;
		if (pSMC->getString (_Entity->getEventFactionID(), ucsTmp))
			_EventFaction->setText(ucsTmp);

		// guildname color depends of PVP faction or not
		_EventFaction->setColor(entityColor);
		_EventFaction->setModulateGlobalColor(!isPvpColor);
	}

	double manhattanDistance = getManhattanDistance( UserEntity->pos()-_Entity->pos() );
	// Does this entity target the user
	if (_Target)
	{
		bool active = _Entity->getTargetSlotNoLag() == 0; // slot 0 is the player
		if (manhattanDistance > CLFECOMMON::THRESHOLD_TARGET_ID_CLIENT_M)
			active = false;
		if (active != _Target->getActive())
			_Target->setActive(active);
	}

	// Is the mission target
	if (_MissionTarget)
	{
		bool active = _Entity->isMissionTarget ();
		if (active != _MissionTarget->getActive())
			_MissionTarget->setActive(active);
	}

	// If the entity is too far, hide the bars because their value is not updated by the server
	if ( manhattanDistance < CLFECOMMON::THRESHOLD_BARS_CLIENT_M )
	{
		if ( ! isLeftGroupActive() )
			setLeftGroupActive( true );

		// retrieve the bar info from the Bar Manager
		CBarManager::CBarInfo	barInfo;
		if(!_Entity->isForageSource())
		{
			barInfo= CBarManager::getInstance()->getBarsByEntityId(_Entity->slot());
		}
		// or directly from the forage source
		else
		{
			// NB: forage don't use CBarManager for 2 reasons: useless (forage bars exist only through VP),
			// and complicated since updated at each frame on client (because of smooth transition code)
			CForageSourceCL *forageSource = static_cast<CForageSourceCL*>(_Entity);
			barInfo.Score[SCORES::hit_points]=	forageSource->getTimeBar();		// Map TimeBar to HP
			barInfo.Score[SCORES::sap]= forageSource->getQuantityBar();			// Map QuantityBar to SAP
			barInfo.Score[SCORES::stamina]= forageSource->getDBar();			// Map D Bar to Sta
			barInfo.Score[SCORES::focus]= forageSource->getEBar();				// Map E Bar to Focus
		}

		// Set the bar
		if (_Bars[HP])
		{
			sint value = (sint)_BatLength * barInfo.Score[SCORES::hit_points] / RZ_BARS_LENGTH;
			if ( ! _Entity->isForageSource() )
			{
				if (value < 0)
				{
					value = -value;
					_Bars[HP]->setColorRGBA(BarColorHPNegative);
				}
				else
				{
					_Bars[HP]->setColorRGBA(BarColor[HP]);
				}
				// if dead creature, force the hp to 0
				if( _Entity->mode() == MBEHAV::DEATH && !_Entity->isPlayer() && !_Entity->isUser())
				{
					if( value > 0 )
					{
						value = 0;
					}
				}
			}
			else
			{
				// Forage source time bar: display in different colours if in extra time
				CForageSourceCL *forageSource = static_cast<CForageSourceCL*>(_Entity);
				if ( forageSource->isInExtraTime() )
				{
					_Bars[HP]->setColorRGBA(
						forageSource->isInProspectionExtraTime() ?
						CRGBA( 255, 0, 0 ) : // red
						CRGBA( 235, 144, 0 ) ); // orange
				}
			}
			clamp (value, 0, (sint)_BatLength);
			if(_Bars[HP]->getW() != value)
				_Bars[HP]->setWAndInvalidateCoords (value);
		}
		if (_Bars[SAP])
		{
			int value = _BatLength * barInfo.Score[SCORES::sap] / RZ_BARS_LENGTH;
			clamp (value, 0, (int)_BatLength);
			if(_Bars[SAP]->getW() != value)
				_Bars[SAP]->setWAndInvalidateCoords (value);

			// Forage source quantity bar: update contents value in tooltip
			if ( _Entity->isForageSource() )
			{
				CInterfaceGroup *group = getGroup ("right");
				CForageSourceCL *forageSource = static_cast<CForageSourceCL*>(_Entity);
				ucstring txt( CI18N::get( "uittForageContent" ) + toString( ": %u", forageSource->getCurrentQuantity() ) );
				CCtrlBase *toolTip = group->getCtrl ("tt1");
				if ( toolTip )
					toolTip->setDefaultContextHelp( txt );
				CCtrlBase *toolTip2 = group->getCtrl ("ttb1");
				if ( toolTip2 )
					toolTip2->setDefaultContextHelp( txt );
			}
		}
		if (_Bars[STA])
		{
			int value = _BatLength * barInfo.Score[SCORES::stamina] / RZ_BARS_LENGTH;
			clamp (value, 0, (int)_BatLength);
			if(_Bars[STA]->getW() != value)
			{
				// Forage source life bar: update danger colour
				if ( _Entity->isForageSource() )
				{
					CRGBA color = _Bars[STA]->getColorRGBA();
					color.blendFromuiRGBOnly( CRGBA( 255, 127, 127 ), CRGBA( 255, 0, 0 ), (RZ_BARS_LENGTH - barInfo.Score[SCORES::stamina]) * (256/RZ_BARS_LENGTH) );
					_Bars[STA]->setColorRGBA( color );
				}
				_Bars[STA]->setWAndInvalidateCoords (value);
			}
		}
		if (_Bars[Focus])
		{
			int value = _BatLength * barInfo.Score[SCORES::focus] / RZ_BARS_LENGTH;
			clamp (value, 0, (int)_BatLength);
			if(_Bars[Focus]->getW() != value)
			{
				// Forage source life bar: update danger colour
				if ( _Entity->isForageSource() )
				{
					CForageSourceCL *forageSource = static_cast<CForageSourceCL*>(_Entity);
					if ( forageSource->isSafe() )
					{
						_Bars[Focus]->setColorRGBA( CForageSourceCL::SafeSourceColor );
					}
					else
					{
						CRGBA color = _Bars[Focus]->getColorRGBA();
						color.blendFromuiRGBOnly( CRGBA( 255, 175, 0 ), CRGBA( 255, 0, 0 ), (RZ_BARS_LENGTH - barInfo.Score[SCORES::focus]) * (256/RZ_BARS_LENGTH) );
						_Bars[Focus]->setColorRGBA( color );
					}
				}
				_Bars[Focus]->setWAndInvalidateCoords (value);
			}
		}

		if (_Entity->isUser() && _Bars[Action])
		{
			sint64 value = 0;
			sint64 begin = 0;
			sint64 end = 1;
			if (_Value)
				value = _Value->getValue64();
			if (_ValueBegin)
				begin = _ValueBegin->getValue64();
			if (_ValueEnd)
				end = _ValueEnd->getValue64();
			end = max(begin, end);
			clamp (value, begin, end);
			if (end!=begin)
			{
				value = (sint32)_BatLength * (sint32)(value-begin) / (sint32)(end-begin);
				clamp (value, 0, (int)_BatLength);
			}
			else
			{
				value= 0;
			}

			if(_Bars[Action]->getW() != value)
				_Bars[Action]->setWAndInvalidateCoords ((sint32)value);
		}
		else if ( _Entity->isForageSource() && _Bars[Action] )
		{
			CForageSourceCL *forageSource = static_cast<CForageSourceCL*>(_Entity);
			sint32 value = (sint32)_BatLength * (sint32)forageSource->getKamiAngerBar() / RZ_BARS_LENGTH;
			if(_Bars[Action]->getW() != value)
				_Bars[Action]->setWAndInvalidateCoords(value);
		}
	}
	else
	{
		// The entity is too far => hide the bars
		if ( isLeftGroupActive() )
			setLeftGroupActive( false );
	}

	// For Entities, set some ZBias if the entity is selected or under cursor
	if(_Entity && !_Entity->isForageSource())
	{
		if( _Entity->slot()==SlotUnderCursor || _Entity->slot()==UserEntity->selection())
			setZBias(-2.f);
		else
			setZBias(0.f);
	}
}

// ***************************************************************************
CGroupInSceneUserInfo *CGroupInSceneUserInfo::newGroupInScene(const std::string &templateName, const std::string &id)
{
	CInterfaceGroup *groupInfo = NULL;
	CInterfaceManager *im = CInterfaceManager::getInstance();
	static volatile bool bypass = false;
	if (R2::getEditor().getMode() == R2::CEditor::EditionMode && !bypass)
	{
		// NB : only use the fast version during edition because duplication of CCDBCtrlSheet not implemented now, but we don't
		// use it for the edition !!!!
		CInterfaceElement *prototype = CWidgetManager::getInstance()->getElementFromId("ui:interface:" + templateName + "_proto");
		if (prototype && dynamic_cast<CInterfaceGroup *>(prototype))
		{
			extern bool NoOpForCCtrlSheetInfo_Serial; // CCDBCtrlSheet::serial not implemented, but prevent an assert in its serial because
													  // we don't use it in this special case (R2, Edition)
			NoOpForCCtrlSheetInfo_Serial = true;
			groupInfo = dynamic_cast<CInterfaceGroup *>(prototype->clone());
			NoOpForCCtrlSheetInfo_Serial = false;
			if (groupInfo)
			{
				groupInfo->setIdRecurse(id);
			}
		}
	}

	if (!groupInfo)
	{
		std::vector<std::pair<std::string,std::string> > templateParams;
		templateParams.push_back (std::pair<std::string,std::string>("id", id));
		groupInfo = CWidgetManager::getInstance()->getParser()->createGroupInstance ( templateName,
			"ui:interface", templateParams.empty()?NULL:&(templateParams[0]), (uint)templateParams.size());
	}

	CGroupInSceneUserInfo *info = dynamic_cast<CGroupInSceneUserInfo*>(groupInfo);
	if (!info)
	{
		delete groupInfo;
	}
	return info;
}

void CGroupInSceneUserInfo::serial(NLMISC::IStream &f)
{
	CGroupInScene::serial(f);
}

