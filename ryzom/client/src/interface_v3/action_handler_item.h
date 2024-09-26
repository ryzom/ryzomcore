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

#ifndef CL_ACTION_HANDLER_ITEM_H
#define CL_ACTION_HANDLER_ITEM_H

#include "inventory_manager.h"

class CInterfaceItemEdition
{
private:
	class	CItemEditionWindow : public IItemInfoWaiter
	{
		public:
			~CItemEditionWindow() {	_CurrItemSheet = NULL;	}
			// start that page (display and setup ui)
			void begin();
			// end that page (hide and deactivate ui)
			void end();
			// update function : it is called at each frame.
			void update();
			void	validate();
			void infoReceived();
		public:
			CItemEditionWindow() : _CurrItemSheet(NULL) {}
			// If the window is in Edition or Display mode
			bool				IsInEditionMode;
			// The window to modify
			std::string WindowName;
			// The item used to open this window
			CDBCtrlSheet		*_CurrItemSheet;

	};

public:
	~CInterfaceItemEdition() {}

	// Get the unique instance of that class
	static CInterfaceItemEdition *getInstance();
	// release singleton
	static void releaseInstance();
	// Update the current page. Should be called at each frame
	void	update();
	// Validate the current page. send Message to server
	void	validate();
	// Get current setupped page, or NULL if none
	CItemEditionWindow	getCurrWindow() const { return _CurrWindow; }
	// Set the current page to display. Any previous page is hidden. Passing NULL just close all windows.
	void setCurrWindow(CDBCtrlSheet* ctrlSheet, const std::string &windowName = "", const bool &isInEditionMode = false);


private:
	CInterfaceItemEdition() {}

	//Manage only one CItemWindow could be changed to a vector
	CItemEditionWindow  _CurrWindow;
	static CInterfaceItemEdition *_Instance;

};

#endif