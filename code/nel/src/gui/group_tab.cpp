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
#include "nel/gui/group_tab.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/lua_ihm.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/view_text.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

NLMISC_REGISTER_OBJECT(CViewBase, CGroupTab, std::string, "tab");

namespace NLGUI
{

	// ***************************************************************************
	CGroupTab::CGroupTab(const TCtorParam &param)
	 : CInterfaceGroup(param)
	{
		_Selection= -1;
		_NextSelection = -1;
		_BaseRenderLayer= 0;
		_Setuped= false;
		_HideOutTabs = false;
		_FirstTabIndex = -1;
		_LastTabIndex = -1;
	}

	std::string CGroupTab::getProperty( const std::string &name ) const
	{
		if( name == "hide_out_tabs" )
		{
			return toString( _HideOutTabs );
		}
		else
		if( name == "onchange" )
		{
			return _AHOnChange;
		}
		else
		if( name == "onchange_params" )
		{
			return _ParamsOnChange;
		}
		else
			return CInterfaceGroup::getProperty( name );
	}


	void CGroupTab::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "hide_out_tabs" )
		{
			bool b;
			if( fromString( value, b ) )
				_HideOutTabs = b;
			return;
		}
		else
		if( name == "onchange" )
		{
			_AHOnChange = value;
			return;
		}
		else
		if( name == "onchange_params" )
		{
			_ParamsOnChange = value;
			return;
		}
		else
			CInterfaceGroup::setProperty( name, value );
	}

	xmlNodePtr CGroupTab::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CInterfaceGroup::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "tab" );
		xmlSetProp( node, BAD_CAST "hide_out_tabs", BAD_CAST toString( _HideOutTabs ).c_str() );
		xmlSetProp( node, BAD_CAST "onchange", BAD_CAST _AHOnChange.c_str() );
		xmlSetProp( node, BAD_CAST "onchange_params", BAD_CAST _ParamsOnChange.c_str() );

		return node;
	}

	// ***************************************************************************
	bool	CGroupTab::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
	{
		if( !CInterfaceGroup::parse(cur, parentGroup) )
			return false;

		CXMLAutoPtr	prop((const char*)xmlGetProp(cur, (xmlChar*)"hide_out_tabs"));
		if (prop)
		{
			_HideOutTabs = convertBool(prop);
		}

		prop = (char*) xmlGetProp( cur, (xmlChar*)"onchange" );
		if (prop) _AHOnChange = (const char *) prop;
		prop = (char*) xmlGetProp( cur, (xmlChar*)"onchange_params" );
		if (prop) _ParamsOnChange = (const char *) prop;

		return true;
	}

	// ***************************************************************************
	void	CGroupTab::setup()
	{
		if(_Setuped)
			return;
		_Setuped= true;

		_Buttons.clear();
		_Groups.clear();

		/* Buttons must be named tab0,tab1,tab2...
			and tab_array0_0, tab_array0_1 .... (for vector of tab)
			Only 10 tab array are allowed
		*/
		for(sint tabArrayIndex= -1;tabArrayIndex<10;tabArrayIndex++)
		{
			// prefix according to array or not
			string	prefix;
			if(tabArrayIndex==-1)
				prefix= "tab";
			else
				prefix= toString("tab_array%d_", tabArrayIndex);

			// for all tab of this type (standard tab or array of tab), find the Buttons and groups.
			uint	tabIndex=0;
			for(;;)
			{
				// find the ctrl named "tab0"
				CCtrlTabButton	*but= dynamic_cast<CCtrlTabButton*>(getCtrl(toString("%s%d", prefix.c_str(), tabIndex)));
				if(!but)
					break;

				// find the associated group
				CInterfaceGroup	*pGroup = NULL;
				CInterfaceGroup	*pFather = this;

				while ((pGroup == NULL) && (pFather != NULL))
				{
					pGroup = pFather->getGroup(but->_AssociatedGroup);
					pFather = pFather->getParent();
				}

				// add to the button and group list
				_Buttons.push_back(but);
				_Groups.push_back(pGroup);

				// try next
				tabIndex++;
			}
		}

		// at the first setup, select by default the 1st
		if(_Selection<0)
			select(0);
	}

	// ***************************************************************************
	void	CGroupTab::addTab(CCtrlTabButton * tabB)
	{
		addCtrl(tabB);
		_Setuped = false;
		updateCoords();
		selectFromCtrl(tabB);

		if(_HideOutTabs && !_AHOnChange.empty())
			CAHManager::getInstance()->runActionHandler(_AHOnChange, this, _ParamsOnChange);

	}

	// ***************************************************************************
	void	CGroupTab::addTab(CCtrlTabButton * tabB, sint index)
	{
		if(index<(sint)_Buttons.size() && index>=0)
		{
			vector<CCtrlTabButton*> buttons = _Buttons;

			for(sint i=0;i<(sint)_Buttons.size();i++)
				delCtrl(_Buttons[i], true);

			_Setuped = false;
			updateCoords();

			uint count=0;
			CCtrlTabButton* lastTab=NULL;
			for(sint i=0;i<(sint)buttons.size();i++)
			{
				if(i==index)
				{
					tabB->setId("tab" + NLMISC::toString(count));
					tabB->setParentPos(lastTab);
					if(i==0)
						tabB->setParentPosRef(Hotspot_TL);
					else
						tabB->setParentPosRef(Hotspot_TR);
					tabB->setPosRef(Hotspot_TL);

					addCtrl(tabB);
					lastTab = tabB;
					count++;
				}

				buttons[i]->setId("tab" + NLMISC::toString(count));
				buttons[i]->setParentPos(lastTab);
				if(i==0 && index!=0)
					buttons[i]->setParentPosRef(Hotspot_TL);
				else
					buttons[i]->setParentPosRef(Hotspot_TR);
				buttons[i]->setPosRef(Hotspot_TL);

				addCtrl(buttons[i]);

				lastTab = buttons[i];
				count++;
			}

			_Setuped = false;
			updateCoords();

			// we have added a new button in first position
			// then it must recover the reference
			if(index==0)
			{
				CCtrlTabButton * tab0 = _Buttons[0];
				for(uint i=0; i<_Buttons.size(); ++i)
					_Buttons[i]->initRBRefFromRadioButton(tab0);

				select(_Selection);
			}
			else
			{
				CCtrlTabButton * tab0 = _Buttons[0];
				_Buttons[index]->initRBRefFromRadioButton(tab0);
			}
		}
		else
		{
			tabB->setId(string("tab") + NLMISC::toString(_Buttons.size()));

			if(_Buttons.empty())
			{
				tabB->setParentPos(NULL);
				tabB->setParentPosRef(Hotspot_TL);
			}
			else
			{
				tabB->setParentPos(_Buttons[_Buttons.size()-1]);
				tabB->setParentPosRef(Hotspot_TR);
			}
			tabB->setPosRef(Hotspot_TL);

			addCtrl(tabB);
		}

		_Setuped = false;
		updateCoords();

		if(_HideOutTabs && !_AHOnChange.empty())
			CAHManager::getInstance()->runActionHandler(_AHOnChange, this, _ParamsOnChange);

	}

	// ***************************************************************************
	int CGroupTab::luaAddTab(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "CGroupTab::addTab", 1);
		CCtrlTabButton *tabB = dynamic_cast<CCtrlTabButton *>(CLuaIHM::getUIOnStack(ls, 1));
		if (tabB)
		{
			// don't use addTab to avoid selection of new tab
			addCtrl(tabB);

			_Setuped = false;
			updateCoords();

			if(_HideOutTabs && !_AHOnChange.empty())
				CAHManager::getInstance()->runActionHandler(_AHOnChange, this, _ParamsOnChange);

		}
		return 0;
	}

	// ***************************************************************************
	int CGroupTab::luaAddTabWithOrder(CLuaState &ls)
	{
		const char *funcName = "addTabWithOrder";
		CLuaIHM::checkArgCount(ls, funcName, 2);
		CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);

		CCtrlTabButton *tabB = dynamic_cast<CCtrlTabButton *>(CLuaIHM::getUIOnStack(ls, 1));
		if (tabB)
		{
			// don't use addTab to avoid selection of new tab
			addTab(tabB, (sint) ls.toInteger(2));
		}
		return 0;
	}

	// ***************************************************************************
	void	CGroupTab::removeTab(sint index)
	{
		if(!(index>=0 && index<(sint)_Buttons.size()))
			return;

		vector<CCtrlTabButton*> buttons = _Buttons;

		for(sint i=0;i<(sint)_Buttons.size();i++)
		{
			bool deleteElt = (i!=index);
			CViewText* tabVT = _Buttons[i]->getViewText();
			if(tabVT && !deleteElt)
				delView(tabVT, deleteElt);

			delCtrl(_Buttons[i], deleteElt);

			if(!deleteElt)
				(_Groups[i]->getParent())->delGroup(_Groups[i], deleteElt);
		}
		_Setuped = false;
		updateCoords();

		uint count=0;
		CCtrlTabButton* lastTab = NULL;
		for(sint i=0;i<(sint)buttons.size();i++)
		{
			if(i!=index)
			{
				buttons[i]->setId("tab"+NLMISC::toString(count));
				buttons[i]->setParentPos(lastTab);
				if((i==0) || (index==0 && i==1))
					buttons[i]->setParentPosRef(Hotspot_TL);
				else
					_Buttons[i]->setParentPosRef(Hotspot_TR);

				buttons[i]->setPosRef(Hotspot_TL);

				lastTab = buttons[i];
				addCtrl(buttons[i]);
				count++;
			}
		}

		_Setuped = false;
		updateCoords();

		// we have removed the first button which is the only one to own the reference
		// then the new first button recovers the reference
		if(index==0)
		{
			CCtrlTabButton * tab0 = _Buttons[0];
			for(uint i=0; i<_Buttons.size(); ++i)
				_Buttons[i]->initRBRefFromRadioButton(tab0);

			select(_Selection);
		}

		if(_HideOutTabs)
		{
			select(_FirstTabIndex);

			if(!_AHOnChange.empty())
				CAHManager::getInstance()->runActionHandler(_AHOnChange, this, _ParamsOnChange);
		}
	}

	// ***************************************************************************

	int CGroupTab::luaRemoveTab(CLuaState &ls)
	{
		const char *funcName = "removeTab";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		removeTab((uint) ls.toInteger(1));
		return 0;
	}

	// ***************************************************************************
	void	CGroupTab::removeAll()
	{
		for(sint i=0;i<(sint)_Buttons.size();i++)
		{
			CViewText* tabVT = _Buttons[i]->getViewText();
			if(tabVT)
				delView(tabVT, false);

			delCtrl(_Buttons[i], false);
			(_Groups[i]->getParent())->delGroup(_Groups[i], false);
		}

		_Setuped = false;
		updateCoords();
	}

	// ***************************************************************************

	int CGroupTab::luaRemoveAll(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "CGroupTab::removeAll", 0);
		removeAll();
		return 0;
	}

	// ***************************************************************************
	CCtrlTabButton*	CGroupTab::getTabButton(sint index)
	{
		if(index>=0 && index<(sint)_Buttons.size())
		{
			return _Buttons[index];
		}
		return NULL;
	}

	// ***************************************************************************
	int	CGroupTab::luaGetTabButton(CLuaState &ls)
	{
		const char *funcName = "getTabButton";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		CCtrlTabButton* tab = getTabButton((uint) ls.toInteger(1));
		if(tab != NULL)
		{
			CLuaIHM::pushUIOnStack(ls, tab);
			return 1;
		}
		return 0;
	}

	// ***************************************************************************
	void CGroupTab::updateFirstTabButton()
	{
		if(!_HideOutTabs || (_Selection<0) || _Buttons.empty() || (_Parent->getWReal()<0)
			|| _FirstTabIndex>=(sint)_Buttons.size())
			return;

		sint oldFirstTabIndex = _FirstTabIndex;
		sint oldLastTabIndex = _LastTabIndex;

		if(_FirstTabIndex<0)
		{
			for(uint i=0; i<_Buttons.size(); i++)
			{
				CCtrlTabButton * tab = _Buttons[i];
				if(tab->getActive())
				{
					_FirstTabIndex = i;
					break;
				}
			}
		}

		sint selection = _Selection;
		if(selection>=(sint)_Buttons.size())
			selection = _FirstTabIndex;

		if(selection < _FirstTabIndex)
			_FirstTabIndex = selection;

		sint32 maxWidth = _Parent->getWReal();
		sint32 buttonsWidth = 0;
		_LastTabIndex = 0;

		// desactive first tabs
		for(uint i=0; i<(uint)_FirstTabIndex; i++)
		{
			CCtrlTabButton * tab = _Buttons[i];
			if(tab->getActive())
				tab->setActive(false);
		}


		// active tabs from _FirstTabIndex and search for last showed tab
		for(uint i=_FirstTabIndex; i<_Buttons.size(); i++)
		{
			CCtrlTabButton * tab = _Buttons[i];
			sint32 tabWidth = tab->getWMax();
			if(buttonsWidth+tabWidth <= maxWidth)
			{
				buttonsWidth += tabWidth;
				if(!tab->getActive())
					tab->setActive(true);
				_LastTabIndex = i;
			}
			else
				break;
		}

		// check if selected tab is in showed tabs
		if(_LastTabIndex < selection)
		{
			for(uint i=_LastTabIndex+1; i<=(uint)selection; i++)
			{
				CCtrlTabButton * tab = _Buttons[i];
				buttonsWidth += tab->getWMax();
				if(!tab->getActive())
					tab->setActive(true);
			}

			while(buttonsWidth>maxWidth)
			{
				CCtrlTabButton * tab = _Buttons[_FirstTabIndex];
				buttonsWidth -= tab->getWMax();
				_FirstTabIndex++;
				if(tab->getActive())
					tab->setActive(false);
			}
		}

		// add tabs before the "_FirstTabIndex" one if it remains place
		while(buttonsWidth<maxWidth && _FirstTabIndex>0)
		{
			CCtrlTabButton * tab = _Buttons[_FirstTabIndex-1];
			buttonsWidth += tab->getWMax();
			if(buttonsWidth<=maxWidth)
			{
				_FirstTabIndex--;
				if(!tab->getActive())
						tab->setActive(true);
			}
		}

		// desactive last tabs
		for(uint i=_LastTabIndex+1; i<_Buttons.size(); i++)
		{
			CCtrlTabButton * tab = _Buttons[i];
			if(tab->getActive())
				tab->setActive(false);
		}

		if(!_AHOnChange.empty() && ((oldFirstTabIndex!=_FirstTabIndex) || (oldLastTabIndex!=_LastTabIndex)))
			CAHManager::getInstance()->runActionHandler(_AHOnChange, this, _ParamsOnChange);

	}

	// ***************************************************************************
	int	CGroupTab::luaShowTabButton(CLuaState &ls)
	{
		const char *funcName = "showTabButton";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		sint showTab = (sint)ls.toInteger(1);

		if(showTab>=0 && showTab<(sint)_Buttons.size())
		{
			sint32 maxWidth = _Parent->getWReal();
			sint32 buttonsWidth = 0;

			if(showTab<_FirstTabIndex)
			{
				_FirstTabIndex = showTab;
				sint lastTabIndex = _FirstTabIndex;
				for(uint i=_FirstTabIndex; i<_Buttons.size(); i++)
				{
					CCtrlTabButton * tab = _Buttons[i];
					sint32 tabWidth = tab->getWMax();
					if(buttonsWidth+tabWidth <= maxWidth)
					{
						buttonsWidth += tabWidth;
						if(!tab->getActive())
							tab->setActive(true);
						lastTabIndex = i;
					}
					else
						break;
				}

				if(lastTabIndex <_Selection)
					select(lastTabIndex);
				else
					updateFirstTabButton();
			}
			else if(showTab>_LastTabIndex)
			{
				for(uint i=_FirstTabIndex; i<=(uint)showTab; i++)
					buttonsWidth += _Buttons[i]->getWMax();

				while(buttonsWidth>maxWidth)
				{
					buttonsWidth -= _Buttons[_FirstTabIndex]->getWMax();
					_FirstTabIndex++;
				}

				if(_Selection<_FirstTabIndex)
					select(_FirstTabIndex);
				else
					updateFirstTabButton();
			}
		}

		return 0;
	}

	// ***************************************************************************
	void	CGroupTab::updateCoords ()
	{
		if(!_Setuped)
			setup();

		// special for groupTab. Because the ctrl may overlap each other from left to right, they are inserted in reverse
		// order. BUT, for correct TR/TL coord handling, must updtae in the reverse sens too!

		// **** just basis
		CInterfaceGroup::doUpdateCoords();

		// **** update in reverse order
		_XReal += _OffsetX;
		_YReal += _OffsetY;
		vector<CViewBase*>::reverse_iterator ite;
		for (ite = _EltOrder.rbegin() ; ite != _EltOrder.rend(); ite++)
		{
			CViewBase *pIE = *ite;
			pIE->updateCoords();
		}
		_XReal -= _OffsetX;
		_YReal -= _OffsetY;

		// **** complete with child resize
		CInterfaceGroup::updateCoords();

		updateFirstTabButton();
	}

	// ***************************************************************************
	void	CGroupTab::select(sint index)
	{
		if(index<0)
			index= -1;

		if(index<(sint)_Buttons.size())
		{
			sint	i;

			// validate this radio button.
			if(index>=0)
				_Buttons[index]->setPushed(true);
			else
				for(i=0;i<(sint)_Buttons.size();i++)
					_Buttons[i]->setPushed(false);

			_NextSelection = index;

			// set all render layer to their correct state
			for(i=0;i<(sint)_Buttons.size();i++)
			{
				// set the selected one +1, so it will be over
				_Buttons[i]->setRenderLayer(_BaseRenderLayer + (i==index?1:0) );
				if (i==index)
				{
					_Buttons[i]->setBlink(false);
					if (_Buttons[i]->_AHOnLeftClick2 != NULL)
						// call like if press on it
						_Buttons[i]->_AHOnLeftClick2->execute(_Buttons[i], _Buttons[i]->getParamsOnLeftClick());
				}
			}

			// show/hide all the associated groups
			for(i=0;i<(sint)_Groups.size();i++)
			{
				if(_Groups[i])
					_Groups[i]->setActive(i==index);
			}

			// ok!
			_Selection= index;

			updateFirstTabButton();
		}
	}

	// ***************************************************************************
	void	CGroupTab::selectFromCtrl(CCtrlTabButton *button)
	{
		// search in all buttons
		for(uint i=0;i<_Buttons.size();i++)
		{
			// found?
			if(_Buttons[i]==button)
			{
				select(i);
				return;
			}
		}
	}

	// ***************************************************************************
	void	CGroupTab::selectDefault(CCtrlTabButton *ifSelectionIs)
	{
		if(!_HideOutTabs && _Selection>=0 && _Selection<(sint)_Buttons.size() && _Buttons[_Selection]==ifSelectionIs)
		{
			// parse all active button
			for(uint i=0;i<_Buttons.size();i++)
			{
				if(_Buttons[i]->getActive())
				{
					select(i);
					return;
				}
			}

			// default: unselect
			select(-1);
		}
	}

	// ***************************************************************************
	void	CGroupTab::selectDefaultIfCurrentHid()
	{
		if(_Selection>=0 && _Selection<(sint)_Buttons.size() &&
			_Buttons[_Selection]!=NULL && _Buttons[_Selection]->getActive()==false)
		{
			selectDefault(_Buttons[_Selection]);
		}
	}

	// ***************************************************************************
	sint	CGroupTab::getSelection() const
	{
		return _Selection;
	}

	NLMISC_REGISTER_OBJECT(CViewBase, CCtrlTabButton, std::string, "tab_button");

	// ***************************************************************************
	std::string	CGroupTab::getAssociatedGroupSelection() const
	{
		if(_Selection>=0 && _Selection<(sint)_Buttons.size())
		{
			return _Buttons[_Selection]->_AssociatedGroup;
		}
		return "";
	}

	// ***************************************************************************
	CInterfaceGroup*	CGroupTab::getGroup(sint index)
	{
		if(index>=0 && index<(sint)_Groups.size())
		{
			return _Groups[index];
		}
		return NULL;
	}

	// ***************************************************************************
	int	CGroupTab::luaGetGroup(CLuaState &ls)
	{
		const char *funcName = "getGroup";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		CInterfaceGroup* group = getGroup((uint) ls.toInteger(1));
		if(group != NULL)
		{
			CLuaIHM::pushUIOnStack(ls, group);
			return 1;
		}
		return 0;
	}

	// ***************************************************************************
	CCtrlTabButton::CCtrlTabButton(const TCtorParam &param)
	: CCtrlTextButton(param)
	{
		_DefaultX= 0;
		_AHOnLeftClick2 = NULL;
		_BlinkDate = 0;
		_Blinking = false;
		_BlinkState = false;
	}

	std::string CCtrlTabButton::getProperty( const std::string &name ) const
	{
		if( name == "group" )
			return _AssociatedGroup;
		else
			return CCtrlTextButton::getProperty( name );
	}

	void CCtrlTabButton::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "group" )
		{
			_AssociatedGroup = value;
		}
		else
			CCtrlTextButton::setProperty( name, value );
	}

	xmlNodePtr CCtrlTabButton::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CCtrlTextButton::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "tab" );
		xmlNewProp( node, BAD_CAST "group", BAD_CAST _AssociatedGroup.c_str() );

		return node;
	}

	// ***************************************************************************
	bool CCtrlTabButton::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
	{
		if(!CCtrlTextButton::parse(cur, parentGroup))
			return false;

		// if left click not setuped, set default
		_AHOnLeftClick2 = _AHOnLeftClick;
		string	dummy;
		_AHOnLeftClick= CAHManager::getInstance()->getAH("tab_select", dummy);

		// read the associated group to show/hide
		CXMLAutoPtr	prop;
		prop = (char*) xmlGetProp( cur, (xmlChar*)"group" );
		if(prop)	_AssociatedGroup= (const char*)prop;

		// backup the x
		_DefaultX= _X;

		return true;
	}

	// ***************************************************************************
	void CCtrlTabButton::setActive(bool state)
	{
		if(state!=getActive())
		{
			CCtrlTextButton::setActive(state);

			// special for correct display of textbuttons. reset to 0 when the button is hid
			if(state)
				setX(_DefaultX);
			else
				setX(0);

			// if hide, and I was the selected tab, select a default active one
			if(state==false)
			{
				CGroupTab	*parent= dynamic_cast<CGroupTab*>(getParent());
				if(parent)
					parent->selectDefault(this);
			}
		}
	}

	// ***************************************************************************
	bool CCtrlTabButton::handleEvent (const NLGUI::CEventDescriptor &event)
	{
		if (event.getType() == NLGUI::CEventDescriptor::system)
		{
			const NLGUI::CEventDescriptorSystem &systemEvent = (const NLGUI::CEventDescriptorSystem &) event;
			if (systemEvent.getEventTypeExtended() == NLGUI::CEventDescriptorSystem::clocktick)
			if (_Blinking)
			{
				uint dbclickDelay = CWidgetManager::getInstance()->getUserDblClickDelay();
				const CWidgetManager::SInterfaceTimes &times = CWidgetManager::getInstance()->getInterfaceTimes();

				if (( times.thisFrameMs - _BlinkDate) > dbclickDelay)
				{
					if (_BlinkState)
					{
						setTextColorNormal(CRGBA::White);
						setTextModulateGlobalColorNormal(false);
					}
					else
					{
						setTextColorNormal(_TextColorNormalBlink);
						setTextModulateGlobalColorNormal(_TextModulateGlobalColorNormalBlink);
					}
					_BlinkState = !_BlinkState;
					_BlinkDate = times.thisFrameMs;
				}
			}
		}
		return CCtrlTextButton::handleEvent(event);
	}

	// ***************************************************************************
	void CCtrlTabButton::setBlink (bool b)
	{
		if (b)
		{
			if (!_Blinking)
			{
				_TextColorNormalBlink = getTextColorNormal();
				_TextModulateGlobalColorNormalBlink = getTextModulateGlobalColorNormal();
				CWidgetManager::getInstance()->registerClockMsgTarget(this);
			}
			_Blinking = true;
		}
		else
		{
			if (_Blinking)
			{
				CWidgetManager::getInstance()->unregisterClockMsgTarget(this);
				setTextColorNormal(_TextColorNormalBlink);
				setTextModulateGlobalColorNormal(_TextModulateGlobalColorNormalBlink);
			}
			_Blinking = false;
		}
	}

	// ***************************************************************************
	// Action handler for Tab selection
	class CHandlerTabSelect : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase *pCaller, const string &/* Params */)
		{
			CCtrlTabButton	*but= dynamic_cast<CCtrlTabButton*>(pCaller);

			// get the parent TabGroup
			CGroupTab	*parent= dynamic_cast<CGroupTab*>(but->getParent());
			if(parent)
				parent->selectFromCtrl(but);
		}
	};
	REGISTER_ACTION_HANDLER(CHandlerTabSelect, "tab_select" );

}

