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



#ifndef CL_CHAT_WINDOW_H
#define CL_CHAT_WINDOW_H

#include "nel/misc/ucstring.h"
#include "nel/misc/smart_ptr.h"

#include "game_share/chat_group.h"


namespace NLGUI
{
	class CCtrlBase;
	class CViewText;
	class CGroupList;
	class CGroupEditBox;
	class CGroupContainer;
	class CCtrlTabButton;
}

class CChatWindow;

/** Interface to react to a chat box entry
  * Derivers should define the msgEntered member function to handle entry event.
  */
struct IChatWindowListener
{
	// the user entered a msg in the given chat box
	virtual void msgEntered(const ucstring &msg, CChatWindow *chatWindow) = 0;
};


/** description of a chat window
  */
struct CChatWindowDesc
{
	typedef	std::vector<std::pair<std::string,std::string> >	TTemplateParams;

	ucstring				Title;                // unique title for the window
	std::string				FatherContainer;      // name of the father container. If empty, the chat box must be added manually in the hierarchy
	std::string				ChatTemplate;         // Template for the chat interface, or "" to use the default one
	TTemplateParams			ChatTemplateParams;	  // optional template parameters
	sint					InsertPosition;       // [optional] -1 if the chat box should be inserted at the end of the container list, or the index otherwise
	bool 					ParentBlink;          // [optional] when true, make the parent group blink
	bool                    Savable;              // should the position of the chat box be saved between session ? Default is false
	bool					Localize;			  // should we have to localize the window?
	IChatWindowListener		*Listener;
	std::string				Id;
	std::string				AHOnActive;
	std::string				AHOnActiveParams;
	std::string				AHOnDeactive;
	std::string				AHOnDeactiveParams;
	std::string				AHOnCloseButton;
	std::string				AHOnCloseButtonParams;
	std::string				HeaderColor;
	// default ctor : build optional parameters with their default values
	CChatWindowDesc();
};

/** This class can be used to easily manipulate a chat box without having to deal directly with the ui.
  * Each chat box must have a unique identifier.
  * For that reason, they should be created from a CChatWindowManager instance
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CChatWindow : public NLMISC::CRefCount
{
public:
	// a listener to know when a chat window is removed, or when a msg is displayed to it
	struct IObserver
	{
		// called by a CChatWindow when it is deleted
		virtual void chatWindowRemoved(CChatWindow * /* cw */) {}
		// called by a CChatWindow when a msg has been displayed in it ('displayMessage' has been called)
		//virtual void displayMessage(CChatWindow *cw, const ucstring &msg, NLMISC::CRGBA col, uint numBlinks = 0) {}
	};
public:
	// display a message in this chat box with the given color
	virtual void displayMessage(const ucstring &msg, NLMISC::CRGBA col, CChatGroup::TGroupType gt, uint32 dynamicChatDbIndex, uint numBlinks = 0, bool *windowVisible = NULL);
	virtual void displayTellMessage(const ucstring &/* msg */, NLMISC::CRGBA /* col */, const ucstring &/* sender */) {}
	virtual void clearMessages(CChatGroup::TGroupType gt, uint32 dynamicChatDbIndex);
	// Test if the window is visible
	bool isVisible() const;
	// set the keyboard focus to this chat window (if it has a edit box)
	void setKeyboardFocus();
	// Make the window blink
	void enableBlink(uint numBlinks);
	// set a command to be displayed and eventually executed in this chat window. std::string version for backward compatibility
	void setCommand(const std::string &command, bool execute);
	// set a command to be displayed and eventually executed in this chat window
	void setCommand(const ucstring &command, bool execute);
	// set a string to be displayed in the edit box of this window (if it has one)
	void setEntry(const ucstring &entry);
	// Set listener to react to a chat entry
	void setListener(IChatWindowListener *listener) { _Listener = listener; }
	IChatWindowListener *getListener() const { return _Listener; }
	// Set the menu for the chat
	void setMenu(const std::string &menuName);
	// Set a new prompt for the chat window
	void setPrompt(const ucstring &prompt);
	// Set the color for the chat window
	void setPromptColor(NLMISC::CRGBA col);
	/** Get the container associated with this chat window
	  * NB : you should not change the name of the window ! Use rename instead
	  */
	NLGUI::CGroupContainer     *getContainer() const { return _Chat; }
	//
	NLGUI::CGroupEditBox       *getEditBox() const;
	/** try to rename the chat window
	  * \return true if success
	  */
	bool rename(const ucstring &newName, bool newNameLocalize);
	/** delete the container
	  * Don't do it in the dtor, because done automatically at the end of the app by the interface manager.
	  * Useful only if querried by the user
	  */
	void deleteContainer();
	// get the last chat window from which a command has been called
	static CChatWindow *getChatWindowLaunchingCommand() { return _ChatWindowLaunchingCommand; }
	// get the title of this chat window
	ucstring getTitle() const;
	// observers
	void addObserver(IObserver *obs);
	void removeObserver(IObserver *obs);
	bool isObserver(const IObserver *obs) const;
	// AH adder
	void setAHOnActive(const std::string &n);
	void setAHOnActiveParams(const std::string &n);
	void setAHOnDeactive(const std::string &n);
	void setAHOnDeactiveParams(const std::string &n);
	void setAHOnCloseButton(const std::string &n);
	void setAHOnCloseButtonParams(const std::string &n);
	void setHeaderColor(const std::string &n);
	//
	void displayLocalPlayerTell(const ucstring &receiver, const ucstring &msg, uint numBlinks = 0);

	/// Encode a color tag '@{RGBA}' in the text. If append is true, append at end of text, otherwise, replace the text
	static void encodeColorTag(const NLMISC::CRGBA &color, ucstring &text, bool append=true);

///////////////////////////////////////////////////////////////////////////////////////
protected:
	// ctor
	CChatWindow();
	// dtor
	virtual ~CChatWindow();
protected:
	IChatWindowListener *_Listener;
	NLMISC::CRefPtr<NLGUI::CGroupContainer> _Chat;
	NLGUI::CGroupEditBox		*_EB;
	bool 				 _ParentBlink;
	static CChatWindow  *_ChatWindowLaunchingCommand;
	std::vector<IObserver *> _Observers;
protected:
	friend class CChatWindowManager;
	friend class CHandlerChatBoxEntry;
	friend class CHandlerContactEntry; // TODO : remove this if CChatBox are used in people lists
	/** Create a chat window
	  * The name and the id should be unique
	  * The id shouldn't contains ui:interface
	  */
	bool create(const CChatWindowDesc &desc, const std::string &id);
};

// -----------------------------------------------------------------------------------
class CChatGroupWindow : public CChatWindow
{
public:
	CChatGroupWindow() {}
	// display a message in this chat box with the given color (callback from chat input filter)
	virtual void displayMessage(const ucstring &msg, NLMISC::CRGBA col, CChatGroup::TGroupType gt, uint32 dynamicChatDbIndex, uint numBlinks = 0, bool *windowVisible = NULL);
	virtual void displayTellMessage(const ucstring &msg, NLMISC::CRGBA col, const ucstring &sender);
	virtual void clearMessages(CChatGroup::TGroupType gt, uint32 dynamicChatDbIndex);
	sint32 getTabIndex();
	void setTabIndex(sint32 n);

	// Free Teller
	NLGUI::CGroupContainer *createFreeTeller(const ucstring &winName, const std::string &winColor="");
	void setActiveFreeTeller(const ucstring &winName, bool bActive=true);
	ucstring getFreeTellerName(const std::string &containerID);
	bool removeFreeTeller(const std::string &containerID); // Return true if free teller found
	void removeAllFreeTellers();
	void saveFreeTeller(NLMISC::IStream &f);
	void loadFreeTeller(NLMISC::IStream &f);
	// update headers of all free tellers
	void updateAllFreeTellerHeaders();


protected:
	friend class CChatWindowManager;

	std::vector<NLGUI::CGroupContainer*>	_FreeTellers;

	void	getAssociatedSubWindow(CChatGroup::TGroupType gt, uint32 dynamicChatDbIndex, NLGUI::CGroupList *&gl, NLGUI::CCtrlTabButton *&tab);
	void    updateFreeTellerHeader(NLGUI::CGroupContainer &ft);

private:
	/** Get a valid string to use like ui id
	  *	\param stringId initial unchecked string
	  * \return valid string
	 **/
	const std::string getValidUiStringId(const std::string &stringId);
};


/** Class that manage several chat windows with unique names
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CChatWindowManager
{
public:
	// dtor
	~CChatWindowManager();
	/** Create a new chat window. Each chat window should have a unique name
	  * The window should be inserted in another container by the caller.
	  * \param name A unique title to affect to that window
	  * \param listener A listener to react to the event of the window
	  * \return A pointer on the window, or NULL, if creation failed or if name already exists.
	  */
	CChatWindow *createChatWindow(const CChatWindowDesc &desc);

	CChatWindow *createChatGroupWindow(const CChatWindowDesc &desc);

	// Get a chat window by its title
	CChatWindow *getChatWindow(const ucstring &title);
	/// Remove a chat window by its title
	void		 removeChatWindow(const ucstring &title);
	// Remove a chat window by its pointer
	void		 removeChatWindow(CChatWindow *cw);
	/// from a ctrl of a chat box that triggered a menu, or an event, retrieve the associated chat box
	CChatWindow *getChatWindowFromCaller(NLGUI::CCtrlBase *caller);
	// Singleton pattern applied to the chat window manager
	static CChatWindowManager &getInstance();
	// try to rename a window
	bool rename(const ucstring &oldName, const ucstring &newName, bool newNameLocalize);
	// warning : this is slow
	uint getNumChatWindow() const { return (uint)_ChatWindowMap.size(); }
	// warning : this is slow : for debug only
	CChatWindow *getChatWindowByIndex(uint index);
///////////////////////////////////////////////////////////////////////////////////////
private:
	typedef std::map<ucstring, NLMISC::CRefPtr<CChatWindow> > TChatWindowMap;
private:
	//
	TChatWindowMap	_ChatWindowMap;
	uint            _WindowID;
private:
	// ctor
	CChatWindowManager();
};

// shortcut to get instance of the chat window manager
inline CChatWindowManager &getChatWndMgr() { return CChatWindowManager::getInstance(); }

#endif
