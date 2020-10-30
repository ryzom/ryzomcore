// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2014-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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



#ifndef RZ_GROUP_MENU_H
#define RZ_GROUP_MENU_H

#include "nel/gui/interface_group.h"
#include "nel/gui/group_modal.h"
#include "nel/gui/group_submenu_base.h"
#include "nel/gui/view_text.h"
#include "nel/gui/ctrl_text_button.h"

namespace NLGUI
{
	class CCtrlScroll;
	class CViewBitmap;
	class CGroupList;
	class CGroupMenu;


	/**
	 * CViewTextMenu is an element of a sub menu
	 * \author Matthieu 'TrapII' Besson
	 * \author Nevrax France
	 * \date 2002
	 */
	class CViewTextMenu : public CViewText
	{
	public:
        DECLARE_UI_CLASS( CViewTextMenu )

		CViewTextMenu(const TCtorParam &param) : CViewText(param)
		{
			_Grayed = false;
			_Checked = false;
			_Checkable = false;
			_CheckBox = NULL;
			Over = false;
		}

		bool getGrayed() const;
		void setGrayed (bool g);
		bool getChecked() const { return _Checked; }
		void setChecked(bool c);
		bool getCheckable() const { return _Checkable; }
		void setCheckable(bool c);
		void setCheckBox(CViewBitmap *checkBox) { _CheckBox = checkBox; }
		CViewBitmap * getCheckBox() const { return _CheckBox; }
		bool getFormatted () const { return getMultiLine (); }

		virtual sint32 getAlpha() const;
		virtual void setAlpha (sint32 a);

		REFLECT_EXPORT_START(CViewTextMenu, CViewText)
			REFLECT_BOOL("grayed", getGrayed, setGrayed);
			REFLECT_BOOL("checked", getChecked, setChecked);
		REFLECT_EXPORT_END

	public:

		bool			Over;
		NLMISC::CRGBA	OldColor;
		NLMISC::CRGBA	OldShadowColor;
		NLMISC::CRGBA	OldColorOver;
		NLMISC::CRGBA	OldShadowColorOver;
		NLMISC::CRGBA	OldColorGrayed;
		NLMISC::CRGBA	OldShadowColorGrayed;

	private:
		CViewBitmap *_CheckBox;
		bool		_Grayed;
		bool		_Checked;
		bool		_Checkable;
	};

	/**
	 * CGroupSubMenu describe an element of a contextual menu (contains text lines and sub menu)
	 * \author Matthieu 'TrapII' Besson
	 * \author Nevrax France
	 * \date 2002
	 */

	class CGroupSubMenu : public CGroupSubMenuBase
	{
	public:
        DECLARE_UI_CLASS( CGroupSubMenu )

		CGroupSubMenu(const TCtorParam &param);
		virtual ~CGroupSubMenu();

		virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parent=NULL);

		virtual void checkCoords();

		virtual void updateCoords ();

		virtual void draw ();

		virtual bool handleEvent (const NLGUI::CEventDescriptor &eventDesc);

		virtual CInterfaceElement* getElement (const std::string &id);

		// retrieve the index of a line from its id (-1 if not found)
		sint getLineFromId(const std::string &id);

		CViewTextMenu* addLine (const std::string &name, const std::string &ah,
								const std::string &params, const std::string &id="",
								const std::string &cond = std::string(), const std::string &texture="",
								bool checkable = false, bool checked = false, bool formatted = false
							   );
		CViewTextMenu* addLineAtIndex(uint index, const std::string &name, const std::string &ah,
									  const std::string &params, const std::string &id="",
									  const std::string &cond = std::string(), const std::string &texture="",
									  bool checkable = false, bool checked = false, bool formatted = false
									 );
		void addSeparator(const std::string &id = "");
		void addSeparatorAtIndex(uint index, const std::string &id = "");


		uint	getNumLine() const { return (uint)_Lines.size(); }
		void	removeLine(uint index);
		const std::string getActionHandler(uint lineIndex) const;
		const std::string getActionHandlerParam(uint lineIndex) const;
		const std::string getRightClickHandler(uint lineIndex) const;
		const std::string getRightClickHandlerParam(uint lineIndex) const;

		void setActionHandler(uint lineIndex, const std::string &ah = "");
		void setActionHandlerParam(uint lineIndex, const std::string &params = "");
		void setRightClickHandler(uint lineIndex, const std::string &ah = "");
		void setRightClickHandlerParam(uint lineIndex, const std::string &params = "");

		void	openSubMenu (sint32 nb);

		void	hideSubMenus ();

		// reset all entries of the sub menu
		void	reset();

		virtual void setActive (bool state);

		// Tell if the line is a separator or not
		bool isSeparator (uint i) const;

		/** Set a user defined group at the given line
		  * 'ownership' tells whether this menu should remove the group when it is deleted
		  * Setting a user group on a line with a separator is illegal
		  */
		void			 setUserGroupRight(uint line, CInterfaceGroup *group, bool ownership);
		CInterfaceGroup *getUserGroupRight(uint line) const;
		//
		void			 setUserGroupLeft(uint line, CInterfaceGroup *group, bool ownership);
		CInterfaceGroup *getUserGroupLeft(uint line) const;

		void removeAllUserGroups();

		uint getNumLines() const { return (uint)_Lines.size(); }

		// return pointer to submenu or NULL if there's none
		CGroupSubMenu *getSubMenu(uint index) const;
		void setSubMenu(uint index, CGroupSubMenu *sub);

		// if a menu isn't selectable, can't click on it, and there's no selection when the mouse is over it (but can click on its widgets, such as a usergroup)
		void setSelectable(uint lineIndex, bool selectable);
		bool getSelectable(uint lineIndex) const;

		// Gray a line.
		void setGrayedLine(uint line,  bool g);

		// Hide a line.
		void setHiddenLine(uint line,  bool h);

		// Max Visible Line (-1 == no limit)
		void setMaxVisibleLine(sint32 mvl);
		sint32 getMaxVisibleLine() { return _MaxVisibleLine; }

		// Get the Line Id (not the full Id)
		const std::string &getLineId(uint index);

		int luaGetNumLine(CLuaState &ls);
		int luaGetSubMenu(CLuaState &ls);
		int luaAddSubMenu(CLuaState &ls);
		int luaGetLineId(CLuaState &ls);
		int luaGetLineFromId(CLuaState &ls);
		int luaIsSeparator(CLuaState &ls);
		int luaAddLine(CLuaState &ls);
		int luaAddIconLine(CLuaState &ls);
		int luaAddLineAtIndex(CLuaState &ls);
		int luaAddSeparator(CLuaState &ls);
		int luaAddSeparatorAtIndex(CLuaState &ls);
		int luaRemoveLine(CLuaState &ls);
		int luaSetUserGroupRight(CLuaState &ls);
		int luaGetUserGroupRight(CLuaState &ls);
		int luaSetUserGroupLeft(CLuaState &ls);
		int luaGetUserGroupLeft(CLuaState &ls);
		int luaReset(CLuaState &ls);
		int luaSetMaxVisibleLine(CLuaState &ls);
		//
		REFLECT_EXPORT_START(CGroupSubMenu, CGroupSubMenuBase)
			REFLECT_LUA_METHOD("getNumLine", luaGetNumLine);
			REFLECT_LUA_METHOD("getLineId", luaGetLineId); // return the id of a line from its index
			REFLECT_LUA_METHOD("getLineFromId", luaGetLineFromId); // return -1 if line with id is not found
			REFLECT_LUA_METHOD("getSubMenu", luaGetSubMenu);
			REFLECT_LUA_METHOD("addSubMenu", luaAddSubMenu);
			REFLECT_LUA_METHOD("isSeparator", luaIsSeparator);
			REFLECT_LUA_METHOD("addLine", luaAddLine); // name, ah, ah_params, id
			REFLECT_LUA_METHOD("addIconLine", luaAddIconLine); // name, ah, ah_params, id, texture
			REFLECT_LUA_METHOD("addLineAtIndex", luaAddLineAtIndex); // index, name, ah, ah_params, id
			REFLECT_LUA_METHOD("addSeparator", luaAddSeparator);
			REFLECT_LUA_METHOD("addSeparatorAtIndex", luaAddSeparatorAtIndex);
			REFLECT_LUA_METHOD("removeLine", luaRemoveLine);
			REFLECT_LUA_METHOD("reset", luaReset);
			REFLECT_LUA_METHOD("setUserGroupRight", luaSetUserGroupRight); // line, group ptr
			REFLECT_LUA_METHOD("getUserGroupRight", luaGetUserGroupRight); // line
			REFLECT_LUA_METHOD("setUserGroupLeft", luaSetUserGroupLeft); // line, group ptr
			REFLECT_LUA_METHOD("getUserGroupLeft", luaGetUserGroupLeft);// line
			REFLECT_LUA_METHOD("setMaxVisibleLine", luaSetMaxVisibleLine);
		REFLECT_EXPORT_END

	protected:

		struct SSubMenuEntry
		{
			CViewTextMenu	*ViewText; // Backup of the children that are in grouplist
			CInterfaceGroup *Separator;
			std::string		AHName;
			std::string		AHParams;
			std::string		AHRightClick;
			std::string		AHRightClickParams;
			std::string		Id;
			std::string     Cond;      // condition to know if the entry is grayed
			CViewBitmap     *CheckBox;
			CViewBitmap     *RightArrow;
			CInterfaceGroup *UserGroupRight; // not for separator, inserted before checkbox & submenu arrow
			CInterfaceGroup *UserGroupLeft;
			bool			 UserGroupRightOwnership;
			bool			 UserGroupLeftOwnership;
			bool			 Selectable;
			sint32			HReal;		// max H of the view text and the other user group
			sint32			TextDY;		// Y of the view text to set
			SSubMenuEntry()
			{
				ViewText = NULL;
				Separator = NULL;
				CheckBox = NULL;
				RightArrow = NULL;
				UserGroupRight = NULL;
				UserGroupLeft = NULL;
				UserGroupRightOwnership = false;
				Selectable = true;
				HReal= 0;
				TextDY= 0;
			}
		};

	protected:

		CGroupList						*_GroupList;
		CCtrlScroll						*_ScrollBar;
		CViewBitmap						*_SelectionView;
		std::vector<SSubMenuEntry>		_Lines;
		std::vector<CGroupSubMenu*>		_SubMenus;

		CGroupMenu						*_GroupMenu; // Master parent
		sint32							_Selected;

		sint32							_MaxVisibleLine; // -1 == no limit

		friend class CGroupMenu;
	private:
		/** Clone this menu, and set its new father
		  * If appendToMenu is NULL, the menu is just copied
		  * otherwise, no copy is made, but this menu entries are appended to the already created 'appendMenu' menu.
		  * NB : user groups are not duplicated
		  */
		CGroupSubMenu *cloneMenu(CGroupSubMenu *appendToMenu, CGroupMenu *newFather, CInterfaceGroup *initGroup = NULL) const;
		void           initOptions(CInterfaceGroup *parent);
		CViewBitmap    *createIcon(CInterfaceElement *parentPos, const std::string &texture);
		CViewBitmap	   *createCheckBox(bool checked);
		CViewBitmap	   *createRightArrow(CInterfaceElement *parentPos, bool center);
	};

	/**
	 * class describing a menu composed of one or more CGroupListSubMenu
	 * \author Matthieu 'TrapII' Besson
	 * \author Nevrax France
	 * \date 2002
	 */
	class CGroupMenu : public CGroupModal
	{

	public:
        DECLARE_UI_CLASS( CGroupMenu )

		CGroupMenu(const TCtorParam &param);
		virtual ~CGroupMenu();

		TCaseMode getCaseMode() { return _CaseMode; }

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);

		virtual void draw ();

		void recurseDraw(CGroupSubMenu *pSubMenu);

		virtual bool handleEvent (const NLGUI::CEventDescriptor &eventDesc);

		virtual CInterfaceElement* getElement (const std::string &id);

		virtual void setActive (bool state);

		virtual bool isWindowUnder (sint32 x, sint32 y);
		
		uint getNumLine() const;
		void deleteLine(uint index);
		const std::string getActionHandler(uint lineIndex) const;
		const std::string getActionHandlerParam(uint lineIndex) const;
		const std::string getRightClickHandler(uint lineIndex) const;
		const std::string getRightClickHandlerParam(uint lineIndex) const;

		void setActionHandler(uint lineIndex, const std::string &ah = "");
		void setActionHandlerParam(uint lineIndex, const std::string &params = "");
		void setRightClickHandler(uint lineIndex, const std::string &ah = "");
		void setRightClickHandlerParam(uint lineIndex, const std::string &params = "");

		void addLine (const std::string &name, const std::string &ah = "", const std::string &params = "",
					  const std::string &id = std::string(),
					  const std::string &cond = std::string(), const std::string &texture="",
					  bool checkable = false, bool checked = false
					 );
		void addLineAtIndex (uint index, const std::string &name, const std::string &ah = "", const std::string &params = "",
							 const std::string &id = std::string(),
							 const std::string &cond = std::string(), const std::string &texture="",
							 bool checkable = false, bool checked = false
							);
		void setUserGroupRight(uint line, CInterfaceGroup *gr, bool ownerShip = true);
		void setUserGroupLeft(uint line, CInterfaceGroup *gr, bool ownerShip = true);

		// clear all sub menus
		void reset ();

		// set the minW of the RootMenu.
		void	setMinW(sint32 minW);
		void	setMinH(sint32 minH);

		// change fontsize for new menu items
		void	setFontSize(uint32 fontSize, bool coef = true);

		// Gray a line on the RootMenu
		void	setGrayedLine(uint line, bool g);

		CGroupSubMenu	*getRootMenu() const { return _RootMenu; }

		// Max Visible Line (-1 == no limit)
		void setMaxVisibleLine(sint32 mvl) { _RootMenu->setMaxVisibleLine(mvl); }
		sint32 getMaxVisibleLine() { return _RootMenu->getMaxVisibleLine(); }

		// special for menu launched from a modal....
		bool	getCloseSubMenuUsingPopModal() const {return _CloseSubMenuUsingPopModal;}
		void	setCloseSubMenuUsingPopModal(bool state) {_CloseSubMenuUsingPopModal= state;}

		int luaGetRootMenu(CLuaState &ls);
		int luaSetMinW(CLuaState &ls);

		REFLECT_EXPORT_START(CGroupMenu, CGroupModal)
			REFLECT_LUA_METHOD("getRootMenu", luaGetRootMenu);
			REFLECT_LUA_METHOD("setMinW", luaSetMinW);
		REFLECT_EXPORT_END

	protected:
		TCaseMode		 _CaseMode;

		CGroupSubMenu	*_RootMenu;

		std::string _Extends;

		// Text lookup
		NLMISC::CRGBA			_Color;
		NLMISC::CRGBA			_ShadowColor;

		bool					_CloseSubMenuUsingPopModal;
		bool					_Shadow;
		bool					_ShadowOutline;
		bool					_Formatted;
		uint8					_Space;
		sint32					_FontSize;
		bool					_FontSizeCoef;

		NLMISC::CRGBA			_ColorOver;			// Color of the text when the mouse is over it
		NLMISC::CRGBA			_ShadowColorOver;	// Color of the shadow when the mouse is over it
		NLMISC::CRGBA			_HighLightOver;		// Background color of the selection

		NLMISC::CRGBA			_ColorGrayed;		// Color of the text when it is unusable
		NLMISC::CRGBA			_ShadowColorGrayed;	// Color of the shadow when it is unusable

		friend class CGroupSubMenu;
	};

}

#endif // RZ_GROUP_MENU_H

/* End of group_menu.h */


