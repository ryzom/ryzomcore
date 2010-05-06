
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