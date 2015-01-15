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




// client
#include "stdpch.h"

#include "game_share/shard_names.h"

#include "../r2/editor.h"

#include "chat_window.h"
#include "chat_text_manager.h"
#include "../user_entity.h"
#include "people_interraction.h"
#include "../connection.h"
//
#include "nel/gui/group_container.h"
#include "nel/gui/group_editbox.h"
#include "nel/gui/group_tab.h"
#include "interface_manager.h"
#include "nel/gui/action_handler.h"
#include "../client_chat_manager.h"
//
#include "../session_browser_impl.h"

#include "../r2/editor.h"
#include "../r2/dmc/client_edition_module.h"

using namespace NLMISC;
using namespace std;

////////////
// STATIC //
////////////

CChatWindow *CChatWindow::_ChatWindowLaunchingCommand = NULL;


////////////
// EXTERN //
////////////

extern NLMISC::CLog	g_log;
extern CClientChatManager		ChatMngr;

/////////////////////
// CChatWindowDesc //
/////////////////////

CChatWindowDesc::CChatWindowDesc() :	InsertPosition(-1),
										ParentBlink(false),
										Savable(false),
										Localize(false),
										Listener(NULL)
{
}

/////////////////
// CChatWindow //
/////////////////

//=================================================================================
CChatWindow::CChatWindow() : _Listener(NULL), _Chat(NULL), _EB(NULL), _ParentBlink(false)
{
}

//=================================================================================
bool CChatWindow::create(const CChatWindowDesc &desc, const std::string &chatId)
{
	deleteContainer();
	CInterfaceManager *im = CInterfaceManager::getInstance();

	// get the father container
	CGroupContainer *fatherContainer = NULL;
	if (!desc.FatherContainer.empty())
	{
		if (desc.FatherContainer != "ui:interface" )
		{
			fatherContainer = dynamic_cast<CGroupContainer *>(CWidgetManager::getInstance()->getElementFromId(desc.FatherContainer));
			if (!fatherContainer)
			{
				nlwarning("<CChatWindow::create> Can't get father group, or bad type");
				return false;
			}
		}
	}

	// get the good template
	std::string chatTemplate;
	if (desc.ChatTemplate.empty())
	{
		// no chat template provided : use default
		chatTemplate = "chat_id";
	}
	else
	{
		chatTemplate = desc.ChatTemplate;
	}

	// build params
	CChatWindowDesc::TTemplateParams	params;
	params.push_back(make_pair(string("id"), chatId));
	params.insert(params.end(), desc.ChatTemplateParams.begin(), desc.ChatTemplateParams.end());

	// create a chat container from the template
	CInterfaceGroup *chatGroup = CWidgetManager::getInstance()->getParser()->createGroupInstance(chatTemplate, "ui:interface", params);
	if (chatGroup)
	{
		_Chat = dynamic_cast<CGroupContainer *>(chatGroup);
		if (!_Chat)
		{
			nlwarning("<CChatWindow::create> Bad type for chat group");
			delete chatGroup;
			return false;
		}
		_Chat->setLocalize (desc.Localize);
		if (desc.Localize)
			_Chat->setTitle(desc.Title.toString());
		else
			_Chat->setUCTitle(desc.Title);
		_Chat->setSavable(desc.Savable);

		// groups like system info don't have edit box.
		_EB = dynamic_cast<CGroupEditBox *>(_Chat->getGroup("eb"));
		if (_EB)
		{
			_EB->setAHOnEnter("chat_box_entry");
		}

		CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));

		if (fatherContainer)
		{
			fatherContainer->attachContainer(_Chat, desc.InsertPosition);
		}

		// If root container
		if (desc.FatherContainer == "ui:interface")
		{
			CWidgetManager::getInstance()->addWindowToMasterGroup("ui:interface", _Chat);
			_Chat->setParent(pRoot);
			_Chat->setMovable(true);
			_Chat->setActive(false);
			_Chat->setOpen(true);
		}

		_ParentBlink  = desc.ParentBlink;
		_Listener     = desc.Listener;

		pRoot->addGroup (_Chat);
		return true;
	}
	else
	{
		return false;
	}
}


//=================================================================================
bool CChatWindow::isVisible() const
{
	if (!_Chat) return false;
	if (_Chat->isOpen())
	{
		CInterfaceGroup *ig = _Chat;
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

//=================================================================================
void CChatWindow::displayMessage(const ucstring &msg, NLMISC::CRGBA col, CChatGroup::TGroupType gt, uint32 dynamicChatDbIndex, uint numBlinks /* = 0*/, bool *windowVisible /*= NULL*/)
{
	if (!_Chat)
	{
		nlwarning("<CChatWindow::displayMessage> There's no global chat");
		return;
	}
	CGroupList *gl;

	CChatTextManager &ctm = getChatTextMngr();

	gl = dynamic_cast<CGroupList *>(_Chat->getGroup("cb:text_list"));
	if (gl)	gl->addChild(ctm.createMsgText(msg, col));

	// if the group is closed, make it blink
	if (!_Chat->isOpen())
	{
		if (numBlinks) _Chat->enableBlink(numBlinks);
	}
	if (_ParentBlink)
	{
		CGroupContainer *father = dynamic_cast<CGroupContainer *>(_Chat->getParent());
		if (father && !father->isOpen())
		{
			father->enableBlink(numBlinks);
		}
	}
	if (windowVisible != NULL)
	{
		*windowVisible = isVisible();
	}
	/*for(std::vector<IObserver *>::iterator it = _Observers.begin(); it != _Observers.end(); ++it)
	{
		(*it)->displayMessage(this, msg, col, numBlinks);
	}*/
}

//=================================================================================
void CChatWindow::setMenu(const std::string &menuName)
{
	if (!_Chat) return;
	if (_Chat->getHeaderOpened())
	{
		_Chat->getHeaderOpened()->setRightClickHandler("active_menu");
		_Chat->getHeaderOpened()->setRightClickHandlerParams("menu=" + menuName);
	}
	if (_Chat->getHeaderClosed())
	{
		_Chat->getHeaderClosed()->setRightClickHandler("active_menu");
		_Chat->getHeaderClosed()->setRightClickHandlerParams("menu=" + menuName);
	}
}

//=================================================================================
void CChatWindow::setPrompt(const ucstring &prompt)
{
	if (!_Chat) return;
	CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(_Chat->getGroup("eb"));
	if (!eb) return;
	eb->setPrompt(prompt);
}

void CChatWindow::setPromptColor(NLMISC::CRGBA col)
{
	if (!_Chat)
		return;

	CGroupEditBox *eb = dynamic_cast<CGroupEditBox *>(_Chat->getGroup("eb"));
	if (!eb)
		return;

	eb->setColor(col);
}

//=================================================================================
void CChatWindow::deleteContainer()
{
	if (!_Chat) return;
	CGroupContainer *proprietaryContainer = _Chat->getProprietaryContainer();
	if (proprietaryContainer)
	{
		if (_Chat->isPopuped())
		{
			_Chat->popin(-1, false); // popin & detach
		}
		else
		{
			proprietaryContainer->detachContainer(_Chat); // just detach
		}
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
		pRoot->delGroup (_Chat);
	}
	else
	{
		CWidgetManager::getInstance()->unMakeWindow(_Chat);
		if (_Chat->getParent())
		{
			_Chat->getParent()->delGroup(_Chat);
		}
	}
	// Removes from parent group

	_Chat = NULL;
}

//=================================================================================
bool CChatWindow::rename(const ucstring &newName, bool newNameLocalize)
{
	return getChatWndMgr().rename(getTitle(), newName, newNameLocalize);
}

//=================================================================================
void CChatWindow::setKeyboardFocus()
{
	if (!_EB || !_Chat) return;
	CWidgetManager::getInstance()->setCaptureKeyboard(_EB);
	if (!_Chat->isOpenable() || _Chat->isOpenWhenPopup())
	{
		if (_Chat->isPopable() && !_Chat->isPopuped())
		{
			_Chat->popup();
			if (_Chat->getPopupW() != -1) // restore previous popup position if there's one
			{
				_Chat->setX(_Chat->getPopupX());
				_Chat->setY(_Chat->getPopupY());
				_Chat->setW(_Chat->getPopupW());
				// must resize the children to get correct height
				//_Chat->setChildrenH(_Chat->getPopupChildrenH());
			}
		}
	}
}

//=================================================================================
void CChatWindow::enableBlink(uint numBlinks)
{
	if (!_Chat) return;
	_Chat->enableBlink(numBlinks);
}


//=================================================================================
void CChatWindow::setCommand(const std::string &command, bool execute)
{
	if (!_EB) return;
	_EB->setCommand(ucstring(command), execute);
}

void CChatWindow::setCommand(const ucstring &command,bool execute)
{
	if (!_EB) return;
	_EB->setCommand(command, execute);
}


//=================================================================================
void CChatWindow::setEntry(const ucstring &entry)
{
	if (!_EB) return;
	_EB->setInputString(entry);
}

//=================================================================================
ucstring CChatWindow::getTitle() const
{
	if (!_Chat)
	{
		return ucstring("");
	}
	else
	{
		return _Chat->getUCTitle();
	}
}

//=================================================================================
void CChatWindow::addObserver(IObserver *obs)
{
	if (!obs)
	{
		nlwarning("NULL observer is invalid");
		return;
	}
	if (isObserver(obs))
	{
		nlwarning("Observer added twice");
		return;
	}
	_Observers.push_back(obs);
}

//=================================================================================
void CChatWindow::removeObserver(IObserver *obs)
{
	std::vector<IObserver *>::iterator it = std::find(_Observers.begin(), _Observers.end(), obs);
	if (it == _Observers.end())
	{
		nlwarning("Observer doesn't belong to this chatbox");
		return;
	}
	_Observers.erase(it);
}

//=================================================================================
bool CChatWindow::isObserver(const IObserver *obs) const
{
	std::vector<IObserver *>::const_iterator it = std::find(_Observers.begin(), _Observers.end(), obs);
	return it != _Observers.end();
}

//=================================================================================
CChatWindow::~CChatWindow()
{
	for(std::vector<IObserver *>::iterator it = _Observers.begin(); it != _Observers.end(); ++it)
	{
		(*it)->chatWindowRemoved(this);
	}
	if (this == _ChatWindowLaunchingCommand)
	{
		_ChatWindowLaunchingCommand = NULL;
	}
}

//=================================================================================
void CChatWindow::setAHOnActive(const std::string &n)
{
	if (_Chat) _Chat->setOnActiveHandler(n);
}

//=================================================================================
void CChatWindow::setAHOnActiveParams(const std::string &n)
{
	if (_Chat) _Chat->setOnActiveParams(n);
}

//=================================================================================
void CChatWindow::setAHOnDeactive(const std::string &n)
{
	if (_Chat) _Chat->setOnDeactiveHandler(n);
}

//=================================================================================
void CChatWindow::setAHOnDeactiveParams(const std::string &n)
{
	if (_Chat) _Chat->setOnDeactiveParams(n);
}

//=================================================================================
void CChatWindow::setAHOnCloseButton(const std::string &n)
{
	if (_Chat) _Chat->setOnCloseButtonHandler(n);
}

//=================================================================================
void CChatWindow::setAHOnCloseButtonParams(const std::string &n)
{
	if (_Chat) _Chat->setOnCloseButtonParams(n);
}

//=================================================================================
void CChatWindow::setHeaderColor(const std::string &n)
{
	if (_Chat) _Chat->setHeaderColor(n);
}

//=================================================================================
void CChatWindow::displayLocalPlayerTell(const ucstring &receiver, const ucstring &msg, uint numBlinks /*= 0*/)
{
	ucstring finalMsg;
	CInterfaceProperty prop;
	prop.readRGBA("UI:SAVE:CHAT:COLORS:SPEAKER"," ");
	encodeColorTag(prop.getRGBA(), finalMsg, false);

	ucstring csr(CHARACTER_TITLE::isCsrTitle(UserEntity->getTitleRaw()) ? "(CSR) " : "");
	finalMsg += csr + CI18N::get("youTell") + ": ";
	prop.readRGBA("UI:SAVE:CHAT:COLORS:TELL"," ");
	encodeColorTag(prop.getRGBA(), finalMsg, true);
	finalMsg += msg;

	ucstring s = CI18N::get("youTellPlayer");
	strFindReplace(s, "%name", receiver);
	strFindReplace(finalMsg, CI18N::get("youTell"), s);
	displayMessage(finalMsg, prop.getRGBA(), CChatGroup::tell, 0, numBlinks);
	CInterfaceManager::getInstance()->log(finalMsg, CChatGroup::groupTypeToString(CChatGroup::tell));
}

void CChatWindow::encodeColorTag(const NLMISC::CRGBA &color, ucstring &text, bool append)
{
	// WARNING : The lookup table MUST contains 17 element (with the last doubled)
	// because we add 7 to the 8 bit color before shifting to right in order to match color
	// more accurately.
	// Have 17 entry remove the need for a %16 for each color component.
	// By the way, this comment is more longer to type than to add the %16...
	//
	static ucchar ConvTable[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'F'};
	ucstring str;
	if (append)
	{
		str.reserve(7 + str.size());
		str = text;
	}
	else
		str.reserve(7);

	str += "@{";
	str += ConvTable[(uint(color.R)+7)>>4];
	str += ConvTable[(uint(color.G)+7)>>4];
	str += ConvTable[(uint(color.B)+7)>>4];
	str += ConvTable[(uint(color.A)+7)>>4];
	str += '}';

	text.swap(str);
}


//=================================================================================
void CChatWindow::clearMessages(CChatGroup::TGroupType /* gt */, uint32 /* dynamicChatDbIndex */)
{
	// if not correctly init, abort
	if(!_Chat)
		return;

	// get the group list
	CGroupList *gl = dynamic_cast<CGroupList *>(_Chat->getGroup("cb:text_list"));
	if (gl)	gl->deleteAllChildren();
}


//////////////////////
// CChatGroupWindow //
//////////////////////

void CChatGroupWindow::displayMessage(const ucstring &msg, NLMISC::CRGBA col, CChatGroup::TGroupType gt, uint32 dynamicChatDbIndex, uint numBlinks, bool *windowVisible)
{
	if (!_Chat)
	{
		nlwarning("<CChatGroupWindow::displayMessage> There's no global chat");
		return;
	}

	CChatTextManager &ctm = getChatTextMngr();

	if (_Chat->getHeaderOpened()==NULL)
		return;


	// *** Display the message in the correct tab window
	// get the gl and tab according to filter
	CGroupList *gl;
	CCtrlTabButton *tab;
	getAssociatedSubWindow(gt, dynamicChatDbIndex, gl, tab);

	// on a new message, change the Tab color
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	CRGBA	newMsgColor= CRGBA::stringToRGBA(CWidgetManager::getInstance()->getParser()->getDefine("chat_group_tab_color_newmsg").c_str());

	ucstring newmsg = msg;
	ucstring prefix;

	if (gl != NULL)
	{
		gl->addChild(ctm.createMsgText(newmsg, col));
		if (!gl->getParent()->getActive())
			if (tab != NULL)
				tab->setTextColorNormal(newMsgColor);
	}

	// *** Display the message in the UserChat (special case)
	{
		tab = dynamic_cast<CCtrlTabButton*>(_Chat->getCtrl("header_opened:channel_select:tab5"));
		gl = NULL;
		CGroupList *gl2 = dynamic_cast<CGroupList *>(_Chat->getGroup("content:cb:user:text_list"));

		CChatWindow *cw = PeopleInterraction.TheUserChat.Window;
		CChatStdInput &ci = PeopleInterraction.ChatInput;

		switch(gt)
		{
			default:
			case CChatGroup::arround:
			case CChatGroup::say:		if (ci.AroundMe.isListeningWindow(cw))		gl = gl2;	break;
			case CChatGroup::region:	if (ci.Region.isListeningWindow(cw))		gl = gl2;	break;
			case CChatGroup::team:		if (ci.Team.isListeningWindow(cw))			gl = gl2;	break;
			case CChatGroup::guild:		if (ci.Guild.isListeningWindow(cw))			gl = gl2;	break;
			case CChatGroup::system:	if (ci.SystemInfo.isListeningWindow(cw))	gl = gl2;	break;
			case CChatGroup::universe:	if (ci.Universe.isListeningWindow(cw))		gl = gl2;	break;
			case CChatGroup::dyn_chat:	
				if (ci.DynamicChat[dynamicChatDbIndex].isListeningWindow(cw))
				{
					gl = gl2;

					// Add dyn chan number before string
					ucstring prefix("[" + NLMISC::toString(dynamicChatDbIndex) + "]");
					// Find position to put the new string
					// After timestamp?
					size_t pos = newmsg.find(ucstring("]"));
					size_t colonpos = newmsg.find(ucstring(": @{"));
					// If no ] found or if found but after the colon (so part of the user chat)
					if (pos == ucstring::npos || (colonpos < pos))
					{
						// No timestamp, so put it right after the color and add a space
						pos = newmsg.find(ucstring("}"));
						prefix += " ";
					}
					newmsg = newmsg.substr(0, pos + 1) + prefix + newmsg.substr(pos + 1);

					// Add dynchannel number and optionally name before text if user channel
					CCDBNodeLeaf* node = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:CHAT:SHOW_DYN_CHANNEL_NAME_IN_CHAT_CB", false);
					if (node && node->getValueBool())
					{
						uint32 textId = ChatMngr.getDynamicChannelNameFromDbIndex(dynamicChatDbIndex);
						ucstring title;
						STRING_MANAGER::CStringManagerClient::instance()->getDynString(textId, title);
						prefix = title.empty() ? ucstring("") : ucstring(" ") + title;
						pos = newmsg.find(ucstring("] "));
						newmsg = newmsg.substr(0, pos) + prefix + newmsg.substr(pos);
					}
				}
				break;

				// NB: the yubo chat cannot be in a user chat
			case CChatGroup::yubo_chat:	gl = NULL;	break;
		}

		if (gl != NULL)
		{
			gl->addChild(ctm.createMsgText(newmsg, col));
			if (!gl->getParent()->getActive())
				if (tab != NULL)
					tab->setTextColorNormal(newMsgColor);
		}
	}


	// *** Blink and visibility event
	// if the group is closed, make it blink
	if (!_Chat->isOpen())
	{
		if (numBlinks) _Chat->enableBlink(numBlinks);
	}
	if (_ParentBlink)
	{
		CGroupContainer *father = dynamic_cast<CGroupContainer *>(_Chat->getParent());
		if (father && !father->isOpen())
		{
			father->enableBlink(numBlinks);
		}
	}
	if (windowVisible != NULL)
	{
		*windowVisible = isVisible();
	}
}

//=================================================================================
void CChatGroupWindow::displayTellMessage(const ucstring &msg, NLMISC::CRGBA col, const ucstring &sender)
{
	// If we are here with a tell message this is because the teller doesn't belong to any people list
	CGroupContainer *gcChat = createFreeTeller(sender);
	if (gcChat == NULL)
	{
		nlwarning("<CChatGroupWindow::displayTellMessage> cannot open chat.");
		return;
	}

	gcChat->requireAttention();

	CWidgetManager::getInstance()->setTopWindow(gcChat);

	// add the text to this window
	CGroupList *gl = dynamic_cast<CGroupList *>(gcChat->getGroup("text_list"));
	if (gl == NULL)
	{
		nlwarning("<CChatGroupWindow::displayTellMessage> can't get text_list.");
		return;
	}

	gl->addChild(getChatTextMngr().createMsgText(msg, col));
}

//=================================================================================
sint32 CChatGroupWindow::getTabIndex()
{
	CGroupTab *pTab = dynamic_cast<CGroupTab*>(_Chat->getGroup("header_opened:channel_select"));
	if (pTab != NULL)
		return pTab->getSelection();
	else
		return -1;
}

//=================================================================================
void CChatGroupWindow::setTabIndex(sint32 n)
{
	CGroupTab *pTab = dynamic_cast<CGroupTab*>(_Chat->getGroup("header_opened:channel_select"));
	if (pTab != NULL)
	{
		pTab->select(n);
		// if the current button is hidden, select default not hid
		pTab->selectDefaultIfCurrentHid();
	}
}

//=================================================================================
const string CChatGroupWindow::getValidUiStringId(const string &stringId)
{
	string validStringId;

	for (uint32 i=0; i < stringId.length(); i++)
	{
		if ((stringId[i] < 'a') || (stringId[i] > 'z'))
			validStringId += '_';
		else
			validStringId += stringId[i];
	}
	return validStringId;
}

//=================================================================================
CGroupContainer *CChatGroupWindow::createFreeTeller(const ucstring &winNameIn, const string &winColor)
{
	// must parse the entity name, and eventually make it Full with shard name (eg: 'ani.yoyo' becomes 'yoyo(Aniro)')
	string winNameFull= CShardNames::getInstance().makeFullNameFromRelative(PlayerSelectedMainland, winNameIn.toString());

	// remove shard name if necessary
	ucstring winName= CEntityCL::removeShardFromName(winNameFull);

	// get the color
	string sWinColor = winColor;
	if (sWinColor.empty())
		sWinColor = "UI:SAVE:WIN:COLORS:COM";

	// Look if the free teller do not already exists
	uint32 i;
	string sWinName = winName.toString();
	sWinName = toLower(sWinName);
	for (i = 0; i < _FreeTellers.size(); ++i)
	{
		CGroupContainer *pGC = _FreeTellers[i];
		if (toLower(pGC->getUCTitle().toString()) == sWinName)
			break;
	}
	// Create container if not present
	if (i == _FreeTellers.size())
	{
		// Corresponding Chat not created -> create and open
		vector<pair<string ,string> > properties;
		properties.push_back(make_pair(string("posparent"), string("parent")));
		properties.push_back(make_pair(string("id"), "free_chat_" + getValidUiStringId(sWinName)));
		properties.push_back(make_pair(string("title"), std::string("")));
		properties.push_back(make_pair(string("header_color"), sWinColor));

		std::string templateName = "contact_chat_friend";

		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CInterfaceGroup *pIG = CWidgetManager::getInstance()->getParser()->createGroupInstance(templateName, "ui:interface", properties);
		if (!pIG) return NULL;
		CGroupContainer *pGC = dynamic_cast<CGroupContainer *>(pIG);
		if (!pGC)
		{
			delete pIG;
			nlwarning("<CChatGroupWindow::createFreeTeller> group is not a container.(%s)", winName.toString().c_str());
			return NULL;
		}
		// set title from the name
		pGC->setUCTitle(winName);
		//
		pGC->setSavable(true);
		pGC->setEscapable(true);

		CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
		pRoot->addGroup (pGC);
		pGC->setParent(pRoot); // must be done before makeWindow
		CWidgetManager::getInstance()->makeWindow(pGC);
		pGC->open();
		pGC->updateCoords();
		pGC->center();
		_FreeTellers.push_back(pGC);

		CInterfaceElement *result = pRoot->findFromShortId(string("invite"));
		if(result)
		{
			CCtrlBaseButton * inviteButton = dynamic_cast<CCtrlBaseButton*>(result);
			if(inviteButton)
			{
				bool isDM = false;
				R2::CEditor::TMode mode = R2::getEditor().getMode();
				if (mode != R2::CEditor::NotInitialized )
				{
					isDM =  R2::getEditor().getDMC().getEditionModule().isSessionOwner()
						&& uint32(R2::CEditor::AnimationModeLoading) <= uint32(mode)
						&& uint32(mode) <= uint32(R2::CEditor::AnimationModeGoingToPlay);
				}
				inviteButton->setActive(isDM);
			}
		}

		// the group is only active on the current desktop
		pGC->setActive(true);
	}

	if (!winColor.empty())
		_FreeTellers[i]->setHeaderColor(winColor);

//	updateFreeTellerHeader(*_FreeTellers[i]);
	return _FreeTellers[i];
}

//=================================================================================
void CChatGroupWindow::updateAllFreeTellerHeaders()
{
	for(uint k = 0; k < _FreeTellers.size(); ++k)
	{
		if (_FreeTellers[k])
		{
			updateFreeTellerHeader(*_FreeTellers[k]);
		}
	}
}

//=================================================================================
void CChatGroupWindow::updateFreeTellerHeader(CGroupContainer &ft)
{
	ucstring name = ft.getUCTitle();
	CCtrlBaseButton *newFriendBut = dynamic_cast<CCtrlBaseButton *>(ft.getCtrl("new_friend"));
	CCtrlBaseButton *ignoreBut = dynamic_cast<CCtrlBaseButton *>(ft.getCtrl("ignore"));
	CCtrlBaseButton *inviteBut = dynamic_cast<CCtrlBaseButton *>(ft.getCtrl("invite"));
	if (newFriendBut)
	{
		newFriendBut->setFrozen(PeopleInterraction.isContactInList(name, 0));
	}
	if (ignoreBut)
	{
		ignoreBut->setFrozen(PeopleInterraction.isContactInList(name, 1));
	}

	if (inviteBut)
	{
		bool isDM = false;
		R2::CEditor::TMode mode = R2::getEditor().getMode();
		if (mode != R2::CEditor::NotInitialized )
		{
			isDM =  R2::getEditor().getDMC().getEditionModule().isSessionOwner()
				&& uint32(R2::CEditor::AnimationModeLoading) <= uint32(mode)
				&& uint32(mode) <= uint32(R2::CEditor::AnimationModeGoingToPlay);
		}

		inviteBut->setActive(isDM);
		if (isDM)
		{
			inviteBut->setFrozen(false); // TODO Boris : true if player is already invited in anim + do
										 // PeopleInterraction.updateAllFreeTellerHeaders() when list is updated
		}
	}
}

//=================================================================================
void CChatGroupWindow::setActiveFreeTeller(const ucstring &winName, bool bActive)
{
	CGroupContainer *pGC = createFreeTeller(winName);
	if (pGC != NULL)
		pGC->setActive(bActive);
}

//=================================================================================
ucstring CChatGroupWindow::getFreeTellerName(const std::string &containerID)
{
	uint32 i;
	for (i = 0; i < _FreeTellers.size(); ++i)
	{
		CGroupContainer *pGC = _FreeTellers[i];
		if (pGC->getId() == containerID)
			break;
	}
	if (i == _FreeTellers.size())
		return ucstring("");
	return _FreeTellers[i]->getUCTitle();
}

//=================================================================================
bool CChatGroupWindow::removeFreeTeller(const std::string &containerID)
{
	uint32 i;
	for (i = 0; i < _FreeTellers.size(); ++i)
	{
		CGroupContainer *pGC = _FreeTellers[i];
		if (pGC->getId() == containerID)
			break;
	}
	if (i == _FreeTellers.size())
		return false;
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	// Create the free teller in all the desktops images
	for (uint m = 0; m < MAX_NUM_MODES; ++m)
	{
		pIM->removeGroupContainerImage(_FreeTellers[i]->getId(), m);
	}
	CInterfaceGroup *pRoot = dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId("ui:interface"));
	CWidgetManager::getInstance()->unMakeWindow(_FreeTellers[i]);
	pRoot->delGroup (_FreeTellers[i]);
	_FreeTellers[i] = NULL;
	_FreeTellers.erase(_FreeTellers.begin()+i);
	return true;
}

//=================================================================================
void CChatGroupWindow::removeAllFreeTellers()
{
	while (!_FreeTellers.empty())
	{
		if (_FreeTellers[0])
		{
			removeFreeTeller(_FreeTellers[0]->getId());
		}
		else
		{
			_FreeTellers.erase(_FreeTellers.begin());
		}
	}
}

//=================================================================================
void CChatGroupWindow::saveFreeTeller(NLMISC::IStream &f)
{
	f.serialVersion(2);

	// Save the free teller only if it is present in the friend list to avoid the only-growing situation
	// because free tellers are never deleted in game if we save/load all the free tellers, we just create more
	// and more container.

	uint32 i, nNbFreeTellerSaved = 0;
	for (i = 0; i < _FreeTellers.size(); ++i)
		if (PeopleInterraction.FriendList.getIndexFromName(_FreeTellers[i]->getUCTitle()) != -1)
			nNbFreeTellerSaved++;

	f.serial(nNbFreeTellerSaved);

	for (i = 0; i < _FreeTellers.size(); ++i)
	{
		CGroupContainer *pGC = _FreeTellers[i];

		if (PeopleInterraction.FriendList.getIndexFromName(pGC->getUCTitle()) != -1)
		{
			ucstring sTitle = pGC->getUCTitle();
			f.serial(sTitle);
		}
	}
}

//=================================================================================
void CChatGroupWindow::loadFreeTeller(NLMISC::IStream &f)
{
	sint ver = f.serialVersion(2);

	if (ver == 1)
	{
		// Old serialized FreeTellIdCounter (deprecated with v2).
		uint32 tmp;
		f.serial(tmp);
	}

	uint32 i, nNbFreeTellerSaved = 0;
	f.serial(nNbFreeTellerSaved);

	for (i = 0; i < nNbFreeTellerSaved; ++i)
	{
		if (ver == 1)
		{
			// Old serialized sID (deprecated with v2).
			string sID;
			f.serial(sID);
		}
		ucstring sTitle;
		f.serial(sTitle);

		CGroupContainer *pGC = createFreeTeller(sTitle, "");

		// With version 1 all tells are active because windows information have "title based" ids and no "sID based".
		if ((ver == 1) && (pGC != NULL))
			pGC->setActive(false);
	}
}

//=================================================================================
void CChatGroupWindow::clearMessages(CChatGroup::TGroupType gt, uint32 dynamicChatDbIndex)
{
	// if not correctly init, abort
	if(!_Chat)
		return;

	// get the gl and tab according to filter
	CGroupList *gl;
	CCtrlTabButton *tab;
	getAssociatedSubWindow(gt, dynamicChatDbIndex, gl, tab);

	// delete all text lines
	if(gl!=NULL)
	{
		gl->deleteAllChildren();
	}
}

//=================================================================================
void CChatGroupWindow::getAssociatedSubWindow(CChatGroup::TGroupType gt, uint32 dynamicChatDbIndex, CGroupList *&gl, CCtrlTabButton *&tab)
{
	nlassert(_Chat);
	gl= NULL;
	tab= NULL;

	switch(gt)
	{
	default:
	case CChatGroup::say:
		gl = dynamic_cast<CGroupList *>(_Chat->getGroup("content:cb:around:text_list"));
		tab = dynamic_cast<CCtrlTabButton*>(_Chat->getCtrl("header_opened:channel_select:tab0"));
		break;
	case CChatGroup::team:
		gl = dynamic_cast<CGroupList *>(_Chat->getGroup("content:cb:team:text_list"));
		tab = dynamic_cast<CCtrlTabButton*>(_Chat->getCtrl("header_opened:channel_select:tab2"));
		break;
	case CChatGroup::guild:
		gl = dynamic_cast<CGroupList *>(_Chat->getGroup("content:cb:guild:text_list"));
		tab = dynamic_cast<CCtrlTabButton*>(_Chat->getCtrl("header_opened:channel_select:tab3"));
		break;
	case CChatGroup::system:
		gl = dynamic_cast<CGroupList *>(_Chat->getGroup("content:cb:sysinfo:text_list"));
		tab = dynamic_cast<CCtrlTabButton*>(_Chat->getCtrl("header_opened:channel_select:tab4"));
		break;
	case CChatGroup::region:
		gl = dynamic_cast<CGroupList *>(_Chat->getGroup("content:cb:region:text_list"));
		tab = dynamic_cast<CCtrlTabButton*>(_Chat->getCtrl("header_opened:channel_select:tab1"));
		break;
	case CChatGroup::yubo_chat:
		gl = dynamic_cast<CGroupList *>(_Chat->getGroup("content:cb:yubo_chat:text_list"));
		tab = dynamic_cast<CCtrlTabButton*>(_Chat->getCtrl("header_opened:channel_select:tab6"));
		break;
	case CChatGroup::dyn_chat:
		{
			// use dynamicChatDbIndex to get the wanted tab button/group
			gl = dynamic_cast<CGroupList *>(_Chat->getGroup(toString("content:cb:dyn_chat%d:text_list", dynamicChatDbIndex)));
			tab = dynamic_cast<CCtrlTabButton*>(_Chat->getCtrl(toString("header_opened:channel_select:tab_array0_%d",dynamicChatDbIndex)));
		}
		break;
	case CChatGroup::universe:
		gl = dynamic_cast<CGroupList *>(_Chat->getGroup("content:cb:universe:text_list"));
		tab = dynamic_cast<CCtrlTabButton*>(_Chat->getCtrl("header_opened:channel_select:tab7"));
		break;
	}
}


////////////////////////
// CChatWindowManager //
////////////////////////

//=================================================================================
CChatWindowManager::CChatWindowManager() : _WindowID(0)
{
}

//=================================================================================
CChatWindowManager::~CChatWindowManager()
{
	for(TChatWindowMap::iterator it = _ChatWindowMap.begin(); it != _ChatWindowMap.end(); ++it)
	{
		delete it->second;
	}
}

//=================================================================================
CChatWindow *CChatWindowManager::createChatWindow(const CChatWindowDesc &desc)
{
	if (getChatWindow(desc.Title))
	{
		return NULL; // duplicate name encountered
	}
	CChatWindow *w;
	w = new CChatWindow;
	string zeId;
	if (desc.Id.empty())
		zeId = NLMISC::toString("chat_window_id_%d", (int) _WindowID);
	else
		zeId = desc.Id;

	if (w->create(desc, zeId))
	{
		if (desc.Id.empty())
			_WindowID++;

		if (desc.Localize)
			_ChatWindowMap[CI18N::get(desc.Title.toString())] = w;
		else
			_ChatWindowMap[desc.Title] = w;

		w->setAHOnActive(desc.AHOnActive);
		w->setAHOnActiveParams(desc.AHOnActiveParams);
		w->setAHOnDeactive(desc.AHOnDeactive);
		w->setAHOnDeactiveParams(desc.AHOnDeactiveParams);
		w->setAHOnCloseButton(desc.AHOnCloseButton);
		w->setAHOnCloseButtonParams(desc.AHOnCloseButtonParams);
		if (!desc.HeaderColor.empty())
			w->setHeaderColor(desc.HeaderColor);

		return w;
	}
	else
	{
		return NULL;
	}
}

//=================================================================================
CChatWindow *CChatWindowManager::createChatGroupWindow(const CChatWindowDesc &desc)
{
	if (getChatWindow(desc.Title))
	{
		return NULL; // duplicate name encountered
	}
	CChatGroupWindow *w;
	w = new CChatGroupWindow;
	string zeId;
	if (desc.Id.empty())
		zeId = NLMISC::toString("chat_window_id_%d", (int) _WindowID);
	else
		zeId = desc.Id;

	if (w->create(desc, zeId))
	{
		if (desc.Id.empty())
			_WindowID++;

		if (desc.Localize)
			_ChatWindowMap[CI18N::get(desc.Title.toString())] = w;
		else
			_ChatWindowMap[desc.Title] = w;

		w->setAHOnActive(desc.AHOnActive);
		w->setAHOnActiveParams(desc.AHOnActiveParams);
		w->setAHOnDeactive(desc.AHOnDeactive);
		w->setAHOnDeactiveParams(desc.AHOnDeactiveParams);
		w->setAHOnCloseButton(desc.AHOnCloseButton);
		w->setAHOnCloseButtonParams(desc.AHOnCloseButtonParams);
		if (!desc.HeaderColor.empty())
			w->setHeaderColor(desc.HeaderColor);

		return w;
	}
	else
	{
		return NULL;
	}
}

//=================================================================================
CChatWindow *CChatWindowManager::getChatWindow(const ucstring &title)
{
	TChatWindowMap::iterator it = _ChatWindowMap.find(title);
	if (it == _ChatWindowMap.end())
		return NULL;
	else
	{
		nlassert(it->second != NULL);
		return it->second;
	}
}

//=================================================================================
void CChatWindowManager::removeChatWindow(const ucstring &title)
{
	TChatWindowMap::iterator it = _ChatWindowMap.find(title);
	if (it == _ChatWindowMap.end())
	{
		nlwarning("unknwown window %s", title.toString().c_str());
		return;
	}
	it->second->deleteContainer();
	delete it->second;
	_ChatWindowMap.erase(it);
}

//=================================================================================
CChatWindowManager &CChatWindowManager::getInstance()
{
	static CChatWindowManager instance;
	return instance;
}

//=================================================================================
CChatWindow *CChatWindowManager::getChatWindowFromCaller(CCtrlBase *caller)
{
	// retrieve pointer on the CChatWindow instance associated with the ui
	// find first enclosing group container
	CGroupContainer *father = NULL;
	while (caller)
	{
		father = dynamic_cast<CGroupContainer *>(caller);
		if (father) break;
		caller = caller->getParent();
	}
	if (!father) return NULL;

	return  getChatWindow(father->getUCTitle());
}

//=================================================================================
bool CChatWindowManager::rename(const ucstring &oldName, const ucstring &newName, bool newNameLocalize)
{
	// if (oldName == newName) return true;
	CChatWindow *newWin = getChatWindow(newName);
	if (newWin != NULL) return false; // target window exists
	TChatWindowMap::iterator it = _ChatWindowMap.find(oldName);
	if (it == _ChatWindowMap.end()) return false;
	if (newNameLocalize)
	{
		_ChatWindowMap[CI18N::get(newName.toString())] = it->second;
		it->second->getContainer()->setLocalize(true);
		it->second->getContainer()->setTitle(newName.toString());
	}
	else
	{
		_ChatWindowMap[newName] = it->second;
		it->second->getContainer()->setLocalize(false);
		it->second->getContainer()->setUCTitle(newName);
	}
	_ChatWindowMap.erase(it);
	return true;
}

//=================================================================================
CGroupEditBox *CChatWindow::getEditBox() const
{
	if (!_Chat) return NULL;
	return dynamic_cast<CGroupEditBox *>(_Chat->getGroup("eb"));
}

//=================================================================================
void CChatWindowManager::removeChatWindow(CChatWindow *cw)
{
	if (!cw) return;
	removeChatWindow(cw->getTitle());
}

//=================================================================================
CChatWindow *CChatWindowManager::getChatWindowByIndex(uint index)
{
	TChatWindowMap::iterator it = _ChatWindowMap.begin();
	while (index--) { ++it; }
	return it->second;
}

/////////////////////
// ACTION HANDLERS //
/////////////////////


// ***************************************************************************************
class CHandlerChatBoxEntry : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		CGroupEditBox *pEB = dynamic_cast<CGroupEditBox*>(pCaller);
		if (pEB == NULL) return;
		ucstring text = pEB->getInputString();
		// If the line is empty, do nothing
		if(text.size() == 0)
			return;


		CChatWindow *chat = getChatWndMgr().getChatWindowFromCaller(pCaller);
		if (!chat)
		{
			nlwarning("No chat box associated with %s", pEB->getId().c_str());
			return;
		}

		// Parse any tokens in the text
		if ( ! CInterfaceManager::parseTokens(text))
		{
			pEB->setInputString (string(""));
			return;
		}

		// if, it s a command, execute it and don't send the command to the server
		if(text[0] == '/')
		{
			CChatWindow::_ChatWindowLaunchingCommand = chat;
			string str = text.toUtf8();
			string cmdWithArgs = str.substr(1);

			// Get the command name from the string, can contain spaces
			string cmd = cmdWithArgs.substr(0, cmdWithArgs.find(' '));
			if (cmdWithArgs.find('"') == 0)
			{
				string::size_type pos = cmdWithArgs.find('"', 1);
				if (string::npos != pos)
				{
					cmd = cmdWithArgs.substr(1, pos - 1);
				}
			}

			if ( NLMISC::ICommand::exists( cmd ) )
			{
				NLMISC::ICommand::execute( cmdWithArgs, g_log );
			}
			else
			{
				CInterfaceManager *im = CInterfaceManager::getInstance();
				im->displaySystemInfo (ucstring(cmd+": ")+CI18N::get ("uiCommandNotExists"));
			}
		}
		else
		{
			if (chat->getListener())
			{
				chat->getListener()->msgEntered(text, chat);
			}
		}
		// Clear input string
		pEB->setInputString (ucstring(""));
		CGroupContainer *gc = static_cast< CGroupContainer* >( pEB->getEnclosingContainer() );

		if (gc)
		{
			// Restore position of enclosing container if it hasn't been moved/scaled/poped by the user
			if (!gc->getTouchFlag(true))
			{
				gc->restorePosition();
			}
		}
	}
};
REGISTER_ACTION_HANDLER(CHandlerChatBoxEntry, "chat_box_entry");



static ucstring getFreeTellerName(CInterfaceElement *pCaller)
{
	if (!pCaller) return ucstring();
	CChatGroupWindow *cgw = PeopleInterraction.getChatGroupWindow();
	if (!cgw) return ucstring();
	CInterfaceGroup *freeTeller = pCaller->getParentContainer();
	if (!freeTeller) return ucstring();
	return cgw->getFreeTellerName( freeTeller->getId() );
}

// ***************************************************************************************
class CHandlerAddTellerToFriendList : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		ucstring playerName = ::getFreeTellerName(pCaller);
		if (!playerName.empty())
		{
			sint playerIndex = PeopleInterraction.IgnoreList.getIndexFromName(playerName);
			// if already in friend list, ask to move rather than add
			if (playerIndex != -1)
			{
				PeopleInterraction.askMoveContact(playerIndex, &PeopleInterraction.IgnoreList, &PeopleInterraction.FriendList);
			}
			else
			{
				PeopleInterraction.askAddContact(playerName, &PeopleInterraction.FriendList);
			}
		}
	}
};
REGISTER_ACTION_HANDLER(CHandlerAddTellerToFriendList, "add_teller_to_friend_list");


// ***************************************************************************************
class CHandlerAddTellerToIgnoreList : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CInterfaceManager *im = CInterfaceManager::getInstance();
		std::string callerId = getParam(sParams, "id");
		CInterfaceElement *prevCaller = CWidgetManager::getInstance()->getElementFromId(callerId);
		ucstring playerName = ::getFreeTellerName(prevCaller);
		if (!playerName.empty())
		{
			// if already in friend list, ask to move rather than add
			sint playerIndex = PeopleInterraction.FriendList.getIndexFromName(playerName);
			if (playerIndex != -1)
			{
				PeopleInterraction.askMoveContact(playerIndex, &PeopleInterraction.FriendList, &PeopleInterraction.IgnoreList);
			}
			else
			{
				PeopleInterraction.askAddContact(playerName, &PeopleInterraction.IgnoreList);
			}
			if (pCaller)
			{
				CInterfaceGroup *win = prevCaller->getParentContainer();
				if (win)
				{
					static_cast< CGroupContainer* >( win )->setActive(false);
				}
			}
		}
	}
};
REGISTER_ACTION_HANDLER(CHandlerAddTellerToIgnoreList, "add_teller_to_ignore_list");

// ***************************************************************************************
class CHandlerInviteToRingSession : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		ucstring playerName = ::getFreeTellerName(pCaller);
		if (!playerName.empty())
		{
			// ask the SBS to invite the character in the session
			CSessionBrowserImpl::getInstance().inviteCharacterByName(CSessionBrowserImpl::getInstance().getCharId(), playerName.toUtf8());
			// additionaly, send a tell to signal the player he has been invited to a ring session
			ChatMngr.tell(playerName.toUtf8(), CI18N::get("uiRingInviteNotification"));
			//
			CInterfaceManager *im = CInterfaceManager::getInstance();
			im->displaySystemInfo(ucstring("@{6F6F}") +  playerName +ucstring(" @{FFFF}") + CI18N::get("uiRingInvitationSent"), "BC");
			// force a refresh of the ui
			CLuaManager::getInstance().executeLuaScript("CharTracking:forceRefresh()");
		}
	}
};
REGISTER_ACTION_HANDLER(CHandlerInviteToRingSession, "invite_to_ring_session");

