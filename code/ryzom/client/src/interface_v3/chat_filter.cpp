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
#include "chat_filter.h"
#include "../client_chat_manager.h"
#include "people_list.h"
#include "../client_cfg.h"
#include "../net_manager.h"
#include "interface_manager.h"

using namespace std;
using NLMISC::CI18N;

extern CClientChatManager ChatMngr;


// ***************************************************************************
// ***************************************************************************
// CChatInputFilter //
// ***************************************************************************
// ***************************************************************************


//=============================================================================================================
CChatInputFilter::~CChatInputFilter()
{
	// Destruct
	for(std::vector<CChatWindow *>::iterator it = _ListeningWindows.begin(); it != _ListeningWindows.end(); ++it)
	{
		(*it)->removeObserver(this);
	}
}

//=============================================================================================================
void CChatInputFilter::setWindowState(CChatWindow *cw, bool listening)
{
	if (listening)
	{
		if (!isListeningWindow(cw))
		{
			addListeningWindow(cw);
		}
	}
	else
	{
		if (isListeningWindow(cw))
		{
			removeListeningWindow(cw);
		}
	}
}

//=============================================================================================================
void CChatInputFilter::addListeningPeopleList(CPeopleList *pl)
{
	if (!pl)
	{
		nlwarning("Can't add NULL people list");
		return;
	}
	if (isListeningPeopleList(pl))
	{
		nlwarning("People list already added");
		return;
	}
	_ListeningPeopleList.push_back(pl);
}

//=============================================================================================================
void CChatInputFilter::removeListeningPeopleList(CPeopleList *pl)
{
	std::vector<CPeopleList *>::iterator it = std::find(_ListeningPeopleList.begin(), _ListeningPeopleList.end(), pl);
	if (it == _ListeningPeopleList.end())
	{
		nlwarning("Bad people list, can't remove");
		return;
	}
	_ListeningPeopleList.erase(it);
}

//=============================================================================================================
bool CChatInputFilter::isListeningPeopleList(CPeopleList *pl) const
{
	std::vector<CPeopleList *>::const_iterator it = std::find(_ListeningPeopleList.begin(), _ListeningPeopleList.end(), pl);
	return it != _ListeningPeopleList.end();
}

//=============================================================================================================
void CChatInputFilter::addListeningWindow(CChatWindow *w)
{
	if (!w)
	{
		nlwarning("Can't add NULL window");
		return;
	}
	if (isListeningWindow(w))
	{
		nlwarning("Window already added");
		return;
	}
	_ListeningWindows.push_back(w);
	w->addObserver(this);
}

//=============================================================================================================
void CChatInputFilter::removeListeningWindow(CChatWindow *w)
{
	std::vector<CChatWindow *>::iterator it = std::find(_ListeningWindows.begin(), _ListeningWindows.end(), w);
	if (it == _ListeningWindows.end())
	{
		nlwarning("Bad window, can't remove");
		return;
	}
	(*it)->removeObserver(this);
	_ListeningWindows.erase(it);
}

//=============================================================================================================
bool CChatInputFilter::isListeningWindow(CChatWindow *w) const
{
	std::vector<CChatWindow *>::const_iterator it = std::find(_ListeningWindows.begin(), _ListeningWindows.end(), w);
	return it != _ListeningWindows.end();
}


//=============================================================================================================
CChatWindow *CChatInputFilter::getListeningWindow(uint index)
{
	if (index > _ListeningWindows.size())
	{
		nlwarning("bad index");
		return NULL;
	}
	return _ListeningWindows[index];
}


//=============================================================================================================
void CChatInputFilter::chatWindowRemoved(CChatWindow *cw)
{
	std::vector<CChatWindow *>::iterator it = std::find(_ListeningWindows.begin(), _ListeningWindows.end(), cw);
	if (it != _ListeningWindows.end())
	{
		_ListeningWindows.erase(it);
	}
	else
	{
		nlwarning("<CChatInputFilter::chatWindowRemoved> Not a listening window of this chat filter");
	}
}

//=============================================================================================================
void CChatInputFilter::displayMessage(const ucstring &msg, NLMISC::CRGBA col, uint numBlinks /*=0*/, bool *windowVisible)
{
	bool windowVisibleTmp = false;
	std::vector<CChatWindow *>::iterator it;
	for(it = _ListeningWindows.begin(); it != _ListeningWindows.end(); ++it)
	{
		windowVisibleTmp |= (*it)->isVisible();
	}
	// If at least one window is visible, no need to make the window blink.
	for(it = _ListeningWindows.begin(); it != _ListeningWindows.end(); ++it)
	{
		(*it)->displayMessage(msg, col, FilterType, DynamicChatDbIndex, windowVisibleTmp ? 0  : numBlinks, NULL);
	}
	if (windowVisible) *windowVisible = windowVisibleTmp;
}

//=============================================================================================================
void CChatInputFilter::displayTellMessage(/*TDataSetIndex &receiverIndex, */const ucstring &msg, const ucstring &sender, NLMISC::CRGBA col, uint numBlinks /*=0*/,bool *windowVisible /*=NULL*/)
{
	ucstring senderLwr;
	senderLwr.fromUtf8(NLMISC::toLower(sender.toUtf8()));

	// look in people lists
	std::vector<CPeopleList *>::iterator peopleListIt;

	for(peopleListIt = _ListeningPeopleList.begin(); peopleListIt != _ListeningPeopleList.end(); ++peopleListIt)
	{
		CPeopleList *pPList = *peopleListIt;
		if (pPList != NULL)
		{
			sint peopleIndex = pPList->getIndexFromName(senderLwr);
			if (peopleIndex != -1)
			{
				// We found a player in this list
				pPList->displayMessage(peopleIndex, msg, col, numBlinks);
				if (windowVisible) *windowVisible = true;
				return;
			}
		}
	}

	// People not found in team or friend list so create/search among free teller (main_chat)

	// If at least one window is visible, no need to make the window blink.
	std::vector<CChatWindow *>::iterator peopleIt;
	for(peopleIt = _ListeningWindows.begin(); peopleIt != _ListeningWindows.end(); ++peopleIt)
	{
		(*peopleIt)->displayTellMessage(msg, col,  sender);
	}
	if (windowVisible) *windowVisible = true;
}

//=============================================================================================================
void CChatInputFilter::clearMessages()
{
	std::vector<CChatWindow *>::iterator it;
	for(it = _ListeningWindows.begin(); it != _ListeningWindows.end(); ++it)
	{
		(*it)->clearMessages(FilterType, DynamicChatDbIndex);
	}
}



// ***************************************************************************
// ***************************************************************************
// CChatTargetFilter //
// ***************************************************************************
// ***************************************************************************



//=============================================================================================================
CChatTargetFilter::CChatTargetFilter() : _Chat(NULL), _TargetPartyChat(NULL)
{
	_TargetGroup= CChatGroup::say;
	_TargetDynamicChannelDbIndex= 0;
}

//=============================================================================================================
CChatTargetFilter::~CChatTargetFilter()
{
	reset();
}

//=============================================================================================================
void CChatTargetFilter::chatWindowRemoved(CChatWindow *cw)
{
	if (cw == _TargetPartyChat)
	{
		_TargetPartyChat = NULL;
		setTargetGroup(CChatGroup::say);
		return;
	}
	if (cw == _Chat)
	{
		_Chat = NULL;
	}
}

//=============================================================================================================
void CChatTargetFilter::setChat(CChatWindow *w)
{
	if (_Chat)
	{
		if (_Chat->getListener() == this)
		{
			_Chat->setListener(NULL);
		}
		_Chat->removeObserver(this);
	}
	_Chat = w;
	if (_Chat)
	{
		_Chat->setListener(this);
		_Chat->addObserver(this);
	}
}

//=============================================================================================================
void CChatTargetFilter::msgEntered(const ucstring &msg, CChatWindow *chatWindow)
{
	// Special case for yubo chat
	if(_TargetGroup==CChatGroup::yubo_chat)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		pIM->sendStringToYuboChat(msg);
		return;
	}

	// Common Target case
	if (ClientCfg.Local)
	{
		chatWindow->displayMessage(msg, CRGBA::White, _TargetGroup, _TargetDynamicChannelDbIndex);
		return;
	}

	// forward to the target
	if (_TargetPartyChat && _TargetPartyChat->getListener())
	{
		_TargetPartyChat->getListener()->msgEntered(msg, chatWindow);
	}
	else if (!_TargetPlayer.empty())
	{
		// the target must be a player, make a tell on him
		ChatMngr.tell(_TargetPlayer.toString(), msg);
		// direct output in the chat
		chatWindow->displayLocalPlayerTell(_TargetPlayer.toString(), msg);
	}
	else
	{
		 // chat to a chat group (say, shout, universe) also for team (with special case in setChatMode)
		 // this mode is cached so this should be ok
		ChatMngr.setChatMode(_TargetGroup, ChatMngr.getDynamicChannelIdFromDbIndex(_TargetDynamicChannelDbIndex));
		// send the string
		ChatMngr.chat(msg, _TargetGroup == CChatGroup::team);
	}
}

//=============================================================================================================
void CChatTargetFilter::setTargetPartyChat(CChatWindow *w)
{
	if (!w) return;
	_TargetPlayer.resize(0);
	_TargetPartyChat = w;
	w->addObserver(this);
	// set the prompt
	//if (_Chat)
	//	_Chat->setPrompt(w->getTitle() + (ucchar) '>');
}

//=============================================================================================================
void CChatTargetFilter::setTargetPlayer(const ucstring &targetPlayer)
{
	_TargetPlayer = targetPlayer;
	if (_TargetPartyChat)
	{
		_TargetPartyChat = NULL;
		_TargetPartyChat->removeObserver(this);
	}
	// set the prompt
	if (_Chat)
	{
		_Chat->setPrompt(targetPlayer + (ucchar) '>');
	}
}

//=============================================================================================================
void CChatTargetFilter::setTargetGroup(CChatGroup::TGroupType groupType, uint32 dynamicChannelDbIndex, bool allowUniverseWarning)
{
	_TargetPlayer.resize(0);
	if (_TargetPartyChat)
	{
		_TargetPartyChat->removeObserver(this);
		_TargetPartyChat = NULL;
	}
	_TargetGroup = groupType;
	_TargetDynamicChannelDbIndex = dynamicChannelDbIndex;

	if (_Chat)
	{
		// set the prompt
		const ucstring prompt("");
		_Chat->setPrompt(prompt + (ucchar) '>');

		// set the color
		string entry="UI:SAVE:CHAT:COLORS:";
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		const bool teamActive = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GROUP:0:NAME")->getValueBool();
		const bool guildActive = NLGUI::CDBManager::getInstance()->getDbProp("SERVER:GUILD:NAME")->getValueBool();
		switch(groupType)
		{
			case CChatGroup::dyn_chat: entry+="DYN:" + NLMISC::toString(dynamicChannelDbIndex); break;
			case CChatGroup::say:	entry+="SAY";	break;
			case CChatGroup::shout:	entry+="SHOUT";	break;
			case CChatGroup::team:	if(!teamActive) return; entry+="GROUP";	break;
			case CChatGroup::guild:	entry+="CLADE";	break;
			case CChatGroup::civilization:	entry+="CIVILIZATION";	break;
			case CChatGroup::territory:	entry+="TERRITORY";	break;
			case CChatGroup::universe:
				{
					entry+="UNIVERSE_NEW";
					if(allowUniverseWarning)
						NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:UNIVERSE_CHANEL_WARNING_WANTED")->setValue64(1);
				}
				break;
			case CChatGroup::region:	entry+="REGION";	break;
			case CChatGroup::tell:	entry+="TELL";	break;
			case CChatGroup::system: return;	// return with no warning
			default:	nlwarning("unknown CChatGroup type");	return;
		}

		// read DB
		CInterfaceProperty prop;
		prop.readRGBA(entry.c_str()," ");
		_Chat->setPromptColor(prop.getRGBA());
	}
}

//=============================================================================================================
void CChatTargetFilter::reset()
{
	// remove the observers we've registered
	if (_TargetPartyChat)
	{
		_TargetPartyChat->removeObserver(this);
		_TargetPartyChat = NULL;
	}
	if (_Chat)
	{
		_Chat->removeObserver(this);
		if (_Chat->getListener() == this)
		{
			_Chat->setListener(NULL);
		}
		_Chat = NULL;
	}
}























