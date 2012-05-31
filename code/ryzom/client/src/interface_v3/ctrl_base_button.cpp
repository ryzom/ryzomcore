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

#include "ctrl_base_button.h"
#include "interface_manager.h"
#include "nel/misc/xml_auto_ptr.h"
#include "../time_client.h"

#include "nel/gui/lua_ihm.h"


using namespace std;
using namespace NLMISC;


// ***************************************************************************
static const uint KEY_REPEAT_MIN = 100;
static const uint KEY_REPEAT_MAX = 750;


sint64 CCtrlBaseButton::_LastLeftClickDate = 0;
NLMISC::CRefPtr<CCtrlBaseButton> CCtrlBaseButton::_LastLeftClickButton;


// ***************************************************************************
CCtrlBaseButton::CCtrlBaseButton(const TCtorParam &param) : CCtrlBase(param), _Type(ToggleButton)
{
	_Pushed = _Over = false;
	_Frozen = false;
	_FrozenHalfTone = true;
	_OverWhenPushed = true;
	_ColorOver = _ColorPushed = _ColorNormal = NLMISC::CRGBA(255,255,255,255);
	_ModulateGlobalColorNormal= _ModulateGlobalColorPushed= _ModulateGlobalColorOver= true;
	_LeftLongClickHandled = true;
	_LeftDblClickHandled = false;
	_ClickWhenPushed = false;
	_RBRefBut = NULL;
	_RBRef = NULL;

	_AHOnOver = NULL;
	_AHOnLeftClick = NULL;
	_AHOnRightClick = NULL;
	_AHOnClockTick = NULL;
	_AHOnLeftLongClick = NULL;
	_AHOnLeftDblClick = NULL;
}



// ***************************************************************************
bool CCtrlBaseButton::parse (xmlNodePtr cur,CInterfaceGroup * parentGroup)
{
	CXMLAutoPtr prop;

	//try to get props that can be inherited from groups
	//if a property is not defined, try to find it in the parent group.
	//if it is undefined, set it to zero
	if (! CCtrlBase::parse(cur,parentGroup) )
		return false;

	_Over = false;

	// *** try to get the NEEDED specific props
	prop = xmlGetProp (cur, (xmlChar*)"button_type");
	string type;
	if (prop) type = (const char*) prop;
	if (type.empty() || type == "toggle_button")
	{
		_Type = ToggleButton;
	}
	else if (type == "push_button")
	{
		_Type = PushButton;
	}
	else if (type == "radio_button")
	{
		_Type = RadioButton;

		initRBRef();
		if (_Pushed)
			*_RBRef = this;
	}
	else
	{
		nlinfo(("cannot parse button type for button " + getId()).c_str());
	}

	prop= (char*) xmlGetProp (cur, (xmlChar*)"pushed");
	_Pushed = false;
	if (prop)
		_Pushed = convertBool(prop);

	prop= (char*) xmlGetProp (cur, (xmlChar*)"over_when_pushed");
	_OverWhenPushed = true;
	if (prop)
		_OverWhenPushed = convertBool(prop);

	prop= (char*) xmlGetProp (cur, (xmlChar*)"click_when_pushed");
	_ClickWhenPushed = false;
	if (prop)
		_ClickWhenPushed = convertBool(prop);

	// *** Read Colors
	// get color normal
	prop= (char*) xmlGetProp( cur, (xmlChar*)"color" );
	_ColorNormal = CRGBA(255,255,255,255);
	if (prop)
		_ColorNormal = convertColor (prop);

	// Get ColorPushed
	prop= (char*) xmlGetProp( cur, (xmlChar*)"col_pushed" );
	_ColorPushed = CRGBA(255,255,255,255);
	if (prop)
		_ColorPushed = convertColor(prop);

	// Get ColorOver
	prop= (char*) xmlGetProp( cur, (xmlChar*)"col_over" );
	_ColorOver = CRGBA(255,255,255,255);
	if (prop)
		_ColorOver = convertColor(prop);

	// Default: take "global_color" param interface_element option.
	_ModulateGlobalColorNormal= _ModulateGlobalColorPushed= _ModulateGlobalColorOver= getModulateGlobalColor();

	// Read special global_color for each state
	prop = (char*) xmlGetProp( cur, (xmlChar*)"global_color_normal" );
	if (prop)	_ModulateGlobalColorNormal= convertBool(prop);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"global_color_pushed" );
	if (prop)	_ModulateGlobalColorPushed= convertBool(prop);
	prop = (char*) xmlGetProp( cur, (xmlChar*)"global_color_over" );
	if (prop)	_ModulateGlobalColorOver= convertBool(prop);


	// *** Read Action handlers
	parseAH(cur, "onover", "params_over", _AHOnOver, _AHOverParams);
	parseAH(cur, "onclick_l", "params_l", _AHOnLeftClick, _AHLeftClickParams);
	parseAH(cur, "ondblclick_l", "params_dblclick_l", _AHOnLeftDblClick, _AHLeftDblClickParams);
	parseAH(cur, "onclick_r", "params_r", _AHOnRightClick, _AHRightClickParams);
	parseAH(cur, "onlongclick_l", "params_longclick_l", _AHOnLeftLongClick, _AHLeftLongClickParams);
	parseAH(cur, "onclock_tick", "params_clock_tick", _AHOnClockTick, _AHClockTickParams);

	// Context menu association
	prop = (char*) xmlGetProp( cur, (xmlChar*)"menu_l" );
	if (prop)
	{
		_ListMenuLeft = NLMISC::toLower(std::string((const char *) prop));
	}
	prop = (char*) xmlGetProp( cur, (xmlChar*)"menu_r" );
	if (prop)
	{
		_ListMenuRight = NLMISC::toLower(std::string((const char *) prop));
	}
	// list menu on both clicks
	prop = (char*) xmlGetProp( cur, (xmlChar*)"menu_b" );
	if (prop)
	{
		setListMenuBoth(NLMISC::toLower(std::string((const char *) prop)));
	}

	prop= (char*) xmlGetProp (cur, (xmlChar*)"frozen");
	_Frozen = false;
	if (prop)
		_Frozen = convertBool(prop);

	prop= (char*) xmlGetProp (cur, (xmlChar*)"frozen_half_tone");
	_FrozenHalfTone = true;
	if (prop)
		_FrozenHalfTone = convertBool(prop);

	return true;
}


// ***************************************************************************
void CCtrlBaseButton::setModulateGlobalColorAll(bool state)
{
	setModulateGlobalColor(state);
	setModulateGlobalColorNormal(state);
	setModulateGlobalColorPushed(state);
	setModulateGlobalColorOver(state);
}


// ***************************************************************************
bool CCtrlBaseButton::handleEvent (const NLGUI::CEventDescriptor& event)
{
	if (CCtrlBase::handleEvent(event)) return true;
	if (!_Active || _Frozen)
		return false;

	if (event.getType() == NLGUI::CEventDescriptor::mouse)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;

		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup)
		{
			if (pIM->getCapturePointerLeft() != this)
				return false;
			_LeftLongClickHandled = true;
		}

		if (!((eventDesc.getX() >= _XReal) &&
			(eventDesc.getX() < (_XReal + _WReal))&&
			(eventDesc.getY() > _YReal) &&
			(eventDesc.getY() <= (_YReal+ _HReal))))
			return false;

		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftdown)
		{
			if (_AHOnLeftDblClick)
			{
				if ((CCtrlBaseButton *) _LastLeftClickButton == this && (T1 - _LastLeftClickDate) < pIM->getUserDblClickDelay())
				{
					pIM->runActionHandler (_AHOnLeftDblClick, this, _AHLeftDblClickParams);
					_LeftDblClickHandled = true;
					_LastLeftClickButton = NULL;
					return true;
				}
			}

			if (_AHOnLeftLongClick != NULL)
			{
				_LeftLongClickHandled = false;
				_LeftLongClickDate = T1;
			}
			_LeftDblClickHandled = false;
			_LastLeftClickButton = NULL;
			return true;
		}

		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup)
		{
			if (pIM->getCapturePointerLeft() != this)
				return false;

			if (_LeftDblClickHandled)  // no effect on mouse up after double click has been handled
			{
				_LeftDblClickHandled = false;
				return true;
			}

			// Do not launch 2 times action handler if we are already pushed ! except if we want.
			if (!_ClickWhenPushed)
			{
				if ((_Type == RadioButton) && _RBRef && (*_RBRef == this))
					return true;
			}

			if (_Type == RadioButton)
			{
				_Pushed = true;
				if(_RBRef)	*_RBRef = this;
			}

			if (_Type == ToggleButton)
				_Pushed = !_Pushed;

			/*
			// RunAction
			if(_AHOnLeftClick != NULL)
			{
				//nlinfo("clicked on %s", _Id.c_str());
				pIM->submitEvent ("button_click:"+getId());//TEMP
				pIM->runActionHandler (_AHOnLeftClick, this, _AHLeftClickParams);
				//pIM->submitEvent ("button_click:"+getId());
			}
			*/
			runLeftClickAction();
			if (pIM->getCapturePointerLeft() == NULL) return true; // event handler may release cpature from this object (if it is removed for example)

			// Run Menu
			if (!_ListMenuLeft.empty())
				CWidgetManager::getInstance()->enableModalWindow (this, _ListMenuLeft);

			if (_AHOnLeftDblClick != NULL)
			{
				_LastLeftClickDate   = T1;
				_LastLeftClickButton = this;
			}

			// Always return true on LeftClick.
			return true;
		}
		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightdown)
		{
			_LastLeftClickButton = NULL;
			return true;
		}
		if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightup)
		{
			_LastLeftClickButton = NULL;
			bool	handled= false;
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			if (pIM->getCapturePointerRight() != this)
				return false;

			// RunAction
			if(_AHOnRightClick != NULL)
			{
				handled= true;
				pIM->runActionHandler (_AHOnRightClick, this, _AHRightClickParams);
			}
			if (pIM->getCapturePointerRight() == NULL) return true; // if this become NULL, this ctrl has been deleted
			// Run Menu
			if (!_ListMenuRight .empty())
			{
				handled= true;
				CWidgetManager::getInstance()->enableModalWindow (this, _ListMenuRight);
			}
			// If not handled here, ret to parent
			return handled;
		}


	}
	else if (event.getType() == NLGUI::CEventDescriptor::system)
	{
		const NLGUI::CEventDescriptorSystem &systemEvent = (const NLGUI::CEventDescriptorSystem &) event;
		if (systemEvent.getEventTypeExtended() == NLGUI::CEventDescriptorSystem::clocktick)
		{
			if (_AHOnClockTick != NULL)
			{
				CInterfaceManager *pIM = CInterfaceManager::getInstance();
				pIM->runActionHandler(_AHOnClockTick, this, _AHClockTickParams);
			}

			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			if (pIM->getCapturePointerLeft() == this)
			{
				if (!_LeftLongClickHandled)
				{
					uint nVal = 50;
					CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:KEY_REPEAT_SPEED");
					if (pNL != NULL)
						nVal = pNL->getValue32();
					uint repeatDelay = (uint)(KEY_REPEAT_MIN + (KEY_REPEAT_MAX-KEY_REPEAT_MIN) * (float)nVal / 100.0f);
					if ((T1 - _LeftLongClickDate) > repeatDelay)
					{
						_LeftLongClickHandled = true;
						pIM->runActionHandler(_AHOnLeftLongClick, this, _AHLeftLongClickParams);
					}
				}
			}
		}
	}
	return false;
}

// ***************************************************************************
void CCtrlBaseButton::initRBRef()
{
	if (_RBRef != NULL) return;
	nlassert(_Parent);
	const vector<CCtrlBase*> &vCB = _Parent->getControls();
	uint i = 0;
	for (i = 0; i < vCB.size(); ++i)
	{
		CCtrlBaseButton *pBut = dynamic_cast<CCtrlBaseButton*>(vCB[i]);
		if (pBut && pBut->_Type == RadioButton)
		{
			_RBRef = &pBut->_RBRefBut;
			break;
		}
	}
	// If we are the first radio button of the group and not added
	if (i == vCB.size())
		_RBRef = &this->_RBRefBut;
}

// ***************************************************************************
void CCtrlBaseButton::initRBRefFromRadioButton(CCtrlBaseButton * pBut)
{
	if(pBut && pBut->_Type == RadioButton)
	{
		_RBRef = &(pBut->_RBRefBut);
		_RBRefBut=NULL;
	}
}


// ***************************************************************************
void CCtrlBaseButton::setPushed (bool state)
{
	_Pushed = state;

	if (_Type == RadioButton && _RBRef)
	{
		if (state == true)
		{
			*_RBRef = this;
		}
		else
		{
			if (*_RBRef == this)	// I have to be pushed to unpush me
				*_RBRef = NULL;		// After that : All radio buttons are NOT pushed
		}
	}
}

// ***************************************************************************
void CCtrlBaseButton::setFrozen (bool state)
{
	_Frozen = state;
	if (_Frozen)
	{
		_Pushed = false;
		_Over = false;
	}
}

// ***************************************************************************
void CCtrlBaseButton::setFrozenHalfTone(bool enabled)
{
	_FrozenHalfTone = enabled;
}

// ***************************************************************************
void CCtrlBaseButton::unselect()
{
	if (_Type == RadioButton)
	{
		if (_RBRef) *_RBRef = NULL;
	}
}


// ***************************************************************************
void CCtrlBaseButton::updateOver(bool &lastOver)
{
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	if (!pIM->isMouseHandlingEnabled())
	{
		_Over = false;
		return;
	}

	if (pIM->getCapturePointerLeft() != NULL)
	{
		if (pIM->getCapturePointerLeft() != this)
		{
			_Over = false;
		}
		return;
	}

	const vector<CCtrlBase*> &rVB = pIM->getCtrlsUnderPointer ();

	if (!_Frozen)
	{
		uint32 i;
		lastOver = _Over;
		// show over if it is the last control that has the same father
		CCtrlBase *candidate = NULL;
		for (i = 0; i < rVB.size(); ++i)
		{
			if (rVB[i]->getParent() == this->getParent())
			{
				candidate = rVB[i];
			}
		}
		_Over = (candidate == this);
	}
	else
		_Over = false;


}

// ***************************************************************************
void CCtrlBaseButton::elementCaptured(CCtrlBase *capturedElement)
{
	// if not me, then reset my '_Over'
	if (capturedElement != this)
	{
		_Over = false;
	}
}


// ***************************************************************************
void CCtrlBaseButton::runLeftClickAction()
{
	if(_AHOnLeftClick != NULL)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();

		//nlinfo("clicked on %s", _Id.c_str());
		pIM->submitEvent ("button_click:"+getId());//TEMP
		pIM->runActionHandler (_AHOnLeftClick, this, _AHLeftClickParams);
		//pIM->submitEvent ("button_click:"+getId());
	}
}

// ***************************************************************************
int CCtrlBaseButton::luaRunLeftClickAction(CLuaState &ls)
{
	const char *funcName = "onLeftClick";
	CLuaIHM::checkArgCount(ls, funcName, 0);

	runLeftClickAction();

	return 0;
}

