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
#include "people_list.h"
#include "nel/gui/group_container.h"
#include "nel/gui/group_list.h"
#include "nel/gui/view_bitmap.h"
#include "interface_manager.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/group_editbox.h"
#include "../client_chat_manager.h"
#include "chat_text_manager.h"
#include "people_interraction.h"
#include "../user_entity.h"

using namespace std;
using namespace NLMISC;

////////////
// EXTERN //
////////////


extern NLMISC::CLog	g_log;
extern CClientChatManager   ChatMngr;

/////////////////////
// MEMBER FUNCTION //
/////////////////////

//==================================================================
CPeopleList::CPeopleList() : _ChatWindow(NULL),
							 _ContactType(CPeopleListDesc::Unknown),
	 						 _CurrPeopleID(0),
							 _Savable(false)

{
	// Construct
}

//==================================================================
bool CPeopleList::create(const CPeopleListDesc &desc, const CChatWindowDesc *chat /* = NULL*/)
{
	reset();
	// get father group
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CGroupContainer *fatherContainer = NULL;
	if (!desc.FatherContainer.empty())
	{
		fatherContainer = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(desc.FatherContainer));
		if (!fatherContainer)
		{
			nlwarning("<CPeopleList::create> Can't get father group, or bad type");
			return false;
		}
	}

	// create the base container
	vector< pair<string, string> > baseContainerParams;
	baseContainerParams.push_back(pair<string, string>("id", desc.Id));
	std::string baseId;
	if (fatherContainer == NULL)
	{
		baseContainerParams.push_back(pair<string, string>("movable","true"));
		baseContainerParams.push_back(pair<string, string>("active","false"));
		baseContainerParams.push_back(pair<string, string>("opened","true"));
		baseId = "ui:interface";
	}
	else
	{
		baseId = fatherContainer->getId() + ":list";
	}
	CInterfaceGroup *mainIg = CWidgetManager::getInstance()->getParser()->createGroupInstance(desc.BaseContainerTemplateName, baseId, baseContainerParams);
	// must attach group to hierarchy before we can use it
	CGroupContainer *gc = dynamic_cast<CGroupContainer  *>(mainIg);
	if (!gc) return false;
	if (fatherContainer != NULL)
	{
		fatherContainer->attachContainer(gc, desc.InsertPosition);
		fatherContainer->setup();
	}
	else
	{

		// Root container
		CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
		CWidgetManager::getInstance()->addWindowToMasterGroup("ui:interface", gc);
		gc->setParent(pRoot);
		pRoot->addGroup (gc);
	}

	// affect resource ptr
	_BaseContainer = gc ? gc->getId ():"";
	if (!_BaseContainer)
	{
		if (_ChatWindow) _ChatWindow->deleteContainer();
		delete mainIg;
		return false;
	}

	_BaseContainer->setOnActiveHandler(desc.AHOnActive);
	_BaseContainer->setOnActiveParams(desc.AHOnActiveParams);
	_BaseContainer->setOnDeactiveHandler(desc.AHOnDeactive);
	_BaseContainer->setOnDeactiveParams(desc.AHOnDeactiveParams);
	if (!desc.HeaderColor.empty())
		_BaseContainer->setHeaderColor(desc.HeaderColor);

	_BaseContainer->setSavable(desc.Savable);
	_BaseContainer->setLocalize(desc.Localize);
	if (desc.Localize)
		_BaseContainer->setTitle(desc.PeopleListTitle.toString());
	else
		_BaseContainer->setUCTitle(desc.PeopleListTitle);
	//_BaseContainer->setId("ui:interface:" + desc.Id);

	// create the chat window if there's one
	if (chat)
	{
		CChatWindowDesc chatDesc = *chat;
		chatDesc.FatherContainer = "";
		_ChatWindow = getChatWndMgr().createChatWindow(chatDesc);
		if (!_ChatWindow)
		{
			nlwarning("Couldn't create chat window");
		}
		else
		{
			_BaseContainer->attachContainer(_ChatWindow->getContainer(), desc.InsertPosition);
		}
	}

	_ContactType = desc.ContactType;
	_ContainerID = desc.Id;
	_Savable     = desc.Savable;

	return true;
}

//==================================================================
sint CPeopleList::getIndexFromName(const ucstring &name) const
{
	string sNameIn = strlwr(name.toString());
	for(uint k = 0; k < _Peoples.size(); ++k)
	{
		string sPeopleName = strlwr(_Peoples[k].getName().toString());
		if (sPeopleName == sNameIn) return k;
	}
	return -1;
}

//==================================================================
sint CPeopleList::getIndexFromContactId(uint32 contactId)
{
	for(uint k = 0; k < _Peoples.size(); ++k)
	{
		if(_Peoples[k].ContactId==contactId)
			return k;
	}
	return -1;
}

//==================================================================
sint CPeopleList::getIndexFromContainerID(const std::string &id) const
{
	for(uint k = 0; k < _Peoples.size(); ++k)
	{
		if (_Peoples[k].Container->getId() == id) return k;
	}
	return -1;
}

//==================================================================
bool CPeopleList::sortExByContactId(const CPeople& a, const CPeople& b)
{
	return (a.ContactId < b.ContactId);
}

//==================================================================
bool CPeopleList::sortExByName(const CPeople& a, const CPeople& b)
{
	ucstring name_a = toUpper(a.getName());
	ucstring name_b = toUpper(b.getName());
	
	return (name_a < name_b);
}

//==================================================================
bool CPeopleList::sortExByOnline(const CPeople& a, const CPeople& b)
{
	ucstring name_a = toUpper(a.getName());
	ucstring name_b = toUpper(b.getName());
	
	// We want order: online/alpha, offworld/alpha, offline/alpha
	if (a.Online == b.Online)
	{
		return (name_a < name_b);
	}
	else
	{
		// Compare online status
		switch (a.Online)
		{
			case ccs_online:
				// a is > if a is online
				return true;
				break;
			case ccs_online_abroad:
				// a is > if b is offline
				return (b.Online == ccs_offline);
				break;
			case ccs_offline:
			default:
				// b is always > if a is offline
				return false;
				break;
		}
	}

	// Should not get here so just return something
	return true;
}

//==================================================================
void CPeopleList::sortEx(TSortOrder order)
{
	// remove all people from the father container
	if (!_BaseContainer) return;
	uint k;
	for(k = 0; k < _Peoples.size(); ++k)
	{
		_BaseContainer->detachContainer(_Peoples[k].Container);
	}
	
	switch (order)
	{
		default:
		case sort_index:
			std::sort(_Peoples.begin(), _Peoples.end(), CPeopleList::sortExByContactId);
			break;
		case sort_name:
			std::sort(_Peoples.begin(), _Peoples.end(), CPeopleList::sortExByName);
			break;

		case sort_online:
			std::sort(_Peoples.begin(), _Peoples.end(), CPeopleList::sortExByOnline);
			break;
	}

	for(k = 0; k < _Peoples.size(); ++k)
	{
		_BaseContainer->attachContainer(_Peoples[k].Container);
	}
}

//==================================================================
void CPeopleList::sort()
{
	// remove all people from the father container
	if (!_BaseContainer) return;
	uint k;
	for(k = 0; k < _Peoples.size(); ++k)
	{
		_BaseContainer->detachContainer(_Peoples[k].Container);
	}
	std::sort(_Peoples.begin(), _Peoples.end());
	for(k = 0; k < _Peoples.size(); ++k)
	{
		_BaseContainer->attachContainer(_Peoples[k].Container);
	}
}

//==================================================================
bool CPeopleList::isPeopleChatVisible(uint index) const
{
	if (index >= _Peoples.size())
	{
		nlwarning("Bad index");
		return false;
	}
	return (_Peoples[index].Chat != NULL);
}
/*
bool CPeopleList::isPeopleWindowVisible(uint index) const
{
	if (index >= _Peoples.size())
	{
		nlwarning("Bad index");
		return false;
	}
	if (!_Peoples[index].Container) return false;
	if (_Peoples[index].Container->isOpen())
	{
		CInterfaceGroup *ig = _Peoples[index].Container;
		do
		{
			if (ig->isGroupContainer())
			{
				if (!static_cast<CGroupContainer *>(ig)->isOpen()) break;
			}
			if (!ig->getActive()) break;
			ig = ig->getParent();
		}
		while(ig);
		return ig == NULL; // all parent windows must be open & visible
	}
	else
	{
		return false;
	}
}
*/

//==================================================================
sint CPeopleList::addPeople(const ucstring &name, uint teamMateIndex /*= 0*/)
{
	if (!_BaseContainer) return - 1;
	// check if not already inserted
	if (getIndexFromName(name) != -1)
	{
		nlwarning("<CPeopleList::addPeople> people %s inserted twice.", name.toString().c_str());
	}

	vector<pair<string ,string> > properties;


	properties.push_back(make_pair(string("posparent"), string("parent")));
	properties.push_back(make_pair(string("id"), _ContainerID + "_" + toString(_CurrPeopleID)));
	properties.push_back(make_pair(string("title"), std::string("")));

	if (_ContactType == CPeopleListDesc::Team)
	{
		properties.push_back(make_pair(string("team_mate_index"), toString(teamMateIndex)));
	}

	std::string templateName;
	switch (_ContactType)
	{
		case CPeopleListDesc::Team:	templateName = "mate_id"; break;
		case CPeopleListDesc::Contact: templateName = "contact_id_friend"; break;
		case CPeopleListDesc::Ignore: templateName = "contact_id_ignore"; break;
		default:
			nlwarning("<CPeopleList::addPeople> Unknown contact type");
			return -1;
		break;
	}

	CInterfaceManager *im = CInterfaceManager::getInstance();
	CInterfaceGroup *group = CWidgetManager::getInstance()->getParser()->createGroupInstance(templateName, "ui:interface", properties, false);
	if (!group) return -1;
	CGroupContainer *gc = dynamic_cast<CGroupContainer *>(group);
	if (!gc)
	{
		delete group;
		nlwarning("<CPeopleList::addPeople> group is not a container.", name.toString().c_str());
		return -1;
	}
	// set title from the name
	gc->setUCTitle(name);
	// People inside list are not savable !
	gc->setSavable(false);
	//
	/*if (_ChatWindow)
	{
		_ChatWindow->getContainer()->attachContainer(gc);
	}
	else*/
	{
		_BaseContainer->attachContainer(gc);
	}

	CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
	pRoot->addGroup (gc);

	_Peoples.push_back(CPeople());
	_Peoples.back().Container = gc;
	_Peoples.back().GlobalID = _CurrPeopleID;
	updatePeopleMenu((uint)_Peoples.size() - 1);

	++_CurrPeopleID;

	return (sint) _Peoples.size() - 1;
}

//==================================================================
void CPeopleList::removePeople(uint index)
{
	if (index >= _Peoples.size())
	{
		nlwarning("<CPeopleList::removePeople> bad index.");
		return;
	}
	if (_Peoples[index].Container->isPopuped())
	{
		// isolate the window and then delete it
		_Peoples[index].Container->popin(-1, false);
	}
	else
	{
		if (_BaseContainer)
			_BaseContainer->detachContainer(_Peoples[index].Container);
	}
	CInterfaceManager *im = CInterfaceManager::getInstance();
	CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));

	pRoot->delGroup (_Peoples[index].Container);
	_Peoples.erase(_Peoples.begin() + index);
}

//==================================================================
uint32 CPeopleList::getContactId(uint index)
{
	if (index >= _Peoples.size())
	{
		nlwarning("<CPeopleList::getContactId> bad index.");
		return 0;
	}
	return _Peoples[index].ContactId;
}

//==================================================================
void CPeopleList::setContactId(uint index, uint32 contactId)
{
	if (index >= _Peoples.size())
	{
		nlwarning("<CPeopleList::setContactId> bad index.");
		return;
	}
	_Peoples[index].ContactId = contactId;
}

//==================================================================
void CPeopleList::displayLocalPlayerTell(const ucstring &receiver, uint index, const ucstring &msg,uint numBlinks /*=0*/)
{
	if (_ContactType == CPeopleListDesc::Ignore)
	{
		nlwarning("<CPeopleList::displayLocalPlayerTell> bad type, can't display message.");
		return;
	}
	if (index >= _Peoples.size())
	{
		nlwarning("<CPeopleList::displayLocalPlayerTell> bad index.");
		return;
	}
	// get the chat box inside the container
	CGroupContainer *gc = _Peoples[index].Chat;
	if (gc == NULL)
		return;
	CGroupList *gl = dynamic_cast<CGroupList *>(gc->getGroup("text_list"));
	if (!gl)
	{
		nlwarning("<CPeopleList::displayLocalPlayerTell> can't get group list.");
		return;
	}

 	ucstring csr(CHARACTER_TITLE::isCsrTitle(UserEntity->getTitleRaw()) ? "(CSR) " : "");
	ucstring finalMsg = csr + CI18N::get("youTell") + ": " + msg;
	// display msg with good color
	CInterfaceProperty prop;
	prop.readRGBA("UI:SAVE:CHAT:COLORS:TELL"," ");

	ucstring s = CI18N::get("youTellPlayer");
	strFindReplace(s, "%name", receiver);
	strFindReplace(finalMsg, CI18N::get("youTell"), s);
	gl->addChild(getChatTextMngr().createMsgText(finalMsg, prop.getRGBA()));
	CInterfaceManager::getInstance()->log(finalMsg, CChatGroup::groupTypeToString(CChatGroup::tell));

	// if the group is closed, make it blink
	if (!gc->isOpen())
	{
		if (numBlinks) gc->enableBlink(numBlinks);
	}
	if (_BaseContainer && !_BaseContainer->isOpen())
	{
		_BaseContainer->enableBlink(numBlinks);
	}
}


//==================================================================
void CPeopleList::displayMessage(uint index, const ucstring &msg, NLMISC::CRGBA col, uint /* numBlinks */ /*= 0*/)
{
	if (_ContactType == CPeopleListDesc::Ignore)
	{
		nlwarning("<CPeopleList::displayMessage> bad type, can't display message.");
		return;
	}
	if (index >= _Peoples.size())
	{
		nlwarning("<CPeopleList::displayMessage> bad index.");
		return;
	}
	// get the chat box inside the container
	CGroupContainer *gcChat = _Peoples[index].Chat;
	CGroupContainer *gcName = _Peoples[index].Container;
	if (gcName == NULL)
	{
		nlwarning("<CPeopleList::displayMessage> can't get group list.");
		return;
	}

	// if the group is closed, open it
	if (gcChat == NULL)
	{
		openCloseChat(index, true);
		gcChat = _Peoples[index].Chat;
	}
	if (gcChat == NULL)
	{
		nlwarning("<CPeopleList::displayMessage> cannot open chat.");
		return;
	}

	gcChat->requireAttention();

	CWidgetManager::getInstance()->setTopWindow(gcChat);

	CGroupList *gl = dynamic_cast<CGroupList *>(gcChat->getGroup("text_list"));
	if (gl == NULL)
	{
		nlwarning("<CPeopleList::displayMessage> can't get text_list.");
		return;
	}

	gl->addChild(getChatTextMngr().createMsgText(msg, col));
}


//==================================================================
void CPeopleList::swapPeople(uint index1, uint index2)
{
	if (index1 >= _Peoples.size() || index2 >= _Peoples.size())
	{
		nlwarning("<CPeopleList::swapPeople> bad index.");
		return;
	}
	if (_BaseContainer)
	{
		if (_BaseContainer->getList())
		{
			_BaseContainer->getList()->swapChildren(index1, index2);
		}
	}
}

//==================================================================
void CPeopleList::reset()
{
	// NB: must delete chat window before container
	if (_ChatWindow)
	{
		getChatWndMgr().removeChatWindow(_ChatWindow);
		_ChatWindow = NULL;
	}
	if (_BaseContainer)
	{
		removeAllPeoples();
		_BaseContainer->setContent(NULL);

		CGroupContainer *father = static_cast<CGroupContainer *>(_BaseContainer->getParent()->getEnclosingContainer());
		if (father)
		{
			father->delGroup(_BaseContainer);
			father->detachContainer(_BaseContainer);
			delete (CGroupContainer*)_BaseContainer;
		}
		else // detach from root
		{
			CInterfaceManager *im = CInterfaceManager::getInstance();
			CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
			pRoot->delGroup(_BaseContainer);
		}
		_BaseContainer = "";
	}
	_CurrPeopleID = 0;
	_ContactType = CPeopleListDesc::Unknown;

}

//==================================================================
void CPeopleList::removeAllPeoples()
{
	if (_BaseContainer)
	{
		_BaseContainer->removeAllContainers();
	}
	NLMISC::contReset(_Peoples);
}

//==================================================================
void CPeopleList::setPeopleMenu(const std::string &menuName)
{
	setPeopleMenuEx(menuName, menuName, menuName, menuName, menuName, menuName);
}

void CPeopleList::setPeopleMenuEx(const std::string &offlineUnblockedMenuName,
								  const std::string &onlineUnblockedMenuName,
								  const std::string &onlineAbroadUnblockedMenuName,
								  const std::string &offlineBockedMenuName,
								  const std::string &onlineBlockedMenuName,
								  const std::string &onlineAbroadBlockedMenuName
								  )
{
	_PeopleMenuOfflineUnblocked = offlineUnblockedMenuName;
	_PeopleMenuOnlineUnblocked = onlineUnblockedMenuName;
	_PeopleMenuOnlineAbroadUnblocked = onlineAbroadUnblockedMenuName;
	_PeopleMenuOfflineBlocked = offlineBockedMenuName;
	_PeopleMenuOnlineBlocked = onlineBlockedMenuName;
	_PeopleMenuOnlineAbroadBlocked = onlineAbroadBlockedMenuName;
	for(uint k = 0; k < _Peoples.size(); ++k)
	{
		updatePeopleMenu(k);
	}
}




//==================================================================
void CPeopleList::updatePeopleMenu(uint index)
{
	if (index >= _Peoples.size())
	{
		nlwarning("bad index");
		return;
	}
	std::string *peopleMenu;
	if (_Peoples[index].Blocked)
	{
		switch(_Peoples[index].Online)
		{
		case ccs_online:
			peopleMenu = &_PeopleMenuOnlineBlocked;
			break;
		case ccs_online_abroad:
			peopleMenu = &_PeopleMenuOnlineAbroadBlocked;
			break;
		default:
			peopleMenu = &_PeopleMenuOfflineBlocked;
			break;
		}
	}
	else
	{
		switch(_Peoples[index].Online)
		{
		case ccs_online:
			peopleMenu = &_PeopleMenuOnlineUnblocked;
			break;
		case ccs_online_abroad:
			peopleMenu = &_PeopleMenuOnlineAbroadUnblocked;
			break;
		default:
			peopleMenu = &_PeopleMenuOfflineUnblocked;
			break;
		}
	}
	if (_Peoples[index].Container->getHeaderOpened())
	{
		_Peoples[index].Container->getHeaderOpened()->setRightClickHandler("active_menu");
		_Peoples[index].Container->getHeaderOpened()->setRightClickHandlerParams("menu=" + *peopleMenu);
	}
	if (_Peoples[index].Container->getHeaderClosed())
	{
		_Peoples[index].Container->getHeaderClosed()->setRightClickHandler("active_menu");
		_Peoples[index].Container->getHeaderClosed()->setRightClickHandlerParams("menu=" + *peopleMenu);
	}
}

//==================================================================
ucstring CPeopleList::getName(uint index) const
{
	if (index >= _Peoples.size())
	{
		nlwarning("bad index");
		return ucstring("BAD INDEX!");
	}
	return _Peoples[index].getName();
}

//==================================================================
void CPeopleList::setMenu(const std::string &menuName)
{
	if (_BaseContainer)
	{
		if (_BaseContainer->getHeaderClosed())
		{
			_BaseContainer->getHeaderClosed()->setRightClickHandler("active_menu");
			_BaseContainer->getHeaderClosed()->setRightClickHandlerParams("menu=" + menuName);
		}
		if (_BaseContainer->getHeaderOpened())
		{
			_BaseContainer->getHeaderOpened()->setRightClickHandler("active_menu");
			_BaseContainer->getHeaderOpened()->setRightClickHandlerParams("menu=" + menuName);
		}
	}
	if (_ChatWindow)
	{
		if (_ChatWindow->getContainer()->getHeaderClosed())
		{
			_ChatWindow->getContainer()->getHeaderClosed()->setRightClickHandler("active_menu");
			_ChatWindow->getContainer()->getHeaderClosed()->setRightClickHandlerParams("menu=" + menuName);
		}
		if (_ChatWindow->getContainer()->getHeaderOpened())
		{
			_ChatWindow->getContainer()->getHeaderOpened()->setRightClickHandler("active_menu");
			_ChatWindow->getContainer()->getHeaderOpened()->setRightClickHandlerParams("menu=" + menuName);
		}
	}
}

//==================================================================
void CPeopleList::childrenMoved(uint srcIndex, uint destIndex, CGroupContainer * /* children */)
{
	CPeople pl = _Peoples[srcIndex];
	_Peoples.erase(_Peoples.begin() + srcIndex);
	_Peoples.insert(_Peoples.begin() + destIndex, pl);
}

//==================================================================
void CPeopleList::setOnline(uint index, TCharConnectionState online)
{
	if (index >= _Peoples.size())
	{
		nlwarning("bad index");
		return;
	}
	CGroupContainer *gc = _Peoples[index].Container;
	if (!gc) return;

	CInterfaceGroup *hc = gc->getHeaderClosed();
	if (hc != NULL)
	{
		CViewBitmap *onlineView = dynamic_cast<CViewBitmap*>(hc->getView("online"));
		if (onlineView != NULL)
		{
			CCtrlBase *toolTip = hc->getCtrl("tt_online");

			switch(online)
			{
			case ccs_online:
				onlineView->setTexture("w_online.tga");
				if (toolTip)
					toolTip->setDefaultContextHelp(CI18N::get("uittFriendsOnline"));
				break;
			case ccs_online_abroad:
				onlineView->setTexture("w_online_abroad.tga");
				if (toolTip)
					toolTip->setDefaultContextHelp(CI18N::get("uittFriendsOnlineAbroad"));
				break;
			default:
				onlineView->setTexture("w_offline.tga");
				if (toolTip)
					toolTip->setDefaultContextHelp(CI18N::get("uittFriendsOffline"));
				break;
			}
		}

		CCtrlBase *chatButton = hc->getCtrl("chat_button");
		if (chatButton != NULL)
			chatButton->setActive(online != ccs_offline);
	}

	_Peoples[index].Online = online;

	updatePeopleMenu(index);
}

//==================================================================
void CPeopleList::setBlocked(uint index, bool blocked)
{
	if (index >= _Peoples.size())
	{
		nlwarning("bad index");
		return;
	}
	CGroupContainer *gc = _Peoples[index].Container;
	if (!gc) return;
	CInterfaceGroup *ho = gc->getHeaderOpened();
	if (ho)
	{
		CViewBase *blockedView = ho->getView("blocked");
		if (blockedView)
		{
			blockedView->setActive(blocked);
		}
		else
		{
			nlwarning("Can't retrieve 'online' bitmap");
		}
	}
	CInterfaceGroup *hc = gc->getHeaderClosed();
	if (hc)
	{
		CViewBase *blockedView = hc->getView("blocked");
		if (blockedView)
		{
			blockedView->setActive(blocked);
		}
		else
		{
			if (_ContactType != CPeopleListDesc::Team)
				nlwarning("Can't retrieve 'online' bitmap");
		}
	}
	_Peoples[index].Blocked = blocked;
	updatePeopleMenu(index);
}

//==================================================================
TCharConnectionState CPeopleList::getOnline(uint index) const
{
	if (index >= _Peoples.size())
	{
		nlwarning("bad index");
		return ccs_offline;
	}
	return _Peoples[index].Online;
}

//==================================================================
bool CPeopleList::getBlocked(uint index) const
{
	if (index >= _Peoples.size())
	{
		nlwarning("bad index");
		return false;
	}
	return _Peoples[index].Blocked;
}

//==================================================================
void CPeopleList::openCloseChat(sint index, bool bOpen)
{
	if (!_BaseContainer) return ;
	nlassert((index >= 0)&&(index < (sint)_Peoples.size()));

	CChatGroupWindow *pCGW = PeopleInterraction.getChatGroupWindow();
	nlassert(PeopleInterraction.ChatGroup.Window != NULL);
	if (_ContactType == CPeopleListDesc::Team)
		_Peoples[index].Chat = pCGW->createFreeTeller(_Peoples[index].getName(), "UI:SAVE:WIN:COLORS:MEM");
	else
		_Peoples[index].Chat = pCGW->createFreeTeller(_Peoples[index].getName(), "UI:SAVE:WIN:COLORS:COM");

	if (_Peoples[index].Chat == NULL)
		return;
	_Peoples[index].Chat->setActive(bOpen);
}

//==================================================================
class CHandlerContactEntry : public IActionHandler
{
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		CGroupEditBox *pEB = dynamic_cast<CGroupEditBox*>(pCaller);
		if (pEB == NULL) return;
		ucstring text = pEB->getInputString();
		// If the line is empty, do nothing
		if(text.size() == 0)
			return;

		// Parse any tokens in the text
		if ( ! CInterfaceManager::parseTokens(text))
		{
			pEB->setInputString (string(""));
			return;
		}

		// is it a command ?
		if(text[0] == '/')
		{
			CChatWindow::_ChatWindowLaunchingCommand = NULL; // no CChatWindow instance there ..
			// TODO : have NLMISC::ICommand accept unicode strings
			std::string str = text.toUtf8().substr(1);
			NLMISC::ICommand::execute( str, g_log );
			pEB->setInputString (string(""));
			return;
		}
		// Well, we could have used CChatWindow class to handle this, but CPeopleList was written earlier, so for now
		// it is simpler to keep it as it and to just use this action handler to manage user input.
		if (!pCaller || !pCaller->getParent()) return;
		CGroupContainer *gc = static_cast< CGroupContainer* >( pCaller->getParent()->getEnclosingContainer() );

		// title gives the name of the player
		ucstring playerName = gc->getUCTitle();

		// Simply do a tell on the player
		ChatMngr.tell(playerName.toString(), text);
		pEB->setInputString (string(""));
		if (gc)
		{
			// Restore position of enclosing container if it hasn't been moved/scaled/poped by the user
			if (!gc->getTouchFlag(true))
			{
				gc->restorePosition();
			}

			// Retrieve name of the container in the list
			string ui_interface_free_chat = "ui:interface:free_chat";
			string str = gc->getId().substr(0, ui_interface_free_chat.length());
			if (str != ui_interface_free_chat)
			{
				string str2 = gc->getId().substr(gc->getId().rfind('_')+1,gc->getId().size());
				str = str.substr(0,str.rfind('_'));
				str = str.substr(str.rfind(':')+1, str.size());
				str = "ui:interface:" + str + "_" + str2;

				CPeopleList *peopleList;
				uint index;
				if (PeopleInterraction.getPeopleFromContainerID(str, peopleList, index))
				{
					peopleList->displayLocalPlayerTell(str2, index, text);
				}
			}
			else
			{
				str = gc->getId();
				CChatGroupWindow *pWin = PeopleInterraction.getChatGroupWindow();
				CInterfaceProperty prop;
				prop.readRGBA("UI:SAVE:CHAT:COLORS:SPEAKER"," ");
				ucstring final;
				CChatWindow::encodeColorTag(prop.getRGBA(), final, false);

				ucstring csr(CHARACTER_TITLE::isCsrTitle(UserEntity->getTitleRaw()) ? "(CSR) " : "");
				final += csr + CI18N::get("youTell")+": ";
				prop.readRGBA("UI:SAVE:CHAT:COLORS:TELL"," ");
				CChatWindow::encodeColorTag(prop.getRGBA(), final, true);
				final += text;
				pWin->displayTellMessage(final, prop.getRGBA(), pWin->getFreeTellerName(str));

				ucstring s = CI18N::get("youTellPlayer");
				strFindReplace(s, "%name", pWin->getFreeTellerName(str));
				strFindReplace(final, CI18N::get("youTell"), s);
				CInterfaceManager::getInstance()->log(final, CChatGroup::groupTypeToString(CChatGroup::tell));
			}
		}
	}
};
REGISTER_ACTION_HANDLER( CHandlerContactEntry, "contact_entry");
