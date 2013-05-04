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



#ifndef NL_INPUT_HANDLER_MANAGER_H
#define NL_INPUT_HANDLER_MANAGER_H

#include <libxml/parser.h>
#include "nel/misc/event_server.h"
#include <vector>
#include <map>
#include <string>
#include "nel/gui/event_descriptor.h"
#include "nel/gui/input_handler.h"
#include "nel/gui/group_editbox.h"


/**
 * CInputManager. Class used to manage Input Handlers. its purpose is to manage all input handler
 * CInputManager get hardware events from input handlers, when they call CInputManager::ProcessEvent
 * CInputMangager convert the incoming events into actions and call the appropriate control's callback
 *
 * This conversion is made through the reading of a config file describing the association between events and actions
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2002
 */


class CInputHandlerManager : public NLMISC::IEventListener, public CGroupEditBox::IComboKeyHandler
{
public:
	/// The EventServer Filled with Filtered Messages the InterfaceManager didn't cactch
	NLMISC::CEventServer	FilteredEventServer;

public:
	/**
	 * getInstance because this class is a singleton
	 */
	static CInputHandlerManager* getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CInputHandlerManager();
		return _Instance;
	}

	// release singleton
	static void releaseInstance();

	/**
	 * set the input server
	 * \param server : the input event server
	 */
	void addToServer(NLMISC::CEventServer * server);


	/**
	 * release all data members
	 */
	void release();


	/**
	 * callback operator called when a low-level hardware event is received
	 *	EventServer
	 * \param event : the event received
	 */
	void operator ()(const NLMISC::CEvent& event);


	/**
	 * read the input config file.
	 * \param fileName : name of the config file
	 * \return true if no error occured
	 */
	bool	readInputConfigFile(const std::string & fileName);


	/** Return true if the event is handled by Text Edition in the interface Manager
	 *	All Events are stored in the xml input config file.
	 *	Basics events such as KeyA, KeyB etc.. should be in this config file.
	 *	Combo Keys like Ctrl+A (select all) should be in this config file too.
	 */
	bool	isComboKeyChat(const NLGUI::CEventDescriptorKey &edk) const;


	/** Pump The Events of The setuped EventServer (ie Driver->EventServer).
	 *	Then pump the filtered ones
	 */
	void	pumpEvents();

	/** Same as pumpEvents() but the messages are not processed by the InterfaceManager, but directly
	 *	posted to FilteredEventServer
	 */
	void	pumpEventsNoIM();

	/** Reset the position of the mouse
	*/
	void	resetPos (sint x, sint y);


	bool hasFocus() const { return _Focus; }

private:

	/**
	 * private constructor for singleton
	 */
	CInputHandlerManager();

	// destructor
	~CInputHandlerManager();

	///the singleton's instance
	static CInputHandlerManager* _Instance;

	///pointer to an event server
	NLMISC::CEventServer* _EventServer;

	// Mouse Infos
	NLMISC::TMouseButton	_MouseButtonsState;

	sint32					_MouseX, _MouseY;
	sint32					_MouseLastX, _MouseLastY;
	sint32					_MouseWheel;

	bool					_Focus;
	bool					_SkipInterfaceManager;
	bool					_RecoverFocusLost;

	// CComboKey
	class	CComboKey
	{
	public:
		enum TControlFlag
		{
			CtrlKey= 1,
			ShiftKey= 2,
			AltKey= 4,
		};

	public:
		NLMISC::TKey	Key;
		uint32			CtrlFlags;

	public:
		CComboKey()
		{
			CtrlFlags= 0;
		}
		void		init(const NLGUI::CEventDescriptorKey &rDK);
		bool		operator<(const CComboKey &c) const;
	};

	// List of key used in Text edition
	std::set<CComboKey>		_ComboKeyChat;

private:

	void parseComboKeyChat(xmlNodePtr cur);
	void parseKey(xmlNodePtr cur, std::vector<CComboKey> &out);

	// return true if handled
	bool updateMousePos(NLMISC::CEventMouse &event, NLGUI::CEventDescriptorMouse &eventDesc);

	NLGUI::CInputHandler inputHandler;

};

#endif // NL_INPUT_HANDLER_MANAGER_H

/* End of input_handler_manager.h */


