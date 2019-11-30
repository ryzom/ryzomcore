// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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



#ifndef NL_GROUP_TAB_H
#define NL_GROUP_TAB_H

#include "nel/misc/types_nl.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/ctrl_text_button.h"

namespace NLGUI
{
	class CCtrlTabButton;



	// ***************************************************************************
	/**
	 * Group handling Ctrl Tab, to easily simulate Tab ctrl.
	 *	NB: controlled groups doesn't have to be child of the GroupTab, they are searched in order:
	 *		- in this group
	 *		- in the parent group
	 *		- in global
	 * \author Lionel Berenguier
	 * \author Nevrax France
	 * \date 2003
	 */
	class CGroupTab : public CInterfaceGroup
	{
	public:
        DECLARE_UI_CLASS( CGroupTab )

		/// Constructor
		CGroupTab(const TCtorParam &param);

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
		virtual void updateCoords ();

		// select the ctrl tab. -1 will invalidate all.
		void	select(sint index);
		sint	getSelection() const;

		// select with a CCtrlTabButton ptr
		void	selectFromCtrl(CCtrlTabButton *button);

		// select a default activated tab, if the current is a special ctrlTab
		void	selectDefault(CCtrlTabButton *ifSelectionIs);

		// select a default activated tab, if the current is hid
		void	selectDefaultIfCurrentHid();

		// add new tab
		void	addTab(CCtrlTabButton *tabB);
		void	addTab(CCtrlTabButton *tabB, sint index);
		int		luaAddTab(CLuaState &ls);
		int		luaAddTabWithOrder(CLuaState &ls);

		// remove selected tab
		void	removeTab(sint index);
		int		luaRemoveTab(CLuaState &ls);

		// remove all tabs
		void	removeAll();
		int		luaRemoveAll(CLuaState &ls);

		// tab number
		void			setTabButtonNb(sint32 /* val */){}
		sint32			getTabButtonNb() const {return (sint32)_Buttons.size();}

		// selection index
		void			setIndexSelection(sint32 val){select((sint)val);}
		sint32			getIndexSelection() const {return (sint32)_NextSelection;}

		// selection index
		void			setAssociatedGroupSelection(const std::string & /* assG */){}
		std::string		getAssociatedGroupSelection() const;

		// get group from index
		CInterfaceGroup*	getGroup(sint index);
		int					luaGetGroup(CLuaState &ls);

		// get tab from index
		CCtrlTabButton*	getTabButton(sint index);
		int					luaGetTabButton(CLuaState &ls);

		// first showed tab button
		sint32			getFirstTabButton() const {return (sint32)_FirstTabIndex;}

		// last showed tab button
		sint32			getLastTabButton() const {return (sint32)_LastTabIndex;}

		// update showed tab buttons on function of GroupTab width
		void				updateFirstTabButton();
		int					luaShowTabButton(CLuaState &ls);

		void dummySet(sint32 /* value */){}

		REFLECT_EXPORT_START(CGroupTab, CInterfaceGroup)
			REFLECT_LUA_METHOD("addTab", luaAddTab)
			REFLECT_LUA_METHOD("addTabWithOrder", luaAddTabWithOrder)
			REFLECT_LUA_METHOD("removeTab", luaRemoveTab)
			REFLECT_LUA_METHOD("removeAll", luaRemoveAll)
			REFLECT_LUA_METHOD("getGroup", luaGetGroup)
			REFLECT_LUA_METHOD("getTabButton", luaGetTabButton)
			REFLECT_LUA_METHOD("showTabButton", luaShowTabButton)
			REFLECT_SINT32 ("tabButtonNb", getTabButtonNb, setTabButtonNb)
			REFLECT_SINT32 ("selection", getIndexSelection, setIndexSelection)
			REFLECT_SINT32 ("firstTabButton", getFirstTabButton, dummySet)
			REFLECT_SINT32 ("lastTabButton", getLastTabButton, dummySet)
			REFLECT_STRING ("associatedGroupSelection", getAssociatedGroupSelection, setAssociatedGroupSelection)
		REFLECT_EXPORT_END

	private:

		std::vector<CCtrlTabButton*>	_Buttons;	// can't be NULL.
		std::vector<CInterfaceGroup*>	_Groups;	// may be NULL
		sint							_Selection;
		sint							_NextSelection;
		sint							_BaseRenderLayer;
		bool							_Setuped;
		bool							_HideOutTabs;
		sint							_FirstTabIndex;
		sint							_LastTabIndex;

		std::string						_AHOnChange;
		std::string						_ParamsOnChange;

		void		setup();
	};

	// ***************************************************************************
	/**
	 *	Used with CGroupTab
	 */
	class CCtrlTabButton : public CCtrlTextButton
	{
	public:
        DECLARE_UI_CLASS( CCtrlTabButton )

		CCtrlTabButton(const TCtorParam &param);

		void setProperty( const std::string &name, const std::string &value );
		std::string getProperty( const std::string &name ) const;
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);

		virtual void setActive(bool state);

		virtual bool handleEvent (const NLGUI::CEventDescriptor &event);

		void setBlink (bool b);

		std::string		_AssociatedGroup;
		IActionHandler *_AHOnLeftClick2;

	private:

		sint32			_DefaultX;
		bool			_Blinking;
		NLMISC::CRGBA	_TextColorNormalBlink;
		bool			_TextModulateGlobalColorNormalBlink;
		sint64			_BlinkDate;
		bool			_BlinkState;
	};

}

#endif // NL_GROUP_TAB_H

/* End of group_tab.h */
