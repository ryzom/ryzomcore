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
#include "guild_manager.h"

#include "interface_manager.h"
#include "../string_manager_client.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/view_text.h"
#include "dbctrl_sheet.h"
#include "nel/gui/group_container.h"
#include "nel/gui/group_menu.h"
#include "nel/gui/group_html.h"
#include "../init_main_loop.h"
#include "inventory_manager.h"

#include "../connection.h"
#include "../entity_cl.h"
#include "../user_entity.h" // UserEntity
#include "nel/gui/view_bitmap.h"
#include "../sheet_manager.h"
#include "../net_manager.h"
#include "../client_sheets/building_sheet.h"

#include "game_share/lift_icons.h"

#include "../r2/editor.h"
#include "chat_window.h"
#include "people_interraction.h"

#include "../misc.h"

using namespace std;
using namespace NLMISC;

extern CPeopleInterraction PeopleInterraction;

NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupListAscensor, std::string, "list_sheet_guild");

// ***************************************************************************
// Interface part
// ***************************************************************************

#define WIN_GUILD							"ui:interface:guild"
#define WIN_GUILD_CHAT						"ui:interface:guild_chat"
#define WIN_GUILD_FORUM						"ui:interface:guild_forum"
#define VIEW_TEXT_GUILD_QUIT				"ui:interface:guild:content:tab_guild:quit_guild"
#define CTRL_SHEET_GUILD_BLASON				"ui:interface:guild:content:tab_guild:blason"
#define VIEW_TEXT_GUILD_MEMBER_COUNT		"ui:interface:guild:content:tab_guild:member_count"


#define LIST_GUILD_MEMBERS					"ui:interface:guild:content:tab_guild:list_member:guild_members"
#define CTRL_QUIT_GUILD						"ui:interface:guild:content:tab_guild:quit_guild"
#define TEMPLATE_GUILD_MEMBER				"member_template"
#define TEMPLATE_GUILD_MEMBER_NAME			"name"
#define TEMPLATE_GUILD_MEMBER_GRADE			"grade"
#define TEMPLATE_GUILD_MEMBER_SCORE			"score"
#define TEMPLATE_GUILD_MEMBER_ENTER_DATE	"enter_date"
#define MENU_GUILD_MEMBER					"ui:interface:menu_member"

#define WIN_ASCENSOR						"ui:interface:ascensor_teleport_list"

#define WIN_JOIN_PROPOSAL					"ui:interface:join_guild_proposal"
#define VIEW_JOIN_PROPOSAL_PHRASE			"ui:interface:join_guild_proposal:content:inside:phrase"

CGuildManager* CGuildManager::_Instance = NULL;

// ***************************************************************************
CGuildManager::CGuildManager()
{
	// TEMP TEMP TEMP&
	initForDebug();

	initDBObservers();
	_NeedRebuild = true;
	_NeedRebuildMembers = true;
	_NeedUpdate = false;
	_NeedUpdateMembers = false;
	_InGuild = false;
	_JoinPropUpdate = false;
	_NewToTheGuild = false;
}

// ***************************************************************************
CGuildManager::~CGuildManager()
{
}


static inline bool lt_member_name(const SGuildMember &m1, const SGuildMember &m2)
{
	return  toLower(m1.Name) < toLower(m2.Name);
}

static inline bool lt_member_grade(const SGuildMember &m1, const SGuildMember &m2)
{
	return m1.Grade < m2.Grade;
}

static inline bool lt_member_online(const SGuildMember &m1, const SGuildMember &m2)
{
	if (m1.Online == m2.Online)
	{
		return lt_member_grade(m1, m2);
	}

	// Compare online status
	switch (m1.Online)
	{
		case ccs_online:
			// m1 is < if m1 is online
			return true;
			break;
		case ccs_online_abroad:
			// m1 is < if m2 is offline
			return (m2.Online == ccs_offline);
			break;
		case ccs_offline:
		default:
			// m2 is always < if m1 is offline
			return false;
			break;
	}
}


// ***************************************************************************
void CGuildManager::sortGuildMembers(TSortOrder order)
{
	if (_GuildMembers.size() < 2) return;

	switch (order)
	{
		default:
		case sort_grade:
			sort(_GuildMembers.begin(), _GuildMembers.end(), lt_member_name);
			stable_sort(_GuildMembers.begin(), _GuildMembers.end(), lt_member_grade);
			break;
		case sort_name:
			sort(_GuildMembers.begin(), _GuildMembers.end(), lt_member_name);
			break;
		case sort_online:
			sort(_GuildMembers.begin(), _GuildMembers.end(), lt_member_name);
			stable_sort(_GuildMembers.begin(), _GuildMembers.end(), lt_member_online);
			break;
	}
}

bool CGuildManager::isProxy()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	return NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:PROXY")->getValueBool();
}


// ***************************************************************************
/*void CGuildManager::init (const std::vector< std::pair<uint32,uint8> > &NameGrade)
{
	_GuildMembers.clear();
	_GuildMembers.resize(NameGrade.size());
	for (uint32 i = 0; i < NameGrade.size(); ++i)
	{
		_GuildMembers[i].Index = i;
		_GuildMembers[i].NameID = NameGrade[i].first;
		_GuildMembers[i].Grade = (EGSPD::CGuildGrade::TGuildGrade)(NameGrade[i].second&0x7f);
		_GuildMembers[i].Online = ((NameGrade[i].second&0x80) != 0);
	}
}*/

// ***************************************************************************
/*void CGuildManager::set (uint32 indexMember, uint32 MemberName, uint8 MemberGrade, bool bOnline)
{
	uint32 i;
	for (i = 0; i < _GuildMembers.size(); ++i)
	{
		if (_GuildMembers[i].Index == indexMember)
			break;
	}

	// indexMember not found create it
	if (i == _GuildMembers.size())
	{
		_GuildMembers.resize(_GuildMembers.size()+1);
		_GuildMembers[i].Index = indexMember;
	}

	if ((MemberName == 0) && (MemberGrade == (uint32)EGSPD::CGuildGrade::EndGuildGrade))
	{
		// Delete entry
		_GuildMembers.erase(_GuildMembers.begin()+i);
	}
	else
	{
		// Setup entry
		_GuildMembers[i].NameID = MemberName;
		_GuildMembers[i].Grade = (EGSPD::CGuildGrade::TGuildGrade)MemberGrade;
		_GuildMembers[i].Online = bOnline;
	}

	rebuild();
}*/

// ***************************************************************************
bool CGuildManager::isInGuild()
{
	// InGuild is set from database (if the guild name id != 0)
	return _InGuild;
}

// ***************************************************************************
bool CGuildManager::canRecruit()
{
	if (!_InGuild) return false;
	if ((_Grade == EGSPD::CGuildGrade::Leader) || (_Grade == EGSPD::CGuildGrade::HighOfficer) ||
		(_Grade == EGSPD::CGuildGrade::Officer) /*|| (_Grade == EGSPD::CGuildGrade::Recruiter)*/)
		return true;
	return false;
}

// ***************************************************************************
bool CGuildManager::isLeaderOfTheGuild()
{
	return (_Grade == EGSPD::CGuildGrade::Leader);
}

// ***************************************************************************
ucstring CGuildManager::getGuildName()
{
	if (_InGuild)
		return _Guild.Name;
	return ucstring("");
}

// ***************************************************************************
uint64 CGuildManager::getMoney()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	return NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:INVENTORY:MONEY")->getValue64();
}

// ***************************************************************************
uint64 CGuildManager::getXP()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	return NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:XP")->getValue64();
}

// ***************************************************************************
void CGuildManager::update()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();

	// *** Need to rebuild the guild data?
	if (_NeedRebuild)
	{
		_NeedUpdate = true;
		_NeedRebuild = false;

		// Rebuild transfert the database to the local structure

		// Guild stuff
		uint32 oldName = _Guild.NameID;
		_Guild.NameID = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:NAME")->getValue32();
		_Guild.Name = "";
		_InGuild = (_Guild.NameID != 0);
		if (!_InGuild)
			closeAllInterfaces();
		_Guild.Icon = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:ICON")->getValue64();
		_Guild.QuitGuildAvailable = true;

		// Guild Members
		if(_NeedRebuildMembers)
		{
			_NeedUpdateMembers = true;
			_NeedRebuildMembers = false;

			_GuildMembers.clear();
			for (uint32 i = 0; i < MAX_GUILD_MEMBER; ++i)
			{
				sint32 name = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:MEMBERS:"+toString(i)+":NAME")->getValue32();
				if (name != 0)
				{
					SGuildMember gm;
					gm.NameID = name;
					gm.Index = i;
					gm.Grade = (EGSPD::CGuildGrade::TGuildGrade)(NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:MEMBERS:"+toString(i)+":GRADE")->getValue32());
					gm.Online = (TCharConnectionState)(NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:MEMBERS:"+toString(i)+":ONLINE")->getValue32());
					gm.EnterDate = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:MEMBERS:"+toString(i)+":ENTER_DATE")->getValue32();
					_GuildMembers.push_back(gm);
				}
			}
		}

		// Does the player are newcomer ?
		// Boris 01/09/2006 : removed : now the guild interface is open if
		// is was active before OR if the EGS ask it to the client

		bool playerNewToTheGuild = _NewToTheGuild &&(oldName != _Guild.NameID) && _InGuild;
		if (playerNewToTheGuild)
		{
			// reset the flag
			_NewToTheGuild = false;
			// Don't pop the guild window in ring mode.
			// NB nico : this test should not be necessary, as the guild infos should be filled during the init of the db
			// However, there are situation where this db info is filled after init (should only happen at guild creation time ...)
			// Maybe an EGS  bug ?
			if (R2::getEditor().getMode() == R2::CEditor::NotInitialized)
			{
				CInterfaceElement *pElt;
				// Open the guild info if we are not in the init phase
				if (!IngameDbMngr.initInProgress())
				{
					pElt = CWidgetManager::getInstance()->getElementFromId(WIN_GUILD);
					if (pElt != NULL)
						pElt->setActive(true);
				}
				// Browse the forum
				pElt = CWidgetManager::getInstance()->getElementFromId(WIN_GUILD_FORUM":content:html");
				if (pElt != NULL)
				{
					CGroupHTML *html = dynamic_cast<CGroupHTML*>(pElt);
					if (html)
						html->browse("home");
				}
			}
		}
	}

	// *** Need to update Names?
	if (_NeedUpdate)
	{
		bool bAllValid = true;
		// Update wait until all the name of members, name of the guild and description are valid

		if (!pSMC->getString (_Guild.NameID, _Guild.Name)) bAllValid = false;

		for (uint i = 0; i < _GuildMembers.size(); ++i)
		{
			if (!pSMC->getString (_GuildMembers[i].NameID, _GuildMembers[i].Name)) bAllValid = false;
			else _GuildMembers[i].Name = CEntityCL::removeTitleAndShardFromName(_GuildMembers[i].Name);
		}

		// If all is valid no more need update and if guild is opened update the interface
		if (bAllValid)
		{
			CCDBNodeLeaf* node = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CHAT:SHOW_ONLINE_OFFLINE_NOTIFICATIONS_CB", false);
			if (node && node->getValueBool())
			{
				// See if we need to show any online/offline messages
				static map<ucstring, SGuildMember> CachedGuildMembers;
				ucstring onlineMessage = CI18N::get("uiPlayerOnline");
				ucstring offlineMessage = CI18N::get("uiPlayerOffline");

				for (uint i = 0; i < _GuildMembers.size(); ++i)
				{
					map<ucstring, SGuildMember>::const_iterator it = CachedGuildMembers.find(_GuildMembers[i].Name);
					if ( it != CachedGuildMembers.end() )
					{
						if ( (*it).second.Online == _GuildMembers[i].Online)
						{
							// Online status not changed for this member
							continue;
						}
						
						if ( (*it).second.Online != ccs_offline && _GuildMembers[i].Online != ccs_offline)
						{
							// Not from offline, or to offline, so don't show anything
							continue;
						}
						
						ucstring msg = (_GuildMembers[i].Online != ccs_offline) ? onlineMessage : offlineMessage;
						strFindReplace(msg, "%s", _GuildMembers[i].Name);
						string cat = getStringCategory(msg, msg);
						map<string, CClientConfig::SSysInfoParam>::const_iterator it;
						NLMISC::CRGBA col = CRGBA::Yellow;
						it = ClientCfg.SystemInfoParams.find(toLower(cat));
						if (it != ClientCfg.SystemInfoParams.end())
						{
							col = it->second.Color;
						}
						bool dummy;
						PeopleInterraction.ChatInput.Guild.displayMessage(msg, col, 2, &dummy);
						break;

					}
				}

				CachedGuildMembers.clear();
				for (uint i = 0; i < _GuildMembers.size(); ++i)
				{
					CachedGuildMembers.insert(make_pair(_GuildMembers[i].Name, _GuildMembers[i]));
				}
			}

			// Search for UserEntity to find our own grade
			if ((UserEntity != NULL) && (_GuildMembers.size() > 0))
			{
				uint i;
				_Grade = EGSPD::CGuildGrade::Member;
				ucstring sUserName = toLower(UserEntity->getEntityName());
				for (i = 0; i < _GuildMembers.size(); ++i)
				{
					if (toLower(_GuildMembers[i].Name) == sUserName)
					{
						_Grade = _GuildMembers[i].Grade;
						break;
					}
				}
			}

			// set this value in the database
			NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER:GUILD_GRADE")->setValue32(_Grade);

			// update the guild display
			CGroupContainer *pGuild = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_GUILD));
			if (pGuild != NULL)
			{
				// if the guild window is visible
				if (pGuild->isOpen() && pGuild->getActive())
				{
					// Close the modal window if the member list will change
					if(CWidgetManager::getInstance()->getModalWindow()!=NULL && _NeedUpdateMembers)
					{
						if (CWidgetManager::getInstance()->getModalWindow()->getId() == MENU_GUILD_MEMBER )
							CWidgetManager::getInstance()->disableModalWindow();
					}

					// Rebuild interface. Rebuild members only if needed
					CAHManager::getInstance()->runActionHandler("guild_sheet_open", NULL, toString("update_members=%d", (uint)_NeedUpdateMembers) );
				}
			}

			// guild updated
			_NeedUpdate = false;
			_NeedUpdateMembers= false;
		}
	}

	// *** Join proposal handling
	if (_JoinPropUpdate)
	{
		bool bAllValid = true;
		if (!pSMC->getDynString (_JoinPropPhraseID, _JoinPropPhrase)) bAllValid = false;
		// If all is valid no more need update and update the interface
		if (bAllValid)
		{
			_JoinPropUpdate = false;
			CGroupContainer *pJoinProp = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_JOIN_PROPOSAL));
			if (pJoinProp != NULL)
			{
				CViewText *pJoinPropPhraseView = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(VIEW_JOIN_PROPOSAL_PHRASE));
				if (pJoinPropPhraseView != NULL)
					pJoinPropPhraseView->setText(_JoinPropPhrase);

				pJoinProp->setActive(true);
				CWidgetManager::getInstance()->setTopWindow(pJoinProp);
				pJoinProp->updateCoords();
				pJoinProp->center();
				pJoinProp->enableBlink(2);
			}
		}
	}
}

// ***************************************************************************
void CGuildManager::launchAscensor()
{
	// Start the huge list exchange
	Ascensors.start();

	// Increment session id
	Ascensors.incrementSessionID();

	// Send request of the first page to the server specifying the session id
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("GUILD:FIRST_ASCENSOR_PAGE", out))
	{
		uint16 session = Ascensors.getSessionID();
		out.serial(session);
		NetMngr.push(out);
		//nlinfo("impulseCallBack : GUILD:FIRST_ASCENSOR_PAGE %d sent",session);
	}
	else
	{
		nlwarning("impulseCallBack : unknown message name : 'GUILD:FIRST_ASCENSOR_PAGE'.");
	}

	// Start Ascensor Interface
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupContainer *pAC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_ASCENSOR));
	if (pAC == NULL) return;
	pAC->setActive(true);
	CWidgetManager::getInstance()->setTopWindow(pAC);
}

// TEMP TEMP TEMP

NLMISC_COMMAND(testAscensorPage, "Temp : Simulate the server that fills the database for ascensor","")
{
	CInterfaceManager *im = CInterfaceManager::getInstance();
	uint64 prop;
	//
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:0:NAME")->setValue32(12);
	prop = CGuildManager::iconMake(1, 1, false, CRGBA(255, 255, 0), CRGBA(0, 255, 0));
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:0:ICON")->setValue64(prop);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:1:NAME")->setValue32(12);
	prop = CGuildManager::iconMake(1, 2, false, CRGBA(0, 255, 255), CRGBA(255, 0, 255));
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:1:ICON")->setValue64(prop);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:2:NAME")->setValue32(12);
	prop = CGuildManager::iconMake(2, 3, false, CRGBA(255, 0, 0), CRGBA(0, 255, 0));
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:2:ICON")->setValue64(prop);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:3:NAME")->setValue32(12);
	prop = CGuildManager::iconMake(2, 4, false, CRGBA(255, 255, 0), CRGBA(0, 255, 255));
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:3:ICON")->setValue64(prop);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:4:NAME")->setValue32(12);
	prop = CGuildManager::iconMake(3, 5, false, CRGBA(255, 255, 0), CRGBA(255, 0, 255));
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:4:ICON")->setValue64(prop);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:5:NAME")->setValue32(12);
	prop = CGuildManager::iconMake(3, 6, false, CRGBA(0, 255, 255), CRGBA(255, 255, 0));
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:5:ICON")->setValue64(prop);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:6:NAME")->setValue32(12);
	prop = CGuildManager::iconMake(4, 7, false, CRGBA(0, 255, 255), CRGBA(255, 0, 255));
	NLGUI::CDBManager::getInstance()->getDbProp("SERVER:ASCENSOR:6:ICON")->setValue64(prop);
	//
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:7:NAME")->setValue32(12);
	prop = CGuildManager::iconMake(4, 8, false, CRGBA(255, 0, 255), CRGBA(0, 255, 255));
	NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:7:ICON")->setValue64(prop);

	return true;
}

// ***************************************************************************
void CGuildManager::quitAscensor()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupContainer *pAC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_ASCENSOR));
	if (pAC == NULL) return;
	pAC->setActive(false);
}

// ***************************************************************************
void CGuildManager::launchJoinProposal(uint32 phraseID)
{
	_JoinPropPhraseID = phraseID;
	_JoinPropUpdate = true;
}

// ***************************************************************************
void CGuildManager::quitJoinProposal()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupContainer *pJoinProp = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_JOIN_PROPOSAL));
	if (pJoinProp != NULL)
		pJoinProp->setActive(false);
}

// ***************************************************************************
void CGuildManager::closeAllInterfaces()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGroupContainer *pGuild = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_GUILD));
	if (pGuild != NULL)
		pGuild->setActive(false);
	CGroupContainer *pGuildForum = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_GUILD_FORUM));
	if (pGuildForum != NULL)
		pGuildForum->setActive(false);
	CGroupContainer *pGuildChat = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(WIN_GUILD_CHAT));
	if (pGuildChat != NULL)
		pGuildChat->setActive(false);
}

// ***************************************************************************
void CGuildManager::openGuildWindow()
{
	_NewToTheGuild = true;
//	CInterfaceManager *pIM = CInterfaceManager::getInstance();
//	// Open the guild window
//
//	CCDBNodeLeaf	*node= NLGUI::CDBManager::getInstance()->getDbProp("CLIENT:GUILD:HAVE_JOINED", false);
//	if(node)
//		node->setValue64(1);
//
//	CInterfaceElement *pElt;
//	pElt = CWidgetManager::getInstance()->getElementFromId(WIN_GUILD);
//	if (pElt != NULL)
//	{
//		pElt->setActive(true);
//	}
//	// Browse the forum
//	pElt = CWidgetManager::getInstance()->getElementFromId(WIN_GUILD_FORUM":content:html");
//	if (pElt != NULL)
//	{
//		CGroupHTML *html = dynamic_cast<CGroupHTML*>(pElt);
//		if (html)
//			html->browse("home");
//	}
}

// ***************************************************************************
void CGuildManager::initForDebug()
{
	_InGuild = true;
	_Guild.Name = "TrapsGuild";
	_Guild.Icon = CGuildManager::iconMake(1,1,false,CRGBA(255,0,255), CRGBA(0,255,255));

	_Grade = EGSPD::CGuildGrade::Officer;

	_GuildMembers.clear();
	SGuildMember m;
	m.Grade = EGSPD::CGuildGrade::Leader;
	m.Name = "Trap";
//	m.Score = 645;
	_GuildMembers.push_back(m);
	m.Grade = EGSPD::CGuildGrade::Officer;
	m.Name = "Nico";
//	m.Score = 354;
	_GuildMembers.push_back(m);
	m.Grade = EGSPD::CGuildGrade::HighOfficer;
	m.Name = "Vianney";
//	m.Score = 1657;
	_GuildMembers.push_back(m);
	m.Grade = EGSPD::CGuildGrade::Member;
	m.Name = "Hulud";
//	m.Score = 16;
	_GuildMembers.push_back(m);
	m.Grade = EGSPD::CGuildGrade::Member;
	m.Name = "Geoffroy";
//	m.Score = 64;
	_GuildMembers.push_back(m);
//	m.Grade = EGSPD::CGuildGrade::Bearer;
	m.Name = "Yoyo";
//	m.Score = 16570;
	_GuildMembers.push_back(m);
//	m.Grade = EGSPD::CGuildGrade::Recruiter;
	m.Name = "Norm";
//	m.Score = 10021;
	_GuildMembers.push_back(m);
//	m.Grade = EGSPD::CGuildGrade::Recruiter;
	m.Name = "Nygot";
//	m.Score = 102651;
	_GuildMembers.push_back(m);
	m.Grade = EGSPD::CGuildGrade::Member;
	m.Name = "Alex";
//	m.Score = 1465;
	_GuildMembers.push_back(m);
	m.Grade = EGSPD::CGuildGrade::Member;
	m.Name = "EricSimon";
//	m.Score = 654;
	_GuildMembers.push_back(m);
}

// ***************************************************************************
void CGuildManager::initDBObservers()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// add an observer on the whole guild
	NLGUI::CDBManager::getInstance()->addBranchObserver( "SERVER:GUILD", &_DBObs );

	// add an observer on members only => need to update all
	NLGUI::CDBManager::getInstance()->addBranchObserver("SERVER:GUILD:MEMBERS", &_DBObsMembers);

	// observer on ascencors
	Ascensors.setListType(CHugeListObs::Ascensor);
	NLGUI::CDBManager::getInstance()->addBranchObserver("SERVER:ASCENSOR", &Ascensors);
}

// ***************************************************************************
// CDBGroupListAscensor
// ***************************************************************************

// **********************************************************************************
void CDBGroupListAscensor::CSheetChildAscensor::init(CDBGroupListSheetText *pFather, uint index)
{
	// init my parent
	CSheetChild::init(pFather, index);

	Setuped = false;
	Index = index;
	SecondSheetIdCache = 0;
}

// **********************************************************************************
bool CDBGroupListAscensor::CSheetChildAscensor::isInvalidated(CDBGroupListSheetText * /* pFather */)
{
	if (Ctrl->getSheetId() != SecondSheetIdCache)
	{
		SecondSheetIdCache = Ctrl->getSheetId();
		Setuped = false;
	}

	if (!Setuped)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		STRING_MANAGER::CStringManagerClient *pSMC = STRING_MANAGER::CStringManagerClient::instance();

		uint32 nameID = NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:" + toString(Index) + ":NAME")->getValue32();
		ucstring name;
		if (nameID && pSMC->getDynString(nameID, name))
		{
			Text->setText(name);

			uint64 icon = NLGUI::CDBManager::getInstance()->getDbProp("LOCAL:ASCENSOR:" + toString(Index) + ":ICON")->getValue64();

			// Slot setup
			if ((icon & UINT64_CONSTANT(0x8000000000000000)) != 0)
			{

				LIFT_ICONS::TLiftIcon li = (LIFT_ICONS::TLiftIcon)(icon & UINT64_CONSTANT(0x7FFFFFFFFFFFFFFF));
				string str = strlwr(LIFT_ICONS::toString(li));
				Ctrl->setType(CCtrlSheetInfo::SheetType_Teleport_Location);
				Ctrl->setSlot("asc_"+str+".tga");
			}
			else // Guild icon
			{
				Ctrl->setType(CCtrlSheetInfo::SheetType_GuildFlag);
				Ctrl->initSheetSize();
				Ctrl->setSheetId(SecondSheetIdCache);
			}

			Setuped = true;
			return true;
		}
	}

	return false;
}

// **********************************************************************************
bool CDBGroupListAscensor::CSheetChildAscensor::isSheetValid(CDBGroupListSheetText * /* pFather */)
{
	if (!Ctrl) return false;
	return Ctrl->getSheetId() != 0;
}

// ***************************************************************************
// CDBObs
// ***************************************************************************

// ***************************************************************************
void CGuildManager::CDBObs::update(ICDBNode* /* node */)
{
	CGuildManager *pGM = CGuildManager::getInstance();
	pGM->rebuildBasic();
}

void CGuildManager::CDBObsMembers::update(ICDBNode* /* node */)
{
	CGuildManager *pGM = CGuildManager::getInstance();
	pGM->rebuildBasicAndMembers();
}

// ***************************************************************************
// ACTION HANDLERS
// ***************************************************************************

// ***************************************************************************
class CAHGuildSheetOpen : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGuildManager *pGM = CGuildManager::getInstance();
		bool	updateMembers= getParam(Params, "update_members")=="1";


		// *** Update interface with data of Guild Manager
		const SGuild &rGuild = pGM->getGuild();

		// Freeze / unfreeze quit button
		CCtrlBaseButton *control = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(VIEW_TEXT_GUILD_QUIT));
		if (control)
			control->setFrozen (!rGuild.QuitGuildAvailable || pGM->isProxy());


		// *** Update Members, if necessary
		if(updateMembers)
		{
			CGuildManager::TSortOrder order = (CGuildManager::TSortOrder)(NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:GUILD_LIST:SORT_ORDER")->getValue32());
			// Sort the members in Guild Manager
			pGM->sortGuildMembers(order);

			// update member count view
			const vector<SGuildMember> &rGuildMembers = pGM->getGuildMembers();
			CViewText	*pVT = dynamic_cast<CViewText*>(CWidgetManager::getInstance()->getElementFromId(VIEW_TEXT_GUILD_MEMBER_COUNT));
			if (pVT)
 				pVT->setText(toString(rGuildMembers.size()));

			// rebuild guild member list
			CGroupList *pParent = dynamic_cast<CGroupList*>(CWidgetManager::getInstance()->getElementFromId(LIST_GUILD_MEMBERS));
			if (pParent == NULL) return;
			pParent->clearGroups();
			pParent->setDynamicDisplaySize(false);
			for (uint i = 0; i < rGuildMembers.size(); i++)
			{
				// create the member line
				string templateId = LIST_GUILD_MEMBERS ":m" + toString(i);
				vector< pair<string, string> > vParams;
				vParams.push_back(vector< pair<string, string> >::value_type("id", templateId));
				CInterfaceGroup *pLine = NULL;
				pLine = CWidgetManager::getInstance()->getParser()->createGroupInstance (TEMPLATE_GUILD_MEMBER, LIST_GUILD_MEMBERS, vParams);
				if (pLine == NULL) continue;

				// Set name
				CViewText *pViewName = dynamic_cast<CViewText*>(pLine->getView(TEMPLATE_GUILD_MEMBER_NAME));
				if (pViewName != NULL)
					pViewName->setText (rGuildMembers[i].Name);

				// Set Grade
				CViewText *pViewGrade = dynamic_cast<CViewText*>(pLine->getView(TEMPLATE_GUILD_MEMBER_GRADE));
				if (pViewGrade != NULL)
				{
					if (rGuildMembers[i].Grade == EGSPD::CGuildGrade::Leader)
						pViewGrade->setText (CI18N::get("uiGuildLeader"));
					else if (rGuildMembers[i].Grade == EGSPD::CGuildGrade::HighOfficer)
						pViewGrade->setText (CI18N::get("uiGuildHighOfficer"));
					else if (rGuildMembers[i].Grade == EGSPD::CGuildGrade::Officer)
						pViewGrade->setText (CI18N::get("uiGuildOfficer"));
					else
						pViewGrade->setText (CI18N::get("uiGuildMember"));
				}

				// online?
				CViewBitmap *onlineView = dynamic_cast<CViewBitmap*>(pLine->getView("online"));
				if (onlineView != NULL)
				{
					CCtrlBase *toolTip = pLine->getCtrl("tt_online");

					switch(rGuildMembers[i].Online)
					{
					case ccs_online:
						onlineView->setTexture("w_online.tga");
						if (toolTip)
							toolTip->setDefaultContextHelp(CI18N::get("uittGuildMemberOnline"));
						break;
					case ccs_online_abroad:
						onlineView->setTexture("w_online_abroad.tga");
						if (toolTip)
							toolTip->setDefaultContextHelp(CI18N::get("uittGuildMemberOnlineAbroad"));
						break;
					default:
						onlineView->setTexture("w_offline.tga");
						if (toolTip)
							toolTip->setDefaultContextHelp(CI18N::get("uittGuildMemberOffline"));
						break;
					}
				}

				// Enter Date
				CViewText *pViewEnterDate = dynamic_cast<CViewText*>(pLine->getView(TEMPLATE_GUILD_MEMBER_ENTER_DATE));
				if (pViewEnterDate != NULL)
				{
					CRyzomTime rt;
					rt.updateRyzomClock(rGuildMembers[i].EnterDate);
					ucstring str = toString("%04d", rt.getRyzomYear()) + " ";
					str += CI18N::get("uiJenaYear") + " : ";
					str += CI18N::get("uiAtysianCycle") + " ";
					str += toString("%01d", rt.getRyzomCycle()+1) +", ";
					str += CI18N::get("ui"+MONTH::toString( (MONTH::EMonth)rt.getRyzomMonthInCurrentCycle() )) + ", ";
					str += toString("%02d", rt.getRyzomDayOfMonth()+1);
					pViewEnterDate->setText(str);
				}

				// Add to the list
				pLine->setParent (pParent);
				pParent->addChild (pLine);
			}
		}
	}
};
REGISTER_ACTION_HANDLER (CAHGuildSheetOpen, "guild_sheet_open");

// ***************************************************************************
static void setRights(bool lead, bool hioff, bool offi, bool recr, bool bear, bool memb, bool kick)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CViewTextMenu *pVTM;
	pVTM = dynamic_cast<CViewTextMenu*>(CWidgetManager::getInstance()->getElementFromId(string(MENU_GUILD_MEMBER":lead")));
	if (pVTM != NULL) pVTM->setGrayed(!lead);
	pVTM = dynamic_cast<CViewTextMenu*>(CWidgetManager::getInstance()->getElementFromId(string(MENU_GUILD_MEMBER":hiof")));
	if (pVTM != NULL) pVTM->setGrayed(!hioff);
	pVTM = dynamic_cast<CViewTextMenu*>(CWidgetManager::getInstance()->getElementFromId(string(MENU_GUILD_MEMBER":offi")));
	if (pVTM != NULL) pVTM->setGrayed(!offi);
	pVTM = dynamic_cast<CViewTextMenu*>(CWidgetManager::getInstance()->getElementFromId(string(MENU_GUILD_MEMBER":recr")));
	if (pVTM != NULL) pVTM->setGrayed(!recr);
	pVTM = dynamic_cast<CViewTextMenu*>(CWidgetManager::getInstance()->getElementFromId(string(MENU_GUILD_MEMBER":bear")));
	if (pVTM != NULL) pVTM->setGrayed(!bear);
	pVTM = dynamic_cast<CViewTextMenu*>(CWidgetManager::getInstance()->getElementFromId(string(MENU_GUILD_MEMBER":memb")));
	if (pVTM != NULL) pVTM->setGrayed(!memb);
	pVTM = dynamic_cast<CViewTextMenu*>(CWidgetManager::getInstance()->getElementFromId(string(MENU_GUILD_MEMBER":kick")));
	if (pVTM != NULL) pVTM->setGrayed(!kick);
}

// ***************************************************************************
class CAHGuildSheetMenuOpen : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGuildManager *pGM = CGuildManager::getInstance();
		const vector<SGuildMember> &rGuildMembers = pGM->getGuildMembers();

		// *** Check and retrieve the current member index (index in the member list)
		CCtrlBase	*ctrlLaunchingModal= CWidgetManager::getInstance()->getCtrlLaunchingModal();
		if (pCaller == NULL || ctrlLaunchingModal == NULL)
		{
			// Error -> Close
			CWidgetManager::getInstance()->disableModalWindow();
			return;
		}
		string sId = ctrlLaunchingModal->getId();
		sId = sId.substr(sId.rfind('m')+1,sId.size());
		sint32 nLineNb;
		fromString(sId, nLineNb);
		if ((nLineNb < 0) || (nLineNb >= (sint32)rGuildMembers.size()))
		{
			// Error -> Close
			CWidgetManager::getInstance()->disableModalWindow();
			return;
		}
		MemberIndexSelected= nLineNb;
		MemberNameSelected= rGuildMembers[nLineNb].Name;

		// If the member is not yet received, do not allow any operation
		if(MemberNameSelected.empty())
		{
			// Error -> Close
			CWidgetManager::getInstance()->disableModalWindow();
			return;
		}

		// *** Check with the grade of the local player which types of actions we can do on the player selected

		// enable or disable menu entries
		if (pGM->isProxy())
		{
			// no action allowed on proxy guild
			setRights(false, false, false, false, false, false, false);
		}
		else
		{
			// Depending on the grade we can do things or other

			// Grade less or equal can't do anything
			if (pGM->getGrade() >= rGuildMembers[nLineNb].Grade)
				setRights(false, false, false, false, false, false, false);
			else if (pGM->getGrade() == EGSPD::CGuildGrade::Leader)
				setRights(true, true, true, true, true, true, true);
			else if (pGM->getGrade() == EGSPD::CGuildGrade::HighOfficer)
				setRights(false, false, true, true, true, true, true);
			else if (pGM->getGrade() == EGSPD::CGuildGrade::Officer)
				setRights(false, false, false, false, true, true, true);
			else
				setRights(false, false, false, false, false, false, false);
		}
	}

public:
	// Current selection
	static sint32	MemberIndexSelected;		// Index of the member selected when right clicked
	static ucstring	MemberNameSelected;			// Name of the member selected when right clicked (for extra check)
};
REGISTER_ACTION_HANDLER (CAHGuildSheetMenuOpen, "guild_member_menu_open");

sint32		CAHGuildSheetMenuOpen::MemberIndexSelected= -1;
ucstring	CAHGuildSheetMenuOpen::MemberNameSelected;


// ***************************************************************************
// Use the control launching modal to know the text to send to the server
static void sendMsgSetGrade(EGSPD::CGuildGrade::TGuildGrade Grade)
{
	const string &message = "GUILD:SET_GRADE";
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CGuildManager *pGM = CGuildManager::getInstance();
	const vector<SGuildMember> &rGuildMembers = pGM->getGuildMembers();

	// check the member selected is still valid
	sint32 MemberNb= CAHGuildSheetMenuOpen::MemberIndexSelected;
	if ((MemberNb < 0) || (MemberNb >= (sint32)rGuildMembers.size())) return;
	// double check the name (in case of some change in the guild members list)
	if(rGuildMembers[MemberNb].Name != CAHGuildSheetMenuOpen::MemberNameSelected)
		return;

	// Ok! let's send the message!
	CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream(message, out))
	{
		uint16 u16MemberNb = (uint16)rGuildMembers[MemberNb].Index;
		out.serial(u16MemberNb);
		uint8 u8Grade = Grade;
		out.serial(u8Grade);
		uint8 u8Counter = (uint8)NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:COUNTER")->getValue8();
		out.serial(u8Counter);
		NetMngr.push(out);
		//nlinfo("impulseCallBack : %s %d %d %d sent", message.c_str(), u16MemberNb, u8Grade, u8Counter);
	}
	else
	{
		nlwarning("<CHandlerAcceptExchange::execute> unknown message name '%s'", message.c_str());
	}
}

// ***************************************************************************
// Sort the guild member list
class CAHGuildSheetSortGuildList : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &/* sParams */)
	{
		CInterfaceManager* pIM= CInterfaceManager::getInstance();
		CGuildManager::TSortOrder order = (CGuildManager::TSortOrder)(NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:GUILD_LIST:SORT_ORDER")->getValue32());

		order = (CGuildManager::TSortOrder)(order + 1);
		if (order == CGuildManager::END_SORT_ORDER)
		{
			order = CGuildManager::START_SORT_ORDER;
		}

		NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:GUILD_LIST:SORT_ORDER")->setValue32((sint32)order);
		CAHManager::getInstance()->runActionHandler("guild_sheet_open", NULL, toString("update_members=1"));
	}
};
REGISTER_ACTION_HANDLER(CAHGuildSheetSortGuildList, "sort_guild_list");

// ***************************************************************************
// Invoke the 'tell' command on a contact from its menu
// The tell command is displayed in the 'around me' window
class CAHGuildSheetTellMember : public IActionHandler
{
public:
	void execute (CCtrlBase * pCaller, const std::string &/* sParams */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGuildManager *pGM = CGuildManager::getInstance();
		const vector<SGuildMember> &rGuildMembers = pGM->getGuildMembers();
		// *** Check and retrieve the current member index (index in the member list)
		CCtrlBase	*ctrlLaunchingModal= CWidgetManager::getInstance()->getCtrlLaunchingModal();
		if (pCaller == NULL)
		{
			// Error -> Close
			return;
		}
		string sId = pCaller->getId();
		sId = sId.substr(sId.rfind('m')+1,sId.size());
		sint32 nLineNb;
		fromString(sId, nLineNb);
		if ((nLineNb < 0) || (nLineNb >= (sint32)rGuildMembers.size()))
		{
			// Error -> Close
			return;
		}
		MemberIndexSelected= nLineNb;
		MemberNameSelected = rGuildMembers[nLineNb].Name;

		CPeopleInterraction::displayTellInMainChat(MemberNameSelected);	
	}

	// Current selection
	static sint32	MemberIndexSelected;		// Index of the member selected when left clicked
	static ucstring	MemberNameSelected;			// Name of the member selected when lef clicked
};
REGISTER_ACTION_HANDLER(CAHGuildSheetTellMember, "guild_tell_member");

sint32		CAHGuildSheetTellMember::MemberIndexSelected= -1;
ucstring	CAHGuildSheetTellMember::MemberNameSelected;

// ***************************************************************************
class CAHGuildSheetSetLeader : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		// Ask if they are sure
		pIM->validMessageBox(CInterfaceManager::QuestionIconMsg, CI18N::get("uiQSetLeader"), "guild_member_do_change_leader");
	}
};
REGISTER_ACTION_HANDLER (CAHGuildSheetSetLeader, "guild_member_chg_to_leader");
		
// ***************************************************************************
class CAHGuildSheetSetLeaderConfirm : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		// Leadership change confirmed
		sendMsgSetGrade(EGSPD::CGuildGrade::Leader);
	}
};
REGISTER_ACTION_HANDLER (CAHGuildSheetSetLeaderConfirm, "guild_member_do_change_leader");



// ***************************************************************************
class CAHGuildSheetSetHighOfficer : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		sendMsgSetGrade(EGSPD::CGuildGrade::HighOfficer);
	}
};
REGISTER_ACTION_HANDLER (CAHGuildSheetSetHighOfficer, "guild_member_chg_to_high_officer");

// ***************************************************************************
class CAHGuildSheetSetOfficer : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		sendMsgSetGrade(EGSPD::CGuildGrade::Officer);
	}
};
REGISTER_ACTION_HANDLER (CAHGuildSheetSetOfficer, "guild_member_chg_to_officer");

// ***************************************************************************
class CAHGuildSheetSetRecruiter : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		//sendMsgSetGrade(EGSPD::CGuildGrade::Recruiter);
	}
};
REGISTER_ACTION_HANDLER (CAHGuildSheetSetRecruiter, "guild_member_chg_to_recruiter");

// ***************************************************************************
class CAHGuildSheetSetBearer : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		//sendMsgSetGrade(EGSPD::CGuildGrade::Bearer);
	}
};
REGISTER_ACTION_HANDLER (CAHGuildSheetSetBearer, "guild_member_chg_to_bearer");

// ***************************************************************************
class CAHGuildSheetSetMember : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		sendMsgSetGrade(EGSPD::CGuildGrade::Member);
	}
};
REGISTER_ACTION_HANDLER (CAHGuildSheetSetMember, "guild_member_chg_to_member");

// ***************************************************************************
class CAHGuildSheetKick : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		const string &message = "GUILD:KICK_MEMBER";
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CGuildManager *pGM = CGuildManager::getInstance();
		const vector<SGuildMember> &rGuildMembers = pGM->getGuildMembers();

		// check the member selected is still valid
		sint32 MemberNb= CAHGuildSheetMenuOpen::MemberIndexSelected;
		if ((MemberNb < 0) || (MemberNb >= (sint32)rGuildMembers.size())) return;
		// double check the name (in case of some change in the guild members list)
		if(rGuildMembers[MemberNb].Name != CAHGuildSheetMenuOpen::MemberNameSelected)
			return;

		// Ok! let's send the message!
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream(message, out))
		{
			uint16 u16MemberNb = (uint16)rGuildMembers[MemberNb].Index;
			out.serial(u16MemberNb);
			uint8 u8Counter = (uint8)NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:COUNTER")->getValue8();
			out.serial(u8Counter);
			NetMngr.push(out);
			//nlinfo("impulseCallBack : %s %d %d sent", message.c_str(), u16MemberNb, u8Counter);
		}
		else
		{
			nlwarning("<CHandlerAcceptExchange::execute> unknown message name '%s'", message.c_str());
		}
	}
};
REGISTER_ACTION_HANDLER (CAHGuildSheetKick, "guild_member_kick");

// ***************************************************************************
class CHandlerAscensorTeleport : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
	{
		CDBCtrlSheet *ctrlSheet = dynamic_cast<CDBCtrlSheet *>(pCaller);
		if (!ctrlSheet) return;

		uint16 nTeleportLocation = ctrlSheet->getIndexInParent();

		const string msgName = "GUILD:TELEPORT";
		CBitMemStream out;
		if(GenericMsgHeaderMngr.pushNameToStream(msgName, out))
		{
			out.serial( nTeleportLocation );
			NetMngr.push(out);

			// Last teleport is an elevator
			LoadingBackground = ElevatorBackground;
		}
		//nlinfo("impulseCallBack : GUILD:TELEPORT %d sent", nTeleportLocation);
	}
};
REGISTER_ACTION_HANDLER (CHandlerAscensorTeleport, "ascensor_teleport");

/*
// ***************************************************************************
class CHandlerInvGuildToBag : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CDBCtrlSheet *pCSDst = dynamic_cast<CDBCtrlSheet*>(pCaller);
		if (!pCSDst->isSheetValid()) return;
		string sTmp = pCSDst->getSheet();

		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(sTmp+":SHEET",false);
		CCDBNodeLeaf *pNLquantity = NLGUI::CDBManager::getInstance()->getDbProp(sTmp+":QUANTITY",false);
		if (pNL == NULL) return;
		if (pNLquantity == NULL) return;

		sTmp = sTmp.substr(sTmp.rfind(':')+1, sTmp.size());
		uint8 GuildIndex;
		fromString(sTmp, GuildIndex);

		CInventoryManager *pInv = CInventoryManager::getInstance();
		uint32 quantity = pNLquantity->getValue32();
		double totalBulk = quantity * pInv->getItemBulk(pNL->getValue32());

		// Only check with the bag for the moment

		bool bPlaceFound = false;
		if (pInv->isInventoryAvailable(INVENTORIES::bag))
			if ((pInv->getBagBulk(0) + totalBulk) <= pInv->getMaxBagBulk(0))
				bPlaceFound = true;

		if (!bPlaceFound)
		{
			ucstring msg = CI18N::get("msgCantPutItemInBag");
			string cat = getStringCategory(msg, msg);
			pIM->displaySystemInfo(msg, cat);
			return;
		}
		// Look for a free entry in the bag
		bPlaceFound = false;
		uint8 BagIndex;
		for (uint32 i = 0; i < MAX_BAGINV_ENTRIES; ++i)
		{
			if (getInventory().getBagItem(i).getSheetID() == 0)
			{
				BagIndex = (uint8)i;
				bPlaceFound = true;
				break;
			}
		}

		if (!bPlaceFound) return;

		uint16 Session = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:INVENTORY:SESSION")->getValue16();

		CBitMemStream out;
		const string sMsg = "GUILD:GUILD_TO_BAG";
		if (!GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
		{
			nlwarning ("don't know message name %s", sMsg.c_str());
		}
		else
		{
			// Fill the message (temporary inventory slot)
			out.serial(GuildIndex);
			out.serial(Session);
			NetMngr.push (out);
			nlinfo("impulseCallBack : %s %d %d sent", sMsg.c_str(), GuildIndex, Session);
		}
	}
};
REGISTER_ACTION_HANDLER (CHandlerInvGuildToBag, "inv_guild_to_bag");



// ***************************************************************************
class CHandlerInvBagToGuild : public IActionHandler
{
	virtual void execute (CCtrlBase *pCaller, const string &Params)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		CDBCtrlSheet *pCS = dynamic_cast<CDBCtrlSheet*>(pCaller);
		string sTmp = pCS->getSheet();
		sTmp = sTmp.substr(sTmp.rfind(':')+1, sTmp.size());
		uint8 BagIndex;
		fromString(sTmp, BagIndex);

		uint16 Session = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:INVENTORY:SESSION")->getValue16();

		CBitMemStream out;
		const string sMsg = "GUILD:BAG_TO_GUILD";
		if (!GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
		{
			nlwarning ("don't know message name %s", sMsg.c_str());
		}
		else
		{
			// Fill the message (temporary inventory slot)
			out.serial(BagIndex);
			out.serial(Session);
			NetMngr.push (out);
			nlinfo("impulseCallBack : %s %d %d sent", sMsg.c_str(), BagIndex, Session);
		}
	}
};
REGISTER_ACTION_HANDLER (CHandlerInvBagToGuild, "inv_bag_to_guild");
*/

// ***************************************************************************
static void sendMoneyServerMessage(const string &sMsg)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	uint64 nMoney = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:CHOOSE_MONEY")->getValue64();
	uint16 Session = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:INVENTORY:SESSION")->getValue16();
	CBitMemStream out;
	if (!GenericMsgHeaderMngr.pushNameToStream(sMsg, out))
	{
		nlwarning ("don't know message name %s", sMsg.c_str());
	}
	else
	{
		// Fill the message (temporary inventory slot)
		out.serial(nMoney);
		out.serial(Session);
		NetMngr.push (out);
		//nlinfo("impulseCallBack : %s %d %d sent", sMsg.c_str(), nMoney, Session);
	}
}

// ***************************************************************************
class CHandlerGuildInvGetMoney : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		sendMoneyServerMessage("GUILD:TAKE_MONEY");
		CWidgetManager::getInstance()->popModalWindow();
	}
};
REGISTER_ACTION_HANDLER (CHandlerGuildInvGetMoney, "guild_inv_get_money");


// ***************************************************************************
class CHandlerGuildInvPutMoney : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		sendMoneyServerMessage("GUILD:PUT_MONEY");
		CWidgetManager::getInstance()->popModalWindow();
	}
};
REGISTER_ACTION_HANDLER (CHandlerGuildInvPutMoney, "guild_inv_put_money");
