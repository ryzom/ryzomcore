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



#ifndef CL_CHAT_FILTER_H
#define CL_CHAT_FILTER_H

#include "chat_window.h"
#include "game_share/chat_group.h"

#include "nel/misc/smart_ptr.h"

class CPeopleList;


// ***************************************************************************
/** Filter for chat input
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CChatInputFilter : public NLMISC::CRefCount, public CChatWindow::IObserver
{
public:
	CChatInputFilter() {FilterType= CChatGroup::say; DynamicChatDbIndex= 0;}
	virtual ~CChatInputFilter();
	/** Display a msg in the chat. The msg will be forwarded to all the listening windows
	  * Listening windows will blick only if there isnt a single visible listening window, so that the player can know if there's a message
	  * \param windowVisible is not NULL, points a bool that will be filled with true if one of the window on the which the msg was displayed is visible.
	  */
	void displayMessage(const ucstring &msg, NLMISC::CRGBA col, uint numBlinks = 0, bool *windowVisible = NULL);
	/** The same as displayMessage, but with sender name, so that the msg will be displayed in attached people lists as well
	  */
	void displayTellMessage(/*TDataSetIndex &senderIndex, */const ucstring &msg, const ucstring &sender, NLMISC::CRGBA col, uint numBlinks = 0, bool *windowVisible = NULL);
	/** Clear the messages in all registered chat windows
	 */
	void clearMessages();

	/** \name Listening windows
	  * NB : Because of observer system, a chat window will call removeListeningWindow automatically when it is destroyed
	  */
	//@{
		void			addListeningWindow(CChatWindow *w);
		void			removeListeningWindow(CChatWindow *w);
		bool			isListeningWindow(CChatWindow *w) const;
		uint			getNumListeningWindows() const { return (uint)_ListeningWindows.size(); }
		CChatWindow		*getListeningWindow(uint index);
		// helpers : depending on the 'listening' flag value, remove or add the window from the list
		void			 setWindowState(CChatWindow *cw, bool listening);
	//@}
	///\name Listening people lists
	//@{
		void			addListeningPeopleList(CPeopleList *pl);
		void			removeListeningPeopleList(CPeopleList *pl);
		bool			isListeningPeopleList(CPeopleList *pl) const;
	//@}

	// For ChatGroup, useful to know for which chat it is destinated
	CChatGroup::TGroupType	FilterType;
	// If FilterType==CChatGroup::dyn_chat, gives the index of dynchat
	uint32					DynamicChatDbIndex;

private:
	std::vector<CChatWindow *> _ListeningWindows;
	std::vector<CPeopleList *> _ListeningPeopleList;
	// from CChatWindow::IObserver
	virtual void chatWindowRemoved(CChatWindow *cw);
	//
	// copy not supported
	CChatInputFilter(const CChatInputFilter &/* other */):NLMISC::CRefCount() { nlassert(0); }
	CChatInputFilter &operator=(const CChatInputFilter &/* other */) { nlassert(0); return *this; }

};


// ***************************************************************************
/** Filter for chat targets
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CChatTargetFilter : public NLMISC::CRefCount,
                          public IChatWindowListener,
						  public CChatWindow::IObserver
{
public:
	// ctor
	CChatTargetFilter();
	// dtor
	~CChatTargetFilter();

	void reset();

	///\name Filtered window
	//@{
		/**  Set the chat on which this filter acts.
		  *  The filter becomes the current listener for the window
		  */
		void             setChat(CChatWindow *w);
		CChatWindow		*getChat() const { return _Chat; }
	//@}


	///\name Target.
	//@{
			/** Set the target to be a party chat.
			  * NB : this replace any previous party chat or target group or player
			  */
			void			setTargetPartyChat(CChatWindow *w);
			CChatWindow		*getTargetPartyChat() const { return _TargetPartyChat; }

			/** Set the target to be a standard target (as described in CChatGroup)
			  * NB : this replace any previous party chat or target group
			  *	\param dynamicChannelId: valid only if group is 'dyn_chat' (for server dynamic channels)
			  */
			void					setTargetGroup(CChatGroup::TGroupType groupType, uint32 dynamicChannelDbIndex= 0, bool allowUniverseWarning= true);
			CChatGroup::TGroupType  getTargetGroup() const { return _TargetGroup; }
			uint32					getTargetDynamicChannelDbIndex() const { return _TargetDynamicChannelDbIndex; }

			/** Set a player as the target. This remove any previous window target
			  * NB : this replace any previous party chat or target group or player
			  */
			void			   setTargetPlayer(const ucstring &targetPlayer);
			const ucstring     &getTargetPlayer() const { return _TargetPlayer; }
	//@}
private:

	///\name chat window on which the filter acts
	// @{
		CChatWindow				   *_Chat;
	// @}

	///\name targets
	// @{
		CChatWindow				   *_TargetPartyChat; // the target party chat
		CChatGroup::TGroupType      _TargetGroup;
		ucstring					_TargetPlayer;
		// relevant only if _TargetGroup==dyn_chat
		uint32						_TargetDynamicChannelDbIndex;
	// @}
private:
	// from IChatWindowListener
	void chatWindowRemoved(CChatWindow *cw);
	void msgEntered(const ucstring &msg, CChatWindow *chatWindow);
	// copy not supported
	CChatTargetFilter(const CChatTargetFilter &/* other */):NLMISC::CRefCount() { nlassert(0); }
	CChatTargetFilter& operator=(const CChatTargetFilter &/* other */) { nlassert(0); return *this; }
};

#endif
