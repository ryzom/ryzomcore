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


#include "stdpch.h"
#include "nel/gui/dbgroup_combo_box.h"
#include "nel/gui/group_menu.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/lua_ihm.h"
#include "nel/gui/widget_manager.h"
#include "nel/misc/i18n.h"

using namespace std;
using namespace NLMISC;

namespace NLGUI
{
	NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupComboBox, std::string, "combo_box");

	void force_link_dbgroup_combo_box_cpp() { }

	// Compare strings
	static inline bool lt_text(const std::pair<int,ucstring> &s1, const std::pair<int,ucstring> &s2)
	{
		return toLower(s1.second) < toLower(s2.second);
	}

	std::string CDBGroupComboBox::measureMenu;
	std::string CDBGroupComboBox::selectMenu;
	std::string CDBGroupComboBox::selectMenuOut;

	// ***************************************************************************
	CDBGroupComboBox::CDBGroupComboBox(const TCtorParam &param)
	:	CInterfaceGroup(param)
	{
		_Setuped= false;
		_ViewText= NULL;
		_SelectButton= NULL;
		_LinkedToDB = true;
		_NotLinkedToDBSelection = 0;
		_CallingOnChangeActionHandler = false;
		_IsExternViewText = false;
		dirt();
	}

	// ***************************************************************************
	CDBGroupComboBox::~CDBGroupComboBox()
	{
	}

	// ***************************************************************************
	sint32 CDBGroupComboBox::evalContentWidth() const
	{
		// get the menu to open.
		CGroupMenu	*groupMenu= dynamic_cast<CGroupMenu*>(CWidgetManager::getInstance()->getElementFromId( CDBGroupComboBox::measureMenu ));
		if( !groupMenu )
		{
			return 0;
		}
		fillMenu(groupMenu);
		groupMenu->setActive(true);
		groupMenu->updateCoords();
		sint32 width = groupMenu->getWReal();
		groupMenu->setActive(false);
		groupMenu->reset();
		// add width for the selection arrow
		return width;
	}

	std::string CDBGroupComboBox::getProperty( const std::string &name ) const
	{
		if( name == "linked_to_db" )
		{
			return toString( _LinkedToDB );
		}
		else
		if( name == "value" )
		{
			if( _Selection.getNodePtr() != NULL )
				return _Selection.getNodePtr()->getFullName();
			else
				return "";
		}
		else
			return CInterfaceGroup::getProperty( name );
	}

	void CDBGroupComboBox::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "linked_to_db" )
		{
			bool b;
			if( fromString( value, b ) )
				_LinkedToDB = b;
			return;
		}
		else
		if( name == "value" )
		{
			_Selection.link( value.c_str() );
			return;
		}
		else
			CInterfaceGroup::setProperty( name, value );
	}

	xmlNodePtr CDBGroupComboBox::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CInterfaceGroup::serialize( parentNode, type );
		if( node == NULL )
			return NULL;
		
		xmlSetProp( node, BAD_CAST "type", BAD_CAST "combo_box" );
		xmlSetProp( node, BAD_CAST "linked_to_db", BAD_CAST toString( _LinkedToDB ).c_str() );

		if( _Selection.getNodePtr() != NULL )
			xmlSetProp( node, BAD_CAST "value", BAD_CAST _Selection.getNodePtr()->getFullName().c_str() );
		else
			xmlSetProp( node, BAD_CAST "value", BAD_CAST "" );


		return node;
	}

	// ***************************************************************************
	bool	CDBGroupComboBox::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
	{
		if( !CInterfaceGroup::parse(cur, parentGroup) )
			return false;

		CXMLAutoPtr	prop((const char*)xmlGetProp(cur, (xmlChar*)"linked_to_db"));
		if (prop)
		{
			_LinkedToDB = convertBool(prop);
		}

		if (_LinkedToDB)
		{
			// read the selection value
			CXMLAutoPtr	prop((const char*)xmlGetProp(cur, (xmlChar*)"value"));
			if(!prop)
			{
				nlwarning("'value' not found in %s", _Id.c_str());
				return false;
			}
			else
			{
				_Selection.link((const char*)prop);
			}
		}

		// read the sons text
		xmlNodePtr child;
		child = cur->children;
		while (child)
		{
			if (stricmp((char*)child->name,"combo_text") == 0)
			{
				CXMLAutoPtr name((const char*) xmlGetProp (child, (xmlChar*)"name"));
				if (name)
				{
					const char *propPtr = name;
					ucstring Text = ucstring::makeFromUtf8(propPtr);
					if ((strlen(propPtr)>2) && (propPtr[0] == 'u') && (propPtr[1] == 'i'))
						Text = CI18N::get (propPtr);
					addText(Text);
				}
			}
			child = child->next;
		}

		// optional ActionHandler on click
		prop= xmlGetProp(cur, (xmlChar*)"on_select_start");
		if(prop)	_AHOnSelectStart= (const char*)prop;

		// optional ActionHandler on change
		prop= xmlGetProp(cur, (xmlChar*)"on_change");
		if(prop)	_AHOnChange= (const char*)prop;

		prop= xmlGetProp(cur, (xmlChar*)"on_change_params");
		if(prop)	_AHOnChangeParams= (const char*)prop;


		return true;
	}

	// ***************************************************************************
	void	CDBGroupComboBox::checkCoords ()
	{
		if(!_ViewText)
			return;
		bool mustUpdate = false;
		if (_LinkedToDB)
		{
			// if some change in texts/selection
			if(_DirtySelection || _CacheSelection!=_Selection.getSInt32())
			{
				_CacheSelection= _Selection.getSInt32();
				mustUpdate = true;
			}
		}
		else
		{
			// if some change in texts/selection
			if(_DirtySelection || _CacheSelection != _NotLinkedToDBSelection)
			{
				_CacheSelection= _NotLinkedToDBSelection;
				mustUpdate = true;
			}
		}
		if (mustUpdate)
		{
			// change selected text
			if(_CacheSelection<0 || _CacheSelection>=(sint32)_Texts.size() )
			{
				_ViewText->setText(ucstring());
			}
			else if(_IsExternViewText)
			{
				_ViewText->setText(_ExternViewText);
			}
			else
			{
				_ViewText->setText(_Texts[_CacheSelection].second);
			}
		}
	}

	// ***************************************************************************
	void	CDBGroupComboBox::updateCoords ()
	{
		if(!_Setuped)
			setup();
		CInterfaceGroup::updateCoords();
	}


	// ***************************************************************************
	void	CDBGroupComboBox::dirt()
	{
		_DirtySelection= true;
	}

	// ***************************************************************************
	void	CDBGroupComboBox::resetTexts()
	{
		dirt();
		_Texts.clear();
		_Textures.clear();
	}

	// ***************************************************************************
	void	CDBGroupComboBox::addText(const ucstring &text)
	{
		dirt();
		_Texts.push_back(make_pair((uint)_Texts.size(), text));
		_Textures.push_back(std::string());
	}

	// ***************************************************************************
	void	CDBGroupComboBox::setText(uint i, const ucstring &text)
	{
		dirt();
		if(i<_Texts.size())
			_Texts[i].second= text;
	}

	// ***************************************************************************
	void	CDBGroupComboBox::insertText(uint i, const ucstring &text)
	{
		dirt();
		if(i<_Texts.size())
		{
			addText(_Texts[_Texts.size()-1].second);

			for(uint t=i; t<_Texts.size()-1; t++)
			{
				_Texts[t+1] = _Texts[t];
				_Textures[t+1] = _Textures[t];
			}
			_Texts[i] = make_pair(i, text);
			_Textures[i] = std::string();
		}
		else if(i==_Texts.size())
			addText(text);
	}

	// ***************************************************************************
	void	CDBGroupComboBox::setTexture(uint i, const ucstring &texture)
	{
		dirt();
		if(i<_Textures.size())
			_Textures[i]= texture;
	}

	// ***************************************************************************
	void	CDBGroupComboBox::removeText(uint nPos)
	{
		dirt();
		if(nPos<_Texts.size())
		{
			_Texts.erase( _Texts.begin()+nPos );
			_Textures.erase( _Textures.begin()+nPos );
		}
	}

	// ***************************************************************************
	const ucstring	&CDBGroupComboBox::getText(uint i) const
	{
		static	ucstring	null;
		if(i<_Texts.size())
			return _Texts[i].second;
		else
			return null;
	}

	// ***************************************************************************
	uint CDBGroupComboBox::getTextId(uint i) const
	{
		static	uint	null = 0;
		if(i<_Texts.size())
			return _Texts[i].first;
		else
			return null;
	}
	
	// ***************************************************************************
	uint	CDBGroupComboBox::getTextPos(uint nId) const
	{
		for(uint i=0; i<_Texts.size(); i++)
		{
			if(nId == _Texts[i].first) {return i;}
		}
		return 0;
	}

	// ***************************************************************************
	void	CDBGroupComboBox::sortText()
	{
		sort(_Texts.begin(), _Texts.end(), lt_text);
	}
	
	// ***************************************************************************
	const ucstring	&CDBGroupComboBox::getTexture(uint i) const
	{
		static	ucstring	null;
		if(i<_Textures.size())
			return _Textures[i];
		else
			return null;
	}


	// ***************************************************************************
	void		CDBGroupComboBox::setSelection(sint32 val)
	{
		_IsExternViewText = false;

		if (_LinkedToDB)
		{
			_Selection.setSInt32(val);
		}
		else
		{
			_NotLinkedToDBSelection = val;
		}
		if (!_AHOnChange.empty())
		{
			if (!_CallingOnChangeActionHandler)
			{
				_CallingOnChangeActionHandler = true;
				CAHManager::getInstance()->runActionHandler (_AHOnChange, this, _AHOnChangeParams);
				_CallingOnChangeActionHandler = false;
			}
		}
	}

	// ***************************************************************************
	void CDBGroupComboBox::setSelectionNoTrigger(sint32 val)
	{
		if (_LinkedToDB)
		{
			_Selection.setSInt32(val);
		}
		else
		{
			_NotLinkedToDBSelection = val;
		}
	}

	// ***************************************************************************
	sint32		CDBGroupComboBox::getSelection() const
	{
		if (_LinkedToDB)
		{
			return _Selection.getSInt32();
		}
		else
		{
			return _NotLinkedToDBSelection;
		}
	}

	// ***************************************************************************
	void		CDBGroupComboBox::setSelectionText(const std::string & val)
	{
		sint32 value;
		for(uint i=0; i<getNumTexts(); i++)
		{
			std::string sText;
			getText(i).toString(sText);
			if(sText == val)
			{
				value = i;
				setSelection(value);
				break;
			}
		}
	}

	// ***************************************************************************
	void CDBGroupComboBox::setViewText(const ucstring & text)
	{
		_IsExternViewText = true;
		_ExternViewText = ucstring(text);
		_ViewText->setText(_ExternViewText);
	}

	// ***************************************************************************
	ucstring CDBGroupComboBox::getViewText() const
	{
		return  _ViewText->getText();
	}

	// ***************************************************************************
	std::string CDBGroupComboBox::getSelectionText() const
	{
		ucstring text;
		if (_LinkedToDB)
		{
			text = getText(_Selection.getSInt32());
		}
		else
		{
			text = getText(_NotLinkedToDBSelection);
		}
		std::string texteS;
		text.toString(texteS);
		return texteS;
	}

	// ***************************************************************************
	void	CDBGroupComboBox::setup()
	{
		_Setuped= true;

		// get the viewText
		_ViewText= dynamic_cast<CViewText*>(CInterfaceGroup::getView("text"));
		// get the button
		_SelectButton= dynamic_cast<CCtrlBaseButton*>(CInterfaceGroup::getCtrl("select"));

		// force the action handler
		if( _SelectButton )
			_SelectButton->setActionOnLeftClick("combo_box_select_start");
	}



	// ***************************************************************************
	int CDBGroupComboBox::luaRemoveSelection(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "removeSelection", 1);
		sint32 value;
		if(CLuaIHM::popSINT32(ls, value))
		{
			removeText(value);
		}
		return 0;
	}

	// ***************************************************************************
	int CDBGroupComboBox::luaRemoveText(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "removeText", 1);
		CLuaIHM::checkArgType(ls, "removeText", 1, LUA_TSTRING);
		std::string text = ls.toString(1);
		sint32 value;
		for(uint i=0; i<getNumTexts(); i++)
		{
			std::string sText;
			getText(i).toString(sText);
			if(sText == text)
			{
				value = i;
				removeText(value);
				break;
			}
		}
		return 0;
	}

	// ***************************************************************************
	int CDBGroupComboBox::luaResetTexts(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "resetTexts", 0);
		resetTexts();
		return 0;
	}

	// ***************************************************************************
	int CDBGroupComboBox::luaAddText(CLuaState &ls)
	{
		const char *funcName = "addText";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgTypeUCString(ls, funcName, 1);
		ucstring text;
		nlverify(CLuaIHM::pop(ls, text));
		addText(text);
		return 0;
	}

	// ***************************************************************************
	int CDBGroupComboBox::luaSetText(CLuaState &ls)
	{
		const char *funcName = "setText";
		CLuaIHM::checkArgCount(ls, funcName, 2);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		CLuaIHM::checkArgTypeUCString(ls, funcName, 2);
		ucstring text;
		nlverify(CLuaIHM::pop(ls, text));
		setText((uint) ls.toNumber(1), text);
		return 0;
	}

	// ***************************************************************************
	int CDBGroupComboBox::luaInsertText(CLuaState &ls)
	{
		const char *funcName = "insertText";
		CLuaIHM::checkArgCount(ls, funcName, 2);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		CLuaIHM::checkArgTypeUCString(ls, funcName, 2);
		ucstring text;
		nlverify(CLuaIHM::pop(ls, text));
		insertText((uint) ls.toNumber(1), text);
		return 0;
	}

	// ***************************************************************************
	int CDBGroupComboBox::luaSetTexture(CLuaState &ls)
	{
		const char *funcName = "setTexture";
		CLuaIHM::checkArgCount(ls, funcName, 2);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		CLuaIHM::checkArgTypeUCString(ls, funcName, 2);
		ucstring texture;
		nlverify(CLuaIHM::pop(ls, texture));
		setTexture((uint) ls.toNumber(1), texture);
		return 0;
	}

	// ***************************************************************************
	int CDBGroupComboBox::luaGetText(CLuaState &ls)
	{
		const char *funcName = "setText";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		CLuaIHM::push(ls, getText((uint) ls.toNumber(1)));
		return 1;
	}

	// ***************************************************************************
	int CDBGroupComboBox::luaRemoveTextByIndex(CLuaState &ls)
	{
		const char *funcName = "removeText";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		removeText((uint) ls.toNumber(1));
		return 0;
	}

	// ***************************************************************************
	int CDBGroupComboBox::luaGetNumTexts(CLuaState &ls)
	{
		const char *funcName = "getNumTexts";
		CLuaIHM::checkArgCount(ls, funcName, 0);
		ls.push((double) getNumTexts());
		return 1;
	}

	// ***************************************************************************
	void CDBGroupComboBox::fillMenu(CGroupMenu *groupMenu) const
	{
		nlassert(groupMenu);

			// Setup the menu with combo action.
			groupMenu->reset();
			for(uint i=0; i<getNumTexts(); i++)
			{
				// set the id as the parameter
				bool checkable = false;
				if(getTexture(i).toString() != std::string())
				{
					checkable = true;
				}
				groupMenu->addLine(getText(i), "combo_box_select_end", toString(i),
					"", std::string(), getTexture(i).toString(), checkable);
			}


	}



	// ***************************************************************************
	class	CHandlerComboBoxSelectStart : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
		{
			CDBGroupComboBox *pCB = dynamic_cast<CDBGroupComboBox*>(pCaller->getParent());
			if (pCB == NULL) return;
			// if no choice, return.
			if( pCB->getNumTexts()==0 )
				return;

			// get the menu to open.
			CGroupMenu	*groupMenu= dynamic_cast<CGroupMenu*>(CWidgetManager::getInstance()->getElementFromId( CDBGroupComboBox::selectMenu ));
			if( !groupMenu )
			{
				groupMenu= dynamic_cast<CGroupMenu*>(CWidgetManager::getInstance()->getElementFromId( CDBGroupComboBox::selectMenuOut ));
			}
			if( !groupMenu )
				return;

			pCB->fillMenu(groupMenu);

			groupMenu->setMaxVisibleLine(8);

			// pos and size wisely the menu.
			groupMenu->setMinW(pCB->getWReal());
			groupMenu->setX(pCB->getXReal());
			groupMenu->setBaseX(pCB->getXReal());
			groupMenu->setY(pCB->getYReal());
			groupMenu->setBaseY(pCB->getYReal());

			// Must ensure the combo menu has same windows priority than the combo box window
			CInterfaceGroup		*rootWin= pCB->getRootWindow();
			if(rootWin)
				groupMenu->setPriority(rootWin->getPriority());

			// After menu init, Call user activation method
			if( !pCB->_AHOnSelectStart.empty() )
			{
				CAHManager::getInstance()->runActionHandler(pCB->_AHOnSelectStart, pCB);
			}

			// launch the menu
			// if the combo box is in a modal, must do a push, else just replace
			if(dynamic_cast<CGroupModal*>(pCB->getRootWindow()))
			{
				groupMenu->setCloseSubMenuUsingPopModal(true);
				CWidgetManager::getInstance()->pushModalWindow(pCB, groupMenu);
			}
			else
			{
				groupMenu->setCloseSubMenuUsingPopModal(false);
				CWidgetManager::getInstance()->enableModalWindow (pCB, groupMenu);
			}
		}
	};
	REGISTER_ACTION_HANDLER (CHandlerComboBoxSelectStart, "combo_box_select_start");




	// ***************************************************************************
	class	CHandlerComboBoxSelectEnd : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase * /* pCaller */, const std::string &Params)
		{
			CDBGroupComboBox *pCB = dynamic_cast<CDBGroupComboBox*>(CWidgetManager::getInstance()->getCtrlLaunchingModal());
			if (pCB == NULL) return;

			// set the selection
			sint32 selection;
			fromString(Params, selection);
			pCB->setSelection(selection);
		}
	};
	REGISTER_ACTION_HANDLER (CHandlerComboBoxSelectEnd, "combo_box_select_end");

}


