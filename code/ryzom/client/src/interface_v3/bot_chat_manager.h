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



#ifndef CL_BOT_CHAT_MANAGER_H
#define CL_BOT_CHAT_MANAGER_H


class CBotChatPage;
class CPrerequisitInfos;

namespace NLGUI
{
	class CInterfaceGroup;
}

class	IMissionPrereqInfosWaiter
{
public:
	IMissionPrereqInfosWaiter() { MissionSlotId = 0; }
	virtual ~IMissionPrereqInfosWaiter() {}

	// The mission SheetId. If differ from current sheet in the SlotId, the infos are not updated / requested
//	uint			ItemSheet;
	// The mission SlotId to retrieve info.
	uint16			MissionSlotId;

	// Called when the info is received for this slot.
	virtual void	missionInfoReceived(const CPrerequisitInfos &infos) = 0;
};


/** Bot chat management.
  * The bot chat manager allow to change the current bot chat page, and contains pointer to the various pages
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date August 2003
  */
class CBotChatManager
{
public:
	~CBotChatManager();

	// Get the unique instance of that class
	static CBotChatManager *getInstance();

	// release singleton
	static void releaseInstance();

	// Get current setupped page, or NULL if none
	CBotChatPage		   *getCurrPage() const { return _CurrPage; }
	/** Set the current page to display. Any previous page is hidden. Passing NULL just close all windows.
	  * NB : this doesn't send the BOT_CHAT:END msg to the server, see endDialog
	  */
	void					setCurrPage(CBotChatPage *page);
	// Increment current session ID. (a session is all talk heppening during the selection of a target npc)
	uint16					getSessionID() { return _SessionID; }
	void					incrementSessionID() { ++ _SessionID; }
	// Update the current page. Should be called at each frame
	void					update();
	// Close the botchat, and send 'end' msg to the server
	void					endDialog();
	// this class retains the flags of mission option that has been chosen in the contextual menu
	/*
	uint                    getChosenMissionFlags() const { return _ChosenMissionFlags; }
	void					setChosenMissionFlags(uint flag) { _ChosenMissionFlags = flag; }
	*/

	// ***
	// Add a Waiter on mission prereq info (MissionHelp opening). no-op if here, but reorder
	void				addMissionInfoWaiter(IMissionPrereqInfosWaiter *waiter);
	// remove a Waiter on mission prereq info (MissionHelp closing). no-op if not here. NB: no delete
	void				removeMissionInfoWaiter(IMissionPrereqInfosWaiter *waiter);
	// Called on impulse
	void				onReceiveMissionInfo(uint16 missionSlotId, const CPrerequisitInfos &infos);
	// Called for local client debugging
	void				debugLocalReceiveMissionInfo();


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
private:
	CBotChatPage  *_CurrPage;
	uint16		   _SessionID;
	static CBotChatManager *_Instance;
	//uint           _ChosenMissionFlags;

	// *** keep infos on opened mission help windows (for prerequisits)
	typedef std::list<IMissionPrereqInfosWaiter*>	TMissionPrereqInfosWaiter;
	TMissionPrereqInfosWaiter						_MissionInfoWaiters;

private:
	CBotChatManager();
};




#endif
