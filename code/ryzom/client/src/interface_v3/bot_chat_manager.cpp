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
#include "bot_chat_manager.h"
#include "bot_chat_page.h"
#include "../net_manager.h"
#include "nel/gui/action_handler.h"
#include "../user_entity.h"
#include "interface_manager.h"
#include "nel/gui/view_text_id.h"
#include "nel/gui/interface_group.h"
#include "game_share/prerequisit_infos.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

CBotChatManager *CBotChatManager::_Instance = NULL;


// ********************************************************************************************
CBotChatManager::CBotChatManager()
{
	_CurrPage = NULL;
	_SessionID = 0;
	//_ChosenMissionFlags = 0;
}

// ********************************************************************************************
CBotChatManager::~CBotChatManager()
{
	// Destruct
	nlassert(_CurrPage == NULL); // should have called setCurrPage(NULL) before quitting (and before releasing the interface) !
}

// ********************************************************************************************
CBotChatManager *CBotChatManager::getInstance()
{
	if (!_Instance) _Instance = new CBotChatManager;
	return _Instance;
}

// ********************************************************************************************
void CBotChatManager::releaseInstance()
{
	if( _Instance )
	{
		delete _Instance;
		_Instance = NULL;
	}
}

// ********************************************************************************************
void CBotChatManager::setCurrPage(CBotChatPage *page)
{
	if (_CurrPage)
	{
		_CurrPage->end();
	}
	if (page)
	{
		page->begin();
	}
	else
	{
		if (UserEntity)
			UserEntity->trader(CLFECOMMON::INVALID_SLOT);
	}
	_CurrPage = page;
}

// ********************************************************************************************
void CBotChatManager::update()
{
	if (_CurrPage) _CurrPage->update();
}

// ********************************************************************************************
void CBotChatManager::endDialog()
{
	NLMISC::CBitMemStream out;
	if(GenericMsgHeaderMngr.pushNameToStream("BOTCHAT:END", out))
	{
		// must increment action counter
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		pIM->incLocalSyncActionCounter();

		// send the msg
		NetMngr.push(out);
		//nldebug("impulseCallBack : BOTCHAT:END sent");
	}
	else
		nlwarning("impulseCallBack : unknown message name : 'BOTCHAT:END'.");
	setCurrPage(NULL);
}

// ***************************************************************************
void CBotChatManager::addMissionInfoWaiter(IMissionPrereqInfosWaiter *waiter)
{
	if(!waiter)
		return;

	// first remove the waiter if already here
	removeMissionInfoWaiter(waiter);

	// **** send a message to server to ask for prerequisite infos
	// Send a msg to server
	if(!ClientCfg.Local)
	{
		CBitMemStream out;
		if (GenericMsgHeaderMngr.pushNameToStream("MISSION_PREREQ:GET", out))
		{
			uint8	slotId = (uint8) waiter->MissionSlotId;
			out.serial( slotId );
			NetMngr.push(out);
			//nlinfo("impulseCallBack : MISSION_PREREQ:GET %d sent", slotId);

			// Then push_front (stack)
			_MissionInfoWaiters.push_front(waiter);
		}
		else
		{
			nlwarning(" unknown message name 'MISSION_PREREQ:GET'");
		}
	}
	// Debug Local
	else
	{
		_MissionInfoWaiters.push_front(waiter);
	}
}

// ***************************************************************************
void CBotChatManager::removeMissionInfoWaiter(IMissionPrereqInfosWaiter *waiter)
{
	TMissionPrereqInfosWaiter::iterator	it;
	for(it= _MissionInfoWaiters.begin();it!=_MissionInfoWaiters.end();it++)
	{
		if(waiter==*it)
		{
			_MissionInfoWaiters.erase(it);
			return;
		}
	}
}

// ***************************************************************************
void CBotChatManager::onReceiveMissionInfo(uint16 missionSlotId, const CPrerequisitInfos &infos)
{
	TMissionPrereqInfosWaiter::iterator	it,itDel;
	for( it = _MissionInfoWaiters.begin() ; it != _MissionInfoWaiters.end() ; )
	{
		IMissionPrereqInfosWaiter *waiter=*it;

		if(waiter->MissionSlotId == missionSlotId)
		{
			waiter->missionInfoReceived(infos);

			// remove waiter as infos received
			itDel = it;
			++it;
			_MissionInfoWaiters.erase(itDel);
		}
		else
		{
			++it;
		}
	}
}

// ***************************************************************************
void CBotChatManager::debugLocalReceiveMissionInfo()
{
#if FINAL_VERSION
	return;
#endif
	TMissionPrereqInfosWaiter::iterator	it,itNext;
	for( it = _MissionInfoWaiters.begin() ; it != _MissionInfoWaiters.end() ;)
	{
		IMissionPrereqInfosWaiter *waiter=*it;
		TMissionPrereqInfosWaiter::iterator	itNext= it;
		itNext++;

		// it will be deleted
		CPrerequisitInfos	infos;
		infos.Prerequisits.push_back(CPrerequisitDesc(1,false, true));
		infos.Prerequisits.push_back(CPrerequisitDesc(2,true, false));
		infos.Prerequisits.push_back(CPrerequisitDesc(3,false, false));
		infos.Prerequisits.push_back(CPrerequisitDesc(4,false, true));
		onReceiveMissionInfo(waiter->MissionSlotId, infos);

		// next
		it= itNext;
	}

}


// ********************************************************************************************
/*void CBotChatManager::processMissionHelpInfos(uint8 index, CPrerequisitInfos &infos)
{
	std::map<uint8,CInterfaceGroup*>::iterator it = _MissionHelpWindowsWaiting.find(index);
	if (it == _MissionHelpWindowsWaiting.end())
	{
		nlwarning("Error, mission help window for mission index %u not found", index);
		return;
	}

	CInterfaceGroup *help = (*it).second;
	nlassert(help != NULL);

	if (infos.Prerequisits.size() > 15)
	{
		// blabla
	}

	for (uint i = 0 ; i < infos.Prerequisits.size() ; ++i)
	{
		const std::string textId = NLMISC::toString("text_id_prereq_%u",i+1);

		CViewTextID	*viewTextID = dynamic_cast<CViewTextID *>(help->getView(textId));
		if(viewTextID)
		{
			viewTextID->setTextId(infos.Prerequisits[i].Description);
		}
#if 0
		if (!help->ScrollTextGroup.empty())
		{
			CInterfaceGroup *viewTextGroup = help->HelpWindow->getGroup(help->ScrollTextGroup);
			if (viewTextGroup)
				viewTextGroup->setActive(false);
		}
		CInterfaceGroup *viewTextGroup = help->HelpWindow->getGroup(help->ScrollTextIdGroup);
		if (viewTextGroup)
			viewTextGroup->setActive(true);
#endif
	}

	_MissionHelpWindowsWaiting.erase(index);
}
*/

/////////////////////
// ACTION HANDLERS //
/////////////////////

/** The user has closed a botchat program
  */
class CHandlerCloseBotChatProgram : public IActionHandler
{
public:
	virtual void execute(CCtrlBase * /* pCaller */, const std::string &/* params */)
	{
		CBotChatManager::getInstance()->endDialog();
	}
};
REGISTER_ACTION_HANDLER( CHandlerCloseBotChatProgram, "close_bot_chat_program");

