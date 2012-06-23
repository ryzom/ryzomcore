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



#ifndef NL_DBGROUP_COMBO_BOX_H
#define NL_DBGROUP_COMBO_BOX_H

#include "nel/misc/types_nl.h"
#include "interface_group.h"



// ***************************************************************************
class	CViewText;
class	CCtrlBaseButton;
class	CGroupMenu;

// ***************************************************************************
#define	WIN_COMBO_BOX_SELECT_MENU	"ui:interface:combo_box_select_menu"
#define	WIN_COMBO_BOX_MEASURE_MENU	"ui:interface:combo_box_measure_menu"

#define	WIN_COMBO_BOX_SELECT_MENU_OUTGAME	"ui:outgame:combo_box_select_menu"

#define	WIN_COMBO_BOX_SELECT_MENU_LOGIN	"ui:login:combo_box_select_menu"
#define	WIN_COMBO_BOX_MEASURE_MENU_LOGIN	"ui:login:combo_box_measure_menu"



// ***************************************************************************
/**
 * Widget: ComboBox of text
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CDBGroupComboBox : public CInterfaceGroup
{
public:

	/// Constructor
	CDBGroupComboBox(const TCtorParam &param);
	~CDBGroupComboBox();

	/// CInterfaceGroup Interface
	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
	virtual void updateCoords ();
	virtual void checkCoords ();

	// Combo Texts
	void			resetTexts();
	void			addText(const ucstring &text);
	void			setText(uint i, const ucstring &text);
	void			insertText(uint i, const ucstring &text);
	const ucstring	&getText(uint i) const;
	const ucstring	&getTexture(uint i) const;
	void			removeText(uint i);
	uint			getNumTexts() const {return (uint)_Texts.size();}

	// selection
	void			setSelection(sint32 val);
	void			setSelectionNoTrigger(sint32 val);
	sint32			getSelection() const;

	// selection number
	void			setSelectionNb(sint32 /* val */){}
	sint32			getSelectionNb() const {return (sint32)_Texts.size();}

	// selection text
	void			setSelectionText(const std::string & val);
	std::string		getSelectionText() const;

	// view text
	void			setViewText(const ucstring & text);
	ucstring		getViewText() const;

	void			setTexture(uint i, const ucstring &texture);

	sint32	evalContentWidth() const;


	int luaAddText(CLuaState &ls);
	int luaRemoveSelection(CLuaState &ls);
	int luaRemoveText(CLuaState &ls);
	int luaRemoveTextByIndex(CLuaState &ls);
	int	luaResetTexts(CLuaState &ls);
	int luaSetText(CLuaState &ls);
	int luaInsertText(CLuaState &ls);
	int luaGetText(CLuaState &ls);
	int luaGetNumTexts(CLuaState &ls);
	int luaSetTexture(CLuaState &ls);


	REFLECT_EXPORT_START(CDBGroupComboBox, CInterfaceGroup)
		REFLECT_SINT32("selection", getSelection, setSelection)
		REFLECT_LUA_METHOD("addText", luaAddText)
		REFLECT_LUA_METHOD("setText", luaSetText)
		REFLECT_LUA_METHOD("insertText", luaInsertText)
		REFLECT_LUA_METHOD("setTexture", luaSetTexture)
		REFLECT_LUA_METHOD("getText", luaGetText)
		REFLECT_LUA_METHOD("getNumTexts", luaGetNumTexts)
		REFLECT_LUA_METHOD("removeSelection", luaRemoveSelection)
		REFLECT_LUA_METHOD("removeText", luaRemoveText)
		REFLECT_LUA_METHOD("removeTextByIndex", luaRemoveTextByIndex)
		REFLECT_LUA_METHOD("resetTexts", luaResetTexts)
		REFLECT_SINT32 ("selectionNb", getSelectionNb, setSelectionNb)
		REFLECT_STRING ("selection_text", getSelectionText, setSelectionText)
		REFLECT_UCSTRING ("view_text", getViewText, setViewText)
	REFLECT_EXPORT_END


protected:
	friend class CHandlerComboBoxSelectStart;

	bool					_LinkedToDB; // if not linked to db, then _NotLinkedToDBSelection is used instead
	bool					_Setuped;
	bool					_DirtySelection;
	sint32					_CacheSelection;

	// sint32
	CInterfaceProperty		_Selection;
	sint32					_NotLinkedToDBSelection;
	std::vector<ucstring>	_Texts;
	std::vector<ucstring>	_Textures;

	// Action Handler called on combo click
	std::string				_AHOnSelectStart;

	// Action handler called when the content is changed
	std::string				_AHOnChange;
	std::string				_AHOnChangeParams;
	bool					_CallingOnChangeActionHandler; // avoid infinite loop here


	// Children
	CViewText				*_ViewText;
	CCtrlBaseButton			*_SelectButton;

	bool					_IsExternViewText;
	ucstring				_ExternViewText;


private:

	void setup();
	void dirt();
public:
	// private : fill a menu with current content
	void fillMenu(CGroupMenu *groupMenu) const;
};


#endif // NL_DBGROUP_COMBO_BOX_H

/* End of dbgroup_combo_box.h */
