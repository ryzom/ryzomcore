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



#ifndef CL_ACTION_HANDLER_HELP_H
#define CL_ACTION_HANDLER_HELP_H


#include "inventory_manager.h"
#include "bot_chat_manager.h"


// Struct that tells how to setup a help window
class CSheetHelpSetup
{
public:
	CInterfaceGroup *HelpWindow;
	CDBCtrlSheet	 *SrcSheet;			// the sheet for which info should be displayed (outside of the help window)
	CDBCtrlSheet	 *DestSheet;		// copy of the sheet for which info are asked (this sheet is in the help window, the source sheet is copied in it)
	std::string      ViewText;			// id of the text inside the help window that receive infos about the sheet
	std::string      ViewTextID;		// id of the text_id inside the help window that receive infos about the sheet (for dynamic infos -> missions)
	std::string		 ScrollTextGroup;   // name of the group that contains the description text
	std::string		 ScrollTextIdGroup; // name of the group that contains the description text_id
	std::string		 CreatorViewTextID; // name of the text_id in the help window that contain the name of the item's creator
	std::string		 PrefixForExtra;	// Id Prefix for list brick and mp stats (from the help window root)
	bool			 FromBotChat;

public:
	// ctor
	CSheetHelpSetup()
	{
		HelpWindow = NULL;
		SrcSheet = NULL;
		DestSheet = NULL;
		FromBotChat= false;
	}
	// set the default ids for help window
	void setupDefaultIDs();
};


// reset some elements in the windows (brick lists etc...)
void resetSheetHelp(CSheetHelpSetup &setup);

// Setup help for a sheet in a window. Type is deduced from the sheet
void setupSheetHelp(CSheetHelpSetup &setup);

// Refresh help for an item
void refreshItemHelp(CSheetHelpSetup &setup);
// refresh help for a mission
void refreshMissionHelp(CSheetHelpSetup &setup, const CPrerequisitInfos &infos);

class CPetAnimalItemInfoWaiter : public IItemInfoWaiter
{
	void infoReceived()
	{
		//ItemSheet
		//ItemSlotId
		CClientItemInfo info = getInventory().getItemInfo(ItemSlotId);
	}
};


// ***************************************************************************
/** static methods to manipulate Helps
 */
class	CInterfaceHelp
{
public:
	// Open and set the group next to the element
	static CInterfaceGroup	*activateNextWindow(CDBCtrlSheet *elt, sint forceKeepWindow=-1);

	// Close all the Help Windows
	static	void			closeAll();

	// remove waiter for ItemInfo
	static	void			removeWaiterItemInfo(uint i);
	static	void			removeWaiterMissionInfo(uint i);

	static	void			changeKeepMode(uint i);
	static	void			setKeepMode(uint i, bool state);

	// init windows default pos.
	static	void			resetWindowPos(sint y);

	// serial info windows
	static void				serialInfoWindows(NLMISC::IStream &f);

	// debug info windows
	static void				debugOpenedInfoWindows();

	// update SPhrase Text of all opened windows
	static void				updateWindowSPhraseTexts();

	// release the manager
	static void				release();

private:
	friend class	CHandlerHelpKeep;
	friend class	CAHItemHelpMpChangeItemPart;

	class	CInfoWindowSave
	{
	public:
		sint32	X, Y;
		void	serial(NLMISC::IStream &f)
		{
			f.serialVersion(0);
			f.serial(X, Y);
		}
	};
	class	CInfoWindow : public IItemInfoWaiter, public IMissionPrereqInfosWaiter
	{
	public:
		// The Info Window
		CInterfaceGroupPtr	Window;
		// The item used to open this window
		CDBCtrlSheet	*CtrlSheet;
		// KeepMode
		bool			KeepMode;
		// The button for KeepMode. Button state == KeepMode
		CCtrlBaseButtonPtr	KeepButton;
	public:
		CInfoWindow() : CtrlSheet(NULL) {KeepMode= false;}
		virtual void	infoReceived();
		virtual void	missionInfoReceived(const CPrerequisitInfos &infos);
	};

	// update the brick help window when weight of hands has changed
	class CFittedWeaponWeightObserver : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		virtual void update(NLMISC::ICDBNode* node);
	};


	static std::deque<uint>				_ActiveWindows;
	static std::vector<CInfoWindow>		_InfoWindows;
	static bool							_InfoWindowInit;
	static CFittedWeaponWeightObserver  _FittedWeaponWeightObserver;

	static void		initWindows();
};



#endif
