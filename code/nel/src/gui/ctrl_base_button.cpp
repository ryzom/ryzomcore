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
#include "nel/gui/ctrl_base_button.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/lua_ihm.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/db_manager.h"


using namespace std;
using namespace NLMISC;

namespace
{
	const uint KEY_REPEAT_MIN = 100;
	const uint KEY_REPEAT_MAX = 750;
}

namespace NLGUI
{

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

	std::string CCtrlBaseButton::getProperty( const std::string &name ) const
	{
		if( name == "button_type" )
		{
			return getTypeString();
		}
		else
		if( name == "pushed" )
		{
			return toString( _Pushed );
		}
		else
		if( name == "over_when_pushed" )
		{
			return toString( _OverWhenPushed );
		}
		else
		if( name == "clicked_when_pushed" )
		{
			return toString( _ClickWhenPushed );
		}
		else
		if( name == "color" )
		{
			return getColorAsString();
		}
		else
		if( name == "col_pushed" )
		{
			return getColorPushedAsString();
		}
		else
		if( name == "col_over" )
		{
			return getColorOverAsString();
		}
		else
		if( name == "global_color_normal" )
		{
			return toString( _ModulateGlobalColorNormal );
		}
		else
		if( name == "global_color_pushed" )
		{
			return toString( _ModulateGlobalColorPushed );
		}
		else
		if( name == "global_color_over" )
		{
			return toString( _ModulateGlobalColorOver );
		}
		else
		if( name == "onover" )
		{
			return getAHString( "onover" );
		}
		else
		if( name == "params_over" )
		{
			return _getParamsOnOver();
		}
		else
		if( name == "onclick_l" )
		{
			return getAHString( "onclick_l" );
		}
		else
		if( name == "params_l" )
		{
			return _getParamsOnLeftClick();
		}
		else
		if( name == "ondblclick_l" )
		{
			return getAHString( "ondblclick_l" );
		}
		else
		if( name == "params_dblclick_l" )
		{
			return _AHLeftDblClickParams.toString();
		}
		else
		if( name == "onlongclick_l" )
		{
			return getAHString( "onlongclick_l" );
		}
		else
		if( name == "params_longclick_l" )
		{
			return _AHLeftLongClickParams.toString();
		}
		else
		if( name == "onclick_r" )
		{
			return getAHString( "onclick_r" );
		}
		else
		if( name == "params_r" )
		{
			return _AHRightClickParams.toString();
		}
		else
		if( name == "onclock_tick" )
		{
			return getAHString( "onclock_tick" );
		}
		else
		if( name == "params_clock_tick" )
		{
			return _AHClockTickParams.toString();
		}
		else
		if( name == "menu_l" )
		{
			return _ListMenuLeft.toString();
		}
		else
		if( name == "menu_r" )
		{
			return _ListMenuRight.toString();
		}
		else
		if( name == "menu_b" )
		{
			if( _ListMenuLeft.toString() == _ListMenuRight.toString() )
				return _ListMenuLeft.toString();
			else
				return "";
		}
		else
		if( name == "frozen" )
		{
			return toString( _Frozen );
		}
		else
		if( name == "frozen_half_tone" )
		{
			return toString( _FrozenHalfTone );
		}
		else
			return CCtrlBase::getProperty( name );
	}

	void CCtrlBaseButton::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "button_type" )
		{
			setTypeFromString( value );
			return;
		}
		else
		if( name == "pushed" )
		{
			bool b;
			if( fromString( value, b ) )
				_Pushed = b;
			return;
		}
		else
		if( name == "over_when_pushed" )
		{
			bool b;
			if( fromString( value, b ) )
				_OverWhenPushed = b;
			return;
		}
		else
		if( name == "clicked_when_pushed" )
		{
			bool b;
			if( fromString( value, b ) )
				_ClickWhenPushed = b;
			return;
		}
		else
		if( name == "color" )
		{
			setColorAsString( value );
			return;
		}
		else
		if( name == "col_pushed" )
		{
			setColorPushedAsString( value );
			return;
		}
		else
		if( name == "col_over" )
		{
			setColorOverAsString( value );
			return;
		}
		else
		if( name == "global_color_normal" )
		{
			bool b;
			if( fromString( value, b ) )
				_ModulateGlobalColorNormal = b;
			return;
		}
		else
		if( name == "global_color_pushed" )
		{
			bool b;
			if( fromString( value, b ) )
				_ModulateGlobalColorPushed = b;
			return;
			
		}
		else
		if( name == "global_color_over" )
		{
			bool b;
			if( fromString( value, b ) )
				_ModulateGlobalColorOver = b;
			return;
		}
		else
		if( name == "onover" )
		{
			std::string dummy;
			_AHOnOver = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( "onover", value );
			return;
		}
		else
		if( name == "params_over" )
		{
			_AHOverParams = value;
			return;
		}
		else
		if( name == "onclick_l" )
		{
			std::string dummy;
			_AHOnLeftClick = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( "onclick_l", value );
			return;
		}
		else
		if( name == "params_l" )
		{
			_AHLeftClickParams = value;
			return;
		}
		else
		if( name == "ondblclick_l" )
		{
			std::string dummy;
			_AHOnLeftDblClick = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( "ondblclick_l", value );
			return;
		}
		else
		if( name == "params_dblclick_l" )
		{
			_AHLeftDblClickParams = value;
			return;
		}
		else
		if( name == "onlongclick_l" )
		{
			std::string dummy;
			_AHOnLeftLongClick = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( "onlongclick_l", value );
			return;
		}
		else
		if( name == "params_longclick_l" )
		{
			_AHLeftLongClickParams = value;
			return;
		}
		else
		if( name == "onclick_r" )
		{
			std::string dummy;
			_AHOnRightClick = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( "onclick_r", value );
			return;
		}
		else
		if( name == "params_r" )
		{
			_AHRightClickParams = value;
			return;
		}
		else
		if( name == "onclock_tick" )
		{
			std::string dummy;
			_AHOnClockTick = CAHManager::getInstance()->getAH( value, dummy );
			mapAHString( "onclock_tick", value );
			return;
		}
		else
		if( name == "params_clock_tick" )
		{
			_AHClockTickParams = value;
			return;
		}
		else
		if( name == "menu_l" )
		{
			_ListMenuLeft = value;
			return;
		}
		else
		if( name == "menu_r" )
		{
			_ListMenuRight = value;
			return;
		}
		else
		if( name == "menu_b" )
		{
			_ListMenuLeft = _ListMenuRight = value;
			return;
		}
		else
		if( name == "frozen" )
		{
			bool b;
			if( fromString( value, b ) )
				_Frozen = b;
			return;
		}
		else
		if( name == "frozen_half_tone" )
		{
			bool b;
			if( fromString( value, b ) )
				_FrozenHalfTone = b;
			return;
		}
		else
			CCtrlBase::setProperty( name, value );
	}


	xmlNodePtr CCtrlBaseButton::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CCtrlBase::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "button" );
		xmlNewProp( node, BAD_CAST "button_type", BAD_CAST getTypeString().c_str() );
		xmlNewProp( node, BAD_CAST "pushed", BAD_CAST toString( _Pushed ).c_str() );
		xmlNewProp( node, BAD_CAST "over_when_pushed", BAD_CAST toString( _OverWhenPushed ).c_str() );
		xmlNewProp( node, BAD_CAST "clicked_when_pushed", BAD_CAST toString( _ClickWhenPushed ).c_str() );
		xmlNewProp( node, BAD_CAST "color", BAD_CAST getColorAsString().c_str() );
		xmlNewProp( node, BAD_CAST "col_pushed", BAD_CAST getColorPushedAsString().c_str() );
		xmlNewProp( node, BAD_CAST "col_over", BAD_CAST getColorOverAsString().c_str() );
		xmlNewProp( node, BAD_CAST "global_color_normal", BAD_CAST toString( _ModulateGlobalColorNormal ).c_str() );
		xmlNewProp( node, BAD_CAST "global_color_pushed", BAD_CAST toString( _ModulateGlobalColorPushed ).c_str() );
		xmlNewProp( node, BAD_CAST "global_color_over", BAD_CAST toString( _ModulateGlobalColorOver ).c_str() );

		xmlNewProp( node, BAD_CAST "onover", BAD_CAST getAHString( "onover" ).c_str() );
		xmlNewProp( node, BAD_CAST "params_over", BAD_CAST _getParamsOnOver().c_str() );
		xmlNewProp( node, BAD_CAST "onclick_l", BAD_CAST getAHString( "onclick_l" ).c_str() );
		xmlNewProp( node, BAD_CAST "params_l", BAD_CAST _getParamsOnLeftClick().c_str() );
		xmlNewProp( node, BAD_CAST "ondblclick_l", BAD_CAST getAHString( "ondblclick_l" ).c_str() );
		xmlNewProp( node, BAD_CAST "params_dblclick_l", BAD_CAST _AHLeftDblClickParams.toString().c_str() );
		xmlNewProp( node, BAD_CAST "onlongclick_l", BAD_CAST getAHString( "onlongclick_l" ).c_str() );
		xmlNewProp( node, BAD_CAST "params_longclick_l", BAD_CAST _AHLeftLongClickParams.toString().c_str() );
		xmlNewProp( node, BAD_CAST "onclick_r", BAD_CAST getAHString( "onclick_r" ).c_str() );
		xmlNewProp( node, BAD_CAST "params_r", BAD_CAST _AHRightClickParams.toString().c_str() );
		xmlNewProp( node, BAD_CAST "onclock_tick", BAD_CAST getAHString( "onclock_tick" ).c_str() );
		xmlNewProp( node, BAD_CAST "params_clock_tick", BAD_CAST _AHClockTickParams.toString().c_str() );
		
		
		xmlNewProp( node, BAD_CAST "menu_l", BAD_CAST _ListMenuLeft.toString().c_str() );
		xmlNewProp( node, BAD_CAST "menu_r", BAD_CAST _ListMenuRight.toString().c_str() );

		if( _ListMenuLeft.toString() == _ListMenuRight.toString() )
			xmlNewProp( node, BAD_CAST "menu_b", BAD_CAST _ListMenuLeft.toString().c_str() );
		else
			xmlNewProp( node, BAD_CAST "menu_b", BAD_CAST "" );

		xmlNewProp( node, BAD_CAST "frozen", BAD_CAST toString( _Frozen ).c_str() );
		xmlNewProp( node, BAD_CAST "frozen_half_tone", BAD_CAST toString( _FrozenHalfTone ).c_str() );

		return node;
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
		if (prop)
		{
			type = (const char*) prop;
		}
		
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
		CAHManager::getInstance()->parseAH(cur, "onover", "params_over", _AHOnOver, _AHOverParams);
		CAHManager::getInstance()->parseAH(cur, "onclick_l", "params_l", _AHOnLeftClick, _AHLeftClickParams);
		CAHManager::getInstance()->parseAH(cur, "ondblclick_l", "params_dblclick_l", _AHOnLeftDblClick, _AHLeftDblClickParams);
		CAHManager::getInstance()->parseAH(cur, "onclick_r", "params_r", _AHOnRightClick, _AHRightClickParams);
		CAHManager::getInstance()->parseAH(cur, "onlongclick_l", "params_longclick_l", _AHOnLeftLongClick, _AHLeftLongClickParams);
		CAHManager::getInstance()->parseAH(cur, "onclock_tick", "params_clock_tick", _AHOnClockTick, _AHClockTickParams);

		if( editorMode )
		{
			prop = (char*) xmlGetProp( cur, BAD_CAST "onover" );
			if (prop)
				mapAHString( "onover", std::string( (const char*)prop ) );

			prop = (char*) xmlGetProp( cur, BAD_CAST "onclick_l" );
			if (prop)
				mapAHString( "onclick_l", std::string( (const char*)prop ) );

			prop = (char*) xmlGetProp( cur, BAD_CAST "ondblclick_l" );
			if (prop)
				mapAHString( "ondblclick_l", std::string( (const char*)prop ) );

			prop = (char*) xmlGetProp( cur, BAD_CAST "onclick_r" );
			if (prop)
				mapAHString( "onclick_r", std::string( (const char*)prop ) );

			prop = (char*) xmlGetProp( cur, BAD_CAST "onlongclick_l" );
			if (prop)
				mapAHString( "onlongclick_l", std::string( (const char*)prop ) );

			prop = (char*) xmlGetProp( cur, BAD_CAST "onclock_tick" );
			if (prop)
				mapAHString( "onclock_tick", std::string( (const char*)prop ) );
		}

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

		sint64 T1 = NLMISC::CTime::getLocalTime();

		if (event.getType() == NLGUI::CEventDescriptor::mouse)
		{
			const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;

			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftup)
			{
				if (CWidgetManager::getInstance()->getCapturePointerLeft() != this)
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
					if ((CCtrlBaseButton *) _LastLeftClickButton == this && (T1 - _LastLeftClickDate) < CWidgetManager::getInstance()->getUserDblClickDelay())
					{
						CAHManager::getInstance()->runActionHandler (_AHOnLeftDblClick, this, _AHLeftDblClickParams);
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
				if (CWidgetManager::getInstance()->getCapturePointerLeft() != this)
					return false;

				if( editorMode )
				{
					CWidgetManager::getInstance()->setCurrentEditorSelection( getId() );
					return true;
				}

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
					CAHManager::getInstance()->runActionHandler (_AHOnLeftClick, this, _AHLeftClickParams);
					//pIM->submitEvent ("button_click:"+getId());
				}
				*/
				runLeftClickAction();
				if (CWidgetManager::getInstance()->getCapturePointerLeft() == NULL) return true; // event handler may release cpature from this object (if it is removed for example)

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
				if (CWidgetManager::getInstance()->getCapturePointerRight() != this)
					return false;

				// RunAction
				if(_AHOnRightClick != NULL)
				{
					handled= true;
					CAHManager::getInstance()->runActionHandler (_AHOnRightClick, this, _AHRightClickParams);
				}
				if (CWidgetManager::getInstance()->getCapturePointerRight() == NULL) return true; // if this become NULL, this ctrl has been deleted
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
					CAHManager::getInstance()->runActionHandler(_AHOnClockTick, this, _AHClockTickParams);
				}

				if (CWidgetManager::getInstance()->getCapturePointerLeft() == this)
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
							CAHManager::getInstance()->runActionHandler(_AHOnLeftLongClick, this, _AHLeftLongClickParams);
						}
					}
				}
			}
		}
		return false;
	}

	std::string CCtrlBaseButton::getTypeString() const
	{
		switch( _Type )
		{
		case PushButton:
			return "push_button";
			break;

		case ToggleButton:
			return "toggle_button";
			break;

		case RadioButton:
			return "radio_button";
			break;

		default:
			break;
		}

		return "";
	}

	void CCtrlBaseButton::setTypeFromString( const std::string &type )
	{
		if( type.empty() || type == "toggle_button" )
		{
			_Type = ToggleButton;
			return;
		}
		else
		if( type == "push_button" )
		{
			_Type = PushButton;
			return;
		}
		else
		if( type == "radio_button" )
		{
			_Type = RadioButton;

			initRBRef();
			if( _Pushed )
				*_RBRef = this;

			return;
		}
		else
		{
			_Type = ToggleButton;
			nlinfo( ( "cannot parse button type for button " + getId() ).c_str() );
		}
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
		if (!CWidgetManager::getInstance()->isMouseHandlingEnabled())
		{
			_Over = false;
			return;
		}

		if (CWidgetManager::getInstance()->getCapturePointerLeft() != NULL)
		{
			if (CWidgetManager::getInstance()->getCapturePointerLeft() != this)
			{
				_Over = false;
			}
			return;
		}

		const vector<CCtrlBase*> &rVB = CWidgetManager::getInstance()->getCtrlsUnderPointer ();

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

			//nlinfo("clicked on %s", _Id.c_str());
			CAHManager::getInstance()->submitEvent( "button_click:" + getId() );
			CAHManager::getInstance()->runActionHandler (_AHOnLeftClick, this, _AHLeftClickParams);
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

}

